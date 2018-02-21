#include "World.hh"

#include "Area.hh"

extern std::time_t current_time;

World::
World() :
	time(current_time),
	weather(time)
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
