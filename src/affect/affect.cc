#include "affect/Affect.hh"
#include "declare.hh"
#include "Flags.hh"
#include "Logging.hh"
#include "merc.hh"
#include "random.hh"

namespace affect {

// reusable callback functions

int fn_debug(Affect *node, void *data) {
	int *count = (int *)data;
//	Affect *node = (Affect *)node;

	(*count)++;
	Logging::bugf("callback %d in affect %d", *count, node->type);
	return 0;
}

int fn_fade_spell(Affect *node, void *data) {
	::affect::type type = ::affect::type::none;

	if (data != nullptr) {
		fn_data_container_type *container = (fn_data_container_type *)data;
		type = container->type;
	}

	if (node->duration > 0
	 && (type == ::affect::type::none || node->type == type)) {
		node->duration--;

		if (node->level > 0 && number_range(0, 4))
			node->level--;  /* spell strength fades with time */
	}

	return 0; // keep going
}

int fn_set_level(Affect *node, void *data) {
	int level = *(int *)data;
	node->level = level;
	return 0; // keep going
}

// comparators (remember equality returns 0, like strcmp)

int comparator_mark(const Affect *lhs, const Affect *rhs) {
	return lhs->mark - rhs->mark;
}

int comparator_duration(const Affect *lhs, const Affect *rhs) {
	return lhs->duration - rhs->duration;
}

int comparator_type(const Affect *lhs, const Affect *rhs) {
	if (lhs->type < rhs->type) return -1;
	if (rhs->type < lhs->type) return  1;
	return 0;
}

int comparator_permanent(const Affect *lhs, const Affect *rhs) {
	return (lhs->permanent == rhs->permanent) ? 0 : 1;
}

// affect utilities

void update(Affect *paf, const Affect *aff_template) {
	paf->type = aff_template->type;
	paf->where = aff_template->where;
	paf->location = aff_template->location;
	paf->duration = aff_template->duration;
	paf->level = aff_template->level;
	paf->modifier = aff_template->modifier;
	paf->bitvector(aff_template->bitvector());
	paf->evolution = aff_template->evolution;
	paf->permanent = aff_template->permanent;
}

// calculate a checksum over the important parts of the Affect structure, for
// determining whether a list of affects is different from another list.  This is
// Bernstein's djb2 algorithm, from http://www.cse.yorku.ca/~oz/hash.html
unsigned long checksum(const Affect *paf) {
	const unsigned char *str = (const unsigned char *)paf;

	// start checksum at data values
	int start = 0           // 0-index
	 + sizeof(Affect *) // next
	 + sizeof(Affect *) // prev
//	 + sizeof(bool)          // valid
	 + sizeof(bool);         // mark
	int end = sizeof(Affect);

	unsigned long hash = 5381;

	// this checksum is intentionally *NOT* insensitive to order of fields/values
	for (int i = start; i < end; i++)
		hash = ((hash << 5) + hash) + str[i]; // hash * 33 + c

	return hash;
}

void swap(Affect *a, Affect *b) {
	if (a == nullptr || b == nullptr)
		return;

	// we could go through a complicated mechanism for swapping nodes in a double
	// linked list, handling both the adjacent and non-adjacent cases, and being
	// correct... or, screw it, swapping the data is easier. -- Montrey
	Affect t;
	t = *a; // copy, including pointers
	*a = *b;
	*b = t;
	// a and b are swapped, t holds copy of original a
	b->next = a->next; // get correct pointers back into b
	b->prev = a->prev;
	a->next = t.next; // get correct pointers back into a
	a->prev = t.prev;
}

::affect::type bit_to_type(Flags::Bit bit) {
	switch (bit) {
		case AFF_BLIND: return ::affect::type::blindness;
		case AFF_INVISIBLE: return ::affect::type::invis;
		case AFF_DETECT_EVIL: return ::affect::type::detect_evil;
		case AFF_DETECT_GOOD: return ::affect::type::detect_good;
		case AFF_DETECT_INVIS: return ::affect::type::detect_invis;
		case AFF_DETECT_MAGIC: return ::affect::type::detect_magic;
		case AFF_DETECT_HIDDEN: return ::affect::type::detect_hidden;
		case AFF_SANCTUARY: return ::affect::type::sanctuary;
		case AFF_FAERIE_FIRE: return ::affect::type::faerie_fire;
		case AFF_INFRARED: return ::affect::type::night_vision;
		case AFF_CURSE: return ::affect::type::curse;
		case AFF_FEAR: return ::affect::type::fear;
		case AFF_POISON: return ::affect::type::poison;
		case AFF_PROTECT_EVIL: return ::affect::type::protection_evil;
		case AFF_PROTECT_GOOD: return ::affect::type::protection_good;
		case AFF_NIGHT_VISION: return ::affect::type::night_vision;
		case AFF_SNEAK: return ::affect::type::sneak;
		case AFF_HIDE: return ::affect::type::hide;
		case AFF_CHARM: return ::affect::type::charm_person;
		case AFF_FLYING: return ::affect::type::fly;
		case AFF_PASS_DOOR: return ::affect::type::pass_door;
		case AFF_BERSERK: return ::affect::type::berserk;
		case AFF_CALM: return ::affect::type::calm;
		case AFF_HASTE: return ::affect::type::haste;
		case AFF_SLOW: return ::affect::type::slow;
		case AFF_PLAGUE: return ::affect::type::plague;
		case AFF_DIVINEREGEN: return ::affect::type::divine_regeneration;
		case AFF_FLAMESHIELD: return ::affect::type::flameshield;
		case AFF_REGENERATION: return ::affect::type::regeneration;
		case AFF_TALON: return ::affect::type::talon;
		case AFF_STEEL: return ::affect::type::steel_mist;
		default:
			Logging::bugf("bit_to_sn: wierd bit %d", bit);
	}

	return ::affect::type::none;
}

int attr_location_check(int location) {
	switch (location) {
		case APPLY_NONE             : return APPLY_NONE;
		case APPLY_STR              : return APPLY_STR;
		case APPLY_DEX              : return APPLY_DEX;
		case APPLY_INT              : return APPLY_INT;
		case APPLY_WIS              : return APPLY_WIS;
		case APPLY_CON              : return APPLY_CON;
		case APPLY_SEX              : return APPLY_SEX;
		case APPLY_AGE              : return APPLY_AGE;
		case APPLY_MANA             : return APPLY_MANA;
		case APPLY_HIT              : return APPLY_HIT;
		case APPLY_STAM             : return APPLY_STAM;
		case APPLY_GOLD             : return APPLY_GOLD;
		case APPLY_EXP_PCT          : return APPLY_EXP_PCT;
		case APPLY_AC               : return APPLY_AC;
		case APPLY_HITROLL          : return APPLY_HITROLL;
		case APPLY_DAMROLL          : return APPLY_DAMROLL;
//		case APPLY_SAVES            : duplicate with an older one
		case APPLY_SAVING_PARA      :
		case APPLY_SAVING_ROD       :
		case APPLY_SAVING_PETRI     :
		case APPLY_SAVING_BREATH    :
		case APPLY_SAVING_SPELL     : return APPLY_SAVES;
		case APPLY_CHR              : return APPLY_CHR;
		case APPLY_QUESTPOINTS      : return APPLY_QUESTPOINTS;
		case APPLY_SKILLPOINTS      : return APPLY_SKILLPOINTS;
		case APPLY_MANA_COST_PCT    : return APPLY_MANA_COST_PCT;
		case APPLY_STAM_COST_PCT    : return APPLY_STAM_COST_PCT;
		case APPLY_WPN_DAMAGE_PCT   : return APPLY_WPN_DAMAGE_PCT;
		case APPLY_SPELL_DAMAGE_PCT : return APPLY_SPELL_DAMAGE_PCT;
		case APPLY_VAMP_BONUS_PCT   : return APPLY_VAMP_BONUS_PCT;
		case APPLY_HP_BLOCK_PCT		: return APPLY_HP_BLOCK_PCT;
		case APPLY_MANA_BLOCK_PCT	: return APPLY_MANA_BLOCK_PCT;
	}

	Logging::bugf("attr_location_check: bad location %d", location);
	return -1;
}

// perform the error checking in building affects from a prototype, including bits.
// fills an Affect struct with one of the bits, removes the bit from
// the vector.  return value indicates whether the struct is valid to insert.
// assumes type, level, duration, evolution, location and modifier already filled,
// but alters if appropriate
bool parse_flags(char letter, Affect *paf, Flags& bitvector) {
	switch (letter) {
	case  0 : break; // use the where that is set
	case 'O': paf->where = TO_OBJECT; break; // location and modifier already set
	case 'W': paf->where = TO_WEAPON; break; // location and modifier already set
	case 'A': paf->where = TO_AFFECTS; break; // location and modifier already set
	case 'D': paf->where = TO_DEFENSE; break; // modifier already set
	case 'I': paf->where = TO_DEFENSE; paf->modifier = 100; break;
	case 'R': paf->where = TO_DEFENSE; paf->modifier = 50; break;
	case 'V': paf->where = TO_DEFENSE; paf->modifier = -50; break;
	default:
		Logging::bugf("parse_flags: bad letter %c", letter);
		return false;
	}

	if (paf->where == TO_DEFENSE) {
		if (paf->modifier == 0) {
			Logging::bug("parse_flags: TO_DEFENSE with modifier of 0", 0);
			return false;
		}

		// weird case, defense flag Flags::A used to be *_SUMMON, changed to a ACT_NOSUMMON.
		// wouldn't be a concern except the index is repurposed to a cache counter.
		bitvector -= Flags::A;

		if (bitvector.has(IMM_WEAPON)) {
			bitvector += IMM_SLASH|IMM_BASH|IMM_PIERCE;
			bitvector -= IMM_WEAPON;
		}

		if (bitvector.has(IMM_MAGIC)) {
			bitvector += IMM_FIRE|IMM_COLD|IMM_ELECTRICITY
				|IMM_ACID|IMM_POISON|IMM_NEGATIVE|IMM_HOLY|IMM_ENERGY
				|IMM_MENTAL|IMM_DISEASE|IMM_DROWNING|IMM_LIGHT|IMM_SOUND;
			bitvector -= IMM_MAGIC;
		}
	}

	// if we're dealing with TO_OBJECT or TO_WEAPON, we leave the bitvector alone and return
	// the whole original affect, after passing through sanity checks.  set local bitvector = 0
	// to fall through the bit conversions

	if (paf->where == TO_AFFECTS && paf->type != ::affect::type::none) {
		// if we passed an sn in, don't parse bits
		bitvector.clear();
		paf->bitvector(0);
	}
	else if (paf->where == TO_OBJECT) {
	 	// or, just quit the outside loop and leave paf->bitvector alone
	 	bitvector.clear();
	}

	// others, parse and remove one bit, and let the outside loop call again
	// if bitvector is 0 here, we'll just fall through

	// treat the bitvector as an array, find the lowest index with a set bit, remove it from
	// the vector, and use the index to match against new constants
	Flags::Bit bit = Flags::none;

	if (!bitvector.empty()) {
		unsigned long bit_val = 1;
		int index = 0;

		while (index < 32 && !bitvector.has(static_cast<Flags::Bit>(bit_val))) {
			bit_val <<= 1;
			index++;
		}

		if (index < 32) {
			bit = static_cast<Flags::Bit>(bit_val);
			bitvector -= bit;
		}
	}

	// if the bit wasn't found, still continue for the TO_OBJECT.  the loop will
	// stop when bitvector is 0

	if (paf->where == TO_WEAPON) {
		switch (bit) {
			case WEAPON_ACIDIC     : paf->type = ::affect::type::weapon_acidic; break;
			case WEAPON_FLAMING    : paf->type = ::affect::type::weapon_flaming; break;
			case WEAPON_FROST      : paf->type = ::affect::type::weapon_frost; break;
			case WEAPON_VAMPIRIC   : paf->type = ::affect::type::weapon_vampiric; break;
			case WEAPON_SHOCKING   : paf->type = ::affect::type::weapon_shocking; break;
			case WEAPON_SHARP      : paf->type = ::affect::type::weapon_sharp; break;
			case WEAPON_VORPAL     : paf->type = ::affect::type::weapon_vorpal; break;
			case WEAPON_POISON     : paf->type = ::affect::type::poison; break;
			case WEAPON_TWO_HANDS  : paf->type = ::affect::type::weapon_two_hands; break;
			case Flags::none       : break; // type already set
			default: {
				Logging::bugf("parse_flags: TO_WEAPON with unknown defense bit %d", bit);
				return false;
			}
		}

		if (paf->type == ::affect::type::none) {
			Logging::bug("parse_flags: TO_WEAPON with no bits and no type", 0);
			return false;
		}

		return true;
	}

	if (paf->where == TO_DEFENSE) {

		switch (bit) {
			case IMM_CHARM       : paf->location = DAM_CHARM; break;
			case IMM_BASH        : paf->location = DAM_BASH; break;
			case IMM_PIERCE      : paf->location = DAM_PIERCE; break;
			case IMM_SLASH       : paf->location = DAM_SLASH; break;
			case IMM_FIRE        : paf->location = DAM_FIRE; break;
			case IMM_COLD        : paf->location = DAM_COLD; break;
			case IMM_ELECTRICITY : paf->location = DAM_ELECTRICITY; break;
			case IMM_ACID        : paf->location = DAM_ACID; break;
			case IMM_POISON      : paf->location = DAM_POISON; break;
			case IMM_NEGATIVE    : paf->location = DAM_NEGATIVE; break;
			case IMM_HOLY        : paf->location = DAM_HOLY; break;
			case IMM_ENERGY      : paf->location = DAM_ENERGY; break;
			case IMM_MENTAL      : paf->location = DAM_MENTAL; break;
			case IMM_DISEASE     : paf->location = DAM_DISEASE; break;
			case IMM_DROWNING    : paf->location = DAM_DROWNING; break;
			case IMM_LIGHT       : paf->location = DAM_LIGHT; break;
			case IMM_SOUND       : paf->location = DAM_SOUND; break;
			case IMM_WOOD        : paf->location = DAM_WOOD; break;
			case IMM_SILVER      : paf->location = DAM_SILVER; break;
			case IMM_IRON        : paf->location = DAM_IRON; break;
			case Flags::none     : /* location already set */ break;
			default: {
				Logging::bugf("parse_flags: TO_DEFENSE with unknown defense bit %d", bit);
				return false;
			}
		}

		if (paf->location == 0) {
			Logging::bug("parse_flags: TO_DEFENSE with location 0", 0);
			return false;
		}

		// modifier was already set or done above
		paf->bitvector(0);
		return true;
	} // done with TO_DEFENSE

	if (paf->where == TO_AFFECTS && paf->type == ::affect::type::none) {
		if (bit == Flags::none) {
			Logging::bug("parse_flags: TO_AFFECTS with no type and no bit", 0);
			return false;
		}

		::affect::type type = bit_to_type(bit);

		if (type == ::affect::type::none) {
			Logging::bugf("parse_flags: TO_AFFECTS: sn not found for bit %d", bit);
			return false;
		}

		paf->type = type;
		paf->bitvector(0);
		// drop down to applies for possible location and modifier
	}

	// from here, we leave bitvector alone: TO_OBJECT and TO_WEAPON can
	// add flags to the object's extra bits

	paf->location = attr_location_check(paf->location);

	if (paf->location == -1) {
		Logging::bugf("parse_flags: affect where=%d with bad location %d", paf->where, paf->location);
		return false;
	}

	if (paf->location == 0)
		paf->modifier = 0; // ensure, so we don't mess up the counter

	// does nothing?
	if (paf->where == TO_OBJECT && paf->bitvector().empty() && paf->location == 0) {
		Logging::bug("parse_flags: TO_OBJECT with no modifiers", 0);
		return false;
	}

	return true;
}

} // namespace affect
