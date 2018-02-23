#pragma once

#include "String.hh"

namespace worldmap {

class Coordinate {
public:
	Coordinate() : x(-1), y(0) {}

	Coordinate(int nx, int ny) : Coordinate() {
		if (nx < 0 || (nx >> num_bits_coord()) > 0)
			return; // error

		if (ny < 0 || (ny >> num_bits_coord()) > 0)
			return; // error

		x = nx; y = ny;
	}

	// deserialize from an integer
	Coordinate(int value) : Coordinate() {
		if (value < 0)
			return; // invalid

		// 32nd bit from right is 0
		// test the 31st bit from right, 1 indicates a coordinate
		if ((value >> 30) == 0)
			return; // invalid

		// this is a coordinate pair, grab last 30 bits
		int mask = (1 << 15)-1; // 17 0s and 15 1s
		x = (value >> 15) & mask;
		y = value & mask;
	}

	Coordinate(const String& str) : Coordinate() {
		if (!str.has_infix(","))
			return; // invalid

		String x_str;
		String y_str = str.lsplit(x_str, ",");

		if (!x_str.is_number()
		 || !y_str.is_number())
			return; // invalid

		*this = Coordinate(atoi(x_str), atoi(y_str));
	}

	// serialize to integer.  leftmost bit indicates validity,
	// next bit always 1, next 15 are x, last 15 are y
	int to_int() const {
		if (!is_valid())
			return -1;

		int ret = 1 << 30; // set 31st bit
		ret |= (x << 15);
		ret |= y;
		return ret;
	}

	const String to_string(bool short_loc = true) const;

	Coordinate(const Coordinate& c) : x(c.x), y(c.y) {}
	Coordinate& operator=(const Coordinate& c) { x = c.x; y = c.y; return *this; }
	virtual ~Coordinate() {}

	inline bool is_valid() const { return !(x < 0); }

	inline friend bool operator==(const Coordinate& lhs, const Coordinate& rhs) {
		return lhs.x == rhs.x && lhs.y == rhs.y;
	}
	inline friend bool operator!=(const Coordinate& lhs, const Coordinate& rhs) {
		return !(lhs == rhs);
	}

	short x;
	short y;

private:
	inline int num_bits_coord() const { return 15; }

};

} // namespace worldmap
