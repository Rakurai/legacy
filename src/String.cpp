//#include <boost/algorithm/string/predicate.hpp>
//#include <strings.h>
#include "String.hpp"
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

#include "c_string.h"

bool operator== (const String &lhs, const String &rhs) {
//	return boost.iequals(lhs, rhs);
	return strcasecmp(lhs.c_str(), rhs.c_str()) == 0;
}

bool operator== (const String &lhs, const char *rhs) {
//	return boost.iequals(lhs, rhs);
	if (rhs)
		return strcasecmp(lhs.c_str(), rhs) == 0;

	return false;
}

String String::
lstrip() const {
	String s(*this);
	s.erase(
		s.begin(),
		std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace)))
	);
	return s;
}

String String::
rstrip() const {
	String s(*this);
	s.erase(
		std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(),
		s.end()
	);
	return s;
}

String String::
strip() const {
	return (*this).lstrip().rstrip();
}

String String::
capitalize() const {
	int pos = this->find_first_not_of(" \t\r\n");

	if (pos == npos)
		return *this;

	String tail = this->substr(pos);
	tail[0] = toupper(tail[0]);
	return this->substr(0, pos) + tail;
}

String String::
lsplit() const {
	return this->substr(0, this->find(' '));
}
