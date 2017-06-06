//#include <boost/algorithm/string/predicate.hpp>
//#include <strings.h>
#include "String.hpp"
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <strings.h> // strcasecmp

#include "c_string.h"

// helper functor for case-insensitive search
struct case_insensitive_equal {
	bool operator() (char lhs, char rhs) {
		return toupper(lhs) == toupper(rhs);
	}
};

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

#include <iostream>

std::size_t String::
find(const String& str, std::size_t start_pos) const {
	if (start_pos > size() - str.size())
		return std::string::npos;

	if (str.size() == 0)
		return 0; 

	auto it = std::search(
		cbegin() + start_pos, cend(),
		str.cbegin(), str.cend(),
		case_insensitive_equal()
	);

	if (it == cend())
		return std::string::npos;

	return it - cbegin();
}

std::size_t String::
find_nth(std::size_t nth, const String& str, std::size_t start_pos) const {
	std::size_t pos = find(str, start_pos);

	if (pos == std::string::npos)
		return pos;

	if (nth > 1)
		pos = find_nth(nth - 1, str, pos + str.size());

	return pos;
}

bool String::
has_prefix(const String& str, std::size_t min_chars) const {
	return str.is_prefix_of(*this, min_chars);
}

bool String::
is_prefix_of(const String& str, std::size_t min_chars) const {
	if (size() < min_chars || str.size() < size())
		return false;

	return *this == str.substr(0, size());
}

bool String::
has_infix(const String& str, std::size_t min_chars) const {
	return str.is_infix_of(*this, min_chars);
}

bool String::
is_infix_of(const String& str, std::size_t min_chars) const {
	if (size() < min_chars || str.size() < size())
		return false;

	return str.find(*this) != std::string::npos;
}

bool String::
has_suffix(const String& str, std::size_t min_chars) const {
	return str.is_suffix_of(*this, min_chars);
}

bool String::
is_suffix_of(const String& str, std::size_t min_chars) const {
	if (size() < min_chars || str.size() < size())
		return false;

	return *this == str.substr(str.size() - size());
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
lstrip(const String& chars) const {
	std::size_t pos = find_first_not_of(chars);
	return pos == std::string::npos ? "" : substr(pos);
}

String String::
rstrip(const String& chars) const {
	std::size_t pos = find_last_not_of(chars);
	return pos == std::string::npos ? "" : substr(0, pos+1);
}

String String::
strip(const String& chars) const {
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
lsplit(const String& chars) const {
	String word;
	lsplit(word, chars);
	return word;
}

String String::
rsplit(const String& chars) const {
	String word;
	rsplit(word, chars);
	return word;
}

String String::
lsplit(String& word, const String& chars) const {
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
rsplit(String& word, const String& chars) const {
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
	if (what.empty())
		return *this;

	std::size_t pos = 0;
	String str(*this);

	while (times-- != 0 && (pos = str.find(what, pos)) != std::string::npos) {
		str.std::string::replace(pos, what.size(), with);
		pos += with.size();
	}

	return str;
}
