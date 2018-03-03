/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

#include <vector>
#include <map>

#include "affect/affect_list.hh"
#include "Area.hh"
#include "Auction.hh"
#include "Clan.hh"
#include "declare.hh"
#include "db.hh"
#include "Disabled.hh"
#include "Exit.hh"
#include "ExtraDescr.hh"
#include "file.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "lookup.hh"
#include "Logging.hh"
#include "macros.hh"
#include "memory.hh"
#include "merc.hh"
#include "music.hh"
#include "MobilePrototype.hh"
#include "MobProg.hh"
#include "ObjectPrototype.hh"
#include "ObjectValue.hh"
#include "QuestArea.hh"
#include "Reset.hh"
#include "Room.hh"
#include "sql.hh"
#include "Shop.hh"
#include "skill/skill.hh"
#include "String.hh"
#include "World.hh"
#include "worldmap/Region.hh"
#include "worldmap/MapColor.hh"

extern  int     _filbuf         args((FILE *));
extern void          affect::copy_to_list         args(( Affect **list_head, const affect::Affect *paf ));

/*
 * Globals.
 */
Shop              *shop_first;
Shop              *shop_last;


char                    bug_buf         [2 * MAX_INPUT_LENGTH];
Character              *char_list;
Player                *pc_list;        /* should probably go somewhere else *shrug* -- Montrey */
String                  help_greeting;
char                    log_buf         [2 * MAX_INPUT_LENGTH];
Object               *object_list;
String                  default_prompt = "%CW<%CC%h%CThp %CG%m%CHma %CB%v%CNst%CW> ";

Object               *donation_pit;

/* records */
unsigned long   record_logins = 0;
int             record_players = 0;
int             record_players_since_boot = 0;


/*
 * Locals.
 */
long                    quest_double = 0;

/*
 * Credits defines stuff
 */

#define RANGE_OK   0
#define RANGE_ALL  1
#define RANGE_CLAN 2
#define RANGE_IMM  3

/*
 * Semi-locals.
 */
bool                    fBootDb;
FILE                   *fpArea;
char                    strArea[MAX_INPUT_LENGTH];
Area *area_last; // currently loading area

/*
 * Local booting procedures.
*/
void    init_mm         args((void));
void    load_area       args((FILE *fp));
void    load_region     args((FILE *fp));
void    load_mobiles    args((FILE *fp));
void    load_objects    args((FILE *fp));
void    load_resets     args((FILE *fp));
void    load_rooms      args((FILE *fp));
void    load_shops      args((FILE *fp));
void    load_specials   args((FILE *fp));
void    load_notes      args((void));
void    create_rooms    args((void));
void    fix_exits       args((void));

