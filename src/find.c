/***********************
 Find.c
 Moved all the functions that search for characters, objects, and rooms
 here after I got sick of sifting through handler.c looking for them
 Montrey
***********************/

#include "merc.h"

/* character finding functions */

/* get_mob_here         (Character *ch, const char *argument, int vis)
   get_mob_area         (Character *ch, const char *argument, int vis)
   get_mob_world        (Character *ch, const char *argument, int vis)

   get_player_here      (Character *ch, const char *argument, int vis)
   get_player_area      (Character *ch, const char *argument, int vis)
   get_player_world     (Character *ch, const char *argument, int vis)

   get_char_here        (Character *ch, const char *argument, int vis)
   get_char_area        (Character *ch, const char *argument, int vis)
   get_char_world       (Character *ch, const char *argument, int vis)
*/

/* Find a mobile in the same room as ch */
Character *get_mob_here(Character *ch, const String& argument, int vis)
{
	Character *rch;
	int count = 0, vnum = 0;
	String arg;
	int etype = entity_argument(argument, arg);
	int number = number_argument(arg, arg);

	if (etype == ENTITY_VM)
		vnum = atoi(arg);

	if (arg == "self")
		return ch;

	for (rch = ch->in_room->people; rch != nullptr; rch = rch->next_in_room) {
		if (rch->in_room == nullptr)
			continue;

		if (!IS_NPC(rch))
			continue;

		if (etype == ENTITY_VM) {
			if (!IS_NPC(rch) || !rch->pIndexData || rch->pIndexData->vnum != vnum)
				continue;
		}
		else {
			if (etype != 0 && !CHARTYPE_MATCH(rch, etype))
				continue;

			if (!rch->name.has_words(arg))
				continue;
		}

		switch (vis) {
		case VIS_PLR:   if (!can_see_who(ch, rch))      continue;       break;

		case VIS_CHAR:  if (!can_see_char(ch, rch))      continue;       break;

		default:                                                        break;
		}

		if (++count == number)
			return rch;
	}

	return nullptr;
}

/* Find a character in the same area as ch.  Use mobonly to only find
   only mobiles, use get_player_area to only find players. */
Character *get_mob_area(Character *ch, const String& argument, int vis)
{
	Character *ach;

	if ((ach = get_mob_here(ch, argument, vis)) != nullptr)
		return ach;

	int count = 0;
	String arg;
	int etype = entity_argument(argument, arg);
	int number = number_argument(arg, arg);

	for (ach = char_list; ach != nullptr; ach = ach->next) {
		if (!IS_NPC(ach))
			continue;

		if (ach->in_room == nullptr)
			continue;

		if (etype != 0 && !CHARTYPE_MATCH(ach, etype))
			continue;

		if (ach->in_room->area != ch->in_room->area
		    || !ach->name.has_words(arg))
			continue;

		switch (vis) {
		case VIS_PLR:   if (!can_see_who(ch, ach))      continue;       break;

		case VIS_CHAR:  if (!can_see_char(ch, ach))      continue;       break;

		default:                                                        break;
		}

		if (++count == number)
			return ach;
	}

	return nullptr;
}

/* Find a character in the world.  Use mobonly to only find
   only mobiles, use get_player_world to only find players. */
Character *get_mob_world(Character *ch, const String& argument, int vis)
{
	Character *wch;

	if ((wch = get_mob_here(ch, argument, vis)) != nullptr)
		return wch;

	int count = 0, vnum = 0;
	String arg;
	int etype = entity_argument(argument, arg);
	int number = number_argument(arg, arg);

	if (etype == ENTITY_VM)
		vnum = atoi(arg);

	for (wch = char_list; wch != nullptr ; wch = wch->next) {
		if (!IS_NPC(wch))
			continue;

		if (wch->in_room == nullptr)
			continue;

		if (etype == ENTITY_VM) {
			if (!IS_NPC(wch) || !wch->pIndexData || wch->pIndexData->vnum != vnum)
				continue;
		}
		else {
			if (etype != 0 && !CHARTYPE_MATCH(wch, etype))
				continue;

			if (!wch->name.has_words(arg))
				continue;
		}

		switch (vis) {
		case VIS_PLR:   if (!can_see_who(ch, wch))      continue;       break;

		case VIS_CHAR:  if (!can_see_char(ch, wch))      continue;       break;

		default:                                                        break;
		}

		if (++count == number)
			return wch;
	}

	return nullptr;
}

/* Find a character in the same room as ch.  Use mobonly to find
   only mobiles, use get_player_here to only find players. */
