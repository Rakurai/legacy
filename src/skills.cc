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

#include "act.hh"
#include "argument.hh"
#include "Character.hh"
#include "declare.hh"
#include "Flags.hh"
#include "Format.hh"
#include "interp.hh"
#include "lootv2.hh"
#include "macros.hh"
#include "magic.hh"
#include "memory.hh"
#include "merc.hh"
#include "Player.hh"
#include "random.hh"
#include "Room.hh"
#include "String.hh"
#include "tables.hh"

/* return TRUE if a player either has the group or has all skills in it */
bool completed_group(Character *ch, int gn)
{
	if (ch->pcdata->group_known[gn])
		return TRUE;

	for (const auto& spell : group_table[gn].spells) {
		int ngn = group_lookup(spell);

		if (ngn >= 0) {
			if (group_table[ngn].rating[ch->cls] <= 0)
				continue;

			if (!completed_group(ch, ngn))
				return FALSE;

			continue;
		}

		if (get_learned(ch, skill::lookup(spell)) <= 0)
			return FALSE;
	}

	return TRUE;
}

/* function for qsort of group numbers by group name */
int comp_groupnames(const void *gn1, const void *gn2)
{
	return strcmp(group_table[*((int *) gn1)].name, group_table[*((int *) gn2)].name);
}

/* structure of sortable spell info */
struct s_spell_info {
	skill::type type;
	int level; /* level for the current ch */
};

/* function for qsort of spells by level and name */
int comp_spells(const void *sn1, const void *sn2)
{
	const struct s_spell_info *si1 = (struct s_spell_info *)sn1;
	const struct s_spell_info *si2 = (struct s_spell_info *)sn2;
	int level_diff = si1->level - si2->level;

	if (level_diff != 0)
		return level_diff;

	return strcmp(skill::lookup(si1->type).name, skill::lookup(si2->type).name);
}

/* SPELLS, extended to list spells by levels, group and percentage -- Elrac */
/* Possible arguments:
   * SPELLS         -> all spells available to player
     SPELLS 50      -> spells from level 50 up
     SPELLS 50 60   -> spells from lv 50 - 60
   * SPELLS GROUPS  -> list of spell groups ('*' = gained)
     SPELLS WEATHER -> list of spells in the WEATHER group
     SPELLS ACID    -> list of spells beginning with 'acid'
*/
void do_spells(Character *ch, String argument)
{
	int level;
	int min_level = 1;
	int max_level = LEVEL_HERO;
	int group = -1;
	int j;
	int group_list[group_table.size()];
	int ngroups = 0;
	struct s_spell_info spell_list[skill_table.size()-1];
	int nspells = 0;
	String arg;
	int cols = 0;
	int pos = 18;
	bool found = FALSE;
	bool new_level = TRUE;
	String buffer;
	bool reached_player_level = FALSE;

	if (IS_NPC(ch)) {
		stc("You know all that you need, trust me. ;)\n", ch);
		return;
	}

	/* SPELLS: list all available spells, as in times of olde */
	if (argument.empty()) {
		do_spells(ch, "1");
		return;
	}

	/* kludge to build a list of only spell groups:
	   spell groups are mixed with skill groups,
	   but currently 'attack' is the first one. */
	for (unsigned int gn = group_lookup("attack"); gn < group_table.size(); gn++) {
		group_list[ngroups++] = gn;
	}

	/* sort groups by name */
	qsort(group_list, ngroups, sizeof(int), comp_groupnames);

	/* SPELLS GROUPS: List spell groups, sorted, indicate which are learned */
	if (argument.is_prefix_of("groups")) {
		stc("Spell groups (* = gained):\n"
		    "--------------------------\n", ch);

		for (j = 0; j < ngroups; j++) {
			int gn = group_list[j];

			if (cols > 0)
				stc("  ", ch);

			ptc(ch, "%c %-22s", ch->pcdata->group_known[gn] ? '*' : ' ', group_table[gn].name);

			if (++cols >= 3) {
				stc("\n", ch);
				cols = 0;
			}
		}

		if (cols > 0)
			stc("\n", ch);

		return;
	}

	/* Check for complete name of a spell group. */
	for (j = 0; j < ngroups; j++) {
		int gn = group_list[j];

		if (argument == group_table[gn].name) {
			/* discard group name argument */
			argument = one_argument(argument, arg);
			group = gn;
			found = TRUE;
			break;
		}
	}

	/* Check for a spell name */
	String spell_name;

	argument = one_argument(argument, arg);

	if (!found && !arg.is_number()) {
		spell_name = arg;
		argument = one_argument(argument, arg);
	}

	/* Check for 1 or 2 numbers. */
	if (arg.is_number()) {
		min_level = atoi(arg);

		if (min_level < 0 || min_level > LEVEL_HERO) {
			stc("That's not a valid minimum level.\n", ch);
			return;
		}

		if (!argument.empty()) {
			argument = one_argument(argument, arg);

			if (arg.is_number()) {
				max_level = atoi(arg);

				if (max_level < min_level)
					max_level = min_level;
				else if (max_level > LEVEL_HERO) {
					stc("That's not a valid maximum level.\n", ch);
					return;
				}
			}
		}
	}

	/* build a list of spells satisfying the given criteria. */
	if (group != -1) {
		/* select spells from given group */
		for (const auto& spell : group_table[group].spells) {
			skill::type type = skill::lookup(spell);

			if (type == skill::type::unknown)
				continue;

			level = skill::lookup(type).skill_level[ch->cls];

			if (level < min_level || max_level < level)
				continue;

			spell_list[nspells].type = type;
			spell_list[nspells].level = level;
			nspells++;
			found = TRUE;
		}
	}
	else {
		/* select spells from player's repertoire or by name */
		for (const auto& pair : skill_table) {
			skill::type type = pair.first;
			const auto& entry = pair.second;

			if (type == skill::type::unknown)
				continue;

			level = entry.skill_level[ch->cls];

			if (level < min_level
			    || max_level < level
			    || entry.spell_fun == spell_null)
				continue;

			if (spell_name.empty()) {
				if (get_learned(ch, type) <= 0
				    || (entry.remort_class != 0 && !IS_IMMORTAL(ch)
				        && ch->cls + 1 != entry.remort_class && !HAS_EXTRACLASS(ch, type)))
					continue;
			}
			else if (!entry.name.has_words(spell_name))
				continue;

			found = TRUE;
			spell_list[nspells].type = type;
			spell_list[nspells].level = level;
			nspells++;
		}
	}

	if (!found) {
		/* if they did a list of all spells... */
		if (min_level == 1 && max_level == LEVEL_HERO && spell_name.empty())
			stc("You know no spells.\n", ch);
		else
			stc("No spell group of that name and\n"
			    "no spells with that name and/or level found.\n", ch);

		return;
	}


	if (group != -1) {
		buffer += Format::format("Spells in group '%s':\n\n", group_table[group].name);
	}

	/* sort */
	if (nspells > 0)
		qsort(spell_list, nspells, sizeof(spell_list[0]), comp_spells);

	/* output the spells found in spell_list */
	level = -1;

	for (j = 0; j < nspells; j++) {
		if (spell_list[j].level != level) {
			new_level = TRUE;
			level = spell_list[j].level;

			if (cols > 0) {
				buffer += "\n";
				cols = 0;
			}

			if (level > ch->level && !reached_player_level) {
				buffer += Format::format("Level %3d: %s  (your level)\n", ch->level,
				        "-------------------------------");
				reached_player_level = TRUE;
			}
		}

		if (cols == 0) {
			String buf = Format::format("Level %3d: ", level);

			if (!new_level)
				buf = Format::format("%*s", buf.size(), " "); // spaces

			new_level = FALSE;
			buffer += buf;
		}
		else {
			buffer += Format::format("%*s", 24 - pos, " ");
		}

		skill::type type = spell_list[j].type;
		const auto& entry = skill::lookup(type);

		if (get_learned(ch, type) <= 0
		    || (entry.remort_class != 0 && !IS_IMMORTAL(ch)
		        && ch->cls + 1 != entry.remort_class && !HAS_EXTRACLASS(ch, type)))
			buffer += "[not  gained] ";
		else if (ch->level < level)
			buffer += "[           ] ";
		else
			buffer += Format::format("[{V%3d%% %3d Ma{x] ", get_learned(ch, type), get_skill_cost(ch, type));

		Format::sprintf(arg, "{C%-1.20s{x", entry.name);
		buffer += arg;

		if (++cols < 2)
			pos = strlen(arg);
		else {
			buffer += "\n";
			cols = 0;
		}
	}

	if (cols > 0) {
		buffer += "\n";
		cols = 0;
	}

	page_to_char(buffer, ch);
} /* end do_spells() */

