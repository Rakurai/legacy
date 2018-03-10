#include "ObjectPrototype.hh"
#include "Area.hh"
#include "file.hh"
#include "lookup.hh"
#include "skill/skill.hh"
#include "affect/affect_list.hh"
#include "ExtraDescr.hh"
#include "Location.hh"
#include "Logging.hh"

ObjectPrototype::
ObjectPrototype(Area& area, const Vnum& vnum, FILE *fp) :
	area(area),
	vnum(vnum)
{
	reset_num            = 0;
	name                 = fread_string(fp);
	short_descr          = fread_string(fp);
	description          = fread_string(fp);
	material             = fread_string(fp);
	item_type            = item_lookup(fread_word(fp));
	extra_flags          = fread_flag(fp);
	wear_flags           = fread_flag(fp);
	num_settings			= 0;


	int val = 0; // prevent accidents in altering below switches
	switch (item_type) {
	case ITEM_WEAPON:
		value[val]         = ObjectValue(get_weapon_type(fread_word(fp)));
		break;

	case ITEM_KEY:
		value[val]         = ObjectValue(fread_flag(fp));
		break;

	default:
		value[val]         = ObjectValue(fread_number(fp));
		break;
	}


	val = 1;
	switch (item_type) {
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	case ITEM_PORTAL:
		value[val]         = ObjectValue(fread_flag(fp));
		break;

	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_SCROLL:
		value[val]         = ObjectValue((int)skill::lookup(fread_word(fp)));
		break;

	default:
		value[val]         = ObjectValue(fread_number(fp));
		break;
	}


	val = 2;
	switch (item_type) {
	case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
		value[val]         = ObjectValue(liq_lookup(fread_word(fp)));

		if (value[val] == -1) {
			value[val] = 0;
			Logging::file_bug(fp, "Unknown liquid type", 0);
		}

		break;

	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_SCROLL:
		value[val]         = ObjectValue((int)skill::lookup(fread_word(fp)));
		break;

	case ITEM_FURNITURE:
	case ITEM_PORTAL:
	case ITEM_ANVIL:
		value[val]         = ObjectValue(fread_flag(fp));
		break;

	default:
		value[val]         = ObjectValue(fread_number(fp));
		break;
	}


	val = 3;
	switch (item_type) {
	case ITEM_WEAPON:
		value[val]         = ObjectValue(attack_lookup(fread_word(fp)));
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_SCROLL:
		value[val]         = ObjectValue((int)skill::lookup(fread_word(fp)));
		break;

	case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
	case ITEM_FOOD:
		value[val]         = ObjectValue(fread_flag(fp));
		break;

	case ITEM_PORTAL:
		value[val]         = ObjectValue(Location(fread_word(fp)).to_int());
		break;

	default:
		value[val]         = ObjectValue(fread_number(fp));
		break;
	}


	val = 4;
	switch (item_type) {
	case ITEM_WEAPON: {
		Flags bitvector               = fread_flag(fp);

		// preserve the bits, we use them for loading old versions of players
		value[val] = ObjectValue(bitvector);

		affect::Affect af;
		af.level              = level;
		af.duration           = -1;
		af.evolution          = 1;
		af.permanent          = true;
		af.location           = 0;
		af.modifier           = 0;

		while (!bitvector.empty()) {
			af.type = affect::type::none; // reset every time

			if (affect::parse_flags('W', &af, bitvector))
				affect::copy_to_list(&affected, &af); 
		}

		break;
	}

	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_SCROLL:
		value[val]         = ObjectValue((int)skill::lookup(fread_word(fp)));
		break;

	default:
		value[val]         = ObjectValue(fread_number(fp));
		break;
	}


	level                = fread_number(fp);

	// this is annoying.  since v0-v4 come before level in an object section,
	// we had to create any affects (like weapon flags) at level 0.
	// fix them up now.
	affect::fn_params params;
	params.owner = nullptr;
	params.data = &level;

	affect::iterate_over_list(
		&affected,
		affect::fn_set_level,
		&params
	);

	weight               = fread_number(fp);
	cost                 = fread_number(fp);
	/* condition */
	char letter                          = fread_letter(fp);

	switch (letter) {
	case ('P') :                condition = 100; break;

	case ('G') :                condition =  90; break;

	case ('A') :                condition =  75; break;

	case ('W') :                condition =  50; break;

	case ('D') :                condition =  25; break;

	case ('B') :                condition =  10; break;

	case ('R') :                condition =   5; break;

	case ('I') :                condition =  -1; break;

	default:                    condition = 100; break;
	}

	for (; ;) {
		char letter;
		letter = fread_letter(fp);

		if (letter == 'A') { // apply
			affect::Affect af;
			af.type               = affect::type::none;
			af.level              = level;
			af.duration           = -1;
			af.location           = fread_number(fp);
			af.modifier           = fread_number(fp);
			af.evolution          = 1;
			af.bitvector(0);
			af.permanent          = true;

			Flags bitvector = 0;
			if (affect::parse_flags('O', &af, bitvector)) {
				affect::copy_to_list(&affected, &af);
			}
		}
		else if (letter == 'F') { // flag, can add bits or do other ->where types
			affect::Affect af;
			af.type               = affect::type::none;
			af.level              = level;
			af.duration           = -1;
			af.evolution          = 1;
			af.permanent          = true;

			letter          = fread_letter(fp);
			af.location     = fread_number(fp);
			af.modifier     = fread_number(fp);

			Flags bitvector    = fread_flag(fp);

			// do at least once even if no bitvector
			do {
				if (affect::parse_flags(letter, &af, bitvector)) {
					affect::copy_to_list(&affected, &af); 

					// don't multiply the modifier, just apply to the first bit
					af.location = 0;
					af.modifier = 0;
				}

				af.type = affect::type::none; // reset every time
			} while (!bitvector.empty());
		}
		else if (letter == 'E') {
			ExtraDescr *ed = new ExtraDescr(fread_string(fp), fread_string(fp));
			ed->next                = extra_descr;
			extra_descr  = ed;
		}
		else if (letter == 'S') {
			num_settings = fread_number(fp);
		}
		else {
			ungetc(letter, fp);
			break;
		}
	}

	// affects are immutable, compute the checksum now
	affect_checksum = affect::checksum_list(&affected);

}
