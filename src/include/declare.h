#pragma once

#define FALSE false
#define TRUE  true

typedef int sh_int;

#define args( list )                    list

// Forward declarations of all major classes on the mud.  This may disappear as
// things become less tightly coupled.
class Actable;
class Affect;
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
class ExtraDescr;
class GameTime;
class Mercenary;
class MobilePrototype;
class MobProg;
class MobProgActList;
class Note;
class Object;
class ObjectPrototype;
class Player;
class Reset;
class RoomPrototype;
class Shop;
class Social;
class StoredPlayer;
class String;
class Tail;
class War;
class Weather;

/*
 * Function types.
 */
typedef void DO_FUN     args( ( Character *ch, String argument ) );
typedef bool SPEC_FUN   args( ( Character *ch ) );
typedef void SPELL_FUN  args( ( int sn, int level, Character *ch, void *vo,
                                int target, int evolution) );

#define DECLARE_DO_FUN( fun )           DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )         SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )        SPELL_FUN fun

#define DIZZYPORT 3000

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_STATS                   6
#define MAX_RAFFECTS	           54
#define MAX_RAFFECT_SLOTS          10
#define MAX_EXTRACLASS_SLOTS        5
#define EXTRACLASS_SLOT_LEVELS     10 // number of remorts per extra extraclass slot
#define MAX_THIEF		  250
#define MAX_KILLER		  250
#define MAX_QUERY                  20
#define MAX_CLASS                   8
#define MAX_LEVEL                 100
// max items in show_list_to_char is 8000
#define MAX_DONATED              8000 // max number of items in the pit, affects do_pit performance
#define LEVEL_AVATAR				(MAX_LEVEL - 30)	/* 80 */
#define LEVEL_HERO                 (MAX_LEVEL - 9)  /*  91 */
#define LEVEL_IMMORTAL             (MAX_LEVEL - 8)  /*  92 */
#define QPS_PER_PRAC		   20
#define QPS_PER_TRAIN	           200
#define MAX_BOW_DISTANCE 3
/* hours it takes to recharge lay on hands */
#define NEW_LAY_COUNTER 12

/* 1 + Total Used ColorItems in do_color */
#define MAX_COLORS                  100

/* Color slot constants for player customizable stuff -- Elrac (Expanded by Xenith) */
#define CSLOT_CHAN_GOSSIP	0	/* default: GREEN, NOBOLD */
#define CSLOT_CHAN_FLAME	1	/* default: RED, BOLD */
#define CSLOT_CHAN_QWEST	2	/* default: YELLOW, BOLD */
#define CSLOT_CHAN_PRAY		3	/* default: RED, NOBOLD*/
#define CSLOT_CHAN_AUCTION	4	/* default: BROWN, NOBOLD */
#define CSLOT_CHAN_ANNOUNCE	5	/* default: WHITE, BOLD */
#define CSLOT_CHAN_MUSIC	6	/* default: BLUE, BOLD */
#define CSLOT_CHAN_QA		7	/* default: CYAN, BOLD */
#define CSLOT_CHAN_SOCIAL	8	/* default: GREEN, NOBOLD */
#define CSLOT_CHAN_GRATS	9	/* default: PURPLE, BOLD */
#define CSLOT_CHAN_IMM		10	/* default: RED, BOLD */
#define CSLOT_CHAN_PAGE		11	/* default: RED, NOBOLD */
#define CSLOT_CHAN_TELL		12	/* default: CYAN, BOLD */
#define CSLOT_CHAN_CLAN		13	/* default: RED, NOBOLD */
#define CSLOT_CHAN_IC		14	/* default: WHITE, BOLD */
#define CSLOT_CHAN_SAY		15	/* default: WHITE, NOBOLD */
#define CSLOT_CHAN_GTELL	16	/* default: PURPLE, BOLD */
#define CSLOT_CHAN_QTELL	17	/* default: PURPLE, BOLD */
#define CSLOT_CHAN_SPOUSE	18	/* default: YELLOW, BOLD */
					/* MAX 29 */

#define CSLOT_OLDSCORE_NAME	30	/* default: CYAN, BOLD */
#define CSLOT_OLDSCORE_PKILL	31	/* default: PURPLE, NOBOLD */
#define CSLOT_OLDSCORE_AKILL	32	/* default: PURPLE, BOLD */
#define CSLOT_OLDSCORE_CLASS	33	/* default: CYAN, NOBOLD */
#define CSLOT_OLDSCORE_STAT	34	/* default: RED, BOLD */
#define CSLOT_OLDSCORE_ARMOR	35	/* default: BLUE, BOLD */
#define CSLOT_OLDSCORE_POINTS	36	/* default: YELLOW, BOLD */
#define CSLOT_OLDSCORE_WEIGHT	37	/* default: GREEN, NOBOLD */
#define CSLOT_OLDSCORE_GAIN	38	/* default: YELLOW, NOBOLD */
#define CSLOT_OLDSCORE_ALIGN	39	/* default: WHITE, BOLD */
#define CSLOT_OLDSCORE_MONEY	40	/* default: RED, NOBOLD */
#define CSLOT_OLDSCORE_DICEROLL	41	/* default: GREEN, BOLD */
#define CSLOT_OLDSCORE_QP	42	/* default: YELLOW, BOLD */
#define CSLOT_OLDSCORE_SP	43	/* default: GREEN, BOLD */
#define CSLOT_OLDSCORE_RPP	44	/* default: PURPLE, BOLD */
					/* MAX 49 */

#define CSLOT_SCORE_TORCH	50	/* default: YELLOW, NOBOLD */
#define CSLOT_SCORE_FLAME	51	/* default: RED, BOLD */
#define CSLOT_SCORE_BORDER	52	/* default: YELLOW, NOBOLD */
#define CSLOT_SCORE_NAME	53	/* default: CYAN, BOLD */
#define CSLOT_SCORE_TITLE	54	/* default: WHITE, BOLD */
#define CSLOT_SCORE_CLASS	55	/* default: CYAN, NOBOLD */
#define CSLOT_SCORE_LEVEL	56	/* default: CYAN, NOBOLD */
#define CSLOT_SCORE_STAT	57	/* default: RED, BOLD */
#define CSLOT_SCORE_MAXSTAT	58	/* default: RED, NOBOLD */
#define CSLOT_SCORE_HEALTHNAME	59	/* default: BLACK, BOLD */
#define CSLOT_SCORE_HEALTHNUM	60	/* default: WHITE, NOBOLD */
#define CSLOT_SCORE_WEALTH	61	/* default: CYAN, BOLD */
#define CSLOT_SCORE_ENCUMB	62	/* default: GREEN, NOBOLD */
#define CSLOT_SCORE_ARMOR	63	/* default: PURPLE, BOLD */
#define CSLOT_SCORE_DICENAME	64	/* default: GREEN, BOLD */
#define CSLOT_SCORE_DICENUM	65	/* default: YELLOW, BOLD */
#define CSLOT_SCORE_POINTNAME	66	/* default: BLUE, BOLD */
#define CSLOT_SCORE_POINTNUM	67	/* default: CYAN, BOLD */
#define CSLOT_SCORE_ALIGN	68	/* default: GREY, BOLD */
#define CSLOT_SCORE_XPNAME	69	/* default: WHITE, BOLD */
#define CSLOT_SCORE_XPNUM	70	/* default: GREY, BOLD */
#define CSLOT_SCORE_POSITION	71	/* default: GREEN, NOBOLD */
#define CSLOT_SCORE_PKRECORD	72	/* default: RED, NOBOLD */
#define CSLOT_SCORE_PKRANK	73	/* default: WHITE, BOLD */
					/* MAX 79 */