/* The guts of 'spells', used here for skill listing - Montrey */
/* Possible arguments:
     SKILLS         -> all spells available to player
     SKILLS 50      -> spells from level 50 up
     SKILLS 50 60   -> spells from lv 50 - 60
     SKILLS DUAL    -> list of spells beginning with 'dual'
*/
void do_skills(Character *ch, String argument)
{
	int level;
	int min_level = 1;
	int max_level = LEVEL_HERO;
	int j;
	struct s_spell_info skill_list[skill_table.size()-1];
	int nskills = 0;
	String buf;
	int cols = 0;
	int pos = 18;
	bool found = FALSE;
	bool new_level = TRUE;
	String buffer;
	bool reached_player_level = FALSE;

	if (IS_NPC(ch)) {
		stc("You know all that you need, trust me. ;)\n", ch);
		return;
	}

	/* SKILLS: list all available skills */
	if (argument.empty()) {
		do_skills(ch, "1");
		return;
	}

	/* Check for a skill name */
	String skill_name;

	String arg;
	argument = one_argument(argument, arg);

	if (!arg.is_number()) {
		skill_name = arg;
		argument = one_argument(argument, arg);
	}

	/* Check for 1 or 2 numbers. */
	if (arg.is_number()) {
		min_level = atoi(arg);

		if (min_level < 1)
			min_level = 1;
		else if (min_level > LEVEL_HERO)
			min_level = LEVEL_HERO;

		if (!argument.empty()) {
			argument = one_argument(argument, arg);

			if (arg.is_number()) {
				max_level = atoi(arg);

				if (max_level < min_level)
					max_level = min_level;
				else if (max_level > LEVEL_HERO)
					max_level = LEVEL_HERO;
			}
		}
	}

	/* select skills from player's repertoire or by name */
	for (const auto& pair : skill_table) {
		skill::type type = pair.first;
		const auto& entry = pair.second;

		if (type == skill::type::unknown)
			continue;

		level = entry.skill_level[ch->cls];

		if (level < min_level
		    || max_level < level
		    || entry.spell_fun != spell_null)
			continue;

		if (skill_name.empty()) {
			if (get_learned(ch, type) <= 0
			    || (entry.remort_class != 0 && !IS_IMMORTAL(ch)
			        && ch->cls + 1 != entry.remort_class && !HAS_EXTRACLASS(ch, type)))
				continue;
		}
		else if (!entry.name.has_words(skill_name))
			continue;

		found = TRUE;
		skill_list[nskills].type = type;
		skill_list[nskills].level = level;
		nskills++;
	}

	if (!found) {
		/* if they did a list of all spells... */
		if (min_level == 1 && max_level == LEVEL_HERO && skill_name.empty())
			stc("You know no skills.\n", ch);
		else
			stc("No skills of that name found.\n", ch);

		return;
	}


	/* sort */
	if (nskills > 0)
		qsort(skill_list, nskills, sizeof(skill_list[0]), comp_spells);

	/* output the skills found in skill_list */
	level = -1;

	for (j = 0; j < nskills; j++) {
		if (skill_list[j].level != level) {
			new_level = TRUE;
			level = skill_list[j].level;

			if (cols > 0) {
				buffer += "\n";
				cols = 0;
			}

			if (level > ch->level && !reached_player_level) {
				Format::sprintf(buf, "Level %3d: %s  (your level)\n", ch->level,
				        "-------------------------------");
				buffer += buf;
				reached_player_level = TRUE;
			}
		}

		if (cols == 0) {
			Format::sprintf(buf, "Level %3d: ", level);

			if (!new_level)
				Format::sprintf(buf, "%*s", (int)strlen(buf), " ");

			new_level = FALSE;
			buffer += buf;
		}
		else {
			Format::sprintf(buf, "%*s", 24 - pos, " ");
			buffer += buf;
		}

		skill::type type = skill_list[j].type;
		const auto& entry = skill::lookup(type);

		if (get_learned(ch, type) <= 0
		    || (entry.remort_class != 0 && !IS_IMMORTAL(ch)
		        && ch->cls + 1 != entry.remort_class && !HAS_EXTRACLASS(ch, type)))
			Format::sprintf(buf, "[not  gained] ");
		else if (ch->level < level)
			Format::sprintf(buf, "[           ] ");
		else
			Format::sprintf(buf, "[{G%3d%% %3d St{x] ", get_learned(ch, type), get_skill_cost(ch, type));

		Format::sprintf(arg, "{H%-1.20s{x", entry.name);
		buf += arg;

		if (++cols < 2)
			pos = strlen(arg);
		else {
			buf += "\n";
			cols = 0;
		}

		buffer += buf;
	}

	if (cols > 0) {
		buffer += "\n";
		cols = 0;
	}

	page_to_char(buffer, ch);
}

