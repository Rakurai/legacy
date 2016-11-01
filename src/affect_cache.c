#include "merc.h"
#include "affect.h"
#include "deps/cprops/splay.h"

#define get_affect_cache(ch) ((cp_splaytree *)(ch)->affect_cache)

// for splay tree below
// equal numbers still returns 1, so we can store duplicates.  bug with splaytree
// implementation, disregards multiple values flag
int gsn_compare(void *lhs, void *rhs) {
	int ilhs = *(int *)lhs, irhs = *(int *)rhs;
	return irhs == ilhs ? 1 : irhs - ilhs;
}

int *copy_int(int *key) {
	int *new_key = malloc(sizeof(int));
	*new_key = *key;
	return new_key;
}

void update_affect_cache(CHAR_DATA *ch, sh_int sn, bool fAdd) {
	if (fAdd) {
		if (get_affect_cache(ch) == NULL) {
			ch->affect_cache = cp_splaytree_create_by_option(
				COLLECTION_MODE_NOSYNC | COLLECTION_MODE_COPY | COLLECTION_MODE_DEEP | COLLECTION_MODE_MULTIPLE_VALUES,
				(cp_compare_fn) gsn_compare,
				(cp_copy_fn) copy_int,
				(cp_destructor_fn) free,
				(cp_copy_fn) copy_int,
				(cp_destructor_fn) free);
		}

		// insert copies for both key and value, it makes our print work later
		cp_splaytree_insert(get_affect_cache(ch), &sn, &sn);
	}
	else {
		cp_splaytree_delete(get_affect_cache(ch), &sn);

		if (cp_splaytree_count(get_affect_cache(ch)) == 0) {
			cp_splaytree_destroy(get_affect_cache(ch));
			ch->affect_cache = NULL;
		}
	}
}

int affect_print_cache_callback(void *entry, void *prm) {
	static int last_sn = 0;
	int sn = *(int *)entry;
	char *str = (char *)prm;

	if (sn != last_sn) {
		if (str[0] != '\0')
			strcat(str, " ");

		strcat(str, skill_table[sn].name);
		last_sn = sn;
	}

	return 0;
}

char *affect_print_cache(CHAR_DATA *ch) {
	static char buf[MSL];
	buf[0] = '\0';

	if (ch->affect_cache != NULL)
		cp_splaytree_callback(get_affect_cache(ch), affect_print_cache_callback, buf);

	return buf;
}
