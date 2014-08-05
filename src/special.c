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

#include "merc.h"

/* command procedures needed */
DECLARE_DO_FUN(do_yell);
DECLARE_DO_FUN(do_open);
DECLARE_DO_FUN(do_close);
DECLARE_DO_FUN(do_say);
DECLARE_DO_FUN(do_order);
DECLARE_DO_FUN(do_backstab);
DECLARE_DO_FUN(do_flee);
DECLARE_DO_FUN(do_kill);
DECLARE_SPELL_FUN(spell_identify);
DECLARE_SPELL_FUN(spell_charm_person);

void     obj_repair      args((CHAR_DATA *ch, OBJ_DATA *obj));

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
const   struct  spec_type    spec_table[] = {
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
	{   NULL,                           NULL                    }
};


/* Given a name, return the appropriate spec fun. */
SPEC_FUN *spec_lookup(const char *name)
{
	int i;

	for (i = 0; spec_table[i].name != NULL; i++) {
		if (LOWER(name[0]) == LOWER(spec_table[i].name[0]) && !str_prefix1(name, spec_table[i].name))
			return spec_table[i].function;
	}

	return 0;
}


char *spec_name(SPEC_FUN *function)
{
	int i;

	for (i = 0; spec_table[i].function != NULL; i++) {
		if (function == spec_table[i].function)
			return spec_table[i].name;
	}

	return NULL;
}


bool spec_troll_member(CHAR_DATA *ch)
{
	CHAR_DATA *vch, *victim = NULL;
	int count = 0;
	char *message;

	if (!IS_AWAKE(ch) || IS_AFFECTED(ch, AFF_CALM) || ch->in_room == NULL
	    || IS_AFFECTED(ch, AFF_CHARM) || ch->fighting != NULL)
		return FALSE;

	/* find an ogre to beat up */
	for (vch = ch->in_room->people;  vch != NULL;  vch = vch->next_in_room) {
		if (!IS_NPC(vch) || ch == vch)
			continue;

		if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN)
			return FALSE;

		if (vch->pIndexData->group == GROUP_VNUM_OGRES &&  ch->level > vch->level - 2 && !is_safe(ch, vch, TRUE)) {
			if (number_range(0, count) == 0)
				victim = vch;

			count++;
		}
	}

	if (victim == NULL)
		return FALSE;

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

	act(message, ch, NULL, victim, TO_ALL);
	multi_hit(ch, victim, TYPE_UNDEFINED);
	return TRUE;
}


bool spec_ogre_member(CHAR_DATA *ch)
{
	CHAR_DATA *vch, *victim = NULL;
	int count = 0;
	char *message;

	if (!IS_AWAKE(ch) || IS_AFFECTED(ch, AFF_CALM) || ch->in_room == NULL
	    ||  IS_AFFECTED(ch, AFF_CHARM) || ch->fighting != NULL)
		return FALSE;

	/* find an troll to beat up */
	for (vch = ch->in_room->people;  vch != NULL;  vch = vch->next_in_room) {
		if (!IS_NPC(vch) || ch == vch)
			continue;

		if (vch->pIndexData->vnum == MOB_VNUM_PATROLMAN)
			return FALSE;

		if (vch->pIndexData->group == GROUP_VNUM_TROLLS && ch->level > vch->level - 2 && !is_safe(ch, vch, TRUE)) {
			if (number_range(0, count) == 0)
				victim = vch;

			count++;
		}
	}

	if (victim == NULL)
		return FALSE;

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

	act(message, ch, NULL, victim, TO_ALL);
	multi_hit(ch, victim, TYPE_UNDEFINED);
	return TRUE;
}


