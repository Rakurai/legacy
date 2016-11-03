#include "merc.h"
#include "affect.h"
#include "affect_list.h"
#include "recycle.h"
#include "tables.h"

// local declarations
void affect_modify_char(void *owner, const AFFECT_DATA *paf, bool fAdd);

// flags

void affect_flag_add_to_char(CHAR_DATA *ch, unsigned int flag) {
//	SET_BIT(ch->affected_by, flag);
}

void affect_flag_remove_from_char(CHAR_DATA *ch, unsigned int flag) {
//	REMOVE_BIT(ch->affected_by, flag);
}

unsigned int affect_flag_get_char(CHAR_DATA *ch) {
//	return ch->affected_by;
}

void affect_flag_clear_char(CHAR_DATA *ch) {
//	ch->affected_by = 0;
}

bool affect_flag_on_char(CHAR_DATA *ch, unsigned int flag) {
//	return IS_SET(ch->affected_by, flag);
}

// searching

const AFFECT_DATA *affect_list_char(CHAR_DATA *ch) {
	return ch->affected;
}

const AFFECT_DATA *affect_find_in_char(CHAR_DATA *ch, int sn) {
	return affect_find_in_list(&ch->affected, sn);
}

// adding

void affect_copy_to_char(CHAR_DATA *ch, const AFFECT_DATA *template)
{
	affect_copy_to_list(&ch->affected, template);
	affect_modify_char(ch, template, TRUE);
}

void affect_join_to_char(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	affect_fn_params params;

	params.owner = ch;
	params.modifier = affect_modify_char;
	params.data = NULL;

	affect_dedup_in_list(&ch->affected, paf, &params);
	affect_copy_to_char(ch, paf);
}

void affect_add_perm_to_char(CHAR_DATA *ch, int sn) {
	AFFECT_DATA af = (AFFECT_DATA){0};
	af.type = sn;
	af.where = TO_AFFECTS;
	af.level = -1;
	af.duration = -1;
	af.evolution = 1;

	affect_copy_to_char(ch, &af);
}

void affect_copy_flags_to_char(CHAR_DATA *ch, char letter, unsigned int bitvector) {
	AFFECT_DATA af;
	af.level = -1; // permanent
	af.duration = -1;
	af.evolution = 1;

	while (bitvector != 0) {
		af.type = 0; // reset every time
		if (affect_parse_prototype(letter, &af, &bitvector))
			affect_copy_to_char(ch, &af);
	}
}

// removing

void affect_remove_from_char(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	affect_remove_from_list(&ch->affected, paf);
	affect_modify_char(ch, paf, FALSE);
	free_affect(paf);
}

void affect_remove_matching_from_char(CHAR_DATA *ch, affect_comparator comp, const AFFECT_DATA *pattern) {
	affect_fn_params params;

	params.owner = ch;
	params.modifier = affect_modify_char;
	params.data = NULL;

	affect_remove_matching_from_list(&ch->affected, comp, pattern, &params);
}

void affect_remove_marked_from_char(CHAR_DATA *ch) {
	AFFECT_DATA pattern;
	pattern.mark = TRUE;

	affect_remove_matching_from_char(ch, affect_comparator_mark, &pattern);
}

void affect_remove_sn_from_char(CHAR_DATA *ch, int sn) {
	AFFECT_DATA pattern;
	pattern.type = sn;

	affect_remove_matching_from_char(ch, affect_comparator_type, &pattern);
}

void affect_remove_all_from_char(CHAR_DATA *ch) {
	affect_remove_matching_from_char(ch, NULL, NULL);
}

// modifying

void affect_iterate_over_char(CHAR_DATA *ch, affect_fn fn, void *data) {
	affect_fn_params params;

	params.owner = ch;
	params.modifier = affect_modify_char;
	params.data = data;

	affect_iterate_over_list(&ch->affected, fn, &params);
}

void affect_sort_char(CHAR_DATA *ch, affect_comparator comp) {
	affect_sort_list(&ch->affected, comp);
}

