#pragma once

#include "progs/data/Type.hh"
#include "progs/contexts/declare.hh"
#include "String.hh"

namespace progs {
namespace symbols {

struct Symbol {
	Symbol(data::Type s) : type(s) {}
	virtual ~Symbol() { }

	virtual const String to_string(contexts::Context&) = 0;
	virtual const String print_stack() const = 0;
	virtual void assign_to(const Symbol *target, contexts::Context&) = 0;

	void execute(contexts::Context&);

	data::Type type;
};

} // namespace symbols
} // namespace progs