/* Big mama top level function */
void boot_db()
{
	fBootDb = TRUE;

	/* Init random number generator */
	{
		init_mm();
	}

	// ensure our tables are in order
	{
		for (int i = (int)skill::type::first; i < (int)skill::type::size; i++) {
			const auto unknown = skill::lookup(skill::type::unknown);

			if (skill::lookup((skill::type)i).name == unknown.name) {
				boot_bug("boot_db: unable to find definition for skill enum %d.", i);
				exit(1);
			}
		}

		for (int i = (int)affect::type::first; i < (int)affect::type::size; i++) {
			const auto unknown = affect::lookup(affect::type::unknown);

			if (affect::lookup((affect::type)i).name == unknown.name) {
				boot_bug("boot_db: unable to find definition for affect enum %d.", i);
				exit(1);
			}
		}
	}

	// go ahead and preload the world, although the first call would do it too
	if (!Game::world().valid()) {
		exit(1);
	}

	/* Load the clan info, needs to be done before the areas due to clanrooms */
	{
		load_clan_table();
		Format::printf("survived load_clan_table\n");
	}
	/* initialize auction */
	{
		auction.init();
	}

	/* Read in all the area files */
	{
		FILE *fpList;

		if ((fpList = fopen(AREA_LIST, "r")) == nullptr) {
			perror(AREA_LIST);
			exit(1);
		}

		for (; ;) {
			strcpy(strArea, fread_word(fpList));

			if (strArea[0] == '$')
				break;

			if (strArea[0] == '#')
				continue;

			if (strArea[0] == '-')
				fpArea = stdin;
			else {
				char abuf[MSL];
				Format::sprintf(abuf, "%s%s", AREA_DIR, strArea);

				if ((fpArea = fopen(abuf, "r")) == nullptr) {
					perror(strArea);
					exit(1);
				}
			}

			Format::printf("Now loading area: %s\n", strArea);

			for (; ;) {
				if (fread_letter(fpArea) != '#') {
					boot_bug("Boot_db: # not found.", 0);
					exit(1);
				}

				String word = fread_word(fpArea);

				if (word[0] == '$')  break;
				else if (word == "AREA")  load_area(fpArea);
				else if (word == "REGION")   load_region(fpArea);
				else if (word == "MOBILES")  load_mobiles(fpArea);
				else if (word == "OBJECTS")  load_objects(fpArea);
				else if (word == "RESETS")  load_resets(fpArea);
				else if (word == "ROOMS")  load_rooms(fpArea);
				else if (word == "SHOPS")  load_shops(fpArea);
				else if (word == "SPECIALS")  load_specials(fpArea);
//				else if (word == "TOURSTARTS")  load_tourstarts(fpArea);
//				else if (word == "TOURROUTES")  load_tourroutes(fpArea);
				else {
					boot_bug("Boot_db: bad section name.", 0);
					exit(1);
				}
			}

			if (fpArea != stdin)
				fclose(fpArea);

			fpArea = nullptr;
		}

		fclose(fpList);
	}

	create_rooms(); // translate room prototypes to real rooms
	Format::printf("survived create_rooms\n");
	fix_exits(); // test all real room exits
	Format::printf("survived fix_exits\n");

	/* initialize quest stuff after areas loaded, maybe areas are needed */
	Game::world().quest.init();

	int itemsloaded = objstate_load_items();   /* load our list of items from disk, before resets! */
	Format::printf("survived objstate_load_items (%d)\n", itemsloaded);

	/* Perform various loading procedures, reset all areas once */
	load_war_table();
	Format::printf("survived load_war_table\n");
	load_war_events();
	Format::printf("survived load_war_events\n");
	load_arena_table();
	Format::printf("survived load_arena_table\n");
	fBootDb = FALSE;
//	area_update();
	Game::world().update();
	Format::printf("survived area_update\n");
	load_notes();
	Format::printf("survived load_notes\n");
	load_disabled();
	Format::printf("survived load_disabled\n");
	MOBtrigger = TRUE;
	load_songs();
	Format::printf("survived load_songs\n");
	load_social_table();
	Format::printf("survived load_social_table\n");
	load_storage_list();
	Format::printf("survived load_storage_list\n");
	load_departed_list();
	Format::printf("survived load_departed_list\n");

	/* read in our record players and record logins */
	if (db_query("boot_db", "SELECT logins, players FROM records") == SQL_OK) {
		if (db_next_row() == SQL_OK) {
			record_logins = db_get_column_int(0);
			record_players = db_get_column_int(1);
		}
		else
			Logging::bug("boot_db: failed to fetch record logins and players", 0);
	}

	/* load our greeting */
	if (db_query("boot_db", "SELECT text FROM helps WHERE keywords='GREETING'") == SQL_OK) {
		if (db_next_row() == SQL_OK)
			help_greeting = db_get_column_str(0);
		else {
			Logging::bug("boot_db: failed to fetch greeting", 0);
			exit(1);
		}
	}

	if (help_greeting.empty())
		help_greeting = "need a greeting! enter your name: ";
} /* end boot_db() */

/*
 * Load an AREA section (only a header, really)
 */
void load_area(FILE *fp)
{
	area_last = new Area(Game::world(), fp);
	Game::world().areas.push_back(area_last);
}

void load_region(FILE *fp) {
	if (area_last == nullptr) {
		boot_bug("Load_region: no #AREA seen yet.", 0);
		exit(1);
	}

	area_last->load_region(fp);
}

/*
 * Snarf a reset section.
 */
void load_resets(FILE *fp)
{
	char letter;

	if (area_last == nullptr) {
		boot_bug("Load_resets: no #AREA seen yet.", 0);
		exit(1);
	}

	for (; ;) {
		if ((letter = fread_letter(fp)) == 'S')
			break;

		if (letter == '*') {
			fread_to_eol(fp);
			continue;
		}

		ungetc(letter, fp);
		Reset *pReset = new Reset(fp);
		area_last->resets.push_back(pReset);
	}
}

void load_mobiles(FILE *fp)
{
	if (area_last == nullptr) {
		boot_bug("Load_mobiles: no #AREA seen yet.", 0);
		exit(1);
	}

	area_last->load_mobiles(fp);
}

void load_objects(FILE *fp)
{
	if (area_last == nullptr) {
		boot_bug("Load_objects: no #AREA seen yet.", 0);
		exit(1);
	}

	area_last->load_objects(fp);
}

void load_rooms(FILE *fp)
{
	if (area_last == nullptr) {
		boot_bug("Load_rooms: no #AREA seen yet.", 0);
		exit(1);
	}

	area_last->load_rooms(fp);
}

/*
 * Snarf a shop section.
 */
