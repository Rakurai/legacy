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
#include "tables.h"
#include "magic.h"

DECLARE_DO_FUN(do_rset);
DECLARE_DO_FUN(do_mset);
DECLARE_DO_FUN(do_oset);
DECLARE_DO_FUN(do_sset);
DECLARE_DO_FUN(do_evoset);
DECLARE_DO_FUN(do_raffset);
DECLARE_DO_FUN(do_extraset);


/* RT set replaces sset, mset, oset, rset and cset */

void do_set(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Syntax:\n\r", ch);
		stc("  set mob   <name> <field> <value>\n\r", ch);
		stc("  set obj   <name> <field> <value>\n\r", ch);
		stc("  set room  <room> <field> <value>\n\r", ch);

		if (IS_HEAD(ch)) {
			stc("  set plr   <name> <field> <value>\n\r", ch);
			stc("  set skill <name> <spell or skill> <value>\n\r", ch);
			stc("  set evol  <name> <spell or skill> <value>\n\r", ch);
			stc("  set raff  <name> <remort affect>\n\r", ch);
			stc("  set extra <name> <skill>\n\r", ch);
		}

		return;
	}

	if (!str_prefix1(arg, "mobile") || !str_prefix1(arg, "character")) {
		do_mset(ch, argument);
		return;
	}

	if (!str_prefix1(arg, "room")) {
		do_rset(ch, argument);
		return;
	}

	if (!str_prefix1(arg, "object")) {
		do_oset(ch, argument);
		return;
	}

	/* non-heads of departments can only set mobs, objects, rooms */
	if (!IS_HEAD(ch)) {
		do_set(ch, "");
		return;
	}

	if (!str_prefix1(arg, "plr") || !str_prefix1(arg, "player")) {
		do_mset(ch, argument);
		return;
	}

	if (!str_prefix1(arg, "skill") || !str_prefix1(arg, "spell")) {
		do_sset(ch, argument);
		return;
	}

	if (!str_prefix1(arg, "evolve") || !str_prefix1(arg, "evolution")) {
		do_evoset(ch, argument);
		return;
	}

	if (!str_prefix1(arg, "raffect")) {
		do_raffset(ch, argument);
		return;
	}

	if (!str_prefix1(arg, "extraclass")) {
		do_extraset(ch, argument);
		return;
	}

	do_set(ch, "");
}


void do_sset(CHAR_DATA *ch, char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	char buf[1024];
	CHAR_DATA *victim;
	int value;
	int sn;
	bool fAll;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		stc("Syntax:\n\r", ch);
		stc("  set skill <player> <spell or skill> <value>\n\r", ch);
		stc("  set skill <player> all <value>\n\r", ch);
		stc("   (use the name of the skill, not the number)\n\r", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == NULL) {
		sprintf(buf, "After searching all over the mud, player %s could not be found.\n\r",
		        arg1);
		stc(buf, ch);
		return;
	}

	if (IS_NPC(victim)) {
		stc("I'm sorry, you cannot set a mobile's skills.\n\r", ch);
		return;
	}

	fAll = !str_cmp(arg2, "all");
	sn   = 0;

	if (!fAll && (sn = skill_lookup(arg2)) < 0) {
		sprintf(buf, "%s is not a valid skill or spell.\n\r", arg2);
		stc(buf, ch);
		return;
	}

	/*
	 * Snarf the value.
	 */
	if (!is_number(arg3)) {
		sprintf(buf, "The third argument given, '%s', is not numeric.\n\r", arg3);
		stc(buf, ch);
		return;
	}

	value = atoi(arg3);

	if (value < 0 || value > 100) {
		sprintf(buf, "You entered %s, the valid range is between 0 and 100.\n\r", arg3);
		stc(buf, ch);
		return;
	}

	if (fAll) {
		for (sn = 0; sn < MAX_SKILL; sn++) {
			if (skill_table[sn].name != NULL)
				victim->pcdata->learned[sn]     = value;
		}

		sprintf(buf, "All of %s's skills and spells set to %d.\n\r", victim->name, value);
	}
	else {
		victim->pcdata->learned[sn] = value;
		sprintf(buf, "%s's %s %s set to %d.\n\r",
		        victim->name,
		        skill_table[sn].name,
		        skill_table[sn].spell_fun != spell_null ? "Spell" : "Skill",
		        value);
	}

	stc(buf, ch);
} /* end do_sset() */


void do_evoset(CHAR_DATA *ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	int value, sn;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);

	if (arg1[0] == '\0' || (arg2[0] != '\0' && arg3[0] == '\0')) {
		stc("Syntax:\n\r", ch);
		stc("  set evolve <player>     (lists all evolved on a player)\n\r", ch);
		stc("  set evolve <player> <spell or skill> <value>\n\r", ch);
		stc("   (use the name of the skill, not the number)\n\r", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == NULL) {
		ptc(ch, "After searching all over the mud, player %s could not be found.\n\r", arg1);
		return;
	}

	if (IS_NPC(victim)) {
		stc("I'm sorry, you cannot set a mobile's skill evolution.\n\r", ch);
		return;
	}

	if (arg2[0] == '\0') {
		extern int can_evolve args((CHAR_DATA * ch, int sn));
		BUFFER *buffer;
		int x, can;
		buffer = new_buf();
		add_buf(buffer, "They have the following skills and spells evolved:\n\r\n\r");

		for (x = 0; x < MAX_SKILL; x++) {
			if (skill_table[x].name == NULL)
				break;

			if ((can = can_evolve(victim, x)) == -1)
				continue;

			sprintf(buf, "They have %s at %d%%, evolution %d.\n\r",
			        skill_table[x].name,
			        victim->pcdata->learned[x],
			        victim->pcdata->evolution[x]);
			add_buf(buffer, buf);

			if (can == 1)
				sprintf(buf, "They need %d skill points to evolve %s to %d.\n\r",
				        victim->pcdata->evolution[x] == 1 ?
				        skill_table[x].evocost_sec[victim->class] :
				        skill_table[x].evocost_pri[victim->class],
				        skill_table[x].name,
				        victim->pcdata->evolution[x] + 1);

			add_buf(buffer, buf);
			add_buf(buffer, "\n\r");
		}

		page_to_char(buf_string(buffer), ch);
		free_buf(buffer);
		return;
	}

	if ((sn = skill_lookup(arg2)) < 0) {
		ptc(ch, "%s is not a valid skill or spell.\n\r", arg2);
		return;
	}

	if (!is_number(arg3)) {
		ptc(ch, "The third argument given, '%s', is not numeric.\n\r", arg3);
		return;
	}

	if (skill_table[sn].evocost_sec[victim->class] <= 0 && !IS_IMMORTAL(victim)) {
		ptc(ch, "%ss cannot evolve %s.\n\r",
		    capitalize(class_table[victim->class].name), skill_table[sn].name);
		return;
	}

	value = atoi(arg3);

	if (value < 1 || value > 4) {
		ptc(ch, "You entered %s, the valid range is between 1 and 4.\n\r", arg3);
		return;
	}

	if (skill_table[sn].evocost_pri[victim->class] <= 0 && value > 2 && !IS_IMMORTAL(victim)) {
		stc("Secondary classes cannot evolve a skill or spell past 2.\n\r", ch);
		return;
	}

	victim->pcdata->evolution[sn] = value;
	ptc(ch, "%s's %s %s has been set to evolution %d.\n\r", victim->name, skill_table[sn].name,
	    skill_table[sn].spell_fun != spell_null ? "spell" : "skill", value);
} /* end do_evoset() */


