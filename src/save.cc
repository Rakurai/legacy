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

#include <map>
#include <utility>
#include <vector>

#include "file.hh"
#include "../deps/cJSON/cJSON.h"
#include "argument.hh"
#include "affect/Affect.hh"
#include "JSON/cJSON.hh"
#include "Character.hh"
#include "Clan.hh"
#include "declare.hh"
#include "Descriptor.hh"
#include "ExtraDescr.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "interp.hh"
#include "lookup.hh"
#include "Logging.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"
#include "ObjectValue.hh"
#include "Player.hh"
#include "Room.hh"
#include "String.hh"
#include "gem/gem.hh"
#include "sql.hh"
#include "World.hh"


int CURRENT_VERSION = 21;   /* version number for pfiles */

bool debug_json = false;

/* Locals */

int rename(const char *oldfname, const char *newfname);

/*
 * Local functions.
 */
cJSON * fwrite_player     args((Character *ch));
cJSON * fwrite_char     args((Character *ch));
cJSON * fwrite_objects  args((Object *head));
cJSON * fwrite_pet      args((Character *pet));
void    fread_char      args((Character *ch,  cJSON *json, int version));
void    fread_player      args((Character *ch,  cJSON *json, int version));
void    fread_pet       args((Character *ch,  cJSON *json, int version));
void	fread_objects	args((Character *ch, cJSON *json, void (*obj_to)(Object *, Character *), int version));
bool check_parse_name(const String& name);

// external
const Object *get_warp_crystal(const String&);
const String get_warp_loc_string(const Object *);

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj(Character *ch)
{
	char strsave[MIL];
	FILE *fp;

	if (ch == nullptr || ch->is_npc())
		return;

	if (ch->desc != nullptr && ch->desc->original != nullptr)
		ch = ch->desc->original;

	ch->pcdata->last_saved = Game::current_time;

	cJSON *root = cJSON_CreateObject();

	cJSON_AddNumberToObject(root, "version", CURRENT_VERSION);
	cJSON_AddItemToObject(root, "player", fwrite_player(ch));
	cJSON_AddItemToObject(root, "character", fwrite_char(ch));

	cJSON_AddItemToObject(root, "inventory", fwrite_objects(ch->carrying));
	cJSON_AddItemToObject(root, "locker", fwrite_objects(ch->pcdata->locker));
	cJSON_AddItemToObject(root, "strongbox", fwrite_objects(ch->pcdata->strongbox));

	if (ch->pet) {
		cJSON_AddItemToObject(root, "pet", fwrite_pet(ch->pet));
		cJSON_AddItemToObject(root, "pet_inventory", fwrite_objects(ch->pet->carrying));
	}

	char *JSONstring = cJSON_Print(root);
	cJSON_Delete(root);

	// added if to avoid closing invalid file
	String buf;
	one_argument(ch->name, buf);
	Format::sprintf(strsave, "%s%s", PLAYER_DIR, buf.lowercase().capitalize());

	if ((fp = fopen(TEMP_FILE, "w")) != nullptr) {
		fputs(JSONstring, fp);
		fclose(fp);
		rename(TEMP_FILE, strsave);
	}
	else {
		Logging::bug("Save_char_obj: fopen", 0);
		perror(strsave);
	}

	free(JSONstring);
	update_pc_index(ch, false);
}

void backup_char_obj(Character *ch)
{
	save_char_obj(ch);

	char strsave[MIL], strback[MIL];
	String buf;
	one_argument(ch->name, buf);

	Format::sprintf(strsave, "%s%s", PLAYER_DIR, buf.lowercase().capitalize());
	Format::sprintf(strback, "%s%s", BACKUP_DIR, buf.lowercase().capitalize());

	Format::sprintf(buf, "cp %s %s", strsave, strback);
	system(buf.c_str());
	Format::sprintf(buf, "gzip -fq %s", strback);
	system(buf.c_str());
} /* end backup_char_obj() */

