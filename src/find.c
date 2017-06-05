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

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
		if (rch->in_room == NULL)
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

			if (!is_name(arg, rch->name))
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

	return NULL;
}

/* Find a character in the same area as ch.  Use mobonly to only find
   only mobiles, use get_player_area to only find players. */
Character *get_mob_area(Character *ch, const String& argument, int vis)
{
	Character *ach;

	if ((ach = get_mob_here(ch, argument, vis)) != NULL)
		return ach;

	int count = 0;
	String arg;
	int etype = entity_argument(argument, arg);
	int number = number_argument(arg, arg);

	for (ach = char_list; ach != NULL; ach = ach->next) {
		if (!IS_NPC(ach))
			continue;

		if (ach->in_room == NULL)
			continue;

		if (etype != 0 && !CHARTYPE_MATCH(ach, etype))
			continue;

		if (ach->in_room->area != ch->in_room->area
		    || !is_name(arg, ach->name))
			continue;

		switch (vis) {
		case VIS_PLR:   if (!can_see_who(ch, ach))      continue;       break;

		case VIS_CHAR:  if (!can_see_char(ch, ach))      continue;       break;

		default:                                                        break;
		}

		if (++count == number)
			return ach;
	}

	return NULL;
}

/* Find a character in the world.  Use mobonly to only find
   only mobiles, use get_player_world to only find players. */
