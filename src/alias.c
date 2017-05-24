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

#include "merc.h"
#include "Format.hpp"

char    *get_multi_command     args((DESCRIPTOR_DATA *d, const char *argument));

void add_alias(PC_DATA *pch, const char *text, const char *sub) {
	pch->alias[text] = String(sub);
}

void remove_alias(PC_DATA *pch, const char *text) {
	pch->alias.erase(text);
}

/* does aliasing and other fun stuff */
void substitute_alias(DESCRIPTOR_DATA *d, const char *argument)
{
	CHAR_DATA *ch;
	char buf[MAX_STRING_LENGTH];
	char prefix[2 * MAX_INPUT_LENGTH];
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
			stc("{PLine too long, truncated!{x\n", ch);

		sprintf(prefix, "%s %s", ch->prefix, argument);
		prefix[MAX_INPUT_LENGTH - 1] = '\0';
		argument = prefix;
	}

	if (IS_NPC(ch)
	    ||  !str_prefix1("alias", argument) || !str_prefix1("una", argument)
	    ||  !str_prefix1("prefix", argument)) {
		interpret(d->character, argument);
		return;
	}

	strcpy(buf, argument); // the default, in case we don't find an alias

	std::string input = argument;
	char word[MAX_INPUT_LENGTH];
	const char *remainder = one_argument(argument, word);
	auto search = ch->pcdata->alias.find(word);

	if (search != ch->pcdata->alias.end()) {
		strcpy(buf, (*search).second.c_str());
		strcat(buf, " ");
		strcat(buf, remainder);

		strcpy(buf, get_multi_command(d, buf));

		if (strlen(buf) > MAX_INPUT_LENGTH) {
			stc("Alias substitution too long. Truncated.\n", ch);
			buf[MAX_INPUT_LENGTH - 1] = '\0';
		}
	}

	interpret(d->character, buf);
} /* end substitute_alias() */

void do_alia(CHAR_DATA *ch, const char *argument)
{
	stc("I'm sorry, alias must be entered in full.\n", ch);
	return;
}

void do_alias(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *rch;
	char arg[MAX_INPUT_LENGTH];

	if (ch->desc == NULL)
		rch = ch;
	else
		rch = ch->desc->original ? ch->desc->original : ch;

	if (IS_NPC(rch))
		return;

	/* Fix the alias bug and add wiznet for those cheaters - Lotus */
	for (const char *x = argument; *x != '\0'; x++) {
		if (*x == '~') {
			wiznet("$N is attempting to use the tilde in alias cheat.", ch,
			       NULL, WIZ_CHEAT, 0, GET_RANK(ch));
			break;
		}
	}

	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		if (rch->pcdata->alias.empty()) {
			stc("You have no aliases defined.\n", ch);
			return;
		}

		stc("Your current aliases are:\n", ch);

		for (auto it = rch->pcdata->alias.begin(); it != rch->pcdata->alias.end(); it++)
			ptc(ch, "    %s:  %s\n", (*it).first.c_str(), (*it).second.c_str());

		return;
	}

	if (!str_prefix1("una", arg) || !str_cmp("alias", arg)) {
		stc("Sorry, that word is reserved.\n", ch);
		return;
	}

	if (argument[0] == '\0') {
		auto search = rch->pcdata->alias.find(arg);

		if (search == rch->pcdata->alias.end())
			stc("That alias is not defined.\n", ch);
		else
			ptc(ch, "%s aliases to '%s'.\n", (*search).first.c_str(), (*search).second.c_str());

		return;
	}

	if (!str_prefix1(argument, "delete") || !str_prefix1(argument, "prefix")) {
		stc("That shall not be done!\n", ch);
		return;
	}

	auto search = rch->pcdata->alias.find(arg);

	ptc(ch, "%s is now %saliased to '%s'.\n",
		arg,
		search == rch->pcdata->alias.end() ? "" : "re",
		argument);

	add_alias(rch->pcdata, arg, argument);
}

void do_unalias(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *rch;
	char arg[MAX_INPUT_LENGTH];

	if (ch->desc == NULL)
		rch = ch;
	else
		rch = ch->desc->original ? ch->desc->original : ch;

	if (IS_NPC(rch))
		return;

	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Unalias what?\n", ch);
		return;
	}

	auto search = rch->pcdata->alias.find(arg);

	if (search == rch->pcdata->alias.end())
		stc("No alias of that name to remove.\n", ch);
	else
		stc("Alias removed.\n", ch);

	remove_alias(rch->pcdata, arg);
}

