#include "progs/Expression.hh"
#include "progs/symbols/declare.hh"
#include "progs/symbols/ValueSymbol.hh"
#include "progs/symbols/deref.hh"

namespace progs {

Expression::
Expression(const String& orig, data::Bindings& bindings) :
	opr(Operator(Operator::Type::is_not_equal_to)),
	rhs(new symbols::ValueSymbol<bool>(data::Type::Boolean, false)) {

	// get rid of unneeded spaces
	String str = orig.strip();

	if (str.empty())
		throw Format::format("progs::Expression: empty expression");

	bool invert = false;

	if (str[0] == '!') {
		invert = true;
		str.erase(0, 1);
	}

	// search for a binary operator in the string
	for (int i = Operator::Type::first; i < Operator::Type::size; i++) {
		Operator temp_opr = Operator((Operator::Type)i);
		std::size_t pos = str.find(temp_opr.to_string());

		if (pos == std::string::npos)
			continue;

		opr = temp_opr;

		if (str.length() - pos - opr.to_string().length() <= 0)
			throw Format::format("progs::Expression: empty expression after operator", opr.to_string());

		if (invert)
			throw Format::format("progs::Expression: unary '!' encountered in binary expression");

		String lhstr = str.substr(0, pos).strip();
		String rhstr = str.substr(pos + opr.to_string().length()).strip();

		// parse rhs first
		rhs = symbols::parse(rhstr, bindings, "");

		// handle assignment, lhs needs to be a variable, add a binding and then parse
		if (opr.type == Operator::Type::set_equal_to) {
			// make sure this is ONLY a variable name
			if (lhstr[0] != '$')
				throw String("assignment expression must have variable on left");

			String copy = lhstr.substr(1);
			String var_name = symbols::parse_identifier(copy);

			if (var_name.empty())
				throw String("assignment expression has bad variable name on left");

			if (!copy.lstrip().empty())
				throw String("assignment expression must have ummodified variable on left");

			bindings.add(var_name, rhs->type);
		}

		lhs = symbols::parse(lhstr, bindings, "");
		return;
	}

	// binary op not found, evaluate lhs agains != false
	lhs = symbols::parse(str, bindings, "");

	if (invert)
		opr = Operator(Operator::Type::is_equal_to);
}

bool Expression::
evaluate(contexts::Context& context) const {
	if (opr.type == Operator::Type::set_equal_to) {
		rhs->assign_to(lhs.get(), context);
		return true;
	}

	String sl = lhs->to_string(context);
	String sr = rhs->to_string(context);

	if (sl.is_number() && sr.is_number())
		return opr.evaluate(atoi(sl), atoi(sr));

	return opr.evaluate(sl, sr);
}

} // namespace progs