Character *get_mob_world(Character *ch, const String& argument, int vis)
{
	Character *wch;

	if ((wch = get_mob_here(ch, argument, vis)) != NULL)
		return wch;

	int count = 0, vnum = 0;
	String arg;
	int etype = entity_argument(argument, arg);
	int number = number_argument(arg, arg);

	if (etype == ENTITY_VM)
		vnum = atoi(arg);

	for (wch = char_list; wch != NULL ; wch = wch->next) {
		if (!IS_NPC(wch))
			continue;

		if (wch->in_room == NULL)
			continue;

		if (etype == ENTITY_VM) {
			if (!IS_NPC(wch) || !wch->pIndexData || wch->pIndexData->vnum != vnum)
				continue;
		}
		else {
			if (etype != 0 && !CHARTYPE_MATCH(wch, etype))
				continue;

			if (!is_name(arg, wch->name))
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

	return NULL;
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

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
		if (rch->in_room == NULL)
			continue;

		if (etype == ENTITY_VM) {
			if (!IS_NPC(rch) || !rch->pIndexData || rch->pIndexData->vnum != vnum)
				continue;
		}
		else {
			if (etype != 0 && !CHARTYPE_MATCH(rch, etype))
				continue;

			if (!is_name(arg, rch->name))
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

	return NULL;
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

	for (rch = room->people; rch != NULL; rch = rch->next_in_room) {
		if (rch->in_room == NULL)
			continue;

		if (etype == ENTITY_VM) {
			if (!IS_NPC(rch) || !rch->pIndexData || rch->pIndexData->vnum != vnum)
				continue;
		}
		else {
			if (etype != 0 && !CHARTYPE_MATCH(rch, etype))
				continue;

			if (!is_name(arg, rch->name))
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

	return NULL;
}

/* Find a character in the same area as ch.  Use mobonly to only find
   only mobiles, use get_player_area to only find players. */
Character *get_char_area(Character *ch, const String& argument, int vis)
{
	Character *ach;

	if ((ach = get_char_here(ch, argument, vis)) != NULL)
		return ach;

	int count = 0;
	String arg;
	int etype = entity_argument(argument, arg);
	int number = number_argument(arg, arg);

	for (ach = char_list; ach != NULL; ach = ach->next) {
		if (ach->in_room == NULL)
			continue;

		if (etype != 0 && !CHARTYPE_MATCH(ach, etype))
			continue;

		if (ach->in_room->area != ch->in_room->area
		    || !is_name(arg, ach->name))
			continue;

		switch (vis) {
		case VIS_PLR:   if (!can_see_who(ch, ach))      continue;       break;

		case VIS_CHAR:  if (!can_see_char(ch, ach))      continue;       break;

		default:                                                        break;
		}

		if (++count == number)
			return ach;
	}

	return NULL;
}

/* Find a character in the world.  Use mobonly to only find
   only mobiles, use get_player_world to only find players. */
Character *get_char_world(Character *ch, const String& argument, int vis)
{
	Character *wch;

	if ((wch = get_char_here(ch, argument, vis)) != NULL)
		return wch;

	int count = 0, vnum = 0;
	String arg;
	int etype = entity_argument(argument, arg);
	int number = number_argument(arg, arg);

	if (etype == ENTITY_VM)
		vnum = atoi(arg);

	for (wch = char_list; wch != NULL ; wch = wch->next) {
		if (wch->in_room == NULL)
			continue;

		if (etype == ENTITY_VM) {
			if (!IS_NPC(wch) || !wch->pIndexData || wch->pIndexData->vnum != vnum)
				continue;
		}
		else {
			if (etype != 0 && !CHARTYPE_MATCH(wch, etype))
				continue;

			if (!is_name(arg, wch->name))
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

	return NULL;
}

/* Find a player in the room. -- Elrac
   This does *not* find '2.Elrac'. Numbering is silly with players. */
Character *get_player_here(Character *ch, const String& argument, int vis)
{
	Character *rch;

	if (argument == "self")
		return ch;

	if (ch->in_room == NULL)
		return NULL;

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
		switch (vis) {
		case VIS_PLR:   if (!can_see_who(ch, rch))      continue;       break;

		case VIS_CHAR:  if (!can_see_char(ch, rch))      continue;       break;

		default:                                                        break;
		}

		if (!IS_NPC(rch)
		    && is_name(argument, rch->name))
			return rch;
	}

	return NULL;
}

/* Find a *player* char in the same area as ch.
 * This does not find '2.Montrey'. Numbering is silly for players. */
Character *get_player_area(Character *ch, const String& argument, int vis)
{
	Character *ach;
	Player *apc;

	if (argument == "self")
		return ch;

	if (ch->in_room == NULL)
		return NULL;

	/* use the pc_data list instead of searching through thousands of mobs -- Montrey */
	for (apc = pc_list; apc != NULL; apc = apc->next) {
		if ((ach = apc->ch) == NULL) {
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

		if (ach->in_room != NULL
		    && ach->in_room->area == ch->in_room->area
		    && is_name(argument, ach->name))
			return ach;
	}

	return NULL;
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
		return NULL;            /* sloppy, prevents Alara from accidentally frying players -- Montrey */

	/* use the pc_data list instead of searching through thousands of mobs -- Montrey */
	for (wpc = pc_list; wpc != NULL; wpc = wpc->next) {
		if ((wch = wpc->ch) == NULL) {
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

		if (wch->in_room != NULL
		    && is_name(argument, wch->name))
			return wch;
	}

	return NULL;
}

/* Find an obj in a list. */
Object *get_obj_list(Character *ch, const String& argument, Object *list)
{
	Object *obj;
	int number, count = 0;
	String arg;
	number = number_argument(argument, arg);

	for (obj = list; obj != NULL; obj = obj->next_content)
		if (can_see_obj(ch, obj) && is_name(arg, obj->name))
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

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
		if (obj->wear_loc != WEAR_NONE
		    && can_see_obj(ch, obj)
		    && is_name(arg, obj->name))
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

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
		if (obj->wear_loc == WEAR_NONE
		    && can_see_obj(ch, obj)
		    && is_name(arg, obj->name))
			if (++count == number)
				break;

	return obj;
}

/* Find an obj in the room or in inventory. */
Object *get_obj_here(Character *ch, const String& argument)
{
	Object *obj;

	if ((obj = get_obj_list(ch, argument, ch->in_room->contents)) != NULL)
		return obj;

	if ((obj = get_obj_carry(ch, argument)) != NULL)
		return obj;

	if ((obj = get_obj_wear(ch, argument)) != NULL)
		return obj;

	return NULL;
}

/* Find an obj in the world. */
Object *get_obj_world(Character *ch, const String& argument)
{
	Object *obj;
	int number, count = 0;

	if ((obj = get_obj_here(ch, argument)) != NULL)
		return obj;

	String arg;
	number = number_argument(argument, arg);

	for (obj = object_list; obj; obj = obj->next)
		if (can_see_obj(ch, obj)
		    && is_name(arg, obj->name))
			if (++count == number)
				break;

	return obj;
}
