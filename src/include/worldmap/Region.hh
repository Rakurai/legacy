#pragma once

#include <map>
#include <set>

#include "worldmap/MapColor.hh"
#include "Vnum.hh"
#include "util/Image.hh"

class Area;

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

	void create_rooms() const;
	std::set<Vnum> allowed_mob_resets; // list of mob vnums that can reset in this region
	std::set<Vnum> allowed_room_resets; // list of room vnums that can have mobs reset into

private:
	Region(const Region&);
	Region& operator=(const Region&);

	util::Image image;
	unsigned int _align_x;
	unsigned int _align_y;
	std::map<Vnum, MapColor> _prototype_color_map;
};

} // namespace worldmap
