#include "merc.h"
#include "affect.h"
#include "recycle.h"
#include "tables.h"
#include "affect_int.h"


AFFECT_DATA *affect_find_in_char(CHAR_DATA *ch, int sn) {
	return affect_find_in_list(ch->affected, sn);
}

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

void affect_remove_from_char(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	affect_remove_from_list(&ch->affected, paf);
	affect_modify_char(ch, paf, FALSE);
	free_affect(paf);
}

void affect_update_in_char(CHAR_DATA *ch, AFFECT_DATA *original, const AFFECT_DATA *template)
{
	affect_modify_char(ch, original, FALSE);
	affect_update(original, template);
	affect_modify_char(ch, original, TRUE);
}

void affect_remove_sn_from_char(CHAR_DATA *ch, int sn) {
	AFFECT_DATA *paf, *head = ch->affected;

	while ((paf = affect_find_in_list(head, sn)) != NULL) {
		head = paf->next; // start next iteration here instead of looping through again
		affect_remove_from_char(ch, paf);
	}
}

void affect_remove_all_from_char(CHAR_DATA *ch) {
	while (ch->affected)
		affect_remove_from_char(ch, ch->affected);
}

void affect_add_perm_to_char(CHAR_DATA *ch, int sn) {
	AFFECT_DATA af;
	af.type = sn;
	af.where = TO_AFFECTS;
	af.level = -1;
	af.duration = -1;
	af.evolution = 1;

	affect_copy_to_char(ch, &af);
}

void affect_modify_char(CHAR_DATA *ch, const AFFECT_DATA *paf, bool fAdd) {

}
