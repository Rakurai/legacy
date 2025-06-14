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
 **************************************************************************/

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
#include "affect/Affect.hh"
#include "Area.hh"
#include "Character.hh"
#include "declare.hh"
#include "find.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "GameTime.hh"
#include "interp.hh"
#include "lookup.hh"
#include "magic.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"
#include "Player.hh"
#include "random.hh"
#include "Room.hh"
#include "String.hh"
#include "World.hh"

DECLARE_SPELL_FUN(spell_identify);
DECLARE_SPELL_FUN(spell_charm_person);

void     obj_repair      args((Character *ch, Object *obj));

/*
 * The following special functions are available for mobiles.
 */
DECLARE_SPEC_FUN(spec_breath_any);
DECLARE_SPEC_FUN(spec_breath_acid);
DECLARE_SPEC_FUN(spec_breath_fire);
DECLARE_SPEC_FUN(spec_breath_frost);
DECLARE_SPEC_FUN(spec_breath_gas);
DECLARE_SPEC_FUN(spec_breath_lightning);
DECLARE_SPEC_FUN(spec_cast_adept);
DECLARE_SPEC_FUN(spec_cast_cleric);
DECLARE_SPEC_FUN(spec_cast_judge);
DECLARE_SPEC_FUN(spec_cast_mage);
DECLARE_SPEC_FUN(spec_cast_undead);
DECLARE_SPEC_FUN(spec_executioner);
DECLARE_SPEC_FUN(spec_fido);
DECLARE_SPEC_FUN(spec_guard);
DECLARE_SPEC_FUN(spec_janitor);
DECLARE_SPEC_FUN(spec_mayor);
DECLARE_SPEC_FUN(spec_poison);
DECLARE_SPEC_FUN(spec_thief);
DECLARE_SPEC_FUN(spec_nasty);
DECLARE_SPEC_FUN(spec_troll_member);
DECLARE_SPEC_FUN(spec_ogre_member);
DECLARE_SPEC_FUN(spec_patrolman);
DECLARE_SPEC_FUN(spec_questmaster);                  /* Vassago */
DECLARE_SPEC_FUN(spec_squestmaster);          /* Montrey */
DECLARE_SPEC_FUN(spec_sage);
DECLARE_SPEC_FUN(spec_blacksmith);
DECLARE_SPEC_FUN(spec_charm);
DECLARE_SPEC_FUN(spec_clanguard);
DECLARE_SPEC_FUN(spec_fight_clanguard);

/* the function table */
const std::vector<spec_type> spec_table = {
	{   "spec_breath_any",              spec_breath_any         },
	{   "spec_breath_acid",             spec_breath_acid        },
	{   "spec_breath_fire",             spec_breath_fire        },
	{   "spec_breath_frost",            spec_breath_frost       },
	{   "spec_breath_gas",              spec_breath_gas         },
	{   "spec_breath_lightning",        spec_breath_lightning   },
	{   "spec_cast_adept",              spec_cast_adept         },
	{   "spec_cast_cleric",             spec_cast_cleric        },
	{   "spec_cast_judge",              spec_cast_judge         },
	{   "spec_cast_mage",               spec_cast_mage          },
	{   "spec_cast_undead",             spec_cast_undead        },
	{   "spec_executioner",             spec_executioner        },
	{   "spec_fido",                    spec_fido               },
	{   "spec_guard",                   spec_guard              },
	{   "spec_janitor",                 spec_janitor            },
	{   "spec_mayor",                   spec_mayor              },
	{   "spec_poison",                  spec_poison             },
	{   "spec_thief",                   spec_thief              },
	{   "spec_nasty",                   spec_nasty              },
	{   "spec_troll_member",            spec_troll_member       },
	{   "spec_ogre_member",             spec_ogre_member        },
	{   "spec_patrolman",               spec_patrolman          },
	{   "spec_questmaster",             spec_questmaster        },
	{   "spec_squestmaster",            spec_squestmaster       },
	{   "spec_sage",                    spec_sage               },
	{   "spec_blacksmith",              spec_blacksmith         },
	{   "spec_charm",                   spec_charm              },
	{   "spec_clanguard",               spec_clanguard          },
	{   "spec_fight_clanguard",         spec_fight_clanguard    },
};

/* Given a name, return the appropriate spec fun. */
SPEC_FUN *spec_lookup(const String& name)
{
	for (const auto& entry : spec_table)
		if (tolower(name[0]) == tolower(entry.name[0]) && name.is_prefix_of(entry.name))
			return entry.function;

	return 0;
}

String spec_name(SPEC_FUN *function)
{
	for (const auto& entry : spec_table)
		if (function == entry.function)
			return entry.name;

	return "(none)";
}

