#include <vector>

#include "affect_int.hh"
#include "affect_list.hh"
#include "Affect.hh"
#include "Character.hh"
#include "declare.hh"
#include "Flags.hh"
#include "Logging.hh"
#include "macros.hh"
#include "merc.hh"
#include "random.hh"

// local declarations
void affect_modify_char(void *owner, const Affect *paf, bool fAdd);

// searching

const Affect *affect_list_char(Character *ch) {
	return ch->affected;
}

bool affect_exists_on_char(const Character *ch, int sn) {
	return affect_in_cache(ch, sn);
}

const Affect *affect_find_on_char(Character *ch, int sn) {
	return affect_find_in_list(&ch->affected, sn);
}

// adding

void affect_copy_to_char(Character *ch, const Affect *aff_template)
{
	affect_copy_to_list(&ch->affected, aff_template);
	affect_modify_char(ch, aff_template, TRUE);
}

void affect_join_to_char(Character *ch, Affect *paf)
{
	affect_fn_params params;

	params.owner = ch;
	params.modifier = affect_modify_char;
	params.data = nullptr;

	affect_dedup_in_list(&ch->affected, paf, &params);
	affect_copy_to_char(ch, paf);
}

void affect_add_perm_to_char(Character *ch, int sn) {
	affect_add_sn_to_char(ch, sn, ch->level, -1, 1, TRUE);
}

// transform a bitvector into a set of affects or defense mods
void affect_copy_flags_to_char(Character *ch, char letter, Flags bitvector, bool permanent) {
	Affect af;
	af.level = ch->level;
	af.duration = -1;
	af.evolution = 1;
	af.permanent = permanent;

	while (!bitvector.empty()) {
		af.type = 0; // reset every time
		if (affect_parse_flags(letter, &af, bitvector)) {
			if (letter == 'A') // special to come up with modifiers
				affect_add_sn_to_char(ch, af.type, ch->level, -1, 1, permanent);
			else
				affect_copy_to_char(ch, &af);
		}
	}
}

void affect_add_racial_to_char(Character *ch) {
	affect_copy_flags_to_char(ch, 'A', race_table[ch->race].aff, TRUE);
	affect_copy_flags_to_char(ch, 'I', race_table[ch->race].imm, TRUE);
	affect_copy_flags_to_char(ch, 'R', race_table[ch->race].res, TRUE);
	affect_copy_flags_to_char(ch, 'V', race_table[ch->race].vuln, TRUE);
}

// removing

void affect_remove_from_char(Character *ch, Affect *paf)
{
	affect_remove_from_list(&ch->affected, paf);
	affect_modify_char(ch, paf, FALSE);
	delete paf;
}

void affect_remove_matching_from_char(Character *ch, affect_comparator comp, const Affect *pattern) {
	affect_fn_params params;

	params.owner = ch;
	params.modifier = affect_modify_char;
	params.data = nullptr;

	affect_remove_matching_from_list(&ch->affected, comp, pattern, &params);
}

void affect_remove_marked_from_char(Character *ch) {
	Affect pattern;
	pattern.mark = TRUE;

	affect_remove_matching_from_char(ch, affect_comparator_mark, &pattern);
}

void affect_remove_sn_from_char(Character *ch, int sn) {
	Affect pattern;
	pattern.type = sn;

	affect_remove_matching_from_char(ch, affect_comparator_type, &pattern);
}

void affect_remove_all_from_char(Character *ch, bool permanent) {
	Affect pattern;
	pattern.permanent = permanent;

	affect_remove_matching_from_char(ch, affect_comparator_permanent, &pattern);
}

// modifying

void affect_iterate_over_char(Character *ch, affect_fn fn, void *data) {
	affect_fn_params params;

	params.owner = ch;
	params.modifier = affect_modify_char;
	params.data = data;

	affect_iterate_over_list(&ch->affected, fn, &params);
}

void affect_sort_char(Character *ch, affect_comparator comp) {
	affect_sort_list(&ch->affected, comp);
}

// utility

