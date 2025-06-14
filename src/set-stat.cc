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

#include <vector>

#include "argument.hh"
#include "affect/Affect.hh"
#include "Area.hh"
#include "Character.hh"
#include "Clan.hh"
#include "declare.hh"
#include "Exit.hh"
#include "ExtraDescr.hh"
#include "find.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "lookup.hh"
#include "Logging.hh"
#include "magic.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"
#include "ObjectValue.hh"
#include "Player.hh"
#include "Room.hh"
#include "String.hh"
#include "tables.hh"
#include "typename.hh"
#include "gem/gem.hh"
#include "World.hh"

/* RT set replaces sset, mset, oset, rset and cset */
DECLARE_DO_FUN( do_mset );
DECLARE_DO_FUN( do_rset );
DECLARE_DO_FUN( do_oset );
DECLARE_DO_FUN( do_sset );
DECLARE_DO_FUN( do_evoset );
DECLARE_DO_FUN( do_raffset );
DECLARE_DO_FUN( do_extraset );

void do_set(Character *ch, String argument)
{
	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty()) {
		stc("Syntax:\n", ch);
		stc("  set mob   <name> <field> <value>\n", ch);
		stc("  set obj   <name> <field> <value>\n", ch);
		stc("  set room  <room> <field> <value>\n", ch);

		if (IS_HEAD(ch)) {
			stc("  set plr   <name> <field> <value>\n", ch);
			stc("  set skill <name> <spell or skill> <value>\n", ch);
			stc("  set evol  <name> <spell or skill> <value>\n", ch);
			stc("  set raff  <name> <remort affect>\n", ch);
			stc("  set extra <name> <skill>\n", ch);
		}

		return;
	}

	if (arg.is_prefix_of("mobile") || arg.is_prefix_of("character")) {
		do_mset(ch, argument);
		return;
	}

	if (arg.is_prefix_of("room")) {
		do_rset(ch, argument);
		return;
	}

	if (arg.is_prefix_of("object")) {
		do_oset(ch, argument);
		return;
	}

	/* non-heads of departments can only set mobs, objects, rooms */
	if (!IS_HEAD(ch)) {
		do_set(ch, "");
		return;
	}

	if (arg.is_prefix_of("plr") || arg.is_prefix_of("player")) {
		do_mset(ch, argument);
		return;
	}

	if (arg.is_prefix_of("skill") || arg.is_prefix_of("spell")) {
		do_sset(ch, argument);
		return;
	}

	if (arg.is_prefix_of("evolve") || arg.is_prefix_of("evolution")) {
		do_evoset(ch, argument);
		return;
	}

	if (arg.is_prefix_of("raffect")) {
		do_raffset(ch, argument);
		return;
	}

	if (arg.is_prefix_of("extraclass")) {
		do_extraset(ch, argument);
		return;
	}

	do_set(ch, "");
}

void do_sset(Character *ch, String argument)
{
	char buf[1024];
	Character *victim;
	int value;
	skill::type sn;
	bool fAll;

	String arg1, arg2, arg3;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);

	if (arg1.empty() || arg2.empty() || arg3.empty()) {
		stc("Syntax:\n", ch);
		stc("  set skill <player> <spell or skill> <value>\n", ch);
		stc("  set skill <player> all <value>\n", ch);
		stc("   (use the name of the skill, not the number)\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == nullptr) {
		Format::sprintf(buf, "After searching all over the mud, player %s could not be found.\n",
		        arg1);
		stc(buf, ch);
		return;
	}

	if (victim->is_npc()) {
		stc("I'm sorry, you cannot set a mobile's skills.\n", ch);
		return;
	}

	fAll = arg2 == "all";

	if (!fAll && (sn = skill::lookup(arg2)) == skill::type::unknown) {
		Format::sprintf(buf, "%s is not a valid skill or spell.\n", arg2);
		stc(buf, ch);
		return;
	}

	/*
	 * Snarf the value.
	 */
	if (!arg3.is_number()) {
		Format::sprintf(buf, "The third argument given, '%s', is not numeric.\n", arg3);
		stc(buf, ch);
		return;
	}

	value = atoi(arg3);

	if (value < 0 || value > 100) {
		Format::sprintf(buf, "You entered %s, the valid range is between 0 and 100.\n", arg3);
		stc(buf, ch);
		return;
	}

	if (fAll) {
		for (const auto& pair : skill_table) {
			if (pair.first == skill::type::unknown)
				continue;

			set_learned(victim, pair.first, value);
		}

		Format::sprintf(buf, "All of %s's skills and spells set to %d.\n", victim->name, value);
	}
	else {
		set_learned(victim, sn, value);
		Format::sprintf(buf, "%s's %s %s set to %d.\n",
		        victim->name,
		        skill::lookup(sn).name,
		        skill::lookup(sn).spell_fun != spell_null ? "Spell" : "Skill",
		        value);
	}

	stc(buf, ch);
} /* end do_sset() */

void do_evoset(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Character *victim;
	int value;

	String arg1, arg2, arg3;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);

	if (arg1.empty() || (!arg2.empty() && arg3.empty())) {
		stc("Syntax:\n", ch);
		stc("  set evolve <player>     (lists all evolved on a player)\n", ch);
		stc("  set evolve <player> <spell or skill> <value>\n", ch);
		stc("   (use the name of the skill, not the number)\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == nullptr) {
		ptc(ch, "After searching all over the mud, player %s could not be found.\n", arg1);
		return;
	}

	if (victim->is_npc()) {
		stc("I'm sorry, you cannot set a mobile's skill evolution.\n", ch);
		return;
	}

	if (victim->guild == Guild::none) {
		stc("A player must join a guild before evolving skills.\n", ch);
		return;
	}

	if (arg2.empty()) {
		extern int can_evolve args((Character * ch, skill::type sn));
		String buffer;
		int can;
		buffer += "They have the following skills and spells evolved:\n\n";

		for (const auto& pair : skill_table) {
			skill::type type = pair.first;
			const auto& entry = pair.second;

			if (type == skill::type::unknown)
				continue;

			if ((can = can_evolve(victim, type)) == -1)
				continue;

			Format::sprintf(buf, "They have %s at %d%%, evolution %d.\n",
			        entry.name,
			        get_learned(victim, type),
			        get_evolution(victim, type));
			buffer += buf;

			if (can == 1)
				Format::sprintf(buf, "They need %d skill points to evolve %s to %d.\n",
				        get_evolution(victim, type) == 1 ?
				        entry.evocost_sec[victim->guild] :
				        entry.evocost_pri[victim->guild],
				        entry.name,
				        get_evolution(victim, type) + 1);

			buffer += buf;
			buffer += "\n";
		}

		page_to_char(buffer, ch);
		return;
	}

	skill::type type = skill::lookup(arg2);

	if (type == skill::type::unknown) {
		ptc(ch, "%s is not a valid skill or spell.\n", arg2);
		return;
	}

	if (!arg3.is_number()) {
		ptc(ch, "The third argument given, '%s', is not numeric.\n", arg3);
		return;
	}

	const auto& entry = skill::lookup(type);

	if (entry.evocost_sec[victim->guild] <= 0 && !IS_IMMORTAL(victim)) {
		ptc(ch, "%ss cannot evolve %s.\n",
		    guild_table[victim->guild].name.capitalize(), entry.name);
		return;
	}

	value = atoi(arg3);

	if (value < 1 || value > 4) {
		ptc(ch, "You entered %s, the valid range is between 1 and 4.\n", arg3);
		return;
	}

	if (entry.evocost_pri[victim->guild] <= 0 && value > 2 && !IS_IMMORTAL(victim)) {
		stc("Secondary classes cannot evolve a skill or spell past 2.\n", ch);
		return;
	}

	set_evolution(victim, type, value);
	ptc(ch, "%s's %s %s has been set to evolution %d.\n", victim->name, entry.name,
	    entry.spell_fun != spell_null ? "spell" : "skill", value);
} /* end do_evoset() */