void do_raffset(CHAR_DATA *ch, char *argument)
{
	extern void fix_blank_raff(CHAR_DATA * ch, int start);
	extern void roll_one_raff(CHAR_DATA * ch, CHAR_DATA * victim, int place);
	extern void roll_raffects(CHAR_DATA * ch, CHAR_DATA * victim);
	extern void rem_raff_affect(CHAR_DATA * ch, int index);
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int i, index;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0') {
		stc("Syntax:\n\r", ch);
		stc("  set raffect                    (displays syntax)\n\r", ch);
		stc("  set raffect list               (displays a list of remort affects)\n\r", ch);
		stc("  set raffect <player>           (displays list of set remort affects)\n\r", ch);
		stc("  set raffect <player> none      (clears all remort affects)\n\r", ch);
		stc("  set raffect <player> rollone   (rolls one random remort affect)\n\r", ch);
		stc("  set raffect <player> reroll    (rerolls all remort affects)\n\r", ch);
		stc("  set raffect <player> <remort affect number>\n\r", ch);
		stc("      (toggles remort affect on or off, use the raffect number)\n\r", ch);
		return;
	}

	if (!str_cmp(arg1, "list")) {
		for (i = 1; i < MAX_RAFFECTS; i++)
			ptc(ch, "{W[{C%3d{W] %10s {x%s\n\r",
			    raffects[i].id, str_dup(raffects[i].shortname), str_dup(raffects[i].description));

		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == NULL) {
		stc("They are not here.\n\r", ch);
		return;
	}

	if (IS_NPC(victim)) {
		stc("Mobiles do not have remort affects.\n\r", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		stc("Immortals do not need remort affects.\n\r", ch);
		return;
	}

	if (!IS_REMORT(victim)) {
		stc("Only remorts can have remort affects.\n\r", ch);
		return;
	}

	/* before we even start, let's fix holes in their raffects */
	fix_blank_raff(victim, 0);

	if (arg2[0] == '\0') {
		if (victim->pcdata->raffect == NULL) {
			stc("That player does not have any remort affects.\n\r", ch);
			return;
		}

		stc("They have been blessed(?) with the following remort affects:\n\r\n\r", ch);

		for (i = 0; i < ((victim->pcdata->remort_count / 10) + 1); i++) {
			int rindex;
			rindex = raff_lookup(victim->pcdata->raffect[i]);

			if (!rindex)
				stc("{W[{C000{W] {xNone\n\r", ch);
			else
				ptc(ch, "{W[{C%3d{W] {x%s\n\r", raffects[rindex].id, str_dup(raffects[rindex].description));
		}

		return;
	}

	if (!str_cmp(arg2, "none") || !str_cmp(arg2, "reroll")) {
		/* loop through and set all remort affects to 0 */
		for (i = 0; i < 10; i++) {
			rem_raff_affect(victim, raff_lookup(victim->pcdata->raffect[i]));
			victim->pcdata->raffect[i] = 0;
		}

		if (!str_cmp(arg2, "none")) {
			stc("The player's remort affects have been cleared.\n\r", ch);
			return;
		}

		stc("You reroll their remort affects...\n\r", ch);
		stc("An immortal rerolls your remort affects...\n\r", victim);
		roll_raffects(ch, victim);
		return;
	}

	if (!str_cmp(arg2, "rollone")) {
		/* find the first blank spot, and add a random raffect */
		for (i = 0; i < ((victim->pcdata->remort_count / 10) + 1); i++) {
			if (victim->pcdata->raffect[i] < 1) {
				stc("You add a random remort affect...\n\r", ch);
				stc("An immortal adds a random remort affect to you...\n\r", victim);
				roll_one_raff(ch, victim, i);
				return;
			}
		}

		stc("That player has the maximum number of remort affects.\n\r", ch);
		return;
	}

	if (!is_number(arg2)) {
		stc("Please use the number id for remort affects.  Type 'set raffect list' for a list.\n\r", ch);
		return;
	}

	index = raff_lookup(atoi(arg2));

	/* if they have it, loop through and remove it, and fix the blank spot */
	if (HAS_RAFF(victim, atoi(arg2))) {
		for (i = 0; i < ((victim->pcdata->remort_count / 10) + 1); i++) {
			if (victim->pcdata->raffect[i] == atoi(arg2)) {
				rem_raff_affect(victim, index);
				victim->pcdata->raffect[i] = 0;
				fix_blank_raff(victim, i);
				stc("Remort affect removed.\n\r", ch);
				return;
			}
		}

		stc("Couldn't find that remort affect on the player.\n\r", ch);
		bug("Couldn't find raffect on player.", 0);
		return;
	}

	/* now check if it's valid before trying to add it */
	if (!index) {
		stc("That is not a valid remort affect.  Type 'set raffect list' for a list.\n\r", ch);
		return;
	}

	/* they don't have it, loop through, find the first empty space, and add it */
	for (i = 0; i < ((victim->pcdata->remort_count / 10) + 1); i++) {
		if (victim->pcdata->raffect[i] < 1) {
			if (raffects[index].group > 0) {
				if (HAS_RAFF_GROUP(victim, raffects[index].group)) {
					stc("They already have the opposite of that remort affect.\n\r", ch);
					return;
				}
			}

			if (raffects[index].add) {
				if ((raffects[index].id >= 900) && (raffects[index].id <= 949)) {
					if (IS_SET(victim->vuln_flags, raffects[index].add)) {
						stc("They have that vulnerability already.\n\r", ch);
						return;
					}
					else
						SET_BIT(victim->vuln_flags, raffects[index].add);
				}
				else if ((raffects[index].id >= 950) && (raffects[index].id <= 999)) {
					if (IS_SET(victim->res_flags, raffects[index].add)) {
						stc("They have that resistance already.\n\r", ch);
						return;
					}
					else
						SET_BIT(victim->res_flags, raffects[index].add);
				}
			}

			victim->pcdata->raffect[i] = atoi(arg2);
			stc("Remort affect added.\n\r", ch);
			return;
		}
	}

	/* all raffect slots for them are full */
	stc("That player has the maximum amount of remort affects.\n\r", ch);
	return;
}

void do_extraset(CHAR_DATA *ch, char *argument)
{
	extern void fix_blank_extraclass(CHAR_DATA * ch, int index);
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	BUFFER *output;
	CHAR_DATA *victim;
	int sn, x, i, gn, cn, col = 0;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0') {
		stc("Syntax:\n\r", ch);
		stc("  set extraclass                    (displays syntax)\n\r", ch);
		stc("  set extraclass list               (displays a list of extraclass skills)\n\r", ch);
		stc("  set extraclass <player>           (displays list of set extraclass skills)\n\r", ch);
		stc("  set extraclass <player> none      (clears all extraclass skills)\n\r", ch);
		stc("  set extraclass <player> <skill>\n\r", ch);
		stc("      (toggles one extraclass skill as gained or not)\n\r", ch);
		return;
	}

	output = new_buf();

	if (!str_cmp(arg1, "list")) {
		add_buf(output, "\n\r                      {BExtraclass Remort Skills{x\n\r");

		for (cn = 0; cn < MAX_CLASS; cn++) {
			sprintf(buf, "\n\r{W%s Skills{x\n\r    ", capitalize(class_table[cn].name));
			add_buf(output, buf);
			col = 0;

			for (gn = 0; gn < MAX_SKILL; gn++) {
				if (skill_table[gn].name == NULL)
					break;

				if (skill_table[gn].remort_class > 0 && skill_table[gn].remort_class == cn + 1) {
					sprintf(buf, "%-15s %-8d", skill_table[gn].name, skill_table[gn].rating[ch->class]);
					add_buf(output, buf);
				}
			}

			add_buf(output, "\n\r");
		}

		if (col % 3 != 0)
			add_buf(output, "\n\r\n\r");

		page_to_char(buf_string(output), ch);
		free_buf(output);
		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == NULL) {
		stc("They are not here.\n\r", ch);
		return;
	}

	if (IS_NPC(victim)) {
		stc("Mobiles do not have extraclass skills.\n\r", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		stc("Immortals do not need extraclass skills.\n\r", ch);
		return;
	}

	if (!IS_REMORT(victim)) {
		stc("Only remorts can have extraclass skills.\n\r", ch);
		return;
	}

	/* before we even start, let's fix holes in their exsk array */
	/*    fix_blank_extraclass(victim,0); */

	if (arg2[0] == '\0') {
		if ((victim->pcdata->extraclass[0] +
		     victim->pcdata->extraclass[1] +
		     victim->pcdata->extraclass[2] +
		     victim->pcdata->extraclass[3] +
		     victim->pcdata->extraclass[4]) > 0) {
			sprintf(buf, "Their current extraclass skill%s", victim->pcdata->extraclass[1] ? "s are" : " is");
			add_buf(output, buf);

			if (victim->pcdata->extraclass[0]) {
				sprintf(buf, " %s", skill_table[victim->pcdata->extraclass[0]].name);
				add_buf(output, buf);
			}

			for (x = 1; x < victim->pcdata->remort_count / 20 + 1; x++) {
				if (victim->pcdata->extraclass[x]) {
					sprintf(buf, ", %s", skill_table[victim->pcdata->extraclass[x]].name);
					add_buf(output, buf);
				}
			}

			add_buf(output, ".\n\r");
			page_to_char(buf_string(output), ch);
			free_buf(output);
			return;
		}

		stc("That player does not have any extraclass skills.\n\r", ch);
		return;
	}

	if (!str_cmp(arg2, "none")) {
		/* loop through and set all exsks to 0 */
		for (i = 0; i < 5; i++)
			victim->pcdata->extraclass[i] = 0;

		stc("The player's extraclass skills have been cleared.\n\r", ch);
		return;
	}

	/* Ok, now we check to see if the skill is a remort skill */
	if (!(sn = skill_lookup(arg2))) {
		stc("That is not even a valid skill, much less a remort skill.\n\r", ch);
		return;
	}

	/* if they have it, loop through and set it to 0 */
	if (HAS_EXTRACLASS(victim, sn)) {
		for (x = 0; x < victim->pcdata->remort_count / 20 + 1; x++) {
			if (victim->pcdata->extraclass[x] == sn) {
				victim->pcdata->extraclass[x] = 0;
				stc("Extraclass skill removed.\n\r", ch);
				/*              fix_blank_extraclass(victim,0); */
			}
		}

		return;
	}

	/* for debugging, put the checks for oddball stuff after the removal part */
	/* Is it a remort skill? */
	if (skill_table[sn].remort_class == 0) {
		stc("That is not a remort skill.\n\r", ch);
		return;
	}

	/* Is it outside of the player's class? */
	if (skill_table[sn].remort_class == victim->class + 1) {
		stc("They cannot have an extraclass skill within their class.  Pick another.\n\r", ch);
		return;
	}

	/* is it barred from that class? */
	if ((skill_table[sn].skill_level[victim->class] < 0)
	    || (skill_table[sn].rating[victim->class] < 0)) {
		stc("Their class cannot gain that skill.\n\r", ch);
		return;
	}

	/* they don't have it, let's put it in the first blank spot */
	for (x = 0; x < victim->pcdata->remort_count / 20 + 1; x++) {
		if (victim->pcdata->extraclass[x] <= 0) {
			victim->pcdata->extraclass[x] = sn;

			if (!victim->pcdata->learned[sn])
				victim->pcdata->learned[sn] = 1;

			stc("Extraclass skill added.\n\r", ch);
			ptc(victim, "You have been given %s as an extraclass remort skill.\n\r", skill_table[sn].name);
			return;
		}
	}

	/* can't find an empty spot, must have the max number of extraclass skills */
	stc("They have the maximum amount of extraclass skills for their remort level.\n\r", ch);
	return;
}

