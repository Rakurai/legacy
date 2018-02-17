#include "affect/affect_int.hh"
#include "affect/affect_list.hh"
#include "affect/Affect.hh"
#include "declare.hh"
#include "Flags.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"

namespace affect {

// local
void modify_obj(void *owner, const Affect *paf, bool fAdd);

// searching

bool enchanted_obj(Object *obj) {
	return checksum_list(&obj->affected) != obj->pIndexData->affect_checksum;
}

const Affect *list_obj(Object *obj) {
	return obj->affected;
}

bool exists_on_obj(Object *obj, Type type) {
	return find_on_obj(obj, type) ? TRUE : FALSE;
}

const Affect *find_on_obj(Object *obj, Type type) {
	return find_in_list(&obj->affected, type);
}

// adding

void copy_to_obj(Object *obj, const Affect *aff_template)
{
	copy_to_list(&obj->affected, aff_template);
	modify_obj(obj, aff_template, TRUE);
}

void join_to_obj(Object *obj, Affect *paf) {
	fn_params params;

	params.owner = obj;
	params.modifier = modify_obj;
	params.data = nullptr;

	dedup_in_list(&obj->affected, paf, &params); // remove
	copy_to_obj(obj, paf); // add and modify holder
}

// removing

void remove_from_obj(Object *obj, Affect *paf)
{
	remove_from_list(&obj->affected, paf);
	modify_obj(obj, paf, FALSE);
	delete paf;
}

void remove_matching_from_obj(Object *obj, comparator comp, const Affect *pattern) {
	fn_params params;

	params.owner = obj;
	params.modifier = modify_obj;
	params.data = nullptr;

	remove_matching_from_list(&obj->affected, comp, pattern, &params);
}

void remove_marked_from_obj(Object *obj) {
	Affect pattern;
	pattern.mark = TRUE;

	remove_matching_from_obj(obj, comparator_mark, &pattern);
}

void remove_type_from_obj(Object *obj, Type type) {
	Affect pattern;
	pattern.type = type;

	remove_matching_from_obj(obj, comparator_type, &pattern);
}

void remove_all_from_obj(Object *obj, bool permanent)
{
	Affect pattern;
	pattern.permanent = permanent;

	remove_matching_from_obj(obj, comparator_permanent, &pattern);
}

// modifying

void iterate_over_obj(Object *obj, affect_fn fn, void *data) {
	fn_params params;

	params.owner = obj;
	params.modifier = modify_obj;
	params.data = data;

	iterate_over_list(&obj->affected, fn, &params);
}

void sort_obj(Object *obj, comparator comp) {
	sort_list(&obj->affected, comp);
}

// utility

// test if an object has an affect
bool obj_has_affect(Object *obj, Type type) {
	return find_in_list(&obj->affected, type) ? TRUE : FALSE;
}

void modify_flag_cache_obj(Object *obj, sh_int where, const Flags& flags, bool fAdd) {
	if (flags.empty())
		return;

	if (where != TO_OBJECT && where != TO_WEAPON)
		return;

	// if we're removing a bit, the only way to know for sure whether the object
	// should still have that bit (from a remaining affect) is to loop through
	// them all, so just rebuild the bit vectors
	if (!fAdd) {
		obj->cached_extra_flags.clear();

		for (const Affect *paf = obj->affected; paf; paf = paf->next)
			modify_flag_cache_obj(obj, paf->where, paf->bitvector(), TRUE);

		for (const Affect *paf = obj->gem_affected; paf; paf = paf->next)
			modify_flag_cache_obj(obj, paf->where, paf->bitvector(), TRUE);
	}
	else {
		switch (where) {
		case TO_OBJECT:
			obj->cached_extra_flags += flags;
			break;
		}
	}
}

// the modify function is called any time there is a potential change to the list of
// affects, and here we update any caches or entities that depend on the affect list.
// it is important that owner->affected reflects the new state of the affects, i.e.
// the Affect.hppas already been inserted or removed, and paf is not a member of the set.
void modify_obj(void *owner, const Affect *paf, bool fAdd) {
	modify_obj((Object *)owner, paf, fAdd);
}

void modify_obj(Object *obj, const Affect *paf, bool fAdd) {
	modify_flag_cache_obj(obj, paf->where, paf->bitvector(), fAdd);

	if (obj->carried_by && obj->wear_loc != WEAR_NONE)
		modify_char(obj->carried_by, paf, fAdd);
}

} // namespace affect
