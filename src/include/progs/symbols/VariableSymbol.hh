#pragma once

#include "progs/symbols/Symbol.hh"
#include "progs/symbols/declare.hh"
#include "progs/contexts/Context.hh"
#include "Format.hh"

namespace progs {
namespace symbols {

template <typename T> struct VariableSymbol : public Symbol {
	VariableSymbol(Symbol::Type c, const String& n) : Symbol(c), name(n) {}
	virtual ~VariableSymbol() {}
	T evaluate(contexts::Context& context) {
		contexts::DataWrapper *wrapper = context.get_var(name);
		T temp;
		contexts::datawrapper_access(wrapper, &temp);
		return temp;
	}

	virtual const String to_string(contexts::Context& context) {
		return var_to_string(evaluate(context)); 
	}

	virtual const String print_stack() const {
		return Format::format("Variable<%s>$%s", type_to_string(), name);
	}

	const String name;
};

} // namespace symbols
} // namespace progs