#define CSLOT_MISC_ROOM		80	/* default: RED, BOLD */
#define CSLOT_MISC_MOBILES	81	/* default: GREEN, BOLD */
#define CSLOT_MISC_OBJECTS	82	/* default: YELLOW, NOBOLD */
#define CSLOT_MISC_PLAYERS	83	/* default: PURPLE, BOLD */
#define CSLOT_MISC_INV		84	/* default: BROWN, NOBOLD */
					/* MAX 99 */

#define PULSE_PER_SECOND		4
#define PULSE_VIOLENCE			(3 * PULSE_PER_SECOND)
#define PULSE_MOBILE			(4 * PULSE_PER_SECOND)
#define PULSE_MUSIC				(6 * PULSE_PER_SECOND)
#define PULSE_TICK				(45 * PULSE_PER_SECOND)
#define PULSE_AREA				(120 * PULSE_PER_SECOND)
#define PULSE_QUEST				(60 * PULSE_PER_SECOND)
#define PULSE_AUCTION			(10 * PULSE_PER_SECOND)
#define PULSE_TELEPORT			(20 * PULSE_PER_SECOND)
#define PULSE_UNDERWATER		(15 * PULSE_PER_SECOND)
#define PULSE_JANITOR			(300 * PULSE_PER_SECOND)
#define PULSE_MYSQL_UPD			(10 * PULSE_PER_SECOND)

/* Timers for the tour coaches. */
//#define PULSE_TOURHALFSTEP		(6 * PULSE_PER_SECOND)
//#define PULSE_TOURSTART			(120 * PULSE_PER_SECOND)
//#define PULSE_TOURBOARD			(30 * PULSE_PER_SECOND)

/*
  Character classes  -- Outsider
*/
#define MAGE_CLASS 0
#define CLERIC_CLASS 1
#define THIEF_CLASS 2
#define WARRIOR_CLASS 3
#define NECROMANCER_CLASS 4
#define PALADIN_CLASS 5
#define BARD_CLASS 6
#define RANGER_CLASS 7


/* there are 2 types of invisibility, all encompassing (wizi, superwiz, lurk)
   which are outside gameplay, and gameplay invis (invis, hide, sneak, midnight)
   which are for in game use only.  The following are used in get_char functions
   to return victims the character can see in either case -- Montrey */
#define VIS_ALL		0
#define VIS_CHAR	1
#define VIS_PLR		2



/*
 * Connected state for a channel.
 */
#define CON_PLAYING                      0
#define CON_GET_NAME                     1
#define CON_GET_OLD_PASSWORD             2
#define CON_CONFIRM_NEW_NAME             3
#define CON_GET_MUD_EXP			21	//
#define CON_GET_NEW_PASSWORD             4
#define CON_CONFIRM_NEW_PASSWORD         5
#define CON_GET_NEW_RACE                 6
#define CON_ROLL_STATS                   7
#define CON_GET_NEW_SEX                  8
#define CON_GET_NEW_CLASS                9
#define CON_GET_ALIGNMENT               10
#define CON_DEFAULT_CHOICE              11
#define CON_GEN_GROUPS                  12
#define CON_PICK_WEAPON                 13
#define CON_READ_IMOTD                  14
#define CON_READ_MOTD                   15
#define CON_READ_NEWMOTD                16
#define CON_BREAK_CONNECT               17
#define CON_COPYOVER_RECOVER            18
#define CON_GREETING                    19
#define CON_DEITY                       20

/*
 * TO types for act.
 */
#define TO_ROOM             0
#define TO_NOTVICT          1
#define TO_VICT             2
#define TO_CHAR             3
#define TO_ALL              4
#define TO_WORLD            5
#define TO_VIEW             6	/* for messages that need to go to the viewing room and don't normally */
#define TO_NOTVIEW          7	/* messages that never go to the viewing rooms, otherwise TO_ROOM */
#define TO_VICT_CHANNEL     8	/* special hack for channel vis, it's changed to normal vict later */

/*
 * Per-class stuff.
 */

#define STAT_STR        0
#define STAT_INT        1
#define STAT_WIS        2
#define STAT_DEX        3
#define STAT_CON        4
#define STAT_CHR        5

/*
 * Values returned by entity_argument() in interp.c
 * These represent players, mobiles, characters (either),
 * objects and rooms, respectively.
 */
#define ENTITY_P (1)
#define ENTITY_M (2)
#define ENTITY_C (ENTITY_P|ENTITY_M)
#define ENTITY_O (4)
#define ENTITY_R (8)

/* More entity types: Mobs, objects and rooms as Vnums */
#define ENTITY_VM (16)   /* Mob Vnum */
#define ENTITY_VO (32)   /* Obj Vnum */
#define ENTITY_VR (64)   /* Room Vnum */

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_FIDO              3090
#define MOB_VNUM_HELGA             3011
#define MOB_VNUM_CITYGUARD         3060
#define MOB_VNUM_VAMPIRE           3404

#define MOB_VNUM_PATROLMAN         2106
#define GROUP_VNUM_TROLLS          2100
#define GROUP_VNUM_OGRES           2101

#define MOB_VNUM_SKELETON          2
#define MOB_VNUM_ZOMBIE            3
#define MOB_VNUM_WRAITH            4
#define MOB_VNUM_GARGOYLE          5
#define MOB_VNUM_RESZOMBIE         6

#define MOB_VNUM_SQUESTMOB	   85

/*
 * the first number bold = 0,1 gives you dim or BRIGHT colors
 * the second number determines the colors as follows:
 * 30 = grey, 31 = red, 32 = green, 33 = yellow, 34 = blue, 35 = purple
 * 36 = cyan, 37 = white
 * special notes: on a MAC the colors may be screwy, 0,30 gives you black
 * 0,34 seems to be REALLY dark so it doesn't show up too well...
 */
#define NOBOLD                   0
#define BOLD                     1
#define BLINK                    5
#define REVERSE                  7
#define UNDERLINE                4

#define GREY                    30
#define RED                     31
#define GREEN                   32
#define YELLOW                  33
#define BLUE                    34
#define PURPLE                  35
#define CYAN                    36
#define WHITE                   37

/*
 * Colour stuff by Lope of Loping Through The MUD
 */
#define C_UNDERLINE       "\033[4m"
#define C_FLASH           "\033[5m"
#define C_REVERSE         "\033[7m"
#define CLEAR           "\033[0m"
#define C_BLACK         "\033[0;30m"
#define C_RED           "\033[0;31m"
#define C_GREEN         "\033[0;32m"
#define C_YELLOW        "\033[0;33m"
#define C_BLUE          "\033[0;34m"
#define C_MAGENTA       "\033[0;35m"
#define C_CYAN          "\033[0;36m"
#define C_WHITE         "\033[0;37m"
#define C_B_GREY        "\033[1;30m"
#define C_B_RED         "\033[1;31m"
#define C_B_GREEN       "\033[1;32m"
#define C_B_YELLOW      "\033[1;33m"
#define C_B_BLUE        "\033[1;34m"
#define C_B_MAGENTA     "\033[1;35m"
#define C_B_CYAN        "\033[1;36m"
#define C_B_WHITE       "\033[1;37m"
#define B_BLACK         "\033[40m"
#define B_RED           "\033[41m"
#define B_GREEN         "\033[42m"
#define B_YELLOW        "\033[43m"
#define B_BLUE          "\033[44m"
#define B_MAGENTA       "\033[45m"
#define B_CYAN          "\033[46m"
#define B_GREY          "\033[47m"


/* RT ASCII conversions -- used so we can have letters in this file */

