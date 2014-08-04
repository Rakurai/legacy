/***********************
 Find.c
 Moved all the functions that search for characters, objects, and rooms
 here after I got sick of sifting through handler.c looking for them
 Montrey
***********************/

//#include <sys/types.h>
//#include <sys/time.h>
//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include <ctype.h>
//#include <signal.h>
#include "merc.h"

/* character finding functions */

/* get_mob_here         (CHAR_DATA *ch, char *argument, int vis)
   get_mob_area         (CHAR_DATA *ch, char *argument, int vis)
   get_mob_world        (CHAR_DATA *ch, char *argument, int vis)

   get_player_here      (CHAR_DATA *ch, char *argument, int vis)
   get_player_area      (CHAR_DATA *ch, char *argument, int vis)
   get_player_world     (CHAR_DATA *ch, char *argument, int vis)

   get_char_here        (CHAR_DATA *ch, char *argument, int vis)
   get_char_area        (CHAR_DATA *ch, char *argument, int vis)
   get_char_world       (CHAR_DATA *ch, char *argument, int vis)
*/


/* Find a mobile in the same room as ch */
CHAR_DATA *get_mob_here(CHAR_DATA *ch, char *argument, int vis)
{
	char arg[MIL];
	CHAR_DATA *rch;
	int number, count = 0, etype, vnum = 0;
	etype = entity_argument(argument, arg);
	number = number_argument(arg, arg);

	if (etype == ENTITY_VM)
		vnum = atoi(arg);

	if (!str_cmp(arg, "self"))
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

		case VIS_CHAR:  if (!can_see(ch, rch))      continue;       break;

		default:                                                        break;
		}

		if (++count == number)
			return rch;
	}

	return NULL;
}


/* Find a character in the same area as ch.  Use mobonly to only find
   only mobiles, use get_player_area to only find players. */
CHAR_DATA *get_mob_area(CHAR_DATA *ch, char *argument, int vis)
{
	char arg[MIL];
	CHAR_DATA *ach;
	int number, etype, count = 0;

	if ((ach = get_mob_here(ch, argument, vis)) != NULL)
		return ach;

	etype = entity_argument(argument, arg);
	number = number_argument(arg, arg);

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

		case VIS_CHAR:  if (!can_see(ch, ach))      continue;       break;

		default:                                                        break;
		}

		if (++count == number)
			return ach;
	}

	return NULL;
}


/* Find a character in the world.  Use mobonly to only find
   only mobiles, use get_player_world to only find players. */
CHAR_DATA *get_mob_world(CHAR_DATA *ch, char *argument, int vis)
{
	char arg[MSL];
	CHAR_DATA *wch;
	int number, etype, count = 0, vnum = 0;

	if ((wch = get_mob_here(ch, argument, vis)) != NULL)
		return wch;

	etype = entity_argument(argument, arg);
	number = number_argument(arg, arg);

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

		case VIS_CHAR:  if (!can_see(ch, wch))      continue;       break;

		default:                                                        break;
		}

		if (++count == number)
			return wch;
	}

	return NULL;
}


/* Find a character in the same room as ch.  Use mobonly to find
   only mobiles, use get_player_here to only find players. */
CHAR_DATA *get_char_here(CHAR_DATA *ch, char *argument, int vis)
{
	char arg[MIL];
	CHAR_DATA *rch;
	int number, count = 0, etype, vnum = 0;
	etype = entity_argument(argument, arg);
	number = number_argument(arg, arg);

	if (etype == ENTITY_VM)
		vnum = atoi(arg);

	if (!str_cmp(arg, "self"))
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

		case VIS_CHAR:  if (!can_see(ch, rch))      continue;       break;

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
CHAR_DATA *get_char_room(CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument, int vis)
{
	char arg[MIL];
	CHAR_DATA *rch;
	int number, count = 0, etype, vnum = 0;
	etype = entity_argument(argument, arg);
	number = number_argument(arg, arg);

	if (etype == ENTITY_VM)
		vnum = atoi(arg);

	if (!str_cmp(arg, "self"))
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

		case VIS_CHAR:  if (!can_see(ch, rch))      continue;       break;

		default:                                                        break;
		}

		if (++count == number)
			return rch;
	}

	return NULL;
}





/* Find a character in the same area as ch.  Use mobonly to only find
   only mobiles, use get_player_area to only find players. */
