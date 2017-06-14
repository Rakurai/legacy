#pragma once

#include "declare.h"

class ObjectValue {
public:
	virtual ~ObjectValue() {}
//	ObjectValue(int v) : _value(v), _as_flags(false) {}
//	ObjectValue(const Flags& v);
//	ObjectValue(const ObjectValue& v) : _value(v._value), _as_flags(v._as_flags) {}

//	operator int() const { return _value; }
//	operator Flags() const;

	virtual int value() const = 0;
	virtual void value(int v) = 0;
	virtual const char *id() = 0;

	operator int() const { return value(); }
	operator Flags() const;

	ObjectValue& operator=(int rhs) { assign(rhs); return *this; }
	ObjectValue& operator=(const Flags& rhs) { assign(rhs); return *this; }
	ObjectValue& operator=(const ObjectValue& rhs) { assign(rhs); return *this; }
	ObjectValue& operator+=(int rhs) { add(rhs); return *this; }
	ObjectValue& operator+=(const Flags& rhs) { add(rhs); return *this; }
	ObjectValue& operator+=(const ObjectValue &rhs) { add(rhs); return *this; }
	ObjectValue& operator-=(int rhs) { sub(rhs); return *this; }
	ObjectValue& operator-=(const Flags& rhs) { sub(rhs); return *this; }
	ObjectValue& operator-=(const ObjectValue &rhs) { sub(rhs); return *this; }
	ObjectValue& operator++() { incr(); return *this; }
//	ObjectValue  operator++(int) { ObjectValue copy(*this); inc(); return copy; }
	ObjectValue& operator--() { decr(); return *this; }
//	ObjectValue  operator--(int) { ObjectValue copy(*this); decr(); return copy; }

	virtual const int operator+(int rhs) const = 0;
	virtual const int operator+(const Flags& rhs) const = 0;
	virtual const int operator+(const ObjectValue &rhs) const = 0;
	virtual const int operator-(int rhs) const = 0;
	virtual const int operator-(const Flags& rhs) const = 0;
	virtual const int operator-(const ObjectValue &rhs) const = 0;

	friend bool operator== (const ObjectValue&, const ObjectValue&);
	friend bool operator!= (const ObjectValue&, const ObjectValue&);

protected:
//	ObjectValue(int v) : _value(v), _as_flags(false) {}
//	ObjectValue(const Flags& v);
//	ObjectValue(const ObjectValue& v); // : _value(v._value), _as_flags(v._as_flags) {}
	virtual void incr() = 0;
	virtual void decr() = 0;

	virtual void add(int v) = 0;
	virtual void sub(int v) = 0;
	virtual void assign(int v) = 0;
	virtual void add(const Flags &v) = 0;
	virtual void sub(const Flags &v) = 0;
	virtual void assign(const Flags &v) = 0;
	virtual void add(const ObjectValue &v) = 0;
	virtual void sub(const ObjectValue &v) = 0;
	virtual void assign(const ObjectValue &v) = 0;
};

class IntObjectValue : public ObjectValue {
public:
	IntObjectValue() {}
	IntObjectValue(int v) : _value(v) {}
	IntObjectValue(const Flags& v);
	IntObjectValue(const IntObjectValue& v) : _value(v._value) {}
	virtual ~IntObjectValue() {}

	virtual int value() const { return _value; }
	virtual void value(int v) { _value = v; }
	virtual const char *id() { return "int"; }

	virtual const int operator+(int rhs) const { return *this + IntObjectValue(rhs); }
	virtual const int operator+(const Flags& rhs) const { return *this + IntObjectValue(rhs); }
	virtual const int operator+(const ObjectValue &rhs) const { return value() + rhs.value(); }
	virtual const int operator-(int rhs) const { return *this - IntObjectValue(rhs); }
	virtual const int operator-(const Flags& rhs) const { return *this - IntObjectValue(rhs); }
	virtual const int operator-(const ObjectValue &rhs) const { return value() - rhs.value(); }

private:
	int _value = 0;
	bool _as_flags = false;

	virtual void incr() { _value++; }
	virtual void decr() { _value--; }

	virtual void add(int v) { add(IntObjectValue(v)); }
	virtual void sub(int v) { sub(IntObjectValue(v)); }
	virtual void assign(int v) { assign(IntObjectValue(v)); }
	virtual void add(const Flags &v) { add(IntObjectValue(v)); }
	virtual void sub(const Flags &v) { sub(IntObjectValue(v)); }
	virtual void assign(const Flags &v) { assign(IntObjectValue(v)); }
	virtual void add(const ObjectValue &v) { _value += v.value(); }
	virtual void sub(const ObjectValue &v) { _value -= v.value(); }
	virtual void assign(const ObjectValue &v) { _value = v.value(); }
};

class FlagsObjectValue : public ObjectValue {
public:
	FlagsObjectValue() {}
	FlagsObjectValue(int v) : _value(v) {}
	FlagsObjectValue(const Flags& v);
	FlagsObjectValue(const FlagsObjectValue& v) : _value(v._value) {}
	virtual ~FlagsObjectValue() {}

	virtual int value() const { return _value; }
	virtual void value(int v) { _value = v; }
	virtual const char *id() { return "flags"; }

	virtual const int operator+(int rhs) const { return *this + FlagsObjectValue(rhs); }
	virtual const int operator+(const Flags& rhs) const { return *this + FlagsObjectValue(rhs); }
	virtual const int operator+(const ObjectValue &rhs) const { return value() + rhs.value(); }
	virtual const int operator-(int rhs) const { return *this - FlagsObjectValue(rhs); }
	virtual const int operator-(const Flags& rhs) const { return *this - FlagsObjectValue(rhs); }
	virtual const int operator-(const ObjectValue &rhs) const { return value() - rhs.value(); }

private:
	int _value = 0;
	bool _as_flags = false;

	virtual void incr() { _value <<= 1; }
	virtual void decr() { _value >>= 1; }

	virtual void add(int v) { add(FlagsObjectValue(v)); }
	virtual void sub(int v) { sub(FlagsObjectValue(v)); }
	virtual void assign(int v) { assign(FlagsObjectValue(v)); }
	virtual void add(const Flags &v) { add(FlagsObjectValue(v)); }
	virtual void sub(const Flags &v) { sub(FlagsObjectValue(v)); }
	virtual void assign(const Flags &v) { assign(FlagsObjectValue(v)); }
	virtual void add(const ObjectValue &v) { _value += v.value(); }
	virtual void sub(const ObjectValue &v) { _value -= v.value(); }
	virtual void assign(const ObjectValue &v) { _value = v.value(); }
};

inline bool operator== (const ObjectValue& lhs, const ObjectValue& rhs) {
	return lhs.value() == rhs.value();
}
inline bool operator!= (const ObjectValue& lhs, const ObjectValue& rhs) {
	return lhs.value() != rhs.value();
}
