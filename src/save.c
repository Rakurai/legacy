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
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "deps/cJSON/cJSON.h"

extern  int     _filbuf         args((FILE *));
extern void     goto_line       args((CHAR_DATA *ch, int row, int column));
extern void     set_window      args((CHAR_DATA *ch, int top, int bottom));

#define CURRENT_VERSION         15   /* version number for pfiles */

bool debug_json = FALSE;

/* Locals */

int rename(const char *oldfname, const char *newfname);

char *print_flags(int flag)
{
	int count, pos = 0;
	static char buf[52];

	for (count = 0; count < 32; count++) {
		if (IS_SET(flag, 1 << count)) {
			if (count < 26)
				buf[pos] = 'A' + count;
			else
				buf[pos] = 'a' + (count - 26);

			pos++;
		}
	}

	if (pos == 0) {
		buf[pos] = '0';
		pos++;
	}

	buf[pos] = '\0';
	return buf;
}

long read_flags(char *str) {
	char *p = str;
	long number = 0;
	bool sign = FALSE;

	if (*p == '-') {
		sign = TRUE;
		p++;
	}

	if (!isdigit(*p)) {
		while (('A' <= *p && *p <= 'Z') || ('a' <= *p && *p <= 'z')) {
			number += flag_convert(*p);
			p++;
		}
	}

	while (isdigit(*p)) {
		number = number * 10 + *p - '0';
		p++;
	}

	if (sign)
		number = 0 - number;

	if (*p == '|')
		number += read_flags(p+1);

	return number;
}

/*
 * Local functions.
 */
void    fwrite_char     args((CHAR_DATA *ch,  FILE *fp));
void    fwrite_obj      args((CHAR_DATA *ch,  OBJ_DATA  *obj,
                              FILE *fp, int iNest, bool locker, bool strongbox));
void    fwrite_pet      args((CHAR_DATA *pet, FILE *fp));
void    fread_char      args((CHAR_DATA *ch,  cJSON *json, int version));
void    fread_player      args((CHAR_DATA *ch,  cJSON *json, int version));
void    fread_pet       args((CHAR_DATA *ch,  cJSON *json, int version));
void	fread_objects	args((CHAR_DATA *ch, cJSON *json, void (*obj_to)(OBJ_DATA *, CHAR_DATA *), int version));

void get_JSON_boolean(cJSON *obj, bool *target, char *key);
void get_JSON_short(cJSON *obj, sh_int *target, char *key);
void get_JSON_int(cJSON *obj, int *target, char *key);
void get_JSON_long(cJSON *obj, long *target, char *key);
void get_JSON_flags(cJSON *obj, long *target, char *key);
void get_JSON_string(cJSON *obj, char **target, char *key);

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj(CHAR_DATA *ch)
{
	char strsave[MIL], buf[MSL];
	FILE *fp;

	if (ch == NULL || IS_NPC(ch))
		return;

	if (ch->desc != NULL && ch->desc->original != NULL)
		ch = ch->desc->original;

	// added if to avoid closing invalid file
	one_argument(ch->name, buf);
	sprintf(strsave, "%s%s", PLAYER_DIR, capitalize(buf));

	if ((fp = fopen(TEMP_FILE, "w")) == NULL) {
		bug("Save_char_obj: fopen", 0);
		perror(strsave);
	}
	else {
		if (ch->pcdata)
			ch->pcdata->last_saved = current_time;

		fwrite_char(ch, fp);

		if (ch->carrying)
			fwrite_obj(ch, ch->carrying, fp, 0, FALSE, FALSE);

		if (ch->pcdata) {
			if (ch->pcdata->locker)
				fwrite_obj(ch, ch->pcdata->locker, fp, 0, TRUE, FALSE);

			if (ch->pcdata->strongbox)
				fwrite_obj(ch, ch->pcdata->strongbox, fp, 0, FALSE, TRUE);
		}

		/* save the pets */
		if (ch->pet)
			fwrite_pet(ch->pet, fp);

		fprintf(fp, "#END\n");
		fclose(fp);
	}

	rename(TEMP_FILE, strsave);
	update_pc_index(ch, FALSE);
}

