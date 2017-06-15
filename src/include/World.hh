#pragma once

#include "declare.hh"
#include "GameTime.hh"
#include "Weather.hh"
#include <vector>

class World
{
public:
	virtual ~World();

	void update();

	GameTime time;
	Weather weather;

	std::vector<Area *> areas;
	Area *quest_area;

private:
	World();
	World(const World&);
	World& operator=(const World&);

	friend class Game;
};
