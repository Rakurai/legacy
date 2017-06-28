#pragma once
/*******************************************
 *			Dynamic Loot System			   *
 *				  by Vegita                *
 *******************************************/

#include "affect/Type.hh"
#include "String.hh"

Object *generate_eq(int level);

//blank object vnums for armor pieces (vnums in vegita.are)
#define OBJ_VNUM_LIGHT				24356				
#define OBJ_VNUM_FINGER				24357	
#define OBJ_VNUM_NECK				24358
#define OBJ_VNUM_TORSO				24359
#define OBJ_VNUM_HEAD				24360
#define OBJ_VNUM_LEGS				24361
#define OBJ_VNUM_FEET				24362
#define OBJ_VNUM_HANDS				24363
#define OBJ_VNUM_ARMS				24364
#define OBJ_VNUM_SHIELD				24365
#define OBJ_VNUM_BODY				24366
#define OBJ_VNUM_WAIST				24367
#define OBJ_VNUM_WRIST				24368
#define OBJ_VNUM_FLOATING			24355
#define OBJ_VNUM_HOLD				24369 

//blank object vnums for weapon types (vnums in vegita.are)
#define OBJ_VNUM_WIELD_AXE				24370
#define OBJ_VNUM_WIELD_DAGGER			24371
#define OBJ_VNUM_WIELD_EXOTIC			24372
#define OBJ_VNUM_WIELD_FLAIL			24373
#define OBJ_VNUM_WIELD_MACE				24374
#define OBJ_VNUM_WIELD_POLEARM			24375
#define OBJ_VNUM_WIELD_SPEAR			24376
#define OBJ_VNUM_WIELD_SWORD			24377
#define OBJ_VNUM_WIELD_WHIP				24378


//weapon and armor type defines 
//armor is 100-115, weapons 200-208
#define ARMOR_LIGHT			100
#define ARMOR_FINGER		101
#define ARMOR_NECK			102
#define ARMOR_TORSO			103
#define ARMOR_HEAD			104
#define ARMOR_LEGS			105
#define ARMOR_FEET			106
#define ARMOR_HANDS			107
#define ARMOR_ARMS			108
#define ARMOR_SHIELD		109
#define ARMOR_BODY			110
#define ARMOR_WAIST			111
#define ARMOR_WRIST			112
#define ARMOR_FLOATING		113
#define ARMOR_HOLD			114
#define WIELD_AXE			200
#define WIELD_DAGGER		201
#define WIELD_EXOTIC		202
#define WIELD_FLAIL			203
#define WIELD_MACE			204
#define WIELD_POLEARM		205
#define WIELD_SPEAR			206
#define WIELD_SWORD			207
#define WIELD_WHIP			208


/*
Begin Equipment mod tables.
*/

// base name tables
extern const std::multimap<int, String> base_name_table;

// legendary name tables
extern const std::vector<String> legendary_base_pool_table;
extern const std::map<int, std::vector<String> > legendary_name_table;

struct mod_t {
	String  text;
	int group;
	int rarity;
	int af_where;
	int af_loc;
	int af_mod_min;
	int af_mod_max;
};

// prefix/suffix tables
extern const std::multimap<int, affect::type> prefixes_allowed;
extern const std::multimap<int, affect::type> suffixes_allowed;
extern const std::map<affect::type, mod_t> mod_table;
