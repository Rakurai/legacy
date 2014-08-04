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

//#include <sys/types.h>
//#include <sys/time.h>
//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include <ctype.h>
//#include <signal.h>
#include "merc.h"
#include "recycle.h"

extern  ROOM_INDEX_DATA *room_index_hash [MAX_KEY_HASH];
extern  AREA_DATA       *area_first;

/* The following locals are for the checkexit command - Lotus */
const sh_int opposite_dir [6] =
{ DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST, DIR_DOWN, DIR_UP };

typedef enum {exit_from, exit_to, exit_both} exit_status;

/* depending on status print > or < or <> between the 2 rooms */
void room_pair(ROOM_INDEX_DATA *left, ROOM_INDEX_DATA *right, exit_status ex, char *buffer)
{
	char *sExit;
	char leftname[MAX_STRING_LENGTH];
	char rightname[MAX_STRING_LENGTH];

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

	sprintf(leftname, "%s", smash_bracket(left->name));
	sprintf(rightname, "%s", smash_bracket(right->name));
	sprintf(buffer, "%5d %-26.26s %s%5d %-26.26s(%-8.8s)\n\r",
	        left->vnum, leftname,
	        sExit,
	        right->vnum, rightname,
	        right->area->name);
}


/* for every exit in 'room' which leads to or from pArea but NOT both,
   print it */
void checkexits(ROOM_INDEX_DATA *room, AREA_DATA *pArea, char *buffer)
{
	char buf[MAX_STRING_LENGTH];
	int i;
	EXIT_DATA *exit;
	ROOM_INDEX_DATA *to_room;
	strcpy(buffer, "");

	for (i = 0; i < 6; i++) {
		exit = room->exit[i];

		if (!exit)
			continue;
		else
			to_room = exit->u1.to_room;

		if (to_room) { /* there is something on the other side */
			if ((room->area == pArea) && (to_room->area != pArea)) {
				/* an exit from our area to another area */
				/* check first if it is a two-way exit */
				if (to_room->exit[opposite_dir[i]] &&
				    to_room->exit[opposite_dir[i]]->u1.to_room == room)
					room_pair(room, to_room, exit_both, buf); /* <> */
				else
					room_pair(room, to_room, exit_to, buf); /* > */

				strcat(buffer, buf);
			}
			else if ((room->area != pArea) && (exit->u1.to_room->area == pArea)) {
				/* an exit from another area to our area */
				if (!(to_room->exit[opposite_dir[i]] &&
				      to_room->exit[opposite_dir[i]]->u1.to_room == room))
					/* two-way exits are handled in the other if */
				{
					room_pair(to_room, room, exit_from, buf);
					strcat(buffer, buf);
				}
			} /* if room->area */
		}
	} /* for */
}

/* for now, no arguments, just list the current area */
void do_exlist(CHAR_DATA *ch, char *argument)
{
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *room;
	int i;
	char buffer[MAX_STRING_LENGTH];
	pArea = ch->in_room->area; /* this is the area we want info on */

	for (i = 0; i < MAX_KEY_HASH; i++) /* room index hash table */
		for (room = room_index_hash[i]; room != NULL; room = room->next)
			/* run through all the rooms on the MUD */
		{
			checkexits(room, pArea, buffer);
			stc(buffer, ch);
		}
}

/* for every exit in 'room' which leads to or from pArea but NOT both,
   print it */
void checkexitstoroom(ROOM_INDEX_DATA *room, ROOM_INDEX_DATA *dest, char *buffer)
{
	char buf[MAX_STRING_LENGTH];
	int i;
	EXIT_DATA *exit;
	ROOM_INDEX_DATA *to_room;
	strcpy(buffer, "");

	for (i = 0; i < 6; i++) {
		exit = room->exit[i];

		if (!exit)
			continue;
		else
			to_room = exit->u1.to_room;

		if (to_room) { /* there is something on the other side */
			if (room == dest) {
				/* an exit  from the room we're looking for */
				/* check first if it is a two-way exit */
				if (to_room->exit[opposite_dir[i]] &&
				    to_room->exit[opposite_dir[i]]->u1.to_room == room)
					room_pair(room, to_room, exit_both, buf); /* <> */
				else
					room_pair(room, to_room, exit_to, buf); /* > */

				strcat(buffer, buf);
			}
			else if (to_room == dest) {
				if (!(to_room->exit[opposite_dir[i]] &&
				      to_room->exit[opposite_dir[i]]->u1.to_room == room))
					/* two-way exits are handled in the other if */
				{
					room_pair(to_room, room, exit_from, buf);
					strcat(buffer, buf);
				}
			}
		}
	} /* for */
}

/* for now, no arguments, just list the current room */
void do_roomexits(CHAR_DATA *ch, char *argument)
{
	ROOM_INDEX_DATA *dest;
	ROOM_INDEX_DATA *room;
	int i;
	char buffer[MAX_STRING_LENGTH];
	dest = ch->in_room; /* this is the room we want info on */

	for (i = 0; i < MAX_KEY_HASH; i++) /* room index hash table */
		for (room = room_index_hash[i]; room != NULL; room = room->next)
			/* run through all the rooms on the MUD */
		{
			checkexitstoroom(room, dest, buffer);
			stc(buffer, ch);
		}
}


