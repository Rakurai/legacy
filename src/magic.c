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
*           Russ Taylor (rtaylor@@pacinfo.com)                             *
*           Gabrielle Taylor (gtaylor@@pacinfo.com)                        *
*           Brian Moore (rom@@rom.efn.org)                                 *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

#include "merc.h"
#include "recycle.h"
#include "magic.h"
#include "lookup.h"
#include "affect.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_get);
DECLARE_DO_FUN(do_sacrifice);
DECLARE_DO_FUN(do_switch);
DECLARE_DO_FUN(do_say);
DECLARE_DO_FUN(do_scan);

/* global focus variable */
extern bool     focus;

void            raw_kill        args((CHAR_DATA *victim));

/* Local functions. */
void    say_spell       args((CHAR_DATA *ch, int sn));

/* imported functions */
bool    remove_obj      args((CHAR_DATA *ch, int iWear, bool fReplace));
void    wear_obj        args((CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace));
int     find_exit       args((CHAR_DATA *ch, char *arg));

/* Lookup a skill by name. */
int skill_lookup(const char *name)
{
	int sn;

	for (sn = 0; sn < MAX_SKILL; sn++) {
		if (skill_table[sn].name == NULL)
			break;

		if (!str_prefix1(name, skill_table[sn].name))
			return sn;
	}

	return -1;
} /* end skill_lookup */

int find_spell(CHAR_DATA *ch, const char *name)
{
	/* finds a spell the character can cast if possible */
	int sn, found = -1;

	if (IS_NPC(ch))
		return skill_lookup(name);

	for (sn = 0; sn < MAX_SKILL; sn++) {
		if (skill_table[sn].name == NULL)
			break;

		if (!str_prefix1(name, skill_table[sn].name)) {
			if (found == -1)
				found = sn;

			if (ch->level >= skill_table[sn].skill_level[ch->class] && ch->pcdata->learned[sn] > 0)
				return sn;
		}
	}

	return found;
} /* end find_spell */

/* Lookup a skill by slot number.  Used for object loading. */
int slot_lookup(int slot)
{
	extern bool fBootDb;
	int sn;

	if (slot <= 0)
		return -1;

	for (sn = 0; sn < MAX_SKILL; sn++)
		if (slot == skill_table[sn].slot)
			return sn;

	if (fBootDb) {
		bug("Slot_lookup: bad slot %d.", slot);
		abort();
	}

	return -1;
} /* end slot_lookup */

/* Utter mystical words for an sn. */
void say_spell(CHAR_DATA *ch, int sn)
{
	char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
	CHAR_DATA *rch;
	char *pName;
	int iSyl, length;
	struct syl_type {
		char   *old;
		char   *new;
	};
	static const struct syl_type syl_table[] = {
		{ " ",          " "             },
		{ "ar",         "abra"          },
		{ "au",         "kada"          },
		{ "bless",      "fido"          },
		{ "blind",      "nose"          },
		{ "bur",        "mosa"          },
		{ "cu",         "judi"          },
		{ "de",         "oculo"         },
		{ "en",         "unso"          },
		{ "light",      "dies"          },
		{ "lo",         "hi"            },
		{ "mor",        "zak"           },
		{ "move",       "sido"          },
		{ "ness",       "lacri"         },
		{ "ning",       "illa"          },
		{ "per",        "duda"          },
		{ "ra",         "gru"           },
		{ "fresh",      "ima"           },
		{ "re",         "candus"        },
		{ "son",        "sabru"         },
		{ "tect",       "infra"         },
		{ "tri",        "cula"          },
		{ "ven",        "nofo"          },
		{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
		{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
		{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
		{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
		{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
		{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
		{ "y", "l" }, { "z", "k" },
		{ "", "" }
	};
	buf[0] = '\0';

	for (pName = skill_table[sn].name; *pName != '\0'; pName += length) {
		for (iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++) {
			if (!str_prefix1(syl_table[iSyl].old, pName)) {
				strcat(buf, syl_table[iSyl].new);
				break;
			}
		}

		if (length == 0)
			length = 1;
	}

	sprintf(buf2, "$n utters the words, '%s'.", buf);
	sprintf(buf,  "$n utters the words, '%s'.", skill_table[sn].name);

	if (char_in_darena_room(ch))
		act(buf, ch, NULL, NULL, TO_VIEW);      /* for viewing rooms */

	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if (rch != ch) {
			if (ch->class == rch->class) {
				act(buf, ch, NULL, rch, TO_VICT);
				continue;
			}

			if (number_percent() < get_skill(rch, gsn_languages)) {
				act(buf, ch, NULL, rch, TO_VICT);
				check_improve(rch, gsn_languages, TRUE, 8);
			}
			else {
				act(buf2, ch, NULL, rch, TO_VICT);

				if (get_skill(rch, gsn_languages))
					check_improve(rch, gsn_languages, FALSE, 8);
			}
		}
	}
} /* end say_spell */

/* Fix those players assisting or hindering when they shouldn't */
/* i hate seeing things repeated a lot, moved this here to save text -- Montrey */
bool help_mob(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (!is_same_group(ch, victim->fighting) && !IS_SET(victim->act, ACT_PET)) {
		stc("You cannot attack/help that mobile!\n", ch);
		wiznet("$N is attempting to help/hinder a mobile illegally.", ch, NULL, WIZ_CHEAT, 0, GET_RANK(ch));
		return TRUE;
	}

	return FALSE;
} /* end help_mob */

/* The kludgy global is for spells who want more stuff from command line. */
const char *target_name;

void do_cast(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	void *vo;
	int mana, sn, target, wait;

	/* Switched NPC's can cast spells, but others can't. */
	if (IS_NPC(ch) && ch->desc == NULL)
		return;

	if (IS_NPC(ch) && IS_SET(ch->act, ACT_MORPH)) {
		stc("Morphed players cannot cast spells.\n", ch);
		return;
	}

	target_name = one_argument(argument, arg1);
	one_argument(target_name, arg2);

	if (arg1[0] == '\0') {
		stc("Cast which what where?\n", ch);
		return;
	}

	if ((sn = find_spell(ch, arg1)) < 0
	    || (!IS_NPC(ch) && (ch->level < skill_table[sn].skill_level[ch->class]
	                        || ch->pcdata->learned[sn] == 0))) {
		stc("You don't know any spells of that name.\n", ch);
		return;
	}

	if (skill_table[sn].spell_fun == spell_null) {
		stc("That's not a spell.\n", ch);
		wiznet("$N is attempting to use the cast skill as spell cheat.", ch, NULL, WIZ_CHEAT, 0, GET_RANK(ch));
		return;
	}

	if (skill_table[sn].remort_class > 0)
		if (!CAN_USE_RSKILL(ch, sn)) {
			stc("You don't know any spells of that name.\n", ch);
			return;
		}

	if (!IS_IMMORTAL(ch)
	    && (get_position(ch) < skill_table[sn].minimum_position)) {
		stc("You can't concentrate enough.\n", ch);
		return;
	}

	mana = get_skill_cost(ch, sn);
	/* Locate targets. */
	victim      = NULL;
	obj         = NULL;
	vo          = NULL;
	target      = TARGET_NONE;

	switch (skill_table[sn].target) {
	default:
		bug("Do_cast: bad target for sn %d.", sn);
		return;

	case TAR_IGNORE:
		break;

	case TAR_CHAR_OFFENSIVE:
		if (arg2[0] == '\0') {
			if ((victim = ch->fighting) == NULL) {
				stc("Cast the spell on whom?\n", ch);
				return;
			}
		}
		else {
			if ((victim = get_char_here(ch, target_name, VIS_CHAR)) == NULL) {
				stc("They aren't here.\n", ch);
				return;
			}

			if (victim->fighting != NULL && !is_same_group(ch, victim->fighting)) {
				stc("Your intended opponent is already fighting someone else.\n", ch);
				return;
			}

			if (IS_NPC(victim) && victim->fighting != NULL && !is_same_group(ch, victim))
				if (help_mob(ch, victim))
					return;
		}

		if (!IS_NPC(ch)) {
			/* hack to make SLOW OK on follower players and yourself in safe rooms */
			if (skill_table[sn].spell_fun == spell_slow
			    && (victim->master == ch || victim == ch))
			{} /* Don't do anything - Poor programming, but I'm too tired to think clearly */
			else {
				if (is_safe(ch, victim, FALSE)) {
					stc("Not on that target.\n", ch);
					return;
				}

				check_killer(ch, victim);
			}
		}

		if (affect_exists_on_char(ch, gsn_charm_person) && ch->master == victim) {
			stc("You can't do that on your own follower.\n", ch);
			return;
		}

		vo = (void *) victim;
		target = TARGET_CHAR;
		break;

	case TAR_CHAR_DEFENSIVE:
		if (arg2[0] == '\0')
			victim = ch;
		else if ((victim = get_char_here(ch, target_name, VIS_CHAR)) == NULL) {
			stc("They aren't here.\n", ch);
			return;
		}

		if (IS_NPC(victim) && victim->fighting != NULL && !is_same_group(ch, victim))
			if (help_mob(ch, victim))
				return;

		vo = (void *) victim;
		target = TARGET_CHAR;
		break;

	case TAR_CHAR_SELF:
		if (arg2[0] != '\0' && !is_name(target_name, ch->name)) {
			stc("You cannot cast this spell on another.\n", ch);
			return;
		}

		vo = (void *) ch;
		target = TARGET_CHAR;
		break;

	case TAR_OBJ_INV:
		if (arg2[0] == '\0') {
			stc("What should the spell be cast upon?\n", ch);
			return;
		}

		if ((obj = get_obj_carry(ch, target_name)) == NULL) {
			stc("You are not carrying that.\n", ch);
			return;
		}

		vo = (void *) obj;
		target = TARGET_OBJ;
		break;

	case TAR_OBJ_CHAR_OFF:
		if (arg2[0] == '\0') {
			if ((victim = ch->fighting) == NULL) {
				stc("Cast the spell on whom or what?\n", ch);
				return;
			}

			target = TARGET_CHAR;
		}
		else if ((victim = get_char_here(ch, target_name, VIS_CHAR)) != NULL)
			target = TARGET_CHAR;

		if (target == TARGET_CHAR) { /* check the sanity of the attack */
			if (is_safe_spell(ch, victim, FALSE)) {
				stc("Not on that target.\n", ch);
				return;
			}

			if (IS_NPC(victim) && victim->fighting != NULL)
				if (help_mob(ch, victim))
					return;

			if (affect_exists_on_char(ch, gsn_charm_person) && ch->master == victim) {
				stc("You can't do that on your own follower.\n", ch);
				return;
			}

			if (!IS_NPC(ch))
				check_killer(ch, victim);

			vo = (void *) victim;
		}
		else if ((obj = get_obj_here(ch, target_name)) != NULL) {
			vo = (void *) obj;
			target = TARGET_OBJ;
		}
		else {
			stc("You don't see that here.\n", ch);
			return;
		}

		break;

	case TAR_OBJ_CHAR_DEF:
		if (arg2[0] == '\0') {
			vo = (void *) ch;
			target = TARGET_CHAR;
		}
		else if ((victim = get_char_here(ch, target_name, VIS_CHAR)) != NULL) {
			vo = (void *) victim;
			target = TARGET_CHAR;

			if (IS_NPC(victim) && victim->fighting != NULL)
				if (help_mob(ch, victim))
					return;
		}
		else if ((obj = get_obj_carry(ch, target_name)) != NULL) {
			vo = (void *) obj;
			target = TARGET_OBJ;
		}
		else {
			stc("You don't see that here.\n", ch);
			return;
		}

		break;
	}

	if (ch->mana < mana) {
		stc("You don't have enough mana.\n", ch);
		return;
	}

	if (sn != gsn_ventriloquate)
		say_spell(ch, sn);

	wait = skill_table[sn].beats;

	if ((ch->class == 0) || (ch->class == 1) || (ch->class == 4))
		wait -= wait / 4;

	/* remort affect - fast casting */
	if (HAS_RAFF(ch, RAFF_FASTCAST))
		wait -= wait / 4;

	/* remort affect - slow casting */
	if (HAS_RAFF(ch, RAFF_SLOWCAST))
		wait += wait / 4;

	WAIT_STATE(ch, wait);

	if (number_percent() > get_skill(ch, sn)) {
		stc("You lost your concentration.\n", ch);
		check_improve(ch, sn, FALSE, 1);
		ch->mana -= mana / 2;
	}
	else {
		ch->mana -= mana;
		(*skill_table[sn].spell_fun)(sn, ch->level, ch, vo, target, get_evolution(ch, sn));
		check_improve(ch, sn, TRUE, 1);
	}

	if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
	     || (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
	    && victim != ch
	    && victim->master != ch) {
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;

		for (vch = ch->in_room->people; vch; vch = vch_next) {
			vch_next = vch->next_in_room;

			if (victim == vch && victim->fighting == NULL) {
				check_killer(victim, ch);
				multi_hit(victim, ch, TYPE_UNDEFINED);
				break;
			}
		}
	}
} /* end do_cast */

/* MPCAST -- Elrac */
/* Might be more appropriate in mob_prog.c but here it can share the
   global 'target_name' variable.
   This is very similar to CAST but with many fewer restrictions and less
   bells & whistles. We assume the mobprogrammer knows what he's doing,
   the mob has the skill and PK restrictions are irrelevant. */
void do_mpcast(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	void *vo;
	int mana, sn, target;

	if (!IS_NPC(ch) || IS_SET(ch->act, ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	target_name = one_argument(argument, arg1);
	one_argument(target_name, arg2);

	if (arg1[0] == '\0') {
		stc("Cast which what where?\n", ch);
		return;
	}

	if ((sn = find_spell(ch, arg1)) < 0) {
		stc("You don't know any spells of that name.\n", ch);
		return;
	}

	if (get_position(ch) < skill_table[sn].minimum_position) {
		stc("You can't concentrate enough.\n", ch);
		return;
	}

	mana = get_skill_cost(ch, sn);
	/* Locate targets. */
	victim      = NULL;
	obj         = NULL;
	vo          = NULL;
	target      = TARGET_NONE;

	switch (skill_table[sn].target) {
	default:
		bug("mpcast: bad target for sn %d.", sn);
		return;

	case TAR_IGNORE:
		break;

	case TAR_CHAR_OFFENSIVE:
		if ((arg2[0] == '\0' && ch->fighting == NULL)
		    || (victim = get_char_here(ch, target_name, VIS_CHAR)) == NULL
		    || (affect_exists_on_char(ch, gsn_charm_person) && ch->master == victim))
			return;

		vo = (void *) victim;
		target = TARGET_CHAR;
		break;

	case TAR_CHAR_DEFENSIVE:
		if (arg2[0] == '\0')
			victim = ch;
		else if ((victim = get_char_here(ch, target_name, VIS_CHAR)) == NULL)
			return;

		vo = (void *) victim;
		target = TARGET_CHAR;
		break;

	case TAR_CHAR_SELF:
		if (arg2[0] != '\0' && !is_name(target_name, ch->name))
			return;

		vo = (void *) ch;
		target = TARGET_CHAR;
		break;

	case TAR_OBJ_INV:
		if (arg2[0] == '\0' || (obj = get_obj_carry(ch, target_name)) == NULL)
			return;

		vo = (void *) obj;
		target = TARGET_OBJ;
		break;

	case TAR_OBJ_CHAR_OFF:
		if (arg2[0] == '\0') {
			if ((victim = ch->fighting) == NULL)
				return;

			target = TARGET_CHAR;
		}
		else if ((victim = get_char_here(ch, target_name, VIS_CHAR)) != NULL)
			target = TARGET_CHAR;

		if (target == TARGET_CHAR) { /* check the sanity of the attack */
			if ((is_safe_spell(ch, victim, FALSE) && victim != ch)
			    || (affect_exists_on_char(ch, gsn_charm_person) && ch->master == victim))
				return;

			vo = (void *) victim;
		}
		else if ((obj = get_obj_here(ch, target_name)) != NULL) {
			vo = (void *) obj;
			target = TARGET_OBJ;
		}
		else
			return;

		break;

	case TAR_OBJ_CHAR_DEF:
		if (arg2[0] == '\0') {
			vo = (void *) ch;
			target = TARGET_CHAR;
		}
		else if ((victim = get_char_here(ch, target_name, VIS_CHAR)) != NULL) {
			vo = (void *) victim;
			target = TARGET_CHAR;
		}
		else if ((obj = get_obj_carry(ch, target_name)) != NULL) {
			vo = (void *) obj;
			target = TARGET_OBJ;
		}
		else
			return;

		break;
	}

	if (ch->mana < mana)
		return;

	if (sn != gsn_ventriloquate)
		say_spell(ch, sn);

	ch->mana -= mana;
	/* right now mobs cast spells at evolution 1 */
	(*skill_table[sn].spell_fun)(sn, ch->level, ch, vo, target, get_evolution(ch, sn));
} /* end do_mpcast */

/* Cast spells at targets using a magical object. */
void obj_cast_spell(int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj)
{
	void *vo;
	int target = TARGET_NONE;

	if (sn <= 0)
		return;

	if (sn >= MAX_SKILL || skill_table[sn].spell_fun == 0) {
		bug("Obj_cast_spell: bad sn %d.", sn);
		return;
	}

	switch (skill_table[sn].target) {
	default:
		bug("Obj_cast_spell: bad target for sn %d.", sn);
		return;

	case TAR_IGNORE:
		vo = NULL;
		break;

	case TAR_CHAR_OFFENSIVE:
		if (victim == NULL)
			victim = ch->fighting;

		if (victim == NULL) {
			stc("You can't do that.\n", ch);
			return;
		}

		if (!IS_NPC(ch)) {
			/* hack to make SLOW OK on follower players and yourself in safe rooms */
			if (skill_table[sn].spell_fun == spell_slow
			    && (victim->master == ch || victim == ch))
			{} /* Don't do anything - Poor programming, but I'm too tired to think clearly */
			else {
				if (is_safe_spell(ch, victim, FALSE)) {
					stc("Not on that target.\n", ch);
					return;
				}

				check_killer(ch, victim);
			}

			if (IS_NPC(victim) && victim->fighting != NULL)
				if (help_mob(ch, victim))
					return;
		}

		vo = (void *) victim;
		target = TARGET_CHAR;
		break;

	case TAR_CHAR_SELF:
		if (victim == NULL)
			victim = ch;

		if (victim != ch) {
			stc("You cannot cast this spell on another.\n", ch);
			return;
		}

	/* no BREAK here -- fall through to TAR_CHAR_DEFENSIVE case */
	case TAR_CHAR_DEFENSIVE:
		if (victim == NULL && obj != NULL) {
			stc("This spell can not be used on objects.\n", ch);
			return;
		}

		vo = (void *) victim;

		if (IS_NPC(victim) && victim->fighting != NULL)
			if (help_mob(ch, victim))
				return;

		target = TARGET_CHAR;
		break;

	case TAR_OBJ_INV:
		if (obj == NULL) {
			stc("You can't do that.\n", ch);
			return;
		}

		vo = (void *) obj;
		target = TARGET_OBJ;
		break;

	case TAR_OBJ_CHAR_OFF:
		if (victim == NULL && obj == NULL) {
			if (ch->fighting != NULL)
				victim = ch->fighting;
			else {
				stc("You can't do that.\n", ch);
				return;
			}

			if (victim != NULL) {
				if (is_safe_spell(ch, victim, FALSE)) {
					stc("Something isn't right...\n", ch);
					return;
				}

				if (IS_NPC(victim) && victim->fighting != NULL)
					if (help_mob(ch, victim))
						return;

				vo = (void *) victim;
				target = TARGET_CHAR;
			}
			else {
				vo = (void *) obj;
				target = TARGET_OBJ;
			}
		}
		else {
			if (victim != NULL) {
				vo = (void *) victim;
				target = TARGET_CHAR;
			}
			else {
				vo = (void *) obj;
				target = TARGET_OBJ;
			}
		}

		break;

	case TAR_OBJ_CHAR_DEF:
		if (victim == NULL && obj == NULL) {
			vo = (void *) ch;
			target = TARGET_CHAR;
		}
		else if (victim != NULL) {
			if (IS_NPC(victim) && victim->fighting != NULL)
				if (help_mob(ch, victim))
					return;

			vo = (void *) victim;
			target = TARGET_CHAR;
		}
		else {
			if (obj->item_type == ITEM_CORPSE_PC) {
				stc("You cannot cast that spell on a corpse.\n", ch);
				return;
			}

			vo = (void *) obj;
			target = TARGET_OBJ;
		}

		break;
	}

	if (!target_name)
		/* target_name = str_dup(""); -- Looks like a memory leak. */
		target_name = "";

	/* I don't see why this is here. Taking it out
	   so that wands will work.
	   -- Outsider
	else
	     target_name[0] = '\0';
	*/
	/* right now objects cast spells at evolution 1 */
	focus = FALSE;
	(*skill_table[sn].spell_fun)(sn, level, ch, vo, target, 1);
	focus = TRUE;

	if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
	     || (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
	    && victim != ch
	    && victim->master != ch) {
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;

		for (vch = ch->in_room->people; vch; vch = vch_next) {
			vch_next = vch->next_in_room;

			if (victim == vch && victim->fighting == NULL) {
				check_killer(victim, ch);
				multi_hit(victim, ch, TYPE_UNDEFINED);
				break;
			}
		}
	}
} /* end obj_cast_spell */

/* Spell functions. */

void spell_acid_blast(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	dam = dice(level, 15);

	if (saves_spell(level, victim, DAM_ACID))
		dam /= 2;

	damage(ch, victim, dam, sn, DAM_ACID, TRUE, TRUE);
}

/* Necromancy spells by Lotus */
void animate_mob(CHAR_DATA *ch, int level, const char *name, long vnum)
{
	CHAR_DATA *mob;
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC(ch)) {
		stc("Mobiles are not skilled Necromancers.\n", ch);
		return;
	}

	if ((vnum == MOB_VNUM_ZOMBIE   && ch->pcdata->zombie   != NULL)
	    || (vnum == MOB_VNUM_SKELETON && ch->pcdata->skeleton != NULL)
	    || (vnum == MOB_VNUM_WRAITH   && ch->pcdata->wraith   != NULL)
	    || (vnum == MOB_VNUM_GARGOYLE && ch->pcdata->gargoyle != NULL)) {
		ptc(ch, "You have already summoned a %s!\n", name);
		return;
	}

	if (IS_SET(GET_ROOM_FLAGS(ch->in_room), ROOM_LAW)) {
		stc("You cannot summon creatures within the city.\n", ch);
		return;
	}

	if (IS_SET(GET_ROOM_FLAGS(ch->in_room), ROOM_SAFE)) {
		stc("Mobiles cannot be summoned in holy or unholy rooms.\n", ch);
		return;
	}

	mob = create_mobile(get_mob_index(vnum));

	/* Check for memory error. -- Outsider */
	if (! mob) {
		bug("Memory error creating mob in animate_mob().", 0);
		stc("Could not create your servent.\n", ch);
		return;
	}

	char_to_room(mob, ch->in_room);
	act("$n magically creates a $N!\n", ch, NULL, mob, TO_ROOM);
	ptc(ch, "You have created a %s to do your bidding.\n", name);
	mob->nectimer = level / 10;

	make_pet(ch, mob);

	sprintf(buf, "%sSummoned from the darkside, this %s serves '%s'.\n",
	        mob->description, name, ch->name);
	free_string(mob->description);
	mob->description = str_dup(buf);

	switch (vnum) {
	case MOB_VNUM_ZOMBIE:           ch->pcdata->zombie = mob;       break;

	case MOB_VNUM_SKELETON:         ch->pcdata->skeleton = mob;     break;

	case MOB_VNUM_WRAITH:           ch->pcdata->wraith = mob;       break;

	case MOB_VNUM_GARGOYLE:         ch->pcdata->gargoyle = mob;     break;
	}
}

void spell_animate_zombie(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	animate_mob(ch, level, "zombie", MOB_VNUM_ZOMBIE);
}

void spell_animate_skeleton(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	animate_mob(ch, level, "skeleton", MOB_VNUM_SKELETON);
}

void spell_animate_wraith(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	animate_mob(ch, level, "wraith", MOB_VNUM_WRAITH);
}

void spell_animate_gargoyle(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	animate_mob(ch, level, "gargoyle", MOB_VNUM_GARGOYLE);
}

void spell_armor(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, sn)) {
		if (victim == ch)
			stc("You are already armored.\n", ch);
		else
			act("$N is already armored.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		24,
		evolution,
		FALSE
	);

	stc("You feel someone protecting you.\n", victim);

	if (ch != victim)
		act("$N is protected by your magic.", ch, NULL, victim, TO_CHAR);
}

void spell_steel_mist(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, sn)) {
		if (victim == ch)
			stc("Your armor is already coated with magical steel.\n", ch);
		else
			act("$N's armor is already coated with magical steel.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		36,
		evolution,
		FALSE
	);

	stc("A veil of mist covers your armor.\n", victim);

	if (ch != victim)
		act("A veil of steel covers $N's armor.", ch, NULL, victim, TO_CHAR);
}

/* Blood Moon by Lotus */
void spell_blood_moon(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, sn)) {
		if (victim == ch)
			stc("You are already bloodthirsty.\n", ch);
		else
			act("$N is already bloodthirsty.", ch, NULL, victim, TO_CHAR);

		return;
	}

	if (victim->alignment > -500  && !IS_IMMORTAL(ch)) {
		if (victim != ch)
			stc("This spell is much too evil for them.\n", ch);
		else
			stc("This spell is much too evil for you.\n", ch);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level,
		evolution,
		FALSE
	);

	stc("You have been blessed with the power of the vampire.\n", victim);
	stc("You are thirsty for blood.\n", victim);

	if (!IS_NPC(victim) && !HAS_RAFF(victim, RAFF_NOTHIRST))
		victim->pcdata->condition[COND_THIRST] = 0;

	if (ch != victim)
		act("$N is now thirsting for blood.", ch, NULL, victim, TO_CHAR);
}

void spell_bless(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af = (AFFECT_DATA){0};

	if (target == TARGET_OBJ) {
		obj = (OBJ_DATA *) vo;

		if (IS_OBJ_STAT(obj, ITEM_BLESS)) {
			act("$p is already blessed.", ch, obj, NULL, TO_CHAR);
			return;
		}

		if (IS_OBJ_STAT(obj, ITEM_EVIL)) {
			// is it cursed or just evil?
			if (affect_exists_on_obj(obj, gsn_curse))
				check_dispel_obj(level, obj, gsn_curse, TRUE);
			else if (!level_save(level, obj->level))
				REMOVE_BIT(obj->extra_flags, ITEM_EVIL);

			if (IS_OBJ_STAT(obj, ITEM_EVIL)) // still evil?
				act("The evil of $p is too powerful for you to overcome.", ch, obj, NULL, TO_CHAR);
			else
				act("$p glows a pale blue.", ch, obj, NULL, TO_ALL);

			return;
		}

		af.where        = TO_OBJECT;
		af.type         = sn;
		af.level        = level;
		af.duration     = 6 + level;
		af.location     = APPLY_SAVES;
		af.modifier     = -1;
		af.bitvector    = ITEM_BLESS;
		af.evolution = evolution;
		affect_copy_to_obj(obj, &af);
		act("$p glows with a holy aura.", ch, obj, NULL, TO_ALL);
		return;
	}

	/* character target */
	victim = (CHAR_DATA *) vo;

	if (victim->fighting) {
		if (victim == ch)
			stc("The attempt to bless you during battle was bound to fail.\n", ch);
		else
			act("$N cannot receive a blessing during battle.", ch, NULL, victim, TO_CHAR);

		return;
	}

	if (victim->alignment < 500 && !IS_IMMORTAL(ch)) {
		if (victim != ch)
			stc("This spell is much too good for them.\n", ch);
		else
			stc("This spell is much too good for you.\n", ch);

		return;
	}

	if (affect_exists_on_char(victim, sn)) {
		if (victim == ch)
			stc("You are already blessed.\n", ch);
		else
			act("$N already has divine favor.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level + 6,
		evolution,
		FALSE
	);

	stc("You feel righteous.\n", victim);

	if (ch != victim)
		act("You grant $N the favor of your god.", ch, NULL, victim, TO_CHAR);
}

void spell_blindness(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	// intentionally allow blindness even in addition to other (dirt kicking etc)

	if (affect_exists_on_char(victim, gsn_blindness)) {
		stc("Your victim doesn't have any sight to lose.\n", ch);
		return;
	}

	if (saves_spell(level, victim, DAM_OTHER)) {
		stc("Your victim blinks but continues to see.\n", ch);
		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level + 1,
		evolution,
		FALSE
	);

	stc("You are blinded!\n", victim);
	act("$n appears to be blinded.", victim, NULL, NULL, TO_ROOM);
}

/* chain spell function -- Montrey */
void chain_spell(CHAR_DATA *ch, void *vo, int sn, int type, int level)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *tmp_vict, *last_vict, *next_vict;
	bool found;
	int dam;
	struct chain_type {
		int start_damage;
		int rebound_damage;
		int elem_type;
		char *start_msg_notvict;
		char *start_msg_char;
		char *start_msg_vict;
		char *rebound_msg_room;
		char *rebound_msg_vict;
		char *fizzle_msg_room;
		char *fizzle_msg_char;
		char *doh_msg_room;
		char *doh_msg_char;
	};
	static const struct chain_type chain_table[] = {
		{
			6,    6,      DAM_ELECTRICITY,                /* Chain Lightning */
			"A lightning bolt leaps from $n's hand and arcs to $N.",
			"A lightning bolt leaps from your hand and arcs to $N.",
			"A lightning bolt leaps from $n's hand and hits you!",
			"The bolt arcs to $n!",
			"The bolt hits you!",
			"The bolt seems to have fizzled out.",
			"The bolt grounds out through your body.",
			"The bolt arcs to $n...whoops!",
			"You are struck by your own lightning!"
		},

		{
			8,    12,     DAM_COLD,               /* Blizzard by Elrac */
			"A flurry of snow flies from $n's hand and smacks into $N!",
			"A flurry of snow flies from your hand and smacks into $N!",
			"A flurry of snow flies from $n's hand and smacks into you!",
			"The avalanche rebounds and smacks $n!",
			"The avalanche rebounds and smacks you!",
			"The blizzard expends its energy.",
			"Your blizzard expends its energy.",
			"The blizzard sweeps over $n.  Whoops!",
			"You are engulfed by your own blizzard!"
		},

		{
			8,    12,     DAM_ACID,               /* Acid Rain by Tarrant */
			"A torrent of acid rain flies from $n's hand and smacks into $N!",
			"A torrent of acid rain flies from your hand and smacks into $N!",
			"A torrent of acid rain flies from $n's hand and smacks into you!",
			"The torrent rebounds and smacks $n!",
			"The torrent rebounds and smacks you!",
			"The storm expends its energy.",
			"Your storm expends its energy.",
			"The acid rain sweeps over $n.  Whoops!",
			"You are engulfed by your own acid rain!"
		},

		{
			12,   12,     DAM_FIRE,               /* Firestorm by Lotus */
			"A flame flies from $n's hand and smacks into $N!",
			"A flame flies from your hand and smacks into $N!",
			"A flame flies from $n's hand and smacks into you!",
			"The flame arcs over and smacks $n!",
			"The flame arcs over and smacks you!",
			"The flame dies down and goes out.",
			"Your flame dies down and goes out.",
			"The flame arcs to $n.  Whoops!",
			"You are hit by your own flame!"
		}
	};
	act(chain_table[type].start_msg_notvict, ch, NULL, victim, TO_NOTVICT);
	act(chain_table[type].start_msg_char, ch, NULL, victim, TO_CHAR);
	act(chain_table[type].start_msg_vict, ch, NULL, victim, TO_VICT);
	dam = dice(level, chain_table[type].start_damage);

	if (saves_spell(level, victim, chain_table[type].elem_type))
		dam /= 3;

	damage(ch, victim, dam, sn, chain_table[type].elem_type, TRUE, TRUE);
	last_vict = victim;
	level -= 4;

	/* new targets!!! */
	while (level > 0) {
		found = FALSE;

		for (tmp_vict = ch->in_room->people; tmp_vict != NULL; tmp_vict = next_vict) {
			next_vict = tmp_vict->next_in_room;

			if (!is_same_group(tmp_vict, ch)
			    && !is_safe(ch, tmp_vict, FALSE)
			    && !is_safe_spell(ch, tmp_vict, TRUE)
			    && tmp_vict != last_vict) {
				found = TRUE;
				last_vict = tmp_vict;
				act(chain_table[type].rebound_msg_room, tmp_vict, NULL, NULL, TO_ROOM);
				act(chain_table[type].rebound_msg_vict, tmp_vict, NULL, NULL, TO_CHAR);
				dam = dice(level, chain_table[type].rebound_damage);

				if (saves_spell(level, tmp_vict, chain_table[type].elem_type))
					dam /= 3;

				damage(ch, tmp_vict, dam, sn, chain_table[type].elem_type, TRUE, TRUE);
				level -= 4;
			}
		}

		if (!found) {    /* no target, hit caster */
			if (ch == NULL)
				return;

			if (last_vict == ch) { /* no double hits */
				act(chain_table[type].fizzle_msg_room, ch, NULL, NULL, TO_ROOM);
				act(chain_table[type].fizzle_msg_char, ch, NULL, NULL, TO_CHAR);
				return;
			}

			last_vict = ch;
			act(chain_table[type].doh_msg_room, ch, NULL, NULL, TO_ROOM);
			act(chain_table[type].doh_msg_char, ch, NULL, NULL, TO_CHAR);
			dam = dice(level, chain_table[type].rebound_damage);

			if (saves_spell(level, ch, chain_table[type].elem_type))
				dam /= 3;

			damage(ch, ch, dam, sn, chain_table[type].elem_type, TRUE, TRUE);
			level -= 4;

			if (ch == NULL)
				return;
		}
	}
}

void spell_chain_lightning(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	chain_spell(ch, vo, sn, 0, level);
}

void spell_blizzard(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	chain_spell(ch, vo, sn, 1, level);
}

void spell_acid_rain(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	chain_spell(ch, vo, sn, 2, level);
}

void spell_firestorm(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	chain_spell(ch, vo, sn, 3, level);
}

void spell_burning_hands(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	static const sh_int dam_each[] = {
		0,
		0,  0,  0,  0, 14,     17, 20, 23, 26, 29,
		29, 29, 30, 30, 31,     31, 32, 32, 33, 33,
		34, 34, 35, 35, 36,     36, 37, 37, 38, 38,
		39, 39, 40, 40, 41,     41, 42, 42, 43, 43,
		44, 44, 45, 45, 46,     46, 47, 47, 48, 48
	};
	level       = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
	level       = UMAX(0, level);
	dam         = number_range(dam_each[level] / 2, dam_each[level] * 2);

	if (saves_spell(level, victim, DAM_FIRE))
		dam /= 2;

	damage(ch, victim, dam, sn, DAM_FIRE, TRUE, TRUE);
	return;
}

void spell_dazzling_light(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int add, dur, i;

	/* To avoid crazy memory issues (see spell_fireproof) make
	sure the target is not the caster. -- Outsider */
	if (ch == vo)
		return;

	if (obj->item_type != ITEM_LIGHT) {
		stc("This spell can only enhance light sources.\n", ch);
		return;
	}

	if (obj->wear_loc != -1 || obj->carried_by == NULL) {
		stc("The light must be in your inventory.\n", ch);
		return;
	}

	if (affect_exists_on_obj(obj, sn)) {
		stc("That light is already quite dazzling.\n", ch);
		return;
	}

	act("$p shines bright enough to transfix your enemies!", ch, obj, NULL, TO_CHAR);
	act("$p shines forth with an intimidating dazzle.", ch, obj, NULL, TO_ROOM);
	dur = number_percent() + (level / 2);
	add = 0;

	for (i = 0; i < 5; i++) {
		if (number_percent() <= level)
			add++;
	}

	AFFECT_DATA af;
	af.where     = TO_OBJECT;
	af.type      = sn;
	af.level     = level;
	af.duration  = dur;
	af.location  = APPLY_HITROLL;
	af.modifier  = add;
	af.bitvector = 0;
	af.evolution = evolution;
	affect_copy_to_obj(obj, &af);

	af.location  = APPLY_DAMROLL;
	affect_copy_to_obj(obj, &af);
}

void spell_light_of_truth(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	/* Avoiding crazy memory issues from objects casting spells on characters...
	   Make sure caster isn't also the target. -- Outsider */
	if (ch == vo)
		return;

	if (obj->item_type != ITEM_LIGHT) {
		stc("This spell only enhances light sources.\n", ch);
		return;
	}

	if (obj->wear_loc != -1 || obj->carried_by == NULL) {
		stc("The light must be in your inventory.\n", ch);
		return;
	}

	if (affect_exists_on_obj(obj, sn)) {
		stc("That light is already somewhat enhanced.\n", ch);
		return;
	}

	AFFECT_DATA af;
	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = 0;
	af.evolution = evolution;
	affect_copy_to_obj(obj, &af); // add gsn_light_of_truth, add the others below

	if ((number_percent() + 5) < ch->pcdata->learned[sn]) {
		af.type = gsn_detect_evil;
		affect_copy_to_obj(obj, &af);
		act("$p throws a red aura around your evil surroundings.", ch, obj, NULL, TO_CHAR);
	}

	if ((number_percent() + 5) < ch->pcdata->learned[sn]) {
		af.type = gsn_detect_good;
		affect_copy_to_obj(obj, &af);
		act("$p shows you good things with a golden aura.", ch, obj, NULL, TO_CHAR);
	}

	if ((number_percent() + 15) < ch->pcdata->learned[sn]) {
		af.type = gsn_detect_invis;
		affect_copy_to_obj(obj, &af);
		act("$p suddenly reveals invisible objects!", ch, obj, NULL, TO_CHAR);
	}

	if ((number_percent() + 15) < ch->pcdata->learned[sn]) {
		af.type = gsn_detect_hidden;
		affect_copy_to_obj(obj, &af);
		act("$p shines into every nook and cranny about you.", ch, obj, NULL, TO_CHAR);
	}

	if ((number_percent() + 25) < ch->pcdata->learned[sn]) {
		af.type = gsn_detect_magic;
		affect_copy_to_obj(obj, &af);
		act("$p reflects strangely off some of your better equipment.", ch, obj, NULL, TO_CHAR);
	}
}

void spell_call_lightning(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int dam;

	if (!IS_OUTSIDE(ch)) {
		stc("You must be out of doors.\n", ch);
		return;
	}

	if (weather_info.sky < SKY_RAINING) {
		stc("You need bad weather.\n", ch);
		return;
	}

	dam = dice(level / 2, 8);
	stc("Heaven's lightning strikes your foes!\n", ch);
	act("$n calls heaven's lightning to strike $s foes!", ch, NULL, NULL, TO_ROOM);

	for (vch = char_list; vch != NULL; vch = vch_next) {
		vch_next = vch->next;

		if (vch->in_room == NULL)
			continue;

		if (vch->in_room == ch->in_room) {
			if (vch != ch)
				damage(ch, vch, saves_spell(level, vch, DAM_ELECTRICITY) ? dam / 2 : dam,
				       sn, DAM_ELECTRICITY, TRUE, TRUE);

			continue;
		}

		if (vch->in_room->area == ch->in_room->area && IS_OUTSIDE(vch) && IS_AWAKE(vch))
			stc("Lightning flashes in the sky.\n", vch);
	}
}

void spell_calm(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *vch;
	int mlevel = 0;
	int count = 0;
	int high_level = 0;
	int chance;
	bool failure = FALSE;

	/* get sum of all mobile levels in the room */
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (vch->fighting) {
			count++;

			if (IS_NPC(vch))
				mlevel += vch->level;
			else
				mlevel += vch->level / 2;

			high_level = UMAX(high_level, vch->level);
		}
	}

	/* compute chance of stopping combat */
	chance = 4 * level - high_level + 2 * count;

	if (!IS_IMMORTAL(ch)) {
		if (number_range(0, chance) < mlevel)
			failure = TRUE;
		else {
			for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
				if (IS_NPC(vch)
				    && (check_immune(vch, DAM_CHARM) >= 100 // TODO: this should check chance individually?
				        || IS_SET(vch->act, ACT_UNDEAD)))
					failure = TRUE;
				else if (affect_exists_on_char(vch, gsn_calm)
				         || affect_exists_on_char(vch, gsn_berserk)
				         || affect_exists_on_char(vch, gsn_frenzy))
					failure = TRUE;
			}
		}
	}

	if (failure) {
		if (count == 0)
			stc("Your spell accomplishes nothing.\n", ch);
		else
			stc("You try to calm those present but nobody pays attention to you.\n", ch);

		return;
	}

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		stc("A wave of calm passes over you.\n", vch);

		if (vch->fighting)
			stop_fighting(vch, FALSE);

		affect_add_sn_to_char(vch,
			sn,
			level,
			level/4,
			evolution,
			FALSE
		);
	}
}

