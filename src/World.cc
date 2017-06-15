#include "World.hh"
#include "Area.hh"

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
