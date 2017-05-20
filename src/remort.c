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
#include "recycle.h"
#include "magic.h"
#include "affect.h"

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

void fix_blank_raff(CHAR_DATA *ch, int start)
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

void rem_raff_affect(CHAR_DATA *ch, int index)
{
	if (raffects[index].add) {
		if ((raffects[index].id >= 900) && (raffects[index].id <= 949))
			remort_affect_modify_char(ch, TO_VULN, raffects[index].add, FALSE);
		else if ((raffects[index].id >= 950) && (raffects[index].id <= 999))
			remort_affect_modify_char(ch, TO_RESIST, raffects[index].add, FALSE);
	}

	return;
}

bool HAS_RAFF(CHAR_DATA *ch, int flag)
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

bool HAS_RAFF_GROUP(CHAR_DATA *ch, int flag)
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

void raff_add_to_char(CHAR_DATA *ch, int raff_id) {
	int index;

	for (index = 1; index < MAX_RAFFECTS; index++)
		if (raffects[index].id == raff_id)
			break;

	if (index == MAX_RAFFECTS) {
		bugf("raff_add_to_char: invalid raffect ID %d", raff_id);
		return;
	}

	if (raffects[index].add != 0) {
		if ((raffects[index].id >= 900) && (raffects[index].id <= 949))
			remort_affect_modify_char(ch, TO_VULN, raffects[index].add, TRUE);
		else if ((raffects[index].id >= 950) && (raffects[index].id <= 999))
			remort_affect_modify_char(ch, TO_RESIST, raffects[index].add, TRUE);
	}
}

void roll_one_raff(CHAR_DATA *ch, CHAR_DATA *victim, int place)
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
		         && (number_percent() <= (raffects[test].chance + (victim->pcdata->remort_count / 10)))
		         && (GET_DEFENSE_MOD(victim, raffects[test].add) < 20)) // don't increase a resistance to immunity
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
		ptc(ch, "({C%3d{x) {W%s{x added.\n", raffects[test].id, str_dup(raffects[test].description));

	ptc(victim, "{C--- {W%s.{x\n", str_dup(raffects[test].description));
}

void roll_raffects(CHAR_DATA *ch, CHAR_DATA *victim)
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

bool HAS_EXTRACLASS(CHAR_DATA *ch, int sn)
{
	int i;

	if (sn <= 0)
		return FALSE;

	if (IS_NPC(ch))
		return FALSE;

	for (i = 0; i < ((ch->pcdata->remort_count / EXTRACLASS_SLOT_LEVELS) + 1); i++) {
		if (ch->pcdata->extraclass[i] == sn)
			return TRUE;
	}

	return FALSE;
}

bool CAN_USE_RSKILL(CHAR_DATA *ch, int sn)
{
	if (IS_NPC(ch)) {
		if (skill_table[sn].spell_fun == spell_null)
			return FALSE;

		return TRUE;
	}

	if (IS_IMMORTAL(ch))
		return TRUE;

	if (!IS_REMORT(ch))
		return FALSE;

	if (!get_skill(ch, sn))
		return FALSE;

	if ((ch->cls + 1 != skill_table[sn].remort_class) && (!HAS_EXTRACLASS(ch, sn)))
		return FALSE;

	return TRUE;
}

void list_extraskill(CHAR_DATA *ch)
{
	BUFFER *output;
	int sn, cn, col;
	output = new_buf();
	add_buf(output, "\n                      {BExtraclass Remort Skills{x\n");

	for (cn = 0; cn < MAX_CLASS; cn++) {
		if (!IS_IMMORTAL(ch))
			if (cn == ch->cls)
				continue;

		ptb(output, "\n{W%s Skills{x\n    ", capitalize(class_table[cn].name));

		for (sn = 0, col = 0; skill_table[sn].name != NULL; sn++) {
			if (skill_table[sn].remort_class != cn + 1)
				continue;

			if (!IS_IMMORTAL(ch)
			    && (skill_table[sn].remort_class == ch->cls + 1
			        || skill_table[sn].skill_level[ch->cls] <= 0
			        || skill_table[sn].skill_level[ch->cls] > LEVEL_HERO))
				continue;

			ptb(output, "%-15s %s%-8d{x",
			    skill_table[sn].name,
			    ch->train >= skill_table[sn].rating[ch->cls] ? "{C" : "{T",
			    skill_table[sn].rating[ch->cls]);

			if (++col % 3 == 0)
				add_buf(output, "\n");
		}

		add_buf(output, "\n");

		if (col % 3 != 0)
			add_buf(output, "\n");
	}

	page_to_char(buf_string(output), ch);
	free_buf(output);
}

