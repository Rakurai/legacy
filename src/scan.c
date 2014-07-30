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
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "lookup.h"

char * const distance[4] =
{
	"right here",
	"nearby to the ",
	"not far ",
	"off in the distance "
};

void	scan_list	args((ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch, sh_int depth, sh_int door));
void	scan_room	args((ROOM_INDEX_DATA *room, CHAR_DATA *ch, int depth, int door, struct exit_data *pexit));
void	scan_char	args((CHAR_DATA *victim, CHAR_DATA *ch, sh_int depth, sh_int door));

void do_scan2(CHAR_DATA *ch, char *argument) {
	extern char *const dir_name[];
	char arg1[MIL], buf[MIL];
	ROOM_INDEX_DATA *room;
	EXIT_DATA *pExit;
	sh_int door, depth;

	argument = one_argument(argument, arg1);

	if (arg1[0] == '\0')
	{
		act("$n scans all around.", ch, NULL, NULL, TO_NOTVIEW);
		stc("{PLooking around you see:{x\n\r", ch);
		scan_room(ch->in_room, ch, 1, -1, NULL);
//		scan_list(ch->in_room, ch, 0, -1);

		for (door = 0; door < 6; door++)
		{
			if ((pExit = ch->in_room->exit[door]) == NULL
			 || (pExit->u1.to_room) == NULL
			 || !can_see_room(ch, pExit->u1.to_room))
				continue;

			scan_room(pExit->u1.to_room, ch, 1, door, pExit);
/*
			if (IS_SET(pExit->exit_info, EX_CLOSED))
			{
				stc(ch, "{G(South) {Y(closed){x");
				ptc(ch, "{YThere is a closed exit to the %s.{x\n\r",dir_name[door]); 
				continue;
			}     

			scan_room(pExit->u1.to_room, ch, door);
			scan_list(pExit->u1.to_room, ch, 1, door);
*/		}

		return;
	}
	else if (!str_prefix1(arg1, "north"))	door = 0;
	else if (!str_prefix1(arg1, "east"))	door = 1;
	else if (!str_prefix1(arg1, "south"))	door = 2;
	else if (!str_prefix1(arg1, "west"))	door = 3;
	else if (!str_prefix1(arg1, "up"))	door = 4;
	else if (!str_prefix1(arg1, "down"))	door = 5;
	else
	{
		stc("Which way do you want to scan?\n\r", ch);
		return;
	}

	act("{PYou peer intently $T.{x", ch, NULL, dir_name[door], TO_CHAR);
	act("$n peers intently $T.", ch, NULL, dir_name[door], TO_NOTVIEW);
	sprintf(buf, "{GLooking %s you see:{x\n\r", dir_name[door]);
	room = ch->in_room;

	for (depth = 1; depth < 4; depth++)
	{
		if ((pExit = room->exit[door]) == NULL
		 || (pExit->u1.to_room) == NULL
		 || !can_see_room(ch, pExit->u1.to_room))
			continue;

		if (IS_SET(pExit->exit_info, EX_CLOSED))
		{
			ptc(ch, "{YThere is a closed exit to the %s.{x\n\r",dir_name[door]); 
			break;
		}     

		room = pExit->u1.to_room;
		scan_list(pExit->u1.to_room, ch, depth, door);
	}

}

void do_scan(CHAR_DATA *ch, char *argument)
{
	extern char *const dir_name[];
	char arg1[MIL], buf[MIL];
	ROOM_INDEX_DATA *scan_room;
	EXIT_DATA *pExit;
	sh_int door, depth;

	argument = one_argument(argument, arg1);

	if (arg1[0] == '\0')
	{
		act("$n scans all around.", ch, NULL, NULL, TO_NOTVIEW);
		stc("{PLooking around you see:{x\n\r", ch);
		scan_list(ch->in_room, ch, 0, -1);

		for (door = 0; door < 6; door++)
		{
			if ((pExit = ch->in_room->exit[door]) == NULL
			 || (pExit->u1.to_room) == NULL
			 || !can_see_room(ch, pExit->u1.to_room))
				continue;

			if (IS_SET(pExit->exit_info, EX_CLOSED))
			{
				ptc(ch, "{YThere is a closed exit to the %s.{x\n\r",dir_name[door]); 
				continue;
			}     

			scan_list(pExit->u1.to_room, ch, 1, door);
		}

		return;
	}
	else if (!str_prefix1(arg1, "north"))	door = 0;
	else if (!str_prefix1(arg1, "east"))	door = 1;
	else if (!str_prefix1(arg1, "south"))	door = 2;
	else if (!str_prefix1(arg1, "west"))	door = 3;
	else if (!str_prefix1(arg1, "up"))	door = 4;
	else if (!str_prefix1(arg1, "down"))	door = 5;
	else
	{
		stc("Which way do you want to scan?\n\r", ch);
		return;
	}

	act("{PYou peer intently $T.{x", ch, NULL, dir_name[door], TO_CHAR);
	act("$n peers intently $T.", ch, NULL, dir_name[door], TO_NOTVIEW);
	sprintf(buf, "{GLooking %s you see:{x\n\r", dir_name[door]);
	scan_room = ch->in_room;

	for (depth = 1; depth < 4; depth++)
	{
		if ((pExit = scan_room->exit[door]) == NULL
		 || (pExit->u1.to_room) == NULL
		 || !can_see_room(ch, pExit->u1.to_room))
			continue;

		if (IS_SET(pExit->exit_info, EX_CLOSED))
		{
			ptc(ch, "{YThere is a closed exit to the %s.{x\n\r",dir_name[door]); 
			break;
		}     

		scan_room = pExit->u1.to_room;
		scan_list(pExit->u1.to_room, ch, depth, door);
	}
}

void scan_room(ROOM_INDEX_DATA *room, CHAR_DATA *ch, int depth, int door, struct exit_data *pexit) {
	extern char *const dir_name[];
	ptc(ch, "{G(%5s){x ",
		door == -1 ? "here" : dir_name[door]
	);

	if (pexit && IS_SET(pexit->exit_info, EX_CLOSED))
		stc("{Y(closed){x\n", ch);
	else
		ptc(ch, "%s {B(%s){x\n",
			room->name,
			sector_lookup(room->sector_type)
		);
	scan_list(room, ch, depth, door);
}

void scan_list(ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch, sh_int depth, sh_int door)
{
	CHAR_DATA *rch;

	if (scan_room == NULL)
		return;

	for (rch = scan_room->people; rch != NULL; rch = rch->next_in_room)
	{
		if (rch == ch)
			continue;

		if (can_see(ch, rch))
		{
			if (IS_NPC(rch))
				new_color(ch, CSLOT_MISC_MOBILES);
			else
				new_color(ch, CSLOT_MISC_PLAYERS);

			scan_char(rch, ch, depth, door);
		}
	}
}


void scan_char(CHAR_DATA *victim, CHAR_DATA *ch, sh_int depth, sh_int door)
{
	extern char *const dir_name[];
	extern char *const distance[];

	ptc(ch, "  {C%s, %s%s.\n\r{x", PERS(victim, ch, VIS_CHAR), distance[depth], depth ? dir_name[door] : "");

/*	buf[0] = '\0';
	strcat(buf, PERS(victim, ch));
	strcat(buf, ", ");
	sprintf(buf2, distance[depth], dir_name[door]);
	strcat(buf, buf2);
	strcat(buf, "\n\r");
	stc(buf, ch); */

	set_color(ch, WHITE, NOBOLD);
}
