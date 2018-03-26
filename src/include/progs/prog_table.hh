#pragma once

#include <map>
#include "progs/Type.hh"
#include "progs/data/Type.hh"

namespace progs {

struct prog_table_t {
	const String name;
	const std::map<String, data::Type> default_bindings;
};

extern const std::map<Type, prog_table_t> prog_table;

} // namespace progs
