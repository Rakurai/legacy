#include "progs/Symbol.hh"
#include "progs/Operator.hh"

namespace progs {

const Symbol Symbol::
parse(String& str) {
	str.assign(str.lstrip());

	String orig = str, buf;

	// take everything before an operator or the end of the string (could be single operand)
	while (true) {
		if (str.empty()
		 || Operator::has_operator_prefix(str)
		 || str[0] == ',') // for parsing comma-separated args, this shouldn't exist in a regular symbol (maybe in quotes?)
			break;

		buf += str[0];
		str.erase(0, 1);
	}

	buf = buf.rstrip(); // buf is stripped now

	if (buf.empty())
		throw Format::format("progs::Symbol: empty symbol in string '%s'", orig);

	// attempt to construct, throwing exception just means it's not that type
	// constructors throw 0 if it doesn't look like that type ofsymbol, and
	// throw 1 if it does look like that symbol and its in error
	try {
		return VariableSymbol(buf);
	}
	catch (int e) {
		if (e == 1) // weird looking variable
			throw Format::format("progs::Symbol: invalid variable in string '%s'", orig);
	}

	try {
		return FunctionSymbol(buf);
	}
	catch (int e) {
		if (e == 1) // weird looking function
			throw Format::format("progs::Symbol: invalid function in string '%s'", orig);
	}

	try {
		return IntegerSymbol(buf);
	}
	catch (...) {} // just is_number or not

	try {
		return BooleanSymbol(buf);
	}
	catch (...) {} // just true/false or not

	return StringSymbol(buf); // just copies the string, no exceptions
}

VariableSymbol::
VariableSymbol(String str) {
	if (str[0] != '$') // no '$', not a variable
		throw 0;

	if (str.length() < 2) // must have a letter
		throw 1;

	var = str.substr(0, 2);
	str.erase(0, 2);
	str = str.lstrip();

	if (str.empty()) // nothing left, we're done
		return;

	if (str[0] != '.' || str.length() < 2) // but could be followed by dereference operator '.' and a member name
		throw 1;

	member_name = str.substr(1);

	// make sure its a valid name
	for (char c : member_name)
		if (!isalpha(c) && !isdigit(c) && c != '_')
			throw 1;
}

FunctionSymbol::
FunctionSymbol(String str) {
	// count the parens
	int lp = 0, rp = 0;
	for (char c : str) {
		if (c == '(') lp++;
		if (c == ')') rp++;
	}

	if (lp == rp == 0) // no parens, not a function
		throw 0;

	if (lp != rp) // not enough parens, bad function
		throw 1;

	if (str[str.length()-1] != ')') // args must close with ')'
		throw 1;

	// function name is everything leading up to '(', only valid chars are alphanumeric and _
	str = str.lsplit(fn, "(");
	fn = fn.rstrip();

	if (fn.empty())
		throw 1;

	for (char c : fn)
		if (!isalpha(c) && !isdigit(c) && c != '_')
			throw 1;

	// function name looks ok, get the args
	str.erase(str.length()-1); // remove trailing paren, leading paren already gone with split

	while (true) {
		str = str.lstrip();

		if (str.empty())
			break;

		args.push_back(Symbol::parse(str));
		str = str.lstrip();

		if (str[0] == ',') {
			str.erase(0, 1);
			continue;		
		}

		throw 1; // encountered a weird char after parsing symbol
	}
}

IntegerSymbol::
IntegerSymbol(String str) {
	if (!str.is_number())
		throw 0;

	val = atoi(str);
}

BooleanSymbol::
BooleanSymbol(String str) {
	if (str == "true")
		val = true;
	else if (str == "false")
		val = false;
	else
		throw 0;
}

const String Symbol::
evaluate(const Context& context) const {
	throw "progs::Symbol::evaluate: base class evaluate called";
}

const String VariableSymbol::
evaluate(const Context& context) const {
	return context.dereference_variable(var, member_name);
}

const String FunctionSymbol::
evaluate(const Context& context) const {
	// expand the args
	std::vector<String> str_args;

	for (Symbol arg : args)
		str_args.push_back(arg.evaluate(context));

	return context.compute_function(fn, str_args);
}

const String IntegerSymbol::
evaluate(const Context& context) const {
	return Format::format("%d", val);
}

const String BooleanSymbol::
evaluate(const Context& context) const {
	return val ? "1" : "0";
}

} // namespace progs