void do_raffset(Character *ch, String argument)
{
	extern void fix_blank_raff(Character * ch, int start);
	extern void roll_one_raff(Character * ch, Character * victim, int place);
	extern void roll_raffects(Character * ch, Character * victim);
	extern void rem_raff_affect(Character * ch, int index);
	Character *victim;
	int i, index;

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1.empty()) {
		stc("Syntax:\n", ch);
		stc("  set raffect                    (displays syntax)\n", ch);
		stc("  set raffect list               (displays a list of remort affects)\n", ch);
		stc("  set raffect <player>           (displays list of set remort affects)\n", ch);
		stc("  set raffect <player> none      (clears all remort affects)\n", ch);
		stc("  set raffect <player> rollone   (rolls one random remort affect)\n", ch);
		stc("  set raffect <player> reroll    (rerolls all remort affects)\n", ch);
		stc("  set raffect <player> <remort affect number>\n", ch);
		stc("      (toggles remort affect on or off, use the raffect number)\n", ch);
		return;
	}

	if (arg1 == "list") {
		for (i = 1; i < MAX_RAFFECTS; i++)
			ptc(ch, "{W[{C%3d{W] %10s {x%s\n",
			    raffects[i].id, raffects[i].shortname, raffects[i].description);

		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == nullptr) {
		stc("They are not here.\n", ch);
		return;
	}

	if (victim->is_npc()) {
		stc("Mobiles do not have remort affects.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		stc("Immortals do not need remort affects.\n", ch);
		return;
	}

	if (!IS_REMORT(victim)) {
		stc("Only remorts can have remort affects.\n", ch);
		return;
	}

	/* before we even start, let's fix holes in their raffects */
	fix_blank_raff(victim, 0);

	if (arg2.empty()) {
		if (victim->pcdata->raffect[0] == 0) {
			stc("That player does not have any remort affects.\n", ch);
			return;
		}

		stc("They have been blessed(?) with the following remort affects:\n\n", ch);

		for (i = 0; i < ((victim->pcdata->remort_count / 10) + 1); i++) {
			int rindex;
			rindex = raff_lookup(victim->pcdata->raffect[i]);

			if (!rindex)
				stc("{W[{C000{W] {xNone\n", ch);
			else
				ptc(ch, "{W[{C%3d{W] {x%s\n", raffects[rindex].id, raffects[rindex].description);
		}

		return;
	}

	if (arg2 == "none" || arg2 == "reroll") {
		/* loop through and set all remort affects to 0 */
		for (i = 0; i < 10; i++) {
			rem_raff_affect(victim, raff_lookup(victim->pcdata->raffect[i]));
			victim->pcdata->raffect[i] = 0;
		}

		if (arg2 == "none") {
			stc("The player's remort affects have been cleared.\n", ch);
			return;
		}

		stc("You reroll their remort affects...\n", ch);
		stc("An immortal rerolls your remort affects...\n", victim);
		roll_raffects(ch, victim);
		return;
	}

	if (arg2 == "rollone") {
		/* find the first blank spot, and add a random raffect */
		for (i = 0; i < ((victim->pcdata->remort_count / 10) + 1); i++) {
			if (victim->pcdata->raffect[i] < 1) {
				stc("You add a random remort affect...\n", ch);
				stc("An immortal adds a random remort affect to you...\n", victim);
				roll_one_raff(ch, victim, i);
				return;
			}
		}

		stc("That player has the maximum number of remort affects.\n", ch);
		return;
	}

	if (!arg2.is_number()) {
		stc("Please use the number id for remort affects.  Type 'set raffect list' for a list.\n", ch);
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
				stc("Remort affect removed.\n", ch);
				return;
			}
		}

		stc("Couldn't find that remort affect on the player.\n", ch);
		Logging::bug("Couldn't find raffect on player.", 0);
		return;
	}

	/* now check if it's valid before trying to add it */
	if (!index) {
		stc("That is not a valid remort affect.  Type 'set raffect list' for a list.\n", ch);
		return;
	}

	/* they don't have it, loop through, find the first empty space, and add it */
	for (i = 0; i < ((victim->pcdata->remort_count / 10) + 1); i++) {
		if (victim->pcdata->raffect[i] < 1) {
			if (raffects[index].group > 0) {
				if (HAS_RAFF_GROUP(victim, raffects[index].group)) {
					stc("They already have the opposite of that remort affect.\n", ch);
					return;
				}
			}

			victim->pcdata->raffect[i] = atoi(arg2);
			stc("Remort affect added.\n", ch);
			return;
		}
	}

	/* all raffect slots for them are full */
	stc("That player has the maximum amount of remort affects.\n", ch);
	return;
}