cJSON *fwrite_player(Character *ch)
{
	cJSON *item;
	cJSON *o = cJSON_CreateObject(); // object to return

	item = nullptr;

	if (!ch->pcdata->alias.empty()) {
		item = cJSON_CreateArray();

		for (const auto& kv: ch->pcdata->alias) {
			cJSON *alias = cJSON_CreateArray();
			cJSON_AddItemToArray(alias, cJSON_CreateString(kv.first.c_str()));
			cJSON_AddItemToArray(alias, cJSON_CreateString(kv.second.c_str()));
			cJSON_AddItemToArray(item, alias);
		}

		cJSON_AddItemToObject(o,	"Alias",		item);
	}

	if (!ch->pcdata->afk.empty())
		JSON::addStringToObject(o,	"Afk",			ch->pcdata->afk);

	cJSON_AddNumberToObject(o,		"Akills",		ch->pcdata->arenakills);
	cJSON_AddNumberToObject(o,		"Akilled",		ch->pcdata->arenakilled);

	if (!ch->pcdata->aura.empty())
		JSON::addStringToObject(o,	"Aura",			ch->pcdata->aura);

	cJSON_AddNumberToObject(o,		"Back",			ch->pcdata->backup);

	if (!ch->pcdata->bamfin.empty())
		JSON::addStringToObject(o,	"Bin",			ch->pcdata->bamfin);

	if (!ch->pcdata->bamfout.empty())
		JSON::addStringToObject(o,	"Bout",			ch->pcdata->bamfout);

	JSON::addStringToObject(o,		"Cgrp",			ch->pcdata->cgroup_flags.to_string());

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

	if (!ch->pcdata->deity.empty())
		JSON::addStringToObject(o,	"Deit",			ch->pcdata->deity);

	if (!ch->pcdata->email.empty())
		JSON::addStringToObject(o,	"Email",		ch->pcdata->email);

	cJSON_AddNumberToObject(o,		"Familiar",		ch->pcdata->familiar);

	if (!ch->pcdata->fingerinfo.empty())
		JSON::addStringToObject(o,	"Finf",			ch->pcdata->fingerinfo);

	if (ch->pcdata->flag_killer)
		cJSON_AddNumberToObject(o,	"FlagKiller",	ch->pcdata->flag_killer);

	if (ch->pcdata->flag_thief)
		cJSON_AddNumberToObject(o,	"FlagThief",	ch->pcdata->flag_thief);

	if (!ch->pcdata->gamein.empty())
		JSON::addStringToObject(o,	"GameIn",		ch->pcdata->gamein);

	if (!ch->pcdata->gameout.empty())
		JSON::addStringToObject(o,	"GameOut",		ch->pcdata->gameout);

	if (ch->pcdata->gold_donated)
		cJSON_AddNumberToObject(o,	"GlDonated",	ch->pcdata->gold_donated);

	item = nullptr;
	for (unsigned int gn = 0; gn < group_table.size(); gn++) {
		if (ch->pcdata->group_known[gn] == 0)
			continue;

		if (item == nullptr)
			item = cJSON_CreateArray();

		cJSON_AddItemToArray(item, cJSON_CreateString(group_table[gn].name.c_str()));
	}
	if (item != nullptr)
		cJSON_AddItemToObject(o,	"Gr",			item);

	cJSON_AddNumberToObject(o,		"Id",			ch->pcdata->id);

	if (!ch->pcdata->ignore.empty()) {
		item = cJSON_CreateArray();

		for (const auto& it: ch->pcdata->ignore)
			cJSON_AddItemToArray(item, cJSON_CreateString(it.c_str()));

		cJSON_AddItemToObject(o,	"Ignore",		item);
	}

	if (!ch->pcdata->immname.empty())
		JSON::addStringToObject(o,	"Immn",			ch->pcdata->immname);
	if (!ch->pcdata->immprefix.empty())
		JSON::addStringToObject(o,	"Immp",			ch->pcdata->immprefix);

	if (ch->guild == Guild::paladin) {
		cJSON_AddNumberToObject(o,	"Lay",			ch->pcdata->lays);
		cJSON_AddNumberToObject(o,	"Lay_Next",		ch->pcdata->next_lay_countdown);
	}

	cJSON_AddNumberToObject(o,		"LLev",			ch->pcdata->last_level);
	cJSON_AddNumberToObject(o,		"LogO",			Game::current_time);

	if (!ch->pcdata->last_lsite.empty())
		JSON::addStringToObject(o,	"Lsit",			ch->pcdata->last_lsite);

	JSON::addStringToObject(o,		"Ltim",			dizzy_ctime(&ch->pcdata->last_ltime));
	JSON::addStringToObject(o,		"LSav",			dizzy_ctime(&ch->pcdata->last_saved));

	if (ch->pcdata->mark_room.is_valid())
		cJSON_AddNumberToObject(o,	"Mark",			ch->pcdata->mark_room.to_int());

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
	JSON::addStringToObject(o,		"Plr",			ch->pcdata->plr_flags.to_string());
	cJSON_AddNumberToObject(o,		"Plyd",			ch->pcdata->played);
	cJSON_AddNumberToObject(o,		"Pnts",			ch->pcdata->points);

	if (!ch->pcdata->query.empty()) {
		item = cJSON_CreateArray();

		for (const auto& it: ch->pcdata->query)
			cJSON_AddItemToArray(item, cJSON_CreateString(it.c_str()));

		cJSON_AddItemToObject(o,	"Query",		item);
	}

	if (ch->pcdata->questpoints_donated)
		cJSON_AddNumberToObject(o,	"QpDonated",	ch->pcdata->questpoints_donated);

	if (ch->pcdata->questpoints)
		cJSON_AddNumberToObject(o,	"QuestPnts",	ch->pcdata->questpoints);

	if (ch->pcdata->nextquest)
		cJSON_AddNumberToObject(o,	"QuestNext",	ch->pcdata->nextquest);
	else if (ch->pcdata->countdown)
		cJSON_AddNumberToObject(o,	"QuestNext",	12);

	if (!ch->pcdata->rank.empty())
		JSON::addStringToObject(o,	"Rank",			ch->pcdata->rank);

	if (ch->pcdata->rolepoints)
		cJSON_AddNumberToObject(o,	"RolePnts",		ch->pcdata->rolepoints);

	item = nullptr;
	for (const auto& pair : skill_table) {
		skill::type type = pair.first;

		if (type == skill::type::unknown)
			continue;

		if (get_learned(ch, type) <= 0)
			continue;

		if (item == nullptr)
			item = cJSON_CreateArray();

		cJSON *sk = cJSON_CreateObject();
		JSON::addStringToObject(sk, "name", skill::lookup(type).name);
		cJSON_AddNumberToObject(sk, "prac", get_learned(ch, type));
		cJSON_AddNumberToObject(sk, "evol", get_evolution(ch, type));
		cJSON_AddItemToArray(item, sk);
	}
	if (item != nullptr)
		cJSON_AddItemToObject(o,	"Sk",			item);

	if (ch->pcdata->skillpoints)
		cJSON_AddNumberToObject(o,	"SkillPnts",	ch->pcdata->skillpoints);

	if (!ch->pcdata->spouse.empty())
		JSON::addStringToObject(o,	"Spou",			ch->pcdata->spouse);

	if (ch->pcdata->nextsquest)
		cJSON_AddNumberToObject(o,	"SQuestNext",	ch->pcdata->nextsquest);
	else if (ch->pcdata->sqcountdown)
		cJSON_AddNumberToObject(o,	"SQuestNext",	20);

	if (ch->pcdata->remort_count > 0) {
		if (!ch->pcdata->status.empty())
			JSON::addStringToObject(o,	"Stus",		ch->pcdata->status);

		cJSON_AddNumberToObject(o,	"RmCt",			ch->pcdata->remort_count);

		item = nullptr;
		for (int i = 0; i < (ch->pcdata->remort_count / EXTRACLASS_SLOT_LEVELS) + 1; i++) {
			if (ch->pcdata->extraclass[i] == skill::type::unknown)
				break;

			if (item == nullptr)
				item = cJSON_CreateArray();

			cJSON_AddItemToArray(item,
				cJSON_CreateString(skill::lookup(ch->pcdata->extraclass[i]).name.c_str()));
		}

		if (item != nullptr)
			cJSON_AddItemToObject(o, "ExSk", item);

		item = cJSON_CreateIntArray(ch->pcdata->raffect, ch->pcdata->remort_count / 10 + 1);
		cJSON_AddItemToObject(o, "Raff", item);
	}

	item = cJSON_CreateObject();
	cJSON_AddNumberToObject(item,	"hit",			ch->pcdata->trains_to_hit);
	cJSON_AddNumberToObject(item,	"mana",			ch->pcdata->trains_to_mana);
	cJSON_AddNumberToObject(item,	"stam",			ch->pcdata->trains_to_stam);
	cJSON_AddItemToObject(o, 		"THMS",	 		item);

	if (!ch->pcdata->title.empty())
		JSON::addStringToObject(o,	"Titl",			ch->pcdata->title[0] == ' ' ?
		ch->pcdata->title.substr(1) : ch->pcdata->title);
	JSON::addStringToObject(o,		"Video",		ch->pcdata->video_flags.to_string());

	if (!ch->pcdata->warp_locs.empty()) {
		item = cJSON_CreateArray();

		for (const auto& it: ch->pcdata->warp_locs)
			cJSON_AddItemToArray(item, cJSON_CreateString(it.c_str()));

		cJSON_AddItemToObject(o,	"WarpLocs",		item);
	}

	if (!ch->pcdata->whisper.empty())
		JSON::addStringToObject(o,	"Wspr",			ch->pcdata->whisper);

	return o;
}

/*
 * Write the char.
 */
