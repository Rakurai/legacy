#include "affect/affect_list.hh"
#include "affect/Affect.hh"
#include "Room.hh"

namespace affect {

// local
void modify_room(void *owner, const Affect *paf, bool fAdd);

// searching

const Affect *list_room(Room *room) {
	return room->affected;
}

bool exists_on_room(Room *room, ::affect::type type) {
	return find_on_room(room, type) ? true : false;
}

const Affect *find_on_room(Room *room, ::affect::type type) {
	return find_in_list(&room->affected, type);
}

// adding

void copy_to_room(Room *room, const Affect *aff_template)
{
	copy_to_list(&room->affected, aff_template);
	modify_room(room, aff_template, true);
}

void join_to_room(Room *room, Affect *paf) {
	fn_params params;

	params.owner = room;
	params.modifier = modify_room;
	params.data = nullptr;

	dedup_in_list(&room->affected, paf, &params); // remove
	copy_to_room(room, paf); // add and modify holder
}

// removing

void remove_from_room(Room *room, Affect *paf)
{
	remove_from_list(&room->affected, paf);
	modify_room(room, paf, false);
	delete paf;
}

void remove_matching_from_room(Room *room, comparator comp, const Affect *pattern) {
	fn_params params;

	params.owner = room;
	params.modifier = modify_room;
	params.data = nullptr;

	remove_matching_from_list(&room->affected, comp, pattern, &params);
}

void remove_marked_from_room(Room *room) {
	Affect pattern;
	pattern.mark = true;

	remove_matching_from_room(room, comparator_mark, &pattern);
}

void remove_type_from_room(Room *room, ::affect::type type) {
	Affect pattern;
	pattern.type = type;

	remove_matching_from_room(room, comparator_type, &pattern);
}

void remove_all_from_room(Room *room, bool permanent)
{
	Affect pattern;
	pattern.permanent = permanent;

	remove_matching_from_room(room, comparator_permanent, &pattern);
}

// modifying

void iterate_over_room(Room *room, affect_fn fn, void *data) {
	fn_params params;

	params.owner = room;
	params.modifier = modify_room;
	params.data = data;

	iterate_over_list(&room->affected, fn, &params);
}

void sort_room(Room *room, comparator comp) {
	sort_list(&room->affected, comp);
}

// utility

void modify_flag_cache_room(Room *room, int where, const Flags& flags, bool fAdd) {
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
			modify_flag_cache_room(room, paf->where, paf->bitvector(), true);
	}
	else {
		room->cached_room_flags += flags;
	}
}

// the modify function is called any time there is a potential change to the list of
// affects, and here we update any caches or entities that depend on the affect list.
// it is important that owner->affected reflects the new state of the affects, i.e.
// the Affect.hppas already been inserted or removed, and paf is not a member of the set.
void modify_room(void *owner, const Affect *paf, bool fAdd) {
	modify_room((Room *)owner, paf, fAdd);
}

void modify_room(Room *room, const Affect *paf, bool fAdd) {
	switch (paf->where) {
	case TO_ROOMFLAGS:
		modify_flag_cache_room(room, paf->where, paf->bitvector(), fAdd);
		break;

	case TO_HPREGEN:
// can't do this right now because i dont want two more ints in a room, supposed to be
// a flyweight.  maybe the room's can lookup against a map of rooms that have altered
// heal/mana rates than their prototypes?
//		room->heal_rate += paf->modifier * (fAdd ? 1 : -1);
		break;

	case TO_MPREGEN:
//		room->mana_rate += paf->modifier * (fAdd ? 1 : -1);
		break;
	}
}

} // namespace affect
