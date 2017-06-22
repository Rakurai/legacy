#include "Customize.hh"

#include "merc.hh"

Customize::Customize() :
	skill_chosen(skill_table.size(), FALSE),
	group_chosen(group_table.size(), FALSE)
{}


/* shows skills, groups and costs (only if not bought) */
void list_group_costs(Character *ch)
{
	int gn, sn, col = 0;

	if (IS_NPC(ch))
		return;

	ptc(ch, "%-18s %-5s %-18s %-5s %-18s %-5s\n",
	    "group", "cp", "group", "cp", "group", "cp");

	for (gn = 0; gn < group_table.size(); gn++) {
		if (!ch->gen_data->group_chosen[gn]
		    && !ch->pcdata->group_known[gn]
		    && group_table[gn].rating[ch->cls] > 0) {
			ptc(ch, "%-18s %-5d ",
			    group_table[gn].name,
			    group_table[gn].rating[ch->cls]);

			if (++col % 3 == 0)
				stc("\n", ch);
		}
	}

	if (col % 3 != 0)
		stc("\n", ch);

	ptc(ch, "\n%-18s %-5s %-18s %-5s %-18s %-5s\n", "skill", "cp", "skill", "cp", "skill", "cp");

	for (sn = 0, col = 0; sn < skill_table.size(); sn++) {
		if (skill_table[sn].remort_class > 0)
			continue;

		if (!ch->gen_data->skill_chosen[sn]
		    &&  ch->pcdata->learned[sn] == 0
		    &&  skill_table[sn].spell_fun == spell_null
		    &&  skill_table[sn].rating[ch->cls] > 0) {
			ptc(ch, "%-18s %-5d ",
			    skill_table[sn].name,
			    skill_table[sn].rating[ch->cls]);

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

void list_group_chosen(Character *ch)
{
	char buf[100];
	int gn, sn, col;

	if (IS_NPC(ch))
		return;

	col = 0;
	Format::sprintf(buf, "%-18s %-5s %-18s %-5s %-18s %-5s", "group", "cp", "group", "cp", "group", "cp\n");
	stc(buf, ch);

	for (gn = 0; gn < group_table.size(); gn++) {
		if (ch->gen_data->group_chosen[gn]
		    &&  group_table[gn].rating[ch->cls] > 0) {
			Format::sprintf(buf, "%-18s %-5d ", group_table[gn].name,
			        group_table[gn].rating[ch->cls]);
			stc(buf, ch);

			if (++col % 3 == 0)
				stc("\n", ch);
		}
	}

	if (col % 3 != 0)
		stc("\n", ch);

	stc("\n", ch);
	col = 0;
	Format::sprintf(buf, "%-18s %-5s %-18s %-5s %-18s %-5s", "skill", "cp", "skill", "cp", "skill", "cp\n");
	stc(buf, ch);

	for (sn = 0; sn < skill_table.size(); sn++) {
		if (ch->gen_data->skill_chosen[sn]
		    &&  skill_table[sn].rating[ch->cls] > 0) {
			Format::sprintf(buf, "%-18s %-5d ", skill_table[sn].name,
			        skill_table[sn].rating[ch->cls]);
			stc(buf, ch);

			if (++col % 3 == 0)
				stc("\n", ch);
		}
	}

	if (col % 3 != 0)
		stc("\n", ch);

	stc("\n", ch);
	Format::sprintf(buf, "Creation points: %d\n", ch->gen_data->points_chosen);
	stc(buf, ch);
	Format::sprintf(buf, "Experience per level: %ld\n",
	        exp_per_level(ch, ch->gen_data->points_chosen));
	stc(buf, ch);
	return;
}

/* this procedure handles the input parsing for the skill generator */
bool parse_gen_groups(Character *ch, String argument)
{
	char buf[100];
	int gn, sn, i;

	if (argument.empty())
		return FALSE;

	String arg;
	argument = one_argument(argument, arg);

	if (arg.is_prefix_of("help")) {
		if (argument.empty()) {
			help(ch, "group help");
			return TRUE;
		}

		help(ch, argument);
		return TRUE;
	}

	if (arg.is_prefix_of("add")) {
		if (argument.empty()) {
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

			if (group_table[gn].rating[ch->cls] < 1) {
				stc("That group is not available.\n", ch);
				return TRUE;
			}

			Format::sprintf(buf, "%s group added\n", group_table[gn].name);
			stc(buf, ch);
			ch->gen_data->group_chosen[gn] = TRUE;
			ch->gen_data->points_chosen += group_table[gn].rating[ch->cls];
			gn_add(ch, gn);
			ch->pcdata->points += group_table[gn].rating[ch->cls];
			return TRUE;
		}

		sn = skill_lookup(argument);

		if (sn != -1) {
			if (ch->gen_data->skill_chosen[sn]
			    ||  ch->pcdata->learned[sn] > 0) {
				stc("You already know that skill!\n", ch);
				return TRUE;
			}

			if (skill_table[sn].rating[ch->cls] < 1
			    ||  skill_table[sn].spell_fun != spell_null
			    ||  skill_table[sn].remort_class != 0) {
				stc("That skill is not available.\n", ch);
				return TRUE;
			}

			Format::sprintf(buf, "%s skill added\n", skill_table[sn].name);
			stc(buf, ch);
			ch->gen_data->skill_chosen[sn] = TRUE;
			ch->gen_data->points_chosen += skill_table[sn].rating[ch->cls];
			ch->pcdata->learned[sn] = 1;
			ch->pcdata->evolution[sn] = 1;
			ch->pcdata->points += skill_table[sn].rating[ch->cls];
			return TRUE;
		}

		stc("No skills or groups by that name...\n", ch);
		return TRUE;
	}

	if (!strcmp(arg, "drop")) {
		if (argument.empty()) {
			stc("You must provide a skill to drop.\n", ch);
			return TRUE;
		}

		gn = group_lookup(argument);

		if (gn != -1 && ch->gen_data->group_chosen[gn]) {
			stc("Group dropped.\n", ch);
			ch->gen_data->group_chosen[gn] = FALSE;
			ch->gen_data->points_chosen -= group_table[gn].rating[ch->cls];
			gn_remove(ch, gn);

			for (i = 0; i < group_table.size(); i++) {
				if (ch->gen_data->group_chosen[gn])
					gn_add(ch, gn);
			}

			ch->pcdata->points -= group_table[gn].rating[ch->cls];
			return TRUE;
		}

		sn = skill_lookup(argument);

		if (sn != -1 && ch->gen_data->skill_chosen[sn]) {
			stc("Skill dropped.\n", ch);
			ch->gen_data->skill_chosen[sn] = FALSE;
			ch->gen_data->points_chosen -= skill_table[sn].rating[ch->cls];
			ch->pcdata->learned[sn] = 0;
			ch->pcdata->evolution[sn] = 0;
			ch->pcdata->points -= skill_table[sn].rating[ch->cls];
			return TRUE;
		}

		stc("You haven't bought any such skill or group.\n", ch);
		return TRUE;
	}

	if (arg.is_prefix_of("premise")) {
		help(ch, "premise");
		return TRUE;
	}

	if (arg.is_prefix_of("list")) {
		list_group_costs(ch);
		return TRUE;
	}

	if (arg.is_prefix_of("learned")) {
		list_group_chosen(ch);
		return TRUE;
	}

	if (arg.is_prefix_of("info")) {
		do_groups(ch, argument);
		return TRUE;
	}

	/*
	    if (arg.is_prefix_of("levels"))
	    {
	        do_levels( ch, argument);
	        return TRUE;
	    }
	*/
	return FALSE;
}
