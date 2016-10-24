#include "merc.h"
#include "affect.h"
#include "affect_int.h"
#include "recycle.h"


AFFECT_DATA *affect_find_in_obj(OBJ_DATA *obj, int sn) {
	return affect_find_in_list(obj->perm_affected, sn);
}

/* give an affect to an object */
void affect_copy_to_obj(OBJ_DATA *obj, const AFFECT_DATA *template)
{
	affect_copy_to_list(&obj->perm_affected, template);
	affect_modify_obj(obj, template, TRUE);
}

void affect_join_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf) {
	affect_dedup_in_list(&obj->perm_affected, paf); // remove
	affect_modify_obj(obj, paf, FALSE); // modify holder
	affect_copy_to_obj(obj, paf); // add and modify holder
}

void affect_remove_from_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
	affect_remove_from_list(&obj->perm_affected, paf);
	affect_modify_obj(obj, paf, FALSE);
	free_affect(paf);
}

void affect_update_in_obj(OBJ_DATA *obj, AFFECT_DATA *original, const AFFECT_DATA *template)
{
	affect_modify_obj(obj, original, FALSE);
	affect_update(original, template);
	affect_modify_obj(obj, original, TRUE);
}

void affect_remove_sn_from_obj(OBJ_DATA *obj, int sn) {
	AFFECT_DATA *paf, *head = obj->perm_affected;

	while ((paf = affect_find_in_list(head, sn)) != NULL) {
		head = paf->next; // start next iteration here instead of looping through again
		affect_remove_from_obj(obj, paf);
	}
}

void affect_remove_all_from_obj(OBJ_DATA *obj)
{
	while (obj->perm_affected)
		affect_remove_from_obj(obj, obj->perm_affected);
}

// test if an object has an affect
bool obj_has_affect(OBJ_DATA *obj, int sn) {
	return affect_find_in_list(obj->perm_affected, sn) ? TRUE : FALSE;
}

void affect_modify_flag_cache_obj(OBJ_DATA *obj, sh_int where, unsigned int flags, bool fAdd) {
	if (flags == 0)
		return;

	if (where != TO_OBJECT && where != TO_WEAPON)
		return;

	// if we're removing a bit, the only way to know for sure whether the object
	// should still have that bit (from a remaining affect) is to loop through
	// them all, so just rebuild the bit vectors
	if (!fAdd) {
		obj->extra_flag_cache = 0;
		obj->weapon_flag_cache = 0;

		for (const AFFECT_DATA *paf = obj->affected; paf; paf = paf->next)
			affect_modify_flag_cache_obj(obj, paf->where, paf->bitvector, TRUE);

		for (const AFFECT_DATA *paf = obj->gem_affected; paf; paf = paf->next)
			affect_modify_flag_cache_obj(obj, paf->where, paf->bitvector, TRUE);
	}
	else {
		switch (where) {
		case TO_OBJECT:
			SET_BIT(obj->extra_flag_cache, flags);
			break;
		case TO_WEAPON:
			if (obj->item_type == ITEM_WEAPON)
				SET_BIT(obj->weapon_flag_cache, flags);
			break;
		}
	}
}

void affect_modify_obj(OBJ_DATA *obj, const AFFECT_DATA *paf, bool fAdd) {
	// set enchanted flag here.  this isnt technically always true, this could be a temp effect,
	// but i'm trying to simplify while deciding whether the affects should write to file.
	obj->enchanted = TRUE;

	switch (paf->where) {
	case TO_OBJECT:
	case TO_WEAPON:
		affect_modify_flag_cache_obj(obj, paf->where, paf->bitvector, fAdd);
		break;
	default:
		if (obj->carried_by && obj->wear_loc != WEAR_NONE)
			affect_modify_char(obj->carried_by, paf, fAdd);
	}
}