CHAR_DATA *get_char_area(CHAR_DATA *ch, char *argument, int vis)
{
	char arg[MIL];
	CHAR_DATA *ach;
	int number, etype, count = 0;

	if ((ach = get_char_here(ch, argument, vis)) != NULL)
		return ach;

	etype = entity_argument(argument, arg);
	number = number_argument(arg, arg);

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

		case VIS_CHAR:  if (!can_see(ch, ach))      continue;       break;

		default:                                                        break;
		}

		if (++count == number)
			return ach;
	}

	return NULL;
}


/* Find a character in the world.  Use mobonly to only find
   only mobiles, use get_player_world to only find players. */
CHAR_DATA *get_char_world(CHAR_DATA *ch, char *argument, int vis)
{
	char arg[MSL];
	CHAR_DATA *wch;
	int number, etype, count = 0, vnum = 0;

	if ((wch = get_char_here(ch, argument, vis)) != NULL)
		return wch;

	etype = entity_argument(argument, arg);
	number = number_argument(arg, arg);

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

		case VIS_CHAR:  if (!can_see(ch, wch))      continue;       break;

		default:                                                        break;
		}

		if (++count == number)
			return wch;
	}

	return NULL;
}


/* Find a player in the room. -- Elrac
   This does *not* find '2.Elrac'. Numbering is silly with players. */
CHAR_DATA *get_player_here(CHAR_DATA *ch, char *argument, int vis)
{
	CHAR_DATA *rch;

	if (!str_cmp(argument, "self"))
		return ch;

	if (ch->in_room == NULL)
		return NULL;

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
		switch (vis) {
		case VIS_PLR:   if (!can_see_who(ch, rch))      continue;       break;

		case VIS_CHAR:  if (!can_see(ch, rch))      continue;       break;

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
CHAR_DATA *get_player_area(CHAR_DATA *ch, char *argument, int vis)
{
	CHAR_DATA *ach;
	PC_DATA *apc;

	if (!str_cmp(argument, "self"))
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

		case VIS_CHAR:  if (!can_see(ch, ach))      continue;       break;

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
CHAR_DATA *get_player_world(CHAR_DATA *ch, char *argument, int vis)
{
	CHAR_DATA *wch;
	PC_DATA *wpc;

	if (!str_cmp(argument, "self"))
		return ch;

	if (argument[0] == '\0')
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

		case VIS_CHAR:  if (!can_see(ch, wch))      continue;       break;

		default:                                                        break;
		}

		if (wch->in_room != NULL
		    && is_name(argument, wch->name))
			return wch;
	}

	return NULL;
}


/* Find an obj in a list. */
OBJ_DATA *get_obj_list(CHAR_DATA *ch, char *argument, OBJ_DATA *list)
{
	char arg[MSL];
	OBJ_DATA *obj;
	int number, count = 0;
	number = number_argument(argument, arg);

	for (obj = list; obj != NULL; obj = obj->next_content)
		if (can_see_obj(ch, obj) && is_name(arg, obj->name))
			if (++count == number)
				break;

	return obj;
}


/* Find an obj in player's equipment. */
OBJ_DATA *get_obj_wear(CHAR_DATA *ch, char *argument)
{
	char arg[MSL];
	OBJ_DATA *obj;
	int number, count = 0;
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
OBJ_DATA *get_obj_carry(CHAR_DATA *ch, char *argument)
{
	char arg[MSL];
	OBJ_DATA *obj;
	int number, count = 0;
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
OBJ_DATA *get_obj_here(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;

	if ((obj = get_obj_carry(ch, argument)) != NULL)
		return obj;

	if ((obj = get_obj_wear(ch, argument)) != NULL)
		return obj;

	return get_obj_list(ch, argument, ch->in_room->contents);
}


/* Find an obj in the world. */
OBJ_DATA *get_obj_world(CHAR_DATA *ch, char *argument)
{
	char arg[MSL];
	OBJ_DATA *obj;
	int number, count = 0;

	if ((obj = get_obj_here(ch, argument)) != NULL)
		return obj;

	number = number_argument(argument, arg);

	for (obj = object_list; obj; obj = obj->next)
		if (can_see_obj(ch, obj)
		    && is_name(arg, obj->name))
			if (++count == number)
				break;

	return obj;
}
