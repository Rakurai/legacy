#include "Area.hh"

#include "argument.hh"
#include "channels.hh"
#include "Character.hh"
#include "Exit.hh"
#include "file.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "Logging.hh"
#include "macros.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"
#include "ObjectValue.hh"
#include "random.hh"
#include "Reset.hh"
#include "Room.hh"
#include "RoomPrototype.hh"
#include "worldmap/Region.hh"
#include "Shop.hh"

Area::
Area(World& w, const String& file_name) : world(w), file_name(file_name) {
	// separated from loading so that the area is valid in the world, for error checking lookups
}

void Area::
load() {
	FILE *fpArea = fopen(String(String(AREA_DIR) + file_name).c_str(), "r");

	if (fpArea == nullptr) {
		perror(file_name.c_str());
		exit(1);
	}

	Format::printf("Now loading area: %s\n", file_name);

	for (; ;) {
		if (fread_letter(fpArea) != '#') {
			Logging::file_bug(fpArea, "Load_area: # not found.", 0);
			exit(1);
		}

		String word = fread_word(fpArea);

		if (name.empty()) {
			if (word != "AREA") {
				Logging::file_bug(fpArea, "Load_area: AREA section not seen yet", 0);
				exit(1);
			}

			load_header(fpArea);
			continue;
		}

		if (word[0] == '$')  break;
		else if (word == "REGION")   load_region(fpArea);
		else if (word == "MOBILES")  load_mobiles(fpArea);
		else if (word == "OBJECTS")  load_objects(fpArea);
		else if (word == "RESETS")  load_resets(fpArea);
		else if (word == "ROOMS")  load_rooms(fpArea);
		else if (word == "SHOPS")  load_shops(fpArea);
		else if (word == "SPECIALS")  load_specials(fpArea);
//				else if (word == "TOURSTARTS")  load_tourstarts(fp);
//				else if (word == "TOURROUTES")  load_tourroutes(fp);
		else {
			Logging::file_bug(fpArea, "Load_area: bad section name.", 0);
			exit(1);
		}
	}

	fclose(fpArea);
}

Area::
~Area() {
	for (Reset *reset: resets)
		delete reset;

	for (auto pair : room_prototypes)
		delete pair.second;

	for (auto pair : mob_prototypes)
		delete pair.second;

	for (auto pair : obj_prototypes)
		delete pair.second;

	if (region != nullptr)
		delete region;

	for (auto& entry : rooms)
		delete entry.second;
}

void Area::
load_header(FILE *fp) {
	fread_string(fp); //	file_name = fread_string(fp);
	name = fread_string(fp);
	credits = fread_string(fp);
	min_vnum = Vnum(fread_number(fp));
	max_vnum = Vnum(fread_number(fp));

	String line = file_name;
	String num;
	line = one_argument(line, num);

	if (num.is_number()) {
		version = atoi(num);
		file_name = line;
	}

	// test for overlapping vnum ranges with already-loaded areas
	for (const auto area : Game::world().areas) {
		if (area == this)
			continue;

		if ((min_vnum >= area->min_vnum && min_vnum <= area->max_vnum)
		 || (max_vnum >= area->min_vnum && max_vnum <= area->max_vnum)) {
			Logging::file_bug(fp, Format::format("Load_area: vnum range overlaps with area file '%s'", area->file_name), 0);
			exit(1);
		}
	}

	scan_credits();
}

void Area::
load_region(FILE *fp) {
	if (region != nullptr)
		delete region;

	region = new worldmap::Region(*this, fp);
}

void Area::
load_rooms(FILE *fp) {
	while (true) {
		char letter = fread_letter(fp);

		if (letter != '#') {
			Logging::file_bug(fp, "Load_rooms: # not found.", 0);
			exit(1);
		}

		Vnum vnum = fread_number(fp);

		if (vnum.value() == 0)
			break;

		if (vnum < min_vnum || vnum > max_vnum) {
			Logging::file_bug(fp, "Load_rooms: room vnum %d out of range.", vnum);
			exit(1);
		}

		if (room_prototypes.find(vnum) != room_prototypes.end()) {
			Logging::file_bug(fp, "Load_rooms: vnum %d duplicated.", vnum);
			exit(1);
		}

		room_prototypes[vnum] = new RoomPrototype(*this, vnum, fp);
		top_room++;
	}
}