void backup_char_obj(CHAR_DATA *ch)
{
	char strsave[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	FILE *fp;

	if (IS_NPC(ch))
		return;

	if (ch->desc != NULL && ch->desc->original != NULL)
		ch = ch->desc->original;

	one_argument(ch->name, buf);
	sprintf(strsave, "%s%s", BACKUP_DIR, capitalize(buf));

	if ((fp = fopen(TEMP_FILE, "w")) == NULL) {
		bug("Save_char_obj: fopen", 0);
		perror(strsave);
	}
	else {
		fwrite_char(ch, fp);

		if (ch->carrying != NULL)
			fwrite_obj(ch, ch->carrying, fp, 0, FALSE, FALSE);

		if (ch->pcdata) {
			if (ch->pcdata->locker)
				fwrite_obj(ch, ch->pcdata->locker, fp, 0, TRUE, FALSE);

			if (ch->pcdata->strongbox)
				fwrite_obj(ch, ch->pcdata->strongbox, fp, 0, FALSE, TRUE);
		}

		/* save the pets */
		if (ch->pet != NULL /* && ch->pet->in_room == ch->in_room */)
			fwrite_pet(ch->pet, fp);

		fprintf(fp, "#END\n");
	}

	fclose(fp);
	rename(TEMP_FILE, strsave);
	sprintf(buf, "gzip -fq %s", strsave);
	system(buf);
} /* end backup_char_obj() */

/*
 * Write the char.
 */
void fwrite_char(CHAR_DATA *ch, FILE *fp)
{
	AFFECT_DATA *paf;
	int sn, gn, pos, i, c;
	int count;   /* count granted commands */
	fprintf(fp, "#PLAYER\n");
	fprintf(fp, "Name %s~\n",  ch->name);
	fprintf(fp, "Id   %ld\n",  ch->id);
	fprintf(fp, "LogO %ld\n",  current_time);
	fprintf(fp, "Vers %d\n",   CURRENT_VERSION);
	fprintf(fp, "Mexp %d\n",   ch->pcdata->mud_exp);
	fprintf(fp, "Pass %s~\n", ch->pcdata->pwd);

	if (ch->short_descr[0] != '\0')
		fprintf(fp, "ShD  %s~\n", ch->short_descr);

	if (ch->long_descr[0] != '\0')
		fprintf(fp, "LnD  %s~\n", ch->long_descr);

	if (ch->description[0] != '\0')
		fprintf(fp, "Desc %s~\n", ch->description);

	if (ch->prompt != NULL)
		fprintf(fp, "Prom %s~\n", ch->prompt);

	fprintf(fp, "Race %s~\n", pc_race_table[ch->race].name);

	if (ch->clan)
		fprintf(fp, "Clan %s~\n", ch->clan->name);

	fprintf(fp, "Cla  %d\n", ch->class);
	fprintf(fp, "Levl %d\n", ch->level);
	fprintf(fp, "Plyd %d\n", ch->pcdata->played); /*w1*/
	fprintf(fp, "Back %d\n", ch->pcdata->backup);
	fprintf(fp, "Note %ld %ld %ld %ld %ld %ld %ld\n",
	        ch->pcdata->last_note,
	        ch->pcdata->last_idea,
	        ch->pcdata->last_roleplay,
	        ch->pcdata->last_immquest,
	        ch->pcdata->last_changes,
	        ch->pcdata->last_personal,
	        ch->pcdata->last_trade);
	fprintf(fp, "Scro %d\n", ch->lines);
	fprintf(fp, "Room %d\n",
	        (ch->in_room == get_room_index(ROOM_VNUM_LIMBO) && ch->was_in_room != NULL)
	        ? ch->was_in_room->vnum
	        : ch->in_room == NULL
	        ? 3001
	        : ch->in_room->vnum);
	fprintf(fp, "HMS  %d %d %d\n", ch->hit, ch->mana, ch->stam);
	fprintf(fp, "Gold %ld\n", UMAX(0, ch->gold));
	fprintf(fp, "Silv %ld\n", UMAX(0, ch->silver));

	if (ch->silver_in_bank > 0)
		fprintf(fp, "Silver_in_bank %ld\n", ch->silver_in_bank);

	if (ch->gold_in_bank > 0)
		fprintf(fp, "Gold_in_bank %ld\n", ch->gold_in_bank);

	fprintf(fp, "Exp  %d\n", ch->exp);
	fprintf(fp, "PCkills  %d\n", ch->pcdata->pckills);
	fprintf(fp, "PCkilled %d\n", ch->pcdata->pckilled);
	fprintf(fp, "Akills   %d\n", ch->pcdata->arenakills);
	fprintf(fp, "Akilled  %d\n", ch->pcdata->arenakilled);
	fprintf(fp, "PKRank   %d\n", ch->pcdata->pkrank);
	fprintf(fp, "Pos  %d\n", ch->position);
	fprintf(fp, "Alig %d\n", ch->alignment);

	if (ch->pcdata->spouse != NULL)
		fprintf(fp, "Spou %s~\n", ch->pcdata->spouse);

	if (ch->pcdata->whisper != NULL)
		fprintf(fp, "Wspr %s~\n", ch->pcdata->whisper);

	if (ch->pcdata->mark_room)
		fprintf(fp, "Mark %d\n", ch->pcdata->mark_room);

	fprintf(fp, "FImm %s\n", print_flags(ch->imm_flags));
	fprintf(fp, "FRes %s\n", print_flags(ch->res_flags));
	fprintf(fp, "FVul %s\n", print_flags(ch->vuln_flags));
	fprintf(fp, "Plr  %s\n", print_flags(ch->pcdata->plr));
	fprintf(fp, "Video %s\n", print_flags(ch->pcdata->video));
	fprintf(fp, "Act  %s\n", print_flags(ch->act));
	fprintf(fp, "AfBy %s\n", print_flags(ch->affected_by));
	fprintf(fp, "Comm %s\n", print_flags(ch->comm));
	fprintf(fp, "Revk %s\n", print_flags(ch->revoke));              /* Xenith */
	fprintf(fp, "Cgrp %s\n", print_flags(ch->pcdata->cgroup));      /* Xenith */
	fprintf(fp, "Cnsr %s\n", print_flags(ch->censor));              /* Montrey */
	fprintf(fp, "Prac %d\n", ch->practice);
	fprintf(fp, "Trai %d\n", ch->train);
	fprintf(fp, "Wimp %d\n", ch->wimpy);

	/* write lay on hands data */
	if (ch->class == PALADIN_CLASS) {
		fprintf(fp, "Lay %d\n", ch->pcdata->lays);
		fprintf(fp, "Lay_Next %d\n", ch->pcdata->next_lay_countdown);
	}

	/* write familiar data */
	fprintf(fp, "Familiar %d\n", ch->pcdata->familiar);

	if (IS_IMMORTAL(ch)) {
		fprintf(fp, "Wizn %s\n", print_flags(ch->wiznet));
		fprintf(fp, "Invi %d\n", ch->invis_level);
		fprintf(fp, "Lurk %d\n", ch->lurk_level);
		fprintf(fp, "Secu %d\n", ch->secure_level);
	}

	fprintf(fp, "Atrib %d %d %d %d %d %d\n",
	        ch->perm_stat[STAT_STR], ch->perm_stat[STAT_INT], ch->perm_stat[STAT_WIS],
	        ch->perm_stat[STAT_DEX], ch->perm_stat[STAT_CON], ch->perm_stat[STAT_CHR]);

	if (ch->pcdata->flag_killer)
		fprintf(fp, "FlagKiller %d\n", ch->pcdata->flag_killer);

	if (ch->pcdata->flag_thief)
		fprintf(fp, "FlagThief  %d\n", ch->pcdata->flag_thief);

	if (ch->pcdata->skillpoints)
		fprintf(fp, "SkillPnts  %d\n", ch->pcdata->skillpoints);

	if (ch->pcdata->rolepoints)
		fprintf(fp, "RolePnts   %d\n", ch->pcdata->rolepoints);

	if (ch->questpoints)
		fprintf(fp, "QuestPnts  %d\n", ch->questpoints);

	if (ch->questpoints_donated)
		fprintf(fp, "QpDonated  %d\n", ch->questpoints_donated);

	if (ch->gold_donated)
		fprintf(fp, "GlDonated  %ld\n", ch->gold_donated);

	if (ch->nextquest)
		fprintf(fp, "QuestNext  %d\n", ch->nextquest);
	else if (ch->countdown)
		fprintf(fp, "QuestNext  %d\n", 12);

	if (ch->pcdata->nextsquest)
		fprintf(fp, "SQuestNext %d\n", ch->pcdata->nextsquest);
	else if (ch->pcdata->sqcountdown)
		fprintf(fp, "SQuestNext %d\n", 20);

	if (ch->pcdata->bamfin[0] != '\0')
		fprintf(fp, "Bin  %s~\n",  ch->pcdata->bamfin);

	if (ch->pcdata->bamfout[0] != '\0')
		fprintf(fp, "Bout %s~\n",      ch->pcdata->bamfout);

	if (ch->pcdata->gamein && ch->pcdata->gamein[0] != '\0')
		fprintf(fp, "GameIn  %s~\n",       ch->pcdata->gamein);

	if (ch->pcdata->gameout && ch->pcdata->gameout[0] != '\0')
		fprintf(fp, "GameOut %s~\n",   ch->pcdata->gameout);

	if (ch->pcdata->afk[0] != '\0')
		fprintf(fp, "Afk %s~\n",    ch->pcdata->afk);

	fprintf(fp, "Titl %s~\n",      ch->pcdata->title);
	fprintf(fp, "Immn %s~\n",      ch->pcdata->immname);
	fprintf(fp, "Pnts %d\n",       ch->pcdata->points);
	fprintf(fp, "Rank %s~\n",      ch->pcdata->rank);
	fprintf(fp, "Aura %s~\n",      ch->pcdata->aura);
	fprintf(fp, "TSex %d\n",       ch->pcdata->true_sex);
	fprintf(fp, "LLev %d\n",       ch->pcdata->last_level);

	if (ch->pcdata->email[0] != '\0')
		fprintf(fp, "Email %s~\n",      ch->pcdata->email);

	fprintf(fp, "Deit %s~\n",      ch->pcdata->deity);

	if (ch->pcdata->remort_count > 0) {
		fprintf(fp, "Stus %s~\n",      ch->pcdata->status);
		fprintf(fp, "RmCt %d\n",       ch->pcdata->remort_count);
		fprintf(fp, "ExSk ");

		for (c = 0; c < (ch->pcdata->remort_count / 20); c++)
			fprintf(fp, "%d ",       skill_table[ch->pcdata->extraclass[c]].slot);

		fprintf(fp, "%d\n", skill_table[ch->pcdata->extraclass[ch->pcdata->remort_count / 20]].slot);
		fprintf(fp, "Raff ");

		for (c = 0; c < (ch->pcdata->remort_count / 10); c++)
			fprintf(fp, "%d ",       ch->pcdata->raffect[c]);

		fprintf(fp, "%d\n", ch->pcdata->raffect[ch->pcdata->remort_count / 10]);
	}

	fprintf(fp, "Lsit %s~\n",      ch->pcdata->last_lsite);
	fprintf(fp, "Ltim %s~\n",      dizzy_ctime(&ch->pcdata->last_ltime));
	fprintf(fp, "LSav %s~\n",      dizzy_ctime(&ch->pcdata->last_saved));

	if (ch->pcdata->fingerinfo[0] != '\0')
		fprintf(fp, "Finf %s~\n",      ch->pcdata->fingerinfo);

	fprintf(fp, "HMSP %d %d %d\n", ch->pcdata->perm_hit,
	        ch->pcdata->perm_mana,
	        ch->pcdata->perm_stam);
	fprintf(fp, "THMS %d %d %d\n", ch->pcdata->trains_to_hit,
	        ch->pcdata->trains_to_mana,
	        ch->pcdata->trains_to_stam);
	fprintf(fp, "Cnd  %d %d %d %d\n",
	        ch->pcdata->condition[0],
	        ch->pcdata->condition[1],
	        ch->pcdata->condition[2],
	        ch->pcdata->condition[3]);

	/* new writing color settings to file -- Montrey */
	for (i = 0; i < MAX_COLORS; i++)
		if (ch->pcdata->color[i] > 0)
			fprintf(fp, "Colr %d %d %d\n", i, ch->pcdata->color[i], ch->pcdata->bold[i]);

	for (pos = 0; pos < MAX_IGNORE; pos++) {
		if (ch->pcdata->ignore[pos] == NULL)
			break;

		fprintf(fp, "Ignore %s~\n", ch->pcdata->ignore[pos]);
	}

	/* write alias */
	for (pos = 0; pos < MAX_ALIAS; pos++) {
		if (ch->pcdata->alias[pos] == NULL
		    ||  ch->pcdata->alias_sub[pos] == NULL)
			break;

		fprintf(fp, "Alias %s %s~\n", ch->pcdata->alias[pos],
		        ch->pcdata->alias_sub[pos]);
	}

	/* write query list */
	for (pos = 0; pos < MAX_QUERY; pos++) {
		if (ch->pcdata->query[pos] == NULL)
			break;

		fprintf(fp, "Query %s~\n", ch->pcdata->query[pos]);
	}

	for (sn = 0; sn < MAX_SKILL; sn++) {
		if (skill_table[sn].name == NULL)
			break;

		if (ch->pcdata->learned[sn] > 0) {
			if (ch->pcdata->evolution[sn] < 1)
				ch->pcdata->evolution[sn] = 1;
			else if (ch->pcdata->evolution[sn] > 4)
				ch->pcdata->evolution[sn] = 4;

			fprintf(fp, "Sk %d %d '%s'\n",
			        ch->pcdata->learned[sn], ch->pcdata->evolution[sn], skill_table[sn].name);
		}
	}

	for (gn = 0; gn < MAX_GROUP; gn++) {
		if (group_table[gn].name != NULL && ch->pcdata->group_known[gn])
			fprintf(fp, "Gr '%s'\n", group_table[gn].name);
	}

	/* write granted commands */
	for (count = 0; count < MAX_GRANT; count++) {
		if (ch->pcdata->granted_commands[count][0])
			fprintf(fp, "Grant %s\n", ch->pcdata->granted_commands[count]);
	}

	for (paf = ch->affected; paf != NULL; paf = paf->next) {
		if (paf->type < 0 || paf->type >= MAX_SKILL)
			continue;

		fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d %d\n",
		        skill_table[paf->type].name,
		        paf->where,
		        paf->level,
		        paf->duration,
		        paf->modifier,
		        paf->location,
		        paf->bitvector,
		        paf->evolution ? paf->evolution : 1
		       );
	}

	fprintf(fp, "End\n\n");
	return;
}

