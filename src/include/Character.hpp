/*
 * One character (PC or NPC).
 */

#define MAX_ATTR                     30 // last apply (that we want to have bonus modifiers for) + 1
#define MAX_ATTR_FLAG                 5 // number of bit vectors that can be added by affects
#define MAX_ATTR_VALUE            30000 // within range of 16 bit signed int

class Player;
class Customize;
class Area;
class Clan;

class Character: public Actable
{
public:
    Character() {}
    virtual ~Character() {}

    Character *         next = NULL;
    Character *         next_in_room = NULL;
    Character *         master = NULL;
    Character *         leader = NULL;
    Character *         fighting = NULL;
    Reset *	    reset = NULL;		/* let's make it keep track of what reset it */
    Character *         pet = NULL;			/* not saving at quit - Montrey */
    SPEC_FUN *          spec_fun = NULL;
    MobilePrototype *   pIndexData = NULL;
    Descriptor *        desc = NULL;
    Affect *       affected = NULL;
    Note      *         pnote = NULL;
    Object *            carrying = NULL;
    Object *            on = NULL;
    RoomPrototype *   in_room = NULL;
    RoomPrototype *   was_in_room = NULL;
    Clan *         clan = NULL;
    Clan *         inviters = NULL;
    Area *         zone = NULL;
    Player *            pcdata = NULL;
    Customize *          gen_data = NULL;
    bool                valid = FALSE;
    String              name;
    long                id = 0;
    String              short_descr;
    String              long_descr;
    String              description;
    String              prompt;
    String              prefix;
    String              reply;
    sh_int              group = 0;
    bool                replylock = FALSE;     /* <--- made this a bool to simplify */
    bool		        invitation_accepted = FALSE;

    int                 skill_fails = 0; // number of times they have failed a skill in a row

    /* stats */
//    sh_int              base_stat[MAX_STATS];
//    sh_int              mod_stat[MAX_STATS];

    int                 attr_base[MAX_ATTR] = {0};
    int *               apply_cache = NULL; // maximum stat without eq/affects
    void *              affect_cache = NULL;
    sh_int *            defense_mod = NULL;
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
    unsigned long       act = 0;
    unsigned long       comm = 0;   /* RT added to pad the vector */
    unsigned long       revoke = 0;  /* New Revoke stuff */
    unsigned long       wiznet = 0; /* wiz stuff */
    unsigned long       censor = 0;			/* New censor flags -- Montrey */
    sh_int              invis_level = 0;
    sh_int              lurk_level = 0;
    sh_int              position = 0;
    sh_int              practice = 0;
    sh_int              train = 0;
    sh_int              alignment = 0;
	sh_int		        armor_base[4] = {0};
    sh_int              wimpy = 0;
    /* parts stuff */
    unsigned long       form = 0;
    unsigned long       parts = 0;
    sh_int              size = 0;
    String              material;
    /* mobile stuff */
    unsigned long       off_flags = 0;
    sh_int              damage[3] = {0};
    sh_int              dam_type = 0;
    sh_int              start_pos = 0;
    sh_int              default_pos = 0;
    Character *         hunting = NULL;
    sh_int              nectimer = 0;
    sh_int              secure_level = 0;
    MobProgActList *    mpact = NULL;
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
    Tail *              tail = NULL;         /* -- Elrac */
    Edit *              edit = NULL;         /* -- Elrac */

    virtual std::string identifier() const { return this->name; }

private:
    Character(const Character&);
    Character& operator=(const Character&);
};