void Area::
load_mobiles(FILE *fp)
{
	while (true) {
		char letter = fread_letter(fp);

		if (letter != '#') {
			Logging::file_bug(fp, "Load_mobiles: # not found.", 0);
			exit(1);
		}

		Vnum vnum = fread_number(fp);

		if (vnum.value() == 0)
			break;

		if (vnum < min_vnum || vnum > max_vnum) {
			Logging::file_bug(fp, "Load_mobiles: room vnum %d out of range.", vnum);
			exit(1);
		}

		if (mob_prototypes.find(vnum) != mob_prototypes.end()) {
			Logging::file_bug(fp, "Load_mobiles: vnum %d duplicated.", vnum);
			exit(1);
		}

		mob_prototypes[vnum] = new MobilePrototype(*this, vnum, fp);
		top_mob_index++;
	}
}

void Area::
load_objects(FILE *fp)
{
	while (true) {
		char letter = fread_letter(fp);

		if (letter != '#') {
			Logging::file_bug(fp, "Load_objects: # not found.", 0);
			exit(1);
		}

		Vnum vnum = fread_number(fp);

		if (vnum.value() == 0)
			break;

		if (vnum < min_vnum || vnum > max_vnum) {
			Logging::file_bug(fp, "Load_objects: room vnum %d out of range.", vnum);
			exit(1);
		}

		if (obj_prototypes.find(vnum) != obj_prototypes.end()) {
			Logging::file_bug(fp, "Load_objects: vnum %d duplicated.", vnum);
			exit(1);
		}

		obj_prototypes[vnum] = new ObjectPrototype(*this, vnum, fp);
		top_obj_index++;
	}
}

void Area::
load_shops(FILE *fp)
{
	for (; ;) {
		int shopkeeper = fread_number(fp);

		if (shopkeeper == 0)
			break;

		MobilePrototype *pMobIndex = Game::world().get_mob_prototype(shopkeeper);

		if (pMobIndex == nullptr) {
			Logging::file_bug(fp, "Load_shops: NULL mob index %d", shopkeeper);
			exit(1);
		}

		pMobIndex->pShop = new Shop(fp);
		top_shop++;
	}

	return;
}

void Area::
load_resets(FILE *fp)
{
	for (; ;) {
		char letter = fread_letter(fp);

		if (letter == 'S')
			break;

		if (letter == '*') {
			fread_to_eol(fp);
			continue;
		}

		ungetc(letter, fp);
		Reset *pReset = new Reset(fp);
		resets.push_back(pReset);
	}
}

/*
void load_mobiles(FILE *fp)
{
	if (area_last == nullptr) {
		Logging::file_bug(fp, "Load_mobiles: no #AREA seen yet.", 0);
		exit(1);
	}

	area_last->load_mobiles(fp);
}

void load_objects(FILE *fp)
{
	if (area_last == nullptr) {
		Logging::file_bug(fp, "Load_objects: no #AREA seen yet.", 0);
		exit(1);
	}

	area_last->load_objects(fp);
}

void load_rooms(FILE *fp)
{
	if (area_last == nullptr) {
		Logging::file_bug(fp, "Load_rooms: no #AREA seen yet.", 0);
		exit(1);
	}

	area_last->load_rooms(fp);
}
*/

/*
 * Snarf spec proc declarations.
 */
void Area::
load_specials(FILE *fp)
{
	MobilePrototype *pMobIndex;
	char letter;

	for (; ;) {
		switch (letter = fread_letter(fp)) {
		default:
			Logging::file_bug(fp, "Load_specials: letter '%c' not *MS.", letter);
			exit(1);

		case 'S':
			return;

		case '*':
			break;

		case 'M':
			pMobIndex           = Game::world().get_mob_prototype(fread_number(fp));

			if (!pMobIndex) {
				Logging::file_bug(fp, "Load_specials: 'M': vnum %d.", pMobIndex->vnum);
				exit(1);
			}

			pMobIndex->spec_fun = spec_lookup(fread_word(fp));

			if (pMobIndex->spec_fun == 0) {
				Logging::file_bug(fp, "Load_specials: 'M': vnum %d.", pMobIndex->vnum);
				exit(1);
			}

			break;
		}

		fread_to_eol(fp);
	}
}

