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
#include "interp.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "cJSON.h"
#include "affect.h"
#include "Format.hpp"

extern  int     _filbuf         args((FILE *));
extern void     goto_line       args((CHAR_DATA *ch, int row, int column));
extern void     set_window      args((CHAR_DATA *ch, int top, int bottom));

#define CURRENT_VERSION         17   /* version number for pfiles */

bool debug_json = FALSE;

/* Locals */

int rename(const char *oldfname, const char *newfname);

/*
 * Local functions.
 */
cJSON * fwrite_player     args((CHAR_DATA *ch));
cJSON * fwrite_char     args((CHAR_DATA *ch));
cJSON * fwrite_objects  args((CHAR_DATA *ch,  OBJ_DATA *head, bool strongbox));
cJSON * fwrite_pet      args((CHAR_DATA *pet));
void    fread_char      args((CHAR_DATA *ch,  cJSON *json, int version));
void    fread_player      args((CHAR_DATA *ch,  cJSON *json, int version));
void    fread_pet       args((CHAR_DATA *ch,  cJSON *json, int version));
void	fread_objects	args((CHAR_DATA *ch, cJSON *json, void (*obj_to)(OBJ_DATA *, CHAR_DATA *), int version));

// external
bool check_parse_name(const String& name);

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj(CHAR_DATA *ch)
{
	char strsave[MIL];
	FILE *fp;

	if (ch == NULL || IS_NPC(ch))
		return;

	if (ch->desc != NULL && ch->desc->original != NULL)
		ch = ch->desc->original;

	ch->pcdata->last_saved = current_time;

	cJSON *root = cJSON_CreateObject();

	cJSON_AddNumberToObject(root, "version", CURRENT_VERSION);
	cJSON_AddItemToObject(root, "player", fwrite_player(ch));
	cJSON_AddItemToObject(root, "character", fwrite_char(ch));

	cJSON_AddItemToObject(root, "inventory", fwrite_objects(ch, ch->carrying, FALSE));
	cJSON_AddItemToObject(root, "locker", fwrite_objects(ch, ch->pcdata->locker, FALSE));
	cJSON_AddItemToObject(root, "strongbox", fwrite_objects(ch, ch->pcdata->strongbox, TRUE));

	if (ch->pet) {
		cJSON_AddItemToObject(root, "pet", fwrite_pet(ch->pet));
		cJSON_AddItemToObject(root, "pet_inventory", fwrite_objects(ch, ch->pet->carrying, FALSE));
	}

	char *JSONstring = cJSON_Print(root);
	cJSON_Delete(root);

	// added if to avoid closing invalid file
	String buf;
	one_argument(ch->name, buf);
	Format::sprintf(strsave, "%s%s", PLAYER_DIR, buf.capitalize());

	if ((fp = fopen(TEMP_FILE, "w")) != NULL) {
		fputs(JSONstring, fp);
		fclose(fp);
		rename(TEMP_FILE, strsave);
	}
	else {
		bug("Save_char_obj: fopen", 0);
		perror(strsave);
	}

	update_pc_index(ch, FALSE);
}

void backup_char_obj(CHAR_DATA *ch)
{
	save_char_obj(ch);

	char strsave[MIL], strback[MIL];
	String buf;
	one_argument(ch->name, buf);

	Format::sprintf(strsave, "%s%s", PLAYER_DIR, buf.capitalize());
	Format::sprintf(strback, "%s%s", BACKUP_DIR, buf.capitalize());

	Format::sprintf(buf, "cp %s %s", strsave, strback);
	system(buf.c_str());
	Format::sprintf(buf, "gzip -fq %s", strback);
	system(buf.c_str());
} /* end backup_char_obj() */

