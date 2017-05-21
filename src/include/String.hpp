#pragma once

#include <string>

class String: public std::string {
public:
	String() : std::string() {};
	String(const char *c_str) : std::string(c_str) {};
	String(const String &name) : std::string(name) {};

	virtual ~String() {}

	String& operator=(const String& name) {
		std::string::operator=(name);
		return *this;
	};

	// the difference from std::string - perform case insensitive comparisons by default
	friend bool operator== (const String &, const String &);
	friend bool operator== (const String &, const char *);

	// TODO: sorting comparison operators
};

