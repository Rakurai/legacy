/* hunt.c -- implementation of the HUNT command by Elrac, Jan 1999 */

/*
    This code supersedes a previous implementation of HUNT which I
    found in Dizzy and which seems to be overly complicated and
    probably very slow.

    The new algorithm is my own variation on the popular breadth first
    search. The required dynamic data structures are built from
    members of the 'room_index_data' structure, so no dynamically
    allocated memory is needed.

    This code requires the following additions to the room_index_data
    structure:

    int                 hunt_id;    // unique id for current hunt
    ROOM_INDEX_DATA *   hunt_next;  // next room in search circle
    ROOM_INDEX_DATA *   hunt_back;  // pointer back toward origin

    'hunt_next' is used to connect rooms in concentric rings growing
    outward from the origin and having equal distance from the origin.

    'hunt_back', for any room already explored in the current hunt,
    points back to the room closer to the origin from which we came
    to the given room.

    'hunt_id' is used to tell whether the data stored in a given
    room pertains to the current hunt.

    Using a hunt key allows us to explore without needing to first
    initialize the entire room list.
*/

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
//#include<time.h>

#include "merc.h"


/* *** GLOBAL VARIABLES *** */

DECLARE_DO_FUN(do_open);
DECLARE_DO_FUN(do_say);

extern const char *dir_name[];


/* *** LOCAL VARIABLES *** */

/* Number of exits is stupidly hardcoded in Merc.
   If it changes, this line needs to be fixed up. */
static const int exit_num = 6;

struct hunt_conditions {
	/* first few fields set up by caller of find_path */
	CHAR_DATA *hunter;
	ROOM_INDEX_DATA *from_room;
	ROOM_INDEX_DATA *to_room;
	int same_area;
	int thru_doors;
	int  steps;
	/* these fields completed by find_path */
	AREA_DATA *area;
};
typedef struct hunt_conditions HUNT_CONDITIONS;


/* Returns the room accessible via exit 'ex' or NULL. */
static ROOM_INDEX_DATA *access_room(HUNT_CONDITIONS *cond, EXIT_DATA *ex)
{
	ROOM_INDEX_DATA *new_room;

	if ((ex == NULL) ||
	    ((new_room = ex->u1.to_room) == NULL) ||
	    (cond->same_area && new_room->area != cond->area) ||
	    (!cond->thru_doors && IS_SET(ex->exit_info, EX_CLOSED)) ||
	    (!can_see_room(cond->hunter, new_room)))
		return NULL;

	return new_room;
} /* end access_room() */


/* find the shortest path from cond->from_room to ->to_room
   and return the direction of the first step or -1. */
static int find_path(HUNT_CONDITIONS *cond)
{
	ROOM_INDEX_DATA *this_ring;
	ROOM_INDEX_DATA *next_ring;
	int jdir;
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA *new_room;
	ROOM_INDEX_DATA *step;
	static int hunt_id = 1;
	hunt_id++;
	cond->area      = cond->from_room->area;
	cond->from_room->hunt_id   = hunt_id;
	cond->from_room->hunt_back = NULL;
	cond->from_room->hunt_next = NULL;
	this_ring = cond->from_room;
	next_ring = NULL;

	/* loop through all rooms in the current ring, spiral outwards */
	while (this_ring != NULL) {
		/* explore all possible exits */
		for (jdir = 0; jdir < exit_num; jdir++) {
			pexit = this_ring->exit[jdir];
			new_room = access_room(cond, pexit);

			if ((new_room != NULL) && (new_room->hunt_id != hunt_id)) {
				new_room->hunt_id   = hunt_id;
				new_room->hunt_back = this_ring;
				new_room->hunt_next = next_ring;
				next_ring = new_room;

				if (new_room == cond->to_room)
					goto foundit;
			}
		}

		if (this_ring->hunt_next != NULL) {
			/* move to next room in current ring */
			this_ring = this_ring->hunt_next;
		}
		else {
			/* move outward */
			this_ring = next_ring;
			next_ring = NULL;

			if (--cond->steps <= 0) {
				/* too many steps */
				return -2;
			}
		}
	}

	/* fallen out of loop? hunt failed */
	return -1;
	/* hunt successful. backtrack from goal room to start room
	   to find first step. */
foundit:
	cond->steps = 1;
	step = cond->to_room;

	while (step->hunt_back != cond->from_room) {
		step = step->hunt_back;

		if (step == NULL) {
			return -3;  /* backtrack dead ends! */
		}

		cond->steps += 1;

		if (cond->steps > 1000) {
			return -4;  /* backtrack loops! */
		}
	}

	if (step->hunt_back != cond->from_room) {
		return -5;      /* backtrack doesn't return to origin! */
	}

	/* find path from start room to second room */
	for (jdir = 0; jdir < exit_num; jdir++) {
		pexit = cond->from_room->exit[jdir];
		new_room = access_room(cond, pexit);

		if (new_room == step)
			return jdir;
	}

	return -6;  /* starting room does not lead to second room! */
} /* end find_path() */


/* hunt command implementation for players.
   aggressing mobs use hunt_victim() instead. */
