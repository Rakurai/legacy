/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

#include "merc.h"

extern AREA_DATA *area_last;

/* values for db2.c */

/* Local for Mob Programs */
void            mprog_read_programs     args((FILE *fp,
                MOB_INDEX_DATA *pMobIndex));

/*
 * Snarf a mob section.  new style
 */
void load_mobiles(FILE *fp)
{
	MOB_INDEX_DATA *pMobIndex;
	sh_int vnum;
	char letter;
	int iHash;

	for (; ;) {
		letter                          = fread_letter(fp);

		if (letter != '#') {
			bug("Load_mobiles: # not found.", 0);
			exit(1);
		}

		vnum                            = fread_number(fp);

		if (vnum == 0)
			break;

		if (vnum < area_last->min_vnum ||
		    vnum > area_last->max_vnum)
			bug("mobile vnum %d out of range.", vnum);

		fBootDb = FALSE;

		if (get_mob_index(vnum) != NULL) {
			bug("Load_mobiles: vnum %d duplicated.", vnum);
			exit(1);
		}

		fBootDb = TRUE;
		pMobIndex                       = alloc_perm(sizeof(*pMobIndex));
		pMobIndex->vnum                 = vnum;
		pMobIndex->version              = aVersion;
		pMobIndex->player_name          = fread_string(fp);
		pMobIndex->short_descr          = fread_string(fp);
		pMobIndex->long_descr           = fread_string(fp);
		pMobIndex->description          = fread_string(fp);
		pMobIndex->race                 = race_lookup(fread_string(fp));
		pMobIndex->long_descr[0]        = UPPER(pMobIndex->long_descr[0]);
		pMobIndex->description[0]       = UPPER(pMobIndex->description[0]);
		pMobIndex->act                  = fread_flag(fp) | ACT_IS_NPC
		                                  | race_table[pMobIndex->race].act;
		pMobIndex->affected_by          = fread_flag(fp)
		                                  | race_table[pMobIndex->race].aff;
		pMobIndex->pShop                = NULL;
		pMobIndex->alignment            = fread_number(fp);
		pMobIndex->group                = fread_flag(fp);
		pMobIndex->level                = fread_number(fp);
		pMobIndex->hitroll              = fread_number(fp);
		/* read hit dice */
		pMobIndex->hit[DICE_NUMBER]     = fread_number(fp);
		/* 'd'          */                fread_letter(fp);
		pMobIndex->hit[DICE_TYPE]       = fread_number(fp);
		/* '+'          */                fread_letter(fp);
		pMobIndex->hit[DICE_BONUS]      = fread_number(fp);
		/* read mana dice */
		pMobIndex->mana[DICE_NUMBER]    = fread_number(fp);
		fread_letter(fp);
		pMobIndex->mana[DICE_TYPE]      = fread_number(fp);
		fread_letter(fp);
		pMobIndex->mana[DICE_BONUS]     = fread_number(fp);
		/* read damage dice */
		pMobIndex->damage[DICE_NUMBER]  = fread_number(fp);
		fread_letter(fp);
		pMobIndex->damage[DICE_TYPE]    = fread_number(fp);
		fread_letter(fp);
		pMobIndex->damage[DICE_BONUS]   = fread_number(fp);
		pMobIndex->dam_type         = attack_lookup(fread_word(fp));
		/* read armor class */
		pMobIndex->ac[AC_PIERCE]        = fread_number(fp) * 10;
		pMobIndex->ac[AC_BASH]          = fread_number(fp) * 10;
		pMobIndex->ac[AC_SLASH]         = fread_number(fp) * 10;
		pMobIndex->ac[AC_EXOTIC]        = fread_number(fp) * 10;
		/* read flags and add in data from the race table */
		pMobIndex->off_flags            = fread_flag(fp)
		                                  | race_table[pMobIndex->race].off;
		pMobIndex->drain_flags          = 0; /* fix when we change the area versions */
		pMobIndex->imm_flags            = fread_flag(fp)
		                                  | race_table[pMobIndex->race].imm;
		pMobIndex->res_flags            = fread_flag(fp)
		                                  | race_table[pMobIndex->race].res;
		pMobIndex->vuln_flags           = fread_flag(fp)
		                                  | race_table[pMobIndex->race].vuln;
		/* vital statistics */
		pMobIndex->start_pos        = position_lookup(fread_word(fp));
		pMobIndex->default_pos      = position_lookup(fread_word(fp));
		pMobIndex->sex              = sex_lookup(fread_word(fp));

		if (pMobIndex->sex < 0) {
			bug("Load_mobiles: bad sex for vnum %d.", vnum);
			exit(1);
		}

		pMobIndex->wealth               = fread_number(fp);
		pMobIndex->form                 = fread_flag(fp)
		                                  | race_table[pMobIndex->race].form;
		pMobIndex->parts                = fread_flag(fp)
		                                  | race_table[pMobIndex->race].parts;
		/* size */
		pMobIndex->size                 = size_lookup(fread_word(fp));

		if (pMobIndex->size < 0) {
			bug("Load_mobiles: bad size for vnum %d.", vnum);
			exit(1);
		}

		pMobIndex->material             = str_dup(fread_word(fp));

		for (; ;) {
			letter = fread_letter(fp);

			if (letter == 'F') {
				char *word;
				long vector;
				word                    = fread_word(fp);
				vector                  = fread_flag(fp);

				if (!str_prefix1(word, "act"))
					REMOVE_BIT(pMobIndex->act, vector);
				else if (!str_prefix1(word, "aff"))
					REMOVE_BIT(pMobIndex->affected_by, vector);
				else if (!str_prefix1(word, "off"))
					REMOVE_BIT(pMobIndex->affected_by, vector);
				else if (!str_prefix1(word, "drn"))
					REMOVE_BIT(pMobIndex->drain_flags, vector);
				else if (!str_prefix1(word, "imm"))
					REMOVE_BIT(pMobIndex->imm_flags, vector);
				else if (!str_prefix1(word, "res"))
					REMOVE_BIT(pMobIndex->res_flags, vector);
				else if (!str_prefix1(word, "vul"))
					REMOVE_BIT(pMobIndex->vuln_flags, vector);
				else if (!str_prefix1(word, "for"))
					REMOVE_BIT(pMobIndex->form, vector);
				else if (!str_prefix1(word, "par"))
					REMOVE_BIT(pMobIndex->parts, vector);
				else {
					bug("Flag remove: flag not found.", 0);
					exit(1);
				}
			}
			else {
				ungetc(letter, fp);
				break;
			}
		}

		letter = fread_letter(fp);

		if (letter == '>') {
			ungetc(letter, fp);
			mprog_read_programs(fp, pMobIndex);
		}
		else ungetc(letter, fp);

		iHash                   = vnum % MAX_KEY_HASH;
		pMobIndex->next         = mob_index_hash[iHash];
		mob_index_hash[iHash]   = pMobIndex;
		top_mob_index++;
		kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL - 1)].number++;
	}

	return;
}

