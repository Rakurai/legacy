#include "merc.h"
#include "affect.h"

// various reusable utilities for affects that don't belong elsewhere, such as
// callback functions for iterating over affect lists.

int affect_fn_debug(AFFECT_DATA *node, void *data) {
	int *count = (int *)data;
//	AFFECT_DATA *node = (AFFECT_DATA *)node;

	(*count)++;
	bugf("callback %d in affect %d", *count, node->type);
	return 0;
}

int affect_fn_fade_spell(AFFECT_DATA *node, void *data) {
	sh_int sn = 0;

	if (data != NULL)
		sn = *(sh_int *)data;

	if (node->duration > 0
	 && (sn <= 0 || node->type == sn)) {
		node->duration--;

		if (node->level > 0 && number_range(0, 4))
			node->level--;  /* spell strength fades with time */
	}

	return 0; // keep going
}

int affect_comparator_mark(const AFFECT_DATA *lhs, const AFFECT_DATA *rhs) {
	return lhs->mark - rhs->mark;
}

int affect_comparator_duration(const AFFECT_DATA *lhs, const AFFECT_DATA *rhs) {
	return lhs->duration - rhs->duration;
}

int affect_comparator_type(const AFFECT_DATA *lhs, const AFFECT_DATA *rhs) {
	return lhs->type - rhs->type;
}
