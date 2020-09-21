/****************************************************
 *              Dynamic Loot System                 *
 *                   By Vegita                      *
 *                   Version 2                      *
 *                                                  *
 *  Special thanks to Montrey for helping me with   *
 *  all the problems I've had while making this     *
 *  and for putting up with my noob ass             *
 *                                                  *
 *          Coded for Legacy 2016-2017              *
 ****************************************************/


 /*
 Notes

 quality prefix, prefix, prefix, base_name, suffix

 (Legendary) Holy Fiery Shocking Siege Axe of the Blighted

 Mods:
 str, int, wis, dex, con, chr
 hp, mana, stam
 ac: pierce, bash, slash, magic
 hitroll, damroll, saves
*/

#include <vector>
#include <map>

#include "affect/Affect.hh"
#include "Game.hh"
#include "Area.hh"
#include "World.hh"
#include "Logging.hh"
#include "lootv2.hh"
#include "Object.hh"
#include "random.hh"
#include "World.hh"
#include "merc.hh"
#include "ObjectPrototype.hh"

Object *generate_armor(int ilevel, int objlevel, int item_qual, int& eq_type);
Object *generate_weapon(int ilevel, int objlevel, int item_qual, int& eq_type);
int random_prototype(String key);
void add_base_stats(Object *obj, int ilevel, int item_qual);
const String roll_mod(Object *obj, int eq_type, const std::multimap<int, affect::type>& mods_allowed);
const String get_base_name(int eq_type, int ilevel);
const String get_legendary_name(int eq_type);

#define EQ_QUALITY_SET		 0
#define EQ_QUALITY_UNIQUE	 1
#define EQ_QUALITY_LEGENDARY 2
#define EQ_QUALITY_RARE      3
#define EQ_QUALITY_UNCOMMON  4
#define EQ_QUALITY_NORMAL    5

struct eq_quality_t {
	int chance;
	String str;
	int max_prefixes;
	int max_suffixes;
};

// these are ordered by rarity, it will first roll for the first entry then
// keep rolling as it fails to get each level.  leave bottom one at 100%
const std::vector<eq_quality_t> eq_quality_table {
	{   100, "{x({GS{He{Gt{x){x ",         	0,  0 }, 
	{   3, "{x({YUn{biq{Yue{x){x ",         0,  0 }, // orig 3 unique, to be handles specially, they have unique powers.
	{   4, "{x({CLe{Tge{gn{Tda{Cry{x){x ",  3,  1 }, // legendary
	{  10, "{x({BR{Nar{Be{x){x ",           2,  1 }, // rare
	{  25, "{x({GUn{Hcomm{Gon{x){x ",       1,  0 }, // uncommon
	{ 100, "",                              0,  0 }, // normal, leave this at 100%
};

struct eq_roll_t {
	int type;
	int vnum;
};

const std::vector<eq_roll_t> armor_eq_rolls {
	{ ARMOR_NECK,     OBJ_VNUM_NECK     },
	{ ARMOR_TORSO,    OBJ_VNUM_TORSO    },
	{ ARMOR_HEAD,     OBJ_VNUM_HEAD     },
	{ ARMOR_LEGS,     OBJ_VNUM_LEGS     },
	{ ARMOR_FEET,     OBJ_VNUM_FEET     },
	{ ARMOR_HANDS,    OBJ_VNUM_HANDS    },
	{ ARMOR_ARMS,     OBJ_VNUM_ARMS     },
	{ ARMOR_SHIELD,   OBJ_VNUM_SHIELD   },
	{ ARMOR_BODY,     OBJ_VNUM_BODY     },
	{ ARMOR_WAIST,    OBJ_VNUM_WAIST    },
	{ ARMOR_WRIST,    OBJ_VNUM_WRIST    },
};

const std::vector<eq_roll_t> accessory_eq_rolls {
	{ ARMOR_LIGHT,    OBJ_VNUM_LIGHT    },
	{ ARMOR_FINGER,   OBJ_VNUM_FINGER   },
	{ ARMOR_FLOATING, OBJ_VNUM_FLOATING },
	{ ARMOR_HOLD,     OBJ_VNUM_HOLD     },
};

