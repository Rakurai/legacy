#pragma once

#include <map>
#include "progs/Type.hh"

namespace progs {

struct prog_table_t {
	const String name;
	const String allowed_vars;
};

extern const std::map<Type, prog_table_t> prog_table;

} // namespace progs
