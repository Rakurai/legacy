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
#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "affect.h"
#include "buffer.h"

// TODO: temporary access, remove when possible
extern void affect_modify_char args((void *owner, const AFFECT_DATA *paf, bool fAdd));

/* friend stuff -- for NPC's mostly */
bool is_friend(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (is_same_group(ch, victim))
		return TRUE;

	if (!IS_NPC(ch))
		return FALSE;

	if (!IS_NPC(victim)) {
		if (IS_SET(ch->off_flags, ASSIST_PLAYERS))
			return TRUE;
		else
			return FALSE;
	}

	if (affect_exists_on_char(ch, gsn_charm_person))
		return FALSE;

	if (IS_SET(ch->off_flags, ASSIST_ALL))
		return TRUE;

	if (ch->group && ch->group == victim->group)
		return TRUE;

	if (IS_SET(ch->off_flags, ASSIST_VNUM)
	    &&  ch->pIndexData == victim->pIndexData)
		return TRUE;

	if (IS_SET(ch->off_flags, ASSIST_RACE) && ch->race == victim->race)
		return TRUE;

	if (IS_SET(ch->off_flags, ASSIST_ALIGN)
	    &&  !IS_SET(ch->act, ACT_NOALIGN) && !IS_SET(victim->act, ACT_NOALIGN)
	    && ((IS_GOOD(ch) && IS_GOOD(victim))
	        || (IS_EVIL(ch) && IS_EVIL(victim))
	        || (IS_NEUTRAL(ch) && IS_NEUTRAL(victim))))
		return TRUE;

	return FALSE;
}

/* returns number of people on an object */
int count_users(OBJ_DATA *obj)
{
	CHAR_DATA *fch;
	int count = 0;

	if (obj->in_room == NULL)
		return 0;

	for (fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room)
		if (fch->on == obj)
			count++;

	return count;
}

/* returns race number */
int race_lookup(const char *name)
{
	int race;

	for (race = 0; race_table[race].name != NULL; race++) {
		if (LOWER(name[0]) == LOWER(race_table[race].name[0])
		    &&  !str_prefix1(name, race_table[race].name))
			return race;
	}

	return 0;
}

int liq_lookup(const char *name)
{
	int liq;

	for (liq = 0; liq_table[liq].liq_name != NULL; liq++) {
		if (LOWER(name[0]) == LOWER(liq_table[liq].liq_name[0])
		    && !str_prefix1(name, liq_table[liq].liq_name))
			return liq;
	}

	return -1;
}

int weapon_lookup(const char *name)
{
	int type;

	for (type = 0; weapon_table[type].name != NULL; type++) {
		if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
		    &&  !str_prefix1(name, weapon_table[type].name))
			return type;
	}

	return -1;
}

int weapon_type(const char *name)
{
	int type;

	for (type = 0; weapon_table[type].name != NULL; type++) {
		if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
		    &&  !str_prefix1(name, weapon_table[type].name))
			return weapon_table[type].type;
	}

	return WEAPON_EXOTIC;
}

int item_lookup(const char *name)
{
	int type;

	for (type = 0; item_table[type].name != NULL; type++) {
		if (LOWER(name[0]) == LOWER(item_table[type].name[0])
		    &&  !str_prefix1(name, item_table[type].name))
			return item_table[type].type;
	}

	return -1;
}

int attack_lookup(const char *name)
{
	int att;

	for (att = 0; attack_table[att].name != NULL; att++) {
		if (LOWER(name[0]) == LOWER(attack_table[att].name[0])
		    &&  !str_prefix1(name, attack_table[att].name))
			return att;
	}

	return 0;
}

/* returns class number */
int class_lookup(const char *name)
{
	int cls;

	for (cls = 0; cls < MAX_CLASS; cls++) {
		if (!str_prefix1(name, class_table[cls].name))
			return cls;
	}

	return -1;
}

bool is_clan(CHAR_DATA *ch)
{
	return (ch->clan != NULL);
}

bool is_same_clan(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (ch == NULL || victim == NULL || ch->clan == NULL || victim->clan == NULL)
		return FALSE;

	if (ch->clan->independent && !IS_IMMORTAL(ch))
		return FALSE;
	else
		return (ch->clan == victim->clan);
}

/* for returning skill information */
int get_skill(const CHAR_DATA *ch, int sn)
{
	int skill = 0;

	if (sn == -1) /* shorthand for level based skills */
		skill = ch->level * 5 / 2;
	else if (sn < -1 || sn > MAX_SKILL) {
		bug("Bad sn %d in get_skill.", sn);
		skill = 0;
	}
	else if (!IS_NPC(ch)) {
		if (ch->level < skill_table[sn].skill_level[ch->cls])
			skill = 0;
		else
			skill = ch->pcdata->learned[sn];
	}
	else { /* mobiles */
		if ((sn == gsn_dodge && IS_SET(ch->off_flags, OFF_DODGE))
		    || (sn == gsn_parry && IS_SET(ch->off_flags, OFF_PARRY))
		    || (sn == gsn_kick && IS_SET(ch->off_flags, OFF_KICK)))
			skill = 10 + ch->level;
		else if (sn == gsn_shield_block)
			skill = 15 + ch->level;
		else if (sn == gsn_second_attack
		         && (IS_SET(ch->act, ACT_WARRIOR) || IS_SET(ch->act, ACT_THIEF)))
			skill = 25 + ch->level;
		else if (sn == gsn_third_attack && IS_SET(ch->act, ACT_WARRIOR))
			skill = 15 + ch->level;
		else if (sn == gsn_fourth_attack && IS_SET(ch->act, ACT_WARRIOR))
			skill = 2 * (ch->level - 60);
		else if (sn == gsn_hand_to_hand)
			skill = ch->level * 3 / 2;
		else if ((sn == gsn_trip && IS_SET(ch->off_flags, OFF_TRIP))
		         || (sn == gsn_dirt_kicking && IS_SET(ch->off_flags, OFF_KICK_DIRT)))
			skill = 10 + (ch->level * 3 / 2);
		else if (sn == gsn_bash && IS_SET(ch->off_flags, OFF_BASH))
			skill = 10 + (ch->level * 5 / 4);
		else if (sn == gsn_crush && IS_SET(ch->off_flags, OFF_CRUSH))
			skill = ch->level;
		else if (sn == gsn_disarm
		         && (IS_SET(ch->off_flags, OFF_DISARM)
		             ||       IS_SET(ch->act, ACT_WARRIOR)
		             ||       IS_SET(ch->act, ACT_THIEF)))
			skill = 20 + (ch->level * 2 / 3);
		else if (sn == gsn_berserk && IS_SET(ch->off_flags, OFF_BERSERK))
			skill = 3 * ch->level;
		else if (sn == gsn_backstab && IS_SET(ch->act, ACT_THIEF))
			skill = 20 + (ch->level * 2);
		else if (sn == gsn_rescue)
			skill = 40 + (ch->level / 2);
		else if (sn == gsn_recall)
			skill = 40 + ch->level;
		else if (sn == gsn_sword
		         ||       sn == gsn_dagger
		         ||       sn == gsn_spear
		         ||       sn == gsn_mace
		         ||       sn == gsn_axe
		         ||       sn == gsn_flail
		         ||       sn == gsn_whip
		         ||       sn == gsn_polearm)
			skill = 40 + (5 * ch->level / 2);
		else if (sn == gsn_scrolls)
			skill = ch->level;
		else
			skill = 0;
	}

	if (ch->daze > 0) {
		if (skill_table[sn].spell_fun != spell_null)
			skill /= 2;
		else
			skill = 2 * skill / 3;
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10)
		skill = 9 * skill / 10;

	return URANGE(0, skill, 100);
}
/* returns deity number */
int deity_lookup(const char *name)
{
	int deity;

	for (deity = 0; deity_table[deity].name != NULL; deity++) {
		if (!str_prefix1(name, deity_table[deity].name))
			return deity;
	}

	return -1;
}

