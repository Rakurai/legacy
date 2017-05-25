#pragma once

#include <iostream>
#include <string>
#include <cstdio>

#include "String.hpp"
#include "memory.h"

/* Variadic template for overloading the *printf functions to use C++ strings.
 * Lets us keep our old C-style printfs and get nice modern strings. */

template<class TMP>
inline auto to_c(TMP&& param) -> decltype(std::forward<TMP>(param)) {
    return std::forward<TMP>(param);
}

inline char const* to_c(String const& s) { return s.c_str(); }
inline char const* to_c(String& s) { return s.c_str(); }

// basic functions that accept the above to_c functions
template<class... Params>
int printf(char const* fmt, Params&&... params) {
    return std::printf(fmt, to_c(params)...);
}

template<class... Params>
int sprintf(char *buf, char const* fmt, Params&&... params) {
    return std::sprintf(buf, fmt, to_c(params)...);
}

template<class... Params>
int snprintf(char *buf, unsigned int len, char const* fmt, Params&&... params) {
    return std::snprintf(buf, len, fmt, to_c(params)...);
}

template<class... Params>
int fprintf(FILE *fp, char const* fmt, Params&&... params) {
    return std::fprintf(fp, fmt, to_c(params)...);
}

// specialized functions

// reinitialize a mutable String object
template<class... Params>
int sprintf(String& str, char const* fmt, Params&&... params) {
	char buf[MAX_STRING_LENGTH];
	snprintf(buf, MAX_STRING_LENGTH, fmt, params...);
    str.assign(buf);
}