void spell_cancellation(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	level += 2;

	if ((!IS_NPC(ch) && IS_NPC(victim) && !(affect_exists_on_char(ch, gsn_charm_person) && ch->master == victim))
	    || (!IS_NPC(victim) && ch != victim)) {
		stc("You failed, try dispel magic.\n", ch);
		return;
	}

	if (dispel_char(victim, level, TRUE))
		stc("Ok.\n", ch);
	else
		stc("Spell failed.\n", ch);
}

void spell_cause_light(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	damage(ch, (CHAR_DATA *) vo, (dice(level / 3, 4) + (level / 2)), sn, DAM_HARM, TRUE, TRUE);
}

void spell_cause_critical(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	damage(ch, (CHAR_DATA *) vo, (dice(level / 3, 10) + (level * 3 / 2)), sn, DAM_HARM, TRUE, TRUE);
}

void spell_cause_serious(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	damage(ch, (CHAR_DATA *) vo, (dice(level / 3, 6) + level), sn, DAM_HARM, TRUE, TRUE);
}

void spell_change_sex(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, sn)) {
		if (victim == ch)
			stc("You've already been changed.\n", ch);
		else
			act("$N has already had $S sex changed.", ch, NULL, victim, TO_CHAR);

		return;
	}

	if (saves_spell(level , victim, DAM_OTHER)) {
		if (victim == ch)
			stc("Perhaps against your wishes, you save yourself from your spell.\n", ch);
		else
			act("The strength of $N's personality saves $M from your spell.\n", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level * 3,
		evolution,
		FALSE
	);

	stc("You feel different.\n", victim);
	act("$n doesn't look like $mself anymore...", victim, NULL, NULL, TO_ROOM);
	return;
}

/* Channel spell by Lotus */
void spell_channel(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim;
	char camount[MAX_STRING_LENGTH];
	int amount, max;
	target_name = one_argument(target_name, camount);

	if ((victim = get_char_here(ch, target_name, VIS_CHAR)) == NULL) {
		stc("They aren't here.\n", ch);
		return;
	}

	amount = atoi(camount);
	max = ((GET_ATTR_STR(ch) + GET_ATTR_CON(ch)) * 5);

	if ((amount <= 0) || (ch == victim)) {
		stc("But the feedback would kill you!!\n", ch);
		return;
	}

	if ((amount > max) || (2 * amount > ch->mana)) {
		stc("You are too drained to channel that much power.\n", ch);
		return;
	}

	if (victim->mana < 31000)
		victim->mana += amount;

	ch->mana -= (2 * amount);

	affect_add_sn_to_char(ch,
		sn,
		level,
		level / 10,
		evolution,
		FALSE
	);

	stc("Raw energy tingles at your fingertips!\n", victim);
	stc("You feel drained as you channel your life energy.\n", ch);
}

