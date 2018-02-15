#pragma once

#include "affect/Affect.hh"
#include "declare.hh"
#include "Pooled.hh"
#include "Flags.hh"
#include "Actable.hh"
#include "String.hh"
#include "Format.hh" // ptc inline
#include "Player.hh"
#include "Valid.hh"

/*
 * One character (PC or NPC).
 */

#define MAX_ATTR                     30 // last apply (that we want to have bonus modifiers for) + 1
#define MAX_ATTR_FLAG                 5 // number of bit vectors that can be added by affects
#define MAX_ATTR_VALUE            30000 // within range of 16 bit signed int

class Character :
public Pooled<Character>, // must be first inherited
public Actable,
public Valid
{
public:
    Character();
    virtual ~Character();

    void update();

    bool has_cgroup(const Flags& cg) const { return IS_NPC(this) ? false : pcdata->cgroup_flags.has_any_of(cg); }
    void add_cgroup(const Flags& cg) { if (!IS_NPC(this)) pcdata->cgroup_flags += cg; }
    void remove_cgroup(const Flags& cg) { if (!IS_NPC(this)) pcdata->cgroup_flags -= cg; }

    Character *         next = nullptr;
    Character *         next_in_room = nullptr;
    Character *         master = nullptr;
    Character *         leader = nullptr;
    Character *         fighting = nullptr;
    const Reset *	    reset = nullptr;		/* let's make it keep track of what reset it */
    Character *         pet = nullptr;			/* not saving at quit - Montrey */
    SPEC_FUN *          spec_fun = nullptr;
    MobilePrototype *   pIndexData = nullptr;
    Descriptor *        desc = nullptr;
    affect::Affect *    affected = nullptr;
    Note      *         pnote = nullptr;
    Object *            carrying = nullptr;
    Object *            on = nullptr;
    RoomPrototype *   in_room = nullptr;
    RoomPrototype *   was_in_room = nullptr;
    Clan *         clan = nullptr;
    Clan *         inviters = nullptr;
    Area *         zone = nullptr;
    Player *            pcdata = nullptr;
    String              name;
    String              short_descr;
    String              long_descr;
    String              description;
    String              prompt;
    String              prefix;
    String              reply;
    Flags               group_flags; // mobs can be members of a group for spec_fun, like ogres/trolls
    bool                replylock = FALSE;     /* <--- made this a bool to simplify */
    bool		        invitation_accepted = FALSE;

    int                 skill_fails = 0; // number of times they have failed a skill in a row

    /* stats */
//    sh_int              base_stat[MAX_STATS];
//    sh_int              mod_stat[MAX_STATS];

    int                 attr_base[MAX_ATTR] = {0};
    int *               apply_cache = nullptr; // maximum stat without eq/affects
    void *              affect_cache = nullptr;
    sh_int *            defense_mod = nullptr;
//    long                affect_bits;
//    long      drain_flags;
//    long                imm_flags;
//    long                res_flags;
//    long                vuln_flags;

    sh_int              hit = 0;
    sh_int              mana = 0;
    sh_int              stam = 0;

    sh_int              cls = 0; // was 'class'
    sh_int              race = 0;
    sh_int              level = 0;
    int                 lines = 0;  /* for the pager */
    time_t              logon = 0;
    time_t              last_bank = 0;  /* For Bank, duh */
    sh_int              timer = 0;  /* Main timer is now part of descriptor */
    sh_int              wait = 0;
    sh_int              daze = 0;
    sh_int              fightpulse = 0;
    long                gold = 0;
    long                silver = 0;
    long                gold_in_bank = 0;
    long                silver_in_bank = 0;
    int                 exp = 0;
    Flags               act_flags;
    Flags               comm_flags;   /* RT added to pad the vector */
    Flags               revoke_flags;  /* New Revoke stuff */
    Flags               wiznet_flags; /* wiz stuff */
    Flags               censor_flags;			/* New censor flags -- Montrey */
    sh_int              invis_level = 0;
    sh_int              lurk_level = 0;
    sh_int              position = 0;
    sh_int              practice = 0;
    sh_int              train = 0;
    sh_int              alignment = 0;
	sh_int		        armor_base[4] = {0};
    sh_int              wimpy = 0;
    /* parts stuff */
    Flags               form_flags;
    Flags               parts_flags;
    sh_int              size = 0;
    String              material;
    /* mobile stuff */
    Flags               off_flags;
    sh_int              damage[3] = {0};
    sh_int              dam_type = 0;
    sh_int              start_pos = 0;
    sh_int              default_pos = 0;
    Character *         hunting = nullptr;
    sh_int              nectimer = 0;
    sh_int              secure_level = 0;
    MobProgActList *    mpact = nullptr;
    int                 mpactnum = 0;
    int                 quest_giver = 0;  /* Elrac */
    int                 questpoints = 0;  /* Vassago */
    int			        questpoints_donated = 0; /* Clerve */
    long		        gold_donated = 0; /* Montrey */
    sh_int              nextquest = 0;    /* Vassago */
    sh_int              countdown = 0;    /* Vassago */
    sh_int              questobj = 0;     /* Vassago */
    sh_int              questmob = 0;     /* Vassago */
    sh_int              questloc = 0;     /* -- Elrac */
    sh_int              questobf = 0;     /* Lotus */
    Tail *              tail = nullptr;         /* -- Elrac */
    Edit *              edit = nullptr;         /* -- Elrac */

    virtual const std::string identifier() const { return this->name; }

private:
    Character(const Character&);
    Character& operator=(const Character&);
};

void    obj_to_char     args(( Object *obj, Character *ch ) );
void    obj_from_char   args(( Object *obj ) );
void    obj_to_locker   args(( Object *obj, Character *ch ) );
void    obj_to_strongbox args(( Object *obj, Character *ch ) );
void    obj_from_locker args(( Object *obj ) );
void    obj_from_strongbox args(( Object *obj) );
bool    can_drop_obj    args(( Character *ch, Object *obj ) );
bool    deduct_cost     args( (Character *ch, long cost) );

void    stc    args( ( const String& txt, Character *ch ) );
void    page_to_char    args( ( const String& txt, Character *ch ) );

// printf to a character
template<class... Params>
void ptc(Character *ch, const String& fmt, Params&&... params)
{
    stc(Format::format(fmt, params...), ch);
}
