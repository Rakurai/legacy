/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

#include "act.hh"
#include "argument.hh"
#include "Character.hh"
#include "declare.hh"
#include "Exit.hh"
#include "Flags.hh"
#include "Format.hh"
#include "lookup.hh"
#include "merc.hh"
#include "Room.hh"
#include "String.hh"

char *const distance[4] = {
	"right here",
	"nearby to the ",
	"not far ",
	"off in the distance "
};

void    scan_list       args((Room *scan_room, Character *ch, int depth, int door));
void    scan_room       args((Room *room, Character *ch, int depth, int door, Exit *pexit));
void    scan_char       args((Character *victim, Character *ch, int depth, int door));

void do_scan(Character *ch, String argument)
{
	char buf[MIL];
	Room *room;
	Exit *pExit;
	int door, depth;

	String arg1;
	argument = one_argument(argument, arg1);

	if (arg1.empty()) {
		act("$n scans all around.", ch, nullptr, nullptr, TO_NOTVIEW);
		stc("{PLooking around you see:{x\n", ch);
		scan_room(ch->in_room, ch, 0, -1, nullptr);

		for (door = 0; door < 6; door++) {
			if ((pExit = ch->in_room->exit[door]) == nullptr
			    || (pExit->to_room) == nullptr
			    || !can_see_room(ch, pExit->to_room))
				continue;

			scan_room(pExit->to_room, ch, 1, door, pExit);
		}

		return;
	}
	else if (arg1.is_prefix_of("north"))   door = 0;
	else if (arg1.is_prefix_of("east"))    door = 1;
	else if (arg1.is_prefix_of("south"))   door = 2;
	else if (arg1.is_prefix_of("west"))    door = 3;
	else if (arg1.is_prefix_of("up"))      door = 4;
	else if (arg1.is_prefix_of("down"))    door = 5;
	else {
		stc("Which way do you want to scan?\n", ch);
		return;
	}

	act("{PYou peer intently $T.{x", ch, nullptr, Exit::dir_name(door), TO_CHAR);
	act("$n peers intently $T.", ch, nullptr, Exit::dir_name(door), TO_NOTVIEW);
	Format::sprintf(buf, "{GLooking %s you see:{x\n", Exit::dir_name(door));
	room = ch->in_room;

	for (depth = 1; depth < 4; depth++) {
		if ((pExit = room->exit[door]) == nullptr
		    || (pExit->to_room) == nullptr
		    || !can_see_room(ch, pExit->to_room))
			break;

		room = pExit->to_room;
		scan_room(room, ch, depth, door, pExit);

		if (pExit->exit_flags.has(EX_CLOSED))
			break;
	}
}

void do_oldscan(Character *ch, String argument)
{
	char buf[MIL];
	Room *scan_room;
	Exit *pExit;
	int door, depth;

	String arg1;
	argument = one_argument(argument, arg1);

	if (arg1.empty()) {
		act("$n scans all around.", ch, nullptr, nullptr, TO_NOTVIEW);
		stc("{PLooking around you see:{x\n", ch);
		scan_list(ch->in_room, ch, 0, -1);

		for (door = 0; door < 6; door++) {
			if ((pExit = ch->in_room->exit[door]) == nullptr
			    || (pExit->to_room) == nullptr
			    || !can_see_room(ch, pExit->to_room)
			    || !can_see_in_room(ch, pExit->to_room))
				continue;

			if (pExit->exit_flags.has(EX_CLOSED)) {
				ptc(ch, "{YThere is a closed exit to the %s.{x\n", Exit::dir_name(door));
				continue;
			}

			scan_list(pExit->to_room, ch, 1, door);
		}

		return;
	}
	else if (arg1.is_prefix_of("north"))   door = 0;
	else if (arg1.is_prefix_of("east"))    door = 1;
	else if (arg1.is_prefix_of("south"))   door = 2;
	else if (arg1.is_prefix_of("west"))    door = 3;
	else if (arg1.is_prefix_of("up"))      door = 4;
	else if (arg1.is_prefix_of("down"))    door = 5;
	else {
		stc("Which way do you want to scan?\n", ch);
		return;
	}

	act("{PYou peer intently $T.{x", ch, nullptr, Exit::dir_name(door), TO_CHAR);
	act("$n peers intently $T.", ch, nullptr, Exit::dir_name(door), TO_NOTVIEW);
	Format::sprintf(buf, "{GLooking %s you see:{x\n", Exit::dir_name(door));
	scan_room = ch->in_room;

	for (depth = 1; depth < 4; depth++) {
		if ((pExit = scan_room->exit[door]) == nullptr
		    || (pExit->to_room) == nullptr
		    || !can_see_room(ch, pExit->to_room))
			continue;

		if (!can_see_in_room(ch, pExit->to_room)) {
			stc("It is too dark to see any farther in that direction.\n", ch);
			break;
		}

		if (pExit->exit_flags.has(EX_CLOSED)) {
			ptc(ch, "{YThere is a closed exit to the %s.{x\n", Exit::dir_name(door));
			break;
		}

		scan_room = pExit->to_room;
		scan_list(pExit->to_room, ch, depth, door);
	}
}

void scan_room(Room *room, Character *ch, int depth, int door, Exit *pexit)
{
	ptc(ch, "{C(%5s){x ",
	    door == -1 ? "here" : Exit::dir_name(door)
	   );

	if (pexit && pexit->exit_flags.has(EX_CLOSED))
		stc("{Y(closed){x\n", ch);
	else if (!can_see_in_room(ch, room))
		stc("{c[too dark to tell]{x\n", ch);
	else {
		ptc(ch, "%s {B(%s){x\n",
		    room->name(),
		    sector_lookup(room->sector_type())
		   );

		scan_list(room, ch, depth, door);
	}
}

void scan_list(Room *scan_room, Character *ch, int depth, int door)
{
	Character *rch;

	if (scan_room == nullptr)
		return;

	for (rch = scan_room->people; rch != nullptr; rch = rch->next_in_room) {
		if (rch == ch)
			continue;

		if (can_see_char(ch, rch)) {
			scan_char(rch, ch, depth, door);
		}
	}
}

void scan_char(Character *victim, Character *ch, int depth, int door)
{
	extern char *const distance[];
	if (victim->is_npc())
		new_color(ch, CSLOT_MISC_MOBILES);
	else
		new_color(ch, CSLOT_MISC_PLAYERS);

	ptc(ch, "   %s, %s%s.\n", PERS(victim, ch, VIS_CHAR), distance[depth], depth ? Exit::dir_name(door) : "");
	set_color(ch, WHITE, NOBOLD);
}
