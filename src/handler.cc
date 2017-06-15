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

#include "Area.hh"
#include "merc.hh"
#include "interp.hh"
#include "magic.hh"
#include "recycle.hh"
#include "tables.hh"
#include "lookup.hh"
#include "Affect.hh"

// TODO: temporary access, remove when possible
extern void affect_modify_char args((void *owner, const Affect *paf, bool fAdd));

/* friend stuff -- for NPC's mostly */
bool is_friend(Character *ch, Character *victim)
{
	if (is_same_group(ch, victim))
		return TRUE;

	if (!IS_NPC(ch))
		return FALSE;

	if (!IS_NPC(victim)) {
		if (ch->off_flags.has(ASSIST_PLAYERS))
			return TRUE;
		else
			return FALSE;
	}

	if (affect_exists_on_char(ch, gsn_charm_person))
		return FALSE;

	if (ch->off_flags.has(ASSIST_ALL))
		return TRUE;

	if (ch->group_flags.has_any_of(victim->group_flags))
		return TRUE;

	if (ch->off_flags.has(ASSIST_VNUM)
	    &&  ch->pIndexData == victim->pIndexData)
		return TRUE;

	if (ch->off_flags.has(ASSIST_RACE) && ch->race == victim->race)
		return TRUE;

	if (ch->off_flags.has(ASSIST_ALIGN)
	    &&  !ch->act_flags.has(ACT_NOALIGN) && !victim->act_flags.has(ACT_NOALIGN)
	    && ((IS_GOOD(ch) && IS_GOOD(victim))
	        || (IS_EVIL(ch) && IS_EVIL(victim))
	        || (IS_NEUTRAL(ch) && IS_NEUTRAL(victim))))
		return TRUE;

	return FALSE;
}

/* returns number of people on an object */
int count_users(Object *obj)
{
	Character *fch;
	int count = 0;

	if (obj->in_room == nullptr)
		return 0;

	for (fch = obj->in_room->people; fch != nullptr; fch = fch->next_in_room)
		if (fch->on == obj)
			count++;

	return count;
}

bool is_clan(Character *ch)
{
	return (ch->clan != nullptr);
}

bool is_same_clan(Character *ch, Character *victim)
{
	if (ch == nullptr || victim == nullptr || ch->clan == nullptr || victim->clan == nullptr)
		return FALSE;

	if (ch->clan->independent && !IS_IMMORTAL(ch))
		return FALSE;
	else
		return (ch->clan == victim->clan);
}

