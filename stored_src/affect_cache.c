#include "merc.h"
#include "affect.h"
#include "deps/cprops/splay.h"

#define get_affect_cache(ch) ((ch)->affect_cache == NULL ? NULL : (cp_splaytree *)(ch)->affect_cache)

int gsn_compare(void *lhs, void *rhs) {
	return *(int *)rhs - *(int *)lhs;
//	int ilhs = *(int *)lhs, irhs = *(int *)rhs;
//	return irhs == ilhs ? 1 : irhs - ilhs;
}

int *copy_int(int *key) {
	int *new_key = new int;
	*new_key = *key;
	return new_key;
}

void free_affect_cache(CHAR_DATA *ch) {
	cp_splaytree_destroy(get_affect_cache(ch));
}

bool affect_in_cache(CHAR_DATA *ch, sh_int sn) {
	int *count = cp_splaytree_get(get_affect_cache(ch), &sn);

	return
		count != NULL
	 && *count > 0;
}

void update_affect_cache(CHAR_DATA *ch, sh_int sn, bool fAdd) {
	if (sn < 1 || sn >= MAX_SKILL) {
		bug("update_affect_cache: called with sn = %d", sn);
		return;
	}

	if (fAdd) {
		if (get_affect_cache(ch) == NULL) {
			ch->affect_cache = cp_splaytree_create_by_option(
				COLLECTION_MODE_NOSYNC | COLLECTION_MODE_COPY | COLLECTION_MODE_DEEP,
				(cp_compare_fn) gsn_compare,
				(cp_copy_fn) copy_int,
				(cp_destructor_fn) delete,
				(cp_copy_fn) copy_int,
				(cp_destructor_fn) delete);
		}

		// insert copies for both key and value, it makes our print work later
		int *count = cp_splaytree_get(get_affect_cache(ch), &sn);

		if (count == NULL) {
			int one = 1;
			cp_splaytree_insert(get_affect_cache(ch), &sn, &one);
		}
		else
			(*count)++;
	}
	else {
		if (get_affect_cache(ch) == NULL) {
			bugf("update_affect_cache: illegal removal from NULL affect cache at sn %d (%s)",
				sn, skill_table[sn].name ? skill_table[sn].name : "");
			return;
		}

		int *count = cp_splaytree_get(get_affect_cache(ch), &sn);

		if (count == NULL) {
			bugf("update_affect_cache: illegal removal of uncounted value at sn %d (%s)",
				sn, skill_table[sn].name ? skill_table[sn].name : "");
			return;
		}

		(*count)--;

		if (*count == 0)
			cp_splaytree_delete(get_affect_cache(ch), &sn);

		if (cp_splaytree_count(get_affect_cache(ch)) == 0) {
			cp_splaytree_destroy(get_affect_cache(ch));
			ch->affect_cache = NULL;
		}
	}
}

int affect_print_cache_callback(void *entry, void *prm) {
	cp_splaynode *node = entry;
	const int *sn = (int *)(node->key);
	int *count = (int *)(node->value);
	char *str = (char *)prm;

	if (*sn < 0 || *sn > MAX_SKILL)
		return 0;

	if (str[0] != '\0')
		str += " ";

	Format::sprintf(str, "%s%s(%d)", str, skill_table[*sn].name, *count);
//	str += skill_table[sn].name;

	return 0;
}

char *affect_print_cache(CHAR_DATA *ch) {
	static char buf[MSL];
	buf[0] = '\0';

	if (ch->affect_cache != NULL)
		cp_splaytree_callback(get_affect_cache(ch), affect_print_cache_callback, buf);

	return buf;
}