void do_eremort(CHAR_DATA *ch, const char *argument)
{
	char arg1[MIL];
	BUFFER *output;
	int x, sn = 0;
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

	output = new_buf();

	if (arg1[0] == '\0') {
		list_extraskill(ch);

		if (ch->pcdata->extraclass[0] +
		    ch->pcdata->extraclass[1] +
		    ch->pcdata->extraclass[2] +
		    ch->pcdata->extraclass[3] +
		    ch->pcdata->extraclass[4] > 0) {
			ptb(output, "\nYour current extraclass skill%s",
			    ch->pcdata->extraclass[1] ? "s are" : " is");

			if (ch->pcdata->extraclass[0])
				ptb(output, " %s",
				    skill_table[ch->pcdata->extraclass[0]].name);

			for (x = 1; x < ch->pcdata->remort_count / EXTRACLASS_SLOT_LEVELS + 1; x++)
				if (ch->pcdata->extraclass[x])
					ptb(output, ", %s",
					    skill_table[ch->pcdata->extraclass[x]].name);

			add_buf(output, ".\n");
			page_to_char(buf_string(output), ch);
			free_buf(output);
		}

		return;
	}

	/* Ok, now we check to see if the skill is a remort skill */
	if ((sn = skill_lookup(arg1)) < 0) {
		stc("That is not even a valid skill, much less a remort skill.\n", ch);
		return;
	}

	/* Is it a remort skill? */
	if (skill_table[sn].remort_class == 0) {
		stc("That is not a remort skill.\n", ch);
		return;
	}

	/* Is it outside of the player's class? */
	if (skill_table[sn].remort_class == ch->cls + 1) {
		stc("You have knowledge of this skill already, pick one outside your class.\n", ch);
		return;
	}

	/* is it barred from that class? */
	if (skill_table[sn].skill_level[ch->cls] <= 0
	    || skill_table[sn].skill_level[ch->cls] > LEVEL_HERO) {
		stc("Your class cannot gain that skill.\n", ch);
		return;
	}

	/* do they have it already? */
	if (HAS_EXTRACLASS(ch, sn)) {
		stc("You already know that skill.\n", ch);
		return;
	}

	if (ch->train < skill_table[sn].rating[ch->cls]) {
		stc("You do not have enough training to master this skill.\n", ch);
		return;
	}

	/* find the first blank spot, and add the skill */
	for (x = 0; x < ch->pcdata->remort_count / EXTRACLASS_SLOT_LEVELS + 1; x++) {
		if (!ch->pcdata->extraclass[x]) {
			ch->pcdata->extraclass[x] = sn;

			if (!ch->pcdata->learned[sn])
				ch->pcdata->learned[sn] = 1;

			ch->train -= skill_table[sn].rating[ch->cls];
			ptc(ch, "You have gained %s as an extraclass remort skill.\n",
			    skill_table[sn].name);
			return;
		}
	}

	/* can't find an empty spot, must have the max number of extraclass skills */
	stc("You have enough extraclass remort skills!  Don't be greedy!\n", ch);
}

/* Remort Code by Lotus */
/* Redone by Endo and Montrey */
void do_remort(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	char arg1[MIL], arg2[MIL], arg3[MIL], buf[MSL];
	int race, x, c;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);

	if (arg1[0] == '\0'
	    || (arg2[0] != '\0'
	        && (arg3[0] == '\0'
	            || argument[0] == '\0'))) {
		stc("Syntax:\n"
		    "  remort <victim>   (must be remort 1 or higher)\n"
		    "  remort <victim> <race> <deity> <title>\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg1, VIS_PLR)) == NULL) {
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

	if (arg2[0] == '\0' && victim->pcdata->remort_count < 1) {
		stc("Syntax:\n"
		    "  remort <victim> <race> <deity> <title>\n", ch);
		return;
	}

	/* they gotta be naked */
	for (x = 0; x < MAX_WEAR; x++) {
		if (get_eq_char(victim, x) != NULL) {
			stc("Tell them to remove all of their eq first.\n", ch);
			return;
		}
	}

	if (arg2[0] == '\0')
		race = victim->race;
	else {
		race = race_lookup(arg2);

		if ((race = race_lookup(arg2)) == 0
		    || !race_table[race].pc_race) {
			stc("That is not a valid race.  Please choose from:\n", ch);

			for (race = 1; race_table[race].name != NULL && race_table[race].pc_race; race++) {
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

	affect_remove_all_from_char(victim, TRUE); // racial and remort affect
	affect_remove_all_from_char(victim, FALSE); // everything else

	victim->race                    = race;

	affect_add_racial_to_char(victim);

	victim->level                   = 1;

	victim->hit = ATTR_BASE(victim, APPLY_HIT)   = 20;
	victim->mana = ATTR_BASE(victim, APPLY_MANA) = 100;
	victim->stam = ATTR_BASE(victim, APPLY_STAM) = 100;

	victim->form                    = race_table[race].form;
	victim->parts                   = race_table[race].parts;

	/* make sure stats aren't above their new maximums */
	for (int stat = 0; stat < MAX_STATS; stat++)
		ATTR_BASE(victim, stat_to_attr(stat))
		 = UMIN(ATTR_BASE(victim, stat_to_attr(stat)), get_max_train(victim, stat));

	if (arg2[0] != '\0') {
		free_string(victim->pcdata->deity);
		free_string(victim->pcdata->status);
		victim->pcdata->deity = str_dup(arg3);
		victim->pcdata->status = str_dup(argument);
	}

	victim->pcdata->remort_count++;
	victim->exp = exp_per_level(victim, victim->pcdata->points);

	if (victim->pet != NULL) {
		affect_remove_all_from_char(victim->pet, FALSE);

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
		victim->pcdata->extraclass[c] = 0;

	stc("Your deity bestows upon you...\n", victim);
	roll_raffects(ch, victim);

	if (victim->pcdata->remort_count == 1)
		SET_BIT(victim->pcdata->plr, PLR_SHOWRAFF);

	sprintf(buf, "%s has been reborn!", victim->name);
	do_send_announce(victim, buf);
	stc("You suddenly feel like a newbie!! Do'h!!!\n", victim);
	stc("Successful Remort.\n", ch);
}

