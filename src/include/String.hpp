#pragma once

#include <string>
#include "Actable.hpp"

class String: public std::string, public Actable {
public:
	String() : std::string() {};
	String(const char *c_str) : std::string(c_str) {};
	String(const std::string &s_str) : std::string(s_str) {};
	String(const String &name) : std::string(name) {};

	virtual ~String() {}

	String& operator=(char ch) {
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

	// TODO: sorting comparison operators

	// String transformations, each return a new String object.  NO modify in place ops

	// capitalize the first letter of a string, skips non-alphanumerics
	String capitalize() const;

	// trim the whitespace (or user-defined characters) from the front, back, or both
	String lstrip(const char *chars = " \t\n\r") const;
	String rstrip(const char *chars = " \t\n\r") const;
	String strip(const char *chars = " \t\n\r") const;

	// split the string on the first occurrence of one of "chars".  return the word,
	// or put word into the reference String and return the remainder
	String lsplit(const char *chars = " ") const;
	String rsplit(const char *chars = " ") const;
	String lsplit(String& word, const char *chars = " ") const;
	String rsplit(String& word, const char *chars = " ") const;

	// replace some or all occurrences of "what" with "with", -1 is all
	String replace(const String& what, const String& with, int times = -1) const;

	// Actable
	virtual std::string identifier() const { return *this; }
};
