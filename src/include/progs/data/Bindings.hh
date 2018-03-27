#pragma once

#include <map>
#include "progs/data/Type.hh"
#include "String.hh"
#include "Format.hh"

namespace progs {
namespace data {

struct Bindings : public std::map<String, Type> {
	using std::map<String, Type>::map; // inherit constructors for std::map

	void add(const String& key, Type type) {
		Type current_binding = get(key);

		if (current_binding != Type::Void
		 && current_binding != type)
			throw Format::format("attempting to rebind variable '%s' to type '%s'",
				key, type_to_string(type));

		(*this)[key] = type;
	}

	Type get(const String& key) const {
		const auto pair = find(key);

		if (pair == cend())
			return Type::Void;

		return pair->second;
	}
};

} // namespace data
} // namespace progs
