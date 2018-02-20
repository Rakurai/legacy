#include <vector>

#include "Character.hh"
#include "declare.hh"
#include "Format.hh"
#include "Logging.hh"
#include "merc.hh"
#include "String.hh"

namespace affect {

#define get_cache(ch) ((ch)->affect_cache == nullptr ? nullptr : (sh_int *)(ch)->affect_cache)
#define cache_size (int)::affect::type::size

void free_cache(Character *ch) {
	delete[] (sh_int *)ch->affect_cache;
	ch->affect_cache = nullptr;
}

bool in_cache(const Character *ch, ::affect::type type) {
	return (
		type > ::affect::type::none
	 && (int)type < cache_size
	 && ch->affect_cache
	 && get_cache(ch)[(int)type] > 0);
}

void update_cache(Character *ch, ::affect::type type, bool fAdd) {
	if (type <= ::affect::type::none /* 0 */ || (int)type >= cache_size) {
		Logging::bug("update_cache: called with type = %d", (int)type);
		return;
	}

	if (fAdd) {
		if (get_cache(ch) == nullptr) {
			ch->affect_cache = new sh_int[cache_size]; // every type

			for (int i = 0; i < cache_size; i++)
				get_cache(ch)[i] = 0;
		}

		get_cache(ch)[(int)type]++;
		get_cache(ch)[0]++;
	}
	else {
		if (get_cache(ch) == nullptr) {
			Logging::bugf("update_cache: illegal removal from nullptr affect cache at type %d (%s)",
				type, lookup(type).name);
			return;
		}

		if (get_cache(ch)[(int)type] == 0) {
			Logging::bugf("update_cache: illegal removal of uncounted value at type %d (%s)",
				type, lookup(type).name);
			return;
		}

		get_cache(ch)[(int)type]--;
		get_cache(ch)[0]--;

		if (get_cache(ch)[0] == 0)
			free_cache(ch);
	}
}

String print_cache(Character *ch) {
	String buf;

	if (ch->affect_cache == nullptr)
		return buf;

	for (int type_n = 1; type_n < cache_size; type_n++) {
		sh_int count = get_cache(ch)[type_n];

		if (count > 0) {
			if (!buf.empty())
				buf += " ";

			buf += Format::format("%s(%d)", lookup((::affect::type)type_n).name, count);
		}
	}

	return buf;
}

} // namespace affect
