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
    RoomPrototype *   hunt_next;  // next room in search circle
    RoomPrototype *   hunt_back;  // pointer back toward origin

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

#include "act.hh"
#include "Character.hh"
#include "declare.hh"
#include "Exit.hh"
#include "find.hh"
#include "Flags.hh"
#include "Format.hh"
#include "interp.hh"
#include "macros.hh"
#include "memory.hh"
#include "merc.hh"
#include "Player.hh"
#include "random.hh"
#include "RoomPrototype.hh"
#include "skill/skill.hh"
#include "String.hh"

/* *** GLOBAL VARIABLES *** */

/* *** LOCAL VARIABLES *** */

/* Number of exits is stupidly hardcoded in Merc.
   If it changes, this line needs to be fixed up. */
static const int exit_num = 6;

struct hunt_conditions {
	/* first few fields set up by caller of find_path */
	Character *hunter;
	RoomPrototype *from_room;
	RoomPrototype *to_room;
	int same_area;
	int thru_doors;
	int  steps;
	/* these fields completed by find_path */
	Area *area;
};
typedef struct hunt_conditions HUNT_CONDITIONS;

/* Returns the room accessible via exit 'ex' or nullptr. */
static RoomPrototype *access_room(HUNT_CONDITIONS *cond, Exit *ex)
{
	RoomPrototype *new_room;

	if ((ex == nullptr) ||
	    ((new_room = ex->u1.to_room) == nullptr) ||
	    (cond->same_area && new_room->area != cond->area) ||
	    (!cond->thru_doors && ex->exit_flags.has(EX_CLOSED)) ||
	    (!can_see_room(cond->hunter, new_room)))
		return nullptr;

	return new_room;
} /* end access_room() */

/* find the shortest path from cond->from_room to ->to_room
   and return the direction of the first step or -1. */
