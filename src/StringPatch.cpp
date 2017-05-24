#include <string>
#include "merc.h"
#include "String.hpp"

char * 
strcat(char *dest, const String& src) {
	return std::strcat(dest, src.c_str());
}

char * 
strncat(char *dest, const String& src, size_t n) {
	return std::strncat(dest, src.c_str(), n);
}

char * 
strcpy(char *dest, const String& src) {
	return std::strcpy(dest, src.c_str());
}

char * 
strncpy(char *dest, const String& src, size_t n) {
	return std::strncpy(dest, src.c_str(), n);
}

size_t
strlen(const String& str) {
	return str.size();
}

const char *
strchr(const String& str, int ch) {
	return std::strchr(str.c_str(), ch);
}

const char *
strstr(const String& astr, const String& bstr) {
	return std::strstr(astr.c_str(), bstr.c_str());
}

int
strcmp(const String& astr, const String& bstr) {
	return std::strcmp(astr.c_str(), bstr.c_str());
}

int
strncmp(const String& astr, const String& bstr, size_t n) {
	return std::strncmp(astr.c_str(), bstr.c_str(), n);
}

int strcasecmp(const String& astr, const String& bstr) {
	return strcasecmp(astr.c_str(), bstr.c_str());
}

int number_argument(const String& argument, char *arg) {
	return number_argument(argument.c_str(), arg);
}

int entity_argument(const String& argument, char *arg) {
	return entity_argument(argument.c_str(), arg);
}

int mult_argument(const String& argument, char *arg) {
	return mult_argument(argument.c_str(), arg);
}

const char *one_argument(const String& argument, char *arg) {
	return one_argument(argument.c_str(), arg);
}
