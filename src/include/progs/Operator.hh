#pragma once

enum class Operator {
	is_equal_to,
	is_not_equal_to,
	has_subset,
	has_not_subset,
	is_greater_than,
	is_less_than,
	is_less_than_or_equal_to,
	is_greater_than_or_equal_to,
	logical_and,
	logical_or,
	error
};

inline const Operator str_to_operator(const String& opr) {
	if (opr == "==") return Operator::is_equal_to;
	else if (opr == "!=") return Operator::is_not_equal_to;
	else if (opr == "/")  return Operator::has_subset;
	else if (opr == "!/")  return Operator::has_not_subset;
	else if (opr == ">")  return Operator::is_greater_than;
	else if (opr == "<")  return Operator::is_less_than;
	else if (opr == "<=")  return Operator::is_less_than_or_equal_to;
	else if (opr == ">=")  return Operator::is_greater_than_or_equal_to;
	else if (opr == "&")  return Operator::logical_and;
	else if (opr == "|")  return Operator::logical_or;
	else {
		Logging::bugf("progs::parse_operator: unknown operator '%s'", opr);
		return Operator::error;
	}
}

inline const String operator_to_str(Operator opr) {
	switch (opr) {
		case Operator::is_equal_to: return "==";
		case Operator::is_not_equal_to: return "!=";
		case Operator::has_subset: return "/";
		case Operator::has_not_subset: return "!/";
		case Operator::is_greater_than: return ">";
		case Operator::is_less_than: return "<";
		case Operator::is_less_than_or_equal_to: return "<=";
		case Operator::is_greater_than_or_equal_to: return ">=";
		case Operator::logical_and: return "&";
		case Operator::logical_or: return "|";
		case Operator::error: return "error";
	}
}

/* These two functions do the basic evaluation of ifcheck operators.
 *  It is important to note that the string operations are not what
 *  you probably expect.  Equality is exact and division is substring.
 *  remember that lhs has been stripped of leading space, but can
 *  still have trailing spaces so be careful when editing since:
 *  "guard" and "guard " are not equal.
 */
inline bool evaluate(const String& lhs, Operator opr, const String& rhs)
{
	switch (opr) {
		case Operator::is_equal_to:                  return lhs == rhs;
		case Operator::is_not_equal_to:              return lhs != rhs;
		case Operator::has_subset:                   return lhs.has_infix(rhs);
		case Operator::has_not_subset:               return !lhs.has_infix(rhs);
		default:
			throw Format::format("progs::evaluate: invalid operator '%s' for int types", operator_to_str(opr));
	}
}

inline bool evaluate(int lhs, Operator opr, int rhs)
{
	switch (opr) {
		case Operator::is_equal_to:                  return lhs == rhs;
		case Operator::is_not_equal_to:              return lhs != rhs;
		case Operator::is_greater_than:              return lhs > rhs;
		case Operator::is_less_than:                 return lhs < rhs;
		case Operator::is_greater_than_or_equal_to:  return lhs >= rhs;
		case Operator::is_less_than_or_equal_to:     return lhs <= rhs;
		case Operator::logical_and:                  return lhs & rhs;
		case Operator::logical_or:                   return lhs | rhs;
		default:
			throw Format::format("progs::evaluate: invalid operator '%s' for int types", operator_to_str(opr));
	}
}