cJSON *fwrite_char(Character *ch)
{
	cJSON *item;
	cJSON *o = cJSON_CreateObject(); // object to return

	JSON::addStringToObject(o,		"Act",			ch->act_flags.to_string());

	item = nullptr;
	for (const affect::Affect *paf = affect::list_char(ch); paf != nullptr; paf = paf->next) {
		if (paf->type <= affect::type::unknown || paf->type >= affect::type::size)
			continue;

		// don't write permanent affects, rebuild them from race and raffects on load
		if (paf->permanent)
			continue;

		if (item == nullptr)
			item = cJSON_CreateArray();

		cJSON *aff = cJSON_CreateObject();
		JSON::addStringToObject(aff, "name", affect::lookup(paf->type).name);
		cJSON_AddNumberToObject(aff, "where", paf->where);
		cJSON_AddNumberToObject(aff, "level", paf->level);
		cJSON_AddNumberToObject(aff, "dur", paf->duration);
		cJSON_AddNumberToObject(aff, "mod", paf->modifier);
		cJSON_AddNumberToObject(aff, "loc", paf->location);
		cJSON_AddNumberToObject(aff, "bitv", paf->bitvector().to_ulong());
		cJSON_AddNumberToObject(aff, "evo", paf->evolution);
		cJSON_AddItemToArray(item, aff);
	}
	if (item != nullptr)
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

	cJSON_AddNumberToObject(o,		"Cla",			ch->guild);
	JSON::addStringToObject(o,		"Cnsr",			ch->censor_flags.to_string());
	JSON::addStringToObject(o,		"Comm",			ch->comm_flags.to_string());

	if (!ch->description.empty())
		JSON::addStringToObject(o,	"Desc",			ch->description);

	cJSON_AddNumberToObject(o,		"Exp",			ch->exp);

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

	cJSON_AddNumberToObject(o,		"Levl",			ch->level);

	if (!ch->long_descr.empty())
		JSON::addStringToObject(o,	"LnD",			ch->long_descr);

	JSON::addStringToObject(o,		"Name",			ch->name);
	cJSON_AddNumberToObject(o,		"Pos",			ch->position);
	cJSON_AddNumberToObject(o,              "PosP",                 ch->start_pos);
	cJSON_AddNumberToObject(o,		"Prac",			ch->practice);

	if (!ch->prompt.empty())
		JSON::addStringToObject(o,	"Prom",			ch->prompt);

	JSON::addStringToObject(o,		"Race",			race_table[ch->race].name);
	JSON::addStringToObject(o,		"Revk",			ch->revoke_flags.to_string());
	cJSON_AddNumberToObject(o,		"Room",			
		(ch->in_room == Game::world().get_room(Location(Vnum(ROOM_VNUM_LIMBO))) && ch->was_in_room != nullptr)
	        ? ch->was_in_room->location.to_int()
	        : ch->in_room == nullptr
	        ? 3001
	        : ch->in_room->location.to_int());

	cJSON_AddNumberToObject(o,		"Scro",			ch->lines);
	cJSON_AddNumberToObject(o,		"Sex",			ATTR_BASE(ch, APPLY_SEX));
	cJSON_AddNumberToObject(o,		"Silv",			ch->silver);

	if (ch->silver_in_bank > 0)
		cJSON_AddNumberToObject(o,	"Silver_in_bank", ch->silver_in_bank);

	if (!ch->short_descr.empty())
		JSON::addStringToObject(o,	"ShD",			ch->short_descr);

	cJSON_AddNumberToObject(o,		"Trai",			ch->train);
	cJSON_AddNumberToObject(o,		"Wimp",			ch->wimpy);

	if (IS_IMMORTAL(ch)) { // why aren't these pcdata?
		JSON::addStringToObject(o,	"Wizn",			ch->wiznet_flags.to_string());
		cJSON_AddNumberToObject(o,	"Invi",			ch->invis_level);
		cJSON_AddNumberToObject(o,	"Lurk",			ch->lurk_level);
		cJSON_AddNumberToObject(o,	"Secu",			ch->secure_level);
	}

	return o;
}

/* write a pet */
cJSON *fwrite_pet(Character *pet)
{
	cJSON *o = fwrite_char(pet);

	cJSON_AddNumberToObject(o, "Vnum", pet->pIndexData->vnum.value());

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
cJSON *fwrite_obj(Object *obj)
{
	cJSON *item;
	cJSON *o = cJSON_CreateObject();

	if (obj->condition != obj->pIndexData->condition)
		cJSON_AddNumberToObject(o,	"Cond",			obj->condition);
	if (obj->cost != obj->pIndexData->cost)
		cJSON_AddNumberToObject(o,	"Cost",			obj->cost);
	if (obj->description != obj->pIndexData->description)
		JSON::addStringToObject(o,	"Desc",			obj->description);

	if (affect::enchanted_obj(obj)) {
		// we could write an empty list here, for a disenchanted item
		item = cJSON_CreateArray();

		for (const affect::Affect *paf = affect::list_obj(obj); paf != nullptr; paf = paf->next) {
			if (paf->type <= affect::type::unknown || paf->type >= affect::type::size)
				continue;

			cJSON *aff = cJSON_CreateObject();
			JSON::addStringToObject(aff, "name", affect::lookup(paf->type).name);
			cJSON_AddNumberToObject(aff, "where", paf->where);
			cJSON_AddNumberToObject(aff, "level", paf->level);
			cJSON_AddNumberToObject(aff, "dur", paf->duration);
			cJSON_AddNumberToObject(aff, "mod", paf->modifier);
			cJSON_AddNumberToObject(aff, "loc", paf->location);
			cJSON_AddNumberToObject(aff, "bitv", paf->bitvector().to_ulong());
			cJSON_AddNumberToObject(aff, "evo", paf->evolution);
			cJSON_AddItemToArray(item, aff);
		}

		cJSON_AddItemToObject(o,	"Affc",			item);
	}

	item = nullptr;
	for (ExtraDescr *ed = obj->extra_descr; ed != nullptr; ed = ed->next) {
		if (item == nullptr)
			item = cJSON_CreateObject();

		JSON::addStringToObject(item, ed->keyword, ed->description);
	}
	if (item != nullptr)
		cJSON_AddItemToObject(o,	"ExDe",			item);

	if (obj->extra_flags != obj->pIndexData->extra_flags)
		cJSON_AddNumberToObject(o,	"ExtF",			obj->extra_flags.to_ulong());
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
	    ||  obj->value[4] != obj->pIndexData->value[4]) {
		// this is ugly, but i don't feel like creating a new overloaded function
		int arr[5];
		for (int i = 0; i < 5; i++)
			arr[i] = obj->value[i];
	    cJSON_AddItemToObject(o,	"Val",			cJSON_CreateIntArray(arr, 5));
	}

	cJSON_AddNumberToObject(o,		"Vnum",			obj->pIndexData->vnum.value());

	if (obj->wear_loc != WEAR_NONE)
		cJSON_AddNumberToObject(o,	"Wear",			obj->wear_loc);

	if (obj->wear_flags != obj->pIndexData->wear_flags)
		cJSON_AddNumberToObject(o,	"WeaF",			obj->wear_flags.to_ulong());
	if (obj->weight != obj->pIndexData->weight)
		cJSON_AddNumberToObject(o,	"Wt",			obj->weight);

	// does nothing if the contains is nullptr
	cJSON_AddItemToObject(o, "contains", fwrite_objects(obj->contains));

	cJSON_AddItemToObject(o, "gems", fwrite_objects(obj->gems));

	return o;
} /* end fwrite_obj() */

cJSON *fwrite_objects(Object *head) {
	cJSON *array = cJSON_CreateArray();

	// old way was to use recursion to write items in reverse order, so loading would
	// be in the original order.  same concept here, except no recursion; we just
	// take advantage of the linked list underlying the cJSON array and insert at
	// index 0, so the array is written backwards.
	for (Object *obj = head; obj; obj = obj->next_content) {
		// unlike adding an item, inserting an item crashes with a NULL item
		// and fwrite_obj could return NULL because of obj pruning
		cJSON *item = fwrite_obj(obj);

		if (item)
			cJSON_InsertItemInArray(array, 0, item);
	}

	// because objects could be nerfed on saving, this could still be empty
	if (cJSON_GetArraySize(array) == 0) {
		cJSON_Delete(array);
		array = nullptr;
	}

	return array;
}

