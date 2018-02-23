#pragma once

#include "Sector.hh"
#include "Coordinate.hh"

namespace worldmap {

class Worldmap {
public:
	Worldmap();
	virtual ~Worldmap();

	const Sector get_sector(const Coordinate& coord) const {
		unsigned char ret = 0;

		if (coord.is_valid()
		 && (unsigned int)coord.x < width()
		 && (unsigned int)coord.y < height())
			ret = rows[coord.y][coord.x];

		return static_cast<Sector>(ret);
	}

	unsigned int height() const { return _height; }
	unsigned int width() const { return _width; }

private:
	Worldmap(const Worldmap&);
	Worldmap& operator=(const Worldmap&);

	const Sector color_to_sector(const MapColor& color) const;
	unsigned int _height, _width;
	unsigned char ** rows = nullptr;
};

} // namespace worldmap
