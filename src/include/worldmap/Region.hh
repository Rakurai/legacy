#pragma once

#include <map>

#include "declare.hh"
#include "worldmap/MapColor.hh"
#include "Vnum.hh"
#include "util/Image.hh"

namespace worldmap {

class Region {
public:
	Region(Area& area, FILE *fp);
	virtual ~Region() {}

	Area& area;

	unsigned int align_x() const { return _align_x; }
	unsigned int align_y() const { return _align_y; }

	const MapColor& vnum_to_color(const Vnum& vnum) const;
	const Vnum color_to_vnum(const MapColor& color) const;
	const MapColor get_color(unsigned int x, unsigned int y) const;
	const Vnum get_vnum(unsigned int x, unsigned int y) const;

	void load_rooms() const;

private:
	Region(const Region&);
	Region& operator=(const Region&);

	util::Image image;
	unsigned int _align_x;
	unsigned int _align_y;
	std::map<Vnum, MapColor> _prototype_color_map;
};

} // namespace worldmap