bool spec_troll_member(Character *ch)
{
	Character *vch, *victim = nullptr;
	int count = 0;
	char *message;

	if (!IS_AWAKE(ch) || affect::exists_on_char(ch, affect::type::calm) || ch->in_room == nullptr
	    || affect::exists_on_char(ch, affect::type::charm_person) || ch->fighting != nullptr)
		return false;

	/* find an ogre to beat up */
	for (vch = ch->in_room->people;  vch != nullptr;  vch = vch->next_in_room) {
		if (!vch->is_npc() || ch == vch)
			continue;

		if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN)
			return false;

		if (vch->pIndexData->group_flags.has(GROUP_VNUM_OGRES) &&  ch->level > vch->level - 2 && !is_safe(ch, vch, true)) {
			if (number_range(0, count) == 0)
				victim = vch;

			count++;
		}
	}

	if (victim == nullptr)
		return false;

	/* say something, then raise hell */
	switch (number_range(0, 6)) {
	default: message = "$n yells 'I've been looking for you, punk!'";
		break;

	case 1: message = "With a scream of rage, $n attacks $N.";
		break;

	case 2: message = "$n says 'What's slimy Ogre trash like you doing around here?'";
		break;

	case 3: message = "$n cracks his knuckles and says 'Do ya feel lucky?'";
		break;

	case 4: message = "$n says 'There's no cops to save you this time!'";
		break;

	case 5: message = "$n says 'Time to join your brother, spud.'";
		break;

	case 6: message = "$n says 'Let's rock.'";
		break;
	}

	act(message, ch, nullptr, victim, TO_ALL);
	multi_hit(ch, victim, skill::type::unknown);
	return true;
}

bool spec_ogre_member(Character *ch)
{
	Character *vch, *victim = nullptr;
	int count = 0;
	char *message;

	if (!IS_AWAKE(ch) || affect::exists_on_char(ch, affect::type::calm) || ch->in_room == nullptr
	    ||  affect::exists_on_char(ch, affect::type::charm_person) || ch->fighting != nullptr)
		return false;

	/* find an troll to beat up */
	for (vch = ch->in_room->people;  vch != nullptr;  vch = vch->next_in_room) {
		if (!vch->is_npc() || ch == vch)
			continue;

		if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN)
			return false;

		if (vch->pIndexData->group_flags.has(GROUP_VNUM_TROLLS) && ch->level > vch->level - 2 && !is_safe(ch, vch, true)) {
			if (number_range(0, count) == 0)
				victim = vch;

			count++;
		}
	}

	if (victim == nullptr)
		return false;

	/* say something, then raise hell */
	switch (number_range(0, 6)) {
	default: message = "$n yells 'I've been looking for you, punk!'";
		break;

	case 1: message = "With a scream of rage, $n attacks $N.'";
		break;

	case 2: message = "$n says 'What's Troll filth like you doing around here?'";
		break;

	case 3: message = "$n cracks his knuckles and says 'Do ya feel lucky?'";
		break;

	case 4: message = "$n says 'There's no cops to save you this time!'";
		break;

	case 5: message = "$n says 'Time to join your brother, spud.'";
		break;

	case 6: message = "$n says 'Let's rock.'";
		break;
	}

	act(message, ch, nullptr, victim, TO_ALL);
	multi_hit(ch, victim, skill::type::unknown);
	return true;
}

bool spec_patrolman(Character *ch)
{
	Character *victim = nullptr;
	Object *obj;
	char *message;
	int count = 0;

	if (!IS_AWAKE(ch) || affect::exists_on_char(ch, affect::type::calm) || ch->in_room == nullptr
	    ||  affect::exists_on_char(ch, affect::type::charm_person) || ch->fighting != nullptr)
		return false;

	/* look for a fight in the room */
	for (Character *vch = ch->in_room->people; vch != nullptr; vch = vch->next_in_room) {
		if (vch == ch)
			continue;

		if (vch->fighting != nullptr) { /* break it up! */
			if (number_range(0, count) == 0)
				victim = (vch->level > vch->fighting->level)
				         ? vch : vch->fighting;

			count++;
		}
	}

	if (victim == nullptr || (victim->is_npc() && victim->spec_fun == ch->spec_fun))
		return false;

	if (((obj = get_eq_char(ch, WEAR_NECK_1)) != nullptr
	     &&   obj->pIndexData->vnum == OBJ_VNUM_WHISTLE)
	    || ((obj = get_eq_char(ch, WEAR_NECK_2)) != nullptr
	        &&   obj->pIndexData->vnum == OBJ_VNUM_WHISTLE)) {
		act("You blow down hard on $p.", ch, obj, nullptr, TO_CHAR);
		act("$n blows on $p, ***WHEEEEEEEEEEEET***", ch, obj, nullptr, TO_ROOM);

		for (auto vch : Game::world().char_list) {
			if (vch->in_room == nullptr)
				continue;

			if (vch->in_room != ch->in_room
			    &&  vch->in_room->area() == ch->in_room->area())
				stc("You hear a shrill whistling sound.\n", vch);
		}
	}

	switch (number_range(0, 6)) {
	default:        message = nullptr;         break;

	case 0: message = "$n yells 'All roit! All roit! break it up!'";
		break;

	case 1: message =
		        "$n says 'Society's to blame, but what's a bloke to do?'";
		break;

	case 2: message =
		        "$n mumbles 'bloody kids will be the death of us all.'";
		break;

	case 3: message = "$n shouts 'Stop that! Stop that!' and attacks.";
		break;

	case 4: message = "$n pulls out his billy and goes to work.";
		break;

	case 5: message =
		        "$n sighs in resignation and proceeds to break up the fight.";
		break;

	case 6: message = "$n says 'Settle down, you hooligans!'";
		break;
	}

	if (message != nullptr)
		act(message, ch, nullptr, nullptr, TO_ALL);

	multi_hit(ch, victim, skill::type::unknown);
	return true;
}
bool spec_questmaster(Character *ch)
{
	if (ch->fighting != nullptr) return spec_cast_mage(ch);

	return false;
}

