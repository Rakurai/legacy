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
#include "Character.hh"
#include "declare.hh"
#include "find.hh"
#include "Flags.hh"
#include "Format.hh"
#include "interp.hh"
#include "lookup.hh"
#include "Logging.hh"
#include "macros.hh"
#include "magic.hh"
#include "merc.hh"
#include "Player.hh"
#include "random.hh"
#include "skill/skill.hh"
#include "String.hh"
#include "tables.hh"

/*****
 Remort Affects Stuff
 *****/

/* take a raff id and return the index number, 0 if a null raff */
int raff_lookup(int index)
{
	int i;

	for (i = 1; i < MAX_RAFFECTS; i++) {
		if (raffects[i].id == index)
			return i;

		/* returns 0 on the first raff, which is null anyway */
	}

	return 0;
}

void fix_blank_raff(Character *ch, int start)
{
	/* hunts through a remort's raffects, takes out zeroes except at the end */
	/* start is the starting point, most cases use 0 */
	int i, x, last;

	if (IS_NPC(ch) || !IS_REMORT(ch))
		return;

	last = ((ch->pcdata->remort_count / 10) + 1);

	/* loop through them all starting where told to */
	for (i = start; i < last; i++) {
		/* if it's null, fix it by shifting all raffects down one */
		if (ch->pcdata->raffect[i] < 1) {
			for (x = i; x < last; x++)
				/* set the affect equal to the next one, if it's the last, set to 0 in case they have more */
				ch->pcdata->raffect[x] = ((x == last) ? 0 : ch->pcdata->raffect[x + 1]);

			/* max raffect fields is 10, set last one to 0 */
		}
	}
}

void rem_raff_affect(Character *ch, int index)
{
	if (!raffects[index].add.empty()) {
		if ((raffects[index].id >= 900) && (raffects[index].id <= 949))
			affect::remort_affect_modify_char(ch, TO_VULN, raffects[index].add, FALSE);
		else if ((raffects[index].id >= 950) && (raffects[index].id <= 999))
			affect::remort_affect_modify_char(ch, TO_RESIST, raffects[index].add, FALSE);
	}

	return;
}

bool HAS_RAFF(Character *ch, int flag)
{
	int i;

	if (flag <= 0)
		return FALSE;

	if (IS_NPC(ch))
		return FALSE;

	for (i = 0; i < ((ch->pcdata->remort_count / 10) + 1); i++) {
		if (ch->pcdata->raffect[i] == flag)
			return TRUE;
	}

	return FALSE;
}

bool HAS_RAFF_GROUP(Character *ch, int flag)
{
	int i;

	if (IS_NPC(ch))
		return FALSE;

	for (i = 0; i < ((ch->pcdata->remort_count / 10) + 1); i++) {
		if (raffects[raff_lookup(ch->pcdata->raffect[i])].group == flag)
			return TRUE;
	}

	return FALSE;
}

void raff_add_to_char(Character *ch, int raff_id) {
	int index;

	for (index = 1; index < MAX_RAFFECTS; index++)
		if (raffects[index].id == raff_id)
			break;

	if (index == MAX_RAFFECTS) {
		Logging::bugf("raffect::add_to_char: invalid raffect ID %d", raff_id);
		return;
	}

	if (!raffects[index].add.empty()) {
		if ((raffects[index].id >= 900) && (raffects[index].id <= 949))
			affect::remort_affect_modify_char(ch, TO_VULN, raffects[index].add, TRUE);
		else if ((raffects[index].id >= 950) && (raffects[index].id <= 999))
			affect::remort_affect_modify_char(ch, TO_RESIST, raffects[index].add, TRUE);
	}
}

