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
#include <map>

#include "affect/Affect.hh"
#include "declare.hh"
#include "Flags.hh"
#include "magic.hh"
#include "merc.hh"
#include "skill/skill.hh"

/* item type list */
const std::vector<item_type> item_table = {
	{       ITEM_LIGHT,     "light"         },
	{       ITEM_SCROLL,    "scroll"        },
	{       ITEM_WAND,      "wand"          },
	{       ITEM_STAFF,     "staff"         },
	{       ITEM_WEAPON,    "weapon"        },
	{       ITEM_TREASURE,  "treasure"      },
	{       ITEM_ARMOR,     "armor"         },
	{       ITEM_POTION,    "potion"        },
	{       ITEM_CLOTHING,  "clothing"      },
	{       ITEM_FURNITURE, "furniture"     },
	{       ITEM_TRASH,     "trash"         },
	{       ITEM_CONTAINER, "container"     },
	{       ITEM_DRINK_CON, "drink"         },
	{       ITEM_KEY,       "key"           },
	{       ITEM_FOOD,      "food"          },
	{       ITEM_MONEY,     "money"         },
	{       ITEM_BOAT,      "boat"          },
	{       ITEM_CORPSE_NPC, "npc_corpse"    },
	{       ITEM_CORPSE_PC, "pc_corpse"     },
	{       ITEM_FOUNTAIN,  "fountain"      },
	{       ITEM_PILL,      "pill"          },
	{       ITEM_MAP,       "map"           },
	{       ITEM_PORTAL,    "portal"        },
	{       ITEM_WARP_STONE, "warp_stone"    },
	{       ITEM_GEM,       "gem"           },
	{       ITEM_JEWELRY,   "jewelry"       },
	{       ITEM_JUKEBOX,   "jukebox"       },
//	{       ITEM_PBTUBE,    "paint_tube"    },
//	{       ITEM_PBGUN,     "paint_gun"     },
	{       ITEM_MATERIAL,  "material"      },
	{       ITEM_ANVIL,     "anvil"         },
//	{       ITEM_COACH,     "coach"         },
	{       ITEM_WEDDINGRING, "weddingring"  },
	{       ITEM_TOKEN,     "token"         },
	{       ITEM_WARP_CRYSTAL, "warp_crystal" },
};

/* weapon selection table */

const std::vector<weapon_table_t> weapon_table = {
	{       "sword",        OBJ_VNUM_SCHOOL_SWORD,   WEAPON_SWORD,   skill::sword      },
	{       "mace",         OBJ_VNUM_SCHOOL_MACE,    WEAPON_MACE,    skill::mace       },
	{       "dagger",       OBJ_VNUM_SCHOOL_DAGGER,  WEAPON_DAGGER,  skill::dagger     },
	{       "axe",          OBJ_VNUM_SCHOOL_AXE,     WEAPON_AXE,     skill::axe        },
	{       "staff",        OBJ_VNUM_SCHOOL_STAFF,   WEAPON_SPEAR,   skill::spear      },
	{       "flail",        OBJ_VNUM_SCHOOL_FLAIL,   WEAPON_FLAIL,   skill::flail      },
	{       "whip",         OBJ_VNUM_SCHOOL_WHIP,    WEAPON_WHIP,    skill::whip       },
	{       "polearm",      OBJ_VNUM_SCHOOL_POLEARM, WEAPON_POLEARM, skill::polearm    },
	{       "bow",          OBJ_VNUM_SCHOOL_BOW,     WEAPON_BOW,     skill::archery     },
};

/* attack table  -- not very organized :( */
const std::vector<attack_type> attack_table = {
	{   "none",         "hit",          -1              },  /*  0 */
	{   "slice",        "slice",        DAM_SLASH       },
	{   "stab",         "stab",         DAM_PIERCE      },
	{   "slash",        "slash",        DAM_SLASH       },
	{   "whip",         "whip",         DAM_SLASH       },
	{   "claw",         "claw",         DAM_SLASH       },  /*  5 */
	{   "blast",        "blast",        DAM_BASH        },
	{   "pound",        "pound",        DAM_BASH        },
	{   "crush",        "crush",        DAM_BASH        },
	{   "grep",         "grep",         DAM_SLASH       },
	{   "bite",         "bite",         DAM_PIERCE      },  /* 10 */
	{   "pierce",       "pierce",       DAM_PIERCE      },
	{   "suction",      "suction",      DAM_BASH        },
	{   "beating",      "beating",      DAM_BASH        },
	{   "digestion",    "digestion",    DAM_ACID        },
	{   "charge",       "charge",       DAM_BASH        },  /* 15 */
	{   "slap",         "slap",         DAM_BASH        },
	{   "punch",        "punch",        DAM_BASH        },
	{   "wrath",        "wrath",        DAM_ENERGY      },
	{   "magic",        "magic",        DAM_ENERGY      },
	{   "divine",       "divine power", DAM_HOLY        },  /* 20 */
	{   "cleave",       "cleave",       DAM_SLASH       },
	{   "scratch",      "scratch",      DAM_PIERCE      },
	{   "peck",         "peck",         DAM_PIERCE      },
	{   "peckb",        "peck",         DAM_BASH        },
	{   "chop",         "chop",         DAM_SLASH       },  /* 25 */
	{   "sting",        "sting",        DAM_PIERCE      },
	{   "smash",        "smash",        DAM_BASH        },
	{   "shbite",       "shocking bite", DAM_ELECTRICITY },
	{   "flbite",       "flaming bite", DAM_FIRE        },
	{   "frbite",       "freezing bite", DAM_COLD        }, /* 30 */
	{   "acbite",       "acidic bite",  DAM_ACID        },
	{   "chomp",        "chomp",        DAM_PIERCE      },
	{   "drain",        "life drain",   DAM_NEGATIVE    },
	{   "thrust",       "thrust",       DAM_PIERCE      },
	{   "slime",        "slime",        DAM_ACID        },  /* 35 */
	{   "shock",        "shock",        DAM_ELECTRICITY },
	{   "thwack",       "thwack",       DAM_BASH        },
	{   "flame",        "flame",        DAM_FIRE        },
	{   "chill",        "chill",        DAM_COLD        },
	{   "hstrike",      "hilt strike",  DAM_BASH        },  /* 40 */ /* no skill for hilt strike */
	{   "arrow",        "arrow",        DAM_PIERCE      },
};