bool spec_squestmaster(Character *ch)
{
	if (ch->fighting != nullptr)
		return spec_cast_mage(ch);

	return false;
}

bool spec_blacksmith(Character *ch)
{
	if (ch->fighting != nullptr) return spec_cast_mage(ch);

	return false;
}
bool spec_sage(Character *ch)
{
	if (ch->fighting != nullptr) return spec_cast_mage(ch);

	return false;
}

bool spec_nasty(Character *ch)
{
	Character *victim, *v_next;
	long gold;

	if (!IS_AWAKE(ch))
		return false;

	if (!ch->fighting) {
		for (victim = ch->in_room->people; victim != nullptr; victim = v_next) {
			v_next = victim->next_in_room;

			if (!victim->is_npc()
			    && (victim->level > ch->level)
			    && (victim->level < ch->level + 10)) {
				do_backstab(ch, victim->name);

				if (!ch->fighting)
					do_kill(ch, victim->name);

				/* should steal some coins right away? :) */
				return true;
			}
		}

		return false;    /*  No one to attack */
	}

	/* okay, we must be fighting.... steal some coins and flee */
	if ((victim = ch->fighting) == nullptr)
		return false;   /* let's be paranoid.... */

	switch (number_bits(2)) {
	case 0:  act("$n rips apart your coin purse, spilling your gold!",
		             ch, nullptr, victim, TO_VICT);
		act("You slash apart $N's coin purse and gather his gold.",
		    ch, nullptr, victim, TO_CHAR);
		act("$N's coin purse is ripped apart!",
		    ch, nullptr, victim, TO_NOTVICT);
		gold = victim->gold / 10;  /* steal 10% of his gold */
		victim->gold -= gold;
		ch->gold     += gold;
		return true;

	case 1:  do_flee(ch, "");
		return true;

	default: return false;
	}
}

/* Core procedure for dragons. */
bool dragon(Character *ch, skill::type sn)
{
	Character *victim;

	if (get_position(ch) != POS_FIGHTING)
		return false;

	for (victim = ch->in_room->people; victim != nullptr; victim = victim->next_in_room)
		if (victim->fighting == ch
		    && can_see_char(ch, victim)
		    && number_bits(3) == 0)
			break;

	if (victim == nullptr)
		return false;

	/* made it so mobs won't keep breathing after mana runs out, but just using the min mana -- Montrey */
	if (ch->mana < skill::lookup(sn).min_mana)
		return false;

	(*skill::lookup(sn).spell_fun)
	(sn, ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, sn));
	return true;
}

/* Special procedures for mobiles. */
bool spec_breath_any(Character *ch)
{
	switch (number_bits(3)) {
	case 0:
	case 1: return dragon(ch, skill::type::lightning_breath);

	case 2:
	case 3:
	case 4: return dragon(ch, skill::type::frost_breath);

	case 5: return dragon(ch, skill::type::fire_breath);

	case 6: return dragon(ch, skill::type::gas_breath);

	case 7: return dragon(ch, skill::type::acid_breath);
	}

	return false;
}

bool spec_breath_acid(Character *ch)
{
	return dragon(ch, skill::type::acid_breath);
}

bool spec_breath_fire(Character *ch)
{
	return dragon(ch, skill::type::fire_breath);
}

bool spec_breath_frost(Character *ch)
{
	return dragon(ch, skill::type::frost_breath);
}

bool spec_breath_gas(Character *ch)
{
	return dragon(ch, skill::type::gas_breath);
}

bool spec_breath_lightning(Character *ch)
{
	return dragon(ch, skill::type::lightning_breath);
}

