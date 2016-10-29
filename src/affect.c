#include "merc.h"
#include "affect.h"


// reusable callback functions

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


// comparators

int affect_comparator_mark(const AFFECT_DATA *lhs, const AFFECT_DATA *rhs) {
	return lhs->mark - rhs->mark;
}

int affect_comparator_duration(const AFFECT_DATA *lhs, const AFFECT_DATA *rhs) {
	return lhs->duration - rhs->duration;
}

int affect_comparator_type(const AFFECT_DATA *lhs, const AFFECT_DATA *rhs) {
	return lhs->type - rhs->type;
}


// affect utilities

void affect_update(AFFECT_DATA *paf, const AFFECT_DATA *template) {
	paf->type = template->type;
	paf->where = template->where;
	paf->location = template->location;
	paf->duration = template->duration;
	paf->level = template->level;
	paf->modifier = template->modifier;
	paf->bitvector = template->bitvector;
	paf->evolution = template->evolution;
}

void affect_swap(AFFECT_DATA *a, AFFECT_DATA *b) {
	if (a == NULL || b == NULL)
		return;

	// we could go through a complicated mechanism for swapping nodes in a double
	// linked list, handling both the adjacent and non-adjacent cases, and being
	// correct... or, screw it, swapping the data is easier. -- Montrey
	AFFECT_DATA t;
	t = *a; // copy, including pointers
	*a = *b;
	*b = t;
	// a and b are swapped, t holds copy of original a
	b->next = a->next; // get correct pointers back into b
	b->prev = a->prev;
	a->next = t.next; // get correct pointers back into a
	a->prev = t.prev;
}

