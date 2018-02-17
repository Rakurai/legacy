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
//#include "affect/Affect.hh"
//#include "AffectKey.hh"
#include "Area.hh"
#include "Auction.hh"
#include "Clan.hh"
#include "declare.hh"
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
#include "RoomPrototype.hh"
#include "sql.hh"
#include "Shop.hh"
#include "skill/skill.hh"
#include "String.hh"
#include "World.hh"

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
MobilePrototype         *mob_index_hash          [MAX_KEY_HASH];
ObjectPrototype         *obj_index_hash          [MAX_KEY_HASH];
std::map<int, RoomPrototype *> room_index_map;

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
sh_int                  aVersion = 1;
Area *area_last; // currently loading area

/*
 * MOBprogram locals
*/

void boot_bug(const String& str, int param);

//int             mprog_name_to_type      args((const String& name));
MobProg     *mprog_file_read         args((const String& f, MobProg *mprg,
                MobilePrototype *pMobIndex));
void            mprog_read_programs     args((FILE *fp,
                MobilePrototype *pMobIndex));

/*
 * Local booting procedures.
*/
void    init_mm         args((void));
void    load_area       args((FILE *fp));
void    load_mobiles    args((FILE *fp));
void    load_objects    args((FILE *fp));
void    load_resets     args((FILE *fp));
void    load_rooms      args((FILE *fp));
void    load_shops      args((FILE *fp));
void    load_specials   args((FILE *fp));
void    load_notes      args((void));
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
		for (int i = skill::first; i < skill::size; i++) {
			const auto unknown = skill::lookup(skill::unknown);

			if (skill::lookup((skill::Type)i).name == unknown.name) {
				boot_bug("boot_db: unable to find definition for skill enum %d.", 0);
				exit(1);
			}
		}

		for (int i = affect::first; i < affect::size; i++) {
			auto unknown = affect::lookup(affect::unknown);

			if (affect::lookup((affect::Type)i).name == unknown.name) {
				boot_bug("boot_db: unable to find definition for affect enum %d.", 0);
				exit(1);
			}
		}
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
	/* initialize quest stuff after areas loaded, maybe areas are needed */
	Game::world().quest.init();

	int itemsloaded = objstate_load_items();   /* load our list of items from disk, before resets! */
	Format::printf("survived objstate_load_items (%d)\n", itemsloaded);

	/* Perform various loading procedures, reset all areas once, fix up exits */
	fix_exits();
	Format::printf("survived fix_exits\n");
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

/*
 * Snarf a mob section.  new style
 */
void load_mobiles(FILE *fp)
{
	MobilePrototype *pMobIndex;
	sh_int vnum;
	char letter;
	int iHash;

	for (; ;) {
		letter                          = fread_letter(fp);

		if (letter != '#') {
			boot_bug("Load_mobiles: # not found.", 0);
			exit(1);
		}

		vnum                            = fread_number(fp);

		if (vnum == 0)
			break;

		if (vnum < area_last->min_vnum ||
		    vnum > area_last->max_vnum)
			boot_bug("mobile vnum %d out of range.", vnum);

		fBootDb = FALSE;

		if (get_mob_index(vnum) != nullptr) {
			boot_bug("Load_mobiles: vnum %d duplicated.", vnum);
			exit(1);
		}

		fBootDb = TRUE;
		pMobIndex = new MobilePrototype(fp, vnum);

		iHash                   = vnum % MAX_KEY_HASH;
		pMobIndex->next         = mob_index_hash[iHash];
		mob_index_hash[iHash]   = pMobIndex;
		top_mob_index++;
	}

	return;
}

/*
 * Snarf an obj section. new style
 */