bool spec_cast_adept(Character *ch)
{
	Character *victim;
	Character *v_next;

	if (!IS_AWAKE(ch))
		return false;

	for (victim = ch->in_room->people; victim != nullptr; victim = v_next) {
		v_next = victim->next_in_room;

		if (victim != ch && can_see_char(ch, victim) && number_bits(1) == 0
		    && !victim->is_npc() && victim->level < 11)
			break;
	}

	if (victim == nullptr)
		return false;

	switch (number_bits(4)) {
	case 0:
#if (defined(HALLOWEEN))
		act("$n utters the word 'bhiae waai'.", ch, nullptr, nullptr, TO_ROOM);
		spell_blood_moon(skill::type::blood_moon, ch->level, ch, victim, TARGET_CHAR,
		                 get_evolution(ch, skill::type::blood_moon));
#else
		act("$n utters the word 'abrazak'.", ch, nullptr, nullptr, TO_ROOM);
		spell_armor(skill::type::armor, ch->level, ch, victim, TARGET_CHAR,
		            get_evolution(ch, skill::type::armor));
#endif
		return true;

	case 1:
		act("$n utters the word 'fido'.", ch, nullptr, nullptr, TO_ROOM);
		spell_bless(skill::type::bless, ch->level, ch, victim, TARGET_CHAR,
		            get_evolution(ch, skill::type::bless));
		return true;

	case 2:
		act("$n utters the words 'judicandus noselacri'.", ch, nullptr, nullptr, TO_ROOM);
		spell_cure_blindness(skill::type::cure_blindness,
		                     ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, skill::type::cure_blindness));
		return true;

	case 3:
		act("$n utters the words 'judicandus dies'.", ch, nullptr, nullptr, TO_ROOM);
		spell_cure_light(skill::type::cure_light,
		                 ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, skill::type::cure_light));
		return true;

	case 4:
		act("$n utters the words 'judicandus sausabru'.", ch, nullptr, nullptr, TO_ROOM);
		spell_cure_poison(skill::type::cure_poison,
		                  ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, skill::type::cure_poison));
		return true;

	case 5:
		act("$n utters the word 'candusima'.", ch, nullptr, nullptr, TO_ROOM);
		spell_refresh(skill::type::refresh, ch->level, ch, victim, TARGET_CHAR,
		              get_evolution(ch, skill::type::refresh));
		return true;

	case 6:
		act("$n utters the words 'judicandus eugzagz'.", ch, nullptr, nullptr, TO_ROOM);
		spell_cure_disease(skill::type::cure_disease,
		                   ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, skill::type::cure_disease));
	}

	return false;
}

bool spec_cast_judge(Character *ch)
{
	Character *victim;
	Character *v_next;
	char *spell;
	skill::type sn;

	if (get_position(ch) != POS_FIGHTING)
		return false;

	for (victim = ch->in_room->people; victim != nullptr; victim = v_next) {
		v_next = victim->next_in_room;

		if (victim->fighting == ch && number_bits(2) == 0)
			break;
	}

	if (victim == nullptr)
		return false;

	spell = "high explosive";

	if ((sn = skill::lookup(spell)) == skill::type::unknown)
		return false;

	(*skill::lookup(sn).spell_fun)(sn, ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, sn));
	return true;
}

bool spec_cast_cleric(Character *ch)
{
	Character *victim;
	skill::type sn;
	int i;

	if (get_position(ch) != POS_FIGHTING)
		return false;

	for (victim = ch->in_room->people; victim != nullptr; victim = victim->next_in_room)
		if (victim->fighting == ch && number_bits(2) == 0)
			break;

	if (victim == nullptr)
		return false;

	for (i = 0; ; i++) {
		if (i == 10)
			return false;

		switch (number_bits(4)) {
		case  0:        sn = skill::type::blindness;     break;

		case  1:        sn = skill::type::cause_serious; break;

		case  2:        sn = skill::type::earthquake;    break;

		case  3:        sn = skill::type::cause_critical; break;

		case  4:        sn = skill::type::dispel_evil;   break;

		case  5:        sn = skill::type::curse;         break;

		case  6:        sn = skill::type::change_sex;    break;

		case  7:        sn = skill::type::flamestrike;   break;

		case  8:
		case  9:
		case 10:        sn = skill::type::harm;          break;

		case 11:        sn = skill::type::plague;        break;

		default:        sn = skill::type::dispel_magic;  break;
		}

		if (ch->level < get_usable_level(sn, Guild::cleric)
		    || ch->mana < skill::lookup(sn).min_mana)
			continue;

		break;
	}

	(*skill::lookup(sn).spell_fun)(sn, ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, sn));
	return true;
}

