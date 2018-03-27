#include "progs/Line.hh"
#include "progs/Expression.hh"
#include "progs/symbols/declare.hh"

namespace progs {

Line::
Line(Type type, const String& text, data::Bindings& bindings)
 : type(type), text(text) {
 	String copy = text;
	if (type == Type::IF || type == Type::AND || type == Type::OR) {
		expression.reset(new Expression(copy, bindings));

		if (expression->opr.type == Operator::Type::set_equal_to)
			throw String("illegal assignment in boolean expression");
	}
	else if (type == Type::ASSIGN) {
		expression.reset(new Expression(copy, bindings));

		// make sure expression matches the line type
		if ((type == Type::IF || type == Type::AND || type == Type::OR)
		 && expression->opr.type == Operator::Type::set_equal_to)
			throw String("boolean comparison operator used in assignment expression");
	}
	else if (type == Type::COMMAND) {
		// test the line for variable and function usage that doesn't make sense
		// make a copy of the string
		String copy = text.lstrip();

		while (!copy.empty()) {
			if (copy[0] == '$') {
				// make sure the variable name is in the allowed list
				String copy2 = copy.substr(1); // parsing will consume this copy
				String var_name = symbols::parse_identifier(copy2);

				if (bindings.get(var_name) == data::Type::Void)
					throw Format::format("progs::Prog: variable $%s is undefined", var_name);

				// go back to the working copy, done with copy2
				// parse variable and function stack, throws errors if it breaks
				symbols::parseVariableSymbol(copy, bindings);
			}
			else
				// parse until a variable start
				symbols::parseStringSymbol(copy, "$");
		}
	}
}

} // namespace progs
