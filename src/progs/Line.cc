#include "progs/Line.hh"
#include "progs/Expression.hh"

namespace progs {

Line::
Line(Type type, const String& text) : type(type), text(text) {
	if (type == Type::IF || type == Type::AND || type == Type::OR)
		expression.reset(new Expression(text));
	else if (type == Type::COMMAND) {
		String copy = text.lstrip();

		while (!copy.empty()) {
			if (copy[0] == '$')
				symbols::parseVariableSymbol(copy);
			else
				symbols::parseStringSymbol(copy, "$");
		}
	}
}

} // namespace progs