/* Levelist by Lotus */
void do_levels(Character *ch, String argument)
{
	String list[LEVEL_HERO];
	char columns[LEVEL_HERO] = {0};
	int lev, x, y;
	char buf[MAX_STRING_LENGTH];
	int cls;
	String buffer;

	if (IS_NPC(ch))
		return;

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		if (!IS_IMMORTAL(ch)) {
			stc("What class do you want level info on?\n", ch);
			return;
		}
		else {
			stc("Skill/Spell           Mag Cle Thi War Nec Pdn Bar Ran\n\n", ch);

			for (const auto& pair : skill_table) {
				skill::type type = pair.first;
				const auto& entry = pair.second;

				if (type == skill::type::unknown)
					continue;

				if (entry.remort_class < 0) /* for completely ungainable spells/skills */
					continue;

				if (entry.remort_class > 0) /* list of remort skills is through 'levels remort' */
					continue;

				Format::sprintf(buf, "%-21s", entry.name);
				buffer += buf;

				for (x = 0; x < MAX_CLASS; x++) {
					if (entry.skill_level[x] < 0 || entry.skill_level[x] > LEVEL_HERO)
						Format::sprintf(buf, "{c  NA{x");
					else
						Format::sprintf(buf, "%4d", entry.skill_level[x]);

					buffer += buf;
				}

				Format::sprintf(buf, "\n");
				buffer += buf;
			}

			page_to_char(buffer, ch);
			return;
		}
	}

	if (arg.is_prefix_of("remort") && IS_IMMORTAL(ch)) {
		stc("                                           Level/Trains\n", ch);
		stc("                       Mag    Cle    Thi    War    Nec    Pdn    Bar    Ran\n", ch);

		for (x = 0; x < MAX_CLASS; x++) {
			Format::sprintf(buf, "{W%s Skills:{x\n", class_table[x].name.capitalize());
			buffer += buf;

			for (const auto& pair : skill_table) {
				skill::type type = pair.first;
				const auto& entry = pair.second;

				if (type == skill::type::unknown)
					continue;

				if (entry.remort_class != x + 1)
					continue;

				Format::sprintf(buf, "  {g%-19s{x", entry.name);
				buffer += buf;

				for (y = 0; y < MAX_CLASS; y++) {
					if (entry.skill_level[y] < 0 || entry.skill_level[y] > LEVEL_HERO)
						Format::sprintf(buf, " {TNA{c/{HNA {x");
					else
						Format::sprintf(buf, "{C%3d{c/{G%-3d{x", entry.skill_level[y], entry.rating[y]);

					buffer += buf;
				}

				Format::sprintf(buf, "\n");
				buffer += buf;
			}
		}

		page_to_char(buffer, ch);
		return;
	}

	if (arg.is_prefix_of("mage"))             cls = 0;
	else if (arg.is_prefix_of("cleric"))      cls = 1;
	else if (arg.is_prefix_of("thief"))       cls = 2;
	else if (arg.is_prefix_of("warrior"))     cls = 3;
	else if (arg.is_prefix_of("necromancer")) cls = 4;
	else if (arg.is_prefix_of("paladin"))     cls = 5;
	else if (arg.is_prefix_of("bard"))        cls = 6;
	else if (arg.is_prefix_of("ranger"))      cls = 7;
	else {
		stc("Invalid class.\n", ch);
		return;
	}

	for (const auto& pair : skill_table) {
		skill::type type = pair.first;
		const auto& entry = pair.second;

		if (type == skill::type::unknown)
			continue;

		if (entry.skill_level[cls] < 0)
			continue;

		if (entry.skill_level[cls] < LEVEL_HERO &&
		    entry.spell_fun != spell_null) {
			lev = entry.skill_level[cls];
			Format::sprintf(buf, "{G%-20s  {C%3d{x {Tmana{x  ", entry.name,
			        entry.min_mana);

			if (list[lev].empty())
				Format::sprintf(list[lev], "\n{HLevel %2d: %s", lev, buf);
			else { /* append */
				if (++columns[lev] % 2 == 0)
					list[lev] += "\n          ";

				list[lev] += buf;
			}
		}
	}

	for (lev = 0; lev < LEVEL_HERO; lev++)
//		if (!list[lev].empty())
			buffer += list[lev];

	buffer += "\n";

	/* Initialize the data */
	for (lev = 0; lev < LEVEL_HERO; lev++) {
		columns[lev] = 0;
		list[lev].clear();
	}

	for (const auto& pair : skill_table) {
		skill::type type = pair.first;
		const auto& entry = pair.second;

		if (type == skill::type::unknown)
			continue;

		if (entry.skill_level[ch->cls] < 0)
			continue;

		if (entry.skill_level[cls] < LEVEL_HERO &&
		    entry.spell_fun == spell_null) {
			lev = entry.skill_level[cls];
			Format::sprintf(buf, "{G%-20s   {x        ", entry.name);

			if (list[lev].empty())
				Format::sprintf(list[lev], "\n{HLevel %2d: %s{x", lev, buf);
			else { /* append */
				if (++columns[lev] % 2 == 0)
					list[lev] += "\n          ";

				list[lev] += buf;
			}
		}
	}

	for (lev = 0; lev < LEVEL_HERO; lev++)
