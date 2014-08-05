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

#include "merc.h"
#include "sql.h"

void ignore_offline(CHAR_DATA *, char *);

bool is_ignoring(CHAR_DATA *ch, CHAR_DATA *victim)
{
	CHAR_DATA *rch;
	int pos;

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

	for (pos = 0; pos < MAX_IGNORE; pos++) {
		if (rch->pcdata->ignore[pos] == NULL)
			break;

		if (!str_cmp(rch->pcdata->ignore[pos], victim->name))
			return TRUE;
	}

	return FALSE;
}


/* Stripped from do_query basically - Lotus */
void do_ignore(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim, *rch;
	char arg[MIL];
	int pos;
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

	if (arg[0] == '\0') {
		if (rch->pcdata->ignore[0] == NULL) {
			stc("You are ignoring nobody.\n", ch);
			return;
		}

		stc("People you are ignoring:\n", ch);

		for (pos = 0; pos < MAX_IGNORE; pos++) {
			if (rch->pcdata->ignore[pos] == NULL)
				break;

			ptc(ch, "[%d] %s\n", pos, rch->pcdata->ignore[pos]);
		}

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

	if (!is_exact_name(victim->name, argument)) {
		stc("You must spell out their entire name.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		stc("You're not going to ignore us that easily!\n", ch);
		return;
	}

	for (pos = 0; pos < MAX_IGNORE; pos++) {
		if (rch->pcdata->ignore[pos] == NULL)
			break;

		if (!str_cmp(arg, rch->pcdata->ignore[pos])) {
			free_string(rch->pcdata->ignore[pos]);
			rch->pcdata->ignore[pos] = NULL;
			ptc(ch, "You stop ignoring %s.\n", victim->name);
			ptc(victim, "%s stops ignoring you.\n", ch->name);
			return;
		}
	}

	if (pos >= MAX_IGNORE) {
		stc("You can't ignore any more people.\n", ch);
		return;
	}

	rch->pcdata->ignore[pos] = str_dup(arg);
	ptc(ch, "You now ignore %s.\n", victim->name);
	ptc(victim, "%s ignores you.\n", ch->name);
}


void ignore_offline(CHAR_DATA *ch, char *arg)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	char *name = NULL;
	int pos;

	if ((result = db_queryf("ignore_offline", "SELECT name, cgroup FROM pc_index WHERE name='%s'", db_esc(arg))) != NULL) {
		if ((row = mysql_fetch_row(result))) {
			if (IS_SET(atol(row[1]), GROUP_GEN)) {
				stc("You're not going to ignore us that easily!\n", ch);
				return;
			}

			strcpy(name, row[0]);
		}
		else {
			stc("There is no one by that name to ignore.\n", ch);
			return;
		}

		mysql_free_result(result);
	}
	else {
		stc("Sorry, we couldn't retrieve that player's data.\nPlease report this with the 'bug' command.\n", ch);
		return;
	}

	for (pos = 0; pos < MAX_IGNORE; pos++) {
		if (ch->pcdata->ignore[pos] == NULL)
			break;

		if (!str_cmp(arg, ch->pcdata->ignore[pos])) {
			free_string(ch->pcdata->ignore[pos]);
			ch->pcdata->ignore[pos] = NULL;
			ptc(ch, "You stop ignoring %s.\n", name);
			return;
		}
	}

	if (pos >= MAX_IGNORE) {
		stc("You can't ignore any more people.\n", ch);
		return;
	}

	ch->pcdata->ignore[pos] = str_dup(name);
	ptc(ch, "You now ignore %s.\n", name);
}