bool spec_patrolman(CHAR_DATA *ch)
{
	CHAR_DATA *vch, *victim = NULL;
	OBJ_DATA *obj;
	char *message;
	int count = 0;

	if (!IS_AWAKE(ch) || IS_AFFECTED(ch, AFF_CALM) || ch->in_room == NULL
	    ||  IS_AFFECTED(ch, AFF_CHARM) || ch->fighting != NULL)
		return FALSE;

	/* look for a fight in the room */
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (vch == ch)
			continue;

		if (vch->fighting != NULL) { /* break it up! */
			if (number_range(0, count) == 0)
				victim = (vch->level > vch->fighting->level)
				         ? vch : vch->fighting;

			count++;
		}
	}

	if (victim == NULL || (IS_NPC(victim) && victim->spec_fun == ch->spec_fun))
		return FALSE;

	if (((obj = get_eq_char(ch, WEAR_NECK_1)) != NULL
	     &&   obj->pIndexData->vnum == OBJ_VNUM_WHISTLE)
	    || ((obj = get_eq_char(ch, WEAR_NECK_2)) != NULL
	        &&   obj->pIndexData->vnum == OBJ_VNUM_WHISTLE)) {
		act("You blow down hard on $p.", ch, obj, NULL, TO_CHAR);
		act("$n blows on $p, ***WHEEEEEEEEEEEET***", ch, obj, NULL, TO_ROOM);

		for (vch = char_list; vch != NULL; vch = vch->next) {
			if (vch->in_room == NULL)
				continue;

			if (vch->in_room != ch->in_room
			    &&  vch->in_room->area == ch->in_room->area)
				stc("You hear a shrill whistling sound.\n\r", vch);
		}
	}

	switch (number_range(0, 6)) {
	default:        message = NULL;         break;

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

	if (message != NULL)
		act(message, ch, NULL, NULL, TO_ALL);

	multi_hit(ch, victim, TYPE_UNDEFINED);
	return TRUE;
}
bool spec_questmaster(CHAR_DATA *ch)
{
	if (ch->fighting != NULL) return spec_cast_mage(ch);

	return FALSE;
}

bool spec_squestmaster(CHAR_DATA *ch)
{
	if (ch->fighting != NULL)
		return spec_cast_mage(ch);

	return FALSE;
}

bool spec_blacksmith(CHAR_DATA *ch)
{
	if (ch->fighting != NULL) return spec_cast_mage(ch);

	return FALSE;
}
bool spec_sage(CHAR_DATA *ch)
{
	if (ch->fighting != NULL) return spec_cast_mage(ch);

	return FALSE;
}

bool spec_nasty(CHAR_DATA *ch)
{
	CHAR_DATA *victim, *v_next;
	long gold;

	if (!IS_AWAKE(ch))
		return FALSE;

	if (!ch->fighting) {
		for (victim = ch->in_room->people; victim != NULL; victim = v_next) {
			v_next = victim->next_in_room;

			if (!IS_NPC(victim)
			    && (victim->level > ch->level)
			    && (victim->level < ch->level + 10)) {
				do_backstab(ch, victim->name);

				if (!ch->fighting)
					do_kill(ch, victim->name);

				/* should steal some coins right away? :) */
				return TRUE;
			}
		}

		return FALSE;    /*  No one to attack */
	}

	/* okay, we must be fighting.... steal some coins and flee */
	if ((victim = ch->fighting) == NULL)
		return FALSE;   /* let's be paranoid.... */

	switch (number_bits(2)) {
	case 0:  act("$n rips apart your coin purse, spilling your gold!",
		             ch, NULL, victim, TO_VICT);
		act("You slash apart $N's coin purse and gather his gold.",
		    ch, NULL, victim, TO_CHAR);
		act("$N's coin purse is ripped apart!",
		    ch, NULL, victim, TO_NOTVICT);
		gold = victim->gold / 10;  /* steal 10% of his gold */
		victim->gold -= gold;
		ch->gold     += gold;
		return TRUE;

	case 1:  do_flee(ch, "");
		return TRUE;

	default: return FALSE;
	}
}

/* Core procedure for dragons. */
bool dragon(CHAR_DATA *ch, int sn)
{
	CHAR_DATA *victim;

	if (get_position(ch) != POS_FIGHTING)
		return FALSE;

	for (victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room)
		if (victim->fighting == ch
		    && can_see(ch, victim)
		    && number_bits(3) == 0)
			break;

	if (victim == NULL)
		return FALSE;

	/* made it so mobs won't keep breathing after mana runs out, but just using the min mana -- Montrey */
	if (ch->mana < skill_table[sn].min_mana)
		return FALSE;

	(*skill_table[sn].spell_fun)
	(sn, ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, sn));
	return TRUE;
}


