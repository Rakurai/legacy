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
#include "recycle.h"
#include "tables.h"
#include "affect.h"

// TODO: temporary access, remove when possible
extern void affect_modify_char args((void *owner, const AFFECT_DATA *paf, bool fAdd));

/* command procedures needed */
DECLARE_DO_FUN(do_return);

/* Note name match exact */
bool note_is_name(const char *str, const char *namelist)
{
	char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
	const char *list, *string;

	/* fix crash on NULL namelist */
	if (namelist == NULL || namelist[0] == '\0')
		return FALSE;

	/* fixed to prevent is_name on "" returning TRUE */
	if (str[0] == '\0')
		return FALSE;

	string = str;

	/* we need ALL parts of string to match part of namelist */
	for (; ;) {  /* start parsing string */
		str = one_argument(str, part);

		if (part[0] == '\0')
			return TRUE;

		/* check to see if this is part of namelist */
		list = namelist;

		for (; ;) {  /* start parsing namelist */
			list = one_argument(list, name);

			if (name[0] == '\0')  /* this name was not found */
				return FALSE;

			if (!str_prefix1(string, name))
				return TRUE; /* full pattern match */

			if (!str_prefix1(part, name))
				break;
		}
	}
}

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

const char *item_name(int item_type)
{
	int type;

	for (type = 0; item_table[type].name != NULL; type++)
		if (item_type == item_table[type].type)
			return item_table[type].name;

	return "none";
}