bool spec_cast_mage(Character *ch)
{
	Character *victim;
	skill::type sn;
	int i;

	if (get_position(ch) != POS_FIGHTING)
		return false;

	for (victim = ch->in_room->people; victim != nullptr; victim = victim->next_in_room)
		if (victim->fighting == ch && number_bits(2) == 0)
			break;

	if (victim == nullptr)
		return false;

	for (i = 0; ; i++) {
		if (i == 10)
			return false;

		switch (number_bits(4)) {
		case  0:        sn = skill::type::blindness;     break;

		case  1:        sn = skill::type::chill_touch;   break;

		case  2:        sn = skill::type::weaken;        break;

		case  3:        sn = skill::type::teleport;      break;

		case  4:        sn = skill::type::colour_spray;  break;

		case  5:        sn = skill::type::change_sex;    break;

		case  6:        sn = skill::type::energy_drain;  break;

		case  7:
		case  8:
		case  9:        sn = skill::type::fireball;      break;

		case 10:
		case 11:        sn = skill::type::acid_blast;    break;

		default:        sn = skill::type::plague;        break;
		}

		if (ch->level < get_usable_level(sn, Guild::mage)
		    || ch->mana < skill::lookup(sn).min_mana)
			continue;

		break;
	}

	(*skill::lookup(sn).spell_fun)(sn, ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, sn));
	return true;
}

bool spec_cast_undead(Character *ch)
{
	Character *victim;
	skill::type sn;
	int i;

	if (get_position(ch) != POS_FIGHTING)
		return false;

	for (victim = ch->in_room->people; victim != nullptr; victim = victim->next_in_room)
		if (victim->fighting == ch && number_bits(2) == 0)
			break;

	if (victim == nullptr)
		return false;

	for (i = 0; ; i++) {
		if (i == 10)
			return false;

		switch (number_bits(4)) {
		case  0:        sn = skill::type::curse;         break;

		case  1:
		case  2:        sn = skill::type::chill_touch;   break;

		case  3:        sn = skill::type::blindness;     break;

		case  4:        sn = skill::type::energy_drain;  break;

		case  5:        sn = skill::type::teleport;      break;

		case  6:
		case  7:        sn = skill::type::poison;        break;

		case  8:
		case  9:        sn = skill::type::plague;        break;

		case 10:
		case 11:        sn = skill::type::harm;          break;

		default:        sn = skill::type::weaken;        break;
		}

		if ((ch->level < get_usable_level(sn, Guild::cleric)
		     && ch->level < get_usable_level(sn, Guild::mage))
		    || ch->mana < skill::lookup(sn).min_mana)
			continue;

		break;
	}

	(*skill::lookup(sn).spell_fun)(sn, ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, sn));
	return true;
}

bool spec_executioner(Character *ch)
{
	char buf[MAX_STRING_LENGTH];
	Character *victim;
	Character *v_next;
	char *crime;

	if (!IS_AWAKE(ch) || ch->fighting != nullptr)
		return false;

	crime = "";

	for (victim = ch->in_room->people; victim != nullptr; victim = v_next) {
		v_next = victim->next_in_room;

		if (!victim->is_npc() && victim->act_flags.has(PLR_KILLER)
		    &&   can_see_char(ch, victim))
		{ crime = "KILLER"; break; }

		if (!victim->is_npc() && victim->act_flags.has(PLR_THIEF)
		    &&   can_see_char(ch, victim))
		{ crime = "THIEF"; break; }
	}

	if (victim == nullptr)
		return false;

	Format::sprintf(buf, "%s is a %s!  PROTECT THE INNOCENT!  MORE BLOOOOD!!!",
	        victim->name, crime);
	ch->comm_flags -= COMM_NOCHANNELS;
	do_yell(ch, buf);
	multi_hit(ch, victim, skill::type::unknown);
	return true;
}

bool spec_fido(Character *ch)
{
	Object *corpse;
	Object *c_next;

	if (!IS_AWAKE(ch))
		return false;

	for (corpse = ch->in_room->contents; corpse != nullptr; corpse = c_next) {
		c_next = corpse->next_content;

		if (corpse->item_type != ITEM_CORPSE_NPC)
			continue;

		act("$n savagely devours a corpse.", ch, nullptr, nullptr, TO_ROOM);
		destroy_obj(corpse);
		return true;
	}

	return false;
}

bool spec_guard(Character *ch)
{
	char buf[MAX_STRING_LENGTH];
	Character *victim;
	Character *v_next;
	Character *ech;
	char *crime;
	int max_evil;

	if (!IS_AWAKE(ch) || ch->fighting != nullptr)
		return false;

	max_evil = 300;
	ech      = nullptr;
	crime    = "";

	for (victim = ch->in_room->people; victim != nullptr; victim = v_next) {
		v_next = victim->next_in_room;

		/* REWORK PK - Lotus
		        if ( !victim->is_npc() && victim->act_flags.has(PLR_KILLER)
		        &&   can_see_char(ch,victim))
		            { crime = "KILLER"; break; }

		        if ( !victim->is_npc() && victim->act_flags.has(PLR_THIEF)
		        &&   can_see_char(ch,victim))
		            { crime = "THIEF"; break; }
		*/

		if (victim->fighting != nullptr
		    &&   victim->fighting != ch
		    &&   victim->alignment < max_evil) {
			max_evil = victim->alignment;
			ech      = victim;
		}
	}

	if (victim != nullptr) {
		Format::sprintf(buf, "%s is a %s!  PROTECT THE INNOCENT!!  BANZAI!!",
		        victim->name, crime);
		ch->comm_flags -= COMM_NOCHANNELS;
		do_yell(ch, buf);
		multi_hit(ch, victim, skill::type::unknown);
		return true;
	}

	if (ech != nullptr) {
		act("$n screams 'PROTECT THE INNOCENT!!  BANZAI!!",
		    ch, nullptr, nullptr, TO_ROOM);
		multi_hit(ch, ech, skill::type::unknown);
		return true;
	}

	return false;
}

