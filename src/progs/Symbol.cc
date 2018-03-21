#include "progs/Symbol.hh"
#include "progs/Operator.hh"

namespace progs {

std::unique_ptr<Symbol> Symbol::
parse(String& orig, const String& until) {
	String str, symbol_type;
	Symbol *ptr;

	try {
		// attempt to construct, throwing exception just means it's not that type.
		// constructors throw an integer if it doesn't look like that type of symbol, and
		// throw a string message if it does look like that symbol and its in error
		symbol_type = "Variable";
		str = orig;
		ptr = parseVariableSymbol(str);

		if (ptr != nullptr) {
			orig.assign(str);
			return std::unique_ptr<Symbol>(ptr);
		}

		symbol_type = "Function";
		str = orig;
		ptr = parseFunctionSymbol(str);

		if (ptr != nullptr) {
			orig.assign(str);
			return std::unique_ptr<Symbol>(ptr);
		}

		symbol_type = "Integer";
		str = orig;
		ptr = parseIntegerSymbol(str);

		if (ptr != nullptr) {
			orig.assign(str);
			return std::unique_ptr<Symbol>(ptr);
		}

		symbol_type = "Boolean";
		str = orig;
		ptr = parseBooleanSymbol(str);

		if (ptr != nullptr) {
			orig.assign(str);
			return std::unique_ptr<Symbol>(ptr);
		}

		symbol_type = "String";
		str = orig;
		ptr = parseStringSymbol(str, until);

		if (ptr != nullptr) {
			orig.assign(str);
			return std::unique_ptr<Symbol>(ptr);
		}

		throw String("unable to parse into a symbol");
	}
	catch (String e) {
		throw Format::format("progs::%sSymbol::parse: %s\nstring = '%s'", symbol_type, e, orig);
	}
}

// we parse leading whitespace, but in all cases encountering EOL is an error
void parse_whitespace(String& str) {
	while (!str.empty() && (str[0] == ' ' || str[0] == 't' || str[0] == '\r'))
		str.erase(0, 1);

	if (str.empty())
		throw String("unexpected end of line encountered");
}

// parse a valid identifier
const String parse_identifier(String& str) {
	parse_whitespace(str);

	String buf;

	while (!str.empty() && (isalpha(str[0]) || isdigit(str[0]) || str[0] == '_')) {
		buf += str[0];
		str.erase(0, 1);
	}

	return buf;
}

Symbol * Symbol::
parseVariableSymbol(String& str) {
	parse_whitespace(str);

	if (str[0] != '$') // no '$', not a variable
		return nullptr;

	str.erase(0, 1);
	String var = "$";
	var += parse_identifier(str);

	if (var.length() == 1)
		throw Format::format("illegal characters '%s' following variable symbol '$'", str);

	String member_name;

	if (var.length() == 2) {
		// expand the convenience variable
		switch (var[1]) {
			case 'i':             member_name = "name";     break;
			case 'n':             member_name = "name";     break;
			case 'b':             member_name = "name";     break;
			case 't':             member_name = "name";     break;
			case 'r':             member_name = "name";     break;

			case 'I': var = "$i"; member_name = "title";    break;
			case 'N': var = "$n"; member_name = "title";    break;
			case 'B': var = "$b"; member_name = "title";    break;
			case 'T': var = "$t"; member_name = "title";    break;
			case 'R': var = "$r"; member_name = "title";    break;

			case 'j': var = "$i"; member_name = "he_she";   break;
			case 'e': var = "$n"; member_name = "he_she";   break;
			case 'f': var = "$b"; member_name = "he_she";   break;
			case 'E': var = "$t"; member_name = "he_she";   break;
			case 'J': var = "$r"; member_name = "he_she";   break;

			case 'k': var = "$i"; member_name = "him_her";  break;
			case 'm': var = "$n"; member_name = "him_her";  break;
			case 'g': var = "$b"; member_name = "him_her";  break;
			case 'M': var = "$t"; member_name = "him_her";  break;
			case 'K': var = "$r"; member_name = "him_her";  break;

			case 'l': var = "$i"; member_name = "his_her";  break;
			case 's': var = "$n"; member_name = "his_her";  break;
			case 'h': var = "$b"; member_name = "his_her";  break;
			case 'S': var = "$t"; member_name = "his_her";  break;
			case 'L': var = "$r"; member_name = "his_her";  break;

			case 'o': var = "$o"; member_name = "name";     break;
			case 'O': var = "$o"; member_name = "sdesc";    break;
			case 'a': var = "$o"; member_name = "ind_art";  break;
			case 'p': var = "$p"; member_name = "name";     break;
			case 'P': var = "$p"; member_name = "sdesc";    break;
			case 'A': var = "$p"; member_name = "ind_art";  break;

			default: throw Format::format("unknown single character variable name '%s'", var);
		}
	}

	return new VariableSymbol(var, member_name);
/*
	if (str.empty() || str[0] != '.')
		return;

	str.erase(0, 1);

	member_name = parse_identifier(str);

	if (member_name.empty())
		throw Format::format("illegal characters '%s' following dereference operator '.'", str);
*/
}

Symbol * Symbol::
parseFunctionSymbol(String& str) {
	parse_whitespace(str);

	// function name is everything leading up to '(', only valid chars are alphanumeric and _
	String fn = parse_identifier(str);

	if (fn.empty())
		return nullptr;

	// no whitespace allowed between identifier and opening paren or dereference op

	if (str.empty() || str[0] != '(')
		return nullptr;

	// after this it looks like a function, throw errors if it doesn't comply
	std::vector<std::unique_ptr<Symbol>> arg_list;

	str.erase(0, 1); // left paren

	while (true) {
		parse_whitespace(str);

		if (str[0] != ')') {
			arg_list.push_back(Symbol::parse(str, ",)"));
			parse_whitespace(str);

			if (str[0] == ',') {
				str.erase(0, 1);
				continue;
			}
		}

		if (str[0] == ')')
			break;

		// encountered something other than , or ) after a symbol, bail out
		throw Format::format("unexpected characters '%s' after argument", str);
	}

	str.erase(0, 1); // right paren
	return new FunctionSymbol(fn, arg_list);
}

Symbol * Symbol::
parseIntegerSymbol(String& str) {
	parse_whitespace(str);

	String buf;
	bool negative = false;

	if (str[0] == '-') {
		negative = true;
		str.erase(0, 1);
	}

	while (!str.empty() && isdigit(str[0])) {
		buf += str[0];
		str.erase(0, 1);
	}

	if (buf.empty() || !buf.is_number())
		return nullptr;

	int val = atoi(buf);

	if (negative)
		val = 0 - val;

	return new IntegerSymbol(val);
}

Symbol * Symbol::
parseBooleanSymbol(String& str) {
	parse_whitespace(str);

	if (str.has_prefix("true")) {
		str.erase(0, 4);
		return new BooleanSymbol(true);
	}

	if (str.has_prefix("false")) {
		str.erase(0, 5);
		return new BooleanSymbol(false);
	}

	return nullptr;
}

Symbol * Symbol::
parseStringSymbol(String& str, const String& until) {
	parse_whitespace(str);

	String val;

	if (str[0] == '\'' || str[0] == '"') {
		std::size_t pos = str.find(str.substr(0, 1));

		if (pos == std::string::npos)
			throw String("unclosed opening quote");

		val = str.substr(1, pos);
		str.erase(0, pos+1);
	}
	else {
		while (!str.empty() && strchr(until, str[0]) == nullptr) {
			val += str[0];
			str.erase(0, 1);
		}
	}

	val = val.strip();

	if (val.empty())
		throw String("empty string");

	return new StringSymbol(val);
}

} // namespace progs
