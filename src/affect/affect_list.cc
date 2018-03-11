#include "affect/affect_list.hh"

namespace affect {

void remove_from_list(Affect **list_head, Affect *paf) {
	if (*list_head == paf)
		*list_head = paf->next;

	if (paf->prev)
		paf->prev->next = paf->next;
	if (paf->next)
		paf->next->prev = paf->prev;

	paf->next = nullptr;
	paf->prev = nullptr;
}

void insert_in_list(Affect **list_head, Affect *paf) {
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

void copy_to_list(Affect **list_head, const Affect *aff_template)
{
	Affect *paf_new = new Affect();
	*paf_new            = *aff_template;
	paf_new->next = nullptr;
	paf_new->prev = nullptr;
	insert_in_list(list_head, paf_new);
}

// remove all affects in list with same type and where, accumulating effects into paf for re-adding
void dedup_in_list(Affect **list_head, Affect *paf, fn_params *params)
{
	Affect *paf_old, *paf_next;

	for (paf_old = *list_head; paf_old != nullptr; paf_old = paf_next) {
		paf_next = paf_old->next;

		if (paf_old->type != paf->type
		 || paf_old->where != paf->where
		 || paf_old->location != paf->location
		 || (paf_old->duration == -1 && paf->duration != -1)
		 || (paf_old->duration != -1 && paf->duration == -1)
		 || paf_old->permanent != paf->permanent)
	 		continue;

		paf->level     = std::max(paf->level, paf_old->level);
		paf->duration  = std::max(paf->duration, paf_old->duration); // ok for -1 duration too
		paf->modifier  += paf_old->modifier;
		paf->bitvector(paf->bitvector() + paf_old->bitvector());
		paf->evolution = std::max(paf->evolution, paf_old->evolution);

		remove_from_list(list_head, paf_old);
		(params->modifier)(params->owner, paf_old, false);
		delete paf_old;
	}
}

void clear_list(Affect **list_head) {
	if (*list_head == nullptr)
		return;

	clear_list(&(*list_head)->next); // recurse
	delete *list_head;
	*list_head = nullptr;
}

const Affect *find_in_list(Affect **list_head, ::affect::type type) {
	for (const Affect *paf = *list_head; paf; paf = paf->next)
		if (paf->type == type)
			return paf;

	return nullptr;
}

void remove_matching_from_list(Affect **list_head, comparator comp, const Affect *pattern, fn_params *params) {
	Affect *paf, *paf_next;

	for (paf = *list_head; paf; paf = paf_next) {
		paf_next = paf->next;

		// never remove permanent affects unless explicitly instructed
		// removing nonpermanent affects is done with a permanent comparator below
		if (paf->permanent && (pattern == nullptr || !pattern->permanent))
			continue;

		if (comp == nullptr || (*comp)(paf, pattern) == 0) {
			remove_from_list(list_head, paf);
			(params->modifier)(params->owner, paf, false);
			delete paf;
		}
	}
}

void iterate_over_list(Affect **list_head, affect_fn fn, fn_params *params) {
	for (Affect *paf = *list_head; paf; paf = paf->next) {
		// unlink the item from the list to call the modifier function
		if (paf->prev) paf->prev->next = paf->next;
		if (paf->next) paf->next->prev = paf->prev;

		if (params->owner) (params->modifier)(params->owner, paf, false);

		(*fn)(paf, params->data); // should return value indicate break?

		// relink the item into the list
		if (paf->prev) paf->prev->next = paf;
		if (paf->next) paf->next->prev = paf;

		if (params->owner) (params->modifier)(params->owner, paf, true);
	}
}

unsigned long checksum_list(Affect **list_head) {
	unsigned long sum = 0;

	// this checksum is intentionally insensitive to order: a->b->c == a->c->b because of
	// the overflow property of unsigned integers, in that they behave as modulo.  therefore,
	// the unsigned property of the checksum is critical.
	for (const Affect *paf = *list_head; paf != nullptr; paf = paf->next)
		sum += checksum(paf);

	return sum;
}

void sort_list(Affect **list_head, comparator comp) {
	bool sorted = false;

	while (!sorted) {
		sorted = true;

		// go through the list, looking for unsorted items
		// TODO: there's a more efficient way to do this, we don't have to start at the beginning
		// with each iteration.  However, more important things to do right now, fix it later.
		for (Affect *paf = *list_head; paf; paf = paf->next) {
			if (paf->next == nullptr)
				break;

			if ((*comp)(paf, paf->next) > 0) { // bubble up
				swap(paf, paf->next); // note that list_head doesn't move, we swapped contents
				sorted = false;
			}
		}
	}
}

} // namespace affect