/* Mass Charm by Lotus */
void spell_charm_person(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (victim == ch) {
		stc("You like yourself even better!\n", ch);
		return;
	}

	if (get_position(victim) < POS_RESTING) {
		stc("But they look so peaceful...\n", ch);
		return;
	}

	if (affect_exists_on_char(ch, gsn_charm_person)) {
		stc("You are charmed yourself, and thus unable to charm others.\n", ch);
		return;
	}

	if (affect_exists_on_char(victim, gsn_charm_person)) {
		act("$E is already charmed, there is nothing more you can do!", ch, NULL, victim, TO_CHAR);
		return;
	}

	int def = check_immune(victim, DAM_CHARM);

	if (!IS_IMMORTAL(ch)) {
		if (!IS_NPC(victim)) {
			stc("You cannot cast this spell on players.\n", ch);
			return;
		}

		if (IS_SET(GET_ROOM_FLAGS(victim->in_room), ROOM_LAW)) {
			stc("The mayor does not allow charming in the city limits.\n", ch);
			return;
		}

		if (is_safe(ch, victim, TRUE) || def >= 100) {
			act("$N scoffs at your attempt to charm $M.", ch, NULL, victim, TO_CHAR);
			return;
		}
	}

	if (number_percent() < (GET_ATTR_CHR(ch) * 3))
		level += 3;

	if (level < victim->level || saves_spell(level, victim, DAM_CHARM)) {
		act("$E does not succumb to your charm.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim->master)
		stop_follower(victim);

	add_follower(victim, ch);
	victim->leader = ch;

	affect_add_sn_to_char(victim,
		sn,
		level,
		number_fuzzy(level / 4),
		evolution,
		FALSE
	);

	act("Isn't $n just so nice?", ch, NULL, victim, TO_VICT);
	act("$N looks at you with adoring eyes.", ch, NULL, victim, TO_CHAR);
}

void spell_chill_touch(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	static const sh_int dam_each[] = {
		0,
		0,  0,  6,  7,  8,      9, 12, 13, 13, 13,
		14, 14, 14, 15, 15,     15, 16, 16, 16, 17,
		17, 17, 18, 18, 18,     19, 19, 19, 20, 20,
		20, 21, 21, 21, 22,     22, 22, 23, 23, 23,
		24, 24, 24, 25, 25,     25, 26, 26, 26, 27
	};

	level       = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
	level       = UMAX(0, level);
	int dam     = number_range(dam_each[level] / 2, dam_each[level] * 2);

	if (!saves_spell(level, victim, DAM_COLD)) {
		act("$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM);

		affect_add_sn_to_char(victim,
			sn,
			level,
			6,
			evolution,
			FALSE
		);
	}
	else
		dam /= 2;

	damage(ch, victim, dam, sn, DAM_COLD, TRUE, TRUE);
}

void spell_colour_spray(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	static const sh_int dam_each[] = {
		0,
		0,  0,  0,  0,  0,      0,  0,  0,  0,  0,
		30, 35, 40, 45, 50,     55, 55, 55, 56, 57,
		58, 58, 59, 60, 61,     61, 62, 63, 64, 64,
		65, 66, 67, 67, 68,     69, 70, 70, 71, 72,
		73, 73, 74, 75, 76,     76, 77, 78, 79, 79
	};
	level       = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
	level       = UMAX(0, level);
	dam         = number_range(dam_each[level] / 2,  dam_each[level] * 2);

	if (saves_spell(level, victim, DAM_LIGHT))
		dam /= 2;
	else
		spell_blindness(gsn_blindness, level / 3, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));

	damage(ch, victim, dam, sn, DAM_LIGHT, TRUE, TRUE);
}

void spell_continual_light(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *light;

	if (target_name[0] != '\0') { /* do a glow on some object */
		if ((light = get_obj_carry(ch, target_name)) == NULL) {
			stc("You don't see that here.\n", ch);
			return;
		}

		if (IS_OBJ_STAT(light, ITEM_GLOW)) {
			act("$p is already glowing.", ch, light, NULL, TO_CHAR);
			return;
		}

		SET_BIT(light->extra_flags, ITEM_GLOW);
		act("$p glows with a white light.", ch, light, NULL, TO_ALL);
		return;
	}

	light = create_object(get_obj_index(OBJ_VNUM_LIGHT_BALL), 0);

	if (! light) {
		bug("Memory error in spell_continual_light.", 0);
		stc("Error creating light source.\n", ch);
		return;
	}

	obj_to_room(light, ch->in_room);
	act("$n twiddles $s thumbs and $p appears.",   ch, light, NULL, TO_ROOM);
	act("You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR);
}

void spell_control_weather(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	if (!str_cmp(target_name, "better"))
		weather_info.change += dice(level / 3, 4);
	else if (!str_cmp(target_name, "worse"))
		weather_info.change -= dice(level / 3, 4);
	else
		stc("Do you want it to get better or worse?\n", ch);

	stc("Ok.\n", ch);
}

void spell_create_food(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *food = NULL;
	char *type;
	char buf[MAX_STRING_LENGTH];
	EXTRA_DESCR_DATA *ed;
	type = str_dup(target_name);

	if (target_name[0] == '\0') {
		switch (number_range(0, 4)) {
		case 0: type = str_dup("a Magic Mushroom");             break;

		case 1: type = str_dup("a Slice of Pizza");             break;

		case 2: type = str_dup("a slice of burnt toast");       break;

		case 3: type = str_dup("a delicious {Ycheesestick{x");  break;

		case 4: type = str_dup("a can of SPAM!!!");             break;
		}
	}

	food = create_object(get_obj_index(GEN_OBJ_FOOD), 0);

	if (! food) {
		bug("Memory error creating food.", 0);
		stc("You were unable to create food.\n", ch);
		return;
	}

	free_string(food->short_descr);
	food->short_descr = str_dup(type);
	free_string(food->name);
	sprintf(buf, "food %s", smash_bracket(food->short_descr));
	food->name = str_dup(buf);
	sprintf(buf, "%s{x is lying on the ground.", type);
	buf[0] = UPPER(buf[0]);
	free_string(food->description);
	food->description = str_dup(buf);
	free_string(food->material);
	food->material = str_dup("food");
	sprintf(buf, "This food was created by %s.\n", ch->name);
	ed = new_extra_descr();
	ed->keyword             = str_dup("chef");
	ed->description         = str_dup(buf);
	ed->next                = food->extra_descr;
	food->extra_descr       = ed;
	ed = new_extra_descr();
	ed->keyword             = str_dup("food");
	ed->description         = str_dup("It looks delicious.\n");
	ed->next                = food->extra_descr;
	food->extra_descr       = ed;
	food->value[0] = level / 2;
	food->value[1] = level;
	obj_to_room(food, ch->in_room);
	act("$p suddenly appears.", ch, food, NULL, TO_ROOM);
	act("$p suddenly appears.", ch, food, NULL, TO_CHAR);
}

void spell_create_rose(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *rose;
	char color[MIL];
	char buf[MAX_STRING_LENGTH];
	EXTRA_DESCR_DATA *ed;

	if (get_carry_number(ch) >= can_carry_n(ch)) {
		act("You can't carry any more items.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (get_carry_weight(ch) >= can_carry_w(ch)) {
		act("You can't carry any more weight.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (target_name[0] == '\0')
		strcpy(color, "red");   /* Red is the default */
	else
		strcpy(color, smash_tilde(target_name));

	rose = create_object(get_obj_index(GEN_OBJ_TREASURE), 0);

	if (! rose) {
		bug("Memory error creating a rose.", 0);
		stc("You were unable to create a rose.\n", ch);
		return;
	}

	ptc(ch, "You have created a beautiful %s{x rose.\n", color);
	sprintf(buf, "%s has created a beautiful %s{x rose.", ch->name, color);
	act(buf, ch, rose, NULL, TO_ROOM);
	sprintf(buf, "a %s{x rose", color);
	free_string(rose->short_descr);
	rose->short_descr = str_dup(buf);
	sprintf(buf, "A %s{x rose is lying on the ground.", color);
	free_string(rose->description);
	rose->description = str_dup(buf);
	free_string(rose->material);
	rose->material = str_dup("silk");
	free_string(rose->name);
	rose->name = str_dup("rose");
	sprintf(buf, "This rose was created by %s.\n", ch->name);
	ed = new_extra_descr();
	ed->keyword             = str_dup("florist");
	ed->description         = str_dup(buf);
	ed->next                = rose->extra_descr;
	rose->extra_descr       = ed;
	ed = new_extra_descr();
	ed->keyword             = str_dup("rose");
	ed->description         = str_dup("It's a beautiful rose, with a soft and romantic fragrance.\n");
	ed->next                = rose->extra_descr;
	rose->extra_descr       = ed;
	rose->value[1] = 1;
	obj_to_char(rose, ch);
}

/* Create Camp Site by Lotus */
void spell_encampment(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *camp;
	camp = create_object(get_obj_index(OBJ_VNUM_CAMP), 0);

	if (! camp) {
		bug("Memory error creating an encampment.", 0);
		stc("You were not able to create an encampment.\n", ch);
		return;
	}

	camp->timer = level;
	obj_to_room(camp, ch->in_room);
	act("$n has created a tent from pure energy.", ch, camp, NULL, TO_ROOM);
	stc("You create a small tent from energy.\n", ch);
}

/* Create Sign by Lotus */
void spell_create_sign(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *sign;
	char buf[MAX_STRING_LENGTH];
	EXTRA_DESCR_DATA *ed, *owner;
	sign = create_object(get_obj_index(OBJ_VNUM_SIGN), 0);

	if (! sign) {
		bug("Memory error trying to create sign.", 0);
		stc("You were unable to create a sign.\n", ch);
		return;
	}

	act("$n has created a road sign.", ch, sign, NULL, TO_ROOM);
	stc("You create a road sign.\n", ch);
	obj_to_room(sign, ch->in_room);
	sign->timer = level * 2;

	if (target_name == '\0') {
		stc("You failed to include words to write on the sign.\n", ch);
		return;
	}

	sprintf(buf, "%s\n", target_name);
	ed = new_extra_descr();
	ed->keyword         = str_dup("sign");
	ed->description     = str_dup(buf);
	ed->next            = sign->extra_descr;
	sign->extra_descr   = ed;
	sprintf(buf, "This road sign was created by %s.\n", ch->name);
	owner = new_extra_descr();
	owner->keyword        = str_dup("carpenter");
	owner->description    = str_dup(buf);
	owner->next           = sign->extra_descr;
	sign->extra_descr     = owner;
}

/* Create Vial by Lotus */
void spell_create_vial(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *vial;

	if (get_carry_number(ch) >= can_carry_n(ch)) {
		act("You can't carry any more items.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (get_carry_weight(ch) >= can_carry_w(ch)) {
		act("You can't carry any more weight.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	vial = create_object(get_obj_index(OBJ_VNUM_VIAL), 0);

	if (! vial) {
		bug("Memory error creating vial.", 0);
		stc("You were unable to create a vial.\n", ch);
		return;
	}

	act("$n has created a magical vial.", ch, vial, NULL, TO_ROOM);
	stc("You create a vial for brewing potions.\n", ch);
	obj_to_char(vial, ch);
	return;
}

/* Create Parchment by Lotus */
void spell_create_parchment(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *parch;

	if (get_carry_number(ch) >= can_carry_n(ch)) {
		act("You can't carry any more items.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (get_carry_weight(ch) >= can_carry_w(ch)) {
		act("You can't carry any more weight.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	parch = create_object(get_obj_index(OBJ_VNUM_PARCH), 0);

	if (! parch) {
		bug("Memory error trying to create parchment.", 0);
		stc("You were unable to create parchment.\n", ch);
		return;
	}

	act("$n has created a magical parchment.", ch, parch, NULL, TO_ROOM);
	stc("You create a parchment for scribing scrolls.\n", ch);
	obj_to_char(parch, ch);
	return;
}

void spell_create_spring(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *spring;
	spring = create_object(get_obj_index(OBJ_VNUM_SPRING), 0);

	/* Make sure we have enough memory. -- Outsider */
	if (! spring) {
		bug("Memory error trying to create spring in spell_create_spring() in magic.", 0);
		stc("You are unable to create a spring.\n", ch);
		return;
	}

	spring->timer = level;
	obj_to_room(spring, ch->in_room);
	act("$p flows from the ground.", ch, spring, NULL, TO_ROOM);
	act("$p flows from the ground.", ch, spring, NULL, TO_CHAR);
	return;
}

void spell_create_water(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int water;

	/* Make sure the target is an object. -- Outsider */
	if (ch == vo)
		return;

	if (obj->item_type != ITEM_DRINK_CON) {
		stc("It is unable to hold water.\n", ch);
		return;
	}

	if (obj->value[2] != LIQ_WATER && obj->value[1] != 0) {
		stc("It contains some other liquid.\n", ch);
		return;
	}

	water = UMIN(level * (weather_info.sky >= SKY_RAINING ? 4 : 2), obj->value[0] - obj->value[1]);

	if (water > 0) {
		obj->value[2] = LIQ_WATER;
		obj->value[1] += water;

		if (!is_name("water", obj->name)) {
			char buf[MAX_STRING_LENGTH];
			sprintf(buf, "%s water", obj->name);
			free_string(obj->name);
			obj->name = str_dup(buf);
		}

		act("$p is filled.", ch, obj, NULL, TO_CHAR);
	}
}

void spell_cure_blindness(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!affect_exists_on_char(victim, gsn_blindness)) {
		if (victim == ch)
			stc("You aren't blind.\n", ch);
		else
			act("$N doesn't appear to be blinded.", ch, NULL, victim, TO_CHAR);

		return;
	}

	if (!check_dispel_char(level, victim, gsn_blindness, FALSE))
		stc("Spell failed.\n", ch);
}

void spell_cure_light(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	victim->hit = UMIN(victim->hit + (dice(1, 8) + level / 3), ATTR_BASE(victim, APPLY_HIT));
	update_pos(victim);
	stc("You feel better!\n", victim);

	if (ch != victim)
		stc("Ok.\n", ch);
}

void spell_cure_serious(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	victim->hit = UMIN(victim->hit + (dice(2, 8) + level / 2), ATTR_BASE(victim, APPLY_HIT));
	update_pos(victim);
	stc("You feel better!\n", victim);

	if (ch != victim)
		stc("Ok.\n", ch);
}

void spell_cure_critical(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	victim->hit = UMIN(victim->hit + (dice(3, 8) + level - 6), ATTR_BASE(victim, APPLY_HIT));
	update_pos(victim);
	stc("You feel better!\n", victim);

	if (ch != victim)
		stc("Ok.\n", ch);
}

/* Darkness - Montrey */
void spell_darkness(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	ROOM_INDEX_DATA *room;
	AFFECT_DATA af = (AFFECT_DATA){0};

	if ((room = ch->in_room) == NULL) {
		stc("You fail.\n", ch);
		return;
	}

	if (room_is_very_dark(ch->in_room)) {
		stc("The room could not possibly get any darker.\n", ch);
		return;
	}

	if (IS_SET(GET_ROOM_FLAGS(room), ROOM_LAW)) {
		stc("The mayor forbids using this spell in the city.\n", ch);
		return;
	}

	if (IS_SET(GET_ROOM_FLAGS(room), ROOM_SAFE)) {
		stc("You cannot use this spell in holy rooms.\n", ch);
		return;
	}

	if (room_is_dark(room))
		stc("The darkness deepens.\n", ch);
	else
		stc("Inky black darkness surrounds you.\n", ch);

	af.where     = TO_ROOMFLAGS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 20;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = ROOM_NOLIGHT;
	af.evolution = evolution;
	affect_copy_to_room(room, &af);
}

/* Divine Healing by Lotus */
void spell_divine_healing(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	victim->hit = UMIN(victim->hit + (dice(15, 15) + (level * 2)), ATTR_BASE(victim, APPLY_HIT));
	update_pos(victim);
	stc("You feel much better!\n", victim);

	if (ch != victim)
		stc("Ok.\n", ch);
}

void spell_cure_disease(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!affect_exists_on_char(victim, gsn_plague)) {
		if (victim == ch)
			stc("You aren't ill.\n", ch);
		else
			act("$N doesn't appear to be diseased.", ch, NULL, victim, TO_CHAR);

		return;
	}

	if (!check_dispel_char(level, victim, gsn_plague, FALSE))
		stc("Spell failed.\n", ch);
}

void spell_cure_poison(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!affect_exists_on_char(victim, gsn_poison)) {
		if (victim == ch)
			stc("You aren't poisoned.\n", ch);
		else
			act("$N doesn't appear to be poisoned.", ch, NULL, victim, TO_CHAR);

		return;
	}

	if (check_dispel_char(level, victim, gsn_poison, FALSE)) {
		stc("A warm feeling runs through your body.\n", victim); // in addition to msg_off
	}
	else
		stc("Spell failed.\n", ch);
}

void spell_curse(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af = (AFFECT_DATA){0};

	/* deal with the object case first */
	if (target == TARGET_OBJ) {
		obj = (OBJ_DATA *) vo;

		if (IS_OBJ_STAT(obj, ITEM_EVIL)) {
			act("$p is already filled with evil.", ch, obj, NULL, TO_CHAR);
			return;
		}

		if (IS_OBJ_STAT(obj, ITEM_BLESS)) {
			// is it cursed or just evil?
			if (affect_exists_on_obj(obj, gsn_bless))
				check_dispel_obj(level, obj, gsn_bless, TRUE);
			else if (!level_save(level, obj->level))
				REMOVE_BIT(obj->extra_flags, ITEM_BLESS);

			if (IS_OBJ_STAT(obj, ITEM_BLESS)) // still good?
				act("The holy aura of $p is too powerful for you to overcome.", ch, obj, NULL, TO_CHAR);
			else
				act("$p glows with a red aura.", ch, obj, NULL, TO_ALL);

			return;
		}

		af.where        = TO_OBJECT;
		af.type         = sn;
		af.level        = level;
		af.duration     = 2 * level;
		af.location     = APPLY_SAVES;
		af.modifier     = +1;
		af.bitvector    = ITEM_EVIL;
		af.evolution = evolution;
		affect_copy_to_obj(obj, &af);
		act("$p glows with a malevolent aura.", ch, obj, NULL, TO_ALL);
		return;
	}

	/* character curses */
	victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, gsn_curse)) {
		if (ch == victim)
			stc("You are already cursed. You don't need a double whammy.\n", ch);
		else
			act("$N is already cursed. You don't want to KILL $M, do you?", ch, NULL, victim, TO_CHAR);

		return;
	}

	if (saves_spell(level, victim, DAM_NEGATIVE)) {
		if (ch == victim)
			stc("Self-pity protects you from your own curse.\n", ch);
		else
			act("Your spell fails to penetrate $N's magical defenses.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level * 2,
		evolution,
		FALSE
	);

	stc("You feel unclean.\n", victim);

	if (ch != victim)
		act("$N looks very uncomfortable.", ch, NULL, victim, TO_CHAR);
}

void spell_demonfire(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	int align;

	if (!IS_NPC(ch) && !IS_EVIL(ch)) {
		victim = ch;
		stc("The demons turn upon you!\n", ch);
	}

	if (ch->class != 5) /* Paladins */
		ch->alignment = UMAX(-1000, ch->alignment - 50);

	if (victim != ch) {
		act("$n calls forth the demons of Hell upon $N!", ch, NULL, victim, TO_ROOM);
		act("$n has assailed you with the demons of Hell!", ch, NULL, victim, TO_VICT);
		stc("You conjure forth the demons of hell!\n", ch);
	}

	dam = dice(level, 6);

	if (saves_spell(level, victim, DAM_NEGATIVE))
		dam /= 2;

	align = victim->alignment;
	align += 1200;
	dam = (dam * align) / 600;
	damage(ch, victim, dam, sn, DAM_NEGATIVE , TRUE, TRUE);

	if (ch->fighting != NULL)
		spell_curse(gsn_curse, 3 * level / 4, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
}

void spell_detect_evil(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, gsn_detect_evil)) {
		if (victim == ch)
			stc("You can already sense evil.\n", ch);
		else
			act("$N can already detect evil.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level,
		evolution,
		FALSE
	);

	stc("Your eyes tingle.\n", victim);

	if (ch != victim)
		stc("Ok.\n", ch);
}

void spell_detect_good(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, gsn_detect_good)) {
		if (victim == ch)
			stc("You can already sense good.\n", ch);
		else
			act("$N can already detect good.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level,
		evolution,
		FALSE
	);

	stc("Your eyes tingle.\n", victim);

	if (ch != victim)
		stc("Ok.\n", ch);
}

void spell_detect_hidden(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, gsn_detect_hidden)) {
		if (victim == ch)
			stc("You are already as alert as you can be. \n", ch);
		else
			act("$N can already sense hidden lifeforms.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level,
		evolution,
		FALSE
	);

	stc("Your awareness improves.\n", victim);

	if (ch != victim)
		stc("Ok.\n", ch);
}

void spell_detect_invis(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, gsn_detect_invis)) {
		if (victim == ch)
			stc("You can already see invisible.\n", ch);
		else
			act("$N can already see invisible things.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level,
		evolution,
		FALSE
	);

	stc("Your eyes tingle.\n", victim);

	if (ch != victim)
		stc("Ok.\n", ch);
}

void spell_detect_magic(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, gsn_detect_magic)) {
		if (victim == ch)
			stc("You can already sense magical auras.\n", ch);
		else
			act("$N can already detect magic.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level,
		evolution,
		FALSE
	);

	stc("Your eyes tingle.\n", victim);

	if (ch != victim)
		stc("Ok.\n", ch);
}

void spell_detect_poison(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	/* Avoid odd memory bug where the target might be a character.
	   -- Outsider
	*/
	if (ch == vo)
		return;

	if (obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD) {
		if (obj->value[3] != 0)
			stc("You smell poisonous fumes.\n", ch);
		else
			stc("It looks delicious.\n", ch);
	}
	else
		stc("It doesn't look poisoned.\n", ch);
}

void spell_dispel_evil(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	char buf[MAX_STRING_LENGTH];
	int dam;

	if (!IS_NPC(ch) && IS_EVIL(ch))
		victim = ch;

	if (IS_GOOD(victim)) {
		if (!IS_NPC(victim)) {
			ptc(victim, "%s protects you.\n", victim->pcdata->deity);
			sprintf(buf, "%s protects $n.", victim->pcdata->deity);
			act(buf, victim, NULL, NULL, TO_ROOM);
		}
		else {
			stc("Cyanos protects you.\n", victim);
			act("Cyanos protects $n.", victim, NULL, NULL, TO_ROOM);
		}

		return;
	}

	if (IS_NEUTRAL(victim)) {
		act("$N does not seem to be affected.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim->hit > (ch->level * 4))
		dam = dice(level, 4);
	else
		dam = UMAX(victim->hit, dice(level, 4));

	if (saves_spell(level, victim, DAM_HOLY))
		dam /= 2;

	damage(ch, victim, dam, sn, DAM_HOLY , TRUE, TRUE);
}

void spell_dispel_good(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (!IS_NPC(ch) && IS_GOOD(ch))
		victim = ch;

	if (IS_EVIL(victim)) {
		act("$N is protected by $S evil.", ch, NULL, victim, TO_ROOM);
		return;
	}

	if (IS_NEUTRAL(victim)) {
		act("$N does not seem to be affected.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim->hit > (ch->level * 4))
		dam = dice(level, 4);
	else
		dam = UMAX(victim->hit, dice(level, 4));

	if (saves_spell(level, victim, DAM_NEGATIVE))
		dam /= 2;

	damage(ch, victim, dam, sn, DAM_NEGATIVE , TRUE, TRUE);
}

void spell_dispel_magic(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (saves_spell(level, victim, DAM_OTHER)) {
		stc("You feel a brief tingling sensation.\n", victim);
		stc("You failed.\n", ch);
		return;
	}

	if (dispel_char(victim, level, FALSE)) {
		stc("Ok.\n", ch);
	}
	else
		stc("Spell failed.\n", ch);
}

void spell_earthquake(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	stc("The earth trembles beneath your feet!\n", ch);
	act("$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM);

	for (vch = char_list; vch != NULL; vch = vch_next) {
		vch_next = vch->next;

		if (vch->in_room == NULL)
			continue;

		if (vch->in_room == ch->in_room) {
			if (vch != ch && !is_safe_spell(ch, vch, TRUE)) {
				if (IS_FLYING(vch))
					damage(ch, vch, 0, sn, DAM_BASH, TRUE, TRUE);
				else
					damage(ch, vch, level + dice(2, 8), sn, DAM_BASH, TRUE, TRUE);
			}

			continue;
		}

		if (vch->in_room->area == ch->in_room->area)
			stc("The earth trembles and shivers.\n", vch);
	}
}

/* Shrink by Lotus */
void spell_shrink(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int result, fail;

	/* Avoid shrinking characters. -- Outsider */
	if (ch == vo)
		return;

	if (obj->wear_loc != -1 || obj->carried_by == NULL) {
		stc("The item must be carried to be enchanted.\n", ch);
		return;
	}

	if (obj->weight <= 0) {
		stc("But, the item is already weightless.\n", ch);
		return;
	}

	fail = 25;  /* base 25% chance of failure */

	/* find the bonuses, only in perm affects */
	for (const AFFECT_DATA *paf = affect_list_obj(obj); paf != NULL; paf = paf->next)
		fail += 20;

	/* apply other modifiers */
	fail -= level;

	if (IS_OBJ_STAT(obj, ITEM_BLESS))
		fail -= 15;

	if (IS_OBJ_STAT(obj, ITEM_GLOW))
		fail -= 5;

	if (IS_OBJ_STAT(obj, ITEM_MAGIC))
		fail += 20;

	fail = URANGE(5, fail, 85);
	result = number_percent();

	if (result < (fail / 5)) { /* Item Destroyed */
		act("$p implodes into nothingness!", ch, obj, NULL, TO_CHAR);
		act("$p implodes into nothingness!", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	}

	if (result < (fail / 3)) { /* item disenchanted */
		act("$p glows slightly, then dims.", ch, obj, NULL, TO_CHAR);
		act("$p glows slightly, then dims.", ch, obj, NULL, TO_ROOM);

		/* remove all affects */
		affect_remove_all_from_obj(obj);
		obj->extra_flags = 0;
		return;
	}

	if (result <= fail) { /* failed, no bad result */
		stc("Nothing seemed to happen.\n", ch);
		return;
	}

	obj_from_char(obj);     /* simple fix for subracting weight from the player */

	if (IS_IMMORTAL(ch)) {
		if (IS_HEAD(ch) || number_percent() > 50) {
			act("$p shines brilliantly with a silver aura.", ch, obj, NULL, TO_CHAR);
			act("$p shines brilliantly with a silver aura.", ch, obj, NULL, TO_ROOM);
			SET_BIT(obj->extra_flags, ITEM_MAGIC);
			obj->weight = 0;
		}
		else {
			act("$p shimmers with a silver aura.", ch, obj, NULL, TO_CHAR);
			act("$p shimmers with a silver aura.", ch, obj, NULL, TO_ROOM);
			SET_BIT(obj->extra_flags, ITEM_MAGIC);
			obj->weight = URANGE(0, obj->weight - 50, obj->weight);
		}
	}
	else if (result <= (90 - level / 5)) { /* success! */
		act("$p shimmers with a silver aura.", ch, obj, NULL, TO_CHAR);
		act("$p shimmers with a silver aura.", ch, obj, NULL, TO_ROOM);
		SET_BIT(obj->extra_flags, ITEM_MAGIC);
		obj->weight = URANGE(0, obj->weight - 50, obj->weight);
	}
	else { /* exceptional enchant */
		act("$p shines brilliantly with a silver aura.", ch, obj, NULL, TO_CHAR);
		act("$p shines brilliantly with a silver aura.", ch, obj, NULL, TO_ROOM);
		SET_BIT(obj->extra_flags, ITEM_MAGIC);
		obj->weight = 0;
	}

	if (obj->level < LEVEL_HERO)
		obj->level = UMIN(LEVEL_HERO - 1, obj->level + 1);

	obj_to_char(obj, ch);
}

void spell_enchant_armor(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int result, fail;
	int ac_bonus, added;
	bool ac_found = FALSE;

	/* Avoid memory error is the target is a character. -- Outsider */
	if (ch == vo)
		return;

	if ((obj->item_type != ITEM_ARMOR)
	    && (obj->item_type != ITEM_LIGHT) && (obj->item_type != ITEM_TREASURE)
	    && (obj->item_type != ITEM_CLOTHING) && (obj->item_type != ITEM_CONTAINER)
	    && (obj->item_type != ITEM_TRASH) && (obj->item_type != ITEM_GEM)
	    && (obj->item_type != ITEM_JEWELRY)) {
		stc("That isn't an armor.\n", ch);
		return;
	}

	if (obj->wear_loc != -1 || obj->carried_by == NULL) {
		stc("The item must be carried to be enchanted.\n", ch);
		return;
	}

	/* this means they have no bonus */
	ac_bonus = 0;
	fail = 25;  /* base 25% chance of failure */

	/* find the bonuses */
	// only in perm affects, don't count gems
	for (const AFFECT_DATA *paf = affect_list_obj(obj); paf != NULL; paf = paf->next) {
		if (paf->location == APPLY_AC) {
			ac_bonus = paf->modifier;
			ac_found = TRUE;
			fail += 5 * (ac_bonus * ac_bonus);
		}
		else /* things get a little harder */
			fail += 20;
	}

	/* apply other modifiers */
	fail -= level;

	if (IS_OBJ_STAT(obj, ITEM_BLESS))
		fail -= 15;

	if (IS_OBJ_STAT(obj, ITEM_GLOW))
		fail -= 5;

	if (IS_OBJ_STAT(obj, ITEM_MAGIC))
		fail += 20;

	fail = URANGE(5, fail, 85);
	result = number_percent();

	if (IS_IMMORTAL(ch))
		result = 100;

	/* the moment of truth */
	if (result < (fail / 5)) { /* item destroyed */
		act("$p flares blindingly... and evaporates!", ch, obj, NULL, TO_CHAR);
		act("$p flares blindingly... and evaporates!", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	}

	if (result < (fail / 3)) { /* item disenchanted */
		act("$p glows brightly, then fades...oops.", ch, obj, NULL, TO_CHAR);
		act("$p glows brightly, then fades.", ch, obj, NULL, TO_ROOM);

		/* remove all affects */
		affect_remove_all_from_obj(obj);
		obj->extra_flags = 0;
		return;
	}

	if (result <= fail) {  /* failed, no bad result */
		stc("Nothing seemed to happen.\n", ch);
		return;
	}

	if (IS_IMMORTAL(ch)) {
		if (IS_HEAD(ch) || number_percent() > 50) {
			/* exceptional enchant */
			act("$p glows a brilliant gold!", ch, obj, NULL, TO_CHAR);
			act("$p glows a brilliant gold!", ch, obj, NULL, TO_ROOM);
			SET_BIT(obj->extra_flags, ITEM_MAGIC);
			SET_BIT(obj->extra_flags, ITEM_GLOW);
			added = -2;
		}
		else {
			/* good enchant */
			act("$p shimmers with a gold aura.", ch, obj, NULL, TO_CHAR);
			act("$p shimmers with a gold aura.", ch, obj, NULL, TO_ROOM);
			SET_BIT(obj->extra_flags, ITEM_MAGIC);
			added = -1;
		}
	}
	else if (result <= (90 - level / 5)) { /* success! */
		/* good enchant */
		act("$p shimmers with a gold aura.", ch, obj, NULL, TO_CHAR);
		act("$p shimmers with a gold aura.", ch, obj, NULL, TO_ROOM);
		SET_BIT(obj->extra_flags, ITEM_MAGIC);
		added = -1;
	}
	else { /* exceptional enchant */
		act("$p glows a brilliant gold!", ch, obj, NULL, TO_CHAR);
		act("$p glows a brilliant gold!", ch, obj, NULL, TO_ROOM);
		SET_BIT(obj->extra_flags, ITEM_MAGIC);
		SET_BIT(obj->extra_flags, ITEM_GLOW);
		added = -2;
	}

	/* now add the enchantments */
	if (obj->level < LEVEL_HERO)
		obj->level = UMIN(LEVEL_HERO - 1, obj->level + 1);

	AFFECT_DATA af;
	af.where      = TO_OBJECT;
	af.type       = sn;
	af.level      = level;
	af.duration   = -1;
	af.location   = APPLY_AC;
	af.modifier   =  added;
	af.bitvector  = 0;
	af.evolution  = evolution;
	affect_join_to_obj(obj, &af);
}

void spell_enchant_weapon(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int result, fail;
	int hit_bonus, dam_bonus, added;
	bool hit_found = FALSE, dam_found = FALSE;

	/* Avoid memory issue when the target may be a character. -- Outsider */
	if (ch == vo)
		return;

	if ((obj->item_type != ITEM_WEAPON) && (obj->item_type != ITEM_ARMOR)
	    && (obj->item_type != ITEM_LIGHT) && (obj->item_type != ITEM_TREASURE)
	    && (obj->item_type != ITEM_CLOTHING) && (obj->item_type != ITEM_CONTAINER)
	    && (obj->item_type != ITEM_TRASH) && (obj->item_type != ITEM_GEM)
	    && (obj->item_type != ITEM_JEWELRY)) {
		stc("That isn't enchantable.\n", ch);
		return;
	}

	if (obj->wear_loc != -1 || obj->carried_by == NULL) {
		stc("The item must be carried to be enchanted.\n", ch);
		return;
	}

	/* this means they have no bonus */
	hit_bonus = 0;
	dam_bonus = 0;
	fail = 25;  /* base 25% chance of failure */

	/* find the bonuses, only in perm affects */
	for (const AFFECT_DATA *paf = affect_list_obj(obj); paf != NULL; paf = paf->next) {
		if (paf->location == APPLY_HITROLL) {
			hit_bonus = paf->modifier;
			hit_found = TRUE;
			fail += 2 * (hit_bonus * hit_bonus);

			if (obj->item_type != ITEM_WEAPON)
				fail += 10 * (hit_bonus * hit_bonus);
		}
		else if (paf->location == APPLY_DAMROLL) {
			dam_bonus = paf->modifier;
			dam_found = TRUE;
			fail += 2 * (dam_bonus * dam_bonus);

			if (obj->item_type != ITEM_WEAPON)
				fail += 10 * (dam_bonus * dam_bonus);
		}
		else /* things get a little harder */
			fail += 25;
	}

	/* apply other modifiers */
	fail -= 3 * level / 2;

	if (IS_OBJ_STAT(obj, ITEM_BLESS))
		fail -= 15;

	if (IS_OBJ_STAT(obj, ITEM_GLOW))
		fail -= 5;

	if (IS_OBJ_STAT(obj, ITEM_MAGIC))
		fail += 20;

	fail = URANGE(5, fail, 95);
	result = number_percent();

	if (IS_IMMORTAL(ch))
		result = 100;

	/* the moment of truth */
	if (result < (fail / 5)) { /* item destroyed */
		act("$p shivers violently and explodes!", ch, obj, NULL, TO_CHAR);
		act("$p shivers violently and explodes!", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	}

	if (result < (fail / 2)) { /* item disenchanted */
		act("$p glows brightly, then fades...oops.", ch, obj, NULL, TO_CHAR);
		act("$p glows brightly, then fades.", ch, obj, NULL, TO_ROOM);

		/* remove all affects */
		affect_remove_all_from_obj(obj);

		if (obj->item_type == ITEM_WEAPON) {
			if (IS_SET(obj->value[4], WEAPON_TWO_HANDS))
				obj->value[4] = WEAPON_TWO_HANDS;
			else
				obj->value[4] = 0;
		}

		obj->extra_flags = 0;
		return;
	}

	if (result <= fail) {  /* failed, no bad result */
		stc("Nothing seemed to happen.\n", ch);
		return;
	}

	if (IS_IMMORTAL(ch)) {
		if (IS_HEAD(ch) || number_percent() > 50) {
			/* exceptional enchant */
			act("$p glows a brilliant blue!", ch, obj, NULL, TO_CHAR);
			act("$p glows a brilliant blue!", ch, obj, NULL, TO_ROOM);
			SET_BIT(obj->extra_flags, ITEM_MAGIC);
			SET_BIT(obj->extra_flags, ITEM_GLOW);
			added = 2;
		}
		else {
			/* good enchant */
			act("$p glows blue.", ch, obj, NULL, TO_CHAR);
			act("$p glows blue.", ch, obj, NULL, TO_ROOM);
			SET_BIT(obj->extra_flags, ITEM_MAGIC);
			added = 1;
		}
	}
	else if (result <= (100 - level / 5)) { /* success! */
		act("$p glows blue.", ch, obj, NULL, TO_CHAR);
		act("$p glows blue.", ch, obj, NULL, TO_ROOM);
		SET_BIT(obj->extra_flags, ITEM_MAGIC);
		added = 1;
	}
	else { /* exceptional enchant */
		act("$p glows a brilliant blue!", ch, obj, NULL, TO_CHAR);
		act("$p glows a brilliant blue!", ch, obj, NULL, TO_ROOM);
		SET_BIT(obj->extra_flags, ITEM_MAGIC);
		SET_BIT(obj->extra_flags, ITEM_GLOW);
		added = 2;
	}

	/* now add the enchantments */
	if ((obj->item_type != ITEM_WEAPON) && (obj->level < 50)) {
		if (obj->level < 30)
			obj->level = obj->level + 20;
		else
			obj->level = 50;
	}

	if (obj->level < LEVEL_HERO - 1)
		obj->level = UMIN(LEVEL_HERO - 1, obj->level + 1);

	AFFECT_DATA af;
	af.where      = TO_OBJECT;
	af.type       = sn;
	af.level      = level;
	af.duration   = -1;
	af.location   = APPLY_DAMROLL;
	af.modifier   =  added;
	af.bitvector  = 0;
	af.evolution  = evolution;
	affect_join_to_obj(obj, &af);

	af.where      = TO_OBJECT;
	af.type       = sn;
	af.level      = level;
	af.duration   = -1;
	af.location   = APPLY_HITROLL;
	af.modifier   =  added;
	af.bitvector  = 0;
	af.evolution  = evolution;
	affect_join_to_obj(obj, &af);
}

/* Drain XP, MANA, HP, stamina.  Caster gains portions. */
void spell_energy_drain(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam, manadrain, stamdrain;

	if (victim != ch && ch->class != 5) /*Paladin*/
		ch->alignment = UMAX(-1000, ch->alignment - 50);

	if (saves_spell(level, victim, DAM_NEGATIVE)) {
		stc("No energy seems to flow.\n", ch);
		stc("You feel a momentary chill.\n", victim);
		return;
	}

	if (victim->in_room->sector_type != SECT_ARENA
	    && victim->in_room->sector_type != SECT_CLANARENA)
		gain_exp(victim, 0 - number_range(10 + level / 2, 10 + (3 * level / 2)));

	if (victim->mana > 0) {
		manadrain = victim->mana / UMAX(4, (GET_ATTR_SAVES(victim) / -5));
		manadrain += ch->level;
		manadrain = UMIN(victim->mana, manadrain);
		victim->mana -= manadrain;

		if (ch->mana < 20000)
			ch->mana += manadrain / 4;
	}

	if (victim->stam > 0) {
		stamdrain = victim->stam / UMAX(4, (GET_ATTR_SAVES(victim) / -5));
		stamdrain += ch->level;
		stamdrain = UMIN(victim->stam, stamdrain);
		victim->stam -= stamdrain;

		if (ch->stam < 20000)
			ch->stam += stamdrain / 4;
	}

	dam              = dice(4, level / 2);

	if (ch->hit < 20000)
		ch->hit += dam;

	stc("You feel your life slipping away!\n", victim);
	stc("Wow....what a rush!\n", ch);
	damage(ch, victim, dam, sn, DAM_NEGATIVE, TRUE, TRUE);
}

/* Fear by Lotus */
void spell_fear(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, gsn_fear)) {
		act("Your attack is senseless, $N is already shaking in $S boots!", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (saves_spell(level, victim, DAM_NEGATIVE)) {
		act("$E isn't afraid of you!", ch, NULL, victim, TO_CHAR);
		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level / 3,
		evolution,
		FALSE
	);

	stc("Oh Crap, you're gonna die! Run for your life!\n", victim);

	if (ch != victim)
		act("$N looks extremely frightened.", ch, NULL, victim, TO_CHAR);
}

/* fireball bash effect */
void fireball_bash(CHAR_DATA *ch, CHAR_DATA *victim, int level, int evolution, bool spread)
{
	int chance = 100;
	bool standfast = FALSE;
	chance += (SIZE_MEDIUM - victim->size) * 15;
	chance -= 2 * (GET_ATTR_STR(victim) - 10);
	chance += (level - victim->level) / 2;
	chance += (evolution - 1) * 10;

	if (IS_SET(victim->off_flags, OFF_FAST) || affect_exists_on_char(victim, gsn_haste))
		chance -= 15;

	chance -= ((victim->stam * 20) / ATTR_BASE(victim, APPLY_STAM));
	chance -= get_skill(victim, gsn_dodge) / 7;

	if (!can_see(victim, ch))
		chance += 20;

	if (!can_see(ch, victim))
		chance -= 20;

	if (affect_exists_on_char(victim, gsn_pass_door))
		chance -= chance / 4;

	if (spread)
		chance -= chance / 3;

	/* this spell is cast by mages at 1/4 the lag of bash, so,
	   cut the chances of the bash to 1/4 */
	chance /= 4;

	if (CAN_USE_RSKILL(victim, gsn_standfast)) {
		chance = chance * (100 - get_skill(victim, gsn_standfast));
		chance /= 100;
		standfast = TRUE;
	}

	chance = URANGE(5, chance, 95);

	if (chance(chance)) {
		char buf[MSL], fb_buf[MSL];
		int wait = number_range(1, 3) * PULSE_VIOLENCE;

		if (evolution == 2)
			sprintf(fb_buf, "{Hf{Gir{Web{Gal{Hl{x");
		else
			sprintf(fb_buf, "{Bf{Cir{Web{Cal{Bl{x");

		sprintf(buf, "The concussive force of $n's %s knocks you to the ground!", fb_buf);
		act(buf, ch, NULL, victim, TO_VICT);
		sprintf(buf, "The concussive force of your %s knocks $N to the ground!", fb_buf);
		act(buf, ch, NULL, victim, TO_CHAR);
		sprintf(buf, "The concussive force of $n's %s knocks $N to the ground!", fb_buf);
		act(buf, ch, NULL, victim, TO_NOTVICT);
		DAZE_STATE(victim, wait);
		WAIT_STATE(victim, wait);
		victim->position = POS_RESTING;

		if (standfast)
			check_improve(victim, gsn_standfast, FALSE, 1);
	}
	else if (standfast)
		check_improve(victim, gsn_standfast, TRUE, 1);
}

/* evolved fireball -- Montrey */
/* fireball_bash above is used for evolved form */
void spell_fireball(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim;
	char arg[MIL];
	int dam;

	if (IS_SET(GET_ROOM_FLAGS(ch->in_room), ROOM_SAFE) && !IS_IMMORTAL(ch)) {
		stc("Oddly enough, in this room you feel peaceful.\n", ch);
		return;
	}

	dam = dice((level / 3) + (IS_NPC(ch) ? 10 : 30), 9);

	/* this effectively increases by around 30% per evolve */
	switch (evolution) {
	case 3:         dam *= 2;       break;

	case 2:         dam += dam / 2;   break;

	default:                        break;
	}

	one_argument(target_name, arg);

	/* fireball the room! */
	if (!str_cmp(arg, "room") && evolution >= 2) {
		char buf[MSL], fb_buf[MSL];
		CHAR_DATA *victim_next;
		int newdam;
		int count = 0;

		// count targets
		for (victim = ch->in_room->people; victim; victim = victim->next)
			if (ch != victim
			 && !is_same_group(victim, ch)
			 && !is_safe_spell(ch, victim, TRUE))
				count++;

		if (count == 0) {
			stc("There are no enemies here, what purpose would it serve?\n", ch);
			return;
		}

		if (evolution == 2)
			sprintf(fb_buf, "{Hf{Gir{Web{Gal{Hl{x");
		else
			sprintf(fb_buf, "{Bf{Cir{Web{Cal{Bl{x");

		dam -= dam / 4;
		ptc(ch, "You blast the room with a powerful %s!\n", fb_buf);
		sprintf(buf, "$n blasts the room with a powerful %s!", fb_buf);
		act(buf, ch, NULL, NULL, TO_ROOM);

		for (victim = ch->in_room->people; victim; victim = victim_next) {
			victim_next = victim->next_in_room;

			if (ch == victim)
				continue;

			if (is_same_group(victim, ch)
			    || is_safe_spell(ch, victim, TRUE))
				continue;

			newdam = number_range(dam / 2, dam + dam / 2);
			damage(ch, victim, saves_spell(level, victim, DAM_FIRE) ? newdam / 2 : newdam,
			       sn, DAM_FIRE, TRUE, TRUE);

			if (evolution == 3
			    && victim != NULL
			    && victim->hit > 0
			    && get_position(victim) == POS_FIGHTING)
				fireball_bash(ch, victim, level, evolution, TRUE);
		}

		return;
	}

	if (target_name[0] == '\0') {
		if ((victim = ch->fighting) == NULL) {
			stc("Cast the spell on whom?\n", ch);
			/* This is a quick hack to give the player
			   back the mana they used to cast this spell.
			   Fireball works oddly, so this isn't handled
			   by the do_cast() function. -- Outsider
			*/
			ch->mana += get_skill_cost(ch, gsn_fireball);

			if (ch->mana > ATTR_BASE(ch, APPLY_MANA))
				ch->mana = ATTR_BASE(ch, APPLY_MANA);

			return;
		}
	}
	else {
		if ((victim = get_char_here(ch, target_name, VIS_CHAR)) == NULL) {
			stc("They aren't here.\n", ch);
			return;
		}

		if (victim->fighting != NULL && !is_same_group(ch, victim->fighting)) {
			stc("Your intended opponent is already fighting someone else.\n", ch);
			return;
		}

		if (IS_NPC(victim) && victim->fighting != NULL && !is_same_group(ch, victim))
			if (help_mob(ch, victim))
				return;
	}

	if (affect_exists_on_char(ch, gsn_charm_person) && ch->master == victim) {
		stc("You can't do that on your own follower.\n", ch);
		return;
	}

	damage(ch, victim, saves_spell(level, victim, DAM_FIRE) ? dam / 2 : dam, sn, DAM_FIRE, TRUE, TRUE);

	if (evolution == 3
	    && victim != NULL
	    && victim->hit > 0
	    && get_position(victim) == POS_FIGHTING)
		fireball_bash(ch, victim, level, evolution, FALSE);
}

void spell_fireproof(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA af = (AFFECT_DATA){0};

	/* This sounds really stupid....but....Here goes.
	   If an object is Fireproof AND a character equips it,
	   the object then tries to cast fireproof on the character. Since
	   the character (obviously) isn't an OBJ, this functions
	   passes bad params to the act() function. Thus causing memory
	   problems.
	   To avoid this, I am doing a check to see if "ch" and "vo"
	   are the same. If they are, then we don't cast the spell.
	   -- Outsider
	*/
	if (ch == vo)   /* a character is not a valid target */
		return;

	if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
		act("$p is already protected from burning.", ch, obj, NULL, TO_CHAR);
		return;
	}

	af.where     = TO_OBJECT;
	af.type      = sn;
	af.level     = level;
	af.duration  = number_fuzzy(level / 4);
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = ITEM_BURN_PROOF;
	af.evolution = evolution;
	affect_copy_to_obj(obj, &af);
	act("You protect $p from fire.", ch, obj, NULL, TO_CHAR);
	act("$p is surrounded by a protective aura.", ch, obj, NULL, TO_ROOM);
}

/* function for bladecraft spells -- Montrey */
bool enhance_blade(CHAR_DATA *ch, OBJ_DATA *obj, int sn, int level, int bit)
{
	AFFECT_DATA af = (AFFECT_DATA){0};

	if (obj->item_type != ITEM_WEAPON) {
		stc("This spell can only enhance weapons.\n", ch);
		return FALSE;
	}

	if (obj->wear_loc != -1 || obj->carried_by == NULL) {
		stc("The weapon must be in your inventory.\n", ch);
		return FALSE;
	}

	if (IS_WEAPON_STAT(obj, WEAPON_FLAMING)
	    || IS_WEAPON_STAT(obj, WEAPON_FROST)
	    || IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)
	    || IS_WEAPON_STAT(obj, WEAPON_SHOCKING)) {
		act("$p is already an enhanced weapon.", ch, obj, NULL, TO_CHAR);
		return FALSE;
	}

	af.where        = TO_WEAPON;
	af.type         = sn;
	af.level        = level;
	af.duration     = (number_percent() + (level / 2));
	af.location     = 0;
	af.modifier     = 0;
	af.bitvector    = bit;
	af.evolution    = get_evolution(ch, sn);
	affect_copy_to_obj(obj, &af);
	return TRUE;
}

void spell_flame_blade(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	/* We don't want to set the caster on fire. -- Outsider */
	if (ch == vo)
		return;

	if (enhance_blade(ch, obj, sn, level, WEAPON_FLAMING)) {
		act("$p bursts into a glorious flame!", ch, obj, NULL, TO_CHAR);
		act("$p bursts into a glorious flame!", ch, obj, NULL, TO_ROOM);
	}
}

void spell_frost_blade(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	/* Don't freeze characters. -- Outsider */
	if (ch == vo)
		return;

	if (enhance_blade(ch, obj, sn, level, WEAPON_FROST)) {
		act("$p is now cold to the touch.", ch, obj, NULL, TO_CHAR);
		act("$p fills the room with a bitter cold!", ch, obj, NULL, TO_ROOM);
	}
}

void spell_blood_blade(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	/* Don't mistakenly cast this spell on other characters. -- Outsider */
	if (ch == vo)
		return;

	if (enhance_blade(ch, obj, sn, level, WEAPON_VAMPIRIC)) {
		act("$p glows with an evil aura.", ch, obj, NULL, TO_CHAR);
		act("$p glows with an evil aura.", ch, obj, NULL, TO_ROOM);
	}
}

void spell_shock_blade(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	/* Avoid casting this on characters. -- Outsider */
	if (ch == vo) return;

	if (enhance_blade(ch, obj, sn, level, WEAPON_SHOCKING)) {
		act("$p crackles with pure energy.", ch, obj, NULL, TO_CHAR);
		act("$p fills the room with sparks of electricity!", ch, obj, NULL, TO_ROOM);
	}
}

void spell_flamestrike(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	dam = dice(6 + level / 2, 8);

	if (saves_spell(level, victim, DAM_FIRE))
		dam /= 2;

	damage(ch, victim, dam, sn, DAM_FIRE , TRUE, TRUE);
}

void spell_faerie_fire(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, gsn_faerie_fire)) {
		act("$N is already glowing pink!", ch, NULL, victim, TO_CHAR);
		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level,
		evolution,
		FALSE
	);

	stc("You are surrounded by a pink outline.\n", victim);
	act("$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM);
}

void spell_faerie_fog(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *ich;
	act("$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM);
	stc("You conjure a cloud of purple smoke.\n", ch);

	for (ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room) {
		if (ich->invis_level > 0)
			continue;

		if (ich == ch || saves_spell(level, ich, DAM_OTHER))
			continue;

		if (!affect_exists_on_char(ich, gsn_hide)
		    && !affect_exists_on_char(ich, gsn_sneak)
		    && !affect_exists_on_char(ich, gsn_invis)
		    && !affect_exists_on_char(ich, gsn_midnight))
			continue;

		affect_remove_sn_from_char(ich, gsn_invis);
		affect_remove_sn_from_char(ich, gsn_sneak);
		affect_remove_sn_from_char(ich, gsn_hide);
		affect_remove_sn_from_char(ich, gsn_midnight);
		act("$n is revealed!", ich, NULL, NULL, TO_ROOM);
		stc("You are revealed!\n", ich);
	}
}

void spell_farsight(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	if (is_blinded(ch)) {
		stc("Maybe it would help if you could see?\n", ch);
		return;
	}

	do_scan(ch, target_name);
}

void spell_floating_disc(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *disc, *floating;
	floating = get_eq_char(ch, WEAR_FLOAT);

	if (floating != NULL && IS_OBJ_STAT(floating, ITEM_NOREMOVE)) {
		act("You can't remove $p.", ch, floating, NULL, TO_CHAR);
		return;
	}

	disc = create_object(get_obj_index(OBJ_VNUM_DISC), 0);

	if (! disc) {
		bug("Memory error in spell_floating_disc", 0);
		stc("You were unable to create a floating disc.\n", ch);
		return;
	}

	disc->value[0]      = ch->level * 100; /* 10 pounds per level capacity */
	disc->value[3]      = ch->level * 50; /* 5 pounds per level max per item */
	disc->timer         = ch->level * 2 - number_range(0, level / 2);
	act("$n has created a floating black disc.", ch, NULL, NULL, TO_ROOM);
	stc("You create a floating disc.\n", ch);
	obj_to_char(disc, ch);
	wear_obj(ch, disc, TRUE);
}

void spell_fly(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (CAN_FLY(victim)) {
		if (victim == ch)
			stc("You already have the ability to fly.\n", ch);
		else
			act("$N doesn't need your help to fly.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level + 3,
		evolution,
		FALSE
	);

	do_fly(victim, "");
}

void spell_frenzy(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, sn) || affect_exists_on_char(victim, gsn_berserk)) {
		if (victim == ch)
			stc("You are already in a frenzy.\n", ch);
		else
			act("$N is already in a frenzy.", ch, NULL, victim, TO_CHAR);

		return;
	}

	if (affect_exists_on_char(victim, gsn_calm)) {
		if (victim == ch)
			stc("Why don't you just relax for a while?\n", ch);
		else
			act("$N doesn't look like $e wants to fight anymore.", ch, NULL, victim, TO_CHAR);

		return;
	}

	if ((IS_GOOD(ch) && !IS_GOOD(victim))
	    || (IS_NEUTRAL(ch) && !IS_NEUTRAL(victim))
	    || (IS_EVIL(ch) && !IS_EVIL(victim))) {
		act("Your god doesn't seem to like $N.", ch, NULL, victim, TO_CHAR);
		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level / 3,
		evolution,
		FALSE
	);

	stc("You are filled with holy wrath!\n", victim);
	act("$n gets a wild look in $s eyes!", victim, NULL, NULL, TO_ROOM);
}

void spell_gate(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim;
	bool gate_pet;

	if (ch->in_room == NULL)
		return;

	if (ch->in_room->area == quest_area) {
		stc("You cannot gate while in the quest area.\n", ch);
		return;
	}

	if (ch->in_room->sector_type == SECT_ARENA || char_in_duel_room(ch)) {
		stc("You cannot gate while in the arena.\n", ch);
		return;
	}

	if (IS_SET(GET_ROOM_FLAGS(ch->in_room), ROOM_NO_RECALL)
	    || (victim = get_char_world(ch, target_name, VIS_CHAR)) == NULL
	    || victim == ch
	    || (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch))
	    || victim->in_room == NULL
	    || !can_see_room(ch, victim->in_room)
	    || IS_SET(GET_ROOM_FLAGS(victim->in_room), ROOM_SAFE | ROOM_PRIVATE | ROOM_SOLITARY | ROOM_NO_RECALL)
	    || victim->in_room->sector_type == SECT_ARENA
	    || victim->in_room->area == quest_area
	    || char_in_duel_room(victim)
	    || victim->in_room->clan
	    || victim->in_room->guild
	    || victim->level > level + (IS_NPC(victim) ? 3 : 8)
	    || (IS_NPC(victim)
	        && (IS_SET(victim->act, ACT_NOSUMMON)
	            || saves_spell(level, victim, DAM_OTHER)))) {
		stc("You failed.\n", ch);
		return;
	}

	if (ch->pet != NULL && ch->in_room == ch->pet->in_room && !IS_SET(ch->pet->act, ACT_STAY))
		gate_pet = TRUE;
	else
		gate_pet = FALSE;

	act("$n steps through a gate and vanishes.", ch, NULL, NULL, TO_ROOM);
	stc("You step through a gate and vanish.\n", ch);
	char_from_room(ch);
	char_to_room(ch, victim->in_room);
	act("$n has arrived through a gate.", ch, NULL, NULL, TO_ROOM);
	do_look(ch, "auto");

	if (gate_pet) {
		act("$n steps through a gate and vanishes.", ch->pet, NULL, NULL, TO_ROOM);
		stc("You step through a gate and vanish.\n", ch->pet);
		char_from_room(ch->pet);
		char_to_room(ch->pet, victim->in_room);
		act("$n has arrived through a gate.", ch->pet, NULL, NULL, TO_ROOM);
		do_look(ch->pet, "auto");
	}
}

void spell_giant_strength(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, sn)) {
		if (victim == ch)
			stc("You are already as strong as you can get!\n", ch);
		else
			act("$N can't get any stronger.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level,
		evolution,
		FALSE
	);

	stc("Your muscles surge with heightened power!\n", victim);
	act("$n's muscles surge with heightened power.", victim, NULL, NULL, TO_ROOM);
}

void spell_harm(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	int calign, valign, aligndiff;
	valign = victim->alignment;
	calign = ch->alignment;

	/*Produce a postive value representing the difference in alignments*/
	if (valign > calign)
		aligndiff = valign - calign;
	else
		aligndiff = calign - valign;

	aligndiff /= 200;
	dam = dice(level / 2, aligndiff);
	damage(ch, victim, dam, sn, DAM_HARM , TRUE, TRUE);
}

void spell_haste(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, sn) || affect_exists_on_char(victim, gsn_haste) || IS_SET(victim->off_flags, OFF_FAST)) {
		if (victim == ch)
			stc("You can't move any faster!\n", ch);
		else
			act("$N is already moving as fast as $E can.", ch, NULL, victim, TO_CHAR);

		return;
	}

	if (affect_exists_on_char(victim, gsn_slow)) {
		if (!check_dispel_char(level, victim, gsn_slow, FALSE)) {
			if (victim != ch)
				stc("Spell failed.\n", ch);

			stc("You feel momentarily faster.\n", victim);
			return;
		}

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		victim == ch ? level/2 : level/4,
		evolution,
		FALSE
	);

	stc("You feel yourself moving more quickly.\n", victim);
	act("$n is moving more quickly.", victim, NULL, NULL, TO_ROOM);

	if (ch != victim)
		stc("Ok.\n", ch);
}

void spell_heal(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	victim->hit = UMIN(victim->hit + 100, ATTR_BASE(victim, APPLY_HIT));
	update_pos(victim);
	stc("A warm feeling fills your body.\n", victim);

	if (ch != victim)
		stc("Ok.\n", ch);
}

void spell_heat_metal(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj_lose, *obj_next;
	int dam = 0;

	if (saves_spell(level + 2, victim, DAM_FIRE)) {
		stc("Your spell had no effect.\n", ch);
		stc("You feel momentarily warmer.\n", victim);
		return;
	}

	for (obj_lose = victim->carrying; obj_lose != NULL; obj_lose = obj_next) {
		obj_next = obj_lose->next_content;

		if (number_range(1, 2 * level) < obj_lose->level
		    || saves_spell(level, victim, DAM_FIRE)
		    || IS_OBJ_STAT(obj_lose, ITEM_NONMETAL)
		    || IS_OBJ_STAT(obj_lose, ITEM_BURN_PROOF))
			continue;

		switch (obj_lose->item_type) {
		case ITEM_ARMOR:
			if (obj_lose->wear_loc != -1) { /* remove the item */
				if (can_drop_obj(victim, obj_lose)
				    && (obj_lose->weight / 10) < number_range(1, 2 * GET_ATTR_DEX(victim))
				    && remove_obj(victim, obj_lose->wear_loc, TRUE)) {
					act("$n yelps and throws $p to the ground!", victim, obj_lose, NULL, TO_ROOM);
					act("You remove and drop $p before it burns you.", victim, obj_lose, NULL, TO_CHAR);
					dam += (number_range(1, obj_lose->level) / 3);
					obj_from_char(obj_lose);

					if (victim->in_room->sector_type == SECT_ARENA
					    || char_in_darena_room(victim))
						obj_to_char(obj_lose, victim);
					else
						obj_to_room(obj_lose, victim->in_room);
				}
				else { /* stuck on the body! ouch! */
					act("Your skin is seared by $p!", victim, obj_lose, NULL, TO_CHAR);
					dam += (number_range(1, obj_lose->level));
				}
			}
			else { /* drop it if we can */
				if (can_drop_obj(victim, obj_lose)) {
					act("$n yelps and throws $p to the ground!", victim, obj_lose, NULL, TO_ROOM);
					act("You yelp and drop $p before it burns you.", victim, obj_lose, NULL, TO_CHAR);
					dam += (number_range(1, obj_lose->level) / 6);
					obj_from_char(obj_lose);

					if (victim->in_room->sector_type == SECT_ARENA
					    || char_in_darena_room(victim))
						obj_to_char(obj_lose, victim);
					else
						obj_to_room(obj_lose, victim->in_room);
				}
				else { /* cannot drop */
					act("Your skin is seared by $p!", victim, obj_lose, NULL, TO_CHAR);
					dam += (number_range(1, obj_lose->level) / 2);
				}
			}

			break;

		case ITEM_WEAPON:
			if (obj_lose->wear_loc != -1) { /* try to drop it */
				if (IS_WEAPON_STAT(obj_lose, WEAPON_FLAMING))
					continue;

				if (can_drop_obj(victim, obj_lose)
				    && remove_obj(victim, obj_lose->wear_loc, TRUE)) {
					act("$n is burned by $p, and throws it to the ground.", victim, obj_lose, NULL, TO_ROOM);
					stc("You throw your red-hot weapon to the ground!\n", victim);
					dam += 1;
					obj_from_char(obj_lose);

					if (victim->in_room->sector_type == SECT_ARENA
					    || char_in_darena_room(victim))
						obj_to_char(obj_lose, victim);
					else
						obj_to_room(obj_lose, victim->in_room);
				}
				else { /* YOWCH! */
					stc("Your weapon sears your flesh!\n", victim);
					dam += number_range(1, obj_lose->level);
				}
			}
			else { /* drop it if we can */
				if (can_drop_obj(victim, obj_lose)) {
					act("$n throws a burning hot $p to the ground!", victim, obj_lose, NULL, TO_ROOM);
					act("You and drop $p before it burns you.", victim, obj_lose, NULL, TO_CHAR);
					dam += (number_range(1, obj_lose->level) / 6);
					obj_from_char(obj_lose);

					if (victim->in_room->sector_type == SECT_ARENA
					    || char_in_darena_room(victim))
						obj_to_char(obj_lose, victim);
					else
						obj_to_room(obj_lose, victim->in_room);
				}
				else { /* cannot drop */
					act("Your skin is seared by $p!", victim, obj_lose, NULL, TO_CHAR);
					dam += (number_range(1, obj_lose->level) / 2);
				}
			}

			break;
		}
	}

	if (dam <= 0) {
		stc("Your spell had no effect.\n", ch);
		stc("You feel momentarily warmer.\n", victim);
	}
	else {
		if (saves_spell(level, victim, DAM_FIRE))
			dam = 2 * dam / 3;

		damage(ch, victim, dam, sn, DAM_FIRE, TRUE, TRUE);
	}
}

void spell_holy_word(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if (ch->stam > 0)
		ch->stam = 0;
	else
		return;

	act("$n utters a word of divine power!", ch, NULL, NULL, TO_ROOM);
	stc("You utter a word of divine power.\n", ch);

	for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
		vch_next = vch->next_in_room;

		if ((IS_GOOD(ch) && IS_GOOD(vch))
		    || (IS_EVIL(ch) && IS_EVIL(vch))
		    || (IS_NEUTRAL(ch) && IS_NEUTRAL(vch))) {
			stc("You feel more powerful.\n", vch);
			spell_frenzy(gsn_frenzy, level, ch, (void *) vch, TARGET_CHAR, get_evolution(ch, sn));
			spell_bless(gsn_bless, level, ch, (void *) vch, TARGET_CHAR, get_evolution(ch, sn));
		}
		else if ((IS_GOOD(ch) && IS_EVIL(vch))
		         || (IS_EVIL(ch) && IS_GOOD(vch))) {
			if (!is_safe_spell(ch, vch, TRUE)) {
				spell_curse(gsn_curse, level, ch, (void *) vch, TARGET_CHAR, get_evolution(ch, sn));
				stc("You are struck down!\n", vch);
				damage(ch, vch, dice(level, 20), sn, DAM_ENERGY, TRUE, TRUE);
			}
		}
		else if ((IS_NEUTRAL(ch) && !IS_NEUTRAL(vch))
		         || (IS_EVIL(ch) && IS_NEUTRAL(vch))
		         || (IS_GOOD(ch) && IS_NEUTRAL(vch))) {
			if (!is_safe_spell(ch, vch, TRUE)) {
				spell_curse(gsn_curse, level / 2, ch, (void *) vch, TARGET_CHAR, get_evolution(ch, sn));
				stc("You are struck down!\n", vch);
				damage(ch, vch, dice(level, 6), sn, DAM_ENERGY, TRUE, TRUE);
			}
		}
	}

	stc("You feel drained.\n", ch);
	ch->hit -= (ch->hit / 3);
}

void spell_imprint(int sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	char buf[MAX_STRING_LENGTH];
	int sp_slot, i, mana;

	/* Don't cast this spell on characters. It cuases memory errors. -- Outsider */
	if (ch == vo)
		return;

	if (skill_table[sn].spell_fun == spell_null) {
		stc("That is not a spell.\n", ch);
		return;
	}

	/* counting the number of spells contained within */
	for (sp_slot = i = 1; i < 4; i++)
		if (obj->value[i] != -1)
			sp_slot++;

	if (sp_slot > 3) {
		act("$p cannot contain any more spells.", ch, obj, NULL, TO_CHAR);
		return;
	}

	/* scribe/brew costs 2 times the normal mana required to cast the spell */
	mana = (2 * get_skill_cost(ch, sn));

	if (!IS_NPC(ch) && ch->mana < mana) {
		stc("You don't have enough mana.\n", ch);
		return;
	}

	if (number_percent() > ch->pcdata->learned[sn]) {
		stc("You lost your concentration.\n", ch);
		check_improve(ch, sn, FALSE, 1);
		ch->mana -= mana / 2;
		return;
	}

	/* executing the imprinting process */
	ch->mana -= mana;
	obj->value[sp_slot] = sn;

	/* Making it successively harder to pack more spells into potions or scrolls - JH */
	switch (sp_slot) {
	default:
		bug("sp_slot has more than %d spells.", sp_slot);
		return;

	case 1:
		break;

	case 2:
		if (number_percent() > 25) {
			ptc(ch, "The magic enchantment has failed --- the %s vanishes.\n", item_type_name(obj));
			extract_obj(obj);
			return;
		}

		break;

	case 3:
		if (number_percent() > 5) {
			ptc(ch, "The magic enchantment has failed --- the %s vanishes.\n", item_type_name(obj));
			extract_obj(obj);
			return;
		}

		break;
	}

	/* labeling the item */
	free_string(obj->short_descr);
	sprintf(buf, "a %s of ", item_type_name(obj));

	for (i = 1; i <= sp_slot ; i++)
		if (obj->value[i] != -1) {
			strcat(buf, skill_table[obj->value[i]].name);
			(i != sp_slot) ? strcat(buf, ", ") : strcat(buf, "") ;
		}

	obj->short_descr = str_dup(buf);
	free_string(obj->description);
	sprintf(buf, "A %s of ", item_type_name(obj));

	for (i = 1; i <= sp_slot ; i++)
		if (obj->value[i] != -1) {
			strcat(buf, skill_table[obj->value[i]].name);
			(i != sp_slot) ? strcat(buf, ", ") : strcat(buf, " lies here.\n");
		}

	obj->description = str_dup(buf);
	sprintf(buf, "%s", item_type_name(obj));
	free_string(obj->name);

	for (i = 1; i <= sp_slot ; i++)
		if (obj->value[i] != -1) {
			strcat(buf, " ");
			strcat(buf, skill_table[obj->value[i]].name);
		}

	obj->name = str_dup(buf);
	ptc(ch, "You have imbued a new spell to the %s.\n", item_type_name(obj));
}

void spell_identify(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	char buf[MAX_STRING_LENGTH];
	int i;
	struct wear_type {
		char *loc;
		int bit;
	};
	const struct wear_type wearloc_table[] = {
		{       "be worn on your finger",               B       },
		{       "be worn around your neck",             C       },
		{       "be worn on your torso",                D       },
		{       "be worn on your head",                 E       },
		{       "be worn on your legs",                 F       },
		{       "be worn on your feet",                 G       },
		{       "be worn on your hands",                H       },
		{       "be worn on your arms",                 I       },
		{       "be worn as a shield",                  J       },
		{       "be worn about your body",              K       },
		{       "be worn around your waist",            L       },
		{       "be worn on your wrist",                M       },
		{       "be used as a weapon",                  N       },
		{       "be held",                              O       },
		{       "float next to you",                    Q       },
		{       "be worn on your ringfinger",           R       },
		{       NULL,                                   0       }
	};

	/* Avoid memory error if the target is a character. -- Outsider */
	if (ch == vo)
		return;

	ptc(ch, "Object '%s' is type %s, extra flags %s.\n",
	    obj->name, item_type_name(obj), extra_bit_name(obj->extra_flags|obj->extra_flag_cache));
	ptc(ch, "Weight is %d, value is %d, level is %d, and material is %s.\n",
	    obj->weight / 10, obj->cost, obj->level, obj->material);
	sprintf(buf, ".");

	for (i = 0; wearloc_table[i].loc != NULL; i++)
		if (CAN_WEAR(obj, wearloc_table[i].bit)) {
			sprintf(buf, ", and can %s.", wearloc_table[i].loc);
			break;
		}

	ptc(ch, "The object's condition is %s [%d]%s\n",
	    condition_lookup(obj->condition), obj->condition, buf);

	if (obj->extra_descr != NULL) {
		EXTRA_DESCR_DATA *ed_next;
		EXTRA_DESCR_DATA *ed;

		for (ed = obj->extra_descr; ed != NULL; ed = ed_next) {
			ed_next = ed->next;

			if (!str_cmp(ed->keyword, KEYWD_OWNER)) {
				char strip[MAX_STRING_LENGTH];
				strcpy(strip, ed->description);
				strip[strlen(strip) - 2] = '\0';
				ptc(ch, "This item is owned by %s.\n", strip);
				break;
			}
		}
	}

	switch (obj->item_type) {
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		ptc(ch, "Level %d spells of:", obj->value[0]);

		if (obj->value[1] >= 0 && obj->value[1] < MAX_SKILL)
			ptc(ch, " '%s'", skill_table[obj->value[1]].name);

		if (obj->value[2] >= 0 && obj->value[2] < MAX_SKILL)
			ptc(ch, " '%s'", skill_table[obj->value[2]].name);

		if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
			ptc(ch, " '%s'", skill_table[obj->value[3]].name);

		if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
			ptc(ch, " '%s'", skill_table[obj->value[4]].name);

		stc(".\n", ch);
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		ptc(ch, "Has %d charges of level %d", obj->value[2], obj->value[0]);

		if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
			ptc(ch, " '%s'", skill_table[obj->value[3]].name);

		stc(".\n", ch);
		break;

	case ITEM_DRINK_CON:
		ptc(ch, "It holds %s-colored %s.\n",
		    liq_table[obj->value[2]].liq_color, liq_table[obj->value[2]].liq_name);
		break;

	case ITEM_CONTAINER:
		ptc(ch, "Capacity: %d#  Maximum weight: %d#  flags: %s\n",
		    obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));

		if (obj->value[4] != 100)
			ptc(ch, "Weight multiplier: %d%%\n", obj->value[4]);

		break;

	case ITEM_WEAPON:
		stc("Weapon type is ", ch);

		switch (obj->value[0]) {
		case (WEAPON_EXOTIC) : stc("exotic.\n", ch);       break;

		case (WEAPON_SWORD)  : stc("sword.\n", ch);        break;

		case (WEAPON_DAGGER) : stc("dagger.\n", ch);       break;

		case (WEAPON_SPEAR)  : stc("spear/staff.\n", ch);  break;

		case (WEAPON_MACE)   : stc("mace/club.\n", ch);    break;

		case (WEAPON_AXE)    : stc("axe.\n", ch);          break;

		case (WEAPON_FLAIL)  : stc("flail.\n", ch);        break;

		case (WEAPON_WHIP)   : stc("whip.\n", ch);         break;

		case (WEAPON_POLEARM): stc("polearm.\n", ch);      break;

		case (WEAPON_BOW)    : stc("bow.\n", ch);          break;

		default             : stc("unknown.\n", ch);      break;
		}

		ptc(ch, "Damage is %dd%d (average %d).\n",
		    obj->value[1], obj->value[2], (1 + obj->value[2]) * obj->value[1] / 2);

		if (obj->value[4] || obj->weapon_flag_cache)  /* weapon flags */
			ptc(ch, "Weapons flags: %s\n", weapon_bit_name(obj->value[4]|obj->weapon_flag_cache));

		break;

	case ITEM_ARMOR:
		ptc(ch, "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n",
		    obj->value[0], obj->value[1], obj->value[2], obj->value[3]);
		break;

	case ITEM_PBTUBE:
		ptc(ch, "It contains %d paintballs.\n", obj->value[0]);
		break;

	case ITEM_PBGUN:
		ptc(ch, "It is holding %d/%d paintballs.  It's rating is %d/%d/%d\n",
		    obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4]);
		break;

	case ITEM_MATERIAL:
		ptc(ch, "Skill modifier: %d, Dice Bonus %d, Sides Bonus %d\n",
		    obj->value[0], obj->value[1], obj->value[2]);
		break;
	}

	for (const AFFECT_DATA *paf = affect_list_obj(obj); paf != NULL; paf = paf->next)
		show_affect_to_char(paf, ch);

    if (obj->gems) {
	    for (OBJ_DATA *gem = obj->gems; gem; gem = gem->next_content)
            ptc(ch, "Has a gem %s of type %d with quality %d.\n",
                            gem->short_descr, gem->value[0], gem->value[1]);

        ptc(ch, "Gems are adding:");

        for (const AFFECT_DATA *paf = obj->gem_affected; paf != NULL; paf = paf->next)
                show_affect_to_char(paf, ch);
    }
}

void spell_infravision(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, sn)) {
		if (victim == ch)
			stc("You can already see in the dark.\n", ch);
		else
			act("$N already has enhanced vision.\n", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level * 2,
		evolution,
		FALSE
	);

	stc("Your eyes glow red.\n", victim);
	act("$n's eyes glow red.\n", ch, NULL, NULL, TO_ROOM);
}

void spell_invis(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af = (AFFECT_DATA){0};

	/* object invisibility */
	if (target == TARGET_OBJ) {
		obj = (OBJ_DATA *) vo;

		if (IS_OBJ_STAT(obj, ITEM_INVIS)) {
			act("$p is already invisible.", ch, obj, NULL, TO_CHAR);
			return;
		}

		af.where        = TO_OBJECT;
		af.type         = sn;
		af.level        = level;
		af.duration     = level + 12;
		af.location     = APPLY_NONE;
		af.modifier     = 0;
		af.bitvector    = ITEM_INVIS;
		af.evolution = evolution;
		affect_copy_to_obj(obj, &af);
		act("$p fades out of sight.", ch, obj, NULL, TO_ALL);
		return;
	}

	/* character invisibility */
	victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, gsn_invis)) {
		if (victim == ch)
			stc("You are already invisible!\n", ch);
		else
			act("$N is already invisible!\n", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level + 12,
		evolution,
		FALSE
	);

	stc("You fade out of existence.\n", victim);
	act("$n fades out of existence.", victim, NULL, NULL, TO_ROOM);
}

