#include <vector>

#include "affect/Affect.hh"
#include "Character.hh"
#include "Logging.hh"

namespace affect {

#define GET_CACHE(ch) ((ch)->affect_cache == nullptr ? nullptr : (int *)(ch)->affect_cache)
#define CACHE_SIZE (int)::affect::type::tsize

void free_cache(Character *ch) {
	delete[] (int *)ch->affect_cache;
	ch->affect_cache = nullptr;
}

bool in_cache(const Character *ch, ::affect::type type) {
	return (
		type > ::affect::type::none
	 && (int)type < CACHE_SIZE
	 && ch->affect_cache
	 && GET_CACHE(ch)[(int)type] > 0);
}

void update_cache(Character *ch, ::affect::type type, bool fAdd) {
	if (type <= ::affect::type::none /* 0 */ || (int)type >= CACHE_SIZE) {
		Logging::bug("update_cache: called with type = %d", (int)type);
		return;
	}

	if (fAdd) {
		if (GET_CACHE(ch) == nullptr) {
			ch->affect_cache = new int[CACHE_SIZE]; // every type

			for (int i = 0; i < CACHE_SIZE; i++)
				GET_CACHE(ch)[i] = 0;
		}

		GET_CACHE(ch)[(int)type]++;
		GET_CACHE(ch)[0]++;
	}
	else {
		if (GET_CACHE(ch) == nullptr) {
			Logging::bugf("update_cache: illegal removal from nullptr affect cache at type %d (%s)",
				type, lookup(type).name);
			return;
		}

		if (GET_CACHE(ch)[(int)type] == 0) {
			Logging::bugf("update_cache: illegal removal of uncounted value at type %d (%s)",
				type, lookup(type).name);
			return;
		}

		GET_CACHE(ch)[(int)type]--;
		GET_CACHE(ch)[0]--;

		if (GET_CACHE(ch)[0] == 0)
			free_cache(ch);
	}
}

String print_cache(Character *ch) {
	String buf;

	if (ch->affect_cache == nullptr)
		return buf;

	for (int type_n = 1; type_n < CACHE_SIZE; type_n++) {
		int count = GET_CACHE(ch)[type_n];

		if (count > 0) {
			if (!buf.empty())
				buf += " ";

			buf += Format::format("%s(%d)", lookup((::affect::type)type_n).name, count);
		}
	}

	return buf;
}

} // namespace affect