const char *weapon_name(int weapon_type)
{
	int type;

	for (type = 0; weapon_table[type].name != NULL; type++)
		if (weapon_type == weapon_table[type].type)
			return weapon_table[type].name;

	return "exotic";
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
int get_skill(CHAR_DATA *ch, int sn)
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

/* Is Exact Name by Lotus */
bool is_exact_name(const char *str, const char *namelist)
{
	char name[MIL], part[MIL];
	const char *list, *string;
	string = str;

	/* we need ALL parts of string to match part of namelist */
	for (; ;) {  /* start parsing string */
		str = one_argument(str, part);

		if (part[0] == '\0')
			return TRUE;

		/* check to see if this is part of namelist */
		list = namelist;

		for (; ;) {  /* start parsing namelist */
			list = one_argument(list, name);

			if (name[0] == '\0')  /* this name was not found */
				return FALSE;

			if (!str_cmp(string, name))
				return TRUE; /* full pattern match */

			if (!str_cmp(part, name))
				break;
		}
	}
}

/*
 * See if a string is one of the names of an object.
 */
bool is_name(const char *str, const char *namelist)
{
	char name[MIL], part[MIL];
	const char *list, *string;
	string = str;

	/* we need ALL parts of string to match part of namelist */
	for (; ;) {  /* start parsing string */
		str = one_argument(str, part);

		if (part[0] == '\0')
			return TRUE;

		/* check to see if this is part of namelist */
		list = namelist;

		for (; ;) {  /* start parsing namelist */
			list = one_argument(list, name);

			if (name[0] == '\0')  /* this name was not found */
				return FALSE;

			if (!str_prefix1(string, name))
				return TRUE; /* full pattern match */

			if (!str_prefix1(part, name))
				break;
		}
	}
}

bool is_exact_name_color(const char *str, const char *namelist)
{
	char name[MIL], part[MIL];
	const char *list, *string;
	/* strip the color codes */
	str = smash_bracket(str);
	namelist = smash_bracket(namelist);
	string = str;

	/* we need ALL parts of string to match part of namelist */
	for (; ;) {  /* start parsing string */
		str = one_argument(str, part);

		if (part[0] == '\0')
			return TRUE;

		/* check to see if this is part of namelist */
		list = namelist;

		for (; ;) {  /* start parsing namelist */
			list = one_argument(list, name);

			if (name[0] == '\0')  /* this name was not found */
				return FALSE;

			if (!str_cmp(string, name))
				return TRUE; /* full pattern match */

			if (!str_cmp(part, name))
				break;
		}
	}
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
	char temp_buffer[256];   /* buffer for debug info */

	if (ch->in_room == NULL) {
		if (ch->name) {
			sprintf(temp_buffer, "extract_char: ch->in_room == NULL for %s", ch->name);
			bug(temp_buffer, 0);
		}
		else
			bug("extract_char: ch->in_room == NULL", 0);

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
		sprintf(name, "coin %s gcash", silver == 1 ? "silver" : "gold");
		sprintf(short_descr, "a %s coin", silver == 1 ? "silver" : "gold");
		sprintf(description, "One %s coin.", silver == 1 ? "miserable silver" : "valuable gold");
		sprintf(material, silver == 1 ? "silver" : "gold");
	}
	else {
		char silver_desc[MSL], gold_desc[MSL];
		/* name */
		sprintf(name, "coins %s%sgcash",
		        silver > 0 ? "silver " : "",
		        gold > 0 ? "gold " : "");

		/* short description */
		if (silver == 1)
			sprintf(silver_desc, "one silver coin");
		else
			sprintf(silver_desc, "%d silver coins", silver);

		if (gold == 1)
			sprintf(gold_desc, "%sone gold coin", silver == 0 ? "" : " and ");
		else
			sprintf(gold_desc, "%s%d gold coins", silver == 0 ? "" : " and ", gold);

		sprintf(short_descr, "%s%s",
		        silver > 0 ? silver_desc : "",
		        gold > 0 ? gold_desc : "");
		/* long description */
		sprintf(description, "%s pile of %scoins.",
		        amount[amt],
		        silver == 0 ? "gold " : gold == 0 ? "silver " : "");
		/* material */
		sprintf(material, "%s%s",
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

bool is_blinded(CHAR_DATA *ch) {
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
bool can_see_char(CHAR_DATA *ch, CHAR_DATA *victim)
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
bool can_see_who(CHAR_DATA *ch, CHAR_DATA *victim)
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
bool can_see_obj(CHAR_DATA *ch, OBJ_DATA *obj)
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

/*
 * Return ascii name of an item type.
 */
const char *item_type_name(OBJ_DATA *obj)
{
	switch (obj->item_type) {
	case ITEM_LIGHT:            return "light";

	case ITEM_SCROLL:           return "scroll";

	case ITEM_WAND:             return "wand";

	case ITEM_STAFF:            return "staff";

	case ITEM_WEAPON:           return "weapon";

	case ITEM_TREASURE:         return "treasure";

	case ITEM_ARMOR:            return "armor";

	case ITEM_CLOTHING:         return "clothing";

	case ITEM_POTION:           return "potion";

	case ITEM_FURNITURE:        return "furniture";

	case ITEM_TRASH:            return "trash";

	case ITEM_CONTAINER:        return "container";

	case ITEM_DRINK_CON:        return "drink container";

	case ITEM_KEY:              return "key";

	case ITEM_FOOD:             return "food";

	case ITEM_MONEY:            return "money";

	case ITEM_BOAT:             return "boat";

	case ITEM_CORPSE_NPC:       return "npc corpse";

	case ITEM_CORPSE_PC:        return "pc corpse";

	case ITEM_FOUNTAIN:         return "fountain";

	case ITEM_PILL:             return "pill";

	case ITEM_MAP:              return "map";

	case ITEM_PORTAL:           return "portal";

	case ITEM_WARP_STONE:       return "warp stone";

	case ITEM_GEM:              return "gem";

	case ITEM_JEWELRY:          return "jewelry";

	case ITEM_JUKEBOX:          return "juke box";

	case ITEM_MATERIAL:         return "material";

	case ITEM_ANVIL:            return "anvil";

//	case ITEM_COACH:            return "coach";

	case ITEM_WEDDINGRING:      return "weddingring";

	case ITEM_TOKEN:            return "token";
	}

	bug("Item_type_name: unknown type %d.", obj->item_type);
	return "(unknown)";
}

/*
 * Return ascii name of a damage type.
 */
const char *dam_type_name(int type)
{
	switch (type) {
		case DAM_NONE        : return "(none)";
		case DAM_CHARM       : return "charm";
		case DAM_BASH        : return "bash";
		case DAM_PIERCE      : return "pierce";
		case DAM_SLASH       : return "slash";
		case DAM_FIRE        : return "fire";
		case DAM_COLD        : return "cold";
		case DAM_ELECTRICITY : return "electricity";
		case DAM_ACID        : return "acid";
		case DAM_POISON      : return "poison";
		case DAM_NEGATIVE    : return "negative";
		case DAM_HOLY        : return "holy";
		case DAM_ENERGY      : return "energy";
		case DAM_MENTAL      : return "mental";
		case DAM_DISEASE     : return "disease";
		case DAM_DROWNING    : return "drowning";
		case DAM_LIGHT       : return "light";
		case DAM_SOUND       : return "sound";
	}

	bug("dam_type_name: unknown type %d.", type);
	return "(unknown)";
}


/*
 * Return ascii name of an affect location.
 */
const char *affect_loc_name(int location)
{
	switch (location) {
	case APPLY_NONE:            return "none";
	case APPLY_STR:             return "strength";
	case APPLY_DEX:             return "dexterity";
	case APPLY_INT:             return "intelligence";
	case APPLY_WIS:             return "wisdom";
	case APPLY_CON:             return "constitution";
	case APPLY_CHR:             return "charisma";
	case APPLY_SEX:             return "sex";
	case APPLY_AGE:             return "age";
	case APPLY_MANA:            return "mana";
	case APPLY_HIT:             return "hp";
	case APPLY_STAM:            return "stamina";
	case APPLY_GOLD:            return "gold";
	case APPLY_EXP:             return "experience";
	case APPLY_AC:              return "armor class";
	case APPLY_HITROLL:         return "hit roll";
	case APPLY_DAMROLL:         return "damage roll";
	case APPLY_SAVES:           return "saves";
	case APPLY_SAVING_ROD:      return "save vs rod";
	case APPLY_SAVING_PETRI:    return "save vs petrification";
	case APPLY_SAVING_BREATH:   return "save vs breath";
	case APPLY_SAVING_SPELL:    return "save vs spell";
	}

	bug("Affect_location_name: unknown location %d.", location);
	return "(unknown)";
}

/*
 * Return ascii name of extra flags vector.
 */
const char *extra_bit_name(int extra_flags)
{
	static char buf[512];
	buf[0] = '\0';

	if (extra_flags & ITEM_GLOW) strcat(buf, " glow");

	if (extra_flags & ITEM_HUM) strcat(buf, " hum");

	if (extra_flags & ITEM_COMPARTMENT) strcat(buf, " compartment");

	if (extra_flags & ITEM_TRANSPARENT) strcat(buf, " transparent");

	if (extra_flags & ITEM_EVIL) strcat(buf, " evil");

	if (extra_flags & ITEM_INVIS) strcat(buf, " invis");

	if (extra_flags & ITEM_MAGIC) strcat(buf, " magic");

	if (extra_flags & ITEM_NODROP) strcat(buf, " nodrop");

	if (extra_flags & ITEM_BLESS) strcat(buf, " bless");

	if (extra_flags & ITEM_ANTI_GOOD) strcat(buf, " anti-good");

	if (extra_flags & ITEM_ANTI_EVIL) strcat(buf, " anti-evil");

	if (extra_flags & ITEM_ANTI_NEUTRAL) strcat(buf, " anti-neutral");

	if (extra_flags & ITEM_NOREMOVE) strcat(buf, " noremove");

	if (extra_flags & ITEM_INVENTORY) strcat(buf, " inventory");

	if (extra_flags & ITEM_NOPURGE) strcat(buf, " nopurge");

	if (extra_flags & ITEM_VIS_DEATH) strcat(buf, " vis_death");

	if (extra_flags & ITEM_ROT_DEATH) strcat(buf, " rot_death");

	if (extra_flags & ITEM_NOLOCATE) strcat(buf, " no_locate");

	if (extra_flags & ITEM_SELL_EXTRACT) strcat(buf, " sell_extract");

	if (extra_flags & ITEM_BURN_PROOF) strcat(buf, " burn_proof");

	if (extra_flags & ITEM_NOSAC) strcat(buf, " no_sac");

	if (extra_flags & ITEM_NONMETAL) strcat(buf, " nonmetal");

	if (extra_flags & ITEM_MELT_DROP) strcat(buf, " melt_drop");

	if (extra_flags & ITEM_NOUNCURSE) strcat(buf, " no_uncurse");

	if (extra_flags & ITEM_QUESTSELL) strcat(buf, " quest_item");

	return (buf[0] != '\0') ? buf + 1 : "none";
}

/* return ascii name of an act vector */
const char *act_bit_name(int act_flags, bool npc)
{
	static char buf[512];
	buf[0] = '\0';

	if (npc) {
		strcat(buf, " npc");
		if (act_flags & ACT_NOSUMMON) strcat(buf, " no_summon");

		if (act_flags & ACT_SENTINEL) strcat(buf, " sentinel");

		if (act_flags & ACT_SCAVENGER) strcat(buf, " scavenger");

		if (act_flags & ACT_AGGRESSIVE) strcat(buf, " aggressive");

		if (act_flags & ACT_STAY_AREA) strcat(buf, " stay_area");

		if (act_flags & ACT_WIMPY) strcat(buf, " wimpy");

		if (act_flags & ACT_MORPH) strcat(buf, " morphed");

		if (act_flags & ACT_NOMORPH) strcat(buf, " nomorph");

		if (act_flags & ACT_STAY) strcat(buf, " stay");

		if (act_flags & ACT_OUTDOORS) strcat(buf, " outdoors");

		if (act_flags & ACT_INDOORS) strcat(buf, " indoors");

		if (act_flags & ACT_PET) strcat(buf, " pet");

		if (act_flags & ACT_TRAIN) strcat(buf, " train");

		if (act_flags & ACT_PRACTICE) strcat(buf, " practice");

		if (act_flags & ACT_UNDEAD) strcat(buf, " undead");

		if (act_flags & ACT_CLERIC) strcat(buf, " cleric");

		if (act_flags & ACT_MAGE) strcat(buf, " mage");

		if (act_flags & ACT_THIEF) strcat(buf, " thief");

		if (act_flags & ACT_WARRIOR) strcat(buf, " warrior");

		if (act_flags & ACT_NOALIGN) strcat(buf, " no_align");

		if (act_flags & ACT_NOPURGE) strcat(buf, " no_purge");

		if (act_flags & ACT_IS_HEALER) strcat(buf, " healer");

		if (act_flags & ACT_IS_CHANGER) strcat(buf, " changer");

		if (act_flags & ACT_GAIN) strcat(buf, " skill_train");

		if (act_flags & ACT_NOPUSH) strcat(buf, " nopush");
	}
	else {
		strcat(buf, " player");
		if (act_flags & PLR_NOSUMMON) strcat(buf, " no_summon");

		if (act_flags & PLR_AUTOASSIST) strcat(buf, " autoassist");

		if (act_flags & PLR_AUTOEXIT) strcat(buf, " autoexit");

		if (act_flags & PLR_AUTOLOOT) strcat(buf, " autoloot");

		if (act_flags & PLR_AUTOSAC) strcat(buf, " autosac");

		if (act_flags & PLR_AUTOGOLD) strcat(buf, " autogold");

		if (act_flags & PLR_AUTOSPLIT) strcat(buf, " autosplit");

		if (act_flags & PLR_TICKS) strcat(buf, " autotick");

		if (act_flags & PLR_CANLOOT) strcat(buf, " loot_corpse");

		if (act_flags & PLR_NOSUMMON) strcat(buf, " no_summon");

		if (act_flags & PLR_NOFOLLOW) strcat(buf, " no_follow");

		if (act_flags & PLR_FREEZE) strcat(buf, " frozen");

		if (act_flags & PLR_THIEF) strcat(buf, " thief");

		if (act_flags & PLR_KILLER) strcat(buf, " killer");

		if (act_flags & PLR_COLOR) strcat(buf, " color");

		if (act_flags & PLR_LOOKINPIT) strcat(buf, " lookinpit");

		if (act_flags & PLR_DEFENSIVE) strcat(buf, " showdef");

		if (act_flags & PLR_WIMPY) strcat(buf, " autorecall");

		if (act_flags & PLR_QUESTOR) strcat(buf, " questor");

		if (act_flags & PLR_COLOR2) strcat(buf, " crazycolor");

		if (act_flags & PLR_MAKEBAG) strcat(buf, " newbiehelp");

		if (act_flags & PLR_PERMIT) strcat(buf, " permit");

		if (act_flags & PLR_CLOSED) strcat(buf, " locker_closed");

		if (act_flags & PLR_LOG) strcat(buf, " logged");

		if (act_flags & PLR_NOPK) strcat(buf, " nopk");
	}

	return (buf[0] != '\0') ? buf + 1 : "none";
}

const char *comm_bit_name(int comm_flags)
{
	static char buf[512];
	buf[0] = '\0';

	if (comm_flags & COMM_QUIET) strcat(buf, " quiet");

	if (comm_flags & COMM_DEAF) strcat(buf, " deaf");

	if (comm_flags & COMM_NOWIZ) strcat(buf, " no_wiz");

	if (comm_flags & COMM_NOPRAY) strcat(buf, " no_pray");

	if (comm_flags & COMM_NOAUCTION) strcat(buf, " no_auction");

	if (comm_flags & COMM_NOGOSSIP) strcat(buf, " no_gossip");

	if (comm_flags & COMM_NOFLAME) strcat(buf, " no_flame");

	if (comm_flags & COMM_NOQUESTION) strcat(buf, " no_question");

	if (comm_flags & COMM_NOMUSIC) strcat(buf, " no_music");

	if (comm_flags & COMM_NOIC) strcat(buf, " no_ic");

	if (comm_flags & COMM_COMPACT) strcat(buf, " compact");

	if (comm_flags & COMM_BRIEF) strcat(buf, " brief");

	if (comm_flags & COMM_PROMPT) strcat(buf, " prompt");

	if (comm_flags & COMM_COMBINE) strcat(buf, " combine");

	if (comm_flags & COMM_NOCLAN) strcat(buf, " no_clantalk");

	if (comm_flags & COMM_SHOW_AFFECTS) strcat(buf, " show_affects");

	if (comm_flags & COMM_NOGRATS) strcat(buf, " no_grats");

	if (comm_flags & COMM_NOQUERY) strcat(buf, " no_query");

	if (comm_flags & COMM_NOPAGE) strcat(buf, " no_page");

	if (comm_flags & COMM_NOANNOUNCE) strcat(buf, " no_announce");

	if (comm_flags & COMM_NOSOCIAL) strcat(buf, " no_social");

	if (comm_flags & COMM_NOQWEST) strcat(buf, " no_qwest");

	if (comm_flags & COMM_AFK) strcat(buf, " afk");

	return (buf[0] != '\0') ? buf + 1 : "none";
}

/* nochannel flags - by Xenith */
const char *revoke_bit_name(int revoke_flags)
{
	static char buf[512];
	buf[0] = '\0';

	if (revoke_flags & REVOKE_NOCHANNELS) strcat(buf, " no_channels");

	if (revoke_flags & REVOKE_FLAMEONLY) strcat(buf, " flameonly");

	if (revoke_flags & REVOKE_GOSSIP) strcat(buf, " revoke_gossip");

	if (revoke_flags & REVOKE_FLAME) strcat(buf, " revoke_flame");

	if (revoke_flags & REVOKE_QWEST) strcat(buf, " revoke_qwest");

	if (revoke_flags & REVOKE_PRAY) strcat(buf, " revoke_pray");

	if (revoke_flags & REVOKE_AUCTION) strcat(buf, " revoke_auction");

	if (revoke_flags & REVOKE_CLAN) strcat(buf, " revoke_clan");

	if (revoke_flags & REVOKE_MUSIC) strcat(buf, " revoke_music");

	if (revoke_flags & REVOKE_QA) strcat(buf, " revoke_qa");

	if (revoke_flags & REVOKE_SOCIAL) strcat(buf, " revoke_social");

	if (revoke_flags & REVOKE_IC) strcat(buf, " revoke_ic");

	if (revoke_flags & REVOKE_GRATS) strcat(buf, " revoke_grats");

	if (revoke_flags & REVOKE_PAGE) strcat(buf, " revoke_page");

	if (revoke_flags & REVOKE_QTELL) strcat(buf, " revoke_qtell");

	if (revoke_flags & REVOKE_EMOTE) strcat(buf, " revoke_emote");

	if (revoke_flags & REVOKE_TELL) strcat(buf, " revoke_tell");

	if (revoke_flags & REVOKE_SAY) strcat(buf, " revoke_say");

	if (revoke_flags & REVOKE_EXP) strcat(buf, " revoke_exp");

	return (buf[0] != '\0') ? buf + 1 : "none";
}

/* Command groups - Xenith */
const char *cgroup_bit_name(int flags)
{
	char buf2[MSL];
	static char buf[512];
	int i = 0;
	buf[0] = '\0';

	while (cgroup_flags[i].name != NULL) {
		if (flags & cgroup_flags[i].bit) {
			sprintf(buf2, " %s", cgroup_flags[i].name);
			strcat(buf, buf2);
		}

		i++;
	}

	return (buf[0] != '\0') ? buf + 1 : "none";
}

const char *censor_bit_name(int censor_flags)
{
	static char buf[512];
	buf[0] = '\0';

	if (censor_flags & CENSOR_CHAN)         strcat(buf, " censor_channels");

	if (censor_flags & CENSOR_SPAM)         strcat(buf, " censor_spam");

	return (buf[0] != '\0') ? buf + 1 : "none";
}

/* return ascii name of a plr vector */
const char *plr_bit_name(int plr_flags)
{
	static char buf[512];
	buf[0] = '\0';

	if (plr_flags & PLR_OOC) strcat(buf, " ooc");

	if (plr_flags & PLR_CHATMODE) strcat(buf, " chatmode");

	if (plr_flags & PLR_PRIVATE) strcat(buf, " private");

	if (plr_flags & PLR_STOPCRASH) strcat(buf, " stopcrash");

	if (plr_flags & PLR_PK) strcat(buf, " pk");

	if (plr_flags & PLR_SHOWEMAIL) strcat(buf, " showemail");

	if (plr_flags & PLR_LINK_DEAD) strcat(buf, " linkdead");

	if (plr_flags & PLR_PAINT) strcat(buf, " paintbow");

	if (plr_flags & PLR_SNOOP_PROOF) strcat(buf, " snoopproof");

	if (plr_flags & PLR_NOSHOWLAST) strcat(buf, " noshowlast");

	if (plr_flags & PLR_AUTOPEEK) strcat(buf, " autopeek");

	if (plr_flags & PLR_HEEDNAME) strcat(buf, " heedname");

	if (plr_flags & PLR_SHOWLOST) strcat(buf, " showlost");

	if (plr_flags & PLR_SHOWRAFF) strcat(buf, " showraff");

	if (plr_flags & PLR_MARRIED) strcat(buf, " married");

	if (plr_flags & PLR_SQUESTOR) strcat(buf, " squestor");

	return (buf[0] != '\0') ? buf + 1 : "none";
}

/* return ascii name of a room vector */
const char *room_bit_name(int flags)
{
	static char buf[512];
	buf[0] = '\0';

	if (flags & ROOM_DARK) strcat(buf, " dark");

	if (flags & ROOM_NOLIGHT) strcat(buf, " nolight");

	if (flags & ROOM_NO_MOB) strcat(buf, " nomob");

	if (flags & ROOM_INDOORS) strcat(buf, " indoors");

	if (flags & ROOM_LOCKER) strcat(buf, " locker");

	if (flags & ROOM_FEMALE_ONLY) strcat(buf, " female");

	if (flags & ROOM_MALE_ONLY) strcat(buf, " male");

	if (flags & ROOM_NOSLEEP) strcat(buf, " nosleep");

	if (flags & ROOM_NOVISION) strcat(buf, " novision");

	if (flags & ROOM_PRIVATE) strcat(buf, " private");

	if (flags & ROOM_SAFE) strcat(buf, " safe");

	if (flags & ROOM_SOLITARY) strcat(buf, " solitary");

	if (flags & ROOM_PET_SHOP) strcat(buf, " petshop");

	if (flags & ROOM_NO_RECALL) strcat(buf, " norecall");

	if (flags & ROOM_IMP_ONLY) strcat(buf, " imp");

	if (flags & ROOM_GODS_ONLY) strcat(buf, " gods");

	if (flags & ROOM_HEROES_ONLY) strcat(buf, " hero");

	if (flags & ROOM_NEWBIES_ONLY) strcat(buf, " newbie");

	if (flags & ROOM_LAW) strcat(buf, " law");

	if (flags & ROOM_NOWHERE) strcat(buf, " nowhere");

	if (flags & ROOM_BANK) strcat(buf, " bank");

	if (flags & ROOM_LEADER_ONLY) strcat(buf, " leader");

	if (flags & ROOM_TELEPORT) strcat(buf, " teleport");

	if (flags & ROOM_UNDER_WATER) strcat(buf, " underwater");

	if (flags & ROOM_NOPORTAL) strcat(buf, " noportal");

	if (flags & ROOM_REMORT_ONLY) strcat(buf, " remort");

	if (flags & ROOM_NOQUEST) strcat(buf, " noquest");

	if (flags & ROOM_SILENT) strcat(buf, " silent");

	return (buf[0] != '\0') ? buf + 1 : "none";
}

const char *wear_bit_name(int wear_flags)
{
	static char buf[512];
	buf [0] = '\0';

	if (wear_flags & ITEM_TAKE) strcat(buf, " take");

	if (wear_flags & ITEM_WEAR_FINGER) strcat(buf, " finger");

	if (wear_flags & ITEM_WEAR_NECK) strcat(buf, " neck");

	if (wear_flags & ITEM_WEAR_BODY) strcat(buf, " torso");

	if (wear_flags & ITEM_WEAR_HEAD) strcat(buf, " head");

	if (wear_flags & ITEM_WEAR_LEGS) strcat(buf, " legs");

	if (wear_flags & ITEM_WEAR_FEET) strcat(buf, " feet");

	if (wear_flags & ITEM_WEAR_HANDS) strcat(buf, " hands");

	if (wear_flags & ITEM_WEAR_ARMS) strcat(buf, " arms");

	if (wear_flags & ITEM_WEAR_SHIELD) strcat(buf, " shield");

	if (wear_flags & ITEM_WEAR_ABOUT) strcat(buf, " body");

	if (wear_flags & ITEM_WEAR_WAIST) strcat(buf, " waist");

	if (wear_flags & ITEM_WEAR_WRIST) strcat(buf, " wrist");

	if (wear_flags & ITEM_WIELD) strcat(buf, " wield");

	if (wear_flags & ITEM_HOLD) strcat(buf, " hold");

	if (wear_flags & ITEM_WEAR_FLOAT) strcat(buf, " float");

	if (wear_flags & ITEM_WEAR_WEDDINGRING) strcat(buf, " weddingring");

	return (buf[0] != '\0') ? buf + 1 : "none";
}

const char *form_bit_name(int form_flags)
{
	static char buf[512];
	buf[0] = '\0';

	if (form_flags & FORM_POISON) strcat(buf, " poison");
	else if (form_flags & FORM_EDIBLE) strcat(buf, " edible");

	if (form_flags & FORM_MAGICAL) strcat(buf, " magical");

	if (form_flags & FORM_INSTANT_DECAY) strcat(buf, " instant_rot");

	if (form_flags & FORM_OTHER) strcat(buf, " other");

	if (form_flags & FORM_ANIMAL) strcat(buf, " animal");

	if (form_flags & FORM_SENTIENT) strcat(buf, " sentient");

	if (form_flags & FORM_UNDEAD) strcat(buf, " undead");

	if (form_flags & FORM_CONSTRUCT) strcat(buf, " construct");

	if (form_flags & FORM_MIST) strcat(buf, " mist");

	if (form_flags & FORM_INTANGIBLE) strcat(buf, " intangible");

	if (form_flags & FORM_BIPED) strcat(buf, " biped");

	if (form_flags & FORM_CENTAUR) strcat(buf, " centaur");

	if (form_flags & FORM_INSECT) strcat(buf, " insect");

	if (form_flags & FORM_SPIDER) strcat(buf, " spider");

	if (form_flags & FORM_CRUSTACEAN) strcat(buf, " crustacean");

	if (form_flags & FORM_WORM) strcat(buf, " worm");

	if (form_flags & FORM_BLOB) strcat(buf, " blob");

	if (form_flags & FORM_MAMMAL) strcat(buf, " mammal");

	if (form_flags & FORM_BIRD) strcat(buf, " bird");

	if (form_flags & FORM_REPTILE) strcat(buf, " reptile");

	if (form_flags & FORM_SNAKE) strcat(buf, " snake");

	if (form_flags & FORM_DRAGON) strcat(buf, " dragon");

	if (form_flags & FORM_AMPHIBIAN) strcat(buf, " amphibian");

	if (form_flags & FORM_FISH) strcat(buf, " fish");

	if (form_flags & FORM_COLD_BLOOD) strcat(buf, " cold_blooded");

	return (buf[0] != '\0') ? buf + 1 : "none";
}

const char *part_bit_name(int part_flags)
{
	static char buf[512];
	buf[0] = '\0';

	if (part_flags & PART_HEAD) strcat(buf, " head");

	if (part_flags & PART_ARMS) strcat(buf, " arms");

	if (part_flags & PART_LEGS) strcat(buf, " legs");

	if (part_flags & PART_HEART) strcat(buf, " heart");

	if (part_flags & PART_BRAINS) strcat(buf, " brains");

	if (part_flags & PART_GUTS) strcat(buf, " guts");

	if (part_flags & PART_HANDS) strcat(buf, " hands");

	if (part_flags & PART_FEET) strcat(buf, " feet");

	if (part_flags & PART_FINGERS) strcat(buf, " fingers");

	if (part_flags & PART_EAR) strcat(buf, " ears");

	if (part_flags & PART_EYE) strcat(buf, " eyes");

	if (part_flags & PART_LONG_TONGUE) strcat(buf, " long_tongue");

	if (part_flags & PART_EYESTALKS) strcat(buf, " eyestalks");

	if (part_flags & PART_TENTACLES) strcat(buf, " tentacles");

	if (part_flags & PART_FINS) strcat(buf, " fins");

	if (part_flags & PART_WINGS) strcat(buf, " wings");

	if (part_flags & PART_TAIL) strcat(buf, " tail");

	if (part_flags & PART_CLAWS) strcat(buf, " claws");

	if (part_flags & PART_FANGS) strcat(buf, " fangs");

	if (part_flags & PART_HORNS) strcat(buf, " horns");

	if (part_flags & PART_SCALES) strcat(buf, " scales");

	return (buf[0] != '\0') ? buf + 1 : "none";
}
const char *wiz_bit_name(int wiz_flags)
{
	static char buf[512];
	char buf2[MAX_INPUT_LENGTH];
	int flag;
	buf[0] = '\0';

	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
		if (wiz_flags & wiznet_table[flag].flag) {
			sprintf(buf2, " %s", wiznet_table[flag].name);
			strcat(buf, buf2);
		}

	return (buf[0] != '\0') ? buf + 1 : "none";
}

const char *weapon_bit_name(int weapon_flags)
{
	static char buf[512];
	buf[0] = '\0';

	if (weapon_flags & WEAPON_FLAMING) strcat(buf, " flaming");

	if (weapon_flags & WEAPON_FROST) strcat(buf, " frost");

	if (weapon_flags & WEAPON_VAMPIRIC) strcat(buf, " vampiric");

	if (weapon_flags & WEAPON_SHARP) strcat(buf, " sharp");

	if (weapon_flags & WEAPON_VORPAL) strcat(buf, " vorpal");

	if (weapon_flags & WEAPON_TWO_HANDS) strcat(buf, " two-handed");

	if (weapon_flags & WEAPON_SHOCKING) strcat(buf, " shocking");

	if (weapon_flags & WEAPON_POISON) strcat(buf, " poison");

	return (buf[0] != '\0') ? buf + 1 : "none";
}

const char *cont_bit_name(int cont_flags)
{
	static char buf[512];
	buf[0] = '\0';

	if (cont_flags & CONT_CLOSEABLE) strcat(buf, " closable");

	if (cont_flags & CONT_PICKPROOF) strcat(buf, " pickproof");

	if (cont_flags & CONT_CLOSED) strcat(buf, " closed");

	if (cont_flags & CONT_LOCKED) strcat(buf, " locked");

	return (buf[0] != '\0') ? buf + 1 : "none";
}

const char *off_bit_name(int off_flags)
{
	static char buf[512];
	buf[0] = '\0';

	if (off_flags & OFF_AREA_ATTACK) strcat(buf, " area attack");

	if (off_flags & OFF_BACKSTAB) strcat(buf, " backstab");

	if (off_flags & OFF_BASH) strcat(buf, " bash");

	if (off_flags & OFF_BERSERK) strcat(buf, " berserk");

	if (off_flags & OFF_DISARM) strcat(buf, " disarm");

	if (off_flags & OFF_DODGE) strcat(buf, " dodge");

	if (off_flags & OFF_FAST) strcat(buf, " fast");

	if (off_flags & OFF_KICK) strcat(buf, " kick");

	if (off_flags & OFF_KICK_DIRT) strcat(buf, " kick_dirt");

	if (off_flags & OFF_PARRY) strcat(buf, " parry");

	if (off_flags & OFF_RESCUE) strcat(buf, " rescue");

	if (off_flags & OFF_TRIP) strcat(buf, " trip");

	if (off_flags & OFF_CRUSH) strcat(buf, " crush");

	if (off_flags & ASSIST_ALL) strcat(buf, " assist_all");

	if (off_flags & ASSIST_ALIGN) strcat(buf, " assist_align");

	if (off_flags & ASSIST_RACE) strcat(buf, " assist_race");

	if (off_flags & ASSIST_PLAYERS) strcat(buf, " assist_players");

	if (off_flags & ASSIST_GUARD) strcat(buf, " assist_guard");

	if (off_flags & ASSIST_VNUM) strcat(buf, " assist_vnum");

	return (buf[0] != '\0') ? buf + 1 : "none";
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

void ptc(CHAR_DATA *ch, const char *fmt, ...)
{
	char buf [MAX_STRING_LENGTH];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, MSL, fmt, args);
	va_end(args);
	stc(buf, ch);
}

/* print stuff, append to buffer. safe. */
int ptb(BUFFER *buffer, const char *fmt, ...)
{
	char buf[MSL];
	va_list args;
	int res;
	va_start(args, fmt);
	res = vsnprintf(buf, MSL, fmt, args);
	va_end(args);

	if (res >= MSL - 1) {
		buf[0] = '\0';
		bug("print_to_buffer: overflow to buffer, aborting", 0);
	}
	else
		add_buf(buffer, buf);

	return res;
}

void bugf(const char *fmt, ...)
{
	char buf [MAX_STRING_LENGTH];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, MSL, fmt, args);
	va_end(args);
	bug(buf, 0);
}
/*
void logf(char *fmt, ...)
{
	char buf [2 * MSL];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 2 * MSL, fmt, args);
	va_end(args);
	log_string(buf);
}

*/
/* Tell if a given string has a slash in it.
   This is useful for making sure a given name is not a directory name. */
bool has_slash(const char *str)
{
	return (strchr(str, '/') != NULL);
} /* end has_slash() */

/* Validate a character by searching for it in the character list.
   This is CPU intensive but the only way I can think of to protect
   ACT from bad sequences -- Elrac */
bool valid_character(CHAR_DATA *cd)
{
	CHAR_DATA *p;

	for (p = char_list; p != NULL; p = p->next) {
		if (p == cd) return TRUE;
	}

	return FALSE;
} /* end valid_character() */

/* Validate an object by searching for it in the object list. */
bool valid_object(OBJ_DATA *od)
{
	OBJ_DATA *p;

	for (p = object_list; p != NULL; p = p->next) {
		if (p == od) return TRUE;
	}

	return FALSE;
} /* end valid_object() */

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

const char *get_color_name(int color, int bold)
{
	int i;

	for (i = 0; color_table[i].name != NULL; i++)
		if (color_table[i].color == color
		    && color_table[i].bold  == bold)
			return color_table[i].name;

	return NULL;
}

const char *get_color_code(int color, int bold)
{
	int i;

	for (i = 0; color_table[i].name != NULL; i++)
		if (color_table[i].color == color
		    && color_table[i].bold  == bold)
			return color_table[i].code;

	return NULL;
}

const char *get_custom_color_name(CHAR_DATA *ch, int slot)
{
	int color, bold;

	if (!IS_NPC(ch) && ch->pcdata->color[slot] != 0) {
		color = ch->pcdata->color[slot];
		bold  = ch->pcdata->bold [slot];
	}
	else {
		color = csetting_table[slot].color;
		bold  = csetting_table[slot].bold;
	}

	return get_color_name(color, bold);
}

const char *get_custom_color_code(CHAR_DATA *ch, int slot)
{
	int color, bold;

	if (!IS_NPC(ch) && ch->pcdata->color[slot] != 0) {
		color = ch->pcdata->color[slot];
		bold  = ch->pcdata->bold [slot];
	}
	else {
		color = csetting_table[slot].color;
		bold  = csetting_table[slot].bold;
	}

	return get_color_code(color, bold);
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
const char *get_extra_descr(const char *name, EXTRA_DESCR_DATA *ed)
{
	for (; ed != NULL; ed = ed->next)
		if (is_name((char *) name, ed->keyword))
			return ed->description;

	return NULL;
}
