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

#include <vector>

#include "Clan.hh"
#include "declare.hh"
#include "macros.hh"
#include "merc.hh"
#include "String.hh"
#include "tables.hh"

Clan *clan_vnum_lookup(int vnum)
{
	Clan *c;

	for (c = clan_table_head->next; c != clan_table_tail; c = c->next)
		if (vnum >= c->area_minvnum && vnum <= c->area_maxvnum)
			return c;

	return nullptr;
}

Clan *clan_lookup(const String& name)
{
	Clan *iterator;

	if (name.empty())
		return nullptr;

	iterator = clan_table_head->next;

	while (iterator != clan_table_tail) {
		if (name.is_prefix_of(iterator->name))
			return iterator;

		iterator = iterator->next;
	}

	return nullptr;
}

int position_lookup(const String& name)
{
	int pos;

	for (pos = 0; pos < position_table.size(); pos++) {
		if (LOWER(name[0]) == LOWER(position_table[pos].name[0])
		    &&  name.is_prefix_of(position_table[pos].name))
			return pos;
	}

	return -1;
}

int sex_lookup(const String& name)
{
	int sex;

	for (sex = 0; sex < sex_table.size(); sex++) {
		if (LOWER(name[0]) == LOWER(sex_table[sex].name[0])
		    &&  name.is_prefix_of(sex_table[sex].name))
			return sex;
	}

	return -1;
}

int size_lookup(const String& name)
{
	int size;

	for (size = 0; size < size_table.size(); size++) {
		if (LOWER(name[0]) == LOWER(size_table[size].name[0])
		    &&  name.is_prefix_of(size_table[size].name))
			return size;
	}

	return -1;
}

String condition_lookup(int condition)
{
	if (condition >= 100)
		return "perfect";
	else if (condition >= 90)
		return "good";
	else if (condition >= 75)
		return "average";
	else if (condition >= 50)
		return "worn";
	else if (condition >= 25)
		return "damaged";
	else if (condition >= 10)
		return "broken";
	else if (condition >= 0)
		return "ruined";
	else if (condition >= -1)
		return "indestructable";
	else
		return "unknown";
}

String sector_lookup(int type)
{
	int i;

	for (i = 0; i < sector_table.size(); i++)
		if (sector_table[i].type == type)
			return sector_table[i].name;

	return "unknown";
}

/* returns race number */
int race_lookup(const String& name)
{
	int race;

	for (race = 0; race < race_table.size(); race++) {
		if (LOWER(name[0]) == LOWER(race_table[race].name[0])
		    &&  name.is_prefix_of(race_table[race].name))
			return race;
	}

	return 0;
}

int weapon_lookup(const String& name)
{
	int type;

	for (type = 0; type < weapon_table.size(); type++) {
		if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
		    &&  name.is_prefix_of(weapon_table[type].name))
			return type;
	}

	return -1;
}

int get_weapon_type(const String& name)
{
	int type;

	for (type = 0; type < weapon_table.size(); type++) {
		if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
		    &&  name.is_prefix_of(weapon_table[type].name))
			return weapon_table[type].type;
	}

	return WEAPON_EXOTIC;
}

int item_lookup(const String& name)
{
	int type;

	for (type = 0; type < item_table.size(); type++) {
		if (LOWER(name[0]) == LOWER(item_table[type].name[0])
		    &&  name.is_prefix_of(item_table[type].name))
			return item_table[type].type;
	}

	return -1;
}

int attack_lookup(const String& name)
{
	int att;

	for (att = 0; att < attack_table.size(); att++) {
		if (LOWER(name[0]) == LOWER(attack_table[att].name[0])
		    &&  name.is_prefix_of(attack_table[att].name))
			return att;
	}

	return 0;
}

/* returns class number */
int class_lookup(const String& name)
{
	int cls;

	for (cls = 0; cls < MAX_CLASS; cls++) {
		if (name.is_prefix_of(class_table[cls].name))
			return cls;
	}

	return -1;
}

int liq_lookup(const String& name)
{
	int liq;

	for (liq = 0; liq < liq_table.size(); liq++)
		if (LOWER(name[0]) == LOWER(liq_table[liq].name[0])
		    && name.is_prefix_of(liq_table[liq].name))
			return liq;

	return -1;
}