/* write a pet */
void fwrite_pet(CHAR_DATA *pet, FILE *fp)
{
	AFFECT_DATA *paf;
	fprintf(fp, "#PET\n");
	fprintf(fp, "Vnum %d\n", pet->pIndexData->vnum);
	fprintf(fp, "Name %s~\n", pet->name);
	fprintf(fp, "LogO %ld\n", current_time);

	if (pet->short_descr != pet->pIndexData->short_descr)
		fprintf(fp, "ShD  %s~\n", pet->short_descr);

	if (pet->long_descr != pet->pIndexData->long_descr)
		fprintf(fp, "LnD  %s~\n", pet->long_descr);

	if (pet->description != pet->pIndexData->description)
		fprintf(fp, "Desc %s~\n", pet->description);

	if (pet->race != pet->pIndexData->race)
		fprintf(fp, "Race %s~\n", race_table[pet->race].name);

	if (pet->clan)
		fprintf(fp, "Clan %s~\n", pet->clan->name);

	fprintf(fp, "Sex  %d\n", pet->sex);

	if (pet->level != pet->pIndexData->level)
		fprintf(fp, "Levl %d\n", pet->level);

	// change this to new 3 style
	fprintf(fp, "HMS  %d %d %d %d %d %d\n",
	        pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->stam, pet->max_stam);

	if (pet->gold > 0)
		fprintf(fp, "Gold %ld\n", pet->gold);

	if (pet->silver > 0)
		fprintf(fp, "Silv %ld\n", pet->silver);

	if (pet->exp > 0)
		fprintf(fp, "Exp  %d\n", pet->exp);

	if (pet->act != pet->pIndexData->act)
		fprintf(fp, "Act  %s\n", print_flags(pet->act));

	if (pet->affected_by != pet->pIndexData->affected_by)
		fprintf(fp, "AfBy %s\n", print_flags(pet->affected_by));

	if (pet->comm != 0)
		fprintf(fp, "Comm %s\n", print_flags(pet->comm));

	if (pet->censor != 0)
		fprintf(fp, "Cnsr %s\n", print_flags(pet->censor)); /* Montrey */

	fprintf(fp, "Pos  %d\n", pet->position);

	if (pet->saving_throw != 0)
		fprintf(fp, "Save %d\n", pet->saving_throw);

	if (pet->alignment != pet->pIndexData->alignment)
		fprintf(fp, "Alig %d\n", pet->alignment);

	if (pet->hitroll != pet->pIndexData->hitroll)
		fprintf(fp, "Hit  %d\n", pet->hitroll);

	if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
		fprintf(fp, "Dam  %d\n", pet->damroll);

	fprintf(fp, "Atrib %d %d %d %d %d %d\n",
	        pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
	        pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
	        pet->perm_stat[STAT_CON], pet->perm_stat[STAT_CHR]);
	fprintf(fp, "AtMod %d %d %d %d %d %d\n",
	        pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
	        pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
	        pet->mod_stat[STAT_CON], pet->mod_stat[STAT_CHR]);

	for (paf = pet->affected; paf != NULL; paf = paf->next) {
		if (paf->type < 0 || paf->type >= MAX_SKILL)
			continue;

		fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d %d\n",
		        skill_table[paf->type].name,
		        paf->where, paf->level, paf->duration, paf->modifier, paf->location,
		        paf->bitvector, paf->evolution ? paf->evolution : 1);
	}

	fprintf(fp, "End\n");
	return;
}

/*
 * Write an object and its contents.
 */
void fwrite_obj(CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest,
                bool locker, bool strongbox)
{
	EXTRA_DESCR_DATA *ed;
	AFFECT_DATA *paf;
	int i2;

	/*
	 * Slick recursion to write lists backwards,
	 *   so loading them will load in forwards order.
	 */
	if (obj->next_content != NULL)
		fwrite_obj(ch, obj->next_content, fp, iNest, locker, strongbox);

	/*
	 * Castrate storage characters.
	 */
	if (!IS_IMMORTAL(ch))
		if ((!strongbox && (obj->level > get_holdable_level(ch)))
		    || (obj->item_type == ITEM_KEY && (obj->value[0] == 0))
		    || (obj->item_type == ITEM_MAP && !obj->value[0]))
			return;

	if (locker)
		fprintf(fp, "#L\n");
	else if (strongbox)
		fprintf(fp, "#B\n");
	else
		fprintf(fp, "#O\n");

	fprintf(fp, "Vnum %d\n",   obj->pIndexData->vnum);

	if (obj->enchanted)
		fprintf(fp, "Enchanted\n");

	fprintf(fp, "Nest %d\n",   iNest);

	/* these data are only used if they do not match the defaults */

	if (obj->name != obj->pIndexData->name)
		fprintf(fp, "Name %s~\n",      obj->name);

	if (obj->short_descr != obj->pIndexData->short_descr)
		fprintf(fp, "ShD  %s~\n",      obj->short_descr);

	if (obj->description != obj->pIndexData->description)
		fprintf(fp, "Desc %s~\n",      obj->description);

	if (obj->material != obj->pIndexData->material)
		fprintf(fp, "Mat %s~\n",       obj->material);

	if (obj->extra_flags != obj->pIndexData->extra_flags)
		fprintf(fp, "ExtF %ld\n",      obj->extra_flags);

	if (obj->wear_flags != obj->pIndexData->wear_flags)
		fprintf(fp, "WeaF %ld\n",      obj->wear_flags);

	if (obj->item_type != obj->pIndexData->item_type)
		fprintf(fp, "Ityp %d\n",       obj->item_type);

	if (obj->weight != obj->pIndexData->weight)
		fprintf(fp, "Wt   %d\n",       obj->weight);

	if (obj->condition != obj->pIndexData->condition)
		fprintf(fp, "Cond %d\n",       obj->condition);

	/* variable data */
	fprintf(fp, "Wear %d\n",   obj->wear_loc);

	if (obj->level != obj->pIndexData->level)
		fprintf(fp, "Lev  %d\n",       obj->level);

	if (obj->timer != 0)
		fprintf(fp, "Time %d\n",       obj->timer);

	fprintf(fp, "Cost %d\n",   obj->cost);

	if (obj->value[0] != obj->pIndexData->value[0]
	    ||  obj->value[1] != obj->pIndexData->value[1]
	    ||  obj->value[2] != obj->pIndexData->value[2]
	    ||  obj->value[3] != obj->pIndexData->value[3]
	    ||  obj->value[4] != obj->pIndexData->value[4])
		fprintf(fp, "Val  %d %d %d %d %d\n",
		        obj->value[0], obj->value[1], obj->value[2], obj->value[3],
		        obj->value[4]);

	/*
	 * Spelled eq by Demonfire
	 * Added on 11.23.1996
	 */
	/* Old crappy way to do this...
	    for (i2 = 1 ; i2 < MAX_SPELL ; i2++)
	        if (obj->spell[i2])
	            fprintf(fp, "Splx %d %d  %d \n",i2,obj->spell[i2],obj->spell_lev[i2]);
	*/
	for (i2 = 1 ; i2 < MAX_SPELL ; i2++)
		if (obj->spell[i2])
			fprintf(fp, "Splxtra %d '%s' %d\n", i2,
			        skill_table[obj->spell[i2]].name,
			        obj->spell_lev[i2]);

	switch (obj->item_type) {
	case ITEM_POTION:
	case ITEM_SCROLL:
		if (obj->value[1] > 0) {
			fprintf(fp, "Spell 1 '%s'\n",
			        skill_table[obj->value[1]].name);
		}

		if (obj->value[2] > 0) {
			fprintf(fp, "Spell 2 '%s'\n",
			        skill_table[obj->value[2]].name);
		}

		if (obj->value[3] > 0) {
			fprintf(fp, "Spell 3 '%s'\n",
			        skill_table[obj->value[3]].name);
		}

		if (obj->value[4] > 0) {
			fprintf(fp, "Spell 4 '%s'\n",
			        skill_table[obj->value[4]].name);
		}

		break;

	case ITEM_PILL:
	case ITEM_STAFF:
	case ITEM_WAND:
		if (obj->value[3] > 0) {
			fprintf(fp, "Spell 3 '%s'\n",
			        skill_table[obj->value[3]].name);
		}

		break;
	}

	for (paf = obj->affected; paf != NULL; paf = paf->next) {
		if (paf->type < 0 || paf->type >= MAX_SKILL)
			continue;

		fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d %d\n",
		        skill_table[paf->type].name,
		        paf->where,
		        paf->level,
		        paf->duration,
		        paf->modifier,
		        paf->location,
		        paf->bitvector,
		        paf->evolution ? paf->evolution : 1
		       );
	}

	for (ed = obj->extra_descr; ed != NULL; ed = ed->next) {
		fprintf(fp, "ExDe %s~ %s~\n",
		        ed->keyword, ed->description);
	}

	fprintf(fp, "End\n\n");

	if (obj->contains != NULL)
		fwrite_obj(ch, obj->contains, fp, iNest + 1, locker, strongbox);
} /* end fwrite_obj() */

