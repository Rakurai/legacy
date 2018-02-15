#include <vector>

#include "affect/affect_int.hh"
#include "affect/affect_list.hh"
#include "affect/Affect.hh"
#include "Character.hh"
#include "declare.hh"
#include "Flags.hh"
#include "Logging.hh"
#include "macros.hh"
#include "merc.hh"
#include "random.hh"

namespace affect {

// local
void modify_char(void *owner, const Affect *paf, bool fAdd);

// searching

const Affect *list_char(Character *ch) {
	return ch->affected;
}

bool exists_on_char(const Character *ch, Type type) {
	return in_cache(ch, type);
}

const Affect *find_on_char(Character *ch, Type type) {
	return find_in_list(&ch->affected, type);
}

// adding

void copy_to_char(Character *ch, const Affect *aff_template)
{
	copy_to_list(&ch->affected, aff_template);
	modify_char(ch, aff_template, TRUE);
}

void join_to_char(Character *ch, Affect *paf)
{
	fn_params params;

	params.owner = ch;
	params.modifier = modify_char;
	params.data = nullptr;

	dedup_in_list(&ch->affected, paf, &params);
	copy_to_char(ch, paf);
}

void add_perm_to_char(Character *ch, Type type) {
	add_type_to_char(ch, type, ch->level, -1, 1, TRUE);
}

// transform a bitvector into a set of affects or defense mods
void copy_flags_to_char(Character *ch, char letter, Flags bitvector, bool permanent) {
	Affect af;
	af.level = ch->level;
	af.duration = -1;
	af.evolution = 1;
	af.permanent = permanent;

	while (!bitvector.empty()) {
		af.type = none; // reset every time
		if (parse_flags(letter, &af, bitvector)) {
			if (letter == 'A') // special to come up with modifiers
				add_type_to_char(ch, af.type, ch->level, -1, 1, permanent);
			else
				copy_to_char(ch, &af);
		}
	}
}

void add_racial_to_char(Character *ch) {
	copy_flags_to_char(ch, 'A', race_table[ch->race].aff, TRUE);
	copy_flags_to_char(ch, 'I', race_table[ch->race].imm, TRUE);
	copy_flags_to_char(ch, 'R', race_table[ch->race].res, TRUE);
	copy_flags_to_char(ch, 'V', race_table[ch->race].vuln, TRUE);
}

// removing

void remove_from_char(Character *ch, Affect *paf)
{
	remove_from_list(&ch->affected, paf);
	modify_char(ch, paf, FALSE);
	delete paf;
}

void remove_matching_from_char(Character *ch, comparator comp, const Affect *pattern) {
	fn_params params;

	params.owner = ch;
	params.modifier = modify_char;
	params.data = nullptr;

	remove_matching_from_list(&ch->affected, comp, pattern, &params);
}

void remove_marked_from_char(Character *ch) {
	Affect pattern;
	pattern.mark = TRUE;

	remove_matching_from_char(ch, comparator_mark, &pattern);
}

void remove_type_from_char(Character *ch, Type type) {
	Affect pattern;
	pattern.type = type;

	remove_matching_from_char(ch, comparator_type, &pattern);
}

void remove_all_from_char(Character *ch, bool permanent) {
	Affect pattern;
	pattern.permanent = permanent;

	remove_matching_from_char(ch, comparator_permanent, &pattern);
}

// modifying

void iterate_over_char(Character *ch, affect_fn fn, void *data) {
	fn_params params;

	params.owner = ch;
	params.modifier = modify_char;
	params.data = data;

	iterate_over_list(&ch->affected, fn, &params);
}

void sort_char(Character *ch, comparator comp) {
	sort_list(&ch->affected, comp);
}

// utility

void add_type_to_char(Character *ch, Type type, sh_int level, sh_int duration, sh_int evolution, bool permanent) {
	struct aff_st {
		Type type;
		sh_int  location;
		int  modifier;
		sh_int  evolution;
	};

	static const std::vector<aff_st> aff_table = {
		{ age,                 APPLY_STR,     -level/20,           1 },
		{ age,                 APPLY_CON,     -level/20,           1 },
		{ age,                 APPLY_WIS,     level/50,           1 },
		{ age,                 APPLY_AGE,     level,           1 },
		{ armor,               APPLY_AC,      -20,             1 },
		{ barrier,             APPLY_NONE,    0,               1 },
		{ berserk,             APPLY_HITROLL, IS_NPC(ch) ? level/8 : GET_ATTR_HITROLL(ch)/5, 1 },
		{ berserk,             APPLY_DAMROLL, IS_NPC(ch) ? level/8 : GET_ATTR_DAMROLL(ch)/5, 1 },
		{ berserk,             APPLY_AC,      UMAX(10, 10 * (ch->level / 5)), 1 },
		{ bless,               APPLY_HITROLL, level/8,         1 },
		{ bless,               APPLY_SAVES,   -level/8,        1 },
		{ blindness,           APPLY_HITROLL, -4,              1 },
		{ blood_moon,          APPLY_HITROLL, level/20,        1 },
		{ blood_moon,          APPLY_DAMROLL, level/12,        1 },
		{ bone_wall,           APPLY_NONE,    0,               1 },
		{ calm,                APPLY_HITROLL, -5,              1 },
		{ calm,                APPLY_DAMROLL, -5,              1 },
		{ charm_person,        APPLY_NONE,    0,               1 },
		{ change_sex,          APPLY_SEX,     number_range(1,2), 1 }, // count on modulo 3 sex, 1 or 2 are different
		{ channel,             APPLY_STR,     -1,              1 },
		{ channel,             APPLY_CON,     -2,              1 },
		{ chill_touch,         APPLY_STR,     -1,              1 },
		{ curse,               APPLY_HITROLL, -level / 8,      1 },
		{ curse,               APPLY_SAVES,   level / 8,       1 },
		{ dazzle,              APPLY_HITROLL, -4,              1 },
		{ detect_evil,         APPLY_NONE,    0,               1 },
		{ detect_good,         APPLY_NONE,    0,               1 },
		{ detect_magic,        APPLY_NONE,    0,               1 },
		{ detect_invis,        APPLY_NONE,    0,               1 },
		{ detect_hidden,       APPLY_NONE,    0,               1 },
		{ dirt_kicking,        APPLY_HITROLL, -4,              1 },
		{ divine_regeneration, APPLY_NONE,    0,               1 },
		{ faerie_fire,         APPLY_AC,      level * 2,       1 },
		{ fear,                APPLY_HITROLL, -level / 10,     1 },
		{ fear,                APPLY_DAMROLL, -level / 16,     1 },
		{ fear,                APPLY_SAVES,   -level / 14,     1 },
		{ fire_breath,         APPLY_HITROLL, -4,              1 },
		{ flameshield,         APPLY_AC,      -20,             1 },
		{ fly,                 APPLY_NONE,    0,               1 },
		{ focus,               APPLY_NONE,    0,               1 },
		{ force_shield,        APPLY_NONE,    0,               1 },
		{ frenzy,              APPLY_HITROLL, level/6,         1 },
		{ frenzy,              APPLY_DAMROLL, level/6,         1 },
		{ frenzy,              APPLY_AC,      10*(level/12),   1 },
		{ giant_strength,      APPLY_STR,     level/25+2,      1 },
		{ haste,               APPLY_DEX,     0,               1 },
		{ hammerstrike,        APPLY_HITROLL, get_unspelled_hitroll(ch)/4, 1 },
		{ hammerstrike,        APPLY_DAMROLL, get_unspelled_damroll(ch)/4, 1 },
		{ hex,                 APPLY_AC,      level * 3,       1 },
		{ hide,                APPLY_NONE,    0,               1 },
		{ invis,               APPLY_NONE,    0,               1 },
//		{ ironskin,            APPLY_AC,      -100,            1 },
		{ midnight,            APPLY_NONE,    0,               1 },
		{ night_vision,        APPLY_NONE,    0,               1 },
		{ paralyze,            APPLY_NONE,    0,               1 },
		{ pass_door,           APPLY_NONE,    0,               1 },
		{ plague,              APPLY_STR,     -level / 20 - 1, 1 },
		{ poison,              APPLY_STR,     -2,              1 },
		{ protection_evil,     APPLY_SAVES,   -1,              1 },
		{ protection_good,     APPLY_SAVES,   -1,              1 },
		{ rayban,              APPLY_NONE,    0,               1 },
		{ regeneration,        APPLY_NONE,    0,               1 },
		{ sanctuary,           APPLY_NONE,    0,               1 },
		{ shadow_form,         APPLY_NONE,    0,               1 },
		{ sheen,               APPLY_NONE,    0,               1 },
		{ shield,              APPLY_AC,      -20,             1 },
		{ sleep,               APPLY_NONE,    0,               1 },
		{ slow,                APPLY_DEX,     0,               1 },
		{ smokescreen,         APPLY_HITROLL, -4,              1 },
		{ sneak,               APPLY_NONE,    0,               1 },
		{ steel_mist,          APPLY_AC,      -level / 10,     1 },
		{ stone_skin,          APPLY_AC,      -40,             1 },
		{ talon,               APPLY_NONE,    0,               1 },
		{ weaken,              APPLY_STR,     -level/5,        1 },
	};

	Affect af;
	af.where = TO_AFFECTS;
	af.type = type;
	af.level = level;
	af.duration = duration;
	af.evolution = evolution;
	af.permanent = permanent;
	bool found = FALSE;

	for (int i = 0; i < aff_table.size(); i++) {
		if (aff_table[i].type != type) {
			if (found)
				break;

			continue;
		}

		// we use join below, so affects of different evolutions are cumulative
		if (aff_table[i].evolution > evolution)
			continue;

		found = TRUE;
		af.location = aff_table[i].location;
		af.modifier = aff_table[i].modifier;
		join_to_char(ch, &af);
	}

	if (!found)
		Logging::bug("add_type_to_char: affect with type %d not found in table", type);
}

void remort_affect_modify_char(Character *ch, int where, Flags bitvector, bool fAdd) {
	Affect af;
	af.type = none;
	af.level = ch->level;
	af.duration = -1;
	af.evolution = 1;
	af.permanent = TRUE;
	// where, location and modifier will be filled by parse

	char letter = 
		where == TO_RESIST ? 'R' : 
		where == TO_VULN ? 'V' : '?'; // let parse handle error

	while (!bitvector.empty()) {
		if (parse_flags(letter, &af, bitvector))
			modify_char(ch, &af, fAdd);
	}
}

/* hinges on af.where:
	where        type       location  modifier
	TO_OBJECT    ignore     attrmod   amount
    TO_AFFECTS   sn,cache   attrmod   amount
    TO_DEFENSE   ignore     defmod    amount
*/

// the modify function is called any time there is a potential change to the list of
// affects, and here we update any caches or entities that depend on the affect list.
// it is important that owner->affected reflects the new state of the affects, i.e.
// the Affect.hppas already been inserted or removed, and paf is not a member of the set.
void modify_char(void *owner, const Affect *paf, bool fAdd) {
	modify_char((Character *)owner, paf, fAdd);
}

void modify_char(Character *ch, const Affect *paf, bool fAdd) {
	if (paf->where != TO_DEFENSE && paf->where != TO_AFFECTS && paf->where != TO_OBJECT)
		return;

	if (paf->where == TO_DEFENSE) {
		if (paf->location < 1 || paf->location > 32) {
			Logging::bugf("modify_char (%s): bad location %d in TO_DEFENSE", ch->name, paf->location);
			return;
		}

		if (paf->modifier == 0)
			return;

		if (fAdd) {
			if (ch->defense_mod == nullptr) {
				ch->defense_mod = new sh_int[DEFENSE_MOD_MEM_SIZE];
				memset(ch->defense_mod, 0, DEFENSE_MOD_MEM_SIZE);
			}

			ch->defense_mod[0]++;
			ch->defense_mod[paf->location] += paf->modifier;
		}
		else {
			if (ch->defense_mod == nullptr) {
				Logging::bugf("modify_char (%s): attempt to remove from nullptr defense_mod", ch->name);
				return;
			}

			ch->defense_mod[0]--;
			ch->defense_mod[paf->location] -= paf->modifier;

			if (ch->defense_mod[0] == 0) {
				delete[] ch->defense_mod;
				ch->defense_mod = nullptr;
			}
		}

		return;
	}

	if (paf->where == TO_AFFECTS) {
		if (paf->type == none) {
			Logging::bugf("modify_char (%s): bad type %d in TO_AFFECTS", ch->name, paf->type);
			return;
		}

		update_cache(ch, paf->type, fAdd);
	}

	// both TO_OBJECT and TO_AFFECTS can set attribute mods

	// affect makes no mods?  we're done
	if (paf->modifier == 0)
		return;

	if (paf->location != APPLY_NONE) {
		if (paf->location < 1 || paf->location > 32) {
			Logging::bugf("modify_char (%s): bad location %d when modifier is %d", ch->name, paf->location, paf->modifier);
			return;
		}

		if (fAdd) {
			if (ch->apply_cache == nullptr) {
				ch->apply_cache = new int[APPLY_CACHE_MEM_SIZE];
				memset(ch->apply_cache, 0, APPLY_CACHE_MEM_SIZE);
			}

			ch->apply_cache[0]++;
			ch->apply_cache[paf->location] += paf->modifier;
		}
		else {
			if (ch->apply_cache == nullptr) {
				Logging::bugf("modify_char (%s): attempt to remove from nullptr apply_cache", ch->name);
				return;
			}

			ch->apply_cache[paf->location] -= paf->modifier;

			if (--ch->apply_cache[0] <= 0) {
				delete[] ch->apply_cache;
				ch->apply_cache = nullptr;
			}
		}
	}
}

} // namespace affect