void do_hunt(CHAR_DATA *ch, char *argument)
{
	char buffer[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	HUNT_CONDITIONS cond;
	int direction;
	bool same_area;
	int steps;

	if (!IS_NPC(ch) && !get_skill(ch, gsn_hunt)) {
		stc("You are not able to hunt.\n\r", ch);
		return;
	}

	if (*argument == '\0') {
		stc("Whom or what are you trying to hunt?\n\r", ch);
		return;
	}

	/* only Imms can hunt to other areas */
	if (IS_IMMORTAL(ch)) {
		same_area = FALSE;
		steps = 500;
	}
	else {
		same_area = TRUE;
		steps = 100;
	}

	if (same_area)
		victim = get_char_area(ch, argument, VIS_CHAR);
	else if (!(victim = get_char_area(ch, argument, VIS_CHAR)))
		victim = get_char_world(ch, argument, VIS_CHAR);

	if (victim == NULL) {
		stc("No-one around by that name.\n\r", ch);
		return;
	}

	if (ch->in_room == victim->in_room) {
		act("$N is here!", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (!deduct_stamina(ch, gsn_hunt))
		return;

	act("$n kneels down and checks for tracks.",
	    ch, NULL, NULL, TO_ROOM);
	WAIT_STATE(ch, skill_table[gsn_hunt].beats);
	/* set up hunt conditions */
	cond.hunter     = ch;
	cond.from_room  = ch->in_room;
	cond.to_room    = victim->in_room;
	cond.same_area  = same_area;
	cond.thru_doors = TRUE;
	cond.steps      = steps;
	/* find path */
	direction = find_path(&cond);

	if (direction == -1) {
		act("You couldn't find a path to $N from here.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}
	else if (direction == -2) {
		act("$N is too far away or cannot be found.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (direction < 0 || direction > 5) {
		stc("Hmm... Something seems to be wrong.\n\r", ch);
		sprintf(buffer, "Please report error code %d from HUNT.\n\r",
		        -direction);
		stc(buffer, ch);
		return;
	}

	/* Give a random direction if the player misses the die roll. */
	if ((IS_NPC(ch) && number_percent() > 75)           /* NPC @ 25% */
	    || (!IS_NPC(ch) && number_percent() >            /* PC @ norm */
	        ch->pcdata->learned[gsn_hunt])) {
		check_improve(ch, gsn_hunt, FALSE, 4);

		do {
			direction = number_door();
		}
		while ((ch->in_room->exit[direction] == NULL)
		       || (ch->in_room->exit[direction]->u1.to_room == NULL));
	}
	else {
		if (IS_IMMORTAL(ch)) {
			sprintf(buffer, "[%d steps] ", cond.steps);
			stc(buffer, ch);
		}

		check_improve(ch, gsn_hunt, TRUE, 4);
	}

	/* Display the results of the search. */
	act("$N is $t from here.", ch,
	    dir_name[direction], victim, TO_CHAR);
	return;
} /* end do_hunt() */


/* hunting function for aggressing mobs. */
void hunt_victim(CHAR_DATA *ch)
{
	int           dir;
	bool          found;
	CHAR_DATA     *tmp;
	HUNT_CONDITIONS cond;

	if (ch == NULL || ch->hunting == NULL || !IS_NPC(ch))
		return;

	/* Make sure the victim still exists. */
	for (found = 0, tmp = char_list; tmp && !found; tmp = tmp->next) {
		if (ch->hunting == tmp) {
			found = 1;
			break;  /* -- Elrac */
		}
	}

	if (!found || !can_see(ch, ch->hunting)) {
		do_say(ch, "Damn!  My prey is gone!!");
		ch->hunting = NULL;
		return;
	}

	if (ch->in_room == ch->hunting->in_room) {
		act("$n glares at $N and says, 'Ye shall DIE!'",
		    ch, NULL, ch->hunting, TO_NOTVICT);
		act("$n glares at you and says, 'Ye shall DIE!'",
		    ch, NULL, ch->hunting, TO_VICT);
		act("You glare at $N and say, 'Ye shall DIE!",
		    ch, NULL, ch->hunting, TO_CHAR);
		multi_hit(ch, ch->hunting, TYPE_UNDEFINED);
		ch->hunting = NULL;
		return;
	}

	WAIT_STATE(ch, skill_table[gsn_hunt].beats);
	/* set up hunt conditions */
	cond.hunter     = ch;
	cond.from_room  = ch->in_room;
	cond.to_room    = ch->hunting->in_room;
	cond.same_area  = TRUE;
	cond.thru_doors = TRUE;
	/* find path */
	dir = find_path(&cond);

	if (dir < 0 || dir > 5) {
		act("$n says 'Damn!  Lost $M!'", ch, NULL, ch->hunting, TO_ROOM);
		ch->hunting = NULL;
		return;
	}

	/*
	 * Give a random direction if the mob misses the die roll.
	 */
	if (number_percent() > 75) {        /* @ 25% */
		do {
			dir = number_door();
		}
		while ((ch->in_room->exit[dir] == NULL) ||
		       (ch->in_room->exit[dir]->u1.to_room == NULL));
	}

	if (cond.thru_doors &&
	    IS_SET(ch->in_room->exit[dir]->exit_info, EX_CLOSED)) {
		do_open(ch, (char *) dir_name[dir]);
		return;
	}

	move_char(ch, dir, FALSE);
	return;
} /* end hunt_victim() */