/* for returning weapon information */
int get_weapon_sn(CHAR_DATA *ch, bool secondary)
{
	OBJ_DATA *wield;
	int sn;

	if (secondary)
		wield = get_eq_char(ch, WEAR_SECONDARY);
	else
		wield = get_eq_char(ch, WEAR_WIELD);

	if (wield == NULL || wield->item_type != ITEM_WEAPON)
		sn = gsn_hand_to_hand;
	else switch (wield->value[0]) {
		default :               sn = -1;                break;

		case (WEAPON_SWORD):     sn = gsn_sword;         break;

		case (WEAPON_DAGGER):    sn = gsn_dagger;        break;

		case (WEAPON_SPEAR):     sn = gsn_spear;         break;

		case (WEAPON_MACE):      sn = gsn_mace;          break;

		case (WEAPON_AXE):       sn = gsn_axe;           break;

		case (WEAPON_FLAIL):     sn = gsn_flail;         break;

		case (WEAPON_WHIP):      sn = gsn_whip;          break;

		case (WEAPON_POLEARM):   sn = gsn_polearm;       break;
		}

	return sn;
} /* end get_weapon_sn() */

int get_weapon_skill(CHAR_DATA *ch, int sn)
{
	int skill;

	/* -1 is exotic */
	if (IS_NPC(ch)) {
		if (sn == -1)
			skill = 3 * ch->level;
		else if (sn == gsn_hand_to_hand)
			skill = 40 + 2 * ch->level;
		else
			skill = 40 + 5 * ch->level / 2;
	}
	else {
		if (sn == -1)
			skill = 3 * ch->level;
		else
			skill = ch->pcdata->learned[sn];
	}

	return URANGE(0, skill, 100);
}

/* command for returning max training score */
int get_max_train(CHAR_DATA *ch, int stat)
{
	int max;

	if (IS_NPC(ch) || IS_IMMORTAL(ch))
		return 25;

	max = pc_race_table[ch->race].max_stats[stat];

	if (class_table[ch->cls].stat_prime == stat) {
		if (ch->race == race_lookup("human"))
			max += 3;
		else
			max += 2;
	}

	return UMIN(max, 25);
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n(CHAR_DATA *ch)
{
	if (IS_IMMORTAL(ch))
		return 9999;

	if (IS_NPC(ch) && IS_SET(ch->act, ACT_PET))
		return 0;

	return MAX_WEAR +  2 * GET_ATTR_DEX(ch) + ch->level;
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w(CHAR_DATA *ch)
{
	if (IS_IMMORTAL(ch))
		return 10000000;

	if (IS_NPC(ch) && IS_SET(ch->act, ACT_PET))
		return 0;

	return str_app[GET_ATTR_STR(ch)].carry * 10 + ch->level * 25;
}


/*
 * Move a char out of a room.
 */
void char_from_room(CHAR_DATA *ch)
{
	OBJ_DATA *obj;

	if (ch == NULL)
		return;

	if (ch->in_room == NULL) {
		bug("Char_from_room: NULL.", 0);
		return;
	}

	if (!IS_NPC(ch))
		--ch->in_room->area->nplayer;

	if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL
	    &&   obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] != 0
	    &&   ch->in_room->light > 0)
		--ch->in_room->light;

	if (ch == ch->in_room->people)
		ch->in_room->people = ch->next_in_room;
	else {
		CHAR_DATA *prev;

		for (prev = ch->in_room->people; prev; prev = prev->next_in_room) {
			if (prev->next_in_room == ch) {
				prev->next_in_room = ch->next_in_room;
				break;
			}
		}

		if (prev == NULL) {
			bug("Char_from_room: ch not found.", 0);
			bug(ch->name, 0);
		}
	}

	ch->in_room      = NULL;
	ch->next_in_room = NULL;
	ch->on           = NULL;  /* sanity check! */
	return;
}

/*
 * Move a char into a room.
 */
void char_to_room(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex)
{
	OBJ_DATA *obj;

	if (ch == NULL)
		return;

	if (pRoomIndex == NULL) {
		ROOM_INDEX_DATA *room;
		bug("Char_to_room: NULL.", 0);
		bug(ch->name, 0);

		if ((room = get_room_index(ROOM_VNUM_TEMPLE)) != NULL)
			char_to_room(ch, room);

		return;
	}

	ch->in_room         = pRoomIndex;
	ch->next_in_room    = pRoomIndex->people;
	pRoomIndex->people  = ch;

	if (!IS_NPC(ch)) {
		if (ch->in_room->area->empty) {
			ch->in_room->area->empty = FALSE;
			ch->in_room->area->age = 0;
		}

		++ch->in_room->area->nplayer;
	}

	if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL
	    &&   obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] != 0)
		++ch->in_room->light;

	const AFFECT_DATA *plague = affect_find_on_char(ch, gsn_plague);
	if (plague)
		spread_plague(ch->in_room, plague, 6);
}

