#include "merc.h"
#include "affect.h"
#include "Format.hpp"

#define get_affect_cache(ch) ((ch)->affect_cache == NULL ? NULL : (sh_int *)(ch)->affect_cache)

void free_affect_cache(CHAR_DATA *ch) {
	delete[] (sh_int *)ch->affect_cache;
	ch->affect_cache = NULL;
}

bool affect_in_cache(const CHAR_DATA *ch, sh_int sn) {
	return (
		sn > 1
	 && sn < MAX_SKILL
	 && ch->affect_cache
	 && get_affect_cache(ch)[sn] > 0);
}

void update_affect_cache(CHAR_DATA *ch, sh_int sn, bool fAdd) {
	if (sn < 1 || sn >= MAX_SKILL) {
		bug("update_affect_cache: called with sn = %d", sn);
		return;
	}

	if (fAdd) {
		if (get_affect_cache(ch) == NULL) {
			ch->affect_cache = new sh_int[MAX_SKILL];

			for (int i = 0; i < MAX_SKILL; i++)
				get_affect_cache(ch)[i] = 0;
		}

		get_affect_cache(ch)[sn]++;
		get_affect_cache(ch)[0]++;
	}
	else {
		if (get_affect_cache(ch) == NULL) {
			bugf("update_affect_cache: illegal removal from NULL affect cache at sn %d (%s)",
				sn, skill_table[sn].name ? skill_table[sn].name : "");
			return;
		}

		if (get_affect_cache(ch)[sn] == 0) {
			bugf("update_affect_cache: illegal removal of uncounted value at sn %d (%s)",
				sn, skill_table[sn].name ? skill_table[sn].name : "");
			return;
		}

		get_affect_cache(ch)[sn]--;
		get_affect_cache(ch)[0]--;

		if (get_affect_cache(ch)[0] == 0)
			free_affect_cache(ch);
	}
}

String affect_print_cache(CHAR_DATA *ch) {
	String buf;

	if (ch->affect_cache == NULL)
		return buf;

	for (int sn = 1; sn < MAX_SKILL; sn++) {
		sh_int count = get_affect_cache(ch)[sn];

		if (count > 0) {
			if (buf[0] != '\0')
				buf += " ";

			Format::sprintf(buf, "%s%s(%d)", buf, skill_table[sn].name, count);
//			str += skill_table[sn].name;
		}
	}

	return buf;
}