const std::vector<eq_roll_t> weapon_eq_rolls {
	{ WIELD_AXE,     OBJ_VNUM_WIELD_AXE     },
	{ WIELD_DAGGER,  OBJ_VNUM_WIELD_DAGGER  },
	{ WIELD_EXOTIC,  OBJ_VNUM_WIELD_EXOTIC  },
	{ WIELD_FLAIL,   OBJ_VNUM_WIELD_FLAIL   },
	{ WIELD_MACE,    OBJ_VNUM_WIELD_MACE    },
	{ WIELD_POLEARM, OBJ_VNUM_WIELD_POLEARM },
	{ WIELD_SPEAR,   OBJ_VNUM_WIELD_SPEAR   },
	{ WIELD_SWORD,   OBJ_VNUM_WIELD_SWORD   },
	{ WIELD_WHIP,    OBJ_VNUM_WIELD_WHIP    },
};

struct eq_meta_t {
	int chance;
	const std::vector<eq_roll_t>& eq_rolls;
};

// these are ordered by rarity, it will first roll for the first entry then
// keep rolling as it fails to get each level.  leave bottom one at 100%
const std::vector<eq_meta_t> eq_meta_table {
	{ 15, accessory_eq_rolls }, // 15% of 100% = 15%
	{ 35, weapon_eq_rolls },    // 35% of  85% = 30%
	{ 100, armor_eq_rolls  },   // leave at 100%
};

Object *generate_eq(int objlevel){
	// get ilevel from 0-8
	// we want item level 0-19 to be ilevel 0, 20-29 is ilevel 1, etc
	// objlevel  1:   (1-10)/10 = 0
	// objlevel  9:   (9-10)/10 = 0
	// objlevel 10:  (10-10)/10 = 0
	// objlevel 19:  (19-10)/10 = 0
	// objlevel 20:  (20-10)/10 = 1
	// objlevel 29:  (29-10)/10 = 1
	// objlevel 30:  (30-10)/10 = 2
	// ...
	// level 100 = ilevel 9, but we cut off to the size of the table
	// in generate_weapon/armor, so level 90 gets max level stuff
	int ilevel = (objlevel-10) / 10;
	ilevel = URANGE(0, ilevel, 8); // just in case :P

	unsigned int meta_index, item_qual; // to use down below
	

	// figure out our eq set to choose from
	// tries successive rolls from most rare to least
	for (meta_index = 0;                           // start with trying for most rare
	     meta_index < eq_meta_table.size()-1;      // stop at the last entry (common)
	     meta_index++)                             // move down by one each time
		if (roll_chance(eq_meta_table[meta_index].chance)) // try getting this level
			break;

	// now roll for a specific eq type within the set
	int roll    = number_range(0, eq_meta_table[meta_index].eq_rolls.size()-1);
	int eq_type = eq_meta_table[meta_index].eq_rolls[roll].type;
	int vnum    = eq_meta_table[meta_index].eq_rolls[roll].vnum;


	/*quality chance.
	Also sets prefix max based on quality
	Suffix max is locked at one due to balancing.
	*/
	// find a quality from 0-3
	// tries successive rolls from most rare to least
	for (item_qual = 0;                           // start with trying for legendary
	     item_qual < eq_quality_table.size()-1;   // stop at the last entry (common)
	     item_qual++)                             // move down by one each time
		if (roll_chance(eq_quality_table[item_qual].chance)) // try getting this level
			break;

	
	// create the object
	if (item_qual == EQ_QUALITY_UNIQUE){
		vnum = random_prototype("uniquegen");
		if (vnum == 0){
			Logging::bug("generate_eq: random_prototype failed to pass vnum %d", vnum);
			return nullptr;
		}
	}
	
	if (item_qual == EQ_QUALITY_SET){
		vnum = random_prototype("setgen");
		if (vnum == 0){
			Logging::bug("generate_eq: random_prototype failed to pass vnum %d", vnum);
			return nullptr;
		}
	}
		
	Object *obj = create_object(Game::world().get_obj_prototype(vnum), objlevel);

	if (obj == nullptr) {
		Logging::bug("Error making obj with vnum %d in generate_eq.", vnum);
		return nullptr;
	}

	//if ((item_qual != EQ_QUALITY_UNIQUE) && (item_qual != EQ_QUALITY_SET))
		if (obj->level <= 91) 
			obj->level = objlevel;

	
	// roll a name for the object
	if ((item_qual != EQ_QUALITY_UNIQUE) && (item_qual != EQ_QUALITY_SET)){
		if (item_qual == EQ_QUALITY_LEGENDARY)
			obj->name = get_legendary_name(eq_type);
		else
			obj->name = get_base_name(eq_type, ilevel);
	}


	// add some common stats for all eq
	//if ((item_qual != EQ_QUALITY_UNIQUE) && (item_qual != EQ_QUALITY_SET))
	if (obj->level <= 91)
		add_base_stats(obj, ilevel, item_qual);


	// add prefixes
	for (int i = 0; i < eq_quality_table[item_qual].max_prefixes; i++) {
		String str = roll_mod(obj, eq_type, prefixes_allowed);

		if (item_qual != EQ_QUALITY_LEGENDARY && !str.empty())
			obj->name = str + "{x " + obj->name; // prepend
	}


	// add suffixes
	for (int i = 0; i < eq_quality_table[item_qual].max_suffixes; i++) {
		String str = roll_mod(obj, eq_type, suffixes_allowed);

		if (item_qual != EQ_QUALITY_LEGENDARY && !str.empty())
			obj->name += " " + str + "{x"; // append
	}


	// stick the rarity string on the front
	if ((item_qual != EQ_QUALITY_UNIQUE) && (item_qual != EQ_QUALITY_SET))
		obj->name = eq_quality_table[item_qual].str + obj->name; // before prefixes

	// finish up
	if ((item_qual != EQ_QUALITY_UNIQUE) && (item_qual != EQ_QUALITY_SET)){
		obj->description = obj->name;
		obj->short_descr = obj->name;
	}

	if (item_qual == EQ_QUALITY_UNIQUE) //strip uniquegen keyword from item 
		obj->name = obj->name.replace("uniquegen", "").strip();
	if (item_qual == EQ_QUALITY_SET)
		obj->name = obj->name.replace("setgen", "").strip();
	
	obj->name = obj->name.uncolor().replace("(", "").replace(")", "");
	return obj;
}