bool spec_janitor(Character *ch)
{
	Object *trash;
	Object *trash_next;

	if (!IS_AWAKE(ch))
		return false;

	for (trash = ch->in_room->contents; trash != nullptr; trash = trash_next) {
		trash_next = trash->next_content;

		if (!trash->wear_flags.has(ITEM_TAKE) || !can_loot(ch, trash))
			continue;

		if (trash->item_type == ITEM_CORPSE_PC)
			continue;

		if (trash->item_type == ITEM_DRINK_CON
		    ||   trash->item_type == ITEM_TRASH
		    ||   trash->cost < 10) {
			act("$n picks up some trash.", ch, nullptr, nullptr, TO_ROOM);
			obj_from_room(trash);
			obj_to_char(trash, ch);
			return true;
		}
	}

	return false;
}

bool spec_mayor(Character *ch)
{
	static const char open_path[] =
	        "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";
	static const char close_path[] =
	        "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";
	static const char *path;
	static int pos;
	static bool move;

	if (!move) {
		if (Game::world().time.hour ==  6) {
			path = open_path;
			move = true;
			pos  = 0;
		}

		if (Game::world().time.hour == 20) {
			path = close_path;
			move = true;
			pos  = 0;
		}
	}

	if (ch->fighting != nullptr)
		return spec_cast_mage(ch);

	if (!move || get_position(ch) < POS_SLEEPING)
		return false;

	switch (path[pos]) {
	case '0':
	case '1':
	case '2':
	case '3':
		move_char(ch, path[pos] - '0', false);
		break;

	case 'W':
		ch->position = POS_STANDING;
		act("$n awakens and groans loudly.", ch, nullptr, nullptr, TO_ROOM);
		break;

	case 'S':
		ch->position = POS_SLEEPING;
		act("$n lies down and falls asleep.", ch, nullptr, nullptr, TO_ROOM);
		break;

	case 'a':
		act("$n says 'Hello Honey!'", ch, nullptr, nullptr, TO_ROOM);
		break;

	case 'b':
		act("$n says 'What a view!  I must do something about that dump!'",
		    ch, nullptr, nullptr, TO_ROOM);
		break;

	case 'c':
		act("$n says 'Vandals!  Youngsters have no respect for anything!'",
		    ch, nullptr, nullptr, TO_ROOM);
		break;

	case 'd':
		act("$n says 'Good day, citizens!'", ch, nullptr, nullptr, TO_ROOM);
		break;

	case 'e':
		act("$n says 'I hereby declare the city of Midgaard open!'",
		    ch, nullptr, nullptr, TO_ROOM);
		break;

	case 'E':
		act("$n says 'I hereby declare the city of Midgaard closed!'",
		    ch, nullptr, nullptr, TO_ROOM);
		break;

	case 'O':
		/*      do_unlock( ch, "gate" ); */
		do_open(ch, "gate");
		break;

	case 'C':
		do_close(ch, "gate");
		/*      do_lock( ch, "gate" ); */
		break;

	case '.' :
		move = false;
		break;
	}

	pos++;
	return false;
}

bool spec_poison(Character *ch)
{
	Character *victim;

	if (get_position(ch) != POS_FIGHTING
	    || (victim = ch->fighting) == nullptr
	    ||   number_percent() > 2 * ch->level)
		return false;

	act("You bite $N!",  ch, nullptr, victim, TO_CHAR);
	act("$n bites $N!",  ch, nullptr, victim, TO_NOTVICT);
	act("$n bites you!", ch, nullptr, victim, TO_VICT);
	spell_poison(skill::type::poison, ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, skill::type::poison));
	return true;
}

