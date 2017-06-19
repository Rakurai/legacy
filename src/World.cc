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
