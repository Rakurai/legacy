#include "worldmap/Region.hh"
#include "file.hh"
#include "Area.hh"
#include "Room.hh"
#include "Logging.hh"
#include "World.hh"

namespace worldmap {

Region::Region(Area& area, FILE *fp) : area(area) {
	String rooms_file = fread_string(fp);

	image.load(rooms_file);

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

		if (letter == 'R') {
			int vnum  = fread_number(fp);
			int red   = fread_number(fp);
			int green = fread_number(fp);
			int blue  = fread_number(fp);

			_prototype_color_map.emplace(vnum, MapColor(red, green, blue));
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
	for (const auto& pair : _prototype_color_map)
		if (pair.second == color)
			return pair.first;

	return 0;
}

const MapColor Region::get_color(unsigned int x, unsigned int y) const {
	return MapColor(
		image.value(util::Image::red, x, y),
		image.value(util::Image::green, x, y),
		image.value(util::Image::blue, x, y)
	);
}

const Vnum Region::get_vnum(unsigned int x, unsigned int y) const {
	return color_to_vnum(get_color(x, y));
}

void Region::
load_rooms() const {
	for (unsigned int y = 0; y < image.height(); y++) {
		for (unsigned int x = 0; x < image.width(); x++) {
			MapColor color = get_color(x, y);
			Vnum vnum = color_to_vnum(color);

			if (vnum == 0) {
				Logging::bugf("got uncolored pixel at %d,%d\n", x, y);
				continue;
			}

			const auto& pair = area.room_prototypes.find(vnum);

			if (pair == area.room_prototypes.cend()) {
				Logging::bugf("load_region(): area %s has undefined region pixel at %d,%d with color %d,%d,%d",
					area.file_name, x, y, color.red, color.green, color.blue);
				continue;
			}

			auto prototype = pair->second;
			Room *room = new Room(*prototype, Coordinate(align_x()+x, align_y()+y));
			prototype->rooms.push_back(room); // owned by the prototype
			area.world.maptree.put(room, room->coord.x, room->coord.y); // but indexed by the tree
		}
	}
}

} // namespace worldmap