//		if (!list[lev].empty())
			buffer += list[lev];

	buffer += "\n";
	page_to_char(buffer, ch);
} /* end do_levels() */


long exp_per_level(Character *ch, int points)
{
	int  inc;
	long expl;

	if (IS_NPC(ch))
		return 1000;

	expl = 1000;

	if (points >= 40) {
		/* processing */
		inc = 500;
		points -= 40;

		while (points >= 10) {
			expl += inc;
			points -= 10;

			if (points >= 10) {
				expl += inc;
				inc *= 2;
				points -= 10;
			}
		}

		expl += points * inc / 10;
	}

	expl *= pc_race_table[ch->race].class_mult[ch->cls];
	expl /= 100;
	/* New remort system - 7/2, Ian
	   Taken out per new requirements
	if ( ch->pcdata->remort_count > 1 )
	{
	    expl += (1000 * (ch->pcdata->remort_count - 1));
	}
	*/
	return expl;
} /* end exp_per_level() */

/* shows all groups, or the sub-members of a group */
void do_groups(Character *ch, String argument)
{
	char buf[100];

	if (IS_NPC(ch))
		return;

	int col = 0;

	if (argument.empty()) {
		/* show all groups */
		for (unsigned int gn = 0; gn < group_table.size(); gn++) {
			if (ch->pcdata->group_known[gn]) {
				Format::sprintf(buf, "%-18s ", group_table[gn].name);
				stc(buf, ch);

				if (++col % 3 == 0)
					stc("\n", ch);
			}
		}

		if (col % 3 != 0)
			stc("\n", ch);

		Format::sprintf(buf, "Creation points: %d\n", ch->pcdata->points);
		stc(buf, ch);
		return;
	}

	if (argument == "all") { /* show all groups */
		for (unsigned int gn = 0; gn < group_table.size(); gn++) {
			Format::sprintf(buf, "%-18s ", group_table[gn].name);
			stc(buf, ch);

			if (++col % 3 == 0)
				stc("\n", ch);
		}

		if (col % 3 != 0)
			stc("\n", ch);

		return;
	}

	/* show the sub-members of a group */
	int gn = group_lookup(argument);

	if (gn == -1) {
		stc("No group of that name exist.\n", ch);
		stc(
		        "Type 'groups all' or 'info all' for a full listing.\n", ch);
		return;
	}

	for (String spell : group_table[gn].spells) {
		Format::sprintf(buf, "%-18s ", spell);
		stc(buf, ch);

		if (++col % 3 == 0)
			stc("\n", ch);
	}

	if (col % 3 != 0)
		stc("\n", ch);
}

/* checks for skill improvement */
void check_improve(Character *ch, skill::type type, bool success, int multiplier)
{
	int chance;
	char buf[100];
	int xp;

	if (IS_NPC(ch))
		return;

	if (ch->level < skill::lookup(type).skill_level[ch->cls]
	    ||  skill::lookup(type).rating[ch->cls] == 0
	    ||  get_learned(ch, type) == 0
	    ||  get_learned(ch, type) == 100)
		return;  /* skill is not known */

	/* check to see if the character has a chance to learn */
	chance = 10 * int_app[GET_ATTR_INT(ch)].learn;
	chance /= (multiplier
	           *       skill::lookup(type).rating[ch->cls]
	           *       4);
	chance += ch->level;

	if (skill::lookup(type).remort_class > 0)
	{   chance *= 2; chance /= 3; }

	/* -1 multiplier means auto success -- Montrey */
	if (number_range(1, 1000) > chance && multiplier != -1)
		return;

	/* pre-calculate the experience gain */
	xp = 2 * skill::lookup(type).rating[ch->cls];
	/*
	if ( ch->pcdata->remort_count > 1 )
	    xp /= ch->pcdata->remort_count;
	*/

	/* now that the character has a CHANCE to learn, see if they really have */

	if (success) {
		chance = URANGE(5, 100 - get_learned(ch, type), 95);

		if (number_percent() < chance || multiplier == -1) {
			Format::sprintf(buf, "{GYou have become better at {H%s{G!{x\n",
			        skill::lookup(type).name);
			stc(buf, ch);
			set_learned(ch, type, get_learned(ch, type) + 1);
			gain_exp(ch, xp);
		}
	}
	else {
		chance = URANGE(5, get_learned(ch, type) / 2, 30);

		if (number_percent() < chance || multiplier == -1) {
			Format::sprintf(buf,
			        "{GYou learn from your mistakes, and your {H%s {Gskill improves.{x\n",
			        skill::lookup(type).name);
			stc(buf, ch);
			set_learned(ch, type, get_learned(ch, type) + number_range(1, 3));
			gain_exp(ch, xp);
		}
	}
} /* end check_improve() */

/* returns a group index number given the name */
int group_lookup(const String& name)
{
	for (unsigned int gn = 0; gn < group_table.size(); gn++)
		if (name == group_table[gn].name)
			return gn;

	return -1;
}

/* recursively adds a group given its number -- uses group_add */
void gn_add(Character *ch, int gn)
{
	ch->pcdata->group_known[gn] = TRUE;

	for (const auto& spell : group_table[gn].spells)
		group_add(ch, spell, FALSE);
}

/* recusively removes a group given its number -- uses group_remove */
void gn_remove(Character *ch, int gn)
{
	ch->pcdata->group_known[gn] = FALSE;

	for (const auto& spell : group_table[gn].spells)
		group_remove(ch, spell);
}

