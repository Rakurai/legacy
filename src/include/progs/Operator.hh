#pragma once

#include "Format.hh"

namespace progs {

class Operator {
public:
	enum Type {
		// important: expression cycles through these trying to split a string in two parts.
		// the two-character operators must come first so it doesn't match on a single char
		is_equal_to = 0,             // ==
		is_not_equal_to,             // !=
		is_less_than_or_equal_to,    // <=
		is_greater_than_or_equal_to, // >=
		has_not_subset,              // !/
		has_subset,                  //  /
		is_greater_than,             //  >
		is_less_than,                //  <
		logical_and,                 //  &
		logical_or,                  //  |

		// assignment
		set_equal_to,                //  =

		size,
		first = 0
	};

	Operator(Type t) : type(t) {}
	virtual ~Operator() {}

	/* These two functions do the basic evaluation of ifcheck operators.
	 *  It is important to note that the string operations are not what
	 *  you probably expect.  Equality is exact and division is substring.
	 *  remember that lhs has been stripped of leading space, but can
	 *  still have trailing spaces so be careful when editing since:
	 *  "guard" and "guard " are not equal.
	 */
	bool evaluate(const String& lhs, const String& rhs) const {
		switch (type) {
			case Type::is_equal_to:                  return lhs == rhs;
			case Type::is_not_equal_to:              return lhs != rhs;
			case Type::has_subset:                   return lhs.has_infix(rhs);
			case Type::has_not_subset:               return !lhs.has_infix(rhs);
			default:
				throw Format::format("progs::Operator::evaluate: invalid operator '%s' for String types", to_string());
		}
	}

	bool evaluate(int lhs, int rhs) const {
		switch (type) {
			case Type::is_equal_to:                  return lhs == rhs;
			case Type::is_not_equal_to:              return lhs != rhs;
			case Type::is_greater_than:              return lhs > rhs;
			case Type::is_less_than:                 return lhs < rhs;
			case Type::is_greater_than_or_equal_to:  return lhs >= rhs;
			case Type::is_less_than_or_equal_to:     return lhs <= rhs;
			case Type::logical_and:                  return lhs & rhs;
			case Type::logical_or:                   return lhs | rhs;
			default:
				throw Format::format("progs::Operator::evaluate: invalid operator '%s' for int types", to_string());
		}
	}
/*
	static bool has_operator_prefix(String str) {
		// str is a copy, we can mangle it
		try {
			Operator opr = parse(str);
		}
		catch (String e) {
			return false;
		}

		return true;
	}

	// take an operator from the string, be careful of snuggling with rhs operand
	static const Operator parse(String& str) {
		str.assign(str.lstrip());

		if (str.empty())
			throw Format::format("progs::Operator::parse: empty string to parse");

		int take_chars = 0;

		if (str.has_prefix("==")
		 || str.has_prefix("!=")
		 || str.has_prefix("!/")
		 || str.has_prefix("<=")
		 || str.has_prefix(">="))
		 	take_chars = 2;
		else if (
			str[0] == '/'
		 || str[0] == '>'
		 || str[0] == '<'
		 || str[0] == '&'
		 || str[0] == '|')
			take_chars = 1;

		if (take_chars == 0)
			throw Format::format("progs::Operator::parse: no valid operator found in '%s'", str);

		Operator opr(str_to_type(str.substr(0, take_chars)));

		str.erase(0, take_chars);
		return opr;
	}
*/
	const String to_string() const {
		switch (type) {
			case Type::is_equal_to: return "==";
			case Type::is_not_equal_to: return "!=";
			case Type::has_subset: return "/";
			case Type::has_not_subset: return "!/";
			case Type::is_greater_than: return ">";
			case Type::is_less_than: return "<";
			case Type::is_less_than_or_equal_to: return "<=";
			case Type::is_greater_than_or_equal_to: return ">=";
			case Type::logical_and: return "&";
			case Type::logical_or: return "|";
			case Type::set_equal_to: return "=";
			default:
				throw Format::format("progs::Operator::type_to_str: operator has invalid type '%d'", type);
		}
	}

	Type type;

private:
	static const Type str_to_type(const String& opr) {
		if (opr == "==") return Type::is_equal_to;
		else if (opr == "!=") return Type::is_not_equal_to;
		else if (opr == "/")  return Type::has_subset;
		else if (opr == "!/")  return Type::has_not_subset;
		else if (opr == ">")  return Type::is_greater_than;
		else if (opr == "<")  return Type::is_less_than;
		else if (opr == "<=")  return Type::is_less_than_or_equal_to;
		else if (opr == ">=")  return Type::is_greater_than_or_equal_to;
		else if (opr == "&")  return Type::logical_and;
		else if (opr == "|")  return Type::logical_or;
		else if (opr == "=")  return Type::set_equal_to;

		throw Format::format("progs::Operator::parse: unknown operator '%s'", opr);
	}
};

} // namespace progs
