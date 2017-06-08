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
*       ROM 2.4 is copyright 1993-1996 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@efn.org)                                  *
*           Gabrielle Taylor                                               *
*           Brian Moore (zump@rom.org)                                     *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

#include "find.h"
#include "merc.h"
#include "sql.h"
#include "Format.hpp"

void ignore_offline(Character *, const String& );

bool is_ignoring(Character *ch, Character *victim)
{
	Character *rch;

	if (ch == NULL || victim == NULL)
		return FALSE;

	if (IS_IMMORTAL(victim))
		return FALSE;

	if (ch->desc == NULL)
		rch = ch;
	else
		rch = ch->desc->original ? ch->desc->original : ch;

	if (IS_NPC(rch))
		return FALSE;

	if (std::find(rch->pcdata->ignore.cbegin(), rch->pcdata->ignore.cend(), victim->name) != rch->pcdata->ignore.cend())
		return TRUE;

	return FALSE;
}

/* Stripped from do_query basically - Lotus */
void do_ignore(Character *ch, String argument)
{
	Character *victim, *rch;

	String arg;
	one_argument(argument, arg);

	if (ch->desc == NULL)
		rch = ch;
	else
		rch = ch->desc->original ? ch->desc->original : ch;

	if (IS_NPC(rch))
		return;

	if (IS_IMMORTAL(rch)) {
		stc("You have to listen to the players, goes with the job ;)\n", ch);
		return;
	}

	if (arg.empty()) {
		if (rch->pcdata->ignore.empty()) {
			stc("You are ignoring nobody.\n", ch);
			return;
		}

		stc("People you are ignoring:\n", ch);

		for (auto it = rch->pcdata->ignore.cbegin(); it != rch->pcdata->ignore.end(); it++)
			ptc(ch, "  %s\n", (*it).c_str());

		return;
	}

	victim = get_player_world(rch, argument, VIS_PLR);

	if (victim == NULL) {
		ignore_offline(ch, argument);
		return;
	}

	if (IS_NPC(victim)) {
		stc("Ignore a mob?  I don't think so.\n", ch);
		return;
	}

	if (ch == victim) {
		stc("I don't think you really want to ignore yourself.\n", ch);
		return;
	}

	if (!argument.has_exact_words(victim->name)) {
		stc("You must spell out their entire name.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		stc("You're not going to ignore us that easily!\n", ch);
		return;
	}

	auto search = std::find(rch->pcdata->ignore.begin(), rch->pcdata->ignore.end(), victim->name);

	if (search == rch->pcdata->ignore.end()) {
		// add
		ptc(ch, "You now ignore %s.\n", victim->name);
		ptc(victim, "%s ignores you.\n", ch->name);
		rch->pcdata->ignore.push_back(victim->name);
	}
	else {
		// remove
		ptc(ch, "You stop ignoring %s.\n", victim->name);
		ptc(victim, "%s stops ignoring you.\n", ch->name);
		rch->pcdata->ignore.erase(search);
	}
}

void ignore_offline(Character *ch, const String& arg)
{
	char name[MIL];

	if (db_queryf("ignore_offline", "SELECT name, cgroup FROM pc_index WHERE name LIKE '%s'", db_esc(arg)) != SQL_OK) {
		if (db_next_row() != SQL_OK) {
			if (IS_SET(db_get_column_int(1), GROUP_GEN)) {
				stc("You're not going to ignore us that easily!\n", ch);
				return;
			}

			strcpy(name, db_get_column_str(0));
		}
		else {
			stc("There is no one by that name to ignore.\n", ch);
			return;
		}
	}
	else {
		stc("Sorry, we couldn't retrieve that player's data.\nPlease report this with the 'bug' command.\n", ch);
		return;
	}

	auto search = std::find(ch->pcdata->ignore.begin(), ch->pcdata->ignore.end(), name);

	if (search == ch->pcdata->ignore.end()) {
		// add
		ptc(ch, "You now ignore %s.\n", name);
		ch->pcdata->ignore.push_back(name);
	}
	else {
		// remove
		ptc(ch, "You stop ignoring %s.\n", name);
		ch->pcdata->ignore.erase(search);
	}
}

