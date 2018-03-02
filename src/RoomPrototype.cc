#include "RoomPrototype.hh"
#include "db.hh"
#include "Exit.hh"
#include "ExtraDescr.hh"
#include "file.hh"
#include "Format.hh"
#include "Logging.hh"
#include "lookup.hh"
#include "Room.hh"

RoomPrototype::
RoomPrototype(Area& a, const Vnum& v, FILE *fp) :
	area(a),
	vnum(v),
	name(fread_string(fp)),
	description(fread_string(fp)),
	tele_dest(Vnum(fread_number(fp))),
	room_flags(fread_flag(fp)) {

	int old_sector_type = fread_number(fp);

	switch (old_sector_type) {
		case  0: /* SECT_INSIDE       */ sector_type = Sector::inside; break;
		case  1: /* SECT_CITY         */ sector_type = Sector::city; break;
		case  2: /* SECT_FIELD        */ sector_type = Sector::field; break;
		case  3: /* SECT_FOREST       */ sector_type = Sector::forest_sparse; break;
		case  4: /* SECT_HILLS        */ sector_type = Sector::hills; break;
		case  5: /* SECT_MOUNTAIN     */ sector_type = Sector::mountain; break;
		case  6: /* SECT_WATER_SWIM   */ sector_type = Sector::water_swim; break;
		case  7: /* SECT_WATER_NOSWIM */ sector_type = Sector::water_noswim; break;
		case  9: /* SECT_AIR          */ sector_type = Sector::air; break;
		case 10: /* SECT_DESERT       */ sector_type = Sector::desert; break;
		case 20: /* SECT_ARENA        */ sector_type = Sector::arena; break;
		case 21: /* SECT_CLANARENA	  */ sector_type = Sector::clanarena; break;
		default:
			boot_bug("Load_rooms(): unknown sector type %d.", old_sector_type);
			sector_type = Sector::none;
	}

	/* horrible hack */
	if (3000 <= vnum && vnum < 3400)
		room_flags += ROOM_LAW;

	if (room_flags.has(ROOM_FEMALE_ONLY)) {
		Format::sprintf(log_buf, "Room %d is FEMALE_ONLY", vnum);
		Logging::log(log_buf);
	}

	if (room_flags.has(ROOM_MALE_ONLY)) {
		Format::sprintf(log_buf, "Room %d is MALE_ONLY", vnum);
		Logging::log(log_buf);
	}

	if (room_flags.has(ROOM_LOCKER)) {
		Format::sprintf(log_buf, "Room %d is LOCKER", vnum);
		Logging::log(log_buf);
	}
/*
	ExtraDescr *ed;
	Exit *pexit;
	int locks;
	char log_buf[MAX_STRING_LENGTH];
	int vnum;
	char letter;
	int door;
*/
	for (; ;) {
		char letter = fread_letter(fp);

		if (letter == 'S')
			break;

		switch (letter) {
		case 'H':       /* healing room */
			heal_rate = fread_number(fp);
			break;

		case 'M':       /* mana room */
			mana_rate = fread_number(fp);
			break;

		case 'C':       /* clan */
			clan = clan_lookup(fread_string(fp));
			break;

		case 'G':       /* guild */
			if (!(guild = class_lookup(fread_string(fp)) + 1)) {
				boot_bug("Load_rooms: invalid class in guild", 0);
				::exit(1);
			}

			break;

		case 'D': {      /* door */
			int door = fread_number(fp);

			if (door < 0 || door > 5) {
				boot_bug("Fread_rooms: vnum %d has bad door number.", vnum);
				::exit(1);
			}

			if (exit[door] != nullptr) {
				boot_bug("Fread_rooms: vnum %d has two exits in same direction.", vnum);
				::exit(1);
			}

			exit[door] = new ExitPrototype(fp);
			top_exit++;
			break;
		}

		case 'E': {      /* extended desc */
			ExtraDescr *ed = new ExtraDescr(fread_string(fp), fread_string(fp));
			ed->next = extra_descr;
			extra_descr = ed;
			break;
		}

		case 'O':
			owner = fread_string(fp);
			break;

		default:
			boot_bug("Load_rooms: vnum %d has flag not 'CDEHMOS'.", vnum);
			::exit(1);
		}
	}

}

RoomPrototype::
~RoomPrototype() {
	while (extra_descr != nullptr) {
		ExtraDescr *extra = extra_descr;
		extra_descr = extra->next;
		delete extra;
	}

	for (int door = 0; door < 6; door++)
		if (exit[door] != nullptr)
			delete exit[door];
}