/* Special procedures for mobiles. */
bool spec_breath_any(CHAR_DATA *ch)
{
	switch (number_bits(3)) {
	case 0:
	case 1: return dragon(ch, gsn_lightning_breath);

	case 2:
	case 3:
	case 4: return dragon(ch, gsn_frost_breath);

	case 5: return dragon(ch, gsn_fire_breath);

	case 6: return dragon(ch, gsn_gas_breath);

	case 7: return dragon(ch, gsn_acid_breath);
	}

	return FALSE;
}


bool spec_breath_acid(CHAR_DATA *ch)
{
	return dragon(ch, gsn_acid_breath);
}


bool spec_breath_fire(CHAR_DATA *ch)
{
	return dragon(ch, gsn_fire_breath);
}


bool spec_breath_frost(CHAR_DATA *ch)
{
	return dragon(ch, gsn_frost_breath);
}


bool spec_breath_gas(CHAR_DATA *ch)
{
	return dragon(ch, gsn_gas_breath);
}


bool spec_breath_lightning(CHAR_DATA *ch)
{
	return dragon(ch, gsn_lightning_breath);
}


bool spec_cast_adept(CHAR_DATA *ch)
{
	CHAR_DATA *victim;
	CHAR_DATA *v_next;

	if (!IS_AWAKE(ch))
		return FALSE;

	for (victim = ch->in_room->people; victim != NULL; victim = v_next) {
		v_next = victim->next_in_room;

		if (victim != ch && can_see(ch, victim) && number_bits(1) == 0
		    && !IS_NPC(victim) && victim->level < 11)
			break;
	}

	if (victim == NULL)
		return FALSE;

	switch (number_bits(4)) {
	case 0:
#if (defined(HALLOWEEN))
		act("$n utters the word 'bhiae waai'.", ch, NULL, NULL, TO_ROOM);
		spell_blood_moon(gsn_blood_moon, ch->level, ch, victim, TARGET_CHAR,
		                 get_evolution(ch, gsn_blood_moon));
#else
		act("$n utters the word 'abrazak'.", ch, NULL, NULL, TO_ROOM);
		spell_armor(gsn_armor, ch->level, ch, victim, TARGET_CHAR,
		            get_evolution(ch, gsn_armor));
#endif
		return TRUE;

	case 1:
		act("$n utters the word 'fido'.", ch, NULL, NULL, TO_ROOM);
		spell_bless(gsn_bless, ch->level, ch, victim, TARGET_CHAR,
		            get_evolution(ch, gsn_bless));
		return TRUE;

	case 2:
		act("$n utters the words 'judicandus noselacri'.", ch, NULL, NULL, TO_ROOM);
		spell_cure_blindness(gsn_cure_blindness,
		                     ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, gsn_cure_blindness));
		return TRUE;

	case 3:
		act("$n utters the words 'judicandus dies'.", ch, NULL, NULL, TO_ROOM);
		spell_cure_light(gsn_cure_light,
		                 ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, gsn_cure_light));
		return TRUE;

	case 4:
		act("$n utters the words 'judicandus sausabru'.", ch, NULL, NULL, TO_ROOM);
		spell_cure_poison(gsn_cure_poison,
		                  ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, gsn_cure_poison));
		return TRUE;

	case 5:
		act("$n utters the word 'candusima'.", ch, NULL, NULL, TO_ROOM);
		spell_refresh(gsn_refresh, ch->level, ch, victim, TARGET_CHAR,
		              get_evolution(ch, gsn_refresh));
		return TRUE;

	case 6:
		act("$n utters the words 'judicandus eugzagz'.", ch, NULL, NULL, TO_ROOM);
		spell_cure_disease(gsn_cure_disease,
		                   ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, gsn_cure_disease));
	}

	return FALSE;
}


bool spec_cast_judge(CHAR_DATA *ch)
{
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	char *spell;
	int sn;

	if (get_position(ch) != POS_FIGHTING)
		return FALSE;

	for (victim = ch->in_room->people; victim != NULL; victim = v_next) {
		v_next = victim->next_in_room;

		if (victim->fighting == ch && number_bits(2) == 0)
			break;
	}

	if (victim == NULL)
		return FALSE;

	spell = "high explosive";

	if ((sn = skill_lookup(spell)) < 0)
		return FALSE;

	(*skill_table[sn].spell_fun)(sn, ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, sn));
	return TRUE;
}