cJSON *fwrite_player(CHAR_DATA *ch)
{
	cJSON *item;
	cJSON *o = cJSON_CreateObject(); // object to return

	item = NULL;

	if (!ch->pcdata->alias.empty()) {
		item = cJSON_CreateArray();

		for (auto it = ch->pcdata->alias.cbegin(); it != ch->pcdata->alias.cend(); it++) {
			cJSON *alias = cJSON_CreateArray();
			cJSON_AddItemToArray(alias, cJSON_CreateString((*it).first.c_str()));
			cJSON_AddItemToArray(alias, cJSON_CreateString((*it).second.c_str()));
			cJSON_AddItemToArray(item, alias);
		}

		cJSON_AddItemToObject(o,	"Alias",		item);
	}

	if (ch->pcdata->afk[0] != '\0')
		JSON::addStringToObject(o,	"Afk",			ch->pcdata->afk);

	cJSON_AddNumberToObject(o,		"Akills",		ch->pcdata->arenakills);
	cJSON_AddNumberToObject(o,		"Akilled",		ch->pcdata->arenakilled);

	if (ch->pcdata->aura[0])
		JSON::addStringToObject(o,	"Aura",			ch->pcdata->aura);

	cJSON_AddNumberToObject(o,		"Back",			ch->pcdata->backup);

	if (ch->pcdata->bamfin[0] != '\0')
		JSON::addStringToObject(o,	"Bin",			ch->pcdata->bamfin);

	if (ch->pcdata->bamfout[0] != '\0')
		JSON::addStringToObject(o,	"Bout",			ch->pcdata->bamfout);

	JSON::addStringToObject(o,		"Cgrp",			flags_to_string(ch->pcdata->cgroup));

	item = cJSON_CreateObject();
	cJSON_AddNumberToObject(item,	"drunk",		ch->pcdata->condition[COND_DRUNK]);
	cJSON_AddNumberToObject(item,	"full",			ch->pcdata->condition[COND_FULL]);
	cJSON_AddNumberToObject(item,	"thirst",		ch->pcdata->condition[COND_THIRST]);
	cJSON_AddNumberToObject(item,	"hunger",		ch->pcdata->condition[COND_HUNGER]);
	cJSON_AddItemToObject(o, 		"Cnd",	 		item);

	item = cJSON_CreateArray();
	for (int pos = 0; pos < MAX_COLORS; pos++) {
		if (ch->pcdata->color[pos] <= 0)
			continue;

		cJSON *p = cJSON_CreateObject();
		cJSON_AddNumberToObject(p, "slot", pos);
		cJSON_AddNumberToObject(p, "color", ch->pcdata->color[pos]);
		cJSON_AddNumberToObject(p, "bold", ch->pcdata->bold[pos]);
		cJSON_AddItemToArray(item, p);
	}
	cJSON_AddItemToObject(o,		"Colr",			item);

	if (ch->pcdata->deity[0])
		JSON::addStringToObject(o,	"Deit",			ch->pcdata->deity);

	if (ch->pcdata->email[0] != '\0')
		JSON::addStringToObject(o,	"Email",		ch->pcdata->email);

	cJSON_AddNumberToObject(o,		"Familiar",		ch->pcdata->familiar);

	if (ch->pcdata->fingerinfo[0] != '\0')
		JSON::addStringToObject(o,	"Finf",			ch->pcdata->fingerinfo);

	if (ch->pcdata->flag_killer)
		cJSON_AddNumberToObject(o,	"FlagKiller",	ch->pcdata->flag_killer);

	if (ch->pcdata->flag_thief)
		cJSON_AddNumberToObject(o,	"FlagThief",	ch->pcdata->flag_thief);

	if (ch->pcdata->gamein[0])
		JSON::addStringToObject(o,	"GameIn",		ch->pcdata->gamein);

	if (ch->pcdata->gameout[0])
		JSON::addStringToObject(o,	"GameOut",		ch->pcdata->gameout);

	item = NULL;
	for (int gn = 0; gn < MAX_GROUP; gn++) {
		if (group_table[gn].name == NULL || ch->pcdata->group_known[gn] == 0)
			continue;

		if (item == NULL)
			item = cJSON_CreateArray();

		cJSON_AddItemToArray(item, cJSON_CreateString(group_table[gn].name));
	}
	if (item != NULL)
		cJSON_AddItemToObject(o,	"Gr",			item);

	if (!ch->pcdata->ignore.empty()) {
		item = cJSON_CreateArray();

		for (auto it = ch->pcdata->ignore.cbegin(); it != ch->pcdata->ignore.cend(); it++)
			cJSON_AddItemToArray(item, cJSON_CreateString((*it).c_str()));

		cJSON_AddItemToObject(o,	"Ignore",		item);
	}

	if (ch->pcdata->immname[0])
		JSON::addStringToObject(o,	"Immn",			ch->pcdata->immname);
	if (ch->pcdata->immprefix[0])
		JSON::addStringToObject(o,	"Immp",			ch->pcdata->immprefix);

	if (ch->cls == PALADIN_CLASS) {
		cJSON_AddNumberToObject(o,	"Lay",			ch->pcdata->lays);
		cJSON_AddNumberToObject(o,	"Lay_Next",		ch->pcdata->next_lay_countdown);
	}

	cJSON_AddNumberToObject(o,		"LLev",			ch->pcdata->last_level);
	cJSON_AddNumberToObject(o,		"LogO",			current_time);

	if (ch->pcdata->last_lsite[0])
		JSON::addStringToObject(o,	"Lsit",			ch->pcdata->last_lsite);

	JSON::addStringToObject(o,		"Ltim",			dizzy_ctime(&ch->pcdata->last_ltime));
	JSON::addStringToObject(o,		"LSav",			dizzy_ctime(&ch->pcdata->last_saved));

	if (ch->pcdata->mark_room)
		cJSON_AddNumberToObject(o,	"Mark",			ch->pcdata->mark_room);

	cJSON_AddNumberToObject(o,		"Mexp",			ch->pcdata->mud_exp);

	item = cJSON_CreateObject();
	cJSON_AddNumberToObject(item,	"note",			ch->pcdata->last_note);
	cJSON_AddNumberToObject(item,	"idea",			ch->pcdata->last_idea);
	cJSON_AddNumberToObject(item,	"role",			ch->pcdata->last_roleplay);
	cJSON_AddNumberToObject(item,	"quest",		ch->pcdata->last_immquest);
	cJSON_AddNumberToObject(item,	"change",		ch->pcdata->last_changes);
	cJSON_AddNumberToObject(item,	"pers",			ch->pcdata->last_personal);
	cJSON_AddNumberToObject(item,	"trade",		ch->pcdata->last_trade);
	cJSON_AddItemToObject(o, 		"Note", 		item);

	JSON::addStringToObject(o,		"Pass",			ch->pcdata->pwd);
	cJSON_AddNumberToObject(o,		"PCkills",		ch->pcdata->pckills);
	cJSON_AddNumberToObject(o,		"PCkilled",		ch->pcdata->pckilled);
	cJSON_AddNumberToObject(o,		"PKRank",		ch->pcdata->pkrank);
	JSON::addStringToObject(o,		"Plr",			flags_to_string(ch->pcdata->plr));
	cJSON_AddNumberToObject(o,		"Plyd",			ch->pcdata->played);
	cJSON_AddNumberToObject(o,		"Pnts",			ch->pcdata->points);

	if (!ch->pcdata->query.empty()) {
		item = cJSON_CreateArray();

		for (auto it = ch->pcdata->query.cbegin(); it != ch->pcdata->query.cend(); it++)
			cJSON_AddItemToArray(item, cJSON_CreateString((*it).c_str()));

		cJSON_AddItemToObject(o,	"Query",		item);
	}

	if (ch->pcdata->rank[0])
		JSON::addStringToObject(o,	"Rank",			ch->pcdata->rank);

	if (ch->pcdata->rolepoints)
		cJSON_AddNumberToObject(o,	"RolePnts",		ch->pcdata->rolepoints);

	item = NULL;
	for (int sn = 0; sn < MAX_SKILL; sn++) {
		if (skill_table[sn].name == NULL)
			break;

		if (ch->pcdata->learned[sn] <= 0)
			continue;

		if (item == NULL)
			item = cJSON_CreateArray();

		if (ch->pcdata->evolution[sn] < 1)
			ch->pcdata->evolution[sn] = 1;
		else if (ch->pcdata->evolution[sn] > 4)
			ch->pcdata->evolution[sn] = 4;

		cJSON *sk = cJSON_CreateObject();
		JSON::addStringToObject(sk, "name", skill_table[sn].name);
		cJSON_AddNumberToObject(sk, "prac", ch->pcdata->learned[sn]);
		cJSON_AddNumberToObject(sk, "evol", ch->pcdata->evolution[sn]);
		cJSON_AddItemToArray(item, sk);
	}
	if (item != NULL)
		cJSON_AddItemToObject(o,	"Sk",			item);

	if (ch->pcdata->skillpoints)
		cJSON_AddNumberToObject(o,	"SkillPnts",	ch->pcdata->skillpoints);

	if (ch->pcdata->spouse[0])
		JSON::addStringToObject(o,	"Spou",			ch->pcdata->spouse);

	if (ch->pcdata->nextsquest)
		cJSON_AddNumberToObject(o,	"SQuestNext",	ch->pcdata->nextsquest);
	else if (ch->pcdata->sqcountdown)
		cJSON_AddNumberToObject(o,	"SQuestNext",	20);

	if (ch->pcdata->remort_count > 0) {
		if (ch->pcdata->status[0])
			JSON::addStringToObject(o,	"Stus",		ch->pcdata->status);

		cJSON_AddNumberToObject(o,	"RmCt",			ch->pcdata->remort_count);

		item = NULL;
		for (int i = 0; i < (ch->pcdata->remort_count / EXTRACLASS_SLOT_LEVELS) + 1; i++) {
			if (ch->pcdata->extraclass[i] == 0)
				break;

			if (item == NULL)
				item = cJSON_CreateArray();

			cJSON_AddItemToArray(item,
				cJSON_CreateString(skill_table[ch->pcdata->extraclass[i]].name));
		}

		if (item != NULL)
			cJSON_AddItemToObject(o, "ExSk", item);

		item = cJSON_CreateIntArray(ch->pcdata->raffect, ch->pcdata->remort_count / 10 + 1);
		cJSON_AddItemToObject(o, "Raff", item);
	}

	item = cJSON_CreateObject();
	cJSON_AddNumberToObject(item,	"hit",			ch->pcdata->trains_to_hit);
	cJSON_AddNumberToObject(item,	"mana",			ch->pcdata->trains_to_mana);
	cJSON_AddNumberToObject(item,	"stam",			ch->pcdata->trains_to_stam);
	cJSON_AddItemToObject(o, 		"THMS",	 		item);

	if (ch->pcdata->title[0])
		JSON::addStringToObject(o,	"Titl",			ch->pcdata->title[0] == ' ' ?
		ch->pcdata->title.substr(1) : ch->pcdata->title);
	JSON::addStringToObject(o,		"Video",		flags_to_string(ch->pcdata->video));

	if (ch->pcdata->whisper[0])
		JSON::addStringToObject(o,	"Wspr",			ch->pcdata->whisper);

	return o;
}

