#pragma once

#include <string>
#include "Actable.hpp"

class String: public std::string, public Actable {
public:
	String() : std::string() {};
	String(const char *c_str) : std::string(c_str == NULL ? "" : c_str) {};
	String(const std::string &s_str) : std::string(s_str) {};
	String(const String &name) : std::string(name) {};

	virtual ~String() {}

	String& operator=(const char ch) {
		std::string::operator=(ch);
		return *this;
	}

	String& operator=(const String& name) {
		std::string::operator=(name);
		return *this;
	};

	// the difference from std::string - perform case insensitive comparisons by default
	friend bool operator== (const String &, const String &);
	friend bool operator== (const String &, const char *);
	friend bool operator!= (const String &, const String &);
	friend bool operator!= (const String &, const char *);

	// TODO: sorting comparison operators

	std::size_t find(const String& str, std::size_t start_pos = 0) const;
	std::size_t find_nth(std::size_t nth, const String& str, std::size_t start_pos = 0) const;

	bool has_prefix(const String& str, std::size_t min_chars = 1) const;
	bool is_prefix_of(const String& str, std::size_t min_chars = 1) const;
	bool has_infix(const String& str, std::size_t min_chars = 1) const;
	bool is_infix_of(const String& str, std::size_t min_chars = 1) const;
	bool has_suffix(const String& str, std::size_t min_chars = 1) const;
	bool is_suffix_of(const String& str, std::size_t min_chars = 1) const;

	bool has_words(const String& wordlist, bool exact = false) const;
	bool has_exact_words(const String& wordlist) const;

	// test if the string is completely numeric.
	bool is_number() const;

	// String transformations, each return a new String object.  NO modify in place ops
	String substr(std::size_t pos = 0, std::size_t count = npos) const;

	// capitalize the first letter of a string, skips non-alphanumerics
	String capitalize() const;
	String uncolor() const;

	// trim the whitespace (or user-defined characters) from the front, back, or both
	String lstrip(const String& chars = " \t\n\r") const;
	String rstrip(const String& chars = " \t\n\r") const;
	String strip(const String& chars = " \t\n\r") const;

	// split the string on the first occurrence of one of "chars".  return the word,
	// or put word into the reference String and return the remainder.  if string
	// starts with one of "chars", they will be trimmed (return and word will be stripped)
	String lsplit(const String& chars = " ") const;
	String rsplit(const String& chars = " ") const;
	String lsplit(String& word, const String& chars = " ") const;
	String rsplit(String& word, const String& chars = " ") const;

	// replace some or all occurrences of "what" with "with", -1 is all
	String replace(const String& what, const String& with, int times = -1) const;

	// insert a string at a specified point
	String insert(const String& what, std::size_t pos) const;

	// center the string in whitespace, color ignored.  truncates if too long
	String center(std::size_t total_len) const;

	// Actable
	virtual std::string identifier() const { return *this; }

private:
	// don't allow NULL operations
	String(std::nullptr_t);
	String& operator=(std::nullptr_t);
	friend bool operator== (const String &, std::nullptr_t) = delete;
	friend bool operator!= (const String &, std::nullptr_t) = delete;
};

// compatibility with legacy char array functions
#include <cstdlib> // atoi

inline char *strcat(char *dest, const String& src) {
	return std::strcat(dest, src.c_str());
}

inline char *strncat(char *dest, const String& src, size_t n) {
	return std::strncat(dest, src.c_str(), n);
}

inline char *strcpy(char *dest, const String& src) {
	return std::strcpy(dest, src.c_str());
}

inline char *strncpy(char *dest, const String& src, size_t n) {
	return std::strncpy(dest, src.c_str(), n);
}

inline size_t strlen(const String& str) {
	return str.size();
}

inline const char *strchr(const String& str, int ch) {
	return std::strchr(str.c_str(), ch);
}

inline const char *strstr(const String& astr, const String& bstr) {
	return std::strstr(astr.c_str(), bstr.c_str());
}

inline char *strstr(char *astr, const String& bstr) {
	return std::strstr(astr, bstr.c_str());
}

inline int strcmp(const String& astr, const String& bstr) {
	return std::strcmp(astr.c_str(), bstr.c_str());
}

inline int strncmp(const String& astr, const String& bstr, size_t n) {
	return std::strncmp(astr.c_str(), bstr.c_str(), n);
}

inline int strcasecmp(const String& astr, const String& bstr) {
	return strcasecmp(astr.c_str(), bstr.c_str());
}

inline int atoi(const String& astr) {
	return atoi(astr.c_str());
}
