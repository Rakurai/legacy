#include "Area.hh"
#include "Exit.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"
#include "Character.hh"
#include "MobilePrototype.hh"
#include "RoomPrototype.hh"
#include "channels.hh"
#include "db.hh"
#include "file.hh"
#include "Reset.hh"
#include "merc.hh"

Area::Area(World& w, FILE *fp) : world(w) {
	file_name        = fread_string(fp);

	String line = file_name;
	String num;
	line = one_argument(line, num);

	if (num.is_number()) {
		version = atoi(num);
		file_name = line;
	}

	name             = fread_string(fp);
	credits          = fread_string(fp);
	min_vnum         = fread_number(fp);
	max_vnum         = fread_number(fp);

	scan_credits();
}

Area::
~Area() {
	for (Reset *reset: resets)
		delete reset;
}

void Area::
update() {
	if (++age < 3)
		return;

	/*
	 * Check age and reset.
	 * Note: Mud School resets every 3 minutes (not 15).
	 */
	if ((!empty && (nplayer == 0 || age >= 15))
	    || age >= 31) {
		reset();
		age = number_range(0, 3);

		if (min_vnum <= ROOM_VNUM_SCHOOL
		 && max_vnum >= ROOM_VNUM_SCHOOL)
			age = 15 - 2;
		else if (nplayer == 0)
			empty = TRUE;
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

	for (const Reset *pReset: resets) {
		RoomPrototype *pRoomIndex;
		MobilePrototype *pMobIndex;
		ObjectPrototype *pObjIndex;
		ObjectPrototype *pObjToIndex;
		Exit *pexit;
		Object *obj;
		Object *obj_to;
		int count, limit;

		switch (pReset->command) {
		default:
			bug("Reset_area: bad command %c.", pReset->command);
			break;

		case 'M':
			if ((pMobIndex = get_mob_index(pReset->arg1)) == nullptr) {
				bug("Reset_area: 'M': bad vnum %d.", pReset->arg1);
				continue;
			}

			if (pMobIndex->count >= pReset->arg2) {
				last = FALSE;
				break;
			}

			if (pReset->arg3 == 0) { /* random room */
				if (!chance(pReset->arg4))
					continue;

				if ((pRoomIndex = get_random_reset_room()) == nullptr) {
					bug("Reset_area: 'R': no random room found.", 0);
					continue;
				}
			}
			else {
				if ((pRoomIndex = get_room_index(pReset->arg3)) == nullptr) {
					bug("Reset_area: 'R': bad vnum %d.", pReset->arg3);
					continue;
				}

				for (mob = pRoomIndex->people, count = 0; mob != nullptr; mob = mob->next_in_room)
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

			/* Check for memory error. -- Outsider */
			if (! mob) {
				bug("Memory error creating mob in reset_area().", 0);
				break;
			}

			mob->reset = pReset;    /* keep track of what reset it -- Montrey */
			/* Check for pet shop. */
			{
				RoomPrototype *pRoomIndexPrev;
				pRoomIndexPrev = get_room_index(pRoomIndex->vnum - 1);

				if (pRoomIndexPrev != nullptr
				    && GET_ROOM_FLAGS(pRoomIndexPrev).has(ROOM_PET_SHOP))
					mob->act_flags += ACT_PET;
			}
			/* set area */
			mob->zone = pRoomIndex->area;
			char_to_room(mob, pRoomIndex);
			level = URANGE(0, mob->level - 2, LEVEL_HERO - 1);
			last  = TRUE;
			break;

		case 'O':
			// might have loaded the pit from the copyover recovery file
			if (pReset->arg1 == OBJ_VNUM_PIT && donation_pit != nullptr) {
				last = FALSE;
				break;
			}

			if (nplayer > 0 && pReset->arg1 != OBJ_VNUM_PIT) {
				last = FALSE;
				break;
			}

			if ((pObjIndex = get_obj_index(pReset->arg1)) == nullptr) {
				bug("Reset_area: 'O': bad vnum %d.", pReset->arg1);
				continue;
			}

			if ((pRoomIndex = get_room_index(pReset->arg3)) == nullptr) {
				bug("Reset_area: 'R': bad vnum %d.", pReset->arg3);
				continue;
			}

			if (count_obj_list(pObjIndex, pRoomIndex->contents) > 0) {
				last = FALSE;
				break;
			}

			obj = create_object(pObjIndex, UMIN(number_fuzzy(level),
			                                    LEVEL_HERO - 1));

			if (! obj) {
				bug("Error creating object in area_reset.", 0);
				return;
			}

			obj->reset = pReset;    /* keep track of what reset it -- Montrey */
			obj_to_room(obj, pRoomIndex);

			if (pObjIndex->vnum == OBJ_VNUM_PIT)
				donation_pit = obj;
			else
				unique_item(obj);

			last = TRUE;
			break;

		case 'P':
			if ((pObjIndex = get_obj_index(pReset->arg1)) == nullptr) {
				bug("Reset_area: 'P': bad vnum %d.", pReset->arg1);
				continue;
			}

			if ((pObjToIndex = get_obj_index(pReset->arg3)) == nullptr) {
				bug("Reset_area: 'P': bad vnum %d.", pReset->arg3);
				continue;
			}

			if (pReset->arg2 > 50) /* old format */
				limit = 6;
			else if (pReset->arg2 == -1) /* no limit */
				limit = 999;
			else
				limit = pReset->arg2;

			if (nplayer > 0
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

				if (! obj) {
					bug("Memory error creating object.", 0);
					return;
				}

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
			if ((pObjIndex = get_obj_index(pReset->arg1)) == nullptr) {
				bug("Reset_area: 'E' or 'G': bad vnum %d.", pReset->arg1);
				continue;
			}

			if (!last)
				break;

			if (mob == nullptr) {
				bug("Reset_area: 'E' or 'G': null mob for vnum %d.",
				    pReset->arg1);
				last = FALSE;
				break;
			}

			if (mob->pIndexData->pShop != nullptr) {
				int olevel = 0;
				obj = create_object(pObjIndex, olevel);

				if (! obj) {
					bug("Error making object for mob inventory.", 0);
					return;
				}

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

					if (! obj) {
						bug("Memory error creating object to equip mob.", 0);
						return;
					}

					unique_item(obj);

					/* error message if it is too high */
					if (obj->level > mob->level + 3)
						Format::fprintf(stderr,
						        "Err: obj %s (%d) -- %d, mob %s (%d) -- %d\n",
						        obj->short_descr, obj->pIndexData->vnum, obj->level,
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
			if ((pRoomIndex = get_room_index(pReset->arg1)) == nullptr) {
				bug("Reset_area: 'D': bad vnum %d.", pReset->arg1);
				continue;
			}

			if ((pexit = pRoomIndex->exit[pReset->arg2]) == nullptr)
				break;

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
			if ((pRoomIndex = get_room_index(pReset->arg1)) == nullptr) {
				bug("Reset_area: 'R': bad vnum %d.", pReset->arg1);
				continue;
			}

			{
				int d0;
				int d1;

				for (d0 = 0; d0 < pReset->arg2 - 1; d0++) {
					d1                   = number_range(d0, pReset->arg2 - 1);
					pexit                = pRoomIndex->exit[d0];
					pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
					pRoomIndex->exit[d1] = pexit;
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
		log_string("scan_credits: No credits available.\n");
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
		log_string("Missing '}' in credits line\n");
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
		log_string("scan_credits: Empty level range string\n");
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
		log_string(buf);
		return -3;
	}
	else {
		int ilow, ihigh;
		area_type = AREA_TYPE_NORM;
		ilow = atoi(levels);

		if (ilow < 0) {
			Format::sprintf(buf, "scan_credits: Bad start level: %d\n", ilow);
			log_string(buf);
			return -4;
		}

		low_range = ilow;

		/* skip digits and spaces. */
		while (isascii(*levels) && isdigit(*levels)) levels++;

		while (isascii(*levels) && isspace(*levels)) levels++;

		if (!isascii(*levels) || !isdigit(*levels)) {
			log_string("scan_credits: Missing second number of range\n");
			return -5;
		}

		ihigh = atoi(levels);

		if (ihigh < ilow || ihigh > 100) {
			Format::sprintf(buf, "scan_credits: Bad ending level : low : %d High : %d\n", ilow, ihigh);
			log_string(buf);
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

/* pick a random room to reset into -- Montrey */

RoomPrototype * Area::
get_random_reset_room() const
{
	RoomPrototype *room;
	int i, count, pick = 0, pass = 1;

	while (pass <= 2) {
		for (i = min_vnum, count = 0; i <= max_vnum; i++) {
			if ((room = get_room_index(i)) == nullptr)
				continue;

			if (GET_ROOM_FLAGS(room).has_any_of(ROOM_NO_MOB
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
