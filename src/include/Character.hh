#pragma once

#include "Pooled.hh"
#include "Flags.hh"
#include "Actable.hh"
#include "String.hh"
#include "Format.hh" // ptc inline
#include "Valid.hh"
#include "Guild.hh"
#include "Player.hh" // only for the macros here, remove when we get rid of them

namespace affect { class Affect; }

/*
 * One character (PC or NPC).
 */

#define MAX_ATTR                     37 // last apply (that we want to have bonus modifiers for) + 1
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

    bool has_cgroup(const Flags& cg) const;
    void add_cgroup(const Flags& cg);
    void remove_cgroup(const Flags& cg);

    bool is_npc() const { return pcdata == nullptr; }

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
    Room *   in_room = nullptr;
    Room *   was_in_room = nullptr;
    Clan *         clan = nullptr;
    Clan *         inviters = nullptr;
    Player *            pcdata = nullptr;
    String              name;
    String              short_descr;
    String              long_descr;
    String              description;
    String              prompt;
    String              prefix;
    String              reply;
    Flags               group_flags; // mobs can be members of a group for spec_fun, like ogres/trolls
    bool                replylock = false;     /* <--- made this a bool to simplify */
    bool		        invitation_accepted = false;

    int                 skill_fails = 0; // number of times they have failed a skill in a row

    /* stats */
//    int              base_stat[MAX_STATS];
//    int              mod_stat[MAX_STATS];

    int                 attr_base[MAX_ATTR] = {0};
    int *               apply_cache = nullptr; // maximum stat without eq/affects
    void *              affect_cache = nullptr;
    int *            defense_mod = nullptr;
//    long                affect_bits;
//    long      drain_flags;
//    long                imm_flags;
//    long                res_flags;
//    long                vuln_flags;

    int              hit = 0;
    int              mana = 0;
    int              stam = 0;

    Guild              guild = Guild::none; // was 'class'
    int              race = 0;
    int              level = 0;
    int                 lines = 0;  /* for the pager */
    time_t              logon = 0;
    time_t              last_bank = 0;  /* For Bank, duh */
    int              timer = 0;  /* Main timer is now part of descriptor */
    int              wait = 0;
    int              daze = 0;
    int              fightpulse = 0;
    int                gold = 0;
    int                silver = 0;
    int                gold_in_bank = 0;
    int                silver_in_bank = 0;
    int                 exp = 0;
    Flags               act_flags;
    Flags               comm_flags;   /* RT added to pad the vector */
    Flags               revoke_flags;  /* New Revoke stuff */
    Flags               wiznet_flags; /* wiz stuff */
    Flags               censor_flags;			/* New censor flags -- Montrey */
    int              invis_level = 0;
    int              lurk_level = 0;
    int              position = 0;
    int              practice = 0;
    int              train = 0;
    int              alignment = 0;
	int		        armor_base[4] = {0};
    int              wimpy = 0;
    /* parts stuff */
    Flags               form_flags;
    Flags               parts_flags;
    int              size = 0;
    String              material;
    /* mobile stuff */
    Flags               off_flags;
    int              damage[3] = {0};
    int              dam_type = 0;
    int              start_pos = 0;
    int              default_pos = 0;
    Character *         hunting = nullptr;
    int              nectimer = 0;
    int              secure_level = 0;
    MobProgActList *    mpact = nullptr;
    int                 mpactnum = 0;
    Tail *              tail = nullptr;         /* -- Elrac */
    Edit *              edit = nullptr;         /* -- Elrac */

    virtual const std::string identifier() const { return this->name; }

private:
    Character(const Character&);
    Character& operator=(const Character&);
};

// macros that should be member functions!  someday
// Character attribute accessors (see attribute.c for explanations)

#define ATTR_BASE(ch, where) ((ch)->attr_base[where]) // intentionally settable
#define GET_ATTR_MOD(ch, where)  ((ch)->apply_cache ? (ch)->apply_cache[where] : 0) // intentionally not settable
#define GET_ATTR(ch, where) (ATTR_BASE(ch, where) + GET_ATTR_MOD(ch, where)) // intentionally not settable

#define GET_ATTR_STR(ch) (URANGE(3, GET_ATTR(ch, APPLY_STR), get_max_stat(ch, STAT_STR)))
#define GET_ATTR_INT(ch) (URANGE(3, GET_ATTR(ch, APPLY_INT), get_max_stat(ch, STAT_INT)))
#define GET_ATTR_WIS(ch) (URANGE(3, GET_ATTR(ch, APPLY_WIS), get_max_stat(ch, STAT_WIS)))
#define GET_ATTR_DEX(ch) (URANGE(3, GET_ATTR(ch, APPLY_DEX), get_max_stat(ch, STAT_DEX)))
#define GET_ATTR_CON(ch) (URANGE(3, GET_ATTR(ch, APPLY_CON), get_max_stat(ch, STAT_CON)))
#define GET_ATTR_CHR(ch) (URANGE(3, GET_ATTR(ch, APPLY_CHR), get_max_stat(ch, STAT_CHR)))
#define GET_ATTR_SEX(ch) (GET_ATTR((ch), APPLY_SEX) % 3) // gives range of 0-2
#define GET_ATTR_AGE(ch) (get_age(ch))
#define GET_ATTR_AC(ch)  (GET_ATTR(ch, APPLY_AC)                              \
                        + ( IS_AWAKE(ch)                                      \
                        ? dex_app[GET_ATTR_DEX(ch)].defensive : 0 )           \
                        - (( !(ch)->is_npc() && ch->pcdata->remort_count > 0 )    \
                        ? (((ch->pcdata->remort_count * ch->level) / 50)) : 0 )) /* should give -1 per 10 levels,
                                                                                   -1 per 5 remorts -- Montrey */