void roll_one_raff(Character *ch, Character *victim, int place)
{
	int test;
	bool can_add = FALSE;

	do {
		test = number_range(1, MAX_RAFFECTS);

		while (test < 1 || test > MAX_RAFFECTS
		       || raffects[test].id < 1
		       || number_percent() < raffects[test].chance)
			test = number_range(1, MAX_RAFFECTS);

		/* Check percentage chance, and whether it's a 'good' or 'bad' remort affect */
		if ((raffects[test].id >= 1)
		    && (raffects[test].id <= 99) /* if it's a good one... */
		    && (number_percent() <= (raffects[test].chance + (victim->pcdata->remort_count / 10))))
			can_add = TRUE;
		else if ((raffects[test].id >= 100)
		         && (raffects[test].id <= 199) /* if it's a bad one... */
		         && (number_percent() <= (raffects[test].chance - (victim->pcdata->remort_count / 10))))
			can_add = TRUE;
		else if ((raffects[test].id >= 900)
		         && (raffects[test].id <= 949) /* if it's a vuln... */
		         && (number_percent() <= (raffects[test].chance - (victim->pcdata->remort_count / 10))))
			can_add = TRUE;
		else if ((raffects[test].id >= 950)
		         && (raffects[test].id <= 999) /* if it's a res... */
		         && (number_percent() <= (raffects[test].chance + (victim->pcdata->remort_count / 10))))
			can_add = TRUE;

		if (HAS_RAFF(victim, raffects[test].id))
			can_add = FALSE;

		/* check if the group isn't 0, and then set can_add to FALSE if they have the group */
		if (raffects[test].group != 0)
			if (HAS_RAFF_GROUP(victim, raffects[test].group))
				can_add = FALSE;
	}
	while (!can_add);

	victim->pcdata->raffect[place] = raffects[test].id;

	raff_add_to_char(victim, test);

	if (ch != victim)
		ptc(ch, "({C%3d{x) {W%s{x added.\n", raffects[test].id, raffects[test].description);

	ptc(victim, "{C--- {W%s.{x\n", raffects[test].description);
}

void roll_raffects(Character *ch, Character *victim)
{
	/* not putting a lot of condition checking in this because it's only used in set and remort, we'll
	   assume that the ch and victim are pcs, ch is an imm, victim is a remort */
	int c;

	for (c = 0; c < victim->pcdata->remort_count / 10 + 1; c++)
		roll_one_raff(ch, victim, c);
}

/*****
 Extraclass Stuff
 *****/

bool HAS_EXTRACLASS(Character *ch, skill::type sn)
{
	int i;

	if (sn == skill::type::unknown)
		return FALSE;

	if (IS_NPC(ch))
		return FALSE;

	for (i = 0; i < ((ch->pcdata->remort_count / EXTRACLASS_SLOT_LEVELS) + 1); i++) {
		if (ch->pcdata->extraclass[i] == sn)
			return TRUE;
	}

	return FALSE;
}

bool CAN_USE_RSKILL(Character *ch, skill::type sn)
{
	if (IS_NPC(ch)) {
		if (skill::lookup(sn).spell_fun == spell_null)
			return FALSE;

		return TRUE;
	}

	if (IS_IMMORTAL(ch))
		return TRUE;

	if (!IS_REMORT(ch))
		return FALSE;

	if (!get_learned(ch, sn))
		return FALSE;

	if ((ch->cls + 1 != skill::lookup(sn).remort_class) && (!HAS_EXTRACLASS(ch, sn)))
		return FALSE;

	return TRUE;
}

void list_extraskill(Character *ch)
{
	String output;
	output += "\n                      {BExtraclass Remort Skills{x\n";

	for (int cn = 0; cn < MAX_CLASS; cn++) {
		if (!IS_IMMORTAL(ch))
			if (cn == ch->cls)
				continue;

		output += Format::format("\n{W%s Skills{x\n    ", class_table[cn].name.capitalize());
		int col = 0;

		for (const auto&[type, entry] : skill_table) {
			if (type == skill::type::unknown)
				continue;

			if (entry.remort_class != cn + 1)
				continue;

			if (!IS_IMMORTAL(ch)
			    && (entry.remort_class == ch->cls + 1
			        || entry.skill_level[ch->cls] <= 0
			        || entry.skill_level[ch->cls] > LEVEL_HERO))
				continue;

			output += Format::format("%-15s %s%-8d{x",
			    entry.name,
			    ch->train >= entry.rating[ch->cls] ? "{C" : "{T",
			    entry.rating[ch->cls]);

			if (++col % 3 == 0)
				output += "\n";
		}

		output += "\n";

		if (col % 3 != 0)
			output += "\n";
	}

	page_to_char(output, ch);
}

