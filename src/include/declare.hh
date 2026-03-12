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
//#define args( list )                    list

// forward declaration of methods that aren't yet sorted into logical places

/*
 * Function types.
 */
typedef void DO_FUN(Character *ch, String argument);
typedef bool SPEC_FUN(Character *ch);

#define DECLARE_DO_FUN( fun )           DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )         SPEC_FUN  fun

/* act_comm.c */
void    add_follower(Character *ch, Character *master);
void    stop_follower(Character *ch);
void    nuke_pets(Character *ch);
void    die_follower(Character *ch);
bool    is_same_group(Character *ach, Character *bch);
void    send_to_clan(Character *ch, Clan *target, const String& text);

/* social-edit.c */
void load_social_table();
void save_social_table();
int count_socials();

/* war.c */
void	load_war_table();
void	load_war_events();
void	save_war_table();
void	save_war_events();
void	war_kill(Character *ch, Character *victim);
bool	char_at_war(Character *ch);
bool	clan_at_war(Clan *clan);
bool	char_opponents(Character *charA, Character *charB);
bool	clan_opponents(Clan *clanA, Clan *clanB);

/* duel.c */
void	duel_update();
void	load_arena_table();
//void	view_room_hpbar();
void	duel_kill(Character *victim);
bool	char_in_darena_room(Character *ch);
bool	char_in_duel_room(Character *ch);
bool	char_in_darena(Character *ch);
bool	char_in_duel(Character *ch);
Duel *get_duel(Character *ch);

/* storage.c */
void load_storage_list();
void save_storage_list();
int count_stored_characters();
void insert_storagedata(StoredPlayer *newdata);
void remove_storagedata(StoredPlayer *olddata);
StoredPlayer *lookup_storage_data(const String& name);

/* departed.c */
void load_departed_list();
void save_departed_list();

void remove_departed(const String& name);
void insert_departed(const String& name);
bool has_departed(const String& name);

/* act_info.c */

void    set_title(Character *ch, const String& title);
void	set_color(Character *ch, int color, int bold);
void	new_color(Character *ch, int slot);

/* act_move.c */
void    move_char(Character *ch, int door, bool follow);
void    recall(Character *ch, bool clan);
Room  *get_random_room(Character *ch);

/* act_obj.c */
void    make_pet(Character *ch, Character *pet);
bool can_loot(Character *ch, Object *obj);
void    get_obj(Character *ch, Object *obj, Object *container);

/* act_wiz.c */
Room *find_location(Character *ch, const String& argument);
int  set_tail(Character *ch, Character *victim, Flags tail_flags);

/* bank.c */
void    find_money(Character *ch);

/* area_handler.c */
Character *    create_mobile(MobilePrototype *pMobIndex);
void    clone_mobile(Character *parent, Character *clone);
Object *    create_object(ObjectPrototype *pObjIndex, int level);
void    clone_object(Object *parent, Object *clone);

/* load_config.c */
int     load_config(const char *filename);

/* help.c */
void	help(Character *ch, const String& argument);

/* effect.c */
void    acid_effect(void *vo, int level, int dam, int target, int evolution);
void    cold_effect(void *vo, int level, int dam, int target, int evolution);
void    fire_effect(void *vo, int level, int dam, int target, int evolution);
void    poison_effect(void *vo, int level, int dam, int target, int evolution);
void    shock_effect(void *vo, int level, int dam, int target, int evolution);


/* fight.c */
void    check_killer(Character *ch, Character *victim);
bool    damage(Character *ch, Character *victim, int dam, skill::type attack_skill, int attack_type, int dam_type, bool show, bool spell);
void    death_cry(Character *ch);
bool    is_safe(Character *ch, Character *victim, bool showmsg);
bool    is_safe_spell(Character *ch, Character *victim, bool area);
bool	is_safe_char(Character *ch, Character *victim, bool showmsg);		/**< Montrey */
void    multi_hit(Character *ch, Character *victim, skill::type attack_skill);
void    stop_fighting(Character *ch, bool fBoth);
void    update_pos(Character *victim);
void    violence_update();

// attribute.c
int stat_to_attr(int stat);
int     get_max_stat(const Character *ch, int stat);
int get_age(Character *ch);
// int get_max_hit(Character *ch);
// int get_max_mana(Character *ch);
// int get_max_stam(Character *ch);
String print_defense_modifiers(Character *ch, int where);
int get_unspelled_hitroll(Character *ch);
int get_unspelled_damroll(Character *ch);
int get_unspelled_ac(Character *ch, int type);