void do_extraset(Character *ch, String argument)
{
	extern void fix_blank_extraclass(Character * ch, int index);
	char buf[MAX_STRING_LENGTH];
	String output;
	Character *victim;
	skill::type sn;
	int x, i, cn, col = 0;

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1.empty()) {
		stc("Syntax:\n", ch);
		stc("  set extraclass                    (displays syntax)\n", ch);
		stc("  set extraclass list               (displays a list of extraclass skills)\n", ch);
		stc("  set extraclass <player>           (displays list of set extraclass skills)\n", ch);
		stc("  set extraclass <player> none      (clears all extraclass skills)\n", ch);
		stc("  set extraclass <player> <skill>\n", ch);
		stc("      (toggles one extraclass skill as gained or not)\n", ch);
		return;
	}


	if (arg1 == "list") {
		output += "\n                      {BExtraclass Remort Skills{x\n";

		for (cn = Guild::first; cn < Guild::size; cn++) {
			Format::sprintf(buf, "\n{W%s Skills{x\n    ", guild_table[cn].name.capitalize());
			output += buf;
			col = 0;

			for (const auto& pair : skill_table) {
				skill::type type = pair.first;
				const auto& entry = pair.second;

				if (type == skill::type::unknown)
					continue;

				if (entry.remort_guild != Guild::none
				 && entry.remort_guild == cn) {
					Format::sprintf(buf, "%-15s %-8d", entry.name, entry.rating[ch->guild]);
					output += buf;
				}
			}

			output += "\n";
		}

		if (col % 3 != 0)
			output += "\n\n";

		page_to_char(output, ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == nullptr) {
		stc("They are not here.\n", ch);
		return;
	}

	if (victim->is_npc()) {
		stc("Mobiles do not have extraclass skills.\n", ch);
		return;
	}

	if (victim->guild == Guild::none) {
		stc("Only players with guilds can have extraclass skills.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		stc("Immortals do not need extraclass skills.\n", ch);
		return;
	}

	if (!IS_REMORT(victim)) {
		stc("Only remorts can have extraclass skills.\n", ch);
		return;
	}

	/* before we even start, let's fix holes in their exsk array */
	/*    fix_blank_extraclass(victim,0); */

	if (arg2.empty()) {
		if (victim->pcdata->extraclass[0] != skill::type::unknown
		 || victim->pcdata->extraclass[1] != skill::type::unknown
		 || victim->pcdata->extraclass[2] != skill::type::unknown
		 || victim->pcdata->extraclass[3] != skill::type::unknown
		 || victim->pcdata->extraclass[4] != skill::type::unknown) {
	  		Format::sprintf(buf, "Their current extraclass skill%s",
	  			victim->pcdata->extraclass[1] != skill::type::unknown ? "s are" : " is");
			output += buf;

			if (victim->pcdata->extraclass[0] != skill::type::unknown) {
				Format::sprintf(buf, " %s", skill::lookup(victim->pcdata->extraclass[0]).name);
				output += buf;
			}

			for (x = 1; x < victim->pcdata->remort_count / EXTRACLASS_SLOT_LEVELS + 1; x++) {
				if (victim->pcdata->extraclass[x] != skill::type::unknown) {
					Format::sprintf(buf, ", %s", skill::lookup(victim->pcdata->extraclass[x]).name);
					output += buf;
				}
			}

			output += ".\n";
			page_to_char(output, ch);
			return;
		}

		stc("That player does not have any extraclass skills.\n", ch);
		return;
	}

	if (arg2 == "none") {
		/* loop through and set all exsks to 0 */
		for (i = 0; i < 5; i++)
			victim->pcdata->extraclass[i] = skill::type::unknown;

		stc("The player's extraclass skills have been cleared.\n", ch);
		return;
	}

	/* Ok, now we check to see if the skill is a remort skill */
	if ((sn = skill::lookup(arg2)) == skill::type::unknown) {
		stc("That is not even a valid skill, much less a remort skill.\n", ch);
		return;
	}

	/* if they have it, loop through and set it to 0 */
	if (HAS_EXTRACLASS(victim, sn)) {
		for (x = 0; x < victim->pcdata->remort_count / EXTRACLASS_SLOT_LEVELS + 1; x++) {
			if (victim->pcdata->extraclass[x] == sn) {
				victim->pcdata->extraclass[x] = skill::type::unknown;
				stc("Extraclass skill removed.\n", ch);
				/*              fix_blank_extraclass(victim,0); */
			}
		}

		return;
	}

	/* for debugging, put the checks for oddball stuff after the removal part */
	/* Is it a remort skill? */
	if (skill::lookup(sn).remort_guild == Guild::none) {
		stc("That is not a remort skill.\n", ch);
		return;
	}

	/* Is it outside of the player's class? */
	if (skill::lookup(sn).remort_guild == victim->guild) {
		stc("They cannot have an extraclass skill within their class.  Pick another.\n", ch);
		return;
	}

	/* is it barred from that class? */
	if ((get_usable_level(sn, victim->guild) < 0)
	    || (skill::lookup(sn).rating[victim->guild] < 0)) {
		stc("Their class cannot gain that skill.\n", ch);
		return;
	}

	/* they don't have it, let's put it in the first blank spot */
	for (x = 0; x < victim->pcdata->remort_count / EXTRACLASS_SLOT_LEVELS + 1; x++) {
		if (victim->pcdata->extraclass[x] == skill::type::unknown) {
			victim->pcdata->extraclass[x] = sn;

			if (get_learned(victim, sn) == 0)
				set_learned(victim, sn, 1);

			stc("Extraclass skill added.\n", ch);
			ptc(victim, "You have been given %s as an extraclass remort skill.\n", skill::lookup(sn).name);
			return;
		}
	}

	/* can't find an empty spot, must have the max number of extraclass skills */
	stc("They have the maximum amount of extraclass skills for their remort level.\n", ch);
	return;
}

void do_mset(Character *ch, String argument)
{
	Character *victim;
	int value;

	String arg1, arg2, arg3, buf;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	arg3 = argument;

	if (arg1.empty() || arg2.empty() || arg3.empty()) {
		stc("Syntax:\n", ch);
		stc("  set char <name> <field> <value>\n", ch);
		stc("  Field being one of:\n",                      ch);
		stc("    str int wis dex con chr sex level race nectimer\n"
		    "    gold silver hp mana stamina align hunt\n"
		    "    damdice damsides damtype\n", ch);

		if (IS_HEAD(ch))
			stc("    class prac train thirst hunger drunk full\n"
			    "    pckills pckilled akills akilled killer thief\n"
			    "    quest squest qp sp rpp remort_count experience\n"
			    "    pkrank\n", ch);

		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == nullptr) {
		ptc(ch, "After searching all over the mud, %s could not be found.\n", arg1);
		return;
	}

	if (!victim->is_npc() && !IS_IMP(ch) && victim != ch) {
		stc("You cannot set players at your level.\n", ch);
		return;
	}

	/* Snarf the value (which need not be numeric). */
	value = arg3.is_number() ? atoi(arg3) : -1;

	/* Set something. */

	if (arg2 == "str") {
		if (value < 3 || value > get_max_train(victim, STAT_STR)) {
			ptc(ch, "Strength range is 3 to %d.\n", get_max_train(victim, STAT_STR));
			return;
		}

		ATTR_BASE(victim, APPLY_STR) = value;
		ptc(ch, "%s's strength set to %d.\n", victim->name, value);
		return;
	}

	if (arg2 == "int") {
		if (value < 3 || value > get_max_train(victim, STAT_INT)) {
			ptc(ch, "Intelligence range is 3 to %d.\n", get_max_train(victim, STAT_INT));
			return;
		}

		ATTR_BASE(victim, APPLY_INT) = value;
		ptc(ch, "%s's intelligence set to %d.\n", victim->name, value);
		return;
	}

	if (arg2 == "wis") {
		if (value < 3 || value > get_max_train(victim, STAT_WIS)) {
			ptc(ch, "Wisdom range is 3 to %d.\n", get_max_train(victim, STAT_WIS));
			return;
		}

		ATTR_BASE(victim, APPLY_WIS) = value;
		ptc(ch, "%s's wisdom set to %d.\n", victim->name, value);
		return;
	}

	if (arg2 == "dex") {
		if (value < 3 || value > get_max_train(victim, STAT_DEX)) {
			ptc(ch, "Dexterity range is 3 to %d.\n", get_max_train(victim, STAT_DEX));
			return;
		}

		ATTR_BASE(victim, APPLY_DEX) = value;
		ptc(ch, "%s's dexterity set to %d.\n", victim->name, value);
		return;
	}

	if (arg2 == "con") {
		if (value < 3 || value > get_max_train(victim, STAT_CON)) {
			ptc(ch, "Constitution range is 3 to %d.\n", get_max_train(victim, STAT_CON));
			return;
		}

		ATTR_BASE(victim, APPLY_CON) = value;
		ptc(ch, "%s's constitution set to %d.\n", victim->name, value);
		return;
	}

	if (arg2 == "chr") {
		if (value < 3 || value > get_max_train(victim, STAT_CHR)) {
			ptc(ch, "Charisma range is 3 to %d.\n", get_max_train(victim, STAT_CHR));
			return;
		}

		ATTR_BASE(victim, APPLY_CHR) = value;
		ptc(ch, "%s's charisma set to %d.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("sex")) {
		switch (value) {
		case 0: Format::sprintf(buf, "%s", "an it");            break;

		case 1: Format::sprintf(buf, "%s", "a male");           break;

		case 2: Format::sprintf(buf, "%s", "a female");         break;

		default:
			stc("Sex range is 0 to 2.\n", ch);
			return;
		}

		ATTR_BASE(victim, APPLY_SEX) = value;
		ptc(ch, "%s is now %s.\n", victim->name, buf);
		return;
	}

	if (arg2.is_prefix_of("level")) {
		if (!victim->is_npc()) {
			stc("You may not set a player's level.\n"
			    "Use ADVANCE instead!\n", ch);
			return;
		}

		if (value < 1 || value > 1000) {
			stc("A valid level must be between 1 and 1000.\n", ch);
			return;
		}

		victim->level = value;
		ptc(ch, "%s is now level %d.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("gold")) {
		victim->gold = value;
		ptc(ch, "%s now has %d gold.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("silver")) {
		victim->silver = value;
		ptc(ch, "%s now has %d silver.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("hp")) {
		if (value < -10 || value > 30000) {
			stc("A valid hp value must be between -10 and 30,000 hit points.\n", ch);
			return;
		}

		ATTR_BASE(victim, APPLY_HIT) = value;

		ptc(ch, "%s now has %d hit points.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("mana")) {
		if (value < -10 || value > 30000) {
			stc("A valid mana value must be between -10 and 30,000 mana points.\n", ch);
			return;
		}

		ATTR_BASE(victim, APPLY_MANA) = value;

		ptc(ch, "%s now has %d mana.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("stamina")) {
		if (value < -10 || value > 30000) {
			stc("A valid stamina value must be between -10 and 30,000 stamina points.\n", ch);
			return;
		}

		ATTR_BASE(victim, APPLY_STAM) = value;

		ptc(ch, "%s now has %d stamina points.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("align")) {
		if (value < -1000 || value > 1000) {
			stc("A valid alignment must be between -1000 and 1000.\n", ch);
			return;
		}

		victim->alignment = value;
		ptc(ch, "%s's alignment is now %d.\n", victim->name, value);
		return;
	}

	if (arg2 == "hunt") {
		Character *hunted = 0;

		if (!victim->is_npc()) {
			stc("You can't make a player hunt!\n", ch);
			return;
		}

		if (arg3 == ".") {
			ptc(ch, "%s is now hunting no one.\n", victim->name);
			return;
		}

		if ((hunted = get_char_area(victim, arg3, VIS_CHAR)) == nullptr) {
			ptc(ch, "%s could not find %s.\n", victim->name, arg3);
			return;
		}

		victim->hunting = hunted;
		ptc(ch, "%s is now hunting %s!\n", victim->name, hunted->name);
		return;
	}

	if (arg2.is_prefix_of("race")) {
		unsigned int race = race_lookup(arg3);

		if (race == 0 || (!victim->is_npc() && !race_table[race].pc_race)) {
			ptc(ch, "%s is not a valid %s race.\n", arg3, !victim->is_npc() ? "PC" : "NPC");
			stc("Valid races are :\n\n", ch);

			for (const auto& entry : race_table) {
				if ((!victim->is_npc() && !entry.pc_race)
				    || (victim->is_npc() && entry.pc_race))
					continue;

				ptc(ch, "%-10.10s\n", entry.name);
			}

			stc("\n", ch);
			return;
		}

		victim->race = race;
		ptc(ch, "%s is now a member of the %s race.\n", victim->name, race_table[race].name);
		return;
	}

	if (arg2.is_prefix_of("nectimer")) {
		if (!victim->is_npc()) {
			stc("You can't set a player's necromancer timer!\n", ch);
			return;
		}

		if (value < -1 || value > 100) {
			stc("A valid NecTimer value must be between  -1 and 100.\n", ch);
			return;
		}

		victim->nectimer = value;
		ptc(ch, "%s's necromancer timer is now set to %d.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("damdice")) {
		if (!victim->is_npc()) {
			stc("You cannot set a player's damage dice.\n", ch);
			return;
		}

		if (value < 1 || value > 30000) {
			stc("The value must be between 1 and 30000.\n", ch);
			return;
		}

		victim->damage[DICE_NUMBER] = value;
		ptc(ch, "%s now rolls %d dice for damage.\n", victim->short_descr, value);
		return;
	}

	if (arg2.is_prefix_of("damsides")) {
		if (!victim->is_npc()) {
			stc("You cannot set a player's damage dice type.\n", ch);
			return;
		}

		if (value < 1 || value > 30000) {
			stc("The value must be between 1 and 30000.\n", ch);
			return;
		}

		victim->damage[DICE_TYPE] = value;
		ptc(ch, "%s now rolls %d sided dice for damage.\n", victim->short_descr, value);
		return;
	}

	if (arg2.is_prefix_of("damtype")) {
		unsigned int i;

		if (!victim->is_npc()) {
			stc("You cannot set a player's damage type.\n", ch);
			return;
		}

		for (i = 0; i < attack_table.size(); i++)
			if (arg3.is_prefix_of(attack_table[i].name))
				break;

		if (i >= attack_table.size()) {
			stc("That is not a valid damage type.\n"
			    "Use {Vtypelist attack{x to see valid types.\n", ch);
			return;
		}

		victim->dam_type = i;
		ptc(ch, "%s now hits with %s.\n", victim->short_descr, attack_table[i].noun);
		return;
	}

	if (victim->is_npc()) {
		stc("You can't set that on mobiles.\n", ch);
		do_mset(ch, "");
		return;
	}

	/*********************************************************
	            Head of department or self below here
	 *********************************************************/

	if (arg2.is_prefix_of("class")) {
		Guild guild = guild_lookup(arg3);

		if (guild == Guild::none) {
			Format::sprintf(buf, "Possible classes are: ");

			for (int i = Guild::first; i < Guild::size; i++) {
				if (i > Guild::first)
					buf += " ";

				buf += guild_table[i].name;
			}

			buf += ".\n";
			stc(buf, ch);
			return;
		}

		victim->guild = guild;
		ptc(ch, "%s is now a %s.\n", victim->name, guild_table[guild].name);
		return;
	}

	if (arg2.is_prefix_of("remort_count")) {
		if (value < 0 || value > 99) {
			stc("A valid remort count must be between 0 and 99.\n", ch);
			return;
		}

		victim->pcdata->remort_count = value;
		ptc(ch, "%s's remort count has been set to %d.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("hours")) {
		if (value < 0 || value > 10000) {
			stc("Hours must be between 0 and 10000.\n", ch);
			return;
		}

		victim->pcdata->played = value * 3600;
		ptc(ch, "%s has now played for %d hours. Shhh, don't tell anyone!\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("experience")) {
		victim->exp = value;
		ptc(ch, "%s now has %d experience points.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("quest")) {
		if (value < 0 || value > 30) {
			stc("A valid Next Quest range is 0 to 10.\n", ch);
			return;
		}

		victim->pcdata->nextquest = value;
		ptc(ch, "%s can now quest again in %d minutes.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("squest")) {
		if (value < 0 || value > 30) {
			stc("A valid next skill quest range is 0 to 30.\n", ch);
			return;
		}

		victim->pcdata->nextsquest = value;
		ptc(ch, "%s can now skill quest again in %d minutes.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("qp")) {
		if (value < 0 || value > 5000) {
			stc("A valid quest point value must be between 0 and 5000.\n", ch);
			return;
		}

		victim->pcdata->questpoints = value;
		ptc(ch, "%s now has %d quest points.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("sp")) {
		if (value < 0 || value > 5000) {
			stc("A valid skill point value must be between 0 and 5000.\n", ch);
			return;
		}

		victim->pcdata->skillpoints = value;
		ptc(ch, "%s now has %d skill points.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("rpp")) {
		if (value < 0 || value > 5000) {
			stc("A valid roleplay point value must be between 0 and 5000.\n", ch);
			return;
		}

		victim->pcdata->rolepoints = value;
		ptc(ch, "%s now has %d roleplay points.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("practice")) {
		if (value < 0 || value > 15000) {
			stc("A valid practice value must be between 0 and 15000 sessions.\n", ch);
			return;
		}

		victim->practice = value;
		ptc(ch, "%s now has %d practice sessions.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("train")) {
		if (value < 0 || value > 15000) {
			stc("A valid train value must be between 0 and 15000 sessions.\n", ch);
			return;
		}

		victim->train = value;
		ptc(ch, "%s now has %d training sessions.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("thirst")) {
		if (value < -1 || value > 100) {
			stc("A valid thirst value must be between -1 and 100.\n", ch);
			return;
		}

		victim->pcdata->condition[COND_THIRST] = value;
		ptc(ch, "%s now has a thirst of %d.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("drunk")) {
		if (value < -1 || value > 100) {
			stc("A valid drunk value must be between -1 and 100.\n", ch);
			return;
		}

		victim->pcdata->condition[COND_DRUNK] = value;
		ptc(ch, "%s now has a drunk value of %d.\n", victim->name, value);

		if (value > 70)
			ptc(ch, "%s is {PW{YA{GS{BT{CE{VD!{x\n", victim->name);

		return;
	}

	if (arg2.is_prefix_of("full")) {
		if (value < -1 || value > 100) {
			stc("A valid full value must be between -1 and 100.\n", ch);
			return;
		}

		victim->pcdata->condition[COND_FULL] = value;
		ptc(ch, "%s now has a full value of %d.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("hunger")) {
		if (value < -1 || value > 100) {
			stc("A valid hunger value must be between -1 and 100.\n", ch);
			return;
		}

		victim->pcdata->condition[COND_HUNGER] = value;
		ptc(ch, "%s now has a hunger value of %d.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("pckills")) {
		if (value < 0 || value > 1000) {
			stc("A valid player kills value must be between 0 and 1000.\n", ch);
			return;
		}

		victim->pcdata->pckills = value;
		ptc(ch, "%s has now killed %d players.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("pckilled")) {
		if (value < 0 || value > 1000) {
			stc("A valid PCKilled value must be between 0 and 1000.\n", ch);
			return;
		}

		victim->pcdata->pckilled = value;
		ptc(ch, "%s's has now been killed by %d players.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("akills")) {
		if (value < 0 || value > 1000) {
			stc("A valid arena kills value must be between 0 and 1000.\n", ch);
			return;
		}

		victim->pcdata->arenakills = value;
		ptc(ch, "%s has now killed %d players in the arena.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("akilled")) {
		if (value < 0 || value > 1000) {
			stc("A valid arena killed value must be between 0 and 1000.\n", ch);
			return;
		}

		victim->pcdata->arenakilled = value;
		ptc(ch, "%s's has now been killed by %d players in the arena.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("pkrank")) {
		if (value < 0 || value > 5) {
			stc("PK Ranks must be between 0 and 5.\n", ch);
			return;
		}

		victim->pcdata->pkrank = value;
		ptc(ch, "%s now has a PK rank of %d.\n", victim->name, value);
		return;
	}

	if (arg2.is_prefix_of("thief")) {
		if (value < 0 || value > MAX_THIEF) {
			ptc(ch, "A valid {BTHIEF{x time is between 0 and %d ticks.\n", MAX_THIEF);
			return;
		}

		victim->pcdata->flag_thief = value;
		victim->act_flags += PLR_NOPK;

		if (value) {
			victim->act_flags += PLR_THIEF;
			ptc(ch, "%s will have a {BTHIEF{x flag for %d ticks.\n", victim->name, value);
		}
		else {
			victim->act_flags -= PLR_THIEF;
			ptc(ch, "%s is no longer a {BTHIEF{x.\n", victim->name);
		}

		return;
	}

	if (arg2.is_prefix_of("killer")) {
		if (value < 0 || value > MAX_KILLER) {
			ptc(ch, "A valid {RKILLER{x time is between 0 and %d ticks.\n", MAX_KILLER);
			return;
		}

		victim->pcdata->flag_killer = value;
		victim->act_flags += PLR_NOPK;

		if (value) {
			victim->act_flags += PLR_KILLER;
			ptc(ch, "%s will have a {RKILLER{x flag for %d ticks.\n", victim->name, value);
		}
		else {
			victim->act_flags -= PLR_KILLER;
			ptc(ch, "%s is no longer a {RKILLER{x.\n", victim->name);
		}

		return;
	}

	/* Generate usage message. */
	do_mset(ch, "");
}

void do_oset(Character *ch, String argument)
{
	char buf[1024];
	Object *obj;
	int value;

	String arg1, arg2, arg3;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	arg3 = argument;

	if (arg1.empty() || arg2.empty() || arg3.empty()) {
		stc("Syntax:\n", ch);
		stc("  set obj <object> <field> <value>\n", ch);
		stc("  Field being one of:\n",                               ch);
		stc("    value0 value1 value2 value3 value4 (v1-v4)\n",      ch);
		stc("    condition level weight cost timer\n",          ch);
		return;
	}

	if ((obj = get_obj_world(ch, arg1)) == nullptr) {
		Format::sprintf(buf, "After searching all over the mud, %s could not be found.\n", arg1);
		stc(buf, ch);
		return;
	}

	/*
	 * Snarf the value (which need not be numeric).
	 */
	/* actually, this does need to be numeric?  atoi just returns 0 for non-numeric strings.
	   it would be interesting if set was smart enough to interpret strings like loading
	   from area files, but maybe someday. -- Montrey */
	value = atoi(arg3);

	/*
	 * Set something.
	 */
	if (arg2 == "value0" || arg2 == "v0") {
		obj->value[0] = value;
		ptc(ch, "%s's v0 has been set to %d.\n", obj->short_descr, value);
		return;
	}

	if (arg2 == "value1" || arg2 == "v1") {
		obj->value[1] = value;
		ptc(ch, "%s's v1 has been set to %d.\n", obj->short_descr, value);
		return;
	}

	if (arg2 == "value2" || arg2 == "v2") {
		/* Hack to keep Crush from crashing the mud */
		if ((obj->item_type == ITEM_FOUNTAIN
		     || obj->item_type == ITEM_DRINK_CON)
		    && (unsigned int)value >= liq_table.size()) {
			ptc(ch, "The max for drinks and fountains is %d.\n", liq_table.size()-1);
			return;
		}

		obj->value[2] = value;
		ptc(ch, "%s's v2 has been set to %d.\n", obj->short_descr, value);
		return;
	}

	if (arg2 == "value3" || arg2 == "v3") {
		/* Hack to keep Crush from crashing the mud */
		if (obj->item_type == ITEM_WEAPON && (unsigned int)value >= attack_table.size()) {
			ptc(ch, "The max for weapons is %d.\n", attack_table.size()-1);
			return;
		}

		if (obj->item_type == ITEM_PORTAL) {
			const Room *room = Game::world().get_room(Location(arg3));

			if (room == nullptr) {
				ptc(ch, "There is no such location.\n");
				return;
			}

			obj->value[3] = room->location.to_int();
			ptc(ch, "%s's v3 has been set to %s.\n", obj->short_descr, room->location);
			return;
		}

		obj->value[3] = value;
		ptc(ch, "%s's v3 has been set to %d.\n", obj->short_descr, value);
		return;
	}

	if (arg2 == "value4" || arg2 == "v4") {
		if (obj->item_type == ITEM_WEAPON) {
			stc("Please use the flag command to add or remove weapon flags.\n", ch);
			return;
		}

		obj->value[4] = value;
		ptc(ch, "%s's v4 has been set to %d.\n", obj->short_descr, value);
		return;
	}

	if (arg2.is_prefix_of("condition")) {
		if (value < -1 || value > 100) {
			stc("A valid condition value must be between -1 and 100.\n", ch);
			return;
		}

		obj->condition = value;
		Format::sprintf(buf, "%s's condition value has been changed to %d.\n", obj->short_descr, value);
		stc(buf, ch);
		return;
	}

	if (arg2.is_prefix_of("level")) {
		obj->level = value;
		Format::sprintf(buf, "%s's level has been changed to %d.\n", obj->short_descr, value);
		stc(buf, ch);
		return;
	}

	if (arg2.is_prefix_of("weight")) {
		obj->weight = value;
		Format::sprintf(buf, "%s's weight has been changed to %d.\n", obj->short_descr, value);
		stc(buf, ch);
		return;
	}

	if (arg2.is_prefix_of("cost")) {
		obj->cost = value;
		Format::sprintf(buf, "%s's cost has been changed to %d.\n", obj->short_descr, value);
		stc(buf, ch);
		return;
	}

	if (arg2.is_prefix_of("timer")) {
		obj->timer = value;
		Format::sprintf(buf, "%s's timer value has been changed to %d.\n", obj->short_descr, value);
		stc(buf, ch);
		return;
	}

	if (arg2.is_prefix_of("settings")) {
		if (value < 0 || value > MAX_GEM_SETTINGS) {
			ptc(ch, "A valid settings vaue is between 0 and %d.\n", MAX_GEM_SETTINGS);
			return;
		}

		obj->num_settings = value;
		Format::sprintf(buf, "%s's number of settings has been changed to %d.\n", obj->short_descr, value);
		stc(buf, ch);
		return;
	}

	/*
	 * Generate usage message.
	 */
	do_oset(ch, "");
} /* end do_oset() */

void do_rset(Character *ch, String argument)
{
	char buf [MAX_STRING_LENGTH];
	Room *location;
//	int value;

	String arg1, arg2, arg3;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	arg3 = argument;

	if (arg1.empty() || arg2.empty()) {
		stc("Syntax:\n", ch);
		stc("  set room <location> <field> <value>\n", ch);
		stc("  Field being one of:\n",                ch);
		stc("    sector\n",                           ch);
		return;
	}

	if ((location = find_location(ch, arg1)) == nullptr) {
		Format::sprintf(buf, "No such location, %s.\n", arg1);
		stc(buf, ch);
		return;
	}

	if (!is_room_owner(ch, location) && ch->in_room != location
	    &&  location->is_private() && !IS_IMP(ch)) {
		Format::sprintf(buf, "I'm sorry, but %s is a private room.\n", location->name());
		stc(buf, ch);
		return;
	}

	if (arg3.empty()) {
		// this needs to reworked into a dynamic table to test values below
		if (arg2.is_prefix_of("sector")) {
			stc("Current sector types are:\n"
			    "  0  inside\n"
			    "  1  city\n"
			    "  2  field\n"
			    "  3  forest\n"
			    "  4  hills\n"
			    "  5  mountain\n"
			    "  6  water_swim\n"
			    "  7  water_noswim\n"
			    "  8  (unused, don't pick)\n"
			    "  9  air\n"
			    " 10  desert\n"
			    " 20  arena\n"
			    " 21  clanarena\n", ch);
			return;
		}

		do_rset(ch, "");
		return;
	}

	/*
	 * Snarf the value.
	 */
	if (!arg3.is_number()) {
		Format::sprintf(buf, "The value entered, %s, is not numeric.\n", arg3);
		stc(buf, ch);
		return;
	}

//	value = atoi(arg3);

	/*
	 * Set something.
	 */

	if (arg2.is_prefix_of("sector")) {
		stc("Sorry, setting sector value is currently disabled.\n", ch);
//		location->sector_type()   = value;
//		Format::sprintf(buf, "%s's sector type has been changed to %d.\n", location->name, value);
//		stc(buf, ch);
		return;
	}

	/*
	 * Generate usage message.
	 */
	do_rset(ch, "");
} /* end do_rset() */

/***** STAT COMMANDS *****/
void format_mstat(Character *ch, Character *victim)
{
	if (victim->is_npc())
		ptc(ch, "Vnum: %d  Group: %s  Count: %d  Killed: %d\n",
		    victim->pIndexData->vnum,
		    victim->group_flags,
		    victim->pIndexData->count, victim->pIndexData->killed);

	ptc(ch, "{WRoom: %s {CName: %s{x\n",
	    victim->in_room == nullptr ? "0" : victim->in_room->location.to_string(), victim->name);

	if (!victim->is_npc())
		ptc(ch, "{CRemort %d, {x", victim->pcdata->remort_count);

	ptc(ch, "{CLevel %d{x", victim->level);

	if (!victim->is_npc())
		ptc(ch, "{C Age: %d (%d hours){x", get_age(victim), get_play_hours(victim));

	stc("\n", ch);
	ptc(ch, "{MRace: %s  Sex: %s  Guild: %s  Size: %s{x\n",
	    race_table[victim->race].name, sex_table[GET_ATTR_SEX(victim)].name,
	    victim->is_npc() ? "mobile" : victim->guild == Guild::none ? "none" : guild_table[victim->guild].name,
	    size_table[victim->size].name);

	if (!victim->is_npc())
		ptc(ch, "Killer: %d\tThief: %d\n",
		    victim->pcdata->flag_killer, victim->pcdata->flag_thief);

	ptc(ch, "Short description: %s{x\nLong  description: %s{x",
	    victim->short_descr, !victim->long_descr.empty() ? victim->long_descr : "(none)\n");
	stc("\n", ch);
	ptc(ch, "{PStr: %-2d(%-2d)\t{BAC Pierce : %-10d{RHit Points: %d/%d\n",
	    ATTR_BASE(victim, APPLY_STR), GET_ATTR_STR(victim),
	    GET_AC(victim, AC_PIERCE), victim->hit, GET_MAX_HIT(victim));
	ptc(ch, "{PInt: %-2d(%-2d)\t{BAC Bash   : %-10d{RMana      : %d/%d\n",
	    ATTR_BASE(victim, APPLY_INT), GET_ATTR_INT(victim),
	    GET_AC(victim, AC_BASH), victim->mana, GET_MAX_MANA(victim));
	ptc(ch, "{PWis: %-2d(%-2d)\t{BAC Slash  : %-10d{RStamina   : %d/%d\n",
	    ATTR_BASE(victim, APPLY_WIS), GET_ATTR_WIS(victim),
	    GET_AC(victim, AC_SLASH), victim->stam, GET_MAX_STAM(victim));
	ptc(ch, "{PDex: %-2d(%-2d)\t{BAC Magic  : %-10d{HItems     : %d\n",
	    ATTR_BASE(victim, APPLY_DEX), GET_ATTR_DEX(victim),
	    GET_AC(victim, AC_EXOTIC), get_carry_number(victim));
	ptc(ch, "{PCon: %-2d(%-2d)\t{bPrac      : %-10d{HWeight    : %d\n",
	    ATTR_BASE(victim, APPLY_CON), GET_ATTR_CON(victim),
	    victim->is_npc() ? 0 : victim->practice, get_carry_weight(victim) / 10);
	ptc(ch, "{PChr: %-2d(%-2d)\t{bTrain     : %-10d{GHit Roll  : %d\n",
	    ATTR_BASE(victim, APPLY_CHR), GET_ATTR_CHR(victim),
	    victim->is_npc() ? 0 : victim->train , GET_ATTR_HITROLL(victim));
	ptc(ch, "\t\t{YGold      : %-10ld{GDam Roll  : %d\n",
	    victim->gold, GET_ATTR_DAMROLL(victim));
	ptc(ch, "{CThirst: %-8d{YSilver    : %-10ld{WAlignment : %d\n",
	    (!victim->is_npc() ? victim->pcdata->condition[COND_THIRST] : -1),
	    victim->silver, victim->alignment);
	ptc(ch, "{CHunger: %-8d{WSaves     : %-10dWimpy     : %d\n",
	    (!victim->is_npc() ? victim->pcdata->condition[COND_HUNGER] : -1),
	    GET_ATTR_SAVES(victim), victim->wimpy);
	ptc(ch, "{CFull  : %-8d{cLast Level: %-10dMobTimer  : %d\n",
	    (!victim->is_npc() ? victim->pcdata->condition[COND_FULL] : -1),
	    (!victim->is_npc() ? victim->pcdata->last_level : -1),
	    (!victim->is_npc() ? victim->timer : -1));
	ptc(ch, "{CDrunk : %-8d{cExp       : %-10dPosition  : %s{x\n",
	    (!victim->is_npc() ? victim->pcdata->condition[COND_DRUNK] : -1),
	    victim->exp, position_table[victim->position].name);

	if (victim->is_npc())
		ptc(ch, "Damage: %dd%d  Message:  %s\n",
		    victim->damage[DICE_NUMBER], victim->damage[DICE_TYPE],
		    attack_table[victim->dam_type].noun);

	ptc(ch, "Fighting: %s\n", victim->fighting ? victim->fighting->name : "(none)");

	if (victim->is_npc() && victim->hunting != nullptr)
		ptc(ch, "Hunting victim: %s (%s)\n",
		    victim->hunting->is_npc() ? victim->hunting->short_descr
		    : victim->hunting->name,
		    victim->hunting->is_npc() ? "MOB" : "PLAYER");

	if (!victim->is_npc())
		if (!victim->pcdata->email.empty())
			ptc(ch, "Email: %s\n", victim->pcdata->email);

	ptc(ch, "{WAct: %s\n", act_bit_name(victim->act_flags, victim->is_npc()));

	if (!victim->is_npc()) {
		ptc(ch, "{WPlr: %s\n", plr_bit_name(victim->pcdata->plr_flags));

		if (!victim->pcdata->cgroup_flags.empty() && IS_IMP(ch))
			ptc(ch, "{WCommand Groups: %s\n", cgroup_bit_name(victim->pcdata->cgroup_flags));
	}

	if (!victim->comm_flags.empty())
		ptc(ch, "{WComm: %s\n", comm_bit_name(victim->comm_flags));

	if (!victim->revoke_flags.empty())
		ptc(ch, "{WRevoke: %s\n", revoke_bit_name(victim->revoke_flags));

	if (!victim->censor_flags.empty())
		ptc(ch, "{WCensor: %s\n", censor_bit_name(victim->censor_flags));

	if (victim->is_npc() && !victim->off_flags.empty())
		ptc(ch, "{WOffense: %s\n", off_bit_name(victim->off_flags));

	{
		String buf;
		buf = print_defense_modifiers(victim, TO_ABSORB);
		if (!buf.empty()) ptc(ch, "Abs : %s\n", buf);
		buf = print_defense_modifiers(victim, TO_IMMUNE);
		if (!buf.empty()) ptc(ch, "Imm : %s\n", buf);
		buf = print_defense_modifiers(victim, TO_RESIST);
		if (!buf.empty()) ptc(ch, "Res : %s\n", buf);
		buf = print_defense_modifiers(victim, TO_VULN);
		if (!buf.empty()) ptc(ch, "Vuln: %s\n", buf);
		buf = affect::print_cache(victim);
		if (!buf.empty()) ptc(ch, "Aff : %s\n", buf);
	}

	ptc(ch, "{xForm: %s\nParts: %s\n", form_bit_name(victim->form_flags), part_bit_name(victim->parts_flags));
	ptc(ch, "Master: %s  Leader: %s  Pet: %s\n",
	    victim->master  ? victim->master->name  : "(none)",
	    victim->leader  ? victim->leader->name  : "(none)",
	    victim->pet     ? victim->pet->name     : "(none)");

	if (!victim->is_npc() && victim->pcdata->mark_room.is_valid())
		ptc(ch, "MARKed room: %s\n", victim->pcdata->mark_room.to_string());

	if (victim->is_npc() && victim->spec_fun != 0)
		ptc(ch, "Mobile has special procedure %s.\n", spec_name(victim->spec_fun));

	if (!victim->is_npc() && victim->pcdata->raffect[0] != 0) {
		String buf;
		int raff, id;
		stc("{VRemort affects{x:\n", ch);
		id = (raff_lookup(victim->pcdata->raffect[0]));

		if (id == -1)
			Format::sprintf(buf, "Unknown");
		else
			Format::sprintf(buf, "%s", raffects[id].shortname);

		for (raff = 1; raff < ((victim->pcdata->remort_count / 10) + 1); raff++) {
			id = (raff_lookup(victim->pcdata->raffect[raff]));

			if (id == -1)
				buf += ", Unknown";
			else {
				buf += ", ";
				buf += raffects[id].shortname;
			}
		}

		buf += "\n";
		stc(buf, ch);
	}

	for (const affect::Affect *paf = affect::list_char(victim); paf != nullptr; paf = paf->next) {
		if (paf->permanent)
			continue;

		ptc(ch, "{bSpell: '%s'", affect::lookup(paf->type).name);

		if (paf->where == TO_AFFECTS) {
			if (paf->location != APPLY_NONE && paf->modifier != 0)
				ptc(ch, " modifies %s by %d", affect_loc_name(paf->location), paf->modifier);
		}
		else if (paf->where == TO_DEFENSE)
			ptc(ch, " %s damage from %s by %d%%",
				paf->modifier > 0 ? "reduces" : "increases",
				dam_type_name(paf->location),
				paf->modifier > 0 ? paf->modifier : -paf->modifier);
		else
			stc(" does something weird", ch);

		if (paf->duration >= 0)
			ptc(ch, " for %d hours", paf->duration);

		ptc(ch, ", level %d", paf->level);

		if (paf->evolution > 1)
			ptc(ch, ", evolve %d", paf->evolution);

		stc(".\n", ch);
	}
}

void format_ostat(Character *ch, Object *obj)
{
	ptc(ch, "{CVnum: %d   Level: %d\n", obj->pIndexData->vnum, obj->level);
	ptc(ch, "{CName(s):{x %s{x\n", obj->name);
	ptc(ch, "Short description: %s{x\nLong  description: %s{x\n",
	    obj->short_descr, obj->description);

	if (obj->extra_descr != nullptr || obj->pIndexData->extra_descr != nullptr) {
		ExtraDescr *ed;
		stc("Extra description keywords: '", ch);

		for (ed = obj->extra_descr; ed != nullptr; ed = ed->next) {
			stc(ed->keyword, ch);

			if (ed->next != nullptr)
				stc(" ", ch);
		}

		stc("'\n", ch);
		stc("Default description keywords: '", ch);

		for (ed = obj->pIndexData->extra_descr; ed != nullptr; ed = ed->next) {
			stc(ed->keyword, ch);

			if (ed->next != nullptr)
				stc(" ", ch);
		}
	}

	stc("\n", ch);
	ptc(ch, "{PValue0 : %-9d{BCost     : %-13d{GIn room   : %s\n",
	    obj->value[0], obj->cost, obj->in_room == nullptr ? "0" : obj->in_room->location.to_string());
	ptc(ch, "{PValue1 : %-9d{BCondition: %-13d{GIn object : %s\n",
	    obj->value[1], obj->condition, obj->in_obj == nullptr ? "(none)" : obj->in_obj->short_descr);
	ptc(ch, "{PValue2 : %-9d{BType     : %-13s{GIn %s : %s\n",
	    obj->value[2], item_type_name(obj),
	    obj->in_strongbox ? "StrBox" : "Locker",
	    obj->in_locker    ? PERS(obj->in_locker, ch, VIS_PLR) :
	    obj->in_strongbox ? PERS(obj->in_strongbox, ch, VIS_PLR) : "(none)");

	ptc(ch, "{PValue3 : ");
	if (obj->item_type == ITEM_PORTAL)
		ptc(ch, "%-9s", Location(obj->value[3]));
	else
		ptc(ch, "%-9d", obj->value[3]);

	ptc(ch, "{BResets   : %-13d{GCarried by: %s\n",
	    obj->pIndexData->reset_num,
	    obj->carried_by == nullptr ? "(none)" : PERS(obj->carried_by, ch, VIS_PLR));
	ptc(ch, "{PValue4 : %-9d{BTimer    : %-13d{GOwned by  : %s\n",
	    obj->value[4], obj->timer, get_owner(ch, obj));
	ptc(ch, "\t\t{BNumber   : %d/%-11d{GWear Loc. : %d\n",
	    1, get_obj_number(obj), obj->wear_loc);
	ptc(ch, "\t\t{BWeight   : %d/%d/%d (10th pounds){x\n",
	    obj->weight, get_obj_weight(obj), get_true_weight(obj));
	ptc(ch, "{W\nWear bits : %s\nExtra bits: %s{b\n\n",
	    wear_bit_name(obj->wear_flags), extra_bit_name(obj->extra_flags + obj->cached_extra_flags));
	if (obj->pIndexData->guild != Guild::none){
		int guildname = obj->pIndexData->guild;
		ptc(ch, "{WClass Restrictions: %s{Y.{x\n\n", guild_table[guildname].name );
	}
	
	//ptc(ch, "{W\nEQ bits : %s\n\n",
		//eq_bit_name(obj->eq_flags));
	/* now give out vital statistics as per identify */
	switch (obj->item_type) {
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		ptc(ch, "Level %d spells of:", obj->value[0]);

		for (int i = 1; i <= 4; i++) {
			skill::type type = skill::from_int(obj->value[i]);

			if (type != skill::type::unknown)
				ptc(ch, " '%s'", skill::lookup(type).name);
		}

		stc(".\n", ch);
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		ptc(ch, "Has %d charges of level %d", obj->value[2], obj->value[0]);
		ptc(ch, " '%s'", skill::lookup(skill::from_int(obj->value[3])).name);

		stc(".\n", ch);
		break;

	case ITEM_DRINK_CON:
		ptc(ch, "It holds %s-colored %s.\n",
		    liq_table[obj->value[2]].color, liq_table[obj->value[2]].name);
		break;

	case ITEM_CONTAINER:
		ptc(ch, "Capacity: %d#  Maximum weight: %d#  flags: %s\n",
		    obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));

		if (obj->value[4] != 100)
			ptc(ch, "Weight multiplier: %d%%\n", obj->value[4]);

		break;

	case ITEM_WEAPON:
		stc("Weapon type is ", ch);

		switch (obj->value[0]) {
		case (WEAPON_EXOTIC) : stc("exotic.\n", ch);       break;

		case (WEAPON_SWORD)  : stc("sword.\n", ch);        break;

		case (WEAPON_DAGGER) : stc("dagger.\n", ch);       break;

		case (WEAPON_SPEAR)  : stc("spear/staff.\n", ch);  break;

		case (WEAPON_MACE)   : stc("mace/club.\n", ch);    break;

		case (WEAPON_AXE)    : stc("axe.\n", ch);          break;

		case (WEAPON_FLAIL)  : stc("flail.\n", ch);        break;

		case (WEAPON_WHIP)   : stc("whip.\n", ch);         break;

		case (WEAPON_POLEARM): stc("polearm.\n", ch);      break;

		default             : stc("unknown.\n", ch);      break;
		}

		ptc(ch, "Damage is %dd%d (average %d).\n",
		    obj->value[1], obj->value[2], (1 + obj->value[2]) * obj->value[1] / 2);
/*
		if (obj->value[4] || !obj->cached_weapon_flags.empty())
			ptc(ch, "Weapons flags: %s\n", weapon_bit_name(obj->value[4] + obj->cached_weapon_flags));
*/
		break;

	case ITEM_ARMOR:
		ptc(ch, "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n",
		    obj->value[0], obj->value[1], obj->value[2], obj->value[3]);
		break;

	case ITEM_MATERIAL:
		ptc(ch, "Skill modifier: %d, Dice Bonus %d, Sides Bonus %d\n",
		    obj->value[0], obj->value[1], obj->value[2]);
		break;
	}

	for (const affect::Affect *paf = affect::list_obj(obj); paf != nullptr; paf = paf->next) {
/* debugging affects measure */
		ptc(ch, "wh: %d tp: %d lv: %d dr: %d lo: %d md: %d ev: %d bv: %d csum: %ld\n",
			paf->where, paf->type, paf->level, paf->duration, paf->location,
			paf->modifier, paf->evolution, paf->bitvector(), affect::checksum(paf));

		show_affect_to_char(paf, ch);
	}

	if (obj->num_settings > 0) {
		ptc(ch, "Has %d settings.\n", obj->num_settings);
	}

	for (Object *gem = obj->gems; gem; gem = gem->next_content)
		ptc(ch, "Has a gem %s of type %d with quality %d.\n",
				gem->short_descr, gem->value[0], gem->value[1]);
	if (obj->gems) {
		stc("Gems are adding:", ch);

		for (const affect::Affect *paf = obj->gem_affected; paf != nullptr; paf = paf->next)
			show_affect_to_char(paf, ch);
	}

}

void format_rstat(Character *ch, Room *location)
{
	String buf;
	Object *obj;
	Character *rch;
	int door;
	ptc(ch, "{W[%s] {gName: {P%s {W(%s){x\n", location->location.to_string(), location->name(), location->area().name);
	ptc(ch, "{YSector: %d\tLight: %d\tHealing: %d\tMana: %d{x\n",
	    location->sector_type(), location->light, location->heal_rate(), location->mana_rate());

	if (location->clan())
		ptc(ch, "{VClan: %s{x\n", location->clan()->who_name);

	if (location->guild() != Guild::none)
		ptc(ch, "{VGuild: %s{x\n", guild_table[location->guild()].name);

	ptc(ch, "{BDescription:{x\n%s\n", location->description());

	if (location->extra_descr() != nullptr) {
		const ExtraDescr *ed;
		stc("{BExtra description keywords: '{x", ch);

		for (ed = location->extra_descr(); ed; ed = ed->next)
			ptc(ch, "%s{x%s", ed->keyword, ed->next == nullptr ? "{B.{x\n" : " ");
	}

	if (!location->flags().empty())
		ptc(ch, "{CRoom flags: %s{x\n", room_bit_name(location->flags()));

	if (location->people) {
		stc("{GCharacters:{x ", ch);

		for (rch = location->people; rch; rch = rch->next_in_room)
			if (can_see_char(ch, rch)) {
				one_argument(rch->name, buf);
				ptc(ch, "%s{x%s", buf, rch->next_in_room ? " " : "\n");
			}
	}

	if (location->contents) {
		stc("{bObjects   :{x ", ch);

		for (obj = location->contents; obj; obj = obj->next_content) {
			one_argument(obj->name, buf);
			ptc(ch, "%s{x%s", buf, obj->next_content ? " " : "\n");
		}
	}

	for (door = 0; door <= 5; door++) {
		Exit *pexit;

		if ((pexit = location->exit[door]) != nullptr) {
			ptc(ch, "{WDoor: %d -> %s{c (Key: %d) Exit flags: %d. Keyword: '%s'{x\n",
			    door, (pexit->to_room == nullptr ? "-1" : pexit->to_room->location.to_string()),
			    pexit->key(), pexit->exit_flags, pexit->keyword());

			if (!pexit->description().empty())
				ptc(ch, "{cDescription: %s{x", pexit->description());
		}
	}

	for (const affect::Affect *paf = affect::list_room(location); paf != nullptr; paf = paf->next)
		ptc(ch, "{bAffect: '%s' modifies %s by %d for %d hours with bits %s, level %d, evolve %d.{x\n",
		    affect::lookup(paf->type).name,
		    affect_loc_name(paf->location),
		    paf->modifier,
		    paf->duration + 1,
		    room_bit_name(paf->bitvector()),
		    paf->level,
		    paf->evolution);
}

/* main stat function */
void do_stat(Character *ch, String argument)
{
	Room *room;
	Object *obj;
	Character *vch;

	if (argument.empty()) {
		stc("Syntax:\n"
		    "  stat <name or room location>\n"
		    "  stat room\n"
		    "  stat char   <name>\n"
		    "  stat mob    <name>\n"
		    "  stat player <name>\n"
		    "  stat obj    <name>\n", ch);
		return;
	}

	String arg1;
	String arg2 = one_argument(argument, arg1);

	if (arg1 == "char") {
		if (arg2.empty()) {
			stc("Stat what character?\n", ch);
			return;
		}

		if ((vch = get_char_here(ch, arg2, VIS_CHAR)) == nullptr)
			if ((vch = get_char_world(ch, arg2, VIS_CHAR)) == nullptr) {
				stc("No players or mobiles by that name are in the world.\n", ch);
				return;
			}

		format_mstat(ch, vch);
	}
	else if (arg1.is_prefix_of("mobile")) {
		if (arg2.empty()) {
			stc("Stat what mobile?\n", ch);
			return;
		}

		if ((vch = get_mob_here(ch, arg2, VIS_CHAR)) == nullptr)
			if ((vch = get_mob_world(ch, arg2, VIS_CHAR)) == nullptr) {
				stc("No mobiles by that name are in the world.\n", ch);
				return;
			}

		format_mstat(ch, vch);
	}
	else if (arg1.is_prefix_of("player")) {
		if (arg2.empty()) {
			stc("Stat what player?\n", ch);
			return;
		}

		if ((vch = get_player_world(ch, arg2, VIS_PLR)) == nullptr) {
			stc("No player by that name is in the world.\n", ch);
			return;
		}

		format_mstat(ch, vch);
	}
	else if (arg1.is_prefix_of("object")) {
		if (arg2.empty()) {
			stc("Stat what object?\n", ch);
			return;
		}

		if ((obj = get_obj_here(ch, arg2)) == nullptr)
			if ((obj = get_obj_world(ch, arg2)) == nullptr) {
				stc("No objects by that name are in the world.\n", ch);
				return;
			}

		format_ostat(ch, obj);
	}
	else if (arg1 == "room")        /* 'stat room' (current room) */
		format_rstat(ch, ch->in_room);
	else if ((room = Game::world().get_room(Location(arg1))) != nullptr) {

		if (!is_room_owner(ch, room)
		    && ch->in_room != room
		    && room->is_private()
		    && !IS_IMP(ch)) {
			stc("That room is private.\n", ch);
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
			stc("It does not exist.\n", ch);
	}
}

/* new player stat -- Elrac */
void do_pstat(Character *ch, String argument)
{
	Character *victim;
	int xpl, xnl; /* experience per/next level */

	if ((victim = get_player_world(ch, argument, VIS_PLR)) == nullptr) {
		ptc(ch, "No player named '%s' found in the game.\n", argument);
		return;
	}

	if (!victim->pcdata) {
		Logging::bug("do_pstat: player without pcdata", 0);
		stc("No information available about that player.\n", ch);
		return;
	}

	/* PLAYER: Elrac [of Oberrad, Lord of the Code] */
	new_color(ch, CSLOT_OLDSCORE_PKILL);
	stc("PLAYER: ", ch);
	new_color(ch, CSLOT_OLDSCORE_NAME);
	stc(victim->name, ch);

	if (!victim->pcdata->title.empty()) {
		set_color(ch, WHITE, NOBOLD);
		stc(" [", ch);
		new_color(ch, CSLOT_OLDSCORE_NAME);
		stc(victim->pcdata->title, ch);
		set_color(ch, WHITE, NOBOLD);
		stc("]", ch);
	}

	stc("\n", ch);
	/* Lvl 94/185 (99) superhuman male warrior, leader of clan "CODER" */
	new_color(ch, CSLOT_OLDSCORE_NAME);
	ptc(ch, "Lvl %d", victim->level);

	if (victim->pcdata->remort_count)
		ptc(ch, "/R%d", victim->pcdata->remort_count);

	ptc(ch, " %s %s %s",
	    race_table[victim->race].name,
	    sex_table[GET_ATTR_SEX(ch)].name,
	    victim->guild == Guild::none ? "none" : guild_table[victim->guild].name);

	if (victim->clan) {
		stc(", ", ch);
		set_color(ch, WHITE, NOBOLD);
		ptc(ch, "%s of clan %s",
		    victim->has_cgroup(GROUP_LEADER) ? "Leader" :
		    victim->has_cgroup(GROUP_DEPUTY) ? "Deputy" :
		    "Member",
		    victim->clan->who_name);
	}

	stc("\n", ch);

	/* email: carls@ipf.de */
	if (!victim->pcdata->email.empty()) {
		set_color(ch, WHITE, NOBOLD);
		ptc(ch, "Email: %s\n", victim->pcdata->email);
	}

	/* Str 25(25)    Align -1000   Hp 31418/31418 */
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Str %-2d(%-2d)",
	    ATTR_BASE(victim, APPLY_STR), GET_ATTR_STR(victim));
	new_color(ch, CSLOT_OLDSCORE_ALIGN);
	ptc(ch, "      Align %s%-7d",
	    victim->alignment < 0 ? "{R" : "{Y", victim->alignment);
	new_color(ch, CSLOT_OLDSCORE_POINTS);
	ptc(ch, "      HP   %-5d/%-5d\n",
	    victim->hit, GET_MAX_HIT(victim));
	/* Int 25(25)    Train 2       Ma 31573/31573 */
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Int %-2d(%-2d)",
	    ATTR_BASE(victim, APPLY_INT), GET_ATTR_INT(victim));
	new_color(ch, CSLOT_OLDSCORE_GAIN);
	ptc(ch, "      Train %-7d", victim->train);
	new_color(ch, CSLOT_OLDSCORE_POINTS);
	ptc(ch, "      Mana %-5d/%-5d\n",
	    victim->mana, GET_MAX_MANA(victim));
	/* Wis 25(25)    Pracs 129     Stm 30603/30603 */
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Wis %-2d(%-2d)",
	    ATTR_BASE(victim, APPLY_WIS), GET_ATTR_WIS(victim));
	new_color(ch, CSLOT_OLDSCORE_GAIN);
	ptc(ch, "      Pracs %-7d",
	    victim->practice);
	new_color(ch, CSLOT_OLDSCORE_POINTS);
	ptc(ch, "      Stm  %-5d/%-5d\n",
	    victim->stam, GET_MAX_STAM(victim));
	/* Dex 25(25)                  QP:  12345      */
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Dex %-2d(%-2d)",
	    ATTR_BASE(victim, APPLY_DEX), GET_ATTR_DEX(victim));
	new_color(ch, CSLOT_OLDSCORE_QP);
	ptc(ch, "                         QP:  %-5d\n", victim->pcdata->questpoints);
	/* Con 25(25)   Gold 7481204   SP:  12345 */
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Con %-2d(%-2d)",
	    ATTR_BASE(victim, APPLY_CON), GET_ATTR_CON(victim));
	new_color(ch, CSLOT_OLDSCORE_MONEY);
	ptc(ch, "      Gold  %-7ld",
	    victim->gold);
	new_color(ch, CSLOT_OLDSCORE_SP);
	ptc(ch, "      SP:  %-5d\n", victim->pcdata->skillpoints);
	/* Chr 25(25)   Silv 2768604   RPP: 12345 */
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Chr %-2d(%-2d)",
	    ATTR_BASE(victim, APPLY_CHR), GET_ATTR_CHR(victim));
	new_color(ch, CSLOT_OLDSCORE_MONEY);
	ptc(ch, "      Silv  %-7ld", victim->silver);
	new_color(ch, CSLOT_OLDSCORE_RPP);
	ptc(ch, "      RPP: %d\n", victim->pcdata->rolepoints);
	/* Xtl 123456      X/lv  1234567      Exp 1234567 */
	xpl = exp_per_level(victim, victim->pcdata->points);
	xnl = (victim->level + 1) * xpl;
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Xtl %-6d      X/lv  %-7d      Exp %d\n",
	    xnl - victim->exp, xpl, victim->exp);
	/* (LinkDead) [AFK] (Invis) (Wizi) (Lurk) (Hide) (Charmed) (KILLER) (THIEF) */
	ptc(ch, "%s%s%s%s%s%s%s%s%s%s{x\n",
	    victim->pcdata->plr_flags.has(PLR_LINK_DEAD) ?        "{G(LinkDead) " : "",
	    victim->comm_flags.has(COMM_AFK) ?             "{b[AFK] "      : "",
	    affect::exists_on_char(victim, affect::type::invis) ?        "{C(Invis) "    : "",
	    affect::exists_on_char(victim, affect::type::hide) ?             "{B(Hide) "     : "",
	    affect::exists_on_char(victim, affect::type::midnight) ?            "{c(Shadowy) "  : "",
	    victim->invis_level ?                                   "{T(Wizi) "     : "",
	    victim->lurk_level ?                                    "{H(Lurk) "     : "",
	    affect::exists_on_char(victim, affect::type::charm_person) ?                        "{M(Charmed) "  : "",
	    victim->act_flags.has(PLR_KILLER) ?                       "{R(KILLER) "   : "",
	    victim->act_flags.has(PLR_THIEF) ?                        "{B(THIEF) "    : "");
	/* fighting in room 3001, in combat with Super Helga */
	ptc(ch, "{W%s in room [%s]",
	    position_table[victim->position].name,
	    victim->in_room ? victim->in_room->location.to_string() : "0");

	if (victim->fighting)
		ptc(ch, ", {Rin combat{x with %s", victim->fighting->name);

	stc("{a{x\n", ch);
	set_color(ch, WHITE, NOBOLD);
}
