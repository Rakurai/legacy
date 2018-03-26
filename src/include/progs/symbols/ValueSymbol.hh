#pragma once

#include "progs/symbols/Symbol.hh"
#include "Format.hh"

namespace progs {
namespace symbols {

template <typename T> struct ValueSymbol : public Symbol {
	ValueSymbol(data::Type c, T v) : Symbol(c), value(v) {}
	virtual ~ValueSymbol() {}
	
	T evaluate(contexts::Context&) { return value; }
	
	virtual const String to_string(contexts::Context& context) {
		return var_to_string(evaluate(context));
	}

	virtual const String print_stack() const {
		return Format::format("Value<%s>", type_to_string(type));
	}

	T value;
};

} // namespace symbols
} // namespace progs
