#pragma once

#include <memory>
#include <map>
#include "progs/symbols/Symbol.hh"
#include "progs/contexts/declare.hh"
#include "progs/data/declare.hh"
#include "progs/Operator.hh"

namespace progs {

struct Expression {
	Expression(const String&, data::Bindings&);
	virtual ~Expression() {}

	bool evaluate(contexts::Context&) const;

	std::unique_ptr<symbols::Symbol> lhs;
	Operator opr;
	std::unique_ptr<symbols::Symbol> rhs;

private:
	Expression(const Expression&);
	Expression& operator=(const Expression&);
};

} // namespace progs
