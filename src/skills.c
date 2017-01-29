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
#include "recycle.h"
#include "magic.h"

/* command procedures needed */
DECLARE_DO_FUN(do_groups);
DECLARE_DO_FUN(do_say);

/* return TRUE if a player either has the group or has all skills in it */
bool completed_group(CHAR_DATA *ch, int gn)
{
	int i, ngn;

	if (ch->pcdata->group_known[gn])
		return TRUE;

	for (i = 0; group_table[gn].spells[i] != NULL; i++) {
		if ((ngn = group_lookup(group_table[gn].spells[i])) >= 0) {
			if (group_table[ngn].rating[ch->class] <= 0)
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
	const struct s_spell_info *si1 = sn1;
	const struct s_spell_info *si2 = sn2;
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
void do_spells(CHAR_DATA *ch, const char *argument)
{
	int level;
	int min_level = 1;
	int max_level = LEVEL_HERO;
	int group = -1;
	int gn, sn;
	int j;
	int group_list[MAX_GROUP];
	int ngroups = 0;
	struct s_spell_info spell_list[MAX_SKILL];
	int nspells = 0;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	int cols = 0;
	int pos = 18;
	char spell_name[MAX_INPUT_LENGTH];
	bool found = FALSE;
	bool new_level = TRUE;
	BUFFER *buffer;
	bool reached_player_level = FALSE;

	if (IS_NPC(ch)) {
		stc("You know all that you need, trust me. ;)\n", ch);
		return;
	}

	/* SPELLS: list all available spells, as in times of olde */
	if (*argument == '\0') {
		do_spells(ch, "1");
		return;
	}

	/* kludge to build a list of only spell groups:
	   spell groups are mixed with skill groups,
	   but currently 'attack' is the first one. */
	for (gn = group_lookup("attack"); gn < MAX_GROUP; gn++) {
		if (group_table[gn].name == NULL)
			break;

		group_list[ngroups++] = gn;
	}

	/* sort groups by name */
	qsort(group_list, ngroups, sizeof(int), comp_groupnames);

	/* SPELLS GROUPS: List spell groups, sorted, indicate which are learned */
	if (!str_prefix1(argument, "groups")) {
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

		if (!str_cmp(argument, group_table[gn].name)) {
			/* discard group name argument */
			argument = one_argument(argument, arg);
			group = gn;
			found = TRUE;
			break;
		}
	}

	/* Check for a spell name */
	spell_name[0] = '\0';
	argument = one_argument(argument, arg);

	if (!found && !is_number(arg)) {
		strcpy(spell_name, arg);
		argument = one_argument(argument, arg);
	}

	/* Check for 1 or 2 numbers. */
	if (is_number(arg)) {
		min_level = atoi(arg);

		if (min_level < 0 || min_level > LEVEL_HERO) {
			stc("That's not a valid minimum level.\n", ch);
			return;
		}

		if (*argument != '\0') {
			argument = one_argument(argument, arg);

			if (is_number(arg)) {
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
		for (j = 0; j < MAX_IN_GROUP; j++) {
			if (group_table[group].spells[j] == NULL)
				break;

			sn = skill_lookup(group_table[group].spells[j]);

			if (sn == -1)
				continue;

			level = skill_table[sn].skill_level[ch->class];

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
		for (sn = 0; sn < MAX_SKILL; sn++) {
			if (skill_table[sn].name == NULL)
				break;

			level = skill_table[sn].skill_level[ch->class];

			if (level < min_level
			    || max_level < level
			    || skill_table[sn].spell_fun == spell_null)
				continue;

			if (spell_name[0] == '\0') {
				if (ch->pcdata->learned[sn] <= 0
				    || (skill_table[sn].remort_class != 0 && !IS_IMMORTAL(ch)
				        && ch->class + 1 != skill_table[sn].remort_class && !HAS_EXTRACLASS(ch, sn)))
					continue;
			}
			else if (!is_name(spell_name, skill_table[sn].name))
				continue;

			found = TRUE;
			spell_list[nspells].sn = sn;
			spell_list[nspells].level = level;
			nspells++;
		}
	}

	if (!found) {
		/* if they did a list of all spells... */
		if (min_level == 1 && max_level == LEVEL_HERO && spell_name[0] == '\0')
			stc("You know no spells.\n", ch);
		else
			stc("No spell group of that name and\n"
			    "no spells with that name and/or level found.\n", ch);

		return;
	}

	buffer = new_buf();

	if (group != -1) {
		sprintf(buf, "Spells in group '%s':\n\n", group_table[group].name);
		add_buf(buffer, buf);
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
				add_buf(buffer, "\n");
				cols = 0;
			}

			if (level > ch->level && !reached_player_level) {
				sprintf(buf, "Level %3d: %s  (your level)\n", ch->level,
				        "-------------------------------");
				add_buf(buffer, buf);
				reached_player_level = TRUE;
			}
		}

		if (cols == 0) {
			sprintf(buf, "Level %3d: ", level);

			if (!new_level)
				sprintf(buf, "%*s", (int)strlen(buf), " ");

			new_level = FALSE;
			add_buf(buffer, buf);
		}
		else {
			sprintf(buf, "%*s", 24 - pos, " ");
			add_buf(buffer, buf);
		}

		sn = spell_list[j].sn;

		if (ch->pcdata->learned[sn] <= 0
		    || (skill_table[sn].remort_class != 0 && !IS_IMMORTAL(ch)
		        && ch->class + 1 != skill_table[sn].remort_class && !HAS_EXTRACLASS(ch, sn)))
			sprintf(buf, "[not  gained] ");
		else if (ch->level < level)
			sprintf(buf, "[           ] ");
		else
			sprintf(buf, "[{V%3d%% %3d Ma{x] ", ch->pcdata->learned[sn], get_skill_cost(ch, sn));

		sprintf(arg, "{C%-1.20s{x", skill_table[sn].name);
		strcat(buf, arg);

		if (++cols < 2)
			pos = strlen(arg);
		else {
			strcat(buf, "\n");
			cols = 0;
		}

		add_buf(buffer, buf);
	}

	if (cols > 0) {
		add_buf(buffer, "\n");
		cols = 0;
	}

	page_to_char(buf_string(buffer), ch);
	free_buf(buffer);
} /* end do_spells() */

/* The guts of 'spells', used here for skill listing - Montrey */
/* Possible arguments:
     SKILLS         -> all spells available to player
     SKILLS 50      -> spells from level 50 up
     SKILLS 50 60   -> spells from lv 50 - 60
     SKILLS DUAL    -> list of spells beginning with 'dual'
*/
void do_skills(CHAR_DATA *ch, const char *argument)
{
	int level;
	int min_level = 1;
	int max_level = LEVEL_HERO;
	int sn;
	int j;
	struct s_spell_info skill_list[MAX_SKILL];
	int nskills = 0;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	int cols = 0;
	int pos = 18;
	char skill_name[MAX_INPUT_LENGTH];
	bool found = FALSE;
	bool new_level = TRUE;
	BUFFER *buffer;
	bool reached_player_level = FALSE;

	if (IS_NPC(ch)) {
		stc("You know all that you need, trust me. ;)\n", ch);
		return;
	}

	/* SKILLS: list all available skills */
	if (*argument == '\0') {
		do_skills(ch, "1");
		return;
	}

	/* Check for a skill name */
	skill_name[0] = '\0';
	argument = one_argument(argument, arg);

	if (!is_number(arg)) {
		strcpy(skill_name, arg);
		argument = one_argument(argument, arg);
	}

	/* Check for 1 or 2 numbers. */
	if (is_number(arg)) {
		min_level = atoi(arg);

		if (min_level < 1)
			min_level = 1;
		else if (min_level > LEVEL_HERO)
			min_level = LEVEL_HERO;

		if (*argument != '\0') {
			argument = one_argument(argument, arg);

			if (is_number(arg)) {
				max_level = atoi(arg);

				if (max_level < min_level)
					max_level = min_level;
				else if (max_level > LEVEL_HERO)
					max_level = LEVEL_HERO;
			}
		}
	}

	/* select skills from player's repertoire or by name */
	for (sn = 0; sn < MAX_SKILL; sn++) {
		if (skill_table[sn].name == NULL)
			break;

		level = skill_table[sn].skill_level[ch->class];

		if (level < min_level
		    || max_level < level
		    || skill_table[sn].spell_fun != spell_null)
			continue;

		if (skill_name[0] == '\0') {
			if (ch->pcdata->learned[sn] <= 0
			    || (skill_table[sn].remort_class != 0 && !IS_IMMORTAL(ch)
			        && ch->class + 1 != skill_table[sn].remort_class && !HAS_EXTRACLASS(ch, sn)))
				continue;
		}
		else if (!is_name(skill_name, skill_table[sn].name))
			continue;

		found = TRUE;
		skill_list[nskills].sn = sn;
		skill_list[nskills].level = level;
		nskills++;
	}

	if (!found) {
		/* if they did a list of all spells... */
		if (min_level == 1 && max_level == LEVEL_HERO && skill_name[0] == '\0')
			stc("You know no skills.\n", ch);
		else
			stc("No skills of that name found.\n", ch);

		return;
	}

	buffer = new_buf();

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
				add_buf(buffer, "\n");
				cols = 0;
			}

			if (level > ch->level && !reached_player_level) {
				sprintf(buf, "Level %3d: %s  (your level)\n", ch->level,
				        "-------------------------------");
				add_buf(buffer, buf);
				reached_player_level = TRUE;
			}
		}

		if (cols == 0) {
			sprintf(buf, "Level %3d: ", level);

			if (!new_level)
				sprintf(buf, "%*s", (int)strlen(buf), " ");

			new_level = FALSE;
			add_buf(buffer, buf);
		}
		else {
			sprintf(buf, "%*s", 24 - pos, " ");
			add_buf(buffer, buf);
		}

		sn = skill_list[j].sn;

		if (ch->pcdata->learned[sn] <= 0
		    || (skill_table[sn].remort_class != 0 && !IS_IMMORTAL(ch)
		        && ch->class + 1 != skill_table[sn].remort_class && !HAS_EXTRACLASS(ch, sn)))
			sprintf(buf, "[not  gained] ");
		else if (ch->level < level)
			sprintf(buf, "[           ] ");
		else
			sprintf(buf, "[{G%3d%% %3d St{x] ", ch->pcdata->learned[sn], get_skill_cost(ch, sn));

		sprintf(arg, "{H%-1.20s{x", skill_table[sn].name);
		strcat(buf, arg);

		if (++cols < 2)
			pos = strlen(arg);
		else {
			strcat(buf, "\n");
			cols = 0;
		}

		add_buf(buffer, buf);
	}

	if (cols > 0) {
		add_buf(buffer, "\n");
		cols = 0;
	}

	page_to_char(buf_string(buffer), ch);
	free_buf(buffer);
}

/* Levelist by Lotus */
void do_levels(CHAR_DATA *ch, const char *argument)
{
	char list[LEVEL_HERO][MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char columns[LEVEL_HERO];
	int sn, lev, x, y;
	char buf[MAX_STRING_LENGTH];
	int class;
	BUFFER *buffer;

	if (IS_NPC(ch))
		return;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		if (!IS_IMMORTAL(ch)) {
			stc("What class do you want level info on?\n", ch);
			return;
		}
		else {
			buffer = new_buf();
			stc("Skill/Spell           Mag Cle Thi War Nec Pdn Bar Ran\n\n", ch);

			for (sn = 0; sn < MAX_SKILL; sn++) {
				if (skill_table[sn].name == NULL)
					break;

				if (skill_table[sn].remort_class < 0) /* for completely ungainable spells/skills */
					continue;

				if (skill_table[sn].remort_class > 0) /* list of remort skills is through 'levels remort' */
					continue;

				sprintf(buf, "%-21s", skill_table[sn].name);
				add_buf(buffer, buf);

				for (x = 0; x < MAX_CLASS; x++) {
					if (skill_table[sn].skill_level[x] < 0 || skill_table[sn].skill_level[x] > LEVEL_HERO)
						sprintf(buf, "{c  NA{x");
					else
						sprintf(buf, "%4d", skill_table[sn].skill_level[x]);

					add_buf(buffer, buf);
				}

				sprintf(buf, "\n");
				add_buf(buffer, buf);
			}

			page_to_char(buf_string(buffer), ch);
			free_buf(buffer);
			return;
		}
	}

	if (!str_prefix1(arg, "remort") && IS_IMMORTAL(ch)) {
		buffer = new_buf();
		stc("                                           Level/Trains\n", ch);
		stc("                       Mag    Cle    Thi    War    Nec    Pdn    Bar    Ran\n", ch);

		for (x = 0; x < MAX_CLASS; x++) {
			sprintf(buf, "{W%s Skills:{x\n", capitalize(class_table[x].name));
			add_buf(buffer, buf);

			for (sn = 0; sn < MAX_SKILL; sn++) {
				if (skill_table[sn].name == NULL)
					break;

				if (skill_table[sn].remort_class != x + 1)
					continue;

				sprintf(buf, "  {g%-19s{x", skill_table[sn].name);
				add_buf(buffer, buf);

				for (y = 0; y < MAX_CLASS; y++) {
					if (skill_table[sn].skill_level[y] < 0 || skill_table[sn].skill_level[y] > LEVEL_HERO)
						sprintf(buf, " {TNA{c/{HNA {x");
					else
						sprintf(buf, "{C%3d{c/{G%-3d{x", skill_table[sn].skill_level[y], skill_table[sn].rating[y]);

					add_buf(buffer, buf);
				}

				sprintf(buf, "\n");
				add_buf(buffer, buf);
			}
		}

		page_to_char(buf_string(buffer), ch);
		free_buf(buffer);
		return;
	}

	if (!str_prefix1(arg, "mage"))        class = 0;
	else if (!str_prefix1(arg, "cleric"))      class = 1;
	else if (!str_prefix1(arg, "thief"))       class = 2;
	else if (!str_prefix1(arg, "warrior"))     class = 3;
	else if (!str_prefix1(arg, "necromancer")) class = 4;
	else if (!str_prefix1(arg, "paladin"))     class = 5;
	else if (!str_prefix1(arg, "bard"))        class = 6;
	else if (!str_prefix1(arg, "ranger"))      class = 7;
	else {
		stc("Invalid class.\n", ch);
		return;
	}

	/* initialize data */
	for (lev = 0; lev < LEVEL_HERO; lev++) {
		columns[lev] = 0;
		list[lev][0] = '\0';
	}

	buffer = new_buf();

	for (sn = 0; sn < MAX_SKILL; sn++) {
		if (skill_table[sn].name == NULL)
			break;

		if (skill_table[sn].skill_level[class] < 0)
			continue;

		if (skill_table[sn].skill_level[class] < LEVEL_HERO &&
		    skill_table[sn].spell_fun != spell_null) {
			lev = skill_table[sn].skill_level[class];
			sprintf(buf, "{G%-20s  {C%3d{x {Tmana{x  ", skill_table[sn].name,
			        skill_table[sn].min_mana);

			if (list[lev][0] == '\0')
				sprintf(list[lev], "\n{HLevel %2d: %s", lev, buf);
			else { /* append */
				if (++columns[lev] % 2 == 0)
					strcat(list[lev], "\n          ");

				strcat(list[lev], buf);
			}
		}
	}

	for (lev = 0; lev < LEVEL_HERO; lev++)
		if (list[lev][0] != '\0')
			add_buf(buffer, list[lev]);

	add_buf(buffer, "\n");

	/* Initialize the data */
	for (lev = 0; lev < LEVEL_HERO; lev++) {
		columns[lev] = 0;
		list[lev][0] = '\0';
	}

	for (sn = 0; sn < MAX_SKILL; sn++) {
		if (skill_table[sn].name == NULL)
			break;

		if (skill_table[sn].skill_level[ch->class] < 0)
			continue;

		if (skill_table[sn].skill_level[class] < LEVEL_HERO &&
		    skill_table[sn].spell_fun == spell_null) {
			lev = skill_table[sn].skill_level[class];
			sprintf(buf, "{G%-20s   {x        ", skill_table[sn].name);

			if (list[lev][0] == '\0')
				sprintf(list[lev], "\n{HLevel %2d: %s{x", lev, buf);
			else { /* append */
				if (++columns[lev] % 2 == 0)
					strcat(list[lev], "\n          ");

				strcat(list[lev], buf);
			}
		}
	}

	for (lev = 0; lev < LEVEL_HERO; lev++)
		if (list[lev][0] != '\0')
			add_buf(buffer, list[lev]);

	add_buf(buffer, "\n");
	page_to_char(buf_string(buffer), ch);
	free_buf(buffer);
} /* end do_levels() */

/* shows skills, groups and costs (only if not bought) */
void list_group_costs(CHAR_DATA *ch)
{
	int gn, sn, col = 0;

	if (IS_NPC(ch))
		return;

	ptc(ch, "%-18s %-5s %-18s %-5s %-18s %-5s\n",
	    "group", "cp", "group", "cp", "group", "cp");

	for (gn = 0; group_table[gn].name != NULL; gn++) {
		if (!ch->gen_data->group_chosen[gn]
		    && !ch->pcdata->group_known[gn]
		    && group_table[gn].rating[ch->class] > 0) {
			ptc(ch, "%-18s %-5d ",
			    group_table[gn].name,
			    group_table[gn].rating[ch->class]);

			if (++col % 3 == 0)
				stc("\n", ch);
		}
	}

	if (col % 3 != 0)
		stc("\n", ch);

	ptc(ch, "\n%-18s %-5s %-18s %-5s %-18s %-5s\n", "skill", "cp", "skill", "cp", "skill", "cp");

	for (sn = 0, col = 0; skill_table[sn].name != NULL; sn++) {
		if (skill_table[sn].remort_class > 0)
			continue;

		if (!ch->gen_data->skill_chosen[sn]
		    &&  ch->pcdata->learned[sn] == 0
		    &&  skill_table[sn].spell_fun == spell_null
		    &&  skill_table[sn].rating[ch->class] > 0) {
			ptc(ch, "%-18s %-5d ",
			    skill_table[sn].name,
			    skill_table[sn].rating[ch->class]);

			if (++col % 3 == 0)
				stc("\n", ch);
		}
	}

	if (col % 3 != 0)
		stc("\n", ch);

	ptc(ch, "\nCreation points: %d\n"
	    "Experience per level: %ld\n",
	    ch->pcdata->points,
	    exp_per_level(ch, ch->gen_data->points_chosen));
}

void list_group_chosen(CHAR_DATA *ch)
{
	char buf[100];
	int gn, sn, col;

	if (IS_NPC(ch))
		return;

	col = 0;
	sprintf(buf, "%-18s %-5s %-18s %-5s %-18s %-5s", "group", "cp", "group", "cp", "group", "cp\n");
	stc(buf, ch);

	for (gn = 0; gn < MAX_GROUP; gn++) {
		if (group_table[gn].name == NULL)
			break;

		if (ch->gen_data->group_chosen[gn]
		    &&  group_table[gn].rating[ch->class] > 0) {
			sprintf(buf, "%-18s %-5d ", group_table[gn].name,
			        group_table[gn].rating[ch->class]);
			stc(buf, ch);

			if (++col % 3 == 0)
				stc("\n", ch);
		}
	}

	if (col % 3 != 0)
		stc("\n", ch);

	stc("\n", ch);
	col = 0;
	sprintf(buf, "%-18s %-5s %-18s %-5s %-18s %-5s", "skill", "cp", "skill", "cp", "skill", "cp\n");
	stc(buf, ch);

	for (sn = 0; sn < MAX_SKILL; sn++) {
		if (skill_table[sn].name == NULL)
			break;

		if (ch->gen_data->skill_chosen[sn]
		    &&  skill_table[sn].rating[ch->class] > 0) {
			sprintf(buf, "%-18s %-5d ", skill_table[sn].name,
			        skill_table[sn].rating[ch->class]);
			stc(buf, ch);

			if (++col % 3 == 0)
				stc("\n", ch);
		}
	}

	if (col % 3 != 0)
		stc("\n", ch);

	stc("\n", ch);
	sprintf(buf, "Creation points: %d\n", ch->gen_data->points_chosen);
	stc(buf, ch);
	sprintf(buf, "Experience per level: %ld\n",
	        exp_per_level(ch, ch->gen_data->points_chosen));
	stc(buf, ch);
	return;
}

long exp_per_level(CHAR_DATA *ch, int points)
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

	expl *= pc_race_table[ch->race].class_mult[ch->class];
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

/* this procedure handles the input parsing for the skill generator */
bool parse_gen_groups(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[100];
	int gn, sn, i;

	if (argument[0] == '\0')
		return FALSE;

	argument = one_argument(argument, arg);

	if (!str_prefix1(arg, "help")) {
		if (argument[0] == '\0') {
			help(ch, "group help");
			return TRUE;
		}

		help(ch, argument);
		return TRUE;
	}

	if (!str_prefix1(arg, "add")) {
		if (argument[0] == '\0') {
			stc("You must provide a skill name.\n", ch);
			return TRUE;
		}

		gn = group_lookup(argument);

		if (gn != -1) {
			if (ch->gen_data->group_chosen[gn]
			    ||  ch->pcdata->group_known[gn]) {
				stc("You already know that group!\n", ch);
				return TRUE;
			}

			if (group_table[gn].rating[ch->class] < 1) {
				stc("That group is not available.\n", ch);
				return TRUE;
			}

			sprintf(buf, "%s group added\n", group_table[gn].name);
			stc(buf, ch);
			ch->gen_data->group_chosen[gn] = TRUE;
			ch->gen_data->points_chosen += group_table[gn].rating[ch->class];
			gn_add(ch, gn);
			ch->pcdata->points += group_table[gn].rating[ch->class];
			return TRUE;
		}

		sn = skill_lookup(argument);

		if (sn != -1) {
			if (ch->gen_data->skill_chosen[sn]
			    ||  ch->pcdata->learned[sn] > 0) {
				stc("You already know that skill!\n", ch);
				return TRUE;
			}

			if (skill_table[sn].rating[ch->class] < 1
			    ||  skill_table[sn].spell_fun != spell_null
			    ||  skill_table[sn].remort_class != 0) {
				stc("That skill is not available.\n", ch);
				return TRUE;
			}

			sprintf(buf, "%s skill added\n", skill_table[sn].name);
			stc(buf, ch);
			ch->gen_data->skill_chosen[sn] = TRUE;
			ch->gen_data->points_chosen += skill_table[sn].rating[ch->class];
			ch->pcdata->learned[sn] = 1;
			ch->pcdata->evolution[sn] = 1;
			ch->pcdata->points += skill_table[sn].rating[ch->class];
			return TRUE;
		}

		stc("No skills or groups by that name...\n", ch);
		return TRUE;
	}

	if (!strcmp(arg, "drop")) {
		if (argument[0] == '\0') {
			stc("You must provide a skill to drop.\n", ch);
			return TRUE;
		}

		gn = group_lookup(argument);

		if (gn != -1 && ch->gen_data->group_chosen[gn]) {
			stc("Group dropped.\n", ch);
			ch->gen_data->group_chosen[gn] = FALSE;
			ch->gen_data->points_chosen -= group_table[gn].rating[ch->class];
			gn_remove(ch, gn);

			for (i = 0; i < MAX_GROUP; i++) {
				if (ch->gen_data->group_chosen[gn])
					gn_add(ch, gn);
			}

			ch->pcdata->points -= group_table[gn].rating[ch->class];
			return TRUE;
		}

		sn = skill_lookup(argument);

		if (sn != -1 && ch->gen_data->skill_chosen[sn]) {
			stc("Skill dropped.\n", ch);
			ch->gen_data->skill_chosen[sn] = FALSE;
			ch->gen_data->points_chosen -= skill_table[sn].rating[ch->class];
			ch->pcdata->learned[sn] = 0;
			ch->pcdata->evolution[sn] = 0;
			ch->pcdata->points -= skill_table[sn].rating[ch->class];
			return TRUE;
		}

		stc("You haven't bought any such skill or group.\n", ch);
		return TRUE;
	}

	if (!str_prefix1(arg, "premise")) {
		help(ch, "premise");
		return TRUE;
	}

	if (!str_prefix1(arg, "list")) {
		list_group_costs(ch);
		return TRUE;
	}

	if (!str_prefix1(arg, "learned")) {
		list_group_chosen(ch);
		return TRUE;
	}

	if (!str_prefix1(arg, "info")) {
		do_groups(ch, argument);
		return TRUE;
	}

	/*
	    if (!str_prefix1(arg,"levels"))
	    {
	        do_levels( ch, argument);
	        return TRUE;
	    }
	*/
	return FALSE;
}

/* shows all groups, or the sub-members of a group */
void do_groups(CHAR_DATA *ch, const char *argument)
{
	char buf[100];
	int gn, sn, col;

	if (IS_NPC(ch))
		return;

	col = 0;

	if (argument[0] == '\0') {
		/* show all groups */
		for (gn = 0; gn < MAX_GROUP; gn++) {
			if (group_table[gn].name == NULL)
				break;

			if (ch->pcdata->group_known[gn]) {
				sprintf(buf, "%-18s ", group_table[gn].name);
				stc(buf, ch);

				if (++col % 3 == 0)
					stc("\n", ch);
			}
		}

		if (col % 3 != 0)
			stc("\n", ch);

		sprintf(buf, "Creation points: %d\n", ch->pcdata->points);
		stc(buf, ch);
		return;
	}

	if (!str_cmp(argument, "all")) { /* show all groups */
		for (gn = 0; gn < MAX_GROUP; gn++) {
			if (group_table[gn].name == NULL)
				break;

			sprintf(buf, "%-18s ", group_table[gn].name);
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

	for (sn = 0; sn < MAX_IN_GROUP; sn++) {
		if (group_table[gn].spells[sn] == NULL)
			break;

		sprintf(buf, "%-18s ", group_table[gn].spells[sn]);
		stc(buf, ch);

		if (++col % 3 == 0)
			stc("\n", ch);
	}

	if (col % 3 != 0)
		stc("\n", ch);
}

/* checks for skill improvement */
void check_improve(CHAR_DATA *ch, int sn, bool success, int multiplier)
{
	int chance;
	char buf[100];
	int xp;

	if (IS_NPC(ch))
		return;

	if (ch->level < skill_table[sn].skill_level[ch->class]
	    ||  skill_table[sn].rating[ch->class] == 0
	    ||  ch->pcdata->learned[sn] == 0
	    ||  ch->pcdata->learned[sn] == 100)
		return;  /* skill is not known */

	/* check to see if the character has a chance to learn */
	chance = 10 * int_app[GET_ATTR_INT(ch)].learn;
	chance /= (multiplier
	           *       skill_table[sn].rating[ch->class]
	           *       4);
	chance += ch->level;

	if (skill_table[sn].remort_class > 0)
	{   chance *= 2; chance /= 3; }

	/* -1 multiplier means auto success -- Montrey */
	if (number_range(1, 1000) > chance && multiplier != -1)
		return;

	/* pre-calculate the experience gain */
	xp = 2 * skill_table[sn].rating[ch->class];
	/*
	if ( ch->pcdata->remort_count > 1 )
	    xp /= ch->pcdata->remort_count;
	*/

	/* now that the character has a CHANCE to learn, see if they really have */

	if (success) {
		chance = URANGE(5, 100 - ch->pcdata->learned[sn], 95);

		if (number_percent() < chance || multiplier == -1) {
			sprintf(buf, "{GYou have become better at {H%s{G!{x\n",
			        skill_table[sn].name);
			stc(buf, ch);
			ch->pcdata->learned[sn]++;
			gain_exp(ch, xp);
		}
	}
	else {
		chance = URANGE(5, ch->pcdata->learned[sn] / 2, 30);

		if (number_percent() < chance || multiplier == -1) {
			sprintf(buf,
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
int group_lookup(const char *name)
{
	int gn;

	for (gn = 0; group_table[gn].name != NULL; gn++)
		if (!str_cmp(name, group_table[gn].name))
			return gn;

	return -1;
}

/* recursively adds a group given its number -- uses group_add */
void gn_add(CHAR_DATA *ch, int gn)
{
	int i;
	ch->pcdata->group_known[gn] = TRUE;

	for (i = 0; i < MAX_IN_GROUP; i++) {
		if (group_table[gn].spells[i] == NULL)
			break;

		group_add(ch, group_table[gn].spells[i], FALSE);
	}
}

/* recusively removes a group given its number -- uses group_remove */
void gn_remove(CHAR_DATA *ch, int gn)
{
	int i;
	ch->pcdata->group_known[gn] = FALSE;

	for (i = 0; i < MAX_IN_GROUP; i ++) {
		if (group_table[gn].spells[i] == NULL)
			break;

		group_remove(ch, group_table[gn].spells[i]);
	}
}

/* use for processing a skill or group for addition  */
void group_add(CHAR_DATA *ch, const char *name, bool deduct)
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
				ch->pcdata->points += skill_table[sn].rating[ch->class];
		}

		return;
	}

	/* now check groups */
	gn = group_lookup(name);

	if (gn != -1) {
		if (ch->pcdata->group_known[gn] == FALSE) {
			ch->pcdata->group_known[gn] = TRUE;

			if (deduct)
				ch->pcdata->points += group_table[gn].rating[ch->class];
		}

		gn_add(ch, gn); /* make sure all skills in the group are known */
	}
}

/* used for processing a skill or group for deletion -- no points back! */

void group_remove(CHAR_DATA *ch, const char *name)
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

int get_evolution(CHAR_DATA *ch, int sn)
{
	int evolution;

	if (IS_NPC(ch))
		evolution = 1;
	else
		evolution = URANGE(1, ch->pcdata->evolution[sn], 4);

	return evolution;
} /* end get_evolution */

int can_evolve(CHAR_DATA *ch, int sn)
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

	if (skill_table[sn].evocost_sec[ch->class] <= 0)
		return -1;

	if (ch->pcdata->learned[sn] <= 0)
		return 1;

	/* skill is evolvable */
	if (skill_table[sn].evocost_pri[ch->class] <= 0) {
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

void evolve_list(CHAR_DATA *ch)
{
	BUFFER *buffer;
	int x, can;
	buffer = new_buf();
	add_buf(buffer, "Currently evolvable skills and spells:\n\n");
	add_buf(buffer, "{GSkill or spell      {C| {GPct {C| {GEvo {C| {GNext{x\n");
	add_buf(buffer, "{C--------------------+-----+-----+-----{x\n");

	for (x = 0; x < MAX_SKILL; x++) {
		if (skill_table[x].name == NULL)
			break;

		if ((can = can_evolve(ch, x)) == -1)
			continue;

		if (ch->pcdata->learned[x] < 1) {
			ptb(buffer, "{H%-20s{C|   {H0 {C|   {H0 {C| {H%4d{x\n",
			    skill_table[x].name,
			    skill_table[x].evocost_sec[ch->class]);
			continue;
		}

		ptb(buffer, "{G%-20s{C| {G%3d {C|   {G%d {C| {G",
		    skill_table[x].name,
		    ch->pcdata->learned[x],
		    ch->pcdata->evolution[x]);

		if (can == 1)
			ptb(buffer, "%4d{x\n",
			    ch->pcdata->evolution[x] == 1 ?
			    skill_table[x].evocost_sec[ch->class] :
			    skill_table[x].evocost_pri[ch->class]);
		else
			add_buf(buffer, "----{x\n");
	}

	ptb(buffer, "\nYou have %d skill points to spend.\n", ch->pcdata->skillpoints);
	page_to_char(buf_string(buffer), ch);
	free_buf(buffer);
}

void evolve_info(CHAR_DATA *ch)
{
	BUFFER *buffer = new_buf();
	int evo_sum[8] = {0};

	add_buf(buffer, "Currently evolvable skills and spells:\n\n");
	add_buf(buffer, "{GSkill or spell      {C|{G");

	for (int class = 0; class < 8; class++) {
		add_buf(buffer, class_table[class].who_name);
		add_buf(buffer, "{C|{G");
	}

	add_buf(buffer, "\n");

	add_buf(buffer, "{C--------------------+---+---+---+---+---+---+---+---+{x\n");

	for (int sn = 0; sn < MAX_SKILL; sn++) {
		if (skill_table[sn].name == NULL)
			break;

		int max_evo[8] = {0};
		bool should_show = FALSE;

		for (int class = 0; class < 8; class++) {
			if (skill_table[sn].evocost_sec[class] > 0) {
				max_evo[class]++;
				should_show = TRUE;
			}

			if (skill_table[sn].evocost_pri[class] > 0)
				max_evo[class]++;
		}

		if (!should_show)
			continue;

		ptb(buffer, "{H%-20s{C|", skill_table[sn].name);

		for (int class = 0; class < 8; class++) {
			if (max_evo[class] == 0)
				add_buf(buffer, "   |");
			else {
				ptb(buffer, "{%s %d {C|",
					max_evo[class] > 1 ? "Y" : "G",
					max_evo[class]+1);

				evo_sum[class] += max_evo[class]+1;
			}
		}

		add_buf(buffer, "\n");

	}

	add_buf(buffer, "{C--------------------+---+---+---+---+---+---+---+---+{x\n");

	ptb(buffer, "{H%-20s{C|", "sum");

	for (int class = 0; class < 8; class++)
		ptb(buffer, "{G%3d{C|", evo_sum[class]);

	add_buf(buffer, "\n");
	page_to_char(buf_string(buffer), ch);
	free_buf(buffer);
}

void do_evolve(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char *type;
	int sn, cost, can, perc;
	int special = 0;

	if (IS_NPC(ch)) {
		stc("Sorry, your skills are limited.\n", ch);
		return;
	}

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		evolve_list(ch);
		return;
	}

	if (IS_IMMORTAL(ch) && !str_cmp(arg, "info")) {
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
		cost = skill_table[sn].evocost_sec[ch->class];
		perc = 0;
	}
	else if (IS_IMMORTAL(ch) && special == 0) {
		cost = 0;
		perc = 1;
	}
	else if (ch->pcdata->evolution[sn] == 1 && special == 0) {
		cost = skill_table[sn].evocost_sec[ch->class];
		perc = 85;
	}
	else if (ch->pcdata->evolution[sn] == 1 && special == 1) {
		cost = skill_table[sn].evocost_pri[ch->class];
		perc = 0;
	}
	else {
		cost = skill_table[sn].evocost_pri[ch->class];
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

int get_skill_cost(CHAR_DATA *ch, int sn)
{
	int cost = skill_table[sn].min_mana;

	/*      if (!skill_table[sn].min_mana)
	                return 0;

	        if (ch->level + 2 == skill_table[sn].skill_level[ch->class])
	                cost = 50;
	        else
	                cost = UMAX(skill_table[sn].min_mana,
	                        100 / (2 + ch->level - skill_table[sn].skill_level[ch->class]));

	        return cost; */

	if (skill_table[sn].spell_fun == spell_null) {
		if (/*skill_table[sn].target == TAR_CHAR_OFFENSIVE && */ch->level <= 50) {
			int pct_max, level = skill_table[sn].skill_level[ch->class];
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

bool deduct_stamina(CHAR_DATA *ch, int sn)
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
void do_gain(CHAR_DATA *ch, const char *argument)
{
	char arg[MIL];
	CHAR_DATA *trainer;
	int gn = 0, sn = 0;

	if (IS_NPC(ch)) {
		stc("Trust me, you know all you need.\n", ch);
		return;
	}

	/* find a trainer */
	for (trainer = ch->in_room->people; trainer; trainer = trainer->next_in_room)
		if (IS_NPC(trainer) && IS_SET(trainer->act, ACT_GAIN) && can_see(ch, trainer))
			break;

	if (trainer == NULL) {
		stc("You can't do that here.\n", ch);
		return;
	}

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Syntax:\n"
		    "gain list\n"
		    "gain convert\n"
		    "gain revert\n"
		    "gain points\n"
		    "gain <skill or spell group>\n", ch);
		return;
	}

	if (!str_prefix1(arg, "list")) {
		int col = 0;
		BUFFER *output;
		bool foundsect = FALSE, foundall = FALSE;
		output = new_buf();
		ptb(output, "%-18s %-5s %-18s %-5s %-18s %-5s\n",
		    "group", "cost", "group", "cost", "group", "cost");

		for (gn = 0; group_table[gn].name != NULL; gn++) {
			if (completed_group(ch, gn)
			    || group_table[gn].rating[ch->class] <= 0)
				continue;

			foundsect = TRUE;
			ptb(output, "%-18s %s%-5d{x ",
			    group_table[gn].name,
			    group_table[gn].rating[ch->class] > ch->train ? "{T" : "{C",
			    group_table[gn].rating[ch->class]);

			if (++col % 3 == 0)
				add_buf(output, "\n");
		}

		if (col % 3 != 0)
			add_buf(output, "\n");

		add_buf(output, "\n");

		if (foundsect) {
			page_to_char(buf_string(output), ch);
			foundall = TRUE;
		}

		free_buf(output);
		output = new_buf();
		ptb(output, "%-18s %-5s %-18s %-5s %-18s %-5s\n",
		    "skill", "cost", "skill", "cost", "skill", "cost");

		for (sn = 0, col = 0, foundsect = FALSE; skill_table[sn].name != NULL; sn++) {
			if (!ch->pcdata->learned[sn]
			    && skill_table[sn].rating[ch->class] > 0
			    && skill_table[sn].spell_fun == spell_null
			    && skill_table[sn].remort_class == 0) {
				foundsect = TRUE;
				ptb(output, "%s%-18s %s%-5d{x ",
				    skill_table[sn].skill_level[ch->class] > ch->level ? "{c" : "{g",
				    skill_table[sn].name,
				    skill_table[sn].rating[ch->class] > ch->train ? "{T" : "{C",
				    skill_table[sn].rating[ch->class]);

				if (++col % 3 == 0)
					add_buf(output, "\n");
			}
		}

		if (col % 3 != 0)
			add_buf(output, "\n");

		if (foundsect) {
			page_to_char(buf_string(output), ch);
			foundall = TRUE;
		}

		free_buf(output);
		output = new_buf();

		if (ch->pcdata->remort_count < 1) {
			if (!foundall)
				add_buf(output, "You have nothing left to learn until you remort.\n");

			ptb(output, "You have %d train%s left.\n",
			    ch->train, ch->train == 1 ? "" : "s");
			page_to_char(buf_string(output), ch);
			free_buf(output);
			return;
		}

		if (foundall)
			add_buf(output, "\n");

		ptb(output, "Remort specials for class %s\n",
		    class_table[ch->class].name);
		add_buf(output, "---------------\n");

		for (sn = 0, col = 0, foundsect = FALSE; skill_table[sn].name != NULL; sn++) {
			if (!ch->pcdata->learned[sn]
			    && skill_table[sn].rating[ch->class] > 0
			    && skill_table[sn].remort_class > 0
			    && skill_table[sn].remort_class == ch->class + 1) {
				foundsect = TRUE;
				ptb(output, "%s%-18s %s%-5d{x ",
				    skill_table[sn].skill_level[ch->class] > ch->level ? "{c" : "{g",
				    skill_table[sn].name,
				    skill_table[sn].rating[ch->class] > ch->train ? "{T" : "{C",
				    skill_table[sn].rating[ch->class]);

				if (++col % 3 == 0)
					add_buf(output, "\n");
			}
		}

		if (col % 3 != 0)
			add_buf(output, "\n");

		if (foundsect) {
			page_to_char(buf_string(output), ch);
			foundall = TRUE;
		}

		free_buf(output);
		output = new_buf();

		if (!foundall)
			add_buf(output, "You have nothing left to learn!\n");

		ptb(output, "You have %d train%s left.\n",
		    ch->train, ch->train == 1 ? "" : "s");
		page_to_char(buf_string(output), ch);
		free_buf(output);
		return;
	}

	if (!str_prefix1(arg, "convert")) {
		if (ch->practice < 10) {
			act("$N tells you 'You are not yet ready.'", ch, NULL, trainer, TO_CHAR);
			return;
		}

		act("$N helps you apply your practice to training.", ch, NULL, trainer, TO_CHAR);
		ch->practice -= 10;
		ch->train++;
		return;
	}

	if (!str_prefix1(arg, "revert")) {
		if (ch->train < 1) {
			act("$N tells you 'You are not yet ready.'", ch, NULL, trainer, TO_CHAR);
			return;
		}

		act("$N helps you apply your training to practices.", ch, NULL, trainer, TO_CHAR);
		ch->practice += 10;
		ch->train--;
		return;
	}

	if (!str_prefix1(arg, "points")) {
		int mod;

		if (ch->train < 1) {
			act("$N tells you 'You are not yet ready.'", ch, NULL, trainer, TO_CHAR);
			return;
		}

		if (ch->pcdata->points <= 40) {
			act("$N tells you 'There would be no point in that.'", ch, NULL, trainer, TO_CHAR);
			return;
		}

		act("$N trains you, and you feel more at ease with your skills.", ch, NULL, trainer, TO_CHAR);
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
			act("$N tells you 'You already know that group!'", ch, NULL, trainer, TO_CHAR);
			return;
		}

		if (group_table[gn].rating[ch->class] <= 0) {
			act("$N tells you 'That group is beyond your powers.'", ch, NULL, trainer, TO_CHAR);
			return;
		}

		if (ch->train < group_table[gn].rating[ch->class]) {
			act("$N tells you 'You are not yet ready for that group.'", ch, NULL, trainer, TO_CHAR);
			return;
		}

		/* add the group */
		gn_add(ch, gn);
		act("$N trains you in the art of $t.", ch, group_table[gn].name, trainer, TO_CHAR);
		ch->train -= group_table[gn].rating[ch->class];
		return;
	}

	sn = skill_lookup(argument);

	if (sn > -1) {
		if (skill_table[sn].spell_fun != spell_null
		    && skill_table[sn].remort_class == 0) {
			act("$N tells you 'You must learn the full group.'", ch, NULL, trainer, TO_CHAR);
			return;
		}

		if (skill_table[sn].remort_class > 0) {
			if (ch->pcdata->remort_count < 1) {
				act("$N tells you 'That skill is available only to remorts.'",
				    ch, NULL, trainer, TO_CHAR);
				return;
			}

			if (ch->class + 1 != skill_table[sn].remort_class) {
				act("$N tells you 'Use extraclass for skills and spells outside your class.'",
				    ch, NULL, trainer, TO_CHAR);
				return;
			}
		}

		if (ch->pcdata->learned[sn]) {
			act("$N tells you 'You already know that skill!'", ch, NULL, trainer, TO_CHAR);
			return;
		}

		if (skill_table[sn].rating[ch->class] <= 0) {
			act("$N tells you 'That skill is beyond your powers.'", ch, NULL, trainer, TO_CHAR);
			return;
		}

		if (ch->train < skill_table[sn].rating[ch->class]) {
			act("$N tells you 'You are not yet ready for that skill.'", ch, NULL, trainer, TO_CHAR);
			return;
		}

		/* add the skill */
		ch->pcdata->learned[sn] = 1;
		ch->pcdata->evolution[sn] = 1;
		act("$N trains you in the art of $t.", ch, skill_table[sn].name, trainer, TO_CHAR);
		ch->train -= skill_table[sn].rating[ch->class];
		return;
	}

	act("$N tells you 'I have no knowledge of that skill or spell group.'", ch, NULL, trainer, TO_CHAR);
}

/* This function converts skill points into practice sessions.
10 skill points is required to make one practice session.

The syntax for this command is
convert <skill_points>
For example: "convert 20" will change 20 SP into 2 practices.

-- Outsider
*/
void do_convert(CHAR_DATA *ch, const char *argument)
{
	sh_int use_points;
	char buffer[MAX_INPUT_LENGTH];

	if (argument[0] == '\0') {
		stc("Convert: Change <X> number of skill points into practice sessions.\n", ch);
		stc("Syntax: convert <skill_points>\n", ch);
		return;
	}

	if (!is_number(argument)) {
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
	sprintf(buffer, "You now have %d skill points and %d practice sessions.\n",
	        ch->pcdata->skillpoints, ch->practice);
	stc(buffer, ch);
}
