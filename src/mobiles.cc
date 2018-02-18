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

#include "act.hh"
#include "argument.hh"
#include "Character.hh"
#include "declare.hh"
#include "Flags.hh"
#include "Format.hh"
#include "macros.hh"
#include "magic.hh"
#include "memory.hh"
#include "merc.hh"
#include "Player.hh"
#include "random.hh"
#include "RoomPrototype.hh"
#include "String.hh"

void do_heal(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	char *words;
	Character *mob;
	Character *rch;
	SPELL_FUN *spell;
	int cost;
	skill::type sn = skill::type::unknown;

	/* check for healer */
	for (mob = ch->in_room->people; mob; mob = mob->next_in_room) {
		if (IS_NPC(mob) && mob->act_flags.has(ACT_IS_HEALER)) break;
	}

	if (mob == nullptr) {
		stc("You can't do that here.\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		/* display price list */
		act("$N says 'I offer the following spells:'", ch, nullptr, mob, TO_CHAR);
		stc("  light: cure light wounds       5 gold\n", ch);
		stc("  serious: cure serious wounds  10 gold\n", ch);
		stc("  critic: cure critical wounds  20 gold\n", ch);
		stc("  heal: healing spell           30 gold\n", ch);
		stc("  blind: cure blindness          1 gold\n", ch);
		stc("  disease: cure disease          1 gold\n", ch);
		stc("  poison:  cure poison           1 gold\n", ch);
		stc("  uncurse: remove curse          1 gold\n", ch);
		stc("  refresh: restore stamina      15 gold\n", ch);
		stc("  mana:  restore mana           35 gold\n", ch);
		stc("  Type heal <type> to be healed.\n", ch);
		return;
	}

	if (arg.is_prefix_of("light")) {
		spell = spell_cure_light;
		sn    = skill::type::cure_light;
		words = "judicandus dies";
		cost  = 500;
	}
	else if (arg.is_prefix_of("serious")) {
		spell = spell_cure_serious;
		sn    = skill::type::cure_serious;
		words = "judicandus gzfuajg";
		cost  = 1000;
	}
	else if (arg.is_prefix_of("critical")) {
		spell = spell_cure_critical;
		sn    = skill::type::cure_critical;
		words = "judicandus qfuhuqar";
		cost  = 2000;
	}
	else if (arg.is_prefix_of("heal")) {
		spell = spell_heal;
		sn = skill::type::heal;
		words = "Parr";
		cost  = 3000;
	}
	else if (arg.is_prefix_of("blindness")) {
		spell = spell_cure_blindness;
		sn    = skill::type::cure_blindness;
		words = "judicandus noselacri";
		cost  = 100;
	}
	else if (arg.is_prefix_of("disease")) {
		spell = spell_cure_disease;
		sn    = skill::type::cure_disease;
		words = "judicandus eugzagz";
		cost = 100;
	}
	else if (arg.is_prefix_of("poison")) {
		spell = spell_cure_poison;
		sn    = skill::type::cure_poison;
		words = "judicandus sausabru";
		cost  = 100;
	}
	else if (arg.is_prefix_of("uncurse") || arg.is_prefix_of("curse")) {
		spell = spell_remove_curse;
		sn    = skill::type::remove_curse;
		words = "candussido judifgz";
		cost  = 100;
	}
	else if (arg.is_prefix_of("mana") || arg.is_prefix_of("energize")) {
		spell = nullptr;
		sn = skill::type::unknown;
		words = "energizer";
		cost = 3500;
	}
	else if (arg.is_prefix_of("refresh") || arg.is_prefix_of("stamina")) {
		spell =  spell_refresh;
		sn    = skill::type::refresh;
		words = "candusima";
		cost  = 1500;
	}
	else {
		act("$N says 'Type 'heal' for a list of spells.'", ch, nullptr, mob, TO_CHAR);
		return;
	}

	if (cost > (ch->gold * 100 + ch->silver)) {
		act("$N says 'You do not have enough gold for my services.'", ch, nullptr, mob, TO_CHAR);
		return;
	}

	WAIT_STATE(ch, PULSE_VIOLENCE);
	deduct_cost(ch, cost);
	mob->gold += cost / 100;

	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if (number_percent() < get_learned(rch, skill::type::languages)) {
			Format::sprintf(buf, "$n utters the words '%s'.", arg);
			act(buf, mob, nullptr, rch, TO_VICT);
			check_improve(rch, skill::type::languages, TRUE, 8);
		}
		else {
			Format::sprintf(buf, "$n utters the words '%s'.", words);
			act(buf, mob, nullptr, rch, TO_VICT);

			if (get_learned(rch, skill::type::languages))
				check_improve(rch, skill::type::languages, FALSE, 8);
		}
	}

	if (spell == nullptr) { /* restore mana trap...kinda hackish */
		ch->mana += dice(9, 10) + mob->level / 2;
		ch->mana = UMIN(ch->mana, GET_MAX_MANA(ch));
		stc("A warm glow passes through you.\n", ch);
		return;
	}

	if (sn == skill::type::unknown) return;

	/* healer casts at evolution 1 currently */
	spell(sn, mob->level, mob, ch, TARGET_CHAR, 1);
}
