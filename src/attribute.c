#include "merc.h"
#include "tables.h"
#include "affect.h"

// temporary file to hold attribute accessors


// TODO: temporary hack to convert a bit to its index, come up with something better
int flag_to_index(unsigned long bit) {
	if (bit == 0)
		return -1;

	int index = 0;
	for (index = 0; !IS_SET(bit, A); index++)
		bit >>= 1;

	return index;
}

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
			bugf("stat_to_attr: invalid stat %d", stat);
	}

	return APPLY_STR;
}

/* command for retrieving stats */
int get_max_stat(CHAR_DATA *ch, int stat)
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
int get_age(CHAR_DATA *ch)
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
int get_unspelled_hitroll(CHAR_DATA *ch)
{
	int sum = GET_ATTR_HITROLL(ch) - GET_ATTR_MOD(ch, APPLY_HITROLL);

	for (OBJ_DATA *obj = ch->carrying; obj; obj = obj->next_content)
		if (obj->wear_loc != WEAR_NONE)
			for (const AFFECT_DATA *paf = affect_list_obj(obj); paf; paf = paf->next)
				if (paf->location == APPLY_HITROLL)
					sum += paf->modifier;

	return sum;
}

int get_unspelled_damroll(CHAR_DATA *ch)
{
	int sum = GET_ATTR_DAMROLL(ch) - GET_ATTR_MOD(ch, APPLY_DAMROLL);

	for (OBJ_DATA *obj = ch->carrying; obj; obj = obj->next_content)
		if (obj->wear_loc != WEAR_NONE)
			for (const AFFECT_DATA *paf = affect_list_obj(obj); paf; paf = paf->next)
				if (paf->location == APPLY_DAMROLL)
					sum += paf->modifier;

	return sum;
}

/* return ac value of a the character's armor only, no dex, no spells */
int get_unspelled_ac(CHAR_DATA *ch, int type)
{
	OBJ_DATA *obj;
	int ac = 100, loc;

	for (loc = 0; loc < MAX_WEAR; loc++)
		if ((obj = get_eq_char(ch, loc)) != NULL)
			ac -= apply_ac(obj, loc, type);

	return ac;
}

void attribute_check(CHAR_DATA *ch) {
	/* Check for weapon wielding.  Guard against recursion (for weapons with affects). */
	OBJ_DATA *weapon;

	if (ch != NULL
	 && ch->in_room != NULL
	 && (weapon = get_eq_char(ch, WEAR_WIELD)) != NULL
	 && get_obj_weight(weapon) > (str_app[GET_ATTR_STR(ch)].wield * 10)) {

		// only do this if they have a strength reducing spell affect (not from EQ)
		bool found = FALSE;
		for (const AFFECT_DATA *paf = affect_list_char(ch); paf; paf = paf->next)
			if (paf->where == TO_AFFECTS && paf->location == APPLY_STR && paf->modifier < 0) {
				found = TRUE;
				break;
			}

		if (found) {
			act("You drop $p.", ch, weapon, NULL, TO_CHAR);
			act("$n drops $p.", ch, weapon, NULL, TO_ROOM);
			obj_from_char(weapon);
			obj_to_room(weapon, ch->in_room);
		}
	}
}

char *print_defense_modifiers(CHAR_DATA *ch, int where) {
	static char buf[MSL];
	buf[0] = '\0';

	if (ch->defense_mod == NULL)
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
				bugf("print_defense_modifiers: unknown where %d", where);
		}

		if (print) {
			if (buf[0] != '\0')
				strcat(buf, " ");

			strcat(buf, dam_type_name(i));

			if (where != TO_IMMUNE) {
				char mbuf[100];
				sprintf(mbuf, "(%+d%%)",
					where == TO_ABSORB ?  ch->defense_mod[i]-100 : // percent beyond immune
					where == TO_RESIST ? -ch->defense_mod[i] : // prints resist as a negative
					                    -ch->defense_mod[i] // prints vuln as a positive
				);
				strcat(buf, mbuf);
			}
		}
	}

	return buf;
}
