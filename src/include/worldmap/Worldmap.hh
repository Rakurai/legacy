#pragma once

#include "Quadtree.hh"
#include "Room.hh"

namespace worldmap {

class Worldmap {
public:
	Worldmap();
	virtual ~Worldmap();

private:
	Worldmap(const Worldmap&);
	Worldmap& operator=(const Worldmap&);

	Quadtree<Room> qt;
};

} // namespace worldmap
