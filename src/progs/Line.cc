#include "progs/Line.hh"
#include "progs/Expression.hh"

namespace progs {

Line::
Line(Type type, const String& text, const std::map<String, data::Type>& var_bindings)
 : type(type), text(text) {
	if (type == Type::IF || type == Type::AND || type == Type::OR)
		expression.reset(new Expression(text, var_bindings));
}

} // namespace progs