/* all races, make sure pc races are in the same order as the pc race table, below */
const std::vector<race_type> race_table = {
	/*
	        {
	                name,           pc_race?,
	                act bits,       affect::by bits,    off bits,
	                imm,            res,            vuln,
	                form,           parts
	        },
	*/
	{       "unique",       FALSE, 0, 0, 0, 0, 0, 0, 0, 0 },

	/*** PC RACES ***/

	{
		"human",        TRUE,
		0,              0,              0,
		0,              0,              0,
		Flags::A|Flags::H|Flags::M|Flags::V,       Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J|Flags::K
	},
	{
		"elf",          TRUE,
		0,              0,              0,
		0,              RES_CHARM,      VULN_IRON,
		Flags::A|Flags::H|Flags::M|Flags::V,       Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J|Flags::K
	},
	{
		"dwarf",        TRUE,
		0,              AFF_NIGHT_VISION,  0,
		0,              RES_POISON | RES_DISEASE, VULN_DROWNING,
		Flags::A|Flags::H|Flags::M|Flags::V,       Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J|Flags::K
	},
	{
		"giant",        TRUE,
		0,              0,              0,
		0,              RES_FIRE | RES_COLD,      VULN_MENTAL | VULN_CHARM,
		Flags::A|Flags::H|Flags::M|Flags::V,       Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J|Flags::K
	},
	{
		"feral",        TRUE,
		0,              AFF_NIGHT_VISION,       OFF_DODGE,
		0,              0,              VULN_DROWNING | VULN_ELECTRICITY,
		Flags::A|Flags::H|Flags::M|Flags::V,       Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J|Flags::K
	},
	{
		"vampire",      TRUE,
		0,              AFF_FLYING,     0,
		0,              RES_NEGATIVE | RES_CHARM, VULN_LIGHT | VULN_HOLY,
		Flags::A|Flags::D|Flags::M|Flags::V,       Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J|Flags::K|Flags::P
	},
	{
		"irda",         TRUE,
		0,              0,              0,
		0,              0,              0,
		Flags::A|Flags::H|Flags::M|Flags::V,       Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J
	},
	{
		"faerie",       TRUE,
		0,              AFF_FLYING | AFF_HASTE,           0,
		0,              0,              0,
		Flags::A|Flags::H|Flags::M|Flags::V,       Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J|Flags::K|Flags::P
	},
	{
		"dragon",       TRUE,
		0,              AFF_FLYING,     0,
		0,              RES_SLASH | RES_BASH,     VULN_PIERCE,
		Flags::A|Flags::H|Flags::Z,         Flags::A|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J|Flags::K|Flags::P|Flags::Q|Flags::U|Flags::V|Flags::X
	},
	{
		"wolf",         TRUE,
		0,              0,              0,
		0,              RES_COLD,       VULN_FIRE,
		Flags::A|Flags::D|Flags::M|Flags::V,       Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J|Flags::K
	},
	{
		"seraph",       TRUE,
		0,              AFF_FLYING,     0,
		0,              RES_HOLY | RES_LIGHT,     VULN_NEGATIVE,
		Flags::A|Flags::H|Flags::M|Flags::V,       Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J|Flags::P
	},
	{
		"superhuman",   TRUE,
		0,              0,              0,
		0,              0,              0,
		Flags::A|Flags::H|Flags::M|Flags::V,       Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J
	},
	{
		"troll",        TRUE,
		0,              AFF_REGENERATION,       0,
		0,              RES_PIERCE | RES_POISON,  VULN_FIRE | VULN_ACID,
		Flags::A|Flags::B|Flags::M|Flags::V,       Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J|Flags::K|Flags::L|Flags::U|Flags::W|Flags::Y
	},

	/*** non-PC RACES ***/

	{
		"bat",          FALSE,
		0,              AFF_FLYING,     OFF_DODGE | OFF_FAST,
		0,              0,              VULN_LIGHT,
		Flags::A|Flags::G|Flags::V,         Flags::A|Flags::C|Flags::D|Flags::E|Flags::F|Flags::H|Flags::J|Flags::K|Flags::P
	},
	{
		"bear",         FALSE,
		0,              0,              OFF_CRUSH | OFF_DISARM | OFF_BERSERK,
		0,              RES_BASH | RES_COLD,      0,
		Flags::A|Flags::G|Flags::V,         Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::H|Flags::J|Flags::K|Flags::U|Flags::V
	},
	{
		"cat",          FALSE,
		0,              0,              OFF_FAST | OFF_DODGE,
		0,              0,              0,
		Flags::A|Flags::G|Flags::V,         Flags::A|Flags::C|Flags::D|Flags::E|Flags::F|Flags::H|Flags::J|Flags::K|Flags::Q|Flags::U|Flags::V
	},
	{
		"centipede",    FALSE,
		0,              0,              0,
		0,              RES_PIERCE | RES_COLD,    VULN_BASH,
		Flags::A|Flags::B|Flags::G|Flags::O,       Flags::A|Flags::C|Flags::K
	},
	{
		"crystal",      FALSE,
		0,              AFF_REGENERATION,       0,
		0,              0,              VULN_FIRE,
		Flags::A|Flags::H|Flags::M|Flags::V,       Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J|Flags::K
	},
	{
		"dog",          FALSE,
		0,              0,              OFF_FAST,
		0,              0,              0,
		Flags::A|Flags::G|Flags::V,         Flags::A|Flags::C|Flags::D|Flags::E|Flags::F|Flags::H|Flags::J|Flags::K|Flags::U|Flags::V
	},
	{
		"doll",         FALSE,
		0,              0,              0,
		IMM_COLD | IMM_POISON | IMM_HOLY | IMM_NEGATIVE | IMM_MENTAL | IMM_DISEASE | IMM_DROWNING,
		RES_BASH | RES_LIGHT,
		VULN_SLASH | VULN_FIRE | VULN_ACID | VULN_ELECTRICITY | VULN_ENERGY,
		Flags::E|Flags::J|Flags::M|Flags::c,      Flags::A|Flags::B|Flags::C|Flags::G|Flags::H|Flags::K
	},
	{
		"fido",         FALSE,
		0,              0,              OFF_DODGE | ASSIST_RACE,
		0,              0,              VULN_MAGIC,
		Flags::A|Flags::B|Flags::G|Flags::V,       Flags::A|Flags::C|Flags::D|Flags::E|Flags::F|Flags::H|Flags::J|Flags::K|Flags::Q|Flags::V
	},
	{
		"fox",          FALSE,
		0,              0,              OFF_FAST | OFF_DODGE,
		0,              0,              0,
		Flags::A|Flags::G|Flags::V,         Flags::A|Flags::C|Flags::D|Flags::E|Flags::F|Flags::H|Flags::J|Flags::K|Flags::Q|Flags::V
	},
	{
		"goblin",       FALSE,
		0,              AFF_NIGHT_VISION,   0,
		0,              RES_DISEASE,    VULN_MAGIC,
		Flags::A|Flags::H|Flags::M|Flags::V,       Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J|Flags::K
	},
	{
		"hobgoblin",    FALSE,
		0,              AFF_NIGHT_VISION,   0,
		0,              RES_DISEASE | RES_POISON, 0,
		Flags::A|Flags::H|Flags::M|Flags::V,       Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J|Flags::K|Flags::Y
	},
	{
		"horse",        FALSE,
		0,              0,              0,
		0,              0,              0,
		Flags::A|Flags::G|Flags::V,         Flags::A|Flags::C|Flags::D|Flags::E|Flags::F|Flags::H|Flags::J|Flags::K
	},
	{
		"kobold",       FALSE,
		0,              AFF_NIGHT_VISION,   0,
		0,              RES_POISON,     VULN_MAGIC,
		Flags::A|Flags::B|Flags::H|Flags::M|Flags::V,     Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J|Flags::K|Flags::Q
	},
	{
		"lizard",       FALSE,
		0,              0,              0,
		0,              RES_POISON,     VULN_COLD,
		Flags::A|Flags::G|Flags::X|Flags::c,      Flags::A|Flags::C|Flags::D|Flags::E|Flags::F|Flags::H|Flags::K|Flags::Q|Flags::V
	},
	{
		"modron",       FALSE,
		0,              AFF_NIGHT_VISION,   ASSIST_RACE | ASSIST_ALIGN,
		IMM_CHARM | IMM_DISEASE | IMM_MENTAL | IMM_HOLY | IMM_NEGATIVE, RES_FIRE | RES_COLD | RES_ACID,     0,
		Flags::H,             Flags::A|Flags::B|Flags::C|Flags::G|Flags::H|Flags::J|Flags::K
	},
	{
		"orc",          FALSE,
		0,              AFF_NIGHT_VISION,   0,
		0,              RES_DISEASE,    VULN_LIGHT,
		Flags::A|Flags::H|Flags::M|Flags::V,       Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J|Flags::K
	},
	{
		"pig",          FALSE,
		0,              0,              0,
		0,              0,              0,
		Flags::A|Flags::G|Flags::V,         Flags::A|Flags::C|Flags::D|Flags::E|Flags::F|Flags::H|Flags::J|Flags::K
	},
	{
		"rabbit",       FALSE,
		0,              0,              OFF_DODGE | OFF_FAST,
		0,              0,              0,
		Flags::A|Flags::G|Flags::V,         Flags::A|Flags::C|Flags::D|Flags::E|Flags::F|Flags::H|Flags::J|Flags::K
	},
	{
		"school monster",       FALSE,
		ACT_NOALIGN|ACT_NOSUMMON,            0,      0,
		IMM_CHARM,   0,      VULN_MAGIC,
		Flags::A|Flags::M|Flags::V,         Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::H|Flags::J|Flags::K|Flags::Q|Flags::U
	},
	{
		"snake",        FALSE,
		0,              0,              0,
		0,              RES_POISON,     VULN_COLD,
		Flags::A|Flags::G|Flags::X|Flags::Y|Flags::c,    Flags::A|Flags::D|Flags::E|Flags::F|Flags::K|Flags::L|Flags::Q|Flags::V|Flags::X
	},
	{
		"song bird",    FALSE,
		0,              AFF_FLYING,     OFF_FAST | OFF_DODGE,
		0,              0,              0,
		Flags::A|Flags::G|Flags::W,         Flags::A|Flags::C|Flags::D|Flags::E|Flags::F|Flags::H|Flags::K|Flags::P
	},
	{
		"teilysa",      FALSE,
		0,              0,              0,
		0,              RES_HOLY,       VULN_BASH | VULN_FIRE | VULN_ACID,
		Flags::A|Flags::H|Flags::M|Flags::V,       Flags::A|Flags::B|Flags::C|Flags::D|Flags::E|Flags::F|Flags::G|Flags::H|Flags::I|Flags::J|Flags::K
	},
	{
		"water fowl",   FALSE,
		0,              AFF_FLYING,     0,
		0,              RES_DROWNING,   0,
		Flags::A|Flags::G|Flags::W,         Flags::A|Flags::C|Flags::D|Flags::E|Flags::F|Flags::H|Flags::K|Flags::P
	},
	{
		"wyvern",       FALSE,
		0,              AFF_FLYING | AFF_DETECT_INVIS | AFF_DETECT_HIDDEN,  OFF_BASH | OFF_FAST | OFF_DODGE,
		IMM_POISON,     0,      VULN_LIGHT,
		Flags::A|Flags::B|Flags::G|Flags::Z,       Flags::A|Flags::C|Flags::D|Flags::E|Flags::F|Flags::H|Flags::J|Flags::K|Flags::Q|Flags::V|Flags::X
	},
};

const std::vector<pc_race_type> pc_race_table = {
	/*
	        {
	                "race name",    short name,base age,    points, { class multipliers },
	                { base stats },                 { max stats },                  size,           remort level,
	                { bonus skills }
	        },
	*/
	{
		"null race",    "",     17,     0,      { 100, 100, 100, 100, 100, 100, 100, 100 },
		{ 13, 13, 13, 13, 13, 13 },     { 18, 18, 18, 18, 18, 18 },     0,              0,
		{ "" }
	},
	{
		"human",        "Hum",  17,     0,      { 100, 100, 100, 100, 100, 100, 100, 100 },
		{ 13, 13, 13, 13, 13, 13 },     { 18, 18, 18, 18, 18, 18 },     SIZE_MEDIUM,    0,
		{ "" }
	},
	{
		"elf",          "Elf",  17,     5,      { 105, 120, 105, 110, 130, 130, 110, 120 },
		{ 11, 15, 13, 15, 11, 14 },     { 16, 21, 18, 21, 16, 18 },     SIZE_SMALL,     0,
		{ "sneak", "hide", "enchantment" }
	},
	{
		"dwarf",        "Dwa",  17,     5,      { 140, 105, 130, 105, 150, 110, 120, 120 },
		{ 15, 12, 14, 10, 17, 11 },     { 21, 16, 20, 15, 21, 16 },     SIZE_MEDIUM,    0,
		{ "berserk", "forge" }
	},
	{
		"giant",        "Gia",  17,     10,     { 150, 130, 130, 105, 150, 150, 120, 120 },
		{ 17, 10, 12, 12, 17, 12 },     { 23, 14, 17, 16, 22, 18 },     SIZE_HUGE,      0,
		{ "bash", "fast healing" }
	},
	{
		"feral",        "Fer",  17,     20,     { 175, 175, 110, 125, 150, 200, 125, 150 },
		{ 14, 15, 13, 18, 13, 12 },     { 18, 19, 18, 22, 18, 17 },     SIZE_MEDIUM,    0,
		{ "sneak", "hide", "dodge" }
	},
	{
		"vampire",      "Vmp",  17,     20,     { 125, 200, 175, 150, 110, 175, 125, 175 },
		{ 16, 16, 16, 15, 12, 14 },     { 19, 20, 22, 20, 17, 19 },     SIZE_MEDIUM,    0,
		{ "illusion" }
	},
	{
		"irda",         "Ird",  17,     10,     { 105, 110, 130, 150, 105, 120, 130, 150 },
		{ 12, 16, 16, 13, 13, 11 },     { 18, 21, 21, 18, 18, 15 },     SIZE_MEDIUM,    0,
		{ "detection", "meditation", "lore" }
	},
	{
		"faerie",       "Fae",  17,     10,     { 105, 130, 110, 200, 130, 190, 150, 180 },
		{ 12, 16, 12, 17, 13, 14 },     { 15, 22, 18, 22, 18, 19 },     SIZE_TINY,      0,
		{ "weather", "dodge" }
	},
	{
		"dragon",       "Dra",  17,     15,     { 125, 150, 150, 125, 150, 150, 150, 150 },
		{ 15, 15, 15, 12, 15, 13 },     { 20, 19, 20, 16, 20, 18 },     SIZE_LARGE,     0,
		{ "draconian", "bash" }
	},
	{
		"wolf",         "Wol",  17,     20,     { 200, 150, 175, 125, 175, 125, 150, 110 },
		{ 15, 13, 14, 16, 16, 14 },     { 20, 18, 18, 20, 21, 18 },     SIZE_MEDIUM,    0,
		{ "hand to hand", "enhancement", "hunt" }
	},
	{
		"seraph",       "Ser",  17,     20,     { 150, 125, 175, 125, 200, 110, 175, 175 },
		{ 16, 14, 14, 13, 15, 16 },     { 21, 18, 20, 17, 20, 21 },     SIZE_MEDIUM,    0,
		{ "curative", "benedictions" }
	},

	/* Remort Races */
	{
		"superhuman",   "Sup",  17,     0,      { 200, 200, 200, 200, 200, 200, 200, 200 },
		{ 16, 16, 16, 16, 16, 16 },     { 20, 20, 20, 20, 20, 20 },     SIZE_MEDIUM,    10,
		{ "" }
	},
	{
		"troll",        "Tro",  17,     0,      { 200, 140, 195, 105, 200, 150, 200, 130 },
		{ 16, 13, 13, 15, 17, 12 },     { 20, 17, 17, 19, 21, 16 },     SIZE_HUGE,      999,
		{ "fast healing", "bash" }
	}
};


/*
 * Deity Table
 */
const std::vector<deity_type> deity_table = {
	{       "Navarre",      "[ Lawful Good		] ",    750     },
	{       "Xenith",       "[ Chaotic Good		] ",   750     },
	{       "Kenneth",      "[ Neutral Evil		] ",   -750    },
	{       "Teotwawki",    "[ Neutral Good		] ",   750     },
	{       "Urza",         "[ True Neutral		] ",   0       },
	{       "Vegita",       "[ Chaotic Evil		] ",   -750    },
	{       "Outsider",     "[ Chaotic Good		] ",   750     },
	{       "Viro",         "[ Lawful Good		] ",    750     },
	{       "Navarre",      "[ True Good		] ",      1000    },
	{       "Kismet",       "[ Chaotic Neutral	] ", 0       },
	{       "Whisper",      "[ Lawful Neutral	] ",  0       },
};

/*
 * Class table.
 */