/* Locker Code */
void obj_from_locker(OBJ_DATA *obj)
{
	CHAR_DATA *ch;

	if ((ch = obj->in_locker) == NULL) {
		bug("obj_from_locker: obj points to null ch", 0);
		return;
	}

	if (IS_NPC(ch)) {
		bug("obj_from_locker: obj points to mobile", 0);
		return;
	}

	if (ch->pcdata->locker == obj)
		ch->pcdata->locker = obj->next_content;
	else {
		OBJ_DATA *prev;

		for (prev = ch->pcdata->locker; prev != NULL; prev = prev->next_content)
			if (prev->next_content == obj) {
				prev->next_content = obj->next_content;
				break;
			}

		if (prev == NULL)
			bug("obj_from_locker: obj not in ch->pcdata->locker list.", 0);
	}

	obj->next_content    = NULL;
	obj->in_locker       = NULL;
}

void obj_from_strongbox(OBJ_DATA *obj)
{
	CHAR_DATA *ch;

	if ((ch = obj->in_strongbox) == NULL) {
		bug("obj_from_strongbox: obj points to null ch", 0);
		return;
	}

	if (IS_NPC(ch)) {
		bug("obj_from_strongbox: obj points to a mobile", 0);
		return;
	}

	if (ch->pcdata->strongbox == obj)
		ch->pcdata->strongbox = obj->next_content;
	else {
		OBJ_DATA *prev;

		for (prev = ch->pcdata->strongbox; prev != NULL; prev = prev->next_content)
			if (prev->next_content == obj) {
				prev->next_content = obj->next_content;
				break;
			}

		if (prev == NULL)
			bug("Obj_from_strongbox: obj not in list.", 0);
	}

	obj->next_content    = NULL;
	obj->in_strongbox       = NULL;
	return;
} /* end obj_from_strongbox() */

void obj_to_locker(OBJ_DATA *obj, CHAR_DATA *ch)
{
	if (IS_NPC(ch)) {
		bug("obj_to_locker: ch is a mobile", 0);
		return;
	}

	obj->next_content       = ch->pcdata->locker;
	ch->pcdata->locker      = obj;
	obj->in_locker          = ch;
	obj->carried_by         = NULL;
	obj->in_room            = NULL;
	obj->in_obj             = NULL;
}

void obj_to_strongbox(OBJ_DATA *obj, CHAR_DATA *ch)
{
	if (IS_NPC(ch)) {
		bug("obj_to_strongbox: ch is a mobile", 0);
		return;
	}

	obj->next_content       = ch->pcdata->strongbox;
	ch->pcdata->strongbox   = obj;
	obj->in_strongbox       = ch;
	obj->carried_by         = NULL;
	obj->in_room            = NULL;
	obj->in_obj             = NULL;
} /* end obj_to_strongbox() */

/*
 * Give an obj to a char.
 */
void obj_to_char(OBJ_DATA *obj, CHAR_DATA *ch)
{
	obj->next_content    = ch->carrying;
	ch->carrying         = obj;
	obj->carried_by      = ch;
	obj->in_locker       = NULL;
	obj->in_strongbox   = NULL;
	obj->in_room         = NULL;
	obj->in_obj          = NULL;
	obj->clean_timer = 0;
}

/*
 * Take an obj from its character.
 */
void obj_from_char(OBJ_DATA *obj)
{
	CHAR_DATA *ch;

	if ((ch = obj->carried_by) == NULL) {
		bug("Obj_from_char: null ch.", 0);
		return;
	}

	if (obj->wear_loc != WEAR_NONE)
		unequip_char(ch, obj);

	if (ch->carrying == obj)
		ch->carrying = obj->next_content;
	else {
		OBJ_DATA *prev;

		for (prev = ch->carrying; prev != NULL; prev = prev->next_content) {
			if (prev->next_content == obj) {
				prev->next_content = obj->next_content;
				break;
			}
		}

		if (prev == NULL)
			bug("Obj_from_char: obj not in list.", 0);
	}

	obj->carried_by      = NULL;
	obj->next_content    = NULL;
}

/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac(OBJ_DATA *obj, int iWear, int type)
{
	if (obj->item_type != ITEM_ARMOR)
		return 0;

	switch (iWear) {
	case WEAR_LIGHT:    return     0; /* cannot be armor */

	case WEAR_FINGER_L: return     obj->value[type];

	case WEAR_FINGER_R: return     obj->value[type];

	case WEAR_NECK_1:   return     obj->value[type];

	case WEAR_NECK_2:   return     obj->value[type];

	case WEAR_BODY:     return 3 * obj->value[type];

	case WEAR_HEAD:     return 2 * obj->value[type];

	case WEAR_LEGS:     return 2 * obj->value[type];

	case WEAR_FEET:     return     obj->value[type];

	case WEAR_HANDS:    return     obj->value[type];

	case WEAR_ARMS:     return     obj->value[type];

	case WEAR_SHIELD:   return     obj->value[type];

	case WEAR_ABOUT:    return 2 * obj->value[type];

	case WEAR_WAIST:    return     obj->value[type];

	case WEAR_WRIST_L:  return     obj->value[type];

	case WEAR_WRIST_R:  return     obj->value[type];

	case WEAR_HOLD:     return     obj->value[type];

	case WEAR_FLOAT:    return     obj->value[type];
	}

	return 0;
}

/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char(CHAR_DATA *ch, int iWear)
{
	OBJ_DATA *obj;

	if (ch == NULL)
		return NULL;

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->wear_loc == iWear)
			return obj;
	}

	return NULL;
}

/*
 * Equip a char with an obj.
 */
