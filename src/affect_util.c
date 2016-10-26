#include "merc.h"
#include "affect.h"

// various reusable utilities for affects that don't belong elsewhere, such as
// callback functions for iterating over affect lists.

int debug_aff_callback(AFFECT_DATA *node, void *data) {
	int *count = (int *)data;
//	AFFECT_DATA *node = (AFFECT_DATA *)node;

	(*count)++;
	bugf("callback %d in affect %d", *count, node->type);
	return 0;
}
