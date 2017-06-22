#include "affect_int.hh"
#include "affect_list.hh"
#include "Affect.hh"
#include "declare.hh"
#include "Flags.hh"
#include "RoomPrototype.hh"

// local declarations
void affect_modify_room(void *owner, const Affect *paf, bool fAdd);

// searching

const Affect *affect_list_room(RoomPrototype *room) {
	return room->affected;
}

bool affect_exists_on_room(RoomPrototype *room, int sn) {
	return affect_find_on_room(room, sn) ? TRUE : FALSE;
}

const Affect *affect_find_on_room(RoomPrototype *room, int sn) {
	return affect_find_in_list(&room->affected, sn);
}

// adding

void affect_copy_to_room(RoomPrototype *room, const Affect *aff_template)
{
	affect_copy_to_list(&room->affected, aff_template);
	affect_modify_room(room, aff_template, TRUE);
}

void affect_join_to_room(RoomPrototype *room, Affect *paf) {
	affect_fn_params params;

	params.owner = room;
	params.modifier = affect_modify_room;
	params.data = nullptr;

	affect_dedup_in_list(&room->affected, paf, &params); // remove
	affect_copy_to_room(room, paf); // add and modify holder
}

// removing

void affect_remove_from_room(RoomPrototype *room, Affect *paf)
{
	affect_remove_from_list(&room->affected, paf);
	affect_modify_room(room, paf, FALSE);
	delete paf;
}

void affect_remove_matching_from_room(RoomPrototype *room, affect_comparator comp, const Affect *pattern) {
	affect_fn_params params;

	params.owner = room;
	params.modifier = affect_modify_room;
	params.data = nullptr;

	affect_remove_matching_from_list(&room->affected, comp, pattern, &params);
}

void affect_remove_marked_from_room(RoomPrototype *room) {
	Affect pattern;
	pattern.mark = TRUE;

	affect_remove_matching_from_room(room, affect_comparator_mark, &pattern);
}

void affect_remove_sn_from_room(RoomPrototype *room, int sn) {
	Affect pattern;
	pattern.type = sn;

	affect_remove_matching_from_room(room, affect_comparator_type, &pattern);
}

void affect_remove_all_from_room(RoomPrototype *room, bool permanent)
{
	Affect pattern;
	pattern.permanent = permanent;

	affect_remove_matching_from_room(room, affect_comparator_permanent, &pattern);
}

// modifying

void affect_iterate_over_room(RoomPrototype *room, affect_fn fn, void *data) {
	affect_fn_params params;

	params.owner = room;
	params.modifier = affect_modify_room;
	params.data = data;

	affect_iterate_over_list(&room->affected, fn, &params);
}

void affect_sort_room(RoomPrototype *room, affect_comparator comp) {
	affect_sort_list(&room->affected, comp);
}

// utility

void affect_modify_flag_cache_room(RoomPrototype *room, sh_int where, const Flags& flags, bool fAdd) {
	if (flags.empty())
		return;

	if (where != TO_ROOMFLAGS)
		return;

	// if we're removing a bit, the only way to know for sure whether the object
	// should still have that bit (from a remaining affect) is to loop through
	// them all, so just rebuild the bit vectors
	if (!fAdd) {
		room->cached_room_flags.clear();

		for (const Affect *paf = room->affected; paf; paf = paf->next)
			affect_modify_flag_cache_room(room, paf->where, paf->bitvector(), TRUE);
	}
	else {
		room->cached_room_flags += flags;
	}
}

// the modify function is called any time there is a potential change to the list of
// affects, and here we update any caches or entities that depend on the affect list.
// it is important that owner->affected reflects the new state of the affects, i.e.
// the Affect.hppas already been inserted or removed, and paf is not a member of the set.
void affect_modify_room(void *owner, const Affect *paf, bool fAdd) {
	RoomPrototype *room = (RoomPrototype *)owner;

	switch (paf->where) {
	case TO_ROOMFLAGS:
		affect_modify_flag_cache_room(room, paf->where, paf->bitvector(), fAdd);
		break;

	case TO_HPREGEN:
		room->heal_rate += paf->modifier * (fAdd ? 1 : -1);
		break;

	case TO_MPREGEN:
		room->mana_rate += paf->modifier * (fAdd ? 1 : -1);
		break;
	}
}