void do_eremort(Character *ch, String argument)
{
	String output;
	int x;
	skill::type sn;

	String arg1;
	argument = one_argument(argument, arg1);

	if (IS_NPC(ch)) {
		do_huh(ch);
		return;
	}

	if (IS_IMMORTAL(ch)) { /* just list em for imms */
		list_extraskill(ch);
		return;
	}

	if (ch->pcdata->remort_count < 1) {
		do_huh(ch);
		return;
	}


	if (arg1.empty()) {
		list_extraskill(ch);

		if (ch->pcdata->extraclass[0] != skill::type::unknown
		 || ch->pcdata->extraclass[1] != skill::type::unknown
		 || ch->pcdata->extraclass[2] != skill::type::unknown
		 || ch->pcdata->extraclass[3] != skill::type::unknown
		 || ch->pcdata->extraclass[4] != skill::type::unknown) {
			output += Format::format("\nYour current extraclass skill%s",
			    ch->pcdata->extraclass[1] != skill::type::unknown ? "s are" : " is");

			if (ch->pcdata->extraclass[0] != skill::type::unknown)
				output += Format::format(" %s",
				    skill::lookup(ch->pcdata->extraclass[0]).name);

			for (x = 1; x < ch->pcdata->remort_count / EXTRACLASS_SLOT_LEVELS + 1; x++)
				if (ch->pcdata->extraclass[x] != skill::type::unknown)
					output += Format::format(", %s",
					    skill::lookup(ch->pcdata->extraclass[x]).name);

			output += ".\n";
			page_to_char(output, ch);
		}

		return;
	}

	/* Ok, now we check to see if the skill is a remort skill */
	if ((sn = skill::lookup(arg1)) == skill::type::unknown) {
		stc("That is not even a valid skill, much less a remort skill.\n", ch);
		return;
	}

	/* Is it a remort skill? */
	if (skill::lookup(sn).remort_class == 0) {
		stc("That is not a remort skill.\n", ch);
		return;
	}

	/* Is it outside of the player's class? */
	if (skill::lookup(sn).remort_class == ch->cls + 1) {
		stc("You have knowledge of this skill already, pick one outside your class.\n", ch);
		return;
	}

	/* is it barred from that class? */
	if (skill::lookup(sn).skill_level[ch->cls] <= 0
	    || skill::lookup(sn).skill_level[ch->cls] > LEVEL_HERO) {
		stc("Your class cannot gain that skill.\n", ch);
		return;
	}

	/* do they have it already? */
	if (HAS_EXTRACLASS(ch, sn)) {
		stc("You already know that skill.\n", ch);
		return;
	}

	if (ch->train < skill::lookup(sn).rating[ch->cls]) {
		stc("You do not have enough training to master this skill.\n", ch);
		return;
	}

	/* find the first blank spot, and add the skill */
	for (x = 0; x < ch->pcdata->remort_count / EXTRACLASS_SLOT_LEVELS + 1; x++) {
		if (ch->pcdata->extraclass[x] == skill::type::unknown) {
			ch->pcdata->extraclass[x] = sn;

			if (get_learned(ch, sn) == 0)
				set_learned(ch, sn, 1);

			ch->train -= skill::lookup(sn).rating[ch->cls];
			ptc(ch, "You have gained %s as an extraclass remort skill.\n",
			    skill::lookup(sn).name);
			return;
		}
	}

	/* can't find an empty spot, must have the max number of extraclass skills */
	stc("You have enough extraclass remort skills!  Don't be greedy!\n", ch);
}