void load_objects(FILE *fp)
{
	ObjectPrototype *pObjIndex;
	sh_int vnum;
	char letter;
	int iHash;

	for (; ;) {
		letter                          = fread_letter(fp);

		if (letter != '#') {
			boot_bug("Load_objects: # not found.", 0);
			exit(1);
		}

		vnum                            = fread_number(fp);

		if (vnum == 0)
			break;

		if (vnum < area_last->min_vnum ||
		    vnum > area_last->max_vnum)
			boot_bug("object vnum %d out of range.", vnum);

		fBootDb = FALSE;

		if (get_obj_index(vnum) != nullptr) {
			boot_bug("Load_objects: vnum %d duplicated.", vnum);
			exit(1);
		}

		fBootDb = TRUE;
		pObjIndex = new ObjectPrototype;
		pObjIndex->vnum                 = vnum;
		pObjIndex->reset_num            = 0;
		pObjIndex->version              = aVersion;
		pObjIndex->name                 = fread_string(fp);
		pObjIndex->short_descr          = fread_string(fp);
		pObjIndex->description          = fread_string(fp);
		pObjIndex->material             = fread_string(fp);
		pObjIndex->item_type            = item_lookup(fread_word(fp));
		pObjIndex->extra_flags          = fread_flag(fp);
		pObjIndex->wear_flags           = fread_flag(fp);
		pObjIndex->num_settings			= 0;


		int val = 0; // prevent accidents in altering below switches
		switch (pObjIndex->item_type) {
		case ITEM_WEAPON:
			pObjIndex->value[val]         = ObjectValue(get_weapon_type(fread_word(fp)));
			break;

		case ITEM_KEY:
			pObjIndex->value[val]         = ObjectValue(fread_flag(fp));
			break;

		default:
			pObjIndex->value[val]         = ObjectValue(fread_number(fp));
			break;
		}


		val = 1;
		switch (pObjIndex->item_type) {
		case ITEM_CONTAINER:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
		case ITEM_PORTAL:
			pObjIndex->value[val]         = ObjectValue(fread_flag(fp));
			break;

		case ITEM_POTION:
		case ITEM_PILL:
		case ITEM_SCROLL:
			pObjIndex->value[val]         = ObjectValue(skill::lookup(fread_word(fp)));
			break;

		default:
			pObjIndex->value[val]         = ObjectValue(fread_number(fp));
			break;
		}


		val = 2;
		switch (pObjIndex->item_type) {
		case ITEM_DRINK_CON:
		case ITEM_FOUNTAIN:
			pObjIndex->value[val]         = ObjectValue(liq_lookup(fread_word(fp)));

			if (pObjIndex->value[val] == -1) {
				pObjIndex->value[val] = 0;
				boot_bug("Unknown liquid type", 0);
			}

			break;

		case ITEM_POTION:
		case ITEM_PILL:
		case ITEM_SCROLL:
			pObjIndex->value[val]         = ObjectValue(skill::lookup(fread_word(fp)));
			break;

		case ITEM_FURNITURE:
		case ITEM_PORTAL:
		case ITEM_ANVIL:
			pObjIndex->value[val]         = ObjectValue(fread_flag(fp));
			break;

		default:
			pObjIndex->value[val]         = ObjectValue(fread_number(fp));
			break;
		}


		val = 3;
		switch (pObjIndex->item_type) {
		case ITEM_WEAPON:
			pObjIndex->value[val]         = ObjectValue(attack_lookup(fread_word(fp)));
			break;

		case ITEM_WAND:
		case ITEM_STAFF:
		case ITEM_POTION:
		case ITEM_PILL:
		case ITEM_SCROLL:
			pObjIndex->value[val]         = ObjectValue(skill::lookup(fread_word(fp)));
			break;

		case ITEM_DRINK_CON:
		case ITEM_FOUNTAIN:
		case ITEM_FOOD:
			pObjIndex->value[val]         = ObjectValue(fread_flag(fp));
			break;

		default:
			pObjIndex->value[val]         = ObjectValue(fread_number(fp));
			break;
		}


		val = 4;
		switch (pObjIndex->item_type) {
		case ITEM_WEAPON: {
			Flags bitvector               = fread_flag(fp);

			// preserve the bits, we use them for loading old versions of players
			pObjIndex->value[val] = ObjectValue(bitvector);

			affect::Affect af;
			af.level              = pObjIndex->level;
			af.duration           = -1;
			af.evolution          = 1;
			af.permanent          = TRUE;
			af.location           = 0;
			af.modifier           = 0;

			while (!bitvector.empty()) {
				af.type = affect::none; // reset every time

				if (affect::parse_flags('W', &af, bitvector))
					affect::copy_to_list(&pObjIndex->affected, &af); 
			}

			break;
		}

		case ITEM_POTION:
		case ITEM_PILL:
		case ITEM_SCROLL:
			pObjIndex->value[val]         = ObjectValue(skill::lookup(fread_word(fp)));
			break;

		default:
			pObjIndex->value[val]         = ObjectValue(fread_number(fp));
			break;
		}


		pObjIndex->level                = fread_number(fp);

		// this is annoying.  since v0-v4 come before level in an object section,
		// we had to create any affects (like weapon flags) at level 0.
		// fix them up now.
		affect::fn_params params;
		params.owner = nullptr;
		params.data = &pObjIndex->level;

		affect::iterate_over_list(
			&pObjIndex->affected,
			affect::fn_set_level,
			&params
		);

		pObjIndex->weight               = fread_number(fp);
		pObjIndex->cost                 = fread_number(fp);
		/* condition */
		letter                          = fread_letter(fp);

		switch (letter) {
		case ('P') :                pObjIndex->condition = 100; break;

		case ('G') :                pObjIndex->condition =  90; break;

		case ('A') :                pObjIndex->condition =  75; break;

		case ('W') :                pObjIndex->condition =  50; break;

		case ('D') :                pObjIndex->condition =  25; break;

		case ('B') :                pObjIndex->condition =  10; break;

		case ('R') :                pObjIndex->condition =   5; break;

		case ('I') :                pObjIndex->condition =  -1; break;

		default:                    pObjIndex->condition = 100; break;
		}

		for (; ;) {
			char letter;
			letter = fread_letter(fp);

			if (letter == 'A') { // apply
				affect::Affect af;
				af.type               = affect::none;
				af.level              = pObjIndex->level;
				af.duration           = -1;
				af.location           = fread_number(fp);
				af.modifier           = fread_number(fp);
				af.evolution          = 1;
				af.bitvector(0);
				af.permanent          = TRUE;

				Flags bitvector = 0;
				if (affect::parse_flags('O', &af, bitvector)) {
					affect::copy_to_list(&pObjIndex->affected, &af);
				}
			}
			else if (letter == 'F') { // flag, can add bits or do other ->where types
				affect::Affect af;
				af.type               = affect::none;
				af.level              = pObjIndex->level;
				af.duration           = -1;
				af.evolution          = 1;
				af.permanent          = TRUE;

				letter          = fread_letter(fp);
				af.location     = fread_number(fp);
				af.modifier     = fread_number(fp);

				Flags bitvector    = fread_flag(fp);

				// do at least once even if no bitvector
				do {
					if (affect::parse_flags(letter, &af, bitvector)) {
						affect::copy_to_list(&pObjIndex->affected, &af); 

						// don't multiply the modifier, just apply to the first bit
						af.location = 0;
						af.modifier = 0;
					}

					af.type = affect::none; // reset every time
				} while (!bitvector.empty());
			}
			else if (letter == 'E') {
				ExtraDescr *ed = new ExtraDescr(fread_string(fp), fread_string(fp));
				ed->next                = pObjIndex->extra_descr;
				pObjIndex->extra_descr  = ed;
			}
			else if (letter == 'S') {
				pObjIndex->num_settings = fread_number(fp);
			}
			else {
				ungetc(letter, fp);
				break;
			}
		}

		// affects are immutable, compute the checksum now
		pObjIndex->affect_checksum = affect::checksum_list(&pObjIndex->affected);

		iHash                   = vnum % MAX_KEY_HASH;
		pObjIndex->next         = obj_index_hash[iHash];
		obj_index_hash[iHash]   = pObjIndex;
		top_obj_index++;
	}

	return;
}

