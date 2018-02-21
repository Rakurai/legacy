#pragma once

namespace worldmap {

class MapColor {
public:
	MapColor(unsigned int r, unsigned int g, unsigned int b) :
		red(r), green(g), blue(b) {}

	virtual ~MapColor() {}

	MapColor(const MapColor& m) :
		red(m.red), green(m.green), blue(m.blue) { }

	MapColor& operator=(const MapColor& m) {
		red = m.red;
		blue = m.blue;
		green = m.green;
		return *this;
	}

	unsigned int red, green, blue;
	static const MapColor uncolored;

	bool operator==(const MapColor& rhs) const {
		return this->red == rhs.red
		    && this->green == rhs.green
		    && this->blue == rhs.blue;
	}

	bool operator!=(const MapColor& rhs) const {
		return !(*this == rhs);
	}

};

} // namespace worldmap