/*
 * Write the char.
 */
cJSON *fwrite_char(CHAR_DATA *ch)
{
	cJSON *item;
	cJSON *o = cJSON_CreateObject(); // object to return

	JSON::addStringToObject(o,		"Act",			flags_to_string(ch->act));

	item = NULL;
	for (const AFFECT_DATA *paf = affect_list_char(ch); paf != NULL; paf = paf->next) {
		if (paf->type < 0 || paf->type >= MAX_SKILL)
			continue;

		// don't write permanent affects, rebuild them from race and raffects on load
		if (paf->permanent)
			continue;

		if (item == NULL)
			item = cJSON_CreateArray();

		cJSON *aff = cJSON_CreateObject();
		JSON::addStringToObject(aff, "name", skill_table[paf->type].name);
		cJSON_AddNumberToObject(aff, "where", paf->where);
		cJSON_AddNumberToObject(aff, "level", paf->level);
		cJSON_AddNumberToObject(aff, "dur", paf->duration);
		cJSON_AddNumberToObject(aff, "mod", paf->modifier);
		cJSON_AddNumberToObject(aff, "loc", paf->location);
		cJSON_AddNumberToObject(aff, "bitv", paf->bitvector);
		cJSON_AddNumberToObject(aff, "evo", paf->evolution);
		cJSON_AddItemToArray(item, aff);
	}
	if (item != NULL)
		cJSON_AddItemToObject(o,	"Affc",			item);

	cJSON_AddNumberToObject(o,		"Alig",			ch->alignment);

	item = cJSON_CreateObject();
	cJSON_AddNumberToObject(item,	"str",			ATTR_BASE(ch, APPLY_STR));
	cJSON_AddNumberToObject(item,	"int",			ATTR_BASE(ch, APPLY_INT));
	cJSON_AddNumberToObject(item,	"wis",			ATTR_BASE(ch, APPLY_WIS));
	cJSON_AddNumberToObject(item,	"dex",			ATTR_BASE(ch, APPLY_DEX));
	cJSON_AddNumberToObject(item,	"con",			ATTR_BASE(ch, APPLY_CON));
	cJSON_AddNumberToObject(item,	"chr",			ATTR_BASE(ch, APPLY_CHR));
	cJSON_AddItemToObject(o, 		"Atrib", 		item);


	if (ch->clan)
		JSON::addStringToObject(o,	"Clan",			ch->clan->name);

	cJSON_AddNumberToObject(o,		"Cla",			ch->cls);
	JSON::addStringToObject(o,		"Cnsr",			flags_to_string(ch->censor));
	JSON::addStringToObject(o,		"Comm",			flags_to_string(ch->comm));

	if (ch->description[0])
		JSON::addStringToObject(o,	"Desc",			ch->description);

	cJSON_AddNumberToObject(o,		"Exp",			ch->exp);

	if (ch->gold_donated)
		cJSON_AddNumberToObject(o,	"GlDonated",	ch->gold_donated);

	cJSON_AddNumberToObject(o,		"Gold",			ch->gold);

	if (ch->gold_in_bank > 0)
		cJSON_AddNumberToObject(o,	"Gold_in_bank",	ch->gold_in_bank);

	item = cJSON_CreateObject();
	cJSON_AddNumberToObject(item,	"hit",			ch->hit);
	cJSON_AddNumberToObject(item,	"mana",			ch->mana);
	cJSON_AddNumberToObject(item,	"stam",			ch->stam);
	cJSON_AddItemToObject(o, 		"HMS",	 		item);

	item = cJSON_CreateObject();
	cJSON_AddNumberToObject(item,	"hit",			ATTR_BASE(ch, APPLY_HIT));
	cJSON_AddNumberToObject(item,	"mana",			ATTR_BASE(ch, APPLY_MANA));
	cJSON_AddNumberToObject(item,	"stam",			ATTR_BASE(ch, APPLY_STAM));
	cJSON_AddItemToObject(o, 		"HMSP",	 		item);

	cJSON_AddNumberToObject(o,		"Id",			ch->id);
	cJSON_AddNumberToObject(o,		"Levl",			ch->level);

	if (ch->long_descr[0])
		JSON::addStringToObject(o,	"LnD",			ch->long_descr);

	JSON::addStringToObject(o,		"Name",			ch->name.c_str());
	cJSON_AddNumberToObject(o,		"Pos",			ch->position);
	cJSON_AddNumberToObject(o,              "PosP",                 ch->start_pos);
	cJSON_AddNumberToObject(o,		"Prac",			ch->practice);

	if (ch->prompt[0])
		JSON::addStringToObject(o,	"Prom",			ch->prompt);

	if (ch->questpoints_donated)
		cJSON_AddNumberToObject(o,	"QpDonated",	ch->questpoints_donated);

	if (ch->questpoints)
		cJSON_AddNumberToObject(o,	"QuestPnts",	ch->questpoints);

	if (ch->nextquest)
		cJSON_AddNumberToObject(o,	"QuestNext",	ch->nextquest);
	else if (ch->countdown)
		cJSON_AddNumberToObject(o,	"QuestNext",	12);

	JSON::addStringToObject(o,		"Race",			race_table[ch->race].name);
	JSON::addStringToObject(o,		"Revk",			flags_to_string(ch->revoke));
	cJSON_AddNumberToObject(o,		"Room",			
		(ch->in_room == get_room_index(ROOM_VNUM_LIMBO) && ch->was_in_room != NULL)
	        ? ch->was_in_room->vnum
	        : ch->in_room == NULL
	        ? 3001
	        : ch->in_room->vnum);

	cJSON_AddNumberToObject(o,		"Sex",			ATTR_BASE(ch, APPLY_SEX));
	cJSON_AddNumberToObject(o,		"Silv",			ch->silver);

	if (ch->silver_in_bank > 0)
		cJSON_AddNumberToObject(o,	"Silver_in_bank", ch->silver_in_bank);

	if (ch->short_descr[0])
		JSON::addStringToObject(o,	"ShD",			ch->short_descr);

	cJSON_AddNumberToObject(o,		"Trai",			ch->train);
	cJSON_AddNumberToObject(o,		"Wimp",			ch->wimpy);

	if (IS_IMMORTAL(ch)) { // why aren't these pcdata?
		JSON::addStringToObject(o,	"Wizn",			flags_to_string(ch->wiznet));
		cJSON_AddNumberToObject(o,	"Invi",			ch->invis_level);
		cJSON_AddNumberToObject(o,	"Lurk",			ch->lurk_level);
		cJSON_AddNumberToObject(o,	"Secu",			ch->secure_level);
	}

	return o;
}