/* find pockets of unused vnums equal to or greater than the argument */
void do_pocket(CHAR_DATA *ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	AREA_DATA *area;
	int vnum, count = 0, size = 50;
	argument = one_argument(argument, arg1);

	if (arg1[0] != '\0' && is_number(arg1))
		size = atoi(arg1);

	if (size < 20) {
		stc("Minimum size to search for is 20.\n\r", ch);
		return;
	}

	for (vnum = 1; vnum < 32600; vnum++) {
		/* figure out if it's in an area's range */
		for (area = area_first; area; area = area->next) {
			if (area->min_vnum <= vnum && area->max_vnum >= vnum) { /* it is */
				if (count >= size)
					ptc(ch, "%5d to %5d, size %d\n\r", vnum - (count - 1), vnum - 1, count - 1);

				count = 0;
				continue;
			}
		}

		count++;
	}
}


/* Room List by Lotus */
void do_roomlist(CHAR_DATA *ch, char *argument)
{
	int first, last, counter;
	bool found = FALSE;
	char arg[MAX_STRING_LENGTH];
	ROOM_INDEX_DATA *room;
	BUFFER *buffer;
	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0') {
		stc("Syntax: roomlist <starting vnum> <ending vnum>\n\r", ch);
		return;
	}

	if (!is_number(arg) || !is_number(argument)) {
		stc("Values must be numeric\n\r", ch);
		return;
	}

	first = atoi(arg);
	last = atoi(argument);
	buffer = new_buf();

	if ((first < 0) || (first > 99999) || (last < 0) || (last > 99999)) {
		stc("Values must be between 0 and 99999.\n\r", ch);
		return;
	}

	if (first >= last) {
		stc("Second value must be greater than first.\n\r", ch);
		return;
	}

	for (counter = first; counter <= last; counter++) {
		if ((room = get_room_index(counter)) != NULL) {
			sprintf(arg, "[%5d] (%s{x) %s{X\n\r",
			        room->vnum, room->area->name,
			        room->name);
			add_buf(buffer, arg);
			found = TRUE;
		}
	}

	if (!found)
		stc("No rooms were found within the range given.\n\r", ch);
	else
		page_to_char(buf_string(buffer), ch);

	free_buf(buffer);
	return;
}


void do_vlist(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char totalbuf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	BUFFER *buffer;
	int vnum, begvnum, endvnum;
	MOB_INDEX_DATA *mobile;
	OBJ_INDEX_DATA *object;
	bool found = FALSE,
	     printed = FALSE,
	     foundmobile = FALSE,
	     founddata = FALSE;
	argument = one_argument(argument, arg);

	if (!is_number(arg)) {
		stc("Syntax: vlist [beg vnum] [end vnum]\n\r", ch);
		return;
	}

	begvnum = atoi(arg);

	if (argument[0] != '\0') {
		if (!is_number(argument)) {
			stc("Syntax: vlist [beg vnum] [end vnum]\n\r", ch);
			return;
		}

		endvnum = atoi(argument);
	}
	else
		endvnum = begvnum;

	if ((begvnum < 0) || (begvnum > 99999)
	    || (endvnum < 0) || (endvnum > 99999)) {
		stc("Values must be between 0 and 99999.\n\r", ch);
		return;
	}

	if (begvnum > endvnum) {
		stc("Second value must be greater than first.\n\r", ch);
		return;
	}

	buffer = new_buf();

	for (vnum = begvnum; vnum <= endvnum; vnum++) {
		found = FALSE;
		foundmobile = FALSE;
		totalbuf[0] = '\0';
		sprintf(totalbuf, "[%5d] ", vnum);

		if ((mobile = get_mob_index(vnum)) != NULL) {
			if (!printed)
				stc("[ Vnum] Mobile                         Object\n\r", ch);

			sprintf(buf, "%s%*s", mobile->short_descr,
			        30 - color_strlen(mobile->short_descr), " ");
			/* sprintf(buf, "%-30s ",mobile->short_descr); Color corrected -- Elrac */
			strcat(totalbuf, buf);
			found = TRUE;
			foundmobile = TRUE;
			printed = TRUE;
			founddata = TRUE;
		}

		if ((object = get_obj_index(vnum)) != NULL) {
			if (!printed)
				stc(" [Vnum] Mobile                         Object\n\r", ch);

			if (foundmobile)
				sprintf(buf, "%-30s ", object->short_descr);
			else
				sprintf(buf, "NONE                           %s", object->short_descr);

			strcat(totalbuf, buf);
			found = TRUE;
			printed = TRUE;
			founddata = TRUE;
		}
		else if (foundmobile) {
			sprintf(buf, "NONE");
			strcat(totalbuf, buf);
		}
		/* error check */
		else
			bug("Error in getting object index in do_vlist().", 0);

		if (found) {
			strcat(totalbuf, "\n\r");
			add_buf(buffer, totalbuf);
		}
	}

	if (!founddata)
		stc("Nothing was found in that vnum range.\n\r", ch);
	else
		page_to_char(buf_string(buffer), ch);

	free_buf(buffer);
}