/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj(DESCRIPTOR_DATA *d, char *name)
{
	char strsave[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	FILE *fp;
	bool found;
	int stat;
	ch = new_char();
	ch->pcdata = new_pcdata();
	d->character                        = ch;
	ch->desc                            = d;
	ch->name                            = str_dup(name);
	ch->id                              = get_pc_id();
	ch->race                            = race_lookup("human");
	ch->act                             = PLR_NOSUMMON | PLR_AUTOASSIST | PLR_AUTOEXIT | PLR_AUTOLOOT |
	                                      PLR_AUTOSAC | PLR_AUTOSPLIT | PLR_AUTOGOLD | PLR_TICKS | PLR_WIMPY |
	                                      PLR_COLOR | PLR_COLOR2;
	ch->comm                            = COMM_COMBINE | COMM_PROMPT;
	ch->revoke                          = 0; /* Xenith */
	ch->secure_level                    = RANK_IMM;
	ch->pcdata->cgroup                  = 0; /* Command groups - Xenith */
	ch->censor                          = CENSOR_CHAN;    /* default rating is PG */
	ch->prompt                          = str_dup("%CW<%CC%h%CThp %CG%m%CHma %CB%v%CNst%CW> ");
	ch->pcdata->ch                      = ch;
	ch->pcdata->confirm_delete          = 0;
	ch->pcdata->pwd                     = str_dup("");
	ch->pcdata->bamfin                  = str_dup("");
	ch->pcdata->bamfout                 = str_dup("");
	ch->pcdata->gamein                  = str_dup("");
	ch->pcdata->gameout                 = str_dup("");
	ch->pcdata->afk                     = str_dup("");
	ch->pcdata->title                   = str_dup("");
	ch->pcdata->immname                 = str_dup("");
	ch->pcdata->email                   = str_dup("");
	ch->pcdata->fingerinfo              = str_dup("");
	ch->pcdata->last_lsite              = str_dup("");
	ch->pcdata->status                  = str_dup("");
	ch->pcdata->rank                    = str_dup("");
	ch->pcdata->aura                    = str_dup("");
	ch->pcdata->deity                   = str_dup("Nobody");
	ch->pcdata->mud_exp                 = MEXP_LEGACY_OLDBIE;
	ch->pcdata->remort_count            = 0;
	ch->pcdata->backup                  = 0;
	ch->pcdata->plr                     = PLR_NEWSCORE;
	ch->pcdata->flag_thief              = 0;
	ch->pcdata->flag_killer             = 0;

	for (stat = 0; stat < MAX_STATS; stat++)
		ch->perm_stat[stat]             = 3;

	ch->pcdata->condition[COND_THIRST]  = 48;
	ch->pcdata->condition[COND_FULL]    = 48;
	ch->pcdata->condition[COND_HUNGER]  = 48;
	ch->silver_in_bank                  = 0;
	ch->gold_in_bank                    = 0;
	ch->last_bank                       = 0;
	ch->pcdata->pckills                 = 0;
	ch->pcdata->pckilled                = 0;
	ch->pcdata->arenakills              = 0;
	ch->pcdata->arenakilled             = 0;
	ch->pcdata->pkrank                  = 0;
	ch->pcdata->perm_hit            = 20;
	ch->pcdata->perm_mana           = 100;
	ch->pcdata->perm_stam           = 100;
	ch->pcdata->trains_to_hit           = 0;
	ch->pcdata->trains_to_mana          = 0;
	ch->pcdata->trains_to_stam          = 0;
	ch->pcdata->skillpoints             = 0;
	ch->pcdata->rolepoints              = 0;
	ch->inviters                        = NULL;
	ch->invitation_accepted             = FALSE;
	ch->clan                            = NULL;
	ch->replylock                       = FALSE;
	ch->pcdata->last_logoff         = current_time;
	found = FALSE;
	// added if here
	/* decompress if .gz file exists */
	/*    #if defined(unix)
	    sprintf( strsave, "%s%s%s", PLAYER_DIR, capitalize(name),".gz");
	    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
	    {
	        fclose(fp);
	        sprintf(buf,"gzip -dfq %s",strsave);
	        system(buf);
	    }
	    #endif */
	sprintf(strsave, "%s%s.json", PLAYER_DIR, capitalize(name));

	cJSON *root = NULL;

	if ((fp = fopen(strsave, "rb")) != NULL) {
		int length;
		char *buffer;

		fseek (fp, 0, SEEK_END);
		length = ftell (fp);
		fseek (fp, 0, SEEK_SET);
		buffer = malloc (length);

		fread (buffer, 1, length, fp);
		fclose (fp);

		root = cJSON_Parse(buffer);
		free(buffer);
	}

	if (root != NULL) {

		int version = CURRENT_VERSION;
		get_JSON_int(root, &version, "Vers");

		fread_char(ch, cJSON_GetObjectItem(root, "character"), version);
		fread_player(ch, cJSON_GetObjectItem(root, "player"), version);

		fread_objects(ch, cJSON_GetObjectItem(root, "inventory"), &obj_to_char, version);
		fread_objects(ch, cJSON_GetObjectItem(root, "locker"), &obj_to_locker, version);
		fread_objects(ch, cJSON_GetObjectItem(root, "strongbox"), &obj_to_strongbox, version);
		found = TRUE;

		// fix things up

		// fix up character stuff here
		if (ch->in_room == NULL)
			ch->in_room = get_room_index(ROOM_VNUM_LIMBO);

		if (ch->secure_level > GET_RANK(ch))
			ch->secure_level = GET_RANK(ch);

		/* removed holylight at 12 -- Montrey */
		if (version < 12 && IS_SET(ch->act, N))
			REMOVE_BIT(ch->act, N);

		// switching to cgroups with old pfiles -- Montrey (2014)
		if (version < 15 && IS_SET(ch->act, N)) { // deputy
			REMOVE_BIT(ch->act, N);
			SET_CGROUP(ch, GROUP_DEPUTY);
		}

		if (version < 15 && IS_SET(ch->act, ee)) { // leader
			REMOVE_BIT(ch->act, ee);
			SET_CGROUP(ch, GROUP_LEADER);
		}

		if (ch->pcdata->remort_count > 0) {
			SET_CGROUP(ch, GROUP_AVATAR);
			SET_CGROUP(ch, GROUP_HERO);
		}

		if (ch->level >= LEVEL_AVATAR)
			SET_CGROUP(ch, GROUP_AVATAR);

		if (ch->level >= LEVEL_HERO)
			SET_CGROUP(ch, GROUP_HERO);

		if (ch->clan == NULL && !IS_IMMORTAL(ch)) {
			REM_CGROUP(ch, GROUP_LEADER);
			REM_CGROUP(ch, GROUP_DEPUTY);
		}

		if (ch->clan != NULL)
			SET_CGROUP(ch, GROUP_CLAN);

		if (!IS_IMMORTAL(ch)) {
			for (int stat = 0; stat < (MAX_STATS); stat++) {
				/* make sure stats aren't above race max, for possible changes to race maximums */
				if (stat == class_table[ch->class].attr_prime) {
					if (ch->race == 1) { /* humans */
						if (ch->perm_stat[stat] > (pc_race_table[ch->race].max_stats[stat] + 3))
							ch->perm_stat[stat] = (pc_race_table[ch->race].max_stats[stat] + 3);
					}
					else {
						if (ch->perm_stat[stat] > (pc_race_table[ch->race].max_stats[stat] + 2))
							ch->perm_stat[stat] = (pc_race_table[ch->race].max_stats[stat] + 2);
					}
				}
				else if (ch->perm_stat[stat] > pc_race_table[ch->race].max_stats[stat])
					ch->perm_stat[stat] = pc_race_table[ch->race].max_stats[stat];
			}
		}
	}

	/* initialize race */
	if (found) {
		int i, percent;

		if (ch->race == 0)
			ch->race = race_lookup("human");

		ch->size = pc_race_table[ch->race].size;
		ch->dam_type = 17; /*punch */

		for (i = 0; i < 5; i++) {
			if (pc_race_table[ch->race].skills[i] == NULL)
				break;

			group_add(ch, pc_race_table[ch->race].skills[i], FALSE);
		}

		ch->affected_by = ch->affected_by | race_table[ch->race].aff;
		ch->imm_flags   = ch->imm_flags | race_table[ch->race].imm;
		ch->res_flags   = ch->res_flags | race_table[ch->race].res;
		ch->vuln_flags  = ch->vuln_flags | race_table[ch->race].vuln;
		ch->form        = race_table[ch->race].form;
		ch->parts       = race_table[ch->race].parts;

		/* let's make sure their remort affect vuln/res is ok */
		for (i = 0; ch->pcdata->remort_count && i <= ch->pcdata->remort_count / 10 + 1; i++) {
			if (ch->pcdata->raffect[i] >= 900 && ch->pcdata->raffect[i] <= 949)
				SET_BIT(ch->vuln_flags, raffects[raff_lookup(ch->pcdata->raffect[i])].add);
			else if (ch->pcdata->raffect[i] >= 950 && ch->pcdata->raffect[i] <= 999)
				SET_BIT(ch->res_flags, raffects[raff_lookup(ch->pcdata->raffect[i])].add);
		}

		/* fix command groups */
		REMOVE_BIT(ch->act, (ee));      /* PLR_LEADER */
		REMOVE_BIT(ch->act, (N));       /* PLR_DEPUTY */
		SET_CGROUP(ch, GROUP_PLAYER);

		/* nuke wiznet flags beyond their level, in case they were temp trusted */
		if (ch->wiznet)
			for (i = 0; wiznet_table[i].name != NULL; i++)
				if (IS_SET(ch->wiznet, wiznet_table[i].flag) && GET_RANK(ch) < wiznet_table[i].level)
					REMOVE_BIT(ch->wiznet, wiznet_table[i].flag);

		reset_char(ch);
		/* adjust hp mana stamina up  -- here for speed's sake */
		percent = (current_time - ch->pcdata->last_logoff) * 25 / (2 * 60 * 60);
		percent = UMIN(percent, 100);

		if (percent > 0 && !IS_AFFECTED(ch, AFF_POISON) && !IS_AFFECTED(ch, AFF_PLAGUE)) {
			ch->hit         += (ch->max_hit - ch->hit) * percent / 100;
			ch->mana        += (ch->max_mana - ch->mana) * percent / 100;
			ch->stam        += (ch->max_stam - ch->stam) * percent / 100;
		}
	}

	return found;
}

/*
 * Read in a char.
 */

#if defined(STRKEY)
#undef STRKEY
#endif

#define STRKEY( literal, field, value )                                    \
	if ( !str_cmp( key, literal ) )        \
	{                                       \
		free_string(field);					\
		field = str_dup(value);	\
		fMatch = TRUE;						\
		break;                              \
	}


#if defined(INTKEY)
#undef INTKEY
#endif

#define INTKEY( literal, field, value )                                    \
	if ( !str_cmp( key, literal ) )        \
	{                                       \
		field  = value;               \
		fMatch = TRUE;                      \
		break;                              \
	}

#if defined(SKIPKEY)
#undef SKIPKEY
#endif

#define SKIPKEY( literal )                  \
	if ( !str_cmp( key, literal ) )			\
	{                                       \
		fMatch = TRUE;                      \
		break;                              \
	}	


void get_JSON_boolean(cJSON *obj, bool *target, char *key) {
	cJSON *val = cJSON_GetObjectItem(obj, key);

	if (val != NULL)
		*target = (val->valueint != 0);
	else
		bugf("JSON field %s not found in object %s", key, obj->string);
}

void get_JSON_short(cJSON *obj, sh_int *target, char *key) {
	cJSON *val = cJSON_GetObjectItem(obj, key);

	if (val != NULL)
		*target = val->valueint;
	else
		bugf("JSON field %s not found in object %s", key, obj->string);
}

void get_JSON_int(cJSON *obj, int *target, char *key) {
	cJSON *val = cJSON_GetObjectItem(obj, key);

	if (val != NULL)
		*target = val->valueint;
	else
		bugf("JSON field %s not found in object %s", key, obj->string);
}

void get_JSON_long(cJSON *obj, long *target, char *key) {
	cJSON *val = cJSON_GetObjectItem(obj, key);

	if (val != NULL)
		*target = val->valueint;
	else
		bugf("JSON field %s not found in object %s", key, obj->string);
}

void get_JSON_flags(cJSON *obj, long *target, char *key) {
	cJSON *val = cJSON_GetObjectItem(obj, key);

	if (val != NULL)
		*target = read_flags(val->valuestring);
	else
		bugf("JSON field %s not found in object %s", key, obj->string);
}

void get_JSON_string(cJSON *obj, char **target, char *key) {
	cJSON *val = cJSON_GetObjectItem(obj, key);

	if (val != NULL) {
		if (*target != NULL) {
			free_string(*target);
		}
		*target = str_dup(val->valuestring);
	}
	else
		bugf("JSON field %s not found in object %s", key, obj->string);
}

void fread_player(CHAR_DATA *ch, cJSON *json, int version) {
	// if there are any player-specific fields that are depended on by others in the list,
	// load them right here, and make sure to use SKIPKEY(key) in the switch

	// none


	for (cJSON *o = json->child; o; o = o->next) {
		char *key = o->string;
		bool fMatch = FALSE;
		int count = 0; // convenience variable to compact this list, resets with every item

		switch (toupper(key[0])) {
			case 'A':
				if (!str_cmp(key, "Alias")) { // array of 2-tuples
					// each alias is a 2-tuple (a list)
					for (cJSON *item = o->child; item != NULL; item = item->next, count++) {
						ch->pcdata->alias[count] = str_dup(item->child->valuestring);
						ch->pcdata->alias_sub[count] = str_dup(item->child->next->valuestring);
					}
					fMatch = TRUE; break;
				}

				STRKEY("Afk",			ch->pcdata->afk,			o->valuestring);
				INTKEY("Akills",		ch->pcdata->arenakills,		o->valueint);
				INTKEY("Akilled",		ch->pcdata->arenakilled, 	o->valueint);
				STRKEY("Aura",			ch->pcdata->aura,			o->valuestring);
				break;
			case 'B':
				INTKEY("Back",			ch->pcdata->backup,			o->valueint);
				STRKEY("Bin",			ch->pcdata->bamfin,			o->valuestring);
				STRKEY("Bout",			ch->pcdata->bamfout,		o->valuestring);
				break;
			case 'C':
				if (!str_cmp(key, "Cnd")) { // 4-tuple
					get_JSON_short(o, &ch->pcdata->condition[COND_DRUNK], "drunk");
					get_JSON_short(o, &ch->pcdata->condition[COND_FULL], "full");
					get_JSON_short(o, &ch->pcdata->condition[COND_THIRST], "thirst");
					get_JSON_short(o, &ch->pcdata->condition[COND_HUNGER], "hunger");
					fMatch = TRUE; break;
				}

				if (!str_cmp(key, "Colr")) { // array of dicts
					for (cJSON *item = o->child; item != NULL; item = item->next) {
						int slot = cJSON_GetObjectItem(item, "slot")->valueint;
						get_JSON_short(item, &ch->pcdata->color[slot], "color");
						get_JSON_short(item, &ch->pcdata->bold[slot], "bold");
					}
					fMatch = TRUE; break;
				}

				INTKEY("Cgrp",			ch->pcdata->cgroup,			read_flags(o->valuestring));
				break;
			case 'D':
				STRKEY("Deit",			ch->pcdata->deity,			o->valuestring);
				break;
			case 'E':
				if (!str_cmp(key, "ExSk")) {
					for (cJSON *item = o->child; item != NULL && count < MAX_EXTRACLASS_SLOTS; item = item->next) {
						bool found = FALSE;

						for (int i = 1; i < MAX_SKILL; i++) {
							if (skill_table[i].slot == item->valueint) {
								ch->pcdata->extraclass[count++] = i;
								found = TRUE;
								break;
							}
						}

						if (!found) {
							bug("Unknown extraclass skill.", 0);
						}
					}
					fMatch = TRUE; break;
				}

				STRKEY("Email",			ch->pcdata->email,			o->valuestring);
				break;
			case 'F':
				INTKEY("Familiar",		ch->pcdata->familiar,		o->valueint);
				STRKEY("Finf",			ch->pcdata->fingerinfo,		o->valuestring);
				INTKEY("FlagThief",		ch->pcdata->flag_thief,		o->valueint);
				INTKEY("FlagKiller",	ch->pcdata->flag_killer,	o->valueint);
				break;
			case 'G':
				if (!str_cmp(key, "Grant")) {
					for (cJSON *item = o->child; item != NULL && count < MAX_GRANT; item = item->next)
						strcpy(ch->pcdata->granted_commands[count++], item->valuestring);
					fMatch = TRUE; break;
				}

				STRKEY("GameIn",		ch->pcdata->gamein,			o->valuestring);
				STRKEY("GameOut",		ch->pcdata->gameout,		o->valuestring);
				break;
			case 'H':
				if (!str_cmp(key, "HMSP")) {
					get_JSON_short(o, &ch->pcdata->perm_hit, "hit");
					get_JSON_short(o, &ch->pcdata->perm_mana, "mana");
					get_JSON_short(o, &ch->pcdata->perm_stam, "stam");
					fMatch = TRUE; break;
				}

				break;
			case 'I':
				if (!str_cmp(key, "Ignore")) {
					for (cJSON *item = o->child; item != NULL && count < MAX_IGNORE; item = item->next)
						ch->pcdata->ignore[count++] = str_dup(item->valuestring);
					fMatch = TRUE; break;
				}

				STRKEY("Immn",			ch->pcdata->immname,		o->valuestring);
				break;
			case 'L':
				INTKEY("Lay",			ch->pcdata->lays,			o->valueint);
				INTKEY("Lay_Next",		ch->pcdata->next_lay_countdown,	o->valueint);
				INTKEY("LLev",			ch->pcdata->last_level,		o->valueint);
				INTKEY("LogO",			ch->pcdata->last_logoff,	o->valueint);
				STRKEY("Lsit",			ch->pcdata->last_lsite,		o->valuestring);
				INTKEY("Ltim",			ch->pcdata->last_ltime,		dizzy_scantime(o->valuestring));
				INTKEY("Lsav",			ch->pcdata->last_saved,		dizzy_scantime(o->valuestring));
				break;
			case 'M':
				INTKEY("Mark",			ch->pcdata->mark_room,		o->valueint);
				INTKEY("Mexp",			ch->pcdata->mud_exp,		o->valueint);
				break;
			case 'N':
				if (!str_cmp(key, "Note")) {
					get_JSON_long(o, &ch->pcdata->last_note, "note");
					get_JSON_long(o, &ch->pcdata->last_idea, "idea");
					get_JSON_long(o, &ch->pcdata->last_roleplay, "role");
					get_JSON_long(o, &ch->pcdata->last_immquest, "quest");
					get_JSON_long(o, &ch->pcdata->last_changes, "change");
					get_JSON_long(o, &ch->pcdata->last_personal, "pers");
					get_JSON_long(o, &ch->pcdata->last_trade, "trade");
					fMatch = TRUE; break;
				}

				break;
			case 'P':
				STRKEY("Pass",			ch->pcdata->pwd,		o->valuestring);
				INTKEY("PCkills",		ch->pcdata->pckills,	o->valueint);
				INTKEY("PCkilled",		ch->pcdata->pckilled,	o->valueint);
				INTKEY("PKRank",		ch->pcdata->pkrank,		o->valueint);
				INTKEY("Plyd",			ch->pcdata->played,		o->valueint);
				INTKEY("Plr",			ch->pcdata->plr,		read_flags(o->valuestring));
				INTKEY("Pnts",			ch->pcdata->points,		o->valueint);
				break;
			case 'Q':
				if (!str_cmp(key, "Query")) {
					for (cJSON *item = o->child; item != NULL && count < MAX_QUERY; item = item->next)
						ch->pcdata->query[count++] = str_dup(item->valuestring);
					fMatch = TRUE; break;
				}

				break;
			case 'R':
				if (!str_cmp(key, "Raff")) {
					for (cJSON *item = o->child; item != NULL && count < MAX_RAFFECT_SLOTS; item = item->next)
						ch->pcdata->raffect[count++] = item->valueint;
					fMatch = TRUE; break;
				}

				STRKEY("Rank",			ch->pcdata->rank,			o->valuestring);
				INTKEY("RmCt",			ch->pcdata->remort_count,	o->valueint);
				INTKEY("RolePnts",		ch->pcdata->rolepoints,		o->valueint);
				break;
			case 'S':
				if (!str_cmp(key, "Sk")) {
					for (cJSON *item = o->child; item != NULL; item = item->next) {
						char *temp = cJSON_GetObjectItem(item, "name")->valuestring;
						int sn = skill_lookup(temp);

						if (sn < 0) {
							fprintf(stderr, "%s", temp);
							bug("Fread_char: unknown skill. ", 0);
							continue;
						}

						ch->pcdata->learned[sn] = cJSON_GetObjectItem(item, "prac")->valueint;
						ch->pcdata->evolution[sn] = cJSON_GetObjectItem(item, "evol")->valueint;
					}
					fMatch = TRUE; break;
				}

				INTKEY("SkillPnts",		ch->pcdata->skillpoints,	o->valueint);
				STRKEY("Stus",			ch->pcdata->status,			o->valuestring);
				STRKEY("Spou",			ch->pcdata->spouse,			o->valuestring);
				INTKEY("SQuestNext",	ch->pcdata->nextsquest,		o->valueint);
				break;
			case 'T':
				if (!str_cmp(key, "THMS")) {
					get_JSON_short(o, &ch->pcdata->trains_to_hit, "hit");
					get_JSON_short(o, &ch->pcdata->trains_to_hit, "mana");
					get_JSON_short(o, &ch->pcdata->trains_to_hit, "stam");
					fMatch = TRUE; break;
				}

				if (!str_cmp(key, "Titl")) {
					free_string(ch->pcdata->title);
					char c = o->valuestring[0];
					char buf[MSL];

					sprintf(buf, "%s%s",
						c == '.' || c == ',' || c == '!' || c == '?' ? "" : " ", // add a space if no punctuation
						o->valuestring);
					ch->pcdata->title = str_dup(buf);
					fMatch = TRUE; break;
				}

				INTKEY("TSex",			ch->pcdata->true_sex,		o->valueint);
				break;
			case 'V':
				INTKEY("Video",			ch->pcdata->video,			read_flags(o->valuestring));
				break;
			case 'W':
				STRKEY("Wspr",			ch->pcdata->whisper,		o->valuestring);
				break;
			default:
				// drop down
				break;
		}

		if (!fMatch)
			bugf("fread_player: unknown key %s", key);
	}

	// fix up pc-only stuff here
}

// this could be PC or NPC!  get act flags first
void fread_char(CHAR_DATA *ch, cJSON *json, int version)
{
	char buf[MSL];
	sprintf(buf, "Loading %s.", ch->name);
	log_string(buf);

	// unlike old pfiles, the order of calls is important here, because we can't
	// guarantee order within the files. If there are any fields that are depended
	// on by others in the list, load them right here, and use SKIPKEY(key) in the list

	get_JSON_flags(json, &ch->act, "Act");

	// now safe to check IS_NPC

	for (cJSON *o = json->child; o; o = o->next) {
		char *key = o->string;
		bool fMatch = FALSE;
//		int count = 0; // convenience variable to compact this list, resets with every item

		switch (toupper(key[0])) {
			case 'A':
				if (!str_cmp(key, "Affc")) {
					for (cJSON *item = o->child; item != NULL; item = item->next) {
						int sn = skill_lookup(cJSON_GetObjectItem(item, "name")->valuestring);

						if (sn < 0) {
							bug("Fread_char: unknown skill.", 0);
							continue;
						}

						AFFECT_DATA *paf = new_affect();
						paf->type = sn;
						get_JSON_short(item, &paf->where, "where");
						get_JSON_short(item, &paf->level, "level");
						get_JSON_short(item, &paf->duration, "dur");
						get_JSON_short(item, &paf->modifier, "mod");
						get_JSON_short(item, &paf->location, "loc");
						get_JSON_int(item, &paf->bitvector, "bitv");
						get_JSON_short(item, &paf->evolution, "evo");

						if (IS_NPC(ch)) {
							bool found = FALSE;

							/* loop through the pet's spells, only add if they don't have it */
							for (AFFECT_DATA *old_af = ch->affected; old_af; old_af = old_af->next)
								if (old_af->type == paf->type && old_af->location == paf->location) {
									found = TRUE;
									break;
								}

							if (found) {
								free_affect(paf);
								continue;
							}
						}

						paf->next       = ch->affected;
						ch->affected    = paf;
					}
					fMatch = TRUE; break;
				}

				if (!str_cmp(key, "Atrib")) {
					get_JSON_short(o, &ch->perm_stat[STAT_STR], "str");
					get_JSON_short(o, &ch->perm_stat[STAT_INT], "int");
					get_JSON_short(o, &ch->perm_stat[STAT_WIS], "wis");
					get_JSON_short(o, &ch->perm_stat[STAT_DEX], "dex");
					get_JSON_short(o, &ch->perm_stat[STAT_CON], "con");
					get_JSON_short(o, &ch->perm_stat[STAT_CHR], "chr");
					fMatch = TRUE; break;
				}

				// npc only
				if (IS_NPC(ch) && !str_cmp(key, "AtMod")) {
					get_JSON_short(o, &ch->mod_stat[STAT_STR], "str");
					get_JSON_short(o, &ch->mod_stat[STAT_INT], "int");
					get_JSON_short(o, &ch->mod_stat[STAT_WIS], "wis");
					get_JSON_short(o, &ch->mod_stat[STAT_DEX], "dex");
					get_JSON_short(o, &ch->mod_stat[STAT_CON], "con");
					get_JSON_short(o, &ch->mod_stat[STAT_CHR], "chr");
					fMatch = TRUE; break;
				}

				SKIPKEY("Act");
				INTKEY("AfBy",			ch->affected_by,			read_flags(o->valuestring));
				INTKEY("Alig",			ch->alignment,				o->valueint);
				break;
			case 'C':
				INTKEY("Clan",			ch->clan,					clan_lookup(o->valuestring));
				INTKEY("Cla",			ch->class,					o->valueint);
				INTKEY("Comm",			ch->comm,					read_flags(o->valuestring));
				INTKEY("Cnsr",			ch->censor,					read_flags(o->valuestring));
				break;
			case 'D':
				INTKEY("Dam",			ch->damroll,				o->valueint);		// NPC
				STRKEY("Desc",			ch->description,			o->valuestring);
				break;
			case 'E':
				INTKEY("Exp",			ch->exp,					o->valueint);
				break;
			case 'F':
				INTKEY("Fimm",			ch->imm_flags,				read_flags(o->valuestring));
				INTKEY("FRes",			ch->res_flags,				read_flags(o->valuestring));
				INTKEY("FVul",			ch->vuln_flags,				read_flags(o->valuestring));
				break;
			case 'G':
				if (!str_cmp(key, "Gr")) {
					for (cJSON *item = o->child; item != NULL; item = item->next) {
						int gn = group_lookup(item->valuestring);

						if (gn < 0) {
							fprintf(stderr, "%s", item->valuestring);
							bug("Unknown group. ", 0);
							continue;
						}

						gn_add(ch, gn);
					}
					fMatch = TRUE; break;
				}

				INTKEY("Gold_in_bank",	ch->gold_in_bank,			o->valueint);
				INTKEY("Gold",			ch->gold,					o->valueint);
				INTKEY("GlDonated",		ch->gold_donated,			o->valueint);
				break;
			case 'H':
				if (!str_cmp(key, "HMS")) {
					get_JSON_short(o, &ch->hit, "hit");
					get_JSON_short(o, &ch->mana, "mana");
					get_JSON_short(o, &ch->stam, "stam");
					fMatch = TRUE; break;
				}

				INTKEY("Hit",			ch->hitroll,				o->valueint); // NPC
				break;
			case 'I':
				INTKEY("Id",			ch->id,						o->valueint);
				INTKEY("Invi",			ch->invis_level,			o->valueint);
				break;
			case 'L':
				INTKEY("Levl",			ch->level,					o->valueint);
				STRKEY("LnD",			ch->long_descr,				o->valuestring);
				INTKEY("Lurk",			ch->lurk_level,				o->valueint);
				break;
			case 'N':
				STRKEY("Name",			ch->name,					o->valuestring);
				break;
			case 'P':
				INTKEY("Pos",			ch->position,				o->valueint);
				INTKEY("Prac",			ch->practice,				o->valueint);
				STRKEY("Prom",			ch->prompt,					o->valuestring);
				break;
			case 'Q':
				INTKEY("QuestPnts",		ch->questpoints,			o->valueint);
				INTKEY("QpDonated",		ch->questpoints_donated,	o->valueint);
				INTKEY("QuestNext",		ch->nextquest,				o->valueint);
				break;
			case 'R':
				INTKEY("Race",			ch->race,					race_lookup(o->valuestring));
				INTKEY("Room",			ch->in_room,				get_room_index(o->valueint));
				INTKEY("Revk",			ch->revoke,					read_flags(o->valuestring));
				break;
			case 'S':
				INTKEY("Save",			ch->saving_throw,			o->valueint); // NPC
				INTKEY("Scro",			ch->lines,					o->valueint);
				INTKEY("Secu",			ch->secure_level,			o->valueint);
				STRKEY("ShD",			ch->short_descr,			o->valuestring);
				INTKEY("Silver_in_bank",ch->silver_in_bank,			o->valueint);
				INTKEY("Silv",			ch->silver,					o->valueint);
				break;
			case 'T':
				INTKEY("Trai",			ch->train,					o->valueint);
				break;
			case 'W':
				INTKEY("Wimp",			ch->wimpy,					o->valueint);
				INTKEY("Wizn",			ch->wiznet,					read_flags(o->valuestring));
				break;
			default:
				// drop down
				break;
		}

		if (!fMatch)
			bugf("fread_char: unknown key %s", key);
	}
}

// read a single item including its contents
OBJ_DATA * fread_obj(cJSON *json, int version) {
	OBJ_DATA *obj = NULL;
	cJSON *o;

	if ((o = cJSON_GetObjectItem(json, "Vnum")) != NULL) {
		OBJ_INDEX_DATA *index = get_obj_index(o->valueint);

		if (index == NULL)
			bug("Fread_obj: bad vnum %d in fread_obj().", o->valueint);
		else {
			obj = create_object(index, -1);

			if (obj == NULL)
				bug("fread_obj: create_object returned NULL", 0);
		}
	}
	else
		bug("fread_obj: no vnum field in JSON object", 0);

//	bug("reading an object", 0);

	if (obj == NULL) { /* either not found or old style */
		bug("obj is null!", 0);
		obj = new_obj();
		obj->name               = str_dup("");
		obj->short_descr        = str_dup("");
		obj->description        = str_dup("");
	}

	for (cJSON *o = json->child; o; o = o->next) {
		char *key = o->string;
		bool fMatch = FALSE;
//		int count = 0; // convenience variable to compact this list, resets with every item

		switch (toupper(key[0])) {
			case 'A':
				if (!str_cmp(key, "Affc")) {
					for (cJSON *item = o->child; item != NULL; item = item->next) {
						int sn = skill_lookup(cJSON_GetObjectItem(item, "name")->valuestring);

						if (sn < 0) {
							bug("Fread_char: unknown skill.", 0);
							continue;
						}

						AFFECT_DATA *paf = new_affect();
						paf->type = sn;

						get_JSON_short(item, &paf->where, "where");
						get_JSON_short(item, &paf->level, "level");
						get_JSON_short(item, &paf->duration, "dur");
						get_JSON_short(item, &paf->modifier, "mod");
						get_JSON_short(item, &paf->location, "loc");
						get_JSON_int(item, &paf->bitvector, "bitv");
						get_JSON_short(item, &paf->evolution, "evo");

						paf->next       = obj->affected;
						obj->affected    = paf;
					}
					fMatch = TRUE; break;
				}
				break;
			case 'C':
				if (!str_cmp(key, "Contains")) {
					// this mirrors code for fread_objects, but uses obj_to_obj instead of obj_to_char/locker/strongbox,
					// so the function pointer doesn't work.  maybe find a way to fix and condense?
					for (cJSON *item = o->child; item; item = item->next) {
						OBJ_DATA *content = fread_obj(item, version);

						if (content->pIndexData) {
							if (content->condition == 0)
								content->condition = content->pIndexData->condition;

							obj_to_obj(content, obj);
						}
						else {
							// deal with contents and extract
							while (content->contains) {
								OBJ_DATA *c = content->contains;
								content->contains = c->next_content;
								obj_to_obj(c, obj);
							}

							free_obj(content);
						}
					}
					fMatch = TRUE; break;
				}

				INTKEY("Cond",			obj->condition,				o->valueint);
				INTKEY("Cost",			obj->cost,					o->valueint);
				break;
			case 'D':
				STRKEY("Desc",			obj->description,			o->valuestring);
				break;
			case 'E':
				if (!str_cmp(key, "ExDe")) {
					for (cJSON *item = o->child; item; item = item->next) {
						EXTRA_DESCR_DATA *ed = new_extra_descr();
						ed->keyword             = str_dup(item->string);
						ed->description         = str_dup(item->valuestring);
						ed->next                = obj->extra_descr;
						obj->extra_descr        = ed;
					}
					fMatch = TRUE; break;
				}

				INTKEY("Enchanted",		obj->enchanted,				o->valueint);
				INTKEY("ExtF",			obj->extra_flags,			o->valueint);
				break;
			case 'I':
				INTKEY("Ityp",			obj->item_type,				o->valueint);
				break;
			case 'L':
				INTKEY("Lev",			obj->level,					o->valueint);
				break;
			case 'M':
				STRKEY("Mat",			obj->material,				o->valuestring);
				break;
			case 'N':
				STRKEY("Name",			obj->name,					o->valuestring);
				break;
			case 'S':
				if (!str_cmp(key, "Spell")) {
					for (cJSON *item = o->child; item; item = item->next) {
						int slot = atoi(item->string);
						int sn = skill_lookup(item->valuestring);

						if (slot < 0 || slot > 4)
							bug("Fread_obj: bad iValue %d.", slot);
						else if (sn < 0)
							bug("Fread_obj: unknown skill.", 0);
						else
							obj->value[slot] = sn;
					}
					fMatch = TRUE; break;
				}

				if (!str_cmp(key, "Splxtra")) {
					int count = 0;
					for (cJSON *item = o->child; item; item = item->next, count++) {
						obj->spell[count] = skill_lookup(cJSON_GetObjectItem(item, "name")->valuestring);
						obj->spell_lev[count] = cJSON_GetObjectItem(item, "level")->valueint;
					}
					fMatch = TRUE; break;
				}

				STRKEY("ShD",			obj->short_descr,			o->valuestring);
				break;
			case 'T':
				INTKEY("Time",			obj->timer,					o->valueint);
				break;
			case 'V':
				if (!str_cmp(key, "Val")) {
					int slot = 0;
					for (cJSON *item = o->child; item; item = item->next, slot++)
						obj->value[slot] = item->valueint;
					fMatch = TRUE; break;
				}

				SKIPKEY("Vnum");
				break;
			case 'W':
				INTKEY("WeaF",			obj->wear_flags,			o->valueint); // no, not read_flags
				INTKEY("Wear",			obj->wear_loc,				o->valueint);
				INTKEY("Wt",			obj->weight,				o->valueint);
				break;
			default:
				break;
		}

		if (!fMatch)
			bugf("fread_obj: unknown key %s", key);
	}

	return obj;
}

// read a list of objects and return the head
void fread_objects(CHAR_DATA *ch, cJSON *contains, void (*obj_to)(OBJ_DATA *, CHAR_DATA *), int version) {
	if (contains == NULL)
		return;

	for (cJSON *item = contains->child; item; item = item->next) {
		OBJ_DATA *content = fread_obj(item, version);

		if (content->pIndexData) {
			if (content->condition == 0)
				content->condition = content->pIndexData->condition;

			(*obj_to)(content, ch);
		}
		else {
			// deal with contents and extract
			while (content->contains) {
				OBJ_DATA *c = content->contains;
				content->contains = c->next_content;
				(*obj_to)(c, ch);
			}

			free_obj(content);
		}
	}
}

/* load a pet from the forgotten reaches */
void fread_pet(CHAR_DATA *ch, cJSON *json, int version)
{
	cJSON *o;

	if (json == NULL)
		return;

	int vnum;

	// error compensation in case their mob goes away, don't poof inventory
	if ((o = cJSON_GetObjectItem(json, "Vnum")) != NULL) {
		vnum = o->valueint;
	}
	else {
		bug("fread_pet: no vnum field in JSON object", 0);
		vnum = MOB_VNUM_FIDO;
	}

	MOB_INDEX_DATA *index = get_mob_index(vnum);

	if (index == NULL) {
		bug("Fread_pet: bad vnum %d in fread_pet().", vnum);
		index = get_mob_index(MOB_VNUM_FIDO);
	}

	CHAR_DATA *pet = create_mobile(index);

	/* Check for memory error. -- Outsider */
	if (!pet) {
		bug("Memory error creating mob in fread_pet().", 0);
		return;
	}

	fread_char(pet, json, version);

	pet->leader = ch;
	pet->master = ch;
	ch->pet = pet;

	/* adjust hp mana stamina up  -- here for speed's sake */
	int percent;
	percent = (current_time - ch->pcdata->last_logoff) * 25 / (2 * 60 * 60);
	percent = UMIN(percent, 100);

	if (percent > 0 && !IS_AFFECTED(ch, AFF_POISON)
	    &&  !IS_AFFECTED(ch, AFF_PLAGUE)) {
		pet->hit    += (pet->max_hit - pet->hit) * percent / 100;
		pet->mana   += (pet->max_mana - pet->mana) * percent / 100;
		pet->stam   += (pet->max_stam - pet->stam) * percent / 100;
	}

	reset_char(pet);
}



/*
 * This function works just like ctime() does on current Linux systems.
 * I am only implementing it to make sure that dizzy_scantime(), which
 * decodes the output from ctime() and dizzy_ctime(), will always work
 * even if the system on which this code is run implements ctime()
 * differently.
 *
 * The output format for dizzy_ctime() is like this:
 *      Wed Jun 30 21:49:08 1993\n
 *
 * Like ctime(), dizzy_ctime() writes to a static string which will change
 * with the next invocation of dizzy_ctime().
 */

static const char *day_names[] =
{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const char *month_names[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

char *dizzy_ctime(time_t *timep)
{
	static char ctime_buf[40];
	struct tm loc_tm;
	loc_tm = *localtime(timep);
	sprintf(ctime_buf, "%s %s %02d %02d:%02d:%02d %04d\n",
	        day_names[loc_tm.tm_wday],
	        month_names[loc_tm.tm_mon],
	        loc_tm.tm_mday,
	        loc_tm.tm_hour, loc_tm.tm_min, loc_tm.tm_sec,
	        1900 + loc_tm.tm_year);
	return ctime_buf;
} /* end dizzy_ctime() */

/*
 * decode a time string as produced by dizzy_ctime()
 * Day of week is scanned in spite of not being needed so that the
 * return value from sprintf() will be significant.
 */
time_t dizzy_scantime(char *ctime)
{
	char cdow[4], cmon[4];
	int year, month, day, hour, minute, second;
	char msg[MAX_INPUT_LENGTH];
	struct tm loc_tm;
	/* this helps initialize local-dependent stuff like TZ, etc. */
	loc_tm = *localtime(&current_time);

	if (sscanf(ctime, " %3s %3s %d %d:%d:%d %d",
	           cdow, cmon, &day, &hour, &minute, &second, &year) < 7) {
		sprintf(msg, "dizzy_scantime(): Error scanning date/time: '%s'", ctime);
		bug(msg, 0);
		goto endoftime;
	}

	for (month = 0; month < 12; month++) {
		if (!str_prefix1(month_names[month], ctime + 4))
			break;
	}

	if (month >= 12) {
		sprintf(msg, "dizzy_scantime(): Bad month in %s", ctime);
		bug(msg, 0);
		goto endoftime;
	}

	loc_tm.tm_mon  = month;
	loc_tm.tm_mday = day;
	loc_tm.tm_hour = hour;
	loc_tm.tm_min  = minute;
	loc_tm.tm_sec  = second;
	loc_tm.tm_year = year - 1900;
endoftime:
	return mktime(&loc_tm);
} /* end dizzy_scantime() */

void do_finger(CHAR_DATA *ch, char *argument)
{
	char filename[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char key[MAX_STRING_LENGTH];
	FILE *pfile = NULL;
	bool in_player = FALSE;
	long pos;
	BUFFER *dbuf = NULL;
	/* the following vars are read from the player file */
	char *email, *fingerinfo, *last_lsite, *name, *title, *spouse, *race, *deity;
	int class, pks, pkd, pkr, aks, akd, level, rmct;
	long cgroup = 0L, plr = 0L;
	time_t last_ltime, last_saved;
	CLAN_DATA *clan = NULL;
	one_argument(argument, arg);

	if (!arg[0]) {
		stc("Syntax:\n"
		    "  {Yfinger{x {Gplayer-name{x : displays info about {Gplayer-name{x\n"
		    "  {Yfinger{X {Gplayer-name{x@{Gmud{x : displays info about player on other muds.\n"
		    "  {YFinger private{x     : hides your e-mail address from FINGER\n\n"
		    "  {YShowlast{x           : hides your last login/save times\n", ch);
		return;
	}

	if (!str_cmp(arg, "private")) {
		if (IS_SET(ch->pcdata->plr, PLR_SHOWEMAIL)) {
			stc("Your email will no longer display in your finger info.\n", ch);
			REMOVE_BIT(ch->pcdata->plr, PLR_SHOWEMAIL);
		}
		else {
			stc("Your email will now display in your finger info.\n", ch);
			SET_BIT(ch->pcdata->plr, PLR_SHOWEMAIL);
		}

		do_save(ch, "");
		return;
	}

	if (has_slash(arg)) {
		stc("That is not a valid player name.\n", ch);
		return;
	}

	/* initialize variables */
	email = fingerinfo = last_lsite = name = title = spouse = race = deity = str_empty;
	class = pks = pkd = pkr = aks = akd = level = rmct = 0;
	sprintf(filename, "%s%s", PLAYER_DIR, capitalize(arg));
	pfile = fopen(filename, "r");

	if (! pfile) {
		stc("That player does not exist.\n", ch);
		return;
	}

#if defined(KEY)
#undef KEY
#endif
#if defined(STRKEY)
#undef STRKEY
#endif
#if defined(GOBACK)
#undef GOBACK
#endif
#define GOBACK do { fseek(pfile, pos, SEEK_SET); (void) fread_word(pfile); } while (FALSE)
#define KEY(keywd, var, func) if (!str_cmp(key, keywd)) { GOBACK; var = func; }
#define STRKEY(keywd, var, func)                        \
	if (!str_cmp(key, keywd))                       \
	{                                               \
		char *tmp;                              \
		GOBACK;                                 \
		var = func(tmp = fread_string(pfile));  \
		free_string(tmp);                       \
	}

	/* scan player file for '#PLAYER' */
	for (; ;) {
		if (feof(pfile))
			break;

		pos = ftell(pfile);

		if (fgets(buf, sizeof(buf), pfile) == NULL)
			break;

		/* skip continued strings */
		if (buf[0] == '\0' || buf[0] == '\r')
			continue;

		/* stoopid fgets() leaves trailing newline in buffer */
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = '\0';

		if (!str_cmp(buf, "#PLAYER")) {
			in_player = TRUE;
			continue;
		}

		if (!in_player)
			continue;

		/* Now in #PLAYER section. Process only keywords of interest. */
		one_argument(buf, key);

		if (!str_cmp(key, "End"))
			break;
		else    KEY("Cgrp",     cgroup,         fread_flag(pfile))
			else STRKEY("Clan",     clan,           clan_lookup)
				else    KEY("Email",    email,          fread_string(pfile))
					else    KEY("Finf",     fingerinfo,     fread_string(pfile))
						else STRKEY("LSav",     last_saved,     dizzy_scantime)
							else    KEY("Levl",     level,          fread_number(pfile))
								else    KEY("Lsit",     last_lsite,     fread_string(pfile))
									else STRKEY("Ltim",     last_ltime,     dizzy_scantime)
										else    KEY("Name",     name,           fread_string(pfile))
											else    KEY("Plr",      plr,            fread_flag(pfile))
												else    KEY("RmCt",     rmct,           fread_number(pfile))
													else    KEY("Spou",     spouse,         fread_string(pfile))
														else    KEY("Titl",     title,          fread_string(pfile))
															else    KEY("Deit",     deity,          fread_string(pfile))
																else    KEY("Cla",      class,          fread_number(pfile))
																	else    KEY("Race",     race,           fread_string(pfile))
																		else    KEY("PCkills",  pks,            fread_number(pfile))
																			else    KEY("PCkilled", pkd,            fread_number(pfile))
																				else    KEY("Akills",   aks,            fread_number(pfile))
																					else    KEY("Akilled",  akd,            fread_number(pfile))
																						else    KEY("PKRank",   pkr,            fread_number(pfile));
	}

	/* ok to leave pfile open, will be closed below */
#undef KEY
#undef STRKEY
#undef GOBACK

	if (!in_player) {
		/* hit EOF without finding #PLAYER */
		sprintf(buf, "do_finger(): bad pfile '%s'\n", arg);
		bug(buf, 0);
		ptc(ch, "No information available about '%s'\n", arg);

		// goto bombout;
		if (pfile) fclose(pfile);

		return;
	}

	if (title[0] != '.' && title[0] != ',' &&  title[0] != '!' && title[0] != '?') {
		sprintf(buf, " %s{x", title);
		free_string(title);
		title = str_dup(buf);
	}

	/* display information */
	dbuf = new_buf();

	if (level >= LEVEL_IMMORTAL)
		sprintf(buf, "{W[{CIMM{W] %s%s{x\n", name, title);
	else if (rmct == 0)
		sprintf(buf, "{W[{B%2d{W] %s%s{x\n", level, name, title);
	else
		sprintf(buf, "{W[{GR%d{T/{B%2d{W] %s%s{x\n", rmct, level, name, title);

	add_buf(dbuf, buf);

	if (clan) {
		if (IS_SET(cgroup, GROUP_LEADER))
			sprintf(buf, "{BLeader of ");
		else if (IS_SET(cgroup, GROUP_DEPUTY))
			sprintf(buf, "{BDeputy of ");
		else
			sprintf(buf, "{BMember of ");

		add_buf(dbuf, buf);
		sprintf(buf, "%s{x\n", clan->clanname);
		add_buf(dbuf, buf);
	}

	sprintf(buf, "{C%s ", capitalize(race));
	add_buf(dbuf, buf);
	sprintf(buf, "{C%s, follower of %s{x\n", capitalize(class_table[class].name), deity);
	add_buf(dbuf, buf);
	sprintf(buf, "{GArena Record:    %d wins,  %d losses{x\n", aks, akd);
	add_buf(dbuf, buf);
	sprintf(buf, "{PBlood Trail (%d): %d kills, %d deaths{x\n\n", pkr, pks, pkd);
	add_buf(dbuf, buf);

	if (fingerinfo[0]) {
		sprintf(buf, "{CAdditional Info:{x\n%s{x\n", fingerinfo);
		add_buf(dbuf, buf);
	}

	if (spouse[0]) {
		if (!IS_SET(plr, PLR_MARRIED))
			sprintf(buf, "{Y%s is engaged to %s.{x\n", name, spouse);
		else
			sprintf(buf, "{Y%s is happily married to %s.{x\n", name, spouse);

		add_buf(dbuf, buf);
	}

	if (email[0] && (IS_IMMORTAL(ch) || IS_SET(plr, PLR_SHOWEMAIL))) {
		sprintf(buf, "{GEmail: %s{x\n", email);
		add_buf(dbuf, buf);
	}

	if (IS_IMMORTAL(ch) || !IS_SET(plr, PLR_NOSHOWLAST)) {
		if (last_ltime) {
			sprintf(buf, "{HLast Login : %s\r{x", dizzy_ctime(&last_ltime));
			add_buf(dbuf, buf);
		}

		if (last_saved) {
			sprintf(buf, "{HLast Saved : %s\r{x", dizzy_ctime(&last_saved));
			add_buf(dbuf, buf);
		}
	}

	if (IS_IMP(ch)) {
		sprintf(buf, "{HLast Site  : %s{x\n", last_lsite);
		add_buf(dbuf, buf);
	}

	add_buf(dbuf, "\n");
	page_to_char(buf_string(dbuf), ch);
	free_buf(dbuf);
	/* clean up dup'd strings */
	free_string(email);
	free_string(fingerinfo);
	free_string(last_lsite);
	free_string(name);
	free_string(title);
	/* I think these should also be cleaned. -- Outsider */
	free_string(spouse);
	free_string(race);
	free_string(deity);

	/*
	I really don't like goto commands. -- Outsider

	bombout:
	*/
	if (pfile != NULL)
		fclose(pfile);

	return;
} /* end do_finger() */