MobilePrototype * Area::
get_mob_prototype(const Vnum& vnum) {
	const auto& pair = mob_prototypes.find(vnum);

	if (pair != mob_prototypes.cend())
		return pair->second;

	return nullptr;
}

ObjectPrototype * Area::
get_obj_prototype(const Vnum& vnum) {
	const auto& pair = obj_prototypes.find(vnum);

	if (pair != obj_prototypes.cend())
		return pair->second;

	return nullptr;
}

RoomPrototype * Area::
get_room_prototype(const Vnum& vnum) {
	const auto& pair = room_prototypes.find(vnum);

	if (pair != room_prototypes.cend())
		return pair->second;

	return nullptr;
}

void Area::
update() {
	// Check age and reset.
	if (--age <= 0) {
		reset();
		age = number_range(3, 5);
	}
}

void Area::
reset() {
	Character *mob;
	bool last;
	int level;
	mob         = nullptr;
	last        = TRUE;
	level       = 0;
	bool empty = num_players() == 0;

	for (const Reset *pReset: resets) {
		Room *room;
		MobilePrototype *pMobIndex;
		ObjectPrototype *pObjIndex;
		ObjectPrototype *pObjToIndex;
		Exit *pexit;
		Object *obj;
		Object *obj_to;
		int count, limit;

		switch (pReset->command) {
		default:
			Logging::bugf("(%s) Reset_area: bad command %c.", file_name, pReset->command);
			break;

		case 'M':
			if ((pMobIndex = Game::world().get_mob_prototype(pReset->arg1)) == nullptr) {
				Logging::bugf("(%s) Reset_area: 'M': bad vnum %d.", file_name, pReset->arg1);
				continue;
			}

			if (pMobIndex->count >= pReset->arg2) {
				last = FALSE;
				break;
			}

			if (pReset->arg3 == 0) { /* random room */
				if (!chance(pReset->arg4))
					continue;

				if ((room = get_random_reset_room(pMobIndex)) == nullptr) {
					Logging::bugf("(%s) Reset_area: 'M': no random room found for mob %d.", file_name, pReset->arg1);
					continue;
				}
			}
			else {
				if ((room = Game::world().get_room(Location(pReset->arg3))) == nullptr) {
					Logging::bugf("(%s) Reset_area: 'M': bad location %s.", file_name, Location(pReset->arg3));
					continue;
				}

				for (mob = room->people, count = 0; mob != nullptr; mob = mob->next_in_room)
					if (mob->pIndexData == pMobIndex) {
						count++;

						if (count >= pReset->arg4) {
							last = FALSE;
							break;
						}
					}

				if (count >= pReset->arg4)
					break;
			}

			mob = create_mobile(pMobIndex);
			mob->reset = pReset;    /* keep track of what reset it -- Montrey */
			/* Check for pet shop. */
			{
				Room *roomPrev;
				roomPrev = Game::world().get_room(Location(Vnum(room->prototype.vnum.value() - 1)));

				if (roomPrev != nullptr
				    && roomPrev->flags().has(ROOM_PET_SHOP))
					mob->act_flags += ACT_PET;
			}
			/* set area */
			char_to_room(mob, room);
			level = URANGE(0, mob->level - 2, LEVEL_HERO - 1);
			last  = TRUE;
			break;

		case 'O':
			// might have loaded the pit from the copyover recovery file
			if (pReset->arg1 == OBJ_VNUM_PIT && Game::world().donation_pit != nullptr) {
				last = FALSE;
				break;
			}

			if (!empty && pReset->arg1 != OBJ_VNUM_PIT) {
				last = FALSE;
				break;
			}

			if ((pObjIndex = Game::world().get_obj_prototype(pReset->arg1)) == nullptr) {
				Logging::bugf("(%s) Reset_area: 'O': bad vnum %d.", file_name, pReset->arg1);
				continue;
			}

			if ((room = Game::world().get_room(Location(pReset->arg3))) == nullptr) {
				Logging::bugf("(%s) Reset_area: 'O': bad location %s.", file_name, Location(pReset->arg3));
				continue;
			}

			if (count_obj_list(pObjIndex, room->contents) > 0) {
				last = FALSE;
				break;
			}

			obj = create_object(pObjIndex, UMIN(number_fuzzy(level), LEVEL_HERO - 1));
			obj->reset = pReset;    /* keep track of what reset it -- Montrey */
			obj_to_room(obj, room);

			if (pObjIndex->vnum == OBJ_VNUM_PIT)
				Game::world().donation_pit = obj;
			else
				unique_item(obj);

			last = TRUE;
			break;

		case 'P':
			if ((pObjIndex = Game::world().get_obj_prototype(pReset->arg1)) == nullptr) {
				Logging::bugf("(%s) Reset_area: 'P': bad vnum %d.", file_name, pReset->arg1);
				continue;
			}

			if ((pObjToIndex = Game::world().get_obj_prototype(pReset->arg3)) == nullptr) {
				Logging::bugf("(%s) Reset_area: 'P': bad vnum %d.", file_name, pReset->arg3);
				continue;
			}

			if (pReset->arg2 > 50) /* old format */
				limit = 6;
			else if (pReset->arg2 == -1) /* no limit */
				limit = 999;
			else
				limit = pReset->arg2;

			if (!empty
			    || (obj_to = get_obj_type(pObjToIndex)) == nullptr
			    || (obj_to->in_room == nullptr && !last)
			    || (pObjIndex->count >= limit && number_range(0, 4) != 0)
			    || (count = count_obj_list(pObjIndex, obj_to->contains))
			    > pReset->arg4) {
				last = FALSE;
				break;
			}

			while (count < pReset->arg4) {
				obj = create_object(pObjIndex, number_fuzzy(obj_to->level));
				obj->reset = pReset;    /* keep track of what reset it -- Montrey */
				unique_item(obj);
				obj_to_obj(obj, obj_to);
				count++;

				if (pObjIndex->count >= limit)
					break;
			}

			/* fix object lock state! */
			obj_to->value[1] = obj_to->pIndexData->value[1];
			last = TRUE;
			break;

		case 'G':
		case 'E':
			if ((pObjIndex = Game::world().get_obj_prototype(pReset->arg1)) == nullptr) {
				Logging::bugf("(%s) Reset_area: '%c': bad vnum %d.", file_name, pReset->command, pReset->arg1);
				continue;
			}

			if (!last)
				break;

			if (mob == nullptr) {
				Logging::bugf("(%s) Reset_area: '%c': null mob for vnum %d.",
				    file_name, pReset->command, pReset->arg1);
				last = FALSE;
				break;
			}

			if (mob->pIndexData->pShop != nullptr) {
				int olevel = 0;
				obj = create_object(pObjIndex, olevel);
				obj->extra_flags += ITEM_INVENTORY;
			}
			else {
				if (pReset->arg2 == -1) /* no limit */
					limit = 999;
				else
					limit = pReset->arg2;

				if (pObjIndex->count < limit || number_range(0, 4) == 0) {
					obj = create_object(pObjIndex, UMIN(number_fuzzy(level),
					                                    LEVEL_HERO - 1));

					unique_item(obj);

					/* error message if it is too high */
					if (obj->level > mob->level + 3)
						Logging::bugf("(%s) Reset_area: '%c': obj %s (%d) -- %d, mob %s (%d) -- %d\n",
						        file_name, pReset->command, obj->short_descr, obj->pIndexData->vnum, obj->level,
						        mob->short_descr, mob->pIndexData->vnum, mob->level);
				}
				else
					break;
			}

			obj->reset = pReset;    /* keep track of what reset it -- Montrey */
			obj_to_char(obj, mob);

			if (pReset->command == 'E')
				equip_char(mob, obj, pReset->arg3);

			last = TRUE;
			break;

		case 'D':
			if ((room = Game::world().get_room(Location(pReset->arg1))) == nullptr) {
				Logging::bugf("(%s) Reset_area: 'D': bad location %s.", file_name, Location(pReset->arg1));
				continue;
			}

			if ((pexit = room->exit[pReset->arg2]) == nullptr
			 || !pexit->exit_flags.has(EX_ISDOOR)) {
//				Logging::bugf("(%s) Reset_area: 'D': bad exit %d from location %s.",
//					file_name, pReset->arg2, Location(pReset->arg1));
				break;
			}

			switch (pReset->arg3) {
			case 0:
				pexit->exit_flags -= EX_CLOSED;
				pexit->exit_flags -= EX_LOCKED;
				break;

			case 1:
				pexit->exit_flags += EX_CLOSED;
				pexit->exit_flags -= EX_LOCKED;
				break;

			case 2:
				pexit->exit_flags += EX_CLOSED;
				pexit->exit_flags += EX_LOCKED;
				break;
			}

			last = TRUE;
			break;

		case 'R':
			if ((room = Game::world().get_room(Location(pReset->arg1))) == nullptr) {
				Logging::bugf("(%s) Reset_area: 'R': bad location %s.", file_name, Location(pReset->arg1));
				continue;
			}

			{
				int d0;
				int d1;

				for (d0 = 0; d0 < pReset->arg2 - 1; d0++) {
					d1                   = number_range(d0, pReset->arg2 - 1);
					pexit                = room->exit[d0];
					room->exit[d0] = room->exit[d1];
					room->exit[d1] = pexit;
				}
			}

			break;
		}
	}

	String buf = Format::format("%s has just been reset.", name);
	wiznet(buf, nullptr, nullptr, WIZ_RESETS, 0, 0);
}