/* write a pet */
cJSON *fwrite_pet(CHAR_DATA *pet)
{
	cJSON *o = fwrite_char(pet);

	cJSON_AddNumberToObject(o, "Vnum", pet->pIndexData->vnum);

	if (ATTR_BASE(pet, APPLY_SAVES) != 0)
		cJSON_AddNumberToObject(o, "Save", ATTR_BASE(pet, APPLY_SAVES));

	if (ATTR_BASE(pet, APPLY_HITROLL) != pet->pIndexData->hitroll)
		cJSON_AddNumberToObject(o, "Hit", ATTR_BASE(pet, APPLY_HITROLL));

	if (ATTR_BASE(pet, APPLY_DAMROLL) != pet->pIndexData->damage[DICE_BONUS])
		cJSON_AddNumberToObject(o, "Dam", ATTR_BASE(pet, APPLY_DAMROLL));

	return o;
}

/*
 * Write an object and its contents.
 */
cJSON *fwrite_obj(CHAR_DATA *ch, OBJ_DATA *obj, bool strongbox)
{
	/*
	 * Castrate storage characters.
	 */
/*	if (!IS_IMMORTAL(ch))
		if ((!strongbox && (obj->level > get_holdable_level(ch)))
		    || (obj->item_type == ITEM_KEY && (obj->value[0] == 0))
		    || (obj->item_type == ITEM_MAP && !obj->value[0]))
			return NULL;
*/
	cJSON *item;
	cJSON *o = cJSON_CreateObject();

	if (obj->condition != obj->pIndexData->condition)
		cJSON_AddNumberToObject(o,	"Cond",			obj->condition);
	if (obj->cost != obj->pIndexData->cost)
		cJSON_AddNumberToObject(o,	"Cost",			obj->cost);
	if (obj->description != obj->pIndexData->description)
		JSON::addStringToObject(o,	"Desc",			obj->description);

	if (affect_enchanted_obj(obj)) {
		// we could write an empty list here, for a disenchanted item
		item = cJSON_CreateArray();

		for (const AFFECT_DATA *paf = affect_list_obj(obj); paf != NULL; paf = paf->next) {
			if (paf->type >= MAX_SKILL)
				continue;

			cJSON *aff = cJSON_CreateObject();
			JSON::addStringToObject(aff, "name", skill_table[paf->type].name);
			cJSON_AddNumberToObject(aff, "where", paf->where);
			cJSON_AddNumberToObject(aff, "level", paf->level);
			cJSON_AddNumberToObject(aff, "dur", paf->duration);
			cJSON_AddNumberToObject(aff, "mod", paf->modifier);
			cJSON_AddNumberToObject(aff, "loc", paf->location);
			cJSON_AddNumberToObject(aff, "bitv", paf->bitvector);
			cJSON_AddNumberToObject(aff, "evo", paf->evolution);
			cJSON_AddItemToArray(item, aff);
		}

		cJSON_AddItemToObject(o,	"Affc",			item);
	}

	item = NULL;
	for (EXTRA_DESCR_DATA *ed = obj->extra_descr; ed != NULL; ed = ed->next) {
		if (item == NULL)
			item = cJSON_CreateObject();

		JSON::addStringToObject(item, ed->keyword, ed->description);
	}
	if (item != NULL)
		cJSON_AddItemToObject(o,	"ExDe",			item);

	if (obj->extra_flags != obj->pIndexData->extra_flags)
		cJSON_AddNumberToObject(o,	"ExtF",			obj->extra_flags);
	if (obj->item_type != obj->pIndexData->item_type)
		cJSON_AddNumberToObject(o,	"Ityp",			obj->item_type);
	if (obj->level != obj->pIndexData->level)
		cJSON_AddNumberToObject(o,	"Lev",			obj->level);
	if (obj->material != obj->pIndexData->material)
		JSON::addStringToObject(o,	"Mat",			obj->material);
	if (obj->name != obj->pIndexData->name)
		JSON::addStringToObject(o,	"Name",			obj->name);
	if (obj->short_descr != obj->pIndexData->short_descr)
		JSON::addStringToObject(o,	"ShD",			obj->short_descr);

	if (obj->timer != 0)
		cJSON_AddNumberToObject(o,	"Time",			obj->timer);

	if (obj->value[0] != obj->pIndexData->value[0]
	    ||  obj->value[1] != obj->pIndexData->value[1]
	    ||  obj->value[2] != obj->pIndexData->value[2]
	    ||  obj->value[3] != obj->pIndexData->value[3]
	    ||  obj->value[4] != obj->pIndexData->value[4])
	    cJSON_AddItemToObject(o,	"Val",			cJSON_CreateIntArray(obj->value, 5));

	cJSON_AddNumberToObject(o,		"Vnum",			obj->pIndexData->vnum);

	if (obj->wear_loc != WEAR_NONE)
		cJSON_AddNumberToObject(o,	"Wear",			obj->wear_loc);

	if (obj->wear_flags != obj->pIndexData->wear_flags)
		cJSON_AddNumberToObject(o,	"WeaF",			obj->wear_flags);
	if (obj->weight != obj->pIndexData->weight)
		cJSON_AddNumberToObject(o,	"Wt",			obj->weight);

	// does nothing if the contains is NULL
	cJSON_AddItemToObject(o, "contains", fwrite_objects(ch, obj->contains, strongbox));

	return o;
} /* end fwrite_obj() */