/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj(Descriptor *d, const String& name)
{
	char strsave[MAX_INPUT_LENGTH];
	Character *ch;
	bool found;
	ch = new Character();
	ch->pcdata = new Player(*ch);
	d->character                        = ch;
	ch->desc                            = d;
	ch->name                            = name;
	ch->race                            = race_lookup("human");
	ch->act_flags                             = PLR_NOSUMMON | PLR_AUTOASSIST | PLR_AUTOEXIT | PLR_AUTOLOOT |
	                                      PLR_AUTOSAC | PLR_AUTOSPLIT | PLR_AUTOGOLD | PLR_TICKS | PLR_WIMPY |
	                                      PLR_COLOR | PLR_COLOR2;
	ch->comm_flags                            = COMM_COMBINE | COMM_PROMPT;
	ch->secure_level                    = RANK_IMM;
	ch->censor_flags                          = CENSOR_CHAN;    /* default rating is PG */
	ch->prompt                          = "%CW<%CC%h%CThp %CG%m%CHma %CB%v%CNst%CW> ";
	ch->pcdata->deity                   = "Nobody";
	ch->pcdata->mud_exp                 = MEXP_LEGACY_OLDBIE;
//	ch->pcdata->plr_flags                     = PLR_NEWSCORE;

	for (int stat = 0; stat < MAX_STATS; stat++)
		ATTR_BASE(ch, stat_to_attr(stat)) = 3;

	ch->pcdata->combattimer             = -1; // 0 means just came out of combat
	ch->pcdata->condition[COND_THIRST]  = 48;
	ch->pcdata->condition[COND_FULL]    = 48;
	ch->pcdata->condition[COND_HUNGER]  = 48;
	ATTR_BASE(ch, APPLY_HIT)            = 20;
	ATTR_BASE(ch, APPLY_MANA)           = 100;
	ATTR_BASE(ch, APPLY_STAM)           = 100;
	ch->pcdata->last_logoff         = Game::current_time;
	found = false;

	Format::sprintf(strsave, "%s%s", PLAYER_DIR, name.lowercase().capitalize());

	cJSON *root = JSON::read_file(strsave);

	if (root != nullptr) {

		int version = CURRENT_VERSION;
		JSON::get_int(root, &version, "version");

		fread_char(ch, cJSON_GetObjectItem(root, "character"), version);
		fread_player(ch, cJSON_GetObjectItem(root, "player"), version);

		fread_objects(ch, cJSON_GetObjectItem(root, "inventory"), &obj_to_char, version);

		for (Object *obj = ch->carrying; obj; obj = obj->next_content)
			if (obj->wear_loc != WEAR_NONE)
				equip_char(ch, obj, obj->wear_loc);

		fread_objects(ch, cJSON_GetObjectItem(root, "locker"), &obj_to_locker, version);
		fread_objects(ch, cJSON_GetObjectItem(root, "strongbox"), &obj_to_strongbox, version);

		fread_pet(ch, cJSON_GetObjectItem(root, "pet"), version);

		if (ch->pet)
			fread_objects(ch->pet, cJSON_GetObjectItem(root, "pet_inventory"), &obj_to_char, version);

		cJSON_Delete(root); // finished with it
		found = true;

		// fix things up

		// fix up character stuff here
		if (ch->in_room == nullptr)
			ch->in_room = Game::world().get_room(Location(Vnum(ROOM_VNUM_TEMPLE)));

		if (ch->secure_level > GET_RANK(ch))
			ch->secure_level = GET_RANK(ch);

		/* removed holylight at 12 -- Montrey */
		if (version < 12 && ch->act_flags.has(Flags::N))
			ch->act_flags -= Flags::N;

		// removed padding and brackets in immname in version 19
		if (version < 19 && !ch->pcdata->immname.empty())
			ch->pcdata->immname = ch->pcdata->immname
				.replace("{W[{x", "")
				.replace("{W]{x", "")
				.strip();

		// removed old score at 16 and new_score flag -- Montrey
		if (version < 16 && ch->pcdata->plr_flags.has(Flags::U))
			ch->pcdata->plr_flags -= Flags::U;

		// switching to cgroups with old pfiles -- Montrey (2014)
		if (version < 15 && ch->act_flags.has(Flags::N)) { // deputy
			ch->act_flags -= Flags::N;
			ch->add_cgroup(GROUP_DEPUTY);
		}

		if (version < 15 && ch->act_flags.has(Flags::e)) { // leader
			ch->act_flags -= Flags::e;
			ch->add_cgroup(GROUP_LEADER);
		}

		// removed act_is_npc bit and moved plr_nosummon to A, used to be Q -- Montrey
		if (version < 16 && ch->act_flags.has(Flags::Q)) {
			ch->act_flags -= Flags::Q;
			ch->act_flags += PLR_NOSUMMON;
		}

		// moved PLR_QUESTOR from act_flags to plr_flags at 21, shouldn't have been saved anyway
		if (version < 21) {
			ch->act_flags -= (Flags::K); // PLR_QUESTOR
		}

		if (ch->pcdata->remort_count > 0) {
			ch->add_cgroup(GROUP_AVATAR);
			ch->add_cgroup(GROUP_HERO);
		}

		if (ch->level >= LEVEL_AVATAR)
			ch->add_cgroup(GROUP_AVATAR);

		if (ch->level >= LEVEL_HERO)
			ch->add_cgroup(GROUP_HERO);

		if (ch->clan == nullptr && !IS_IMMORTAL(ch)) {
			ch->remove_cgroup(GROUP_LEADER);
			ch->remove_cgroup(GROUP_DEPUTY);
		}

		if (ch->clan != nullptr)
			ch->add_cgroup(GROUP_CLAN);

		if (!IS_IMMORTAL(ch)) {
			for (int stat = 0; stat < MAX_STATS; stat++)
				ATTR_BASE(ch, stat_to_attr(stat))
				 = std::min(ATTR_BASE(ch, stat_to_attr(stat)), get_max_train(ch, stat));
		}
	}

	/* initialize race */
	if (found) {
		int i, percent;

		if (ch->race == 0)
			ch->race = race_lookup("human");

		// permanent affects from race and raffects aren't saved (in case of changes),
		// rebuild them now
		affect::add_racial_to_char(ch);

		extern void raff_add_to_char(Character *ch, int raff);
		if (ch->pcdata->remort_count > 0)
			for (int c = 0; c < ch->pcdata->remort_count / 10 + 1; c++)
				raff_add_to_char(ch, ch->pcdata->raffect[c]);

		ch->size = pc_race_table[ch->race].size;
		ch->dam_type = 17; /*punch */

		for (i = 0; i < 5; i++) {
			if (pc_race_table[ch->race].skills[i].empty())
				break;

			group_add(ch, pc_race_table[ch->race].skills[i], false);
		}

		/* fix command groups */
		ch->act_flags -= (Flags::e);      /* PLR_LEADER */
		ch->act_flags -= (Flags::N);       /* PLR_DEPUTY */
		ch->add_cgroup(GROUP_PLAYER);

		/* nuke wiznet flags beyond their level, in case they were temp trusted */
		if (!ch->wiznet_flags.empty())
			for (const auto& entry : wiznet_table)
				if (ch->wiznet_flags.has(entry.flag) && GET_RANK(ch) < entry.level)
					ch->wiznet_flags -= entry.flag;

//		reset_char(ch);
		/* adjust hp mana stamina up  -- here for speed's sake */
		percent = (Game::current_time - ch->pcdata->last_logoff) * 25 / (2 * 60 * 60);
		percent = std::min(percent, 100);

		if (percent > 0 && !affect::exists_on_char(ch, affect::type::poison) && !affect::exists_on_char(ch, affect::type::plague)) {
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
	if (  key == literal  )        \
	{                                       \
		field = value;                      \
		fMatch = true;						\
		break;                              \
	}

#if defined(INTKEY)
#undef INTKEY
#endif

#define INTKEY( literal, field, value )                                    \
	if (  key == literal  )        \
	{                                       \
		field  = value;               \
		fMatch = true;                      \
		break;                              \
	}

#if defined(FLAGKEY)
#undef FLAGKEY
#endif

#define FLAGKEY( literal, field, value )                                    \
	if (  key == literal  )        \
	{                                       \
		field  = Flags(value);               \
		fMatch = true;                      \
		break;                              \
	}

#if defined(SKIPKEY)
#undef SKIPKEY
#endif

#define SKIPKEY( literal )                  \
	if (  key == literal  )			\
	{                                       \
		fMatch = true;                      \
		break;                              \
	}	


void fread_player(Character *ch, cJSON *json, int version) {
	if (json == nullptr)
		return;

	// if there are any player-specific fields that are depended on by others in the list,
	// load them right here, and make sure to use SKIPKEY(key) in the switch

	// none


	for (cJSON *o = json->child; o; o = o->next) {
		String key = o->string;
		bool fMatch = false;
		int count = 0; // convenience variable to compact this list, resets with every item

		switch (toupper(key[0])) {
			case 'A':
				if (key == "Alias") { // array of 2-tuples
					// each alias is a 2-tuple (a list)
					for (cJSON *item = o->child; item != nullptr; item = item->next, count++)
						ch->pcdata->alias[item->child->valuestring] = item->child->next->valuestring;

					fMatch = true; break;
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
				if (key == "Cnd") { // 4-tuple
					JSON::get_short(o, &ch->pcdata->condition[COND_DRUNK], "drunk");
					JSON::get_short(o, &ch->pcdata->condition[COND_FULL], "full");
					JSON::get_short(o, &ch->pcdata->condition[COND_THIRST], "thirst");
					JSON::get_short(o, &ch->pcdata->condition[COND_HUNGER], "hunger");
					fMatch = true; break;
				}

				if (key == "Colr") { // array of dicts
					for (cJSON *item = o->child; item != nullptr; item = item->next) {
						int slot = cJSON_GetObjectItem(item, "slot")->valueint;
						JSON::get_short(item, &ch->pcdata->color[slot], "color");
						JSON::get_short(item, &ch->pcdata->bold[slot], "bold");
					}
					fMatch = true; break;
				}

				FLAGKEY("Cgrp",			ch->pcdata->cgroup_flags,			o->valuestring);
				break;
			case 'D':
				STRKEY("Deit",			ch->pcdata->deity,			o->valuestring);
				break;
			case 'E':
				if (key == "ExSk") {
					count = 0;
					for (cJSON *item = o->child; item != nullptr && count < MAX_EXTRACLASS_SLOTS; item = item->next) {
						skill::type sn = skill::lookup(item->valuestring);

						if (sn == skill::type::unknown) {
							Logging::bugf("unknown extraclass skill '%s'", item->valuestring);
							continue;
						}

						const auto entry = skill::lookup(sn);

						if (entry.remort_guild == Guild::none)
							continue;

						ch->pcdata->extraclass[count++] = sn;
					}
					fMatch = true; break;
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
				if (key == "Gr") {
					for (cJSON *item = o->child; item != nullptr; item = item->next) {
						int gn = group_lookup(item->valuestring);

						if (gn < 0) {
							Format::fprintf(stderr, "%s", item->valuestring);
							Logging::bug("Unknown group. ", 0);
							continue;
						}

						gn_add(ch, gn);
					}
					fMatch = true; break;
				}

				STRKEY("GameIn",		ch->pcdata->gamein,			o->valuestring);
				STRKEY("GameOut",		ch->pcdata->gameout,		o->valuestring);
				INTKEY("GlDonated",		ch->pcdata->gold_donated,	o->valueint);
				break;
			case 'H':
				if (key == "HMSP") { // removed in version 16, moved to fread_char
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_HIT), "hit");
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_MANA), "mana");
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_STAM), "stam");
					fMatch = true; break;
				}

				break;
			case 'I':
				if (key == "Ignore") {
					for (cJSON *item = o->child; item != nullptr; item = item->next)
						ch->pcdata->ignore.push_back(item->valuestring);
					fMatch = true; break;
				}

				INTKEY("Id",            ch->pcdata->id,             o->valueint); // moved from char in version 18
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
				INTKEY("Mark",          ch->pcdata->mark_room,      Location(o->valueint));
				INTKEY("Mexp",			ch->pcdata->mud_exp,		o->valueint);
				break;
			case 'N':
				if (key == "Note") {
					JSON::get_long(o, &ch->pcdata->last_note, "note");
					JSON::get_long(o, &ch->pcdata->last_idea, "idea");
					JSON::get_long(o, &ch->pcdata->last_roleplay, "role");
					JSON::get_long(o, &ch->pcdata->last_immquest, "quest");
					JSON::get_long(o, &ch->pcdata->last_changes, "change");
					JSON::get_long(o, &ch->pcdata->last_personal, "pers");
					JSON::get_long(o, &ch->pcdata->last_trade, "trade");
					fMatch = true; break;
				}

				break;
			case 'P':
				STRKEY("Pass",			ch->pcdata->pwd,		o->valuestring);
				INTKEY("PCkills",		ch->pcdata->pckills,	o->valueint);
				INTKEY("PCkilled",		ch->pcdata->pckilled,	o->valueint);
				INTKEY("PKRank",		ch->pcdata->pkrank,		o->valueint);
				INTKEY("Plyd",			ch->pcdata->played,		o->valueint);
				FLAGKEY("Plr",			ch->pcdata->plr_flags,		o->valuestring);
				INTKEY("Pnts",			ch->pcdata->points,		o->valueint);
				break;
			case 'Q':
				if (key == "Query") {
					for (cJSON *item = o->child; item != nullptr && count < MAX_QUERY; item = item->next)
						ch->pcdata->query.push_back(item->valuestring);
					fMatch = true; break;
				}

				INTKEY("QuestPnts",		ch->pcdata->questpoints,			o->valueint);
				INTKEY("QpDonated",		ch->pcdata->questpoints_donated,	o->valueint);
				INTKEY("QuestNext",		ch->pcdata->nextquest,				o->valueint);
				break;
			case 'R':
				if (key == "Raff") {
					for (cJSON *item = o->child; item != nullptr && count < MAX_RAFFECT_SLOTS; item = item->next)
						ch->pcdata->raffect[count++] = item->valueint;
					fMatch = true; break;
				}

				STRKEY("Rank",			ch->pcdata->rank,			o->valuestring);
				INTKEY("RmCt",			ch->pcdata->remort_count,	o->valueint);
				INTKEY("RolePnts",		ch->pcdata->rolepoints,		o->valueint);
				break;
			case 'S':
				if (key == "Sk") {
					for (cJSON *item = o->child; item != nullptr; item = item->next) {
						skill::type sn = skill::lookup(cJSON_GetObjectItem(item, "name")->valuestring);

						if (sn == skill::type::unknown) {
							Logging::bugf("Fread_char: unknown skill '%s'.", cJSON_GetObjectItem(item, "name")->valuestring);
							continue;
						}

						set_learned(ch, sn, cJSON_GetObjectItem(item, "prac")->valueint);
						set_evolution(ch, sn, cJSON_GetObjectItem(item, "evol")->valueint);
					}
					fMatch = true; break;
				}

				INTKEY("SkillPnts",		ch->pcdata->skillpoints,	o->valueint);
				STRKEY("Stus",			ch->pcdata->status,			o->valuestring);
				STRKEY("Spou",			ch->pcdata->spouse,			o->valuestring);
				INTKEY("SQuestNext",	ch->pcdata->nextsquest,		o->valueint);
				break;
			case 'T':
				if (key == "THMS") {
					JSON::get_short(o, &ch->pcdata->trains_to_hit, "hit");
					JSON::get_short(o, &ch->pcdata->trains_to_hit, "mana");
					JSON::get_short(o, &ch->pcdata->trains_to_hit, "stam");
					fMatch = true; break;
				}

				if (key == "Titl") {
					set_title(ch, o->valuestring);
					fMatch = true; break;
				}

				INTKEY("TSex",			ATTR_BASE(ch, APPLY_SEX),	o->valueint); // removed in version 16
				break;
			case 'V':
				FLAGKEY("Video",		ch->pcdata->video_flags,			o->valuestring);
				break;
			case 'W':
				if (key == "WarpLocs") {
					for (cJSON *item = o->child; item != nullptr; item = item->next) {
						const Object *obj = get_warp_crystal(item->valuestring);

						if (!obj)
							continue;

						// use the object's string, in case it has been updated for color or caps
						ch->pcdata->warp_locs.emplace(get_warp_loc_string(obj));
					}
					fMatch = true; break;
				}

				STRKEY("Wspr",			ch->pcdata->whisper,		o->valuestring);
				break;
			default:
				// drop down
				break;
		}

		if (!fMatch)
			Logging::bugf("fread_player: unknown key %s", key);
	}

	// fix up pc-only stuff here
}

