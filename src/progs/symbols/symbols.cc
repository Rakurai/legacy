#include "progs/Operator.hh"
#include "progs/symbols/declare.hh"
#include "progs/symbols/ValueSymbol.hh"
#include "progs/symbols/VariableSymbol.hh"
#include "progs/symbols/FunctionSymbol.hh"
#include "Character.hh"
#include "Object.hh"

namespace progs {
namespace symbols {

const String var_to_string(Character * var) { return var ? var->name : "0"; }
const String var_to_string(Object * var) { return var ? var->name : "0"; }
const String var_to_string(bool var) { return var ? "1" : "0"; }
const String var_to_string(int var) { return Format::format("%d", var); }
const String var_to_string(const String& var) { return var; }

bool evaluate(const Operator& opr, const std::unique_ptr<Symbol>& lhs, const std::unique_ptr<Symbol>&rhs, contexts::Context& context) {
	String sl = lhs->to_string(context);
	String sr = rhs->to_string(context);

	if (sl.is_number() && sr.is_number())
		return opr.evaluate(atoi(sl), atoi(sr));

	return opr.evaluate(sl, sr);
}

std::unique_ptr<Symbol>
parse(String& orig, const String& until) {
	String str, symbol_type;
	std::unique_ptr<Symbol> ptr;

	try {
		// attempt to construct, throwing exception just means it's not that type.
		// constructors throw an integer if it doesn't look like that type of symbol, and
		// throw a string message if it does look like that symbol and its in error
		symbol_type = "Variable";
		str = orig;

		if ((ptr = parseVariableSymbol(str)) != nullptr) {
			orig.assign(str);
			return ptr;
		}

		symbol_type = "Function";
		str = orig;
		std::unique_ptr<Symbol> parent;

		if ((ptr = parseFunctionSymbol(str, parent)) != nullptr) {
			orig.assign(str);
			return ptr;
		}

		symbol_type = "Integer";
		str = orig;
		if ((ptr = parseIntegerSymbol(str)) != nullptr) {
			orig.assign(str);
			return ptr;
		}

		symbol_type = "Boolean";
		str = orig;

		if ((ptr = parseBooleanSymbol(str)) != nullptr) {
			orig.assign(str);
			return ptr;
		}

		symbol_type = "String";
		str = orig;

		if ((ptr = parseStringSymbol(str, until)) != nullptr) {
			orig.assign(str);
			return ptr;
		}

		throw String("unable to parse into a symbol");
	}
	catch (String e) {
		throw Format::format("progs::symbols::parse: %sSymbol:: %s\nstring = '%s'", symbol_type, e, orig);
	}
}

// we parse leading whitespace, but in all cases encountering EOL is an error
void parse_whitespace(String& str) {
	while (!str.empty() && (str[0] == ' ' || str[0] == '\t' || str[0] == '\r'))
		str.erase(0, 1);
}

// parse a valid identifier
const String parse_identifier(String& str) {
	parse_whitespace(str);

	if (str.empty())
		throw String("unexpected end of line encountered");

	String buf;

	while (!str.empty() && (isalnum(str[0]) || str[0] == '_')) {
		buf += str[0];
		str.erase(0, 1);
	}

	return buf;
}

std::unique_ptr<Symbol>
parseVariableSymbol(String& str) {
	parse_whitespace(str);

	if (str.empty())
		return nullptr;

	if (str[0] != '$') // no '$', not a variable
		return nullptr;

	str.erase(0, 1);
	String var = parse_identifier(str);

	if (var.empty())
		throw Format::format("illegal characters '%s' following variable symbol '$'", str);

	String member_name;

	if (var.length() == 1) {
		// expand the convenience variable
		switch (var[0]) {
			case 'i':
			case 'n':
			case 'b':
			case 't':
			case 'r':
			case 'o':
			case 'p': break;

			case 'I': var = "i"; member_name = "title()";    break;
			case 'N': var = "n"; member_name = "title()";    break;
			case 'B': var = "b"; member_name = "title()";    break;
			case 'T': var = "t"; member_name = "title()";    break;
			case 'R': var = "r"; member_name = "title()";    break;

			case 'j': var = "i"; member_name = "he_she()";   break;
			case 'e': var = "n"; member_name = "he_she()";   break;
			case 'f': var = "b"; member_name = "he_she()";   break;
			case 'E': var = "t"; member_name = "he_she()";   break;
			case 'J': var = "r"; member_name = "he_she()";   break;

			case 'k': var = "i"; member_name = "him_her()";  break;
			case 'm': var = "n"; member_name = "him_her()";  break;
			case 'g': var = "b"; member_name = "him_her()";  break;
			case 'M': var = "t"; member_name = "him_her()";  break;
			case 'K': var = "r"; member_name = "him_her()";  break;

			case 'l': var = "i"; member_name = "his_her()";  break;
			case 's': var = "n"; member_name = "his_her()";  break;
			case 'h': var = "b"; member_name = "his_her()";  break;
			case 'S': var = "t"; member_name = "his_her()";  break;
			case 'L': var = "r"; member_name = "his_her()";  break;

			case 'O': var = "o"; member_name = "sdesc()";    break;
			case 'a': var = "o"; member_name = "ind_art()";  break;
			case 'P': var = "p"; member_name = "sdesc()";    break;
			case 'A': var = "p"; member_name = "ind_art()";  break;

			default: throw Format::format("unknown single character variable name '%s'", var);
		}
	}

	data::Type sym_class = data::Type::Void;

	     if (var == "i") sym_class = data::Type::Character;
	else if (var == "n") sym_class = data::Type::Character;
	else if (var == "b") sym_class = data::Type::Character;
	else if (var == "t") sym_class = data::Type::Character;
	else if (var == "r") sym_class = data::Type::Character;
	else if (var == "o") sym_class = data::Type::Object;
	else if (var == "p") sym_class = data::Type::Object;

	// create the variable symbol
	std::unique_ptr<Symbol> sym;

	switch(sym_class) {
	case data::Type::Character: sym.reset(new VariableSymbol<Character *>(sym_class, var)); break;
	case data::Type::Object:    sym.reset(new VariableSymbol<Object *>(sym_class, var)); break;
	default:
		throw Format::format("unknown variable binding for '%s'", var);
	}

	// test if its followed by function, and wrap this variable in that accessor
	if (!str.empty() && str[0] == '.') {
		String copy = str.substr(1);
		auto fsym = parseFunctionSymbol(copy, sym); // success will take ownership of sym

		if (fsym != nullptr) {
			str.assign(copy);
			return fsym;
		}

		// failure will not have modified str
	}

	// if it was a convenience variable name, wrap in the appropriate accessor
	if (!member_name.empty())
		return parseFunctionSymbol(member_name, sym);

	// otherwise return the variable
	return sym;
}

std::unique_ptr<Symbol>
parseFunctionSymbol(String& str, std::unique_ptr<Symbol>& parent) {
	parse_whitespace(str);

	if (str.empty())
		return nullptr;

	// function name is everything leading up to '(', only valid chars are alphanumeric and _
	String name = parse_identifier(str);

	if (name.empty())
		return nullptr;

	// no whitespace allowed between identifier and opening paren or dereference op

	if (str.empty() || str[0] != '(')
		return nullptr;

	// after this it looks like a function, throw errors if it doesn't comply
	std::vector<std::unique_ptr<Symbol>> arg_list;

	str.erase(0, 1); // left paren

	while (true) {
		parse_whitespace(str);

		if (str.empty())
			throw String("unexpected end of line encountered");

		if (str[0] != ')') {
			arg_list.push_back(parse(str, ",)"));
			parse_whitespace(str);

			if (str.empty())
				throw String("unexpected end of line encountered");

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

	unsigned int entry_index = 0;
	data::Type parent_class = parent ? parent->type : data::Type::Void;

	// find a defined function with the same signature
	for ( ; entry_index < fn_table.size(); entry_index++) {
		const auto& entry = fn_table[entry_index];

		if (entry.parent_class != parent_class        // same member accessor or global
		 || entry.arg_list.size() != arg_list.size()  // same number of args
		 || entry.name != name)                       // same name
			continue;

		// make sure all args are same type
		bool different_arg_types = false;
		for (unsigned int i = 0; i < arg_list.size(); i++)
			if (entry.arg_list[0] != arg_list[0]->type) {
				different_arg_types = true;
				break;
			}

		if (different_arg_types)
			continue;

		break; // found a matching entry
	}

	if (entry_index >= fn_table.size()) {
		String buf = Format::format("no prog function '%s(", name);

		for (unsigned int i = 0; i < arg_list.size(); i++)
			buf += Format::format("%s%s",
				i > 0 ? ", " : "",
				type_to_string(arg_list[i]->type)
			);

		buf += Format::format(")' in the %s context", type_to_string(parent_class));

		throw buf;
	}

	std::unique_ptr<Symbol> sym;

//Logging::bugf("parsed %s function '%s' with %d args, entry index %d", sym_class_to_string(parent_class), name, arg_list.size(), entry_index);

	switch(fn_table[entry_index].return_class) {
	case data::Type::Character: sym.reset(new FunctionSymbol<Character *>(parent, entry_index, arg_list)); break;
	case data::Type::Object:    sym.reset(new FunctionSymbol<Object *>(parent, entry_index, arg_list)); break;
	case data::Type::String:    sym.reset(new FunctionSymbol<const String>(parent, entry_index, arg_list)); break;
	case data::Type::Boolean:   sym.reset(new FunctionSymbol<bool>(parent, entry_index, arg_list)); break;
	case data::Type::Integer:   sym.reset(new FunctionSymbol<int>(parent, entry_index, arg_list)); break;
	case data::Type::Void:      sym.reset(new FunctionSymbol<int>(parent, entry_index, arg_list)); break;
	default:
		throw Format::format("unhandled function return type");
	}

	// test if its followed by function, and wrap this variable in that accessor
	if (!str.empty() && str[0] == '.') {
		String copy = str.substr(1);
		auto fsym = parseFunctionSymbol(copy, sym); // success will take ownership of sym

		if (fsym != nullptr) {
			str.assign(copy);
			return fsym;
		}

		// failure will not have modified str
	}

	return sym;
}

std::unique_ptr<Symbol>
parseIntegerSymbol(String& str) {
	parse_whitespace(str);

	if (str.empty())
		return nullptr;

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

	return std::unique_ptr<Symbol>(new ValueSymbol<int>(data::Type::Integer, val));
}

std::unique_ptr<Symbol>
parseBooleanSymbol(String& str) {
	parse_whitespace(str);

	if (str.empty())
		return nullptr;

	if (str.has_prefix("true")) {
		str.erase(0, 4);
		return std::unique_ptr<Symbol>(new ValueSymbol<bool>(data::Type::Boolean, true));
	}

	if (str.has_prefix("false")) {
		str.erase(0, 5);
		return std::unique_ptr<Symbol>(new ValueSymbol<bool>(data::Type::Boolean, false));
	}

	return nullptr;
}

std::unique_ptr<Symbol>
parseStringSymbol(String& str, const String& until) {
	parse_whitespace(str);

	if (str.empty())
		return nullptr;

	String val;

	if (str[0] == '"') {
		str.erase(0, 1);
		std::size_t pos = str.find("\"");

		if (pos == std::string::npos)
			throw String("unclosed opening quote");

		val = str.substr(0, pos);
		str.erase(0, pos+1);
	}
	else {
		while (!str.empty() && strchr(until, str[0]) == nullptr) {
			val += str[0];
			str.erase(0, 1);
		}
	}

	val = val.strip();

	return std::unique_ptr<Symbol>(new ValueSymbol<const String>(data::Type::String, val));
}

} // namespace symbols
} // namespace progs