#define BIT_A                       1
#define BIT_B                       2
#define BIT_C                       4
#define BIT_D                       8
#define BIT_E                       16
#define BIT_F                       32
#define BIT_G                       64
#define BIT_H                       128
#define BIT_I                       256
#define BIT_J                       512
#define BIT_K                       1024
#define BIT_L                       2048
#define BIT_M                       4096
#define BIT_N                       8192
#define BIT_O                       16384
#define BIT_P                       32768
#define BIT_Q                       65536
#define BIT_R                       131072
#define BIT_S                       262144
#define BIT_T                       524288
#define BIT_U                       1048576
#define BIT_V                       2097152
#define BIT_W                       4194304
#define BIT_X                       8388608
#define BIT_Y                       16777216
#define BIT_Z                       33554432
#define BIT_aa                      67108864        /* doubled due to conflicts */
#define BIT_bb                      134217728
#define BIT_cc                      268435456
#define BIT_dd                      536870912
#define BIT_ee                      1073741824
#define BIT_ff                      2147483648
#define BIT_gg                      4294967296

/* BAN flags */
#define BAN_SUFFIX              BIT_A
#define BAN_PREFIX              BIT_B
#define BAN_NEWBIES             BIT_C
#define BAN_ALL                 BIT_D
#define BAN_PERMIT              BIT_E

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
//#define ACT_IS_NPC              (BIT_A)             /* Auto set for mobs    */
#define ACT_NOSUMMON            (BIT_A)             // now matches plr_nosummon
#define ACT_SENTINEL            (BIT_B)             /* Stays in one room    */
#define ACT_SCAVENGER           (BIT_C)             /* Picks up objects     */
#define ACT_WHOKNOWS1           (BIT_D)             /* Unknown flag         */
#define ACT_MORPH               (BIT_E)             /* For Morphed PC's     */
#define ACT_AGGRESSIVE          (BIT_F)             /* Attacks PC's         */
#define ACT_STAY_AREA           (BIT_G)             /* Won't leave area     */
#define ACT_WIMPY               (BIT_H)
#define ACT_PET                 (BIT_I)             /* Auto set for pets    */
#define ACT_TRAIN               (BIT_J)             /* Can train PC's       */
#define ACT_PRACTICE            (BIT_K)             /* Can practice PC's    */
#define ACT_STAY                (BIT_L)
#define ACT_AGGR_ALIGN          (BIT_M)             /* Fights opp. align    */
#define ACT_SUPERMOB            (BIT_N)             /* complete invis       */
#define ACT_UNDEAD              (BIT_O)

#define ACT_CLERIC              (BIT_Q)
#define ACT_MAGE                (BIT_R)
#define ACT_THIEF               (BIT_S)
#define ACT_WARRIOR             (BIT_T)
#define ACT_NOALIGN             (BIT_U)
#define ACT_NOPURGE             (BIT_V)
#define ACT_OUTDOORS            (BIT_W)
#define ACT_INDOORS             (BIT_Y)
#define ACT_NOPUSH              (BIT_Z)
#define ACT_IS_HEALER           (BIT_aa)
#define ACT_GAIN                (BIT_bb)
#define ACT_NOMORPH             (BIT_cc)
#define ACT_IS_CHANGER          (BIT_dd)


/* damage classes */
#define DAM_NONE                0
#define DAM_BASH                1
#define DAM_PIERCE              2
#define DAM_SLASH               3
#define DAM_FIRE                4
#define DAM_COLD                5
#define DAM_ELECTRICITY         6
#define DAM_ACID                7
#define DAM_POISON              8
#define DAM_NEGATIVE            9
#define DAM_HOLY                10
#define DAM_ENERGY              11
#define DAM_MENTAL              12
#define DAM_DISEASE             13
#define DAM_DROWNING            14
#define DAM_LIGHT               15
#define DAM_OTHER               16
#define DAM_HARM                17
#define DAM_CHARM               18
#define DAM_SOUND               19
#define DAM_WATER               20

/* OFF bits for mobiles */
#define OFF_AREA_ATTACK         (BIT_A)
#define OFF_BACKSTAB            (BIT_B)
#define OFF_BASH                (BIT_C)
#define OFF_BERSERK             (BIT_D)
#define OFF_DISARM              (BIT_E)
#define OFF_DODGE               (BIT_F)
#define OFF_FADE                (BIT_G)
#define OFF_FAST                (BIT_H)
#define OFF_KICK                (BIT_I)
#define OFF_KICK_DIRT           (BIT_J)
#define OFF_PARRY               (BIT_K)
#define OFF_RESCUE              (BIT_L)
#define OFF_TAIL                (BIT_M)
#define OFF_TRIP                (BIT_N)
#define OFF_CRUSH               (BIT_O)
#define ASSIST_ALL              (BIT_P)
#define ASSIST_ALIGN            (BIT_Q)
#define ASSIST_RACE             (BIT_R)
#define ASSIST_PLAYERS          (BIT_S)
#define ASSIST_GUARD            (BIT_T)
#define ASSIST_VNUM             (BIT_U)

/* return values for check_imm */
#define IS_VULNERABLE           0
#define IS_NORMAL               1
#define IS_RESISTANT            2
#define IS_IMMUNE               3
#define IS_ABSORBING		4

/* IMM bits for mobs */
//#define IMM_SUMMON              (BIT_A)
#define IMM_CHARM               (BIT_B)
#define IMM_MAGIC               (BIT_C)
#define IMM_WEAPON              (BIT_D)
#define IMM_BASH                (BIT_E)
#define IMM_PIERCE              (BIT_F)
#define IMM_SLASH               (BIT_G)
#define IMM_FIRE                (BIT_H)
#define IMM_COLD                (BIT_I)
#define IMM_ELECTRICITY         (BIT_J)
#define IMM_ACID                (BIT_K)
#define IMM_POISON              (BIT_L)
#define IMM_NEGATIVE            (BIT_M)
#define IMM_HOLY                (BIT_N)
#define IMM_ENERGY              (BIT_O)
#define IMM_MENTAL              (BIT_P)
#define IMM_DISEASE             (BIT_Q)
#define IMM_DROWNING            (BIT_R)
#define IMM_LIGHT               (BIT_S)
#define IMM_SOUND               (BIT_T)
#define IMM_WOOD                (BIT_X)
#define IMM_SILVER              (BIT_Y)
#define IMM_IRON                (BIT_Z)

/* RES bits for mobs */
#define RES_CHARM               (BIT_B)
#define RES_MAGIC               (BIT_C)
#define RES_WEAPON              (BIT_D)
#define RES_BASH                (BIT_E)
#define RES_PIERCE              (BIT_F)
#define RES_SLASH               (BIT_G)
#define RES_FIRE                (BIT_H)
#define RES_COLD                (BIT_I)
#define RES_ELECTRICITY         (BIT_J)
#define RES_ACID                (BIT_K)
#define RES_POISON              (BIT_L)
#define RES_NEGATIVE            (BIT_M)
#define RES_HOLY                (BIT_N)
#define RES_ENERGY              (BIT_O)
#define RES_MENTAL              (BIT_P)
#define RES_DISEASE             (BIT_Q)
#define RES_DROWNING            (BIT_R)
#define RES_LIGHT               (BIT_S)
#define RES_SOUND               (BIT_T)
#define RES_WOOD                (BIT_X)
#define RES_SILVER              (BIT_Y)
#define RES_IRON                (BIT_Z)

/* VULN bits for mobs */
#define VULN_CHARM              (BIT_B)
#define VULN_MAGIC              (BIT_C)
#define VULN_WEAPON             (BIT_D)
#define VULN_BASH               (BIT_E)
#define VULN_PIERCE             (BIT_F)
#define VULN_SLASH              (BIT_G)
#define VULN_FIRE               (BIT_H)
#define VULN_COLD               (BIT_I)
#define VULN_ELECTRICITY        (BIT_J)
#define VULN_ACID               (BIT_K)
#define VULN_POISON             (BIT_L)
#define VULN_NEGATIVE           (BIT_M)
#define VULN_HOLY               (BIT_N)
#define VULN_ENERGY             (BIT_O)
#define VULN_MENTAL             (BIT_P)
#define VULN_DISEASE            (BIT_Q)
#define VULN_DROWNING           (BIT_R)
#define VULN_LIGHT              (BIT_S)
#define VULN_SOUND              (BIT_T)
#define VULN_WOOD               (BIT_X)
#define VULN_SILVER             (BIT_Y)
#define VULN_IRON               (BIT_Z)

