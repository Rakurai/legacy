#pragma once

#include "declare.hh"
#include "GameTime.hh"
#include "Weather.hh"
#include "QuestArea.hh"
#include <vector>

class World
{
public:
	virtual ~World();

	void update();

	GameTime time;
	Weather weather;
	QuestArea quest;

	std::vector<Area *> areas;

private:
	World();
	World(const World&);
	World& operator=(const World&);

	friend class Game;
};