void spell_know_alignment(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	char *msg;
	int ap = victim->alignment;

	if (ap >  700)     msg = "$N has a pure and good aura.";
	else if (ap >  350)     msg = "$N is of excellent moral character.";
	else if (ap >  100)     msg = "$N is often kind and thoughtful.";
	else if (ap > -100)     msg = "$N doesn't have a firm moral commitment.";
	else if (ap > -350)     msg = "$N lies to $S friends.";
	else if (ap > -700)     msg = "$N is a black-hearted murderer.";
	else                    msg = "$N is the embodiment of pure evil!";

	act(msg, ch, NULL, victim, TO_CHAR);
}

void spell_lightning_bolt(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	static const sh_int dam_each[] = {
		0,
		0,  0,  0,  0,  0,      0,  0,  0, 25, 28,
		31, 34, 37, 40, 40,     41, 42, 42, 43, 44,
		44, 45, 46, 46, 47,     48, 48, 49, 50, 50,
		51, 52, 52, 53, 54,     54, 55, 56, 56, 57,
		58, 58, 59, 60, 60,     61, 62, 62, 63, 64
	};
	level       = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
	level       = UMAX(0, level);
	dam         = number_range(dam_each[level] / 2, dam_each[level] * 2);

	if (saves_spell(level, victim, DAM_ELECTRICITY))
		dam /= 2;

	damage(ch, victim, dam, sn, DAM_ELECTRICITY , TRUE, TRUE);
}

