#include "World.hpp"
#include "Area.hpp"

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