/* body form */
#define FORM_EDIBLE             (BIT_A)
#define FORM_POISON             (BIT_B)
#define FORM_MAGICAL            (BIT_C)
#define FORM_INSTANT_DECAY      (BIT_D)
#define FORM_OTHER              (BIT_E)  /* defined by material bit */

/* actual form */
#define FORM_ANIMAL             (BIT_G)
#define FORM_SENTIENT           (BIT_H)
#define FORM_UNDEAD             (BIT_I)
#define FORM_CONSTRUCT          (BIT_J)
#define FORM_MIST               (BIT_K)
#define FORM_INTANGIBLE         (BIT_L)

#define FORM_BIPED              (BIT_M)
#define FORM_CENTAUR            (BIT_N)
#define FORM_INSECT             (BIT_O)
#define FORM_SPIDER             (BIT_P)
#define FORM_CRUSTACEAN         (BIT_Q)
#define FORM_WORM               (BIT_R)
#define FORM_BLOB               (BIT_S)

#define FORM_MAMMAL             (BIT_V)
#define FORM_BIRD               (BIT_W)
#define FORM_REPTILE            (BIT_X)
#define FORM_SNAKE              (BIT_Y)
#define FORM_DRAGON             (BIT_Z)
#define FORM_AMPHIBIAN          (BIT_aa)
#define FORM_FISH               (BIT_bb)
#define FORM_COLD_BLOOD         (BIT_cc)

/* body parts */
#define PART_HEAD               (BIT_A)
#define PART_ARMS               (BIT_B)
#define PART_LEGS               (BIT_C)
#define PART_HEART              (BIT_D)
#define PART_BRAINS             (BIT_E)
#define PART_GUTS               (BIT_F)
#define PART_HANDS              (BIT_G)
#define PART_FEET               (BIT_H)
#define PART_FINGERS            (BIT_I)
#define PART_EAR                (BIT_J)
#define PART_EYE                (BIT_K)
#define PART_LONG_TONGUE        (BIT_L)
#define PART_EYESTALKS          (BIT_M)
#define PART_TENTACLES          (BIT_N)
#define PART_FINS               (BIT_O)
#define PART_WINGS              (BIT_P)
#define PART_TAIL               (BIT_Q)
/* for combat */
#define PART_CLAWS              (BIT_U)
#define PART_FANGS              (BIT_V)
#define PART_HORNS              (BIT_W)
#define PART_SCALES             (BIT_X)
#define PART_TUSKS              (BIT_Y)


/*
 * Bits for 'affect_bits'.
 * Used in #MOBILES.
 */
#define AFF_BLIND               (BIT_A)
#define AFF_INVISIBLE           (BIT_B)
#define AFF_DETECT_EVIL         (BIT_C)
#define AFF_DETECT_INVIS        (BIT_D)
#define AFF_DETECT_MAGIC        (BIT_E)
#define AFF_DETECT_HIDDEN       (BIT_F)
#define AFF_DETECT_GOOD         (BIT_G)
#define AFF_SANCTUARY           (BIT_H)
#define AFF_FAERIE_FIRE         (BIT_I)
#define AFF_INFRARED            (BIT_J)
#define AFF_CURSE               (BIT_K)
#define AFF_FEAR                (BIT_L)
#define AFF_POISON              (BIT_M)
#define AFF_PROTECT_EVIL        (BIT_N)
#define AFF_PROTECT_GOOD        (BIT_O)
#define AFF_SNEAK               (BIT_P)
#define AFF_HIDE                (BIT_Q)
#define AFF_NIGHT_VISION        (BIT_R)
#define AFF_CHARM               (BIT_S)
#define AFF_FLYING              (BIT_T)
#define AFF_PASS_DOOR           (BIT_U)
#define AFF_HASTE               (BIT_V)
#define AFF_CALM                (BIT_W)
#define AFF_PLAGUE              (BIT_X)
#define AFF_STEEL               (BIT_Y)
#define AFF_DIVINEREGEN         (BIT_Z)
#define AFF_BERSERK             (BIT_aa)
#define AFF_FLAMESHIELD         (BIT_bb)
#define AFF_REGENERATION        (BIT_cc)
#define AFF_SLOW                (BIT_dd)
#define AFF_TALON               (BIT_ee)


/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL                   0
#define SEX_MALE                      1
#define SEX_FEMALE                    2

/* AC types */
#define AC_PIERCE                       0
#define AC_BASH                         1
#define AC_SLASH                        2
#define AC_EXOTIC                       3

/* dice */
#define DICE_NUMBER                     0
#define DICE_TYPE                       1
#define DICE_BONUS                      2

/* size */
#define SIZE_TINY                       0
#define SIZE_SMALL                      1
#define SIZE_MEDIUM                     2
#define SIZE_LARGE                      3
#define SIZE_HUGE                       4
#define SIZE_GIANT                      5

/* Generic objects for creating -- Montrey */
#define GEN_OBJ_TREASURE		108
#define GEN_OBJ_FOOD			119
#define GEN_OBJ_MONEY			120
#define GEN_OBJ_TOKEN			141

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_CORPSE_NPC          10
#define OBJ_VNUM_CORPSE_PC           11
#define OBJ_VNUM_SEVERED_HEAD        12
#define OBJ_VNUM_TORN_HEART          13
#define OBJ_VNUM_SLICED_ARM          14
#define OBJ_VNUM_SLICED_LEG          15
#define OBJ_VNUM_GUTS                16
#define OBJ_VNUM_BRAINS              17
#define OBJ_VNUM_DOO                 71

#define OBJ_VNUM_LIGHT_BALL          21
#define OBJ_VNUM_SPRING              22
#define OBJ_VNUM_DISC                23
#define OBJ_VNUM_PORTAL              25
#define OBJ_VNUM_TORCH             3030
#define OBJ_VNUM_NEWBIEBAG           72
#define OBJ_VNUM_WEAPON            1004
#define OBJ_VNUM_HOLYSWORD           18

#define OBJ_VNUM_SIGN              1003
#define OBJ_VNUM_CAMP              1002
#define OBJ_VNUM_VIAL              3381
#define OBJ_VNUM_PARCH             3380

#define OBJ_VNUM_PIT               3010

#define OBJ_VNUM_SCHOOL_MACE       3700
#define OBJ_VNUM_SCHOOL_DAGGER     3701
#define OBJ_VNUM_SCHOOL_SWORD      3702
#define OBJ_VNUM_SCHOOL_SPEAR      3717
#define OBJ_VNUM_SCHOOL_STAFF      3718
#define OBJ_VNUM_SCHOOL_AXE        3719
#define OBJ_VNUM_SCHOOL_FLAIL      3720
#define OBJ_VNUM_SCHOOL_WHIP       3721
#define OBJ_VNUM_SCHOOL_POLEARM    3722
#define OBJ_VNUM_SCHOOL_BOW        3727

#define OBJ_VNUM_SCHOOL_VEST       3703
#define OBJ_VNUM_SCHOOL_SHIELD     3704
#define OBJ_VNUM_SCHOOL_BANNER     3716
#define OBJ_VNUM_MAP               3162
#define OBJ_VNUM_TOKEN             3379

#define OBJ_VNUM_WHISTLE           2116

#define OBJ_VNUM_SQUESTOBJ	     85