/* Locate Life by Lotus */
void spell_locate_life(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	char buf[MAX_STRING_LENGTH];
	BUFFER *buffer;
	CHAR_DATA *victim;
	bool found = FALSE;
	int number = 0, max_found = 2 * level;
	buffer = new_buf();

	for (victim = char_list; victim != NULL; victim = victim->next) {
		if (!can_see(ch , victim)        /* NOT can_see_who */
		    || !is_name(target_name, victim->name)
		    || victim->in_room == NULL
		    || number_percent() > 2 * level
		    || ch->level < victim->level
		    || !can_see_room(ch, victim->in_room)
		    || IS_SET(GET_ROOM_FLAGS(victim->in_room),
		              ROOM_PRIVATE | ROOM_IMP_ONLY | ROOM_GODS_ONLY | ROOM_NOWHERE))
			continue;

		found = TRUE;
		number++;
		sprintf(buf, "[%d] %s is located at %s\n",
		        number, IS_NPC(victim) ? victim->short_descr : victim->name, victim->in_room->name);
		buf[0] = UPPER(buf[0]);
		add_buf(buffer, buf);

		if (number >= max_found)
			break;
	}

	if (!found)
		stc("No one around appears to go by that name.\n", ch);
	else
		page_to_char(buf_string(buffer), ch);

	free_buf(buffer);
}

