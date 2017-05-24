#pragma once

#include <iostream>
#include <string>
#include <cstdio>

#include "merc.h" // for type defines, can't count on merc.h being included before this
#include "recycle.h" // for add_buf
//#include "String.hpp"

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

// printf to a character
template<class... Params>
void ptc(CHAR_DATA *ch, const char *fmt, Params&&... params)
{
	char buf[MAX_STRING_LENGTH];
	snprintf(buf, MAX_STRING_LENGTH, fmt, params...);
	stc(buf, ch);
}

// print stuff, append to buffer. safe.
template<class... Params>
int ptb(BUFFER *buffer, const char *fmt, Params&&... params)
{
	char buf[MAX_STRING_LENGTH];
	int res = snprintf(buf, MAX_STRING_LENGTH, fmt, params...);

	if (res >= MAX_STRING_LENGTH - 1) {
		buf[0] = '\0';
		bug("print_to_buffer: overflow to buffer, aborting", 0);
	}
	else
		add_buf(buffer, buf);

	return res;
}

template<class... Params>
void bugf(const char *fmt, Params&&... params)
{
	char buf[MAX_STRING_LENGTH];
	snprintf(buf, MAX_STRING_LENGTH, fmt, params...);
	bug(buf, 0);
}
