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

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

char    *get_multi_command     args((DESCRIPTOR_DATA *d, char *argument));

/* does aliasing and other fun stuff */
void substitute_alias(DESCRIPTOR_DATA *d, char *argument)
{
	CHAR_DATA *ch;
	char buf[MAX_STRING_LENGTH];
	char prefix[2 * MAX_INPUT_LENGTH];
	char name[MAX_INPUT_LENGTH];
	char *point;
	int alias;
	ch = d->original ? d->original : d->character;

	if (!ch) {
		bug("substitute_alias: null ch", 0);
		return; /* crashed on a null character in this a few times -- Montrey */
	}

	if (!ch->pcdata) {
		bug("substitute_alias: null pcdata", 0);
		return;
	}

	/* check for prefix */
	if (ch->prefix[0] != '\0' && str_prefix1("prefix", argument)) {
		if (strlen(ch->prefix) + 1 + strlen(argument) + 1 > MAX_INPUT_LENGTH)
			stc("{PLine too long, truncated!{x\n\r", ch);

		sprintf(prefix, "%s %s", ch->prefix, argument);
		prefix[MAX_INPUT_LENGTH - 1] = '\0';
		argument = prefix;
	}

	if (IS_NPC(ch) || ch->pcdata->alias[0] == NULL || ch->pcdata->alias[0] == '\0'
	    ||  !str_prefix1("alias", argument) || !str_prefix1("una", argument)
	    ||  !str_prefix1("prefix", argument)) {
		interpret(d->character, argument);
		return;
	}

	strcpy(buf, argument);

	for (alias = 0; alias < MAX_ALIAS; alias++) { /* go through the aliases */
		if (ch->pcdata->alias[alias] == NULL)
			break;

		if (!str_prefix1(ch->pcdata->alias[alias], argument)) {
			point = one_argument(argument, name);

			if (!strcmp(ch->pcdata->alias[alias], name)) {
				strcpy(buf, ch->pcdata->alias_sub[alias]);
				strcat(buf, " ");
				strcat(buf, point);
				strcpy(buf, get_multi_command(d, buf));
				break;
			}

			if (strlen(buf) > MAX_INPUT_LENGTH) {
				stc("Alias substitution too long. Truncated.\r\n", ch);
				buf[MAX_INPUT_LENGTH - 1] = '\0';
			}
		}
	}

	interpret(d->character, buf);
} /* end substitute_alias() */


void do_alia(CHAR_DATA *ch, char *argument)
{
	stc("I'm sorry, alias must be entered in full.\n\r", ch);
	return;
}


void do_alias(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *rch;
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	char *x;
	int pos;

	if (ch->desc == NULL)
		rch = ch;
	else
		rch = ch->desc->original ? ch->desc->original : ch;

	if (IS_NPC(rch))
		return;

	/* Fix the alias bug and add wiznet for those cheaters - Lotus */
	for (x = argument; *x != '\0'; x++) {
		if (*x == '~') {
			wiznet("$N is attempting to use the tilde in alias cheat.", ch,
			       NULL, WIZ_CHEAT, 0, GET_RANK(ch));
			break;
		}
	}

	smash_tilde(argument);
	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		if (rch->pcdata->alias[0] == NULL) {
			stc("You have no aliases defined.\n\r", ch);
			return;
		}

		stc("Your current aliases are:\n\r", ch);

		for (pos = 0; pos < MAX_ALIAS; pos++) {
			if (rch->pcdata->alias[pos] == NULL
			    ||  rch->pcdata->alias_sub[pos] == NULL)
				break;

			sprintf(buf, "    %s:  %s\n\r", rch->pcdata->alias[pos],
			        rch->pcdata->alias_sub[pos]);
			stc(buf, ch);
		}

		return;
	}

	if (!str_prefix1("una", arg) || !str_cmp("alias", arg)) {
		stc("Sorry, that word is reserved.\n\r", ch);
		return;
	}

	if (argument[0] == '\0') {
		for (pos = 0; pos < MAX_ALIAS; pos++) {
			if (rch->pcdata->alias[pos] == NULL
			    ||  rch->pcdata->alias_sub[pos] == NULL)
				break;

			if (!str_cmp(arg, rch->pcdata->alias[pos])) {
				sprintf(buf, "%s aliases to '%s'.\n\r", rch->pcdata->alias[pos],
				        rch->pcdata->alias_sub[pos]);
				stc(buf, ch);
				return;
			}
		}

		stc("That alias is not defined.\n\r", ch);
		return;
	}

	if (!str_prefix1(argument, "delete") || !str_prefix1(argument, "prefix")) {
		stc("That shall not be done!\n\r", ch);
		return;
	}

	for (pos = 0; pos < MAX_ALIAS; pos++) {
		if (rch->pcdata->alias[pos] == NULL)
			break;

		if (!str_cmp(arg, rch->pcdata->alias[pos])) { /* redefine an alias */
			free_string(rch->pcdata->alias_sub[pos]);
			rch->pcdata->alias_sub[pos] = str_dup(argument);
			sprintf(buf, "%s is now realiased to '%s'.\n\r", arg, argument);
			stc(buf, ch);
			return;
		}
	}

	if (pos >= MAX_ALIAS) {
		stc("Sorry, you have reached the alias limit.\n\r", ch);
		return;
	}

	/* make a new alias */
	rch->pcdata->alias[pos]            = str_dup(arg);
	rch->pcdata->alias_sub[pos]        = str_dup(argument);
	sprintf(buf, "%s is now aliased to '%s'.\n\r", arg, argument);
	stc(buf, ch);
}


void do_unalias(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *rch;
	char arg[MAX_INPUT_LENGTH];
	int pos;
	bool found = FALSE;

	if (ch->desc == NULL)
		rch = ch;
	else
		rch = ch->desc->original ? ch->desc->original : ch;

	if (IS_NPC(rch))
		return;

	argument = one_argument(argument, arg);

	if (arg == '\0') {
		stc("Unalias what?\n\r", ch);
		return;
	}

	for (pos = 0; pos < MAX_ALIAS; pos++) {
		if (rch->pcdata->alias[pos] == NULL)
			break;

		if (found) {
			rch->pcdata->alias[pos - 1]           = rch->pcdata->alias[pos];
			rch->pcdata->alias_sub[pos - 1]       = rch->pcdata->alias_sub[pos];
			rch->pcdata->alias[pos]             = NULL;
			rch->pcdata->alias_sub[pos]         = NULL;
			continue;
		}

		if (!strcmp(arg, rch->pcdata->alias[pos])) {
			stc("Alias removed.\n\r", ch);
			free_string(rch->pcdata->alias[pos]);
			free_string(rch->pcdata->alias_sub[pos]);
			rch->pcdata->alias[pos] = NULL;
			rch->pcdata->alias_sub[pos] = NULL;
			found = TRUE;
		}
	}

	if (!found)
		stc("No alias of that name to remove.\n\r", ch);
}















