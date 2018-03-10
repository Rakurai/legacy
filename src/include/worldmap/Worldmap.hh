#pragma once

#include "Sector.hh"

class MapColor;

namespace worldmap {

class Coordinate;

class Worldmap {
public:
	Worldmap();
	virtual ~Worldmap();

	const Sector get_sector(const Coordinate& coord) const;

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
