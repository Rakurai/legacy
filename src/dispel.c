#include "merc.h"
#include "affect.h"

struct dispel_type {
	sh_int *sn;
	bool can_cancel; // can cancel on self
	bool can_dispel; // can dispel on others with "dispel magic"
	char *msg_to_room;
};

// note: the table includes spells like 'plague' that cannot be dispelled or cancelled, but
// the 'cure *' routines use this table as well.  anything *NOT* in this table cannot be removed
// by any cure, undo, dispel, cancel, etc. (only time).  breath effects and dirt kicking, for example

const struct dispel_type dispel_table[] = {
	{ &gsn_armor,              TRUE,  TRUE,  NULL                                            },
	{ &gsn_bless,              TRUE,  TRUE,  NULL                                            },
	{ &gsn_blindness,          FALSE, FALSE, "$n is no longer blinded."                      },
	{ &gsn_blood_moon,         TRUE,  TRUE,  NULL                                            },
	{ &gsn_calm,               TRUE,  TRUE,  "$n no longer looks so peaceful..."             },
	{ &gsn_change_sex,         FALSE, FALSE, "$n looks more like $mself again."              },
	{ &gsn_charm_person,       FALSE, TRUE,  "$n regains $s free will."                      },
	{ &gsn_chill_touch,        FALSE, FALSE, "$n looks warmer."                              },
	{ &gsn_curse,              FALSE, FALSE, "$n looks more relaxed."                        },
	{ &gsn_fear,               FALSE, FALSE, "$n looks less panicked."                       },
	{ &gsn_detect_evil,        TRUE,  TRUE,  NULL                                            },
	{ &gsn_detect_good,        TRUE,  TRUE,  NULL                                            },
	{ &gsn_detect_hidden,      TRUE,  TRUE,  NULL                                            },
	{ &gsn_detect_invis,       TRUE,  TRUE,  NULL                                            },
	{ &gsn_detect_magic,       TRUE,  TRUE,  NULL                                            },
	{ &gsn_faerie_fire,        TRUE,  FALSE, "$n's outline fades."                           },
	{ &gsn_flameshield,        TRUE,  TRUE,  "The flames around $n fade away."               },
	{ &gsn_fly,                TRUE,  TRUE,  "$n falls to the ground!"                       },
	{ &gsn_frenzy,             TRUE,  TRUE,  "$n no longer looks so wild."                   },
	{ &gsn_giant_strength,     TRUE,  TRUE,  "$n no longer looks so mighty."                 },
	{ &gsn_haste,              TRUE,  TRUE,  "$n is no longer moving so quickly."            },
	{ &gsn_infravision,        TRUE,  TRUE,  NULL                                            },
	{ &gsn_invis,              TRUE,  TRUE,  "$n fades into existance."                      },
	{ &gsn_mass_invis,         TRUE,  TRUE,  "$n fades into existance."                      },
	{ &gsn_pass_door,          TRUE,  TRUE,  "$n becomes less translucent."                  },
	{ &gsn_poison,             FALSE, FALSE, "$n looks much better."                         },
	{ &gsn_plague,             FALSE, FALSE, "$n looks relieved as $s sores vanish."         },
	{ &gsn_protection_evil,    TRUE,  TRUE,  "$n's holy aura fades."                         },
	{ &gsn_protection_good,    TRUE,  TRUE,  "$n's unholy aura fades."                       },
	{ &gsn_sanctuary,          TRUE,  TRUE,  "The white aura around $n's body vanishes."     },
	{ &gsn_shield,             TRUE,  TRUE,  "The shield protecting $n vanishes."            },
	{ &gsn_sleep,              TRUE,  FALSE, NULL                                            },
	{ &gsn_slow,               FALSE, FALSE, "$n is no longer moving so slowly."             },
	{ &gsn_smokescreen,        FALSE, TRUE,  NULL                                            },
	{ &gsn_stone_skin,         TRUE,  TRUE,  "$n's skin regains it's normal texture."        },
	{ &gsn_weaken,             FALSE, FALSE, "$n looks stronger."                            },
	{ NULL,                    TRUE,  TRUE,  NULL                                            }
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
bool saves_spell(int level, CHAR_DATA *victim, int dam_type)
{
	int save;
	save = (victim->level - level) * 3 - (GET_ATTR(victim, APPLY_SAVES) * 4 / 3);

	if (affect_find_in_char(victim, gsn_berserk))
		save += victim->level / 4;

	int def = check_immune(victim, dam_type);

	if (def >= 100)
		return TRUE;

	save += 20 * def / 100; // could be negative, if vuln
	save = URANGE(5, save, 95);

	return chance(save);
} /* end saves_spell */

struct dispel_params {
	void *target;
	int level;
	int sn;
	bool save;
	int count;
};

int affect_fn_dispel_obj(AFFECT_DATA *node, void *data) {
	struct dispel_params *params = (struct dispel_params *)data;

	if (node->type != params->sn)
		return 0;

	if (node->permanent)
		return 0;

	int dis_level = params->level;
//	OBJ_DATA *obj = (OBJ_DATA *)params->target;

	if (node->duration == -1)
		dis_level -= 3;

	if (dis_level >= MAX_LEVEL
	 || (!params->save && !level_save(dis_level, node->level))) {
		node->mark = TRUE;
		params->count++;
	}
	else
		node->level--;

	return 0;
}

int affect_fn_dispel_char(AFFECT_DATA *node, void *data) {
	struct dispel_params *params = (struct dispel_params *)data;

	if (node->type != params->sn)
		return 0;

	if (node->permanent)
		return 0;

	int dis_level = params->level;
	CHAR_DATA *victim = (CHAR_DATA *)params->target;

	if (node->duration == -1)
		dis_level -= 3;

	if (dis_level >= MAX_LEVEL
	 || (params->save && !saves_spell(dis_level, victim, DAM_OTHER))
	 || (!params->save && !level_save(dis_level, node->level))) {
		node->mark = TRUE;
		params->count++;
	}
	else
		node->level--;

	return 0;
}

/* co-routine for dispel magic and cancellation */
bool check_dispel_obj(int dis_level, OBJ_DATA *obj, int sn, bool save)
{
	struct dispel_params params = {
		.target = obj,
		.level  = dis_level,
		.sn     = sn,
		.save   = save,
		.count  = 0
	};

	// check save, mark for deletion
	affect_iterate_over_obj(obj, affect_fn_dispel_obj, &params);

	if (params.count == 0)
		return FALSE;

	affect_remove_marked_from_obj(obj);
	return TRUE;
}

/* co-routine for dispel magic and cancellation */
bool check_dispel_char(int dis_level, CHAR_DATA *victim, int sn, bool save)
{
	struct dispel_params params = {
		.target = victim,
		.level  = dis_level,
		.sn     = sn,
		.save   = save,
		.count  = 0
	};

	// check save, mark for deletion
	affect_iterate_over_char(victim, affect_fn_dispel_char, &params);

	if (params.count == 0)
		return FALSE;

	affect_remove_marked_from_char(victim);

	if (!affect_find_in_char(victim, sn)) {
		for (int i = 0; dispel_table[i].sn != NULL; i++) {
			if (*dispel_table[i].sn == sn) {
				if (dispel_table[i].msg_to_room != NULL)
					act(dispel_table[i].msg_to_room, victim, NULL, NULL, TO_ROOM);

				break;
			}
		}

		if (skill_table[sn].msg_off)
			ptc(victim, "%s\n", skill_table[sn].msg_off);
	}

	return TRUE;
}

bool dispel_char(CHAR_DATA *victim, int level, bool cancellation)
{
	bool found = FALSE;

	for (int i = 0; dispel_table[i].sn != NULL; i++) {
		if (cancellation && !dispel_table[i].can_cancel)
			continue;

		if (!cancellation && !dispel_table[i].can_dispel)
			continue;

		if (check_dispel_char(level, victim, *dispel_table[i].sn, FALSE))
			found = TRUE;
	}

	return found;
}