const std::vector<class_type> class_table = {
	/*
	        {
	                name,           whoname,prime_attr,     vnum of newbie weapon,
	                adept%, thac0_00,thac0_32,basic group,  default group,
	                hp_min, hp_max, mana_min,mana_max,stp_min,stp_max
	        }
	*/
	{
		"mage",         "Mag",  STAT_INT,       OBJ_VNUM_SCHOOL_DAGGER,
		75,     20,     6,      "mage basics",  "mage default",
		6,      8,      7,      11,     2,      4
	},
	{
		"cleric",       "Cle",  STAT_WIS,       OBJ_VNUM_SCHOOL_MACE,
		75,     20,     2,      "cleric basics", "cleric default",
		7,      10,     6,      10,     3,      5
	},
	{
		"thief",        "Thi",  STAT_DEX,       OBJ_VNUM_SCHOOL_DAGGER,
		75,     20,     -4,     "thief basics", "thief default",
		8,      13,     2,      6,      6,      9
	},
	{
		"warrior",      "War",  STAT_STR,       OBJ_VNUM_SCHOOL_SWORD,
		75,     20,     -10,    "warrior basics", "warrior default",
		11,     15,     2,      4,      7,      11
	},
	{
		"necromancer",  "Nec",  STAT_INT,       OBJ_VNUM_SCHOOL_POLEARM,
		75,     20,     3,      "necromancer basics", "necromancer default",
		5,      8,      6,      11,     2,      4
	},
	{
		"paladin",      "Pld",  STAT_CHR,       OBJ_VNUM_SCHOOL_SWORD,
		75,     20,     -8,     "paladin basics", "paladin default",
		10,     14,     4,      8,      7,      9
	},
	{
		"bard",         "Bar",  STAT_DEX,       OBJ_VNUM_SCHOOL_FLAIL,
		75,     20,     -4,     "bard basics", "bard default",
		8,      13,     4,      7,      5,      9
	},
	{
		"ranger",       "Ran",  STAT_CON,       OBJ_VNUM_SCHOOL_AXE,
		75,     20,     -5,     "ranger basics", "ranger default",
		13,     17,     3,      6,      7,      10
	}
};

/*
 * Attribute bonus tables.
 */

const struct str_app_type str_app[26] = {
	/*      tohit,  todam,  carry,  wield,  stp     */
	{ -5,    -4,       0,     0,     -4}, /* 0  */
	{ -5,    -4,       3,     1,     -3}, /* 1  */
	{ -3,    -2,       3,     2,     -2},
	{ -3,    -1,      10,     3,     -2}, /* 3  */
	{ -2,    -1,      25,     4,     -1},
	{ -2,    -1,      55,     5,     -1}, /* 5  */
	{ -1,     0,      80,     6,     -1},
	{ -1,     0,      90,     7,     0},
	{ 0,     0,     100,     8,     0},
	{ 0,     0,     100,     9,     0},
	{ 0,     0,     115,    10,     0}, /* 10  */
	{ 0,     0,     115,    11,     0},
	{ 0,     0,     130,    12,     0},
	{ 0,     0,     130,    13,     0}, /* 13  */
	{ 0,     1,     140,    14,     0},
	{ 1,     1,     150,    15,     1}, /* 15  */
	{ 1,     2,     165,    16,     1},
	{ 2,     3,     180,    22,     1},
	{ 2,     3,     200,    25,     2}, /* 18  */
	{ 3,     4,     225,    30,     2},
	{ 3,     5,     250,    35,     2}, /* 20  */
	{ 4,     6,     300,    40,     2},
	{ 4,     6,     350,    45,     3},
	{ 5,     7,     400,    50,     3},
	{ 5,     8,     450,    55,     4},
	{ 6,     9,     500,    60,     4}  /* 25   */
};

const struct int_app_type int_app[26] = {
	/*      learn, bonus mana   */
	{  3,   -4 },     /*  0 */
	{  5,   -3 },     /*  1 */
	{  7,   -2 },
	{  8,   -2 },     /*  3 */
	{  9,   -1 },
	{ 10,   -1 },     /*  5 */
	{ 11,   -1 },
	{ 12,   0 },
	{ 13,   0 },
	{ 15,   0 },
	{ 17,   0 },     /* 10 */
	{ 19,   0 },
	{ 22,   0 },
	{ 25,   0 },
	{ 28,   0 },
	{ 31,   1 },     /* 15 */
	{ 34,   1 },
	{ 37,   1 },
	{ 40,   2 },     /* 18 */
	{ 44,   2 },
	{ 49,   2 },     /* 20 */
	{ 55,   2 },
	{ 60,   3 },
	{ 70,   3 },
	{ 80,   4 },
	{ 85,   4 }      /* 25 */
};

const struct wis_app_type wis_app[26] = {
	/*      practice        */
	{ 0 },      /*  0 */
	{ 0 },      /*  1 */
	{ 0 },
	{ 0 },      /*  3 */
	{ 0 },
	{ 1 },      /*  5 */
	{ 1 },
	{ 1 },
	{ 1 },
	{ 1 },
	{ 1 },      /* 10 */
	{ 1 },
	{ 1 },
	{ 1 },
	{ 1 },
	{ 2 },      /* 15 */
	{ 2 },
	{ 2 },
	{ 3 },      /* 18 */
	{ 3 },
	{ 3 },      /* 20 */
	{ 3 },
	{ 4 },
	{ 4 },
	{ 4 },
	{ 5 }       /* 25 */
};

const struct dex_app_type dex_app[26] = {
	/*      defensive       */
	{   60 },   /* 0 */
	{   50 },   /* 1 */
	{   50 },
	{   40 },
	{   30 },
	{   20 },   /* 5 */
	{   10 },
	{    0 },
	{    0 },
	{    0 },
	{    0 },   /* 10 */
	{    0 },
	{    0 },
	{    0 },
	{    0 },
	{ - 10 },   /* 15 */
	{ - 15 },
	{ - 20 },
	{ - 30 },
	{ - 40 },
	{ - 50 },   /* 20 */
	{ - 60 },
	{ - 75 },
	{ - 90 },
	{ -105 },
	{ -120 }    /* 25 */
};

const struct con_app_type con_app [26] = {
	/*      hitp,   shock   */
	{ -4,   20 },   /*  0 */
	{ -3,   25 },   /*  1 */
	{ -2,   30 },
	{ -2,   35 },   /*  3 */
	{ -1,   40 },
	{ -1,   45 },   /*  5 */
	{ -1,   50 },
	{  0,   55 },
	{  0,   60 },
	{  0,   65 },
	{  0,   70 },   /* 10 */
	{  0,   75 },
	{  0,   80 },
	{  0,   85 },
	{  0,   88 },
	{  1,   90 },   /* 15 */
	{  2,   95 },
	{  2,   97 },
	{  3,   99 },   /* 18 */
	{  3,   99 },
	{  4,   99 },   /* 20 */
	{  4,   99 },
	{  5,   99 },
	{  6,   99 },
	{  7,   99 },
	{  8,   99 }    /* 25 */
};

const struct chr_app_type chr_app[26] = {
	/*      chance  */
	{ 0 },      /*  0 */
	{ 0 },      /*  1 */
	{ 0 },
	{ 0 },      /*  3 */
	{ 0 },
	{ 0 },      /*  5 */
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },      /* 10 */
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 1 },      /* 15 */
	{ 1 },
	{ 1 },
	{ 2 },      /* 18 */
	{ 2 },
	{ 3 },
	{ 5 },      /* 21 */
	{ 7 },
	{ 8 },
	{ 10},
	{ 15}       /* 25 */
};

/*
 * Liquid properties.
 */
const std::vector<liq_type> liq_table = {
	/*        name                      color       proof, full, thirst, food, ssize */
	{ "water",                  "clear",        {   0, 1, 10, 0, 16 }   },
	{ "beer",                   "amber",        {  12, 1,  8, 1, 12 }   },
	{ "red wine",               "burgundy",     {  30, 1,  8, 1,  5 }   },
	{ "ale",                    "brown",        {  15, 1,  8, 1, 12 }   },
	{ "dark ale",               "dark",         {  16, 1,  8, 1, 12 }   },
	{ "whisky",                 "golden",       { 120, 1,  5, 0,  2 }   },
	{ "lemonade",               "yellow",       {   0, 1,  9, 2, 12 }   },
	{ "firebreather",           "boiling",      { 190, 0,  4, 0,  2 }   },
	{ "local specialty",        "clear",        { 151, 1,  3, 0,  2 }   },
	{ "slime mold juice",       "green",        {   0, 2, -8, 1,  2 }   },
	{ "milk",                   "white",        {   0, 2,  9, 3, 12 }   },
	{ "tea",                    "tan",          {   0, 1,  8, 0,  6 }   },
	{ "coffee",                 "black",        {   0, 1,  8, 0,  6 }   },
	{ "blood",                  "red",          {   0, 2, -1, 2,  6 }   },
	{ "salt water",             "clear",        {   0, 1, -2, 0,  1 }   },
	{ "coke",                   "brown",        {   0, 2,  9, 2, 12 }   },
	{ "root beer",              "brown",        {   0, 2,  9, 2, 12 }   },
	{ "elvish wine",            "green",        {  35, 2,  8, 1,  5 }   },
	{ "white wine",             "golden",       {  28, 1,  8, 1,  5 }   },
	{ "champagne",              "golden",       {  32, 1,  8, 1,  5 }   },
	{ "mead",                   "honey-colored", {  34, 2,  8, 2, 12 }   },
	{ "rose wine",              "pink",         {  26, 1,  8, 1,  5 }   },
	{ "benedictine wine",       "burgundy",     {  40, 1,  8, 1,  5 }   },
	{ "vodka",                  "clear",        { 130, 1,  5, 0,  2 }   },
	{ "cranberry juice",        "red",          {   0, 1,  9, 2, 12 }   },
	{ "orange juice",           "orange",       {   0, 2,  9, 3, 12 }   },
	{ "absinthe",               "green",        { 200, 1,  4, 0,  2 }   },
	{ "brandy",                 "golden",       {  80, 1,  5, 0,  4 }   },
	{ "aquavit",                "clear",        { 140, 1,  5, 0,  2 }   },
	{ "schnapps",               "clear",        {  90, 1,  5, 0,  2 }   },
	{ "icewine",                "purple",       {  50, 2,  6, 1,  5 }   },
	{ "amontillado",            "burgundy",     {  35, 2,  8, 1,  5 }   },
	{ "sherry",                 "red",          {  38, 2,  7, 1,  5 }   },
	{ "framboise",              "red",          {  50, 1,  7, 1,  5 }   },
	{ "rum",                    "amber",        { 151, 1,  4, 0,  2 }   },
	{ "cordial",                "clear",        { 100, 1,  5, 0,  2 }   },
	{ "tomato beer",            "red",          {  12, 2,  9, 3, 12 }   },
	{ "rice wine",              "clear",        { 190, 0,  4, 0,  2 }   },
	{ "liquid brimstone",       "red",          { 25,  0, 15, 0,  2 }   },
	{ "protein shake",          "white",        {   0, 2,  9, 3, 12 }   },
	{ "pumpkin spice latte",    "creamy",       {   0, 2,  9, 3, 12 }   },
};

/*
 * The skill and spell table.
 */

