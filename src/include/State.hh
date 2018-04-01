#pragma once

#include <map>
#include "String.hh"
#include "Logging.hh"

struct State {
	const String get_str(const String& key) const {
		if (state.count(key) == 0)
			return "0";

		return state.at(key);
	}

	int get_int(const String& key) const {
		String value = get_str(key);

		if (!value.is_number()) {
			Logging::bugf("State: attempt to retrieve non-numeric pair '%s:%s' as an int", key, value);
			return 0;
		}

		return atoi(value);
	}

	void set(const String& key, const String& value) {
		if (value.empty() || value == "0") {
			if (state.count(key) != 0)
				state.erase(key);
		}
		else
			state[key] = value;
	}

	void set(const String& key, int value) {
		set(key, Format::format("%d", value));
	}

	void erase(const String& key) {
		if (state.count(key) != 0)
			state.erase(key);
	}

	void erase_prefix(const String& prefix) {
		for (auto it = state.begin(); it != state.end(); /**/) {
			if (it->first.has_prefix(prefix))
				it = state.erase(it);
			else
				++it;
		}
	}

	void increment(const String& key) {
		String value = get_str(key);

		if (!value.is_number()) {
			Logging::bugf("State: attempt to increment non-numeric pair '%s:%s'", key, value);
			return;
		}

		set(key, atoi(value)+1);
	}

	void decrement(const String& key) {
		String value = get_str(key);

		if (!value.is_number()) {
			Logging::bugf("State: attempt to decrement non-numeric pair '%s:%s'", key, value);
			return;
		}

		set(key, atoi(value)-1);
	}

	std::map<String, String> state;
};
