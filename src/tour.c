/* tour.c -- ROM extension for coach tours */

/* (C) 1997 Elrac of DizzyMUD.

   If you want to use this code, ask Elrac ( carls@ipf.de ).

*/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
/* added stdlib to avoid errors from exit() function call
   -- Outsider
*/
#include <stdlib.h>
//#include<time.h>
#include "merc.h"

/* Passengers */
#define TOUR_MAX_PAX            4

/* Tour message (ACT) types */
#define TOUR_MSG_COACH          A
#define TOUR_MSG_HORSE          B
#define TOUR_MSG_DRIVER         C

/* Tour data structures */
typedef struct  tourstart_data          TOURSTART_DATA;
typedef struct  tourroute_data          TOURROUTE_DATA;
typedef struct  tourstep_data           TOURSTEP_DATA;

struct tourstart_data {
	TOURSTART_DATA *next;
	int room_vnum;
	char *desc;
	TOURROUTE_DATA *tours;
	int ntours;
};

struct tourroute_data {
	TOURROUTE_DATA *next;
	char *destination;
	int cost;
	TOURSTEP_DATA *steps, *where;
	int coach_vnum, horse_vnum, driver_vnum;
	OBJ_DATA *coach;
	CHAR_DATA *horse, *driver;
	int timer;
	int npax;
	CHAR_DATA *pax[TOUR_MAX_PAX];
};

struct tourstep_data {
	TOURSTEP_DATA *next;
	int room_vnum;
	int msg_flags;
	char *msg_coach;
	char *msg_horse;
	char *msg_driver;
};

/* Tour-related data */
TOURSTART_DATA *tourstart_list;

/*
 * Load tour routes for Thera Tours coach
 *
 * Format:
 * -----------------------------------
 * #TOURROUTES
 *
 * * comment
 * #start_vnum coach_vnum horse_vnum driver_vnum price
 * destination~
 * * comment
 * >room_vnum msg_flags * comment
 * msg_coach~  }
 * msg_horse~  > if corresponding flag bit set
 * msg_driver~ }
 * * comment
 * >room_vnum msg_flags * comment
 * msg_coach~  }
 * msg_horse~  > if corresponding flag bit set
 * msg_driver~ }
 * ...
 * * comment
 * >room_vnum msg_flags *comment
 * msg_coach~  }
 * msg_horse~  > if corresponding flag bit set
 * msg_driver~ }
 * >0
 *
 * * comment
 * #start_vnum coach_vnum destination~
 * >room_vnum msg_flags *comment
 * ...
 * >0
 *
 * #0
 * -----------------------------------
 */
void load_tourroutes(FILE *fp)
{
	char letter;
	int startv, roomv;
	TOURSTART_DATA *pStart;
	TOURROUTE_DATA *pRoute = NULL;
	TOURSTEP_DATA *pLastStep = NULL;

	for (; ;) {
		letter = fread_letter(fp);

		if (letter == '*')   /* comment */
			fread_to_eol(fp);
		else if (letter == '#') { /* route start */
			startv = fread_number(fp);

			if (startv == 0)   /* end of final route */
				break;

			/* check for valid start room */
			for (pStart = tourstart_list; pStart; pStart = pStart->next) {
				if (pStart->room_vnum == startv)
					break;
			}

			if (pStart == NULL) {
				bug("No tour start location at room %d", startv);
				exit(1);
			}

			pRoute = alloc_perm(sizeof(TOURROUTE_DATA));
			pRoute->coach_vnum = fread_number(fp);

			if (get_obj_index(pRoute->coach_vnum) == NULL) {
				bug("No such vnum for coach: %d", pRoute->coach_vnum);
				exit(1);
			}

			pRoute->horse_vnum = fread_number(fp);

			if (get_mob_index(pRoute->horse_vnum) == NULL) {
				bug("No such vnum for horse: %d", pRoute->horse_vnum);
				exit(1);
			}

			pRoute->driver_vnum = fread_number(fp);

			if (get_mob_index(pRoute->driver_vnum) == NULL) {
				bug("No such vnum for driver: %d", pRoute->driver_vnum);
				exit(1);
			}

			pRoute->cost = fread_number(fp);
			pRoute->destination = fread_string(fp);
			pRoute->coach = NULL;
			pRoute->horse = NULL;
			pRoute->driver = NULL;
			pRoute->next = pStart->tours;
			pRoute->steps = pLastStep = NULL;
			pStart->tours = pRoute;
			pStart->ntours++;
		}
		else if (letter == '>') {
			if (pRoute == NULL) {
				bug("No route started yet", 0);
				exit(1);
			}

			roomv = fread_number(fp);

			if (roomv == 0) { /* end of rooms for route */
				if (pLastStep == NULL) {
					bug("Empty route!", 0);
					exit(1);
				}

				pLastStep = NULL;
				continue;
			}

			if (get_room_index(roomv) == NULL) {
				bug("No such vnum for room: %d", roomv);
				exit(1);
			}

			if (pLastStep == NULL) {
				pRoute->steps = alloc_perm(sizeof(TOURSTEP_DATA));
				pLastStep = pRoute->steps;
			}
			else {
				pLastStep->next = alloc_perm(sizeof(TOURSTEP_DATA));
				pLastStep = pLastStep->next;
			}

			pLastStep->room_vnum = roomv;
			pLastStep->msg_flags = fread_flag(fp);
			pLastStep->msg_coach =
			        (IS_SET(pLastStep->msg_flags, TOUR_MSG_COACH)) ?
			        fread_string(fp) : NULL;
			pLastStep->msg_horse =
			        (IS_SET(pLastStep->msg_flags, TOUR_MSG_HORSE)) ?
			        fread_string(fp) : NULL;
			pLastStep->msg_driver =
			        (IS_SET(pLastStep->msg_flags, TOUR_MSG_DRIVER)) ?
			        fread_string(fp) : NULL;
		}
		else {
			bug("Missing '#' line in TOURROUTES", 0);
			exit(1);
		}
	}
} /* end load_tourroutes() */


