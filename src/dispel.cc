#include "act.hh"
#include "affect/Affect.hh"
#include "Character.hh"
#include "declare.hh"
#include "macros.hh"
#include "merc.hh"
#include "random.hh"

struct dispel_type {
	bool can_undo;
	bool can_cancel; // can cancel on self
	bool can_dispel; // can dispel on others with "dispel magic"
	const String msg_to_room;
};

// table for magical removal of affects
// note: the table includes spells like 'plague' that cannot be dispelled or cancelled, but
// the 'cure *' routines use this table as well.
// can_undo   - can be removed on self or others with 'undo spell'
// can_cancel - can be removed on self with 'cancellation'
// can_dispel - can be removed on others with 'dispel magic'
// none       - can be removed on self or others ONLY with the appropriate 'cure *'
// anything *NOT* in this table cannot be removed (only by time).  breath effects and dirt kicking, for example

static const std::map<affect::type, const dispel_type> dispel_table = {
	{ affect::type::age,                { TRUE,  FALSE, FALSE }},
	{ affect::type::armor,              { TRUE,  TRUE,  TRUE  }},
	{ affect::type::barrier,            { TRUE,  FALSE, FALSE }},
	{ affect::type::bless,              { TRUE,  TRUE,  TRUE  }},
	{ affect::type::blindness,          { FALSE, FALSE, FALSE }},
	{ affect::type::blood_moon,         { TRUE,  TRUE,  TRUE  }},
	{ affect::type::bone_wall,          { TRUE,  FALSE, FALSE }},
	{ affect::type::calm,               { TRUE,  FALSE, TRUE  }},
	{ affect::type::change_sex,         { TRUE,  FALSE, FALSE }},
	{ affect::type::charm_person,       { TRUE,  FALSE, TRUE  }},
	{ affect::type::change_sex,         { TRUE,  FALSE, FALSE }},
	{ affect::type::chill_touch,        { TRUE,  FALSE, FALSE }},
	{ affect::type::curse,              { FALSE, FALSE, FALSE }},
	{ affect::type::detect_evil,        { TRUE,  TRUE,  TRUE  }},
	{ affect::type::detect_good,        { TRUE,  TRUE,  TRUE  }},
	{ affect::type::detect_hidden,      { TRUE,  TRUE,  TRUE  }},
	{ affect::type::detect_invis,       { TRUE,  TRUE,  TRUE  }},
	{ affect::type::detect_magic,       { TRUE,  TRUE,  TRUE  }},
	{ affect::type::divine_regeneration,{ TRUE,  TRUE,  TRUE  }},
	{ affect::type::faerie_fire,        { TRUE,  TRUE,  FALSE }},
	{ affect::type::fear,               { TRUE,  FALSE, FALSE }},
	{ affect::type::flameshield,        { TRUE,  TRUE,  TRUE  }},
	{ affect::type::fly,                { TRUE,  TRUE,  TRUE  }},
	{ affect::type::focus,              { TRUE,  FALSE, FALSE }},
	{ affect::type::force_shield,       { TRUE,  FALSE, FALSE }},
	{ affect::type::frenzy,             { TRUE,  TRUE,  TRUE  }},
	{ affect::type::giant_strength,     { TRUE,  TRUE,  TRUE  }},
	{ affect::type::haste,              { TRUE,  TRUE,  TRUE  }},
	{ affect::type::hex,                { TRUE,  FALSE, FALSE }},
	{ affect::type::night_vision,       { TRUE,  TRUE,  TRUE  }},
	{ affect::type::invis,              { TRUE,  TRUE,  TRUE  }},
//	{ affect::type::ironskin,           { TRUE,  FALSE, FALSE }},
	{ affect::type::paralyze,           { TRUE,  FALSE, FALSE }},
	{ affect::type::pass_door,          { TRUE,  TRUE,  TRUE  }},
	{ affect::type::plague,             { FALSE, FALSE, FALSE }},
	{ affect::type::poison,             { FALSE, FALSE, FALSE }},
	{ affect::type::protection_evil,    { TRUE,  TRUE,  TRUE  }},
	{ affect::type::protection_good,    { TRUE,  TRUE,  TRUE  }},
    { affect::type::rayban,             { TRUE,  TRUE,  TRUE  }},
	{ affect::type::regeneration,       { TRUE,  TRUE,  TRUE  }},
	{ affect::type::sanctuary,          { TRUE,  TRUE,  TRUE  }},
	{ affect::type::sheen,              { TRUE,  FALSE, FALSE }},
	{ affect::type::shield,             { TRUE,  TRUE,  TRUE  }},
	{ affect::type::sleep,              { TRUE,  TRUE,  FALSE }},
	{ affect::type::slow,               { TRUE,  FALSE, FALSE }},
	{ affect::type::smokescreen,        { TRUE,  FALSE, TRUE  }},
	{ affect::type::steel_mist,         { TRUE,  TRUE,  TRUE  }},
	{ affect::type::stone_skin,         { TRUE,  TRUE,  TRUE  }},
	{ affect::type::talon,              { TRUE,  TRUE,  TRUE  }},
	{ affect::type::weaken,             { TRUE,  FALSE, FALSE }}
};