/* use for processing a skill or group for addition  */
void group_add(Character *ch, const String& name, bool deduct)
{
	if (IS_NPC(ch)) /* NPCs do not have skills */
		return;

	skill::type type = skill::lookup(name);

	if (type != skill::type::unknown) {
		if (get_learned(ch, type) == 0) { /* i.e. not known */
			set_learned(ch, type, 1);
			set_evolution(ch, type, 1);

			if (deduct)
				ch->pcdata->points += skill::lookup(type).rating[ch->cls];
		}

		return;
	}

	/* now check groups */
	int gn = group_lookup(name);

	if (gn != -1) {
		if (ch->pcdata->group_known[gn] == FALSE) {
			ch->pcdata->group_known[gn] = TRUE;

			if (deduct)
				ch->pcdata->points += group_table[gn].rating[ch->cls];
		}

		gn_add(ch, gn); /* make sure all skills in the group are known */
	}
}

/* used for processing a skill or group for deletion -- no points back! */

void group_remove(Character *ch, const String& name)
{
	skill::type type = skill::lookup(name);

	if (type != skill::type::unknown) {
		set_learned(ch, type, 0);
		set_evolution(ch, type, 1);
		return;
	}

	/* now check groups */
	int gn = group_lookup(name);

	if (gn != -1 && ch->pcdata->group_known[gn] == TRUE) {
		ch->pcdata->group_known[gn] = FALSE;
		gn_remove(ch, gn); /* be sure to call gn_add on all remaining groups */
	}
}

void set_learned(Character *ch, skill::type sn, int value) {
	if (IS_NPC(ch) || sn == skill::type::unknown)
		return;

	ch->pcdata->learned[(int)sn] = URANGE(0, value, 100);
}

int get_learned(const Character *ch, skill::type sn) {
	if (IS_NPC(ch) || sn == skill::type::unknown)
		return 0;

	return ch->pcdata->learned[(int)sn];
}

/* for returning skill information */
int get_skill_level(const Character *ch, skill::type sn)
{
	int skill = 0;

	if (sn == skill::type::unknown) /* shorthand for level based skills */
		skill = ch->level * 5 / 2;
	else if (!IS_NPC(ch)) {
		if (ch->level < skill::lookup(sn).skill_level[ch->cls])
			skill = 0;
		else
			skill = get_learned(ch, sn);
	}
	else { /* mobiles */
		switch (sn) {
		case skill::type::dodge:
			if (ch->off_flags.has(OFF_DODGE))						skill = 10 + ch->level; break;
		case skill::type::parry:
			if (ch->off_flags.has(OFF_PARRY))						skill = 10 + ch->level; break;
		case skill::type::kick:
			if (ch->off_flags.has(OFF_KICK))						skill = 10 + ch->level; break;
		case skill::type::second_attack:
		    if (ch->act_flags.has_any_of(ACT_WARRIOR|ACT_THIEF))	skill = 25 + ch->level; break;
		case skill::type::third_attack:
			if (ch->act_flags.has(ACT_WARRIOR))						skill = 15 + ch->level; break;
		case skill::type::fourth_attack:
			if (ch->act_flags.has(ACT_WARRIOR))						skill = 2 * (ch->level - 60); break;
		case skill::type::hand_to_hand:								skill = ch->level * 3 / 2; break;
		case skill::type::trip:
			if (ch->off_flags.has(OFF_TRIP))						skill = 10 + (ch->level * 3 / 2); break;
		case skill::type::dirt_kicking:
			if (ch->off_flags.has(OFF_KICK_DIRT))					skill = 10 + (ch->level * 3 / 2); break;
		case skill::type::bash:
			if (ch->off_flags.has(OFF_BASH))						skill = 10 + (ch->level * 5 / 4); break;
		case skill::type::crush:
			if (ch->off_flags.has(OFF_CRUSH))						skill = ch->level; break;
		case skill::type::disarm:
		    if (ch->off_flags.has(OFF_DISARM)
		     || ch->act_flags.has_any_of(ACT_WARRIOR|ACT_THIEF))	skill = 20 + (ch->level * 2 / 3); break;
		case skill::type::berserk:
			if (ch->off_flags.has(OFF_BERSERK))						skill = 3 * ch->level; break;
		case skill::type::backstab:
			if (ch->act_flags.has(ACT_THIEF))						skill = 20 + (ch->level * 2); break;
		case skill::type::shield_block:								skill = 15 + ch->level; break;
		case skill::type::rescue:									skill = 40 + (ch->level / 2); break;
		case skill::type::recall:									skill = 40 + ch->level; break;
		case skill::type::scrolls:									skill = ch->level; break;
		case skill::type::hunt:										skill = 75; break;
		case skill::type::sword:
		case skill::type::dagger:
		case skill::type::spear:
		case skill::type::mace:
		case skill::type::axe:
		case skill::type::flail:
		case skill::type::whip:
		case skill::type::polearm:									skill = 40 + (5 * ch->level / 2); break;
		default:													skill = 0; break;
		}
	}

	if (ch->daze > 0) {
		if (skill::lookup(sn).spell_fun != spell_null)
			skill /= 2;
		else
			skill = 2 * skill / 3;
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10)
		skill = 9 * skill / 10;

	return URANGE(0, skill, 100);
}

void set_evolution(Character *ch, skill::type sn, int value) {
	if (IS_NPC(ch) || sn == skill::type::unknown)
		return;

	ch->pcdata->evolution[(int)sn] = URANGE(1, value, 4);
}

int get_evolution(const Character *ch, skill::type type)
{
	int evolution;

	if (IS_NPC(ch))
		evolution = 1;
	else
		evolution = URANGE(1, ch->pcdata->evolution[(int)type], 4);

	return evolution;
} /* end get_evolution */

int can_evolve(Character *ch, skill::type type)
{
	/* returns 1 if evolvable, 0 if already at max, -1 if not evolvable */
	if (IS_IMMORTAL(ch)) {
		int i, tcost = 0;

		for (i = 0; i < 8; i++) {
			tcost += skill::lookup(type).evocost_sec[i];
			tcost += skill::lookup(type).evocost_pri[i];
		}

		if (!tcost)
			return -1;
		else if (get_evolution(ch, type) >= 4)
			return 0;
		else
			return 1;
	}

	if (skill::lookup(type).evocost_sec[ch->cls] <= 0)
		return -1;

	if (get_learned(ch, type) <= 0)
		return 1;

	/* skill is evolvable */
	if (skill::lookup(type).evocost_pri[ch->cls] <= 0) {
		if (get_evolution(ch, type) >= 2)
			return 0;
		else
			return 1;
	}
	else {
		if (get_evolution(ch, type) >= 3)
			return 0;
		else
			return 1;
	}

	return -1;      /* just in case */
}