/* handler.c */
int     count_users(Object *obj);
int     get_weapon_type(const String& name);
bool    is_clan(Character *ch);
bool    is_same_clan(Character *ch, Character *victim);
skill::type     get_weapon_skill(Character *ch, bool secondary);
int     get_weapon_learned(Character *ch, skill::type sn);
// void    reset_char(Character *ch);
int     get_max_train(Character *ch, int stat);
int     can_carry_n(Character *ch);
int     can_carry_w(Character *ch);
void    char_from_room(Character *ch);
void    char_to_room(Character *ch, Room *room);
int     apply_ac(Object *obj, int iWear, int type);
Object *    get_eq_char(Character *ch, int iWear);
void    equip_char(Character *ch, Object *obj, int iWear);
void    unequip_char(Character *ch, Object *obj);
int     count_obj_list(ObjectPrototype *proto, Object *list);
void    obj_from_room(Object *obj);
void    obj_to_room(Object *obj, Room *room);
void    obj_to_obj(Object *obj, Object *obj_to);
void    obj_from_obj(Object *obj);
void    extract_char(Character *ch, bool fPull);
Object *    get_obj_type(ObjectPrototype *proto);
Object *    create_money(int gold, int silver);
int     get_obj_number(Object *obj);
int     get_obj_weight(Object *obj);
int     get_true_weight(Object *obj);
bool    is_room_owner(Character *ch, Room *room);
bool    is_blinded(const Character *ch);
bool    can_see_char(const Character *ch, const Character *victim);
bool    can_see_who(const Character *ch, const Character *victim);
bool    can_see_obj(const Character *ch, const Object *obj);
bool    can_see_room(const Character *ch, const Room *room);
bool    can_see_in_room(const Character *ch, const Room *room);
//const char *  first_arg(const char *argument, char *arg_first, bool fCase);
//const char *  get_who_line(Character *ch, Character *victim);
bool    mob_exists(const char *name);
//int	round(float fNum, int iInc);	/**< below is Montrey's list of */
int	parse_deity(const String& dstring);		/**< hacks!  Beware! :) */
int	get_usable_level(Character *ch);
int	get_holdable_level(Character *ch);
String	get_owner(Character *ch, Object *obj);
Character *	get_obj_carrier(Object *obj);
int	get_locker_number(Character *ch);
int	get_locker_weight(Character *ch);
int	get_strongbox_number(Character *ch);
int	get_carry_number(Character *ch);
int	get_carry_weight(Character *ch);
int	get_position(Character *ch);
int	get_play_hours(Character *ch);
int	get_play_seconds(Character *ch);
int	get_affect_evolution(Character *ch, affect::type type);
int	interpolate(int level, int value_00, int value_32);
const ExtraDescr *get_extra_descr(const String& name, const ExtraDescr *ed);

/* objstate.c */
int     objstate_load_items();
int     objstate_save_items();

/* hunt.c */
void hunt_victim(Character *ch);

/* interp.c */
void    interpret(Character *ch, String argument);
//bool    is_number(const String& arg);
bool    check_social(Character *ch, const String& command, const String& argument);
void	do_huh(Character *ch);	/**< Xenith */


/* magic.c */
skill::type    find_spell(Character *ch, const String& name);
void    obj_cast_spell(skill::type sn, int level, Character *ch, Character *victim, Object *obj);
void spell_imprint(skill::type sn, int level, Character *ch, void *vo);

/* quest.c */
void    quest_cleanup(Character *ch);
void    sq_cleanup(Character *ch);

/* save.c */
void    save_char_obj(Character *ch);
void    backup_char_obj(Character *ch);
bool    load_char_obj(Descriptor *d, const String& name);
void	update_pc_index(const Character *ch, bool remove);

/* skills.c */
void    set_learned(Character *ch, skill::type sn, int value);
void    set_evolution(Character *ch, skill::type sn, int value);
int     get_learned(const Character *ch, skill::type sn);
int     get_evolution(const Character *ch, skill::type sn);
int    exp_per_level(Character *ch, int points);
void    check_improve(Character *ch, skill::type sn, bool success, int multiplier);
int     group_lookup(const String& name);
void    gn_add(Character *ch, int gn);
void    gn_remove(Character *ch, int gn);
void    group_add(Character *ch, const String& name, bool deduct);
void    group_remove(Character *ch, const String& name);
int	    get_skill_cost(Character *ch, skill::type sn);
int     get_usable_level(skill::type sn, Guild guild);
int     get_skill_level(const Character *ch, skill::type sn);
bool	deduct_stamina(Character *ch, skill::type sn);

/* special.c */
SPEC_FUN *    spec_lookup(const String& name);
String spec_name(SPEC_FUN *function);
bool    IS_SPECIAL(Character *ch);

/* teleport.c */
//Room *   room_by_name(const char *target, int level, bool error);

/* tour.c */
//void    do_alight(Character *ch, const char *argument);
//void    do_board(Character *ch, const char *argument);
//void    load_tourstarts(FILE *fp);
//void    load_tourroutes(FILE *fp);
//void    tour_update();

/* update.c */
void    advance_level(Character *ch);
void    npc_advance_level(Character *ch);
void    demote_level(Character *ch);
void    gain_exp(Character *ch, int gain);
void    gain_condition(Character *ch, int iCond, int value);
void    update_handler();

/* remort.c */
int     raff_lookup(int index);
bool    HAS_RAFF(Character *ch, int flag);
bool    HAS_RAFF_GROUP(Character *ch, int flag);
bool    HAS_EXTRACLASS(Character *ch, skill::type sn);
bool    CAN_USE_RSKILL(Character *ch, skill::type sn);
void    list_extraskill(Character *ch);
