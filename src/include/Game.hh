#pragma once

#include "String.hh"

class World;

class Game
{
public:
	virtual ~Game() {}
	static World& world();

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
	static bool wizlock;
	static bool newlock;

private:
	Game() {} // static class
	Game(const Game&);
	Game& operator=(const Game&);
};
