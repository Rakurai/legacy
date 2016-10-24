#include "merc.h"

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

int affect_bit_to_sn(int bit) {
	switch (bit) {
		case AFF_BLIND: return gsn_blindness;
		case AFF_INVISIBLE: return gsn_invis;
		case AFF_DETECT_EVIL: return gsn_detect_evil;
		case AFF_DETECT_GOOD: return gsn_detect_good;
		case AFF_DETECT_INVIS: return gsn_detect_invis;
		case AFF_DETECT_MAGIC: return gsn_detect_magic;
		case AFF_DETECT_HIDDEN: return gsn_detect_hidden;
		case AFF_SANCTUARY: return gsn_sanctuary;
		case AFF_FAERIE_FIRE: return gsn_faerie_fire;
		case AFF_INFRARED: return gsn_infravision;
		case AFF_CURSE: return gsn_curse;
		case AFF_FEAR: return gsn_fear;
		case AFF_POISON: return gsn_poison;
		case AFF_PROTECT_EVIL: return gsn_prot_evil;
		case AFF_PROTECT_GOOD: return gsn_prot_good;
		case AFF_NIGHT_VISION: return gsn_night_vision;
		case AFF_SNEAK: return gsn_sneak;
		case AFF_HIDE: return gsn_hide;
		case AFF_CHARM: return gsn_charm;
		case AFF_FLYING: return gsn_flying;
		case AFF_PASS_DOOR: return gsn_pass_door;
		case AFF_BERSERK: return gsn_berserk;
		case AFF_CALM: return gsn_calm;
		case AFF_HASTE: return gsn_haste;
		case AFF_SLOW: return gsn_slow;
		case AFF_PLAGUE: return gsn_plague;
		case AFF_DIVINEREGEN: return gsn_divreg;
		case AFF_FLAMESHIELD: return gsn_flameshield;
		case AFF_REGENERATION: return gsn_regen;
		case AFF_TALON: return gsn_talon;
		case AFF_STEEL: return gsn_steel_mist;
		default:
			bugf("affect_bit_to_sn: wierd bit %d", paf->bitvector);
	}

	return -1;
}

/* command for retrieving stats */
int get_curr_stat(CHAR_DATA *ch, int stat)
{
	int max = 25;

	if (IS_NPC(ch))
		max = GET_ATTR_BASE(ch, stat_to_attr(stat)) + 4;
	else if (!IS_IMMORTAL(ch)) {
		max = pc_race_table[ch->race].max_stats[stat] + 4;

		if (class_table[ch->class].stat_prime == stat)
			max += 2;
	}

	// if the player has a familiar, they get +1 in whatever the familiar's max stat is
	if (!IS_NPC(ch) && ch->pcdata->familiar && ch->pet) {
		int max_slot = 0;

		for (int i = 1; i < MAX_STAT; i++)
			if (ch->pet->perm_stat[i] > ch->pet->perm_stat[max_slot])
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

char *print_damage_modifiers(CHAR_DATA *ch, char type) {
	static char buf[MSL];
	buf[0] = '\0';

	if (ch->damage_mod == NULL)
		return buf;

	for (int i = 1; i < 32; i++) {
		bool print = FALSE;

		switch (type) {
			case TO_ABSORB:  if (ch->damage_mod[i] > 100)  print = TRUE; break;
			case TO_IMMUNE:  if (ch->damage_mod[i] == 100) print = TRUE; break;
			case TO_RESIST:  if (ch->damage_mod[i] > 0)    print = TRUE; break;
			case TO_VULN:    if (ch->damage_mod[i] < 0)    print = TRUE; break;
			default:
				bugf("print_damage_modifiers: unknown type %d", type);
		}

		if (print) {
			if (buf[0] != '\0')
				strcat(buf, " ");

			strcat(buf, imm_flags[i].name);

			if (type != TO_IMMUNE) {
				char mbuf[100];
				sprintf(mbuf, "(%+d%%)",
					type == TO_ABSORB ?  ch->damage_mod[i]-100 : // percent beyond immune
					type == TO_RESIST ? -ch->damage_mod[i] : // prints resist as a negative
					                    -ch->damage_mod[i] : // prints vuln as a positive
				);
				strcat(buf, mbuf);
			}
		}
	}

	return buf;
}
