#pragma once

//#include "declare.hh"
#include "String.hh"
#include "Sector.hh"
#include "Guild.hh"

class Clan;
class Vnum;

Clan *clan_vnum_lookup(const Vnum& vnum);
Clan *	clan_lookup(const String& name);
int		position_lookup(const String& name);
int 	sex_lookup(const String& name);
int 	size_lookup(const String& name);
String	condition_lookup(int condition);
int		liq_lookup(const String& name);
String	sector_lookup(Sector type);
int     liq_lookup(const String& name);
int     weapon_lookup(const String& name);
int     item_lookup(const String& name);
int     attack_lookup(const String& name);
int     race_lookup(const String& name);
Guild     guild_lookup(const String& name);
int     deity_lookup(const String& name);