// this could be PC or NPC!
void fread_char(Character *ch, cJSON *json, int version)
{
	if (json == nullptr)
		return;

	char buf[MSL];
	Format::sprintf(buf, "Loading %s.", ch->name);
	Logging::log(buf);

	// unlike old pfiles, the order of calls is important here, because we can't
	// guarantee order within the files. If there are any fields that are depended
	// on by others in the list, load them right here, and use SKIPKEY(key) in the list

	for (cJSON *o = json->child; o; o = o->next) {
		String key = o->string;
		bool fMatch = false;
//		int count = 0; // convenience variable to compact this list, resets with every item

		switch (toupper(key[0])) {
			case 'A':
				if (key == "Affc") {
					// these are the non-permanent affects (not racial or remort affect),
					// those are added after the character is loaded
					for (cJSON *item = o->child; item != nullptr; item = item->next) {
						affect::type type = affect::lookup(cJSON_GetObjectItem(item, "name")->valuestring);

						if (type == affect::type::unknown) {
							Logging::bugf("Fread_char: unknown affect '%s'.", cJSON_GetObjectItem(item, "name")->valuestring);
							continue;
						}

						affect::Affect af;
						af.type = type;
						JSON::get_short(item, &af.where, "where");
						JSON::get_short(item, &af.level, "level");
						JSON::get_short(item, &af.duration, "dur");
						JSON::get_short(item, &af.modifier, "mod");
						JSON::get_short(item, &af.location, "loc");
						int bitvector;
						JSON::get_int(item, &bitvector, "bitv");
						af.bitvector(Flags(bitvector));
						JSON::get_short(item, &af.evolution, "evo");
						af.permanent = false;

						affect::copy_to_char(ch, &af);
					}
					fMatch = true; break;
				}

				if (key == "Atrib") {
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_STR), "str");
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_INT), "int");
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_WIS), "wis");
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_DEX), "dex");
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_CON), "con");
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_CHR), "chr");
					fMatch = true; break;
				}

				FLAGKEY("Act",          ch->act_flags,              o->valuestring);
				INTKEY("Alig",			ch->alignment,				o->valueint);
				break;
			case 'C':
				INTKEY("Clan",			ch->clan,					clan_lookup(o->valuestring));
				INTKEY("Cla",			ch->guild,					(Guild)o->valueint);
				FLAGKEY("Comm",			ch->comm_flags,					o->valuestring);
				FLAGKEY("Cnsr",			ch->censor_flags,					o->valuestring);
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
				if (key == "GlDonated" && !ch->is_npc()) { // moved to pcdata in version 21
					ch->pcdata->gold_donated = o->valueint;
					fMatch = true; break;
				}

				INTKEY("Gold_in_bank",	ch->gold_in_bank,			o->valueint);
				INTKEY("Gold",			ch->gold,					o->valueint);
				break;
			case 'H':
				if (key == "HMS") {
					JSON::get_short(o, &ch->hit, "hit");
					JSON::get_short(o, &ch->mana, "mana");
					JSON::get_short(o, &ch->stam, "stam");
					fMatch = true; break;
				}

				if (key == "HMSP") {
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_HIT), "hit");
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_MANA), "mana");
					JSON::get_int(o, &ATTR_BASE(ch, APPLY_STAM), "stam");
					fMatch = true; break;
				}

				INTKEY("Hit",			ATTR_BASE(ch, APPLY_HITROLL), o->valueint); // NPC
				break;
			case 'I':
				if (key == "Id" && !ch->is_npc()) { // moved to pcdata in version 18
					ch->pcdata->id = o->valueint;
					fMatch = true; break;
				}
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
				if (key == "QuestPnts" && !ch->is_npc()) { // moved to pcdata in version 21
					ch->pcdata->questpoints = o->valueint;
					fMatch = true; break;
				}
				if (key == "QpDonated" && !ch->is_npc()) { // moved to pcdata in version 21
					ch->pcdata->questpoints_donated = o->valueint;
					fMatch = true; break;
				}
				if (key == "QuestNext" && !ch->is_npc()) { // moved to pcdata in version 21
					ch->pcdata->nextquest = o->valueint;
					fMatch = true; break;
				}
				break;
			case 'R':
				INTKEY("Race",			ch->race,					race_lookup(o->valuestring));
				INTKEY("Room",			ch->in_room,				Game::world().get_room(Location(o->valueint)));
				FLAGKEY("Revk",			ch->revoke_flags,					o->valuestring);
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
				FLAGKEY("Wizn",			ch->wiznet_flags,					o->valuestring);
				break;
			default:
				// drop down
				break;
		}

		if (!fMatch)
			Logging::bugf("fread_char: unknown key %s", key);
	}
}

