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
	if (rhs)
		return lhs == String(rhs);

	return false;
}

bool operator!= (const String &lhs, const String &rhs) {
	return !(lhs == rhs);
}

bool operator!= (const String &lhs, const char *rhs) {
	return !(lhs == rhs);
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
	return pos == std::string::npos ? "" : substr(pos);
}

String String::
rstrip(const char *chars) const {
	std::size_t pos = find_last_not_of(chars);
	return pos == std::string::npos ? "" : substr(0, pos+1);
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
	String word;
	lsplit(word, chars);
	return word;
}

String String::
rsplit(const char *chars) const {
	String word;
	rsplit(word, chars);
	return word;
}

String String::
lsplit(String& word, const char *chars) const {
	std::size_t start_pos = find_first_not_of(chars);

	if (start_pos == std::string::npos) {
		word.clear();
		return "";
	}

	std::size_t end_pos = find_first_of(chars, start_pos);

	if (end_pos == std::string::npos) {
		word.assign(substr(start_pos));
		return "";
	}

	word.assign(substr(start_pos, end_pos - start_pos));
	return substr(end_pos).lstrip(chars);
}

String String::
rsplit(String& word, const char *chars) const {
	std::size_t end_pos = find_last_not_of(chars);

	if (end_pos == std::string::npos) {
		word.clear();
		return "";
	}

	std::size_t start_pos = find_last_of(chars, end_pos);

	if (start_pos == std::string::npos) {
		word.assign(substr(0, end_pos + 1));
		return "";
	}

	word.assign(substr(start_pos + 1, end_pos - start_pos));
	return substr(0, start_pos + 1).rstrip();
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