/* for returning skill information */
int get_skill(const Character *ch, int sn)
{
	int skill = 0;

	if (sn == -1) /* shorthand for level based skills */
		skill = ch->level * 5 / 2;
	else if (sn < -1 || sn > skill_table.size()) {
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
		if ((sn == gsn_dodge && ch->off_flags.has(OFF_DODGE))
		    || (sn == gsn_parry && ch->off_flags.has(OFF_PARRY))
		    || (sn == gsn_kick && ch->off_flags.has(OFF_KICK)))
			skill = 10 + ch->level;
		else if (sn == gsn_shield_block)
			skill = 15 + ch->level;
		else if (sn == gsn_second_attack
		         && (ch->act_flags.has(ACT_WARRIOR) || ch->act_flags.has(ACT_THIEF)))
			skill = 25 + ch->level;
		else if (sn == gsn_third_attack && ch->act_flags.has(ACT_WARRIOR))
			skill = 15 + ch->level;
		else if (sn == gsn_fourth_attack && ch->act_flags.has(ACT_WARRIOR))
			skill = 2 * (ch->level - 60);
		else if (sn == gsn_hand_to_hand)
			skill = ch->level * 3 / 2;
		else if ((sn == gsn_trip && ch->off_flags.has(OFF_TRIP))
		         || (sn == gsn_dirt_kicking && ch->off_flags.has(OFF_KICK_DIRT)))
			skill = 10 + (ch->level * 3 / 2);
		else if (sn == gsn_bash && ch->off_flags.has(OFF_BASH))
			skill = 10 + (ch->level * 5 / 4);
		else if (sn == gsn_crush && ch->off_flags.has(OFF_CRUSH))
			skill = ch->level;
		else if (sn == gsn_disarm
		         && (ch->off_flags.has(OFF_DISARM)
		             ||       ch->act_flags.has(ACT_WARRIOR)
		             ||       ch->act_flags.has(ACT_THIEF)))
			skill = 20 + (ch->level * 2 / 3);
		else if (sn == gsn_berserk && ch->off_flags.has(OFF_BERSERK))
			skill = 3 * ch->level;
		else if (sn == gsn_backstab && ch->act_flags.has(ACT_THIEF))
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
int deity_lookup(const String& name)
{
	int deity;

	for (deity = 0; deity < deity_table.size(); deity++) {
		if (name.is_prefix_of(deity_table[deity].name))
			return deity;
	}

	return -1;
}

/* for returning weapon information */
int get_weapon_sn(Character *ch, bool secondary)
{
	Object *wield;
	int sn;

	if (secondary)
		wield = get_eq_char(ch, WEAR_SECONDARY);
	else
		wield = get_eq_char(ch, WEAR_WIELD);

	if (wield == nullptr || wield->item_type != ITEM_WEAPON)
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

int get_weapon_skill(Character *ch, int sn)
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
int get_max_train(Character *ch, int stat)
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
int can_carry_n(Character *ch)
{
	if (IS_IMMORTAL(ch))
		return 9999;

	if (IS_NPC(ch) && ch->act_flags.has(ACT_PET))
		return 0;

	return MAX_WEAR +  2 * GET_ATTR_DEX(ch) + ch->level;
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w(Character *ch)
{
	if (IS_IMMORTAL(ch))
		return 10000000;

	if (IS_NPC(ch) && ch->act_flags.has(ACT_PET))
		return 0;

	return str_app[GET_ATTR_STR(ch)].carry * 10 + ch->level * 25;
}


/*
 * Move a char out of a room.
 */
void char_from_room(Character *ch)
{
	Object *obj;

	if (ch == nullptr)
		return;

	if (ch->in_room == nullptr) {
		bug("Char_from_room: nullptr.", 0);
		return;
	}

	if (!IS_NPC(ch))
		--ch->in_room->area->nplayer;

	if ((obj = get_eq_char(ch, WEAR_LIGHT)) != nullptr
	    &&   obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] != 0
	    &&   ch->in_room->light > 0)
		--ch->in_room->light;

	if (ch == ch->in_room->people)
		ch->in_room->people = ch->next_in_room;
	else {
		Character *prev;

		for (prev = ch->in_room->people; prev; prev = prev->next_in_room) {
			if (prev->next_in_room == ch) {
				prev->next_in_room = ch->next_in_room;
				break;
			}
		}

		if (prev == nullptr) {
			bug("Char_from_room: ch not found.", 0);
			bug(ch->name, 0);
		}
	}

	ch->in_room      = nullptr;
	ch->next_in_room = nullptr;
	ch->on           = nullptr;  /* sanity check! */
	return;
}

/*
 * Move a char into a room.
 */
void char_to_room(Character *ch, RoomPrototype *pRoomIndex)
{
	Object *obj;

	if (ch == nullptr)
		return;

	if (pRoomIndex == nullptr) {
		RoomPrototype *room;
		bug("Char_to_room: nullptr.", 0);
		bug(ch->name, 0);

		if ((room = get_room_index(ROOM_VNUM_TEMPLE)) != nullptr)
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

	if ((obj = get_eq_char(ch, WEAR_LIGHT)) != nullptr
	    &&   obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] != 0)
		++ch->in_room->light;

	const Affect *plague = affect_find_on_char(ch, gsn_plague);
	if (plague)
		spread_plague(ch->in_room, plague, 6);
}

/* Locker Code */
void obj_from_locker(Object *obj)
{
	Character *ch;

	if ((ch = obj->in_locker) == nullptr) {
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
		Object *prev;

		for (prev = ch->pcdata->locker; prev != nullptr; prev = prev->next_content)
			if (prev->next_content == obj) {
				prev->next_content = obj->next_content;
				break;
			}

		if (prev == nullptr)
			bug("obj_from_locker: obj not in ch->pcdata->locker list.", 0);
	}

	obj->next_content    = nullptr;
	obj->in_locker       = nullptr;
}

void obj_from_strongbox(Object *obj)
{
	Character *ch;

	if ((ch = obj->in_strongbox) == nullptr) {
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
		Object *prev;

		for (prev = ch->pcdata->strongbox; prev != nullptr; prev = prev->next_content)
			if (prev->next_content == obj) {
				prev->next_content = obj->next_content;
				break;
			}

		if (prev == nullptr)
			bug("Obj_from_strongbox: obj not in list.", 0);
	}

	obj->next_content    = nullptr;
	obj->in_strongbox       = nullptr;
	return;
} /* end obj_from_strongbox() */

void obj_to_locker(Object *obj, Character *ch)
{
	if (IS_NPC(ch)) {
		bug("obj_to_locker: ch is a mobile", 0);
		return;
	}

	obj->next_content       = ch->pcdata->locker;
	ch->pcdata->locker      = obj;
	obj->in_locker          = ch;
	obj->carried_by         = nullptr;
	obj->in_room            = nullptr;
	obj->in_obj             = nullptr;
}

void obj_to_strongbox(Object *obj, Character *ch)
{
	if (IS_NPC(ch)) {
		bug("obj_to_strongbox: ch is a mobile", 0);
		return;
	}

	obj->next_content       = ch->pcdata->strongbox;
	ch->pcdata->strongbox   = obj;
	obj->in_strongbox       = ch;
	obj->carried_by         = nullptr;
	obj->in_room            = nullptr;
	obj->in_obj             = nullptr;
} /* end obj_to_strongbox() */

/*
 * Give an obj to a char.
 */
void obj_to_char(Object *obj, Character *ch)
{
	obj->next_content    = ch->carrying;
	ch->carrying         = obj;
	obj->carried_by      = ch;
	obj->in_locker       = nullptr;
	obj->in_strongbox   = nullptr;
	obj->in_room         = nullptr;
	obj->in_obj          = nullptr;
	obj->clean_timer = 0;
}

/*
 * Take an obj from its character.
 */
void obj_from_char(Object *obj)
{
	Character *ch;

	if ((ch = obj->carried_by) == nullptr) {
		bug("Obj_from_char: null ch.", 0);
		return;
	}

	if (obj->wear_loc != WEAR_NONE)
		unequip_char(ch, obj);

	if (ch->carrying == obj)
		ch->carrying = obj->next_content;
	else {
		Object *prev;

		for (prev = ch->carrying; prev != nullptr; prev = prev->next_content) {
			if (prev->next_content == obj) {
				prev->next_content = obj->next_content;
				break;
			}
		}

		if (prev == nullptr)
			bug("Obj_from_char: obj not in list.", 0);
	}

	obj->carried_by      = nullptr;
	obj->next_content    = nullptr;
}

/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac(Object *obj, int iWear, int type)
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
Object *get_eq_char(Character *ch, int iWear)
{
	Object *obj;

	if (ch == nullptr)
		return nullptr;

	for (obj = ch->carrying; obj != nullptr; obj = obj->next_content) {
		if (obj->wear_loc == iWear)
			return obj;
	}

	return nullptr;
}

/*
 * Equip a char with an obj.
 */
void equip_char(Character *ch, Object *obj, int iWear)
{
	// see if some *other* object is equipped here.  allow equipping the object even
	// if get_eq_char (dumbly) says its already equipped, because of how load_char_obj
	// loads the whole list and then equips it all based on the object's wear location
	Object *equipped = get_eq_char(ch, iWear);
	if (equipped != nullptr && equipped != obj) {
		bug("Equip_char: already equipped (%d).", iWear);
		return;
	}

	for (int i = 0; i < 4; i++)
		ch->armor_base[i] -= apply_ac(obj, iWear, i);

	obj->wear_loc = iWear;

	for (const Affect *paf = affect_list_obj(obj); paf != nullptr; paf = paf->next)
		affect_modify_char(ch, paf, TRUE);

	if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room != nullptr)
		++ch->in_room->light;
}

/*
 * Unequip a char with an obj.
 */
void unequip_char(Character *ch, Object *obj)
{
	if (obj->wear_loc == WEAR_NONE) {
		bug("Unequip_char: already unequipped.", 0);
		return;
	}

	for (int i = 0; i < 4; i++)
		ch->armor_base[i]    += apply_ac(obj, obj->wear_loc, i);

	obj->wear_loc        = -1;

	for (const Affect *paf = affect_list_obj(obj); paf != nullptr; paf = paf->next)
		affect_modify_char(ch, paf, FALSE);

	if (obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] != 0
	    &&   ch->in_room != nullptr
	    &&   ch->in_room->light > 0)
		--ch->in_room->light;

	return;
}

/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list(ObjectPrototype *pObjIndex, Object *list)
{
	Object *obj;
	int nMatch;
	nMatch = 0;

	for (obj = list; obj != nullptr; obj = obj->next_content) {
		if (obj->pIndexData == pObjIndex)
			nMatch++;
	}

	return nMatch;
}

/*
 * Move an obj out of a room.
 */
void obj_from_room(Object *obj)
{
	RoomPrototype *in_room;
	Character *ch;

	if ((in_room = obj->in_room) == nullptr) {
		bug("obj_from_room: nullptr.", 0);
		bug(obj->short_descr, 0); /* Bug huntin - Lotus */
		return;
	}

	for (ch = in_room->people; ch != nullptr; ch = ch->next_in_room)
		if (ch->on == obj)
			ch->on = nullptr;

	if (obj == in_room->contents)
		in_room->contents = obj->next_content;
	else {
		Object *prev;

		for (prev = in_room->contents; prev; prev = prev->next_content) {
			if (prev->next_content == obj) {
				prev->next_content = obj->next_content;
				break;
			}
		}

		if (prev == nullptr) {
			bug("Obj_from_room: obj not found.", 0);
			return;
		}
	}

	obj->in_room      = nullptr;
	obj->next_content = nullptr;
	return;
}

/*
 * Move an obj into a room.
 */
void obj_to_room(Object *obj, RoomPrototype *pRoomIndex)
{
	/* make sure there is a room to put the object in -- Elrac */
	if (pRoomIndex == nullptr) {
		bug("obj_to_room: nullptr room", 0);
		return;
	}

	/* Floating Rooms by Lotus - Idea from WWW site */
	while (pRoomIndex &&
	       pRoomIndex->sector_type == SECT_AIR &&
	       obj->wear_flags.has(ITEM_TAKE) &&
	       pRoomIndex->exit[DIR_DOWN] &&
	       pRoomIndex->exit[DIR_DOWN]->u1.to_room) {
		RoomPrototype *new_room =
		        pRoomIndex->exit[DIR_DOWN]->u1.to_room;
		Character *rch;

		if ((rch = pRoomIndex->people) != nullptr) {
			act("$p falls away.", rch, obj, nullptr, TO_ROOM);
			act("$p falls away.", rch, obj, nullptr, TO_CHAR);
		}

		pRoomIndex = new_room;

		if ((rch = new_room->people) != nullptr) {
			act("$p floats by.", rch, obj, nullptr, TO_ROOM);
			act("$p floats by.", rch, obj, nullptr, TO_CHAR);
		}
	}

	obj->next_content           = pRoomIndex->contents;
	pRoomIndex->contents        = obj;
	obj->in_room                = pRoomIndex;
	obj->carried_by             = nullptr;
	obj->in_obj                 = nullptr;
	return;
}

/*
 * Move an object into an object.
 */
void obj_to_obj(Object *obj, Object *obj_to)
{
	if (obj_to == donation_pit) {
		obj->cost = 0;

		// don't *always* set, because do_pit temporarily removes items
		if (obj->donated == 0) { // new object going in
			obj->donated = current_time;

			// count items in the pit, if there are too many, remove the oldest
			Object *oldest = nullptr;
			int count = 0;

			for (Object *c = donation_pit->contains; c; c = c->next_content) {
				count++;
				if (oldest == nullptr || oldest->donated > c->donated)
					oldest = c;
			}

			if (count >= MAX_DONATED && oldest != nullptr)
				extract_obj(oldest); // will remove from the pit
		}
	}

	obj->next_content       = obj_to->contains;
	obj_to->contains        = obj;
	obj->in_obj             = obj_to;
	obj->in_room            = nullptr;
	obj->carried_by         = nullptr;
}

/*
 * Move an object out of an object.
 */
void obj_from_obj(Object *obj)
{
	Object *obj_from;

	if ((obj_from = obj->in_obj) == nullptr) {
		bug("Obj_from_obj: null obj_from.", 0);
		return;
	}

	if (obj == obj_from->contains)
		obj_from->contains = obj->next_content;
	else {
		Object *prev;

		for (prev = obj_from->contains; prev; prev = prev->next_content) {
			if (prev->next_content == obj) {
				prev->next_content = obj->next_content;
				break;
			}
		}

		if (prev == nullptr) {
			bug("Obj_from_obj: obj not found.", 0);
			return;
		}
	}

	obj->next_content = nullptr;
	obj->in_obj       = nullptr;
}

/*
 * Extract an obj from the world.
 */
void extract_obj(Object *obj)
{
	Object *obj_content;
	Object *obj_next;

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
		Object *prev;

		for (prev = object_list; prev != nullptr; prev = prev->next) {
			if (prev->next == obj) {
				prev->next = obj->next;
				break;
			}
		}

		if (prev == nullptr) {
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
void extract_char(Character *ch, bool fPull)
{
	Character *wch;
	Object *obj;
	Object *obj_next;

	if (ch->in_room == nullptr) {
//		bugf("extract_char: ch->in_room == nullptr for %s", ch->name);
		return;
	}

	nuke_pets(ch);
	ch->pet = nullptr; /* just in case */

	if (fPull)
		die_follower(ch);

	stop_fighting(ch, TRUE);

	for (obj = ch->carrying; obj != nullptr; obj = obj_next) {
		obj_next = obj->next_content;
		extract_obj(obj);
	}

	char_from_room(ch);

	/* Death room is set in the clan table now */
	if (!fPull) {
		char_to_room(ch, get_room_index((ch->clan != nullptr) ? ch->clan->hall : ROOM_VNUM_ALTAR));
		return;
	}

	if (!IS_NPC(ch)) {
		for (obj = ch->pcdata->locker; obj != nullptr; obj = obj_next) {
			obj_next = obj->next_content;
			extract_obj(obj);
		}

		for (obj = ch->pcdata->strongbox; obj != nullptr; obj = obj_next) {
			obj_next = obj->next_content;
			extract_obj(obj);
		}
	}

	if (IS_NPC(ch))
		--ch->pIndexData->count;

	if (ch->desc != nullptr && ch->desc->original != nullptr) {
		do_return(ch, "");
		ch->desc = nullptr;
	}

	for (wch = char_list; wch != nullptr; wch = wch->next)
		if (! strcasecmp(wch->reply, ch->name))
			wch->reply[0] = '\0';

	if (ch == char_list)
		char_list = ch->next;
	else {
		Character *prev;

		for (prev = char_list; prev != nullptr; prev = prev->next) {
			if (prev->next == ch) {
				prev->next = ch->next;
				break;
			}
		}

		if (prev == nullptr) {
			bug("extract_char: char not found in char_list", 0);
			return;
		}
	}

	if (ch->pcdata) {
		if (ch->pcdata == pc_list)
			pc_list = ch->pcdata->next;
		else {
			Player *prev;

			for (prev = pc_list; prev != nullptr; prev = prev->next) {
				if (prev->next == ch->pcdata) {
					prev->next = ch->pcdata->next;
					break;
				}
			}

			if (prev == nullptr)
				bug("extract_char: pc_data not found in pc_list", 0);
		}
	}

	if (ch->desc != nullptr)
		ch->desc->character = nullptr;

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
	MobilePrototype *index;
	int i;
	extern MobilePrototype *mob_index_hash[MAX_KEY_HASH];

	for (i = 0; i < MAX_KEY_HASH; i++)
		for (index = mob_index_hash[i]; index != nullptr; index = index->next)
			if (index->player_name.has_exact_words(name))
				return TRUE;

	return FALSE;
}

/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
Object *get_obj_type(ObjectPrototype *pObjIndex)
{
	Object *obj;

	for (obj = object_list; obj != nullptr; obj = obj->next) {
		if (obj->pIndexData == pObjIndex)
			return obj;
	}

	return nullptr;
}

/* deduct cost from a character */
bool deduct_cost(Character *ch, long cost)
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
Object *create_money(int gold, int silver)
{
	Object *obj;
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

	if ((obj = create_object(get_obj_index(GEN_OBJ_MONEY), 0)) == nullptr) {
		bug("create_money: no generic money object", 0);
		return nullptr;
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

	obj->name               = name;
	obj->short_descr        = short_descr;
	obj->description        = description;
	obj->material           = material;
	obj->value[0]           = silver;
	obj->value[1]           = gold;
	obj->cost               = 100 * gold + silver;
	obj->weight             = URANGE(1, ((gold / 5) + (silver / 20)), 30000);
	return obj;
}

/* below 2 functions replace the ints in pcdata, the complexity of trying to keep
   them correct wasn't worth the speed savings -- Montrey */
int get_locker_number(Character *ch)
{
	Object *obj;
	int number = 0;

	if (IS_NPC(ch))
		return 0;

	for (obj = ch->pcdata->locker; obj; obj = obj->next_content)
		number += get_obj_number(obj);

	return number;
}

int get_strongbox_number(Character *ch)
{
	Object *obj;
	int number = 0;

	if (IS_NPC(ch))
		return 0;

	for (obj = ch->pcdata->strongbox; obj; obj = obj->next_content)
		number += get_obj_number(obj);

	return number;
}

int get_carry_number(Character *ch)
{
	Object *obj;
	int number = 0;

	for (obj = ch->carrying; obj; obj = obj->next_content)
		number += get_obj_number(obj);

	return number;
}

int get_obj_number(Object *obj)
{
	int number = 1;

	if (((obj->item_type == ITEM_CONTAINER || IS_OBJ_STAT(obj, ITEM_COMPARTMENT)) && obj->contains)
	    || obj->item_type == ITEM_MONEY || obj->item_type == ITEM_TOKEN)
		number = 0;

	for (obj = obj->contains; obj != nullptr; obj = obj->next_content)
		number += get_obj_number(obj);

	return number;
}

int get_locker_weight(Character *ch)
{
	Object *obj;
	int weight = 0;

	if (IS_NPC(ch))
		return 0;

	for (obj = ch->pcdata->locker; obj; obj = obj->next_content)
		weight += get_obj_weight(obj);

	return weight;
}

int get_carry_weight(Character *ch)
{
	Object *obj;
	int weight = 0;

	for (obj = ch->carrying; obj; obj = obj->next_content)
		weight += get_obj_weight(obj);

	weight += gold_weight(ch->gold) + silver_weight(ch->silver);
	return URANGE(0, weight, 9999);         /* prevent score from screwing up */
}

int get_obj_weight(Object *obj)
{
	int weight;
	Object *tobj;
	weight = obj->weight;

	for (tobj = obj->contains; tobj != nullptr; tobj = tobj->next_content)
		weight += get_obj_weight(tobj) * WEIGHT_MULT(obj) / 100;

	return weight;
}

int get_true_weight(Object *obj)
{
	int weight;
	weight = obj->weight;

	for (obj = obj->contains; obj != nullptr; obj = obj->next_content)
		weight += get_obj_weight(obj);

	return weight;
}

/*
 * True if room is dark.
 */
bool room_is_dark(RoomPrototype *room)
{
	if (room == nullptr)
		return TRUE;

	if (room_is_very_dark(room))
		return TRUE;

	if (room->light > 0)
		return FALSE;

	if (GET_ROOM_FLAGS(room).has(ROOM_DARK))
		return TRUE;

	if (room->sector_type == SECT_INSIDE
	    || room->sector_type == SECT_CITY)
		return FALSE;

	if (weather_info.sunlight == SUN_DARK)
		return TRUE;

	return FALSE;
}

bool room_is_very_dark(RoomPrototype *room)
{
	if (room == nullptr)
		return TRUE;

	if (GET_ROOM_FLAGS(room).has(ROOM_NOLIGHT))
		return TRUE;

	return FALSE;
}

bool is_room_owner(Character *ch, RoomPrototype *room)
{
	if (room->owner.empty())
		return FALSE;

	return room->owner.has_words(ch->name);
}

/*
 * True if room is private.
 */
bool room_is_private(RoomPrototype *pRoomIndex)
{
	Character *rch;
	int count;

	if (!pRoomIndex->owner.empty())
		return TRUE;

	count = 0;

	for (rch = pRoomIndex->people; rch != nullptr; rch = rch->next_in_room)
		count++;

	if (GET_ROOM_FLAGS(pRoomIndex).has(ROOM_PRIVATE)  && count >= 2)
		return TRUE;

	if (GET_ROOM_FLAGS(pRoomIndex).has(ROOM_SOLITARY) && count >= 1)
		return TRUE;

	return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room(Character *ch, RoomPrototype *pRoomIndex)
{
	if (GET_ROOM_FLAGS(pRoomIndex).has(ROOM_IMP_ONLY)
	    &&  GET_RANK(ch) < RANK_IMP)
		return FALSE;

	if (IS_IMMORTAL(ch))
		return TRUE;

	/* restrictions below this line do not apply to immortals of any level. */

	if (GET_ROOM_FLAGS(pRoomIndex).has(ROOM_GODS_ONLY))
		return FALSE;

	if (GET_ROOM_FLAGS(pRoomIndex).has(ROOM_REMORT_ONLY)
	    && !IS_REMORT(ch))
		return FALSE;

	if (GET_ROOM_FLAGS(pRoomIndex).has(ROOM_HEROES_ONLY)
	    &&  !IS_HEROIC(ch))
		return FALSE;

	if (GET_ROOM_FLAGS(pRoomIndex).has(ROOM_NEWBIES_ONLY)
	    &&  ch->level > 5 && !ch->act_flags.has(PLR_MAKEBAG))
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

	if (GET_ROOM_FLAGS(pRoomIndex).has(ROOM_LEADER_ONLY)
	    && !ch->pcdata->cgroup_flags.has(GROUP_LEADER))
		return FALSE;

	if (GET_ROOM_FLAGS(pRoomIndex).has(ROOM_MALE_ONLY) && GET_ATTR_SEX(ch) != SEX_MALE)
		return FALSE;

	if (GET_ROOM_FLAGS(pRoomIndex).has(ROOM_FEMALE_ONLY) && GET_ATTR_SEX(ch) != SEX_FEMALE)
		return FALSE;

	return TRUE;
}

bool is_blinded(const Character *ch) {
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
bool can_see_char(const Character *ch, const Character *victim)
{
	if (ch == victim)
		return TRUE;

	if (IS_IMP(ch))
		return TRUE;

	if (!IS_NPC(victim) && victim->act_flags.has(PLR_SUPERWIZ))
		return FALSE;

	if (IS_IMMORTAL(ch))
		return TRUE;

	if (IS_NPC(victim) && victim->act_flags.has(ACT_SUPERMOB))
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
	    &&   victim->fighting == nullptr) {
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
	    &&   victim->fighting == nullptr)
		return FALSE;

	return TRUE;
} /* end can_see_char() */

/*
 * True if char can see victim in WHO -- Elrac
 * This makes almost all characters visible in who, except hiding imms
 */
bool can_see_who(const Character *ch, const Character *victim)
{
	/* wizi still rules */
	if (victim->invis_level && !IS_IMMORTAL(ch))
		return FALSE;

	/* so does SUPERWIZ */
	if (victim->act_flags.has(PLR_SUPERWIZ) && !IS_NPC(victim) && !IS_IMP(ch))
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
bool can_see_in_room(Character *ch, RoomPrototype *room)
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
bool can_see_obj(const Character *ch, const Object *obj)
{
	if (IS_IMMORTAL(ch))
		return TRUE;

	if (room_is_very_dark(ch->in_room))
		return FALSE;

	if (is_blinded(ch))
		return FALSE;

	if (IS_OBJ_STAT(obj, ITEM_VIS_DEATH) && obj->carried_by != ch)
		return FALSE;

	if (obj->timer > 0 && !IS_NPC(ch) && obj->name.has_words("mox")
	    && !obj->name.substr(4).has_words(ch->name))
		return FALSE;

	if (obj->pIndexData->vnum == OBJ_VNUM_SQUESTOBJ) {
		if (IS_NPC(ch))
			return FALSE;

		if (!IS_SQUESTOR(ch) || ch->pcdata->squestobj == nullptr || obj != ch->pcdata->squestobj)
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
bool can_drop_obj(Character *ch, Object *obj)
{
	if (!IS_OBJ_STAT(obj, ITEM_NODROP))
		return TRUE;

	if (IS_IMMORTAL(ch))
		return TRUE;

	return FALSE;
}

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
int parse_deity(const String& dstring)
{
	int i;

	if (dstring.empty())
		return -1;

	for (i = 0; i < deity_table.size(); i++)
		if (strstr(dstring.uncolor(), deity_table[i].name))
			return i;

	return -1;
}

int get_usable_level(Character *ch)
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

int get_holdable_level(Character *ch)
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

String get_owner(Character *ch, Object *obj)
{
	if (obj->extra_descr != nullptr) {
		ExtraDescr *ed_next;
		ExtraDescr *ed;

		for (ed = obj->extra_descr; ed != nullptr; ed = ed_next) {
			ed_next = ed->next;

			if (ed->keyword == KEYWD_OWNER) {
				if (can_see_obj(ch, obj))
					return ed->description;
				else
					return "someone";
			}
		}
	}

	return "(none)";
}

Character *get_obj_carrier(Object *obj)
{
	Object *in_obj;

	for (in_obj = obj; in_obj->in_obj != nullptr; in_obj = in_obj->in_obj)
		;

	return in_obj->carried_by;
}

/* Return what position a character is in.  This replaces all direct
   accessing of position, because POS_FIGHTING is now not an actual
   position but is returned when ch->fighting is not null and their
   actual position is standing.  Later it can be used to simplify
   adding positions, like POS_FLYING (future project).  -- Montrey */
int get_position(Character *ch)
{
	if (ch == nullptr)
		return -1;

	if (ch->fighting
	 && ch->position >= POS_STANDING) // includes POS_FLYING
		return POS_FIGHTING;

	return ch->position;
}

/* retrieve a character's playing time in hours */
int get_play_hours(Character *ch)
{
	return (IS_NPC(ch) ? 0 : ch->pcdata->played / 3600);
}

/* retrieve a character's playing time in seconds */
int get_play_seconds(Character *ch)
{
	return (IS_NPC(ch) ? 0 : ch->pcdata->played);
}

// TODO: this doesn't take eq affects into account, but short of looping through those...
/* used with affect_exists_on_char, checks to see if the Affect.hppas an evolution rating, returns 1 if not */
int get_affect_evolution(Character *ch, int sn)
{
	int evo = 1;

	for (const Affect *paf = affect_list_char(ch); paf != nullptr; paf = paf->next)
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
ExtraDescr *get_extra_descr(const String& name, ExtraDescr *ed)
{
	for (; ed != nullptr; ed = ed->next)
		if (ed->keyword.has_words(name))
			return ed;

	return nullptr;
}