/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT                    1
#define ITEM_SCROLL                   2
#define ITEM_WAND                     3
#define ITEM_STAFF                    4
#define ITEM_WEAPON                   5
#define ITEM_TREASURE                 8
#define ITEM_ARMOR                    9
#define ITEM_POTION                  10
#define ITEM_CLOTHING                11
#define ITEM_FURNITURE               12
#define ITEM_TRASH                   13
#define ITEM_CONTAINER               15
#define ITEM_DRINK_CON               17
#define ITEM_KEY                     18
#define ITEM_FOOD                    19
#define ITEM_MONEY                   20
#define ITEM_BOAT                    22
#define ITEM_CORPSE_NPC              23
#define ITEM_CORPSE_PC               24
#define ITEM_FOUNTAIN                25
#define ITEM_PILL                    26
#define ITEM_MAP                     28
#define ITEM_PORTAL                  29
#define ITEM_WARP_STONE              30
#define ITEM_ROOM_KEY                31
#define ITEM_GEM                     32
#define ITEM_JEWELRY                 33
#define ITEM_JUKEBOX                 34
#define ITEM_PBTUBE                  35
#define ITEM_PBGUN                   36
#define ITEM_MATERIAL                37
#define ITEM_ANVIL                   38
//#define ITEM_COACH                   39
#define ITEM_WEDDINGRING	     40
#define ITEM_TOKEN                   41
#define ITEM_QUESTSHOP              999 /* pseudo item type for quest shop -- Elrac */

/* token types
#define TOKEN_NONE		0
#define TOKEN_WILD		1
#define TOKEN_TRIVIA		2
#define TOKEN_HIDDEN		3
#define TOKEN_QUEST		4 */

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW               (BIT_A)
#define ITEM_HUM                (BIT_B)
#define ITEM_COMPARTMENT        (BIT_C)	/* replaced ITEM_DARK */
#define ITEM_TRANSPARENT        (BIT_D)	/* replaced ITEM_LOCK */
#define ITEM_EVIL               (BIT_E)
#define ITEM_INVIS              (BIT_F)
#define ITEM_MAGIC              (BIT_G)
#define ITEM_NODROP             (BIT_H)
#define ITEM_BLESS              (BIT_I)
#define ITEM_ANTI_GOOD          (BIT_J)
#define ITEM_ANTI_EVIL          (BIT_K)
#define ITEM_ANTI_NEUTRAL       (BIT_L)
#define ITEM_NOREMOVE           (BIT_M)
#define ITEM_INVENTORY          (BIT_N)
#define ITEM_NOPURGE            (BIT_O)
#define ITEM_ROT_DEATH          (BIT_P)
#define ITEM_VIS_DEATH          (BIT_Q)
#define ITEM_NOSAC              (BIT_R)
#define ITEM_NONMETAL           (BIT_S)
#define ITEM_NOLOCATE           (BIT_T)
#define ITEM_MELT_DROP          (BIT_U)
//#define ITEM_HAD_TIMER          (BIT_V)
#define ITEM_SELL_EXTRACT       (BIT_W)
#define ITEM_BURN_PROOF         (BIT_Y)
#define ITEM_NOUNCURSE          (BIT_Z)
#define ITEM_QUESTSELL          (BIT_aa)


/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE               (BIT_A)
#define ITEM_WEAR_FINGER        (BIT_B)
#define ITEM_WEAR_NECK          (BIT_C)
#define ITEM_WEAR_BODY          (BIT_D)
#define ITEM_WEAR_HEAD          (BIT_E)
#define ITEM_WEAR_LEGS          (BIT_F)
#define ITEM_WEAR_FEET          (BIT_G)
#define ITEM_WEAR_HANDS         (BIT_H)
#define ITEM_WEAR_ARMS          (BIT_I)
#define ITEM_WEAR_SHIELD        (BIT_J)
#define ITEM_WEAR_ABOUT         (BIT_K)
#define ITEM_WEAR_WAIST         (BIT_L)
#define ITEM_WEAR_WRIST         (BIT_M)
#define ITEM_WIELD              (BIT_N)
#define ITEM_HOLD               (BIT_O)
#define ITEM_NO_SAC             (BIT_P)
#define ITEM_WEAR_FLOAT         (BIT_Q)
#define ITEM_WEAR_WEDDINGRING	(BIT_R)

/* weapon class */
#define WEAPON_EXOTIC           0
#define WEAPON_SWORD            1
#define WEAPON_DAGGER           2
#define WEAPON_SPEAR            3
#define WEAPON_MACE             4
#define WEAPON_AXE              5
#define WEAPON_FLAIL            6
#define WEAPON_WHIP             7
#define WEAPON_POLEARM          8
#define WEAPON_BOW              9


/* weapon types */
#define WEAPON_FLAMING          (BIT_A)
#define WEAPON_FROST            (BIT_B)
#define WEAPON_VAMPIRIC         (BIT_C)
#define WEAPON_SHARP            (BIT_D)
#define WEAPON_VORPAL           (BIT_E)
#define WEAPON_TWO_HANDS        (BIT_F)
#define WEAPON_SHOCKING         (BIT_G)
#define WEAPON_POISON           (BIT_H)

/* gate flags */
#define GATE_NORMAL_EXIT        (BIT_A)
#define GATE_NOCURSE            (BIT_B)
#define GATE_GOWITH             (BIT_C)
#define GATE_BUGGY              (BIT_D)
#define GATE_RANDOM             (BIT_E)

/* furniture flags */
#define STAND_AT                (BIT_A)
#define STAND_ON                (BIT_B)
#define STAND_IN                (BIT_C)
#define SIT_AT                  (BIT_D)
#define SIT_ON                  (BIT_E)
#define SIT_IN                  (BIT_F)
#define REST_AT                 (BIT_G)
#define REST_ON                 (BIT_H)
#define REST_IN                 (BIT_I)
#define SLEEP_AT                (BIT_J)
#define SLEEP_ON                (BIT_K)
#define SLEEP_IN                (BIT_L)
#define PUT_AT                  (BIT_M)
#define PUT_ON                  (BIT_N)
#define PUT_IN                  (BIT_O)
#define PUT_INSIDE              (BIT_P)

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
/* 2016 revamp of stats.  These values are now used as the things that affects
   can modify, and make up the bonus vectors that determine bonuses to hp, mana,
   hitroll, etc.  Changing these numbers can have drastic affects on both player
   files and area files. -- Montrey */
#define APPLY_NONE                    0 // wasted in bonus vector, but oh well
#define APPLY_STR                     1
#define APPLY_DEX                     2
#define APPLY_INT                     3
#define APPLY_WIS                     4
#define APPLY_CON                     5
#define APPLY_SEX                     6
//#define APPLY_CLASS                   7
//#define APPLY_LEVEL                   8
#define APPLY_AGE                     9
//#define APPLY_HEIGHT                 10
//#define APPLY_WEIGHT                 11
#define APPLY_MANA                   12
#define APPLY_HIT                    13
#define APPLY_STAM                   14
#define APPLY_GOLD                   15
#define APPLY_EXP                    16
#define APPLY_AC                     17
#define APPLY_HITROLL                18
#define APPLY_DAMROLL                19
#define APPLY_SAVES                  20
#define APPLY_SAVING_PARA            20
#define APPLY_SAVING_ROD             21
#define APPLY_SAVING_PETRI           22
#define APPLY_SAVING_BREATH          23
#define APPLY_SAVING_SPELL           24
//#define APPLY_SPELL_AFFECT           25
#define APPLY_CHR                    26
//#define APPLY_SHEEN                  27
//#define APPLY_BARRIER                28
//#define APPLY_FOCUS                  29


/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE                1
#define CONT_PICKPROOF                2
#define CONT_CLOSED                   4
#define CONT_LOCKED                   8
#define CONT_PUT_ON                  16



/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO               2
#define ROOM_VNUM_CHAT             1200
#define ROOM_VNUM_TEMPLE           3001
#define ROOM_VNUM_PARK             3105
#define ROOM_VNUM_ALTAR            3054
#define ROOM_VNUM_MORGUE           1251
#define ROOM_VNUM_SCHOOL           3700
#define ROOM_VNUM_BALANCE          4500
#define ROOM_VNUM_CIRCLE           4400
#define ROOM_VNUM_DEMISE           4201
#define ROOM_VNUM_HONOR            4300
#define ROOM_VNUM_ARENACENTER       700
#define ROOM_VNUM_STRONGBOX        1269
#define ROOM_VNUM_ARENATICKET      3368
#define ROOM_VNUM_TICKETBACKROOM   3369