const String get_base_name(int eq_type, int ilevel) {
	// annoying way to have to write this:  first count all the things matching
	// 'eq_type' in the multimap to make sure there are some.
	int count = base_name_table.count(eq_type);

	if (count == 0) {
		Logging::bug("no base names for type %d", eq_type);
		return "";
	}

	// limit our search to reasonable bounds
	count = URANGE(0, ilevel, count - 1);

	// then get all of the values matching eq_type
	auto base_names = base_name_table.equal_range(eq_type);//origin -1

	// get an iterator and increment it to the value we want (this is stupid)
	auto it = base_names.first;
	for (int i = 0; i < count; i++)
		++it;

	// finally base_name is the second part of the map pair
	return it->second;
}

const String get_legendary_name(int eq_type) {
	auto legendary_names = legendary_name_table.find(eq_type)->second;

	String ret = legendary_base_pool_table[number_range(0, legendary_base_pool_table.size()-1)];
	ret += " ";
	ret += legendary_names[number_range(0, legendary_names.size()-1)];
	ret += "{x";
	return ret;
}

const String roll_mod(Object *obj, int eq_type, const std::multimap<int, affect::type>& mods_allowed) {
	// annoying way to have to write this:  first count all the things matching
	// 'eq_type' in the multimap to make sure there are some.
	int count = mods_allowed.count(eq_type);

	if (count == 0) {
		return ""; // fine, none allowed for this type
	}

	// then get all of the values matching eq_type
	auto mods = mods_allowed.equal_range(eq_type);
	affect::type mod_type;

	while (true) {
		auto it = mods.first;
		int index = number_range(0, count - 1); // choose a random mod

		// increment it up to the mod
		for (int i = 0; i < index; i++)
			++it;

		// then roll to see if we can get this mod based on rarity
		mod_type = it->second;

		auto mod = mod_table.find(mod_type);

		// this shouldn't happen, can't find mod in table.  try again
		if (mod == mod_table.cend())
			continue;

		if (roll_chance(mod->second.rarity))
			break; // got it!
	}

	auto mod = mod_table.find(mod_type)->second; // already tested above

	affect::Affect af;
	af.where        = mod.af_where;
	af.type         = mod_type;
	af.location     = mod.af_loc;
	af.modifier     = number_range(mod.af_mod_min, mod.af_mod_max);

	if (mod.scaling) {
		float scalar = obj->level / 75.0; // range 1/75 to 100/75, constrain below
		if (af.modifier > 0)
			af.modifier = URANGE(1, af.modifier * scalar, mod.af_mod_max);
		else if (af.modifier < 0)
			af.modifier = URANGE(mod.af_mod_max, af.modifier * scalar, -1);
	}

	af.duration     = -1;
	af.evolution    = 1;
	af.level        = obj->level;
	af.bitvector(0);

	// prevent 'sturdy sturdy'
	if (affect::exists_on_obj(obj, af.type)) {
		affect::join_to_obj(obj, &af);
		return "";
	}

	affect::copy_to_obj(obj, &af);
	return mod.text;
}

