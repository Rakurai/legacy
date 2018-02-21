#pragma once

namespace worldmap {

class Coordinate {
public:
	Coordinate() : x(-1), y(-1) {}
	Coordinate(short x, short y) : x(x), y(y) {}
	Coordinate(const Coordinate& c) : x(c.x), y(c.y) {}
	Coordinate& operator=(const Coordinate& c) { x = c.x; y = c.y; return *this; }
	virtual ~Coordinate() {}

	short x;
	short y;

	bool is_valid() const { return x >= 0; } // we use -1 to mark an invalid pair
};

} // namespace worldmap