void equip_char(CHAR_DATA *ch, OBJ_DATA *obj, int iWear)
{
	// see if some *other* object is equipped here.  allow equipping the object even
	// if get_eq_char (dumbly) says its already equipped, because of how load_char_obj
	// loads the whole list and then equips it all based on the object's wear location
	OBJ_DATA *equipped = get_eq_char(ch, iWear);
	if (equipped != NULL && equipped != obj) {
		bug("Equip_char: already equipped (%d).", iWear);
		return;
	}

	for (int i = 0; i < 4; i++)
		ch->armor_base[i] -= apply_ac(obj, iWear, i);

	obj->wear_loc = iWear;

	for (const AFFECT_DATA *paf = affect_list_obj(obj); paf != NULL; paf = paf->next)
		affect_modify_char(ch, paf, TRUE);

	if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room != NULL)
		++ch->in_room->light;
}

/*
 * Unequip a char with an obj.
 */
void unequip_char(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (obj->wear_loc == WEAR_NONE) {
		bug("Unequip_char: already unequipped.", 0);
		return;
	}

	for (int i = 0; i < 4; i++)
		ch->armor_base[i]    += apply_ac(obj, obj->wear_loc, i);

	obj->wear_loc        = -1;

	for (const AFFECT_DATA *paf = affect_list_obj(obj); paf != NULL; paf = paf->next)
		affect_modify_char(ch, paf, FALSE);

	if (obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] != 0
	    &&   ch->in_room != NULL
	    &&   ch->in_room->light > 0)
		--ch->in_room->light;

	return;
}

/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list(OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list)
{
	OBJ_DATA *obj;
	int nMatch;
	nMatch = 0;

	for (obj = list; obj != NULL; obj = obj->next_content) {
		if (obj->pIndexData == pObjIndex)
			nMatch++;
	}

	return nMatch;
}

/*
 * Move an obj out of a room.
 */
void obj_from_room(OBJ_DATA *obj)
{
	ROOM_INDEX_DATA *in_room;
	CHAR_DATA *ch;

	if ((in_room = obj->in_room) == NULL) {
		bug("obj_from_room: NULL.", 0);
		bug(obj->short_descr, 0); /* Bug huntin - Lotus */
		return;
	}

	for (ch = in_room->people; ch != NULL; ch = ch->next_in_room)
		if (ch->on == obj)
			ch->on = NULL;

	if (obj == in_room->contents)
		in_room->contents = obj->next_content;
	else {
		OBJ_DATA *prev;

		for (prev = in_room->contents; prev; prev = prev->next_content) {
			if (prev->next_content == obj) {
				prev->next_content = obj->next_content;
				break;
			}
		}

		if (prev == NULL) {
			bug("Obj_from_room: obj not found.", 0);
			return;
		}
	}

	obj->in_room      = NULL;
	obj->next_content = NULL;
	return;
}

/*
 * Move an obj into a room.
 */
void obj_to_room(OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex)
{
	/* make sure there is a room to put the object in -- Elrac */
	if (pRoomIndex == NULL) {
		bug("obj_to_room: NULL room", 0);
		return;
	}

	/* Floating Rooms by Lotus - Idea from WWW site */
	while (pRoomIndex &&
	       pRoomIndex->sector_type == SECT_AIR &&
	       (obj->wear_flags & ITEM_TAKE) &&
	       pRoomIndex->exit[DIR_DOWN] &&
	       pRoomIndex->exit[DIR_DOWN]->u1.to_room) {
		ROOM_INDEX_DATA *new_room =
		        pRoomIndex->exit[DIR_DOWN]->u1.to_room;
		CHAR_DATA *rch;

		if ((rch = pRoomIndex->people) != NULL) {
			act("$p falls away.", rch, obj, NULL, TO_ROOM);
			act("$p falls away.", rch, obj, NULL, TO_CHAR);
		}

		pRoomIndex = new_room;

		if ((rch = new_room->people) != NULL) {
			act("$p floats by.", rch, obj, NULL, TO_ROOM);
			act("$p floats by.", rch, obj, NULL, TO_CHAR);
		}
	}

	obj->next_content           = pRoomIndex->contents;
	pRoomIndex->contents        = obj;
	obj->in_room                = pRoomIndex;
	obj->carried_by             = NULL;
	obj->in_obj                 = NULL;
	return;
}

/*
 * Move an object into an object.
 */
void obj_to_obj(OBJ_DATA *obj, OBJ_DATA *obj_to)
{
	if (obj_to == donation_pit) {
		obj->cost = 0;

		// don't *always* set, because do_pit temporarily removes items
		if (obj->donated == 0) { // new object going in
			obj->donated = current_time;

			// count items in the pit, if there are too many, remove the oldest
			OBJ_DATA *oldest = NULL;
			int count = 0;

			for (OBJ_DATA *c = donation_pit->contains; c; c = c->next_content) {
				count++;
				if (oldest == NULL || oldest->donated > c->donated)
					oldest = c;
			}

			if (count >= MAX_DONATED && oldest != NULL)
				extract_obj(oldest); // will remove from the pit
		}
	}

	obj->next_content       = obj_to->contains;
	obj_to->contains        = obj;
	obj->in_obj             = obj_to;
	obj->in_room            = NULL;
	obj->carried_by         = NULL;
}

/*
 * Move an object out of an object.
 */
void obj_from_obj(OBJ_DATA *obj)
{
	OBJ_DATA *obj_from;

	if ((obj_from = obj->in_obj) == NULL) {
		bug("Obj_from_obj: null obj_from.", 0);
		return;
	}

	if (obj == obj_from->contains)
		obj_from->contains = obj->next_content;
	else {
		OBJ_DATA *prev;

		for (prev = obj_from->contains; prev; prev = prev->next_content) {
			if (prev->next_content == obj) {
				prev->next_content = obj->next_content;
				break;
			}
		}

		if (prev == NULL) {
			bug("Obj_from_obj: obj not found.", 0);
			return;
		}
	}

	obj->next_content = NULL;
	obj->in_obj       = NULL;
}

/*
 * Extract an obj from the world.
 */