void spell_locate_object(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	char buf[MAX_STRING_LENGTH];
	/*      BUFFER *buffer;*/
	OBJ_DATA *obj, *in_obj;
	bool found = FALSE;
	int number = 0, max_found = 2 * level;

	/*      buffer = new_buf();*/

	for (obj = object_list; obj != NULL; obj = obj->next) {
		if (!can_see_obj(ch, obj)
		    || !is_name(target_name, obj->name)
		    ||   IS_OBJ_STAT(obj, ITEM_NOLOCATE)
		    || number_percent() > 2 * level
		    ||   ch->level < obj->level)
			continue;

		found = TRUE;
		number++;

		for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj)
			;

		if (in_obj->carried_by != NULL && can_see(ch, in_obj->carried_by))
			sprintf(buf, "%s is carried by %s\n", obj->short_descr, PERS(in_obj->carried_by, ch, VIS_CHAR));
		else {
			if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
				sprintf(buf, "%s is in %s [Room %d]\n",
				        obj->short_descr, in_obj->in_room->name, in_obj->in_room->vnum);
			else
				sprintf(buf, "%s is in %s\n",
				        obj->short_descr, in_obj->in_room == NULL ? "somewhere" : in_obj->in_room->name);
		}

		buf[0] = UPPER(buf[0]);
		/*              add_buf(buffer,buf);*/
		stc(buf, ch);

		if (number >= max_found)
			break;
	}

	if (!found)
		stc("Nothing like that in heaven or earth.\n", ch);

	/*      else
	                page_to_char(buf_string(buffer),ch);

	        free_buf(buffer); */
}

/* New magic missile -- Montrey */
void spell_magic_missile(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam, i;
	int clevel = ((level / 10) + 1);
	static const sh_int dam_each[] = {
		0,
		3,  3,  4,  4,  5,      6,  6,  6,  6,  6,
		7,  7,  7,  7,  7,      8,  8,  8,  8,  8,
		9,  9,  9,  9,  9,     10, 10, 10, 10, 10,
		11, 11, 11, 11, 11,     12, 12, 12, 12, 12,
		13, 13, 13, 13, 13,     14, 14, 14, 14, 14
	};

	switch (clevel) {
	case 1:
		stc("You extend your palm and let fly a magic missile!\n", ch);
		act("$n extends $s palm and lets fly a magic missile!", ch, NULL, NULL, TO_ROOM);
		break;

	case 2: case 3: case 4:
		stc("You extend your palm and let fly a burst of magic missiles!\n", ch);
		act("$n extends $s palm and lets fly a burst of magic missiles!", ch, NULL, NULL, TO_ROOM);
		break;

	case 5: case 6: case 7:
		stc("You extend your palm and let fly a flurry of magic missiles!\n", ch);
		act("$n extends $s palm and lets fly a flurry of magic missiles!", ch, NULL, NULL, TO_ROOM);
		break;

	default:
		stc("You extend your palm and unleash a devastating hail of magic missiles!\n", ch);
		act("$n extends $s palm and unleashes a devastating hail of magic missiles!", ch, NULL, NULL, TO_ROOM);
		break;
	}

	for (i = 0; i < clevel; i++) {
		level = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
		level = UMAX(0, level);
		dam   = number_range(dam_each[level] / 2, dam_each[level] * 2);

		if (saves_spell(level, victim, DAM_ENERGY))
			dam /= 2;

		damage(ch, victim, dam, sn, DAM_ENERGY, TRUE, TRUE);

		if (ch->fighting == NULL)
			return;
	}
}

void spell_mass_healing(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *gch;

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
		if ((IS_NPC(ch) && IS_NPC(gch))
		    || (!IS_NPC(ch) && !IS_NPC(gch))) {
			spell_heal(gsn_heal, level, ch, (void *) gch, TARGET_CHAR, get_evolution(ch, sn));
			spell_refresh(gsn_refresh, level, ch, (void *) gch, TARGET_CHAR, get_evolution(ch, sn));
		}
	}
}

void spell_mass_invis(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *gch;

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
		if (!is_same_group(gch, ch) || affect_exists_on_char(gch, gsn_invis))
			continue;

		act("$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM);
		stc("You slowly fade out of existence.\n", gch);

		affect_add_sn_to_char(gch,
			gsn_invis,
			level/2,
			24,
			evolution,
			FALSE
		);
	}

	stc("Ok.\n", ch);
}

void spell_nexus(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim;
	OBJ_DATA *portal, *stone;
	ROOM_INDEX_DATA *to_room, *from_room;
	from_room = ch->in_room;

	if (from_room == NULL)
		return;

	if (IS_IMMORTAL(ch)) {
		if ((victim = get_char_world(ch, target_name, VIS_CHAR)) == NULL) {
			stc("You cannot find them.\n", ch);
			return;
		}

		if (victim == ch
		    || (to_room = victim->in_room) == NULL
		    || to_room == from_room
		    || !can_see_room(ch, to_room)
		    || !can_see_room(ch, from_room)) {
			stc("You failed.\n", ch);
			return;
		}
	}
	else {
		if (from_room->area == quest_area) {
			stc("You cannot nexus while in the quest area.\n", ch);
			return;
		}

		if (from_room->sector_type == SECT_ARENA || char_in_duel_room(ch)) {
			stc("You cannot nexus while in the arena.\n", ch);
			return;
		}

		if (ch->fighting
		    || (victim = get_char_world(ch, target_name, VIS_CHAR)) == NULL
		    || victim == ch
		    || (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch))
		    || (to_room = victim->in_room) == NULL
		    || to_room == from_room
		    || to_room->area == quest_area
		    || !can_see_room(ch, to_room)
		    || !can_see_room(ch, from_room)
		    || IS_SET(GET_ROOM_FLAGS(to_room), ROOM_SAFE | ROOM_PRIVATE | ROOM_SOLITARY)
		    || to_room->sector_type == SECT_ARENA
		    || char_in_duel_room(victim)
		    || to_room->clan  || from_room->clan
		    || to_room->guild || from_room->guild
		    || victim->level >= level + (IS_NPC(victim) ? 3 : 8)
		    || (IS_NPC(victim)
		        && (IS_SET(victim->act, ACT_NOSUMMON)
		            || saves_spell(level, victim, DAM_OTHER)))) {
			stc("You failed.\n", ch);
			return;
		}
	}

	stone = get_eq_char(ch, WEAR_HOLD);

	if (!IS_IMMORTAL(ch) && (stone == NULL || stone->item_type != ITEM_WARP_STONE)) {
		stc("You lack the proper component for this spell.\n", ch);
		return;
	}

	if (stone != NULL && stone->item_type == ITEM_WARP_STONE) {
		act("You draw upon the power of $p.\nIt flares brightly and vanishes!", ch, stone, NULL, TO_CHAR);
		extract_obj(stone);
	}

	/* portal one */
	portal = create_object(get_obj_index(OBJ_VNUM_PORTAL), 0);

	if (! portal) {
		bug("Memory error in spell_nexus -- portal one.", 0);
		stc("You were unable to create a portal.\n", ch);
		return;
	}

	portal->timer = 1 + level / 10;
	portal->value[3] = to_room->vnum;
	obj_to_room(portal, from_room);
	act("$p rises up from the ground.", ch, portal, NULL, TO_ROOM);
	act("$p rises up before you.", ch, portal, NULL, TO_CHAR);
	/* portal two */
	portal = create_object(get_obj_index(OBJ_VNUM_PORTAL), 0);

	if (! portal) {
		bug("Memory error in spell_nexus -- portal two.", 0);
		stc("You were unable to create a portal.\n", ch);
		return;
	}

	portal->timer = 1 + level / 10;
	portal->value[3] = from_room->vnum;
	obj_to_room(portal, to_room);

	if (to_room->people != NULL) {
		act("$p rises up from the ground.", to_room->people, portal, NULL, TO_ROOM);
		act("$p rises up from the ground.", to_room->people, portal, NULL, TO_CHAR);
	}
}

void spell_null(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	stc("That's not a spell!\n", ch);
}

/* Polymorph by Lotus */
void spell_polymorph(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim;
	CHAR_DATA *mobile;

	if (IS_NPC(ch)) {
		stc("Mobiles cannot morph.\n", ch);
		return;
	}

	if (char_in_duel(ch)) {
		stc("You cannot polymorph while duelling.\n", ch);
		return;
	}

	if (target_name[0] == '\0') {
		stc("Morph into what?\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, target_name, VIS_CHAR)) == NULL) {
		stc("You failed.\n", ch);
		return;
	}

	if (ch->level < (victim->level - 15)) {
		stc("That creature is too powerful to imitate.\n", ch);
		return;
	}

	if (ch->desc == NULL)
		return;

	if (ch->desc->original != NULL) {
		stc("You are already morphed.\n", ch);
		return;
	}

	if (ch == victim) {
		stc("What?  Morph into yourself?\n", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		stc("You can only switch into mobiles.\n", ch);
		return;
	}

	/* don't let em switch into mobs with the stronger immunities -- Montrey */
	if (IS_SET(victim->act, ACT_NOMORPH)
	 || check_immune(victim, DAM_BASH) >= 100
	 || check_immune(victim, DAM_PIERCE) >= 100
	 || check_immune(victim, DAM_SLASH) >= 100
	 || check_immune(victim, DAM_FIRE) >= 100
	 || check_immune(victim, DAM_COLD) >= 100
	 || check_immune(victim, DAM_ELECTRICITY) >= 100
	 || check_immune(victim, DAM_ACID) >= 100
	 || check_immune(victim, DAM_NEGATIVE) >= 100
	 || check_immune(victim, DAM_HOLY) >= 100) {
		stc("The gods have restricted the use of this creature for morphing.\n", ch);
		return;
	}

	mobile = create_mobile(get_mob_index(victim->pIndexData->vnum));

	if (! mobile) { /* Check for memory error. -- Outsider */
		bug("Memory error creating mob in spell_polymorph().", 0);
		stc("You were unable to polymorph.\n", ch);
		return;
	}

	act("$n morphs into $N!\n", ch, NULL, mobile, TO_ROOM);
	REMOVE_BIT(mobile->act, ACT_AGGRESSIVE);
	REMOVE_BIT(mobile->act, ACT_IS_HEALER);
	REMOVE_BIT(mobile->act, ACT_IS_CHANGER);
	SET_BIT(mobile->act, PLR_COLOR);
	SET_BIT(mobile->act, ACT_MORPH);

	mobile->hit = ATTR_BASE(mobile, APPLY_HIT) = 100;
	mobile->mana = ATTR_BASE(mobile, APPLY_MANA) = 100;
	mobile->stam = ATTR_BASE(mobile, APPLY_STAM) = 100;
	ATTR_BASE(mobile, APPLY_HITROLL) = 1;
	ATTR_BASE(mobile, APPLY_DAMROLL) = 1;

	mobile->damage[DICE_NUMBER] = 1;
	mobile->damage[DICE_TYPE]  = 1;
	mobile->level = 1;
	mobile->gold = 0;
	mobile->silver = 0;
	char_to_room(mobile, ch->in_room);
	do_switch(ch, mobile->name);
	char_from_room(ch);
	char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO));
}

void spell_pass_door(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, gsn_pass_door)) {
		if (victim == ch)
			stc("You are already out of phase.\n", ch);
		else
			act("$N is already shifted out of phase.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		number_fuzzy(level/4),
		evolution,
		FALSE
	);

	act("$n turns translucent.", victim, NULL, NULL, TO_ROOM);
	stc("You turn translucent.\n", victim);
}

void spread_plague(ROOM_INDEX_DATA *room, const AFFECT_DATA *plague, int chance) {
	if (room == NULL || plague == NULL)
		return;

	if (plague->level <= 1)
		return;

	for (CHAR_DATA *vch = room->people; vch; vch = vch->next_in_room) {
		if (!saves_spell(plague->level - 2, vch, DAM_DISEASE)
		 && !IS_IMMORTAL(vch)
		 && !affect_exists_on_char(vch, gsn_plague)
		 && number_bits(chance) == 0) {
			stc("You feel hot and feverish.\n", vch);
			act("$n shivers and looks very ill.", vch, NULL, NULL, TO_ROOM);

			affect_add_sn_to_char(vch,
				gsn_plague,
				plague->level - 1,
				number_range(1, 2 * (plague->level - 1)),
				plague->evolution,
				FALSE
			);
		}
	}
}

void spell_plague(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (saves_spell(level, victim, DAM_DISEASE)
	    || (IS_NPC(victim) && IS_SET(victim->act, ACT_UNDEAD))) {
		if (ch == victim)
			stc("You feel momentarily ill, but it passes.\n", ch);
		else
			act("$N seems to be unaffected.", ch, NULL, victim, TO_CHAR);

		return;
	}

	stc("You scream in agony as plague sores erupt from your skin.\n", victim);
	act("$n screams in agony as plague sores erupt from $s skin.", victim, NULL, NULL, TO_ROOM);

	affect_add_sn_to_char(victim,
		gsn_plague,
		level * 3 / 4,
		level,
		evolution,
		FALSE
	);
}

void spell_poison(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af = (AFFECT_DATA){0};

	if (target == TARGET_OBJ) {
		obj = (OBJ_DATA *) vo;

		if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON) {
			if (IS_OBJ_STAT(obj, ITEM_BLESS)) {
				act("Your spell fails to corrupt $p.", ch, obj, NULL, TO_CHAR);
				return;
			}

			obj->value[3] = 1;
			act("$p is infused with poisonous vapors.", ch, obj, NULL, TO_ALL);
			return;
		}

		if (obj->item_type == ITEM_WEAPON) {
			if (IS_WEAPON_STAT(obj, WEAPON_POISON)) {
				act("$p is already envenomed.", ch, obj, NULL, TO_CHAR);
				return;
			}

			af.where     = TO_WEAPON;
			af.type      = sn;
			af.level     = level / 2;
			af.duration  = level / 4;
			af.location  = 0;
			af.modifier  = 0;
			af.bitvector = WEAPON_POISON;
			af.evolution = evolution;
			affect_copy_to_obj(obj, &af);
			act("$p is coated with deadly venom.", ch, obj, NULL, TO_ALL);
			return;
		}

		act("You can't poison $p.", ch, obj, NULL, TO_CHAR);
		return;
	}

	victim = (CHAR_DATA *) vo;

	if (saves_spell(level, victim, DAM_POISON)) {
		act("$n turns slightly green, but it passes.", victim, NULL, NULL, TO_ROOM);
		stc("You feel momentarily ill, but it passes.\n", victim);
		return;
	}

	stc("You feel very sick.\n", victim);
	act("$n looks very ill.", victim, NULL, NULL, TO_ROOM);

	affect_add_sn_to_char(victim,
		sn,
		level,
		level,
		evolution,
		FALSE
	);
}

void spell_portal(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim;
	OBJ_DATA *portal, *stone;

	if (ch->in_room == NULL)
		return;

	if (IS_IMMORTAL(ch)) {
		if ((victim = get_char_world(ch, target_name, VIS_CHAR)) == NULL) {
			stc("You cannot find them.\n", ch);
			return;
		}

		if (victim == ch
		    || victim->in_room == NULL
		    || !can_see_room(ch, victim->in_room)) {
			stc("You failed.\n", ch);
			return;
		}
	}
	else {
		if (ch->in_room->area == quest_area) {
			stc("You cannot portal while in the quest area.\n", ch);
			return;
		}

		if (ch->in_room->sector_type == SECT_ARENA || char_in_duel_room(ch)) {
			stc("You cannot portal while in the arena.\n", ch);
			return;
		}

		if (ch->fighting
		    || (victim = get_char_world(ch, target_name, VIS_CHAR)) == NULL
		    || victim == ch
		    || (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch))
		    || victim->in_room == NULL
		    || !can_see_room(ch, victim->in_room)
		    || IS_SET(GET_ROOM_FLAGS(victim->in_room), ROOM_SAFE | ROOM_PRIVATE | ROOM_SOLITARY)
		    || victim->in_room->sector_type == SECT_ARENA
		    || victim->in_room->area == quest_area
		    || char_in_duel_room(victim)
		    || victim->in_room->clan
		    || victim->in_room->guild
		    || victim->level >= level + (IS_NPC(victim) ? 3 : 8)
		    || (IS_NPC(victim)
		        && (IS_SET(victim->act, ACT_NOSUMMON)
		            || saves_spell(level, victim, DAM_OTHER)))) {
			stc("You failed.\n", ch);
			return;
		}
	}

	stone = get_eq_char(ch, WEAR_HOLD);

	if (!IS_IMMORTAL(ch) && (stone == NULL || stone->item_type != ITEM_WARP_STONE)) {
		stc("You lack the proper component for this spell.\n", ch);
		return;
	}

	if (stone != NULL && stone->item_type == ITEM_WARP_STONE) {
		act("You draw upon the power of $p.\nIt flares brightly and vanishes!", ch, stone, NULL, TO_CHAR);
		extract_obj(stone);
	}

	portal = create_object(get_obj_index(OBJ_VNUM_PORTAL), 0);

	if (! portal) {
		bug("Memory error creating a portal.", 0);
		stc("You were unable to create a portal.\n", ch);
		return;
	}

	portal->timer = 2 + level / 25;
	portal->value[3] = victim->in_room->vnum;
	obj_to_room(portal, ch->in_room);
	act("$p rises up from the ground.", ch, portal, NULL, TO_ROOM);
	act("$p rises up before you.", ch, portal, NULL, TO_CHAR);
}

void spell_power_word(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (number_percent() < (GET_ATTR_CHR(ch) * 3))
		level += 5;

	if ((IS_NPC(victim) && IS_SET(victim->act, ACT_UNDEAD))
	    || (!IS_NPC(victim))
	    || (level / 2) < victim->level
	    || saves_spell(level, victim, DAM_CHARM)) {
		stc("You failed.\n", ch);
		return;
	}

	act("$n points $s finger at $N and says, 'DIE!'", ch, NULL, victim, TO_ROOM);
	act("You point your finger at $N and say, 'DIE!'", ch, NULL, victim, TO_CHAR);
	act("$n points $s finger at you and says, 'DIE!!'", ch, NULL, victim, TO_VICT);
	raw_kill(victim); /* temp until i can modify merc.h */
	/*      kill_off(ch, victim); */
}

void spell_protect_container(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int cost, fail, result;

	/* This causes a memory issue if the target is a character. -- Outsider */
	if (ch == vo)
		return;

	if (obj->item_type != ITEM_CONTAINER) {
		stc("You can only protect containers.\n", ch);
		return;
	}

	if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
		act("$p is already protected.", ch, obj, NULL, TO_CHAR);
		return;
	}

	cost = obj->value[0] * 100;

	if ((ch->silver + 100 * ch->gold) < cost) {
		stc("You cannot afford to protect that.\n", ch);
		return;
	}

	/* base 10% - succes based on relative level and container capacity */
	fail = 10 + obj->level - level + obj->value[0] / 100;
	result = number_percent();

	if (result < (fail / 5)) { /* item destroyed */
		act("$p begins to vibrate... then explodes!", ch, obj, NULL, TO_CHAR);
		act("$p begins to vibrate... then explodes!", ch, obj, NULL, TO_ROOM);

		if (obj->contains) {
			/* dump the contents */
			OBJ_DATA *t_obj, *n_obj;

			for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj) {
				n_obj = t_obj->next_content;
				obj_from_obj(t_obj);

				if ((obj->carried_by != NULL) && (number_range(0, 4) != 0))
					obj_to_room(t_obj, obj->carried_by->in_room);
				else {
					extract_obj(t_obj);
					continue;
				}
			}
		}

		extract_obj(obj);
		return;
	}

	if (result <= fail) {  /* failed, no bad result */
		stc("Nothing seemed to happen.\n", ch);
		return;
	}

	/* Success */
	deduct_cost(ch, cost);
	SET_BIT(obj->extra_flags, ITEM_BURN_PROOF);
	act("$p is covered with a thin sheen of adamantine.", ch, obj, NULL, TO_CHAR);
	act("$p is covered with a thin sheen of adamantine.", ch, obj, NULL, TO_ROOM);
}

void spell_protection_evil(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, gsn_protection_evil)) {
		if (victim == ch)
			stc("You are already protected.\n", ch);
		else
			act("$N is already protected.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		24,
		evolution,
		FALSE
	);

	stc("You feel holy and pure.\n", victim);

	if (ch != victim)
		act("$N is protected from evil.", ch, NULL, victim, TO_CHAR);
}

void spell_protection_good(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, gsn_protection_good)) {
		if (victim == ch)
			stc("You are already protected.\n", ch);
		else
			act("$N is already protected.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		24,
		evolution,
		FALSE
	);

	stc("You feel aligned with darkness.\n", victim);

	if (ch != victim)
		act("$N is protected from good.", ch, NULL, victim, TO_CHAR);
}

void spell_resurrect(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj;
	CHAR_DATA *mob;
	char buf[MAX_STRING_LENGTH];
	int i;
	obj = get_obj_here(ch, target_name);

	if (obj == NULL) {
		stc("Resurrect what?\n", ch);
		return;
	}

	if (obj->item_type != ITEM_CORPSE_NPC) {
		if (obj->item_type == ITEM_CORPSE_PC)
			stc("You can't resurrect players.\n", ch);
		else
			stc("It would serve no purpose...\n", ch);

		return;
	}

	if (obj->level > (ch->level + 2)) {
		stc("You couldn't call forth such a great spirit.\n", ch);
		return;
	}

	if (ch->pet != NULL) {
		stc("You already have a pet.\n", ch);
		return;
	}

	mob = create_mobile(get_mob_index(MOB_VNUM_RESZOMBIE));

	if (! mob) {  /* Check for memory errors. -- Outsider */
		bug("Memory error creating mob in spell_resurrect().", 0);
		stc("You were unable to resurrect this creature.\n", ch);
		return;
	}

	mob->level                             = obj->level;
	mob->hit  = ATTR_BASE(mob, APPLY_HIT)  = number_range(mob->level * mob->level / 4, mob->level * mob->level);
	mob->mana = ATTR_BASE(mob, APPLY_MANA) = 100 + dice(mob->level, 10);
	mob->stam = ATTR_BASE(mob, APPLY_STAM) = 100 + dice(mob->level, 10);

	for (i = 0; i < 3; i++)
		mob->armor_base[i]         = interpolate(mob->level, 50, -50);

	mob->armor_base[3]                 = interpolate(mob->level, 40, 0);

	for (int stat = 0; stat < MAX_STATS; stat++)
		ATTR_BASE(mob, stat_to_attr(stat)) = 11 + mob->level / 5;

	ATTR_BASE(mob, APPLY_HITROLL) = mob->level / 10;
	ATTR_BASE(mob, APPLY_DAMROLL) = mob->level / 20;
	mob->damage[DICE_NUMBER]        = mob->level / 4;
	mob->damage[DICE_TYPE]          = 2;
	free_string(mob->long_descr);
	sprintf(buf, "A zombie made from %s is here.\n", obj->short_descr);
	mob->long_descr = str_dup(buf);
	char_to_room(mob, ch->in_room);
	act("$p springs to life as a hideous zombie!", ch, obj, NULL, TO_ROOM);
	act("$p springs to life as a hideous zombie!", ch, obj, NULL, TO_CHAR);
	extract_obj(obj);

	make_pet(ch, mob);

	do_say(mob, "How may I serve you, master?");
}