/*
 * Snarf an obj section. new style
 */
void load_objects(FILE *fp)
{
	OBJ_INDEX_DATA *pObjIndex;
	sh_int vnum;
	char letter;
	int iHash;

	for (; ;) {
		letter                          = fread_letter(fp);

		if (letter != '#') {
			bug("Load_objects: # not found.", 0);
			exit(1);
		}

		vnum                            = fread_number(fp);

		if (vnum == 0)
			break;

		if (vnum < area_last->min_vnum ||
		    vnum > area_last->max_vnum)
			bug("object vnum %d out of range.", vnum);

		fBootDb = FALSE;

		if (get_obj_index(vnum) != NULL) {
			bug("Load_objects: vnum %d duplicated.", vnum);
			exit(1);
		}

		fBootDb = TRUE;
		pObjIndex                       = alloc_perm(sizeof(*pObjIndex));
		pObjIndex->vnum                 = vnum;
		pObjIndex->reset_num            = 0;
		pObjIndex->version              = aVersion;
		pObjIndex->name                 = fread_string(fp);
		pObjIndex->short_descr          = fread_string(fp);
		pObjIndex->description          = fread_string(fp);
		pObjIndex->material             = fread_string(fp);
		pObjIndex->item_type            = item_lookup(fread_word(fp));
		pObjIndex->extra_flags          = fread_flag(fp);
		pObjIndex->wear_flags           = fread_flag(fp);

		switch (pObjIndex->item_type) {
		case ITEM_WEAPON:
			pObjIndex->value[0]         = weapon_type(fread_word(fp));
			pObjIndex->value[1]         = fread_number(fp);
			pObjIndex->value[2]         = fread_number(fp);
			pObjIndex->value[3]         = attack_lookup(fread_word(fp));
			pObjIndex->value[4]         = fread_flag(fp);
			break;

		case ITEM_CONTAINER:
			pObjIndex->value[0]         = fread_number(fp);
			pObjIndex->value[1]         = fread_flag(fp);
			pObjIndex->value[2]         = fread_number(fp);
			pObjIndex->value[3]         = fread_number(fp);
			pObjIndex->value[4]         = fread_number(fp);
			break;

		case ITEM_DRINK_CON:
		case ITEM_FOUNTAIN:
			pObjIndex->value[0]         = fread_number(fp);
			pObjIndex->value[1]         = fread_number(fp);
			pObjIndex->value[2]         = liq_lookup(fread_word(fp));

			if (pObjIndex->value[2] == -1) {
				pObjIndex->value[2] = 0;
				bug("Unknown liquid type", 0);
			}

			pObjIndex->value[3]         = fread_number(fp);
			pObjIndex->value[4]         = fread_number(fp);
			break;

		case ITEM_WAND:
		case ITEM_STAFF:
			pObjIndex->value[0]         = fread_number(fp);
			pObjIndex->value[1]         = fread_number(fp);
			pObjIndex->value[2]         = fread_number(fp);
			pObjIndex->value[3]         = skill_lookup(fread_word(fp));
			pObjIndex->value[4]         = fread_number(fp);
			break;

		case ITEM_POTION:
		case ITEM_PILL:
		case ITEM_SCROLL:
			pObjIndex->value[0]         = fread_number(fp);
			pObjIndex->value[1]         = skill_lookup(fread_word(fp));
			pObjIndex->value[2]         = skill_lookup(fread_word(fp));
			pObjIndex->value[3]         = skill_lookup(fread_word(fp));
			pObjIndex->value[4]         = skill_lookup(fread_word(fp));
			break;

		case ITEM_TOKEN:
			pObjIndex->value[0]         = fread_number(fp);
			pObjIndex->value[1]         = fread_number(fp);
			pObjIndex->value[2]         = fread_number(fp);
			pObjIndex->value[3]         = fread_number(fp);
			pObjIndex->value[4]         = fread_number(fp);
			break;

		default:
			pObjIndex->value[0]             = fread_flag(fp);
			pObjIndex->value[1]             = fread_flag(fp);
			pObjIndex->value[2]             = fread_flag(fp);
			pObjIndex->value[3]             = fread_flag(fp);
			pObjIndex->value[4]             = fread_flag(fp);
			break;
		}

		pObjIndex->level                = fread_number(fp);
		pObjIndex->weight               = fread_number(fp);
		pObjIndex->cost                 = fread_number(fp);
		/* condition */
		letter                          = fread_letter(fp);

		switch (letter) {
		case ('P') :                pObjIndex->condition = 100; break;

		case ('G') :                pObjIndex->condition =  90; break;

		case ('A') :                pObjIndex->condition =  75; break;

		case ('W') :                pObjIndex->condition =  50; break;

		case ('D') :                pObjIndex->condition =  25; break;

		case ('B') :                pObjIndex->condition =  10; break;

		case ('R') :                pObjIndex->condition =   5; break;

		case ('I') :                pObjIndex->condition =  -1; break;

		default:                    pObjIndex->condition = 100; break;
		}

		for (; ;) {
			char letter;
			letter = fread_letter(fp);

			if (letter == 'A') {
				AFFECT_DATA *paf;
				paf                     = alloc_perm(sizeof(*paf));
				paf->where              = TO_OBJECT;
				paf->type               = -1;
				paf->level              = pObjIndex->level;
				paf->duration           = -1;
				paf->location           = fread_number(fp);
				paf->modifier           = fread_number(fp);
				paf->bitvector          = 0;
				paf->evolution          = 1;
				paf->next               = pObjIndex->affected;
				pObjIndex->affected     = paf;
				top_affect++;
			}
			else if (letter == 'F') {
				AFFECT_DATA *paf;
				paf                     = alloc_perm(sizeof(*paf));
				letter                  = fread_letter(fp);

				switch (letter) {
				case 'A':
					paf->where          = TO_AFFECTS;
					break;

				case 'D':
					paf->where          = TO_DRAIN;
					break;

				case 'I':
					paf->where          = TO_IMMUNE;
					break;

				case 'R':
					paf->where          = TO_RESIST;
					break;

				case 'V':
					paf->where          = TO_VULN;
					break;

				default:
					bug("Load_objects: Bad where on flag set.", 0);
					exit(1);
				}

				paf->type               = -1;
				paf->level              = pObjIndex->level;
				paf->duration           = -1;
				paf->location           = fread_number(fp);
				paf->modifier           = fread_number(fp);
				paf->bitvector          = fread_flag(fp);
				paf->evolution          = 1;
				paf->next               = pObjIndex->affected;
				pObjIndex->affected     = paf;
				top_affect++;
			}
			else if (letter == 'E') {
				EXTRA_DESCR_DATA *ed;
				ed                      = alloc_perm(sizeof(*ed));
				ed->keyword             = fread_string(fp);
				ed->description         = fread_string(fp);
				ed->next                = pObjIndex->extra_descr;
				pObjIndex->extra_descr  = ed;
				top_ed++;
			}
			else {
				ungetc(letter, fp);
				break;
			}
		}

		iHash                   = vnum % MAX_KEY_HASH;
		pObjIndex->next         = obj_index_hash[iHash];
		obj_index_hash[iHash]   = pObjIndex;
		top_obj_index++;
	}

	return;
}