void extract_obj(OBJ_DATA *obj)
{
	OBJ_DATA *obj_content;
	OBJ_DATA *obj_next;

	if (obj->in_room)          obj_from_room(obj);
	else if (obj->carried_by)       obj_from_char(obj);
	else if (obj->in_obj)           obj_from_obj(obj);
	else if (obj->in_locker)        obj_from_locker(obj);
	else if (obj->in_strongbox)     obj_from_strongbox(obj);

	for (obj_content = obj->contains; obj_content; obj_content = obj_next) {
		obj_next = obj_content->next_content;
		extract_obj(obj_content);
	}

	if (object_list == obj)
		object_list = obj->next;
	else {
		OBJ_DATA *prev;

		for (prev = object_list; prev != NULL; prev = prev->next) {
			if (prev->next == obj) {
				prev->next = obj->next;
				break;
			}
		}

		if (prev == NULL) {
			bug("Extract_obj: obj %d not found.", obj->pIndexData->vnum);
			return;
		}
	}

	--obj->pIndexData->count;
	free_obj(obj);
	return;
}

/*
 * Extract a char from the world.
 */
void extract_char(CHAR_DATA *ch, bool fPull)
{
	CHAR_DATA *wch;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if (ch->in_room == NULL) {
//		bugf("extract_char: ch->in_room == NULL for %s", ch->name);
		return;
	}

	nuke_pets(ch);
	ch->pet = NULL; /* just in case */

	if (fPull)
		die_follower(ch);

	stop_fighting(ch, TRUE);

	for (obj = ch->carrying; obj != NULL; obj = obj_next) {
		obj_next = obj->next_content;
		extract_obj(obj);
	}

	char_from_room(ch);

	/* Death room is set in the clan table now */
	if (!fPull) {
		char_to_room(ch, get_room_index((ch->clan != NULL) ? ch->clan->hall : ROOM_VNUM_ALTAR));
		return;
	}

	if (!IS_NPC(ch)) {
		for (obj = ch->pcdata->locker; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			extract_obj(obj);
		}

		for (obj = ch->pcdata->strongbox; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			extract_obj(obj);
		}
	}

	if (IS_NPC(ch))
		--ch->pIndexData->count;

	if (ch->desc != NULL && ch->desc->original != NULL) {
		do_return(ch, "");
		ch->desc = NULL;
	}

	for (wch = char_list; wch != NULL; wch = wch->next)
		if (! strcasecmp(wch->reply, ch->name))
			wch->reply[0] = '\0';

	if (ch == char_list)
		char_list = ch->next;
	else {
		CHAR_DATA *prev;

		for (prev = char_list; prev != NULL; prev = prev->next) {
			if (prev->next == ch) {
				prev->next = ch->next;
				break;
			}
		}

		if (prev == NULL) {
			bug("extract_char: char not found in char_list", 0);
			return;
		}
	}

	if (ch->pcdata) {
		if (ch->pcdata == pc_list)
			pc_list = ch->pcdata->next;
		else {
			PC_DATA *prev;

			for (prev = pc_list; prev != NULL; prev = prev->next) {
				if (prev->next == ch->pcdata) {
					prev->next = ch->pcdata->next;
					break;
				}
			}

			if (prev == NULL)
				bug("extract_char: pc_data not found in pc_list", 0);
		}
	}

	if (ch->desc != NULL)
		ch->desc->character = NULL;

	free_char(ch);
}

/*
 * Find out if a mob with a given name exists anywhere -- Elrac
 * This is for the mob name creation check in comm.c
 * and does not require a 'ch' argument.

        old way of doing this was to loop through all mobiles, but
        that takes too long.  loop through the mob index and we can
        check all possible mobs, not just ones alive now -- Montrey
 */
bool mob_exists(const char *name)
{
	MOB_INDEX_DATA *index;
	int i;
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];

	for (i = 0; i < MAX_KEY_HASH; i++)
		for (index = mob_index_hash[i]; index != NULL; index = index->next)
			if (is_exact_name(name, index->player_name))
				return TRUE;

	return FALSE;
}

/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type(OBJ_INDEX_DATA *pObjIndex)
{
	OBJ_DATA *obj;

	for (obj = object_list; obj != NULL; obj = obj->next) {
		if (obj->pIndexData == pObjIndex)
			return obj;
	}

	return NULL;
}

/* deduct cost from a character */
bool deduct_cost(CHAR_DATA *ch, long cost)
{
	int silver, gold = 0;

	if (cost < 0) { /* a refund? */
		cost *= -1;     /* make it positive */
		gold = cost / 100;
		silver = cost - (gold * 100);
		ch->gold += gold;
		ch->silver += silver;
		return TRUE;
	}

	if (ch->silver + (100 * ch->gold) < cost)               /* not enough */
		return FALSE;

	silver = UMIN(ch->silver, cost);

	if (silver < cost) {
		gold = ((cost - silver + 99) / 100);
		silver = cost - (100 * gold);
	}

	ch->gold -= gold;
	ch->silver -= silver;
	return TRUE;
}