void load_shops(FILE *fp)
{
	Shop *pShop;
	MobilePrototype *pMobIndex;
	int shopkeeper;
	int iTrade;

	for (; ;) {
		if ((shopkeeper = fread_number(fp)) == 0)
			break;

		pShop = new Shop;
		pShop->next             = nullptr;
		pShop->keeper           = shopkeeper;

		for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
			pShop->buy_type[iTrade]     = fread_number(fp);

		pShop->profit_buy       = fread_number(fp);
		pShop->profit_sell      = fread_number(fp);
		pShop->open_hour        = fread_number(fp);
		pShop->close_hour       = fread_number(fp);
		fread_to_eol(fp);
		pMobIndex               = Game::world().get_mob_prototype(pShop->keeper);

		if (pMobIndex == nullptr) {
			boot_bug("Load_shops: NULL mob index %d", pShop->keeper);
			exit(1);
		}

		pMobIndex->pShop        = pShop;

		if (shop_first == nullptr)
			shop_first = pShop;

		if (shop_last  != nullptr)
			shop_last->next = pShop;

		shop_last       = pShop;
		pShop->next     = nullptr;
		top_shop++;
	}

	return;
}

/*
 * Snarf spec proc declarations.
 */
void load_specials(FILE *fp)
{
	MobilePrototype *pMobIndex;
	char letter;

	for (; ;) {
		switch (letter = fread_letter(fp)) {
		default:
			boot_bug("Load_specials: letter '%c' not *MS.", letter);
			exit(1);

		case 'S':
			return;

		case '*':
			break;

		case 'M':
			pMobIndex           = Game::world().get_mob_prototype(fread_number(fp));

			if (!pMobIndex) {
				boot_bug("Load_specials: 'M': vnum %d.", pMobIndex->vnum);
				exit(1);
			}

			pMobIndex->spec_fun = spec_lookup(fread_word(fp));

			if (pMobIndex->spec_fun == 0) {
				boot_bug("Load_specials: 'M': vnum %d.", pMobIndex->vnum);
				exit(1);
			}

			break;
		}

		fread_to_eol(fp);
	}
}

// create rooms from prototypes, place into hash map for fast lookup
// this is a temporary data structure, needs to be reworked for new room IDs.
// possibly a map<vnum, vector<Room *>>, but only if looking up by vnum is
// worthwhile (possibly true because of searching by area, only have to look
// up each prototype vnum)
void create_rooms() {
	for (Area *area : Game::world().areas) {
		area->create_rooms();
	}
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits(void)
{
	for (const Area *area : Game::world().areas) {
		for (const auto& pair : area->rooms) {
			const auto& location = pair.first;
			Room* room = pair.second;

			bool found_exit = FALSE;

			for (int door = 0; door <= 5; door++) {
				if (room->prototype.exit[door] == nullptr)
					continue;

				if (room->is_on_map() && room->prototype.exit[door]->to_vnum == 0) {
					// an auto-exit to connect up rooms on the map
					worldmap::Coordinate from = room->location.coord;
					int to_x, to_y;

					switch (door) {
						case 0: to_x = from.x;   to_y = from.y-1; break;
						case 1: to_x = from.x+1; to_y = from.y;   break;
						case 2: to_x = from.x;   to_y = from.y+1; break;
						case 3: to_x = from.x-1; to_y = from.y;   break;
						default:
							Logging::bugf("fix_exits: room at %s has bad auto-exit in direction %d",
								location.to_string(), door);
							continue;
					}

					Room *dest = area->world.maptree.get(worldmap::Coordinate(to_x, to_y));

					if (dest == nullptr) {
//							Logging::bugf("room %d, direction %d, no room", vnum, door);
						continue; // no error, just no rooms in that direction
					}
fBootDb = FALSE;
					room->exit[door] = new Exit(*room->prototype.exit[door], dest);
fBootDb = TRUE;
					found_exit = TRUE;
					continue;
				}

				Room *dest = Game::world().get_room(Location(Vnum(room->prototype.exit[door]->to_vnum)));
				room->exit[door] = new Exit(*room->prototype.exit[door], dest);

				if (room->exit[door]->to_room == nullptr) {
					delete room->exit[door]->to_room;
					room->exit[door] = nullptr;
					Logging::bugf("fix_exits: room at %s has unknown exit vnum %d.",
						location.to_string(), room->prototype.exit[door]->to_vnum);
				}

				if (room->exit[door])
					found_exit = TRUE;
			}

			if (!found_exit)
				room->room_flags += ROOM_NO_MOB;
		}
	}
}

/*
 * Reports a bug.
 */
void boot_bug(const String& str, int param)
{
	if (fpArea != nullptr) {
		int iLine = 0;
		int iChar = 0;

		if (fpArea != stdin) {
			iChar = ftell(fpArea);
			fseek(fpArea, 0, 0);

			for (iLine = 0; ftell(fpArea) < iChar; iLine++) {
				while (getc(fpArea) != '\n')
					;
			}

			fseek(fpArea, iChar, 0);
		}

		Logging::bugf("[*****] FILE: %s LINE: %d", strArea, iLine);
	}

	Logging::bugf(str, param);
}

void boot_bug(const String& str, const Vnum& vnum) {
	boot_bug(str, vnum.value());
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain(void)
{
	return;
}

