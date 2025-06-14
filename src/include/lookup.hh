#pragma once

//#include "declare.hh"
#include "String.hh"
#include "Sector.hh"
#include "Guild.hh"

class Clan;
class Vnum;

Clan *clan_vnum_lookup	args((const Vnum& vnum));
Clan *	clan_lookup		args((const String& name));
int		position_lookup		args((const String& name));
int 	sex_lookup		args((const String& name));
int 	size_lookup		args((const String& name));
String	condition_lookup	args((int condition));
int		liq_lookup		args((const String& name));
String	sector_lookup		args((Sector type));
int     liq_lookup      args( ( const String& name) );
int     weapon_lookup   args( ( const String& name) );
int     item_lookup     args( ( const String& name) );
int     attack_lookup   args(( const String& name) );
int     race_lookup     args(( const String& name) );
Guild     guild_lookup    args(( const String& name) );
int     deity_lookup    args(( const String& name) );
