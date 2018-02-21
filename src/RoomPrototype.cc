#include "RoomPrototype.hh"
#include "db.hh"
#include "Exit.hh"
#include "ExtraDescr.hh"
#include "file.hh"
#include "Format.hh"
#include "Logging.hh"
#include "lookup.hh"

RoomPrototype::
RoomPrototype(Area& a, const Vnum& v, FILE *fp) :
	area(a),
	vnum(v),
	name(fread_string(fp)),
	description(fread_string(fp)),
	tele_dest(fread_number(fp)),
	room_flags(fread_flag(fp)),
	sector_type(fread_number(fp)) {

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

			ExitPrototype *pexit = new ExitPrototype(fp);
			exit[door] = pexit;
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
