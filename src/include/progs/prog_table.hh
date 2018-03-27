#pragma once

#include <map>
#include "progs/Type.hh"
#include "progs/data/Bindings.hh"

namespace progs {

struct prog_table_t {
	const String name;
	const data::Bindings default_bindings;
};

extern const std::map<Type, prog_table_t> prog_table;

} // namespace progs
