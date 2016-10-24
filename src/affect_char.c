#include "merc.h"
#include "affect.h"
#include "affect_int.h"
#include "recycle.h"
#include "tables.h"


AFFECT_DATA *affect_find_in_char(CHAR_DATA *ch, int sn) {
	return affect_find_in_list(ch->affected, sn);
}

void affect_copy_to_char(CHAR_DATA *ch, const AFFECT_DATA *template)
{
	affect_copy_to_list(&ch->affected, template);
	affect_modify_char(ch, template, TRUE);
}

void affect_join_to_char(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	affect_dedup_in_list(&ch->affected, paf);
	affect_modify_char(ch, paf, FALSE);
	affect_copy_to_char(ch, paf);
}

void affect_remove_from_char(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	affect_remove_from_list(&ch->affected, paf);
	affect_modify_char(ch, paf, FALSE);
	free_affect(paf);
}

void affect_update_in_char(CHAR_DATA *ch, AFFECT_DATA *original, const AFFECT_DATA *template)
{
	affect_modify_char(ch, original, FALSE);
	affect_update(original, template);
	affect_modify_char(ch, original, TRUE);
}

void affect_remove_sn_from_char(CHAR_DATA *ch, int sn) {
	AFFECT_DATA *paf, *head = ch->affected;

	while ((paf = affect_find_in_list(head, sn)) != NULL) {
		head = paf->next; // start next iteration here instead of looping through again
		affect_remove_from_char(ch, paf);
	}
}

void affect_remove_all_from_char(CHAR_DATA *ch) {
	while (ch->affected)
		affect_remove_from_char(ch, ch->affected);
}

void affect_add_perm_to_char(CHAR_DATA *ch, int sn) {
	AFFECT_DATA af;
	af.type = sn;
	af.where = TO_AFFECTS;
	af.level = -1;
	af.duration = -1;
	af.evolution = 1;

	affect_copy_to_char(ch, &af);
}

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

void update_affect_cache(CHAR_DATA *ch, int sn, bool fAdd) {
	if (fAdd) {
		if (ch->affect_cache == NULL) {
			ch->affect_cache = cp_splaytree_create_by_option(
				COLLECTION_MODE_NOSYNC | COLLECTION_MODE_COPY | COLLECTION_MODE_DEEP | COLLECTION_MODE_MULTIPLE_VALUES,
				(cp_compare_fn) gsn_compare,
				(cp_copy_fn) copy_int,
				(cp_destructor_fn) free,
				(cp_copy_fn) copy_int,
				(cp_destructor_fn) free);
		}

		// insert copies for both key and value, it makes our print work later
		cp_splaytree_insert(ch->affect_cache, &sn, &sn);
	}
	else {
		cp_splaytree_delete(ch->affect_cache, &sn);

		if (cp_splaytree_count(ch->affect_cache) == 0) {
			cp_splaytree_destroy(ch->affect_cache);
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
		cp_splaytree_callback(ch->affect_cache, affect_print_cache_callback, buf);

	return buf;
}

bool is_affected(CHAR_DATA *ch, int sn) {
	return ch->affect_cache && cp_splaytree_contains(ch->affect_cache, &sn);
}

/*
 * Apply or remove an affect to a character.
 */

#define DEFENSE_MOD_MEM_SIZE (sizeof(sh_int) * 32)
#define APPLY_CACHE_MEM_SIZE (sizeof(int) * MAX_ATTR)
/* hinges on af.where:
	where        type       location  modifier
	TO_OBJECT    ignore     attrmod   amount
    TO_AFFECTS   sn,cache   attrmod   amount
    TO_DEFENSE   ignore     defmod    amount
*/
void affect_modify_char(CHAR_DATA *ch, const AFFECT_DATA *paf, bool fAdd)
{
	if (paf->where != TO_DEFENSE && paf->where != TO_AFFECTS && paf->where != TO_OBJECT) {
		bugf("affect_modify_char: bad where %d", paf->where);
		return;
	}

	if (paf->where == TO_DEFENSE) {
		if (paf->location < 1 || paf->location > 32) {
			bugf("affect_modify_char: bad location %d in TO_DEFENSE", paf->location);
			return;
		}

		if (paf->modifier == 0)
			return;

		if (fAdd) {
			if (ch->defense_mod == NULL) {
				ch->defense_mod = alloc_mem(DEFENSE_MOD_MEM_SIZE);
				memset(ch->defense_mod, 0, DEFENSE_MOD_MEM_SIZE);
			}

			ch->defense_mod[0]++;
			ch->defense_mod[paf->location] += paf->modifier;
		}
		else {
			ch->defense_mod[0]--;
			ch->defense_mod[paf->location] -= paf->modifier;

			if (ch->defense_mod[0] == 0) {
				free_mem(ch->defense_mod, DEFENSE_MOD_MEM_SIZE);
				ch->defense_mod = NULL;
			}
		}

		return;
	}

	if (paf->where == TO_AFFECTS) {
		if (paf->type < 1 || paf->type > MAX_SKILL) {
			bugf("affect_modify_char: bad type %d in TO_AFFECTS", paf->type);
			return;
		}

		update_affect_cache(ch, paf->type, fAdd);
	}

	// both TO_OBJECT and TO_AFFECTS can set attribute mods

	// affect makes no mods?  we're done
	if (paf->modifier == 0)
		return;

	if (paf->location != APPLY_NONE && paf->modifier != 0) {
		if (paf->location < 1 || paf->location > 32) {
			bugf("affect_modify_char: bad location %d when modifier is %d", paf->location, paf->modifier);
			return;
		}

		if (fAdd) {
			if (ch->apply_cache == NULL) {
				ch->apply_cache = alloc_mem(APPLY_CACHE_MEM_SIZE);
				memset(ch->apply_cache, 0, APPLY_CACHE_MEM_SIZE);
			}

			ch->apply_cache[0]++;
			ch->apply_cache[paf->location] += paf->modifier;
		}
		else {
			ch->apply_cache[paf->location] -= paf->modifier;

			if (--ch->apply_cache[0] <= 0) {
				free_mem(ch->apply_cache, APPLY_CACHE_MEM_SIZE);
				ch->apply_cache = NULL;
			}
		}
	}
} /* end affect_modify() */


