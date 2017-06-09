#pragma once

#include "declare.h"
#include <vector>

class World
{
public:
	virtual ~World();

	void update();

	std::vector<Area *> areas;
	Area *quest_area;

private:
	World() {}
	World(const World&);
	World& operator=(const World&);

	friend class Game;
};
