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

#include "argument.hh"
#include "Area.hh"
#include "Character.hh"
#include "declare.hh"
#include "Exit.hh"
#include "Format.hh"
#include "Game.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "ObjectPrototype.hh"
#include "RoomPrototype.hh"
#include "Room.hh"
#include "String.hh"
#include "World.hh"

/* The following locals are for the checkexit command - Lotus */
const int opposite_dir [6] =
{ DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST, DIR_DOWN, DIR_UP };

typedef enum {exit_from, exit_to, exit_both} exit_status;

/* depending on status print > or < or <> between the 2 rooms */
String room_pair(Room *left, Room *right, exit_status ex) {
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

	String leftname = left->name().uncolor();
	String rightname = right->name().uncolor();
	return Format::format("%9s %-26.26s %s%9s %-26.26s(%-8.8s)\n",
	        left->location.to_string(false), leftname,
	        sExit,
	        right->location.to_string(false), rightname,
	        right->area().name);
}

/* for every exit in 'room' which leads to or from pArea but NOT both,
   print it */
String checkexits(Room *room, const Area *pArea) {
	String buf;

	for (int i = 0; i < 6; i++) {
		Exit *exit = room->exit[i];

		if (!exit)
			continue;

		Room *to_room = exit->to_room;

		if (to_room) { /* there is something on the other side */
			if ((room->area() == *pArea) && (to_room->area() != *pArea)) {
				/* an exit from our area to another area */
				/* check first if it is a two-way exit */
				if (to_room->exit[opposite_dir[i]] &&
				    to_room->exit[opposite_dir[i]]->to_room == room)
					buf += room_pair(room, to_room, exit_both); /* <> */
				else
					buf += room_pair(room, to_room, exit_to); /* > */
			}
			else if ((room->area() != *pArea) && (exit->to_room->area() == *pArea)) {
				/* an exit from another area to our area */
				if (!(to_room->exit[opposite_dir[i]] &&
				      to_room->exit[opposite_dir[i]]->to_room == room))
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
void do_exlist(Character *ch, String argument)
{
	const Area *to_area = &ch->in_room->area(); /* this is the area we want info on */

	/* run through all the rooms on the MUD */
	for (const auto& from_area : Game::world().areas)
		for (const auto& pair : from_area.second->rooms)
			stc(checkexits(pair.second, to_area), ch);
}

/* for every exit in 'room' which leads to or from pArea but NOT both,
   print it */
String checkexitstoroom(Room *room, Room *dest)
{
	String buf;

	for (int i = 0; i < 6; i++) {
		Exit *exit = room->exit[i];

		if (!exit)
			continue;

		Room *to_room = exit->to_room;

		if (to_room) { /* there is something on the other side */
			if (room == dest) {
				/* an exit  from the room we're looking for */
				/* check first if it is a two-way exit */
				if (to_room->exit[opposite_dir[i]] &&
				    to_room->exit[opposite_dir[i]]->to_room == room)
					buf += room_pair(room, to_room, exit_both); /* <> */
				else
					buf += room_pair(room, to_room, exit_to); /* > */
			}
			else if (to_room == dest) {
				if (!(to_room->exit[opposite_dir[i]] &&
				      to_room->exit[opposite_dir[i]]->to_room == room))
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
void do_roomexits(Character *ch, String argument)
{
	Room *dest = ch->in_room; /* this is the room we want info on */

	for (const auto& from_area : Game::world().areas)
		for (const auto& pair : from_area.second->rooms)
			stc(checkexitstoroom(pair.second, dest), ch);
}

/* find pockets of unused vnums equal to or greater than the argument */
void do_pocket(Character *ch, String argument)
{
	int vnum, count = 0, size = 50;

	String arg1;
	argument = one_argument(argument, arg1);

	if (!arg1.empty() && arg1.is_number())
		size = atoi(arg1);

	if (size < 20) {
		stc("Minimum size to search for is 20.\n", ch);
		return;
	}

	for (vnum = 1; vnum < 32600; vnum++) {
		/* figure out if it's in an area's range */
		for (const auto& area_pair: Game::world().areas) {
			const auto area = area_pair.second;

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
void do_roomlist(Character *ch, String argument)
{
	int first, last, counter;
	bool found = false;
	Room *room;
	String buffer;

	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty() || argument.empty()) {
		stc("Syntax: roomlist <starting vnum> <ending vnum>\n", ch);
		return;
	}

	if (!arg.is_number() || !argument.is_number()) {
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
		if ((room = Game::world().get_room(Location(Vnum(counter)))) != nullptr) {
			Format::sprintf(arg, "[%5d] (%s{x) %s{X\n",
			        room->prototype.vnum.value(), room->area().name,
			        room->name());
			buffer += arg;
			found = true;
		}
	}

	if (!found)
		stc("No rooms were found within the range given.\n", ch);
	else
		page_to_char(buffer, ch);

	return;
}

void do_vlist(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	String totalbuf;
	String buffer;
	int vnum, begvnum, endvnum;
	MobilePrototype *mobile;
	ObjectPrototype *object;
	bool found = false,
	     printed = false,
	     foundmobile = false,
	     founddata = false;

	String arg;
	argument = one_argument(argument, arg);

	if (!arg.is_number()) {
		stc("Syntax: vlist [beg vnum] [end vnum]\n", ch);
		return;
	}

	begvnum = atoi(arg);

	if (!argument.empty()) {
		if (!argument.is_number()) {
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
		found = false;
		foundmobile = false;
		totalbuf = Format::format("[%5d] ", vnum);

		if ((mobile = Game::world().get_mob_prototype(vnum)) != nullptr) {
			if (!printed)
				stc("[ Vnum] Mobile                        Object\n", ch);

			Format::sprintf(buf, "%s%*s", mobile->short_descr,
			        30 - mobile->short_descr.uncolor().size(), " ");
			/* Format::sprintf(buf, "%-30s ",mobile->short_descr); Color corrected -- Elrac */
			totalbuf += buf;
			found = true;
			foundmobile = true;
			printed = true;
			founddata = true;
		}

		if ((object = Game::world().get_obj_prototype(vnum)) != nullptr) {
			if (!printed)
				stc("[ Vnum] Mobile                        Object\n", ch);

			if (foundmobile)
				Format::sprintf(buf, "%-30s ", object->short_descr);
			else
				Format::sprintf(buf, "NONE                          %s", object->short_descr);

			totalbuf += buf;
			found = true;
			printed = true;
			founddata = true;
		}
		else if (foundmobile) {
			Format::sprintf(buf, "NONE");
			totalbuf += buf;
		}
		/* error check */
//		else
//			Logging::bug("Error in getting object index in do_vlist().", 0);

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