/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money(int gold, int silver)
{
	OBJ_DATA *obj;
	char name[MSL], short_descr[MSL], description[MSL], material[MSL];
	int amt = 0;
	const char *amount[] = {
		"A tiny",
		"A small",
		"A fair sized",
		"A large",
		"A huge",
		"An insanely enormous",
		"A monumental",
		"An engulfing"
	};

	if (gold <= 0 && silver <= 0) {
		bug("Create_money: zero or negative money.", 0);
		gold = UMAX(1, gold);
		silver = UMAX(1, silver);
	}

	if (gold + silver > 1) {
		if (gold + silver > 500000)        amt = 7;
		else if (gold + silver > 100000)        amt = 6;
		else if (gold + silver > 25000)         amt = 5;
		else if (gold + silver > 5000)          amt = 4;
		else if (gold + silver > 500)           amt = 3;
		else if (gold + silver > 100)           amt = 2;
		else if (gold + silver > 20)            amt = 1;
		else                                    amt = 0;
	}

	if ((obj = create_object(get_obj_index(GEN_OBJ_MONEY), 0)) == NULL) {
		bug("create_money: no generic money object", 0);
		return NULL;
	}

	if (gold + silver == 1) {
		Format::sprintf(name, "coin %s gcash", silver == 1 ? "silver" : "gold");
		Format::sprintf(short_descr, "a %s coin", silver == 1 ? "silver" : "gold");
		Format::sprintf(description, "One %s coin.", silver == 1 ? "miserable silver" : "valuable gold");
		Format::sprintf(material, silver == 1 ? "silver" : "gold");
	}
	else {
		char silver_desc[MSL], gold_desc[MSL];
		/* name */
		Format::sprintf(name, "coins %s%sgcash",
		        silver > 0 ? "silver " : "",
		        gold > 0 ? "gold " : "");

		/* short description */
		if (silver == 1)
			Format::sprintf(silver_desc, "one silver coin");
		else
			Format::sprintf(silver_desc, "%d silver coins", silver);

		if (gold == 1)
			Format::sprintf(gold_desc, "%sone gold coin", silver == 0 ? "" : " and ");
		else
			Format::sprintf(gold_desc, "%s%d gold coins", silver == 0 ? "" : " and ", gold);

		Format::sprintf(short_descr, "%s%s",
		        silver > 0 ? silver_desc : "",
		        gold > 0 ? gold_desc : "");
		/* long description */
		Format::sprintf(description, "%s pile of %scoins.",
		        amount[amt],
		        silver == 0 ? "gold " : gold == 0 ? "silver " : "");
		/* material */
		Format::sprintf(material, "%s%s",
		        silver > 0 ? "silver" : "",
		        gold > 0 ? silver > 0 ? " and gold" : "gold" : "");
	}

	free_string(obj->name);
	free_string(obj->short_descr);
	free_string(obj->description);
	free_string(obj->material);
	obj->name               = str_dup(name);
	obj->short_descr        = str_dup(short_descr);
	obj->description        = str_dup(description);
	obj->material           = str_dup(material);
	obj->value[0]           = silver;
	obj->value[1]           = gold;
	obj->cost               = 100 * gold + silver;
	obj->weight             = URANGE(1, ((gold / 5) + (silver / 20)), 30000);
	return obj;
}

/* below 2 functions replace the ints in pcdata, the complexity of trying to keep
   them correct wasn't worth the speed savings -- Montrey */
int get_locker_number(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	int number = 0;

	if (IS_NPC(ch))
		return 0;

	for (obj = ch->pcdata->locker; obj; obj = obj->next_content)
		number += get_obj_number(obj);

	return number;
}

int get_strongbox_number(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	int number = 0;

	if (IS_NPC(ch))
		return 0;

	for (obj = ch->pcdata->strongbox; obj; obj = obj->next_content)
		number += get_obj_number(obj);

	return number;
}

int get_carry_number(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	int number = 0;

	for (obj = ch->carrying; obj; obj = obj->next_content)
		number += get_obj_number(obj);

	return number;
}

int get_obj_number(OBJ_DATA *obj)
{
	int number = 1;

	if (((obj->item_type == ITEM_CONTAINER || IS_OBJ_STAT(obj, ITEM_COMPARTMENT)) && obj->contains)
	    || obj->item_type == ITEM_MONEY || obj->item_type == ITEM_TOKEN)
		number = 0;

	for (obj = obj->contains; obj != NULL; obj = obj->next_content)
		number += get_obj_number(obj);

	return number;
}

int get_locker_weight(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	int weight = 0;

	if (IS_NPC(ch))
		return 0;

	for (obj = ch->pcdata->locker; obj; obj = obj->next_content)
		weight += get_obj_weight(obj);

	return weight;
}

int get_carry_weight(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	int weight = 0;

	for (obj = ch->carrying; obj; obj = obj->next_content)
		weight += get_obj_weight(obj);

	weight += gold_weight(ch->gold) + silver_weight(ch->silver);
	return URANGE(0, weight, 9999);         /* prevent score from screwing up */
}

int get_obj_weight(OBJ_DATA *obj)
{
	int weight;
	OBJ_DATA *tobj;
	weight = obj->weight;

	for (tobj = obj->contains; tobj != NULL; tobj = tobj->next_content)
		weight += get_obj_weight(tobj) * WEIGHT_MULT(obj) / 100;

	return weight;
}

int get_true_weight(OBJ_DATA *obj)
{
	int weight;
	weight = obj->weight;

	for (obj = obj->contains; obj != NULL; obj = obj->next_content)
		weight += get_obj_weight(obj);

	return weight;
}

/*
 * True if room is dark.
 */
bool room_is_dark(ROOM_INDEX_DATA *room)
{
	if (room == NULL)
		return TRUE;

	if (room_is_very_dark(room))
		return TRUE;

	if (room->light > 0)
		return FALSE;

	if (IS_SET(GET_ROOM_FLAGS(room), ROOM_DARK))
		return TRUE;

	if (room->sector_type == SECT_INSIDE
	    || room->sector_type == SECT_CITY)
		return FALSE;

	if (weather_info.sunlight == SUN_DARK)
		return TRUE;

	return FALSE;
}

bool room_is_very_dark(ROOM_INDEX_DATA *room)
{
	if (room == NULL)
		return TRUE;

	if (IS_SET(GET_ROOM_FLAGS(room), ROOM_NOLIGHT))
		return TRUE;

	return FALSE;
}

bool is_room_owner(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
	if (room->owner == NULL || room->owner[0] == '\0')
		return FALSE;

	return is_name(ch->name, room->owner);
}

/*
 * True if room is private.
 */