void evolve_list(Character *ch)
{
	String buffer;
	int can;
	buffer += "Currently evolvable skills and spells:\n\n";
	buffer += "{GSkill or spell      {C| {GPct {C| {GEvo {C| {GNext{x\n";
	buffer += "{C--------------------+-----+-----+-----{x\n";

	for (const auto& pair : skill_table) {
		skill::type type = pair.first;
		const auto& entry = pair.second;

		if (type == skill::type::unknown)
			continue;

		if ((can = can_evolve(ch, type)) == -1)
			continue;

		if (get_learned(ch, type) < 1) {
			buffer += Format::format("{H%-20s{C|   {H0 {C|   {H0 {C| {H%4d{x\n",
			    entry.name,
			    entry.evocost_sec[ch->cls]);
			continue;
		}

		buffer += Format::format("{G%-20s{C| {G%3d {C|   {G%d {C| {G",
		    entry.name,
		    get_learned(ch, type),
		    get_evolution(ch, type));

		if (can == 1)
			buffer += Format::format("%4d{x\n",
			    get_evolution(ch, type) == 1 ?
			    entry.evocost_sec[ch->cls] :
			    entry.evocost_pri[ch->cls]);
		else
			buffer += "----{x\n";
	}

	buffer += Format::format("\nYou have %d skill points to spend.\n", ch->pcdata->skillpoints);
	page_to_char(buffer, ch);
}

void evolve_info(Character *ch)
{
	String buffer;
	int evo_sum[8] = {0};

	buffer += "Currently evolvable skills and spells:\n\n";
	buffer += "{GSkill or spell      {C|{G";

	for (int cls = 0; cls < 8; cls++) {
		buffer += class_table[cls].who_name;
		buffer += "{C|{G";
	}

	buffer += "\n";

	buffer += "{C--------------------+---+---+---+---+---+---+---+---+{x\n";

	for (const auto& pair : skill_table) {
		skill::type type = pair.first;
		const auto& entry = pair.second;

		if (type == skill::type::unknown)
			continue;

		int max_evo[8] = {0};
		bool should_show = FALSE;

		for (int cls = 0; cls < 8; cls++) {
			if (entry.evocost_sec[cls] > 0) {
				max_evo[cls]++;
				should_show = TRUE;
			}

			if (entry.evocost_pri[cls] > 0)
				max_evo[cls]++;
		}

		if (!should_show)
			continue;

		buffer += Format::format("{H%-20s{C|", entry.name);

		for (int cls = 0; cls < 8; cls++) {
			if (max_evo[cls] == 0)
				buffer += "   |";
			else {
				buffer += Format::format("{%s %d {C|",
					max_evo[cls] > 1 ? "Y" : "G",
					max_evo[cls]+1);

				evo_sum[cls] += max_evo[cls]+1;
			}
		}

		buffer += "\n";

	}

	buffer += "{C--------------------+---+---+---+---+---+---+---+---+{x\n";

	buffer += Format::format("{H%-20s{C|", "sum");

	for (int cls = 0; cls < 8; cls++)
		buffer += Format::format("{G%3d{C|", evo_sum[cls]);

	buffer += "\n";
	page_to_char(buffer, ch);
}