/*
 * Load list of tour starting points
 *
 * Format:
 * -----------------------------------
 * #TOURSTARTS
 * * comment
 * #start_room_vnum coach stop name~
 * ...
 * #start_room_vnum coach stop name~
 * #0
 * -----------------------------------
 */
void load_tourstarts(FILE *fp)
{
	char letter;
	int roomv;
	TOURSTART_DATA *pStart;

	for (; ;) {
		letter = fread_letter(fp);

		if (letter == '*') { /* comment */
			fread_to_eol(fp);
			continue;
		}

		if (letter != '#') { /* new tour start loc? */
			bug("Missing #start_room in TOURSTARTS", 0);
			exit(1);
		}

		roomv = fread_number(fp);

		if (roomv == 0)   /* loc room vnum 0 => end of starts */
			break;

		if (get_room_index(roomv) == NULL) {
			bug("No such vnum for starting room: %d", roomv);
			exit(1);
		}

		/* check for duplicate starting room */
		for (pStart = tourstart_list; pStart; pStart = pStart->next) {
			if (pStart->room_vnum == roomv) {
				bug("Duplicate start room number %d", roomv);
				exit(1);
			}
		}

		pStart = alloc_perm(sizeof(TOURSTART_DATA));
		pStart->room_vnum = roomv;
		pStart->desc = fread_string(fp);
		pStart->tours = NULL;
		pStart->ntours = 0;
		pStart->next = tourstart_list;
		tourstart_list = pStart;
	}
} /* end load_tourstarts() */


/* find the tour for a given coach. Reverse lookup. */
TOURROUTE_DATA *tour_for_coach(OBJ_DATA *coach)
{
	TOURSTART_DATA *pStart;
	TOURROUTE_DATA *pTour;

	for (pStart = tourstart_list; pStart; pStart = pStart->next)
		for (pTour = pStart->tours; pTour; pTour = pTour->next)
			if (pTour->coach == coach)
				return pTour;

	bug("No tour found for coach %d", coach->pIndexData->vnum);
	return NULL;
} /* end tour_for_coach() */


/* find a printable direction to the next room,
   a funky lie if no connection, or "" if buggy */
char *dir_to_next(ROOM_INDEX_DATA *here, ROOM_INDEX_DATA *there)
{
	if (here == NULL) {
		bug("NULL here", 0);
		return "";
	}

	if (there == NULL) {
		bug("NULL there", 0);
		return "";
	}

	if (here->exit[DIR_NORTH] && here->exit[DIR_NORTH]->u1.to_room == there)
		return "north";

	if (here->exit[DIR_EAST] && here->exit[DIR_EAST]->u1.to_room == there)
		return "east";

	if (here->exit[DIR_SOUTH] && here->exit[DIR_SOUTH]->u1.to_room == there)
		return "south";

	if (here->exit[DIR_WEST] && here->exit[DIR_WEST]->u1.to_room == there)
		return "west";

	if (here->exit[DIR_UP] && here->exit[DIR_UP]->u1.to_room == there)
		return "up";

	if (here->exit[DIR_DOWN] && here->exit[DIR_DOWN]->u1.to_room == there)
		return "down";

	return "through uncharted reaches of space and time";
} /* end dir_to_next() */


