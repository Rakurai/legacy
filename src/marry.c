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

/***************************************************************************
*       MARRY.C written by Ryouga for Vilaross Mud (baby.indstate.edu 4000)*
*       Please leave this and all other credit include in this package.    *
*       Email questions/comments to ryouga@jessi.indstate.edu              *
***************************************************************************/

/***************************************************************************
*      MARRY.C rewritten by Justin Rocha (Xenith) for Legacy               *
*      legacy.kyndig.com 3000   http://legacy.kyndig.com/                  *
*      xenith@xenith.org        legacy@kyndig.com                          *
***************************************************************************/

#include "find.h"
#include "merc.h"
#include "Format.hpp"

void do_marry(Character *ch, String argument)
{
	Character *victim;
	Character *victim2;

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1.empty() || arg2.empty()) {
		stc("Syntax: marry <char1> <char2>\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg1, VIS_PLR)) == nullptr) {
		stc("The first person mentioned isn't playing.\n", ch);
		return;
	}

	if ((victim2 = get_player_world(ch, arg2, VIS_PLR)) == nullptr) {
		stc("The second person mentioned isn't playing.\n", ch);
		return;
	}

	if (victim->pcdata->spouse[0] == '\0'
	    || victim2->pcdata->spouse[0] == '\0'
	    || victim->pcdata->spouse != victim2->name
	    || victim2->pcdata->spouse != victim->name) {
		stc("They are not engaged.\n", ch);
		return;
	}

	if (victim->pcdata->plr_flags.has(PLR_MARRIED) || victim2->pcdata->plr_flags.has(PLR_MARRIED)) {
		stc("They are already married.\n", ch);
		return;
	}

	stc("You pronounce them husband and wife!\n", ch);
	ptc(ch, "You marry %s and %s.\n", victim->name, victim2->name);
	stc("You say the big 'I do!'\n", victim);
	ptc(victim, "You are now married to %s.\n", victim2->name);
	stc("You say the big 'I do!'\n", victim2);
	ptc(victim2, "You are now married to %s.\n", victim->name);
	victim->pcdata->plr_flags += PLR_MARRIED;
	victim2->pcdata->plr_flags += PLR_MARRIED;
	save_char_obj(victim);
	save_char_obj(victim2);
	return;
}

void do_divorce(Character *ch, String argument)
{
	Character *victim;
	Character *victim2;

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1.empty() || arg2.empty()) {
		stc("Syntax: divorce <char1> <char2>\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg1, VIS_PLR)) == nullptr) {
		stc("The first person mentioned isn't playing.\n", ch);
		return;
	}

	if ((victim2 = get_player_world(ch, arg2, VIS_PLR)) == nullptr) {
		stc("The second person mentioned isn't playing.\n", ch);
		return;
	}

	if (!victim->pcdata->plr_flags.has(PLR_MARRIED) || !victim2->pcdata->plr_flags.has(PLR_MARRIED)) {
		stc("They aren't even married.\n", ch);
		return;
	}

	if (victim->pcdata->spouse != victim2->name) {
		stc("They aren't even married to each other.\n", ch);
		return;
	}

	stc("You hand them their papers.\n", ch);
	ptc(ch, "You divorce %s and %s.\n", victim->name, victim2->name);
	stc("Your divorce is final.\n", victim);
	ptc(victim, "You are now divorced from %s.\n", victim2->name);
	stc("Your divorce is final.\n", victim2);
	ptc(victim2, "You are now divorced from %s.\n", victim->name);
	victim->pcdata->spouse.erase();
	victim2->pcdata->spouse.erase();
	victim->pcdata->plr_flags -= PLR_MARRIED;
	victim2->pcdata->plr_flags -= PLR_MARRIED;
	save_char_obj(victim);
	save_char_obj(victim2);
	return;
}

