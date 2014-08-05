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
#include "magic.h"


void do_heal(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char *words;
	CHAR_DATA *mob;
	CHAR_DATA *rch;
	SPELL_FUN *spell;
	int cost, sn;

	/* check for healer */
	for (mob = ch->in_room->people; mob; mob = mob->next_in_room) {
		if (IS_NPC(mob) && IS_SET(mob->act, ACT_IS_HEALER)) break;
	}

	if (mob == NULL) {
		stc("You can't do that here.\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		/* display price list */
		act("$N says 'I offer the following spells:'", ch, NULL, mob, TO_CHAR);
		stc("  light: cure light wounds       5 gold\n\r", ch);
		stc("  serious: cure serious wounds  10 gold\n\r", ch);
		stc("  critic: cure critical wounds  20 gold\n\r", ch);
		stc("  heal: healing spell           30 gold\n\r", ch);
		stc("  blind: cure blindness          1 gold\n\r", ch);
		stc("  disease: cure disease          1 gold\n\r", ch);
		stc("  poison:  cure poison           1 gold\n\r", ch);
		stc("  uncurse: remove curse          1 gold\n\r", ch);
		stc("  refresh: restore stamina      15 gold\n\r", ch);
		stc("  mana:  restore mana           35 gold\n\r", ch);
		stc("  Type heal <type> to be healed.\n\r", ch);
		return;
	}

	if (!str_prefix1(arg, "light")) {
		spell = spell_cure_light;
		sn    = gsn_cure_light;
		words = "judicandus dies";
		cost  = 500;
	}
	else if (!str_prefix1(arg, "serious")) {
		spell = spell_cure_serious;
		sn    = gsn_cure_serious;
		words = "judicandus gzfuajg";
		cost  = 1000;
	}
	else if (!str_prefix1(arg, "critical")) {
		spell = spell_cure_critical;
		sn    = gsn_cure_critical;
		words = "judicandus qfuhuqar";
		cost  = 2000;
	}
	else if (!str_prefix1(arg, "heal")) {
		spell = spell_heal;
		sn = gsn_heal;
		words = "Parr";
		cost  = 3000;
	}
	else if (!str_prefix1(arg, "blindness")) {
		spell = spell_cure_blindness;
		sn    = gsn_cure_blindness;
		words = "judicandus noselacri";
		cost  = 100;
	}
	else if (!str_prefix1(arg, "disease")) {
		spell = spell_cure_disease;
		sn    = gsn_cure_disease;
		words = "judicandus eugzagz";
		cost = 100;
	}
	else if (!str_prefix1(arg, "poison")) {
		spell = spell_cure_poison;
		sn    = gsn_cure_poison;
		words = "judicandus sausabru";
		cost  = 100;
	}
	else if (!str_prefix1(arg, "uncurse") || !str_prefix1(arg, "curse")) {
		spell = spell_remove_curse;
		sn    = gsn_remove_curse;
		words = "candussido judifgz";
		cost  = 100;
	}
	else if (!str_prefix1(arg, "mana") || !str_prefix1(arg, "energize")) {
		spell = NULL;
		sn = -1;
		words = "energizer";
		cost = 3500;
	}
	else if (!str_prefix1(arg, "refresh") || !str_prefix1(arg, "stamina")) {
		spell =  spell_refresh;
		sn    = gsn_refresh;
		words = "candusima";
		cost  = 1500;
	}
	else {
		act("$N says 'Type 'heal' for a list of spells.'", ch, NULL, mob, TO_CHAR);
		return;
	}

	if (cost > (ch->gold * 100 + ch->silver)) {
		act("$N says 'You do not have enough gold for my services.'", ch, NULL, mob, TO_CHAR);
		return;
	}

	WAIT_STATE(ch, PULSE_VIOLENCE);
	deduct_cost(ch, cost);
	mob->gold += cost / 100;

	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if (number_percent() < get_skill(rch, gsn_languages)) {
			sprintf(buf, "$n utters the words '%s'.", arg);
			act(buf, mob, NULL, rch, TO_VICT);
			check_improve(rch, gsn_languages, TRUE, 8);
		}
		else {
			sprintf(buf, "$n utters the words '%s'.", words);
			act(buf, mob, NULL, rch, TO_VICT);

			if (get_skill(rch, gsn_languages))
				check_improve(rch, gsn_languages, FALSE, 8);
		}
	}

	if (spell == NULL) { /* restore mana trap...kinda hackish */
		ch->mana += dice(9, 10) + mob->level / 2;
		ch->mana = UMIN(ch->mana, ch->max_mana);
		stc("A warm glow passes through you.\n\r", ch);
		return;
	}

	if (sn == -1) return;

	/* healer casts at evolution 1 currently */
	spell(sn, mob->level, mob, ch, TARGET_CHAR, 1);
}