/*
 * Pull the level ranges from the area credits field -- Elrac
 * Assumed credit line format example:
 * {H{{ 5 25} {MFinn    {TThe Fourth Tower
 * - Elrac
 */
int Area::
scan_credits()
{
	char line[MIL], buf[MIL];
	String keywords;
	char *p, *levels, *author, *title;
	int nblanks;

	if (credits.empty()) {
		Logging::log("scan_credits: No credits available.\n");
		return -1;
	}

	/* default area type/levels */
	area_type  = AREA_TYPE_NORM;
	low_range  = 1;
	high_range = LEVEL_HERO;

	/* credit line gets mangled in scanning. copy for safekeeping. */
	strcpy(line, credits);
	/*** scan low/high level range numbers ***/
	/* terminate level range at first closing brace. */
	p = strchr(line, '}');

	if (p == nullptr) {
		Logging::log("Missing '}' in credits line\n");
		return -1;
	}

	*p = '\0';
	author = p + 1;
	/* find the first nonblank after the last opening brace char in levels. */
	levels = line;

	while ((p = strchr(levels, '{')) != nullptr) levels = p + 1;

	while (isascii(*levels) && isspace(*levels)) levels++;

	/* truncate level range at last non-blank ascii char */
	nblanks = 0;

	for (p = levels; *p; p++) {
		if (!isascii(*p))
			nblanks = 0;
		else if (isspace(*p))
			nblanks++;
		else {
			*p = toupper(*p);
			nblanks = 0;
		}
	}

	*(p -= nblanks) = '\0';

	if (*levels == '\0') {
		Logging::log("scan_credits: Empty level range string\n");
		return -2;
	}

	if (!strcmp(levels, "ALL")) {
		area_type  = AREA_TYPE_ALL;
		keywords = "ALL ";
	}
	else if (!strcmp(levels, "CLANS") || !strcmp(levels, "CLAN")) {
		low_range  = 15; /* min for clan membership */
		area_type  = AREA_TYPE_CLAN;
		keywords = "CLAN ";
	}
	else if (!strcmp(levels, "IMM") || !strcmp(levels, "IMMS")) {
		low_range  = LEVEL_IMMORTAL;
		high_range = MAX_LEVEL;
		area_type  = AREA_TYPE_IMMS;
		keywords = "IMMS ";
	}
	else if (!strcmp(levels, "HRO") || !strcmp(levels, "HERO")) {
		low_range  = LEVEL_HERO;
		high_range = MAX_LEVEL;
		area_type  = AREA_TYPE_HERO;
		keywords = "HRO HERO ";
	}
	else if (!strcmp(levels, "ARENA")) {
		low_range  = 1;
		high_range = MAX_LEVEL;
		area_type  = AREA_TYPE_ARENA;
		keywords = "ARENA ";
	}
	else if (!strcmp(levels, "XXX")) {
		low_range  = MAX_LEVEL;
		high_range = MAX_LEVEL;
		area_type  = AREA_TYPE_XXX;
		keywords = "XXX ";
	}
	else if (!isascii(*levels) || !isdigit(*levels)) {
		Format::sprintf(buf, "scan_credits: Unrecognized level range: '%s'\n", levels);
		Logging::log(buf);
		return -3;
	}
	else {
		int ilow, ihigh;
		area_type = AREA_TYPE_NORM;
		ilow = atoi(levels);

		if (ilow < 0) {
			Format::sprintf(buf, "scan_credits: Bad start level: %d\n", ilow);
			Logging::log(buf);
			return -4;
		}

		low_range = ilow;

		/* skip digits and spaces. */
		while (isascii(*levels) && isdigit(*levels)) levels++;

		while (isascii(*levels) && isspace(*levels)) levels++;

		if (!isascii(*levels) || !isdigit(*levels)) {
			Logging::log("scan_credits: Missing second number of range\n");
			return -5;
		}

		ihigh = atoi(levels);

		if (ihigh < ilow || ihigh > 100) {
			Format::sprintf(buf, "scan_credits: Bad ending level : low : %d High : %d\n", ilow, ihigh);
			Logging::log(buf);
			return -6;
		}

		high_range = ihigh;
	}

	/*** scan author out of credits line (we assume it's one string) ***/

	while (*author == ' ') author++;

	for (p = author; *p && *p != ' '; p++);

	if (*p == '\0') { /* only one string for author & title */
		title = author;
		author = "anonymous";
	}
	else {
		*p = '\0';

		for (title = p + 1; *title == ' '; title++);
	}

	this->author = author;
	keywords += String(author).uncolor();
	keywords += " ";
	/*** title is the remainder ***/
	this->title = title;
	keywords += String(title).uncolor();

	for (char &c: keywords)
		c = LOWER(c);

	this->keywords = keywords;
	return area_type;
} /* end scan_credits() */

