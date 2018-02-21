#pragma once

/* internally, a vnum is a signed long so that its value can be stored in an ObjectValue for portals
   and other stuff.  However, we only allow it to be created with an unsigned long, and we reserve
   the top bit to always be 0.  Later if we continue double-purposing vnums as room identifiers,
   we can use the top bits while reserving the very top bit.
*/

class Vnum {
public:
	Vnum(unsigned long v) : _value(constrain_value(v)) {}

	virtual ~Vnum() {}
	Vnum(const Vnum& v) : _value(constrain_value(v._value)) {}
	Vnum& operator=(const Vnum& v) { _value = constrain_value(v._value); return *this; }

	long value() const { return _value; }

	inline friend bool  operator<(const Vnum& lhs, const Vnum& rhs) { return lhs._value < rhs._value; }
	inline friend bool  operator>(const Vnum& lhs, const Vnum& rhs) { return rhs < lhs; }
	inline friend bool operator<=(const Vnum& lhs, const Vnum& rhs) { return !(lhs > rhs); }
	inline friend bool operator>=(const Vnum& lhs, const Vnum& rhs) { return !(lhs < rhs); }
	inline friend bool operator==(const Vnum& lhs, const Vnum& rhs) { return lhs._value == rhs._value; }
	inline friend bool operator!=(const Vnum& lhs, const Vnum& rhs) { return !(lhs == rhs); }

private:
	static unsigned long constrain_value(unsigned long v);

	long _value = 0;
};
