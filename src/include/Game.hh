#pragma once

#include "declare.hh"
#include "String.hh"
#include "World.hh"
#include "control/PlayerController.hh"

class Game
{
public:
	virtual ~Game() {}
	static World& world() {
		static World w;
		return w;
	}

	static std::vector<control::PlayerController *> players;

    static String motd;

private:
	Game() {} // static class
	Game(const Game&);
	Game& operator=(const Game&);
};
