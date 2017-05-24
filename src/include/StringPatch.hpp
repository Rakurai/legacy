// replacements for C char array ops that take String objects as arguments (temporary)

#pragma once

char * strcat(char *dest, const String& src);
char * strncat(char *dest, const String& src, size_t n);
char * strcpy(char *dest, const String& src);
char * strncpy(char *dest, const String& src, size_t n);
size_t strlen(const String& str);
const char * strchr(const String& str, int ch);
const char * strstr(const String& astr, const String& bstr);
int strcmp(const String& astr, const String& bstr);
int strncmp(const String& astr, const String& bstr, size_t n);
int strcasecmp(const String& astr, const String& bstr);
int number_argument(const String& argument, char *arg);
int entity_argument(const String& argument, char *arg);
int mult_argument(const String& argument, char *arg);
const char *one_argument(const String& argument, char *arg);
