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
#include "macros.hh"
#include "magic.hh"
#include "memory.hh"
#include "merc.hh"
#include "Player.hh"
#include "random.hh"
#include "RoomPrototype.hh"
#include "String.hh"

/* return TRUE if a player either has the group or has all skills in it */
bool completed_group(Character *ch, int gn)
{
	int i, ngn;

	if (ch->pcdata->group_known[gn])
		return TRUE;

	for (i = 0; i < group_table[gn].spells.size(); i++) {
		if ((ngn = group_lookup(group_table[gn].spells[i])) >= 0) {
			if (group_table[ngn].rating[ch->cls] <= 0)
				continue;

			if (!completed_group(ch, ngn))
				return FALSE;

			continue;
		}

		if (ch->pcdata->learned[skill_lookup(group_table[gn].spells[i])] <= 0)
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
	int sn;
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

	return strcmp(skill_table[si1->sn].name, skill_table[si2->sn].name);
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
	int gn, sn;
	int j;
	int group_list[group_table.size()];
	int ngroups = 0;
	struct s_spell_info spell_list[skill_table.size()];
	int nspells = 0;
	String arg, buf;
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
	for (gn = group_lookup("attack"); gn < group_table.size(); gn++) {
		group_list[ngroups++] = gn;
	}

	/* sort groups by name */
	qsort(group_list, ngroups, sizeof(int), comp_groupnames);

	/* SPELLS GROUPS: List spell groups, sorted, indicate which are learned */
	if (argument.is_prefix_of("groups")) {
		stc("Spell groups (* = gained):\n"
		    "--------------------------\n", ch);
		buf[0] = '\0';

		for (j = 0; j < ngroups; j++) {
			gn = group_list[j];

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
		gn = group_list[j];

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
		for (j = 0; j < group_table[group].spells.size(); j++) {
			sn = skill_lookup(group_table[group].spells[j]);

			if (sn == -1)
				continue;

			level = skill_table[sn].skill_level[ch->cls];

			if (level < min_level || max_level < level)
				continue;

			spell_list[nspells].sn = sn;
			spell_list[nspells].level = level;
			nspells++;
			found = TRUE;
		}
	}
	else {
		/* select spells from player's repertoire or by name */
		for (sn = 0; sn < skill_table.size(); sn++) {
			level = skill_table[sn].skill_level[ch->cls];

			if (level < min_level
			    || max_level < level
			    || skill_table[sn].spell_fun == spell_null)
				continue;

			if (spell_name.empty()) {
				if (ch->pcdata->learned[sn] <= 0
				    || (skill_table[sn].remort_class != 0 && !IS_IMMORTAL(ch)
				        && ch->cls + 1 != skill_table[sn].remort_class && !HAS_EXTRACLASS(ch, sn)))
					continue;
			}
			else if (!skill_table[sn].name.has_words(spell_name))
				continue;

			found = TRUE;
			spell_list[nspells].sn = sn;
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
		Format::sprintf(buf, "Spells in group '%s':\n\n", group_table[group].name);
		buffer += buf;
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

		sn = spell_list[j].sn;

		if (ch->pcdata->learned[sn] <= 0
		    || (skill_table[sn].remort_class != 0 && !IS_IMMORTAL(ch)
		        && ch->cls + 1 != skill_table[sn].remort_class && !HAS_EXTRACLASS(ch, sn)))
			Format::sprintf(buf, "[not  gained] ");
		else if (ch->level < level)
			Format::sprintf(buf, "[           ] ");
		else
			Format::sprintf(buf, "[{V%3d%% %3d Ma{x] ", ch->pcdata->learned[sn], get_skill_cost(ch, sn));

		Format::sprintf(arg, "{C%-1.20s{x", skill_table[sn].name);
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
	int sn;
	int j;
	struct s_spell_info skill_list[skill_table.size()];
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
	for (sn = 0; sn < skill_table.size(); sn++) {
		level = skill_table[sn].skill_level[ch->cls];

		if (level < min_level
		    || max_level < level
		    || skill_table[sn].spell_fun != spell_null)
			continue;

		if (skill_name.empty()) {
			if (ch->pcdata->learned[sn] <= 0
			    || (skill_table[sn].remort_class != 0 && !IS_IMMORTAL(ch)
			        && ch->cls + 1 != skill_table[sn].remort_class && !HAS_EXTRACLASS(ch, sn)))
				continue;
		}
		else if (!skill_table[sn].name.has_words(skill_name))
			continue;

		found = TRUE;
		skill_list[nskills].sn = sn;
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

		sn = skill_list[j].sn;

		if (ch->pcdata->learned[sn] <= 0
		    || (skill_table[sn].remort_class != 0 && !IS_IMMORTAL(ch)
		        && ch->cls + 1 != skill_table[sn].remort_class && !HAS_EXTRACLASS(ch, sn)))
			Format::sprintf(buf, "[not  gained] ");
		else if (ch->level < level)
			Format::sprintf(buf, "[           ] ");
		else
			Format::sprintf(buf, "[{G%3d%% %3d St{x] ", ch->pcdata->learned[sn], get_skill_cost(ch, sn));

		Format::sprintf(arg, "{H%-1.20s{x", skill_table[sn].name);
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
	char columns[LEVEL_HERO];
	int sn, lev, x, y;
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

			for (sn = 0; sn < skill_table.size(); sn++) {
				if (skill_table[sn].remort_class < 0) /* for completely ungainable spells/skills */
					continue;

				if (skill_table[sn].remort_class > 0) /* list of remort skills is through 'levels remort' */
					continue;

				Format::sprintf(buf, "%-21s", skill_table[sn].name);
				buffer += buf;

				for (x = 0; x < MAX_CLASS; x++) {
					if (skill_table[sn].skill_level[x] < 0 || skill_table[sn].skill_level[x] > LEVEL_HERO)
						Format::sprintf(buf, "{c  NA{x");
					else
						Format::sprintf(buf, "%4d", skill_table[sn].skill_level[x]);

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

			for (sn = 0; sn < skill_table.size(); sn++) {
				if (skill_table[sn].remort_class != x + 1)
					continue;

				Format::sprintf(buf, "  {g%-19s{x", skill_table[sn].name);
				buffer += buf;

				for (y = 0; y < MAX_CLASS; y++) {
					if (skill_table[sn].skill_level[y] < 0 || skill_table[sn].skill_level[y] > LEVEL_HERO)
						Format::sprintf(buf, " {TNA{c/{HNA {x");
					else
						Format::sprintf(buf, "{C%3d{c/{G%-3d{x", skill_table[sn].skill_level[y], skill_table[sn].rating[y]);

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

	/* initialize data */
	for (lev = 0; lev < LEVEL_HERO; lev++) {
		columns[lev] = 0;
		list[lev][0] = '\0';
	}


	for (sn = 0; sn < skill_table.size(); sn++) {
		if (skill_table[sn].skill_level[cls] < 0)
			continue;

		if (skill_table[sn].skill_level[cls] < LEVEL_HERO &&
		    skill_table[sn].spell_fun != spell_null) {
			lev = skill_table[sn].skill_level[cls];
			Format::sprintf(buf, "{G%-20s  {C%3d{x {Tmana{x  ", skill_table[sn].name,
			        skill_table[sn].min_mana);

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
		if (list[lev][0] != '\0')
			buffer += list[lev];

	buffer += "\n";

	/* Initialize the data */
	for (lev = 0; lev < LEVEL_HERO; lev++) {
		columns[lev] = 0;
		list[lev][0] = '\0';
	}

	for (sn = 0; sn < skill_table.size(); sn++) {
		if (skill_table[sn].skill_level[ch->cls] < 0)
			continue;

		if (skill_table[sn].skill_level[cls] < LEVEL_HERO &&
		    skill_table[sn].spell_fun == spell_null) {
			lev = skill_table[sn].skill_level[cls];
			Format::sprintf(buf, "{G%-20s   {x        ", skill_table[sn].name);

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
		if (list[lev][0] != '\0')
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
	int gn, sn, col;

	if (IS_NPC(ch))
		return;

	col = 0;

	if (argument.empty()) {
		/* show all groups */
		for (gn = 0; gn < group_table.size(); gn++) {
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
		for (gn = 0; gn < group_table.size(); gn++) {
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
	gn = group_lookup(argument);

	if (gn == -1) {
		stc("No group of that name exist.\n", ch);
		stc(
		        "Type 'groups all' or 'info all' for a full listing.\n", ch);
		return;
	}

	for (sn = 0; sn < group_table[gn].spells.size(); sn++) {
		Format::sprintf(buf, "%-18s ", group_table[gn].spells[sn]);
		stc(buf, ch);

		if (++col % 3 == 0)
			stc("\n", ch);
	}

	if (col % 3 != 0)
		stc("\n", ch);
}

/* checks for skill improvement */
void check_improve(Character *ch, int sn, bool success, int multiplier)
{
	int chance;
	char buf[100];
	int xp;

	if (IS_NPC(ch))
		return;

	if (ch->level < skill_table[sn].skill_level[ch->cls]
	    ||  skill_table[sn].rating[ch->cls] == 0
	    ||  ch->pcdata->learned[sn] == 0
	    ||  ch->pcdata->learned[sn] == 100)
		return;  /* skill is not known */

	/* check to see if the character has a chance to learn */
	chance = 10 * int_app[GET_ATTR_INT(ch)].learn;
	chance /= (multiplier
	           *       skill_table[sn].rating[ch->cls]
	           *       4);
	chance += ch->level;

	if (skill_table[sn].remort_class > 0)
	{   chance *= 2; chance /= 3; }

	/* -1 multiplier means auto success -- Montrey */
	if (number_range(1, 1000) > chance && multiplier != -1)
		return;

	/* pre-calculate the experience gain */
	xp = 2 * skill_table[sn].rating[ch->cls];
	/*
	if ( ch->pcdata->remort_count > 1 )
	    xp /= ch->pcdata->remort_count;
	*/

	/* now that the character has a CHANCE to learn, see if they really have */

	if (success) {
		chance = URANGE(5, 100 - ch->pcdata->learned[sn], 95);

		if (number_percent() < chance || multiplier == -1) {
			Format::sprintf(buf, "{GYou have become better at {H%s{G!{x\n",
			        skill_table[sn].name);
			stc(buf, ch);
			ch->pcdata->learned[sn]++;
			gain_exp(ch, xp);
		}
	}
	else {
		chance = URANGE(5, ch->pcdata->learned[sn] / 2, 30);

		if (number_percent() < chance || multiplier == -1) {
			Format::sprintf(buf,
			        "{GYou learn from your mistakes, and your {H%s {Gskill improves.{x\n",
			        skill_table[sn].name);
			stc(buf, ch);
			ch->pcdata->learned[sn] += number_range(1, 3);
			ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn], 100);
			gain_exp(ch, xp);
		}
	}
} /* end check_improve() */

/* returns a group index number given the name */
int group_lookup(const String& name)
{
	int gn;

	for (gn = 0; gn < group_table.size(); gn++)
		if (name == group_table[gn].name)
			return gn;

	return -1;
}

/* recursively adds a group given its number -- uses group_add */
void gn_add(Character *ch, int gn)
{
	int i;
	ch->pcdata->group_known[gn] = TRUE;

	for (i = 0; i < group_table[gn].spells.size(); i++) {
		group_add(ch, group_table[gn].spells[i], FALSE);
	}
}

/* recusively removes a group given its number -- uses group_remove */
void gn_remove(Character *ch, int gn)
{
	int i;
	ch->pcdata->group_known[gn] = FALSE;

	for (i = 0; i < group_table[gn].spells.size(); i ++) {
		group_remove(ch, group_table[gn].spells[i]);
	}
}

/* use for processing a skill or group for addition  */
void group_add(Character *ch, const String& name, bool deduct)
{
	int sn, gn;

	if (IS_NPC(ch)) /* NPCs do not have skills */
		return;

	sn = skill_lookup(name);

	if (sn != -1) {
		if (ch->pcdata->learned[sn] == 0) { /* i.e. not known */
			ch->pcdata->learned[sn] = 1;
			ch->pcdata->evolution[sn] = 1;

			if (deduct)
				ch->pcdata->points += skill_table[sn].rating[ch->cls];
		}

		return;
	}

	/* now check groups */
	gn = group_lookup(name);

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
	int sn, gn;
	sn = skill_lookup(name);

	if (sn != -1) {
		ch->pcdata->learned[sn] = 0;
		ch->pcdata->evolution[sn] = 0;
		return;
	}

	/* now check groups */
	gn = group_lookup(name);

	if (gn != -1 && ch->pcdata->group_known[gn] == TRUE) {
		ch->pcdata->group_known[gn] = FALSE;
		gn_remove(ch, gn); /* be sure to call gn_add on all remaining groups */
	}
}

int get_evolution(Character *ch, int sn)
{
	int evolution;

	if (IS_NPC(ch))
		evolution = 1;
	else
		evolution = URANGE(1, ch->pcdata->evolution[sn], 4);

	return evolution;
} /* end get_evolution */

int can_evolve(Character *ch, int sn)
{
	/* returns 1 if evolvable, 0 if already at max, -1 if not evolvable */
	if (IS_IMMORTAL(ch)) {
		int i, tcost = 0;

		for (i = 0; i < 8; i++) {
			tcost += skill_table[sn].evocost_sec[i];
			tcost += skill_table[sn].evocost_pri[i];
		}

		if (!tcost)
			return -1;
		else if (ch->pcdata->evolution[sn] >= 4)
			return 0;
		else
			return 1;
	}

	if (skill_table[sn].evocost_sec[ch->cls] <= 0)
		return -1;

	if (ch->pcdata->learned[sn] <= 0)
		return 1;

	/* skill is evolvable */
	if (skill_table[sn].evocost_pri[ch->cls] <= 0) {
		if (ch->pcdata->evolution[sn] >= 2)
			return 0;
		else
			return 1;
	}
	else {
		if (ch->pcdata->evolution[sn] >= 3)
			return 0;
		else
			return 1;
	}

	return -1;      /* just in case */
}

void evolve_list(Character *ch)
{
	String buffer;
	int x, can;
	buffer += "Currently evolvable skills and spells:\n\n";
	buffer += "{GSkill or spell      {C| {GPct {C| {GEvo {C| {GNext{x\n";
	buffer += "{C--------------------+-----+-----+-----{x\n";

	for (x = 0; x < skill_table.size(); x++) {
		if ((can = can_evolve(ch, x)) == -1)
			continue;

		if (ch->pcdata->learned[x] < 1) {
			buffer += Format::format("{H%-20s{C|   {H0 {C|   {H0 {C| {H%4d{x\n",
			    skill_table[x].name,
			    skill_table[x].evocost_sec[ch->cls]);
			continue;
		}

		buffer += Format::format("{G%-20s{C| {G%3d {C|   {G%d {C| {G",
		    skill_table[x].name,
		    ch->pcdata->learned[x],
		    ch->pcdata->evolution[x]);

		if (can == 1)
			buffer += Format::format("%4d{x\n",
			    ch->pcdata->evolution[x] == 1 ?
			    skill_table[x].evocost_sec[ch->cls] :
			    skill_table[x].evocost_pri[ch->cls]);
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

	for (int sn = 0; sn < skill_table.size(); sn++) {
		int max_evo[8] = {0};
		bool should_show = FALSE;

		for (int cls = 0; cls < 8; cls++) {
			if (skill_table[sn].evocost_sec[cls] > 0) {
				max_evo[cls]++;
				should_show = TRUE;
			}

			if (skill_table[sn].evocost_pri[cls] > 0)
				max_evo[cls]++;
		}

		if (!should_show)
			continue;

		buffer += Format::format("{H%-20s{C|", skill_table[sn].name);

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
	char *type;
	int sn, cost, can, perc;
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

	if ((sn = skill_lookup(arg)) == -1) {
		stc("No skill or spell by that name exists.\n", ch);
		return;
	}

	if (skill_table[sn].spell_fun == spell_null)
		type = "skill";
	else
		type = "spell";

	if (ch->pcdata->learned[sn] < 1 && special < 1) {
		ptc(ch, "You have no knowledge of that %s.", type);
		return;
	}

	can = can_evolve(ch, sn);

	if (can == -1) {
		ptc(ch, "That %s is not evolvable.  Type 'evolve' by itself for a list\n"
		    "of skills and spells that can be evolved by your class.\n", type);
		return;
	}

	if (can == 0) {
		ptc(ch, "You cannot evolve that %s any higher.\n", type);
		return;
	}

	if (special == 1) {
		cost = skill_table[sn].evocost_sec[ch->cls];
		perc = 0;
	}
	else if (IS_IMMORTAL(ch) && special == 0) {
		cost = 0;
		perc = 1;
	}
	else if (ch->pcdata->evolution[sn] == 1 && special == 0) {
		cost = skill_table[sn].evocost_sec[ch->cls];
		perc = 85;
	}
	else if (ch->pcdata->evolution[sn] == 1 && special == 1) {
		cost = skill_table[sn].evocost_pri[ch->cls];
		perc = 0;
	}
	else {
		cost = skill_table[sn].evocost_pri[ch->cls];
		perc = 95;
	}

	if (ch->pcdata->skillpoints < cost) {
		ptc(ch, "You have not earned enough skill points to evolve that %s furthur.\n", type);
		return;
	}

	if (ch->pcdata->learned[sn] < perc && special < 1) {
		ptc(ch, "You only have that %s at %d%%, you need %d%% to evolve.\n",
		    type, ch->pcdata->learned[sn], perc);
		return;
	}

	/* good to go */
	ch->pcdata->skillpoints -= cost;
	ch->pcdata->evolution[sn]++;

	if (!IS_IMMORTAL(ch))
		ch->pcdata->learned[sn] = 1;

	ptc(ch, "Insight dawns on you as you envision new ways to use %s.\n", skill_table[sn].name);
}

int get_skill_cost(Character *ch, int sn)
{
	int cost = skill_table[sn].min_mana;

	/*      if (!skill_table[sn].min_mana)
	                return 0;

	        if (ch->level + 2 == skill_table[sn].skill_level[ch->cls])
	                cost = 50;
	        else
	                cost = UMAX(skill_table[sn].min_mana,
	                        100 / (2 + ch->level - skill_table[sn].skill_level[ch->cls]));

	        return cost; */

	if (skill_table[sn].spell_fun == spell_null) {
		if (/*skill_table[sn].target == TAR_CHAR_OFFENSIVE && */ch->level <= 50) {
			int pct_max, level = skill_table[sn].skill_level[ch->cls];
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

bool deduct_stamina(Character *ch, int sn)
{
	if (skill_table[sn].spell_fun != spell_null)
		return FALSE;

	if (skill_table[sn].min_mana <= 0)
		return TRUE;

	if (ch->stam < get_skill_cost(ch, sn)) {
		ptc(ch, "You are too tired to %s.\n", skill_table[sn].name);
		return FALSE;
	}

	ch->stam -= get_skill_cost(ch, sn);
	return TRUE;
}

/* used to get new skills */
void do_gain(Character *ch, String argument)
{
	Character *trainer;
	int gn = 0, sn = 0;

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

		for (gn = 0; gn < group_table.size(); gn++) {
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
			foundall = TRUE;
		}

		output += Format::format("%-18s %-5s %-18s %-5s %-18s %-5s\n",
		    "skill", "cost", "skill", "cost", "skill", "cost");

		for (sn = 0, col = 0, foundsect = FALSE; sn < skill_table.size(); sn++) {
			if (!ch->pcdata->learned[sn]
			    && skill_table[sn].rating[ch->cls] > 0
			    && skill_table[sn].spell_fun == spell_null
			    && skill_table[sn].remort_class == 0) {
				foundsect = TRUE;
				output += Format::format("%s%-18s %s%-5d{x ",
				    skill_table[sn].skill_level[ch->cls] > ch->level ? "{c" : "{g",
				    skill_table[sn].name,
				    skill_table[sn].rating[ch->cls] > ch->train ? "{T" : "{C",
				    skill_table[sn].rating[ch->cls]);

				if (++col % 3 == 0)
					output += "\n";
			}
		}

		if (col % 3 != 0)
			output += "\n";

		if (foundsect) {
			page_to_char(output, ch);
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

		for (sn = 0, col = 0, foundsect = FALSE; sn < skill_table.size(); sn++) {
			if (!ch->pcdata->learned[sn]
			    && skill_table[sn].rating[ch->cls] > 0
			    && skill_table[sn].remort_class > 0
			    && skill_table[sn].remort_class == ch->cls + 1) {
				foundsect = TRUE;
				output += Format::format("%s%-18s %s%-5d{x ",
				    skill_table[sn].skill_level[ch->cls] > ch->level ? "{c" : "{g",
				    skill_table[sn].name,
				    skill_table[sn].rating[ch->cls] > ch->train ? "{T" : "{C",
				    skill_table[sn].rating[ch->cls]);

				if (++col % 3 == 0)
					output += "\n";
			}
		}

		if (col % 3 != 0)
			output += "\n";

		if (foundsect) {
			page_to_char(output, ch);
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
	gn = group_lookup(argument);

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

	sn = skill_lookup(argument);

	if (sn > -1) {
		if (skill_table[sn].spell_fun != spell_null
		    && skill_table[sn].remort_class == 0) {
			act("$N tells you 'You must learn the full group.'", ch, nullptr, trainer, TO_CHAR);
			return;
		}

		if (skill_table[sn].remort_class > 0) {
			if (ch->pcdata->remort_count < 1) {
				act("$N tells you 'That skill is available only to remorts.'",
				    ch, nullptr, trainer, TO_CHAR);
				return;
			}

			if (ch->cls + 1 != skill_table[sn].remort_class) {
				act("$N tells you 'Use extraclass for skills and spells outside your class.'",
				    ch, nullptr, trainer, TO_CHAR);
				return;
			}
		}

		if (ch->pcdata->learned[sn]) {
			act("$N tells you 'You already know that skill!'", ch, nullptr, trainer, TO_CHAR);
			return;
		}

		if (skill_table[sn].rating[ch->cls] <= 0) {
			act("$N tells you 'That skill is beyond your powers.'", ch, nullptr, trainer, TO_CHAR);
			return;
		}

		if (ch->train < skill_table[sn].rating[ch->cls]) {
			act("$N tells you 'You are not yet ready for that skill.'", ch, nullptr, trainer, TO_CHAR);
			return;
		}

		/* add the skill */
		ch->pcdata->learned[sn] = 1;
		ch->pcdata->evolution[sn] = 1;
		act("$N trains you in the art of $t.", ch, skill_table[sn].name, trainer, TO_CHAR);
		ch->train -= skill_table[sn].rating[ch->cls];
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
	sh_int use_points;
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
