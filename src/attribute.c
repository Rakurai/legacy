#include "merc.h"
#include "tables.h"

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
int get_curr_stat(CHAR_DATA *ch, int stat)
{
	int max = 25;

	if (IS_NPC(ch))
		max = ATTR_BASE(ch, stat_to_attr(stat)) + 4;
	else if (!IS_IMMORTAL(ch)) {
		max = pc_race_table[ch->race].max_stats[stat] + 4;

		if (class_table[ch->class].stat_prime == stat)
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

	return URANGE(3, GET_ATTR(ch, stat_to_attr(stat)), UMIN(max, 25));
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

int get_max_hit(CHAR_DATA *ch) {
	return URANGE(1, ATTR_BASE(ch, APPLY_HIT) + GET_ATTR_MOD(ch, APPLY_HIT), 30000);
}
int get_max_mana(CHAR_DATA *ch) {
	return URANGE(1, ATTR_BASE(ch, APPLY_MANA) + GET_ATTR_MOD(ch, APPLY_MANA), 30000);
}
int get_max_stam(CHAR_DATA *ch) {
	return URANGE(1, ATTR_BASE(ch, APPLY_STAM) + GET_ATTR_MOD(ch, APPLY_STAM), 30000);
}

/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */

int check_immune(CHAR_DATA *ch, int dam_type)
{
	int def = 0;
	int bit;

	if (dam_type == DAM_NONE)
		return 0;

	if (ch->defense_mod == NULL) // no modifiers
		return 0;

	if (dam_type <= 3) // weapon attack
		def = ch->defense_mod[flag_to_index(IMM_WEAPON)];
	else if (dam_type != DAM_WEAPON) /* magical attack */
		def = ch->defense_mod[flag_to_index(IMM_MAGIC)];

	// stop here for simple types
	if (dam_type == DAM_WEAPON || dam_type == DAM_MAGIC)
		return def;

	/* set bits to check -- VULN etc. must ALL be the same or this will fail */
	switch (dam_type) {
	case (DAM_BASH):         bit = IMM_BASH;         break;
	case (DAM_PIERCE):       bit = IMM_PIERCE;       break;
	case (DAM_SLASH):        bit = IMM_SLASH;        break;
	case (DAM_FIRE):         bit = IMM_FIRE;         break;
	case (DAM_COLD):         bit = IMM_COLD;         break;
	case (DAM_ELECTRICITY):  bit = IMM_ELECTRICITY;  break;
	case (DAM_ACID):         bit = IMM_ACID;         break;
	case (DAM_POISON):       bit = IMM_POISON;       break;
	case (DAM_NEGATIVE):     bit = IMM_NEGATIVE;     break;
	case (DAM_HOLY):         bit = IMM_HOLY;         break;
	case (DAM_ENERGY):       bit = IMM_ENERGY;       break;
	case (DAM_MENTAL):       bit = IMM_MENTAL;       break;
	case (DAM_DISEASE):      bit = IMM_DISEASE;      break;
	case (DAM_DROWNING):     bit = IMM_DROWNING;     break;
	case (DAM_LIGHT):        bit = IMM_LIGHT;        break;
	case (DAM_CHARM):        bit = IMM_CHARM;        break;
	case (DAM_SOUND):        bit = IMM_SOUND;        break;
	default:                return def;
	}

	def += ch->defense_mod[flag_to_index(bit)];
	return def;
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
			case TO_RESIST:  if (ch->defense_mod[i] > 0)    print = TRUE; break;
			case TO_VULN:    if (ch->defense_mod[i] < 0)    print = TRUE; break;
			default:
				bugf("print_defense_modifiers: unknown where %d", where);
		}

		if (print) {
			if (buf[0] != '\0')
				strcat(buf, " ");

			strcat(buf, imm_flags[i].name);

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