bool spec_thief(Character *ch)
{
	Character *victim;
	Character *v_next;
	int gold, silver;

	if (get_position(ch) < POS_STANDING)
		return false;

	for (victim = ch->in_room->people; victim != nullptr; victim = v_next) {
		v_next = victim->next_in_room;

		if (victim->is_npc()
		    ||   IS_IMMORTAL(victim)
		    ||   number_bits(5) != 0
		    ||   !can_see_char(ch, victim))
			continue;

		if (IS_AWAKE(victim) && number_range(0, ch->level) == 0) {
			act("You discover $n's hands in your wallet!",
			    ch, nullptr, victim, TO_VICT);
			act("$N discovers $n's hands in $S wallet!",
			    ch, nullptr, victim, TO_NOTVICT);
			return true;
		}
		else {
			gold = victim->gold * std::min(number_range(1, 20), ch->level / 2) / 100;
			gold = std::min(gold, ch->level * ch->level * 10);
			ch->gold     += gold;
			victim->gold -= gold;
			silver = victim->silver * std::min(number_range(1, 20), ch->level / 2) / 100;
			silver = std::min(silver, ch->level * ch->level * 25);
			ch->silver  += silver;
			victim->silver -= silver;
			return true;
		}
	}

	return false;
}
void do_identify(Character *ch, String argument)
{
	Object *obj;
	Character *rch;
	char buf[MAX_STRING_LENGTH];

	if ((obj = get_obj_carry(ch, argument)) == nullptr) {
		stc("You are not carrying that.\n", ch);
		return;
	}

	for (rch = ch->in_room->people; rch != nullptr; rch = rch->next_in_room)
		if (rch->is_npc() && rch->spec_fun == spec_lookup("spec_sage"))
			break;

	if (!rch) {
		stc("No one here has any clue what that is.\n", ch);
		return;
	}
	else if ((ch->silver + 100 * ch->gold) < obj->level * 10 + 50) {
		act("$n goes about his business and ignores $p.",
		    rch, obj, nullptr, TO_ROOM);
		return;
	}
	else {
		deduct_cost(ch, obj->level * 10 + 50);
		Format::sprintf(buf, "The identify on %s has cost you %d silver.\n",
		        obj->short_descr, obj->level * 10 + 50);
		stc(buf, ch);
	}

	act("$n fondles $p and ponders its purpose.",
	    rch, obj, nullptr, TO_ROOM);
	spell_identify(skill::type::identify, 0, ch, obj, TAR_OBJ_INV, 1);
}

bool spec_charm(Character *ch)
{
	Character *victim;
	Character *v_next;

	if (get_position(ch) != POS_FIGHTING) {
		switch (number_range(0, 15)) {
		default:
			break;

		case 0:
			do_order(ch, "all bth self");
			break;

		case 1:
//          do_order( ch, "all philo I just love Legacy's Mobiles!!!");
			break;

		case 2:
			do_order(ch, "all bonk self");
			break;
		};

		return true;
	}

	for (victim = ch->in_room->people; victim != nullptr; victim = v_next) {
		v_next = victim->next_in_room;

		if (victim->fighting == ch && number_bits(2) == 0)
			break;
	}

	if (victim == nullptr)
		return false;

	act("$n begins playing a new, beautiful song.",
	    ch, nullptr, nullptr, TO_ROOM);
	spell_charm_person(skill::type::charm_person, ch->level, ch, victim,
	                   TAR_CHAR_OFFENSIVE, get_evolution(ch, skill::type::charm_person));

	if (affect::exists_on_char(victim, affect::type::charm_person))
		stop_fighting(victim, true);

	return true;
}
void do_repair(Character *ch, String argument)
{
	int iWear;
	Object *obj;
	char buf[MAX_STRING_LENGTH];

	if (argument.empty()) {
		bool etched = false;
		stc("{WItems you are wearing:\n", ch);

		for (iWear = 0; iWear < MAX_WEAR; iWear++) {
			if ((obj = get_eq_char(ch, iWear)) == nullptr)
				continue;

			etched = affect::exists_on_obj(obj, affect::type::acid_breath) ? true : false;

			Format::sprintf(buf, "{M[{V%14s{M] {x%s %s\n"
			        , condition_lookup(obj->condition),
			        obj->short_descr, (etched) ? "{G({HEtched{G){x" : "");
			stc(buf, ch);
			etched = false;
		}

		return;
	}

	if (argument == "all") {
		for (iWear = 0; iWear < MAX_WEAR; iWear++) {
			if ((obj = get_eq_char(ch, iWear)) == nullptr)
				continue;

			obj_repair(ch, obj);
		}

		return;
	}

	if ((obj = get_obj_carry(ch, argument)) == nullptr) {
		stc("You are not carrying that.\n", ch);
		return;
	}

	obj_repair(ch, obj);
}

