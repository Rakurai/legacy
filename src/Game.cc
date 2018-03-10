#include "Game.hh"
#include "skill/skill.hh"
#include "Logging.hh"
#include "Clan.hh"
#include "Auction.hh"
#include "Disabled.hh"
#include "Note.hh"
#include "MobProg.hh"
#include "music.hh"
#include "sql.hh"
#include "World.hh"
#include "affect/Affect.hh"

// static data for global access through singleton Game
int Game::port;
String Game::motd;
String Game::help_greeting;
bool Game::booting = false;
time_t Game::current_time = 0;
bool Game::log_all = false;
unsigned long Game::record_logins = 0;
int Game::record_players = 0;
int Game::record_players_since_boot = 0;
int Game::quest_double = false;
bool Game::wizlock = false;
bool Game::newlock = false;

// makes Game a singleton holder of world
World& Game::
world() {
	static World w;
	return w;
}

/* Big mama top level function */
void Game::
boot() {
	Game::booting = true;

	/* Init random number generator */
	extern void init_mm();
	init_mm();

	// ensure our tables are in order
	for (int i = (int)skill::type::first; i < (int)skill::type::size; i++) {
		const auto unknown = skill::lookup(skill::type::unknown);

		if (skill::lookup((skill::type)i).name == unknown.name) {
			Logging::bug("Game::boot: unable to find definition for skill enum %d.", i);
			exit(1);
		}
	}

	for (int i = (int)affect::type::first; i < (int)affect::type::size; i++) {
		const auto unknown = affect::lookup(affect::type::unknown);

		if (affect::lookup((affect::type)i).name == unknown.name) {
			Logging::bug("Game::boot: unable to find definition for affect enum %d.", i);
			exit(1);
		}
	}

	// go ahead and preload the world, although the first call would do it too
	if (!world().valid()) {
		exit(1);
	}

	/* Load the clan info, needs to be done before the areas due to clanrooms */
	load_clan_table();
	Format::printf("survived load_clan_table\n");

	/* initialize auction */
	auction.init();

	// load all the areas
	world().load_areas();

	world().create_rooms(); // translate room prototypes to real rooms
	Format::printf("survived create_rooms\n");

	world().create_exits(); // test all real room exits
	Format::printf("survived create_exits\n");

	/* initialize quest stuff after areas loaded, maybe areas are needed */
	world().quest.init();

	int itemsloaded = objstate_load_items();   /* load our list of items from disk, before resets! */
	Format::printf("survived objstate_load_items (%d)\n", itemsloaded);

	/* Perform various loading procedures, reset all areas once */
	load_war_table();
	Format::printf("survived load_war_table\n");
	load_war_events();
	Format::printf("survived load_war_events\n");
	load_arena_table();
	Format::printf("survived load_arena_table\n");

	booting = false;

	world().update();
	Format::printf("survived area_update\n");
	load_notes();
	Format::printf("survived load_notes\n");
	load_disabled();
	Format::printf("survived load_disabled\n");
	MOBtrigger = true;
	load_songs();
	Format::printf("survived load_songs\n");
	load_social_table();
	Format::printf("survived load_social_table\n");
	load_storage_list();
	Format::printf("survived load_storage_list\n");
	load_departed_list();
	Format::printf("survived load_departed_list\n");

	/* read in our record players and record logins */
	if (db_query("Game::boot", "SELECT logins, players FROM records") == SQL_OK) {
		if (db_next_row() == SQL_OK) {
			Game::record_logins = db_get_column_int(0);
			Game::record_players = db_get_column_int(1);
		}
		else
			Logging::bug("Game::boot: failed to fetch record logins and players", 0);
	}

	/* load our greeting */
	if (db_query("Game::boot", "SELECT text FROM helps WHERE keywords='GREETING'") == SQL_OK) {
		if (db_next_row() == SQL_OK)
			Game::help_greeting = db_get_column_str(0);
		else {
			Logging::bug("Game::boot: failed to fetch greeting", 0);
			exit(1);
		}
	}

	if (Game::help_greeting.empty())
		Game::help_greeting = "need a greeting! enter your name: ";
} /* end Game::boot() */

