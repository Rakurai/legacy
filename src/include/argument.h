#pragma once

#include "String.hpp"
#include "Flags.hpp"

int number_argument(const char * argument, char* arg);
Flags::Bit entity_argument(const char * argument, char* arg);
int mult_argument(const char * argument, char* arg);
const char *one_argument(const char * argument, char* arg);

inline int number_argument(const String& argument, String& arg) {
	char buf[argument.size()+1];
	int ret = number_argument(argument.c_str(), buf);
	arg.assign(buf);
	return ret;
}
inline Flags::Bit entity_argument(const String& argument, String& arg) {
	char buf[argument.size()+1];
	Flags::Bit ret = entity_argument(argument.c_str(), buf);
	arg.assign(buf);
	return ret;
}
inline int mult_argument(const String& argument, String& arg) {
	char buf[argument.size()+1];
	int ret = mult_argument(argument.c_str(), buf);
	arg.assign(buf);
	return ret;
}
inline const char *one_argument(const String& argument, String& arg) {
	char buf[argument.size()+1];
	const char *ret = one_argument(argument.c_str(), buf);
	arg.assign(buf);
	return ret;
}
