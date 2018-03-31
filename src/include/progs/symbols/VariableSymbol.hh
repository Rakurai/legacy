#pragma once

#include "progs/symbols/Symbol.hh"
#include "progs/contexts/Context.hh"
#include "Format.hh"

namespace progs {
namespace symbols {

template <typename T> struct VariableSymbol : public Symbol {
	VariableSymbol(data::Type c, const String& n) : Symbol(c), name(n) {}
	virtual ~VariableSymbol() {}
	T evaluate(contexts::Context& context) {
		T temp;
		context.get_var(name, &temp);
		return temp;
	}

	virtual const String to_string(contexts::Context& context) {
		return var_to_string(evaluate(context)); 
	}

	virtual void assign_to(const Symbol *target, contexts::Context& context) {
		assign(target, context, type, evaluate(context));
	}

	virtual const String print_stack() const {
		return Format::format("<%s>$%s", type_to_string(type), name);
	}

	const String name;
};

} // namespace symbols
} // namespace progs
