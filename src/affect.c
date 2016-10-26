#include "merc.h"
#include "affect.h"
#include "recycle.h"
#include "affect_int.h"

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
	paf_new->next = NULL;
	paf_new->prev = NULL;
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

void affect_iterate_over_list(AFFECT_DATA *list_head, affect_callback_wrapper fn, affect_callback_params *params) {
	for (AFFECT_DATA *paf = list_head; paf; paf = paf->next)
		if ((*fn)(paf, params) != 0)
			break;
}
