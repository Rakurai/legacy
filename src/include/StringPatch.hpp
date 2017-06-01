// replacements for C char array ops that take String objects as arguments (temporary)

#pragma once

#include <cstring>

char * strcat(char *dest, const String& src);
char * strncat(char *dest, const String& src, size_t n);
char * strcpy(char *dest, const String& src);
char * strncpy(char *dest, const String& src, size_t n);
size_t strlen(const String& str);
inline char * strstr(char *in, const String& what) { return std::strstr(in, what.c_str()); }
const char * strchr(const String& str, int ch);
const char * strstr(const String& astr, const String& bstr);
int strcmp(const String& astr, const String& bstr);
int strncmp(const String& astr, const String& bstr, size_t n);
int strcasecmp(const String& astr, const String& bstr);
inline int atoi(const String& astr) { return atoi(astr.c_str()); }
bool is_number(const String& str);

int number_argument(const char * argument, char* arg);
int entity_argument(const char * argument, char* arg);
int mult_argument(const char * argument, char* arg);
const char *one_argument(const char * argument, char* arg);

inline int number_argument(const String& argument, String& arg) {
	char buf[MSL];
	int ret = number_argument(argument.c_str(), buf);
	arg.assign(buf);
	return ret;
}
inline int entity_argument(const String& argument, String& arg) {
	char buf[MSL];
	int ret = entity_argument(argument.c_str(), buf);
	arg.assign(buf);
	return ret;
}
inline int mult_argument(const String& argument, String& arg) {
	char buf[MSL];
	int ret = mult_argument(argument.c_str(), buf);
	arg.assign(buf);
	return ret;
}
inline const char *one_argument(const String& argument, String& arg) {
	char buf[MSL];
	const char *ret = one_argument(argument.c_str(), buf);
	arg.assign(buf);
	return ret;
}