void affect_add_sn_to_char(Character *ch, sh_int sn, sh_int level, sh_int duration, sh_int evolution, bool permanent) {
	struct aff_st {
		sh_int  sn;
		sh_int  location;
		int  modifier;
		sh_int  evolution;
	};

	static const std::vector<aff_st> aff_table = {
		{ gsn_age,                 APPLY_STR,     -level/20,           1 },
		{ gsn_age,                 APPLY_CON,     -level/20,           1 },
		{ gsn_age,                 APPLY_WIS,     level/50,           1 },
		{ gsn_age,                 APPLY_AGE,     level,           1 },
		{ gsn_armor,               APPLY_AC,      -20,             1 },
		{ gsn_barrier,             APPLY_NONE,    0,               1 },
		{ gsn_berserk,             APPLY_HITROLL, IS_NPC(ch) ? level/8 : GET_ATTR_HITROLL(ch)/5, 1 },
		{ gsn_berserk,             APPLY_DAMROLL, IS_NPC(ch) ? level/8 : GET_ATTR_DAMROLL(ch)/5, 1 },
		{ gsn_berserk,             APPLY_AC,      UMAX(10, 10 * (ch->level / 5)), 1 },
		{ gsn_bless,               APPLY_HITROLL, level/8,         1 },
		{ gsn_bless,               APPLY_SAVES,   -level/8,        1 },
		{ gsn_blindness,           APPLY_HITROLL, -4,              1 },
		{ gsn_blood_moon,          APPLY_HITROLL, level/20,        1 },
		{ gsn_blood_moon,          APPLY_DAMROLL, level/12,        1 },
		{ gsn_bone_wall,           APPLY_NONE,    0,               1 },
		{ gsn_calm,                APPLY_HITROLL, -5,              1 },
		{ gsn_calm,                APPLY_DAMROLL, -5,              1 },
		{ gsn_charm_person,        APPLY_NONE,    0,               1 },
		{ gsn_change_sex,          APPLY_SEX,     number_range(1,2), 1 }, // count on modulo 3 sex, 1 or 2 are different
		{ gsn_channel,             APPLY_STR,     -1,              1 },
		{ gsn_channel,             APPLY_CON,     -2,              1 },
		{ gsn_chill_touch,         APPLY_STR,     -1,              1 },
		{ gsn_curse,               APPLY_HITROLL, -level / 8,      1 },
		{ gsn_curse,               APPLY_SAVES,   level / 8,       1 },
		{ gsn_dazzle,              APPLY_HITROLL, -4,              1 },
		{ gsn_detect_evil,         APPLY_NONE,    0,               1 },
		{ gsn_detect_good,         APPLY_NONE,    0,               1 },
		{ gsn_detect_magic,        APPLY_NONE,    0,               1 },
		{ gsn_detect_invis,        APPLY_NONE,    0,               1 },
		{ gsn_detect_hidden,       APPLY_NONE,    0,               1 },
		{ gsn_dirt_kicking,        APPLY_HITROLL, -4,              1 },
		{ gsn_divine_regeneration, APPLY_NONE,    0,               1 },
		{ gsn_faerie_fire,         APPLY_AC,      level * 2,       1 },
		{ gsn_fear,                APPLY_HITROLL, -level / 10,     1 },
		{ gsn_fear,                APPLY_DAMROLL, -level / 16,     1 },
		{ gsn_fear,                APPLY_SAVES,   -level / 14,     1 },
		{ gsn_fire_breath,         APPLY_HITROLL, -4,              1 },
		{ gsn_flameshield,         APPLY_AC,      -20,             1 },
		{ gsn_fly,                 APPLY_NONE,    0,               1 },
		{ gsn_focus,               APPLY_NONE,    0,               1 },
		{ gsn_force_shield,        APPLY_NONE,    0,               1 },
		{ gsn_frenzy,              APPLY_HITROLL, level/6,         1 },
		{ gsn_frenzy,              APPLY_DAMROLL, level/6,         1 },
		{ gsn_frenzy,              APPLY_AC,      10*(level/12),   1 },
		{ gsn_giant_strength,      APPLY_STR,     level/25+2,      1 },
		{ gsn_haste,               APPLY_DEX,     0,               1 },
		{ gsn_hammerstrike,        APPLY_HITROLL, get_unspelled_hitroll(ch)/4, 1 },
		{ gsn_hammerstrike,        APPLY_DAMROLL, get_unspelled_damroll(ch)/4, 1 },
		{ gsn_hex,                 APPLY_AC,      level * 3,       1 },
		{ gsn_hide,                APPLY_NONE,    0,               1 },
		{ gsn_invis,               APPLY_NONE,    0,               1 },
//		{ gsn_ironskin,            APPLY_AC,      -100,            1 },
		{ gsn_midnight,            APPLY_NONE,    0,               1 },
		{ gsn_night_vision,        APPLY_NONE,    0,               1 },
		{ gsn_paralyze,            APPLY_NONE,    0,               1 },
		{ gsn_pass_door,           APPLY_NONE,    0,               1 },
		{ gsn_plague,              APPLY_STR,     -level / 20 - 1, 1 },
		{ gsn_poison,              APPLY_STR,     -2,              1 },
		{ gsn_protection_evil,     APPLY_SAVES,   -1,              1 },
		{ gsn_protection_good,     APPLY_SAVES,   -1,              1 },
		{ gsn_rayban,              APPLY_NONE,    0,               1 },
		{ gsn_regeneration,        APPLY_NONE,    0,               1 },
		{ gsn_sanctuary,           APPLY_NONE,    0,               1 },
		{ gsn_shadow_form,         APPLY_NONE,    0,               1 },
		{ gsn_sheen,               APPLY_NONE,    0,               1 },
		{ gsn_shield,              APPLY_AC,      -20,             1 },
		{ gsn_sleep,               APPLY_NONE,    0,               1 },
		{ gsn_slow,                APPLY_DEX,     0,               1 },
		{ gsn_smokescreen,         APPLY_HITROLL, -4,              1 },
		{ gsn_sneak,               APPLY_NONE,    0,               1 },
		{ gsn_steel_mist,          APPLY_AC,      -level / 10,     1 },
		{ gsn_stone_skin,          APPLY_AC,      -40,             1 },
		{ gsn_talon,               APPLY_NONE,    0,               1 },
		{ gsn_weaken,              APPLY_STR,     -level/5,        1 },
	};

	Affect af;
	af.where = TO_AFFECTS;
	af.type = sn;
	af.level = level;
	af.duration = duration;
	af.evolution = evolution;
	af.permanent = permanent;
	bool found = FALSE;

	for (int i = 0; i < aff_table.size(); i++) {
		if (aff_table[i].sn != sn) {
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
		affect_join_to_char(ch, &af);
	}

	if (!found)
		Logging::bug("affect_add_sn_to_char: affect with sn %d not found in table", sn);
}

void remort_affect_modify_char(Character *ch, int where, Flags bits, bool fAdd) {
	Affect af;
	af.type = 0;
	af.level = ch->level;
	af.duration = -1;
	af.evolution = 1;
	af.permanent = TRUE;
	// where, location and modifier will be filled by parse

	char letter = 
		where == TO_RESIST ? 'R' : 
		where == TO_VULN ? 'V' : '?'; // let parse handle error

	while (affect_parse_flags(letter, &af, bits))
		affect_modify_char(ch, &af, fAdd);
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
void affect_modify_char(void *owner, const Affect *paf, bool fAdd) {
	Character *ch = (Character *)owner;

	if (paf->where != TO_DEFENSE && paf->where != TO_AFFECTS && paf->where != TO_OBJECT)
		return;

	if (paf->where == TO_DEFENSE) {
		if (paf->location < 1 || paf->location > 32) {
			Logging::bugf("affect_modify_char: bad location %d in TO_DEFENSE", paf->location);
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
				Logging::bug("affect_modify_char: attempt to remove from nullptr defense_mod", 0);
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
		if (paf->type < 1 || paf->type >= skill_table.size()) {
			Logging::bugf("affect_modify_char: bad type %d in TO_AFFECTS", paf->type);
			return;
		}

		update_affect_cache(ch, paf->type, fAdd);
	}

	// both TO_OBJECT and TO_AFFECTS can set attribute mods

	// affect makes no mods?  we're done
	if (paf->modifier == 0)
		return;

	if (paf->location != APPLY_NONE) {
		if (paf->location < 1 || paf->location > 32) {
			Logging::bugf("affect_modify_char: bad location %d when modifier is %d", paf->location, paf->modifier);
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
				Logging::bug("affect_modify_char: attempt to remove from nullptr apply_cache", 0);
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