const std::map<skill::Type, skill::skill_type> skill_table = {

/*
 * Magic spells.
 */
/*
    Legend:

    {	skill::acid_blast		{
        "acid blast",
        { 28, 33, 35, 32, 28, 33, 35, 32 }, { 1, 1, 2, 2, 1, 1, 2, 2 },
        spell_acid_blast,       TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        20,     12,	"acid blast",           0,
        { 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
    }},

		1,
        2,
        3,                                  4,
        5,                      6,                      7,
        8,       9, 10,                    11,
        12,                             13

    1. skill number
    2. name, referred to in skill_lookup, various other places.
    3. skill_level[], level at which a class gets a skill/spell.
    4. rating[], how much it costs, in trains for skills and pracs for spells.
            mage, cleric, thief, warrior, necro, paladin, bard, ranger
    5. spell_fun, use spell_null for non-spells, and the function's name otherwise.
    6. target, used in do_cast.  Determines legal targets.  For skills, used
       to determine stamina cost, TAR_CHAR_OFFENSIVE is variable cost skills
    7. minimum_position, sets required position for use.
    8. min_mana, minimum mana cost to cast.
    9. beats, amount of lag in quarter seconds.
    10. noun_damage, damage message in combat.
    11. remort_class, sets what class gets it as a remort spell.  Use -1 to make it inaccessible.
    12. evocost_sec, cost for secondary or higher class to evolve to 2.  Use 0 for tertiary classes.
    13. evocost_pri, cost for primary class to evolve to 3.  Use 0 for secondary classes.
*/

	{	skill::unknown,          {
		"unknown",
		{ 999, 999, 999, 999, 999, 999, 999, 999 }, { 999, 999, 999, 999, 999, 999, 999, 999 },		
		0,                      TAR_IGNORE,             POS_STANDING,
		0,      0,	"",                     -1,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},

	{	skill::acid_blast,        {
		"acid blast",
		{ 28, 33, 35, 32, 28, 33, 35, 32 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_acid_blast,       TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		20,     12,	"acid blast",           0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::acid_breath,       {
		"acid breath",
		{ 31, 32, 33, 34, 31, 32, 33, 34 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_acid_breath,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		50,     12,	"blast of acid",        0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::acid_rain,         {
		"acid rain",
		{ 51, 49, 54, 53, 52, 49, 48, 50 }, {  1,  1,  2,  2,  1,  1,  1,  1 },		
		spell_acid_rain,        TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		30,     12,	"acid rain",            0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::age,               {
		"age",
		{ 53, 54, 65, 63, 51, 54, 59, 68 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_age,              TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		35,     12,	"spell",                0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::animate_skeleton,  {
		"animate skeleton",
		{ 32, 43, 70, 70, 25, 70, 70, 70 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_animate_skeleton, TAR_IGNORE,             POS_STANDING,
		50,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::animate_wraith,    {
		"animate wraith",
		{ 55, 66, 75, 75, 52, 75, 75, 75 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_animate_wraith,   TAR_IGNORE,             POS_STANDING,
		50,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::animate_gargoyle,  {
		"animate gargoyle",
		{ 70, 78, 80, 80, 63, 80, 80, 80 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_animate_gargoyle, TAR_IGNORE,             POS_STANDING,
		50,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::animate_zombie,    {
		"animate zombie",
		{ 46, 54, 73, 73, 37, 73, 73, 73 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_animate_zombie,   TAR_IGNORE,             POS_STANDING,
		50,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::armor,             {
		"armor",
		{  5,  2, 10,  5,  5,  2, 10,  5 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_armor,            TAR_CHAR_DEFENSIVE,     POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::bless,             {
		"bless",
		{ 10,  2, 60,  8, 44,  4, 10,  8 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_bless,            TAR_OBJ_CHAR_DEF,       POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::blindness,         {
		"blindness",
		{ 12,  8, 17, 15, 12,  8, 17, 15 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_blindness,        TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::blizzard,          {
		"blizzard",
		{ 51, 49, 54, 53, 52, 49, 48, 50 }, {  1,  1,  2,  2,  1,  1,  1,  1 },		
		spell_blizzard,         TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		30,     12,	"snow flurry",          0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::blood_moon,        {
		"blood moon",
		{ 33, 38, 68, 68, 12, 75, 68, 68 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_blood_moon,       TAR_CHAR_DEFENSIVE,     POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::blood_blade,       {
		"blood blade",
		{ 30, 58, 60, 78, 35, 70, 63, 70 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_blood_blade,      TAR_OBJ_INV,            POS_STANDING,
		100,    12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::burning_hands,     {
		"burning hands",
		{  7, 11, 10,  9,  7, 11, 10,  9 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_burning_hands,    TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		6,      12,	"burning hands",        0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::call_lightning,    {
		"call lightning",
		{ 26, 16, 31, 22, 26, 18, 31, 22 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_call_lightning,   TAR_IGNORE,             POS_FIGHTING,
		10,     12,	"lightning bolt",       0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::calm,              {
		"calm",
		{ 20, 16, 60, 20, 20, 16, 20, 20 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_calm,             TAR_IGNORE,             POS_FIGHTING,
		30,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::cancellation,      {
		"cancellation",
		{ 18, 26, 34, 34, 18, 26, 34, 34 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_cancellation,     TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		20,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::cause_light,       {
		"cause light",
		{  5,  6,  5,  5,  1,  6,  5,  5 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_cause_light,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		5,      12,	"spell",                0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::cause_serious,     {
		"cause serious",
		{ 10, 12, 13, 13,  7,  7, 13, 13 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_cause_serious,    TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		7,      12,	"spell",                0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::cause_critical,    {
		"cause critical",
		{ 21, 18, 23, 23, 13, 18, 23, 23 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_cause_critical,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		10,     12,	"spell",                0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::chain_lightning,   {
		"chain lightning",
		{ 33, 35, 39, 36, 33, 35, 39, 36 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_chain_lightning,  TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		25,     12,	"lightning",            0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::change_sex,        {
		"change sex",
		{ 60, 60, 60, 60, 60, 60, 60, 60 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_change_sex,       TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		15,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::channel,           {
		"channel",
		{ 63, 65, 77, 78, 65, 66, 76, 78 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_channel,          TAR_IGNORE,             POS_STANDING,
		10,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::charm_person,      {
		"charm person",
		{ 20, 26, 25, 60, 20, 23, 25, 60 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_charm_person,     TAR_CHAR_OFFENSIVE,     POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::chill_touch,       {
		"chill touch",
		{  4,  7,  6,  6,  4,  7,  6,  6 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_chill_touch,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		5,      12,	"chilling touch",       0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::colour_spray,      {
		"colour spray",
		{ 16, 24, 22, 20, 16, 24, 22, 20 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_colour_spray,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		11,     12,	"colour spray",         0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::continual_light,   {
		"continual light",
		{  5,  4,  6,  9,  6,  4,  6,  9 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_continual_light,  TAR_IGNORE,             POS_STANDING,
		7,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::control_weather,   {
		"control weather",
		{ 15, 19, 28, 22, 15, 19, 28, 22 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_control_weather,  TAR_IGNORE,             POS_STANDING,
		25,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::create_food,       {
		"create food",
		{ 10,  5, 11, 12, 10,  5, 11, 12 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_create_food,      TAR_IGNORE,             POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::create_parchment,  {
		"create parchment",
		{ 16, 11, 60, 60, 16, 11, 60, 60 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_create_parchment, TAR_IGNORE,             POS_STANDING,
		30,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::create_rose,       {
		"create rose",
		{ 16, 11, 10, 24, 16, 11, 10, 24 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_create_rose,      TAR_IGNORE,             POS_STANDING,
		30,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::create_sign,       {
		"create sign",
		{ 25, 20, 23, 22, 26, 21, 22, 21 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_create_sign,      TAR_IGNORE,             POS_STANDING,
		45,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::create_spring,     {
		"create spring",
		{ 14, 16, 23, 20, 14, 16, 23, 20 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_create_spring,    TAR_IGNORE,             POS_STANDING,
		20,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::create_vial,       {
		"create vial",
		{ 16, 11, 60, 60, 16, 11, 60, 60 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_create_vial,      TAR_IGNORE,             POS_STANDING,
		30,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::create_water,      {
		"create water",
		{  8,  3, 12, 11,  8,  3, 12, 11 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_create_water,     TAR_OBJ_INV,            POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::cure_blindness,    {
		"cure blindness",
		{ 10,  6, 60,  8, 27,  6, 60,  8 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_cure_blindness,   TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::cure_critical,                     {
		"cure critical",
		{ 18, 13, 28, 19, 26, 13, 21, 19 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_cure_critical,    TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		20,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::cure_disease,      {
		"cure disease",
		{ 19, 13, 60, 14, 34, 13, 60, 14 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_cure_disease,     TAR_CHAR_DEFENSIVE,     POS_STANDING,
		20,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::cure_light,        {
		"cure light",
		{  2,  1, 10,  3,  9,  1,  6,  3 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_cure_light,       TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		10,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::cure_poison,       {
		"cure poison",
		{ 21, 14, 51, 16, 38, 14, 48, 16 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_cure_poison,      TAR_CHAR_DEFENSIVE,     POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::cure_serious,      {
		"cure serious",
		{  9,  7, 15, 10, 15,  7, 12, 10 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_cure_serious,     TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		15,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::curse,             {
		"curse",
		{ 18, 18, 26, 22, 17, 18, 26, 22 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_curse,            TAR_OBJ_CHAR_OFF,       POS_FIGHTING,
		20,     12,	"curse",                0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::darkness,          {
		"darkness",
		{ 31, 39, 58, 68, 26, 53, 61, 57 }, {  2,  2,  4,  4,  2,  3,  4,  4 },		
		spell_darkness,         TAR_IGNORE,             POS_STANDING,
		60,     18,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::dazzling_light,    {
		"dazzling light",
		{ 10, 18, 30, 30, 12, 18, 25, 30 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_dazzling_light,   TAR_OBJ_INV,            POS_STANDING,
		50,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::demonfire,         {
		"demonfire",
		{ 38, 34, 60, 45, 30, 34, 60, 45 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_demonfire,        TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		18,     12,	"torments",             0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::detect_evil,       {
		"detect evil",
		{ 11,  4, 12, 60, 11,  4, 12, 60 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_detect_evil,      TAR_CHAR_SELF,          POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::detect_good,       {
		"detect good",
		{ 11,  4, 12, 60, 11,  4, 12, 60 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_detect_good,      TAR_CHAR_SELF,          POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::detect_hidden,     {
		"detect hidden",
		{ 15, 11, 12, 60, 15, 11, 12, 60 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_detect_hidden,    TAR_CHAR_SELF,          POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::detect_invis,      {
		"detect invis",
		{  3,  8,  6, 60,  3,  8,  6, 60 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_detect_invis,     TAR_CHAR_SELF,          POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::detect_magic,      {
		"detect magic",
		{  2,  6,  5, 60,  2,  6,  5, 60 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_detect_magic,     TAR_CHAR_SELF,          POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::detect_poison,     {
		"detect poison",
		{ 15,  7,  9, 60, 15,  7,  9, 60 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_detect_poison,    TAR_OBJ_INV,            POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::dispel_evil,       {
		"dispel evil",
		{ 18, 15, 60, 23, 20, 15, 45, 24 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_dispel_evil,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		10,     12,	"dispel evil",          0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::dispel_good,       {
		"dispel good",
		{ 18, 15, 35, 24, 18, 15, 35, 24 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_dispel_good,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		10,     12,	"dispel good",          0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::dispel_magic,      {
		"dispel magic",
		{ 16, 24, 30, 30, 16, 24, 30, 30 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_dispel_magic,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		15,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::divine_healing,    {
		"divine healing",
		{ 55, 51, 62, 61, 75, 46, 62, 62 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_divine_healing,   TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		150,    12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::divine_regeneration, {
		"divine regeneration",
		{ 87, 88, 88, 89, 87, 86, 88, 89 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_divine_regeneration, TAR_CHAR_DEFENSIVE,   POS_STANDING,
		50,     12,	"",                     0,
		{ 470, 220, 0, 0, 0, 420, 0, 0 },       { 0, 410, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::earthquake,        {
		"earthquake",
		{ 19, 10, 21, 14, 18, 10, 20, 14 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_earthquake,       TAR_IGNORE,             POS_FIGHTING,
		8,      12,	"earthquake",           0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::encampment,        {
		"encampment",
		{ 26, 18, 32, 29,  6, 18, 32, 29 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_encampment,       TAR_IGNORE,             POS_STANDING,
		28,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::enchant_armor,     {
		"enchant armor",
		{ 16, 40, 60, 60, 16, 40, 60, 60 }, {  2,  2,  4,  4,  2,  2,  4,  4 },		
		spell_enchant_armor,    TAR_OBJ_INV,            POS_STANDING,
		100,    24,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::enchant_weapon,    {
		"enchant weapon",
		{ 17, 40, 60, 60, 17, 40, 60, 60 }, {  2,  2,  4,  4,  2,  2,  4,  4 },		
		spell_enchant_weapon,   TAR_OBJ_INV,            POS_STANDING,
		100,    24,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::energy_drain,      {
		"energy drain",
		{ 20, 35, 29, 29, 9, 60, 24, 29 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_energy_drain,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		40,     12,	"energy drain",         0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::faerie_fire,       {
		"faerie fire",
		{  6,  3,  5,  8,  6,  3,  5,  8 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_faerie_fire,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		5,      12,	"faerie fire",          0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::faerie_fog,        {
		"faerie fog",
		{ 14, 21, 16, 24, 14, 21, 16, 24 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_faerie_fog,       TAR_IGNORE,             POS_STANDING,
		12,     12,	"faerie fog",           0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::farsight,          {
		"farsight",
		{ 14, 16, 16, 60, 14, 16, 16, 60 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_farsight,         TAR_IGNORE,             POS_STANDING,
		36,     20,	"farsight",             0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::fear,              {
		"fear",
		{ 17, 20, 24, 21, 12, 20, 23, 23 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_fear,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		20,     12,	"fear",                 0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::fire_breath,       {
		"fire breath",
		{ 40, 45, 50, 51, 40, 45, 50, 51 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_fire_breath,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		60,     12,	"blast of flame",       0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::fireball,          {
		"fireball",
		{ 22, 32, 30, 26, 22, 32, 30, 26 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_fireball,         TAR_IGNORE,             POS_FIGHTING,
		10,     12,	"fireball",             0,
		{ 220, 0, 0, 0, 380, 0, 0, 0 }, { 460, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::fireproof,         {
		"fireproof",
		{ 13, 12, 19, 18, 13, 12, 19, 18 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_fireproof,        TAR_OBJ_INV,            POS_STANDING,
		10,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::firestorm,         {
		"firestorm",
		{ 78, 78, 80, 80, 78, 79, 80, 80 }, {  2,  2,  2,  2,  2,  2,  2,  2 },		
		spell_firestorm,        TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		35,     12,	"firestorm",            0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::flame_blade,       {
		"flame blade",
		{ 30, 58, 60, 78, 35, 54, 63, 70 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_flame_blade,      TAR_OBJ_INV,            POS_STANDING,
		100,    12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::flameshield,               {
		"flameshield",
		{ 50, 65, 65, 70, 60, 55, 65, 55 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_flameshield,      TAR_CHAR_SELF,          POS_STANDING,
		45,     12,	"flameshield",          0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::flamestrike,       {
		"flamestrike",
		{ 29, 20, 39, 27, 29, 20, 39, 27 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_flamestrike,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		12,     12,	"flamestrike",          0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::fly,               {
		"fly",
		{ 10, 18, 20, 22, 10, 18, 20, 22 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_fly,              TAR_CHAR_DEFENSIVE,     POS_STANDING,
		10,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::floating_disc,     {
		"floating disc",
		{  4, 10,  7, 16,  4, 10,  7, 16 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_floating_disc,    TAR_IGNORE,             POS_STANDING,
		40,     24,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::frenzy,            {
		"frenzy",
		{ 26, 24, 60, 26, 60, 24, 60, 26 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_frenzy,           TAR_CHAR_DEFENSIVE,     POS_STANDING,
		30,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::frost_blade,       {
		"frost blade",
		{ 30, 58, 60, 78, 35, 54, 63, 70 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_frost_blade,      TAR_OBJ_INV,            POS_STANDING,
		100,    12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::frost_breath,      {
		"frost breath",
		{ 34, 36, 38, 40, 34, 36, 38, 40 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_frost_breath,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		50,     12,	"blast of frost",       0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::gas_breath,        {
		"gas breath",
		{ 29, 33, 37, 40, 29, 33, 37, 40 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_gas_breath,       TAR_IGNORE,             POS_FIGHTING,
		45,     12,	"blast of gas",         0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::gate,              {
		"gate",
		{ 22, 17, 32, 28, 22, 17, 32, 28 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_gate,             TAR_IGNORE,             POS_STANDING,
		80,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::general_purpose,   {
		"general purpose",
		{ -1, -1, -1, -1, -1, -1, -1, -1 }, { -1, -1, -1, -1, -1, -1, -1, -1 },		
		spell_general_purpose,  TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		5,      12,	"general purpose ammo", -1,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::giant_strength,    {
		"giant strength",
		{ 11, 15, 22, 20, 11, 15, 22, 20 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_giant_strength,   TAR_CHAR_DEFENSIVE,     POS_STANDING,
		20,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::harm,              {
		"harm",
		{ 30, 23, 30, 28, 30, 23, 30, 28 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_harm,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		20,     12,	"harm spell",           0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::haste,             {
		"haste",
		{ 21, 30, 26, 29, 21, 30, 26, 29 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_haste,            TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		30,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::heal,              {
		"heal",
		{ 28, 21, 33, 30, 35, 21, 33, 30 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_heal,             TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		50,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::heat_metal,        {
		"heat metal",
		{ 25, 16, 60, 23, 20, 16, 41, 23 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_heat_metal,       TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		25,     18,	"spell",                0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::high_explosive,    {
		"high explosive",
		{ -1, -1, -1, -1, -1, -1, -1, -1 }, { -1, -1, -1, -1, -1, -1, -1, -1 },		
		spell_high_explosive,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		5,      12,	"high explosive ammo",  -1,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::holy_word,         {
		"holy word",
		{ 60, 36, 60, 42, 60, 36, 60, 42 }, {  2,  2,  4,  4,  2,  2,  4,  4 },		
		spell_holy_word,        TAR_IGNORE,             POS_FIGHTING,
		200,    24,	"divine wrath",         0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::identify,          {
		"identify",
		{ 15, 16, 18, 58, 15, 16, 18, 58 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_identify,         TAR_OBJ_HERE,            POS_STANDING,
		12,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::night_vision,       {
		"infravision",
		{  9, 13, 10, 16,  9, 13, 10, 16 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_infravision,      TAR_CHAR_DEFENSIVE,     POS_STANDING,
		5,      18,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::invis,             {
		"invisibility",
		{  5, 12,  9, 60,  5, 12,  9, 60 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_invis,            TAR_OBJ_CHAR_DEF,       POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::know_alignment,    {
		"know alignment",
		{ 12,  9, 20, 59, 12,  9, 20, 59 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_know_alignment,   TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		9,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::light_of_truth,    {
		"light of truth",
		{ 30, 20, 45, 60, 31, 19, 42, 60 }, {  2,  1,  2,  3,  2,  1,  2,  3 },		
		spell_light_of_truth,   TAR_OBJ_INV,            POS_STANDING,
		100,    12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::lightning_bolt,    {
		"lightning bolt",
		{ 13, 20, 18, 16, 13, 20, 18, 16 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_lightning_bolt,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		8,      12,	"lightning bolt",       0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::lightning_breath,  {
		"lightning breath",
		{ 37, 40, 43, 46, 37, 40, 43, 46 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_lightning_breath, TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		65,     12,	"blast of lightning",   0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::locate_life,       {
		"locate life",
		{  9, 15, 11, 45,  9, 15, 11, 44 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_locate_life,      TAR_IGNORE,             POS_STANDING,
		20,     18,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::locate_object,     {
		"locate object",
		{  9, 15, 11, 40,  9, 15, 11, 40 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_locate_object,    TAR_IGNORE,             POS_STANDING,
		20,     18,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::magic_missile,     {
		"magic missile",
		{  1,  3,  2,  2,  1,  3,  2,  2 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_magic_missile,    TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		5,      12,	"magic missile",        0,
		{ 220, 220, 0, 0, 220, 220, 0, 0 },     { 440, 0, 0, 0, 440, 0, 0, 0 }
	}},
	{	skill::mass_healing,      {
		"mass healing",
		{ 38, 32, 60, 46, 60, 32, 60, 46 }, {  2,  2,  4,  4,  2,  2,  4,  4 },		
		spell_mass_healing,     TAR_IGNORE,             POS_STANDING,
		100,    36,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::mass_invis,        {
		"mass invis",
		{ 22, 28, 31, 60, 22, 28, 31, 60 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_mass_invis,       TAR_IGNORE,             POS_STANDING,
		20,     24,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::nexus,             {
		"nexus",
		{ 40, 35, 50, 45, 40, 35, 50, 45 }, {  2,  2,  4,  4,  2,  2,  4,  4 },		
		spell_nexus,            TAR_IGNORE,             POS_STANDING,
		150,    36,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::pass_door,         {
		"pass door",
		{ 24, 32, 25, 37, 24, 32, 25, 37 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_pass_door,        TAR_CHAR_DEFENSIVE,     POS_STANDING,
		20,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::plague,            {
		"plague",
		{ 23, 17, 36, 26, 20, 17, 36, 26 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_plague,           TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		20,     12,	"sickness",             0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::poison,            {
		"poison",
		{ 17, 12, 15, 21,  7, 12, 15, 21 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_poison,           TAR_OBJ_CHAR_OFF,       POS_FIGHTING,
		10,     12,	"poison",               0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::power_word,        {
		"power word",
		{ 70, 80, 85, 88, 68, 82, 83, 89 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_power_word,       TAR_CHAR_OFFENSIVE,     POS_STANDING,
		50,     12,	"power word",           0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::polymorph,         {
		"polymorph",
		{ 82, 84, 88, 89, 83, 85, 87, 88 }, {  2,  2,  2,  2,  2,  2,  2,  2 },		
		spell_polymorph,        TAR_IGNORE,             POS_STANDING,
		100,    18,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::portal,            {
		"portal",
		{ 35, 30, 45, 40, 35, 30, 45, 40 }, {  2,  2,  4,  4,  2,  2,  4,  4 },		
		spell_portal,           TAR_IGNORE,             POS_STANDING,
		100,    24,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::protect_container, {
		"protect container",
		{ 53, 51, 55, 57, 53, 51, 55, 57 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_protect_container, TAR_OBJ_INV,            POS_STANDING,
		50,     18,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::protection_evil,   {
		"protection evil",
		{ 12,  9, 17, 11, 12,  9, 17, 11 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_protection_evil,  TAR_CHAR_SELF,          POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 220, 0, 0, 0, 260, 0, 0 },     { 0, 360, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::protection_good,   {
		"protection good",
		{ 12,  9, 17, 11, 12,  9, 17, 11 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_protection_good,  TAR_CHAR_SELF,          POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 220, 0, 0, 240, 260, 0, 0 },     { 0, 360, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::rayban,            {
		"rayban",
		{ 35, 30, 60, 60, 38, 40, 60, 60 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_rayban,           TAR_CHAR_SELF,          POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::ray_of_truth,      {
		"ray of truth",
		{ 40, 35, 60, 47, 33, 35, 60, 47 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_ray_of_truth,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		18,     12,	"ray of truth",         0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::recharge,          {
		"recharge",
		{  9, 20, 60, 60,  9, 25, 60, 60 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_recharge,         TAR_OBJ_INV,            POS_STANDING,
		60,     24,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::refresh,           {
		"refresh",
		{  8,  5, 12,  9,  8,  5, 12,  9 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_refresh,          TAR_CHAR_DEFENSIVE,     POS_STANDING,
		12,     18,	"refresh",              0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::resurrect,         {
		"resurrect",
		{ 83, 82, 90, 90, 70, 81, 90, 90 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_resurrect,        TAR_IGNORE,             POS_STANDING,
		100,    15,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::regeneration,      {
		"regeneration",
		{ 20, 25, 35, 40, 20, 25, 35, 40 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_regeneration,     TAR_CHAR_DEFENSIVE,     POS_STANDING,
		20,     12,	"",                     0,
		{ 320, 220, 370, 400, 330, 310, 360, 280 },     { 0, 340, 0, 0, 0, 0, 0, 400 }
	}},
	{	skill::remove_alignment,  {
		"remove alignment",
		{ 40, 23, 60, 60, 40, 23, 60, 60 }, {  4,  2,  4,  4,  4,  2,  4,  4 },		
		spell_remove_alignment, TAR_OBJ_INV,            POS_STANDING,
		75,     18,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::remove_invis,      {
		"remove invisibility",
		{ 30, 23, 60, 60, 30, 23, 60, 60 }, {  4,  2,  4,  4,  4,  2,  4,  4 },		
		spell_remove_invis,     TAR_OBJ_INV,            POS_STANDING,
		40,     18,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::remove_curse,      {
		"remove curse",
		{ 20, 18, 60, 22, 25, 18, 60, 22 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_remove_curse,     TAR_OBJ_CHAR_DEF,       POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::sanctuary,         {
		"sanctuary",
		{ 25, 20, 42, 30, 32, 20, 42, 30 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_sanctuary,        TAR_CHAR_DEFENSIVE,     POS_STANDING,
		75,     12,	"sanctuary",            0,
		{ 400, 250, 0, 0, 0, 400, 0, 0 },       { 0, 500, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::scry, {
		"scry",
		{ 30, 50, 60, 70, 40, 50, 50, 30 }, { 1, 1, 2, 2, 1, 2, 1, 1},		
		spell_scry,  TAR_IGNORE,  POS_RESTING,
		30, 20,	"", 0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0}
	}},
	{	skill::shield,            {
		"shield",
		{ 20, 35, 35, 40, 20, 35, 35, 40 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_shield,           TAR_CHAR_DEFENSIVE,     POS_STANDING,
		12,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::shock_blade,       {
		"shock blade",
		{ 30, 58, 60, 78, 35, 54, 63, 70 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_shock_blade,      TAR_OBJ_INV,            POS_STANDING,
		100,    12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::shocking_grasp,    {
		"shocking grasp",
		{ 10, 15, 14, 13, 10, 15, 14, 13 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_shocking_grasp,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		7,      12,	"shocking grasp",       0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::shrink,            {
		"shrink",
		{ 22, 35, 60, 60, 22, 40, 60, 60 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_shrink,           TAR_OBJ_INV,            POS_STANDING,
		100,    12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::sleep,             {
		"sleep",
		{ 10, 31, 11, 60, 10, 31, 11, 60 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_sleep,            TAR_CHAR_OFFENSIVE,     POS_STANDING,
		15,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::slow,              {
		"slow",
		{ 23, 30, 29, 32, 23, 30, 29, 32 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_slow,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		30,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::smokescreen,       {
		"smokescreen",
		{ 21, 28, 23, 33, 21, 28, 23, 33 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_smokescreen,      TAR_IGNORE,             POS_STANDING,
		25,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::starve,            {
		"starve",
		{ 30, 25, 40, 45, 22, 52, 40, 26 }, { 1, 1, 2, 2, 1, 2, 2, 1 },		
		spell_starve,           TAR_CHAR_OFFENSIVE,         POS_STANDING,
		10,    8,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::steel_mist,        {
		"steel mist",
		{ 17, 12, 20, 15, 17, 12, 20, 15 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_steel_mist,       TAR_CHAR_DEFENSIVE,     POS_STANDING,
		35,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::stone_skin,        {
		"stone skin",
		{ 25, 40, 40, 45, 25, 40, 40, 45 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_stone_skin,       TAR_CHAR_SELF,          POS_STANDING,
		12,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::summon,            {
		"summon",
		{ 23, 15, 29, 22, 23, 15, 29, 22 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_summon,           TAR_IGNORE,             POS_STANDING,
		50,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::summon_object,     {
		"summon object",
		{ 74, 72, 89, 82, 74, 78, 89, 82 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_summon_object,    TAR_IGNORE,             POS_STANDING,
		65,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::sunray,            {
		"sunray",
		{ 79, 70, 82, 84, 79, 75, 80, 83 }, {  2,  2,  2,  2,  2,  2 , 2,  2 },		
		spell_sunray,           TAR_CHAR_OFFENSIVE,    POS_FIGHTING,
		40, 12,	"sunray",               0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },    { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::talon,             {
		"talon",
		{ 53, 51, 55, 57, 53, 51, 55, 57 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_talon,            TAR_CHAR_DEFENSIVE,     POS_STANDING,
		25,     12,	"",                     0,
		{ 250, 400, 0, 0, 400, 0, 0, 0 },       { 500, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::teleport,          {
		"teleport",
		{ 13, 22, 25, 36, 13, 22, 25, 36 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_teleport,         TAR_CHAR_SELF,          POS_FIGHTING,
		35,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::teleport_object,   {
		"teleport object",
		{ 52, 53, 55, 56, 52, 53, 55, 56 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_teleport_object,  TAR_IGNORE,             POS_STANDING,
		25,     18,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::undo_spell,        {
		"undo spell",
		{ 38, 40, 45, 45, 38, 38, 45, 45 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_undo_spell,       TAR_IGNORE,             POS_FIGHTING,
		15,     18,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::ventriloquate,     {
		"ventriloquate",
		{  1, 60,  2, 60,  1, 60,  2, 60 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_ventriloquate,    TAR_IGNORE,             POS_STANDING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::vision,            {
		"vision",
		{ 29, 37, 62, 58, 29, 37, 62, 48 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_vision,           TAR_IGNORE,             POS_FIGHTING,
		80,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::weaken,            {
		"weaken",
		{ 11, 14, 16, 17, 11, 14, 16, 17 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_weaken,           TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		20,     12,	"spell",                0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::word_of_recall,    {
		"word of recall",
		{ 32, 28, 40, 30, 32, 28, 40, 30 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_word_of_recall,   TAR_CHAR_SELF,          POS_RESTING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::wrath,             {
		"wrath",
		{ 85, 86, 87, 88, 85, 86, 87, 88 }, {  2,  2,  2,  2,  2,  2,  2,  2 },		
		spell_wrath,            TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		30,     12,	"wrath",                0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},

	/* combat and weapons skills */

	{	skill::axe,               {
		"axe",
		{  1,  1,  1,  1,  1,  1,  1,  1 }, {  6,  6,  5,  4,  6,  5,  5,  4 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::dagger,            {
		"dagger",
		{  1,  1,  1,  1,  1,  1,  1,  1 }, {  2,  3,  2,  2,  2,  3,  2,  2 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::flail,             {
		"flail",
		{  1,  1,  1,  1,  1,  1,  1,  1 }, {  6,  3,  6,  4,  6,  4,  6,  4 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::mace,              {
		"mace",
		{  1,  1,  1,  1,  1,  1,  1,  1 }, {  5,  2,  3,  3,  5,  2,  3,  3 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::polearm,           {
		"polearm",
		{  1,  1,  1,  1,  1,  1,  1,  1 }, {  6,  6,  6,  4,  6,  5,  6,  4 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::spear,             {
		"spear",
		{  1,  1,  1,  1,  1,  1,  1,  1 }, {  4,  4,  4,  3,  4,  4,  4,  3 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::sword,             {
		"sword",
		{  1,  1,  1,  1,  1,  1,  1,  1 }, {  5,  6,  3,  2,  5,  2,  3,  2 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::whip,              {
		"whip",
		{  1,  1,  1,  1,  1,  1,  1,  1 }, {  6,  5,  5,  4,  6,  5,  5,  4 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::archery,                  {
		"archery",
		{ 61, 41, 35, 5, 61, 25, 1, 1 }, { 6, 5, 5, 4, 6, 5, 5, 4 },		
		spell_null,                TAR_IGNORE,            POS_STANDING,
		0,      0,	"",                        0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },       { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::shield_block,      {
		"shield block",
		{  1,  1,  1,  1,  1,  1,  1,  1 }, {  6,  4,  6,  2,  6,  3,  6,  2 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::brew,              {
		"brew",
		{  1,  1, 60, 60,  1, 40, 60, 60 }, {  3,  3, 15, 15,  3, 10, 15, 15 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		15,     24,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::scribe,            {
		"scribe",
		{  1,  1, 60, 60,  1, 40, 60, 60 }, {  3,  3, 15, 15,  3, 10, 15, 15 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		15,     24,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::backstab,          {
		"backstab",
		{ 60, 60,  1, 63, 56, 70,  5, 63 }, { 22, 24,  5, 20, 22, 25,  5, 20 },		
		spell_null,             TAR_CHAR_OFFENSIVE,             POS_STANDING,
		30,     24,	"backstab",             0,
		{ 0, 0, 200, 0, 0, 0, 275, 0 },     { 0, 0, 425, 0, 0, 0, 475, 0 }
	}},
	{	skill::bash,              {
		"bash",
		{ 65, 58, 60,  1, 65, 15, 60,  1 }, {  8,  6,  5,  4,  8,  5,  5,  4 },		
		spell_null,             TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
		15,     24,	"bash",                 0,
		{ 0, 0, 0, 200, 0, 0, 0, 0 },   { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::berserk,           {
		"berserk",
		{ 60, 60, 60, 18, 60, 27, 60, 18 }, { 14, 13, 10,  5, 14,  8, 10,  5 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		30,     24,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::circle,            {
		"circle",
		{ 66, 66, 31, 66, 66, 68, 40, 66 }, { 22, 24,  6, 20, 22, 24, 10, 20 },		
		spell_null,             TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
		20,     36,	"circle",               0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::crush,             {
		"crush",
		{ -1, -1, -1, -1, -1, -1, -1, -1 }, { -1, -1, -1, -1, -1, -1, -1, -1 },		
		spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		15,     12,	"crushing blow",        -1,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::dirt_kicking,      {
		"dirt kicking",
		{ 60, 60,  3,  3, 60, 14,  3,  3 }, {  6,  6,  4,  4,  6,  5,  4,  4 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		10,     24,	"kicked dirt",          0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::disarm,            {
		"disarm",
		{ 60, 60, 12, 11, 60, 27, 12, 11 }, {  8,  8,  6,  4,  8,  7,  6,  4 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		15,     24,	"",                     0,
		{0, 0, 500, 350, 0, 500, 0, 500 },      { 0, 0, 0, 700, 0, 0, 0, 0 }
	}},
	{	skill::dodge,             {
		"dodge",
		{ 20, 22,  1, 13, 20, 13,  1, 13 }, {  8,  8,  4,  6,  8,  6,  4,  6 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::enhanced_damage,   {
		"enhanced damage",
		{ 45, 30, 25,  1, 45, 15, 25,  1 }, { 20, 20,  5,  3, 20,  4,  5,  3 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::envenom,           {
		"envenom",
		{ 55, 55, 10, 60, 50, 60, 10, 60 }, { 10, 10,  4,  8, 10, 12,  4,  8 },		
		spell_null,             TAR_IGNORE,             POS_RESTING,
		10,     36,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::hand_to_hand,      {
		"hand to hand",
		{ 25, 10, 15,  6, 25,  6, 15,  6 }, {  8,  5,  6,  4,  8,  5,  6,  4 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::kick,              {
		"kick",
		{ 48, 12, 14,  8, 48, 10, 14,  8 }, {  8,  4,  6,  3,  8,  4,  6,  3 },		
		spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		15,     12,	"kick",                 0,
		{ 0, 0, 300, 250, 0, 300, 0, 250 },     { 0, 0, 0, 425, 0, 0, 0, 425 }
	}},
	/*wchange added for evo 2+ kick's second hit*/
	{	skill::roundhouse,         {
		"roundhouse",
		{ 0, 0, 0,  0, 0, 0, 0,  0 }, {  0,  0,  0,  0,  0,  0,  0,  0 },		
		spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		15,     12,	"roundhouse",                 0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	/* wchange evo 3 kick's foot sweep*/
	{	skill::footsweep,         {
		"footsweep",
		{ 0, 0, 0,  0, 0, 0, 0,  0 }, {  0,  0,  0,  0,  0,  0,  0,  0 },		
		spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		15,     12,	"footsweep",                 0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},

	{	skill::parry,             {
		"parry",
		{ 22, 20, 13,  1, 22, 10, 13,  1 }, {  8,  8,  6,  4,  8,  5,  6,  4 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::rescue,            {
		"rescue",
		{ 60, 60, 60,  1, 60,  1, 60,  1 }, {  8,  7,  6,  4,  8,  2,  6,  4 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		5,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::trip,              {
		"trip",
		{ 60, 60,  1, 15, 60, 60,  1, 15 }, { 10, 10,  4,  8, 10, 10,  4,  8 },		
		spell_null,             TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
		15,     24,	"trip",                 0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::second_attack,     {
		"second attack",
		{ 30, 24, 12,  5, 30, 12, 12,  5 }, { 10,  8,  5,  3, 10,  5,  5,  3 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::third_attack,      {
		"third attack",
		{ 60, 60, 24, 12, 60, 25, 24, 12 }, { 14, 12, 10,  4, 14,  8, 10,  4 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::dual_wield,        {
		"dual wield",
		{ 53, 53, 24, 12, 53, 14,  1, 12 }, { 15, 13, 11,  5, 15,  6, 11,  5 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     0,
		{ 0, 0, 550, 470, 0, 510, 280, 300 },   { 0, 0, 0, 0, 0, 0, 550, 600 }
	}},
	{	skill::hunt,              {
		"hunt",
		{ 53, 53, 15, 20, 53, 50, 17,  3 }, {  8,  8,  4,  4,  8,  8,  4,  4 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		20,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::unarmed,  {
		"unarmed",
		{ 45, 20, 18, 2, 45, 6, 8, 18 }, { 2, 2, 1, 1, 2, 1, 1, 1 },		
		spell_null,  TAR_IGNORE, POS_FIGHTING,
		0, 0,	"smash",   0,
		{ 0, 0, 0, 0, 0, 0, 0, 0},   { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},

	/* non-combat skills */

	{	skill::swimming,          {
		"swimming",
		{  8,  8,  8,  8,  8,  8,  8,  8 }, {  1,  1,  2,  2,  1,  1,  2,  2 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"swimming",             0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::fast_healing,      {
		"fast healing",
		{ 15,  9, 16,  6, 15,  4, 16,  6 }, {  8,  5,  6,  4,  8,  3,  6,  4 },		
		spell_null,             TAR_IGNORE,             POS_SLEEPING,
		0,      0,	"",                     0,
		{ 0, 0, 520, 280, 0, 500, 510, 480 },   { 0, 0, 0, 560, 0, 0, 0, 0 }
	}},
	{	skill::firebuilding,      {
		"firebuilding",
		{  1,  1,  1,  1,  1,  1,  1,  1 }, {  2,  2,  1,  1,  2,  1,  1,  1 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		5,      24,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::forge,             {
		"forge",
		{ 20, 20, 15, 16, 26, 16, 10, 15 }, { 16, 12,  2,  3, 16,  3,  1,  3 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		20,     24,	"",                     0,
		{ 0, 0, 0, 300, 0, 0, 300, 300 },       { 0, 0, 0, 400, 0, 0, 0, 0 }
	}},
	{	skill::repair,            {
		"repair",
		{ 25, 30,  5, 10, 35, 20,  5, 12 }, {  2,  2,  1,  1,  2,  2,  1,  1 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		15,     24,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::rotate,            {
		"rotate",
		{  1,  1,  1,  1,  1,  1,  1,  1 }, {  2,  2,  1,  1,  2,  1,  1,  1 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		5,      24,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::languages,         {
		"languages",
		{  1,  1,  1,  1,  1,  1,  1,  1 }, {  1,  1,  2,  2,  1,  2,  2,  2 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		0,      24,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::haggle,            {
		"haggle",
		{  7, 18,  1, 14,  7, 18,  1, 14 }, {  5,  8,  3,  6,  5,  8,  3,  6 },		
		spell_null,             TAR_IGNORE,             POS_RESTING,
		0,      0,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::hide,              {
		"hide",
		{ 60, 60,  1, 12, 60, 60,  1, 12 }, { 10,  8,  4,  6, 10,  8,  4,  6 },		
		spell_null,             TAR_IGNORE,             POS_RESTING,
		15,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::lore,              {
		"lore",
		{ 10, 10,  6, 20, 10, 20,  6, 20 }, {  6,  6,  4,  8,  6,  8,  4,  8 },		
		spell_null,             TAR_IGNORE,             POS_RESTING,
		5,      36,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::meditation,        {
		"meditation",
		{  6,  6, 15, 15,  6,  6, 15, 15 }, {  5,  5,  8,  8,  5,  5,  8,  8 },		
		spell_null,             TAR_IGNORE,             POS_SLEEPING,
		0,      0,	"",                     0,
		{ 440, 300, 0, 0, 460, 480, 0, 0 },     { 0, 600, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::peek,              {
		"peek",
		{  8, 21,  1, 14,  8, 21,  1, 14 }, {  8,  7,  3,  6,  8,  7,  3,  6 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		5,      0,	"",                     0,
		{ 0, 0, 320, 0, 0, 0, 480, 0 }, { 0, 0, 580, 0, 0, 0, 0, 0 }
	}},
	{	skill::pick_lock,         {
		"pick lock",
		{ 25, 25,  7, 25, 25, 30,  7, 25 }, {  8,  8,  4,  8,  8,  9,  4,  8 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		15,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::scan,              {
		"scan",
		{  1,  1,  1,  1,  1,  1,  1,  1 }, {  1,  1,  1,  1,  1,  1,  1,  1 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		0,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::sneak,             {
		"sneak",
		{ 60, 60,  4, 10, 60, 60,  4, 10 }, { 10,  8,  4,  6, 10,  8,  4,  6 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		15,     12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::steal,             {
		"steal",
		{ 60, 60,  5, 60, 60, 60,  5, 60 }, { 10,  9,  4,  8, 10, 15,  4,  8 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		10,     24,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::sing,              {
		"sing",
		{ 60, 60, 60, 60, 60, 60, 12, 60 }, {  6,  6,  4,  6,  7,  6,  2,  5 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		15,     24,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::scrolls,           {
		"scrolls",
		{  1,  1,  1,  1,  1,  1,  1,  1 }, {  2,  3,  5,  8,  2,  4,  5,  8 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		10,     18,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::spousegate,        {
		"spousegate",
		{ 19, 16, 20, 18, 25, 13, 16, 14 }, { 2, 2, 3, 3, 4, 2, 2, 1 },		
		spell_null,             TAR_IGNORE, POS_STANDING,
		25, 12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::staves,            {
		"staves",
		{  1,  1,  1,  1,  1,  1,  1,  1 }, {  2,  3,  5,  8,  2,  5,  5,  8 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		10,     18,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::wands,             {
		"wands",
		{  1,  1,  1,  1,  1,  1,  1,  1 }, {  2,  3,  5,  8,  2,  5,  5,  8 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		10,     18,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::recall,            {
		"recall",
		{  1,  1,  1,  1,  1,  1,  1,  1 }, {  2,  2,  2,  2,  2,  2,  2,  2 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		0,      12,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::lay_on_hands,      {
		"lay",
		{ 92, 92, 92, 92, 92, 10, 92, 92 }, { 2, 1, 2, 2, 2, 1, 2, 2},		
		spell_null,             TAR_IGNORE,             POS_RESTING,
		0,      24,	"",                     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::familiar, {
		"familiar",
		{ 25, 35, 45, 45, 35, 35, 35, 25 }, { 1, 1, 1, 1, 1, 1, 1, 1},		
		spell_null, TAR_IGNORE, POS_RESTING,
		0, 20,	"", 0,
		{0, 0, 0, 0, 0, 0, 0, 0}, { 0, 0, 0, 0, 0, 0, 0, 0}
	}},
	{	skill::die_hard, {
		"die hard",
		{ 22, 16, 16, 10, 21, 12, 16, 14 }, { 2, 1, 2, 1, 2, 1, 2, 1 },		
		spell_null, TAR_IGNORE, POS_DEAD,
		0, 4,	"", 0,
		{0, 0, 0, 0, 0, 0, 0, 0}, { 0, 0, 0, 0, 0, 0, 0, 0}
	}},

	/* Remort spells and skills, originally by Elrac */
	/* Subdivided by Class */
	/* Everything has slot numbers here, it's more as an ID for saving than for area files */

	/* Mages */

	{	skill::sheen,             {
		"sheen",
		{ 10, 30, 50, 50, 20, 40, 50, 50 }, { 40, 55, 70, 70, 50, 60, 70, 70 },		
		spell_sheen,            TAR_CHAR_SELF,          POS_STANDING,
		80,     12,	"sheen",                1,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::focus,             {
		"focus",
		{ 40, 60, 80, 80, 50, 80, 80, 80 }, { 50, 70, 80, 80, 60, 80, 80, 80 },		
		spell_focus,            TAR_CHAR_SELF,          POS_STANDING,
		200,     12,	"focus",                1,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::paralyze,          {
		"paralyze",
		{ 30, 60, 70, 70, 50, 70, 70, 70 }, { 50, 80, 100, 100, 70, 100, 100, 100 },		
		spell_paralyze,         TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		50,     12,	"paralyze",             1,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	/*    {	skill::ironskin,          {
	        "ironskin",
		{ 20, 20, 60, 60, 20, 20, 60, 60 }, { 30, 30, 30, 30, 30, 30, 30, 30 },		
	        spell_ironskin,         TAR_CHAR_SELF,          POS_STANDING,
	        100,     12,	"iron skin",            1,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	    }}, */

	/* Clerics */

	{	skill::barrier,           {
		"barrier",
		{ 55, 40, 70, 70, 60, 65, 70, 70 }, { 60, 50, 90, 90, 70, 80, 90, 90 },		
		spell_barrier,          TAR_CHAR_SELF,          POS_STANDING,
		150,     12,	"barrier",              2,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::dazzle,            {
		"dazzle",
		{ 30, 10, 50, 50, 40, 40, 50, 50 }, { 60, 40, 80, 80, 70, 70, 80, 80 },		
		spell_dazzle,           TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		50,     12,	"dazzle",               2,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::full_heal,         {
		"full heal",
		{ 55, 30, 70, 70, 70, 50, 70, 70 }, { 80, 60, 100, 100, 100, 70, 100, 100 },		
		spell_full_heal,        TAR_CHAR_DEFENSIVE,     POS_STANDING,
		100,     12,	"full heal",            2,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},

	/* Thieves */

	{	skill::midnight,          {
		"midnight",
		{ 50, 50, 25, 50, 50, 50, 40, 40 }, { 80, 80, 50, 80, 80, 80, 70, 80 },		
		spell_midnight,         TAR_CHAR_SELF,     POS_STANDING,
		100,     12,	"midnight",             3,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::shadow_form,       {
		"shadow form",
		{ 70, 80, 50, 80, 65, 80, 60, 80 }, { 80, 90, 60, 90, 75, 90, 70, 90 },		
		spell_null,             TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
		50,     36,	"shadow form",          3,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::hone,              {
		"hone",
		{ 35, 40, 10, 25, 35, 25, 25, 25 }, { 65, 70, 40, 55, 65, 55, 55, 55 },		
		spell_null,             TAR_IGNORE,             POS_STANDING,
		40,     24,	"",                     3,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},

	/* Warriors */

	{	skill::riposte,           {
		"riposte",
		{ 70, 65, 55, 25, 70, 40, 55, 40 }, { 90, 90, 80, 50, 90, 70, 80, 70 },		
		spell_null,             TAR_IGNORE,          POS_STANDING,
		0,      0,	"riposte",              4,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::fourth_attack,     {
		"fourth attack",
		{ 80, 80, 75, 50, 80, 70, 75, 70 }, { 100, 100, 90, 60, 100, 80, 90, 80 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     4,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::rage,              {
		"rage",
		{ 70, 70, 60, 40, 70, 55, 60, 55 }, { 80, 80, 70, 50, 80, 60, 70, 60 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		40,     36,	"rage",                 4,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::blind_fight,        {
		"blind fight",
		{ 70, 61, 55, 28, 85, 36, 55, 60 }, { 60, 60, 50, 30, 60, 40, 40, 40 },		
		spell_null,              TAR_IGNORE,   POS_STANDING,
		0, 0,	"",                      4,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },   { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},

	/* Necromancers */

	{	skill::sap,               {
		"sap",
		{ 65, 75, 80, 80, 50, 80, 75, 80 }, { 70, 80, 90, 90, 60, 90, 80, 90 },		
		spell_sap,              TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		80,     12,	"sap",                  5,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::pain,              {
		"pain",
		{ 45, 55, 70, 70, 30, 60, 70, 70 }, { 60, 70, 80, 80, 50, 70, 80, 80 },		
		spell_pain,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		50,     12,	"torments",             5,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::hex,               {
		"hex",
		{ 40, 50, 60, 60, 20, 50, 60, 60 }, { 60, 70, 80, 80, 50, 70, 80, 80 },		
		spell_hex,              TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		100,     12,	"hex",                  5,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::bone_wall,         {
		"bone wall",
		{ -1, -1, -1, -1, -1, -1, -1, -1 }, { -1, -1, -1, -1, -1, -1, -1, -1 },		
		spell_bone_wall,                TAR_CHAR_SELF,          POS_STANDING,
		80,     12,	"wall of bones",        5,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},

	/* Paladins */

	{	skill::hammerstrike,      {
		"hammerstrike",
		{ 70, 45, 60, 50, 70, 30, 60, 50 }, { 90, 65, 80, 70, 90, 50, 80, 70 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,     36,	"hammerstrike",         6,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::force_shield,      {
		"force shield",
		{ 70, 60, 80, 80, 70, 50, 80, 80 }, { 70, 60, 80, 80, 70, 50, 80, 80 },		
		spell_force,            TAR_CHAR_SELF,          POS_FIGHTING,
		150,     12,	"force shield",         6,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::holy_sword,        {
		"holy sword",
		{ 70, 50, 70, 70, 70, 10, 70, 70 }, { 100, 70, 100, 100, 100, 40, 100, 100 },		
		spell_holy_sword,       TAR_IGNORE,             POS_STANDING,
		40,     36,	"",                     6,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},

	/* Bards */

	{	skill::align,             {
		"align",
		{ 30, 40, 30, 35, 40, -1 /*40*/, 10, 30 }, { 70, 80, 70, 75, 80, -1 /*80*/, 50, 70 },		
		spell_null,             TAR_IGNORE,          POS_STANDING,
		5,      0,	"",                     7,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::blur,              {
		"blur",
		{ 70, 70, 45, 60, 70, 60, 30, 60 }, { 100, 100, 75, 90, 100, 90, 60, 90 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     7,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::dual_second,       {
		"dual second",
		{ 80, 80, 70, 60, 80, 80, 30, 60 }, { 90, 90, 80, 70, 90, 90, 50, 70 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     7,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::quick,             {
		"quick",
		{ -1, -1, -1, -1, -1, -1, -1, -1 }, { -1, -1, -1, -1, -1, -1, -1, -1 },		
		spell_quick,            TAR_IGNORE,             POS_FIGHTING,
		60,     12,	"quick",                7,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},

	/* Rangers */

	{	skill::standfast,         {
		"standfast",
		{ 40, 35, 25, 20, 40, 10, 25, 20 }, { 70, 65, 55, 40, 70, 65, 55, 50 },		
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		0,      0,	"",                     8,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::mark,              {
		"mark",
		{ 60, 60, 50, 50, 60, 50, 40, 30 }, { 80, 80, 70, 70, 80, 70, 60, 50 },		
		spell_null,             TAR_IGNORE,          POS_STANDING,
		50,      0,	"",                     8,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},
	{	skill::critical_blow,     {
		"critical blow",
		{ 80, 80, 60, 70, 80, 65, 70, 50 }, { 100, 100, 75, 90, 100, 70, 75, 60 },		
		spell_null,             TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
		30,     36,	"critical blow",        8,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	}},

	/* End Remort Skills/Spells */
};

const std::vector<group_type> group_table = {
	{
		"rom basics",           { 0, 0, 0, 0 , 0, 0, 0, 0 },
		{ "scrolls", "staves", "wands", "recall", "scan" }
	},
	{
		"mage basics",          { 0, -1, -1, -1 , -1, -1, -1, -1 },
		{ "dagger", "brew" }
	},
	{
		"cleric basics",        { -1, 0, -1, -1 , -1, -1, -1, -1 },
		{ "mace", "scribe" }
	},
	{
		"thief basics",         { -1, -1, 0, -1 , -1, -1, -1, -1 },
		{ "dagger", "steal" }
	},
	{
		"warrior basics",       { -1, -1, -1, 0, -1, -1, -1, -1 },
		{ "sword", "bow", "second attack" }
	},
	{
		"necromancer basics",   { -1, -1, -1, -1, 0, -1, -1, -1 },
		{ "polearm", "dagger", "scribe" }
	},
	{
		"paladin basics",       { -1, -1, -1, -1, -1, 0, -1, -1 },
		{ "sword", "rescue", "shield block" }
	},
	{
		"bard basics",          { -1, -1, -1, -1, -1, -1, 0, -1 },
		{ "axe", "flail", "sing"}
	},
	{
		"ranger basics",        { -1, -1, -1, -1, -1, -1, -1, 0 },
		{ "axe", "flail", "bow", "dual wield" }
	},
	{
		"mage default",         { 40, -1, -1, -1, -1, -1, -1, -1 },
		{
			"lore", "beguiling", "combat", "detection", "enhancement", "illusion",
			"maladictions", "protective", "transportation", "weather", "familiar"
		}
	},
	{
		"cleric default",       { -1, 40, -1, -1, -1, -1, -1, -1 },
		{
			"flail", "attack", "creation", "curative",  "benedictions", "detection", "healing",
			"maladictions", "protective", "shield block", "transportation", "weather"
		}
	},
	{
		"thief default",        { -1, -1, 40, -1, -1, -1, -1, -1 },
		{
			"mace", "sword", "backstab", "disarm", "dodge", "second attack",
			"trip", "hide", "peek", "pick lock", "sneak", "circle"
		}
	},
	{
		"warrior default",      { -1, -1, -1, 40, -1, -1, -1, -1 },
		{
			"weaponsmaster", "shield block", "bash", "disarm", "enhanced damage",
			"parry", "rescue", "third attack", "unarmed"
		}
	},
	{
		"necromancer default",  { -1, -1, -1, -1, 40, -1, -1, -1 },
		{
			"beguiling", "combat", "detection", "necromancy", "illusion",
			"maladictions", "protective", "transportation", "weather"
		}
	},
	{
		"paladin default",      { -1, -1, -1, -1, -1, 40, -1, -1 },
		{
			"mace", "creation", "benedictions", "detection", "healing", "second attack",
			"third attack", "rotate", "parry", "scribe", "lay"
		}
	},
	{
		"bard default",         { -1, -1, -1, -1, -1, -1, 40, -1 },
		{
			"weaponsmaster", "haggle", "disarm", "dodge", "second attack",
			"trip", "hide", "sneak", "parry"
		}
	},
	{
		"ranger default",       { -1, -1, -1, -1, -1, -1, -1, 40 },
		{
			"weaponsmaster", "shield block", "kick", "disarm", "enhanced damage",
			"parry", "trip", "second attack", "hunt", "familiar"
		}
	},
	{
		"weaponsmaster",        { 40, 40, 40, 20, 40, 25, 10, 20 },
		{ "axe", "dagger", "flail", "mace", "polearm", "spear", "sword", "whip" }
	},
	{
		"attack",               { 8, 5, 12, 8, 10, 9, 12, 8 },
		{
			"demonfire", "dispel evil", "dispel good", "earthquake",
			"flamestrike", "heat metal", "ray of truth", "sunray"
		}
	},
	{
		"beguiling",            { 4, 8, 6, 15, 4, 10, 6, 15 },
		{ "calm", "charm person", "sleep", "power word"}
	},
	{
		"benedictions",         { 6, 4, 14, 8, 8, 4, 14, 8 },
		{
			"bless", "calm", "frenzy", "holy word", "remove curse",
			"remove alignment", "remove invisibility"
		}
	},
	{
		"combat",               { 6, 10, 10, 9, 6, 10, 10, 9 },
		{
			"acid blast", "burning hands", "chain lightning", "chill touch", "colour spray",
			"fireball", "lightning bolt", "magic missile", "shocking grasp", "firestorm", "wrath"
		}
	},
	{
		"creation",             { 4, 4, 8, 8, 4, 4, 8, 8 },
		{
			"continual light", "create food", "create spring", "create water", "create rose",
			"create vial", "create parchment", "floating disc", "encampment", "create sign"
		}
	},
	{
		"curative",             { 8, 4, 12, 10, 8, 4, 12, 10 },
		{ "cure blindness", "cure disease", "cure poison" }
	},
	{
		"detection",            { 4, 3, 6, 10, 4, 3, 6, 10 },
		{
			"detect evil", "detect good", "detect hidden", "detect invis", "detect magic", "detect poison",
			"identify", "light of truth", "know alignment", "locate object", "locate life", "scry", "vision"
		}
	},
	{
		"draconian",            { 10, 10, 12, 15, 6, 10, 12, 15 },
		{ "acid breath", "fire breath", "frost breath", "gas breath", "lightning breath"  }
	},
	{
		"enchantment",          { 6, 15, 23, 25, 6, 20, 23, 25 },
		{
			"enchant armor", "enchant weapon", "recharge", "protect container", "dazzling light",
			"remove alignment", "remove invisibility", "shrink"
		}
	},
	{
		"enhancement",          { 5, 9, 9, 9, 5, 10, 9, 9 },
		{
			"giant strength", "haste", "infravision", "refresh",
			"divine regeneration", "regeneration", "talon", "channel"
		}
	},
	{
		"harmful",              { 7, 3, 10, 6, 6, 11, 10, 6 },
		{ "cause critical", "cause light", "cause serious", "harm" }
	},
	{
		"healing",              { 6, 3, 15, 6, 12, 4, 15, 6 },
		{
			"cure critical", "cure light", "cure serious", "heal",
			"divine healing", "mass healing", "refresh"
		}
	},
	{
		"illusion",             { 4, 10, 7, 15, 4, 10, 7, 15 },
		{ "invis", "mass invis", "ventriloquate", "smokescreen" }
	},
	{
		"maladictions",         { 4, 5, 9, 9, 4, 12, 9, 9 },
		{
			"blindness", "curse", "energy drain", "plague", "poison",
			"slow", "weaken", "age", "starve"
		}
	},
	{
		"necromancy",           { 6, 12, 20, 20, 2, 22, 20, 20 },
		{
			"blood moon", "animate skeleton", "animate zombie", "animate wraith",
			"animate gargoyle", "fear", "resurrect", "energy drain"
		}
	},
	{
		"bladecraft",           { 4, 9, 11, 15, 5, 8, 12, 14 },
		{ "flame blade", "frost blade", "blood blade", "shock blade" }
	},
	{
		"protective",           { 4, 4, 7, 8, 4, 5, 7, 8 },
		{
			"armor", "cancellation", "dispel magic", "fireproof", "protection evil",
			"protection good", "rayban", "sanctuary", "shield", "stone skin", "undo spell"
		}
	},
	{
		"transportation",       { 4, 4, 8, 9, 4, 5, 8, 9 },
		{
			"fly", "gate", "nexus", "pass door", "portal", "summon", "teleport",
			"word of recall", "teleport object", "summon object"
		}
	},
	{
		"weather",              { 4, 4, 8, 8, 4, 5, 8, 8 },
		{
			"call lightning", "control weather", "faerie fire", "faerie fog",
			"lightning bolt", "flameshield", "steel mist", "blizzard", "acid rain"
		}
	},
	{
		"alteration",           { 5, 7, 9, 15, 5, 15, 7, 9 },
		{ "darkness", "change sex", "polymorph", "heat metal" }
	},
};