void obj_repair(Character *ch, Object *obj)
{
	Character *rch;
	char buf[MAX_STRING_LENGTH];
	int max = 100;

	for (rch = ch->in_room->people; rch != nullptr; rch = rch->next_in_room)
		if (rch->is_npc() && rch->spec_fun == spec_lookup("spec_blacksmith"))
			break;

	if (obj->condition == -1) {
		stc("That object is indestructable!\n", ch);
		return;
	}

	const affect::Affect *paf;
	if ((paf = affect::find_on_obj(obj, affect::type::acid_breath)) != nullptr)
		max = 100 - (5 * paf->modifier);

	if (obj->condition >= max) {
		stc("That object cannot be repaired further.\n", ch);
		return;
	}

	if (!rch) {
		if (get_skill_level(ch, skill::type::repair)) {
			if (!deduct_stamina(ch, skill::type::repair))
				return;

			WAIT_STATE(ch, skill::lookup(skill::type::repair).beats);

			if (number_percent() > get_skill_level(ch, skill::type::repair)) {
				stc("You accidentally damage it more!\n", ch);
				check_improve(ch, skill::type::repair, false, 8);
				obj->condition -= number_range(10, 15);

				if (obj->condition < 0)
					obj->condition = 0;

				if (obj->condition == 0) {
					act("{W$p{x has been {Wdestroyed{x!", ch, obj, nullptr, TO_CHAR);
					destroy_obj(obj);
				}

				return;
			}

			Format::sprintf(buf, "$n repairs $p to %s condition.",
			        condition_lookup(obj->condition));
			act(buf, rch, obj, nullptr, TO_ROOM);
			obj->condition = max;
			Format::sprintf(buf, "You repair $p to %s condition.",
			        condition_lookup(obj->condition));
			act(buf, ch, obj, nullptr, TO_CHAR);
			check_improve(ch, skill::type::repair, true, 8);
			return;
		}

		stc("No one here can repair that item.\n", ch);
		return;
	}

	if ((ch->silver + 100 * ch->gold) < (max - obj->condition) * 100) {
		act("$n says, 'You don't have enough money to repair $p.",
		    rch, obj, nullptr, TO_ROOM);
		return;
	}
	else {
		deduct_cost(ch, (max - obj->condition) * 100);
		Format::sprintf(buf, "The repairs on %s has cost you %d gold.\n",
		        obj->short_descr, (max - obj->condition));
		stc(buf, ch);
	}

	obj->condition = max;
	Format::sprintf(buf, "$n repairs $p to %s condition.",
	        condition_lookup(obj->condition));
	act(buf, rch, obj, nullptr, TO_ROOM);
}

bool spec_fight_clanguard(Character *ch)
{
	return true;
}

bool spec_clanguard(Character *ch)
{
	Clan *clan;
	Character *victim, *v_next;
	bool found = false;

	if (!ch->is_npc()
	    || !IS_AWAKE(ch)
	    || affect::exists_on_char(ch, affect::type::calm)
	    || affect::exists_on_char(ch, affect::type::charm_person)
	    || ch->in_room == nullptr)
		return false;

	if ((clan = clan_vnum_lookup(ch->pIndexData->vnum)) == nullptr)
		return false;

	if (ch->in_room->clan() != clan)
		return false;

	for (victim = ch->in_room->people; victim != nullptr; victim = v_next) {
		v_next = victim->next_in_room;

		if (victim->fighting == ch || IS_IMMORTAL(victim))
			continue;

		if (!victim->clan || victim->clan == clan || victim->inviters == clan)
			continue;

		if (!clan_opponents(clan, victim->clan))
			continue;

		if (!found) {
			ch->comm_flags -= COMM_NOCHANNELS;
			do_yell(ch, "Invaders!  KILL THE INFIDELS!!");
		}

		multi_hit(ch, victim, skill::type::unknown);
		found = true;
	}

	if (ch->fighting)
		spec_fight_clanguard(ch);

	return found;
}

/**
 * This function determines whether or not a character
 * is "special". This function was written by Outsider
 * to replace the IS_SPECIAL macro (merc.h).
 * The function returns true if the character is special
 * and false if they are not.
 *
 * Explanation:  this is a hack.  it is not a back door,
 * it is a means to get around the head and imp flags
 * for coders.  just because they don't carry the rank
 * of imp or head does not mean they don't need all
 * commands.  -- Montrey
 */
bool IS_SPECIAL(Character *ch)
{
	if (ch->is_npc())
		return false;

	if (ch->name == "Montrey"
	    || ch->name == "Xenith"
            || ch->name == "Vegita")
		return true;

	return false;
}

/*
This function attempts to make a pet into a familiar.
Newly made familiars cause the pcdata->familiar field to
point at the character's pet. The owner then gets a
bonus ability point in the same ability which is
highest in the pet.
-- Outsider
*/
void do_familiar(Character *ch, String argument)
{
	/* first check to make sure this is a character */
	if (ch->is_npc())
		return;

	/* make sure we have the skill */
	if (get_skill_level(ch, skill::type::familiar) < 1) {
		stc("You don't know how to create a familiar.\n", ch);
		return;
	}

	/* make sure we don't already have a familiar */
	if (ch->pcdata->familiar) {
		stc("You already have a familiar.\n", ch);
		return;
	}

	/* we need a pet */
	if (! ch->pet) {
		stc("You need a pet first.\n", ch);
		return;
	}

	ch->pcdata->familiar = true;
	stc("You feel at one with your pet.\n", ch);
	check_improve(ch, skill::type::familiar, true, 1);
	return;
}
