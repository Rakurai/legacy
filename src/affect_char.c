#include "merc.h"
#include "affect.h"
#include "affect_list.h"
#include "recycle.h"
#include "tables.h"

// local declarations
void affect_modify_char(void *owner, const AFFECT_DATA *paf, bool fAdd);

// flags

void affect_flag_add_to_char(CHAR_DATA *ch, unsigned int flag) {
	SET_BIT(ch->affected_by, flag);
}

void affect_flag_remove_from_char(CHAR_DATA *ch, unsigned int flag) {
	REMOVE_BIT(ch->affected_by, flag);
}

unsigned int affect_flag_get_char(CHAR_DATA *ch) {
	return ch->affected_by;
}

void affect_flag_clear_char(CHAR_DATA *ch) {
	ch->affected_by = 0;
}

bool affect_flag_on_char(CHAR_DATA *ch, unsigned int flag) {
	return IS_SET(ch->affected_by, flag);
}

// searching

const AFFECT_DATA *affect_list_char(CHAR_DATA *ch) {
	return ch->affected;
}

const AFFECT_DATA *affect_find_in_char(CHAR_DATA *ch, int sn) {
	return affect_find_in_list(&ch->affected, sn);
}

// adding

void affect_copy_to_char(CHAR_DATA *ch, const AFFECT_DATA *template)
{
	affect_copy_to_list(&ch->affected, template);
	affect_modify_char(ch, template, TRUE);
}

void affect_join_to_char(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	affect_dedup_in_list(&ch->affected, paf);
	affect_modify_char(ch, paf, FALSE);
	affect_copy_to_char(ch, paf);
}

void affect_add_perm_to_char(CHAR_DATA *ch, int sn) {
	AFFECT_DATA af = (AFFECT_DATA){0};
	af.type = sn;
	af.where = TO_AFFECTS;
	af.level = -1;
	af.duration = -1;
	af.evolution = 1;

	affect_copy_to_char(ch, &af);
}

// removing

void affect_remove_from_char(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	affect_remove_from_list(&ch->affected, paf);
	affect_modify_char(ch, paf, FALSE);
	free_affect(paf);
}

void affect_remove_matching_from_char(CHAR_DATA *ch, affect_comparator comp, const AFFECT_DATA *pattern) {
	affect_fn_params params;

	params.owner = ch;
	params.modifier = affect_modify_char;
	params.data = NULL;

	affect_remove_matching_from_list(&ch->affected, comp, pattern, &params);
}

void affect_remove_marked_from_char(CHAR_DATA *ch) {
	AFFECT_DATA pattern;
	pattern.mark = TRUE;

	affect_remove_matching_from_char(ch, affect_comparator_mark, &pattern);
}

void affect_remove_sn_from_char(CHAR_DATA *ch, int sn) {
	AFFECT_DATA pattern;
	pattern.type = sn;

	affect_remove_matching_from_char(ch, affect_comparator_type, &pattern);
}

void affect_remove_all_from_char(CHAR_DATA *ch) {
	affect_remove_matching_from_char(ch, NULL, NULL);
}

// modifying

void affect_iterate_over_char(CHAR_DATA *ch, affect_fn fn, void *data) {
	affect_fn_params params;

	params.owner = ch;
	params.modifier = affect_modify_char;
	params.data = data;

	affect_iterate_over_list(&ch->affected, fn, &params);
}

void affect_sort_char(CHAR_DATA *ch, affect_comparator comp) {
	affect_sort_list(&ch->affected, comp);
}

// utility