/* saving throw based on level only */
bool level_save(int dis_level, int save_level)
{
	int save;
	save = 50 + ((save_level - dis_level) * 3);
	save = URANGE(5, save, 95);
	return chance(save);
}

/* Compute a saving throw.  Negative apply's make saving throw better. */
bool saves_spell(int level, Character *victim, int dam_type)
{
	int save;
	save = (victim->level - level) * 3 - (GET_ATTR_SAVES(victim) * 4 / 3);

	if (affect::exists_on_char(victim, affect::type::berserk))
		save += victim->level / 4;

	int def = GET_DEFENSE_MOD(victim, dam_type);

	if (def >= 100)
		return TRUE;

	save += 20 * def / 100; // could be negative, if vuln
	save = URANGE(5, save, 95);

	return chance(save);
} /* end saves_spell */

struct dispel_params {
	void *target;
	int level;
	affect::type type;
	bool save;
	int count;
};

int affect_fn_dispel_obj(affect::Affect *node, void *data) {
	struct dispel_params *params = (struct dispel_params *)data;

	if (node->type != params->type)
		return 0;

	if (node->permanent)
		return 0;

	int dis_level = params->level;
//	Object *obj = (Object *)params->target;

	if (node->duration == -1)
		dis_level -= 3;

	if (dis_level >= MAX_LEVEL
	 || (!params->save && !level_save(dis_level, node->level))) {
		node->mark = TRUE;
		params->count++;
	}
	else
		node->level -= number_range(1,3);

	return 0;
}

int affect_fn_dispel_char(affect::Affect *node, void *data) {
	struct dispel_params *params = (struct dispel_params *)data;

	if (node->type != params->type)
		return 0;

	if (node->permanent)
		return 0;

	int dis_level = params->level;
	Character *victim = (Character *)params->target;

	if (node->duration == -1)
		dis_level -= 3;

	if (dis_level >= MAX_LEVEL
	 || (params->save && !saves_spell(dis_level, victim, DAM_OTHER))
	 || (!params->save && !level_save(dis_level, node->level))) {
		node->mark = TRUE;
		params->count++;
	}
	else
		node->level -= number_range(1,3);

	return 0;
}

/* co-routine for dispel magic and cancellation */
bool check_dispel_obj(int dis_level, Object *obj, affect::type type, bool save)
{
	struct dispel_params params = {
		.target = obj,
		.level  = dis_level,
		.type   = type,
		.save   = save,
		.count  = 0
	};

	// check save, mark for deletion
	affect::iterate_over_obj(obj, affect_fn_dispel_obj, &params);

	if (params.count == 0)
		return FALSE;

	affect::remove_marked_from_obj(obj);
	return TRUE;
}

// try to remove all of a single affect from a character
bool check_dispel_char(int dis_level, Character *victim, affect::type type, bool save)
{
	struct dispel_params params = {
		.target = victim,
		.level  = dis_level,
		.type   = type,
		.save   = save,
		.count  = 0
	};

	// check save, mark for deletion
	affect::iterate_over_char(victim, affect_fn_dispel_char, &params);

	if (params.count == 0)
		return FALSE;

	affect::remove_marked_from_char(victim);

	if (!affect::exists_on_char(victim, type)) {
		const auto& entry = affect::lookup(type);

		if (!entry.msg_room.empty())
			act(entry.msg_room, victim, nullptr, nullptr, TO_ROOM);

		if (!entry.msg_off.empty())
			ptc(victim, "%s\n", entry.msg_off);
	}

	return TRUE;
}

// dispel a single spell with undo spell
bool undo_spell(int dis_level, Character *victim, affect::type type, bool save) {
	const auto& pair = dispel_table.find(type);

	if (pair != dispel_table.cend()
	 && pair->second.can_undo)
		return check_dispel_char(dis_level, victim, type, save);

	return FALSE;
}

// dispel a list of spells with dispel magic or cancellation
bool dispel_char(Character *victim, int level, bool cancellation)
{
	bool found = FALSE;

	for (const auto& entry : dispel_table) {
		if (cancellation && !entry.second.can_cancel)
			continue;

		if (!cancellation && !entry.second.can_dispel)
			continue;

		if (check_dispel_char(level, victim, entry.first, !cancellation))
			found = TRUE;
	}

	return found;
}
