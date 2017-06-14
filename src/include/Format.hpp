#pragma once

#include <iostream>
#include <string>
#include <cstdio>

#include "memory.h"

// need to include whole String class declaration because variadic templates can't
// be in the .cpp file, and we access the String members here.
#include "String.hpp"
#include "Flags.hpp"
#include "ObjectValue.hpp"

/* Variadic template for overloading the *printf functions to use C++ strings.
 * Lets us keep our old C-style printfs and get nice modern strings. */

namespace Format {

template<class TMP>
inline auto to_c(TMP&& param) -> decltype(std::forward<TMP>(param)) {
    return std::forward<TMP>(param);
}

inline const char *to_c(const String& s) { return s.c_str(); }
inline const char *to_c(String& s) { return s.c_str(); }
inline const char *to_c(const Flags& s) { return s.to_string().c_str(); }
inline const char *to_c(Flags& s) { return s.to_string().c_str(); }
inline int to_c(const ObjectValue& v) { return int(v); }
inline int to_c(ObjectValue& v) { return int(v); }

// wrappers around standard functions that accept objects with to_c functions (above)
template<class... Params>
int printf(const String& fmt, Params&&... params) {
    return std::printf(fmt.c_str(), to_c(params)...);
}

template<class... Params>
int sprintf(char *buf, const String& fmt, Params&&... params) {
    return std::sprintf(buf, fmt.c_str(), to_c(params)...);
}

template<class... Params>
int snprintf(char *buf, unsigned int len, const String& fmt, Params&&... params) {
    return std::snprintf(buf, len, fmt.c_str(), to_c(params)...);
}

template<class... Params>
int fprintf(FILE *fp, const String& fmt, Params&&... params) {
    return std::fprintf(fp, fmt.c_str(), to_c(params)...);
}

// versions of sprintf that replace the contents of a String

template<class... Params>
int snprintf(String& str, unsigned int len, const String& fmt, Params&&... params) {
	char buf[len+1];
	unsigned int copied = snprintf(buf, len, fmt, params...);
    str.assign(buf);
    return copied;
}

template<class... Params>
int sprintf(String& str, const String& fmt, Params&&... params) {
	return snprintf(str, MAX_STRING_LENGTH*4, fmt, params...);
}

// string builder.  Can't be in String class because of circular dependency

template<class... Params>
String format(const String& fmt, Params&&... params) {
	String str;
	sprintf(str, fmt, params...);
	return str;
}

} // namespace Format
