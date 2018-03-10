#pragma once

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

#include "constants.hh"
#include "declare.hh"
#include "String.hh"

/* system calls */
//int unlink();
//int system();

// no better place to put this right now
#define URANGE(a, b, c)         ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))


/*
 * Attribute bonus structures.
 */
struct  str_app_type
{
	int      tohit;
	int      todam;
	int      carry;
	int      wield;
	int	stp;		/* bonus to stamina -- Montrey */
};

struct  int_app_type
{
	int      learn;
	int	manap;		/* bonus to mana -- Montrey */
};

struct  wis_app_type
{
	int      practice;
};

struct  dex_app_type
{
	int      defensive;
};

struct  con_app_type
{
	int      hitp;
	int      shock;
};

struct  chr_app_type
{
	int      chance;
};



struct deity_type
{
	String      name;
	String      align;
	int         value;
};

struct  guild_type
{
	String      name;                   /* the full name of the class */
	String      who_name;               /* Three-letter name for 'who'  */
	int      stat_prime;             /* Prime attribute              */
	int      weapon;                 /* First weapon                 */
	int      skill_adept;            /* Maximum skill level          */
	int      thac0_00;               /* Thac0 for level  0           */
	int      thac0_32;               /* Thac0 for level 32           */
	String      base_group;             /* base skills gained           */
	String      default_group;          /* default skills gained        */
	int      hp_min;                 /* Min hp gained on leveling    */
	int      hp_max;                 /* Max hp gained on leveling    */
	int	mana_min;		/* Min mana gained on leveling  */
	int	mana_max;		/* Max mana gained on leveling  */
	int	stam_min;		/* Min stamina gained on leveling */
	int	stam_max;		/* Max stamina gained on leveling */
};

struct item_type
{
	int         type;
	String      name;
};

struct weapon_table_t
{
	String      name;
	int      vnum;
	int      type;
	skill::type skill;
};

struct wiznet_type
{
	String      name;
	Flags::Bit  flag;
	int         level;
	String      desc;
};

struct attack_type
{
	String      name;                   /* name */
	String      noun;                   /* message */
	int         damage;                 /* damage class */
};

struct race_type
{
	String      name;                   /* call name of the race */
	bool        pc_race;                /* can be chosen by pcs */
	Flags       act;                    /* act bits for the race */
	Flags       aff;                    /* aff bits for the race */
	Flags       off;                    /* off bits for the race */
	Flags       imm;                    /* imm bits for the race */
	Flags       res;                    /* res bits for the race */
	Flags       vuln;                   /* vuln bits for the race */
	Flags       form;                   /* default form flag for the race */
	Flags       parts;                  /* default parts for the race */
};


struct pc_race_type  /* additional data for pc races */
{
	String      name;                   /* MUST be in race_type */
	String      who_name;		/* first 3 characters are used in 'who' */
	int	    base_age;		/* the base age for the race */
	int      points;                 /* cost in points of the race */
	int      guild_mult[Guild::size];  /* exp multiplier for class, * 100 */
	int      stats[MAX_STATS];       /* starting stats */
	int      max_stats[MAX_STATS];   /* maximum stats */
	int      size;                   /* aff bits for the race */
	int         remort_level;           /* remort level of race */
	String      skills[5];              /* bonus skills for the race */
};


struct spec_type
{
	String      name;                   /* special function name */
	SPEC_FUN *  function;               /* the function */
};


/*
 * Liquids.
 */

struct  liq_type
{
	String      name;
	String      color;
	int      affect[5];
};



struct  group_type
{
	String      name;
	int      rating[Guild::size];
	std::vector<String> spells;
};




/*
 * Global constants.
 */
extern  const   struct  str_app_type    str_app         [26];
extern  const   struct  int_app_type    int_app         [26];
extern  const   struct  wis_app_type    wis_app         [26];
extern  const   struct  dex_app_type    dex_app         [26];
extern  const   struct  con_app_type    con_app         [26];
extern  const   struct  chr_app_type    chr_app         [26];

extern  const   std::vector<guild_type>      guild_table;
extern  const   std::vector<deity_type>      deity_table;
extern  const   std::vector<weapon_table_t>     weapon_table;
extern  const   std::vector<item_type>       item_table;
extern  const   std::vector<wiznet_type>     wiznet_table;
extern  const   std::vector<attack_type>     attack_table;
extern  const   std::vector<race_type>       race_table;
extern  const   std::vector<pc_race_type>    pc_race_table;
extern  const   std::vector<spec_type>       spec_table;
extern  const   std::vector<liq_type>        liq_table;
extern  const   std::vector<group_type>      group_table;

/* new social system by Clerve */
extern          Social      *social_table_head;
extern          Social      *social_table_tail;

/* storage list */
extern		StoredPlayer		*storage_list_head;
extern		StoredPlayer		*storage_list_tail;

/* departed list */
extern		DepartedPlayer		*departed_list_head;
extern		DepartedPlayer		*departed_list_tail;
extern		char 			*departed_list_line;

/* War stuff */
extern 		War		*war_table_head;
extern 		War		*war_table_tail;
//extern		Mercenary	*merc_table_head; // currently unused
//extern		Mercenary	*merc_table_tail;


