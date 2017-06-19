#pragma once

#include <bitset>
#include "declare.h"
#include "ObjectValue.hpp"

#define FLAGS_NBITS 32

class Flags
{
public:
	enum class Bit : long {
		none = 0,
		A = 1,
		B = 2,
		C = 4,
		D = 8,
		E = 16,
		F = 32,
		G = 64,
		H = 128,
		I = 256,
		J = 512,
		K = 1024,
		L = 2048,
		M = 4096,
		N = 8192,
		O = 16384,
		P = 32768,
		Q = 65536,
		R = 131072,
		S = 262144,
		T = 524288,
		U = 1048576,
		V = 2097152,
		W = 4194304,
		X = 8388608,
		Y = 16777216,
		Z = 33554432,
		a = 67108864,
		b = 134217728,
		c = 268435456,
		d = 536870912,
		e = 1073741824,
		f = 2147483648,
		g = 4294967296,
		all = A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z|a|b|c|d|e|f|g

		bool is_any_of(const Flags& f) { return f.has_all_of(*this); }
	};

	Flags() {}
	explicit Flags(int f) : negative(f < 0), bits(f) {}
	explicit Flags(long f) : negative(f < 0), bits(f) {}
	explicit Flags(unsigned int f) : bits(f) {}
	explicit Flags(unsigned long f) : bits(f) {}
	Flags(void *p) : Flags() {} // allow implicit ctor with '0'
	Flags(const Bit& f) : bits(long(f)) {}
//	Flags(const std::bitset<FLAGS_NBITS>& b) : bits(b) {}
	Flags(const Flags& f) : negative(f.negative), bits(f.bits) {}
	Flags(const String& s);
	virtual ~Flags() {}

	Flags& operator=(const Flags& f) {
		negative = f.negative;
		bits = f.bits;
		return *this;
	}

	Flags& operator=(const String& s) {
		*this = Flags(s);
		return *this;
	}

	// auto conversion
//	operator bool() const { return bits.any(); }
	unsigned long to_ulong() const { return bits.to_ulong(); }

	Flags& operator+= (const Flags& rhs) {
		negative ? bits &= ~rhs.bits : bits |= rhs.bits; 
		return *this;
	}
	Flags& operator-= (const Flags& rhs) {
		negative ? bits |= rhs.bits : bits &= ~rhs.bits;
		return *this;
	}
	Flags& operator^= (const Flags& rhs) {
		bits ^= rhs.bits;
		return *this;
	}
	const Flags operator+ (const Flags& rhs) const { return Flags(*this) += rhs; }
	const Flags operator- (const Flags& rhs) const { return Flags(*this) -= rhs; }

	const Flags operator+ (const Bit& rhs) const { return Flags(*this) += Flags(rhs); }
	const Flags operator| (const Bit& rhs) const { return Flags(*this) += Flags(rhs); }

	friend bool operator== (const Flags&, const Flags&);
	friend bool operator!= (const Flags&, const Flags&);

	bool has_all_of (const Flags& f) const { return (bits & f.bits) == f.bits; }
	bool has_any_of (const Flags& f) const { return (bits & f.bits) != 0; }
	bool has_none_of(const Flags& f) const { return (bits & f.bits) == 0; }
	bool has_all() const { return bits.all(); }
	bool has_any() const { return bits.any(); }
	bool has_none() const { return bits.none(); }

	void clear() { bits.reset(); negative = false; }
	const String to_string() const;

private:
	bool negative = false;
	std::bitset<FLAGS_NBITS> bits = {0};
};

inline bool IS_SET(const Flags& lhs, const Flags& rhs) {
	return lhs.has_all_of(rhs);
}

inline void SET_BIT(Flags& lhs, const Flags& rhs) {
	lhs += rhs;
}

inline void REMOVE_BIT(Flags& lhs, const Flags& rhs) {
	lhs -= rhs;
}

inline bool IS_SET(const ObjectValue& lhs, const Flags& rhs) {
	return lhs.flags().has_all_of(rhs);
}

inline void SET_BIT(ObjectValue& lhs, const Flags& rhs) {
	lhs += rhs;
}

inline void REMOVE_BIT(ObjectValue& lhs, const Flags& rhs) {
	lhs -= rhs;
}


inline bool operator== (const Flags& lhs, const Flags& rhs) {
	return lhs.bits == rhs.bits;
}

inline bool operator!= (const Flags& lhs, const Flags& rhs) {
	return lhs.bits != rhs.bits;
}

inline const Flags operator+ (const Flags::Bit& lhs, const Flags::Bit& rhs) {
	return Flags(lhs) + rhs;
}

inline const Flags operator| (const Flags::Bit& lhs, const Flags::Bit& rhs) {
	return Flags(lhs) + rhs;
}
//#define IS_SET(flag, bit)       ((flag) & (bit))
//#define SET_BIT(var, bit)       ((var) |= (bit))
//#define REMOVE_BIT(var, bit)    ((var) &= ~(bit))
//inline bool IS_SET(unsigned long lhs, unsigned long rhs) { return lhs & rhs; }
//inline void SET_BIT(int &lhs, const Flags& rhs) { lhs = (Flags(lhs) += rhs).to_ulong(); }
//inline void REMOVE_BIT(int &lhs, const Flags& rhs) { lhs = (Flags(lhs) -= rhs).to_ulong(); }
//inline void SET_BIT(long &lhs, const Flags& rhs) { lhs = (Flags(lhs) += rhs).to_ulong(); }
//inline void REMOVE_BIT(long &lhs, const Flags& rhs) { lhs = (Flags(lhs) -= rhs).to_ulong(); }
//inline void SET_BIT(unsigned int &lhs, const Flags& rhs) { lhs = (Flags(lhs) += rhs).to_ulong(); }
//inline void REMOVE_BIT(unsigned int &lhs, const Flags& rhs) { lhs = (Flags(lhs) -= rhs).to_ulong(); }
//inline void SET_BIT(unsigned long &lhs, const Flags& rhs) { lhs = (Flags(lhs) += rhs).to_ulong(); }
//inline void REMOVE_BIT(unsigned long &lhs, const Flags& rhs) { lhs = (Flags(lhs) -= rhs).to_ulong(); }