/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK               (BIT_A)
#define ROOM_NOLIGHT		(BIT_B)
#define ROOM_NO_MOB             (BIT_C)
#define ROOM_INDOORS            (BIT_D)
#define ROOM_LOCKER             (BIT_E)
#define ROOM_FEMALE_ONLY        (BIT_F)
#define ROOM_MALE_ONLY          (BIT_G)
#define ROOM_NOSLEEP		(BIT_H)
#define ROOM_NOVISION		(BIT_I)
#define ROOM_PRIVATE            (BIT_J)
#define ROOM_SAFE               (BIT_K)
#define ROOM_SOLITARY           (BIT_L)
#define ROOM_PET_SHOP           (BIT_M)
#define ROOM_NO_RECALL          (BIT_N)
#define ROOM_IMP_ONLY           (BIT_O)
#define ROOM_GODS_ONLY          (BIT_P)
#define ROOM_HEROES_ONLY        (BIT_Q)
#define ROOM_NEWBIES_ONLY       (BIT_R)
#define ROOM_LAW                (BIT_S)
#define ROOM_NOWHERE            (BIT_T)
#define ROOM_BANK               (BIT_U)
#define ROOM_LEADER_ONLY        (BIT_V)
#define ROOM_TELEPORT           (BIT_W)
#define ROOM_UNDER_WATER        (BIT_X)
#define ROOM_NOPORTAL           (BIT_Y)
#define ROOM_REMORT_ONLY        (BIT_Z)
#define ROOM_NOQUEST		(BIT_aa)
#define ROOM_SILENT		(BIT_bb)
#define ROOM_NORANDOMRESET	(BIT_cc)

/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH                     0
#define DIR_EAST                      1
#define DIR_SOUTH                     2
#define DIR_WEST                      3
#define DIR_UP                        4
#define DIR_DOWN                      5



/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR                     (BIT_A)
#define EX_CLOSED                     (BIT_B)
#define EX_LOCKED                     (BIT_C)
#define EX_PICKPROOF                  (BIT_F)
#define EX_NOPASS                     (BIT_G)
#define EX_EASY                       (BIT_H)
#define EX_HARD                       (BIT_I)
#define EX_INFURIATING                (BIT_J)
#define EX_NOCLOSE                    (BIT_K)
#define EX_NOLOCK                     (BIT_L)



/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE                   0
#define SECT_CITY                     1
#define SECT_FIELD                    2
#define SECT_FOREST                   3
#define SECT_HILLS                    4
#define SECT_MOUNTAIN                 5
#define SECT_WATER_SWIM               6
#define SECT_WATER_NOSWIM             7
#define SECT_UNUSED                   8
#define SECT_AIR                      9
#define SECT_DESERT                  10
#define SECT_MAX                     11
#define SECT_ARENA                   20
#define SECT_CLANARENA	             21
#define SECT_NULL                   99




/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE                    -1
#define WEAR_LIGHT                    0
#define WEAR_FINGER_L                 1
#define WEAR_FINGER_R                 2
#define WEAR_NECK_1                   3
#define WEAR_NECK_2                   4
#define WEAR_BODY                     5
#define WEAR_HEAD                     6
#define WEAR_LEGS                     7
#define WEAR_FEET                     8
#define WEAR_HANDS                    9
#define WEAR_ARMS                    10
#define WEAR_SHIELD                  11
#define WEAR_ABOUT                   12
#define WEAR_WAIST                   13
#define WEAR_WRIST_L                 14
#define WEAR_WRIST_R                 15
#define WEAR_WIELD                   16
#define WEAR_HOLD                    17
#define WEAR_FLOAT                   18
#define WEAR_SECONDARY               19
#define WEAR_WEDDINGRING	     20
#define MAX_WEAR                     21



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
#define COND_DRUNK                    0
#define COND_FULL                     1
#define COND_THIRST                   2
#define COND_HUNGER                   3



/*
 * Positions.
 */
#define POS_DEAD                      0
#define POS_MORTAL                    1
#define POS_INCAP                     2
#define POS_STUNNED                   3
#define POS_SLEEPING                  4
#define POS_RESTING                   5
#define POS_SITTING                   6
#define POS_FIGHTING                  7		/* used only on the interp table */
#define POS_STANDING                  8
#define POS_FLYING                    9

#define POS_SNEAK                    19		/* used only for movement act messages */



/*
 * ACT bits for players.
 */
//#define PLR_IS_NPC              (BIT_A)             /* Don't EVER set.      */

/* RT auto flags */
#define PLR_NOSUMMON            (BIT_A)
#define PLR_LOOKINPIT           (BIT_B)
#define PLR_AUTOASSIST          (BIT_C)
#define PLR_AUTOEXIT            (BIT_D)
#define PLR_AUTOLOOT            (BIT_E)
#define PLR_AUTOSAC             (BIT_F)
#define PLR_AUTOGOLD            (BIT_G)
#define PLR_AUTOSPLIT           (BIT_H)
#define PLR_DEFENSIVE           (BIT_I)
#define PLR_WIMPY               (BIT_J)
#define PLR_QUESTOR             (BIT_K)

/* RT personal flags */
#define PLR_COLOR2              (BIT_L)
#define PLR_VT100               (BIT_M)
#define PLR_MAKEBAG             (BIT_O)
#define PLR_CANLOOT             (BIT_P)
//#define PLR_NOSUMMON            (BIT_Q)  moved to A to match NPC flag
#define PLR_NOFOLLOW            (BIT_R)
#define PLR_COLOR               (BIT_S)

/* penalty flags */
#define PLR_PERMIT              (BIT_U)
#define PLR_CLOSED              (BIT_V)
#define PLR_LOG                 (BIT_W)
#define PLR_FREEZE              (BIT_Y)
#define PLR_THIEF               (BIT_Z)
#define PLR_KILLER              (BIT_aa)
#define PLR_SUPERWIZ            (BIT_bb)
#define PLR_TICKS               (BIT_cc)
#define PLR_NOPK                (BIT_dd)

/* Lotus - Extra PLR flags for only Players in pcdata->plr */
#define PLR_OOC                 (BIT_A)
#define PLR_CHATMODE            (BIT_B)
#define PLR_PRIVATE             (BIT_C)
#define PLR_STOPCRASH           (BIT_D)
#define PLR_PK                  (BIT_E)
#define PLR_SHOWEMAIL           (BIT_G)
#define PLR_LINK_DEAD           (BIT_H)
#define PLR_PAINT               (BIT_I)
#define PLR_SNOOP_PROOF         (BIT_J)
#define PLR_NOSHOWLAST          (BIT_K)
#define PLR_NONOTIFY            (BIT_L) /* Comm flags full, pc only */
#define PLR_AUTOPEEK            (BIT_M)
#define PLR_HEEDNAME		(BIT_N)
#define PLR_SHOWLOST            (BIT_O)
#define PLR_SHOWRAFF            (BIT_Q)
#define PLR_MARRIED		(BIT_R)
#define PLR_SQUESTOR		(BIT_S)	/* Montrey */
#define PLR_DUEL_IGNORE		(BIT_T)	/* Montrey */
//#define PLR_NEWSCORE		(BIT_U)	/* Montrey */

/* channel flags, for new channel code -- Montrey */
#define CHAN_GOSSIP		0
#define CHAN_FLAME		1
#define CHAN_QWEST		2
#define CHAN_PRAY		3
#define CHAN_CLAN		4
#define CHAN_MUSIC		5
#define CHAN_IC			6
#define CHAN_GRATS		7
#define CHAN_IMMTALK	8
#define CHAN_QA			9
#define CHAN_AUCTION	10

