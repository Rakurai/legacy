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

static const std::map<affect::Type, const dispel_type> dispel_table = {
	{ affect::age,                { TRUE,  FALSE, FALSE, "$n looks younger."                             }},
	{ affect::armor,              { TRUE,  TRUE,  TRUE,  ""                                              }},
	{ affect::barrier,            { TRUE,  FALSE, FALSE, ""                                              }},
	{ affect::bless,              { TRUE,  TRUE,  TRUE,  ""                                              }},
	{ affect::blindness,          { FALSE, FALSE, FALSE, "$n is no longer blinded."                      }},
	{ affect::blood_moon,         { TRUE,  TRUE,  TRUE,  ""                                              }},
	{ affect::bone_wall,          { TRUE,  FALSE, FALSE, ""                                              }},
	{ affect::calm,               { TRUE,  FALSE, TRUE,  "$n no longer looks so peaceful..."             }},
	{ affect::change_sex,         { TRUE,  FALSE, FALSE, "$n looks more like $mself again."              }},
	{ affect::charm_person,       { TRUE,  FALSE, TRUE,  "$n regains $s free will."                      }},
	{ affect::change_sex,         { TRUE,  FALSE, FALSE, ""                                              }},
	{ affect::chill_touch,        { TRUE,  FALSE, FALSE, "$n looks warmer."                              }},
	{ affect::curse,              { FALSE, FALSE, FALSE, "$n looks more relaxed."                        }},
	{ affect::detect_evil,        { TRUE,  TRUE,  TRUE,  ""                                              }},
	{ affect::detect_good,        { TRUE,  TRUE,  TRUE,  ""                                              }},
	{ affect::detect_hidden,      { TRUE,  TRUE,  TRUE,  ""                                              }},
	{ affect::detect_invis,       { TRUE,  TRUE,  TRUE,  ""                                              }},
	{ affect::detect_magic,       { TRUE,  TRUE,  TRUE,  ""                                              }},
	{ affect::divine_regeneration,{ TRUE,  TRUE,  TRUE,  ""                                              }},
	{ affect::faerie_fire,        { TRUE,  TRUE,  FALSE, "$n's outline fades."                           }},
	{ affect::fear,               { TRUE,  FALSE, FALSE, "$n looks less panicked."                       }},
	{ affect::flameshield,        { TRUE,  TRUE,  TRUE,  "The flames around $n fade away."               }},
	{ affect::fly,                { TRUE,  TRUE,  TRUE,  "$n falls to the ground!"                       }},
	{ affect::focus,              { TRUE,  FALSE, FALSE, ""                                              }},
	{ affect::force_shield,       { TRUE,  FALSE, FALSE, ""                                              }},
	{ affect::frenzy,             { TRUE,  TRUE,  TRUE,  "$n no longer looks so wild."                   }},
	{ affect::giant_strength,     { TRUE,  TRUE,  TRUE,  "$n no longer looks so mighty."                 }},
	{ affect::haste,              { TRUE,  TRUE,  TRUE,  "$n is no longer moving so quickly."            }},
	{ affect::hex,                { TRUE,  FALSE, FALSE, ""                                              }},
	{ affect::night_vision,       { TRUE,  TRUE,  TRUE,  ""                                              }},
	{ affect::invis,              { TRUE,  TRUE,  TRUE,  "$n fades into existance."                      }},
//	{ affect::ironskin,           { TRUE,  FALSE, FALSE, ""                                              }},
	{ affect::paralyze,           { TRUE,  FALSE, FALSE, ""                                              }},
	{ affect::pass_door,          { TRUE,  TRUE,  TRUE,  "$n becomes less translucent."                  }},
	{ affect::plague,             { FALSE, FALSE, FALSE, "$n looks relieved as $s sores vanish."         }},
	{ affect::poison,             { FALSE, FALSE, FALSE, "$n looks much better."                         }},
	{ affect::protection_evil,    { TRUE,  TRUE,  TRUE,  "$n's holy aura fades."                         }},
	{ affect::protection_good,    { TRUE,  TRUE,  TRUE,  "$n's unholy aura fades."                       }},
    { affect::rayban,             { TRUE,  TRUE,  TRUE,  "$n blinks as $s eye protection fades."         }},
	{ affect::sanctuary,          { TRUE,  TRUE,  TRUE,  "The white aura around $n's body vanishes."     }},
	{ affect::sheen,              { TRUE,  FALSE, FALSE, ""                                              }},
	{ affect::shield,             { TRUE,  TRUE,  TRUE,  "The shield protecting $n vanishes."            }},
	{ affect::sleep,              { TRUE,  TRUE,  FALSE, ""                                              }},
	{ affect::slow,               { TRUE,  FALSE, FALSE, "$n is no longer moving so slowly."             }},
	{ affect::smokescreen,        { TRUE,  FALSE, TRUE,  ""                                              }},
	{ affect::steel_mist,         { TRUE,  TRUE,  TRUE,  ""                                              }},
	{ affect::stone_skin,         { TRUE,  TRUE,  TRUE,  "$n's skin regains it's normal texture."        }},
	{ affect::talon,              { TRUE,  TRUE,  TRUE,  ""                                              }},
	{ affect::weaken,             { TRUE,  FALSE, FALSE, "$n looks stronger."                            }}
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

	if (affect::exists_on_char(victim, affect::berserk))
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
	affect::Type type;
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
bool check_dispel_obj(int dis_level, Object *obj, affect::Type type, bool save)
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
bool check_dispel_char(int dis_level, Character *victim, affect::Type type, bool save)
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
		String message = dispel_table.find(type)->second.msg_to_room;

		if (!message.empty())
			act(message, victim, nullptr, nullptr, TO_ROOM);

		message = affect::lookup(type).msg_off;

		if (!message.empty())
			ptc(victim, "%s\n", message);
	}

	return TRUE;
}

// dispel a single spell with undo spell
bool undo_spell(int dis_level, Character *victim, affect::Type type, bool save) {
	if (dispel_table.find(type)->second.can_undo)
		return check_dispel_char(dis_level, victim, type, save);

	return FALSE;
}

// dispel a list of spells with dispel magic or cancellation
bool dispel_char(Character *victim, int level, bool cancellation)
{
	bool found = FALSE;

	for (auto entry : dispel_table) {
		if (cancellation && !entry.second.can_cancel)
			continue;

		if (!cancellation && !entry.second.can_dispel)
			continue;

		if (check_dispel_char(level, victim, entry.first, !cancellation))
			found = TRUE;
	}

	return found;
}