#define GET_ATTR_HITROLL(ch) \
                (GET_ATTR((ch), APPLY_HITROLL) + str_app[GET_ATTR_STR((ch))].tohit)
#define GET_ATTR_DAMROLL(ch) \
                (GET_ATTR((ch), APPLY_DAMROLL) + str_app[GET_ATTR_STR((ch))].todam)
#define GET_ATTR_SAVES(ch) (GET_ATTR((ch), APPLY_SAVES))
#define GET_MAX_HIT(ch)    (URANGE(1, GET_ATTR((ch), APPLY_HIT), 30000))
#define GET_MAX_MANA(ch)   (URANGE(1, GET_ATTR((ch), APPLY_MANA), 30000))
#define GET_MAX_STAM(ch)   (URANGE(1, GET_ATTR((ch), APPLY_STAM), 30000))
#define GET_DEFENSE_MOD(ch, dam_type) (dam_type == DAM_NONE ? 0 :             \
                          (ch)->defense_mod ? (ch)->defense_mod[dam_type] : 0)
#define GET_AC(ch, type) ((ch)->armor_base[type] + GET_ATTR_AC((ch)))


/* permission checking stuff */
#define IS_HERO(ch)         (!(ch)->is_npc() && ch->level >= LEVEL_HERO)
#define IS_REMORT(ch)       (!(ch)->is_npc() && ch->pcdata->remort_count > 0)
#define IS_HEROIC(ch)       (IS_HERO(ch) || IS_REMORT(ch))
#define IS_IMM_GROUP(flags) (Flags(GROUP_GEN|GROUP_QUEST|GROUP_BUILD|GROUP_CODE|GROUP_SECURE).has_any_of(flags))
#define RANK(flags)         (IS_IMM_GROUP(flags) ?                                      \
                            (flags.has(GROUP_LEADER) ?  RANK_IMP    :                   \
                            (flags.has(GROUP_DEPUTY) ?  RANK_HEAD   : RANK_IMM))    :   \
                            (flags.has(GROUP_PLAYER) ?  RANK_MORTAL : RANK_MOBILE))
#define GET_RANK(ch)        ((ch)->is_npc() ? RANK_MOBILE : RANK(ch->pcdata->cgroup_flags))
#define IS_IMMORTAL(ch)     (GET_RANK(ch) >= RANK_IMM)
#define IS_IMP(ch)          (GET_RANK(ch) == RANK_IMP)
#define IS_HEAD(ch)         (GET_RANK(ch) >= RANK_HEAD)
#define OUTRANKS(ch, victim)    (GET_RANK(ch) > GET_RANK(victim))

/* other shortcuts */
#define IS_GOOD(ch)             (ch->alignment >= 350)
#define IS_EVIL(ch)             (ch->alignment <= -350)
#define IS_NEUTRAL(ch)          (!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)            (ch->position > POS_SLEEPING)
#define IS_OUTSIDE(ch)          (!(ch)->in_room->flags().has(ROOM_INDOORS))

#define WAIT_STATE(ch, npulse)  (!IS_IMMORTAL(ch) ? \
    (ch->wait = std::max(ch->wait, npulse)) : (ch->wait = 0))
#define DAZE_STATE(ch, npulse)  (!IS_IMMORTAL(ch) ? \
    (ch->daze = std::max(ch->daze, npulse)) : (ch->daze = 0))
#define gold_weight(amount)  ((amount) * 2 / 5)
#define silver_weight(amount) ((amount)/ 10)
#define IS_QUESTOR(ch)     (!(ch)->is_npc() && (ch)->pcdata->plr_flags.has(PLR_QUESTOR))
#define IS_SQUESTOR(ch)    (!(ch)->is_npc() && (ch)->pcdata->plr_flags.has(PLR_SQUESTOR))
#define IS_KILLER(ch)       ((ch)->act_flags.has(PLR_KILLER))
#define IS_THIEF(ch)        ((ch)->act_flags.has(PLR_THIEF))
#define CAN_FLY(ch)         (affect::exists_on_char((ch), affect::type::fly))
#define IS_FLYING(ch)       ((ch)->position >= POS_FLYING)

/*
 * Description macros.
 */
#define PERS(ch, looker, vis)   (  (vis == VIS_ALL                  \
                || (vis == VIS_CHAR && can_see_char(looker, ch))        \
                || (vis == VIS_PLR && can_see_who(looker, ch))) ?   \
                (ch)->is_npc() ? ch->short_descr : ch->name : "someone")


void    obj_to_char     args(( Object *obj, Character *ch ) );
void    obj_from_char   args(( Object *obj ) );
void    obj_to_locker   args(( Object *obj, Character *ch ) );
void    obj_to_strongbox args(( Object *obj, Character *ch ) );
void    obj_from_locker args(( Object *obj ) );
void    obj_from_strongbox args(( Object *obj) );
bool    can_drop_obj    args(( Character *ch, Object *obj ) );
bool    deduct_cost     args( (Character *ch, int cost) );

void    stc    args( ( const String& txt, Character *ch ) );
void    page_to_char    args( ( const String& txt, Character *ch ) );

// printf to a character
template<class... Params>
void ptc(Character *ch, const String& fmt, Params&&... params)
{
    stc(Format::format(fmt, params...), ch);
}
