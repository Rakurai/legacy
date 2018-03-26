#pragma once

#include <memory>
#include "progs/symbols/declare.hh"
#include "progs/contexts/declare.hh"
#include "progs/Operator.hh"

namespace progs {

struct Expression {
	Expression(const String&, const std::map<String, data::Type>& var_bindings);
	virtual ~Expression() {}

	bool evaluate(contexts::Context&) const;

private:
	Expression(const Expression&);
	Expression& operator=(const Expression&);

	std::unique_ptr<symbols::Symbol> lhs;
	Operator opr;
	std::unique_ptr<symbols::Symbol> rhs;
};

} // namespace progs
