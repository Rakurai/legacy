#pragma once

// Forward declarations of all major classes on the mud.  This may disappear as
// things become less tightly coupled.
class Actable;
class Area;
class Battle;
class Character;
class Clan;
class Customize;
class DepartedPlayer;
class Descriptor;
class Disabled;
class Duel;
class Edit;
class EQSocket;
class Exit;
class ExitPrototype;
class ExtraDescr;
class Flags;
class Game;
class GameTime;
class MobilePrototype;
class MobProg;
class MobProgActList;
class Note;
class Object;
class ObjectPrototype;
class ObjectValue;
class ObjectValues;
class Player;
class QuestArea;
class Reset;
class Room;
class RoomID;
class RoomPrototype;
class Shop;
class Social;
class StoredPlayer;
class String;
class Tail;
class War;
class Weather;
class Game;
class World;
class Vnum;
class Location;

namespace conn {
struct State;
}

namespace worldmap {
class Region;
class MapColor;
class Worldmap;
class Coordinate;
}

namespace util {
class Image;
}

// gross, C++14(17?) and back don't allow forward declarations of enums,
// so they have to be included here.  remove when possible
#include "Flags.hh"
#include "skill/Type.hh"
#include "affect/Type.hh"
#include "Guild.hh"

// prevent expansion of arguments
#define args( list )                    list

// forward declaration of methods that aren't yet sorted into logical places

/*
 * Function types.
 */
typedef void DO_FUN     args( ( Character *ch, String argument ) );
typedef bool SPEC_FUN   args( ( Character *ch ) );

#define DECLARE_DO_FUN( fun )           DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )         SPEC_FUN  fun

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
Room  *get_random_room   args( ( Character *ch ) );

/* act_obj.c */
void    make_pet        args((Character *ch, Character *pet));
bool can_loot           args( (Character *ch, Object *obj) );
void    get_obj         args( ( Character *ch, Object *obj,
							Object *container ) );

/* act_wiz.c */
Room *find_location      args( (Character *ch, const String& argument) );
int  set_tail           args((Character *ch, Character *victim, Flags tail_flags));

/* alias.c */
void    substitute_alias args( (Descriptor *d, String input) );

/* bank.c */
void    find_money      args( ( Character *ch ) );

/* area_handler.c */
Character *    create_mobile   args( ( MobilePrototype *pMobIndex ) );
void    clone_mobile    args( ( Character *parent, Character *clone) );
Object *    create_object   args( ( ObjectPrototype *pObjIndex, int level ) );
void    clone_object    args( ( Object *parent, Object *clone ) );

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
void    char_to_room    args(( Character *ch, Room *room ) );
int     apply_ac        args(( Object *obj, int iWear, int type ) );
Object *    get_eq_char     args(( Character *ch, int iWear ) );
void    equip_char      args(( Character *ch, Object *obj, int iWear ) );
void    unequip_char    args(( Character *ch, Object *obj ) );
int     count_obj_list  args(( ObjectPrototype *obj, Object *list ) );
void    obj_from_room   args(( Object *obj ) );
void    obj_to_room     args(( Object *obj, Room *room ) );
void    obj_to_obj      args(( Object *obj, Object *obj_to ) );
void    obj_from_obj    args(( Object *obj ) );
void    extract_char    args(( Character *ch, bool fPull ) );
Object *    get_obj_type    args(( ObjectPrototype *pObjIndexData ) );
Object *    create_money    args(( int gold, int silver ) );
int     get_obj_number  args(( Object *obj ) );
int     get_obj_weight  args(( Object *obj ) );
int     get_true_weight args(( Object *obj ) );
bool    room_is_dark    args(( const Room *room));
bool    room_is_very_dark args((const Room *room));
bool    is_room_owner   args(( Character *ch, Room *room) );
bool    room_is_private args(( Room *room ) );
bool    is_blinded      args(( const Character *ch ));
bool    can_see_char    args(( const Character *ch, const Character *victim ) );
bool    can_see_who     args(( const Character *ch, const Character *victim ) );
bool    can_see_obj     args(( const Character *ch, const Object *obj ) );
bool    can_see_room    args(( const Character *ch, const Room *room) );
bool    can_see_in_room args(( const Character *ch, const Room *room));
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
int	get_affect_evolution	args((Character *ch, affect::type type));
int	interpolate		args((int level, int value_00, int value_32));
const ExtraDescr *get_extra_descr		args((const String& name, const ExtraDescr *ed));

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

// loot.c 
void generate_eq		  args(( Character *ch));

/* quest.c */
void    quest_cleanup	 args( ( Character *ch ) );
void    sq_cleanup	 args( ( Character *ch ) );

/* save.c */
void    save_char_obj    args( ( Character *ch ) );
void    backup_char_obj  args( ( Character *ch ) );
bool    load_char_obj    args( ( Descriptor *d, const String& name ) );
void	update_pc_index		args((const Character *ch, bool remove));

/* skills.c */
void    set_learned       args(( Character *ch, skill::type type, int value ) );
void    set_evolution     args(( Character *ch, skill::type type, int value ) );
int     get_learned       args(( const Character *ch, skill::type type ) );
int     get_evolution     args(( const Character *ch, skill::type ) );
int    exp_per_level   args( ( Character *ch, int points ) );
void    check_improve   args( ( Character *ch, skill::type, bool success,
									int multiplier ) );
int     group_lookup    args( (const String& name) );
void    gn_add          args( ( Character *ch, int gn) );
void    gn_remove       args( ( Character *ch, int gn) );
void    group_add       args( ( Character *ch, const String& name, bool deduct) );
void    group_remove    args( ( Character *ch, const String& name) );
int	    get_skill_cost  args(( Character *ch, skill::type ) );
int     get_usable_level args(( skill::type sn, Guild guild ));
int     get_skill_level args(( const Character *ch, skill::type type ) );
bool	deduct_stamina  args( ( Character *ch, skill::type ) );

/* special.c */
SPEC_FUN *    spec_lookup     args( ( const String& name ) );
String spec_name       args( ( SPEC_FUN *function ) );
bool    IS_SPECIAL      args( ( Character *ch) );

/* teleport.c */
Room *   room_by_name    args( ( const char *target, int level, bool error) );

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