Character *get_char_here(Character *ch, const String& argument, int vis)
{
	Character *rch;
	int count = 0, vnum = 0;
	String arg;
	int etype = entity_argument(argument, arg);
	int number = number_argument(arg, arg);

	if (etype == ENTITY_VM)
		vnum = atoi(arg);

	if (arg == "self")
		return ch;

	for (rch = ch->in_room->people; rch != nullptr; rch = rch->next_in_room) {
		if (rch->in_room == nullptr)
			continue;

		if (etype == ENTITY_VM) {
			if (!IS_NPC(rch) || !rch->pIndexData || rch->pIndexData->vnum != vnum)
				continue;
		}
		else {
			if (etype != 0 && !CHARTYPE_MATCH(rch, etype))
				continue;

			if (!rch->name.has_words(arg))
				continue;
		}

		switch (vis) {
		case VIS_PLR:   if (!can_see_who(ch, rch))      continue;       break;

		case VIS_CHAR:  if (!can_see_char(ch, rch))      continue;       break;

		default:                                                        break;
		}

		if (++count == number)
			return rch;
	}

	return nullptr;
}

/*
This function does the same as get_char_here but
it searches a given room, rather than the one the
player is in.
-- Outsider
*/
Character *get_char_room(Character *ch, RoomPrototype *room, const String& argument, int vis)
{
	Character *rch;
	int count = 0, vnum = 0;
	String arg;
	int etype = entity_argument(argument, arg);
	int number = number_argument(arg, arg);

	if (etype == ENTITY_VM)
		vnum = atoi(arg);

	if (arg == "self")
		return ch;

	for (rch = room->people; rch != nullptr; rch = rch->next_in_room) {
		if (rch->in_room == nullptr)
			continue;

		if (etype == ENTITY_VM) {
			if (!IS_NPC(rch) || !rch->pIndexData || rch->pIndexData->vnum != vnum)
				continue;
		}
		else {
			if (etype != 0 && !CHARTYPE_MATCH(rch, etype))
				continue;

			if (!rch->name.has_words(arg))
				continue;
		}

		switch (vis) {
		case VIS_PLR:   if (!can_see_who(ch, rch))      continue;       break;

		case VIS_CHAR:  if (!can_see_char(ch, rch))      continue;       break;

		default:                                                        break;
		}

		if (++count == number)
			return rch;
	}

	return nullptr;
}

/* Find a character in the same area as ch.  Use mobonly to only find
   only mobiles, use get_player_area to only find players. */
Character *get_char_area(Character *ch, const String& argument, int vis)
{
	Character *ach;

	if ((ach = get_char_here(ch, argument, vis)) != nullptr)
		return ach;

	int count = 0;
	String arg;
	int etype = entity_argument(argument, arg);
	int number = number_argument(arg, arg);

	for (ach = char_list; ach != nullptr; ach = ach->next) {
		if (ach->in_room == nullptr)
			continue;

		if (etype != 0 && !CHARTYPE_MATCH(ach, etype))
			continue;

		if (ach->in_room->area != ch->in_room->area
		    || !ach->name.has_words(arg))
			continue;

		switch (vis) {
		case VIS_PLR:   if (!can_see_who(ch, ach))      continue;       break;

		case VIS_CHAR:  if (!can_see_char(ch, ach))      continue;       break;

		default:                                                        break;
		}

		if (++count == number)
			return ach;
	}

	return nullptr;
}

/* Find a character in the world.  Use mobonly to only find
   only mobiles, use get_player_world to only find players. */
Character *get_char_world(Character *ch, const String& argument, int vis)
{
	Character *wch;

	if ((wch = get_char_here(ch, argument, vis)) != nullptr)
		return wch;

	int count = 0, vnum = 0;
	String arg;
	int etype = entity_argument(argument, arg);
	int number = number_argument(arg, arg);

	if (etype == ENTITY_VM)
		vnum = atoi(arg);

	for (wch = char_list; wch != nullptr ; wch = wch->next) {
		if (wch->in_room == nullptr)
			continue;

		if (etype == ENTITY_VM) {
			if (!IS_NPC(wch) || !wch->pIndexData || wch->pIndexData->vnum != vnum)
				continue;
		}
		else {
			if (etype != 0 && !CHARTYPE_MATCH(wch, etype))
				continue;

			if (!wch->name.has_words(arg))
				continue;
		}

		switch (vis) {
		case VIS_PLR:   if (!can_see_who(ch, wch))      continue;       break;

		case VIS_CHAR:  if (!can_see_char(ch, wch))      continue;       break;

		default:                                                        break;
		}

		if (++count == number)
			return wch;
	}

	return nullptr;
}

/* Find a player in the room. -- Elrac
   This does *not* find '2.Elrac'. Numbering is silly with players. */
Character *get_player_here(Character *ch, const String& argument, int vis)
{
	Character *rch;

	if (argument == "self")
		return ch;

	if (ch->in_room == nullptr)
		return nullptr;

	for (rch = ch->in_room->people; rch != nullptr; rch = rch->next_in_room) {
		switch (vis) {
		case VIS_PLR:   if (!can_see_who(ch, rch))      continue;       break;

		case VIS_CHAR:  if (!can_see_char(ch, rch))      continue;       break;

		default:                                                        break;
		}

		if (!IS_NPC(rch)
		    && rch->name.has_words(argument))
			return rch;
	}

	return nullptr;
}

