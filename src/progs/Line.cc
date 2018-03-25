#include "progs/Line.hh"
#include "progs/Expression.hh"

namespace progs {

Line::
Line(Type type, const String& text) : type(type), text(text) {
	if (type == Type::IF || type == Type::AND || type == Type::OR)
		expression.reset(new Expression(text));
}

} // namespace progs
