#pragma once

namespace worldmap {

class MapColor {
public:
	MapColor(unsigned int r, unsigned int g, unsigned int b) :
		red(r), green(g), blue(b) { precompute(); }

	virtual ~MapColor() {}

	MapColor(const MapColor& m) :
		red(m.red), green(m.green), blue(m.blue) { precompute(); }

	MapColor& operator=(const MapColor& m) {
		red = m.red;
		blue = m.blue;
		green = m.green;
		precompute();
		return *this;
	}

	unsigned int red, green, blue;
	static const MapColor uncolored;

	inline bool operator==(const MapColor& rhs) const {
		return this->red == rhs.red
		    && this->green == rhs.green
		    && this->blue == rhs.blue;
	}

	inline bool operator!=(const MapColor& rhs) const {
		return !(*this == rhs);
	}

	inline bool operator<(const MapColor& rhs) const {
		return this->_computed < rhs._computed;
	}

private:
	void precompute() {
		_computed = (red << 16) + (green << 8) + blue;
	}

	unsigned int _computed = 0;
};

} // namespace worldmap
