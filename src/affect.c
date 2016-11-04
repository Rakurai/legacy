#include "merc.h"
#include "affect.h"


// reusable callback functions

int affect_fn_debug(AFFECT_DATA *node, void *data) {
	int *count = (int *)data;
//	AFFECT_DATA *node = (AFFECT_DATA *)node;

	(*count)++;
	bugf("callback %d in affect %d", *count, node->type);
	return 0;
}

int affect_fn_fade_spell(AFFECT_DATA *node, void *data) {
	sh_int sn = 0;

	if (data != NULL)
		sn = *(sh_int *)data;

	if (node->duration > 0
	 && (sn <= 0 || node->type == sn)) {
		node->duration--;

		if (node->level > 0 && number_range(0, 4))
			node->level--;  /* spell strength fades with time */
	}

	return 0; // keep going
}


// comparators (remember equality returns 0, like strcmp)

int affect_comparator_mark(const AFFECT_DATA *lhs, const AFFECT_DATA *rhs) {
	return lhs->mark - rhs->mark;
}

int affect_comparator_duration(const AFFECT_DATA *lhs, const AFFECT_DATA *rhs) {
	return lhs->duration - rhs->duration;
}

int affect_comparator_type(const AFFECT_DATA *lhs, const AFFECT_DATA *rhs) {
	return lhs->type - rhs->type;
}

int affect_comparator_permanent(const AFFECT_DATA *lhs, const AFFECT_DATA *rhs) {
	return (lhs->permanent == rhs->permanent) ? 0 : 1;
}

// affect utilities

void affect_update(AFFECT_DATA *paf, const AFFECT_DATA *template) {
	paf->type = template->type;
	paf->where = template->where;
	paf->location = template->location;
	paf->duration = template->duration;
	paf->level = template->level;
	paf->modifier = template->modifier;
	paf->bitvector = template->bitvector;
	paf->evolution = template->evolution;
	paf->permanent = template->permanent;
}

// calculate a checksum over the important parts of the AFFECT_DATA structure, for
// determining whether a list of affects is different from another list.  This is
// Bernstein's djb2 algorithm, from http://www.cse.yorku.ca/~oz/hash.html
unsigned long affect_checksum(const AFFECT_DATA *paf) {
	const unsigned char *str = (const unsigned char *)paf;

	// start checksum at data values
	int start = 0           // 0-index
	 + sizeof(AFFECT_DATA *) // next
	 + sizeof(AFFECT_DATA *) // prev
	 + sizeof(bool)          // valid
	 + sizeof(bool);         // mark
	int end = sizeof(AFFECT_DATA);

	unsigned long hash = 5381;

	// this checksum is intentionally *NOT* insensitive to order of fields/values
	for (int i = start; i < end; i++)
		hash = ((hash << 5) + hash) + str[i]; // hash * 33 + c

	return hash;
}

void affect_swap(AFFECT_DATA *a, AFFECT_DATA *b) {
	if (a == NULL || b == NULL)
		return;

	// we could go through a complicated mechanism for swapping nodes in a double
	// linked list, handling both the adjacent and non-adjacent cases, and being
	// correct... or, screw it, swapping the data is easier. -- Montrey
	AFFECT_DATA t;
	t = *a; // copy, including pointers
	*a = *b;
	*b = t;
	// a and b are swapped, t holds copy of original a
	b->next = a->next; // get correct pointers back into b
	b->prev = a->prev;
	a->next = t.next; // get correct pointers back into a
	a->prev = t.prev;
}

int affect_bit_to_sn(int bit) {
	switch (bit) {
		case AFF_BLIND: return gsn_blindness;
		case AFF_INVISIBLE: return gsn_invis;
		case AFF_DETECT_EVIL: return gsn_detect_evil;
		case AFF_DETECT_GOOD: return gsn_detect_good;
		case AFF_DETECT_INVIS: return gsn_detect_invis;
		case AFF_DETECT_MAGIC: return gsn_detect_magic;
		case AFF_DETECT_HIDDEN: return gsn_detect_hidden;
		case AFF_SANCTUARY: return gsn_sanctuary;
		case AFF_FAERIE_FIRE: return gsn_faerie_fire;
		case AFF_INFRARED: return gsn_night_vision;
		case AFF_CURSE: return gsn_curse;
		case AFF_FEAR: return gsn_fear;
		case AFF_POISON: return gsn_poison;
		case AFF_PROTECT_EVIL: return gsn_protection_evil;
		case AFF_PROTECT_GOOD: return gsn_protection_good;
		case AFF_NIGHT_VISION: return gsn_night_vision;
		case AFF_SNEAK: return gsn_sneak;
		case AFF_HIDE: return gsn_hide;
		case AFF_CHARM: return gsn_charm_person;
		case AFF_FLYING: return gsn_fly;
		case AFF_PASS_DOOR: return gsn_pass_door;
		case AFF_BERSERK: return gsn_berserk;
		case AFF_CALM: return gsn_calm;
		case AFF_HASTE: return gsn_haste;
		case AFF_SLOW: return gsn_slow;
		case AFF_PLAGUE: return gsn_plague;
		case AFF_DIVINEREGEN: return gsn_divine_regeneration;
		case AFF_FLAMESHIELD: return gsn_flameshield;
		case AFF_REGENERATION: return gsn_regeneration;
		case AFF_TALON: return gsn_talon;
		case AFF_STEEL: return gsn_steel_mist;
		default:
			bugf("affect_bit_to_sn: wierd bit %d", bit);
	}

	return -1;
}

