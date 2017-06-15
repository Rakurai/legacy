#include "merc.hh"
#include "Affect.hh"
#include "affect_list.hh"
#include "recycle.hh"

// local declarations
void affect_modify_obj(void *owner, const Affect *paf, bool fAdd);

// searching

bool affect_enchanted_obj(Object *obj) {
	return affect_checksum_list(&obj->affected) != obj->pIndexData->affect_checksum;
}

const Affect *affect_list_obj(Object *obj) {
	return obj->affected;
}

bool affect_exists_on_obj(Object *obj, int sn) {
	return affect_find_on_obj(obj, sn) ? TRUE : FALSE;
}

const Affect *affect_find_on_obj(Object *obj, int sn) {
	return affect_find_in_list(&obj->affected, sn);
}

// adding

void affect_copy_to_obj(Object *obj, const Affect *aff_template)
{
	affect_copy_to_list(&obj->affected, aff_template);
	affect_modify_obj(obj, aff_template, TRUE);
}

void affect_join_to_obj(Object *obj, Affect *paf) {
	affect_fn_params params;

	params.owner = obj;
	params.modifier = affect_modify_obj;
	params.data = nullptr;

	affect_dedup_in_list(&obj->affected, paf, &params); // remove
	affect_copy_to_obj(obj, paf); // add and modify holder
}

// removing

void affect_remove_from_obj(Object *obj, Affect *paf)
{
	affect_remove_from_list(&obj->affected, paf);
	affect_modify_obj(obj, paf, FALSE);
	free_affect(paf);
}

void affect_remove_matching_from_obj(Object *obj, affect_comparator comp, const Affect *pattern) {
	affect_fn_params params;

	params.owner = obj;
	params.modifier = affect_modify_obj;
	params.data = nullptr;

	affect_remove_matching_from_list(&obj->affected, comp, pattern, &params);
}

void affect_remove_marked_from_obj(Object *obj) {
	Affect pattern;
	pattern.mark = TRUE;

	affect_remove_matching_from_obj(obj, affect_comparator_mark, &pattern);
}

void affect_remove_sn_from_obj(Object *obj, int sn) {
	Affect pattern;
	pattern.type = sn;

	affect_remove_matching_from_obj(obj, affect_comparator_type, &pattern);
}

void affect_remove_all_from_obj(Object *obj, bool permanent)
{
	Affect pattern;
	pattern.permanent = permanent;

	affect_remove_matching_from_obj(obj, affect_comparator_permanent, &pattern);
}

// modifying

void affect_iterate_over_obj(Object *obj, affect_fn fn, void *data) {
	affect_fn_params params;

	params.owner = obj;
	params.modifier = affect_modify_obj;
	params.data = data;

	affect_iterate_over_list(&obj->affected, fn, &params);
}

void affect_sort_obj(Object *obj, affect_comparator comp) {
	affect_sort_list(&obj->affected, comp);
}

// utility

// test if an object has an affect
bool obj_has_affect(Object *obj, int sn) {
	return affect_find_in_list(&obj->affected, sn) ? TRUE : FALSE;
}

void affect_modify_flag_cache_obj(Object *obj, sh_int where, const Flags& flags, bool fAdd) {
	if (flags.empty())
		return;

	if (where != TO_OBJECT && where != TO_WEAPON)
		return;

	// if we're removing a bit, the only way to know for sure whether the object
	// should still have that bit (from a remaining affect) is to loop through
	// them all, so just rebuild the bit vectors
	if (!fAdd) {
		obj->cached_extra_flags.clear();
		obj->cached_weapon_flags.clear();

		for (const Affect *paf = obj->affected; paf; paf = paf->next)
			affect_modify_flag_cache_obj(obj, paf->where, paf->bitvector(), TRUE);

		for (const Affect *paf = obj->gem_affected; paf; paf = paf->next)
			affect_modify_flag_cache_obj(obj, paf->where, paf->bitvector(), TRUE);
	}
	else {
		switch (where) {
		case TO_OBJECT:
			obj->cached_extra_flags += flags;
			break;
		case TO_WEAPON:
			if (obj->item_type == ITEM_WEAPON)
				obj->cached_weapon_flags += flags;
			break;
		}
	}
}

// the modify function is called any time there is a potential change to the list of
// affects, and here we update any caches or entities that depend on the affect list.
// it is important that owner->affected reflects the new state of the affects, i.e.
// the Affect.hppas already been inserted or removed, and paf is not a member of the set.
void affect_modify_obj(void *owner, const Affect *paf, bool fAdd) {
	Object *obj = (Object *)owner;
	extern void affect_modify_char(void *owner, const Affect *paf, bool fAdd);

	affect_modify_flag_cache_obj(obj, paf->where, paf->bitvector(), fAdd);

	if (obj->carried_by && obj->wear_loc != WEAR_NONE)
		affect_modify_char(obj->carried_by, paf, fAdd);
}
