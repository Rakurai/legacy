#include "merc.h"
#include "affect.h"
#include "affect_list.h"
#include "recycle.h"

// local declarations
void affect_modify_room(void *owner, const AFFECT_DATA *paf, bool fAdd);

// searching

const AFFECT_DATA *affect_list_room(ROOM_INDEX_DATA *room) {
	return room->affected;
}

bool affect_exists_on_room(ROOM_INDEX_DATA *room, int sn) {
	return affect_find_on_room(room, sn) ? TRUE : FALSE;
}

const AFFECT_DATA *affect_find_on_room(ROOM_INDEX_DATA *room, int sn) {
	return affect_find_in_list(&room->affected, sn);
}

// adding

void affect_copy_to_room(ROOM_INDEX_DATA *room, const AFFECT_DATA *aff_template)
{
	affect_copy_to_list(&room->affected, aff_template);
	affect_modify_room(room, aff_template, TRUE);
}

void affect_join_to_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf) {
	affect_fn_params params;

	params.owner = room;
	params.modifier = affect_modify_room;
	params.data = NULL;

	affect_dedup_in_list(&room->affected, paf, &params); // remove
	affect_copy_to_room(room, paf); // add and modify holder
}

// removing

void affect_remove_from_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf)
{
	affect_remove_from_list(&room->affected, paf);
	affect_modify_room(room, paf, FALSE);
	free_affect(paf);
}

void affect_remove_matching_from_room(ROOM_INDEX_DATA *room, affect_comparator comp, const AFFECT_DATA *pattern) {
	affect_fn_params params;

	params.owner = room;
	params.modifier = affect_modify_room;
	params.data = NULL;

	affect_remove_matching_from_list(&room->affected, comp, pattern, &params);
}

void affect_remove_marked_from_room(ROOM_INDEX_DATA *room) {
	AFFECT_DATA pattern;
	pattern.mark = TRUE;

	affect_remove_matching_from_room(room, affect_comparator_mark, &pattern);
}

void affect_remove_sn_from_room(ROOM_INDEX_DATA *room, int sn) {
	AFFECT_DATA pattern;
	pattern.type = sn;

	affect_remove_matching_from_room(room, affect_comparator_type, &pattern);
}

void affect_remove_all_from_room(ROOM_INDEX_DATA *room, bool permanent)
{
	AFFECT_DATA pattern;
	pattern.permanent = permanent;

	affect_remove_matching_from_room(room, affect_comparator_permanent, &pattern);
}

// modifying

void affect_iterate_over_room(ROOM_INDEX_DATA *room, affect_fn fn, void *data) {
	affect_fn_params params;

	params.owner = room;
	params.modifier = affect_modify_room;
	params.data = data;

	affect_iterate_over_list(&room->affected, fn, &params);
}

void affect_sort_room(ROOM_INDEX_DATA *room, affect_comparator comp) {
	affect_sort_list(&room->affected, comp);
}

// utility

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

// the modify function is called any time there is a potential change to the list of
// affects, and here we update any caches or entities that depend on the affect list.
// it is important that owner->affected reflects the new state of the affects, i.e.
// the affect has already been inserted or removed, and paf is not a member of the set.
void affect_modify_room(void *owner, const AFFECT_DATA *paf, bool fAdd) {
	ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *)owner;

	switch (paf->where) {
	case TO_ROOMFLAGS:
		affect_modify_flag_cache_room(room, paf->where, paf->bitvector, fAdd);
		break;

	case TO_HPREGEN:
		room->heal_rate += paf->modifier * (fAdd ? 1 : -1);
		break;

	case TO_MPREGEN:
		room->mana_rate += paf->modifier * (fAdd ? 1 : -1);
		break;
	}
}
