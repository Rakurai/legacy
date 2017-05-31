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

std::size_t String::
find_nth(std::size_t nth, char val) const {
	if (nth > 0)
		for (std::size_t pos = 0, occurrence = 0; pos < size(); pos++)
			if ((*this)[pos] == val && ++occurrence == nth)
				return pos;

	return std::string::npos;
}

/*
 * String transformations.  The paradigm here is IMMUTABLE - never modify the string
 * in place.  This allows syntax like print(str.capitalize()), without unexpected side effects.
 */

String String::
substr(std::size_t pos, std::size_t count) const {
	return std::string::substr(pos, count);
}

String String::
lstrip(const char *chars) const {
	std::size_t pos = find_first_not_of(chars);
	return pos == std::string::npos ? *this : substr(find_first_not_of(chars));
}

String String::
rstrip(const char *chars) const {
	std::size_t pos = find_last_not_of(chars);
	return substr(0, pos == std::string::npos ? size() : pos);
}

String String::
strip(const char *chars) const {
	return lstrip(chars).rstrip(chars);
}

String String::
capitalize() const {
	String str(*this);
	std::size_t pos = str.find_first_not_of(" \t\r\n");

	if (pos != std::string::npos)
		str[pos] = toupper(str[pos]);

	return str;
}

String String::
lsplit(const char *chars) const {
	std::size_t pos = find_first_of(chars);
	return substr(0, pos == std::string::npos ? size() : pos);
}

String String::
rsplit(const char *chars) const {
	std::size_t pos = find_last_of(chars);
	return substr(pos == std::string::npos ? 0 : pos);
}

String String::
lsplit(String& word, const char *chars) const {
	std::size_t pos = find_first_of(chars);

	if (pos == std::string::npos) {
		word.assign(*this);
		return "";
	}

	word.assign(substr(0, pos));
	return substr(pos);
}

String String::
rsplit(String& word, const char *chars) const {
	std::size_t pos = find_last_of(chars);

	if (pos == std::string::npos) {
		word.assign(*this);
		return "";
	}

	word.assign(substr(pos));
	return substr(0, pos);
}

String String::
replace(const String& what, const String& with, int times) const {
	std::size_t pos;
	String str;

	// if -1 specified, replace all (will never be 0)
	while (times-- != 0 && (pos = str.find(what)) != std::string::npos)
		str.std::string::replace(pos, with.size(), with);

	return str;
}
