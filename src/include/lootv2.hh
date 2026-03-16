#pragma once
/*******************************************
 *			Dynamic Loot System			   *
 *				  by Vegita                *
 *******************************************/

#include "affect/Type.hh"
#include "String.hh"

Object *generate_eq(int objlevel);

/**
 * @defgroup LootArmorVnums Blank object vnums for armor pieces
 * Vnums are objects in vegita.are
 * @{
 */
constexpr int OBJ_VNUM_LIGHT      = 24356;
constexpr int OBJ_VNUM_FINGER     = 24357;
constexpr int OBJ_VNUM_NECK       = 24358;
constexpr int OBJ_VNUM_TORSO      = 24359;
constexpr int OBJ_VNUM_HEAD       = 24360;
constexpr int OBJ_VNUM_LEGS       = 24361;
constexpr int OBJ_VNUM_FEET       = 24362;
constexpr int OBJ_VNUM_HANDS      = 24363;
constexpr int OBJ_VNUM_ARMS       = 24364;
constexpr int OBJ_VNUM_SHIELD     = 24365;
constexpr int OBJ_VNUM_BODY       = 24366;
constexpr int OBJ_VNUM_WAIST      = 24367;
constexpr int OBJ_VNUM_WRIST      = 24368;
constexpr int OBJ_VNUM_FLOATING   = 24355;
constexpr int OBJ_VNUM_HOLD       = 24369;
/** @} */

/**
 * @defgroup LootWeaponVnums Blank object vnums for weapon types
 * Vnums are objects in vegita.are
 * @{
 */
constexpr int OBJ_VNUM_WIELD_AXE      = 24370;
constexpr int OBJ_VNUM_WIELD_DAGGER   = 24371;
constexpr int OBJ_VNUM_WIELD_EXOTIC   = 24372;
constexpr int OBJ_VNUM_WIELD_FLAIL    = 24373;
constexpr int OBJ_VNUM_WIELD_MACE     = 24374;
constexpr int OBJ_VNUM_WIELD_POLEARM  = 24375;
constexpr int OBJ_VNUM_WIELD_SPEAR    = 24376;
constexpr int OBJ_VNUM_WIELD_SWORD    = 24377;
constexpr int OBJ_VNUM_WIELD_WHIP     = 24378;
constexpr int OBJ_VNUM_WIELD_BOW      = 24379;
/** @} */

/**
 * @defgroup LootWeaponArmorTypes Weapon and armor type definitions
 * Armor is 100-199
 * Weapons are 200-299
 * @{
 */
constexpr int ARMOR_LIGHT      = 100;
constexpr int ARMOR_FINGER     = 101;
constexpr int ARMOR_NECK       = 102;
constexpr int ARMOR_TORSO      = 103;
constexpr int ARMOR_HEAD       = 104;
constexpr int ARMOR_LEGS       = 105;
constexpr int ARMOR_FEET       = 106;
constexpr int ARMOR_HANDS      = 107;
constexpr int ARMOR_ARMS       = 108;
constexpr int ARMOR_SHIELD     = 109;
constexpr int ARMOR_BODY       = 110;
constexpr int ARMOR_WAIST      = 111;
constexpr int ARMOR_WRIST      = 112;
constexpr int ARMOR_FLOATING   = 113;
constexpr int ARMOR_HOLD       = 114;
constexpr int WIELD_AXE        = 200;
constexpr int WIELD_DAGGER     = 201;
constexpr int WIELD_EXOTIC     = 202;
constexpr int WIELD_FLAIL      = 203;
constexpr int WIELD_MACE       = 204;
constexpr int WIELD_POLEARM    = 205;
constexpr int WIELD_SPEAR      = 206;
constexpr int WIELD_SWORD      = 207;
constexpr int WIELD_WHIP       = 208;
constexpr int WIELD_BOW        = 209;
/** @} */


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
	bool scaling;
};

// prefix/suffix tables
extern const std::multimap<int, affect::type> prefixes_allowed;
extern const std::multimap<int, affect::type> suffixes_allowed;
extern const std::map<affect::type, mod_t> mod_table;

// set gear info

struct set_gear {
	int set_name;			///<use exact set name (IE:SET_MAGE_INVOKER)
	String display;			///<what is displayed to player
	String set1;			///<1 pc bonus
	String set2;			///<2 pc bonus
	String set3;			///<3 pc bonus
	String set4;			///<4 pc bonus
	String set5;			///<5 pc bonus
};

extern const std::vector<set_gear> set_table;