// utility

void remort_affect_modify_char(CHAR_DATA *ch, int where, unsigned int bits, bool fAdd) {
	AFFECT_DATA af;
	af.type = 0;
	af.level = -1;
	af.duration = -1;
	af.evolution = 1;
	// where, location and modifier will be filled by parse

	char letter = 
		where == TO_RESIST ? 'R' : 
		where == TO_VULN ? 'V' : '?'; // let parse handle error

	while (affect_parse_prototype(letter, &af, &bits))
		affect_modify_char(ch, &af, fAdd);
}

/* hinges on af.where:
	where        type       location  modifier
	TO_OBJECT    ignore     attrmod   amount
    TO_AFFECTS   sn,cache   attrmod   amount
    TO_DEFENSE   ignore     defmod    amount
*/

// the modify function is called any time there is a potential change to the list of
// affects, and here we update any caches or entities that depend on the affect list.
// it is important that owner->affected reflects the new state of the affects, i.e.
// the affect has already been inserted or removed, and paf is not a member of the set.
void affect_modify_char(void *owner, const AFFECT_DATA *paf, bool fAdd) {
	CHAR_DATA *ch = (CHAR_DATA *)owner;

	if (paf->where != TO_DEFENSE && paf->where != TO_AFFECTS && paf->where != TO_OBJECT)
		return;

	if (paf->where == TO_DEFENSE) {
		if (paf->location < 1 || paf->location > 32) {
			bugf("affect_modify_char: bad location %d in TO_DEFENSE", paf->location);
			return;
		}

		if (paf->modifier == 0)
			return;

		if (fAdd) {
			if (ch->defense_mod == NULL) {
				ch->defense_mod = alloc_mem(DEFENSE_MOD_MEM_SIZE);
				memset(ch->defense_mod, 0, DEFENSE_MOD_MEM_SIZE);
			}

			ch->defense_mod[0]++;
			ch->defense_mod[paf->location] += paf->modifier;
		}
		else {
			if (ch->defense_mod == NULL) {
				bug("affect_modify_char: attempt to remove from NULL defense_mod", 0);
				return;
			}

			ch->defense_mod[0]--;
			ch->defense_mod[paf->location] -= paf->modifier;

			if (ch->defense_mod[0] == 0) {
				free_mem(ch->defense_mod, DEFENSE_MOD_MEM_SIZE);
				ch->defense_mod = NULL;
			}
		}

		return;
	}

	if (paf->where == TO_AFFECTS) {
		if (paf->type < 1 || paf->type >= MAX_SKILL) {
			bugf("affect_modify_char: bad type %d in TO_AFFECTS", paf->type);
			return;
		}

		update_affect_cache(ch, paf->type, fAdd);
	}

	// both TO_OBJECT and TO_AFFECTS can set attribute mods

	// affect makes no mods?  we're done
	if (paf->modifier == 0)
		return;

	if (paf->location != APPLY_NONE && paf->modifier != 0) {
		if (paf->location < 1 || paf->location > 32) {
			bugf("affect_modify_char: bad location %d when modifier is %d", paf->location, paf->modifier);
			return;
		}

		if (fAdd) {
			if (ch->apply_cache == NULL) {
				ch->apply_cache = alloc_mem(APPLY_CACHE_MEM_SIZE);
				memset(ch->apply_cache, 0, APPLY_CACHE_MEM_SIZE);
			}

			ch->apply_cache[0]++;
			ch->apply_cache[paf->location] += paf->modifier;
		}
		else {
			if (ch->apply_cache == NULL) {
				bug("affect_modify_char: attempt to remove from NULL apply_cache", 0);
				return;
			}

			ch->apply_cache[paf->location] -= paf->modifier;

			if (--ch->apply_cache[0] <= 0) {
				free_mem(ch->apply_cache, APPLY_CACHE_MEM_SIZE);
				ch->apply_cache = NULL;
			}
		}
	}
}
