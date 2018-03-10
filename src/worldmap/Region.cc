#include "worldmap/Region.hh"
#include "file.hh"
#include "Area.hh"
#include "Room.hh"
#include "Logging.hh"
#include "World.hh"
#include "RoomPrototype.hh"

namespace worldmap {

Region::Region(Area& area, FILE *fp) : area(area) {
	String rooms_file = fread_string(fp);

	image.load(String(WORLDMAP_DIR) + rooms_file);

	_align_x = fread_number(fp);
	_align_y = fread_number(fp);

	for (; ;) {
		char letter;
		if ((letter = fread_letter(fp)) == 'S')
			break;

		if (letter == '*') {
			fread_to_eol(fp);
			continue;
		}

		if (letter == 'M') {
			int num;

			while ((num = fread_number(fp)) != 0)
				allowed_mob_resets.insert(num);

			continue;
		}

		if (letter == 'R') {
			int num;

			while ((num = fread_number(fp)) != 0)
				allowed_room_resets.insert(num);

			continue;
		}

		if (letter == 'C') {
			int vnum  = fread_number(fp);
			int red   = fread_number(fp);
			int green = fread_number(fp);
			int blue  = fread_number(fp);

			MapColor color(red, green, blue);

			if (_prototype_color_map.find(vnum) != _prototype_color_map.cend()) {
				Logging::bugf("load_region(): vnum %d redefined with another pixel color", vnum);
				continue;
			}

			_prototype_color_map.emplace(vnum, MapColor(red, green, blue));
			continue;
		}
	}

	// check for color duplicates.  this is terrible O(n^2), but this map should be very small.
	// rewrite with boost::multimap if we ever start using boost.
	for (const auto& pair1 : _prototype_color_map) {
		for (const auto& pair2 : _prototype_color_map) {
			if (pair1.first == pair2.first)
				continue;
			if (pair1.second == pair2.second) {
				Logging::bugf("load_region(): multiple definition of color with vnums %d and %d",
					pair1.first, pair2.first);
				continue;
			}
		}
	}
}	

const MapColor& Region::vnum_to_color(const Vnum& vnum) const {
	const auto pair = _prototype_color_map.find(vnum);

	if (pair == _prototype_color_map.cend())
		return MapColor::uncolored;

	return pair->second;
}

const Vnum Region::color_to_vnum(const MapColor& color) const {
	if (color == MapColor::uncolored)
		return 0; // not an error, just no room there

	for (const auto& pair : _prototype_color_map)
		if (pair.second == color)
			return pair.first;

	return -1; // error, vnum not found, bad pixel color
}

const MapColor Region::get_color(unsigned int x, unsigned int y) const {
	if (image.value(util::Image::alpha, x, y) > 0) {
		return MapColor(
			image.value(util::Image::red, x, y),
			image.value(util::Image::green, x, y),
			image.value(util::Image::blue, x, y)
		);
	}

	return MapColor::uncolored;
}

const Vnum Region::get_vnum(unsigned int x, unsigned int y) const {
	return color_to_vnum(get_color(x, y));
}

void Region::
create_rooms() const {
	for (unsigned int y = 0; y < image.height(); y++) {
		for (unsigned int x = 0; x < image.width(); x++) {
			MapColor color = get_color(x, y);
			Vnum vnum = color_to_vnum(color);

			if (vnum == 0) {
				continue;
			}

			if (vnum == -1) {
				Logging::bugf("load_region(): area %s has undefined region pixel at %d,%d with color %d,%d,%d",
					area.file_name, x, y, color.red, color.green, color.blue);
				continue;
			}

			const auto& pair = area.room_prototypes.find(vnum);

			if (pair == area.room_prototypes.cend()) {
				Logging::bugf("load_region(): area %s has no prototype defined for vnum %d at region pixel %d,%d",
					area.file_name, vnum, x, y);
				continue;
			}

			auto prototype = pair->second;

			if (prototype->count > (1 << 14)-1) { // max 16k rooms per prototype, only 14 bits
				Logging::bugf("load_region(): prototype vnum %d has more than %d rooms",
					prototype->vnum, (1 << 14)-1);
				exit(1);
			}

			Room *room = new Room(*prototype);
			room->location.coord = Coordinate(align_x()+x, align_y()+y);
			area.rooms.emplace(room->location.room_id, room); // owned by the area
			area.world.maptree.put(room, room->location.coord); // but indexed by the tree
		}
	}
}

} // namespace worldmap