// read a single item including its contents
Object * fread_obj(cJSON *json, int version) {
	Object *obj = nullptr;
	cJSON *o;

	if ((o = cJSON_GetObjectItem(json, "Vnum")) != nullptr) {
		ObjectPrototype *index = Game::world().get_obj_prototype(o->valueint);

		if (index == nullptr)
			Logging::bug("Fread_obj: bad vnum %d in fread_obj().", o->valueint);
		else {
			obj = create_object(index, -1);

			if (obj == nullptr)
				Logging::bug("fread_obj: create_object returned nullptr", 0);
		}
	}
	else
		Logging::bug("fread_obj: no vnum field in JSON object", 0);

	if (obj == nullptr) { /* either not found or old style */
		// if the vnum is missing or isn't found in the game anymore,
		// we don't want to just skip loading it because it could have
		// been a container with stuff in it.  instead, we'll create a
		// dummy object that we can blow up later and extract the contents
		// into the next containing object or inventory.
		obj = new Object();
	}

	// version 20 changed weapon flags to affects on the object, and only uses the flags
	// for loading area files.  clear any weapon flag effects we got from the prototype
	// and we'll re-add them based on the bits in value[4], which we preserved from the
	// area file on the prototype
	if (version < 20
	 && obj->item_type == ITEM_WEAPON) {
	 	affect::remove_type_from_obj(obj, affect::type::weapon_flaming);
	 	affect::remove_type_from_obj(obj, affect::type::weapon_frost);
	 	affect::remove_type_from_obj(obj, affect::type::weapon_shocking);
	 	affect::remove_type_from_obj(obj, affect::type::weapon_vampiric);
//	 	affect::remove_type_from_obj(obj, affect::type::weapon_acidic); // acidic added with version 20
	 	affect::remove_type_from_obj(obj, affect::type::poison);
	 	affect::remove_type_from_obj(obj, affect::type::weapon_sharp);
	 	affect::remove_type_from_obj(obj, affect::type::weapon_vorpal);
	 	affect::remove_type_from_obj(obj, affect::type::weapon_two_hands);
	}

	for (cJSON *o = json->child; o; o = o->next) {
		String key = o->string;
		bool fMatch = false;
//		int count = 0; // convenience variable to compact this list, resets with every item

		switch (toupper(key[0])) {
			case 'A':
				if (key == "Affc") {
					// ugh.  when I put this in, it took a while for a bug to show up where
					// the object's affects were not in fact being cleared before the saved
					// affects were applied, so any enchanted gear was having affects multiplied.
					// the easiest way to fix this is just to reset enchantments back to stock.
					// i'm fixing gear for anyone who complains, but we only have like 4 players,
					// so hopefully we can just get past this.
					if (version > 15 && version < 17) {
						fMatch = true; break;
					}

					// this object has different affects than the index, free the old ones
					affect::remove_all_from_obj(obj, true);

					for (cJSON *item = o->child; item != nullptr; item = item->next) {
						affect::Affect af;
						af.type = affect::lookup(cJSON_GetObjectItem(item, "name")->valuestring);

						// go ahead and read the whole affect before error checking, we need to fix up
						JSON::get_short(item, &af.where, "where");
						JSON::get_short(item, &af.level, "level");
						JSON::get_short(item, &af.duration, "dur");
						JSON::get_short(item, &af.modifier, "mod");
						JSON::get_short(item, &af.location, "loc");
						int tbitv;
						JSON::get_int(item, &tbitv, "bitv");
						af.bitvector(Flags(tbitv));
						JSON::get_short(item, &af.evolution, "evo");

						// some old objects made with 'addapply' have things affects without an sn,
						// or modifies nothing with no bits.  also, some items that were enchanted
						// under the old system could have bitvectors storing affects. clean up here,
						// hopefully remove someday.
						// obj name might not be read yet, use vnum
						if (af.where == TO_AFFECTS && af.type == affect::type::none && af.bitvector().empty())
							af.where = TO_OBJECT; // try making it an object apply

						if (af.type == affect::type::unknown) {
							Logging::bugf("Fread_obj: unknown affect type '%s'.", cJSON_GetObjectItem(item, "name")->valuestring);

							// newish pfiles (>17?) don't save temp weapon flags to v4, but older
							// ones might.  strip it just in case
							if (af.where == TO_WEAPON)
								obj->value[4] -= Flags(af.bitvector());

							continue;
						}

						// let the parsing handle TO_OBJECT with no modifiers

						Flags bitvector(af.bitvector());

						// run bitvector down to 0 (unless certain cases)
						// do at least once even if no bitvector
						do {
							if (affect::parse_flags(0, &af, bitvector)) {
								affect::copy_to_obj(obj, &af);
								
								// don't multiply the modifier, just apply to the first bit
								af.location = 0;
								af.modifier = 0;
							}

							af.type = affect::type::none; // reset, in case we're parsing multiple TO_AFFECTS bits
						} while (!bitvector.empty());
					}
					fMatch = true; break;
				}
				break;
			case 'C':
				if (key == "contains") {
					// this mirrors code for fread_objects, but uses obj_to_obj instead of obj_to_char/locker/strongbox,
					// so the function pointer doesn't work.  maybe find a way to fix and condense?
					for (cJSON *item = o->child; item; item = item->next) {
						Object *content = fread_obj(item, version);

						if (content->pIndexData) {
							if (content->condition == 0)
								content->condition = content->pIndexData->condition;

							obj_to_obj(content, obj);
						}
						else {
							// deal with contents and extract
							while (content->contains) {
								Object *c = content->contains;
								content->contains = c->next_content;
								obj_to_obj(c, obj);
							}

							extract_obj(content);
						}
					}
					fMatch = true; break;
				}

				INTKEY("Cond",			obj->condition,				o->valueint);
				INTKEY("Cost",			obj->cost,					o->valueint);
				break;
			case 'D':
				STRKEY("Desc",			obj->description,			o->valuestring);
				break;
			case 'E':
				if (key == "ExDe") {
					for (cJSON *item = o->child; item; item = item->next) {
						ExtraDescr *ed = new ExtraDescr(item->string, item->valuestring);
						ed->next                = obj->extra_descr;
						obj->extra_descr        = ed;
					}
					fMatch = true; break;
				}
				
				FLAGKEY("ExtF",			obj->extra_flags,			o->valueint); // no, not string_to_flags
				break;
			case 'G':
				if (key == "gems") {
					// this mirrors code for fread_objects, but uses obj_to_obj instead of obj_to_char/locker/strongbox,
					// so the function pointer doesn't work.  maybe find a way to fix and condense?
					for (cJSON *item = o->child; item; item = item->next) {
						Object *gem = fread_obj(item, version);

						if (gem->pIndexData) {
							gem::inset(gem, obj);
						}
						else {
							extract_obj(gem);
						}
					}
					fMatch = true; break;
				}
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
				STRKEY("Name",			obj->name,		String(o->valuestring).uncolor().replace("(", "").replace(")", ""));
				break;
			case 'S':
				STRKEY("ShD",			obj->short_descr,			o->valuestring);
				break;	
			case 'T':
				INTKEY("Time",			obj->timer,					o->valueint);
				break;
			case 'V':
				if (key == "Val") {
					int slot = 0;
					for (cJSON *item = o->child; item; item = item->next, slot++)
						obj->value[slot] = item->valueint;

					fMatch = true; break;
				}

				SKIPKEY("Vnum");
				break;
			case 'W':
				FLAGKEY("WeaF",			obj->wear_flags,			o->valueint); // no, not string_to_flags
				INTKEY("Wear",			obj->wear_loc,				o->valueint);
				INTKEY("Wt",			obj->weight,				o->valueint);
				break;
			default:
				break;
		}

		if (!fMatch)
			Logging::bugf("fread_obj: unknown key %s", key);
	}

	// version 20 changed weapon flags to affects on the object, and only uses the flags
	// for loading area files.  we cleared the effects from the prototype above, and
	// since we preserved the bits in value[4] from the area file, we can re-add them
	// here.
	if (version < 20
	 && obj->item_type == ITEM_WEAPON) {
		Flags bitvector = obj->value[4];

		affect::Affect af;
		af.level              = obj->level;
		af.duration           = -1;
		af.evolution          = 1;
		af.permanent          = true;
		af.location           = 0;
		af.modifier           = 0;

		while (!bitvector.empty()) {
			af.type = affect::type::none; // reset every time

			if (affect::parse_flags('W', &af, bitvector)) {
				// could exist as envenom or bladecraft spell, don't add a new perm
				if (!affect::exists_on_obj(obj, af.type))
					affect::copy_to_obj(obj, &af); 
			}
		}

		obj->value[4] = 0; // clear, we won't write it back
	}

	return obj;
}

