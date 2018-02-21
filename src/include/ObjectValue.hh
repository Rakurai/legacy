#pragma once

#include "declare.hh"
#include "skill/Type.hh"
#include "Vnum.hh"

class ObjectValue {
public:
	ObjectValue() {}
	explicit ObjectValue(int v) : _value(v) {}
	explicit ObjectValue(long v) : _value(v) {}
//	explicit ObjectValue(const Vnum& v) : _value(v.value()) {}
	explicit ObjectValue(const Flags& v);
	ObjectValue(const ObjectValue& v) : _value(v._value), _as_flags(v._as_flags) {}
	virtual ~ObjectValue() {}

	const long value() const { return _value; }
	const Flags flags() const;

//	operator int() const { return value(); }
	operator long() const { return value(); }
	operator Flags() const;
	operator Vnum() const { return Vnum(value()); }

	ObjectValue& operator=(long rhs) { this->_value = rhs; return *this; }
	ObjectValue& operator=(const Vnum& rhs) { this->_value = rhs.value(); return *this; }
	ObjectValue& operator=(const Flags& rhs);

	// the only way to change _as_flags once constructed.  used in area loading
	// to assign to an ObjectPrototype, and then when creating Objects from
	// the prototype.  Prevents accidental reassignment of the type.
	ObjectValue& operator=(const ObjectValue& rhs) {
		_value = rhs._value;
		_as_flags = rhs._as_flags;
		return *this;
	}

	ObjectValue& operator+=(long rhs) { *this += ObjectValue(rhs); return *this; }
	ObjectValue& operator+=(const Flags& rhs) { *this += ObjectValue(rhs); return *this; }
	ObjectValue& operator+=(const ObjectValue &rhs);

	ObjectValue& operator-=(long rhs) { *this -= ObjectValue(rhs); return *this; }
	ObjectValue& operator-=(const Flags& rhs) { *this -= ObjectValue(rhs); return *this; }
	ObjectValue& operator-=(const ObjectValue &rhs);

	ObjectValue& operator^=(const Flags& rhs);

	ObjectValue& operator++() { ++_value; return *this; }
	const ObjectValue operator++(int) { ObjectValue copy(*this); ++_value; return copy; }
	ObjectValue& operator--() { --_value; return *this; }
	const ObjectValue operator--(int) { ObjectValue copy(*this); --_value; return copy; }

	const ObjectValue operator+(long rhs) { return ObjectValue(*this) += rhs; }
	const ObjectValue operator+(const Flags& rhs) { return ObjectValue(*this) += rhs; }
	const ObjectValue operator+(const ObjectValue& rhs) { return ObjectValue(*this) += rhs; }

	const ObjectValue operator-(long rhs) { return ObjectValue(*this) -= rhs; }
	const ObjectValue operator-(const Flags& rhs) { return ObjectValue(*this) -= rhs; }
	const ObjectValue operator-(const ObjectValue& rhs) { return ObjectValue(*this) -= rhs; }

	friend bool operator== (const ObjectValue&, const ObjectValue&);
	friend bool operator!= (const ObjectValue&, const ObjectValue&);
//	friend bool operator== (const ObjectValue&, long);
//	friend bool operator!= (const ObjectValue&, long);

private:
	long _value = 0;
	bool _as_flags = false;
};

inline bool operator==(const ObjectValue& lhs, const ObjectValue& rhs) {
	return lhs._value == rhs._value;
}

inline bool operator!=(const ObjectValue& lhs, const ObjectValue& rhs) {
	return !(lhs == rhs);
}
/*
inline bool operator==(const ObjectValue& lhs, long rhs) {
	return lhs._value == rhs;
}
inline bool operator!=(const ObjectValue& lhs, long rhs) {
	return !(lhs._value == rhs);
}
*/