int random_prototype(String key)
{
	int pick = 0;

	// two passes, the first will count the eligible vnums and select one, the second
	// will count up to that choice and return the vnum
	for (int pass = 1; pass <= 2; pass++) {

		int count = 0;

		// loop through all areas
		for (const auto& area_pair : Game::world().areas){

			// loop through all prototypes
			for (const auto& proto_pair : area_pair.second->obj_prototypes){
								
				ObjectPrototype *proto = proto_pair.second; //ObjectPrototype
			
				if (!proto->name.has_words(key))
					continue;

				if (pass == 2 && count == pick)				//got our unique
					return proto->vnum.value();              // return the vnum

				count++;
			}
		}

		if (count == 0)
			break;

		// only pass 1 should get here
		pick = number_range(0, count);					//pick is range between 0 and count it found above
	}

	return 0;
}

void add_base_stats(Object *obj, int ilevel, int item_qual) {
	// item type specific parts
	if (obj->item_type == ITEM_ARMOR){
		int ac_min, ac_max;
		switch (ilevel) {
			case 0:  ac_min =  1; ac_max =  5; break; // level 1-19
			case 1:  ac_min =  5; ac_max =  9; break; // level 20-29
			case 2:  ac_min =  8; ac_max = 13; break; // level 30-39
			case 3:  ac_min = 11; ac_max = 17; break; // level 40-49
			case 4:  ac_min = 13; ac_max = 22; break; // level 50-59
			case 5:  ac_min = 16; ac_max = 26; break; // level 60-69
			case 6:  ac_min = 19; ac_max = 30; break; // level 70-79
			case 7:  ac_min = 24; ac_max = 35; break; // level 80-89
			default: ac_min = 28; ac_max = 40; break; // level 90+
		}
		
		// following makes additions to the rolls based on the quality.
		// uniques get 10% bump
		// set pieces get a 15% bump
		switch (item_qual) {
			case EQ_QUALITY_SET :
				ac_min += (ac_min * 15 / 100);
				ac_max += (ac_max * 15 / 100);
				break;
			
			case EQ_QUALITY_UNIQUE :
				ac_min += (ac_min * 10 / 100);
				ac_max += (ac_max * 10 / 100);
				break;
				
			default : 
				//do nothing.
				break;
		}
		
		obj->value[0] = number_range(ac_min, ac_max);
		obj->value[1] = number_range(ac_min, ac_max);
		obj->value[2] = number_range(ac_min, ac_max);
		obj->value[3] = number_range(ac_min, ac_max);
	}
	else if (obj->item_type == ITEM_WEAPON) {
		switch (item_qual) {
			case EQ_QUALITY_SET :
				// hero 14d14 (105) to 15d15 (120)
				// lv50 10d10  (55) to 11d11 (66)
				obj->value[1] = obj->level / 10 + number_range(5,6); 
				obj->value[2] = obj->level / 10 + number_range(5,6);
				break;
			
			case EQ_QUALITY_UNIQUE :
				// hero 13d13 (91) to 15d15 (120)
				// lv50 9d9    (45) to 11d11 (66)
				obj->value[1] = obj->level / 10 + number_range(4,6); 
				obj->value[2] = obj->level / 10 + number_range(4,6);
				break;
			
			case EQ_QUALITY_LEGENDARY :
				// hero 12d12 (78) to 14d14 (105)
				// lv50  7d7  (28) to 10d10 (55)
				obj->value[1] = obj->level / 10 + number_range(3,5); 
				obj->value[2] = obj->level / 10 + number_range(3,5);
				break;
				
			case EQ_QUALITY_RARE :
				// hero 11d11 (66) to 13d13 (91)
				// lv50  6d6  (21) to  9d9  (45)
				obj->value[1] = obj->level / 10 + number_range(2,4); 
				obj->value[2] = obj->level / 10 + number_range(2,4);
				break;
			
			case EQ_QUALITY_UNCOMMON :
				// hero 10d10 (55) to 12d12 (78)
				// lv50  5d5  (15) to  8d8  (36)
				obj->value[1] = obj->level / 10 + number_range(1,3); 
				obj->value[2] = obj->level / 10 + number_range(1,3);
				break;
			default:
				// hero  9d9  (45) to 11d11 (66)
				// lv50  4d4  (10) to  7d7  (28)
				obj->value[1] = obj->level / 10 + number_range(0,2);  
				obj->value[2] = obj->level / 10 + number_range(0,2);
				break;
		}
	}			

	// everything gets a chance at base applies
	affect::Affect af;
	af.where      = TO_OBJECT;
	af.type       = affect::type::none;
	af.level      = obj->level;
	af.duration   = -1;
	af.bitvector(0);
	af.evolution  = 1;
	
	struct stat_t {
		int hp_min, hp_max;
		int mana_min, mana_max;
		int stam_min, stam_max;
		int hitroll_min, hitroll_max;
		int damroll_min, damroll_max;
	};

	static const std::vector<stat_t> base_stats_table = {
	//   |    hp   |   mana   |   stam   |  hitroll | damroll |
		{   1,   5,    1,   5,    1,   5,    1,   2,    1,   2 }, // level 1-19
		{  12,  22,   12,  22,   12,  22,    1,   3,    1,   3 }, // level 20-29
		{  18,  33,   18,  33,   18,  33,    2,   4,    2,   4 }, // level 30-39
		{  24,  44,   24,  44,   24,  44,    2,   5,    2,   5 }, // level 40-49
		{  30,  55,   30,  55,   30,  55,    3,   6,    3,   6 }, // level 50-59
		{  36,  66,   36,  66,   36,  66,    3,   7,    3,   7 }, // level 60-69
		{  42,  77,   42,  77,   42,  77,    4,   9,    4,   9 }, // level 70-79
		{  60, 120,   60, 120,   60, 120,    5,  12,    5,  12 }, // level 80-89
		{ 100, 200,  100, 200,  100, 200,   10,  25,   10,  25 }, // level 90+
	};
	
	/*switch (item_qual) {
			case EQ_QUALITY_SET :
				
				break;
			
			case EQ_QUALITY_UNIQUE :
				
				break;

			default:
				// do nothing
				break;
	}*/
	
	if (roll_chance(30)){
		af.location   = APPLY_HIT;
		af.modifier   = number_range(base_stats_table[ilevel].hp_min, base_stats_table[ilevel].hp_max);
		affect::join_to_obj(obj, &af);
		Logging::bug("generate_eq: apply hit %d", af.modifier);
	}

	if (roll_chance(20)){
		af.location   = APPLY_MANA;
		af.modifier   = number_range(base_stats_table[ilevel].mana_min, base_stats_table[ilevel].mana_max);
		affect::join_to_obj(obj, &af);
		Logging::bug("generate_eq: apply mana %d", af.modifier);
	}

	if (roll_chance(20)){
		af.location   = APPLY_STAM;
		af.modifier   = number_range(base_stats_table[ilevel].stam_min, base_stats_table[ilevel].stam_max);
		affect::join_to_obj(obj, &af);
		Logging::bug("generate_eq: apply stam %d", af.modifier);
	}
	
	if (roll_chance(30)){
		af.location   = APPLY_HITROLL;
		af.modifier   = number_range(base_stats_table[ilevel].hitroll_min, base_stats_table[ilevel].hitroll_max);
		affect::join_to_obj(obj, &af);
		Logging::bug("generate_eq: apply hitroll %d", af.modifier);
	}
	
	if (roll_chance(30)){
		af.location   = APPLY_DAMROLL;
		af.modifier   = number_range(base_stats_table[ilevel].damroll_min, base_stats_table[ilevel].damroll_max);
		affect::join_to_obj(obj, &af);
		Logging::bug("generate_eq: apply damroll %d", af.modifier);
	}
}