// read a list of objects and return the head
void fread_objects(Character *ch, cJSON *contains, void (*obj_to)(Object *, Character *), int version) {
	if (contains == nullptr)
		return;

	for (cJSON *item = contains->child; item; item = item->next) {
		Object *content = fread_obj(item, version);

		if (content->pIndexData) {
			if (content->condition == 0)
				content->condition = content->pIndexData->condition;

			(*obj_to)(content, ch);
		}
		else {
			// vnum not found, deal with contents and extract
			while (content->contains) {
				Object *c = content->contains;
				content->contains = c->next_content;
				(*obj_to)(c, ch);
			}

			// need something better, something down the line may rely on a non-null prototype
			extract_obj(content);
		}
	}
}

/* load a pet from the forgotten reaches */
void fread_pet(Character *ch, cJSON *json, int version)
{
	cJSON *o;

	if (json == nullptr)
		return;

	int vnum;

	// error compensation in case their mob goes away, don't poof inventory
	if ((o = cJSON_GetObjectItem(json, "Vnum")) != nullptr) {
		vnum = o->valueint;
	}
	else {
		Logging::bug("fread_pet: no vnum field in JSON object", 0);
		vnum = MOB_VNUM_FIDO;
	}

	MobilePrototype *index = Game::world().get_mob_prototype(vnum);

	if (index == nullptr) {
		Logging::bug("Fread_pet: bad vnum %d in fread_pet().", vnum);
		index = Game::world().get_mob_prototype(MOB_VNUM_FIDO);
	}

	Character *pet = create_mobile(index);

	/* Check for memory error. -- Outsider */
	if (!pet) {
		Logging::bug("Memory error creating mob in fread_pet().", 0);
		return;
	}

	// blow away any affects that are not permanent, from non-racial affect flags
	affect::remove_all_from_char(pet, false);

	fread_char(pet, json, version);

	pet->leader = ch;
	pet->master = ch;
	ch->pet = pet;

	/* adjust hp mana stamina up  -- here for speed's sake */
	int percent;
	percent = (Game::current_time - ch->pcdata->last_logoff) * 25 / (2 * 60 * 60);
	percent = std::min(percent, 100);

	if (percent > 0 && !affect::exists_on_char(ch, affect::type::poison)
	    &&  !affect::exists_on_char(ch, affect::type::plague)) {
		pet->hit    += (GET_MAX_HIT(pet) - pet->hit) * percent / 100;
		pet->mana   += (GET_MAX_MANA(pet) - pet->mana) * percent / 100;
		pet->stam   += (GET_MAX_STAM(pet) - pet->stam) * percent / 100;
	}
}


