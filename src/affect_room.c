#include "merc.h"
#include "affect.h"
#include "affect_int.h"
#include "recycle.h"


AFFECT_DATA *affect_find_in_room(ROOM_INDEX_DATA *room, int sn) {
	return affect_find_in_list(room->affected, sn);
}

/* Give an affect to a room */
void affect_copy_to_room(ROOM_INDEX_DATA *room, const AFFECT_DATA *template)
{
	affect_copy_to_list(&room->affected, template);
	affect_modify_room(room, template, TRUE);
}

void affect_join_to_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf)
{
	affect_dedup_in_list(&room->affected, paf);
	affect_modify_room(room, paf, FALSE);
	affect_copy_to_room(room, paf);
}

void affect_remove_from_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf)
{
	affect_remove_from_list(&room->affected, paf);
	affect_modify_room(room, paf, FALSE);
	free_affect(paf);
}

void affect_update_in_room(ROOM_INDEX_DATA *room, AFFECT_DATA *original, const AFFECT_DATA *template)
{
	affect_modify_room(room, original, FALSE);
	affect_update(original, template);
	affect_modify_room(room, original, TRUE);
}

void affect_remove_sn_from_room(ROOM_INDEX_DATA *room, int sn) {
	AFFECT_DATA *paf, *head = room->affected;

	while ((paf = affect_find_in_list(head, sn)) != NULL) {
		head = paf->next; // start next iteration here instead of looping through again
		affect_remove_from_room(room, paf);
	}
}

void affect_remove_all_from_room(ROOM_INDEX_DATA *room) {
	while (room->affected)
		affect_remove_from_room(room, room->affected);
}

void affect_modify_flag_cache_room(ROOM_INDEX_DATA *room, sh_int where, unsigned int flags, bool fAdd) {
	if (flags == 0)
		return;

	if (where != TO_ROOMFLAGS)
		return;

	// if we're removing a bit, the only way to know for sure whether the object
	// should still have that bit (from a remaining affect) is to loop through
	// them all, so just rebuild the bit vectors
	if (!fAdd) {
		room->room_flag_cache = 0;

		for (const AFFECT_DATA *paf = room->affected; paf; paf = paf->next)
			affect_modify_flag_cache_room(room, paf->where, paf->bitvector, TRUE);
	}
	else {
		SET_BIT(room->room_flag_cache, flags);
	}
}

void affect_modify_room(ROOM_INDEX_DATA *room, const AFFECT_DATA *paf, bool fAdd) {
	switch (paf->where) {
	case TO_ROOMFLAGS:
		affect_modify_flag_cache_room(room, paf->where, paf->bitvector, fAdd);
		break;

	case TO_HPREGEN:
		room->heal_rate += paf->modifier * fAdd ? 1 : -1;
		break;

	case TO_MPREGEN:
		room->mana_rate += paf->modifier * fAdd ? 1 : -1;
		break;
	}
}
