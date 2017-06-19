#include "act.hh"
#include "Affect.hh"
#include "Character.hh"
#include "declare.hh"
#include "Flags.hh"
#include "Format.hh"
#include "GameTime.hh"
#include "lookup.hh"
#include "Logging.hh"
#include "macros.hh"
#include "merc.hh"
#include "Object.hh"
#include "Player.hh"
#include "String.hh"
#include "typename.hh"

// temporary file to hold attribute accessors


// this is here because stats are 0-5, attributes are the old apply flags that are 1-5 and 26.
// maybe someday we can reconcile this, but it'll probably take a sweep of area files.
int stat_to_attr(int stat) {
	switch (stat) {
		case STAT_STR: return APPLY_STR;
		case STAT_DEX: return APPLY_DEX;
		case STAT_CON: return APPLY_CON;
		case STAT_WIS: return APPLY_WIS;
		case STAT_INT: return APPLY_INT;
		case STAT_CHR: return APPLY_CHR;
		default:
			Logging::bugf("stat_to_attr: invalid stat %d", stat);
	}

	return APPLY_STR;
}

/* command for retrieving stats */
int get_max_stat(const Character *ch, int stat)
{
	int max = 25;

	if (IS_NPC(ch))
		max = ATTR_BASE(ch, stat_to_attr(stat)) + 4;
	else if (!IS_IMMORTAL(ch)) {
		max = pc_race_table[ch->race].max_stats[stat] + 4;

		if (class_table[ch->cls].stat_prime == stat)
			max += 2;
	}

	// if the player has a familiar, they get +1 in whatever the familiar's max stat is
	if (!IS_NPC(ch) && ch->pcdata->familiar && ch->pet) {
		int max_slot = 0;

		for (int i = 0; i < MAX_STATS; i++)
			if (ATTR_BASE(ch->pet, stat_to_attr(i)) > ATTR_BASE(ch->pet, stat_to_attr(max_slot)))
				max_slot = i;

		if (max_slot == stat)
			max++;
	}

	if (ch->race == race_lookup("human"))
		max++;

	return URANGE(3, max, 25);
}

/* Retrieve a character's age in mud years.
   (178.5 hours = 1 year) */
int get_age(Character *ch)
{
	int age = 17;

	if (!IS_NPC(ch))
		age = pc_race_table[ch->race].base_age;

	age += get_play_seconds(ch) / (MUD_YEAR * MUD_MONTH * MUD_DAY * MUD_HOUR);
	age += GET_ATTR_MOD(ch, APPLY_AGE);
	return age;
}

/* below two functions recalculate a character's hitroll and damroll
   based solely on their strength and equipment, and not spells.
   used when finding adjustment for hammerstrike and berserk
   TODO: these may need fixing for gem affects
                                                -- Montrey */
int get_unspelled_hitroll(Character *ch)
{
	int sum = GET_ATTR_HITROLL(ch) - GET_ATTR_MOD(ch, APPLY_HITROLL);

	for (Object *obj = ch->carrying; obj; obj = obj->next_content)
		if (obj->wear_loc != WEAR_NONE)
			for (const Affect *paf = affect_list_obj(obj); paf; paf = paf->next)
				if (paf->location == APPLY_HITROLL)
					sum += paf->modifier;

	return sum;
}

int get_unspelled_damroll(Character *ch)
{
	int sum = GET_ATTR_DAMROLL(ch) - GET_ATTR_MOD(ch, APPLY_DAMROLL);

	for (Object *obj = ch->carrying; obj; obj = obj->next_content)
		if (obj->wear_loc != WEAR_NONE)
			for (const Affect *paf = affect_list_obj(obj); paf; paf = paf->next)
				if (paf->location == APPLY_DAMROLL)
					sum += paf->modifier;

	return sum;
}

/* return ac value of a the character's armor only, no dex, no spells */
int get_unspelled_ac(Character *ch, int type)
{
	Object *obj;
	int ac = 100, loc;

	for (loc = 0; loc < MAX_WEAR; loc++)
		if ((obj = get_eq_char(ch, loc)) != nullptr)
			ac -= apply_ac(obj, loc, type);

	return ac;
}

void attribute_check(Character *ch) {
	/* Check for weapon wielding.  Guard against recursion (for weapons with affects). */
	Object *weapon;

	if (ch != nullptr
	 && ch->in_room != nullptr
	 && (weapon = get_eq_char(ch, WEAR_WIELD)) != nullptr
	 && get_obj_weight(weapon) > (str_app[GET_ATTR_STR(ch)].wield * 10)) {

		// only do this if they have a strength reducing spell affect (not from EQ)
		bool found = FALSE;
		for (const Affect *paf = affect_list_char(ch); paf; paf = paf->next)
			if (paf->where == TO_AFFECTS && paf->location == APPLY_STR && paf->modifier < 0) {
				found = TRUE;
				break;
			}

		if (found) {
			act("You drop $p.", ch, weapon, nullptr, TO_CHAR);
			act("$n drops $p.", ch, weapon, nullptr, TO_ROOM);
			obj_from_char(weapon);
			obj_to_room(weapon, ch->in_room);
		}
	}
}

String print_defense_modifiers(Character *ch, int where) {
	String buf;

	if (ch->defense_mod == nullptr)
		return buf;

	for (int i = 1; i < 32; i++) {
		bool print = FALSE;

		switch (where) {
			case TO_ABSORB:  if (ch->defense_mod[i] > 100)  print = TRUE; break;
			case TO_IMMUNE:  if (ch->defense_mod[i] == 100) print = TRUE; break;
			case TO_RESIST:  if (ch->defense_mod[i] > 0 && ch->defense_mod[i] < 100)
			    print = TRUE; break;
			case TO_VULN:    if (ch->defense_mod[i] < 0)    print = TRUE; break;
			default:
				Logging::bugf("print_defense_modifiers: unknown where %d", where);
		}

		if (print) {
			if (buf[0] != '\0')
				buf += " ";

			buf += dam_type_name(i);

			if (where != TO_IMMUNE) {
				char mbuf[100];
				Format::sprintf(mbuf, "(%+d%%)",
					where == TO_ABSORB ?  ch->defense_mod[i]-100 : // percent beyond immune
					where == TO_RESIST ? -ch->defense_mod[i] : // prints resist as a negative
					                    -ch->defense_mod[i] // prints vuln as a positive
				);
				buf += mbuf;
			}
		}
	}

	return buf;
}
