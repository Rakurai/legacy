#include <vector>

#include "Character.hh"
#include "declare.hh"
#include "Format.hh"
#include "Logging.hh"
#include "merc.hh"
#include "String.hh"

#define get_affect_cache(ch) ((ch)->affect_cache == nullptr ? nullptr : (sh_int *)(ch)->affect_cache)

void free_affect_cache(Character *ch) {
	delete[] (sh_int *)ch->affect_cache;
	ch->affect_cache = nullptr;
}

bool affect_in_cache(const Character *ch, sh_int sn) {
	return (
		sn > 1
	 && sn < skill_table.size()
	 && ch->affect_cache
	 && get_affect_cache(ch)[sn] > 0);
}

void update_affect_cache(Character *ch, sh_int sn, bool fAdd) {
	if (sn < 1 || sn >= skill_table.size()) {
		Logging::bug("update_affect_cache: called with sn = %d", sn);
		return;
	}

	if (fAdd) {
		if (get_affect_cache(ch) == nullptr) {
			ch->affect_cache = new sh_int[skill_table.size()];

			for (int i = 0; i < skill_table.size(); i++)
				get_affect_cache(ch)[i] = 0;
		}

		get_affect_cache(ch)[sn]++;
		get_affect_cache(ch)[0]++;
	}
	else {
		if (get_affect_cache(ch) == nullptr) {
			Logging::bugf("update_affect_cache: illegal removal from nullptr affect cache at sn %d (%s)",
				sn, skill_table[sn].name);
			return;
		}

		if (get_affect_cache(ch)[sn] == 0) {
			Logging::bugf("update_affect_cache: illegal removal of uncounted value at sn %d (%s)",
				sn, skill_table[sn].name);
			return;
		}

		get_affect_cache(ch)[sn]--;
		get_affect_cache(ch)[0]--;

		if (get_affect_cache(ch)[0] == 0)
			free_affect_cache(ch);
	}
}

String affect_print_cache(Character *ch) {
	String buf;

	if (ch->affect_cache == nullptr)
		return buf;

	for (int sn = 1; sn < skill_table.size(); sn++) {
		sh_int count = get_affect_cache(ch)[sn];

		if (count > 0) {
			if (buf[0] != '\0')
				buf += " ";

			buf += Format::format("%s(%d)", skill_table[sn].name, count);
//			str += skill_table[sn].name;
		}
	}

	return buf;
}