bool spec_cast_cleric(CHAR_DATA *ch)
{
	CHAR_DATA *victim;
	int sn, i;

	if (get_position(ch) != POS_FIGHTING)
		return FALSE;

	for (victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room)
		if (victim->fighting == ch && number_bits(2) == 0)
			break;

	if (victim == NULL)
		return FALSE;

	for (i = 0; ; i++) {
		if (i == 10)
			return FALSE;

		switch (number_bits(4)) {
		case  0:        sn = gsn_blindness;     break;

		case  1:        sn = gsn_cause_serious; break;

		case  2:        sn = gsn_earthquake;    break;

		case  3:        sn = gsn_cause_critical; break;

		case  4:        sn = gsn_dispel_evil;   break;

		case  5:        sn = gsn_curse;         break;

		case  6:        sn = gsn_change_sex;    break;

		case  7:        sn = gsn_flamestrike;   break;

		case  8:
		case  9:
		case 10:        sn = gsn_harm;          break;

		case 11:        sn = gsn_plague;        break;

		default:        sn = gsn_dispel_magic;  break;
		}

		if (ch->level < skill_table[sn].skill_level[1]
		    || ch->mana < skill_table[sn].min_mana)
			continue;

		break;
	}

	(*skill_table[sn].spell_fun)(sn, ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, sn));
	return TRUE;
}


bool spec_cast_mage(CHAR_DATA *ch)
{
	CHAR_DATA *victim;
	int sn, i;

	if (get_position(ch) != POS_FIGHTING)
		return FALSE;

	for (victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room)
		if (victim->fighting == ch && number_bits(2) == 0)
			break;

	if (victim == NULL)
		return FALSE;

	for (i = 0; ; i++) {
		if (i == 10)
			return FALSE;

		switch (number_bits(4)) {
		case  0:        sn = gsn_blindness;     break;

		case  1:        sn = gsn_chill_touch;   break;

		case  2:        sn = gsn_weaken;        break;

		case  3:        sn = gsn_teleport;      break;

		case  4:        sn = gsn_colour_spray;  break;

		case  5:        sn = gsn_change_sex;    break;

		case  6:        sn = gsn_energy_drain;  break;

		case  7:
		case  8:
		case  9:        sn = gsn_fireball;      break;

		case 10:
		case 11:        sn = gsn_acid_blast;    break;

		default:        sn = gsn_plague;        break;
		}

		if (ch->level < skill_table[sn].skill_level[0]
		    || ch->mana < skill_table[sn].min_mana)
			continue;

		break;
	}

	(*skill_table[sn].spell_fun)(sn, ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, sn));
	return TRUE;
}


bool spec_cast_undead(CHAR_DATA *ch)
{
	CHAR_DATA *victim;
	int sn, i;

	if (get_position(ch) != POS_FIGHTING)
		return FALSE;

	for (victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room)
		if (victim->fighting == ch && number_bits(2) == 0)
			break;

	if (victim == NULL)
		return FALSE;

	for (i = 0; ; i++) {
		if (i == 10)
			return FALSE;

		switch (number_bits(4)) {
		case  0:        sn = gsn_curse;         break;

		case  1:
		case  2:        sn = gsn_chill_touch;   break;

		case  3:        sn = gsn_blindness;     break;

		case  4:        sn = gsn_energy_drain;  break;

		case  5:        sn = gsn_teleport;      break;

		case  6:
		case  7:        sn = gsn_poison;        break;

		case  8:
		case  9:        sn = gsn_plague;        break;

		case 10:
		case 11:        sn = gsn_harm;          break;

		default:        sn = gsn_weaken;        break;
		}

		if ((ch->level < skill_table[sn].skill_level[1]
		     && ch->level < skill_table[sn].skill_level[0])
		    || ch->mana < skill_table[sn].min_mana)
			continue;

		break;
	}

	(*skill_table[sn].spell_fun)(sn, ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, sn));
	return TRUE;
}


