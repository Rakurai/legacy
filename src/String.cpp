//#include <boost/algorithm/string/predicate.hpp>
//#include <strings.h>
#include "String.hpp"

bool operator== (const String &lhs, const String &rhs) {
//	return boost.iequals(lhs, rhs);
	return strcasecmp(lhs.c_str(), rhs.c_str()) == 0;
}

bool operator== (const String &lhs, const char *rhs) {
//	return boost.iequals(lhs, rhs);
	return strcasecmp(lhs.c_str(), rhs) == 0;
}