bool room_is_private(ROOM_INDEX_DATA *pRoomIndex)
{
	CHAR_DATA *rch;
	int count;

	if (pRoomIndex->owner != NULL && pRoomIndex->owner[0] != '\0')
		return TRUE;

	count = 0;

	for (rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room)
		count++;

	if (IS_SET(GET_ROOM_FLAGS(pRoomIndex), ROOM_PRIVATE)  && count >= 2)
		return TRUE;

	if (IS_SET(GET_ROOM_FLAGS(pRoomIndex), ROOM_SOLITARY) && count >= 1)
		return TRUE;

	return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex)
{
	if (IS_SET(GET_ROOM_FLAGS(pRoomIndex), ROOM_IMP_ONLY)
	    &&  GET_RANK(ch) < RANK_IMP)
		return FALSE;

	if (IS_IMMORTAL(ch))
		return TRUE;

	/* restrictions below this line do not apply to immortals of any level. */

	if (IS_SET(GET_ROOM_FLAGS(pRoomIndex), ROOM_GODS_ONLY))
		return FALSE;

	if (IS_SET(GET_ROOM_FLAGS(pRoomIndex), ROOM_REMORT_ONLY)
	    && !IS_REMORT(ch))
		return FALSE;

	if (IS_SET(GET_ROOM_FLAGS(pRoomIndex), ROOM_HEROES_ONLY)
	    &&  !IS_HEROIC(ch))
		return FALSE;

	if (IS_SET(GET_ROOM_FLAGS(pRoomIndex), ROOM_NEWBIES_ONLY)
	    &&  ch->level > 5 && !IS_SET(ch->act, PLR_MAKEBAG))
		return FALSE;

	/* can see other clanhall in times of war */
	if (pRoomIndex->clan) {
		/*
		if (ch->clan)
		  {
		          if (clan->opponents(ch->clan, pRoomIndex->clan))
		                  return TRUE;
		  }
		*/

		/* Check if character has a master in the clan -- Outsider */
		/* but only if they're a mob.  a pc following someone in a group has a master too -- Montrey */
		if (IS_NPC(ch) && ch->master) {
			if (ch->master->clan != pRoomIndex->clan)
				return FALSE;
		}
		/* Only check if we have no master -- Outsider */
		else if (ch->clan != pRoomIndex->clan && ch->inviters != pRoomIndex->clan)
			return FALSE;
	}

	if (IS_SET(GET_ROOM_FLAGS(pRoomIndex), ROOM_LEADER_ONLY)
	    && !HAS_CGROUP(ch, GROUP_LEADER))
		return FALSE;

	if (IS_SET(GET_ROOM_FLAGS(pRoomIndex), ROOM_MALE_ONLY) && GET_ATTR_SEX(ch) != SEX_MALE)
		return FALSE;

	if (IS_SET(GET_ROOM_FLAGS(pRoomIndex), ROOM_FEMALE_ONLY) && GET_ATTR_SEX(ch) != SEX_FEMALE)
		return FALSE;

	return TRUE;
}

bool is_blinded(const CHAR_DATA *ch) {
	if (IS_IMMORTAL(ch))
		return FALSE;

	if (affect_exists_on_char(ch, gsn_blindness)
	 || affect_exists_on_char(ch, gsn_dirt_kicking)
	 || affect_exists_on_char(ch, gsn_fire_breath)
	 || affect_exists_on_char(ch, gsn_smokescreen)
	 || affect_exists_on_char(ch, gsn_dazzle))
		return TRUE;

	return FALSE;
}

/*
 * True if char can see victim.
 */
bool can_see_char(const CHAR_DATA *ch, const CHAR_DATA *victim)
{
	if (ch == victim)
		return TRUE;

	if (IS_IMP(ch))
		return TRUE;

	if (!IS_NPC(victim) && IS_SET(victim->act, PLR_SUPERWIZ))
		return FALSE;

	if (IS_IMMORTAL(ch))
		return TRUE;

	if (IS_NPC(victim) && IS_SET(victim->act, ACT_SUPERMOB))
		return FALSE;

	if (affect_exists_on_char(victim, gsn_midnight))
		return FALSE;

	if (!IS_IMMORTAL(ch) && victim->lurk_level && ch->in_room != victim->in_room)
		return FALSE;

	if (victim->invis_level)
		return FALSE;

	if (is_blinded(ch))
		return FALSE;

	if ((room_is_dark(ch->in_room) && !affect_exists_on_char(ch, gsn_night_vision))
	 || room_is_very_dark(ch->in_room))
		return FALSE;

	if (affect_exists_on_char(victim, gsn_invis)
	    &&   !affect_exists_on_char(ch, gsn_detect_invis))
		return FALSE;

	/* sneaking */
	if (affect_exists_on_char(victim, gsn_sneak)
	    &&   !affect_exists_on_char(ch, gsn_detect_hidden)
	    &&   victim->fighting == NULL) {
		int chance;
		chance = get_skill(victim, gsn_sneak);
		chance += GET_ATTR_DEX(ch) * 3 / 2;
		chance -= GET_ATTR_INT(ch) * 2;
		chance += ch->level - victim->level * 3 / 2;

		if (number_percent() < chance)
			return FALSE;
	}

	if (affect_exists_on_char(victim, gsn_hide)
	    &&   !affect_exists_on_char(ch, gsn_detect_hidden)
	    &&   victim->fighting == NULL)
		return FALSE;

	return TRUE;
} /* end can_see_char() */

/*
 * True if char can see victim in WHO -- Elrac
 * This makes almost all characters visible in who, except hiding imms
 */
bool can_see_who(const CHAR_DATA *ch, const CHAR_DATA *victim)
{
	/* wizi still rules */
	if (victim->invis_level && !IS_IMMORTAL(ch))
		return FALSE;

	/* so does SUPERWIZ */
	if (IS_SET(victim->act, PLR_SUPERWIZ) && !IS_NPC(victim) && !IS_IMP(ch))
		return FALSE;

	/* so does LURK */
	if (!IS_IMMORTAL(ch) && victim->lurk_level && ch->in_room != victim->in_room)
		return FALSE;

	/* Otherwise, I guess WHO should see them. */
	return TRUE;
}

/*
 * True if char can see characters and objects inside a room.  Not a permission thing,
 * but for darkness and vision.
 */
bool can_see_in_room(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
	if (IS_IMMORTAL(ch))
		return TRUE;

	if (room_is_very_dark(room))
		return FALSE;

	if (is_blinded(ch))
		return FALSE;

	if (room_is_dark(room) && !affect_exists_on_char(ch, gsn_night_vision))
		return FALSE;

	return TRUE;
}

/*
 * True if char can see obj.
 */
bool can_see_obj(const CHAR_DATA *ch, const OBJ_DATA *obj)
{
	if (IS_IMMORTAL(ch))
		return TRUE;

	if (room_is_very_dark(ch->in_room))
		return FALSE;

	if (is_blinded(ch))
		return FALSE;

	if (IS_OBJ_STAT(obj, ITEM_VIS_DEATH) && obj->carried_by != ch)
		return FALSE;

	if (obj->timer > 0 && !IS_NPC(ch) && is_name("mox", obj->name)
	    && !is_name(ch->name, &obj->name[4]))
		return FALSE;

	if (obj->pIndexData->vnum == OBJ_VNUM_SQUESTOBJ) {
		if (IS_NPC(ch))
			return FALSE;

		if (!IS_SQUESTOR(ch) || ch->pcdata->squestobj == NULL || obj != ch->pcdata->squestobj)
			return FALSE;
	}

	if (IS_OBJ_STAT(obj, ITEM_INVIS)
	    && !affect_exists_on_char(ch, gsn_detect_invis))
		return FALSE;

	if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
		return TRUE;

	if (IS_OBJ_STAT(obj, ITEM_GLOW))
		return TRUE;

	if (room_is_dark(ch->in_room) && !affect_exists_on_char(ch, gsn_night_vision))
		return FALSE;

	return TRUE;
}

