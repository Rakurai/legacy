/*************************************************
*                                                *
*               The Age of Legacy                *
*                                                *
* Based originally on ROM 2.4, tested, enhanced, *
* and maintained by the Legacy Team.  If that    *
* doesn't mean you, and you're stealing our      *
* code, at least tell us and boost our egos ;)   *
*************************************************/

/*************************************************
* IMM builder commands and related functions.    *
* Most of these are in the builder command       *
* group.                                         *
*************************************************/

#include "merc.h"
#include "recycle.h"
#include "Format.hpp"

extern  ROOM_INDEX_DATA *room_index_hash [MAX_KEY_HASH];
extern  AREA_DATA       *area_first;

/* The following locals are for the checkexit command - Lotus */
const sh_int opposite_dir [6] =
{ DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST, DIR_DOWN, DIR_UP };

typedef enum {exit_from, exit_to, exit_both} exit_status;

/* depending on status print > or < or <> between the 2 rooms */
String room_pair(ROOM_INDEX_DATA *left, ROOM_INDEX_DATA *right, exit_status ex) {
	String sExit;

	switch (ex) {
	default:
		sExit = "??"; break; /* invalid usage */

	case exit_from:
		sExit = "< "; break;

	case exit_to:
		sExit = " >"; break;

	case exit_both:
		sExit = "<>"; break;
	}

	String leftname = smash_bracket(left->name);
	String rightname = smash_bracket(right->name);
	return Format::format("%5d %-26.26s %s%5d %-26.26s(%-8.8s)\n",
	        left->vnum, leftname,
	        sExit,
	        right->vnum, rightname,
	        right->area->name);
}

/* for every exit in 'room' which leads to or from pArea but NOT both,
   print it */
String checkexits(ROOM_INDEX_DATA *room, AREA_DATA *pArea) {
	String buf;

	for (int i = 0; i < 6; i++) {
		EXIT_DATA *exit = room->exit[i];

		if (!exit)
			continue;

		ROOM_INDEX_DATA *to_room = exit->u1.to_room;

		if (to_room) { /* there is something on the other side */
			if ((room->area == pArea) && (to_room->area != pArea)) {
				/* an exit from our area to another area */
				/* check first if it is a two-way exit */
				if (to_room->exit[opposite_dir[i]] &&
				    to_room->exit[opposite_dir[i]]->u1.to_room == room)
					buf += room_pair(room, to_room, exit_both); /* <> */
				else
					buf += room_pair(room, to_room, exit_to); /* > */
			}
			else if ((room->area != pArea) && (exit->u1.to_room->area == pArea)) {
				/* an exit from another area to our area */
				if (!(to_room->exit[opposite_dir[i]] &&
				      to_room->exit[opposite_dir[i]]->u1.to_room == room))
					/* two-way exits are handled in the other if */
				{
					buf += room_pair(to_room, room, exit_from);
				}
			} /* if room->area */
		}
	} /* for */

	return buf;
}

/* for now, no arguments, just list the current area */
void do_exlist(CHAR_DATA *ch, String argument)
{
	AREA_DATA *pArea = ch->in_room->area; /* this is the area we want info on */

	for (int i = 0; i < MAX_KEY_HASH; i++) /* room index hash table */
		for (ROOM_INDEX_DATA *room = room_index_hash[i]; room; room = room->next)
			/* run through all the rooms on the MUD */
		{
			stc(checkexits(room, pArea), ch);
		}
}

/* for every exit in 'room' which leads to or from pArea but NOT both,
   print it */
String checkexitstoroom(ROOM_INDEX_DATA *room, ROOM_INDEX_DATA *dest)
{
	String buf;

	for (int i = 0; i < 6; i++) {
		EXIT_DATA *exit = room->exit[i];

		if (!exit)
			continue;

		ROOM_INDEX_DATA *to_room = exit->u1.to_room;

		if (to_room) { /* there is something on the other side */
			if (room == dest) {
				/* an exit  from the room we're looking for */
				/* check first if it is a two-way exit */
				if (to_room->exit[opposite_dir[i]] &&
				    to_room->exit[opposite_dir[i]]->u1.to_room == room)
					buf += room_pair(room, to_room, exit_both); /* <> */
				else
					buf += room_pair(room, to_room, exit_to); /* > */
			}
			else if (to_room == dest) {
				if (!(to_room->exit[opposite_dir[i]] &&
				      to_room->exit[opposite_dir[i]]->u1.to_room == room))
					/* two-way exits are handled in the other if */
				{
					buf += room_pair(to_room, room, exit_from);
				}
			}
		}
	} /* for */

	return buf;
}

/* for now, no arguments, just list the current room */
void do_roomexits(CHAR_DATA *ch, String argument)
{
	ROOM_INDEX_DATA *dest = ch->in_room; /* this is the room we want info on */

	for (int i = 0; i < MAX_KEY_HASH; i++) /* room index hash table */
		for (ROOM_INDEX_DATA *room = room_index_hash[i]; room; room = room->next)
			/* run through all the rooms on the MUD */
		{
			stc(checkexitstoroom(room, dest), ch);
		}
}