static int find_path(HUNT_CONDITIONS *cond)
{
	RoomPrototype *this_ring;
	RoomPrototype *next_ring;
	int jdir;
	Exit *pexit;
	RoomPrototype *new_room;
	RoomPrototype *step;
	static int hunt_id = 1;
	hunt_id++;
	cond->area      = cond->from_room->area;
	cond->from_room->hunt_id   = hunt_id;
	cond->from_room->hunt_back = nullptr;
	cond->from_room->hunt_next = nullptr;
	this_ring = cond->from_room;
	next_ring = nullptr;

	/* loop through all rooms in the current ring, spiral outwards */
	while (this_ring != nullptr) {
		/* explore all possible exits */
		for (jdir = 0; jdir < exit_num; jdir++) {
			pexit = this_ring->exit[jdir];
			new_room = access_room(cond, pexit);

			if ((new_room != nullptr) && (new_room->hunt_id != hunt_id)) {
				new_room->hunt_id   = hunt_id;
				new_room->hunt_back = this_ring;
				new_room->hunt_next = next_ring;
				next_ring = new_room;

				if (new_room == cond->to_room)
					goto foundit;
			}
		}

		if (this_ring->hunt_next != nullptr) {
			/* move to next room in current ring */
			this_ring = this_ring->hunt_next;
		}
		else {
			/* move outward */
			this_ring = next_ring;
			next_ring = nullptr;

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

		if (step == nullptr) {
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
void do_hunt(Character *ch, String argument)
{
	char buffer[MAX_INPUT_LENGTH];
	Character *victim;
	HUNT_CONDITIONS cond;
	int direction;
	bool same_area;
	int steps;

	if (!IS_NPC(ch) && !get_learned(ch, skill::type::hunt)) {
		stc("You are not able to hunt.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("Whom or what are you trying to hunt?\n", ch);
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

	if (victim == nullptr) {
		stc("No-one around by that name.\n", ch);
		return;
	}

	if (ch->in_room == victim->in_room) {
		act("$N is here!", ch, nullptr, victim, TO_CHAR);
		return;
	}

	if (!deduct_stamina(ch, skill::type::hunt))
		return;

	act("$n kneels down and checks for tracks.",
	    ch, nullptr, nullptr, TO_ROOM);
	WAIT_STATE(ch, skill::lookup(skill::type::hunt).beats);
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
		    ch, nullptr, victim, TO_CHAR);
		return;
	}
	else if (direction == -2) {
		act("$N is too far away or cannot be found.",
		    ch, nullptr, victim, TO_CHAR);
		return;
	}

	if (direction < 0 || direction > 5) {
		stc("Hmm... Something seems to be wrong.\n", ch);
		Format::sprintf(buffer, "Please report error code %d from HUNT.\n",
		        -direction);
		stc(buffer, ch);
		return;
	}

	/* Give a random direction if the player misses the die roll. */
	if (number_percent() > get_learned(ch, skill::type::hunt)) {
		check_improve(ch, skill::type::hunt, FALSE, 4);

		do {
			direction = number_door();
		}
		while ((ch->in_room->exit[direction] == nullptr)
		       || (ch->in_room->exit[direction]->u1.to_room == nullptr));
	}
	else {
		if (IS_IMMORTAL(ch)) {
			Format::sprintf(buffer, "[%d steps] ", cond.steps);
			stc(buffer, ch);
		}

		check_improve(ch, skill::type::hunt, TRUE, 4);
	}

	/* Display the results of the search. */
	act("$N is $t from here.", ch,
	    Exit::dir_name(direction), victim, TO_CHAR);
	return;
} /* end do_hunt() */

/* hunting function for aggressing mobs. */
void hunt_victim(Character *ch)
{
	int           dir;
	bool          found;
	Character     *tmp;
	HUNT_CONDITIONS cond;

	if (ch == nullptr || ch->hunting == nullptr || !IS_NPC(ch))
		return;

	/* Make sure the victim still exists. */
	for (found = 0, tmp = char_list; tmp && !found; tmp = tmp->next) {
		if (ch->hunting == tmp) {
			found = 1;
			break;  /* -- Elrac */
		}
	}

	if (!found || !can_see_char(ch, ch->hunting)) {
		do_say(ch, "Damn!  My prey is gone!!");
		ch->hunting = nullptr;
		return;
	}

	if (ch->in_room == ch->hunting->in_room) {
		act("$n glares at $N and says, 'Ye shall DIE!'",
		    ch, nullptr, ch->hunting, TO_NOTVICT);
		act("$n glares at you and says, 'Ye shall DIE!'",
		    ch, nullptr, ch->hunting, TO_VICT);
		act("You glare at $N and say, 'Ye shall DIE!",
		    ch, nullptr, ch->hunting, TO_CHAR);
		multi_hit(ch, ch->hunting, skill::type::unknown);
		ch->hunting = nullptr;
		return;
	}

	WAIT_STATE(ch, skill::lookup(skill::type::hunt).beats);
	/* set up hunt conditions */
	cond.hunter     = ch;
	cond.from_room  = ch->in_room;
	cond.to_room    = ch->hunting->in_room;
	cond.same_area  = TRUE;
	cond.thru_doors = TRUE;
	/* find path */
	dir = find_path(&cond);

	if (dir < 0 || dir > 5) {
		act("$n says 'Damn!  Lost $M!'", ch, nullptr, ch->hunting, TO_ROOM);
		ch->hunting = nullptr;
		return;
	}

	/*
	 * Give a random direction if the mob misses the die roll.
	 */
	if (number_percent() > 75) {        /* @ 25% */
		do {
			dir = number_door();
		}
		while ((ch->in_room->exit[dir] == nullptr) ||
		       (ch->in_room->exit[dir]->u1.to_room == nullptr));
	}

	if (cond.thru_doors &&
	    ch->in_room->exit[dir]->exit_flags.has(EX_CLOSED)) {
		do_open(ch, Exit::dir_name(dir));
		return;
	}

	move_char(ch, dir, FALSE);
	return;
} /* end hunt_victim() */