void step_messages(TOURROUTE_DATA *pTour, TOURSTEP_DATA *where)
{
	if (IS_SET(where->msg_flags, TOUR_MSG_COACH))
		act(where->msg_coach, pTour->driver, pTour->coach, pTour->coach, TO_ROOM);

	if (IS_SET(where->msg_flags, TOUR_MSG_HORSE))
		act(where->msg_horse, pTour->horse, pTour->horse, pTour->horse, TO_ROOM);

	if (IS_SET(where->msg_flags, TOUR_MSG_DRIVER))
		act(where->msg_driver, pTour->driver, pTour->driver, pTour->driver, TO_ROOM);
} /* end step_messages() */


char *calculate_price(TOURROUTE_DATA *pTour, int *gold, int *silver)
{
	TOURSTEP_DATA *pStep;
	int distance, price, goldprice, silverprice;
	static char longprice[50];
	/* calculate distance */
	distance = 0;

	for (pStep = pTour->where; pStep; pStep = pStep->next)
		distance++;

	/* calculate price */
	price = pTour->cost * distance;
	goldprice = price / 100;
	silverprice = price - 100 * goldprice;

	if (goldprice == 0)
		sprintf(longprice, "%d silver coin%s",
		        silverprice, (silverprice == 1) ? "" : "s");
	else if (silverprice == 0)
		sprintf(longprice, "%d gold coin%s",
		        goldprice, (goldprice == 1) ? "" : "s");
	else
		sprintf(longprice, "%d gold and %d silver coin%s",
		        goldprice, silverprice, (silverprice == 1) ? "" : "s");

	/* return values */
	if (gold)
		*gold = goldprice;

	if (silver)
		*silver = silverprice;

	return longprice;
} /* end calculate_price() */


/*
 * Tour update. Every minute or so, start coaches in all
 * start locations. Every odd 5 seconds (or so), start
 * them moving toward their destination. Every even 5
 * seconds (or so), move them toward their destinations
 * and display any ACTs for coach, horse or driver.
 */
