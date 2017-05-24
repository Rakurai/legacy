#pragma once

#include <string>

class String: public std::string {
public:
	String() : std::string() {};
	String(const char *c_str) : std::string(c_str) {};
	String(const std::string &s_str) : std::string(s_str) {};
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
	String capitalize() const;
//	const String& substr(size_type pos = 0, size_type count = npos) const;
	String lstrip() const;
	String rstrip() const;
	String strip() const;
};