/*
 * Snarf a room section.
 */
void load_rooms(FILE *fp)
{
	RoomPrototype *pRoomIndex;
	ExtraDescr *ed;
	Exit *pexit;
	int locks;
	char log_buf[MAX_STRING_LENGTH];
	sh_int vnum;
	char letter;
	int door;

	if (area_last == nullptr) {
		boot_bug("Load_resets: no #AREA seen yet.", 0);
		exit(1);
	}

	for (; ;) {
		letter                          = fread_letter(fp);

		if (letter != '#') {
			boot_bug("Load_rooms: # not found.", 0);
			exit(1);
		}

		vnum                            = fread_number(fp);

		if (vnum == 0)
			break;

		if (vnum < area_last->min_vnum ||
		    vnum > area_last->max_vnum)
			boot_bug("room   vnum %d out of range.", vnum);

		fBootDb = FALSE;

		if (get_room_index(vnum) != nullptr) {
			boot_bug("Load_rooms: vnum %d duplicated.", vnum);
			exit(1);
		}

		fBootDb = TRUE;
		pRoomIndex = new RoomPrototype;
		pRoomIndex->version             = aVersion;
		pRoomIndex->people              = nullptr;
		pRoomIndex->contents            = nullptr;
		pRoomIndex->extra_descr         = nullptr;
		pRoomIndex->area                = area_last;
		pRoomIndex->vnum                = vnum;
		pRoomIndex->name                = fread_string(fp);
		pRoomIndex->description         = fread_string(fp);
		pRoomIndex->tele_dest           = fread_number(fp);
		pRoomIndex->room_flags          = fread_flag(fp);

		/* horrible hack */
		if (3000 <= vnum && vnum < 3400)
			pRoomIndex->room_flags += ROOM_LAW;

		pRoomIndex->sector_type         = fread_number(fp);
		pRoomIndex->light               = 0;

		for (door = 0; door <= 5; door++)
			pRoomIndex->exit[door] = nullptr;

		if (GET_ROOM_FLAGS(pRoomIndex).has(ROOM_FEMALE_ONLY)) {
			Format::sprintf(log_buf, "Room %d is FEMALE_ONLY", pRoomIndex->vnum);
			Logging::log(log_buf);
		}

		if (GET_ROOM_FLAGS(pRoomIndex).has(ROOM_MALE_ONLY)) {
			Format::sprintf(log_buf, "Room %d is MALE_ONLY", pRoomIndex->vnum);
			Logging::log(log_buf);
		}

		if (GET_ROOM_FLAGS(pRoomIndex).has(ROOM_LOCKER)) {
			Format::sprintf(log_buf, "Room %d is LOCKER", pRoomIndex->vnum);
			Logging::log(log_buf);
		}

		/* defaults */
		pRoomIndex->heal_rate = 100;
		pRoomIndex->mana_rate = 100;
		pRoomIndex->guild = 0;

		for (; ;) {
			letter = fread_letter(fp);

			if (letter == 'S')
				break;

			switch (letter) {
			case 'H':       /* healing room */
				pRoomIndex->heal_rate = fread_number(fp);
				break;

			case 'M':       /* mana room */
				pRoomIndex->mana_rate = fread_number(fp);
				break;

			case 'C':       /* clan */
				pRoomIndex->clan = clan_lookup(fread_string(fp));
				break;

			case 'G':       /* guild */
				if (!(pRoomIndex->guild = class_lookup(fread_string(fp)) + 1)) {
					boot_bug("Load_rooms: invalid class in guild", 0);
					exit(1);
				}

				break;

			case 'D':       /* door */
				door = fread_number(fp);

				if (door < 0 || door > 5) {
					boot_bug("Fread_rooms: vnum %d has bad door number.", vnum);
					exit(1);
				}

				pexit = new Exit;
				pexit->description      = fread_string(fp);
				pexit->keyword          = fread_string(fp);
				pexit->exit_flags        = Flags::none;
				locks                   = fread_number(fp);
				pexit->key              = fread_number(fp);
				pexit->u1.vnum          = fread_number(fp);

				switch (locks) {
				case 1: pexit->exit_flags = EX_ISDOOR;                   break;

				case 2: pexit->exit_flags = EX_ISDOOR | EX_PICKPROOF;      break;

				case 3: pexit->exit_flags = EX_ISDOOR | EX_NOPASS;         break;

				case 4: pexit->exit_flags = EX_ISDOOR | EX_NOPASS | EX_PICKPROOF; break;
				}

				if (pexit->u1.vnum <= 0 || pexit->u1.vnum >= 32700) {
					boot_bug("load_rooms: vnum %d has invalid exit", vnum);
					exit(1);
				}

				pRoomIndex->exit[door] = pexit;
				pRoomIndex->old_exit[door] = pexit;
				top_exit++;
				break;

			case 'E':       /* extended desc */
				ed = new ExtraDescr(fread_string(fp), fread_string(fp));
				ed->next                = pRoomIndex->extra_descr;
				pRoomIndex->extra_descr = ed;
				break;

			case 'O':
				pRoomIndex->owner = fread_string(fp);
				break;

			default:
				boot_bug("Load_rooms: vnum %d has flag not 'CDEHMOS'.", vnum);
				exit(1);
			}
		}

		room_index_map[vnum] = pRoomIndex;
		top_room++;
	}

	return;
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
		pShop->version          = aVersion;
		pShop->keeper           = shopkeeper;

		for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
			pShop->buy_type[iTrade]     = fread_number(fp);

		pShop->profit_buy       = fread_number(fp);
		pShop->profit_sell      = fread_number(fp);
		pShop->open_hour        = fread_number(fp);
		pShop->close_hour       = fread_number(fp);
		fread_to_eol(fp);
		pMobIndex               = get_mob_index(pShop->keeper);

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
			pMobIndex           = get_mob_index(fread_number(fp));

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

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits(void)
{
//    char buf[MAX_STRING_LENGTH];
//    RoomPrototype *to_room;
	Exit *pexit;
//    Exit *pexit_rev;
	int door;

	for (auto it = room_index_map.begin(); it != room_index_map.end(); ++it) {
		RoomPrototype *pRoomIndex = it->second;

		bool fexit = FALSE;

		for (door = 0; door <= 5; door++) {
			if ((pexit = pRoomIndex->exit[door]) != nullptr) {
				pexit->u1.to_room = get_room_index(pexit->u1.vnum);

				if (pexit->u1.to_room == nullptr) {
					delete pexit;
					pRoomIndex->exit[door] = nullptr;
				}
				else
					fexit = TRUE;
			}
		}

		if (!fexit)
			pRoomIndex->room_flags += ROOM_NO_MOB;
	}

	/* nobody cares about the Fix_exits() messages -- Elrac
	  for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	  {
	      for ( pRoomIndex  = room_index_hash[iHash];
	            pRoomIndex != nullptr;
	            pRoomIndex  = pRoomIndex->next )
	      {
	          for ( door = 0; door <= 5; door++ )
	          {
	              if ( ( pexit     = pRoomIndex->exit[door]       ) != nullptr
	              &&   ( to_room   = pexit->u1.to_room            ) != nullptr
	              &&   ( pexit_rev = to_room->exit[Exit::rev_dir(door)] ) != nullptr
	              &&   pexit_rev->u1.to_room != pRoomIndex
	              &&   (pRoomIndex->vnum < 1200 || pRoomIndex->vnum > 1299))
	              {
	                  Format::sprintf( buf, "Fix_exits: %d:%d -> %d:%d -> %d.",
	                      pRoomIndex->vnum, door,
	                      to_room->vnum,    Exit::rev_dir(door),
	                      (pexit_rev->u1.to_room == nullptr)
	                          ? 0 : pexit_rev->u1.to_room->vnum );
	                  boot_bug( buf, 0 );
	              }
	          }
	      }
	  }
	*/
	return;
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

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

Flags::Bit mprog_name_to_type(const String& name)
{
	if (name == "in_file_prog")    return IN_FILE_PROG;

	if (name == "act_prog")    return ACT_PROG;

	if (name == "speech_prog")    return SPEECH_PROG;

	if (name == "rand_prog")    return RAND_PROG;
	if (name == "rand_area_prog") return RAND_AREA_PROG;

	if (name == "boot_prog")    return BOOT_PROG;

	if (name == "fight_prog")    return FIGHT_PROG;

	if (name == "buy_prog")    return BUY_PROG;

	if (name == "hitprcnt_prog")    return HITPRCNT_PROG;

	if (name == "death_prog")    return DEATH_PROG;

	if (name == "entry_prog")    return ENTRY_PROG;

	if (name == "greet_prog")    return GREET_PROG;

	if (name == "all_greet_prog")    return ALL_GREET_PROG;

	if (name == "give_prog")    return GIVE_PROG;

	if (name == "bribe_prog")    return BRIBE_PROG;

	if (name == "tick_prog")    return TICK_PROG;

	return (ERROR_PROG);
}
/* This routine reads in scripts of MOBprograms from a file */

MobProg *mprog_file_read(const String& f, MobProg *mprg,
                            MobilePrototype *pMobIndex)
{
	char        MOBProgfile[ MAX_INPUT_LENGTH ];
	MobProg *mprg2;
	FILE       *progfile;
	char        letter;
	bool        done = FALSE;
	Format::sprintf(MOBProgfile, "%s%s", MOB_DIR, f);
	progfile = fopen(MOBProgfile, "r");

	if (!progfile) {
		boot_bug("Mob: %d couldnt open mobprog file", pMobIndex->vnum);
		exit(1);
	}

	mprg2 = mprg;

	switch (letter = fread_letter(progfile)) {
	case '>':
		break;

	case '|':
		boot_bug("empty mobprog file.", 0);
		exit(1);
		break;

	default:
		boot_bug("in mobprog file syntax error.", 0);
		exit(1);
		break;
	}

	while (!done) {
		mprg2->type = mprog_name_to_type(fread_word(progfile));

		switch (mprg2->type) {
		case ERROR_PROG:
			boot_bug("mobprog file type error", 0);
			exit(1);
			break;

		case IN_FILE_PROG:
			boot_bug("mprog file contains a call to file.", 0);
			exit(1);
			break;

		default:
			pMobIndex->progtype_flags = pMobIndex->progtype_flags + mprg2->type;
			mprg2->arglist       = fread_string(progfile);
			mprg2->comlist       = fread_string(progfile);

			switch (letter = fread_letter(progfile)) {
			case '>':
				mprg2->next = new MobProg;
				mprg2       = mprg2->next;
				mprg2->next = nullptr;
				break;

			case '|':
				done = TRUE;
				break;

			default:
				boot_bug("in mobprog file syntax error.", 0);
				exit(1);
				break;
			}

			break;
		}
	}

	fclose(progfile);
	return mprg2;
}

/* This procedure is responsible for reading any in_file MOBprograms.
 */

void mprog_read_programs(FILE *fp, MobilePrototype *pMobIndex)
{
	MobProg *mprg;
	char        letter;
	bool        done = FALSE;

	if ((letter = fread_letter(fp)) != '>') {
		boot_bug("Load_mobiles: vnum %d MOBPROG char", pMobIndex->vnum);
		exit(1);
	}

	pMobIndex->mobprogs = new MobProg;
	mprg = pMobIndex->mobprogs;

	while (!done) {
		mprg->type = mprog_name_to_type(fread_word(fp));

		switch (mprg->type) {
		case ERROR_PROG:
			boot_bug("Load_mobiles: vnum %d MOBPROG type.", pMobIndex->vnum);
			exit(1);
			break;

		case IN_FILE_PROG:
			mprg = mprog_file_read(fread_string(fp), mprg, pMobIndex);
			fread_to_eol(fp);

			switch (letter = fread_letter(fp)) {
			case '>':
				mprg->next = new MobProg;
				mprg       = mprg->next;
				mprg->next = nullptr;
				break;

			case '|':
				mprg->next = nullptr;
				fread_to_eol(fp);
				done = TRUE;
				break;

			default:
				boot_bug("Load_mobiles: vnum %d bad MOBPROG.", pMobIndex->vnum);
				exit(1);
				break;
			}

			break;

		default:
			pMobIndex->progtype_flags = pMobIndex->progtype_flags | mprg->type;
			mprg->arglist        = fread_string(fp);
			fread_to_eol(fp);
			mprg->comlist        = fread_string(fp);
			fread_to_eol(fp);

			switch (letter = fread_letter(fp)) {
			case '>':
				mprg->next = new MobProg;
				mprg       = mprg->next;
				mprg->next = nullptr;
				break;

			case '|':
				mprg->next = nullptr;
				fread_to_eol(fp);
				done = TRUE;
				break;

			default:
				boot_bug("Load_mobiles: vnum %d bad MOBPROG.", pMobIndex->vnum);
				exit(1);
				break;
			}

			break;
		}
	}

	return;
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

