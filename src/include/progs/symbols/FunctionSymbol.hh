#pragma once

#include "progs/symbols/Symbol.hh"
#include "Format.hh"

namespace progs {
namespace symbols {

struct fn_type {
	String name;
	Symbol::Type return_class;
	Symbol::Type parent_class;
	std::vector<Symbol::Type> arg_list;
};

extern const std::vector<fn_type> fn_table;

template <typename Ret> struct FunctionSymbol : public Symbol {
	FunctionSymbol(std::unique_ptr<Symbol>& p, unsigned int i, std::vector<std::unique_ptr<Symbol>>& a) :
		Symbol(fn_table[i].return_class), parent(std::move(p)), fn_index(i), arg_list(std::move(a)) {}
	virtual ~FunctionSymbol() {}
	Ret evaluate(contexts::Context&);

	virtual const String to_string(contexts::Context& context) {
		return var_to_string(evaluate(context));
	}

	virtual const String print_stack() const {
		String buf = parent ? parent->print_stack() + "." : "";

		buf += Format::format("Function<%s>%s(%s%s%s)",
			parent ? parent->type_to_string() : "global",
			fn_table[fn_index].name,
			arg_list.size() > 0 ? arg_list[0]->type_to_string() : "",
			arg_list.size() > 1 ? Format::format(", %s", arg_list[1]->type_to_string()) : "",
			arg_list.size() > 2 ? Format::format(", %s", arg_list[2]->type_to_string()) : ""
		);

		return buf;
	}

	std::unique_ptr<Symbol> parent;
	int fn_index;
	std::vector<std::unique_ptr<Symbol>> arg_list;
};

} // namespace symbols
} // namespace progs