/* Remort Code by Lotus */
/* Redone by Endo and Montrey */
void do_remort(Character *ch, String argument)
{
	Character *victim;
	char buf[MSL];
	int race, x, c;

	String arg1, arg2, arg3;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);

	if (arg1.empty()
	    || (!arg2.empty()
	        && (arg3.empty()
	            || argument.empty()))) {
		stc("Syntax:\n"
		    "  remort <victim>   (must be remort 1 or higher)\n"
		    "  remort <victim> <race> <deity> <title>\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg1, VIS_PLR)) == nullptr) {
		stc("Hmmm...they must have ran off in fear >=).\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		stc("This can only be used on mortals.\n", ch);
		return;
	}

	if (victim->level != LEVEL_HERO) {
		stc("Only heroes can remort.\n", ch);
		return;
	}

	if (victim->pcdata->remort_count >= 99) { /* To keep who in line -Endo */
		stc("That player can no longer remort.  If you've hit this limit you need\n"
		    "to talk the coders into changing the who list.  Aren't you special?\n", ch);
		return;
	}

	if (arg2.empty() && victim->pcdata->remort_count < 1) {
		stc("Syntax:\n"
		    "  remort <victim> <race> <deity> <title>\n", ch);
		return;
	}

	/* they gotta be naked */
	for (x = 0; x < MAX_WEAR; x++) {
		if (get_eq_char(victim, x) != nullptr) {
			stc("Tell them to remove all of their eq first.\n", ch);
			return;
		}
	}

	if (arg2.empty())
		race = victim->race;
	else {
		race = race_lookup(arg2);

		if ((race = race_lookup(arg2)) == 0
		    || !race_table[race].pc_race) {
			stc("That is not a valid race.  Please choose from:\n", ch);

			for (race = 1; race < race_table.size() && race_table[race].pc_race; race++) {
				stc(race_table[race].name, ch);
				stc(" ", ch);
			}

			stc("\n", ch);
			return;
		}

		if ((victim->pcdata->remort_count + 1) < pc_race_table[race].remort_level) {
			stc("They are not experienced enough for that race.\n", ch);
			return;
		}
	}

	affect::remove_all_from_char(victim, TRUE); // racial and remort affect
	affect::remove_all_from_char(victim, FALSE); // everything else

	victim->race                    = race;

	affect::add_racial_to_char(victim);

	victim->level                   = 1;

	victim->hit = ATTR_BASE(victim, APPLY_HIT)   = 20;
	victim->mana = ATTR_BASE(victim, APPLY_MANA) = 100;
	victim->stam = ATTR_BASE(victim, APPLY_STAM) = 100;

	victim->form_flags                    = race_table[race].form;
	victim->parts_flags                   = race_table[race].parts;

	/* make sure stats aren't above their new maximums */
	for (int stat = 0; stat < MAX_STATS; stat++)
		ATTR_BASE(victim, stat_to_attr(stat))
		 = UMIN(ATTR_BASE(victim, stat_to_attr(stat)), get_max_train(victim, stat));

	if (!arg2.empty()) {
		victim->pcdata->deity = arg3;
		victim->pcdata->status = argument;
	}

	victim->pcdata->remort_count++;
	victim->exp = exp_per_level(victim, victim->pcdata->points);

	if (victim->pet != nullptr) {
		affect::remove_all_from_char(victim->pet, FALSE);

		/* About the same stats as a Kitten */
		victim->pet->level                      = 1;
		victim->pet->hit = ATTR_BASE(victim->pet, APPLY_HIT) = 20;
		victim->pet->mana = ATTR_BASE(victim->pet, APPLY_MANA) = 100;
		victim->pet->stam = ATTR_BASE(victim->pet, APPLY_STAM) = 100;
		ATTR_BASE(victim->pet, APPLY_HITROLL) = 2;
		ATTR_BASE(victim->pet, APPLY_DAMROLL) = 0;
		victim->pet->damage[DICE_NUMBER]        = 1;
		victim->pet->damage[DICE_TYPE]          = 4;
		for (int stat = 0; stat < MAX_STATS; stat++)
			ATTR_BASE(victim->pet, stat_to_attr(stat)) = 12;
		ATTR_BASE(victim->pet, APPLY_SAVES) = 0;

		for (c = 0; c < 4; c++)
			victim->pet->armor_base[c] = 100;
	}

	victim->pcdata->trains_to_hit = 0;
	victim->pcdata->trains_to_mana = 0;
	victim->pcdata->trains_to_stam = 0;

	/* clear all old raffects */
	for (c = 0; c < 10; c++)
		victim->pcdata->raffect[c] = 0;

	/* clear all old extraclass skills */
	for (c = 0; c < 5; c++)
		victim->pcdata->extraclass[c] = skill::type::unknown;

	stc("Your deity bestows upon you...\n", victim);
	roll_raffects(ch, victim);

	if (victim->pcdata->remort_count == 1)
		victim->pcdata->plr_flags += PLR_SHOWRAFF;

	Format::sprintf(buf, "%s has been reborn!", victim->name);
	do_send_announce(victim, buf);
	stc("You suddenly feel like a newbie!! Do'h!!!\n", victim);
	stc("Successful Remort.\n", ch);
}

