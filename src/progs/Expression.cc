#include "progs/Expression.hh"
#include "progs/symbols/declare.hh"
#include "progs/symbols/ValueSymbol.hh"

namespace progs {

using namespace symbols;

Expression::
Expression(const String& orig) :
	opr(Operator(Operator::Type::is_equal_to)) {
	// get rid of unneeded spaces
	String str = orig.strip();

	if (str.empty())
		throw Format::format("progs::Expression: empty expression");

	bool invert = false;

	if (str[0] == '!') {
		invert = true;
		str.erase(0, 1);
	}

	// search for a binary operator in the string.  if none found,
	// treat the whole expression as a symbol and evaluate against == 1
	for (int i = Operator::Type::first; i < Operator::Type::size; i++) {
		opr = Operator((Operator::Type)i);
		std::size_t pos = str.find(opr.to_string());

		if (pos == std::string::npos)
			continue;

		if (str.length() - pos - opr.to_string().length() <= 0)
			throw Format::format("progs::Expression: empty expression after operator", opr.to_string());

		if (invert)
			throw Format::format("progs::Expression: unary '!' encountered in binary expression");

		String lhstr = str.substr(0, pos);
		String rhstr = str.substr(pos + opr.to_string().length());

		lhs = parse(lhstr, "");
		rhs = parse(rhstr, "");
		return;
	}

	static std::unique_ptr<Symbol> true_sym((Symbol *)(new ValueSymbol<bool>(Symbol::Type::Boolean, true)));
	static std::unique_ptr<Symbol> false_sym((Symbol *)(new ValueSymbol<bool>(Symbol::Type::Boolean, false)));

	lhs = parse(str, "");
	opr = Operator(Operator::Type::is_equal_to);

	if (invert)
		rhs.reset(new ValueSymbol<bool>(Symbol::Type::Boolean, false));
	else
		rhs.reset(new ValueSymbol<bool>(Symbol::Type::Boolean, true));
}

bool Expression::
evaluate(contexts::Context& context) const {
	return symbols::evaluate(opr, lhs, rhs, context);
}

} // namespace progs
