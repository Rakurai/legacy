#pragma once

#include "declare.hh"
#include "String.hh"
#include "World.hh"

class Game
{
public:
	virtual ~Game() {}
	static World& world() {
		static World w;
		return w;
	}

	static void boot();

	static int port;
    static String motd;
	static String help_greeting;
    static bool booting;
    static time_t current_time;
    static bool log_all;
	static unsigned long record_logins;
	static int record_players;
	static int record_players_since_boot;
	static int quest_double;
private:
	Game() {} // static class
	Game(const Game&);
	Game& operator=(const Game&);
};