/* RT comm flags -- may be used on both mobs and chars */
#define COMM_QUIET              (BIT_A)
#define COMM_DEAF               (BIT_B)
#define COMM_NOWIZ              (BIT_C)
#define COMM_NOAUCTION          (BIT_D)
#define COMM_NOGOSSIP           (BIT_E)
#define COMM_NOQUESTION         (BIT_F)
#define COMM_NOMUSIC            (BIT_G)
#define COMM_NOCLAN             (BIT_H)
#define COMM_NOIC               (BIT_I)
#define COMM_NOANNOUNCE         (BIT_K)
#define COMM_COMPACT            (BIT_L)
#define COMM_BRIEF              (BIT_M)
#define COMM_PROMPT             (BIT_N)
#define COMM_COMBINE            (BIT_O)
#define COMM_NOFLAME            (BIT_P)
#define COMM_SHOW_AFFECTS       (BIT_Q)
#define COMM_NOGRATS            (BIT_R)
#define COMM_NOEMOTE            (BIT_T)
#define COMM_NOCHANNELS          (BIT_W) // this isn't even for players, is it needed? Montrey
#define COMM_NOSOCIAL           (BIT_X)
#define COMM_NOQUERY            (BIT_Y)
#define COMM_AFK                (BIT_Z)
#define COMM_NOQWEST            (BIT_aa)
#define COMM_NOPAGE             (BIT_bb)
#define COMM_NOPRAY             (BIT_cc)
#define COMM_RPRAY		(BIT_dd) /* Defunct - replaced by REVOKE_PRAY */
#define COMM_ATBPROMPT          (BIT_ee)

/* NOchannel flags */
#define REVOKE_NOCHANNELS	(BIT_A)
#define REVOKE_FLAMEONLY	(BIT_B)
#define REVOKE_GOSSIP		(BIT_C)
#define REVOKE_FLAME            (BIT_D)
#define REVOKE_QWEST            (BIT_F)
#define REVOKE_PRAY		(BIT_G)
#define REVOKE_AUCTION          (BIT_H)
#define REVOKE_CLAN             (BIT_I)
#define REVOKE_MUSIC            (BIT_J)
#define REVOKE_QA               (BIT_K)
#define REVOKE_SOCIAL           (BIT_L)
#define REVOKE_IC               (BIT_M)
#define REVOKE_GRATS            (BIT_N)
#define REVOKE_PAGE             (BIT_Q)
#define REVOKE_QTELL            (BIT_R)
#define REVOKE_TELL		(BIT_S)
#define REVOKE_EMOTE		(BIT_T)
#define REVOKE_SAY		(BIT_U)
#define REVOKE_EXP		(BIT_V)
#define REVOKE_NOTE		(BIT_W)
#define REVOKE_NULL		(BIT_ee)	/* for use with new channels code -- Montrey */

/* Command Group flags */
/* Command Group flags */
// A
#define GROUP_GEN		(BIT_B)
#define GROUP_QUEST		(BIT_C)
// D
#define GROUP_BUILD		(BIT_E)
#define GROUP_CODE		(BIT_F)
#define GROUP_SECURE	(BIT_G)
// H
#define GROUP_PLAYER	(BIT_I)		/* player only, for interp table (not set in cgroup) */
#define GROUP_MOBILE	(BIT_J)		/* mob only, for interp table (not set in cgroup) */
#define GROUP_CLAN		(BIT_K)		/* clan commands */
#define GROUP_AVATAR	(BIT_L)		/* level 80 stuff like scon */
#define GROUP_HERO		(BIT_M)		/* hero only stuff like herochat */
#define GROUP_DEPUTY	(BIT_N)		/* stuff for deputies and imm heads, makes them a deputy/head */
#define GROUP_LEADER	(BIT_O)		/* stuff for leaders and imm imps, makes them a leader/imp */

/* WIZnet flags */
#define WIZ_ON                  (BIT_A)
#define WIZ_PURGE		(BIT_B)
#define WIZ_LOGINS              (BIT_C)
#define WIZ_SITES               (BIT_D)
#define WIZ_LINKS               (BIT_E)
#define WIZ_DEATHS              (BIT_F)
#define WIZ_RESETS              (BIT_G)
#define WIZ_MOBDEATHS           (BIT_H)
#define WIZ_FLAGS               (BIT_I)
#define WIZ_PENALTIES           (BIT_J)
#define WIZ_SACCING             (BIT_K)
#define WIZ_LEVELS              (BIT_L)
#define WIZ_SECURE              (BIT_M)
#define WIZ_SWITCHES            (BIT_N)
#define WIZ_SNOOPS              (BIT_O)
#define WIZ_RESTORE             (BIT_P)
#define WIZ_LOAD                (BIT_Q)
#define WIZ_NEWBIE              (BIT_R)
#define WIZ_PREFIX              (BIT_S)
#define WIZ_SPAM                (BIT_T)
#define WIZ_MISC                (BIT_U)
#define WIZ_BUGS                (BIT_V) /* W,X,Y open */
#define WIZ_CHEAT               (BIT_Z)
#define WIZ_MAIL                (BIT_aa)
#define WIZ_AUCTION             (BIT_bb)
#define WIZ_QUEST               (BIT_cc)
#define WIZ_MALLOC              (BIT_dd)


/* New censor flags to replace COMM_SWEARON -- Montrey */
#define CENSOR_CHAN		(BIT_A)
#define CENSOR_SPAM		(BIT_C)






/*
 * Video mode flag bits
 */
#define VIDEO_FLASH_OFF         (BIT_A)
#define VIDEO_FLASH_LINE        (BIT_B)
/* spare                        (BIT_C) */
#define VIDEO_DARK_MOD          (BIT_D)
/* spare                        (BIT_E) */
#define VIDEO_VT100             (BIT_F)
/* spare                        (BIT_G) */
#define VIDEO_CODES_SHOW        (BIT_H)


#define LIQ_WATER        0

#define MAX_EQSOCKETS 6


#define AREA_TYPE_ALL   'A'
#define AREA_TYPE_CLAN  'C'
#define AREA_TYPE_IMMS  'I'
#define AREA_TYPE_HERO  'H'
#define AREA_TYPE_ARENA 'R'
#define AREA_TYPE_XXX   'X'
#define AREA_TYPE_NORM  ' '

/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000



/*
 *  Target types.
 */
#define TAR_IGNORE                  0
#define TAR_CHAR_OFFENSIVE          1
#define TAR_CHAR_DEFENSIVE          2
#define TAR_CHAR_SELF               3
#define TAR_OBJ_INV                 4
#define TAR_OBJ_CHAR_DEF            5
#define TAR_OBJ_CHAR_OFF            6
#define TAR_OBJ_HERE                7 // new, in inventory or room

#define TARGET_CHAR                 0
#define TARGET_OBJ                  1
#define TARGET_ROOM                 2
#define TARGET_NONE                 3


/* general ranks, for imm comparisons */
#define	RANK_MOBILE		0
#define	RANK_MORTAL		1
#define	RANK_IMM		2
#define	RANK_HEAD		3
#define	RANK_IMP		4

/* for command types, these are also used in some other locations as abbreviations */
#define IMP	RANK_IMP		/* implementor */
#define HED	RANK_HEAD		/* head of department */
#define IMM	RANK_IMM		/* immortal */
#define MTL	RANK_MORTAL		/* mortal - not used here, but in flag tables */

/* shorthand */
#define GP		GROUP_PLAYER
#define GM		GROUP_MOBILE
#define GC		GROUP_CLAN
#define GAV		GROUP_AVATAR
#define GH		GROUP_HERO
#define GD		GROUP_DEPUTY
#define GL		GROUP_LEADER
#define GWG		GROUP_GEN
#define GWQ		GROUP_QUEST
#define GWB		GROUP_BUILD
#define GWC		GROUP_CODE
#define GWS		GROUP_SECURE