void do_evolve(Character *ch, String argument)
{
	String which;
	int cost, can, perc;
	int special = 0;

	if (IS_NPC(ch)) {
		stc("Sorry, your skills are limited.\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		evolve_list(ch);
		return;
	}

	if (IS_IMMORTAL(ch) && arg == "info") {
		evolve_info(ch);
		return;
	}

	skill::type type = skill::lookup(arg);

	if (type == skill::type::unknown) {
		stc("No skill or spell by that name exists.\n", ch);
		return;
	}

	const auto& entry = skill::lookup(type);

	if (entry.spell_fun == spell_null)
		which = "skill";
	else
		which = "spell";

	if (get_learned(ch, type) < 1 && special < 1) {
		ptc(ch, "You have no knowledge of that %s.", which);
		return;
	}

	can = can_evolve(ch, type);

	if (can == -1) {
		ptc(ch, "That %s is not evolvable.  Type 'evolve' by itself for a list\n"
		    "of skills and spells that can be evolved by your class.\n", which);
		return;
	}

	if (can == 0) {
		ptc(ch, "You cannot evolve that %s any higher.\n", which);
		return;
	}

	if (special == 1) {
		cost = entry.evocost_sec[ch->cls];
		perc = 0;
	}
	else if (IS_IMMORTAL(ch) && special == 0) {
		cost = 0;
		perc = 1;
	}
	else if (get_evolution(ch, type) == 1 && special == 0) {
		cost = entry.evocost_sec[ch->cls];
		perc = 85;
	}
	else if (get_evolution(ch, type) == 1 && special == 1) {
		cost = entry.evocost_pri[ch->cls];
		perc = 0;
	}
	else {
		cost = entry.evocost_pri[ch->cls];
		perc = 95;
	}

	if (ch->pcdata->skillpoints < cost) {
		ptc(ch, "You have not earned enough skill points to evolve that %s furthur.\n", which);
		return;
	}

	if (get_learned(ch, type) < perc && special < 1) {
		ptc(ch, "You only have that %s at %d%%, you need %d%% to evolve.\n",
		    which, get_learned(ch, type), perc);
		return;
	}

	/* good to go */
	ch->pcdata->skillpoints -= cost;
	set_evolution(ch, type, get_evolution(ch, type)+1);

	if (!IS_IMMORTAL(ch))
		set_learned(ch, type, 1);

	ptc(ch, "Insight dawns on you as you envision new ways to use %s.\n", entry.name);
}

int get_skill_cost(Character *ch, skill::type type)
{
	int cost = skill::lookup(type).min_mana;

	/*      if (!skill::lookup(type).min_mana)
	                return 0;

	        if (ch->level + 2 == skill::lookup(type).skill_level[ch->cls])
	                cost = 50;
	        else
	                cost = UMAX(skill::lookup(type).min_mana,
	                        100 / (2 + ch->level - skill::lookup(type).skill_level[ch->cls]));

	        return cost; */

	if (skill::lookup(type).spell_fun == spell_null) {
		if (/*skill::lookup(type).target == TAR_CHAR_OFFENSIVE && */ch->level <= 50) {
			int pct_max, level = skill::lookup(type).skill_level[ch->cls];
			pct_max = 100 * (ch->level - level) / UMAX(50 - level, 1);
			cost = (cost / 2) + (((cost / 2) * pct_max) / 100);
		}
	}
	else if (!IS_NPC(ch)) {
		/* remort affect - costly spells */
		if (HAS_RAFF(ch, RAFF_COSTLYSPELLS))
			cost += cost / 5;

		/* remort affect - cheap spells */
		if (HAS_RAFF(ch, RAFF_CHEAPSPELLS))
			cost -= cost / 5;
	}

	return cost;
}

bool deduct_stamina(Character *ch, skill::type type)
{
	if (skill::lookup(type).spell_fun != spell_null)
		return FALSE;

	if (skill::lookup(type).min_mana <= 0)
		return TRUE;

	int stam_cost = get_skill_cost(ch, type);
	
	/*suffix
	 *placeholder for stamina cost modifying suffixes
	 *
	 *of The Swallow			2% stamina cost reduction
	 *of The Hawk				4% stamina cost reduction
	 *of The Falcon				6% stamina cost reduction
	 
	 replace section between snips with the following:
	*/
	stam_cost -= stam_cost * GET_ATTR(ch, APPLY_STAM_COST_PCT) / 100;

	if (ch->stam < stam_cost) {
		ptc(ch, "You are too tired to %s.\n", skill::lookup(type).name);
		return FALSE;
	}
	
	ch->stam -= stam_cost;
	return TRUE;
}

/* used to get new skills */
void do_gain(Character *ch, String argument)
{
	Character *trainer;

	if (IS_NPC(ch)) {
		stc("Trust me, you know all you need.\n", ch);
		return;
	}

	/* find a trainer */
	for (trainer = ch->in_room->people; trainer; trainer = trainer->next_in_room)
		if (IS_NPC(trainer) && trainer->act_flags.has(ACT_GAIN) && can_see_char(ch, trainer))
			break;

	if (trainer == nullptr) {
		stc("You can't do that here.\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		stc("Syntax:\n"
		    "gain list\n"
		    "gain convert\n"
		    "gain revert\n"
		    "gain points\n"
		    "gain <skill or spell group>\n", ch);
		return;
	}

	if (arg.is_prefix_of("list")) {
		int col = 0;
		String output;
		bool foundsect = FALSE, foundall = FALSE;
		output += Format::format("%-18s %-5s %-18s %-5s %-18s %-5s\n",
		    "group", "cost", "group", "cost", "group", "cost");

		for (unsigned int gn = 0; gn < group_table.size(); gn++) {
			if (completed_group(ch, gn)
			    || group_table[gn].rating[ch->cls] <= 0)
				continue;

			foundsect = TRUE;
			output += Format::format("%-18s %s%-5d{x ",
			    group_table[gn].name,
			    group_table[gn].rating[ch->cls] > ch->train ? "{T" : "{C",
			    group_table[gn].rating[ch->cls]);

			if (++col % 3 == 0)
				output += "\n";
		}

		if (col % 3 != 0)
			output += "\n";

		output += "\n";

		if (foundsect) {
			page_to_char(output, ch);
			output.erase();
			foundall = TRUE;
		}

		output += Format::format("%-18s %-5s %-18s %-5s %-18s %-5s\n",
		    "skill", "cost", "skill", "cost", "skill", "cost");

		col = 0;
		foundsect = FALSE;

		for (const auto& pair : skill_table) {
			skill::type type = pair.first;
			const auto& entry = pair.second;

			if (type == skill::type::unknown)
				continue;

			if (get_learned(ch, type) == 0
			    && entry.rating[ch->cls] > 0
			    && entry.spell_fun == spell_null
			    && entry.remort_class == 0) {
				foundsect = TRUE;
				output += Format::format("%s%-18s %s%-5d{x ",
				    entry.skill_level[ch->cls] > ch->level ? "{c" : "{g",
				    entry.name,
				    entry.rating[ch->cls] > ch->train ? "{T" : "{C",
				    entry.rating[ch->cls]);

				if (++col % 3 == 0)
					output += "\n";
			}
		}

		if (col % 3 != 0)
			output += "\n";

		if (foundsect) {
			page_to_char(output, ch);
			output.erase();
			foundall = TRUE;
		}


		if (ch->pcdata->remort_count < 1) {
			if (!foundall)
				output += "You have nothing left to learn until you remort.\n";

			output += Format::format("You have %d train%s left.\n",
			    ch->train, ch->train == 1 ? "" : "s");
			page_to_char(output, ch);
			return;
		}

		if (foundall)
			output += "\n";

		output += Format::format("Remort specials for class %s\n",
		    class_table[ch->cls].name);
		output += "---------------\n";

		col = 0;
		foundsect = FALSE;

		for (const auto& pair : skill_table) {
			skill::type type = pair.first;
			const auto& entry = pair.second;

			if (type == skill::type::unknown)
				continue;

			if (get_learned(ch, type) == 0
			    && entry.rating[ch->cls] > 0
			    && entry.remort_class > 0
			    && entry.remort_class == ch->cls + 1) {
				foundsect = TRUE;
				output += Format::format("%s%-18s %s%-5d{x ",
				    entry.skill_level[ch->cls] > ch->level ? "{c" : "{g",
				    entry.name,
				    entry.rating[ch->cls] > ch->train ? "{T" : "{C",
				    entry.rating[ch->cls]);

				if (++col % 3 == 0)
					output += "\n";
			}
		}

		if (col % 3 != 0)
			output += "\n";

		if (foundsect) {
			page_to_char(output, ch);
			output.erase();
			foundall = TRUE;
		}


		if (!foundall)
			output += "You have nothing left to learn!\n";

		output += Format::format("You have %d train%s left.\n",
		    ch->train, ch->train == 1 ? "" : "s");
		page_to_char(output, ch);
		return;
	}

	if (arg.is_prefix_of("convert")) {
		if (ch->practice < 10) {
			act("$N tells you 'You are not yet ready.'", ch, nullptr, trainer, TO_CHAR);
			return;
		}

		act("$N helps you apply your practice to training.", ch, nullptr, trainer, TO_CHAR);
		ch->practice -= 10;
		ch->train++;
		return;
	}

	if (arg.is_prefix_of("revert")) {
		if (ch->train < 1) {
			act("$N tells you 'You are not yet ready.'", ch, nullptr, trainer, TO_CHAR);
			return;
		}

		act("$N helps you apply your training to practices.", ch, nullptr, trainer, TO_CHAR);
		ch->practice += 10;
		ch->train--;
		return;
	}

	if (arg.is_prefix_of("points")) {
		int mod;

		if (ch->train < 1) {
			act("$N tells you 'You are not yet ready.'", ch, nullptr, trainer, TO_CHAR);
			return;
		}

		if (ch->pcdata->points <= 40) {
			act("$N tells you 'There would be no point in that.'", ch, nullptr, trainer, TO_CHAR);
			return;
		}

		act("$N trains you, and you feel more at ease with your skills.", ch, nullptr, trainer, TO_CHAR);
		mod = ch->exp - (exp_per_level(ch, ch->pcdata->points) * ch->level);
		ch->train--;
		ch->pcdata->points--;
		ch->exp = UMIN(exp_per_level(ch, ch->pcdata->points) * ch->level + mod,
		               exp_per_level(ch, ch->pcdata->points) * (ch->level + 1) - 1);
		return;
	}

	/* else add a group/skill */
	int gn = group_lookup(argument);

	if (gn >= 0) {
		if (ch->pcdata->group_known[gn]) {
			act("$N tells you 'You already know that group!'", ch, nullptr, trainer, TO_CHAR);
			return;
		}

		if (group_table[gn].rating[ch->cls] <= 0) {
			act("$N tells you 'That group is beyond your powers.'", ch, nullptr, trainer, TO_CHAR);
			return;
		}

		if (ch->train < group_table[gn].rating[ch->cls]) {
			act("$N tells you 'You are not yet ready for that group.'", ch, nullptr, trainer, TO_CHAR);
			return;
		}

		/* add the group */
		gn_add(ch, gn);
		act("$N trains you in the art of $t.", ch, group_table[gn].name, trainer, TO_CHAR);
		ch->train -= group_table[gn].rating[ch->cls];
		return;
	}

	skill::type type = skill::lookup(argument);

	if (type > skill::type::unknown) {
		if (skill::lookup(type).spell_fun != spell_null
		    && skill::lookup(type).remort_class == 0) {
			act("$N tells you 'You must learn the full group.'", ch, nullptr, trainer, TO_CHAR);
			return;
		}

		if (skill::lookup(type).remort_class > 0) {
			if (ch->pcdata->remort_count < 1) {
				act("$N tells you 'That skill is available only to remorts.'",
				    ch, nullptr, trainer, TO_CHAR);
				return;
			}

			if (ch->cls + 1 != skill::lookup(type).remort_class) {
				act("$N tells you 'Use extraclass for skills and spells outside your class.'",
				    ch, nullptr, trainer, TO_CHAR);
				return;
			}
		}

		if (get_learned(ch, type) > 0) {
			act("$N tells you 'You already know that skill!'", ch, nullptr, trainer, TO_CHAR);
			return;
		}

		if (skill::lookup(type).rating[ch->cls] <= 0) {
			act("$N tells you 'That skill is beyond your powers.'", ch, nullptr, trainer, TO_CHAR);
			return;
		}

		if (ch->train < skill::lookup(type).rating[ch->cls]) {
			act("$N tells you 'You are not yet ready for that skill.'", ch, nullptr, trainer, TO_CHAR);
			return;
		}

		/* add the skill */
		set_learned(ch, type, 1);
		set_evolution(ch, type, 1);
		act("$N trains you in the art of $t.", ch, skill::lookup(type).name, trainer, TO_CHAR);
		ch->train -= skill::lookup(type).rating[ch->cls];
		return;
	}

	act("$N tells you 'I have no knowledge of that skill or spell group.'", ch, nullptr, trainer, TO_CHAR);
}

/* This function converts skill points into practice sessions.
10 skill points is required to make one practice session.

The syntax for this command is
convert <skill_points>
For example: "convert 20" will change 20 SP into 2 practices.

-- Outsider
*/
void do_convert(Character *ch, String argument)
{
	int use_points;
	char buffer[MAX_INPUT_LENGTH];

	if (argument.empty()) {
		stc("Convert: Change <X> number of skill points into practice sessions.\n", ch);
		stc("Syntax: convert <skill_points>\n", ch);
		return;
	}

	if (!argument.is_number()) {
		stc("Please provide a valid number of skill points to convert.\n", ch);
		return;
	}

	use_points = atoi(argument);

	if ((use_points < 10) || (use_points > 1000)) {
		stc("Please use a number in the range of 10-1000. Only multiples of 10 are accepted, rounded down.\n", ch);
		return;
	}

	/* round off to the nearest multiple of 10 */
	while (use_points % 10)
		use_points--;

    if (ch->pcdata->skillpoints < use_points) {
        stc("You have insufficient skill points.\n", ch);
        return;
    }

	/* convert skill points into practices */
	ch->pcdata->skillpoints -= use_points;
	ch->practice += use_points / 10;
	Format::sprintf(buffer, "You now have %d skill points and %d practice sessions.\n",
	        ch->pcdata->skillpoints, ch->practice);
	stc(buffer, ch);
}