void tour_update(void)
{
	static int start_timer = 0;
	int which_tour = 0;
	TOURSTART_DATA *pStart;
	TOURROUTE_DATA *pTour;
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *coach;
	ROOM_INDEX_DATA *start_room;
	ROOM_INDEX_DATA *this_room, *next_room;
	MOB_INDEX_DATA *pMobIndex;
	CHAR_DATA *horse, *driver, *passenger;
	char buf[MAX_STRING_LENGTH];
	char longprice[MAX_STRING_LENGTH];
	char *direction;
	int jpax;
	int gold_price;

	/* cycle through all tours for all start locations, advance all coaches */
	for (pStart = tourstart_list; pStart; pStart = pStart->next) {
		for (pTour = pStart->tours; pTour; pTour = pTour->next) {
			if (pTour->coach == NULL)
				continue;

			/* chart a course. */
			this_room = pTour->coach->in_room;
			next_room = get_room_index(pTour->where->next->room_vnum);
			direction = dir_to_next(this_room, next_room);
			pTour->timer++;

			/* Allow boarding time. Don't start before positive timer. */
			if (pTour->timer < 1) {
				if (pTour->timer == -2) {
					act("$n announces, 'leaving for $t shortly!",
					    pTour->driver, pTour->destination, NULL, TO_ROOM);
				}

				continue;
			}

			if (pTour->timer & 1) { /* odd valued timer: starts to move */
				sprintf(buf, "The coach starts to move %s.", direction);

				for (jpax = 0; jpax < pTour->npax; jpax++)
					if (pTour->pax[jpax])
						act(buf, pTour->pax[jpax], pTour->coach, pTour->coach, TO_CHAR);

				continue;
			}

			/* even valued timer: moves */
			obj_from_room(pTour->coach);
			obj_to_room(pTour->coach, next_room);
			char_from_room(pTour->horse);
			char_to_room(pTour->horse, next_room);
			act("$P arrives", pTour->horse, NULL, pTour->coach, TO_ROOM);
			char_from_room(pTour->horse);   /* horse is briefly nowhere */

			/* move all players */
			for (jpax = 0; jpax < pTour->npax; jpax++) {
				if ((passenger = pTour->pax[jpax]) == NULL)
					continue;

				char_from_room(passenger);
				char_to_room(passenger, next_room);
				passenger->on = pTour->coach;
			}

			/* LOOK all players */
			for (jpax = 0; jpax < pTour->npax; jpax++) {
				if ((passenger = pTour->pax[jpax]) == NULL)
					continue;

				do_look(passenger, "");
			}

			char_to_room(pTour->horse, next_room);   /* horse in next room now */
			act("$P leaves.", pTour->driver, NULL, pTour->coach, TO_ROOM);
			char_from_room(pTour->driver);
			char_to_room(pTour->driver, next_room);
			pTour->where = pTour->where->next;
			step_messages(pTour, pTour->where);

			if (pTour->where->next == NULL) { /* end of the line */
				if (pTour->npax == 0) {
					act("$n looks a little sad that his coach had no passengers.",
					    pTour->driver, NULL, NULL, TO_ROOM);
				}
				else {
					/* alight the passengers before blowing up the coach */
					for (jpax = 0; jpax < pTour->npax; jpax++) {
						if ((passenger = pTour->pax[jpax]) != NULL)
							do_alight(passenger, "");
					}

					check_social(pTour->driver, "wave", "");
				}

				act("The coach disappears in a shimmering haze.",
				    pTour->driver, pTour->coach, pTour->coach, TO_ROOM);
				extract_obj(pTour->coach);
				extract_char(pTour->horse, TRUE);
				extract_char(pTour->driver, TRUE);
				pTour->coach = NULL;
				pTour->horse = NULL;
				pTour->driver = NULL;
			}
		}
	}

	/* check if it's time to start more tours */
	if (--start_timer > 0)
		return;

	/* Restart timer and send off new coaches */
	start_timer = PULSE_TOURSTART / PULSE_TOURHALFSTEP;

	for (pStart = tourstart_list; pStart; pStart = pStart->next) {
		if (pStart->ntours <= 0)
			continue;

		start_room = get_room_index(pStart->room_vnum);
		/* choose a destination at random */
		which_tour = number_range(0, pStart->ntours - 1);

		for (pTour = pStart->tours; which_tour > 0; which_tour--)
			pTour = pTour->next;

		if (pTour->coach != NULL)   /* tour still in progress? */
			continue;

		/* create coach */
		pObjIndex = get_obj_index(pTour->coach_vnum);
		coach = create_object(pObjIndex, 92);
		/* short description */
		sprintf(buf, coach->short_descr, pTour->destination);
		free_string(coach->short_descr);
		coach->short_descr = str_dup(buf);
		/* (long) description */
		sprintf(buf, coach->description, pTour->destination);
		free_string(coach->description);
		coach->description = str_dup(buf);
		/* finished. place it at the coach stop. */
		obj_to_room(coach, start_room);
		pTour->coach = coach;
		pTour->timer = -PULSE_TOURBOARD / PULSE_TOURHALFSTEP;
		pTour->npax = 0;
		pTour->where = pTour->steps;
		/* create horse */
		pMobIndex = get_mob_index(pTour->horse_vnum);
		horse = create_mobile(pMobIndex);
		char_to_room(horse, start_room);
		pTour->horse = horse;
		/* create driver */
		pMobIndex = get_mob_index(pTour->driver_vnum);
		driver = create_mobile(pMobIndex);
		char_to_room(driver, start_room);
		pTour->driver = driver;
		/* announce */
		sprintf(buf, "A {PThera Tours{x coach pulls up to the %s.", pStart->desc);
		act(buf, driver, NULL, NULL, TO_ROOM);
		step_messages(pTour, pTour->where);
		strcpy(longprice, calculate_price(pTour, &gold_price, NULL));
		sprintf(buf, "$n cheerfully announces:\r\n   'To %s, %s%s!'",
		        pTour->destination, ((gold_price < 10) ? "only " : ""), longprice);
		act(buf, driver, NULL, NULL, TO_ROOM);
	}
} /* end tour_update() */