void spell_ray_of_truth(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam, align;

	if (IS_EVIL(ch)) {
		victim = ch;
		stc("The energy explodes inside you!\n", ch);
	}

	if (victim != ch) {
		act("$n raises $s hand, and a blinding ray of light shoots forth!", ch, NULL, NULL, TO_ROOM);
		stc("You raise your hand and a blinding ray of light shoots forth!\n", ch);
	}

	if (IS_GOOD(victim)) {
		act("$n seems unharmed by the light.", victim, NULL, victim, TO_ROOM);
		stc("The light seems powerless to affect you.\n", victim);
		return;
	}

	dam = dice(level / 3, 20);

	if (saves_spell(level, victim, DAM_HOLY))
		dam /= 2;

	align = victim->alignment;

	if (align < 350) {
		align -= 350;
		align *= -1;
		dam *= align / 400;
	}
	else
		dam = 0;

	damage(ch, victim, dam, sn, DAM_HOLY , TRUE, TRUE);

	if (ch->fighting != NULL)
		spell_blindness(gsn_blindness, 3 * level / 4, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
}

void spell_recharge(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance, percent;

	/* Make sure that the target is not a character. -- Outsider */
	if (ch == vo)
		return;

	if (obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF) {
		stc("That item does not carry charges.\n", ch);
		return;
	}

	if (obj->value[3] >= 3 * level / 2) {
		stc("Your skills are not great enough for that.\n", ch);
		return;
	}

	if (obj->value[1] == 0) {
		stc("That item has already been recharged once.\n", ch);
		return;
	}

	chance = 40 + 2 * level;
	chance -= obj->value[3]; /* harder to do high-level spells */
	chance -= (obj->value[1] - obj->value[2]) * (obj->value[1] - obj->value[2]);
	chance = UMAX(level / 2, chance);
	percent = number_percent();

	if (percent < chance / 2) {
		act("$p glows a bright green.", ch, obj, NULL, TO_CHAR);
		act("$p glows a bright green.", ch, obj, NULL, TO_ROOM);
		obj->value[2] = UMAX(obj->value[1], obj->value[2]);
		obj->value[1] = 0;
		return;
	}
	else if (percent <= chance) {
		int chargeback, chargemax;
		act("$p glows a soft green.", ch, obj, NULL, TO_CHAR);
		act("$p glows a soft green.", ch, obj, NULL, TO_ROOM);
		chargemax = obj->value[1] - obj->value[2];

		if (chargemax > 0)
			chargeback = UMAX(1, chargemax * percent / 100);
		else
			chargeback = 0;

		obj->value[2] += chargeback;
		obj->value[1] = 0;
		return;
	}
	else if (percent <= UMIN(95, 3 * chance / 2)) {
		stc("Nothing seems to happen.\n", ch);

		if (obj->value[1] > 1)
			obj->value[1]--;

		return;
	}
	else { /* whoops! */
		act("$p glows brightly and explodes!", ch, obj, NULL, TO_CHAR);
		act("$p glows brightly and explodes!", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
	}
}

void spell_refresh(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	victim->stam = UMIN(victim->stam + level, ATTR_BASE(victim, APPLY_STAM));

	if (ATTR_BASE(victim, APPLY_STAM) == victim->stam)
		stc("You feel fully refreshed!\n", victim);
	else
		stc("You feel less tired.\n", victim);

	if (ch != victim)
		stc("Ok.\n", ch);
}

/* Divine Regeneration by Lotus */
void spell_divine_regeneration(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!IS_IMMORTAL(ch) && (victim != ch)) {
		stc("This spell cannot be cast on others", ch);
		return;
	}

	if ((affect_exists_on_char(victim, sn))
	    || (affect_exists_on_char(victim, gsn_regeneration))
	    || (affect_exists_on_char(victim, gsn_divine_regeneration))) {
		if (victim == ch)
			stc("You can't possibly feel any more vibrant!\n", ch);
		else
			act("$N already feels as vibrant as possible.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level,
		evolution,
		FALSE
	);

	switch (evolution) {
	case 1:
		stc("You feel amazingly vibrant!\n", victim);
		act("$n looks amazingly vibrant!", victim, NULL, NULL, TO_ROOM);
		break;

	case 2:
		stc("You feel supremely vibrant!\n", victim);
		act("$n looks supremely vibrant!", victim, NULL, NULL, TO_ROOM);
		break;

	case 3:
		stc("You feel divinely vibrant!\n", victim);
		act("$n looks divinely vibrant!", victim, NULL, NULL, TO_ROOM);
		break;

	case 4:
		stc("You are filled with holy vibrance!\n", victim);
		act("$n is filled with holy vibrance!", victim, NULL, NULL, TO_ROOM);
		break;
	}
}

void spell_regeneration(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, sn)
	    || affect_exists_on_char(victim, gsn_regeneration)
	    || affect_exists_on_char(victim, gsn_divine_regeneration)) {
		if (victim == ch)
			stc("You can't possibly feel any more vibrant!\n", ch);
		else
			act("$N already feels as vibrant as possible.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level / 2,
		evolution,
		FALSE
	);

	switch (evolution) {
	case 1:
		stc("You feel more vibrant!\n", victim);
		act("$n looks more vibrant!", victim, NULL, NULL, TO_ROOM);
		break;

	case 2:
		stc("You feel absolutely vibrant!\n", victim);
		act("$n looks absolutely vibrant!", victim, NULL, NULL, TO_ROOM);
		break;

	case 3:
		stc("You feel incredibly vibrant!\n", victim);
		act("$n looks incredibly vibrant!", victim, NULL, NULL, TO_ROOM);
		break;

	case 4:
		stc("You feel unbelievably vibrant!\n", victim);
		act("$n looks unbelievably vibrant!", victim, NULL, NULL, TO_ROOM);
		break;
	}
}

void spell_remove_alignment(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int result, fail;
	sh_int align;

	/* Make sure the target is not a character. -- Outsider */
	if (ch == vo)
		return;

	if (obj->wear_loc != -1 || obj->carried_by == NULL) {
		stc("The item must be carried to remove alignment.\n", ch);
		return;
	}

	if ((!IS_OBJ_STAT(obj, ITEM_ANTI_GOOD))
	    && (!IS_OBJ_STAT(obj, ITEM_ANTI_EVIL))
	    && (!IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL))) {
		stc("The item has no alignment.\n", ch);
		return;
	}

	/* remove some of the character's alignment, if not a Paladin */
	if (ch->class != 5) { /* Paladins */
		align = 25 * ch->alignment / 1000;
		ch->alignment = URANGE(-1000, (ch->alignment - align), 1000);
		stc("The powerful nature of the spell removes some of your alignment!\n", ch);
		act("The powerful nature of $n's spell removes some of $s alignment!", ch, NULL, NULL, TO_ROOM);
	}

	fail = 15;  /* base 15% chance of failure */

	/* find the bonuses (Its harder to remove align from a powerful object) */
	for (const AFFECT_DATA *paf = affect_list_obj(obj); paf != NULL; paf = paf->next)
		fail += 15;

	/* apply other modifiers */
	fail += obj->level;
	fail -= 3 * level / 2;

	/* harder to remove align from objects inherently blessed or evil */
	if (IS_OBJ_STAT(obj, ITEM_BLESS))
		fail += 5;

	if (IS_OBJ_STAT(obj, ITEM_EVIL))
		fail += 5;

	fail = URANGE(5, fail, 90);
	result = number_percent();

	/* the moment of truth */
	if (!IS_IMMORTAL(ch) && result < (fail / 5)) { /* item destroyed */
		act("$p shivers and shudders... then implodes!", ch, obj, NULL, TO_CHAR);
		act("$p shivers and shudders... then implodes!", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return;
	}

	/* Removes more of the caster's alignment, if not a Paladin */
	if (result < (fail / 3)) {
		if (ch->class != 5) {
			align = 25 * ch->alignment / 1000;
			ch->alignment = URANGE(-1000, (ch->alignment - align), 1000);
			stc("The spell backfires and removes some of YOUR alignment!\n", ch);
			act("$n's spell backfires and removes some of $s alignment!", ch, NULL, NULL, TO_ROOM);
		}
		else {
			stc("The spell fails.\n", ch);
			act("$n's spell fails.", ch, NULL, NULL, TO_ROOM);
		}

		return;
	}

	if (!IS_IMMORTAL(ch) && result <= fail) {  /* failed, no bad result */
		stc("Nothing seemed to happen.\n", ch);
		return;
	}

	/* Success!  Remove the item's alignment */
	REMOVE_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
	REMOVE_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
	REMOVE_BIT(obj->extra_flags, ITEM_ANTI_NEUTRAL);
	act("You remove $p's alignment!", ch, obj, NULL, TO_CHAR);
	act("$n removes $p's alignment!", ch, obj, NULL, TO_ROOM);
}

void spell_remove_invis(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	/* Avoid memory error from the target being a character, and not
	   and object. -- Outsider */
	if (ch == vo)
		return;

	if (!IS_OBJ_STAT(obj, ITEM_INVIS)) {
		act("$p is already visible.", ch, obj, NULL, TO_CHAR);
		return;
	}

	if (!IS_IMMORTAL(ch) && level_save(level, obj->level)) {
		act("$p flickers... but remains invisible.", ch, obj, NULL, TO_ALL);
		return;
	}

	REMOVE_BIT(obj->extra_flags, ITEM_INVIS);
	act("$p appears out of thin air!", ch, obj, NULL, TO_ALL);
}

void spell_remove_curse(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	bool affected = FALSE;
	bool object = FALSE;

	/* do object cases first */
	if (target == TARGET_OBJ) {
		obj = (OBJ_DATA *) vo;

		if (!IS_OBJ_STAT(obj, ITEM_NODROP) && !IS_OBJ_STAT(obj, ITEM_NOREMOVE)) {
			act("There doesn't seem to be a curse on $p.", ch, obj, NULL, TO_CHAR);
			return;
		}

		if (IS_OBJ_STAT(obj, ITEM_NOUNCURSE)
		    || (!IS_IMMORTAL(ch) && level_save(level + 2, obj->level))) {
			act("The curse on $p is beyond your power.", ch, obj, NULL, TO_CHAR);
			return;
		}

		REMOVE_BIT(obj->extra_flags, ITEM_NODROP);
		REMOVE_BIT(obj->extra_flags, ITEM_NOREMOVE);
		act("$p glows blue.", ch, obj, NULL, TO_ALL);
		return;
	}

	/* characters */
	victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, gsn_curse)) {
		affected = TRUE;

		if (check_dispel_char(level, victim, gsn_curse, FALSE)) {
			return;
		}
	}

	for (obj = victim->carrying; obj != NULL; obj = obj->next_content) {
		if (IS_OBJ_STAT(obj, ITEM_NODROP) || IS_OBJ_STAT(obj, ITEM_NOREMOVE)) {
			affected = TRUE;
			object = TRUE;

			if (IS_OBJ_STAT(obj, ITEM_NOUNCURSE)) {
				act("You fail to remove the curse on $p.", victim, obj, NULL, TO_CHAR);
				continue;
			}

			/* attempt to remove curse */
			if (IS_IMMORTAL(ch) || !level_save(level, obj->level)) {
				REMOVE_BIT(obj->extra_flags, ITEM_NODROP);
				REMOVE_BIT(obj->extra_flags, ITEM_NOREMOVE);
				act("Your $p glows blue.", victim, obj, NULL, TO_CHAR);
				act("$n's $p glows blue.", victim, obj, NULL, TO_ROOM);
				return;
			}
			else
				act("You fail to remove the curse on $p.", victim, obj, NULL, TO_CHAR);
		}
	}

	if (affected) {
		if (!object)
			stc("You fail to remove the curse.\n", ch);
	}
	else if (ch == victim)
		stc("Neither you nor anything you are wearing is cursed!\n", ch);
	else
		act("Neither $E nor anything $E is wearing is cursed!\n", ch, NULL, victim, TO_CHAR);
}

void spell_sanctuary(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, gsn_sanctuary)) {
		if (victim == ch)
			stc("You are already in sanctuary.\n", ch);
		else
			act("$N is already in sanctuary.", ch, NULL, victim, TO_CHAR);

		return;
	}

	switch (evolution) {
	default: /* 1 */
		act("$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM);
		stc("You are surrounded by a white aura.\n", victim);
		break;

	case 2:
		act("$n is surrounded by a bright white aura.", victim, NULL, NULL, TO_ROOM);
		stc("You are surrounded by a bright white aura.\n", victim);
		break;

	case 3:
		act("$n is surrounded by a brilliant white aura.", victim, NULL, NULL, TO_ROOM);
		stc("You are surrounded by a brilliant white aura.\n", victim);
		break;

	case 4:
		act("A dazzling white aura of holiness permeates the air around $n.",
		    victim, NULL, NULL, TO_ROOM);
		stc("A dazzling white aura of holiness permeates the air around you.\n", victim);
		break;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level / 6,
		evolution,
		FALSE
	);
}

void spell_shield(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, sn)) {
		if (victim == ch)
			stc("You are already shielded from harm.\n", ch);
		else
			act("$N is already protected by a shield.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level + 8,
		evolution,
		FALSE
	);

	act("$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM);
	stc("You are surrounded by a force shield.\n", victim);
	return;
}

void spell_sunray(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	stc("You call forth a divine ray of light!\n", ch);
	act("$n brings forth a beam of holy light from the heavens.", ch, NULL, NULL, TO_ROOM);
	dam = dice(level, 25);

	if (victim->alignment > 350)
		dam /= 2;

	damage(ch, victim, dam, sn, DAM_LIGHT, TRUE, TRUE);
}

void spell_flameshield(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, sn)) {
		if (victim == ch)
			stc("You are already circled by flames.\n", ch);
		else
			act("$N is already circled by flames.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level / 2,
		evolution,
		FALSE
	);

	act("$n is surrounded by a circle of flames.", victim, NULL, NULL,
	    TO_ROOM);
	stc("You are surrounded by a circle of flames.\n", victim);
	return;
}

void spell_shocking_grasp(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const int dam_each[] = {
		0,
		0,  0,  0,  0,  0,      0, 20, 25, 29, 33,
		36, 39, 39, 39, 40,     40, 41, 41, 42, 42,
		43, 43, 44, 44, 45,     45, 46, 46, 47, 47,
		48, 48, 49, 49, 50,     50, 51, 51, 52, 52,
		53, 53, 54, 54, 55,     55, 56, 56, 57, 57
	};
	int dam;
	level       = UMIN(level, sizeof(dam_each) / sizeof(dam_each[0]) - 1);
	level       = UMAX(0, level);
	dam         = number_range(dam_each[level] / 2, dam_each[level] * 2);

	if (saves_spell(level, victim, DAM_ELECTRICITY))
		dam /= 2;

	damage(ch, victim, dam, sn, DAM_ELECTRICITY , TRUE, TRUE);
	return;
}

void spell_sleep(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (number_percent() < (GET_ATTR_CHR(ch) * 3))
		level += 5;

	if (affect_exists_on_char(victim, gsn_sleep)) {
		act("$E isn't awake enough to be affected by your spell.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_NPC(victim) && IS_SET(victim->act, ACT_UNDEAD)) {
		act("$E isn't sufficiently alive to be affected by your spell.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if ((level + 2) < victim->level
	    || saves_spell(level - 4, victim, DAM_CHARM)) {
		act("$N's eyes close, but only for a moment. You have failed.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level + 4,
		evolution,
		FALSE
	);

	if (IS_AWAKE(victim)) {
		stc("You feel very sleepy ..... zzzzzz.\n", victim);
		act("$n goes to sleep.", victim, NULL, NULL, TO_ROOM);
		victim->position = POS_SLEEPING;
	}
} /* end spell_sleep() */

void spell_slow(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, sn) || affect_exists_on_char(victim, gsn_slow)) {
		if (victim == ch)
			stc("You can't move any slower!\n", ch);
		else
			act("$N can't get any slower than that.",
			    ch, NULL, victim, TO_CHAR);

		return;
	}

	if (affect_exists_on_char(victim, gsn_haste)) {
		if (!check_dispel_char(level, victim, gsn_haste, (ch != victim))) {
			if (victim != ch)
				stc("Spell failed.\n", ch);

			stc("You feel momentarily slower.\n", victim);
			return;
		}

		return;
	}

	if (victim != ch && saves_spell(level, victim, DAM_OTHER)) {
		stc("Nothing seemed to happen.\n", ch);
		stc("You feel momentarily lethargic.\n", victim);
		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level / 2,
		evolution,
		FALSE
	);

	stc("You feel yourself slowing d o w n...\n", victim);
	act("$n starts to move in slow motion.", victim, NULL, NULL, TO_ROOM);
	return;
}

/* Smokescreen by Corwyn */
void spell_smokescreen(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *vch;
	ROOM_INDEX_DATA *in_room;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	int door;
	char arg[MAX_STRING_LENGTH];
	one_argument(target_name, arg);

	if (arg[0] == '\0') {
		stc("Without direction, the smoke has no purpose...\n", ch);
		return;
	}

	door = find_exit(ch, arg);
	in_room = ch->in_room;

	if ((door < 0) || (door > 5)
	    || (pexit   = in_room->exit[door]) == NULL
	    || (to_room = pexit->u1.to_room) == NULL
	    ||   !can_see_room(ch, pexit->u1.to_room)
	    || (IS_SET(pexit->exit_info, EX_CLOSED))) {
		stc("The smoke has nowhere to go and dissipates.\n", ch);
		return;
	}

	if (number_percent() < 5) {
		affect_add_sn_to_char(ch,
			sn,
			level + 5,
			number_range(0, level / 2),
			evolution,
			FALSE
		);

		stc("You lose control of the smoke and it turns on you!\n", ch);
		act("$n conjures up a cloud of smoke and it turns on $m!", ch, NULL, NULL, TO_ROOM);
		return;
	}

	// TODO: should this be a room affect, and not on the characters?

	for (vch = to_room->people; vch != NULL; vch = vch->next_in_room) {
		bool already_blinded = is_blinded(vch);

		if (is_safe_spell(ch, vch, TRUE))
			continue;

		if (saves_spell(level, vch, DAM_OTHER)) {
			if (!already_blinded)
				stc("Smoke momentarily clouds your vision.\n", vch);
		}
		else {
			affect_add_sn_to_char(vch,
				sn,
				level,
				number_range(0, level / 10),
				evolution,
				FALSE
			);

			act("$n's vision is obscured by a strange cloud of smoke.", vch, NULL, NULL, TO_ROOM);

			if (!already_blinded)
				stc("Smoke clouds your vision.\n", vch);
		}
	}

	stc("You conjure up a smokescreen to confound your enemies.\n", ch);
	act("$n conjures up a smokescreen to confound $s enemies.", ch, NULL, NULL, TO_ROOM);
}

void spell_stone_skin(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(ch, sn)) {
		if (victim == ch)
			stc("Your skin is already as hard as a rock.\n", ch);
		else
			act("$N is already as hard as can be.", ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level,
		evolution,
		FALSE
	);

	act("$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM);
	stc("Your skin turns to stone.\n", victim);
	return;
}

void spell_summon(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim;
	CHAR_DATA *rch;

	if ((victim = get_char_world(ch, target_name, VIS_CHAR)) == NULL
	    ||   victim == ch
	    ||   victim->in_room == NULL
//	    || (victim->on && victim->on->pIndexData->item_type == ITEM_COACH)
	    ||   ch->in_room == NULL) {
		stc("You failed.\n", ch);
		return;
	}

	if (ch->in_room->area == quest_area) {
		stc("You cannot summon while in the quest area.\n", ch);
		return;
	}

	if (ch->in_room->sector_type == SECT_ARENA && !IS_NPC(victim)) {
		stc("You cannot summon players while in the arena.\n", ch);
		return;
	}

	if (char_in_duel_room(ch)) {
		stc("You cannot summon in the duel arena.\n", ch);
		return;
	}

	if (victim->in_room->area == quest_area) {
		stc("Your target is in the quest area. You cannot summon from there.\n",
		    ch);
		return;
	}

	if (victim->in_room->sector_type == SECT_ARENA || char_in_duel_room(victim)) {
		stc("Your target is in the arena. You cannot summon from there.\n",
		    ch);
		return;
	}

	if (IS_SET(GET_ROOM_FLAGS(ch->in_room), ROOM_SAFE)
	    ||   IS_SET(GET_ROOM_FLAGS(victim->in_room), ROOM_SAFE)
	    ||   IS_SET(GET_ROOM_FLAGS(victim->in_room), ROOM_PRIVATE)
	    ||   IS_SET(GET_ROOM_FLAGS(victim->in_room), ROOM_SOLITARY)
	    ||   IS_SET(GET_ROOM_FLAGS(victim->in_room), ROOM_NO_RECALL)
	    || (IS_NPC(victim) && IS_SET(victim->act, ACT_AGGRESSIVE))
	    ||   victim->level >= level + 3
	    || IS_IMMORTAL(victim)
	    ||   victim->fighting != NULL
	    || IS_SET(victim->act, ACT_NOSUMMON)
	    || (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
	    || (IS_SET(GET_ROOM_FLAGS(ch->in_room), ROOM_MALE_ONLY) && GET_ATTR_SEX(victim) != SEX_MALE)
	    || (IS_SET(GET_ROOM_FLAGS(ch->in_room), ROOM_FEMALE_ONLY) && GET_ATTR_SEX(victim) != SEX_FEMALE)
	   ) {
		stc("You failed.\n", ch);
		return;
	}

	/* Can't summon players into a room with an aggie mobile higher level than them */

	if (!IS_NPC(victim)) {
		for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
			if (IS_NPC(rch) && IS_SET(rch->act, ACT_AGGRESSIVE)
			    && rch->level + 6 > victim->level) {
				act("I wouldn't do that! $N would attack them immediately."
				    , ch, NULL, rch, TO_CHAR);
				wiznet("$N is attempting to summon a PC into an aggie room.", ch,
				       NULL, WIZ_CHEAT, 0, GET_RANK(ch));
				return;
			}
		}
	}

	if (IS_NPC(victim) && saves_spell(level, victim, DAM_OTHER)) {
		stc("You failed!\n", ch);
		return;
	}

	act("$n disappears suddenly.", victim, NULL, NULL, TO_ROOM);
	char_from_room(victim);
	char_to_room(victim, ch->in_room);
	act("$n arrives suddenly.", victim, NULL, NULL, TO_ROOM);
	act("$n has summoned you!", ch, NULL, victim,   TO_VICT);
	do_look(victim, "auto");
	return;
}

/* summon object spell by Lotus */
void spell_summon_object(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *obj;
	char arg[MIL];
	int number, count = 0;
	number = number_argument(target_name, arg);

	if (arg[0] == '\0') {
		stc("You must specify an object if you want to summon one.\n", ch);
		return;
	}

	/* imms don't need all the checking below */
	if (IS_IMMORTAL(ch)) {
		for (obj = object_list; obj != NULL; obj = obj->next) {
			if (!can_see_obj(ch, obj)
			    || !is_name(arg, obj->name))
				continue;

			if (++count < number)
				continue;

			if (obj->in_locker || obj->in_strongbox || obj->in_obj)
				continue;
			else if (obj->carried_by) {
				if (!IS_NPC(obj->carried_by))
					continue;

				obj_from_char(obj);
				break;
			}
			else if (obj->in_room) {
				if (obj->in_room->people)
					act("$p disappears suddenly.", obj->in_room->people, obj, NULL, TO_ALL);

				obj_from_room(obj);
				break;
			}
			else {
				/* neither held nor lying, nor in a locker or container. What's left?? */
				bugf("(%s) spell_summon_obj: object %d seems to be nowhere",
				     ch->name, obj->pIndexData->vnum);
				stc("It could not be found.\n", ch);
				return;
			}
		}

		if (!obj) {
			stc("You could not find it.\n", ch);
			return;
		}

		if (!CAN_WEAR(obj, ITEM_TAKE))
			obj_to_room(obj, ch->in_room);
		else
			obj_to_char(obj, ch);

		stc("Success.\n", ch);
		return;
	}

	if (ch->in_room->area == quest_area) {
		stc("You are in the quest area. You cannot summon objects here.\n", ch);
		return;
	}

	if (IS_SET(GET_ROOM_FLAGS(ch->in_room), ROOM_SAFE)) {
		stc("You fail to disturb the peace of this room.\n", ch);
		return;
	}

	for (obj = object_list; obj != NULL; obj = obj->next) {
		if (!can_see_obj(ch, obj)
		    || !is_name(arg, obj->name)
		    || IS_OBJ_STAT(obj, ITEM_NOLOCATE))
			continue;

		/* right kind of object: increase count */
		count++;

		/* n.obj specified and not yet reached: skip other tests. */
		if (count < number)
			continue;

		if (obj->level > ch->level + 2) {
			if (number == 1)
				continue;
			else {
				stc("You failed.\n", ch);
				return;
			}
		}

		if (obj->carried_by != NULL) {
			/* object in someone's hands */
			if ((obj->carried_by->in_room == NULL)
			    || IS_SET(GET_ROOM_FLAGS(obj->carried_by->in_room),
			              ROOM_SAFE | ROOM_PRIVATE | ROOM_SOLITARY | ROOM_NO_RECALL)
			    || obj->carried_by->in_room->sector_type == SECT_ARENA
			    || obj->carried_by->in_room->area == quest_area
			    || (!IS_NPC(obj->carried_by))
			    || (obj->carried_by->level > ch->level + 3)
			    || (IS_NPC(obj->carried_by) && obj->carried_by->pIndexData->pShop != NULL)
			    || (!can_drop_obj(obj->carried_by, obj))
			    || (IS_NPC(obj->carried_by) && saves_spell(level, obj->carried_by, DAM_OTHER))) {
				if (number == 1)
					continue;
				else {
					stc("You failed.\n", ch);
					return;
				}
			}
		}
		else if (obj->in_room != NULL) {
			/* lying around somewhere */
			if (IS_SET(GET_ROOM_FLAGS(obj->in_room),
			           ROOM_SAFE | ROOM_PRIVATE | ROOM_SOLITARY | ROOM_NO_RECALL)
			    || obj->in_room->sector_type == SECT_ARENA
			    || obj->in_room->area == quest_area
			    || (!CAN_WEAR(obj, ITEM_TAKE))) {
				if (number == 1)
					continue;
				else {
					stc("You failed.\n", ch);
					return;
				}
			}
		}
		else if (obj->in_locker || obj->in_strongbox || obj->in_obj) {
			/* in a locker or container */
			continue;
		}
		else {
			/* neither held nor lying, nor in a locker or container. What's left?? */
			bugf("(%s) spell_summon_obj: object %d seems to be nowhere",
			     ch->name, obj->pIndexData->vnum);
			continue;
		}

		/* We found an object!! */
		break;
	}

	if (obj == NULL) {
		stc("You failed.\n", ch);
		return;
	}

	/* no summoning pc corpses to clanhalls!  little bastards -- Montrey */
	if (obj->item_type == ITEM_CORPSE_PC) {
		if (ch->in_room->clan
		    || ch->in_room->guild
		    || !str_cmp(ch->in_room->area->name, "Playpen")
		    || !str_cmp(ch->in_room->area->name, "IMM-Zone")
		    || !str_cmp(ch->in_room->area->name, "Limbo")
		    || !str_cmp(ch->in_room->area->name, "Eilyndrae")      /* hack to make eilyndrae and torayna cri unquestable */
		    || !str_cmp(ch->in_room->area->name, "Torayna Cri")
		    || !str_cmp(ch->in_room->area->name, "Battle Arenas")
		    || ch->in_room->sector_type == SECT_ARENA
		    || IS_SET(GET_ROOM_FLAGS(ch->in_room),
		              ROOM_MALE_ONLY
		              | ROOM_FEMALE_ONLY
		              | ROOM_PRIVATE
		              | ROOM_SOLITARY
		              | ROOM_PET_SHOP)) {
			stc("You cannot summon a player corpse here.\n", ch);
			return;
		}
	}

	if (level_save(ch->level, obj->level)) {
		stc("You failed.\n", ch);
		return;
	}

	if (obj->carried_by != NULL)
		act("$p disappears suddenly.", obj->carried_by, obj, NULL, TO_ALL);
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
		act("$p disappears suddenly.", obj->in_room->people, obj, NULL, TO_ALL);

	if (obj->carried_by != NULL)
		obj_from_char(obj);
	else
		obj_from_room(obj);

	obj_to_room(obj, ch->in_room);

	if (obj->carried_by != NULL)
		act("$p appears suddenly.", obj->carried_by, obj, NULL, TO_ALL);
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
		act("$p appears suddenly.", obj->in_room->people, obj, NULL, TO_ALL);
}

/* Code by Corwyn ... Idea from MadROM */

void spell_talon(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, gsn_talon)) {
		if (victim == ch)
			stc("If you hold your weapon any tighter people will start to wonder...\n"
			    , ch);
		else
			act("$N is already holding his weapon as tightly as is prudent."
			    , ch, NULL, victim, TO_CHAR);

		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level / 8,
		evolution,
		FALSE
	);

	stc("You hold your weapon in a vice-like grip.\n", victim);
	act("$n is NOT letting go of $s weapon.", victim, NULL, NULL, TO_ROOM);
	return;
}