cJSON *fwrite_objects(CHAR_DATA *ch, OBJ_DATA *head, bool strongbox) {
	cJSON *array = cJSON_CreateArray();

	// old way was to use recursion to write items in reverse order, so loading would
	// be in the original order.  same concept here, except no recursion; we just
	// take advantage of the linked list underlying the cJSON array and insert at
	// index 0, so the array is written backwards.
	for (OBJ_DATA *obj = head; obj; obj = obj->next_content) {
		cJSON *item = fwrite_obj(ch, obj, strongbox);
		if (item)
			cJSON_InsertItemInArray(array, 0, item);
	}

	// because objects could be nerfed on saving, this could still be empty
	if (cJSON_GetArraySize(array) == 0) {
		cJSON_Delete(array);
		array = NULL;
	}

	return array;
}

/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj(DESCRIPTOR_DATA *d, const char *name)
{
	char strsave[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	bool found;
	ch = new_char();
	ch->pcdata = new_pcdata();
	d->character                        = ch;
	ch->desc                            = d;
	ch->name                            = name;
	ch->id                              = get_pc_id();
	ch->race                            = race_lookup("human");
	ch->act                             = PLR_NOSUMMON | PLR_AUTOASSIST | PLR_AUTOEXIT | PLR_AUTOLOOT |
	                                      PLR_AUTOSAC | PLR_AUTOSPLIT | PLR_AUTOGOLD | PLR_TICKS | PLR_WIMPY |
	                                      PLR_COLOR | PLR_COLOR2;
	ch->comm                            = COMM_COMBINE | COMM_PROMPT;
	ch->secure_level                    = RANK_IMM;
	ch->censor                          = CENSOR_CHAN;    /* default rating is PG */
	ch->prompt                          = "%CW<%CC%h%CThp %CG%m%CHma %CB%v%CNst%CW> ";
	ch->pcdata->ch                      = ch;
	ch->pcdata->deity                   = "Nobody";
	ch->pcdata->mud_exp                 = MEXP_LEGACY_OLDBIE;
//	ch->pcdata->plr                     = PLR_NEWSCORE;

	for (int stat = 0; stat < MAX_STATS; stat++)
		ATTR_BASE(ch, stat_to_attr(stat)) = 3;

	ch->pcdata->combattimer             = -1; // 0 means just came out of combat
	ch->pcdata->condition[COND_THIRST]  = 48;
	ch->pcdata->condition[COND_FULL]    = 48;
	ch->pcdata->condition[COND_HUNGER]  = 48;
	ATTR_BASE(ch, APPLY_HIT)            = 20;
	ATTR_BASE(ch, APPLY_MANA)           = 100;
	ATTR_BASE(ch, APPLY_STAM)           = 100;
	ch->pcdata->last_logoff         = current_time;
	found = FALSE;

	Format::sprintf(strsave, "%s%s", PLAYER_DIR, capitalize(name));

	cJSON *root = JSON::read_file(strsave);

	if (root != NULL) {

		int version = CURRENT_VERSION;
		JSON::get_int(root, &version, "version");

		fread_char(ch, cJSON_GetObjectItem(root, "character"), version);
		fread_player(ch, cJSON_GetObjectItem(root, "player"), version);

		fread_objects(ch, cJSON_GetObjectItem(root, "inventory"), &obj_to_char, version);

		for (OBJ_DATA *obj = ch->carrying; obj; obj = obj->next_content)
			if (obj->wear_loc != WEAR_NONE)
				equip_char(ch, obj, obj->wear_loc);

		fread_objects(ch, cJSON_GetObjectItem(root, "locker"), &obj_to_locker, version);
		fread_objects(ch, cJSON_GetObjectItem(root, "strongbox"), &obj_to_strongbox, version);

		fread_pet(ch, cJSON_GetObjectItem(root, "pet"), version);

		if (ch->pet)
			fread_objects(ch->pet, cJSON_GetObjectItem(root, "pet_inventory"), &obj_to_char, version);

		cJSON_Delete(root); // finished with it
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

		// removed old score at 16 and new_score flag -- Montrey
		if (version < 16 && IS_SET(ch->pcdata->plr, U))
			REMOVE_BIT(ch->pcdata->plr, U);

		// switching to cgroups with old pfiles -- Montrey (2014)
		if (version < 15 && IS_SET(ch->act, N)) { // deputy
			REMOVE_BIT(ch->act, N);
			SET_CGROUP(ch, GROUP_DEPUTY);
		}

		if (version < 15 && IS_SET(ch->act, ee)) { // leader
			REMOVE_BIT(ch->act, ee);
			SET_CGROUP(ch, GROUP_LEADER);
		}

		// removed act_is_npc bit and moved plr_nosummon to A, used to be Q -- Montrey
		if (version < 16 && IS_SET(ch->act, Q)) {
			REMOVE_BIT(ch->act, Q);
			SET_BIT(ch->act, PLR_NOSUMMON);
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
			for (int stat = 0; stat < MAX_STATS; stat++)
				ATTR_BASE(ch, stat_to_attr(stat))
				 = UMIN(ATTR_BASE(ch, stat_to_attr(stat)), get_max_train(ch, stat));
		}
	}

	/* initialize race */
	if (found) {
		int i, percent;

		if (ch->race == 0)
			ch->race = race_lookup("human");

		// permanent affects from race and raffects aren't saved (in case of changes),
		// rebuild them now
		affect_add_racial_to_char(ch);

		extern void raff_add_to_char(CHAR_DATA *ch, int raff);
		if (ch->pcdata->remort_count > 0)
			for (int c = 0; c < ch->pcdata->remort_count / 10 + 1; c++)
				raff_add_to_char(ch, ch->pcdata->raffect[c]);

		ch->size = pc_race_table[ch->race].size;
		ch->dam_type = 17; /*punch */

		for (i = 0; i < 5; i++) {
			if (pc_race_table[ch->race].skills[i] == NULL)
				break;

			group_add(ch, pc_race_table[ch->race].skills[i], FALSE);
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

//		reset_char(ch);
		/* adjust hp mana stamina up  -- here for speed's sake */
		percent = (current_time - ch->pcdata->last_logoff) * 25 / (2 * 60 * 60);
		percent = UMIN(percent, 100);

		if (percent > 0 && !affect_exists_on_char(ch, gsn_poison) && !affect_exists_on_char(ch, gsn_plague)) {
			ch->hit         += (GET_MAX_HIT(ch) - ch->hit) * percent / 100;
			ch->mana        += (GET_MAX_MANA(ch) - ch->mana) * percent / 100;
			ch->stam        += (GET_MAX_STAM(ch) - ch->stam) * percent / 100;
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

void setstr(String *field, const char* value) {
	*field = value;
}

#define STRKEY( literal, field, value )                                    \
	if ( !str_cmp( key, literal ) )        \
	{                                       \
		field = value;                      \
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


void fread_player(CHAR_DATA *ch, cJSON *json, int version) {
	if (json == NULL)
		return;

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
					for (cJSON *item = o->child; item != NULL; item = item->next, count++)
						ch->pcdata->alias[item->child->valuestring] = item->child->next->valuestring;

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
					JSON::get_short(o, &ch->pcdata->condition[COND_DRUNK], "drunk");
					JSON::get_short(o, &ch->pcdata->condition[COND_FULL], "full");
					JSON::get_short(o, &ch->pcdata->condition[COND_THIRST], "thirst");
					JSON::get_short(o, &ch->pcdata->condition[COND_HUNGER], "hunger");
					fMatch = TRUE; break;
				}

				if (!str_cmp(key, "Colr")) { // array of dicts
					for (cJSON *item = o->child; item != NULL; item = item->next) {
						int slot = cJSON_GetObjectItem(item, "slot")->valueint;
						JSON::get_short(item, &ch->pcdata->color[slot], "color");
						JSON::get_short(item, &ch->pcdata->bold[slot], "bold");
					}
					fMatch = TRUE; break;
				}

				INTKEY("Cgrp",			ch->pcdata->cgroup,			string_to_flags(o->valuestring));
				break;
			case 'D':
				STRKEY("Deit",			ch->pcdata->deity,			o->valuestring);
				break;
			case 'E':
				if (!str_cmp(key, "ExSk")) {
					count = 0;
					for (cJSON *item = o->child; item != NULL && count < MAX_EXTRACLASS_SLOTS; item = item->next) {
						int sn = skill_lookup(item->valuestring);

						if (sn <= 0) {
							bugf("unknown extraclass skill '%s'", item->valuestring);
							continue;
						}

						ch->pcdata->extraclass[count++] = sn;
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
				if (!str_cmp(key, "Gr")) {
					for (cJSON *item = o->child; item != NULL; item = item->next) {
						int gn = group_lookup(item->valuestring);

						if (gn < 0) {
							Format::fprintf(stderr, "%s", item->valuestring);
							bug("Unknown group. ", 0);
							continue;
						}

						gn_add(ch, gn);
					}
					fMatch = TRUE; break;
				}

				STRKEY("GameIn",		ch->pcdata->gamein,			o->valuestring);
				STRKEY("GameOut",		ch->pcdata->gameout,		o->valuestring);
				break;
			case 'H':
				if (!str_cmp(key, "HMSP")) { // removed in version 16, moved to fread_char
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_HIT), "hit");
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_MANA), "mana");
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_STAM), "stam");
					fMatch = TRUE; break;
				}

				break;
			case 'I':
				if (!str_cmp(key, "Ignore")) {
					for (cJSON *item = o->child; item != NULL; item = item->next)
						ch->pcdata->ignore.push_back(item->valuestring);
					fMatch = TRUE; break;
				}

				STRKEY("Immn",			ch->pcdata->immname,		o->valuestring);
				STRKEY("Immp",			ch->pcdata->immprefix,		o->valuestring);
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
					JSON::get_long(o, &ch->pcdata->last_note, "note");
					JSON::get_long(o, &ch->pcdata->last_idea, "idea");
					JSON::get_long(o, &ch->pcdata->last_roleplay, "role");
					JSON::get_long(o, &ch->pcdata->last_immquest, "quest");
					JSON::get_long(o, &ch->pcdata->last_changes, "change");
					JSON::get_long(o, &ch->pcdata->last_personal, "pers");
					JSON::get_long(o, &ch->pcdata->last_trade, "trade");
					fMatch = TRUE; break;
				}

				break;
			case 'P':
				STRKEY("Pass",			ch->pcdata->pwd,		o->valuestring);
				INTKEY("PCkills",		ch->pcdata->pckills,	o->valueint);
				INTKEY("PCkilled",		ch->pcdata->pckilled,	o->valueint);
				INTKEY("PKRank",		ch->pcdata->pkrank,		o->valueint);
				INTKEY("Plyd",			ch->pcdata->played,		o->valueint);
				INTKEY("Plr",			ch->pcdata->plr,		string_to_flags(o->valuestring));
				INTKEY("Pnts",			ch->pcdata->points,		o->valueint);
				break;
			case 'Q':
				if (!str_cmp(key, "Query")) {
					for (cJSON *item = o->child; item != NULL && count < MAX_QUERY; item = item->next)
						ch->pcdata->query.push_back(item->valuestring);
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
							Format::fprintf(stderr, "%s", temp);
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
					JSON::get_short(o, &ch->pcdata->trains_to_hit, "hit");
					JSON::get_short(o, &ch->pcdata->trains_to_hit, "mana");
					JSON::get_short(o, &ch->pcdata->trains_to_hit, "stam");
					fMatch = TRUE; break;
				}

				if (!str_cmp(key, "Titl")) {
					set_title(ch, o->valuestring);
					fMatch = TRUE; break;
				}

				INTKEY("TSex",			ATTR_BASE(ch, APPLY_SEX),	o->valueint); // removed in version 16
				break;
			case 'V':
				INTKEY("Video",			ch->pcdata->video,			string_to_flags(o->valuestring));
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

// this could be PC or NPC!
void fread_char(CHAR_DATA *ch, cJSON *json, int version)
{
	if (json == NULL)
		return;

	char buf[MSL];
	Format::sprintf(buf, "Loading %s.", ch->name);
	log_string(buf);

	// unlike old pfiles, the order of calls is important here, because we can't
	// guarantee order within the files. If there are any fields that are depended
	// on by others in the list, load them right here, and use SKIPKEY(key) in the list

	for (cJSON *o = json->child; o; o = o->next) {
		char *key = o->string;
		bool fMatch = FALSE;
//		int count = 0; // convenience variable to compact this list, resets with every item

		switch (toupper(key[0])) {
			case 'A':
				if (!str_cmp(key, "Affc")) {
					// these are the non-permanent affects (not racial or remort affect),
					// those are added after the character is loaded
					for (cJSON *item = o->child; item != NULL; item = item->next) {
						int sn = skill_lookup(cJSON_GetObjectItem(item, "name")->valuestring);

						if (sn < 0) {
							bug("Fread_char: unknown skill.", 0);
							continue;
						}

						AFFECT_DATA af;
						af.type = sn;
						JSON::get_short(item, &af.where, "where");
						JSON::get_short(item, &af.level, "level");
						JSON::get_short(item, &af.duration, "dur");
						JSON::get_short(item, &af.modifier, "mod");
						JSON::get_short(item, &af.location, "loc");
						JSON::get_int(item, &af.bitvector, "bitv");
						JSON::get_short(item, &af.evolution, "evo");
						af.permanent = FALSE;

						affect_copy_to_char(ch, &af);
					}
					fMatch = TRUE; break;
				}

				if (!str_cmp(key, "Atrib")) {
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_STR), "str");
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_INT), "int");
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_WIS), "wis");
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_DEX), "dex");
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_CON), "con");
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_CHR), "chr");
					fMatch = TRUE; break;
				}

				INTKEY("Act",           ch->act,                    string_to_flags(o->valuestring));
				INTKEY("Alig",			ch->alignment,				o->valueint);
				break;
			case 'C':
				INTKEY("Clan",			ch->clan,					clan_lookup(o->valuestring));
				INTKEY("Cla",			ch->cls,					o->valueint);
				INTKEY("Comm",			ch->comm,					string_to_flags(o->valuestring));
				INTKEY("Cnsr",			ch->censor,					string_to_flags(o->valuestring));
				break;
			case 'D':
				INTKEY("Dam",			ATTR_BASE(ch, APPLY_DAMROLL), o->valueint);		// NPC
				STRKEY("Desc",			ch->description,			o->valuestring);
				break;
			case 'E':
				INTKEY("Exp",			ch->exp,					o->valueint);
				break;
			case 'F':
				break;
			case 'G':
				INTKEY("Gold_in_bank",	ch->gold_in_bank,			o->valueint);
				INTKEY("Gold",			ch->gold,					o->valueint);
				INTKEY("GlDonated",		ch->gold_donated,			o->valueint);
				break;
			case 'H':
				if (!str_cmp(key, "HMS")) {
					JSON::get_short(o, &ch->hit, "hit");
					JSON::get_short(o, &ch->mana, "mana");
					JSON::get_short(o, &ch->stam, "stam");
					fMatch = TRUE; break;
				}

				if (!str_cmp(key, "HMSP")) {
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_HIT), "hit");
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_MANA), "mana");
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_STAM), "stam");
					fMatch = TRUE; break;
				}

				INTKEY("Hit",			ATTR_BASE(ch, APPLY_HITROLL), o->valueint); // NPC
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
				INTKEY("PosP",			ch->start_pos,				o->valueint);
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
				INTKEY("Revk",			ch->revoke,					string_to_flags(o->valuestring));
				break;
			case 'S':
				INTKEY("Save",			ATTR_BASE(ch, APPLY_SAVES),	o->valueint); // NPC
				INTKEY("Scro",			ch->lines,					o->valueint);
				INTKEY("Secu",			ch->secure_level,			o->valueint);
				INTKEY("Sex",			ATTR_BASE(ch, APPLY_SEX),	o->valueint);
				STRKEY("ShD",			ch->short_descr,			o->valuestring);
				INTKEY("Silver_in_bank",ch->silver_in_bank,			o->valueint);
				INTKEY("Silv",			ch->silver,					o->valueint);
				break;
			case 'T':
				INTKEY("Trai",			ch->train,					o->valueint);
				break;
			case 'V':
				SKIPKEY("Vnum"); // for NPCs
				break;
			case 'W':
				INTKEY("Wimp",			ch->wimpy,					o->valueint);
				INTKEY("Wizn",			ch->wiznet,					string_to_flags(o->valuestring));
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
	}

	for (cJSON *o = json->child; o; o = o->next) {
		char *key = o->string;
		bool fMatch = FALSE;
//		int count = 0; // convenience variable to compact this list, resets with every item

		switch (toupper(key[0])) {
			case 'A':
				if (!str_cmp(key, "Affc")) {
					// ugh.  when I put this in, it took a while for a bug to show up where
					// the object's affects were not in fact being cleared before the saved
					// affects were applied, so any enchanted gear was having affects multiplied.
					// the easiest way to fix this is just to reset enchantments back to stock.
					// i'm fixing gear for anyone who complains, but we only have like 4 players,
					// so hopefully we can just get past this.
					if (version > 15 && version < 17) {
						fMatch = TRUE; break;
					}

					// this object has different affects than the index, free the old ones
					affect_remove_all_from_obj(obj, TRUE);

					for (cJSON *item = o->child; item != NULL; item = item->next) {
						int sn = skill_lookup(cJSON_GetObjectItem(item, "name")->valuestring);

						if (sn < 0) {
							bug("Fread_obj: unknown skill.", 0);
							continue;
						}

						AFFECT_DATA af;
						af.type = sn;

						JSON::get_short(item, &af.where, "where");
						JSON::get_short(item, &af.level, "level");
						JSON::get_short(item, &af.duration, "dur");
						JSON::get_short(item, &af.modifier, "mod");
						JSON::get_short(item, &af.location, "loc");
						JSON::get_int(item, &af.bitvector, "bitv");
						JSON::get_short(item, &af.evolution, "evo");

						// some old objects made with 'addapply' have things affects without an sn,
						// or modifies nothing with no bits.  also, some items that were enchanted
						// under the old system could have bitvectors storing affects. clean up here,
						// hopefully remove someday.
						// obj name might not be read yet, use vnum
						if (af.where == TO_AFFECTS && af.type == 0 && af.bitvector == 0)
							af.where = TO_OBJECT; // try making it an object apply

						// let the parsing handle TO_OBJECT with no modifiers

						unsigned int bitvector = af.bitvector;

						// run bitvector down to 0 (unless certain cases)
						// do at least once even if no bitvector
						do {
							if (affect_parse_flags(0, &af, &bitvector)) {
								affect_copy_to_obj(obj, &af);
								
								// don't multiply the modifier, just apply to the first bit
								af.location = 0;
								af.modifier = 0;
							}

							af.type = 0; // reset, in case we're parsing multiple TO_AFFECTS bits
						} while (bitvector != 0);
					}
					fMatch = TRUE; break;
				}
				break;
			case 'C':
				if (!str_cmp(key, "contains")) {
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
						ed->keyword             = item->string;
						ed->description         = item->valuestring;
						ed->next                = obj->extra_descr;
						obj->extra_descr        = ed;
					}
					fMatch = TRUE; break;
				}

				INTKEY("ExtF",			obj->extra_flags,			o->valueint); // no, not fstring_to_flags
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
				INTKEY("WeaF",			obj->wear_flags,			o->valueint); // no, not string_to_flags
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

	// blow away any affects that are not permanent, from non-racial affect flags
	affect_remove_all_from_char(pet, FALSE);

	fread_char(pet, json, version);

	pet->leader = ch;
	pet->master = ch;
	ch->pet = pet;

	/* adjust hp mana stamina up  -- here for speed's sake */
	int percent;
	percent = (current_time - ch->pcdata->last_logoff) * 25 / (2 * 60 * 60);
	percent = UMIN(percent, 100);

	if (percent > 0 && !affect_exists_on_char(ch, gsn_poison)
	    &&  !affect_exists_on_char(ch, gsn_plague)) {
		pet->hit    += (GET_MAX_HIT(pet) - pet->hit) * percent / 100;
		pet->mana   += (GET_MAX_MANA(pet) - pet->mana) * percent / 100;
		pet->stam   += (GET_MAX_STAM(pet) - pet->stam) * percent / 100;
	}
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

const char *dizzy_ctime(time_t *timep)
{
	static char ctime_buf[40];
	struct tm loc_tm;
	loc_tm = *localtime(timep);
	Format::sprintf(ctime_buf, "%s %s %02d %02d:%02d:%02d %04d\n",
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
 * return value from Format::sprintf() will be significant.
 */
time_t dizzy_scantime(const String& ctime)
{
	char cdow[4], cmon[4];
	int year, month, day, hour, minute, second;
	char msg[MAX_INPUT_LENGTH];
	struct tm loc_tm;
	/* this helps initialize local-dependent stuff like TZ, etc. */
	loc_tm = *localtime(&current_time);

	if (sscanf(ctime.c_str(), " %3s %3s %d %d:%d:%d %d",
	           cdow, cmon, &day, &hour, &minute, &second, &year) < 7) {
		Format::sprintf(msg, "dizzy_scantime(): Error scanning date/time: '%s'", ctime);
		bug(msg, 0);
		goto endoftime;
	}

	for (month = 0; month < 12; month++) {
		if (!str_prefix1(month_names[month], ctime.substr(4)))
			break;
	}

	if (month >= 12) {
		Format::sprintf(msg, "dizzy_scantime(): Bad month in %s", ctime);
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

void do_finger(CHAR_DATA *ch, String argument)
{
	char filename[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	String dbuf;

	/* the following vars are read from the player file */
	String email, fingerinfo, last_lsite, name, title, spouse, race, deity;
	int cls, pks, pkd, pkr, aks, akd, level, rmct;
	long cgroup = 0L, plr = 0L;
	time_t last_ltime, last_saved;
	CLAN_DATA *clan = NULL;

	String arg;
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

	if (!check_parse_name(arg)) {
		stc("That is not a valid player name.\n", ch);
		return;
	}

	Format::sprintf(filename, "%s%s", PLAYER_DIR, arg.capitalize());

	cJSON *root = JSON::read_file(filename);

	if (root == NULL) {
		stc("That player does not exist.\n", ch);
		return;
	}

	/* initialize variables */
	// strings are empty by default
	cls = pks = pkd = pkr = aks = akd = level = rmct = 0;

	cJSON *section, *item;
	section = cJSON_GetObjectItem(root, "character");
	JSON::get_string(section, &name, "Name");
	JSON::get_string(section, &race, "Race");
	JSON::get_int(section, &level, "Levl");
	JSON::get_int(section, &cls, "Cla");

	if ((item = cJSON_GetObjectItem(section, "Clan")) != NULL)
		clan = clan_lookup(item->valuestring);

	section = cJSON_GetObjectItem(root, "player");
	JSON::get_string(section, &email, "Email");
	JSON::get_string(section, &fingerinfo, "Finf");
	JSON::get_string(section, &title, "Titl");
	JSON::get_string(section, &spouse, "Spou");
	JSON::get_string(section, &deity, "Deit");
	JSON::get_string(section, &last_lsite, "Lsit");
	JSON::get_int(section, &pks, "PCkills");
	JSON::get_int(section, &pkd, "PCkilled");
	JSON::get_int(section, &pkr, "PKRank");
	JSON::get_int(section, &aks, "Akills");
	JSON::get_int(section, &akd, "Akilled");
	JSON::get_int(section, &rmct, "RmCt");
	JSON::get_flags(section, &cgroup, "Cgrp");
	JSON::get_flags(section, &plr, "Plr");

	if ((item = cJSON_GetObjectItem(section, "Ltim")) != NULL)
		last_ltime = dizzy_scantime(item->valuestring);
	if ((item = cJSON_GetObjectItem(section, "LSav")) != NULL)
		last_saved = dizzy_scantime(item->valuestring);

	cJSON_Delete(root); // finished with it

	/* display information */

	if (title[0] != '.' && title[0] != ',' &&  title[0] != '!' && title[0] != '?') {
		Format::sprintf(buf, " %s{x", title);
		title = buf;
	}

	if (RANK(cgroup) >= RANK_IMM)
		Format::sprintf(buf, "{W[{CIMM{W] %s%s{x\n", name, title);
	else if (rmct == 0)
		Format::sprintf(buf, "{W[{B%2d{W] %s%s{x\n", level, name, title);
	else
		Format::sprintf(buf, "{W[{GR%d{T/{B%2d{W] %s%s{x\n", rmct, level, name, title);

	dbuf += buf;

	if (clan) {
		if (IS_SET(cgroup, GROUP_LEADER))
			Format::sprintf(buf, "{BLeader of ");
		else if (IS_SET(cgroup, GROUP_DEPUTY))
			Format::sprintf(buf, "{BDeputy of ");
		else
			Format::sprintf(buf, "{BMember of ");

		dbuf += buf;
		Format::sprintf(buf, "%s{x\n", clan->clanname);
		dbuf += buf;
	}

	Format::sprintf(buf, "{C%s ", race.capitalize());
	dbuf += buf;
	Format::sprintf(buf, "{C%s, follower of %s{x\n", capitalize(class_table[cls].name), deity);
	dbuf += buf;
	Format::sprintf(buf, "{GArena Record:    %d wins,  %d losses{x\n", aks, akd);
	dbuf += buf;
	Format::sprintf(buf, "{PBlood Trail (%d): %d kills, %d deaths{x\n\n", pkr, pks, pkd);
	dbuf += buf;

	if (fingerinfo[0]) {
		Format::sprintf(buf, "{CAdditional Info:{x\n%s{x\n", fingerinfo);
		dbuf += buf;
	}

	if (spouse[0]) {
		if (!IS_SET(plr, PLR_MARRIED))
			Format::sprintf(buf, "{Y%s is engaged to %s.{x\n", name, spouse);
		else
			Format::sprintf(buf, "{Y%s is happily married to %s.{x\n", name, spouse);

		dbuf += buf;
	}

	if (email[0] && (IS_IMMORTAL(ch) || IS_SET(plr, PLR_SHOWEMAIL))) {
		Format::sprintf(buf, "{GEmail: %s{x\n", email);
		dbuf += buf;
	}

	if (IS_IMMORTAL(ch) || !IS_SET(plr, PLR_NOSHOWLAST)) {
		if (last_ltime) {
			Format::sprintf(buf, "{HLast Login : %s\n{x", dizzy_ctime(&last_ltime));
			dbuf += buf;
		}

		if (last_saved) {
			Format::sprintf(buf, "{HLast Saved : %s\n{x", dizzy_ctime(&last_saved));
			dbuf += buf;
		}
	}

	if (IS_IMP(ch)) {
		Format::sprintf(buf, "{HLast Site  : %s{x\n", last_lsite);
		dbuf += buf;
	}

	dbuf += "\n";
	page_to_char(dbuf, ch);
}

