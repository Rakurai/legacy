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


#ifndef MERC_HEADER_FILE__
#define MERC_HEADER_FILE__

// all system includes here, not in .c files
// include local headers as needed in .c files only, starting with this header
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

// hopefully temporary mud-wide includes, this can change as things get consolidated into class files

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

#include "declare.h"
#include "argument.h"

#include "memory.h"
#include "Actable.hpp"
#include "String.hpp"
#include "Format.hpp"
#include "Note.hpp"
#include "Object.hpp"
#include "Affect.hpp"
#include "ExtraDescr.hpp"
#include "Disabled.hpp"
#include "Battle.hpp"
#include "Weather.hpp"
#include "Tail.hpp"
#include "Descriptor.hpp"
#include "Shop.hpp"
#include "Edit.hpp"
#include "MobilePrototype.hpp"
#include "MobProg.hpp"
#include "MobProgActList.hpp"
#include "Character.hpp"
#include "Player.hpp"
#include "ObjectPrototype.hpp"
#include "Customize.hpp"
#include "Exit.hpp"
#include "Reset.hpp"
#include "Area.hpp"
#include "RoomPrototype.hpp"
#include "Social.hpp"
#include "Clan.hpp"
#include "StoredPlayer.hpp"
#include "DepartedPlayer.hpp"
#include "War.hpp"
#include "Mercenary.hpp"
#include "Duel.hpp"

/* system calls */
//int unlink();
int system();

/* global access to our port number, set in comm.c */
extern int port;





/*
 * Attribute bonus structures.
 */
struct  str_app_type
{
	sh_int      tohit;
	sh_int      todam;
	sh_int      carry;
	sh_int      wield;
	sh_int	stp;		/* bonus to stamina -- Montrey */
};

struct  int_app_type
{
	sh_int      learn;
	sh_int	manap;		/* bonus to mana -- Montrey */
};

struct  wis_app_type
{
	sh_int      practice;
};

struct  dex_app_type
{
	sh_int      defensive;
};

struct  con_app_type
{
	sh_int      hitp;
	sh_int      shock;
};

struct  chr_app_type
{
	sh_int      chance;
};



struct deity_type
{
	String      name;
	String      align;
	int         value;
};

struct  class_type
{
	String      name;                   /* the full name of the class */
	char        who_name        [4];    /* Three-letter name for 'who'  */
	sh_int      stat_prime;             /* Prime attribute              */
	sh_int      weapon;                 /* First weapon                 */
	sh_int      skill_adept;            /* Maximum skill level          */
	sh_int      thac0_00;               /* Thac0 for level  0           */
	sh_int      thac0_32;               /* Thac0 for level 32           */
	String      base_group;             /* base skills gained           */
	String      default_group;          /* default skills gained        */
	sh_int      hp_min;                 /* Min hp gained on leveling    */
	sh_int      hp_max;                 /* Max hp gained on leveling    */
	sh_int	mana_min;		/* Min mana gained on leveling  */
	sh_int	mana_max;		/* Max mana gained on leveling  */
	sh_int	stam_min;		/* Min stamina gained on leveling */
	sh_int	stam_max;		/* Max stamina gained on leveling */
};

struct item_type
{
	int         type;
	String      name;
};

struct weapon_type
{
	String      name;
	sh_int      vnum;
	sh_int      type;
	sh_int      *gsn;
};

struct wiznet_type
{
	String      name;
	long        flag;
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
	long	act;			/* act bits for the race */
	long        aff;                    /* aff bits for the race */
	long        off;                    /* off bits for the race */
	long        imm;                    /* imm bits for the race */
	long        res;                    /* res bits for the race */
	long        vuln;                   /* vuln bits for the race */
	long        form;                   /* default form flag for the race */
	long        parts;                  /* default parts for the race */
};


struct pc_race_type  /* additional data for pc races */
{
	String      name;                   /* MUST be in race_type */
	char        who_name[4];		/* first 3 characters are used in 'who' */
	sh_int	base_age;		/* the base age for the race */
	sh_int      points;                 /* cost in points of the race */
	sh_int      class_mult[MAX_CLASS];  /* exp multiplier for class, * 100 */
	sh_int      stats[MAX_STATS];       /* starting stats */
	sh_int      max_stats[MAX_STATS];   /* maximum stats */
	sh_int      size;                   /* aff bits for the race */
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
	sh_int      affect[5];
};


