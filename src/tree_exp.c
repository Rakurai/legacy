#include <stdio.h>
#include <stdlib.h>
#include "deps/cprops/splay.h"

typedef struct data {
	int sn;
} data;

int gsn_cmp(data *a, data *b) {
	return b->sn - a->sn;
}

int *copy_key(int *key) {
	int * new_key = malloc(sizeof(int));
	*new_key = *key;
	return new_key;
}

data *copy_val(data *val) {
	data *new_val = malloc(sizeof(data));
	*new_val = *val;
	return new_val;
}

int callback(void *entry, void *prm) {
	printf("in callback\n");
	(*(int *)prm)++;
	return 0;
}

int main() {
	cp_splaytree *t = cp_splaytree_create_by_option(
		COLLECTION_MODE_NOSYNC | COLLECTION_MODE_COPY | COLLECTION_MODE_DEEP | COLLECTION_MODE_MULTIPLE_VALUES,
		(cp_compare_fn) gsn_cmp,
		(cp_copy_fn) copy_key,
		(cp_destructor_fn) free,
		(cp_copy_fn) copy_val,
		(cp_destructor_fn) free);

	data d;
	d.sn = 5;

	cp_splaytree_insert(t, &d.sn, &d);
	cp_splaytree_insert(t, &d.sn, &d);

	printf("mappings: %d\n", cp_splaytree_count(t));

	int count = 0;
	cp_splaytree_callback(t, callback, &count);

	printf("nodes: %d\n", count);

	cp_splaytree_destroy(t);
}