/* event types */
#define EVENT_NULL		0
#define EVENT_WAR_START		1
#define EVENT_WAR_STOP_WIN	2
#define EVENT_WAR_STOP_IMM	3
#define EVENT_WAR_DECLARE	4
#define EVENT_WAR_JOIN		5
#define EVENT_KILL		6
#define EVENT_CLAN_DEFEAT	7
#define EVENT_CLAN_WIPEOUT	8
#define EVENT_CLAN_INVADE	9
#define EVENT_ADJUST_SCORE	10
#define EVENT_ADJUST_POWER	11
#define EVENT_ADJUST_CLANQP	12
#define EVENT_CLAN_SURRENDER	13

/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#define PLAYER_DIR      "../player/"            /* Player files */
#define TEMP_FILE       PLAYER_DIR "romtmp"
#define BACKUP_DIR      PLAYER_DIR "backup/"     /* Player files */
#define STORAGE_DIR	PLAYER_DIR "storage/"	/* Player files in storage */
#define STORAGE_FILE	STORAGE_DIR "player_list.txt" /* list of names of stored players */
#define OLDCHAR_DIR	PLAYER_DIR "oldchar/"	/* Player files wiped out by autocleanup */

#define HELP_DIR	"../help/"			/* Help file temporary storage */

#define AREA_DIR        "../area/"
#define AREA_LIST       "area.lst"  /* List of areas*/
#define CHVNUM_FILE     AREA_DIR "chvnum.txt"	/* list of changed object vnums -- Montrey (in progress) */
#define MOB_DIR         AREA_DIR "MOBProgs/"     /* MOBProg files                */

#define BIN_DIR         "../bin/"
#define EXE_FILE        BIN_DIR "legacy"

#define MISC_DIR        "../misc/"              /* Miscellaneous Stuff */
#define DB_FILE         MISC_DIR "database.sl3"
#define CONFIG_FILE     MISC_DIR "config.json"  // new config file
#define NULL_FILE       MISC_DIR "NULL_FILE"             /* To reserve one stream */
#define PID_FILE        MISC_DIR "legacy.pid"    /* pid file for the autoreboot script */
#define COPYOVER_FILE   MISC_DIR "copyover.data"
#define COPYOVER_LOG	MISC_DIR "copyover.log"  /* This is to tell who does copyovers */
#define COPYOVER_ITEMS	MISC_DIR "copyover.item"	/* saves items through copyovers */
#define BUG_FILE        MISC_DIR "bugs.txt"  /* For 'bug' and bug()*/
#define PUNISHMENT_FILE MISC_DIR "punishment.txt" /* For punishments*/
#define TYPO_FILE       MISC_DIR "typos.txt" /* For 'typo'*/
#define RIDEA_FILE      MISC_DIR "ridea.txt" /* random ideas, perm storage */
#define WBI_FILE        MISC_DIR "wbi.txt"   /* For 'will be imped' */
#define HBI_FILE        MISC_DIR "hbi.txt"   /* For 'has been imped' */
#define WBB_FILE        MISC_DIR "wbb.txt"   /* For 'will be built' */
#define HBB_FILE        MISC_DIR "hbb.txt"   /* For 'has been built' */
#define WORK_FILE	MISC_DIR "work.txt"  /* For future projects */
#define IMMAPP_FILE	MISC_DIR "immapp.txt"  /* For players to be considered for immhood */
#define DEPARTED_FILE	MISC_DIR "departed.txt"	/* departed imms files */
#define SHUTDOWN_FILE   MISC_DIR "shutdown.txt"/* For 'shutdown'*/
#define BAN_FILE        MISC_DIR "ban.txt"
#define MUSIC_FILE      MISC_DIR "music.txt"
#define EMAIL_FILE      MISC_DIR "email.txt"
#define DISABLED_FILE   MISC_DIR "disabled.txt"  /* disabled commands */
#define BOOT_FILE       MISC_DIR "bootfile"      /* flags incomplete boot */

#define NOTE_DIR        "../notes/"
#define NOTE_FILE       NOTE_DIR "notes.not" /* For 'notes'*/
#define IDEA_FILE       NOTE_DIR "ideas.not"
#define ROLEPLAY_FILE   NOTE_DIR "roleplay.not"
#define IMMQUEST_FILE   NOTE_DIR "immquest.not"
#define CHANGES_FILE    NOTE_DIR "change.not"
#define PERSONAL_FILE   NOTE_DIR "personal.not"
#define TRADE_FILE	NOTE_DIR "trade.not"

#define LOG_DIR         "../log/"
#define SLOG_FILE       LOG_DIR "slog.txt" /* Secure Logs */

/*
 * Holiday specials
 */
 #define HALLOWEEN

/* for personally owned equipment */
#define KEYWD_OWNER     "owned by"

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */

#define MSL MAX_STRING_LENGTH
#define MIL MAX_INPUT_LENGTH

/* New remort system raffect defines */
#define  RAFF_TEST			1

/* The Good... */
#define  RAFF_LIGHTFEET			2
#define  RAFF_FASTCAST			3
#define  RAFF_MAGEREGEN			4
#define  RAFF_VAMPREGEN			5
#define  RAFF_MOREEXP			6
#define  RAFF_MOREDAMAGE		7
#define  RAFF_CHEAPSPELLS		8
#define  RAFF_NOHUNGER			9
#define  RAFF_NOTHIRST			10
#define  RAFF_FAVORGOD			11
/* #define  RAFF_FREEPORTALS		14 */
/* #define  RAFF_SIXTHSENSE		24 */
/* #define  RAFF_FARSIGHT		25 */

/* The Bad... */
#define  RAFF_BUGGYREC			100
/*#define  RAFF_BUGGYCLANREC		101 */
#define  RAFF_SLOWWALK			102
#define  RAFF_CLUMSY			103
#define  RAFF_WEAKGRIP			104
#define  RAFF_SLOWCAST			105
#define  RAFF_LESSEXP			106
#define  RAFF_LESSDAMAGE		107
#define  RAFF_COSTLYSPELLS		108
#define  RAFF_EXTRAHUNGRY		109
#define  RAFF_EXTRATHIRSTY		110
#define  RAFF_LAUGHTERGOD		111
/* #define  RAFF_BADSENSE		23 */

/* And the... oh.  Vuln/Res */
#define  RAFF_VULN_SLASH		900
#define  RAFF_VULN_PIERCE		901
#define  RAFF_VULN_BLUNT		902
#define  RAFF_VULN_FIRE			903
#define  RAFF_VULN_COLD			904
#define  RAFF_VULN_ELECTRICITY		905
#define  RAFF_VULN_ACID			906
#define  RAFF_VULN_POISON		907
#define  RAFF_VULN_DISEASE		908
#define  RAFF_VULN_NEGATIVE		919
#define  RAFF_VULN_HOLY			910
#define  RAFF_VULN_ENERGY		911
#define  RAFF_VULN_MENTAL		912
#define  RAFF_VULN_DROWNING		913
#define  RAFF_VULN_LIGHT		914
#define  RAFF_VULN_SOUND		915

#define  RAFF_RES_SLASH			950
#define  RAFF_RES_PIERCE		951
#define  RAFF_RES_BLUNT			952
#define  RAFF_RES_FIRE			953
#define  RAFF_RES_COLD			954
#define  RAFF_RES_ELECTRICITY		955
#define  RAFF_RES_ACID			956
#define  RAFF_RES_POISON		957
#define  RAFF_RES_DISEASE		958
#define  RAFF_RES_NEGATIVE		959
#define  RAFF_RES_HOLY			960
#define  RAFF_RES_ENERGY		961
#define  RAFF_RES_MENTAL		962
#define  RAFF_RES_DROWNING		963
#define  RAFF_RES_LIGHT			964
#define  RAFF_RES_SOUND			965

#include "macros.h"