/* find pockets of unused vnums equal to or greater than the argument */
void do_pocket(CHAR_DATA *ch, String argument)
{
	AREA_DATA *area;
	int vnum, count = 0, size = 50;

	String arg1;
	argument = one_argument(argument, arg1);

	if (!arg1.empty() && is_number(arg1))
		size = atoi(arg1);

	if (size < 20) {
		stc("Minimum size to search for is 20.\n", ch);
		return;
	}

	for (vnum = 1; vnum < 32600; vnum++) {
		/* figure out if it's in an area's range */
		for (area = area_first; area; area = area->next) {
			if (area->min_vnum <= vnum && area->max_vnum >= vnum) { /* it is */
				if (count >= size)
					ptc(ch, "%5d to %5d, size %d\n", vnum - (count - 1), vnum - 1, count - 1);

				count = 0;
				continue;
			}
		}

		count++;
	}
}

/* Room List by Lotus */
void do_roomlist(CHAR_DATA *ch, String argument)
{
	int first, last, counter;
	bool found = FALSE;
	ROOM_INDEX_DATA *room;
	String buffer;

	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty() || argument.empty()) {
		stc("Syntax: roomlist <starting vnum> <ending vnum>\n", ch);
		return;
	}

	if (!is_number(arg) || !is_number(argument)) {
		stc("Values must be numeric\n", ch);
		return;
	}

	first = atoi(arg);
	last = atoi(argument);

	if ((first < 0) || (first > 99999) || (last < 0) || (last > 99999)) {
		stc("Values must be between 0 and 99999.\n", ch);
		return;
	}

	if (first >= last) {
		stc("Second value must be greater than first.\n", ch);
		return;
	}

	for (counter = first; counter <= last; counter++) {
		if ((room = get_room_index(counter)) != NULL) {
			Format::sprintf(arg, "[%5d] (%s{x) %s{X\n",
			        room->vnum, room->area->name,
			        room->name);
			buffer += arg;
			found = TRUE;
		}
	}

	if (!found)
		stc("No rooms were found within the range given.\n", ch);
	else
		page_to_char(buffer, ch);

	return;
}

void do_vlist(CHAR_DATA *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	String totalbuf;
	String buffer;
	int vnum, begvnum, endvnum;
	MOB_INDEX_DATA *mobile;
	OBJ_INDEX_DATA *object;
	bool found = FALSE,
	     printed = FALSE,
	     foundmobile = FALSE,
	     founddata = FALSE;

	String arg;
	argument = one_argument(argument, arg);

	if (!is_number(arg)) {
		stc("Syntax: vlist [beg vnum] [end vnum]\n", ch);
		return;
	}

	begvnum = atoi(arg);

	if (!argument.empty()) {
		if (!is_number(argument)) {
			stc("Syntax: vlist [beg vnum] [end vnum]\n", ch);
			return;
		}

		endvnum = atoi(argument);
	}
	else
		endvnum = begvnum;

	if ((begvnum < 0) || (begvnum > 99999)
	    || (endvnum < 0) || (endvnum > 99999)) {
		stc("Values must be between 0 and 99999.\n", ch);
		return;
	}

	if (begvnum > endvnum) {
		stc("Second value must be greater than first.\n", ch);
		return;
	}


	for (vnum = begvnum; vnum <= endvnum; vnum++) {
		found = FALSE;
		foundmobile = FALSE;
		totalbuf[0] = '\0';
		Format::sprintf(totalbuf, "[%5d] ", vnum);

		if ((mobile = get_mob_index(vnum)) != NULL) {
			if (!printed)
				stc("[ Vnum] Mobile                        Object\n", ch);

			Format::sprintf(buf, "%s%*s", mobile->short_descr,
			        30 - color_strlen(mobile->short_descr), " ");
			/* Format::sprintf(buf, "%-30s ",mobile->short_descr); Color corrected -- Elrac */
			totalbuf += buf;
			found = TRUE;
			foundmobile = TRUE;
			printed = TRUE;
			founddata = TRUE;
		}

		if ((object = get_obj_index(vnum)) != NULL) {
			if (!printed)
				stc("[ Vnum] Mobile                        Object\n", ch);

			if (foundmobile)
				Format::sprintf(buf, "%-30s ", object->short_descr);
			else
				Format::sprintf(buf, "NONE                          %s", object->short_descr);

			totalbuf += buf;
			found = TRUE;
			printed = TRUE;
			founddata = TRUE;
		}
		else if (foundmobile) {
			Format::sprintf(buf, "NONE");
			totalbuf += buf;
		}
		/* error check */
//		else
//			bug("Error in getting object index in do_vlist().", 0);

		if (found) {
			totalbuf += "\n";
			buffer += totalbuf;
		}
	}

	if (!founddata)
		stc("Nothing was found in that vnum range.\n", ch);
	else
		page_to_char(buffer, ch);

}