void do_spousetalk(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC(ch)) {
		do_huh(ch);
		return;
	}

	if (!ch->pcdata->plr_flags.has(PLR_MARRIED)) {
		new_color(ch, CSLOT_CHAN_SPOUSE);
		stc("But you aren't married.\n", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (ch->pcdata->spouse[0] == '\0') {
		new_color(ch, CSLOT_CHAN_SPOUSE);
		stc("You aren't even engaged. How do you expect to talk to your other half?\n", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	if (argument.empty()) {
		new_color(ch, CSLOT_CHAN_SPOUSE);
		stc("What do you wish to tell your other half?\n", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}
	else { /* message sent */
		Character *victim;

		/* Us married folk wanna see our spouses - Lotus */

		for (victim = char_list; victim != nullptr ; victim = victim->next) {
			if (victim->in_room != nullptr &&
			    !IS_NPC(victim) &&
			    victim->name.has_words(ch->pcdata->spouse))
				break;
		}

		if (victim != nullptr) {
			new_color(ch, CSLOT_CHAN_SPOUSE);
			new_color(victim, CSLOT_CHAN_SPOUSE);
			Format::sprintf(buf, "%s says to you, '%s'\n" , ch->name, argument);
			stc(buf, victim);
			Format::sprintf(buf, "You say to %s, '%s'\n", ch->pcdata->spouse, argument);
			stc(buf, ch);
			set_color(ch, WHITE, NOBOLD);
			set_color(victim, WHITE, NOBOLD);
		}
		else {
			new_color(ch, CSLOT_CHAN_SPOUSE);
			stc("Your spouse is not here.\n", ch);
			set_color(ch, WHITE, NOBOLD);
		}

		return;
	}
}

void do_propose(Character *ch, String argument)
{
	Character *victim;

	if (IS_NPC(ch)) {
		do_huh(ch);
		return;
	}

	if (argument.empty()) {
		stc("Propose to who?\n", ch);
		return;
	}

	if (ch->pcdata->plr_flags.has(PLR_MARRIED)) {
		stc("You are already married.\n", ch);
		return;
	}

	if (ch->pcdata->spouse[0] != '\0') {
		stc("You are already engaged.\n", ch);
		return;
	}

	if ((victim = get_char_here(ch, argument, VIS_CHAR)) == nullptr) {
		stc("They aren't in the room.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		stc("That's just silly.\n", ch);
		return;
	}

	if (victim == ch) {
		stc("You utterly reject yourself.\n", ch);
		return;
	}

	if (victim->pcdata->plr_flags.has(PLR_MARRIED)) {
		stc("They are already married.\n", ch);
		return;
	}

	if (victim->pcdata->spouse[0] != '\0') {
		stc("They are already engaged.\n", ch);
		return;
	}

	ch->pcdata->propose = victim->name;
	act("You propose marriage to $M.", ch, nullptr, victim, TO_CHAR);
	act("$n gets down on one knee and proposes to $N.", ch, nullptr, victim, TO_NOTVICT);
	act("$n asks you quietly 'Will you marry me?'", ch, nullptr, victim, TO_VICT);
	return;
}

void do_accept(Character *ch, String argument)
{
	Character *victim;
	char buf[MSL];
	Descriptor *d;

	if (IS_NPC(ch)) {
		do_huh(ch);
		return;
	}

	if (argument.empty()) {
		stc("Accept who's proposal?\n", ch);
		return;
	}

	if (ch->pcdata->plr_flags.has(PLR_MARRIED)) {
		stc("You are already married.\n", ch);
		return;
	}

	if (ch->pcdata->spouse[0] != '\0') {
		stc("You are already engaged.\n", ch);
		return;
	}

	if ((victim = get_char_here(ch, argument, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		stc("Now why would they propose to you?\n", ch);
		return;
	}

	if (victim == ch) {
		stc("Not a good idea there.\n", ch);
		return;
	}

	if (!victim->pcdata->propose[0] || victim->pcdata->propose != ch->name) {
		stc("They haven't proposed to you.\n", ch);
		return;
	}

	victim->pcdata->propose.erase();
	ch->pcdata->propose.erase();
	victim->pcdata->spouse = ch->name;
	ch->pcdata->spouse = victim->name;
	act("You accept $S offer of marriage.  Woohoo!", ch, nullptr, victim, TO_CHAR);
	act("$n accepts $N's offer of marriage.  Woohoo!", ch, nullptr, victim, TO_NOTVICT);
	act("$n accepts your offer of marriage.  Woohoo!", ch, nullptr, victim, TO_VICT);
	Format::sprintf(buf, "{W[FYI] %s and %s are now engaged!  Congratulations!{x\n", ch->name, victim->name);

	for (d = descriptor_list; d != nullptr; d = d->next) {
		Character *msgvict;
		msgvict = d->original ? d->original : d->character;

		if (IS_PLAYING(d) &&
		    !msgvict->comm_flags.has(COMM_NOANNOUNCE) &&
		    !msgvict->comm_flags.has(COMM_QUIET))
			stc(buf, msgvict);
	}

	save_char_obj(ch);
	save_char_obj(victim);
	return;
}

void do_reject(Character *ch, String argument)
{
	Character *victim;

	if (IS_NPC(ch)) {
		do_huh(ch);
		return;
	}

	if (argument.empty()) {
		stc("Reject who's proposal?\n", ch);
		return;
	}

	if (ch->pcdata->plr_flags.has(PLR_MARRIED)) {
		stc("You are already married.\n", ch);
		return;
	}

	if (ch->pcdata->spouse[0] != '\0') {
		stc("You are already engaged.\n", ch);
		return;
	}

	if ((victim = get_char_here(ch, argument, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		stc("Now why would they propose to you?\n", ch);
		return;
	}

	if (victim == ch) {
		stc("Not a good idea there.\n", ch);
		return;
	}

	if (victim->pcdata->propose != ch->name) {
		stc("They haven't proposed to you.\n", ch);
		return;
	}

	victim->pcdata->propose.erase();
	ch->pcdata->propose.erase();
	act("You reject $S offer of marriage.", ch, nullptr, victim, TO_CHAR);
	act("$n rejects $N's offer of marriage.", ch, nullptr, victim, TO_NOTVICT);
	act("$n rejects your offer of marriage.", ch, nullptr, victim, TO_VICT);
	save_char_obj(ch);
	save_char_obj(victim);
	return;
}

void do_breakup(Character *ch, String argument)
{
	Character *victim;
	char buf[MSL];
	Descriptor *d;

	if (IS_NPC(ch)) {
		do_huh(ch);
		return;
	}

	if (argument.empty()) {
		stc("Breakup with who?\n", ch);
		return;
	}

	if (ch->pcdata->plr_flags.has(PLR_MARRIED)) {
		stc("You are married, you have to get a divorce.\n", ch);
		return;
	}

	if (ch->pcdata->spouse[0] == '\0') {
		stc("You aren't engaged.\n", ch);
		return;
	}

	if ((victim = get_char_here(ch, argument, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (victim == ch) {
		stc("Not with yourself.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		stc("Bad idea.\n", ch);
		return;
	}

	if (ch->pcdata->spouse != victim->name
	    || victim->pcdata->spouse[0] == '\0'
	    || victim->pcdata->spouse != ch->name) {
		stc("They aren't even engaged to you.\n\t", ch);
		return;
	}

	victim->pcdata->spouse.erase();
	ch->pcdata->spouse.erase();
	act("You break off your engagement with $M.", ch, nullptr, victim, TO_CHAR);
	act("$n breaks off $s engagement with $N.", ch, nullptr, victim, TO_NOTVICT);
	act("$n breaks off $s engagement with you.", ch, nullptr, victim, TO_VICT);
	Format::sprintf(buf, "{W[FYI] %s and %s have broken off their engagement.{x\n", ch->name, victim->name);

	for (d = descriptor_list; d != nullptr; d = d->next) {
		Character *msgvict;
		msgvict = d->original ? d->original : d->character;

		if (IS_PLAYING(d) &&
		    !msgvict->comm_flags.has(COMM_NOANNOUNCE) &&
		    !msgvict->comm_flags.has(COMM_QUIET))
			stc(buf, msgvict);
	}

	save_char_obj(ch);
	save_char_obj(victim);
	return;
}
