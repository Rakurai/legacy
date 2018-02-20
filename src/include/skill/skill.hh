#pragma once

#include "skill/Type.hh"
#include "String.hh"
#include "declare.hh"
#include "magic.hh"


namespace skill {

/*
 * Skills include spells as a particular case.
 */
struct  skill_table_t
{
	String      name;                   /* Name of skill                */
	int      skill_level[MAX_CLASS]; /* Level needed by class        */
	int      rating[MAX_CLASS];      /* How hard it is to learn      */
	SPELL_FUN * spell_fun;              /* Spell pointer (for spells)   */
	int      target;                 /* Legal targets                */
	int      minimum_position;       /* Position for caster / user   */
	int      min_mana;               /* Minimum mana used            */
	int      beats;                  /* Waiting time after use       */
	String      noun_damage;            /* Damage message               */
	int         remort_class;           /* Required remort level or 0   */
	int		evocost_sec[MAX_CLASS];	/* Class cost to evolve to 2    */
	int		evocost_pri[MAX_CLASS];	/* Class cost to evolve to 3    */
};

type lookup(const String& name);
const skill_table_t& lookup(type t);
int num_skills();
type from_int(int);

} // namespace skill

extern const std::map<skill::type, skill::skill_table_t> skill_table;