int affect_attr_location_check(int location) {
	switch (location) {
		case APPLY_NONE          : return APPLY_NONE;
		case APPLY_STR           : return APPLY_STR;
		case APPLY_DEX           : return APPLY_DEX;
		case APPLY_INT           : return APPLY_INT;
		case APPLY_WIS           : return APPLY_WIS;
		case APPLY_CON           : return APPLY_CON;
		case APPLY_SEX           : return APPLY_SEX;
		case APPLY_AGE           : return APPLY_AGE;
		case APPLY_MANA          : return APPLY_MANA;
		case APPLY_HIT           : return APPLY_HIT;
		case APPLY_STAM          : return APPLY_STAM;
		case APPLY_GOLD          : return APPLY_GOLD;
		case APPLY_EXP           : return APPLY_EXP;
		case APPLY_AC            : return APPLY_AC;
		case APPLY_HITROLL       : return APPLY_HITROLL;
		case APPLY_DAMROLL       : return APPLY_DAMROLL;
		case APPLY_SAVING_PARA   :
		case APPLY_SAVING_ROD    :
		case APPLY_SAVING_PETRI  :
		case APPLY_SAVING_BREATH : return APPLY_SAVES;
		case APPLY_SAVING_SPELL  : return APPLY_SAVING_SPELL;
		case APPLY_SPELL_AFFECT  : return APPLY_SPELL_AFFECT;
		case APPLY_CHR           : return APPLY_CHR;
		case APPLY_SHEEN         : return APPLY_SHEEN;
		case APPLY_BARRIER       : return APPLY_BARRIER;
		case APPLY_FOCUS         : return APPLY_FOCUS;
	}

	bugf("affect_attr_location_check: bad location %d", location);
	return -1;
}

// perform the error checking in building affects from a prototype, including bits.
// fills an AFFECT_DATA struct with one of the bits, removes the bit from
// the vector.  return value indicates whether the struct is valid to insert.
// assumes type, level, duration, evolution, location and modifier already filled,
// but alters if appropriate
bool affect_parse_prototype(char letter, AFFECT_DATA *paf, unsigned int *bitvector) {
	switch (letter) {
	case 'O': paf->where = TO_OBJECT; break; // location and modifier already set
	case 'A': paf->where = TO_AFFECTS; break; // location and modifier already set
	case 'D': paf->where = TO_DEFENSE; break; // modifier already set
	case 'I': paf->where = TO_DEFENSE; paf->modifier = 100; break;
	case 'R': paf->where = TO_DEFENSE; paf->modifier = 50; break;
	case 'V': paf->where = TO_DEFENSE; paf->modifier = -50; break;
	default:
		bugf("affect_parse_prototype: bad letter %c", letter);
		return FALSE;
	}

	// weird case, defense flag A used to be *_SUMMON, changed to a ACT_NOSUMMON.
	// wouldn't be a concern except the index is repurposed to a cache counter.
	if (paf->where == TO_DEFENSE)
		REMOVE_BIT(*bitvector, A);

	// treat the bitvector as an array, find the lowest index with a set bit, remove it from
	// the vector, and use the index to match against new constants
	unsigned int bit = 1;
	int index = 0;

	while (index < 32 && !IS_SET(bit, *bitvector)) {
		bit <<= 1;
		index++;
	}

	if (index < 32)
		REMOVE_BIT(*bitvector, bit);

	// if the bit wasn't found, still continue for the TO_OBJECT.  the loop will
	// stop when bitvector is 0

	if (paf->where == TO_DEFENSE) {
		if (index < 1 || index >= 32) // no bits, not an error, just skip it
			return FALSE;

		paf->location = index;
		// modifier was already set or done above
		paf->bitvector = 0;
		return TRUE;
	}

	if (paf->where == TO_AFFECTS) {
		int sn = affect_bit_to_sn(bit);
		if (sn <= 0) {
			bugf("affect_parse_prototype: sn not found for bit %d in TO_AFFECTS", bit);
			return FALSE;
		}

		paf->type = sn;
		paf->bitvector = 0;
		// drop down to applies for possible location and modifier
	}

	// 'O' apply could theoretically be used to add extra bits to an object,
	// but for now we'll just zero it to make sure weird stuff doesn't happen
	paf->bitvector = 0;

	paf->location = affect_attr_location_check(paf->location);

	if (paf->location == -1)
		return FALSE;

	if (paf->location == 0)
		paf->modifier = 0; // ensure, so we don't mess up the counter

	return TRUE;
}
