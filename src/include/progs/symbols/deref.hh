#pragma once

#include "progs/symbols/ValueSymbol.hh"
#include "progs/symbols/VariableSymbol.hh"
#include "progs/symbols/FunctionSymbol.hh"

namespace progs {
namespace symbols {

template <typename T>
T deref(Symbol *sym, contexts::Context& context) {
	auto var_sym = dynamic_cast<VariableSymbol<T> *>(sym);
	if (var_sym != nullptr) return var_sym->evaluate(context);

	auto fn_sym  = dynamic_cast<FunctionSymbol<T> *>(sym);
	if (fn_sym != nullptr)  return fn_sym->evaluate(context);

	auto val_sym  = dynamic_cast<ValueSymbol<T> *>(sym);
	if (val_sym != nullptr) return val_sym->evaluate(context);

	throw String("function parent symbol is not variable, function, or value");
}

template <typename T>
void assign(const Symbol *sym, contexts::Context& context, data::Type type, T data) {
	const auto var_sym = dynamic_cast<const VariableSymbol<T> *>(sym);

	if (var_sym == nullptr)
		throw String("attempt to assign to a non-variable");

	context.set_var(var_sym->name, type, data);
}

} // namespace symbols
} // namespace progs