void Area::
create_rooms() {
	for (const auto& pair : room_prototypes) {
		const auto& vnum = pair.first;
		const auto prototype = pair.second;

		// skip rooms associated with a region color
		if (region != nullptr 
		 && region->vnum_to_color(vnum) != worldmap::MapColor::uncolored)
			continue;

		Room *room = new Room(*prototype);

		if (room == nullptr) {
			Logging::bugf("Create_rooms: unable to create room from prototype %d.", vnum);
			exit(1);
		}

		rooms.emplace(room->location.room_id, room);
	}

	// load rooms in region
	if (region != nullptr)
		region->create_rooms();
}

void Area::
add_char(Character *ch) {
	if (IS_NPC(ch))
		return;

	if (IS_IMMORTAL(ch))
		_num_imms++;
	else
		_num_players++;
}

void Area::
remove_char(Character *ch) {
	if (IS_NPC(ch))
		return;

	// very slim chance of someone entering a room as a mortal
	// and leaving as an immortal, or vice versa.  rather than
	// complicated extra logic, just limit the potential damage
	// and fix it with a reboot.
	if (IS_IMMORTAL(ch))
		_num_imms = UMAX(0, _num_imms-1);
	else
		_num_players = UMAX(0, _num_players-1);
}

/* pick a random room to reset into -- Montrey */

Room * Area::
get_random_reset_room(const MobilePrototype *mob)
{
	if (region == nullptr
	 || region->allowed_mob_resets.count(mob->vnum) == 0)
		return nullptr;

	int pick = 0, pass = 1;

	while (pass <= 2) {
		int count = 0;

		for (auto& entry : rooms) {

			Room *room = entry.second;

			if (region->allowed_room_resets.count(room->prototype.vnum) == 0)
				continue;

			if (room->flags().has_any_of(ROOM_NO_MOB
			           | ROOM_PRIVATE
			           | ROOM_SAFE
			           | ROOM_SOLITARY
			           | ROOM_PET_SHOP
			           | ROOM_IMP_ONLY
			           | ROOM_GODS_ONLY
			           | ROOM_LEADER_ONLY
			           | ROOM_TELEPORT
			           | ROOM_NORANDOMRESET))
				continue;

			count++;

			if (pass == 2 && count == pick)
				return room;
		}

		if (pass++ == 2 || count == 0)
			break;

		pick = number_range(0, count);
	}

	return nullptr;
}