/* Code by Corwyn ... Idea from MadROM */
void spell_teleport_object(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	char object[MAX_STRING_LENGTH];
	char name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];

	if (ch->in_room != NULL && ch->in_room->area == quest_area) {
		stc("You cannot teleport objects in the quest area.\n",
		    ch);
		return;
	}

	/* target_name is a global in magic.c */
	target_name = one_argument(target_name, object);
	one_argument(target_name, name);

	/* find object carried by ch */

	if (object[0] == '\0') {
		stc("What should the spell be cast upon?\n", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, object)) == NULL) {
		stc("You are not carrying that.\n", ch);
		return;
	}

	if (obj->wear_loc != WEAR_NONE) {
		stc("Perhaps you should remove it first...\n", ch);
		return;
	}

	if (IS_OBJ_STAT(obj, ITEM_NODROP)) {
		stc("You can't seem to let go of it.\n", ch);
		return;
	}

	/* find victim in world */

	if (name[0] == '\0')
		victim = ch;
	else {
		if ((victim = get_char_world(ch, name, VIS_CHAR)) == NULL ||
		    victim->in_room == NULL) {
			stc("You failed.\n", ch);
			return;
		}
	}

	if (victim == ch) {
		stc("There would be no point!\n", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		if (IS_SET(victim->pcdata->plr, PLR_LINK_DEAD)) {
			sprintf(buf, "$N is trying to teleport an object to the linkdead character %s.", victim->name);
			wiznet(buf, ch, NULL, WIZ_CHEAT, 0, GET_RANK(ch));
			stc("Your recipient cannot receive teleported objects in their current state.\n", ch);
			return;
		}
	}

	if (ch->in_room->sector_type == SECT_ARENA
	    || victim->in_room->sector_type == SECT_ARENA
	    || char_in_darena_room(victim)) {
		stc("You failed.\n", ch);
		return;
	}

	if (victim->in_room->area == quest_area) {
		stc(
		        "Your recipient cannot receive teleported objects in the quest area.\n",
		        ch);
		return;
	}

	if (IS_NPC(victim)) {
		stc("Mobiles do not need such things.\n", ch);
		return;
	}

	obj_from_char(obj);
	obj_to_char(obj, victim);
	act("$n opens a hole in the fabric of reality and gives $p to $N."
	    , ch, obj, victim, TO_NOTVICT);
	act("$n opens a hole in the fabric of reality and gives $p to you."
	    , ch, obj, victim, TO_VICT);
	act("You open a hole in the fabric of reality and give $p to $N."
	    , ch, obj, victim, TO_CHAR);

	if (ch->in_room != victim->in_room)
		act("$N opens a hole in the fabric of reality and gives $p to $n."
		    , victim, obj, ch, TO_ROOM);
}

void spell_teleport(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	ROOM_INDEX_DATA *room;

	if (ch->in_room != NULL && ch->in_room->area == quest_area) {
		stc("Teleportation does not work in the quest area.\n", ch);
		return;
	}

	if (victim->in_room == NULL
//	    || (victim->on && victim->on->pIndexData->item_type == ITEM_COACH)
	    || IS_SET(GET_ROOM_FLAGS(victim->in_room), ROOM_NO_RECALL)
	    || victim->in_room->sector_type == SECT_ARENA
	    || victim->in_room->sector_type == SECT_CLANARENA
	    || char_in_duel_room(ch)
	    || char_in_duel_room(victim)
	    || ch->in_room->sector_type == SECT_ARENA
	    || (victim != ch && IS_SET(victim->act, ACT_NOSUMMON))
	    || (!IS_NPC(ch) && victim->fighting != NULL)
	    || (victim != ch && saves_spell(level, victim, DAM_OTHER))) {
		stc("You failed.\n", ch);
		return;
	}

	room = get_random_room(victim);

	if (victim != ch)
		stc("You have been teleported!\n", victim);

	act("$n vanishes!", victim, NULL, NULL, TO_ROOM);
	char_from_room(victim);
	char_to_room(victim, room);
	act("$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM);
	do_look(victim, "auto");
}

/* Code by Corwyn ... Idea from MadROM */

void spell_undo_spell(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim;
	char spell[MSL], name[MSL];
	int undo_sn;
	/* target_name is a global in magic.c */
	target_name = one_argument(target_name, spell);
	one_argument(target_name, name);

	/* find undo_sn */
	if ((undo_sn = skill_lookup(spell)) < 1
	    || (skill_table[undo_sn].spell_fun == spell_null)) {
		stc("You failed.\n", ch);
		return;
	}

	if (name[0] == '\0')
		victim = ch;
	else {
		if ((victim = get_char_here(ch, target_name, VIS_CHAR)) == NULL) {
			stc("They aren't here.\n", ch);
			return;
		}
	}

	if (!IS_NPC(ch)) {
		if (!is_same_group(ch, victim) && victim != ch) {
			if (is_safe(ch, victim, FALSE)) {
				stc("Not on that target.\n", ch);
				return;
			}

			check_killer(ch, victim);
		}
	}

	if ((!IS_NPC(ch) && IS_NPC(victim)
	     && !(affect_exists_on_char(ch, gsn_charm_person) && ch->master == victim))
	    || (IS_NPC(ch) && !IS_NPC(victim))) {
		stc("You failed, try dispel magic.\n", ch);
		return;
	}

	if (undo_spell(level, victim, undo_sn, (ch != victim))) {
		stc("Ok.\n", ch);
	}
	else
		stc("Spell failed.\n", ch);
}

/* Vision spell by Lotus */
void spell_vision(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim;
	ROOM_INDEX_DATA *original_room;

	if ((victim = get_char_world(ch, target_name, VIS_CHAR)) == NULL
	    ||   victim == ch
	    ||   victim->in_room == NULL
	    ||   !can_see_room(ch, victim->in_room)
	    ||   IS_SET(GET_ROOM_FLAGS(victim->in_room), ROOM_PRIVATE)
	    ||   IS_SET(GET_ROOM_FLAGS(victim->in_room), ROOM_SOLITARY)
	    ||   IS_SET(GET_ROOM_FLAGS(victim->in_room), ROOM_NOVISION)
	    ||   victim->level >= level + 3
	    || IS_IMMORTAL(victim)
	    || (IS_NPC(victim) && saves_spell(level, victim, DAM_OTHER))) {
		stc("You failed.\n", ch);
		return;
	}

	stc("You conjure a mystical sphere and peer inside...\n", ch);
	original_room = ch->in_room;
	char_from_room(ch);
	char_to_room(ch, victim->in_room);
	act("You feel a presence in the room.", ch, NULL, NULL, TO_ROOM);
	do_look(ch, "auto");
	char_from_room(ch);
	char_to_room(ch, original_room);
}

void spell_ventriloquate(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	char buf1[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char speaker[MAX_INPUT_LENGTH];
	CHAR_DATA *vch;
	target_name = one_argument(target_name, speaker);
	sprintf(buf1, "%s says '%s'.\n",              speaker, target_name);
	sprintf(buf2, "Someone makes %s say '%s'.\n", speaker, target_name);
	buf1[0] = UPPER(buf1[0]);

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (!is_name(speaker, vch->name))
			stc(saves_spell(level, vch, DAM_OTHER) ? buf2 : buf1, vch);
	}

	return;
}

/* Wrath by Lotus */
void spell_wrath(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	damage(ch, victim, dice(level, 23), sn, DAM_ENERGY, TRUE, TRUE);
}

void spell_weaken(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, sn)) {
		act("Your spell comes too late. $N is already weak.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (saves_spell(level, victim, DAM_OTHER)) {
		act("$N's resistance is stronger than your spell.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	stc("You feel your strength slip away.\n", victim);
	act("$n looks tired and weak.", victim, NULL, NULL, TO_ROOM);

	affect_add_sn_to_char(victim,
		sn,
		level,
		level / 2,
		evolution,
		FALSE
	);
} /* end spell_weaken() */

/* RT recall spell is back */
void spell_word_of_recall(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	ROOM_INDEX_DATA *location;

	if (IS_NPC(victim))
		return;

	if (ch->in_room->sector_type != SECT_ARENA) {
		if ((location = get_room_index(ROOM_VNUM_TEMPLE)) == NULL) {
			stc("You are completely lost.\n", victim);
			return;
		}
	}
	else {
		if ((location = get_room_index(ROOM_VNUM_ARENACENTER)) == NULL) {
			stc("You are completely lost.\n", ch);
			return;
		}
	}

	if (!IS_NPC(ch))
		if (ch->pcdata->pktimer) {
			stc("The gods laugh at your cowardice...\n", ch);
			return;
		}

	if (IS_SET(GET_ROOM_FLAGS(victim->in_room), ROOM_NO_RECALL) ||
	    affect_exists_on_char(victim, gsn_curse) || char_in_duel_room(ch)) {
		stc("Spell failed.\n", victim);
		return;
	}

	if (victim->fighting != NULL)
		stop_fighting(victim, TRUE);

	if (!ch->in_room->clan || ch->in_room->clan != ch->clan)
		ch->stam = (ch->stam * 3) / 4;

	act("$n disappears.", victim, NULL, NULL, TO_ROOM);
	char_from_room(victim);
	char_to_room(victim, location);
	act("$n appears in the room.", victim, NULL, NULL, TO_ROOM);
	do_look(victim, "auto");
}

/*
 * NPC spells.
 */
void spell_acid_breath(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam, hp_dam, dice_dam, hpch;
	/*    if (victim->in_room->sector_type == SECT_ARENA)
	    {
	        stc("You cannot use this spell in the Arena.\n",ch);
	        return;
	    }
	*/
	act("$n spits acid at $N.", ch, NULL, victim, TO_NOTVICT);
	act("$n spits a stream of corrosive acid at you.", ch, NULL, victim, TO_VICT);
	act("You spit acid at $N.", ch, NULL, victim, TO_CHAR);
	hpch = UMAX(12, ch->hit);
	hp_dam = number_range(hpch / 11 + 1, hpch / 6);
	dice_dam = dice(level, 14);
	dam = UMAX(hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
	dam *= 2; /*Bring damage to a reasonable pk level */

	/* Players get more damage cause mobs have SOOO much hp */
	if (IS_NPC(victim))
		dam *= 4 / 3;

	if (saves_spell(level, victim, DAM_ACID)) {
		if (victim->in_room->sector_type != SECT_ARENA)
			acid_effect(victim, level / 2, dam / 4, TARGET_CHAR, evolution);

		damage(ch, victim, dam / 2, sn, DAM_ACID, TRUE, TRUE);
	}
	else {
		if (victim->in_room->sector_type != SECT_ARENA)
			acid_effect(victim, level, dam, TARGET_CHAR, evolution);

		damage(ch, victim, dam, sn, DAM_ACID, TRUE, TRUE);
	}
}

void spell_fire_breath(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *vch, *vch_next;
	int dam, hp_dam, dice_dam;
	int hpch;
	/*    if (victim->in_room->sector_type == SECT_ARENA)
	    {
	        stc("You cannot use this spell in the Arena.\n",ch);
	        return;
	    }
	*/
	act("$n breathes forth a cone of fire.", ch, NULL, victim, TO_NOTVICT);
	act("$n breathes a cone of hot fire over you!", ch, NULL, victim, TO_VICT);
	act("You breath forth a cone of fire.", ch, NULL, NULL, TO_CHAR);
	hpch = UMAX(10, ch->hit);
	hp_dam  = number_range(hpch / 9 + 1, hpch / 5);
	dice_dam = dice(level, 18);
	dam = UMAX(hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
	dam = dam / 2;
	dam *= 2; /*Bring damage to a reasonable pk level */

	/* Players get more damage cause mobs have SOOO much hp */
	if (IS_NPC(victim))
		dam *= 4 / 3;

	fire_effect(victim->in_room, level, dam / 2, TARGET_ROOM, evolution);

	for (vch = victim->in_room->people; vch != NULL; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch, vch, TRUE))
			continue;

		if (IS_NPC(vch) && IS_NPC(ch)
		    && (ch->fighting != vch || vch->fighting != ch))
			continue;

		/* special request: groupies safe -- Elrac */
		if (is_same_group(vch, ch))
			continue;

		if (vch == victim) { /* full damage */
			if (saves_spell(level, vch, DAM_FIRE)) {
				fire_effect(vch, level / 2, dam / 4, TARGET_CHAR, evolution);
				damage(ch, vch, 2 * dam / 2, sn, DAM_FIRE, TRUE, TRUE);
			}
			else {
				fire_effect(vch, level, dam, TARGET_CHAR, evolution);
				damage(ch, vch, 2 * dam, sn, DAM_FIRE, TRUE, TRUE);
			}
		}
		else { /* partial damage */
			if (saves_spell(level - 2, vch, DAM_FIRE)) {
				fire_effect(vch, level / 4, dam / 8, TARGET_CHAR, evolution);
				damage(ch, vch, dam / 4, sn, DAM_FIRE, TRUE, TRUE);
			}
			else {
				fire_effect(vch, level / 2, dam / 4, TARGET_CHAR, evolution);
				damage(ch, vch, dam / 2, sn, DAM_FIRE, TRUE, TRUE);
			}
		}
	}
}

void spell_frost_breath(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *vch, *vch_next;
	int dam, hp_dam, dice_dam, hpch;
	/*    if (victim->in_room->sector_type == SECT_ARENA)
	    {
	        stc("You cannot use this spell in the Arena.\n",ch);
	        return;
	    }
	*/
	act("$n breathes out a freezing cone of frost!", ch, NULL, victim, TO_NOTVICT);
	act("$n breathes a freezing cone of frost over you!",
	    ch, NULL, victim, TO_VICT);
	act("You breath out a cone of frost.", ch, NULL, NULL, TO_CHAR);
	hpch = UMAX(12, ch->hit);
	hp_dam = number_range(hpch / 11 + 1, hpch / 6);
	dice_dam = dice(level, 14);
	dam = UMAX(hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
	dam /= 2;
	dam *= 2; /*Bring damage to a reasonable pk level */

	/* Players get more damage cause mobs have SOOO much hp */
	if (IS_NPC(victim))
		dam *= 4 / 3;

	cold_effect(victim->in_room, level, dam / 2, TARGET_ROOM, evolution);

	for (vch = victim->in_room->people; vch != NULL; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch, vch, TRUE)
		    || (IS_NPC(vch) && IS_NPC(ch)
		        && (ch->fighting != vch || vch->fighting != ch)))
			continue;

		/* special request: groupies safe -- Elrac */
		if (is_same_group(vch, ch))
			continue;

		if (vch == victim) { /* full damage */
			if (saves_spell(level, vch, DAM_COLD)) {
				cold_effect(vch, level / 2, dam / 4, TARGET_CHAR, evolution);
				damage(ch, vch, 2 * dam / 2, sn, DAM_COLD, TRUE, TRUE);
			}
			else {
				cold_effect(vch, level, dam, TARGET_CHAR, evolution);
				damage(ch, vch, 2 * dam, sn, DAM_COLD, TRUE, TRUE);
			}
		}
		else {
			if (saves_spell(level - 2, vch, DAM_COLD)) {
				cold_effect(vch, level / 4, dam / 8, TARGET_CHAR, evolution);
				damage(ch, vch, dam / 4, sn, DAM_COLD, TRUE, TRUE);
			}
			else {
				cold_effect(vch, level / 2, dam / 4, TARGET_CHAR, evolution);
				damage(ch, vch, dam / 2, sn, DAM_COLD, TRUE, TRUE);
			}
		}
	}
}

void spell_gas_breath(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int dam, hp_dam, dice_dam, hpch;
	/*    if (ch->in_room->sector_type == SECT_ARENA)
	    {
	        stc("You cannot use this spell in the Arena.\n",ch);
	        return;
	    }
	*/
	act("$n breathes out a cloud of poisonous gas!", ch, NULL, NULL, TO_ROOM);
	act("You breath out a cloud of poisonous gas.", ch, NULL, NULL, TO_CHAR);
	hpch = UMAX(16, ch->hit);
	hp_dam = number_range(hpch / 15 + 1, 8);
	dice_dam = dice(level, 12);
	dam = UMAX(hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
	dam /= 2;
	dam *= 2; /*Bring damage to a reasonable pk level */
	poison_effect(ch->in_room, level, dam, TARGET_ROOM, evolution);

	for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch, vch, TRUE)
		    || (IS_NPC(ch) && IS_NPC(vch)
		        && (ch->fighting == vch || vch->fighting == ch)))
			continue;

		/* special request: groupies safe -- Elrac */
		if (is_same_group(vch, ch))
			continue;

		if (saves_spell(level, vch, DAM_POISON)) {
			poison_effect(vch, level / 2, dam / 4, TARGET_CHAR, evolution);
			damage(ch, vch, 2 * dam / 2, sn, DAM_POISON, TRUE, TRUE);
		}
		else {
			poison_effect(vch, level, dam, TARGET_CHAR, evolution);
			damage(ch, vch, 2 * dam, sn, DAM_POISON, TRUE, TRUE);
		}
	}
}

void spell_lightning_breath(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam, hp_dam, dice_dam, hpch;
	/*    if (victim->in_room->sector_type == SECT_ARENA)
	    {
	        stc("You cannot use this spell in the Arena.\n",ch);
	        return;
	    }
	*/
	act("$n breathes a bolt of lightning at $N.", ch, NULL, victim, TO_NOTVICT);
	act("$n breathes a bolt of lightning at you!", ch, NULL, victim, TO_VICT);
	act("You breathe a bolt of lightning at $N.", ch, NULL, victim, TO_CHAR);
	hpch = UMAX(10, ch->hit);
	hp_dam = number_range(hpch / 9 + 1, hpch / 5);
	dice_dam = dice(level, 18);
	dam = UMAX(hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
	dam *= 2; /*Bring damage to a reasonable pk level */

	/* Players get more damage cause mobs have SOOO much hp */
	if (IS_NPC(victim))
		dam *= 4 / 3;

	if (saves_spell(level, victim, DAM_ELECTRICITY)) {
		shock_effect(victim, level / 2, dam / 4, TARGET_CHAR, evolution);
		damage(ch, victim, dam / 2, sn, DAM_ELECTRICITY, TRUE, TRUE);
	}
	else {
		shock_effect(victim, level, dam, TARGET_CHAR, evolution);
		damage(ch, victim, dam, sn, DAM_ELECTRICITY, TRUE, TRUE);
	}
}

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
void spell_general_purpose(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	dam = number_range(25, 100);

	if (saves_spell(level, victim, DAM_PIERCE))
		dam /= 2;

	damage(ch, victim, dam, sn, DAM_PIERCE , TRUE, TRUE);
	return;
}

void spell_high_explosive(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	dam = number_range(30, 120);

	if (saves_spell(level, victim, DAM_PIERCE))
		dam /= 2;

	damage(ch, victim, dam, sn, DAM_PIERCE , TRUE, TRUE);
	return;
}

/* age by Lotus */
void spell_age(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (affect_exists_on_char(victim, sn)) {
		act("$N is already aged beyond $S years.", ch, NULL, vo, TO_CHAR);
		return;
	}

	if (saves_spell(level, victim, DAM_OTHER)) {
		act("As if by a miracle, $N retains $S youth.", ch, NULL, vo, TO_CHAR);
		return;
	}

	affect_add_sn_to_char(victim,
		sn,
		level,
		level / 4,
		evolution,
		FALSE
	);

	stc("You feel yourself grow old and frail.\n", victim);
	act("$n looks much older.", victim, NULL, NULL, TO_ROOM);
} /* end spell_age() */

/*
This spell function makes the target both hungry and thirsty,
and makes them not-full.
It's more of an annoyance than anything else.

-- Outsider
*/
void spell_starve(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (! victim) return;

	/* do not cast this spell on NPCs, will seg fault */
	if (IS_NPC(victim)) {
		stc("Your spell seems to have no effect.\n", ch);
		return;
	}

	if (saves_spell(level, victim, DAM_OTHER)) {
		act("$N appears uneffected.", ch, NULL, vo, TO_CHAR);
		return;
	}

	victim->pcdata->condition[COND_HUNGER] = 0;
	victim->pcdata->condition[COND_THIRST] = 0;
	victim->pcdata->condition[COND_FULL] = 0;
	stc("You are suddenly very hungry.\n", victim);
	act("$n's stomach growls.", victim, NULL, NULL, TO_ROOM);
}