void do_board(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *coach;
	CHAR_DATA *rch, *pet, *follower;
	TOURROUTE_DATA *pTour = NULL;
	int price, goldprice, silverprice;
	char buf[MAX_STRING_LENGTH];
	char longprice[MAX_STRING_LENGTH];
	int pets, followers;

	for (coach = ch->in_room->contents; coach; coach = coach->next_content)
		if (coach->item_type == ITEM_COACH)
			break;

	if (coach == NULL) {
		stc("There is no coach here for you to board.\n\r", ch);
		return;
	}

	if (ch->on == coach) {
		stc("You are already on the coach.\n\r", ch);
		return;
	}

	/* find the tour for this coach */
	pTour = tour_for_coach(coach);

	if (pTour == NULL) {
		stc("This coach seems to have broken down.\n\r", ch);
		return;
	}

	if (pTour->npax >= TOUR_MAX_PAX) {
		stc("The coach is already full, sorry!\n\r", ch);
		return;
	}

	/* Find distance and price. */
	strcpy(longprice, calculate_price(pTour, &goldprice, &silverprice));
	price = 100 * goldprice + silverprice;

	if (ch->silver + 100 * ch->gold < price) {
		sprintf(buf, "The price for the ride is %s.\n\r", longprice);
		stc(buf, ch);
		stc("You don't have that much, sorry!\n\r", ch);
		return;
	}

	/* pay */
	if (ch->silver < silverprice) {
		ch->silver += 100;
		ch->gold--;
	}

	ch->silver -= silverprice;
	ch->gold -= goldprice;
	sprintf(buf, "You pay the driver %s for your ride to %s.\n\r",
	        longprice, pTour->destination);
	stc(buf, ch);

	if (ch->invis_level < LEVEL_IMMORTAL) {
		act("$n pays the driver for the ride.", ch, NULL, NULL, TO_ROOM);
		check_social(pTour->driver, "thank", ch->name);
	}

	pTour->pax[pTour->npax++] = ch;
	ch->position = POS_SITTING;
	ch->on = coach;
	stc("You board the coach and have a seat.\n\r", ch);
	/* look for his pet and followers. */
	pets = 0;
	pet = NULL;
	followers = 0;
	follower = NULL;

	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if (rch == ch->pet) {
			pet = rch;
			pets++;
		}
		else if (rch->master == ch) {
			follower = rch;
			followers++;
			act("You will need to BOARD if you want to follow $N onto the coach.",
			    rch, NULL, ch, TO_CHAR);
		}
	}

	/* check for pets and/or followers */
	if (pets)
		act("$N is not allowed on the coach, alas.", ch, NULL, pet, TO_CHAR);

	if (followers == 1)
		act("$N will need to buy $S own ticket if $E wants to follow you.",
		    ch, NULL, follower, TO_CHAR);

	if (followers > 1)
		act("Your followers will need to buy their own tickets if they want to follow you.",
		    ch, NULL, NULL, TO_CHAR);

	if (ch->invis_level < LEVEL_IMMORTAL)
		act("$n boards the coach and sits down in it.", ch, NULL, NULL, TO_ROOM);
} /* end do_board() */


void do_alight(CHAR_DATA *ch, char *argument)
{
	int jpax;
	OBJ_DATA *coach;
	TOURROUTE_DATA *pTour = NULL;
	bool found = FALSE;
	char longprice[MAX_STRING_LENGTH];
	int goldprice, silverprice;

	if (!ch->on || ch->on->item_type != ITEM_COACH) {
		stc(
		        "You are not on a coach, so you cannot alight from one.\n\r",
		        ch);
		return;
	}

	coach = ch->on;
	ch->position = POS_STANDING;
	ch->on = NULL;
	pTour = tour_for_coach(coach);

	if (pTour && pTour->where && pTour->where->next) {
		strcpy(longprice, calculate_price(pTour, &goldprice, &silverprice));
		ch->gold += goldprice;
		ch->silver += silverprice;
		act("'Driver,' you say, 'Please stop, I would like to get off!'",
		    ch, NULL, NULL, TO_CHAR);
		act("$n briefly stops the coach to drop you off.",
		    pTour->driver, NULL, ch, TO_VICT);
		act("The driver reimburses you $t for the remainder of the tour.",
		    ch, longprice, NULL, TO_CHAR);

		if (ch->invis_level < LEVEL_IMMORTAL) {
			act("$n asks the driver to stop so $e can leave the tour.",
			    ch, NULL, NULL, TO_ROOM);
			act("$n briefly stops the coach to drop $N off.",
			    pTour->driver, NULL, ch, TO_NOTVICT);
			act("The driver reimburses $n for the remainder of the tour.",
			    ch, NULL, NULL, TO_NOTVICT);
		}
	}

	stc(
	        "You alight from the coach and stand back to avoid being hurt.\n\r",
	        ch);

	if (ch->invis_level < LEVEL_IMMORTAL)
		act("$n alights from the coach and stands back.", ch, NULL, NULL, TO_ROOM);

	if (pTour == NULL)
		return;

	for (jpax = 0; jpax < pTour->npax; jpax++) {
		if (pTour->pax[jpax] == ch) {
			found = TRUE;
			pTour->npax--;

			if (jpax < pTour->npax)
				pTour->pax[jpax] = pTour->pax[pTour->npax];

			pTour->pax[pTour->npax] = NULL;
		}

		break;
	}

	if (!found) {
		bug("Can't find passenger on his route.", 0);
		return;
	}
} /* end do_alight() */

