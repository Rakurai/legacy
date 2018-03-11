#include "act.hh"
#include "affect/Affect.hh"
#include "Character.hh"
#include "declare.hh"
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
	{ affect::type::age,                { true,  false, false }},
	{ affect::type::armor,              { true,  true,  true  }},
	{ affect::type::barrier,            { true,  false, false }},
	{ affect::type::bless,              { true,  true,  true  }},
	{ affect::type::blindness,          { false, false, false }},
	{ affect::type::blood_moon,         { true,  true,  true  }},
	{ affect::type::bone_wall,          { true,  false, false }},
	{ affect::type::calm,               { true,  false, true  }},
	{ affect::type::change_sex,         { true,  false, false }},
	{ affect::type::charm_person,       { true,  false, true  }},
	{ affect::type::change_sex,         { true,  false, false }},
	{ affect::type::chill_touch,        { true,  false, false }},
	{ affect::type::curse,              { false, false, false }},
	{ affect::type::detect_evil,        { true,  true,  true  }},
	{ affect::type::detect_good,        { true,  true,  true  }},
	{ affect::type::detect_hidden,      { true,  true,  true  }},
	{ affect::type::detect_invis,       { true,  true,  true  }},
	{ affect::type::detect_magic,       { true,  true,  true  }},
	{ affect::type::divine_regeneration,{ true,  true,  true  }},
	{ affect::type::faerie_fire,        { true,  true,  false }},
	{ affect::type::fear,               { true,  false, false }},
	{ affect::type::flameshield,        { true,  true,  true  }},
	{ affect::type::fly,                { true,  true,  true  }},
	{ affect::type::focus,              { true,  false, false }},
	{ affect::type::force_shield,       { true,  false, false }},
	{ affect::type::frenzy,             { true,  true,  true  }},
	{ affect::type::giant_strength,     { true,  true,  true  }},
	{ affect::type::haste,              { true,  true,  true  }},
	{ affect::type::hex,                { true,  false, false }},
	{ affect::type::night_vision,       { true,  true,  true  }},
	{ affect::type::invis,              { true,  true,  true  }},
//	{ affect::type::ironskin,           { true,  false, false }},
	{ affect::type::paralyze,           { true,  false, false }},
	{ affect::type::pass_door,          { true,  true,  true  }},
	{ affect::type::plague,             { false, false, false }},
	{ affect::type::poison,             { false, false, false }},
	{ affect::type::protection_evil,    { true,  true,  true  }},
	{ affect::type::protection_good,    { true,  true,  true  }},
    { affect::type::rayban,             { true,  true,  true  }},
	{ affect::type::regeneration,       { true,  true,  true  }},
	{ affect::type::sanctuary,          { true,  true,  true  }},
	{ affect::type::sheen,              { true,  false, false }},
	{ affect::type::shield,             { true,  true,  true  }},
	{ affect::type::sleep,              { true,  true,  false }},
	{ affect::type::slow,               { true,  false, false }},
	{ affect::type::smokescreen,        { true,  false, true  }},
	{ affect::type::steel_mist,         { true,  true,  true  }},
	{ affect::type::stone_skin,         { true,  true,  true  }},
	{ affect::type::talon,              { true,  true,  true  }},
	{ affect::type::weaken,             { true,  false, false }}
};

/* saving throw based on level only */
bool level_save(int dis_level, int save_level)
{
	int save;
	save = 20 + ((save_level - dis_level) / 2);
	save = URANGE(5, save, 95);
	return roll_chance(save);
}

/* Compute a saving throw.  Negative apply's make saving throw better. */
bool saves_spell(int level, Character *victim, int dam_type)
{
	int save = 20; // base chance to save
	save += (victim->level - level) / 2; // level difference
	save -= GET_ATTR_SAVES(victim); // negative number, probably

	if (affect::exists_on_char(victim, affect::type::berserk))
		save += save / 4; // + 25%

	int def = GET_DEFENSE_MOD(victim, dam_type);

	if (def >= 100)
		return true;

	save += 40 * def / 100; // could be negative, if vuln
	save = URANGE(5, save, 95);

	return roll_chance(save);
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
		node->mark = true;
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
		node->mark = true;
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
		return false;

	affect::remove_marked_from_obj(obj);
	return true;
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
		return false;

	affect::remove_marked_from_char(victim);

	if (!affect::exists_on_char(victim, type)) {
		const auto& entry = affect::lookup(type);

		if (!entry.msg_room.empty())
			act(entry.msg_room, victim, nullptr, nullptr, TO_ROOM);

		if (!entry.msg_off.empty())
			ptc(victim, "%s\n", entry.msg_off);
	}

	return true;
}

// dispel a single spell with undo spell
bool undo_spell(int dis_level, Character *victim, affect::type type, bool save) {
	const auto& pair = dispel_table.find(type);

	if (pair != dispel_table.cend()
	 && pair->second.can_undo)
		return check_dispel_char(dis_level, victim, type, save);

	return false;
}

// dispel a list of spells with dispel magic or cancellation
bool dispel_char(Character *victim, int level, bool cancellation)
{
	bool found = false;

	for (const auto& entry : dispel_table) {
		if (cancellation && !entry.second.can_cancel)
			continue;

		if (!cancellation && !entry.second.can_dispel)
			continue;

		if (check_dispel_char(level, victim, entry.first, !cancellation))
			found = true;
	}

	return found;
}