void update_pc_index(const Character *ch, bool remove)
{
	db_commandf("update_pc_index", "DELETE FROM pc_index WHERE name='%s'", db_esc(ch->name));

	if (!remove)
		db_commandf("update_pc_index",
		            "INSERT INTO pc_index VALUES('%s','%s','%s','%s',%ld,%d,%d,'%s','%s')",
		            db_esc(ch->name),
		            db_esc(ch->pcdata->title),
		            db_esc(ch->pcdata->deity),
		            db_esc(ch->pcdata->deity.uncolor()),
		            ch->pcdata->cgroup_flags.to_ulong(),
		            ch->level,
		            ch->pcdata->remort_count,
		            ch->clan ? db_esc(ch->clan->name) : "",
		            ch->clan && !ch->pcdata->rank.empty() ? db_esc(ch->pcdata->rank) : "");
}


void do_finger(Character *ch, String argument)
{
	char filename[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	String dbuf;

	/* the following vars are read from the player file */
	String email, fingerinfo, last_lsite, name, title, spouse, race, deity;
	int guild, pks, pkd, pkr, aks, akd, level, rmct;
	Flags cgroup, plr;
	time_t last_ltime, last_saved;
	Clan *clan = nullptr;

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		stc("Syntax:\n"
		    "  {Yfinger{x {Gplayer-name{x : displays info about {Gplayer-name{x\n"
		    "  {YFinger private{x     : hides your e-mail address from FINGER\n\n"
		    "  {YShowlast{x           : hides your last login/save times\n", ch);
		return;
	}

	if (arg == "private") {
		if (ch->pcdata->plr_flags.has(PLR_SHOWEMAIL)) {
			stc("Your email will no longer display in your finger info.\n", ch);
			ch->pcdata->plr_flags -= PLR_SHOWEMAIL;
		}
		else {
			stc("Your email will now display in your finger info.\n", ch);
			ch->pcdata->plr_flags += PLR_SHOWEMAIL;
		}

		do_save(ch, "");
		return;
	}

	if (!check_parse_name(arg)) {
		stc("That is not a valid player name.\n", ch);
		return;
	}

	Format::sprintf(filename, "%s%s", PLAYER_DIR, arg.lowercase().capitalize());

	cJSON *root = JSON::read_file(filename);

	if (root == nullptr) {
		stc("That player does not exist.\n", ch);
		return;
	}

	/* initialize variables */
	// strings are empty by default
	guild = pks = pkd = pkr = aks = akd = level = rmct = 0;

	cJSON *section, *item;
	section = cJSON_GetObjectItem(root, "character");
	JSON::get_string(section, &name, "Name");
	JSON::get_string(section, &race, "Race");
	JSON::get_int(section, &level, "Levl");
	JSON::get_int(section, &guild, "Cla");

	if ((item = cJSON_GetObjectItem(section, "Clan")) != nullptr)
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

	if ((item = cJSON_GetObjectItem(section, "Ltim")) != nullptr)
		last_ltime = dizzy_scantime(item->valuestring);
	if ((item = cJSON_GetObjectItem(section, "LSav")) != nullptr)
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
		if (cgroup.has(GROUP_LEADER))
			Format::sprintf(buf, "{BLeader of ");
		else if (cgroup.has(GROUP_DEPUTY))
			Format::sprintf(buf, "{BDeputy of ");
		else
			Format::sprintf(buf, "{BMember of ");

		dbuf += buf;
		Format::sprintf(buf, "%s{x\n", clan->clanname);
		dbuf += buf;
	}

	Format::sprintf(buf, "{C%s ", race.capitalize());
	dbuf += buf;
	Format::sprintf(buf, "{C%s, follower of %s{x\n",
		(Guild)guild == Guild::none ? "adventurer" : guild_table[guild].name.capitalize(), deity);
	dbuf += buf;
	Format::sprintf(buf, "{GArena Record:    %d wins,  %d losses{x\n", aks, akd);
	dbuf += buf;
	Format::sprintf(buf, "{PBlood Trail (%d): %d kills, %d deaths{x\n\n", pkr, pks, pkd);
	dbuf += buf;

	if (!fingerinfo.empty()) {
		Format::sprintf(buf, "{CAdditional Info:{x\n%s{x\n", fingerinfo);
		dbuf += buf;
	}

	if (!spouse.empty()) {
		if (!plr.has(PLR_MARRIED))
			Format::sprintf(buf, "{Y%s is engaged to %s.{x\n", name, spouse);
		else
			Format::sprintf(buf, "{Y%s is happily married to %s.{x\n", name, spouse);

		dbuf += buf;
	}

	if (!email.empty() && (IS_IMMORTAL(ch) || plr.has(PLR_SHOWEMAIL))) {
		Format::sprintf(buf, "{GEmail: %s{x\n", email);
		dbuf += buf;
	}

	if (IS_IMMORTAL(ch) || !plr.has(PLR_NOSHOWLAST)) {
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

/*
 * Parse a name for acceptability.
 */
bool check_parse_name(const String& name)
{
	Clan *clan;

	/*
	 * Reserved words.
	 */
	if (String(
		"all auto immortal self remort imms private someone something the you"
		).has_words(name))
		return false;

	if ((clan = clan_lookup(name)) != nullptr)
		return false;

	/*
	 * Length restrictions.
	 */

	if (strlen(name) <  2)
		return false;

	if (strlen(name) > 12)
		return false;

	/*
	 * Alphanumerics only.
	 * Lock out IllIll twits.
	 */
	{
		bool fIll, adjcaps = false, cleancaps = false;
		unsigned int total_caps = 0;
		fIll = true;

		for (const char *pc = name.c_str(); *pc != '\0'; pc++) {
			if (!isalpha(*pc))
				return false;

			if (isupper(*pc)) { /* ugly anti-caps hack */
				if (adjcaps)
					cleancaps = true;

				total_caps++;
				adjcaps = true;
			}
			else
				adjcaps = false;

			if (tolower(*pc) != 'i' && tolower(*pc) != 'l')
				fIll = false;
		}

		if (fIll)
			return false;

		if (cleancaps || (total_caps > (strlen(name)) / 2 && strlen(name) < 3))
			return false;
	}

	return true;
}
