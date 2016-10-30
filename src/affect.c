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

// calculate a checksum over the important parts of the AFFECT_DATA structure, for
// determining whether a list of affects is different from another list.  This is
// Bernstein's djb2 algorithm, from http://www.cse.yorku.ca/~oz/hash.html
unsigned long affect_checksum(const AFFECT_DATA *paf) {
	const unsigned char *str = (const unsigned char *)paf;

	// start checksum at data values
	int start = 0           // 0-index
	 + sizeof(AFFECT_DATA *) // next
	 + sizeof(AFFECT_DATA *) // prev
	 + sizeof(bool)          // valid
	 + sizeof(bool);         // mark
	int end = sizeof(AFFECT_DATA);

	unsigned long hash = 5381;

	// this checksum is intentionally *NOT* insensitive to order of fields/values
	for (int i = start; i < end; i++)
		hash = ((hash << 5) + hash) + str[i]; // hash * 33 + c

	return hash;
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

