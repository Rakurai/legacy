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
#include "magic.h"

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
	{       ITEM_ROOM_KEY,  "room_key"      },
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
};

/* weapon selection table */

const std::vector<weapon_type> weapon_table = {
	{       "sword",        OBJ_VNUM_SCHOOL_SWORD,  WEAPON_SWORD,   &gsn_sword      },
	{       "mace",         OBJ_VNUM_SCHOOL_MACE,   WEAPON_MACE,    &gsn_mace       },
	{       "dagger",       OBJ_VNUM_SCHOOL_DAGGER, WEAPON_DAGGER,  &gsn_dagger     },
	{       "axe",          OBJ_VNUM_SCHOOL_AXE,    WEAPON_AXE,     &gsn_axe        },
	{       "staff",        OBJ_VNUM_SCHOOL_STAFF,  WEAPON_SPEAR,   &gsn_spear      },
	{       "flail",        OBJ_VNUM_SCHOOL_FLAIL,  WEAPON_FLAIL,   &gsn_flail      },
	{       "whip",         OBJ_VNUM_SCHOOL_WHIP,   WEAPON_WHIP,    &gsn_whip       },
	{       "polearm",      OBJ_VNUM_SCHOOL_POLEARM, WEAPON_POLEARM, &gsn_polearm    },
	{       "bow",          OBJ_VNUM_SCHOOL_BOW,    WEAPON_BOW,     &gsn_archery     },
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
	                act bits,       aff_by bits,    off bits,
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
		BIT_A|BIT_H|BIT_M|BIT_V,       BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J|BIT_K
	},
	{
		"elf",          TRUE,
		0,              0,              0,
		0,              RES_CHARM,      VULN_IRON,
		BIT_A|BIT_H|BIT_M|BIT_V,       BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J|BIT_K
	},
	{
		"dwarf",        TRUE,
		0,              AFF_NIGHT_VISION,  0,
		0,              RES_POISON | RES_DISEASE, VULN_DROWNING,
		BIT_A|BIT_H|BIT_M|BIT_V,       BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J|BIT_K
	},
	{
		"giant",        TRUE,
		0,              0,              0,
		0,              RES_FIRE | RES_COLD,      VULN_MENTAL | VULN_CHARM,
		BIT_A|BIT_H|BIT_M|BIT_V,       BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J|BIT_K
	},
	{
		"feral",        TRUE,
		0,              AFF_NIGHT_VISION,       OFF_DODGE,
		0,              0,              VULN_DROWNING | VULN_ELECTRICITY,
		BIT_A|BIT_H|BIT_M|BIT_V,       BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J|BIT_K
	},
	{
		"vampire",      TRUE,
		0,              AFF_FLYING,     0,
		0,              RES_NEGATIVE | RES_CHARM, VULN_LIGHT | VULN_HOLY,
		BIT_A|BIT_D|BIT_M|BIT_V,       BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J|BIT_K|BIT_P
	},
	{
		"irda",         TRUE,
		0,              0,              0,
		0,              0,              0,
		BIT_A|BIT_H|BIT_M|BIT_V,       BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J
	},
	{
		"faerie",       TRUE,
		0,              AFF_FLYING | AFF_HASTE,           0,
		0,              0,              0,
		BIT_A|BIT_H|BIT_M|BIT_V,       BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J|BIT_K|BIT_P
	},
	{
		"dragon",       TRUE,
		0,              AFF_FLYING,     0,
		0,              RES_SLASH | RES_BASH,     VULN_PIERCE,
		BIT_A|BIT_H|BIT_Z,         BIT_A|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J|BIT_K|BIT_P|BIT_Q|BIT_U|BIT_V|BIT_X
	},
	{
		"wolf",         TRUE,
		0,              0,              0,
		0,              RES_COLD,       VULN_FIRE,
		BIT_A|BIT_D|BIT_M|BIT_V,       BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J|BIT_K
	},
	{
		"seraph",       TRUE,
		0,              AFF_FLYING,     0,
		0,              RES_HOLY | RES_LIGHT,     VULN_NEGATIVE,
		BIT_A|BIT_H|BIT_M|BIT_V,       BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J|BIT_P
	},
	{
		"superhuman",   TRUE,
		0,              0,              0,
		0,              0,              0,
		BIT_A|BIT_H|BIT_M|BIT_V,       BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J
	},
	{
		"troll",        TRUE,
		0,              AFF_REGENERATION,       0,
		0,              RES_PIERCE | RES_POISON,  VULN_FIRE | VULN_ACID,
		BIT_A|BIT_B|BIT_M|BIT_V,       BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J|BIT_K|BIT_L|BIT_U|BIT_W|BIT_Y
	},

	/*** non-PC RACES ***/

	{
		"bat",          FALSE,
		0,              AFF_FLYING,     OFF_DODGE | OFF_FAST,
		0,              0,              VULN_LIGHT,
		BIT_A|BIT_G|BIT_V,         BIT_A|BIT_C|BIT_D|BIT_E|BIT_F|BIT_H|BIT_J|BIT_K|BIT_P
	},
	{
		"bear",         FALSE,
		0,              0,              OFF_CRUSH | OFF_DISARM | OFF_BERSERK,
		0,              RES_BASH | RES_COLD,      0,
		BIT_A|BIT_G|BIT_V,         BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_H|BIT_J|BIT_K|BIT_U|BIT_V
	},
	{
		"cat",          FALSE,
		0,              0,              OFF_FAST | OFF_DODGE,
		0,              0,              0,
		BIT_A|BIT_G|BIT_V,         BIT_A|BIT_C|BIT_D|BIT_E|BIT_F|BIT_H|BIT_J|BIT_K|BIT_Q|BIT_U|BIT_V
	},
	{
		"centipede",    FALSE,
		0,              0,              0,
		0,              RES_PIERCE | RES_COLD,    VULN_BASH,
		BIT_A|BIT_B|BIT_G|BIT_O,       BIT_A|BIT_C|BIT_K
	},
	{
		"crystal",      FALSE,
		0,              AFF_REGENERATION,       0,
		0,              0,              VULN_FIRE,
		BIT_A|BIT_H|BIT_M|BIT_V,       BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J|BIT_K
	},
	{
		"dog",          FALSE,
		0,              0,              OFF_FAST,
		0,              0,              0,
		BIT_A|BIT_G|BIT_V,         BIT_A|BIT_C|BIT_D|BIT_E|BIT_F|BIT_H|BIT_J|BIT_K|BIT_U|BIT_V
	},
	{
		"doll",         FALSE,
		0,              0,              0,
		IMM_COLD | IMM_POISON | IMM_HOLY | IMM_NEGATIVE | IMM_MENTAL | IMM_DISEASE | IMM_DROWNING,
		RES_BASH | RES_LIGHT,
		VULN_SLASH | VULN_FIRE | VULN_ACID | VULN_ELECTRICITY | VULN_ENERGY,
		BIT_E|BIT_J|BIT_M|BIT_cc,      BIT_A|BIT_B|BIT_C|BIT_G|BIT_H|BIT_K
	},
	{
		"fido",         FALSE,
		0,              0,              OFF_DODGE | ASSIST_RACE,
		0,              0,              VULN_MAGIC,
		BIT_A|BIT_B|BIT_G|BIT_V,       BIT_A|BIT_C|BIT_D|BIT_E|BIT_F|BIT_H|BIT_J|BIT_K|BIT_Q|BIT_V
	},
	{
		"fox",          FALSE,
		0,              0,              OFF_FAST | OFF_DODGE,
		0,              0,              0,
		BIT_A|BIT_G|BIT_V,         BIT_A|BIT_C|BIT_D|BIT_E|BIT_F|BIT_H|BIT_J|BIT_K|BIT_Q|BIT_V
	},
	{
		"goblin",       FALSE,
		0,              AFF_NIGHT_VISION,   0,
		0,              RES_DISEASE,    VULN_MAGIC,
		BIT_A|BIT_H|BIT_M|BIT_V,       BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J|BIT_K
	},
	{
		"hobgoblin",    FALSE,
		0,              AFF_NIGHT_VISION,   0,
		0,              RES_DISEASE | RES_POISON, 0,
		BIT_A|BIT_H|BIT_M|BIT_V,       BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J|BIT_K|BIT_Y
	},
	{
		"horse",        FALSE,
		0,              0,              0,
		0,              0,              0,
		BIT_A|BIT_G|BIT_V,         BIT_A|BIT_C|BIT_D|BIT_E|BIT_F|BIT_H|BIT_J|BIT_K
	},
	{
		"kobold",       FALSE,
		0,              AFF_NIGHT_VISION,   0,
		0,              RES_POISON,     VULN_MAGIC,
		BIT_A|BIT_B|BIT_H|BIT_M|BIT_V,     BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J|BIT_K|BIT_Q
	},
	{
		"lizard",       FALSE,
		0,              0,              0,
		0,              RES_POISON,     VULN_COLD,
		BIT_A|BIT_G|BIT_X|BIT_cc,      BIT_A|BIT_C|BIT_D|BIT_E|BIT_F|BIT_H|BIT_K|BIT_Q|BIT_V
	},
	{
		"modron",       FALSE,
		0,              AFF_NIGHT_VISION,   ASSIST_RACE | ASSIST_ALIGN,
		IMM_CHARM | IMM_DISEASE | IMM_MENTAL | IMM_HOLY | IMM_NEGATIVE, RES_FIRE | RES_COLD | RES_ACID,     0,
		BIT_H,             BIT_A|BIT_B|BIT_C|BIT_G|BIT_H|BIT_J|BIT_K
	},
	{
		"orc",          FALSE,
		0,              AFF_NIGHT_VISION,   0,
		0,              RES_DISEASE,    VULN_LIGHT,
		BIT_A|BIT_H|BIT_M|BIT_V,       BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J|BIT_K
	},
	{
		"pig",          FALSE,
		0,              0,              0,
		0,              0,              0,
		BIT_A|BIT_G|BIT_V,         BIT_A|BIT_C|BIT_D|BIT_E|BIT_F|BIT_H|BIT_J|BIT_K
	},
	{
		"rabbit",       FALSE,
		0,              0,              OFF_DODGE | OFF_FAST,
		0,              0,              0,
		BIT_A|BIT_G|BIT_V,         BIT_A|BIT_C|BIT_D|BIT_E|BIT_F|BIT_H|BIT_J|BIT_K
	},
	{
		"school monster",       FALSE,
		ACT_NOALIGN|ACT_NOSUMMON,            0,      0,
		IMM_CHARM,   0,      VULN_MAGIC,
		BIT_A|BIT_M|BIT_V,         BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_H|BIT_J|BIT_K|BIT_Q|BIT_U
	},
	{
		"snake",        FALSE,
		0,              0,              0,
		0,              RES_POISON,     VULN_COLD,
		BIT_A|BIT_G|BIT_X|BIT_Y|BIT_cc,    BIT_A|BIT_D|BIT_E|BIT_F|BIT_K|BIT_L|BIT_Q|BIT_V|BIT_X
	},
	{
		"song bird",    FALSE,
		0,              AFF_FLYING,     OFF_FAST | OFF_DODGE,
		0,              0,              0,
		BIT_A|BIT_G|BIT_W,         BIT_A|BIT_C|BIT_D|BIT_E|BIT_F|BIT_H|BIT_K|BIT_P
	},
	{
		"teilysa",      FALSE,
		0,              0,              0,
		0,              RES_HOLY,       VULN_BASH | VULN_FIRE | VULN_ACID,
		BIT_A|BIT_H|BIT_M|BIT_V,       BIT_A|BIT_B|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G|BIT_H|BIT_I|BIT_J|BIT_K
	},
	{
		"water fowl",   FALSE,
		0,              AFF_FLYING,     0,
		0,              RES_DROWNING,   0,
		BIT_A|BIT_G|BIT_W,         BIT_A|BIT_C|BIT_D|BIT_E|BIT_F|BIT_H|BIT_K|BIT_P
	},
	{
		"wyvern",       FALSE,
		0,              AFF_FLYING | AFF_DETECT_INVIS | AFF_DETECT_HIDDEN,  OFF_BASH | OFF_FAST | OFF_DODGE,
		IMM_POISON,     0,      VULN_LIGHT,
		BIT_A|BIT_B|BIT_G|BIT_Z,       BIT_A|BIT_C|BIT_D|BIT_E|BIT_F|BIT_H|BIT_J|BIT_K|BIT_Q|BIT_V|BIT_X
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
#define SLOT(n) n

const std::vector<skill_type> skill_table = {

	/*
	 * Magic spells.
	 */
	/*
	        Legend:

	        {
	                "acid blast",           { 28, 33, 35, 32, 28, 33, 35, 32 },
	                                        { 1, 1, 2, 2, 1, 1, 2, 2 },
	                spell_acid_blast,       TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	                NULL,                   SLOT(70),       20,     12,
	                "acid blast",           "!Acid Blast!",         "",     0,
	                { 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	        },

	                1,                      2,
	                                        3,
	                4,                      5,                      6,
	                7,                      8,              9,      10,
	                11,                     12,                     13,     14,
	                15,                             16

	        1. name, referred to in skill_lookup, various other places.
	        2. skill_level[], level at which a class gets a skill/spell.
	        3. rating[], how much it costs, in trains for skills and pracs for spells.
	                mage, cleric, thief, warrior, necro, paladin, bard, ranger
	        4. spell_fun, use spell_null for non-spells, and the function's name otherwise.
	        5. target, used in do_cast.  Determines legal targets.  For skills, used
	           to determine stamina cost, TAR_CHAR_OFFENSIVE is variable cost skills
	        6. minimum_position, sets required position for use.
	        7. pgsn, names associated gsn.
	        8. slot, slot number for object loading.  Do not change these.
	        9. min_mana, minimum mana cost to cast.
	        10. beats, amount of lag in quarter seconds.
	        11. noun_damage, damage message in combat.
	        12. msg_off, message when affect wears off.
	        13. msg_obj, message when affect wears off objects.
	        14. remort_class, sets what class gets it as a remort spell.  Use -1 to make it inaccessible.
	        15. evocost_sec, cost for secondary or higher class to evolve to 2.  Use 0 for tertiary classes.
	        16. evocost_pri, cost for primary class to evolve to 3.  Use 0 for secondary classes.
	*/

	{
		"reserved",             { 999, 999, 999, 999, 999, 999, 999, 999 },
		{ 999, 999, 999, 999, 999, 999, 999, 999 },
		0,                      TAR_IGNORE,             POS_STANDING,
		&gsn_reserved,          SLOT(0),        0,      0,
		"",                     "",                     "",     -1,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"acid blast",           { 28, 33, 35, 32, 28, 33, 35, 32 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_acid_blast,       TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_acid_blast,        SLOT(70),       20,     12,
		"acid blast",           "!Acid Blast!",         "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},

	{
		"acid breath",          { 31, 32, 33, 34, 31, 32, 33, 34 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_acid_breath,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_acid_breath,       SLOT(200),      50,     12,
		"blast of acid",        "!Acid Breath!",        "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"acid rain",            { 51, 49, 54, 53, 52, 49, 48, 50 },
		{  1,  1,  2,  2,  1,  1,  1,  1 },
		spell_acid_rain,        TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_acid_rain,         SLOT(807),      30,     12,
		"acid rain",            "!Acid Rain!",          "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"age",                  { 53, 54, 65, 63, 51, 54, 59, 68 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_age,              TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_age,               SLOT(799),      35,     12,
		"spell",                "You feel younger.",    "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"animate skeleton",     { 32, 43, 70, 70, 25, 70, 70, 70 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_animate_skeleton, TAR_IGNORE,             POS_STANDING,
		&gsn_animate_skeleton,  SLOT(600),      50,     12,
		"",                     "!Animate Skeleton!",   "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"animate wraith",       { 55, 66, 75, 75, 52, 75, 75, 75 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_animate_wraith,   TAR_IGNORE,             POS_STANDING,
		&gsn_animate_wraith,    SLOT(602),      50,     12,
		"",                     "!Animate Wraith!",     "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"animate gargoyle",     { 70, 78, 80, 80, 63, 80, 80, 80 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_animate_gargoyle, TAR_IGNORE,             POS_STANDING,
		&gsn_animate_gargoyle,  SLOT(603),      50,     12,
		"",                     "!Animate Gargoyle!",   "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"animate zombie",       { 46, 54, 73, 73, 37, 73, 73, 73 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_animate_zombie,   TAR_IGNORE,             POS_STANDING,
		&gsn_animate_zombie,    SLOT(601),      50,     12,
		"",                     "!Animate Zombie!",     "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"armor",                {  5,  2, 10,  5,  5,  2, 10,  5 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_armor,            TAR_CHAR_DEFENSIVE,     POS_STANDING,
		&gsn_armor,             SLOT(1),        5,      12,
		"",                     "You feel less armored.",       "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"bless",                { 10,  2, 60,  8, 44,  4, 10,  8 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_bless,            TAR_OBJ_CHAR_DEF,       POS_STANDING,
		&gsn_bless,             SLOT(3),        5,      12,
		"",                     "You feel less righteous.",     "$p's holy aura fades.",        0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"blindness",            { 12,  8, 17, 15, 12,  8, 17, 15 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_blindness,        TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_blindness,         SLOT(4),        5,      12,
		"",                     "You can see again.",   "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"blizzard",             { 51, 49, 54, 53, 52, 49, 48, 50 },
		{  1,  1,  2,  2,  1,  1,  1,  1 },
		spell_blizzard,         TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_blizzard,          SLOT(808),      30,     12,
		"snow flurry",          "!Blizzard!",           "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"blood moon",           { 33, 38, 68, 68, 12, 75, 68, 68 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_blood_moon,       TAR_CHAR_DEFENSIVE,     POS_STANDING,
		&gsn_blood_moon,        SLOT(913),      5,      12,
		"",                     "You feel less bloodthirsty.",  "$p's evil aura fades.",        0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"blood blade",          { 30, 58, 60, 78, 35, 70, 63, 70 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_blood_blade,      TAR_OBJ_INV,            POS_STANDING,
		&gsn_blood_blade,       SLOT(802),      100,    12,
		"",                     "!Blood Blade!",        "$p is no longer a bloodthirsty weapon.",       0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"burning hands",        {  7, 11, 10,  9,  7, 11, 10,  9 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_burning_hands,    TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_burning_hands,     SLOT(5),        6,      12,
		"burning hands",        "!Burning Hands!",      "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"call lightning",       { 26, 16, 31, 22, 26, 18, 31, 22 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_call_lightning,   TAR_IGNORE,             POS_FIGHTING,
		&gsn_call_lightning,    SLOT(6),        10,     12,
		"lightning bolt",       "!Call Lightning!",     "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"calm",                 { 20, 16, 60, 20, 20, 16, 20, 20 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_calm,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_calm,              SLOT(509),      30,     12,
		"",                     "You have lost your peace of mind.",    "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"cancellation",         { 18, 26, 34, 34, 18, 26, 34, 34 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_cancellation,     TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		&gsn_cancellation,      SLOT(507),      20,     12,
		"",                     "!Cancellation!",       "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"cause light",          {  5,  6,  5,  5,  1,  6,  5,  5 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_cause_light,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_cause_light,       SLOT(62),       5,      12,
		"spell",                "!Cause Light!",        "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"cause serious",        { 10, 12, 13, 13,  7,  7, 13, 13 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_cause_serious,    TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_cause_serious,     SLOT(64),       7,      12,
		"spell",                "!Cause Serious!",      "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"cause critical",       { 21, 18, 23, 23, 13, 18, 23, 23 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_cause_critical,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_cause_critical,    SLOT(63),       10,     12,
		"spell",                "!Cause Critical!",     "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"chain lightning",      { 33, 35, 39, 36, 33, 35, 39, 36 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_chain_lightning,  TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_chain_lightning,   SLOT(500),      25,     12,
		"lightning",            "!Chain Lightning!",    "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"change sex",           { 60, 60, 60, 60, 60, 60, 60, 60 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_change_sex,       TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		&gsn_change_sex,        SLOT(82),       15,     12,
		"",                     "Your body feels familiar again.",      "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"channel",              { 63, 65, 77, 78, 65, 66, 76, 78 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_channel,          TAR_IGNORE,             POS_STANDING,
		&gsn_channel,           SLOT(93),       10,     12,
		"",                     "You feel revived.",    "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"charm person",         { 20, 26, 25, 60, 20, 23, 25, 60 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_charm_person,     TAR_CHAR_OFFENSIVE,     POS_STANDING,
		&gsn_charm_person,      SLOT(7),        5,      12,
		"",                     "You feel more self-confident.",        "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"chill touch",          {  4,  7,  6,  6,  4,  7,  6,  6 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_chill_touch,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_chill_touch,       SLOT(8),        5,      12,
		"chilling touch",       "You feel less cold.",  "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"colour spray",         { 16, 24, 22, 20, 16, 24, 22, 20 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_colour_spray,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_colour_spray,      SLOT(10),       11,     12,
		"colour spray",         "!Colour Spray!",       "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"continual light",      {  5,  4,  6,  9,  6,  4,  6,  9 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_continual_light,  TAR_IGNORE,             POS_STANDING,
		&gsn_continual_light,   SLOT(57),       7,      12,
		"",                     "!Continual Light!",    "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"control weather",      { 15, 19, 28, 22, 15, 19, 28, 22 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_control_weather,  TAR_IGNORE,             POS_STANDING,
		&gsn_control_weather,   SLOT(11),       25,     12,
		"",                     "!Control Weather!",    "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"create food",          { 10,  5, 11, 12, 10,  5, 11, 12 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_create_food,      TAR_IGNORE,             POS_STANDING,
		&gsn_create_food,       SLOT(12),       5,      12,
		"",                     "!Create Food!",        "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"create parchment",     { 16, 11, 60, 60, 16, 11, 60, 60 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_create_parchment, TAR_IGNORE,             POS_STANDING,
		&gsn_create_parchment,  SLOT(912),      30,     12,
		"",                     "!Create Parchment!",   "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"create rose",          { 16, 11, 10, 24, 16, 11, 10, 24 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_create_rose,      TAR_IGNORE,             POS_STANDING,
		&gsn_create_rose,       SLOT(511),      30,     12,
		"",                     "!Create Rose!",        "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"create sign",          { 25, 20, 23, 22, 26, 21, 22, 21 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_create_sign,      TAR_IGNORE,             POS_STANDING,
		&gsn_create_sign,       SLOT(169),      45,     12,
		"",                     "!Create Sign!",        "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"create spring",        { 14, 16, 23, 20, 14, 16, 23, 20 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_create_spring,    TAR_IGNORE,             POS_STANDING,
		&gsn_create_spring,     SLOT(80),       20,     12,
		"",                     "!Create Spring!",      "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"create vial",          { 16, 11, 60, 60, 16, 11, 60, 60 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_create_vial,      TAR_IGNORE,             POS_STANDING,
		&gsn_create_vial,       SLOT(911),      30,     12,
		"",                     "!Create Vial!",        "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"create water",         {  8,  3, 12, 11,  8,  3, 12, 11 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_create_water,     TAR_OBJ_INV,            POS_STANDING,
		&gsn_create_water,      SLOT(13),       5,      12,
		"",                     "!Create Water!",       "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"cure blindness",       { 10,  6, 60,  8, 27,  6, 60,  8 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_cure_blindness,   TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		&gsn_cure_blindness,    SLOT(14),       5,      12,
		"",                     "!Cure Blindness!",     "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"cure critical",        { 18, 13, 28, 19, 26, 13, 21, 19 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_cure_critical,    TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		&gsn_cure_critical,                     SLOT(15),       20,     12,
		"",                     "!Cure Critical!",      "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"cure disease",         { 19, 13, 60, 14, 34, 13, 60, 14 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_cure_disease,     TAR_CHAR_DEFENSIVE,     POS_STANDING,
		&gsn_cure_disease,      SLOT(501),      20,     12,
		"",                     "!Cure Disease!",       "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"cure light",           {  2,  1, 10,  3,  9,  1,  6,  3 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_cure_light,       TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		&gsn_cure_light,        SLOT(16),       10,     12,
		"",                     "!Cure Light!",         "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"cure poison",          { 21, 14, 51, 16, 38, 14, 48, 16 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_cure_poison,      TAR_CHAR_DEFENSIVE,     POS_STANDING,
		&gsn_cure_poison,       SLOT(43),       5,      12,
		"",                     "!Cure Poison!",        "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"cure serious",         {  9,  7, 15, 10, 15,  7, 12, 10 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_cure_serious,     TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		&gsn_cure_serious,      SLOT(61),       15,     12,
		"",                     "!Cure Serious!",       "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"curse",                { 18, 18, 26, 22, 17, 18, 26, 22 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_curse,            TAR_OBJ_CHAR_OFF,       POS_FIGHTING,
		&gsn_curse,             SLOT(17),       20,     12,
		"curse",                "The curse wears off.",         "$p is no longer impure.",      0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"darkness",             { 31, 39, 58, 68, 26, 53, 61, 57 },
		{  2,  2,  4,  4,  2,  3,  4,  4 },
		spell_darkness,         TAR_IGNORE,             POS_STANDING,
		&gsn_darkness,          SLOT(421),      60,     18,
		"",                     "",     "The room is no longer so dark.",       0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"dazzling light",       { 10, 18, 30, 30, 12, 18, 25, 30 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_dazzling_light,   TAR_OBJ_INV,            POS_STANDING,
		&gsn_dazzling_light,    SLOT(590),      50,     12,
		"",                     "!Dazzling Light!",     "$p's light seems less penetrating.",   0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"demonfire",            { 38, 34, 60, 45, 30, 34, 60, 45 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_demonfire,        TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_demonfire,         SLOT(505),      18,     12,
		"torments",             "!Demonfire!",          "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"detect evil",          { 11,  4, 12, 60, 11,  4, 12, 60 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_detect_evil,      TAR_CHAR_SELF,          POS_STANDING,
		&gsn_detect_evil,       SLOT(18),       5,      12,
		"",                     "The red in your vision disappears.",   "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"detect good",          { 11,  4, 12, 60, 11,  4, 12, 60 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_detect_good,      TAR_CHAR_SELF,          POS_STANDING,
		&gsn_detect_good,       SLOT(513),      5,      12,
		"",                     "The gold in your vision disappears.",  "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"detect hidden",        { 15, 11, 12, 60, 15, 11, 12, 60 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_detect_hidden,    TAR_CHAR_SELF,          POS_STANDING,
		&gsn_detect_hidden,     SLOT(44),       5,      12,
		"",                     "You feel less aware of your surroundings.",    "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"detect invis",         {  3,  8,  6, 60,  3,  8,  6, 60 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_detect_invis,     TAR_CHAR_SELF,          POS_STANDING,
		&gsn_detect_invis,      SLOT(19),       5,      12,
		"",                     "You no longer see the invisible.",     "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"detect magic",         {  2,  6,  5, 60,  2,  6,  5, 60 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_detect_magic,     TAR_CHAR_SELF,          POS_STANDING,
		&gsn_detect_magic,      SLOT(20),       5,      12,
		"",                     "The detect magic wears off.",  "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"detect poison",        { 15,  7,  9, 60, 15,  7,  9, 60 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_detect_poison,    TAR_OBJ_INV,            POS_STANDING,
		&gsn_detect_poison,     SLOT(21),       5,      12,
		"",                     "!Detect Poison!",      "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"dispel evil",          { 18, 15, 60, 23, 20, 15, 45, 24 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_dispel_evil,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_dispel_evil,       SLOT(22),       10,     12,
		"dispel evil",          "!Dispel Evil!",        "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"dispel good",          { 18, 15, 35, 24, 18, 15, 35, 24 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_dispel_good,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_dispel_good,       SLOT(512),      10,     12,
		"dispel good",          "!Dispel Good!",        "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"dispel magic",         { 16, 24, 30, 30, 16, 24, 30, 30 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_dispel_magic,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_dispel_magic,      SLOT(59),       15,     12,
		"",                     "!Dispel Magic!",       "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"divine healing",       { 55, 51, 62, 61, 75, 46, 62, 62 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_divine_healing,   TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		&gsn_divine_healing,    SLOT(599),      150,    12,
		"",                     "!Divine Healing!",     "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"divine regeneration",  { 87, 88, 88, 89, 87, 86, 88, 89 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_divine_regeneration, TAR_CHAR_DEFENSIVE,   POS_STANDING,
		&gsn_divine_regeneration, SLOT(531),     50,     12,
		"",                     "You no longer feel so vibrant.",       "",     0,
		{ 470, 220, 0, 0, 0, 420, 0, 0 },       { 0, 410, 0, 0, 0, 0, 0, 0 }
	},
	{
		"earthquake",           { 19, 10, 21, 14, 18, 10, 20, 14 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_earthquake,       TAR_IGNORE,             POS_FIGHTING,
		&gsn_earthquake,        SLOT(23),       8,      12,
		"earthquake",           "!Earthquake!",         "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"encampment",           { 26, 18, 32, 29,  6, 18, 32, 29 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_encampment,       TAR_IGNORE,             POS_STANDING,
		&gsn_encampment,        SLOT(555),      28,     12,
		"",                     "!Encampment!",         "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"enchant armor",        { 16, 40, 60, 60, 16, 40, 60, 60 },
		{  2,  2,  4,  4,  2,  2,  4,  4 },
		spell_enchant_armor,    TAR_OBJ_INV,            POS_STANDING,
		&gsn_enchant_armor,     SLOT(510),      100,    24,
		"",                     "!Enchant Armor!",      "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"enchant weapon",       { 17, 40, 60, 60, 17, 40, 60, 60 },
		{  2,  2,  4,  4,  2,  2,  4,  4 },
		spell_enchant_weapon,   TAR_OBJ_INV,            POS_STANDING,
		&gsn_enchant_weapon,    SLOT(24),       100,    24,
		"",                     "!Enchant Weapon!",     "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"energy drain",         { 20, 35, 29, 29, 9, 60, 24, 29 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_energy_drain,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_energy_drain,      SLOT(25),       40,     12,
		"energy drain",         "!Energy Drain!",       "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"faerie fire",          {  6,  3,  5,  8,  6,  3,  5,  8 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_faerie_fire,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_faerie_fire,       SLOT(72),       5,      12,
		"faerie fire",          "The pink aura around you fades away.", "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"faerie fog",           { 14, 21, 16, 24, 14, 21, 16, 24 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_faerie_fog,       TAR_IGNORE,             POS_STANDING,
		&gsn_faerie_fog,        SLOT(73),       12,     12,
		"faerie fog",           "!Faerie Fog!",         "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"farsight",             { 14, 16, 16, 60, 14, 16, 16, 60 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_farsight,         TAR_IGNORE,             POS_STANDING,
		&gsn_farsight,          SLOT(521),      36,     20,
		"farsight",             "!Farsight!",           "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"fear",                 { 17, 20, 24, 21, 12, 20, 23, 23 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_fear,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_fear,              SLOT(535),      20,     12,
		"fear",                 "You are no longer afraid.",    "$p is no longer afraid.",      0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"fire breath",          { 40, 45, 50, 51, 40, 45, 50, 51 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_fire_breath,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_fire_breath,       SLOT(201),      60,     12,
		"blast of flame",       "The smoke leaves your eyes.",  "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"fireball",             { 22, 32, 30, 26, 22, 32, 30, 26 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_fireball,         TAR_IGNORE,             POS_FIGHTING,
		&gsn_fireball,          SLOT(26),       10,     12,
		"fireball",             "!Fireball!",           "",     0,
		{ 220, 0, 0, 0, 380, 0, 0, 0 }, { 460, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"fireproof",            { 13, 12, 19, 18, 13, 12, 19, 18 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_fireproof,        TAR_OBJ_INV,            POS_STANDING,
		&gsn_fireproof,         SLOT(523),      10,     12,
		"",                     "",     "$p's protective aura fades.",  0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"firestorm",            { 78, 78, 80, 80, 78, 79, 80, 80 },
		{  2,  2,  2,  2,  2,  2,  2,  2 },
		spell_firestorm,        TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_firestorm,         SLOT(805),      35,     12,
		"firestorm",            "!Firestorm!",          "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"flame blade",          { 30, 58, 60, 78, 35, 54, 63, 70 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_flame_blade,      TAR_OBJ_INV,            POS_STANDING,
		&gsn_flame_blade,       SLOT(804),      100,    12,
		"",                     "!Flame Blade!",        "$p's flame diminishes.",       0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"flameshield",          { 50, 65, 65, 70, 60, 55, 65, 55 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_flameshield,      TAR_CHAR_SELF,          POS_STANDING,
		&gsn_flameshield,               SLOT(625),      45,     12,
		"flameshield",          "The circle of flames around your body dissipates.",
		"$p's flameshield dissipates.", 0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"flamestrike",          { 29, 20, 39, 27, 29, 20, 39, 27 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_flamestrike,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_flamestrike,       SLOT(65),       12,     12,
		"flamestrike",          "!Flamestrike!",        "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"fly",                  { 10, 18, 20, 22, 10, 18, 20, 22 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_fly,              TAR_CHAR_DEFENSIVE,     POS_STANDING,
		&gsn_fly,               SLOT(56),       10,     12,
		"",                     "You slowly float to the ground.",      "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"floating disc",        {  4, 10,  7, 16,  4, 10,  7, 16 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_floating_disc,    TAR_IGNORE,             POS_STANDING,
		&gsn_floating_disc,     SLOT(522),      40,     24,
		"",                     "!Floating Disc!",      "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"frenzy",               { 26, 24, 60, 26, 60, 24, 60, 26 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_frenzy,           TAR_CHAR_DEFENSIVE,     POS_STANDING,
		&gsn_frenzy,            SLOT(504),      30,     12,
		"",                     "Your rage ebbs.",      "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"frost blade",          { 30, 58, 60, 78, 35, 54, 63, 70 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_frost_blade,      TAR_OBJ_INV,            POS_STANDING,
		&gsn_frost_blade,       SLOT(801),      100,    12,
		"",                     "!Frost Blade!",        "$p is no longer so cold to the touch.",        0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"frost breath",         { 34, 36, 38, 40, 34, 36, 38, 40 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_frost_breath,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_frost_breath,      SLOT(202),      50,     12,
		"blast of frost",       "!Frost Breath!",       "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"gas breath",           { 29, 33, 37, 40, 29, 33, 37, 40 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_gas_breath,       TAR_IGNORE,             POS_FIGHTING,
		&gsn_gas_breath,        SLOT(203),      45,     12,
		"blast of gas",         "!Gas Breath!",         "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"gate",                 { 22, 17, 32, 28, 22, 17, 32, 28 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_gate,             TAR_IGNORE,             POS_STANDING,
		&gsn_gate,              SLOT(83),       80,     12,
		"",                     "!Gate!",               "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{   /* Spell for mega1.are from Glop/Erkenbrand. */
		"general purpose",      { -1, -1, -1, -1, -1, -1, -1, -1 },
		{ -1, -1, -1, -1, -1, -1, -1, -1 },
		spell_general_purpose,  TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_general_purpose,   SLOT(401),      5,      12,
		"general purpose ammo", "!General Purpose Ammo!",       "",     -1,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"giant strength",       { 11, 15, 22, 20, 11, 15, 22, 20 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_giant_strength,   TAR_CHAR_DEFENSIVE,     POS_STANDING,
		&gsn_giant_strength,    SLOT(39),       20,     12,
		"",                     "You feel weaker.",     "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"harm",                 { 30, 23, 30, 28, 30, 23, 30, 28 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_harm,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_harm,              SLOT(27),       20,     12,
		"harm spell",           "!Harm!",               "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"haste",                { 21, 30, 26, 29, 21, 30, 26, 29 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_haste,            TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		&gsn_haste,             SLOT(502),      30,     12,
		"",                     "You feel yourself slow down.", "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"heal",                 { 28, 21, 33, 30, 35, 21, 33, 30 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_heal,             TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		&gsn_heal,              SLOT(28),       50,     12,
		"",                     "!Heal!",               "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"heat metal",           { 25, 16, 60, 23, 20, 16, 41, 23 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_heat_metal,       TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_heat_metal,        SLOT(516),      25,     18,
		"spell",                "!Heat Metal!",         "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{   /* Spell for mega1.are from Glop/Erkenbrand. */
		"high explosive",       { -1, -1, -1, -1, -1, -1, -1, -1 },
		{ -1, -1, -1, -1, -1, -1, -1, -1 },
		spell_high_explosive,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_high_explosive,    SLOT(402),      5,      12,
		"high explosive ammo",  "!High Explosive Ammo!",        "",     -1,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"holy word",            { 60, 36, 60, 42, 60, 36, 60, 42 },
		{  2,  2,  4,  4,  2,  2,  4,  4 },
		spell_holy_word,        TAR_IGNORE,             POS_FIGHTING,
		&gsn_holy_word,         SLOT(506),      200,    24,
		"divine wrath",         "!Holy Word!",          "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"identify",             { 15, 16, 18, 58, 15, 16, 18, 58 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_identify,         TAR_OBJ_HERE,            POS_STANDING,
		&gsn_identify,          SLOT(53),       12,     12,
		"",                     "!Identify!",           "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"infravision",          {  9, 13, 10, 16,  9, 13, 10, 16 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_infravision,      TAR_CHAR_DEFENSIVE,     POS_STANDING,
		&gsn_night_vision,       SLOT(77),       5,      18,
		"",                     "You no longer see in the dark.",       "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"invisibility",         {  5, 12,  9, 60,  5, 12,  9, 60 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_invis,            TAR_OBJ_CHAR_DEF,       POS_STANDING,
		&gsn_invis,             SLOT(29),       5,      12,
		"",                     "You are no longer invisible.", "$p fades into view.",  0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"know alignment",       { 12,  9, 20, 59, 12,  9, 20, 59 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_know_alignment,   TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		&gsn_know_alignment,    SLOT(58),       9,      12,
		"",                     "!Know Alignment!",     "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"light of truth",       { 30, 20, 45, 60, 31, 19, 42, 60 },
		{  2,  1,  2,  3,  2,  1,  2,  3 },
		spell_light_of_truth,   TAR_OBJ_INV,            POS_STANDING,
		&gsn_light_of_truth,    SLOT(591),      100,    12,
		"",                     "!Light of Truth!",
		"$p no longer shows you the world in the harsh light of truth.",        0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"lightning bolt",       { 13, 20, 18, 16, 13, 20, 18, 16 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_lightning_bolt,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_lightning_bolt,    SLOT(30),       8,      12,
		"lightning bolt",       "!Lightning Bolt!",     "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"lightning breath",     { 37, 40, 43, 46, 37, 40, 43, 46 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_lightning_breath, TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_lightning_breath,  SLOT(204),      65,     12,
		"blast of lightning",   "!Lightning Breath!",   "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"locate life",          {  9, 15, 11, 45,  9, 15, 11, 44 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_locate_life,      TAR_IGNORE,             POS_STANDING,
		&gsn_locate_life,       SLOT(331),      20,     18,
		"",                     "!Locate Life!",        "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"locate object",        {  9, 15, 11, 40,  9, 15, 11, 40 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_locate_object,    TAR_IGNORE,             POS_STANDING,
		&gsn_locate_object,     SLOT(31),       20,     18,
		"",                     "!Locate Object!",      "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{ /*mod vegita*/
		"magic missile",        {  1,  3,  2,  2,  1,  3,  2,  2 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_magic_missile,    TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_magic_missile,     SLOT(32),       5,      12,
		"magic missile",        "!Magic Missile!",      "",     0,
		{ 220, 220, 0, 0, 220, 220, 0, 0 },     { 440, 0, 0, 0, 440, 0, 0, 0 }
	},
	{
		"mass healing",         { 38, 32, 60, 46, 60, 32, 60, 46 },
		{  2,  2,  4,  4,  2,  2,  4,  4 },
		spell_mass_healing,     TAR_IGNORE,             POS_STANDING,
		&gsn_mass_healing,      SLOT(508),      100,    36,
		"",                     "!Mass Healing!",       "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"mass invis",           { 22, 28, 31, 60, 22, 28, 31, 60 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_mass_invis,       TAR_IGNORE,             POS_STANDING,
		&gsn_mass_invis,        SLOT(69),       20,     24,
		"",                     "You are no longer invisible.", "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"nexus",                { 40, 35, 50, 45, 40, 35, 50, 45 },
		{  2,  2,  4,  4,  2,  2,  4,  4 },
		spell_nexus,            TAR_IGNORE,             POS_STANDING,
		&gsn_nexus,             SLOT(520),      150,    36,
		"",                     "!Nexus!",              "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"pass door",            { 24, 32, 25, 37, 24, 32, 25, 37 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_pass_door,        TAR_CHAR_DEFENSIVE,     POS_STANDING,
		&gsn_pass_door,         SLOT(74),       20,     12,
		"",                     "You feel solid again.",        "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"plague",               { 23, 17, 36, 26, 20, 17, 36, 26 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_plague,           TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_plague,            SLOT(503),      20,     12,
		"sickness",             "Your sores vanish.",   "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"poison",               { 17, 12, 15, 21,  7, 12, 15, 21 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_poison,           TAR_OBJ_CHAR_OFF,       POS_FIGHTING,
		&gsn_poison,            SLOT(33),       10,     12,
		"poison",               "You feel less sick.",  "The poison on $p dries up.",   0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"power word",           { 70, 80, 85, 88, 68, 82, 83, 89 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_power_word,       TAR_CHAR_OFFENSIVE,     POS_STANDING,
		&gsn_power_word,        SLOT(107),      50,     12,
		"power word",           "!Power Word!",         "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"polymorph",            { 82, 84, 88, 89, 83, 85, 87, 88 },
		{  2,  2,  2,  2,  2,  2,  2,  2 },
		spell_polymorph,        TAR_IGNORE,             POS_STANDING,
		&gsn_polymorph,         SLOT(505),      100,    18,
		"",                     "!Polymorph!",          "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"portal",               { 35, 30, 45, 40, 35, 30, 45, 40 },
		{  2,  2,  4,  4,  2,  2,  4,  4 },
		spell_portal,           TAR_IGNORE,             POS_STANDING,
		&gsn_portal,            SLOT(519),      100,    24,
		"",                     "!Portal!",             "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"protect container",    { 53, 51, 55, 57, 53, 51, 55, 57 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_protect_container, TAR_OBJ_INV,            POS_STANDING,
		&gsn_protect_container, SLOT(527),      50,     18,
		"",                     "!Protect Container!",  "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"protection evil",      { 12,  9, 17, 11, 12,  9, 17, 11 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_protection_evil,  TAR_CHAR_SELF,          POS_STANDING,
		&gsn_protection_evil,   SLOT(34),       5,      12,
		"",                     "You feel less protected from evil.",   "",     0,
		{ 0, 220, 0, 0, 0, 260, 0, 0 },     { 0, 360, 0, 0, 0, 0, 0, 0 }
	},
	{
		"protection good",      { 12,  9, 17, 11, 12,  9, 17, 11 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_protection_good,  TAR_CHAR_SELF,          POS_STANDING,
		&gsn_protection_good,   SLOT(514),      5,      12,
		"",                     "You feel less protected from good.",   "",     0,
		{ 0, 220, 0, 0, 240, 260, 0, 0 },     { 0, 360, 0, 0, 0, 0, 0, 0 }
	},
	{
		"ray of truth",         { 40, 35, 60, 47, 33, 35, 60, 47 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_ray_of_truth,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_ray_of_truth,      SLOT(518),      18,     12,
		"ray of truth",         "!Ray of Truth!",       "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"recharge",             {  9, 20, 60, 60,  9, 25, 60, 60 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_recharge,         TAR_OBJ_INV,            POS_STANDING,
		&gsn_recharge,          SLOT(517),      60,     24,
		"",                     "!Recharge!",           "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"refresh",              {  8,  5, 12,  9,  8,  5, 12,  9 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_refresh,          TAR_CHAR_DEFENSIVE,     POS_STANDING,
		&gsn_refresh,           SLOT(81),       12,     18,
		"refresh",              "!Refresh!",            "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"resurrect",            { 83, 82, 90, 90, 70, 81, 90, 90 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_resurrect,        TAR_IGNORE,             POS_STANDING,
		&gsn_resurrect,         SLOT(604),      100,    15,
		"",                     "!Resurrect!",          "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"regeneration",         { 20, 25, 35, 40, 20, 25, 35, 40 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_regeneration,     TAR_CHAR_DEFENSIVE,     POS_STANDING,
		&gsn_regeneration,      SLOT(524),      20,     12,
		"",                     "You no longer feel so vibrant.",       "",     0,
		{ 320, 220, 370, 400, 330, 310, 360, 280 },     { 0, 340, 0, 0, 0, 0, 0, 400 }
	},
	{
		"remove alignment",     { 40, 23, 60, 60, 40, 23, 60, 60 },
		{  4,  2,  4,  4,  4,  2,  4,  4 },
		spell_remove_alignment, TAR_OBJ_INV,            POS_STANDING,
		&gsn_remove_alignment,  SLOT(525),      75,     18,
		"",                     "!Remove Alignment!",   "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"remove invisibility",  { 30, 23, 60, 60, 30, 23, 60, 60 },
		{  4,  2,  4,  4,  4,  2,  4,  4 },
		spell_remove_invis,     TAR_OBJ_INV,            POS_STANDING,
		&gsn_remove_invis,      SLOT(526),      40,     18,
		"",                     "!Remove Invisibility!",        "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"remove curse",         { 20, 18, 60, 22, 25, 18, 60, 22 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_remove_curse,     TAR_OBJ_CHAR_DEF,       POS_STANDING,
		&gsn_remove_curse,      SLOT(35),       5,      12,
		"",                     "!Remove Curse!",       "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"sanctuary",            { 25, 20, 42, 30, 32, 20, 42, 30 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_sanctuary,        TAR_CHAR_DEFENSIVE,     POS_STANDING,
		&gsn_sanctuary,         SLOT(36),       75,     12,
		"sanctuary",            "The white aura around your body fades.",       "",     0,
		{ 400, 250, 0, 0, 0, 400, 0, 0 },       { 0, 500, 0, 0, 0, 0, 0, 0 }
	},
	{
		"scry",        { 30, 50, 60, 70, 40, 50, 50, 30 },
		{ 1, 1, 2, 2, 1, 2, 1, 1},
		spell_scry,  TAR_IGNORE,  POS_RESTING,
		&gsn_scry, SLOT(0), 30, 20,
		"", "", "", 0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0}
	},
	{
		"shield",               { 20, 35, 35, 40, 20, 35, 35, 40 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_shield,           TAR_CHAR_DEFENSIVE,     POS_STANDING,
		&gsn_shield,            SLOT(67),       12,     12,
		"",                     "Your force shield shimmers then fades away.",  "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"shock blade",          { 30, 58, 60, 78, 35, 54, 63, 70 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_shock_blade,      TAR_OBJ_INV,            POS_STANDING,
		&gsn_shock_blade,       SLOT(803),      100,    12,
		"",                     "!Flame Blade!",        "$p's energy fizzles out.",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"shocking grasp",       { 10, 15, 14, 13, 10, 15, 14, 13 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_shocking_grasp,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_shocking_grasp,    SLOT(53),       7,      12,
		"shocking grasp",       "!Shocking Grasp!",     "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"shrink",               { 22, 35, 60, 60, 22, 40, 60, 60 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_shrink,           TAR_OBJ_INV,            POS_STANDING,
		&gsn_shrink,            SLOT(515),      100,    12,
		"",                     "!Shrink!",             "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"sleep",                { 10, 31, 11, 60, 10, 31, 11, 60 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_sleep,            TAR_CHAR_OFFENSIVE,     POS_STANDING,
		&gsn_sleep,             SLOT(38),       15,     12,
		"",                     "You feel less tired.", "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"slow",                 { 23, 30, 29, 32, 23, 30, 29, 32 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_slow,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_slow,              SLOT(515),      30,     12,
		"",                     "You feel yourself speed up.",  "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"smokescreen",          { 21, 28, 23, 33, 21, 28, 23, 33 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_smokescreen,      TAR_IGNORE,             POS_STANDING,
		&gsn_smokescreen,       SLOT(531),      25,     12,
		"",                     "Smoke no longer clouds your vision.",  "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"starve",               { 30, 25, 40, 45, 22, 52, 40, 26 },
		{ 1, 1, 2, 2, 1, 2, 2, 1 },
		spell_starve,           TAR_CHAR_OFFENSIVE,         POS_STANDING,
		&gsn_starve,            SLOT(0),        10,    8,
		"",                     "", "", 0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"steel mist",           { 17, 12, 20, 15, 17, 12, 20, 15 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_steel_mist,       TAR_CHAR_DEFENSIVE,     POS_STANDING,
		&gsn_steel_mist,        SLOT(622),      35,     12,
		"",                     "The steel mist fades from your armor.",        "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"stone skin",           { 25, 40, 40, 45, 25, 40, 40, 45 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_stone_skin,       TAR_CHAR_SELF,          POS_STANDING,
		&gsn_stone_skin,        SLOT(66),       12,     12,
		"",                     "Your skin feels soft again.",  "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"summon",               { 23, 15, 29, 22, 23, 15, 29, 22 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_summon,           TAR_IGNORE,             POS_STANDING,
		&gsn_summon,            SLOT(40),       50,     12,
		"",                     "!Summon!",     "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"summon object",        { 74, 72, 89, 82, 74, 78, 89, 82 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_summon_object,    TAR_IGNORE,             POS_STANDING,
		&gsn_summon_object,     SLOT(95),       65,     12,
		"",                     "!Summon Object!",      "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"sunray",               { 79, 70, 82, 84, 79, 75, 80, 83 },
		{  2,  2,  2,  2,  2,  2 , 2,  2 },
		spell_sunray,           TAR_CHAR_OFFENSIVE,    POS_FIGHTING,
		&gsn_sunray,            SLOT(367),  40, 12,
		"sunray",               "!Sunray!",             "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },    { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"talon",                { 53, 51, 55, 57, 53, 51, 55, 57 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_talon,            TAR_CHAR_DEFENSIVE,     POS_STANDING,
		&gsn_talon,             SLOT(528),      25,     12,
		"",                     "You no longer hold your weapon so tightly.",   "",     0,
		{ 250, 400, 0, 0, 400, 0, 0, 0 },       { 500, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"teleport",             { 13, 22, 25, 36, 13, 22, 25, 36 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_teleport,         TAR_CHAR_SELF,          POS_FIGHTING,
		&gsn_teleport,          SLOT(2),        35,     12,
		"",                     "!Teleport!",           "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"teleport object",      { 52, 53, 55, 56, 52, 53, 55, 56 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_teleport_object,  TAR_IGNORE,             POS_STANDING,
		&gsn_teleport_object,   SLOT(530),      25,     18,
		"",                     "!Teleport Object!",    "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"undo spell",           { 38, 40, 45, 45, 38, 38, 45, 45 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_undo_spell,       TAR_IGNORE,             POS_FIGHTING,
		&gsn_undo_spell,        SLOT(529),      15,     18,
		"",                     "!Undo Spell!",         "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"ventriloquate",        {  1, 60,  2, 60,  1, 60,  2, 60 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_ventriloquate,    TAR_IGNORE,             POS_STANDING,
		&gsn_ventriloquate,     SLOT(41),       5,      12,
		"",                     "!Ventriloquate!",      "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"vision",               { 29, 37, 62, 58, 29, 37, 62, 48 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_vision,           TAR_IGNORE,             POS_FIGHTING,
		&gsn_vision,            SLOT(800),      80,     12,
		"",                     "!Vision!",             "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"weaken",               { 11, 14, 16, 17, 11, 14, 16, 17 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_weaken,           TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_weaken,            SLOT(68),       20,     12,
		"spell",                "You feel stronger.",   "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"word of recall",       { 32, 28, 40, 30, 32, 28, 40, 30 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_word_of_recall,   TAR_CHAR_SELF,          POS_RESTING,
		&gsn_word_of_recall,    SLOT(42),       5,      12,
		"",                     "!Word of Recall!",     "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"wrath",                { 85, 86, 87, 88, 85, 86, 87, 88 },
		{  2,  2,  2,  2,  2,  2,  2,  2 },
		spell_wrath,            TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_wrath,             SLOT(806),      30,     12,
		"wrath",                "!Wrath!",              "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},

	/* combat and weapons skills */

	{
		"axe",                  {  1,  1,  1,  1,  1,  1,  1,  1 },
		{  6,  6,  5,  4,  6,  5,  5,  4 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_axe,               SLOT(0),        0,      0,
		"",                     "!Axe!",                "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"dagger",               {  1,  1,  1,  1,  1,  1,  1,  1 },
		{  2,  3,  2,  2,  2,  3,  2,  2 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_dagger,            SLOT(0),        0,      0,
		"",                     "!Dagger!",             "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"flail",                {  1,  1,  1,  1,  1,  1,  1,  1 },
		{  6,  3,  6,  4,  6,  4,  6,  4 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_flail,             SLOT(0),        0,      0,
		"",                     "!Flail!",              "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"mace",                 {  1,  1,  1,  1,  1,  1,  1,  1 },
		{  5,  2,  3,  3,  5,  2,  3,  3 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_mace,              SLOT(0),        0,      0,
		"",                     "!Mace!",               "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"polearm",              {  1,  1,  1,  1,  1,  1,  1,  1 },
		{  6,  6,  6,  4,  6,  5,  6,  4 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_polearm,           SLOT(0),        0,      0,
		"",                     "!Polearm!",            "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"spear",                {  1,  1,  1,  1,  1,  1,  1,  1 },
		{  4,  4,  4,  3,  4,  4,  4,  3 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_spear,             SLOT(0),        0,      0,
		"",                     "!Spear!",              "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"sword",                {  1,  1,  1,  1,  1,  1,  1,  1 },
		{  5,  6,  3,  2,  5,  2,  3,  2 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_sword,             SLOT(0),        0,      0,
		"",                     "!Sword!",              "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"whip",                 {  1,  1,  1,  1,  1,  1,  1,  1 },
		{  6,  5,  5,  4,  6,  5,  5,  4 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_whip,              SLOT(0),        0,      0,
		"",                     "!Whip!",               "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"archery",                     { 61, 41, 35, 5, 61, 25, 1, 1 },
		{ 6, 5, 5, 4, 6, 5, 5, 4 },
		spell_null,                TAR_IGNORE,            POS_STANDING,
		&gsn_archery,                  SLOT(0),       0,      0,
		"",                        "!Bow!",               "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },       { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"shield block",         {  1,  1,  1,  1,  1,  1,  1,  1 },
		{  6,  4,  6,  2,  6,  3,  6,  2 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_shield_block,      SLOT(0),        0,      0,
		"",                     "!Shield!",             "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"brew",                 {  1,  1, 60, 60,  1, 40, 60, 60 },
		{  3,  3, 15, 15,  3, 10, 15, 15 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_brew,              SLOT(0),        15,     24,
		"",                     "!Brew!",               "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"scribe",               {  1,  1, 60, 60,  1, 40, 60, 60 },
		{  3,  3, 15, 15,  3, 10, 15, 15 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_scribe,            SLOT(0),        15,     24,
		"",                     "!Scribe!",             "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"backstab",             { 60, 60,  1, 63, 56, 70,  5, 63 },
		{ 22, 24,  5, 20, 22, 25,  5, 20 },
		spell_null,             TAR_CHAR_OFFENSIVE,             POS_STANDING,
		&gsn_backstab,          SLOT(0),        30,     24,
		"backstab",             "!Backstab!",           "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"bash",                 { 65, 58, 60,  1, 65, 15, 60,  1 },
		{  8,  6,  5,  4,  8,  5,  5,  4 },
		spell_null,             TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
		&gsn_bash,              SLOT(0),        15,     24,
		"bash",                 "!Bash!",               "",     0,
		{ 0, 0, 0, 200, 0, 0, 0, 0 },   { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"berserk",              { 60, 60, 60, 18, 60, 27, 60, 18 },
		{ 14, 13, 10,  5, 14,  8, 10,  5 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_berserk,           SLOT(0),        30,     24,
		"",                     "You feel your pulse slow down.",       "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"circle",               { 66, 66, 31, 66, 66, 68, 40, 66 },
		{ 22, 24,  6, 20, 22, 24, 10, 20 },
		spell_null,             TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
		&gsn_circle,            SLOT(0),        20,     36,
		"circle",               "!Circle!",             "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"crush",                { -1, -1, -1, -1, -1, -1, -1, -1 },
		{ -1, -1, -1, -1, -1, -1, -1, -1 },
		spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_crush,             SLOT(0),        15,     12,
		"crushing blow",        "!Crushing Blow!",      "",     -1,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"dirt kicking",         { 60, 60,  3,  3, 60, 14,  3,  3 },
		{  6,  6,  4,  4,  6,  5,  4,  4 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_dirt_kicking,      SLOT(0),        10,     24,
		"kicked dirt",          "You rub the dirt out of your eyes.",   "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"disarm",               { 60, 60, 12, 11, 60, 27, 12, 11 },
		{  8,  8,  6,  4,  8,  7,  6,  4 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_disarm,            SLOT(0),        15,     24,
		"",                     "!Disarm!",             "",     0,
		{0, 0, 500, 350, 0, 500, 0, 500 },      { 0, 0, 0, 700, 0, 0, 0, 0 }
	},
	{
		"dodge",                { 20, 22,  1, 13, 20, 13,  1, 13 },
		{  8,  8,  4,  6,  8,  6,  4,  6 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_dodge,             SLOT(0),        0,      0,
		"",                     "!Dodge!",              "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"enhanced damage",      { 45, 30, 25,  1, 45, 15, 25,  1 },
		{ 20, 20,  5,  3, 20,  4,  5,  3 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_enhanced_damage,   SLOT(0),        0,      0,
		"",                     "!Enhanced Damage!",    "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"envenom",              { 55, 55, 10, 60, 50, 60, 10, 60 },
		{ 10, 10,  4,  8, 10, 12,  4,  8 },
		spell_null,             TAR_IGNORE,             POS_RESTING,
		&gsn_envenom,           SLOT(0),        10,     36,
		"",                     "!Envenom!",            "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"hand to hand",         { 25, 10, 15,  6, 25,  6, 15,  6 },
		{  8,  5,  6,  4,  8,  5,  6,  4 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_hand_to_hand,      SLOT(0),        0,      0,
		"",                     "!Hand to Hand!",       "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"kick",                 { 48, 12, 14,  8, 48, 10, 14,  8 },
		{  8,  4,  6,  3,  8,  4,  6,  3 },
		spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_kick,              SLOT(0),        15,     12,
		"kick",                 "!Kick!",               "",     0,
		{ 0, 0, 300, 250, 0, 300, 0, 250 },     { 0, 0, 0, 425, 0, 0, 0, 425 }
	},
	/*wchange added for evo 2+ kick's second hit*/
	{
		"roundhouse",                 { 0, 0, 0,  0, 0, 0, 0,  0 },
		{  0,  0,  0,  0,  0,  0,  0,  0 },
		spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_roundhouse,         SLOT(0),       15,     12,
		"roundhouse",                 "!Roundhouse!",               "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	/* wchange evo 3 kick's foot sweep*/
	{
		"footsweep",                 { 0, 0, 0,  0, 0, 0, 0,  0 },
		{  0,  0,  0,  0,  0,  0,  0,  0 },
		spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_footsweep,         SLOT(0),       15,     12,
		"footsweep",                 "!FootSweep!",               "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},

	{
		"parry",                { 22, 20, 13,  1, 22, 10, 13,  1 },
		{  8,  8,  6,  4,  8,  5,  6,  4 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_parry,             SLOT(0),        0,      0,
		"",                     "!Parry!",              "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"rescue",               { 60, 60, 60,  1, 60,  1, 60,  1 },
		{  8,  7,  6,  4,  8,  2,  6,  4 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_rescue,            SLOT(0),        5,      12,
		"",                     "!Rescue!",             "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"trip",                 { 60, 60,  1, 15, 60, 60,  1, 15 },
		{ 10, 10,  4,  8, 10, 10,  4,  8 },
		spell_null,             TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
		&gsn_trip,              SLOT(0),        15,     24,
		"trip",                 "!Trip!",               "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"second attack",        { 30, 24, 12,  5, 30, 12, 12,  5 },
		{ 10,  8,  5,  3, 10,  5,  5,  3 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_second_attack,     SLOT(0),        0,      0,
		"",                     "!Second Attack!",      "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"third attack",         { 60, 60, 24, 12, 60, 25, 24, 12 },
		{ 14, 12, 10,  4, 14,  8, 10,  4 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_third_attack,      SLOT(0),        0,      0,
		"",                     "!Third Attack!",       "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"dual wield",           { 53, 53, 24, 12, 53, 14,  1, 12 },
		{ 15, 13, 11,  5, 15,  6, 11,  5 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_dual_wield,        SLOT(0),        0,      0,
		"",                     "!Dual Wield!",         "",     0,
		{ 0, 0, 550, 470, 0, 510, 280, 300 },   { 0, 0, 0, 0, 0, 0, 550, 600 }
	},
	{
		"hunt",                 { 53, 53, 15, 20, 53, 50, 17,  3 },
		{  8,  8,  4,  4,  8,  8,  4,  4 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_hunt,              SLOT(0),        20,     12,
		"",                     "!Hunt!",               "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"unarmed",    { 45, 20, 18, 2, 45, 6, 8, 18 },
		{ 2, 2, 1, 1, 2, 1, 1, 1 },
		spell_null,  TAR_IGNORE, POS_FIGHTING,
		&gsn_unarmed,  SLOT(0),   0, 0,
		"smash",   "!Unarmed!", "", 0,
		{ 0, 0, 0, 0, 0, 0, 0, 0},   { 0, 0, 0, 0, 0, 0, 0, 0 }
	},

	/* non-combat skills */

	{
		"swimming",             {  8,  8,  8,  8,  8,  8,  8,  8 },
		{  1,  1,  2,  2,  1,  1,  2,  2 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_swimming,          SLOT(0),        0,      0,
		"swimming",             "!Swimming!",           "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"fast healing",         { 15,  9, 16,  6, 15,  4, 16,  6 },
		{  8,  5,  6,  4,  8,  3,  6,  4 },
		spell_null,             TAR_IGNORE,             POS_SLEEPING,
		&gsn_fast_healing,      SLOT(0),        0,      0,
		"",                     "!Fast Healing!",       "",     0,
		{ 0, 0, 520, 280, 0, 500, 510, 480 },   { 0, 0, 0, 560, 0, 0, 0, 0 }
	},
	{
		"firebuilding",         {  1,  1,  1,  1,  1,  1,  1,  1 },
		{  2,  2,  1,  1,  2,  1,  1,  1 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_firebuilding,      SLOT(0),        5,      24,
		"",                     "!Firebuilding!",       "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"forge",                { 20, 20, 15, 16, 26, 16, 10, 15 },
		{ 16, 12,  2,  3, 16,  3,  1,  3 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_forge,             SLOT(0),        20,     24,
		"",                     "!Forge!",              "",     0,
		{ 0, 0, 0, 300, 0, 0, 300, 300 },       { 0, 0, 0, 400, 0, 0, 0, 0 }
	},
	{
		"repair",               { 25, 30,  5, 10, 35, 20,  5, 12 },
		{  2,  2,  1,  1,  2,  2,  1,  1 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_repair,            SLOT(0),        15,     24,
		"",                     "!Repair!",             "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"rotate",               {  1,  1,  1,  1,  1,  1,  1,  1 },
		{  2,  2,  1,  1,  2,  1,  1,  1 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_rotate,            SLOT(0),        5,      24,
		"",                     "!Rotate!",             "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"languages",            {  1,  1,  1,  1,  1,  1,  1,  1 },
		{  1,  1,  2,  2,  1,  2,  2,  2 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_languages,         SLOT(0),        0,      24,
		"",                     "!Languages!",          "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"haggle",               {  7, 18,  1, 14,  7, 18,  1, 14 },
		{  5,  8,  3,  6,  5,  8,  3,  6 },
		spell_null,             TAR_IGNORE,             POS_RESTING,
		&gsn_haggle,            SLOT(0),        0,      0,
		"",                     "!Haggle!",             "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"hide",                 { 60, 60,  1, 12, 60, 60,  1, 12 },
		{ 10,  8,  4,  6, 10,  8,  4,  6 },
		spell_null,             TAR_IGNORE,             POS_RESTING,
		&gsn_hide,              SLOT(0),        15,     12,
		"",                     "You are no longer hidden.",    "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"lore",                 { 10, 10,  6, 20, 10, 20,  6, 20 },
		{  6,  6,  4,  8,  6,  8,  4,  8 },
		spell_null,             TAR_IGNORE,             POS_RESTING,
		&gsn_lore,              SLOT(0),        5,      36,
		"",                     "!Lore!",               "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"meditation",           {  6,  6, 15, 15,  6,  6, 15, 15 },
		{  5,  5,  8,  8,  5,  5,  8,  8 },
		spell_null,             TAR_IGNORE,             POS_SLEEPING,
		&gsn_meditation,        SLOT(0),        0,      0,
		"",                     "Meditation",           "",     0,
		{ 440, 300, 0, 0, 460, 480, 0, 0 },     { 0, 600, 0, 0, 0, 0, 0, 0 }
	},
	{
		"peek",                 {  8, 21,  1, 14,  8, 21,  1, 14 },
		{  8,  7,  3,  6,  8,  7,  3,  6 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_peek,              SLOT(0),        5,      0,
		"",                     "!Peek!",               "",     0,
		{ 0, 0, 320, 0, 0, 0, 480, 0 }, { 0, 0, 580, 0, 0, 0, 0, 0 }
	},
	{
		"pick lock",            { 25, 25,  7, 25, 25, 30,  7, 25 },
		{  8,  8,  4,  8,  8,  9,  4,  8 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_pick_lock,         SLOT(0),        15,     12,
		"",                     "!Pick Lock!",          "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"scan",                 {  1,  1,  1,  1,  1,  1,  1,  1 },
		{  1,  1,  1,  1,  1,  1,  1,  1 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_scan,              SLOT(0),        0,      12,
		"",                     "!Scan!",               "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"sneak",                { 60, 60,  4, 10, 60, 60,  4, 10 },
		{ 10,  8,  4,  6, 10,  8,  4,  6 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_sneak,             SLOT(0),        15,     12,
		"",                     "You no longer feel stealthy.", "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"steal",                { 60, 60,  5, 60, 60, 60,  5, 60 },
		{ 10,  9,  4,  8, 10, 15,  4,  8 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_steal,             SLOT(0),        10,     24,
		"",                     "!Steal!",              "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"sing",                 { 60, 60, 60, 60, 60, 60, 12, 60 },
		{  6,  6,  4,  6,  7,  6,  2,  5 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_sing,              SLOT(0),        15,     24,
		"",                     "!Sing!",               "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"scrolls",              {  1,  1,  1,  1,  1,  1,  1,  1 },
		{  2,  3,  5,  8,  2,  4,  5,  8 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_scrolls,           SLOT(0),        10,     18,
		"",                     "!Scrolls!",            "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"spousegate",           { 19, 16, 20, 18, 25, 13, 16, 14 },
		{ 2, 2, 3, 3, 4, 2, 2, 1 },
		spell_null,             TAR_IGNORE, POS_STANDING,
		&gsn_spousegate,        SLOT(0),         25, 12,
		"",                     "!SpouseGate!",        "", 0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"staves",               {  1,  1,  1,  1,  1,  1,  1,  1 },
		{  2,  3,  5,  8,  2,  5,  5,  8 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_staves,            SLOT(0),        10,     18,
		"",                     "!Staves!",             "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"wands",                {  1,  1,  1,  1,  1,  1,  1,  1 },
		{  2,  3,  5,  8,  2,  5,  5,  8 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_wands,             SLOT(0),        10,     18,
		"",                     "!Wands!",              "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"recall",               {  1,  1,  1,  1,  1,  1,  1,  1 },
		{  2,  2,  2,  2,  2,  2,  2,  2 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_recall,            SLOT(0),        0,      12,
		"",                     "!Recall!",             "",     0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"lay",                  { 92, 92, 92, 92, 92, 10, 92, 92 },
		{ 2, 1, 2, 2, 2, 1, 2, 2},
		spell_null,             TAR_IGNORE,             POS_RESTING,
		&gsn_lay_on_hands,      SLOT(0),        0,      24,
		"",                     "",                             "", 0,
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"familiar",        { 25, 35, 45, 45, 35, 35, 35, 25 },
		{ 1, 1, 1, 1, 1, 1, 1, 1},
		spell_null, TAR_IGNORE, POS_RESTING,
		&gsn_familiar, SLOT(0), 0, 20,
		"", "", "", 0,
		{0, 0, 0, 0, 0, 0, 0, 0}, { 0, 0, 0, 0, 0, 0, 0, 0}
	},
	{
		"die hard",        { 22, 16, 16, 10, 21, 12, 16, 14 },
		{ 2, 1, 2, 1, 2, 1, 2, 1 },
		spell_null, TAR_IGNORE, POS_DEAD,
		&gsn_die_hard, SLOT(0), 0, 4,
		"", "", "", 0,
		{0, 0, 0, 0, 0, 0, 0, 0}, { 0, 0, 0, 0, 0, 0, 0, 0}
	},

	/* Remort spells and skills, originally by Elrac */
	/* Subdivided by Class */
	/* Everything has slot numbers here, it's more as an ID for saving than for area files */

	/* Mages */

	{
		"sheen",                { 10, 30, 50, 50, 20, 40, 50, 50 },
		{ 40, 55, 70, 70, 50, 60, 70, 70 },
		spell_sheen,            TAR_CHAR_SELF,          POS_STANDING,
		&gsn_sheen,             SLOT(1000),     80,     12,
		"sheen",                "Your armor loses its sheen.",          "",     1,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"focus",                { 40, 60, 80, 80, 50, 80, 80, 80 },
		{ 50, 70, 80, 80, 60, 80, 80, 80 },
		spell_focus,            TAR_CHAR_SELF,          POS_STANDING,
		&gsn_focus,             SLOT(1001),    200,     12,
		"focus",                "Your spells lose their focus.",        "",     1,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"paralyze",             { 30, 60, 70, 70, 50, 70, 70, 70 },
		{ 50, 80, 100, 100, 70, 100, 100, 100 },
		spell_paralyze,         TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_paralyze,          SLOT(1002),     50,     12,
		"paralyze",             "You can feel your limbs again!",       "",     1,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	/*    {
	        "ironskin",             { 20, 20, 60, 60, 20, 20, 60, 60 },
	                                { 30, 30, 30, 30, 30, 30, 30, 30 },
	        spell_ironskin,         TAR_CHAR_SELF,          POS_STANDING,
	        &gsn_ironskin,          SLOT(1003),    100,     12,
	        "iron skin",            "Your skin softens considerably.",
	        "", 1, { 0, 0, 0 }
	    }, */

	/* Clerics */

	{
		"barrier",              { 55, 40, 70, 70, 60, 65, 70, 70 },
		{ 60, 50, 90, 90, 70, 80, 90, 90 },
		spell_barrier,          TAR_CHAR_SELF,          POS_STANDING,
		&gsn_barrier,           SLOT(1100),    150,     12,
		"barrier",              "The barrier around you crumbles.",     "",     2,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"dazzle",               { 30, 10, 50, 50, 40, 40, 50, 50 },
		{ 60, 40, 80, 80, 70, 70, 80, 80 },
		spell_dazzle,           TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
		&gsn_dazzle,            SLOT(1101),     50,     12,
		"dazzle",               "The spots in your vision fade.",       "",     2,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"full heal",            { 55, 30, 70, 70, 70, 50, 70, 70 },
		{ 80, 60, 100, 100, 100, 70, 100, 100 },
		spell_full_heal,        TAR_CHAR_DEFENSIVE,     POS_STANDING,
		&gsn_full_heal,         SLOT(1102),    100,     12,
		"full heal",            "!Full Heal!",          "",     2,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},

	/* Thieves */

	{
		"midnight",             { 50, 50, 25, 50, 50, 50, 40, 40 },
		{ 80, 80, 50, 80, 80, 80, 70, 80 },
		spell_midnight,         TAR_CHAR_SELF,     POS_STANDING,
		&gsn_midnight,          SLOT(1200),        100,     12,
		"midnight",             "You step out of the shadows.",         "",     3,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"shadow form",          { 70, 80, 50, 80, 65, 80, 60, 80 },
		{ 80, 90, 60, 90, 75, 90, 70, 90 },
		spell_null,             TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
		&gsn_shadow_form,       SLOT(1201),        50,     36,
		"shadow form",          "!Shadow Form!",        "",     3,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"hone",                 { 35, 40, 10, 25, 35, 25, 25, 25 },
		{ 65, 70, 40, 55, 65, 55, 55, 55 },
		spell_null,             TAR_IGNORE,             POS_STANDING,
		&gsn_hone,              SLOT(1202),        40,     24,
		"",                     "!Hone!",               "",     3,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},

	/* Warriors */

	{
		"riposte",              { 70, 65, 55, 25, 70, 40, 55, 40 },
		{ 90, 90, 80, 50, 90, 70, 80, 70 },
		spell_null,             TAR_IGNORE,          POS_STANDING,
		&gsn_riposte,           SLOT(1300),        0,      0,
		"riposte",              "!Riposte!",            "",     4,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"fourth attack",        { 80, 80, 75, 50, 80, 70, 75, 70 },
		{ 100, 100, 90, 60, 100, 80, 90, 80 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_fourth_attack,     SLOT(1301),        0,      0,
		"",                     "!Fourth Attack!",      "",     4,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"rage",                 { 70, 70, 60, 40, 70, 55, 60, 55 },
		{ 80, 80, 70, 50, 80, 60, 70, 60 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_rage,              SLOT(1302),        40,     36,
		"rage",                 "!RAGE!",               "",     4,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"blind fight",           { 70, 61, 55, 28, 85, 36, 55, 60 },
		{ 60, 60, 50, 30, 60, 40, 40, 40 },
		spell_null,              TAR_IGNORE,   POS_STANDING,
		&gsn_blind_fight,        SLOT(1303),    0, 0,
		"",                      "!Blind Fight!", "", 4,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },   { 0, 0, 0, 0, 0, 0, 0, 0 }
	},

	/* Necromancers */

	{
		"sap",                  { 65, 75, 80, 80, 50, 80, 75, 80 },
		{ 70, 80, 90, 90, 60, 90, 80, 90 },
		spell_sap,              TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_sap,               SLOT(1400),     80,     12,
		"sap",                  "!Sap!",                "",     5,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"pain",                 { 45, 55, 70, 70, 30, 60, 70, 70 },
		{ 60, 70, 80, 80, 50, 70, 80, 80 },
		spell_pain,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_pain,              SLOT(1401),     50,     12,
		"torments",             "!Pain!",               "",     5,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"hex",                  { 40, 50, 60, 60, 20, 50, 60, 60 },
		{ 60, 70, 80, 80, 50, 70, 80, 80 },
		spell_hex,              TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
		&gsn_hex,               SLOT(1402),    100,     12,
		"hex",                  "You feel again the warmth of light.",  "",     5,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"bone wall",            { -1, -1, -1, -1, -1, -1, -1, -1 },
		{ -1, -1, -1, -1, -1, -1, -1, -1 },
		spell_bone_wall,                TAR_CHAR_SELF,          POS_STANDING,
		&gsn_bone_wall,         SLOT(1403),     80,     12,
		"wall of bones",        "The swirling bones fall to the ground.", "",    5,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},

	/* Paladins */

	{
		"hammerstrike",         { 70, 45, 60, 50, 70, 30, 60, 50 },
		{ 90, 65, 80, 70, 90, 50, 80, 70 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_hammerstrike,      SLOT(1500),        0,     36,
		"hammerstrike",         "The power of the gods has left you.",  "",     6,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"force shield",         { 70, 60, 80, 80, 70, 50, 80, 80 },
		{ 70, 60, 80, 80, 70, 50, 80, 80 },
		spell_force,            TAR_CHAR_SELF,          POS_FIGHTING,
		&gsn_force_shield,      SLOT(1501),    150,     12,
		"force shield",         "Your mystical aura fades.",            "",     6,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"holy sword",           { 70, 50, 70, 70, 70, 10, 70, 70 },
		{ 100, 70, 100, 100, 100, 40, 100, 100 },
		spell_holy_sword,       TAR_IGNORE,             POS_STANDING,
		&gsn_holy_sword,        SLOT(1502),     40,     36,
		"",                     "!Holy Sword!",         "",     6,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},

	/* Bards */

	{
		"align",                { 30, 40, 30, 35, 40, -1 /*40*/, 10, 30 },
		{ 70, 80, 70, 75, 80, -1 /*80*/, 50, 70 },
		spell_null,             TAR_IGNORE,          POS_STANDING,
		&gsn_align,             SLOT(1600),        5,      0,
		"",                     "!Align!",              "",     7,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"blur",                 { 70, 70, 45, 60, 70, 60, 30, 60 },
		{ 100, 100, 75, 90, 100, 90, 60, 90 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_blur,              SLOT(1601),        0,      0,
		"",                     "!Blur!",               "",     7,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"dual second",          { 80, 80, 70, 60, 80, 80, 30, 60 },
		{ 90, 90, 80, 70, 90, 90, 50, 70 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_dual_second,       SLOT(1602),        0,      0,
		"",                     "!Dual Second!",        "",     7,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"quick",                { -1, -1, -1, -1, -1, -1, -1, -1 },
		{ -1, -1, -1, -1, -1, -1, -1, -1 },
		spell_quick,            TAR_IGNORE,             POS_FIGHTING,
		&gsn_quick,             SLOT(1603),     60,     12,
		"quick",                "!Quick!",              "",     7,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},

	/* Rangers */

	{
		"standfast",            { 40, 35, 25, 20, 40, 10, 25, 20 },
		{ 70, 65, 55, 40, 70, 65, 55, 50 },
		spell_null,             TAR_IGNORE,             POS_FIGHTING,
		&gsn_standfast,         SLOT(1700),        0,      0,
		"",                     "!Standfast!",          "",     8,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"mark",                 { 60, 60, 50, 50, 60, 50, 40, 30 },
		{ 80, 80, 70, 70, 80, 70, 60, 50 },
		spell_null,             TAR_IGNORE,          POS_STANDING,
		&gsn_mark,              SLOT(1701),        50,      0,
		"",                     "!Mark!",               "",     8,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		"critical blow",        { 80, 80, 60, 70, 80, 65, 70, 50 },
		{ 100, 100, 75, 90, 100, 70, 75, 60 },
		spell_null,             TAR_CHAR_OFFENSIVE,             POS_FIGHTING,
		&gsn_critical_blow,     SLOT(1702),        30,     36,
		"critical blow",        "!Critical Blow!",      "",     8,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },     { 0, 0, 0, 0, 0, 0, 0, 0 }
	},

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
			"protection good", "sanctuary", "shield", "stone skin", "undo spell"
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

