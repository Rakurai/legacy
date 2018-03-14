#include "World.hh"

#include "Area.hh"
#include "Character.hh"
#include "worldmap/Coordinate.hh"
#include "Game.hh"
#include "Logging.hh"
#include "file.hh"
#include "Exit.hh"
#include "ExitPrototype.hh"
#include "RoomPrototype.hh"
#include "Room.hh"

World::
World() :
	time(Game::current_time),
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
	static int pulse_area = 0;

	if (--pulse_area <= 0) {
		pulse_area = PULSE_AREA;

		for (Area *area: areas)
			area->update();
	}

	// clean up things that have been destroyed
	// it would be really nice to shove these into a clean_list method that just
	// takes any list of things deriving from Valid, which would be fine because
	// we use virtual destructors everywhere, except it would mean using a
	// static cast to the derived type everywhere we iterate over the list.
	// unless of course we templatized some sort of helper function, but this
	// is fine for now.
	// see: https://stackoverflow.com/questions/25355154/c11-range-based-for-loop-on-derived-objects
	for (auto it = pc_list.begin(); it != pc_list.end(); ) {
		auto ptr = *it;
		if (ptr->valid())
			++it;
		else {
			delete ptr;
			it = pc_list.erase(it);
		}
	}
}

void World::
add_char(Character *ch) {
	if (ch == nullptr)
		return;

	ch->next = char_list;
	char_list = ch;
	ch->validate();
}

void World::
remove_char(Character *ch) {
	if (ch == nullptr)
		return;

	ch->invalidate();

	if (ch == char_list)
		char_list = ch->next;
	else {
		Character *prev;

		for (prev = char_list; prev != nullptr; prev = prev->next) {
			if (prev->next == ch) {
				prev->next = ch->next;
				break;
			}
		}

		if (prev == nullptr) {
			Logging::bug("World::remove_char: char not found in char_list", 0);
			return;
		}
	}
}

void World::
add_player(Player *plr) {
	plr->validate();
	pc_list.push_front(plr); // taking ownership
}

void World::
remove_player(Player *plr) {
	plr->invalidate();
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

	if (proto == nullptr && Game::booting) {
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

	if (proto == nullptr && Game::booting) {
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

	if (proto == nullptr && Game::booting) {
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
					case Sector::forest_dense:    buf += "{HI"; break;
					case Sector::road:            buf += "{b%"; break;
					case Sector::area_entrance:   buf += "{Y#"; break;
					case Sector::city:            buf += "{gM"; break;
					default:                buf += " ";   break;
				}
			}
			else {
				if (enemy->is_npc())
					buf += "{R+";
				else
					buf += "{CP";
			}
		}

		buf += "{x";
		vec.push_back(buf);
	}
}

void World::
load_areas() {
	/* Read in all the area files */
	FILE *fpList = fopen(AREA_LIST, "r");

	if (fpList == nullptr) {
		perror(AREA_LIST);
		exit(1);
	}

	for (; ;) {
		String file_name = fread_word(fpList);

		if (file_name[0] == '$')
			break;

		if (file_name[0] == '#')
			continue;

		Area *area = new Area(*this, file_name);
		areas.push_back(area);

		area->load();
	}

	fclose(fpList);
}

// create rooms from prototypes, place into hash map for fast lookup
// this is a temporary data structure, needs to be reworked for new room IDs.
// possibly a map<vnum, vector<Room *>>, but only if looking up by vnum is
// worthwhile (possibly true because of searching by area, only have to look
// up each prototype vnum)
void World::
create_rooms() {
	for (Area *area : areas) {
		area->create_rooms();
	}
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void World::
create_exits(void)
{
	for (const Area *area : areas) {
		for (const auto& pair : area->rooms) {
			const auto& location = pair.first;
			Room* room = pair.second;

			bool found_exit = false;

			for (int door = 0; door <= 5; door++) {
				if (room->prototype.exit[door] == nullptr)
					continue;

				if (room->is_on_map() && room->prototype.exit[door]->to_vnum == 0) {
					// an auto-exit to connect up rooms on the map
					worldmap::Coordinate from = room->location.coord;
					int to_x, to_y;

					switch (door) {
						case 0: to_x = from.x;   to_y = from.y-1; break;
						case 1: to_x = from.x+1; to_y = from.y;   break;
						case 2: to_x = from.x;   to_y = from.y+1; break;
						case 3: to_x = from.x-1; to_y = from.y;   break;
						default:
							Logging::bugf("fix_exits: room at %s has bad auto-exit in direction %d",
								location.to_string(), door);
							continue;
					}

					Room *dest = area->world.maptree.get(worldmap::Coordinate(to_x, to_y));

					if (dest == nullptr) {
//							Logging::bugf("room %d, direction %d, no room", vnum, door);
						continue; // no error, just no rooms in that direction
					}
Game::booting = false;
					room->exit[door] = new Exit(*room->prototype.exit[door], dest);
Game::booting = true;
					found_exit = true;
					continue;
				}

				Room *dest = get_room(Location(Vnum(room->prototype.exit[door]->to_vnum)));
				room->exit[door] = new Exit(*room->prototype.exit[door], dest);

				if (room->exit[door]->to_room == nullptr) {
					delete room->exit[door]->to_room;
					room->exit[door] = nullptr;
					Logging::bugf("fix_exits: room at %s has unknown exit vnum %d.",
						location.to_string(), room->prototype.exit[door]->to_vnum);
				}

				if (room->exit[door])
					found_exit = true;
			}

			if (!found_exit)
				room->room_flags += ROOM_NO_MOB;
		}
	}
}