// the modify function is called any time there is a potential change to the list of
// affects, and here we update any caches or entities that depend on the affect list.
// it is important that owner->affected reflects the new state of the affects, i.e.
// the affect has already been inserted or removed, and paf is not a member of the set.
void affect_modify_char(void *owner, const AFFECT_DATA *paf, bool fAdd) {
	CHAR_DATA *ch = (CHAR_DATA *)owner;
	OBJ_DATA *obj;
	int mod, i;
	mod = paf->modifier;

	if (fAdd) {
		switch (paf->where) {
		case TO_AFFECTS:        SET_BIT(ch->affected_by, paf->bitvector);       break;

		case TO_ABSORB:          SET_BIT(ch->absorb_flags, paf->bitvector);       break;

		case TO_IMMUNE:         SET_BIT(ch->imm_flags, paf->bitvector);         break;

		case TO_RESIST:         SET_BIT(ch->res_flags, paf->bitvector);         break;

		case TO_VULN:           SET_BIT(ch->vuln_flags, paf->bitvector);        break;
		}
	}
	else {
		/* special for removing a flag:
		   we search through their affects to see if there's another with the same.
		   if there is, don't remove it.  replaces the complicated eq_imm_flags and
		   junk, and solves the problem of removing eq and losing the flag even if
		   it's racial, permanent, or raffect                  -- Montrey */
		AFFECT_DATA *saf;
		bool found_dup = FALSE;

		/* let's start with their racial affects, there is no racial drain affect */
		if ((paf->where == TO_AFFECTS && (race_table[ch->race].aff  & paf->bitvector))
		    || (paf->where == TO_IMMUNE  && (race_table[ch->race].imm  & paf->bitvector))
		    || (paf->where == TO_RESIST  && (race_table[ch->race].res  & paf->bitvector))
		    || (paf->where == TO_VULN    && (race_table[ch->race].vuln & paf->bitvector)))
			found_dup = TRUE;

		/* ok, try their affects */
		if (!found_dup)
			for (saf = ch->affected; saf != NULL; saf = saf->next)
				if (saf != paf
				    && saf->where == paf->where
				    && saf->bitvector == paf->bitvector) {
					found_dup = TRUE;
					break;
				}

		/* no?  try their eq */
		if (!found_dup)
			for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
				if (obj->wear_loc == -1)
					continue;

				for (saf = obj->affected; saf != NULL; saf = saf->next)
					if (saf != paf
					    && saf->where == paf->where
					    && saf->bitvector == paf->bitvector) {
						found_dup = TRUE;
						break;
					}

				if (found_dup)
					break;
			}

		/* last but not least, their raffects, if applicable */
		if (!found_dup && IS_REMORT(ch) && ch->pcdata->raffect[0]
		    && (paf->where == TO_VULN || paf->where == TO_RESIST))
			for (i = 0; i <= ch->pcdata->remort_count / 10 + 1; i++)
				if ((raffects[ch->pcdata->raffect[i]].id >= 900
				     && raffects[ch->pcdata->raffect[i]].id <= 949
				     && raffects[ch->pcdata->raffect[i]].add == paf->bitvector
				     && paf->where == TO_VULN)
				    || (raffects[ch->pcdata->raffect[i]].id >= 950
				        && raffects[ch->pcdata->raffect[i]].id <= 999
				        && raffects[ch->pcdata->raffect[i]].add == paf->bitvector
				        && paf->where == TO_RESIST))
					found_dup = TRUE;

		/* if we found it, don't remove the bit, but continue on to remove modifiers */
		if (!found_dup)
			switch (paf->where) {
			case TO_AFFECTS: REMOVE_BIT(ch->affected_by, paf->bitvector);   break;

			case TO_ABSORB:   REMOVE_BIT(ch->absorb_flags, paf->bitvector);   break;

			case TO_IMMUNE:  REMOVE_BIT(ch->imm_flags, paf->bitvector);     break;

			case TO_RESIST:  REMOVE_BIT(ch->res_flags, paf->bitvector);     break;

			case TO_VULN:    REMOVE_BIT(ch->vuln_flags, paf->bitvector);    break;
			}

		mod = 0 - mod;
	}

	switch (paf->location) {
	default:                                                                break;

	case APPLY_STR:                 ch->mod_stat[STAT_STR]  += mod;         break;

	case APPLY_DEX:                 ch->mod_stat[STAT_DEX]  += mod;         break;

	case APPLY_INT:                 ch->mod_stat[STAT_INT]  += mod;         break;

	case APPLY_WIS:                 ch->mod_stat[STAT_WIS]  += mod;         break;

	case APPLY_CON:                 ch->mod_stat[STAT_CON]  += mod;         break;

	case APPLY_CHR:                 ch->mod_stat[STAT_CHR]  += mod;         break;

	case APPLY_SEX:                 ch->sex                 += mod;         break;

	case APPLY_MANA:                ch->max_mana            += mod;         break;

	case APPLY_HIT:                 ch->max_hit             += mod;         break;

	case APPLY_STAM:                ch->max_stam            += mod;         break;

	case APPLY_AC:          for (i = 0; i < 4; i++) ch->armor_m[i] += mod;    break;

	case APPLY_HITROLL:             ch->hitroll             += mod;         break;

	case APPLY_DAMROLL:             ch->damroll             += mod;         break;

	case APPLY_SAVES:               ch->saving_throw        += mod;         break;

	case APPLY_SAVING_ROD:          ch->saving_throw        += mod;         break;

	case APPLY_SAVING_PETRI:        ch->saving_throw        += mod;         break;

	case APPLY_SAVING_BREATH:       ch->saving_throw        += mod;         break;

	case APPLY_SAVING_SPELL:        ch->saving_throw        += mod;         break;
	}

	/* Check for weapon wielding.  Guard against recursion (for weapons with affects). */
	if ((obj = get_eq_char(ch, WEAR_WIELD)) != NULL
	    && get_obj_weight(obj) > (str_app[get_curr_stat(ch, STAT_STR)].wield * 10)) {
		static int depth;

		if (depth == 0) {
			depth++;
			act("You drop $p.", ch, obj, NULL, TO_CHAR);
			act("$n drops $p.", ch, obj, NULL, TO_ROOM);
			obj_from_char(obj);
			obj_to_room(obj, ch->in_room);
			depth--;
		}
	}
}