void do_mset(CHAR_DATA *ch, char *argument)
{
	char arg1[MIL], arg2[MIL], arg3[MIL], buf[1024];
	CHAR_DATA *victim;
	int value;
	smash_tilde(argument);
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	strcpy(arg3, argument);

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		stc("Syntax:\n\r", ch);
		stc("  set char <name> <field> <value>\n\r", ch);
		stc("  Field being one of:\n\r",                      ch);
		stc("    str int wis dex con chr sex level race nectimer\n\r"
		    "    group gold silver hp mana stamina align hunt\n\r"
		    "    damdice damsides damtype\n\r", ch);

		if (IS_HEAD(ch))
			stc("    class prac train thirst hunger drunk full\n\r"
			    "    pckills pckilled akills akilled killer thief\n\r"
			    "    quest squest qp sp rpp remort_count experience\n\r"
			    "    pkrank\n\r", ch);

		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == NULL) {
		ptc(ch, "After searching all over the mud, %s could not be found.\n\r", arg1);
		return;
	}

	if (!IS_NPC(victim) && !IS_IMP(ch) && victim != ch) {
		stc("You cannot set players at your level.\n\r", ch);
		return;
	}

	/* Snarf the value (which need not be numeric). */
	value = is_number(arg3) ? atoi(arg3) : -1;

	/* Set something. */

	if (!str_cmp(arg2, "str")) {
		if (value < 3 || value > get_max_train(victim, STAT_STR)) {
			ptc(ch, "Strength range is 3 to %d.\n\r", get_max_train(victim, STAT_STR));
			return;
		}

		victim->perm_stat[STAT_STR] = value;
		ptc(ch, "%s's strength set to %d.\n\r", victim->name, value);
		return;
	}

	if (!str_cmp(arg2, "int")) {
		if (value < 3 || value > get_max_train(victim, STAT_INT)) {
			ptc(ch, "Intelligence range is 3 to %d.\n\r", get_max_train(victim, STAT_INT));
			return;
		}

		victim->perm_stat[STAT_INT] = value;
		ptc(ch, "%s's intelligence set to %d.\n\r", victim->name, value);
		return;
	}

	if (!str_cmp(arg2, "wis")) {
		if (value < 3 || value > get_max_train(victim, STAT_WIS)) {
			ptc(ch, "Wisdom range is 3 to %d.\n\r", get_max_train(victim, STAT_WIS));
			return;
		}

		victim->perm_stat[STAT_WIS] = value;
		ptc(ch, "%s's wisdom set to %d.\n\r", victim->name, value);
		return;
	}

	if (!str_cmp(arg2, "dex")) {
		if (value < 3 || value > get_max_train(victim, STAT_DEX)) {
			ptc(ch, "Dexterity range is 3 to %d.\n\r", get_max_train(victim, STAT_DEX));
			return;
		}

		victim->perm_stat[STAT_DEX] = value;
		ptc(ch, "%s's dexterity set to %d.\n\r", victim->name, value);
		return;
	}

	if (!str_cmp(arg2, "con")) {
		if (value < 3 || value > get_max_train(victim, STAT_CON)) {
			ptc(ch, "Constitution range is 3 to %d.\n\r", get_max_train(victim, STAT_CON));
			return;
		}

		victim->perm_stat[STAT_CON] = value;
		ptc(ch, "%s's constitution set to %d.\n\r", victim->name, value);
		return;
	}

	if (!str_cmp(arg2, "chr")) {
		if (value < 3 || value > get_max_train(victim, STAT_CHR)) {
			ptc(ch, "Charisma range is 3 to %d.\n\r", get_max_train(victim, STAT_CHR));
			return;
		}

		victim->perm_stat[STAT_CHR] = value;
		ptc(ch, "%s's charisma set to %d.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "sex")) {
		switch (value) {
		case 0: sprintf(buf, "%s", "an it");            break;

		case 1: sprintf(buf, "%s", "a male");           break;

		case 2: sprintf(buf, "%s", "a female");         break;

		default:
			stc("Sex range is 0 to 2.\n\r", ch);
			return;
		}

		victim->sex = value;

		if (!IS_NPC(victim))
			victim->pcdata->true_sex = value;

		ptc(ch, "%s is now %s.\n\r", victim->name, buf);
		return;
	}

	if (!str_prefix1(arg2, "level")) {
		if (!IS_NPC(victim)) {
			stc("You may not set a player's level.\n\r"
			    "Use ADVANCE instead!\n\r", ch);
			return;
		}

		if (value < 1 || value > 1000) {
			stc("A valid level must be between 1 and 1000.\n\r", ch);
			return;
		}

		victim->level = value;
		ptc(ch, "%s is now level %d.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "gold")) {
		victim->gold = value;
		ptc(ch, "%s now has %d gold.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "silver")) {
		victim->silver = value;
		ptc(ch, "%s now has %d silver.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "hp")) {
		if (value < -10 || value > 30000) {
			stc("A valid hp value must be between -10 and 30,000 hit points.\n\r", ch);
			return;
		}

		victim->max_hit = value;

		if (!IS_NPC(victim))
			victim->pcdata->perm_hit = value;

		ptc(ch, "%s now has %d hit points.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "mana")) {
		if (value < -10 || value > 30000) {
			stc("A valid mana value must be between -10 and 30,000 mana points.\n\r", ch);
			return;
		}

		victim->max_mana = value;

		if (!IS_NPC(victim))
			victim->pcdata->perm_mana = value;

		ptc(ch, "%s now has %d mana.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "stamina")) {
		if (value < -10 || value > 30000) {
			stc("A valid stamina value must be between -10 and 30,000 stamina points.\n\r", ch);
			return;
		}

		victim->max_stam = value;

		if (!IS_NPC(victim))
			victim->pcdata->perm_stam = value;

		ptc(ch, "%s now has %d stamina points.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "align")) {
		if (value < -1000 || value > 1000) {
			stc("A valid alignment must be between -1000 and 1000.\n\r", ch);
			return;
		}

		victim->alignment = value;
		ptc(ch, "%s's alignment is now %d.\n\r", victim->name, value);
		return;
	}

	if (!str_cmp(arg2, "hunt")) {
		CHAR_DATA *hunted = 0;

		if (!IS_NPC(victim)) {
			stc("You can't make a player hunt!\n\r", ch);
			return;
		}

		if (!str_cmp(arg3, ".")) {
			ptc(ch, "%s is now hunting no one.\n\r", victim->name);
			return;
		}

		if ((hunted = get_char_area(victim, arg3, VIS_CHAR)) == NULL) {
			ptc(ch, "%s could not find %s.\n\r", victim->name, arg3);
			return;
		}

		victim->hunting = hunted;
		ptc(ch, "%s is now hunting %s!\n\r", victim->name, hunted->name);
		return;
	}

	if (!str_prefix1(arg2, "race")) {
		int race, loop = 0;
		race = race_lookup(arg3);

		if (race == 0 || (!IS_NPC(victim) && !race_table[race].pc_race)) {
			ptc(ch, "%s is not a valid %s race.\n\r", arg3, !IS_NPC(victim) ? "PC" : "NPC");
			ptc(ch, "Valid races are :\n\r\n\r");

			while (race_table[loop].name != NULL) {
				loop++;

				if ((!IS_NPC(victim) && !race_table[loop].pc_race)
				    || (IS_NPC(victim) && race_table[loop].pc_race))
					continue;

				ptc(ch, "%-10.10s\n\r", race_table[loop].name);
			}

			stc("\n\r", ch);
			return;
		}

		victim->race = race;
		ptc(ch, "%s is now a member of the %s race.\n\r", victim->name, race_table[race].name);
		return;
	}

	if (!str_prefix1(arg2, "group")) {
		if (!IS_NPC(victim)) {
			stc("You can't set a player's group!\n\r", ch);
			return;
		}

		victim->group = value;
		ptc(ch, "%s's group value is now set to %d.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "nectimer")) {
		if (!IS_NPC(victim)) {
			stc("You can't set a player's necromancer timer!\n\r", ch);
			return;
		}

		if (value < -1 || value > 100) {
			stc("A valid NecTimer value must be between  -1 and 100.\n\r", ch);
			return;
		}

		victim->nectimer = value;
		ptc(ch, "%s's necromancer timer is now set to %d.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "damdice")) {
		if (!IS_NPC(victim)) {
			stc("You cannot set a player's damage dice.\n\r", ch);
			return;
		}

		if (value < 1 || value > 30000) {
			stc("The value must be between 1 and 30000.\n\r", ch);
			return;
		}

		victim->damage[DICE_NUMBER] = value;
		ptc(ch, "%s now rolls %d dice for damage.\n\r", victim->short_descr, value);
		return;
	}

	if (!str_prefix1(arg2, "damsides")) {
		if (!IS_NPC(victim)) {
			stc("You cannot set a player's damage dice type.\n\r", ch);
			return;
		}

		if (value < 1 || value > 30000) {
			stc("The value must be between 1 and 30000.\n\r", ch);
			return;
		}

		victim->damage[DICE_TYPE] = value;
		ptc(ch, "%s now rolls %d sided dice for damage.\n\r", victim->short_descr, value);
		return;
	}

	if (!str_prefix1(arg2, "damtype")) {
		int i;

		if (!IS_NPC(victim)) {
			stc("You cannot set a player's damage type.\n\r", ch);
			return;
		}

		for (i = 0; attack_table[i].name != NULL; i++)
			if (!str_prefix1(arg3, attack_table[i].name))
				break;

		if (attack_table[i].name == NULL) {
			stc("That is not a valid damage type.\n\r"
			    "Use {Vtypelist attack{x to see valid types.\n\r", ch);
			return;
		}

		victim->dam_type = i;
		ptc(ch, "%s now hits with %s.\n\r", victim->short_descr, attack_table[i].noun);
		return;
	}

	if (IS_NPC(victim)) {
		stc("You can't set that on mobiles.\n\r", ch);
		do_mset(ch, "");
		return;
	}

	/*********************************************************
	            Head of department or self below here
	 *********************************************************/

	if (!str_prefix1(arg2, "class")) {
		int class = class_lookup(arg3);

		if (class == -1) {
			sprintf(buf, "Possible classes are: ");

			for (class = 0; class < MAX_CLASS; class++) {
				if (class > 0)
					strcat(buf, " ");

				strcat(buf, class_table[class].name);
			}

			strcat(buf, ".\n\r");
			stc(buf, ch);
			return;
		}

		victim->class = class;
		ptc(ch, "%s is now a %s.\n\r", victim->name, class_table[class].name);
		return;
	}

	if (!str_prefix1(arg2, "remort_count")) {
		if (value < 0 || value > 99) {
			stc("A valid remort count must be between 0 and 99.\n\r", ch);
			return;
		}

		victim->pcdata->remort_count = value;
		ptc(ch, "%s's remort count has been set to %d.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "hours")) {
		if (value < 0 || value > 10000) {
			stc("Hours must be between 0 and 10000.\n\r", ch);
			return;
		}

		victim->pcdata->played = value * 3600;
		ptc(ch, "%s has now played for %d hours. Shhh, don't tell anyone!\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "experience")) {
		victim->exp = value;
		ptc(ch, "%s now has %d experience points.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "quest")) {
		if (value < 0 || value > 30) {
			stc("A valid Next Quest range is 0 to 10.\n\r", ch);
			return;
		}

		victim->nextquest = value;
		ptc(ch, "%s can now quest again in %d minutes.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "squest")) {
		if (value < 0 || value > 30) {
			stc("A valid next skill quest range is 0 to 30.\n\r", ch);
			return;
		}

		victim->pcdata->nextsquest = value;
		ptc(ch, "%s can now skill quest again in %d minutes.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "qp")) {
		if (value < 0 || value > 5000) {
			stc("A valid quest point value must be between 0 and 5000.\n\r", ch);
			return;
		}

		victim->questpoints = value;
		ptc(ch, "%s now has %d quest points.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "sp")) {
		if (value < 0 || value > 5000) {
			stc("A valid skill point value must be between 0 and 5000.\n\r", ch);
			return;
		}

		victim->pcdata->skillpoints = value;
		ptc(ch, "%s now has %d skill points.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "rpp")) {
		if (value < 0 || value > 5000) {
			stc("A valid roleplay point value must be between 0 and 5000.\n\r", ch);
			return;
		}

		victim->pcdata->rolepoints = value;
		ptc(ch, "%s now has %d roleplay points.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "practice")) {
		if (value < 0 || value > 15000) {
			stc("A valid practice value must be between 0 and 15000 sessions.\n\r", ch);
			return;
		}

		victim->practice = value;
		ptc(ch, "%s now has %d practice sessions.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "train")) {
		if (value < 0 || value > 15000) {
			stc("A valid train value must be between 0 and 15000 sessions.\n\r", ch);
			return;
		}

		victim->train = value;
		ptc(ch, "%s now has %d training sessions.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "thirst")) {
		if (value < -1 || value > 100) {
			stc("A valid thirst value must be between -1 and 100.\n\r", ch);
			return;
		}

		victim->pcdata->condition[COND_THIRST] = value;
		ptc(ch, "%s now has a thirst of %d.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "drunk")) {
		if (value < -1 || value > 100) {
			stc("A valid drunk value must be between -1 and 100.\n\r", ch);
			return;
		}

		victim->pcdata->condition[COND_DRUNK] = value;
		ptc(ch, "%s now has a drunk value of %d.\n\r", victim->name, value);

		if (value > 70)
			ptc(ch, "%s is {PW{YA{GS{BT{CE{VD!{x\n\r", victim->name);

		return;
	}

	if (!str_prefix1(arg2, "full")) {
		if (value < -1 || value > 100) {
			stc("A valid full value must be between -1 and 100.\n\r", ch);
			return;
		}

		victim->pcdata->condition[COND_FULL] = value;
		ptc(ch, "%s now has a full value of %d.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "hunger")) {
		if (value < -1 || value > 100) {
			stc("A valid hunger value must be between -1 and 100.\n\r", ch);
			return;
		}

		victim->pcdata->condition[COND_HUNGER] = value;
		ptc(ch, "%s now has a hunger value of %d.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "pckills")) {
		if (value < 0 || value > 1000) {
			stc("A valid player kills value must be between 0 and 1000.\n\r", ch);
			return;
		}

		victim->pcdata->pckills = value;
		ptc(ch, "%s has now killed %d players.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "pckilled")) {
		if (value < 0 || value > 1000) {
			stc("A valid PCKilled value must be between 0 and 1000.\n\r", ch);
			return;
		}

		victim->pcdata->pckilled = value;
		ptc(ch, "%s's has now been killed by %d players.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "akills")) {
		if (value < 0 || value > 1000) {
			stc("A valid arena kills value must be between 0 and 1000.\n\r", ch);
			return;
		}

		victim->pcdata->arenakills = value;
		ptc(ch, "%s has now killed %d players in the arena.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "akilled")) {
		if (value < 0 || value > 1000) {
			stc("A valid arena killed value must be between 0 and 1000.\n\r", ch);
			return;
		}

		victim->pcdata->arenakilled = value;
		ptc(ch, "%s's has now been killed by %d players in the arena.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "pkrank")) {
		if (value < 0 || value > 5) {
			stc("PK Ranks must be between 0 and 5.\n\r", ch);
			return;
		}

		victim->pcdata->pkrank = value;
		ptc(ch, "%s now has a PK rank of %d.\n\r", victim->name, value);
		return;
	}

	if (!str_prefix1(arg2, "thief")) {
		if (value < 0 || value > MAX_THIEF) {
			ptc(ch, "A valid {BTHIEF{x time is between 0 and %d ticks.\n\r", MAX_THIEF);
			return;
		}

		victim->pcdata->flag_thief = value;
		SET_BIT(victim->act, PLR_NOPK);

		if (value) {
			SET_BIT(victim->act, PLR_THIEF);
			ptc(ch, "%s will have a {BTHIEF{x flag for %d ticks.\n\r", victim->name, value);
		}
		else {
			REMOVE_BIT(victim->act, PLR_THIEF);
			ptc(ch, "%s is no longer a {BTHIEF{x.\n\r", victim->name);
		}

		return;
	}

	if (!str_prefix1(arg2, "killer")) {
		if (value < 0 || value > MAX_KILLER) {
			ptc(ch, "A valid {RKILLER{x time is between 0 and %d ticks.\n\r", MAX_KILLER);
			return;
		}

		victim->pcdata->flag_killer = value;
		SET_BIT(victim->act, PLR_NOPK);

		if (value) {
			SET_BIT(victim->act, PLR_KILLER);
			ptc(ch, "%s will have a {RKILLER{x flag for %d ticks.\n\r", victim->name, value);
		}
		else {
			REMOVE_BIT(victim->act, PLR_KILLER);
			ptc(ch, "%s is no longer a {RKILLER{x.\n\r", victim->name);
		}

		return;
	}

	/* Generate usage message. */
	do_mset(ch, "");
}

void do_oset(CHAR_DATA *ch, char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	char buf[1024];
	OBJ_DATA *obj;
	int value;
	smash_tilde(argument);
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	strcpy(arg3, argument);

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		stc("Syntax:\n\r", ch);
		stc("  set obj <object> <field> <value>\n\r", ch);
		stc("  Field being one of:\n\r",                               ch);
		stc("    value0 value1 value2 value3 value4 (v1-v4)\n\r",      ch);
		stc("    condition level weight cost timer\n\r",          ch);
		return;
	}

	if ((obj = get_obj_world(ch, arg1)) == NULL) {
		sprintf(buf, "After searching all over the mud, %s could not be found.\n\r", arg1);
		stc(buf, ch);
		return;
	}

	/*
	 * Snarf the value (which need not be numeric).
	 */
	value = atoi(arg3);

	/*
	 * Set something.
	 */
	if (!str_cmp(arg2, "value0") || !str_cmp(arg2, "v0")) {
		obj->value[0] = value;
		ptc(ch, "%s's v0 has been set to %d.\n\r", obj->short_descr, value);
		return;
	}

	if (!str_cmp(arg2, "value1") || !str_cmp(arg2, "v1")) {
		obj->value[1] = value;
		ptc(ch, "%s's v1 has been set to %d.\n\r", obj->short_descr, value);
		return;
	}

	if (!str_cmp(arg2, "value2") || !str_cmp(arg2, "v2")) {
		/* Hack to keep Crush from crashing the mud */
		if ((obj->item_type == ITEM_FOUNTAIN
		     || obj->item_type == ITEM_DRINK_CON)
		    && value > MAX_DRINK) {
			ptc(ch, "The max for drinks and fountains is %d.\n\r", MAX_DRINK);
			return;
		}

		obj->value[2] = value;
		ptc(ch, "%s's v2 has been set to %d.\n\r", obj->short_descr, value);
		return;
	}

	if (!str_cmp(arg2, "value3") || !str_cmp(arg2, "v3")) {
		/* Hack to keep Crush from crashing the mud */
		if (obj->item_type == ITEM_WEAPON && value > MAX_WEAPON) {
			ptc(ch, "The max for weapons is %d.\n\r", MAX_WEAPON);
			return;
		}

		obj->value[3] = value;
		ptc(ch, "%s's v3 has been set to %d.\n\r", obj->short_descr, value);
		return;
	}

	if (!str_cmp(arg2, "value4") || !str_cmp(arg2, "v4")) {
		obj->value[4] = value;
		ptc(ch, "%s's v4 has been set to %d.\n\r", obj->short_descr, value);
		return;
	}

	if (!str_prefix1(arg2, "condition")) {
		if (value < -1 || value > 100) {
			stc("A valid condition value must be between -1 and 100.\n\r", ch);
			return;
		}

		obj->condition = value;
		sprintf(buf, "%s's condition value has been changed to %d.\n\r", obj->short_descr, value);
		stc(buf, ch);
		return;
	}

	if (!str_prefix1(arg2, "level")) {
		obj->level = value;
		sprintf(buf, "%s's level has been changed to %d.\n\r", obj->short_descr, value);
		stc(buf, ch);
		return;
	}

	if (!str_prefix1(arg2, "weight")) {
		obj->weight = value;
		sprintf(buf, "%s's weight has been changed to %d.\n\r", obj->short_descr, value);
		stc(buf, ch);
		return;
	}

	if (!str_prefix1(arg2, "cost")) {
		obj->cost = value;
		sprintf(buf, "%s's cost has been changed to %d.\n\r", obj->short_descr, value);
		stc(buf, ch);
		return;
	}

	if (!str_prefix1(arg2, "timer")) {
		obj->timer = value;
		sprintf(buf, "%s's timer value has been changed to %d.\n\r", obj->short_descr, value);
		stc(buf, ch);
		return;
	}

	/*
	 * Generate usage message.
	 */
	do_oset(ch, "");
} /* end do_oset() */


void do_rset(CHAR_DATA *ch, char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	char buf [MAX_STRING_LENGTH];
	ROOM_INDEX_DATA *location;
	int value;
	smash_tilde(argument);
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	strcpy(arg3, argument);

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		stc("Syntax:\n\r", ch);
		stc("  set room <location> <field> <value>\n\r", ch);
		stc("  Field being one of:\n\r",                ch);
		stc("    sector\n\r",                           ch);
		return;
	}

	if ((location = find_location(ch, arg1)) == NULL) {
		sprintf(buf, "No such location, %s.\n\r", arg1);
		stc(buf, ch);
		return;
	}

	if (!is_room_owner(ch, location) && ch->in_room != location
	    &&  room_is_private(location) && !IS_IMP(ch)) {
		sprintf(buf, "I'm sorry, but %s is a private room.\n\r", location->name);
		stc(buf, ch);
		return;
	}

	/*
	 * Snarf the value.
	 */
	if (!is_number(arg3)) {
		sprintf(buf, "The value entered, %s, is not numeric.\n\r", arg3);
		stc(buf, ch);
		return;
	}

	value = atoi(arg3);

	/*
	 * Set something.
	 */

	if (!str_prefix1(arg2, "sector")) {
		location->sector_type   = value;
		sprintf(buf, "%s's sector type has been changed to %d.\n\r", location->name, value);
		stc(buf, ch);
		return;
	}

	/*
	 * Generate usage message.
	 */
	do_rset(ch, "");
} /* end do_rset() */



/***** STAT COMMANDS *****/
void format_mstat(CHAR_DATA *ch, CHAR_DATA *victim)
{
	AFFECT_DATA *paf;

	if (IS_NPC(victim))
		ptc(ch, "Vnum: %d  Group: %d  Count: %d  Killed: %d\n\r",
		    victim->pIndexData->vnum,
		    victim->group,
		    victim->pIndexData->count, victim->pIndexData->killed);

	ptc(ch, "{WRoom: %d {CName: %s{x\n\r",
	    victim->in_room == NULL ? 0 : victim->in_room->vnum, victim->name);

	if (!IS_NPC(victim))
		ptc(ch, "{CRemort %d, {x", victim->pcdata->remort_count);

	ptc(ch, "{CLevel %d{x", victim->level);

	if (!IS_NPC(victim))
		ptc(ch, "{C Age: %d (%d hours){x", get_age(victim), get_play_hours(victim));

	stc("\n\r", ch);
	ptc(ch, "{MRace: %s  Sex: %s  Class: %s  Size: %s{x\n",
	    race_table[victim->race].name, sex_table[GET_SEX(victim)].name,
	    IS_NPC(victim) ? "mobile" : class_table[victim->class].name,
	    size_table[victim->size].name);

	if (!IS_NPC(victim))
		ptc(ch, "Killer: %d\tThief: %d\n\r",
		    victim->pcdata->flag_killer, victim->pcdata->flag_thief);

	ptc(ch, "Short description: %s{x\n\rLong  description: %s{x",
	    victim->short_descr, victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r");
	stc("\n\r", ch);
	ptc(ch, "{PStr: %-2d(%-2d)\t{BAC Pierce : %-10d{RHit Points: %d/%d\n\r",
	    victim->perm_stat[STAT_STR], get_curr_stat(victim, STAT_STR),
	    GET_AC(victim, AC_PIERCE), victim->hit, victim->max_hit);
	ptc(ch, "{PInt: %-2d(%-2d)\t{BAC Bash   : %-10d{RMana      : %d/%d\n\r",
	    victim->perm_stat[STAT_INT], get_curr_stat(victim, STAT_INT),
	    GET_AC(victim, AC_BASH), victim->mana, victim->max_mana);
	ptc(ch, "{PWis: %-2d(%-2d)\t{BAC Slash  : %-10d{RStamina   : %d/%d\n\r",
	    victim->perm_stat[STAT_WIS], get_curr_stat(victim, STAT_WIS),
	    GET_AC(victim, AC_SLASH), victim->stam, victim->max_stam);
	ptc(ch, "{PDex: %-2d(%-2d)\t{BAC Magic  : %-10d{HItems     : %d\n\r",
	    victim->perm_stat[STAT_DEX], get_curr_stat(victim, STAT_DEX),
	    GET_AC(victim, AC_EXOTIC), get_carry_number(victim));
	ptc(ch, "{PCon: %-2d(%-2d)\t{bPrac      : %-10d{HWeight    : %d\n\r",
	    victim->perm_stat[STAT_CON], get_curr_stat(victim, STAT_CON),
	    IS_NPC(victim) ? 0 : victim->practice, get_carry_weight(victim) / 10);
	ptc(ch, "{PChr: %-2d(%-2d)\t{bTrain     : %-10d{GHit Roll  : %d\n\r",
	    victim->perm_stat[STAT_CHR], get_curr_stat(victim, STAT_CHR),
	    IS_NPC(victim) ? 0 : victim->train , GET_HITROLL(victim));
	ptc(ch, "\t\t{YGold      : %-10ld{GDam Roll  : %d\n\r",
	    victim->gold, GET_DAMROLL(victim));
	ptc(ch, "{CThirst: %-8d{YSilver    : %-10ld{WAlignment : %d\n\r",
	    (!IS_NPC(victim) ? victim->pcdata->condition[COND_THIRST] : -1),
	    victim->silver, victim->alignment);
	ptc(ch, "{CHunger: %-8d{WSaves     : %-10dWimpy     : %d\n\r",
	    (!IS_NPC(victim) ? victim->pcdata->condition[COND_HUNGER] : -1),
	    victim->saving_throw, victim->wimpy);
	ptc(ch, "{CFull  : %-8d{cLast Level: %-10dMobTimer  : %d\n\r",
	    (!IS_NPC(victim) ? victim->pcdata->condition[COND_FULL] : -1),
	    (!IS_NPC(victim) ? victim->pcdata->last_level : -1),
	    (!IS_NPC(victim) ? victim->timer : -1));
	ptc(ch, "{CDrunk : %-8d{cExp       : %-10dPosition  : %s{x\n\r",
	    (!IS_NPC(victim) ? victim->pcdata->condition[COND_DRUNK] : -1),
	    victim->exp, position_table[victim->position].name);

	if (IS_NPC(victim))
		ptc(ch, "Damage: %dd%d  Message:  %s\n\r",
		    victim->damage[DICE_NUMBER], victim->damage[DICE_TYPE],
		    attack_table[victim->dam_type].noun);

	ptc(ch, "Fighting: %s\n\r", victim->fighting ? victim->fighting->name : "(none)");

	if (IS_NPC(victim) && victim->hunting != NULL)
		ptc(ch, "Hunting victim: %s (%s)\n\r",
		    IS_NPC(victim->hunting) ? victim->hunting->short_descr
		    : victim->hunting->name,
		    IS_NPC(victim->hunting) ? "MOB" : "PLAYER");

	if (!IS_NPC(victim))
		if (victim->pcdata->email != '\0')
			ptc(ch, "Email: %s\n\r", victim->pcdata->email);

	ptc(ch, "{WAct: %s\n\r", act_bit_name(victim->act));

	if (!IS_NPC(victim)) {
		ptc(ch, "{WPlr: %s\n\r", plr_bit_name(victim->pcdata->plr));

		if (victim->pcdata->cgroup && IS_IMP(ch))
			ptc(ch, "{WCommand Groups: %s\n\r", cgroup_bit_name(victim->pcdata->cgroup));
	}

	if (victim->comm)
		ptc(ch, "{WComm: %s\n\r", comm_bit_name(victim->comm));

	if (victim->revoke)
		ptc(ch, "{WRevoke: %s\n\r", revoke_bit_name(victim->revoke));

	if (victim->censor)
		ptc(ch, "{WCensor: %s\n\r", censor_bit_name(victim->censor));

	if (IS_NPC(victim) && victim->off_flags)
		ptc(ch, "{WOffense: %s\n\r", off_bit_name(victim->off_flags));

	if (victim->drain_flags)
		ptc(ch, "{YDrain: %s\n\r", imm_bit_name(victim->drain_flags));

	if (victim->imm_flags)
		ptc(ch, "{YImmune: %s\n\r", imm_bit_name(victim->imm_flags));

	if (victim->res_flags)
		ptc(ch, "{YResist: %s\n\r", imm_bit_name(victim->res_flags));

	if (victim->vuln_flags)
		ptc(ch, "{YVulnerable: %s\n\r", imm_bit_name(victim->vuln_flags));

	ptc(ch, "{xForm: %s\n\rParts: %s\n\r", form_bit_name(victim->form), part_bit_name(victim->parts));
	ptc(ch, "Master: %s  Leader: %s  Pet: %s\n\r",
	    victim->master  ? victim->master->name  : "(none)",
	    victim->leader  ? victim->leader->name  : "(none)",
	    victim->pet     ? victim->pet->name     : "(none)");

	if (!IS_NPC(victim) && victim->pcdata->mark_room != 0)
		ptc(ch, "MARKed room: %d\n\r", victim->pcdata->mark_room);

	if (IS_NPC(victim) && victim->spec_fun != 0)
		ptc(ch, "Mobile has special procedure %s.\n\r", spec_name(victim->spec_fun));

	if (!IS_NPC(victim) && victim->pcdata->raffect[0] != 0) {
		char buf[MAX_STRING_LENGTH];
		int raff, id;
		stc("{VRemort affects{x:\n\r", ch);
		id = (raff_lookup(victim->pcdata->raffect[0]));

		if (id == -1)
			sprintf(buf, "Unknown");
		else
			sprintf(buf, "%s", raffects[id].shortname);

		for (raff = 1; raff < ((victim->pcdata->remort_count / 10) + 1); raff++) {
			id = (raff_lookup(victim->pcdata->raffect[raff]));

			if (id == -1)
				sprintf(buf, "%s, Unknown", buf);
			else
				sprintf(buf, "%s, %s", buf, raffects[id].shortname);
		}

		sprintf(buf, "%s\n\r", buf);
		stc(buf, ch);
	}

	if (victim->affected_by)
		ptc(ch, "{bAffected by %s{x\n\r", affect_bit_name(victim->affected_by));

	for (paf = victim->affected; paf != NULL; paf = paf->next)
		ptc(ch, "{bSpell: '%s' modifies %s by %d for %d hours with bits %s, level %d, evolve %d.{x\n\r",
		    skill_table[(int) paf->type].name,
		    affect_loc_name(paf->location),
		    paf->modifier,
		    paf->duration + 1,
		    affect_bit_name(paf->bitvector),
		    paf->level,
		    paf->evolution);
}


void format_ostat(CHAR_DATA *ch, OBJ_DATA *obj)
{
	AFFECT_DATA *paf;
	int i;
	ptc(ch, "{CVnum: %d   Level: %d\n\r", obj->pIndexData->vnum, obj->level);
	ptc(ch, "{CName(s):{x %s{x\n\r", obj->name);
	ptc(ch, "Short description: %s{x\n\rLong  description: %s{x\n\r",
	    obj->short_descr, obj->description);

	if (obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL) {
		EXTRA_DESCR_DATA *ed;
		stc("Extra description keywords: '", ch);

		for (ed = obj->extra_descr; ed != NULL; ed = ed->next) {
			stc(ed->keyword, ch);

			if (ed->next != NULL)
				stc(" ", ch);
		}

		stc("'\n\r", ch);
		stc("Default description keywords: '", ch);

		for (ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next) {
			stc(ed->keyword, ch);

			if (ed->next != NULL)
				stc(" ", ch);
		}
	}

	stc("\n\r", ch);
	ptc(ch, "{PValue0 : %-7d{BCost     : %-13d{GIn room   : %d\n\r",
	    obj->value[0], obj->cost, obj->in_room == NULL ? 0 : obj->in_room->vnum);
	ptc(ch, "{PValue1 : %-7d{BCondition: %-13d{GIn object : %s\n\r",
	    obj->value[1], obj->condition, obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr);
	ptc(ch, "{PValue2 : %-7d{BType     : %-13s{GIn %s : %s\n\r",
	    obj->value[2], item_type_name(obj),
	    obj->in_strongbox ? "StrBox" : "Locker",
	    obj->in_locker    ? PERS(obj->in_locker, ch, VIS_PLR) :
	    obj->in_strongbox ? PERS(obj->in_strongbox, ch, VIS_PLR) : "(none)");
	ptc(ch, "{PValue3 : %-7d{BResets   : %-13d{GCarried by: %s\n\r",
	    obj->value[3], obj->pIndexData->reset_num,
	    obj->carried_by == NULL ? "(none)" : PERS(obj->carried_by, ch, VIS_PLR));
	ptc(ch, "{PValue4 : %-7d{BTimer    : %-13d{GOwned by  : %s\n\r",
	    obj->value[4], obj->timer, get_owner(ch, obj));
	ptc(ch, "\t\t{BNumber   : %d/%-11d{GWear Loc. : %d\n\r",
	    1, get_obj_number(obj), obj->wear_loc);
	ptc(ch, "\t\t{BWeight   : %d/%d/%d (10th pounds){x\n\r",
	    obj->weight, get_obj_weight(obj), get_true_weight(obj));
	ptc(ch, "{W\n\rWear bits : %s\n\rExtra bits: %s{b\n\r\n\r",
	    wear_bit_name(obj->wear_flags), extra_bit_name(obj->extra_flags));

	/* now give out vital statistics as per identify */
	switch (obj->item_type) {
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		ptc(ch, "Level %d spells of:", obj->value[0]);

		if (obj->value[1] >= 0 && obj->value[1] < MAX_SKILL)
			ptc(ch, " '%s'", skill_table[obj->value[1]].name);

		if (obj->value[2] >= 0 && obj->value[2] < MAX_SKILL)
			ptc(ch, " '%s'", skill_table[obj->value[2]].name);

		if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
			ptc(ch, " '%s'", skill_table[obj->value[3]].name);

		if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
			ptc(ch, " '%s'", skill_table[obj->value[4]].name);

		stc(".\n\r", ch);
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		ptc(ch, "Has %d charges of level %d", obj->value[2], obj->value[0]);

		if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
			ptc(ch, " '%s'", skill_table[obj->value[3]].name);

		stc(".\n\r", ch);
		break;

	case ITEM_DRINK_CON:
		ptc(ch, "It holds %s-colored %s.\n\r",
		    liq_table[obj->value[2]].liq_color, liq_table[obj->value[2]].liq_name);
		break;

	case ITEM_CONTAINER:
		ptc(ch, "Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
		    obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));

		if (obj->value[4] != 100)
			ptc(ch, "Weight multiplier: %d%%\n\r", obj->value[4]);

		break;

	case ITEM_WEAPON:
		stc("Weapon type is ", ch);

		switch (obj->value[0]) {
		case (WEAPON_EXOTIC) : stc("exotic.\n\r", ch);       break;

		case (WEAPON_SWORD)  : stc("sword.\n\r", ch);        break;

		case (WEAPON_DAGGER) : stc("dagger.\n\r", ch);       break;

		case (WEAPON_SPEAR)  : stc("spear/staff.\n\r", ch);  break;

		case (WEAPON_MACE)   : stc("mace/club.\n\r", ch);    break;

		case (WEAPON_AXE)    : stc("axe.\n\r", ch);          break;

		case (WEAPON_FLAIL)  : stc("flail.\n\r", ch);        break;

		case (WEAPON_WHIP)   : stc("whip.\n\r", ch);         break;

		case (WEAPON_POLEARM): stc("polearm.\n\r", ch);      break;

		default             : stc("unknown.\n\r", ch);      break;
		}

		ptc(ch, "Damage is %dd%d (average %d).\n\r",
		    obj->value[1], obj->value[2], (1 + obj->value[2]) * obj->value[1] / 2);

		if (obj->value[4])  /* weapon flags */
			ptc(ch, "Weapons flags: %s\n\r", weapon_bit_name(obj->value[4]));

		break;

	case ITEM_ARMOR:
		ptc(ch, "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n\r",
		    obj->value[0], obj->value[1], obj->value[2], obj->value[3]);
		break;

	case ITEM_PBTUBE:
		ptc(ch, "It contains %d paintballs.\n\r", obj->value[0]);
		break;

	case ITEM_PBGUN:
		ptc(ch, "It is holding %d/%d paintballs.  It's rating is %d/%d/%d\n\r",
		    obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4]);
		break;

	case ITEM_MATERIAL:
		ptc(ch, "Skill modifier: %d, Dice Bonus %d, Sides Bonus %d\n\r",
		    obj->value[0], obj->value[1], obj->value[2]);
		break;
	}

	if (!obj->enchanted)
		for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) {
			ptc(ch, "Affects %s by %d, level %d.\n\r",
			    affect_loc_name(paf->location), paf->modifier, paf->level);

			if (paf->bitvector) {
				switch (paf->where) {
				case TO_AFFECTS:
					ptc(ch, "Adds %s affect.\n\r", affect_bit_name(paf->bitvector));
					break;

				case TO_OBJECT:
					ptc(ch, "Adds %s object flag.\n\r", extra_bit_name(paf->bitvector));
					break;

				case TO_DRAIN:
					ptc(ch, "Drains %s.\n\r", imm_bit_name(paf->bitvector));
					break;

				case TO_IMMUNE:
					ptc(ch, "Adds immunity to %s.\n\r", imm_bit_name(paf->bitvector));
					break;

				case TO_RESIST:
					ptc(ch, "Adds resistance to %s.\n\r", imm_bit_name(paf->bitvector));
					break;

				case TO_VULN:
					ptc(ch, "Adds vulnerability to %s.\n\r", imm_bit_name(paf->bitvector));
					break;

				default:
					ptc(ch, "Unknown bit %d: %d\n\r", paf->where, paf->bitvector);
					break;
				}
			}
		}

	for (paf = obj->affected; paf != NULL; paf = paf->next) {
		ptc(ch, "Affects %s by %d, level %d.",
		    affect_loc_name(paf->location), paf->modifier, paf->level);

		if (paf->duration > -1)
			ptc(ch, ", %d hours.\n\r", paf->duration);
		else
			ptc(ch, ".\n\r");

		if (paf->bitvector) {
			switch (paf->where) {
			case TO_AFFECTS:
				ptc(ch, "Adds %s affect.\n", affect_bit_name(paf->bitvector));
				break;

			case TO_OBJECT:
				ptc(ch, "Adds %s object flag.\n", extra_bit_name(paf->bitvector));
				break;

			case TO_WEAPON:
				ptc(ch, "Adds %s weapon flags.\n", weapon_bit_name(paf->bitvector));
				break;

			case TO_DRAIN:
				ptc(ch, "Drains %s.\n\r", imm_bit_name(paf->bitvector));
				break;

			case TO_IMMUNE:
				ptc(ch, "Adds immunity to %s.\n", imm_bit_name(paf->bitvector));
				break;

			case TO_RESIST:
				ptc(ch, "Adds resistance to %s.\n\r", imm_bit_name(paf->bitvector));
				break;

			case TO_VULN:
				ptc(ch, "Adds vulnerability to %s.\n\r", imm_bit_name(paf->bitvector));
				break;

			default:
				ptc(ch, "Unknown bit %d: %d\n\r", paf->where, paf->bitvector);
				break;
			}
		}
	}

	for (i = 1; i < MAX_SPELL; i++)
		if (obj->spell[i] != 0)
			ptc(ch, "Spelled with '%s' at level %d.\n\r",
			    skill_table[obj->spell[i]].name, obj->spell_lev[i]);
}


void format_rstat(CHAR_DATA *ch, ROOM_INDEX_DATA *location)
{
	char buf[MSL];
	AFFECT_DATA *paf = NULL;
	OBJ_DATA *obj;
	CHAR_DATA *rch;
	int door;
	ptc(ch, "{W[%d] {gName: {P%s {W(%s){x\n\r", location->vnum, location->name, location->area->name);
	ptc(ch, "{YSector: %d\tLight: %d\tHealing: %d\tMana: %d{x\n\r",
	    location->sector_type, location->light, location->heal_rate, location->mana_rate);

	if (location->clan)
		ptc(ch, "{VClan: %s{x\n\r", location->clan->who_name);

	if (location->guild)
		ptc(ch, "{VGuild: %s{x\n\r", class_table[location->guild].name);

	ptc(ch, "{BDescription:{x\n\r%s\n\r", location->description);

	if (location->extra_descr != NULL) {
		EXTRA_DESCR_DATA *ed;
		stc("{BExtra description keywords: '{x", ch);

		for (ed = location->extra_descr; ed; ed = ed->next)
			ptc(ch, "%s{x%s", ed->keyword, ed->next == NULL ? "{B.{x\n\r" : " ");
	}

	if (location->room_flags)
		ptc(ch, "{CRoom flags: %s{x\n\r", room_bit_name(location->room_flags));

	if (location->people) {
		stc("{GCharacters:{x ", ch);

		for (rch = location->people; rch; rch = rch->next_in_room)
			if (can_see(ch, rch)) {
				one_argument(rch->name, buf);
				ptc(ch, "%s{x%s", buf, rch->next_in_room ? " " : "\n\r");
			}
	}

	if (location->contents) {
		stc("{bObjects   :{x ", ch);

		for (obj = location->contents; obj; obj = obj->next_content) {
			one_argument(obj->name, buf);
			ptc(ch, "%s{x%s", buf, obj->next_content ? " " : "\n\r");
		}
	}

	for (door = 0; door <= 5; door++) {
		EXIT_DATA *pexit;

		if ((pexit = location->exit[door]) != NULL) {
			ptc(ch, "{WDoor: %d -> %d{c (Key: %d) Exit flags: %d. Keyword: '%s'{x\n\r",
			    door, (pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
			    pexit->key, pexit->exit_info, pexit->keyword);

			if (pexit->description[0] != '\0')
				ptc(ch, "{cDescription: %s{x", pexit->description);
		}
	}

	for (paf = location->affected; paf != NULL; paf = paf->next)
		ptc(ch, "{bAffect: '%s' modifies %s by %d for %d hours with bits %s, level %d, evolve %d.{x\n\r",
		    skill_table[(int) paf->type].name,
		    affect_loc_name(paf->location),
		    paf->modifier,
		    paf->duration + 1,
		    room_bit_name(paf->bitvector),
		    paf->level,
		    paf->evolution);
}


/* main stat function */
void do_stat(CHAR_DATA *ch, char *argument)
{
	ROOM_INDEX_DATA *room;
	OBJ_DATA *obj;
	CHAR_DATA *vch;
	char arg1[MIL], *arg2;

	if (argument[0] == '\0') {
		stc("Syntax:\n\r"
		    "  stat <name or room vnum>\n\r"
		    "  stat char   <name>\n\r"
		    "  stat mob    <name>\n\r"
		    "  stat player <name>\n\r"
		    "  stat obj    <name>\n\r", ch);
		return;
	}

	arg2 = one_argument(argument, arg1);

	if (!str_cmp(arg1, "char")) {
		if (arg2[0] == '\0') {
			stc("Stat what character?\n\r", ch);
			return;
		}

		if ((vch = get_char_here(ch, arg2, VIS_CHAR)) == NULL)
			if ((vch = get_char_world(ch, arg2, VIS_CHAR)) == NULL) {
				stc("No players or mobiles by that name are in the world.\n\r", ch);
				return;
			}

		format_mstat(ch, vch);
	}
	else if (!str_prefix1(arg1, "mobile")) {
		if (arg2[0] == '\0') {
			stc("Stat what mobile?\n\r", ch);
			return;
		}

		if ((vch = get_mob_here(ch, arg2, VIS_CHAR)) == NULL)
			if ((vch = get_mob_world(ch, arg2, VIS_CHAR)) == NULL) {
				stc("No mobiles by that name are in the world.\n\r", ch);
				return;
			}

		format_mstat(ch, vch);
	}
	else if (!str_prefix1(arg1, "player")) {
		if (arg2[0] == '\0') {
			stc("Stat what player?\n\r", ch);
			return;
		}

		if ((vch = get_player_world(ch, arg2, VIS_PLR)) == NULL) {
			stc("No player by that name is in the world.\n\r", ch);
			return;
		}

		format_mstat(ch, vch);
	}
	else if (!str_prefix1(arg1, "object")) {
		if (arg2[0] == '\0') {
			stc("Stat what object?\n\r", ch);
			return;
		}

		if ((obj = get_obj_here(ch, arg2)) == NULL)
			if ((obj = get_obj_world(ch, arg2)) == NULL) {
				stc("No objects by that name are in the world.\n\r", ch);
				return;
			}

		format_ostat(ch, obj);
	}
	else if (!str_cmp(arg1, "room"))        /* 'stat room' (current room) */
		format_rstat(ch, ch->in_room);
	else if (is_number(arg1)) {
		if ((room = get_room_index(atoi(arg1))) == NULL) {
			stc("There is no room with that vnum.\n\r", ch);
			return;
		}

		if (!is_room_owner(ch, room)
		    && ch->in_room != room
		    && room_is_private(room)
		    && !IS_IMP(ch)) {
			stc("That room is private.\n\r", ch);
			return;
		}

		format_rstat(ch, room);
	}
	else {  /* No keyword given, figure out what they want */
		if ((vch = get_player_world(ch, argument, VIS_PLR)))format_mstat(ch, vch);
		else if ((vch = get_mob_here(ch, argument, VIS_CHAR)))  format_mstat(ch, vch);
		else if ((obj = get_obj_here(ch, argument)))            format_ostat(ch, obj);
		else if ((obj = get_obj_world(ch, argument)))           format_ostat(ch, obj);
		else if ((vch = get_mob_world(ch, argument, VIS_CHAR))) format_mstat(ch, vch);
		else
			stc("It does not exist.\n\r", ch);
	}
}


/* new player stat -- Elrac */
void do_pstat(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	int xpl, xnl; /* experience per/next level */

	if ((victim = get_player_world(ch, argument, VIS_PLR)) == NULL) {
		ptc(ch, "No player named '%s' found in the game.\n\r", argument);
		return;
	}

	if (!victim->pcdata) {
		bug("do_pstat: player without pcdata", 0);
		stc("No information available about that player.\n\r", ch);
		return;
	}

	/* PLAYER: Elrac [of Oberrad, Lord of the Code] */
	new_color(ch, CSLOT_OLDSCORE_PKILL);
	stc("PLAYER: ", ch);
	new_color(ch, CSLOT_OLDSCORE_NAME);
	stc(victim->name, ch);

	if (victim->pcdata->title && victim->pcdata->title[0]) {
		set_color(ch, WHITE, NOBOLD);
		stc(" [", ch);
		new_color(ch, CSLOT_OLDSCORE_NAME);
		stc(victim->pcdata->title, ch);
		set_color(ch, WHITE, NOBOLD);
		stc("]", ch);
	}

	stc("\n\r", ch);
	/* Lvl 94/185 (99) superhuman male warrior, leader of clan "CODER" */
	new_color(ch, CSLOT_OLDSCORE_NAME);
	ptc(ch, "Lvl %d", victim->level);

	if (victim->pcdata->remort_count)
		ptc(ch, "/R%d", victim->pcdata->remort_count);

	ptc(ch, " %s %s %s",
	    race_table[victim->race].name,
	    GET_SEX(victim) == SEX_NEUTRAL ? "sexless" : GET_SEX(victim) == SEX_MALE ? "male" : "female",
	    class_table[victim->class].name);

	if (victim->clan) {
		stc(", ", ch);
		set_color(ch, WHITE, NOBOLD);
		ptc(ch, "%s of clan %s",
		    HAS_CGROUP(victim, GROUP_LEADER) ? "Leader" :
		    HAS_CGROUP(victim, GROUP_DEPUTY) ? "Deputy" :
		    "Member",
		    victim->clan->who_name);
	}

	stc("\n\r", ch);

	/* email: carls@ipf.de */
	if (victim->pcdata->email && victim->pcdata->email[0]) {
		set_color(ch, WHITE, NOBOLD);
		ptc(ch, "Email: %s\n\r", victim->pcdata->email);
	}

	/* Str 25(25)    Align -1000   Hp 31418/31418 */
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Str %-2d(%-2d)",
	    victim->perm_stat[STAT_STR], get_curr_stat(victim, STAT_STR));
	new_color(ch, CSLOT_OLDSCORE_ALIGN);
	ptc(ch, "      Align %s%-7d",
	    victim->alignment < 0 ? "{R" : "{Y", victim->alignment);
	new_color(ch, CSLOT_OLDSCORE_POINTS);
	ptc(ch, "      HP   %-5d/%-5d\n\r",
	    victim->hit, victim->max_hit);
	/* Int 25(25)    Train 2       Ma 31573/31573 */
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Int %-2d(%-2d)",
	    victim->perm_stat[STAT_INT], get_curr_stat(victim, STAT_INT));
	new_color(ch, CSLOT_OLDSCORE_GAIN);
	ptc(ch, "      Train %-7d", victim->train);
	new_color(ch, CSLOT_OLDSCORE_POINTS);
	ptc(ch, "      Mana %-5d/%-5d\n\r",
	    victim->mana, victim->max_mana);
	/* Wis 25(25)    Pracs 129     Stm 30603/30603 */
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Wis %-2d(%-2d)",
	    victim->perm_stat[STAT_WIS], get_curr_stat(victim, STAT_WIS));
	new_color(ch, CSLOT_OLDSCORE_GAIN);
	ptc(ch, "      Pracs %-7d",
	    victim->practice);
	new_color(ch, CSLOT_OLDSCORE_POINTS);
	ptc(ch, "      Stm  %-5d/%-5d\n\r",
	    victim->stam, victim->max_stam);
	/* Dex 25(25)                  QP:  12345      */
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Dex %-2d(%-2d)",
	    victim->perm_stat[STAT_DEX], get_curr_stat(victim, STAT_DEX));
	new_color(ch, CSLOT_OLDSCORE_QP);
	ptc(ch, "                         QP:  %-5d\n\r", victim->questpoints);
	/* Con 25(25)   Gold 7481204   SP:  12345 */
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Con %-2d(%-2d)",
	    victim->perm_stat[STAT_CON], get_curr_stat(victim, STAT_CON));
	new_color(ch, CSLOT_OLDSCORE_MONEY);
	ptc(ch, "      Gold  %-7ld",
	    victim->gold);
	new_color(ch, CSLOT_OLDSCORE_SP);
	ptc(ch, "      SP:  %-5d\n\r", victim->pcdata->skillpoints);
	/* Chr 25(25)   Silv 2768604   RPP: 12345 */
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Chr %-2d(%-2d)",
	    victim->perm_stat[STAT_CHR], get_curr_stat(victim, STAT_CHR));
	new_color(ch, CSLOT_OLDSCORE_MONEY);
	ptc(ch, "      Silv  %-7ld", victim->silver);
	new_color(ch, CSLOT_OLDSCORE_RPP);
	ptc(ch, "      RPP: %d\n\r", victim->pcdata->rolepoints);
	/* Xtl 123456      X/lv  1234567      Exp 1234567 */
	xpl = exp_per_level(victim, victim->pcdata->points);
	xnl = (victim->level + 1) * xpl;
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Xtl %-6d      X/lv  %-7d      Exp %d\n\r",
	    xnl - victim->exp, xpl, victim->exp);
	/* (LinkDead) [AFK] (Invis) (Wizi) (Lurk) (Hide) (Charmed) (KILLER) (THIEF) */
	ptc(ch, "%s%s%s%s%s%s%s%s%s%s{x\n\r",
	    IS_SET(victim->pcdata->plr,     PLR_LINK_DEAD) ?        "{G(LinkDead) " : "",
	    IS_SET(victim->comm,            COMM_AFK) ?             "{b[AFK] "      : "",
	    IS_AFFECTED(victim,             AFF_INVISIBLE) ?        "{C(Invis) "    : "",
	    IS_AFFECTED(victim,             AFF_HIDE) ?             "{B(Hide) "     : "",
	    get_affect(victim->affected, gsn_midnight) ?            "{c(Shadowy) "  : "",
	    victim->invis_level ?                                   "{T(Wizi) "     : "",
	    victim->lurk_level ?                                    "{H(Lurk) "     : "",
	    IS_AFFECTED(victim, AFF_CHARM) ?                        "{M(Charmed) "  : "",
	    IS_SET(victim->act, PLR_KILLER) ?                       "{R(KILLER) "   : "",
	    IS_SET(victim->act, PLR_THIEF) ?                        "{B(THIEF) "    : "");
	/* fighting in room 3001, in combat with Super Helga */
	ptc(ch, "{W%s in room [%d]",
	    position_table[victim->position].name,
	    victim->in_room ? victim->in_room->vnum : 0);

	if (victim->fighting)
		ptc(ch, ", {Rin combat{x with %s", victim->fighting->name);

	stc("{a{x\n\r", ch);
	set_color(ch, WHITE, NOBOLD);
}