/*
 * Skills include spells as a particular case.
 */
struct  skill_type
{
	String      name;                   /* Name of skill                */
	sh_int      skill_level[MAX_CLASS]; /* Level needed by class        */
	sh_int      rating[MAX_CLASS];      /* How hard it is to learn      */
	SPELL_FUN * spell_fun;              /* Spell pointer (for spells)   */
	sh_int      target;                 /* Legal targets                */
	sh_int      minimum_position;       /* Position for caster / user   */
	sh_int *    pgsn;                   /* Pointer to associated gsn    */
	sh_int      slot;                   /* Slot for #OBJECT loading     */
	sh_int      min_mana;               /* Minimum mana used            */
	sh_int      beats;                  /* Waiting time after use       */
	String      noun_damage;            /* Damage message               */
	String      msg_off;                /* Wear off message             */
	String      msg_obj;                /* Wear off message for obects  */
	int         remort_class;           /* Required remort level or 0   */
	int		evocost_sec[MAX_CLASS];	/* Class cost to evolve to 2    */
	int		evocost_pri[MAX_CLASS];	/* Class cost to evolve to 3    */
};

struct  group_type
{
	String      name;
	sh_int      rating[MAX_CLASS];
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

extern  const   std::vector<class_type>      class_table;
extern  const   std::vector<deity_type>      deity_table;
extern  const   std::vector<weapon_type>     weapon_table;
extern  const   std::vector<item_type>       item_table;
extern  const   std::vector<wiznet_type>     wiznet_table;
extern  const   std::vector<attack_type>     attack_table;
extern  const   std::vector<race_type>       race_table;
extern  const   std::vector<pc_race_type>    pc_race_table;
extern  const   std::vector<spec_type>       spec_table;
extern  const   std::vector<liq_type>        liq_table;
extern  const   std::vector<skill_type>      skill_table;
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
extern		Mercenary	*merc_table_head;
extern		Mercenary	*merc_table_tail;
extern		Duel::Arena	*arena_table_head;
extern		Duel::Arena	*arena_table_tail;
extern		Duel	*duel_table_head;
extern		Duel	*duel_table_tail;

/*
 * Global variables.
 */
extern          Character         *     char_list;
extern          Object          *     object_list;

extern		Player *		pc_list;	/* Montrey */

extern          char                    bug_buf         [];
extern          time_t                  current_time;
extern          bool                    fLogAll;
extern          char                    log_buf         [];
extern		Object	  *	donation_pit;

/* records */
extern	unsigned long	record_logins;
extern	int		record_players;
extern	int		record_players_since_boot;

/*** Quest-related global data ***/
/* quest_open, if true, says that the quest area is open.
   The other variables are defined only if quest_open is true. */
/* quest_upk is restricted pk in quest area or not, quest_double is double qp time */
extern bool            quest_open;
extern bool            quest_upk;
extern long	       quest_double;
extern int             quest_min, quest_max;
extern RoomPrototype *quest_startroom;
extern Area       *quest_area;


/*
 * These are skill_lookup return values for common skills and spells.
 */
extern sh_int   gsn_reserved;
extern sh_int   gsn_acid_blast;
extern sh_int   gsn_acid_breath;
extern sh_int   gsn_acid_rain;
extern sh_int   gsn_age;
extern sh_int   gsn_animate_skeleton;
extern sh_int   gsn_animate_wraith;
extern sh_int   gsn_animate_gargoyle;
extern sh_int   gsn_animate_zombie;
extern sh_int   gsn_armor;
extern sh_int   gsn_bless;
extern sh_int   gsn_blind_fight;
extern sh_int   gsn_blindness;
extern sh_int   gsn_blizzard;
extern sh_int   gsn_blood_blade;
extern sh_int   gsn_blood_moon;
extern sh_int   gsn_burning_hands;
extern sh_int   gsn_call_lightning;
extern sh_int   gsn_calm;
extern sh_int   gsn_cancellation;
extern sh_int   gsn_cause_light;
extern sh_int   gsn_cause_serious;
extern sh_int   gsn_cause_critical;
extern sh_int   gsn_chain_lightning;
extern sh_int   gsn_change_sex;
extern sh_int   gsn_channel;
extern sh_int   gsn_charm_person;
extern sh_int   gsn_chill_touch;
extern sh_int   gsn_colour_spray;
extern sh_int   gsn_continual_light;
extern sh_int   gsn_control_weather;
extern sh_int   gsn_create_food;
extern sh_int   gsn_create_parchment;
extern sh_int   gsn_create_rose;
extern sh_int   gsn_create_sign;
extern sh_int   gsn_create_spring;
extern sh_int   gsn_create_vial;
extern sh_int   gsn_create_water;
extern sh_int   gsn_cure_blindness;
extern sh_int   gsn_cure_critical;
extern sh_int   gsn_cure_disease;
extern sh_int   gsn_cure_light;
extern sh_int   gsn_cure_poison;
extern sh_int   gsn_cure_serious;
extern sh_int   gsn_curse;
extern sh_int   gsn_darkness;
extern sh_int   gsn_dazzling_light;
extern sh_int   gsn_demonfire;
extern sh_int   gsn_detect_evil;
extern sh_int   gsn_detect_good;
extern sh_int   gsn_detect_hidden;
extern sh_int   gsn_detect_invis;
extern sh_int   gsn_detect_magic;
extern sh_int   gsn_detect_poison;
extern sh_int   gsn_dispel_evil;
extern sh_int   gsn_dispel_good;
extern sh_int   gsn_dispel_magic;
extern sh_int   gsn_divine_healing;
extern sh_int   gsn_divine_regeneration;
extern sh_int   gsn_earthquake;
extern sh_int   gsn_encampment;
extern sh_int   gsn_enchant_armor;
extern sh_int   gsn_enchant_weapon;
extern sh_int   gsn_energy_drain;
extern sh_int   gsn_faerie_fire;
extern sh_int   gsn_faerie_fog;
extern sh_int   gsn_farsight;
extern sh_int   gsn_fear;
extern sh_int   gsn_fire_breath;
extern sh_int   gsn_fireball;
extern sh_int   gsn_fireproof;
extern sh_int   gsn_firestorm;
extern sh_int   gsn_flame_blade;
extern sh_int   gsn_flameshield;
extern sh_int   gsn_flamestrike;
extern sh_int   gsn_fly;
extern sh_int   gsn_floating_disc;
extern sh_int   gsn_frenzy;
extern sh_int   gsn_frost_blade;
extern sh_int   gsn_frost_breath;
extern sh_int   gsn_gas_breath;
extern sh_int   gsn_gate;
extern sh_int   gsn_general_purpose;
extern sh_int   gsn_giant_strength;
extern sh_int   gsn_harm;
extern sh_int   gsn_haste;
extern sh_int   gsn_heal;
extern sh_int   gsn_heat_metal;
extern sh_int   gsn_high_explosive;
extern sh_int   gsn_holy_word;
extern sh_int   gsn_identify;
extern sh_int   gsn_invis;
extern sh_int   gsn_know_alignment;
extern sh_int   gsn_light_of_truth;
extern sh_int   gsn_lightning_bolt;
extern sh_int   gsn_lightning_breath;
extern sh_int   gsn_locate_life;
extern sh_int   gsn_locate_object;
extern sh_int   gsn_magic_missile;
extern sh_int   gsn_mass_healing;
extern sh_int   gsn_mass_invis;
extern sh_int   gsn_nexus;
extern sh_int   gsn_night_vision;
extern sh_int   gsn_pass_door;
extern sh_int   gsn_plague;
extern sh_int   gsn_poison;
extern sh_int   gsn_power_word;
extern sh_int   gsn_polymorph;
extern sh_int   gsn_portal;
extern sh_int   gsn_protect_container;
extern sh_int   gsn_protection_evil;
extern sh_int   gsn_protection_good;
extern sh_int   gsn_ray_of_truth;
extern sh_int   gsn_recharge;
extern sh_int   gsn_refresh;
extern sh_int   gsn_resurrect;
extern sh_int   gsn_regeneration;
extern sh_int   gsn_remove_alignment;
extern sh_int   gsn_remove_invis;
extern sh_int   gsn_remove_curse;
extern sh_int   gsn_sanctuary;
extern sh_int   gsn_scry;
extern sh_int   gsn_shield;
extern sh_int   gsn_shock_blade;
extern sh_int   gsn_shocking_grasp;
extern sh_int   gsn_shrink;
extern sh_int   gsn_sleep;
extern sh_int   gsn_slow;
extern sh_int   gsn_smokescreen;
extern sh_int   gsn_starve;
extern sh_int   gsn_steel_mist;
extern sh_int   gsn_stone_skin;
extern sh_int   gsn_summon;
extern sh_int   gsn_summon_object;
extern sh_int   gsn_sunray;
extern sh_int   gsn_talon;
extern sh_int   gsn_teleport;
extern sh_int   gsn_teleport_object;
extern sh_int   gsn_undo_spell;
extern sh_int   gsn_ventriloquate;
extern sh_int   gsn_vision;
extern sh_int   gsn_weaken;
extern sh_int   gsn_word_of_recall;
extern sh_int   gsn_wrath;
extern sh_int   gsn_axe;
extern sh_int   gsn_dagger;
extern sh_int   gsn_flail;
extern sh_int   gsn_mace;
extern sh_int   gsn_polearm;
extern sh_int   gsn_spear;
extern sh_int   gsn_sword;
extern sh_int   gsn_whip;
extern sh_int   gsn_archery;
extern sh_int   gsn_shield_block;
extern sh_int   gsn_brew;
extern sh_int   gsn_scribe;
extern sh_int   gsn_backstab;
extern sh_int   gsn_bash;
extern sh_int   gsn_berserk;
extern sh_int   gsn_circle;
extern sh_int   gsn_crush;
extern sh_int   gsn_dirt_kicking;
extern sh_int   gsn_disarm;
extern sh_int   gsn_dodge;
extern sh_int   gsn_enhanced_damage;
extern sh_int   gsn_envenom;
extern sh_int   gsn_hand_to_hand;
extern sh_int   gsn_kick;
extern sh_int   gsn_roundhouse; /*wchange added for evo 2+'s second hit*/
extern sh_int   gsn_footsweep;
extern sh_int gsn_necromancy;
extern sh_int   gsn_parry;
extern sh_int   gsn_rescue;
extern sh_int   gsn_trip;
extern sh_int   gsn_second_attack;
extern sh_int   gsn_third_attack;
extern sh_int   gsn_dual_wield;
extern sh_int   gsn_hunt;
extern sh_int  gsn_unarmed;
extern sh_int   gsn_swimming;
extern sh_int   gsn_fast_healing;
extern sh_int   gsn_firebuilding;
extern sh_int   gsn_forge;
extern sh_int   gsn_repair;
extern sh_int   gsn_rotate;
extern sh_int   gsn_languages;
extern sh_int   gsn_haggle;
extern sh_int   gsn_hide;
extern sh_int   gsn_lore;
extern sh_int   gsn_meditation;
extern sh_int   gsn_peek;
extern sh_int   gsn_pick_lock;
extern sh_int   gsn_scan;
extern sh_int   gsn_sneak;
extern sh_int   gsn_steal;
extern sh_int   gsn_sing;
extern sh_int   gsn_scrolls;
extern sh_int   gsn_spousegate;
extern sh_int   gsn_staves;
extern sh_int   gsn_wands;
extern sh_int   gsn_recall;
extern sh_int  gsn_lay_on_hands;
extern sh_int  gsn_familiar;
extern sh_int  gsn_die_hard;
extern sh_int   gsn_sheen;
extern sh_int   gsn_focus;
extern sh_int   gsn_paralyze;
extern sh_int   gsn_barrier;
extern sh_int   gsn_dazzle;
extern sh_int   gsn_full_heal;
extern sh_int   gsn_midnight;
extern sh_int   gsn_shadow_form;
extern sh_int   gsn_hone;
extern sh_int   gsn_riposte;
extern sh_int   gsn_fourth_attack;
extern sh_int   gsn_rage;
extern sh_int  gsn_blind_fight;
extern sh_int   gsn_sap;
extern sh_int   gsn_pain;
extern sh_int   gsn_hex;
extern sh_int   gsn_bone_wall;
extern sh_int   gsn_hammerstrike;
extern sh_int   gsn_force_shield;
extern sh_int   gsn_holy_sword;
extern sh_int   gsn_align;
extern sh_int   gsn_blur;
extern sh_int   gsn_dual_second;
extern sh_int   gsn_quick;
extern sh_int   gsn_standfast;
extern sh_int   gsn_mark;
extern sh_int   gsn_critical_blow;

/* act_comm.c */
void    add_follower    args( ( Character *ch, Character *master ) );
void    stop_follower   args( ( Character *ch ) );
void    nuke_pets       args( ( Character *ch ) );
void    die_follower    args( ( Character *ch ) );
bool    is_same_group   args( ( Character *ach, Character *bch ) );
void    send_to_clan    args( ( Character *ch, Clan *target, const char *text ) );

/* social-edit.c */
void load_social_table();
void save_social_table();
int count_socials();

/* war.c */
void	load_war_table();
void	load_war_events();
void	save_war_table();
void	save_war_events();
void	war_kill		args((Character *ch, Character *victim));
bool	char_at_war		args((Character *ch));
bool	clan_at_war		args((Clan *clan));
bool	char_opponents		args((Character *charA, Character *charB));
bool	clan_opponents		args((Clan *clanA, Clan *clanB));

/* duel.c */
void	duel_update();
void	load_arena_table();
//void	view_room_hpbar();
void	duel_kill		args((Character *victim));
bool	char_in_darena_room	args((Character *ch));
bool	char_in_duel_room	args((Character *ch));
bool	char_in_darena		args((Character *ch));
bool	char_in_duel		args((Character *ch));
Duel *get_duel		args((Character *ch));

/* storage.c */
void load_storage_list();
void save_storage_list();
int count_stored_characters();
void insert_storagedata(StoredPlayer *);
void remove_storagedata(StoredPlayer *);
StoredPlayer *lookup_storage_data(const String&);

/* departed.c */
void load_departed_list();
void save_departed_list();

void remove_departed(const String&);
void insert_departed(const String&);
bool has_departed(const String&);

/* act_info.c */
void    set_title       args( ( Character *ch, const String& title ) );
void	set_color	args((Character *ch, int color, int bold));
void	new_color	args((Character *ch, int custom));
void    show_affect_to_char  args((const Affect *paf, Character *ch));

/* act_move.c */
void    move_char       args( ( Character *ch, int door, bool follow ) );
void    recall		args( ( Character *ch, bool clan ) );
RoomPrototype  *get_random_room   args( ( Character *ch ) );

/* act_obj.c */
void    make_pet        args((Character *ch, Character *pet));
bool can_loot           args( (Character *ch, Object *obj) );
void    get_obj         args( ( Character *ch, Object *obj,
							Object *container ) );

/* act_wiz.c */
RoomPrototype *find_location      args( (Character *ch, const String& argument) );
int  set_tail           args( (Character *ch, Character *victim, int tail_flags) );

/* alias.c */
void    substitute_alias args( (Descriptor *d, String input) );

/* bank.c */
void    find_money      args( ( Character *ch ) );

/* comm.c */
void    show_string     args( ( Descriptor *d, const String& input) );
void    close_socket    args( ( Descriptor *dclose ) );
void	cwtb		args((Descriptor *d, const String& txt));
void    write_to_buffer args( ( Descriptor *d, const String& txt ) );

/* nanny.c */
void	update_pc_index		args((Character *ch, bool remove));

/* db.c */
void    boot_db         args( ( void ) );
void    clear_char      args( ( Character *ch ) );
void    bug             args( ( const String& str, int param ) );
void    log_string      args( ( const String& str ) );
void    tail_chain      args( ( void ) );

/* area_handler.c */
void    area_update     args( ( void ) );
Character *    create_mobile   args( ( MobilePrototype *pMobIndex ) );
void    clone_mobile    args( ( Character *parent, Character *clone) );
Object *    create_object   args( ( ObjectPrototype *pObjIndex, int level ) );
void    clone_object    args( ( Object *parent, Object *clone ) );
MobilePrototype *   get_mob_index   args( ( int vnum ) );
ObjectPrototype *   get_obj_index   args( ( int vnum ) );
RoomPrototype *   get_room_index  args( ( int vnum ) );
int	get_location_ac args( (Character *ch, int wear, int type) );


/* file.c */
char	fread_letter		args((FILE *fp));
int	fread_number		args((FILE *fp));
long	fread_flag		args((FILE *fp));
String	fread_string		args((FILE *fp, char to_char = '~'));
String	fread_string_eol	args((FILE *fp));
void	fread_to_eol		args((FILE *fp));
String	fread_word		args((FILE *fp));
void	fappend			args((const char *file, const char *str));


/* random.c */
int	number_fuzzy		args((int number));
int	number_range		args((int from, int to));
int	number_percent		args((void));
int	number_door		args((void));
int	number_bits		args((int width));
long	number_mm		args((void));
int	dice			args((int number, int size));
bool prd_chance      args(( int *prev_fails, int percent ));

/* load_config.c */
int     load_config     args((const char *filename));

/* help.c */
void	help		args((Character *ch, const String& argument));

/* effect.c */
void    acid_effect     args( (void *vo, int level, int dam, int target, int evolution) );
void    cold_effect     args( (void *vo, int level, int dam, int target, int evolution) );
void    fire_effect     args( (void *vo, int level, int dam, int target, int evolution) );
void    poison_effect   args( (void *vo, int level, int dam, int target, int evolution) );
void    shock_effect    args( (void *vo, int level, int dam, int target, int evolution) );


/* fight.c */
void    check_killer    args( ( Character *ch, Character *victim) );
bool    damage          args( ( Character *ch, Character *victim, int dam,
								int dt, int cls, bool show, bool spell ) );
void    death_cry       args( ( Character *ch ) );
bool    is_safe         args( (Character *ch, Character *victim, bool showmsg ) );
bool    is_safe_spell   args( (Character *ch, Character *victim, bool area ) );
bool	is_safe_char	args((Character *ch, Character *victim, bool showmsg));		/* Montrey */
void    multi_hit       args( ( Character *ch, Character *victim, int dt ) );
void    raw_kill        args( ( Character *victim ) );
void    stop_fighting   args( ( Character *ch, bool fBoth ) );
void    update_pos      args( ( Character *victim ) );
void    violence_update args( ( void ) );

/* mob_prog.c */
void    mprog_wordlist_check    args ( ( const String& arg, Character *mob,
										Character* actor, Object* object,
										void* vo, int type ) );
void    mprog_percent_check     args ( ( Character *mob, Character* actor,
										Object* object, void* vo,
										int type ) );
void    mprog_act_trigger       args ( ( const char* buf, Character* mob,
										Character* ch, Object* obj,
										void* vo ) );
void    mprog_bribe_trigger     args ( ( Character* mob, Character* ch,
										int amount ) );
void    mprog_entry_trigger     args ( ( Character* mob ) );
void    mprog_give_trigger      args ( ( Character* mob, Character* ch,
										Object* obj ) );
void    mprog_greet_trigger     args ( ( Character* mob ) );
void    mprog_fight_trigger     args ( ( Character* mob, Character* ch ) );
void    mprog_buy_trigger       args ( ( Character* mob, Character* ch ) );
void    mprog_hitprcnt_trigger  args ( ( Character* mob, Character* ch ) );
void    mprog_death_trigger     args ( ( Character* mob ) );
void    mprog_random_trigger    args ( ( Character* mob ) );
void    mprog_tick_trigger      args ( ( Character* mob ) );
void 	mprog_boot_trigger	args ( ( Character* mob	) );
void    mprog_speech_trigger    args ( ( const String& txt, Character* mob ) );


// attribute.c
int flag_to_index args((unsigned long flag));
int affect_bit_to_sn args((int bit));
int stat_to_attr args((int stat));
int     get_max_stat   args(( const Character *ch, int stat ) );
int get_age         args((Character *ch));
int get_max_hit args((Character *ch));
int get_max_mana args((Character *ch));
int get_max_stam args((Character *ch));
String print_defense_modifiers args((Character *ch, int where));
int get_unspelled_hitroll    args((Character *ch));
int get_unspelled_damroll    args((Character *ch));
int get_unspelled_ac        args((Character *ch, int type));

/* handler.c */
String  flags_to_string args((int flag));
long    string_to_flags args((const String& str));
int     count_users     args( (Object *obj) );
int     get_weapon_type     args( ( const String& name) );
bool    is_clan         args((Character *ch) );
bool    is_same_clan    args((Character *ch, Character *victim));
int     get_skill       args(( const Character *ch, int sn ) );
int     get_weapon_sn   args(( Character *ch, bool secondary ) );
int     get_weapon_skill args(( Character *ch, int sn ) );
void    reset_char      args(( Character *ch )  );
int     get_max_train   args(( Character *ch, int stat ) );
int     can_carry_n     args(( Character *ch ) );
int     can_carry_w     args(( Character *ch ) );
void    char_from_room  args(( Character *ch ) );
void    char_to_room    args(( Character *ch, RoomPrototype *pRoomIndex ) );
int     apply_ac        args(( Object *obj, int iWear, int type ) );
Object *    get_eq_char     args(( Character *ch, int iWear ) );
void    equip_char      args(( Character *ch, Object *obj, int iWear ) );
void    unequip_char    args(( Character *ch, Object *obj ) );
int     count_obj_list  args(( ObjectPrototype *obj, Object *list ) );
void    obj_from_room   args(( Object *obj ) );
void    obj_to_room     args(( Object *obj, RoomPrototype *pRoomIndex ) );
void    obj_to_obj      args(( Object *obj, Object *obj_to ) );
void    obj_from_obj    args(( Object *obj ) );
void    extract_char    args(( Character *ch, bool fPull ) );
Object *    get_obj_type    args(( ObjectPrototype *pObjIndexData ) );
Object *    create_money    args(( int gold, int silver ) );
int     get_obj_number  args(( Object *obj ) );
int     get_obj_weight  args(( Object *obj ) );
int     get_true_weight args(( Object *obj ) );
bool    room_is_dark    args(( RoomPrototype *room));
bool    room_is_very_dark args((RoomPrototype *room));
bool    is_room_owner   args(( Character *ch, RoomPrototype *room) );
bool    room_is_private args(( RoomPrototype *pRoomIndex ) );
bool    is_blinded      args(( const Character *ch ));
bool    can_see_char    args(( const Character *ch, const Character *victim ) );
bool    can_see_who     args(( const Character *ch, const Character *victim ) );
bool    can_see_obj     args(( const Character *ch, const Object *obj ) );
bool    can_see_room    args(( Character *ch, RoomPrototype *pRoomIndex) );
bool    can_see_in_room args(( Character *ch, RoomPrototype *room));
//const char *  first_arg       args(( const char *argument, char *arg_first, bool fCase ) );
const char *  get_who_line    args(( Character *ch, Character *victim ) );
bool    mob_exists      args(( const char *name ) );
//int	round			args((float fNum, int iInc));	/* below is Montrey's list of */
int	parse_deity		args((const String& dstring));		/* hacks!  Beware! :) */
int	get_usable_level	args((Character *ch));
int	get_holdable_level	args((Character *ch));
String	get_owner		args((Character *ch, Object *obj));
Character *	get_obj_carrier		args((Object *obj));
int	get_locker_number	args((Character *ch));
int	get_locker_weight	args((Character *ch));
int	get_strongbox_number	args((Character *ch));
int	get_carry_number	args((Character *ch));
int	get_carry_weight	args((Character *ch));
int	get_position		args((Character *ch));
int	get_play_hours		args((Character *ch));
int	get_play_seconds	args((Character *ch));
int	get_affect_evolution	args((Character *ch, int sn));
long	flag_convert		args((char letter));
int	interpolate		args((int level, int value_00, int value_32));
ExtraDescr *get_extra_descr		args((const String& name, ExtraDescr *ed));

/* objstate.c */
int     objstate_load_items		args((void));
int     objstate_save_items		args((void));

/* hunt.c */
void hunt_victim        args( ( Character *ch) );

/* interp.c */
void    interpret       args( ( Character *ch, String argument ) );
bool    is_number       args( ( const String& arg ) );
bool    check_social    args( ( Character *ch, const String& command,
							const String& argument ) );
void	do_huh		args( ( Character *ch ) );	/* Xenith */


/* magic.c */
void    spread_plague   args(( RoomPrototype *room, const Affect *plague, int chance));
int     find_spell      args( ( Character *ch, const String& name) );
int     skill_lookup    args( ( const String& name ) );
int     slot_lookup     args( ( int slot ) );
void    obj_cast_spell  args( ( int sn, int level, Character *ch,
									Character *victim, Object *obj ) );
void spell_imprint      args( ( int sn, int level, Character *ch, void *vo ));

// dispel.c
bool    saves_spell       args(( int level, Character *victim, int dam_type ) );
bool    check_dispel_char args(( int dis_level, Character *victim, int sn, bool save ));
bool    check_dispel_obj  args(( int dis_level, Object *obj, int sn, bool save ));
bool    undo_spell        args(( int dis_level, Character *victim, int sn, bool save ));
bool    dispel_char       args(( Character *victim, int level, bool cancellation ));
bool    level_save        args(( int dis_level, int save_level));

/* quest.c */
void    quest_init       args( (void) );
void    sq_cleanup	 args( ( Character *ch ) );

/* save.c */
void    save_char_obj    args( ( Character *ch ) );
void    backup_char_obj  args( ( Character *ch ) );
bool    load_char_obj    args( ( Descriptor *d, const String& name ) );
const char    *dizzy_ctime     args( ( time_t *timep ) );
time_t  dizzy_scantime   args( ( const String& ctime ) );

/* skills.c */
bool    parse_gen_groups args( ( Character *ch, String argument ) );
void    list_group_costs args( ( Character *ch ) );
void    list_group_known args( ( Character *ch ) );
long    exp_per_level   args( ( Character *ch, int points ) );
void    check_improve   args( ( Character *ch, int sn, bool success,
									int multiplier ) );
int     group_lookup    args( (const String& name) );
void    gn_add          args( ( Character *ch, int gn) );
void    gn_remove       args( ( Character *ch, int gn) );
void    group_add       args( ( Character *ch, const String& name, bool deduct) );
void    group_remove    args( ( Character *ch, const String& name) );
int     get_evolution   args( ( Character *ch, int sn ) );
int	get_skill_cost  args( ( Character *ch, int sn ) );
bool	deduct_stamina  args( ( Character *ch, int sn ) );

/* special.c */
SPEC_FUN *    spec_lookup     args( ( const String& name ) );
String spec_name       args( ( SPEC_FUN *function ) );
bool    IS_SPECIAL      args( ( Character *ch) );

/* teleport.c */
RoomPrototype *   room_by_name    args( ( const char *target, int level, bool error) );

/* tour.c */
//void    do_alight       args( ( Character *ch, const char *argument ) );
//void    do_board        args( ( Character *ch, const char *argument ) );
//void    load_tourstarts args( ( FILE *fp ) );
//void    load_tourroutes args( ( FILE *fp ) );
//void    tour_update     args( ( void ) );

/* update.c */
void    advance_level   args( ( Character *ch ) );
void    npc_advance_level args( ( Character *ch ) );
void    demote_level    args( ( Character *ch ) );
void    gain_exp        args( ( Character *ch, int gain ) );
void    gain_condition  args( ( Character *ch, int iCond, int value ) );
void    update_handler  args( ( void ) );

/* remort.c */
int     raff_lookup     args( ( int index ) );
bool    HAS_RAFF	args( ( Character *ch, int flag ) );
bool    HAS_RAFF_GROUP  args( ( Character *ch, int flag ) );
bool    HAS_EXTRACLASS	args( ( Character *ch, int sn ) );
bool    CAN_USE_RSKILL  args( ( Character *ch, int sn ) );
void    list_extraskill args( ( Character *ch ) );


template<class... Params>
void bugf(const String& fmt, Params... params)
{
	bug(Format::format(fmt, params...), 0);
}



// here's the place for header files that depend on merc.h, that we need in almost every .c file
#include "act.h"

#endif