/* Find a *player* char in the same area as ch.
 * This does not find '2.Montrey'. Numbering is silly for players. */
Character *get_player_area(Character *ch, const String& argument, int vis)
{
	Character *ach;
	Player *apc;

	if (argument == "self")
		return ch;

	if (ch->in_room == nullptr)
		return nullptr;

	/* use the pc_data list instead of searching through thousands of mobs -- Montrey */
	for (apc = pc_list; apc != nullptr; apc = apc->next) {
		if ((ach = apc->ch) == nullptr) {
			bug("get_player_area: pc_data without char_data", 0);
			continue;
		}

		if (IS_NPC(ach)) {
			bug("get_player_area: pc_data with mobile char_data", 0);
			continue;
		}

		switch (vis) {
		case VIS_PLR:   if (!can_see_who(ch, ach))      continue;       break;

		case VIS_CHAR:  if (!can_see_char(ch, ach))      continue;       break;

		default:                                                        break;
		}

		if (ach->in_room != nullptr
		    && ach->in_room->area == ch->in_room->area
		    && ach->name.has_words(argument))
			return ach;
	}

	return nullptr;
}

/* Find a *player* char in the world. -- Elrac
 * This does not find '2.Elrac'. Numbering is silly for players. */
Character *get_player_world(Character *ch, const String& argument, int vis)
{
	Character *wch;
	Player *wpc;

	if (argument == "self")
		return ch;

	if (argument.empty())
		return nullptr;            /* sloppy, prevents Alara from accidentally frying players -- Montrey */

	/* use the pc_data list instead of searching through thousands of mobs -- Montrey */
	for (wpc = pc_list; wpc != nullptr; wpc = wpc->next) {
		if ((wch = wpc->ch) == nullptr) {
			bug("get_player_world: pc_data without char_data", 0);
			continue;
		}

		if (IS_NPC(wch)) {
			bug("get_player_world: pc_data with mobile char_data", 0);
			continue;
		}

		switch (vis) {
		case VIS_PLR:   if (!can_see_who(ch, wch))      continue;       break;

		case VIS_CHAR:  if (!can_see_char(ch, wch))      continue;       break;

		default:                                                        break;
		}

		if (wch->in_room != nullptr
		    && wch->name.has_words(argument))
			return wch;
	}

	return nullptr;
}

/* Find an obj in a list. */
Object *get_obj_list(Character *ch, const String& argument, Object *list)
{
	Object *obj;
	int number, count = 0;
	String arg;
	number = number_argument(argument, arg);

	for (obj = list; obj != nullptr; obj = obj->next_content)
		if (can_see_obj(ch, obj) && obj->name.has_words(arg))
			if (++count == number)
				break;

	return obj;
}

/* Find an obj in player's equipment. */
Object *get_obj_wear(Character *ch, const String& argument)
{
	Object *obj;
	int number, count = 0;
	String arg;
	number = number_argument(argument, arg);

	for (obj = ch->carrying; obj != nullptr; obj = obj->next_content)
		if (obj->wear_loc != WEAR_NONE
		    && can_see_obj(ch, obj)
		    && obj->name.has_words(arg))
			if (++count == number)
				break;

	return obj;
}

/* Find an obj in player's inventory. */
Object *get_obj_carry(Character *ch, const String& argument)
{
	Object *obj;
	int number, count = 0;
	String arg;
	number = number_argument(argument, arg);

	for (obj = ch->carrying; obj != nullptr; obj = obj->next_content)
		if (obj->wear_loc == WEAR_NONE
		    && can_see_obj(ch, obj)
		    && obj->name.has_words(arg))
			if (++count == number)
				break;

	return obj;
}

/* Find an obj in the room or in inventory. */
Object *get_obj_here(Character *ch, const String& argument)
{
	Object *obj;

	if ((obj = get_obj_list(ch, argument, ch->in_room->contents)) != nullptr)
		return obj;

	if ((obj = get_obj_carry(ch, argument)) != nullptr)
		return obj;

	if ((obj = get_obj_wear(ch, argument)) != nullptr)
		return obj;

	return nullptr;
}

/* Find an obj in the world. */
Object *get_obj_world(Character *ch, const String& argument)
{
	Object *obj;
	int number, count = 0;

	if ((obj = get_obj_here(ch, argument)) != nullptr)
		return obj;

	String arg;
	number = number_argument(argument, arg);

	for (obj = object_list; obj; obj = obj->next)
		if (can_see_obj(ch, obj)
		    && obj->name.has_words(arg))
			if (++count == number)
				break;

	return obj;
}
