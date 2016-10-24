#include "merc.h"
#include "affect.h"
#include "recycle.h"

void affect_remove_from_list(AFFECT_DATA **list_head, AFFECT_DATA *paf) {
	if (*list_head == paf)
		*list_head = paf->next;

	if (paf->prev)
		paf->prev->next = paf->next;
	if (paf->next)
		paf->next->prev = paf->prev;

	paf->next = NULL;
	paf->prev = NULL;
}

void affect_insert_in_list(AFFECT_DATA **list_head, AFFECT_DATA *paf) {
	if (*list_head) {
		if ((*list_head)->prev) {
			(*list_head)->prev->next = paf;
			paf->prev = (*list_head)->prev;
		}

		(*list_head)->prev = paf;
	}

	paf->next = *list_head;
	*list_head = paf;
}

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

/*
 * Add a new affect.
 */
void affect_copy_to_list(AFFECT_DATA **list_head, const AFFECT_DATA *template)
{
	AFFECT_DATA *paf_new = new_affect();
	*paf_new            = *template;
	affect_insert_in_list(list_head, paf_new);
}

// remove all affects in list with same type and where, accumulating effects into paf for re-adding
void affect_dedup_in_list(AFFECT_DATA **list_head, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_old, *paf_next;

	for (paf_old = *list_head; paf_old != NULL; paf_old = paf_next) {
		paf_next = paf_old->next;

		if (paf_old->type != paf->type
		 || paf_old->where != paf->where
		 || paf_old->location != paf->location
		 || (paf_old->duration == -1 && paf->duration != -1)
		 || (paf_old->duration != -1 && paf->duration == -1))
	 		continue;

		paf->level     = UMAX(paf->level, paf_old->level);
		paf->duration  = UMAX(paf->duration, paf_old->duration); // ok for -1 duration too
		paf->modifier  += paf_old->modifier;
		paf->bitvector |= paf_old->bitvector;
		paf->evolution = UMAX(paf->evolution, paf_old->evolution);
		affect_remove_from_list(list_head, paf_old);
		free_affect(paf_old);
	}
}

void affect_clear_list(AFFECT_DATA **list_head) {
	if (*list_head == NULL)
		return;

	affect_clear_list(&(*list_head)->next); // recurse
	free_affect(*list_head);
	*list_head = NULL;
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

AFFECT_DATA *affect_find_in_list(AFFECT_DATA *list_head, int sn) {
	for (AFFECT_DATA *paf = list_head; paf; paf = paf->next)
		if (paf->type == sn)
			return paf;

	return NULL;
}

// perform the error checking in building affects from a prototype, including bits.
// fills an AFFECT_DATA struct with one of the bits, removes the bit from
// the vector.  return value indicates whether the struct is valid to insert.
// assumes type, level, duration, evolution, location and modifier already filled,
// but alters if appropriate
bool affect_parse_prototype(char letter, AFFECT_DATA *paf, unsigned int *bitvector) {
	// weird case, defense flag A used to be *_SUMMON, changed to a ACT_NOSUMMON.
	// wouldn't be a concern except the index is repurposed to a cache counter.
	if (paf->where == TO_DEFENSE)
		REMOVE_BIT(*bitvector, A);

	unsigned int bit = 1;
	int index = 0;

	while (index < 32 && !IS_SET(bit, *bitvector)) {
		bit <<= 1;
		index++;
	}

	if (index < 32)
		REMOVE_BIT(*bitvector, bit);

	// if the bit wasn't found, still continue for the TO_OBJECT.  the loop will
	// stop when bitvector is 0

	switch (letter) {
	case 'O': paf->where = TO_OBJECT; break; // location and modifier already set
	case 'A': paf->where = TO_AFFECTS; break; // location and modifier already set
	case 'D': paf->where = TO_DEFENSE; break; // modifier already set
	case 'I': paf->where = TO_DEFENSE; paf->modifier = 100; break;
	case 'R': paf->where = TO_DEFENSE; paf->modifier = 50; break;
	case 'V': paf->where = TO_DEFENSE; paf->modifier = -50; break;
	default:
		bugf("affect_parse_prototype: bad letter %c", letter);
		return FALSE;
	}

	if (paf->where == TO_DEFENSE) {
		if (index < 1 || index > 32) // no bits, not an error, just skip it
			return FALSE;

		paf->location = index;
		return TRUE;
	}

	if (paf->where == TO_AFFECTS) {
		int sn = affect_bit_to_sn(bit);
		if (sn <= 0) {
			bugf("affect_parse_prototype: sn not found for bit %d in TO_AFFECTS", bit);
			return FALSE;
		}

		paf->type = sn;
		// drop down to applies
	}

	if (paf->location < 1 || paf->location > 32) {
		bugf("affect_parse_prototype: bad location %d", paf->location);
		return FALSE;
	}

	return TRUE;
}
