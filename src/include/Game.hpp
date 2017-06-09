#pragma once

#include "declare.h"
#include "World.hpp"

class Game
{
public:
	virtual ~Game() {}
	static World& world() {
		static World w;
		return w;
	}

private:
	Game() {} // static class
	Game(const Game&);
	Game& operator=(const Game&);
};