/*
 * True if char can drop obj.
 */
bool can_drop_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (!IS_OBJ_STAT(obj, ITEM_NODROP))
		return TRUE;

	if (IS_IMMORTAL(ch))
		return TRUE;

	return FALSE;
}

/* Count characters in a string, ignoring color codes */
/* color_strlen now counts {{s as a character, fixes a few bugs -- Montrey */
int color_strlen(const char *argument)
{
	const char *str;
	int length;

	if (argument == NULL || argument[0] == '\0')
		return 0;

	length = 0;
	str = argument;

	while (*str != '\0') {
		if (*str != '{') {
			str++;
			length++;
			continue;
		}

		if (*(++str) == '{')
			length++;

		str++;
	}

	return length;
}

/* Tell if a given string has a slash in it.
   This is useful for making sure a given name is not a directory name. */
bool has_slash(const char *str)
{
	return (strchr(str, '/') != NULL);
} /* end has_slash() */

/* round to the nearest whole number, in increments */
/*int round(float fNum, int iInc)
{
	int iNum;
	iNum = fNum;

	if ((fNum - iNum) >= (1 / 2))
		iNum++;

	if (iInc > 1) {
		if ((iNum % iInc) >= (iInc / 2))
			iNum += (iNum % iInc);
		else
			iNum -= (iNum % iInc);
	}

	return iNum;
}
*/
/* take a deity string, pull out a valid deity -- Montrey */
int parse_deity(const char *dstring)
{
	int i;

	if (dstring == NULL || dstring[0] == '\0')
		return -1;

	for (i = 0; deity_table[i].name; i++)
		if (strstr(smash_bracket(dstring), deity_table[i].name))
			return i;

	return -1;
}

int get_usable_level(CHAR_DATA *ch)
{
	int level = 0;

	if (IS_IMMORTAL(ch))
		return 1000;

	if (IS_NPC(ch))
		return ch->level;

	if (IS_REMORT(ch))
		level = ch->level + UMIN((ch->pcdata->remort_count / 3) + 6, 16);
	else
		level = ch->level + 5;

	return UMIN(level, LEVEL_HERO);
}

int get_holdable_level(CHAR_DATA *ch)
{
	int level = 0;

	if (IS_IMMORTAL(ch))
		return 1000;

	if (IS_NPC(ch))
		return ch->level;

	if (IS_REMORT(ch))
		level = ch->level + UMIN((ch->pcdata->remort_count / 3) + 9, 19);
	else
		level = ch->level + 8;

	return level;
}

/* get_owner
 *
 * returns the name of the owner of the object.
 * includes detection if player can see the owner.
 *
 * Clerve
 */

const char *get_owner(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (obj->extra_descr != NULL) {
		EXTRA_DESCR_DATA *ed_next;
		EXTRA_DESCR_DATA *ed;

		for (ed = obj->extra_descr; ed != NULL; ed = ed_next) {
			ed_next = ed->next;

			if (!str_cmp(ed->keyword, KEYWD_OWNER)) {
				if (can_see_obj(ch, obj))
					return ed->description;
				else
					return "someone";
			}
		}
	}

	return "(none)";
}

CHAR_DATA *get_obj_carrier(OBJ_DATA *obj)
{
	OBJ_DATA *in_obj;

	for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj)
		;

	return in_obj->carried_by;
}

/* Return what position a character is in.  This replaces all direct
   accessing of position, because POS_FIGHTING is now not an actual
   position but is returned when ch->fighting is not null and their
   actual position is standing.  Later it can be used to simplify
   adding positions, like POS_FLYING (future project).  -- Montrey */
int get_position(CHAR_DATA *ch)
{
	if (ch == NULL)
		return -1;

	if (ch->fighting
	 && ch->position >= POS_STANDING) // includes POS_FLYING
		return POS_FIGHTING;

	return ch->position;
}

/* retrieve a character's playing time in hours */
int get_play_hours(CHAR_DATA *ch)
{
	return (IS_NPC(ch) ? 0 : ch->pcdata->played / 3600);
}

/* retrieve a character's playing time in seconds */
int get_play_seconds(CHAR_DATA *ch)
{
	return (IS_NPC(ch) ? 0 : ch->pcdata->played);
}

// TODO: this doesn't take eq affects into account, but short of looping through those...
/* used with affect_exists_on_char, checks to see if the affect has an evolution rating, returns 1 if not */
int get_affect_evolution(CHAR_DATA *ch, int sn)
{
	int evo = 1;

	for (const AFFECT_DATA *paf = affect_list_char(ch); paf != NULL; paf = paf->next)
		if (paf->type == sn && paf->evolution > evo)            /* returns the evolution of the highest */
			evo = paf->evolution;

	return URANGE(1, evo, 3);
}

long flag_convert(char letter)
{
	long bitsum = 0;
	char i;

	if ('A' <= letter && letter <= 'Z') {
		bitsum = 1;

		for (i = letter; i > 'A'; i--)
			bitsum *= 2;
	}
	else if ('a' <= letter && letter <= 'z') {
		bitsum = 67108864; /* 2^26 */

		for (i = letter; i > 'a'; i--)
			bitsum *= 2;
	}

	return bitsum;
}

/* Simple linear interpolation. */
int interpolate(int level, int value_00, int value_32)
{
	return value_00 + level * (value_32 - value_00) / 32;
}

/* Get an extra description from a list. */
EXTRA_DESCR_DATA *get_extra_descr(const char *name, EXTRA_DESCR_DATA *ed)
{
	for (; ed != NULL; ed = ed->next)
		if (is_name((char *) name, ed->keyword))
			return ed;

	return NULL;
}
