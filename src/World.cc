#include "World.hh"

#include "Area.hh"
#include "Character.hh"
#include "worldmap/Coordinate.hh"
#include "Logging.hh"
#include "db.hh"

extern std::time_t current_time;

World::
World() :
	time(current_time),
	weather(time),
	overworld(),
	maptree(overworld.width())
{}

World::
~World() {
	for (Area *area: areas)
		delete area;
}

void World::
update() {
	for (Area *area: areas)
		area->update();
}

Area * World::
get_area(const Vnum& vnum) const {
	// it would be really nice if we could sort the areas on their vnum ranges
	// and then do a binary search here, but i have bigger fish to fry.  there
	// should only be 100 or so areas anyway
	for (Area * area : areas)
		if (vnum >= area->min_vnum && vnum <= area->max_vnum)
			return area;

	return nullptr;
}

MobilePrototype * World::
get_mob_prototype(const Vnum& vnum)
{
	Area *area = get_area(vnum);
	MobilePrototype *proto = nullptr;

	if (area != nullptr)
		proto = area->get_mob_prototype(vnum);

	if (proto == nullptr && fBootDb) {
		Logging::bugf("Get_mob_index: bad vnum %d.", vnum);
	}

	return proto;
}

ObjectPrototype * World::
get_obj_prototype(const Vnum& vnum)
{
	Area *area = get_area(vnum);
	ObjectPrototype *proto = nullptr;

	if (area != nullptr)
		proto = area->get_obj_prototype(vnum);

	if (proto == nullptr && fBootDb) {
		Logging::bugf("Get_obj_index: bad vnum %d.", vnum);
	}

	return proto;
}

RoomPrototype * World::
get_room_prototype(const Vnum& vnum) {
	Area *area = get_area(vnum);
	RoomPrototype *proto = nullptr;

	if (area != nullptr)
		proto = area->get_room_prototype(vnum);

	if (proto == nullptr && fBootDb) {
		Logging::bugf("Game::world().get_room_prototype: bad vnum %d.", vnum);
	}

	return proto;
}

Room * World::
get_room(const Location& location)
{
	if (location.coord.is_valid())
		return maptree.get(location.coord);

	if (!location.room_id.is_valid())
		return nullptr;

	// it would be nice if we could figure out a way to quickly identify
	// the area from location without an O(n) search
	for (auto area : areas) {
		Room *room = area->get_room(location.room_id);

		if (room != nullptr)
			return room;
	}

	return nullptr;
}

void World::
get_minimap(Character *ch, std::vector<String>& vec) const {
	const int map_width = 30;
	const int map_height = 22;

	if (!ch->in_room->is_on_map())
		return;

	worldmap::Coordinate start_coord = ch->in_room->location.coord;

	// this could be a lot faster if we just allocated an array and filled
	for (int y = start_coord.y - map_height/2; y < (int)(start_coord.y + map_height/2); y++) {
		String buf;

		// write the map line
		for (int x = start_coord.x - map_width/2; x < (int)(start_coord.x + map_width/2); x++) {

			if (x < 0 || (unsigned int)x >= overworld.width()
			 || y < 0 || (unsigned int)y >= overworld.height()) {
				buf += ' ';
				continue;
			}

			worldmap::Coordinate this_coord(x, y);

			if (this_coord == start_coord) {
				buf += "{PX";
				continue;
			}

			const Room *room = maptree.get(this_coord);
			Character *enemy = nullptr;

			if (room != nullptr) {
				for (enemy = room->people; enemy; enemy = enemy->next_in_room)
					if (can_see_char(ch, enemy))
						break;
			}

			if (enemy == nullptr) {
				Sector terrain = overworld.get_sector(this_coord);

				switch (terrain) {
					case Sector::forest_sparse:
					case Sector::field:     buf += "{G."; break;

					case Sector::forest_medium:    buf += "{GI"; break;
					case Sector::forest_dense:    buf += ((x + (y%2)) % 2 == 0) ? "{HI" : "{H&"; break;
					case Sector::road:            buf += "{b%"; break;
					case Sector::area_entrance:   buf += "{Y#"; break;
					case Sector::city:            buf += "{gM"; break;
					default:                buf += " ";   break;
				}
			}
			else {
				if (IS_NPC(enemy))
					buf += "{R+";
				else
					buf += "{CP";
			}
		}

		buf += "{x";
		vec.push_back(buf);
	}
}
