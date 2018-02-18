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
#include <map>

#include "skill/Type.hh"
#include "affect/Type.hh"
#include "String.hh"
#include "Flags.hh"

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
	String      who_name;               /* Three-letter name for 'who'  */
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

struct weapon_table_t
{
	String      name;
	sh_int      vnum;
	sh_int      type;
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
	sh_int	    base_age;		/* the base age for the race */
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
extern		Mercenary	*merc_table_head;
extern		Mercenary	*merc_table_tail;

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

extern long	       quest_double;


/* act_comm.c */
void    add_follower    args( ( Character *ch, Character *master ) );
void    stop_follower   args( ( Character *ch ) );
void    nuke_pets       args( ( Character *ch ) );
void    die_follower    args( ( Character *ch ) );
bool    is_same_group   args( ( Character *ach, Character *bch ) );
void    send_to_clan    args( ( Character *ch, Clan *target, const String& text ) );

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
int  set_tail           args( (Character *ch, Character *victim, Flags::Bit tail_flags) );

/* alias.c */
void    substitute_alias args( (Descriptor *d, String input) );

/* bank.c */
void    find_money      args( ( Character *ch ) );

/* comm.c */
void    close_socket    args( ( Descriptor *dclose ) );
void	cwtb		args((Descriptor *d, const String& txt));
void    write_to_buffer args( ( Descriptor *d, const String& txt ) );

/* nanny.c */
void	update_pc_index		args((Character *ch, bool remove));

/* db.c */
void    boot_db         args( ( void ) );
void    clear_char      args( ( Character *ch ) );
void    tail_chain      args( ( void ) );

/* area_handler.c */
Character *    create_mobile   args( ( MobilePrototype *pMobIndex ) );
void    clone_mobile    args( ( Character *parent, Character *clone) );
Object *    create_object   args( ( ObjectPrototype *pObjIndex, int level ) );
void    clone_object    args( ( Object *parent, Object *clone ) );
MobilePrototype *   get_mob_index   args( ( int vnum ) );
ObjectPrototype *   get_obj_index   args( ( int vnum ) );
RoomPrototype *   get_room_index  args( ( int vnum ) );


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
								skill::type attack_skill, int attack_type, int damage_type, bool show, bool spell ) );
void    death_cry       args( ( Character *ch ) );
bool    is_safe         args( (Character *ch, Character *victim, bool showmsg ) );
bool    is_safe_spell   args( (Character *ch, Character *victim, bool area ) );
bool	is_safe_char	args((Character *ch, Character *victim, bool showmsg));		/* Montrey */
void    multi_hit       args( ( Character *ch, Character *victim, skill::type ) );
void    raw_kill        args( ( Character *victim ) );
void    stop_fighting   args( ( Character *ch, bool fBoth ) );
void    update_pos      args( ( Character *victim ) );
void    violence_update args( ( void ) );

/* mob_prog.c */
void    mprog_wordlist_check    args ( ( const String& arg, Character *mob,
										Character* actor, Object* object,
										void* vo, Flags::Bit type ) );
void    mprog_percent_check     args ( ( Character *mob, Character* actor,
										Object* object, void* vo,
										Flags::Bit type ) );
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
void    mprog_random_area_trigger    args ( ( Character* mob ) );
void    mprog_tick_trigger      args ( ( Character* mob ) );
void 	mprog_boot_trigger	args ( ( Character* mob	) );
void    mprog_speech_trigger    args ( ( const String& txt, Character* mob ) );


// attribute.c
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
int     count_users     args( (Object *obj) );
int     get_weapon_type     args( ( const String& name) );
bool    is_clan         args((Character *ch) );
bool    is_same_clan    args((Character *ch, Character *victim));
skill::type     get_weapon_skill   args(( Character *ch, bool secondary ) );
int     get_weapon_learned args(( Character *ch, skill::type type ) );
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
int	get_affect_evolution	args((Character *ch, affect::Type type));
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
skill::type    find_spell      args( ( Character *ch, const String& name) );
void    obj_cast_spell  args( ( skill::type, int level, Character *ch,
									Character *victim, Object *obj ) );
void spell_imprint      args( ( skill::type, int level, Character *ch, void *vo ));

/* quest.c */
void    sq_cleanup	 args( ( Character *ch ) );

/* save.c */
void    save_char_obj    args( ( Character *ch ) );
void    backup_char_obj  args( ( Character *ch ) );
bool    load_char_obj    args( ( Descriptor *d, const String& name ) );
const char    *dizzy_ctime     args( ( time_t *timep ) );
time_t  dizzy_scantime   args( ( const String& ctime ) );

/* skills.c */
void    set_learned       args(( Character *ch, skill::type type, int value ) );
void    set_evolution     args(( Character *ch, skill::type type, int value ) );
int     get_learned       args(( const Character *ch, skill::type type ) );
int     get_evolution     args(( const Character *ch, skill::type ) );
long    exp_per_level   args( ( Character *ch, int points ) );
void    check_improve   args( ( Character *ch, skill::type, bool success,
									int multiplier ) );
int     group_lookup    args( (const String& name) );
void    gn_add          args( ( Character *ch, int gn) );
void    gn_remove       args( ( Character *ch, int gn) );
void    group_add       args( ( Character *ch, const String& name, bool deduct) );
void    group_remove    args( ( Character *ch, const String& name) );
int	get_skill_cost  args( ( Character *ch, skill::type ) );
bool	deduct_stamina  args( ( Character *ch, skill::type ) );

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
bool    HAS_EXTRACLASS	args( ( Character *ch, skill::type ) );
bool    CAN_USE_RSKILL  args( ( Character *ch, skill::type ) );
void    list_extraskill args( ( Character *ch ) );