bool spec_executioner(CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	char *crime;

	if (!IS_AWAKE(ch) || ch->fighting != NULL)
		return FALSE;

	crime = "";

	for (victim = ch->in_room->people; victim != NULL; victim = v_next) {
		v_next = victim->next_in_room;

		if (!IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER)
		    &&   can_see(ch, victim))
		{ crime = "KILLER"; break; }

		if (!IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF)
		    &&   can_see(ch, victim))
		{ crime = "THIEF"; break; }
	}

	if (victim == NULL)
		return FALSE;

	sprintf(buf, "%s is a %s!  PROTECT THE INNOCENT!  MORE BLOOOOD!!!",
	        victim->name, crime);
	REMOVE_BIT(ch->comm, COMM_NOCHANNELS);
	do_yell(ch, buf);
	multi_hit(ch, victim, TYPE_UNDEFINED);
	return TRUE;
}




bool spec_fido(CHAR_DATA *ch)
{
	OBJ_DATA *corpse;
	OBJ_DATA *c_next;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if (!IS_AWAKE(ch))
		return FALSE;

	for (corpse = ch->in_room->contents; corpse != NULL; corpse = c_next) {
		c_next = corpse->next_content;

		if (corpse->item_type != ITEM_CORPSE_NPC)
			continue;

		act("$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM);

		for (obj = corpse->contains; obj; obj = obj_next) {
			obj_next = obj->next_content;
			obj_from_obj(obj);
			obj_to_room(obj, ch->in_room);
		}

		extract_obj(corpse);
		return TRUE;
	}

	return FALSE;
}



bool spec_guard(CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	CHAR_DATA *ech;
	char *crime;
	int max_evil;

	if (!IS_AWAKE(ch) || ch->fighting != NULL)
		return FALSE;

	max_evil = 300;
	ech      = NULL;
	crime    = "";

	for (victim = ch->in_room->people; victim != NULL; victim = v_next) {
		v_next = victim->next_in_room;

		/* REWORK PK - Lotus
		        if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER)
		        &&   can_see(ch,victim))
		            { crime = "KILLER"; break; }

		        if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF)
		        &&   can_see(ch,victim))
		            { crime = "THIEF"; break; }
		*/

		if (victim->fighting != NULL
		    &&   victim->fighting != ch
		    &&   victim->alignment < max_evil) {
			max_evil = victim->alignment;
			ech      = victim;
		}
	}

	if (victim != NULL) {
		sprintf(buf, "%s is a %s!  PROTECT THE INNOCENT!!  BANZAI!!",
		        victim->name, crime);
		REMOVE_BIT(ch->comm, COMM_NOCHANNELS);
		do_yell(ch, buf);
		multi_hit(ch, victim, TYPE_UNDEFINED);
		return TRUE;
	}

	if (ech != NULL) {
		act("$n screams 'PROTECT THE INNOCENT!!  BANZAI!!",
		    ch, NULL, NULL, TO_ROOM);
		multi_hit(ch, ech, TYPE_UNDEFINED);
		return TRUE;
	}

	return FALSE;
}



bool spec_janitor(CHAR_DATA *ch)
{
	OBJ_DATA *trash;
	OBJ_DATA *trash_next;

	if (!IS_AWAKE(ch))
		return FALSE;

	for (trash = ch->in_room->contents; trash != NULL; trash = trash_next) {
		trash_next = trash->next_content;

		if (!IS_SET(trash->wear_flags, ITEM_TAKE) || !can_loot(ch, trash))
			continue;

		if (trash->item_type == ITEM_CORPSE_PC)
			continue;

		if (trash->item_type == ITEM_DRINK_CON
		    ||   trash->item_type == ITEM_TRASH
		    ||   trash->cost < 10) {
			act("$n picks up some trash.", ch, NULL, NULL, TO_ROOM);
			obj_from_room(trash);
			obj_to_char(trash, ch);
			return TRUE;
		}
	}

	return FALSE;
}



bool spec_mayor(CHAR_DATA *ch)
{
	static const char open_path[] =
	        "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";
	static const char close_path[] =
	        "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";
	static const char *path;
	static int pos;
	static bool move;

	if (!move) {
		if (time_info.hour ==  6) {
			path = open_path;
			move = TRUE;
			pos  = 0;
		}

		if (time_info.hour == 20) {
			path = close_path;
			move = TRUE;
			pos  = 0;
		}
	}

	if (ch->fighting != NULL)
		return spec_cast_mage(ch);

	if (!move || get_position(ch) < POS_SLEEPING)
		return FALSE;

	switch (path[pos]) {
	case '0':
	case '1':
	case '2':
	case '3':
		move_char(ch, path[pos] - '0', FALSE);
		break;

	case 'W':
		ch->position = POS_STANDING;
		act("$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM);
		break;

	case 'S':
		ch->position = POS_SLEEPING;
		act("$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM);
		break;

	case 'a':
		act("$n says 'Hello Honey!'", ch, NULL, NULL, TO_ROOM);
		break;

	case 'b':
		act("$n says 'What a view!  I must do something about that dump!'",
		    ch, NULL, NULL, TO_ROOM);
		break;

	case 'c':
		act("$n says 'Vandals!  Youngsters have no respect for anything!'",
		    ch, NULL, NULL, TO_ROOM);
		break;

	case 'd':
		act("$n says 'Good day, citizens!'", ch, NULL, NULL, TO_ROOM);
		break;

	case 'e':
		act("$n says 'I hereby declare the city of Midgaard open!'",
		    ch, NULL, NULL, TO_ROOM);
		break;

	case 'E':
		act("$n says 'I hereby declare the city of Midgaard closed!'",
		    ch, NULL, NULL, TO_ROOM);
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
		move = FALSE;
		break;
	}

	pos++;
	return FALSE;
}



bool spec_poison(CHAR_DATA *ch)
{
	CHAR_DATA *victim;

	if (get_position(ch) != POS_FIGHTING
	    || (victim = ch->fighting) == NULL
	    ||   number_percent() > 2 * ch->level)
		return FALSE;

	act("You bite $N!",  ch, NULL, victim, TO_CHAR);
	act("$n bites $N!",  ch, NULL, victim, TO_NOTVICT);
	act("$n bites you!", ch, NULL, victim, TO_VICT);
	spell_poison(gsn_poison, ch->level, ch, victim, TARGET_CHAR, get_evolution(ch, gsn_poison));
	return TRUE;
}



bool spec_thief(CHAR_DATA *ch)
{
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	long gold, silver;

	if (get_position(ch) < POS_STANDING)
		return FALSE;

	for (victim = ch->in_room->people; victim != NULL; victim = v_next) {
		v_next = victim->next_in_room;

		if (IS_NPC(victim)
		    ||   victim->level >= LEVEL_IMMORTAL
		    ||   number_bits(5) != 0
		    ||   !can_see(ch, victim))
			continue;

		if (IS_AWAKE(victim) && number_range(0, ch->level) == 0) {
			act("You discover $n's hands in your wallet!",
			    ch, NULL, victim, TO_VICT);
			act("$N discovers $n's hands in $S wallet!",
			    ch, NULL, victim, TO_NOTVICT);
			return TRUE;
		}
		else {
			gold = victim->gold * UMIN(number_range(1, 20), ch->level / 2) / 100;
			gold = UMIN(gold, ch->level * ch->level * 10);
			ch->gold     += gold;
			victim->gold -= gold;
			silver = victim->silver * UMIN(number_range(1, 20), ch->level / 2) / 100;
			silver = UMIN(silver, ch->level * ch->level * 25);
			ch->silver  += silver;
			victim->silver -= silver;
			return TRUE;
		}
	}

	return FALSE;
}
void do_identify(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *rch;
	char buf[MAX_STRING_LENGTH];

	if ((obj = get_obj_carry(ch, argument)) == NULL) {
		stc("You are not carrying that.\n\r", ch);
		return;
	}

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
		if (IS_NPC(rch) && rch->spec_fun == spec_lookup("spec_sage"))
			break;

	if (!rch) {
		stc("No one here has any clue what that is.\n\r", ch);
		return;
	}
	else if ((ch->silver + 100 * ch->gold) < obj->level * 10 + 50) {
		act("$n goes about his business and ignores $p.",
		    rch, obj, 0, TO_ROOM);
		return;
	}
	else {
		deduct_cost(ch, obj->level * 10 + 50);
		sprintf(buf, "The identify on %s has cost you %d silver.\n\r",
		        obj->short_descr, obj->level * 10 + 50);
		stc(buf, ch);
	}

	act("$n fondles $p and ponders its purpose.",
	    rch, obj, 0, TO_ROOM);
	spell_identify(0, 0, ch, obj, TAR_OBJ_INV, get_evolution(ch, gsn_identify));
}
bool spec_charm(CHAR_DATA *ch)
{
	CHAR_DATA *victim;
	CHAR_DATA *v_next;

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

		return TRUE;
	}

	for (victim = ch->in_room->people; victim != NULL; victim = v_next) {
		v_next = victim->next_in_room;

		if (victim->fighting == ch && number_bits(2) == 0)
			break;
	}

	if (victim == NULL)
		return FALSE;

	act("$n begins playing a new, beautiful song.",
	    ch, NULL, NULL, TO_ROOM);
	spell_charm_person(gsn_charm_person, ch->level, ch, victim,
	                   TAR_CHAR_OFFENSIVE, get_evolution(ch, gsn_charm_person));

	if (IS_AFFECTED(victim, AFF_CHARM))
		stop_fighting(victim, TRUE);

	return TRUE;
}
void do_repair(CHAR_DATA *ch, char *argument)
{
	int iWear;
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH];

	if (argument[0] == '\0') {
		bool etched = FALSE;
		AFFECT_DATA *paf;
		stc("{WItems you are wearing:\n\r", ch);

		for (iWear = 0; iWear < MAX_WEAR; iWear++) {
			if ((obj = get_eq_char(ch, iWear)) == NULL)
				continue;

			for (paf = obj->affected; paf != NULL; paf = paf->next)
				if (paf->type == gsn_acid_breath)
					etched = TRUE;

			sprintf(buf, "{M[{V%14s{M] {x%s %s\n\r"
			        , condition_lookup(obj->condition),
			        obj->short_descr, (etched) ? "{G({HEtched{G){x" : "");
			stc(buf, ch);
			etched = FALSE;
		}

		return;
	}

	if (!str_cmp(argument, "all")) {
		for (iWear = 0; iWear < MAX_WEAR; iWear++) {
			if ((obj = get_eq_char(ch, iWear)) == NULL)
				continue;

			obj_repair(ch, obj);
		}

		return;
	}

	if ((obj = get_obj_carry(ch, argument)) == NULL) {
		stc("You are not carrying that.\n\r", ch);
		return;
	}

	obj_repair(ch, obj);
}

void obj_repair(CHAR_DATA *ch, OBJ_DATA *obj)
{
	CHAR_DATA *rch;
	AFFECT_DATA *paf;
	char buf[MAX_STRING_LENGTH];
	int max = 100;

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
		if (IS_NPC(rch) && rch->spec_fun == spec_lookup("spec_blacksmith"))
			break;

	if (obj->condition == -1) {
		stc("That object is indestructable!\n\r", ch);
		return;
	}

	for (paf = obj->affected; paf != NULL; paf = paf->next)
		if (paf->type == gsn_acid_breath)
			max = 100 - (5 * paf->modifier);

	if (obj->condition >= max) {
		stc("That object cannot be repaired further.\n\r", ch);
		return;
	}

	if (!rch) {
		if (get_skill(ch, gsn_repair)) {
			if (!deduct_stamina(ch, gsn_repair))
				return;

			WAIT_STATE(ch, skill_table[gsn_repair].beats);

			if (number_percent() > get_skill(ch, gsn_repair)) {
				stc("You accidentally damage it more!\n\r", ch);
				check_improve(ch, gsn_repair, FALSE, 8);
				obj->condition -= number_range(10, 15);

				if (obj->condition < 0)
					obj->condition = 0;

				if (obj->condition == 0) {
					act("{W$p{x has been {Wdestroyed{x!", ch, obj, NULL, TO_CHAR);
					extract_obj(obj);
				}

				return;
			}

			sprintf(buf, "$n repairs $p to %s condition.",
			        condition_lookup(obj->condition));
			act(buf, rch, obj, 0, TO_ROOM);
			obj->condition = max;
			sprintf(buf, "You repair $p to %s condition.",
			        condition_lookup(obj->condition));
			act(buf, ch, obj, NULL, TO_CHAR);
			check_improve(ch, gsn_repair, TRUE, 8);
			return;
		}

		stc("No one here can repair that item.\n\r", ch);
		return;
	}

	if ((ch->silver + 100 * ch->gold) < (max - obj->condition) * 100) {
		act("$n says, 'You don't have enough money to repair $p.",
		    rch, obj, 0, TO_ROOM);
		return;
	}
	else {
		deduct_cost(ch, (max - obj->condition) * 100);
		sprintf(buf, "The repairs on %s has cost you %d gold.\n\r",
		        obj->short_descr, (max - obj->condition));
		stc(buf, ch);
	}

	obj->condition = max;
	sprintf(buf, "$n repairs $p to %s condition.",
	        condition_lookup(obj->condition));
	act(buf, rch, obj, 0, TO_ROOM);
}

bool spec_fight_clanguard(CHAR_DATA *ch)
{
	return TRUE;
}

bool spec_clanguard(CHAR_DATA *ch)
{
	CLAN_DATA *clan;
	CHAR_DATA *victim, *v_next;
	bool found = FALSE;

	if (!IS_NPC(ch)
	    || !IS_AWAKE(ch)
	    || IS_AFFECTED(ch, AFF_CALM | AFF_CHARM)
	    || ch->in_room == NULL)
		return FALSE;

	if ((clan = clan_vnum_lookup(ch->pIndexData->vnum)) == NULL)
		return FALSE;

	if (ch->in_room->clan != clan)
		return FALSE;

	for (victim = ch->in_room->people; victim != NULL; victim = v_next) {
		v_next = victim->next_in_room;

		if (victim->fighting == ch || IS_IMMORTAL(victim))
			continue;

		if (!victim->clan || victim->clan == clan || victim->inviters == clan)
			continue;

		if (!clan_opponents(clan, victim->clan))
			continue;

		if (!found) {
			REMOVE_BIT(ch->comm, COMM_NOCHANNELS);
			do_yell(ch, "Invaders!  KILL THE INFIDELS!!");
		}

		multi_hit(ch, victim, TYPE_UNDEFINED);
		found = TRUE;
	}

	if (ch->fighting)
		spec_fight_clanguard(ch);

	return found;
}


/**
 * This function determines whether or not a character
 * is "special". This function was written by Outsider
 * to replace the IS_SPECIAL macro (merc.h).
 * The function returns TRUE if the character is special
 * and FALSE if they are not.
 *
 * Explanation:  this is a hack.  it is not a back door,
 * it is a means to get around the head and imp flags
 * for coders.  just because they don't carry the rank
 * of imp or head does not mean they don't need all
 * commands.  -- Montrey
 */
bool IS_SPECIAL(CHAR_DATA *ch)
{
	if (IS_NPC(ch))
		return FALSE;

	if (!str_cmp(ch->name, "Montrey")
	    || !str_cmp(ch->name, "Xenith"))
		return TRUE;

	return FALSE;
}

/*
This function attempts to make a pet into a familiar.
Newly made familiars cause the pcdata->familiar field to
point at the character's pet. The owner then gets a
bonus ability point in the same ability which is
highest in the pet.
-- Outsider
*/
void do_familiar(CHAR_DATA *ch, char *argument)
{
	sh_int max_stat, max_position;
	sh_int current_stat, current_position;

	/* first check to make sure this is a character */
	if (IS_NPC(ch))
		return;

	/* make sure we have the skill */
	if (get_skill(ch, gsn_familiar) < 1) {
		stc("You don't know how to create a familiar.\n\r", ch);
		return;
	}

	/* make sure we don't already have a familiar */
	if (ch->pcdata->familiar) {
		stc("You already have a familiar.\n\r", ch);
		return;
	}

	/* we need a pet */
	if (! ch->pet) {
		stc("You need a pet first.\n\r", ch);
		return;
	}

	/* give the player the max stat of the pet */
	max_stat = 0; max_position = 0;
	current_position = 0;

	while (current_position < MAX_STATS) {
		current_stat = ch->pet->perm_stat[current_position];

		if (current_stat > max_stat) {
			max_stat = current_stat;
			max_position = current_position;
		}

		current_position++;
	}

	ch->mod_stat[max_position] += 1;

	/* make sure stats don't get too high */
	if ((ch->perm_stat[max_position] + ch->mod_stat[max_position]) > 25)
		ch->mod_stat[max_position] -= 1;

	/* keep track of which stat we received + 1 */
	ch->pcdata->familiar = max_position + 1;
	stc("You feel at one with your pet.\n\r", ch);
	check_improve(ch, gsn_familiar, TRUE, 1);
	return;
}
