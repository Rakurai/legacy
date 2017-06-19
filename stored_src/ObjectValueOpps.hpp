#pragma once

#include "declare.h"

class ObjectValue {
public:
	ObjectValue() {}
	explicit ObjectValue(int v) : _value(v) {}
	explicit ObjectValue(const Flags& v);
	ObjectValue(const ObjectValue& v) : _value(v._value), _as_flags(v._as_flags) {}
	virtual ~ObjectValue() {}

	int value() const { return _value; }
	const Flags flags() const;

	operator int() const { return value(); }
	operator Flags() const;

	ObjectValue& operator=(int rhs) { this->_value = rhs; return *this; }
	ObjectValue& operator=(const Flags& rhs);

	// the only way to change _as_flags once constructed.  used in area loading
	// to assign to an ObjectPrototype, and then when creating Objects from
	// the prototype.  Prevents accidental reassignment of the type.
	ObjectValue& operator=(const ObjectValue& rhs) {
		_value = rhs._value;
		_as_flags = rhs._as_flags;
		return *this;
	}

	ObjectValue& operator+=(int rhs) { *this += ObjectValue(rhs); return *this; }
	ObjectValue& operator+=(const Flags& rhs) { *this += ObjectValue(rhs); return *this; }
	ObjectValue& operator+=(const ObjectValue &rhs);

	ObjectValue& operator-=(int rhs) { *this -= ObjectValue(rhs); return *this; }
	ObjectValue& operator-=(const Flags& rhs) { *this -= ObjectValue(rhs); return *this; }
	ObjectValue& operator-=(const ObjectValue &rhs);

	ObjectValue& operator++() { ++_value; return *this; }
	const ObjectValue operator++(int) { ObjectValue copy(*this); ++_value; return copy; }
	ObjectValue& operator--() { --_value; return *this; }
	const ObjectValue operator--(int) { ObjectValue copy(*this); --_value; return copy; }

	const ObjectValue operator+(int rhs) { return ObjectValue(*this) += rhs; }
	const ObjectValue operator+(const Flags& rhs) { return ObjectValue(*this) += rhs; }
	const ObjectValue operator+(const ObjectValue& rhs) { return ObjectValue(*this) += rhs; }

	const ObjectValue operator-(int rhs) { return ObjectValue(*this) -= rhs; }
	const ObjectValue operator-(const Flags& rhs) { return ObjectValue(*this) -= rhs; }
	const ObjectValue operator-(const ObjectValue& rhs) { return ObjectValue(*this) -= rhs; }

	friend bool operator== (const ObjectValue&, const ObjectValue&);
	friend bool operator!= (const ObjectValue&, const ObjectValue&);
	friend bool operator== (const ObjectValue&, int);
	friend bool operator!= (const ObjectValue&, int);

private:
	int _value = 0;
	bool _as_flags = false;
};

inline bool operator==(const ObjectValue& lhs, const ObjectValue& rhs) {
	return lhs._value == rhs._value;
}

inline bool operator!=(const ObjectValue& lhs, const ObjectValue& rhs) {
	return !(lhs == rhs);
}

inline bool operator==(const ObjectValue& lhs, int rhs) {
	return lhs._value == rhs;
}
inline bool operator!=(const ObjectValue& lhs, int rhs) {
	return !(lhs._value == rhs);
}
