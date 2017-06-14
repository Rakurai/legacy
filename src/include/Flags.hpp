#pragma once

#include <bitset>

#define FLAGS_NBITS 32

class String;

class Flags
{
public:
	enum Bit : unsigned long {
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
	};

	Flags() {}
	explicit Flags(unsigned long f) : bits(f) {}
//	Flags(void *p) : Flags() {} // allow implicit ctor with '0'
	Flags(const Bit& b) : bits(static_cast<unsigned long>(b)) {}
	Flags(const Flags& f) : bits(f.bits) {}
	Flags(const char *s);
	Flags(const String& s);
	virtual ~Flags() {}

	Flags& operator=(const Flags& f) { bits = f.bits; return *this; }

	Flags& operator+= (const Flags& rhs) { bits |=  rhs.bits; return *this; }
	Flags& operator-= (const Flags& rhs) { bits &= ~rhs.bits; return *this; }
	Flags& operator^= (const Flags& rhs) { bits ^=  rhs.bits; return *this; }

	const Flags operator+ (const Flags& rhs) const { return Flags(*this) += rhs; }
	const Flags operator- (const Flags& rhs) const { return Flags(*this) -= rhs; }

	const Flags operator| (const Bit& rhs) const { return Flags(*this) += Flags(rhs); }

	bool has_all_of (const Flags& f) const { return (bits & f.bits) == f.bits; }
	bool has_any_of (const Flags& f) const { return (bits & f.bits) != 0; }
	bool has_none_of(const Flags& f) const { return (bits & f.bits) == 0; }
	bool has(const Flags::Bit& b) const { return has_all_of(b); }

	bool empty() const { return bits.none(); }
	void clear() { bits.reset(); }

	unsigned long to_ulong() const { return bits.to_ulong(); }
	const String to_string() const;

	friend bool operator== (const Flags&, const Flags&);
	friend bool operator!= (const Flags&, const Flags&);
	friend std::ostream& operator<<(std::ostream&, const Flags&);

private:
	std::bitset<FLAGS_NBITS> bits = {0};
};

inline bool operator== (const Flags& lhs, const Flags& rhs) {
	return lhs.bits == rhs.bits;
}

inline bool operator!= (const Flags& lhs, const Flags& rhs) {
	return lhs.bits != rhs.bits;
}

inline std::ostream& operator<<(std::ostream& os, const Flags& f) {
	os << f.to_ulong();
	return os;
}

inline const Flags operator+ (const Flags::Bit& lhs, const Flags::Bit& rhs) {
	return Flags(lhs) + rhs;
}

inline const Flags operator| (const Flags::Bit& lhs, const Flags::Bit& rhs) {
	return Flags(lhs) + rhs;
}
