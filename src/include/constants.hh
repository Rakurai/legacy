#pragma once

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


/* there are 2 types of invisibility, all encompassing (wizi, superwiz, lurk)
   which are outside gameplay, and gameplay invis (invis, hide, sneak, midnight)
   which are for in game use only.  The following are used in get_char functions
   to return victims the character can see in either case -- Montrey */
#define VIS_ALL		0
#define VIS_CHAR	1
#define VIS_PLR		2



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
#define MOB_VNUM_PIXIE             4250

#define MOB_VNUM_PATROLMAN         2106
#define GROUP_VNUM_TROLLS          (Flags::A) // 2100
#define GROUP_VNUM_OGRES           (Flags::B) // 2101

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


/* BAN flags */
#define BAN_SUFFIX              Flags::A
#define BAN_PREFIX              Flags::B
#define BAN_NEWBIES             Flags::C
#define BAN_ALL                 Flags::D
#define BAN_PERMIT              Flags::E

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
//#define ACT_IS_NPC              (Flags::A)             /* Auto set for mobs    */
#define ACT_NOSUMMON            (Flags::A)             // now matches plr_nosummon
#define ACT_SENTINEL            (Flags::B)             /* Stays in one room    */
#define ACT_SCAVENGER           (Flags::C)             /* Picks up objects     */
#define ACT_WHOKNOWS1           (Flags::D)             /* Unknown flag         */
#define ACT_MORPH               (Flags::E)             /* For Morphed PC's     */
#define ACT_AGGRESSIVE          (Flags::F)             /* Attacks PC's         */
#define ACT_STAY_AREA           (Flags::G)             /* Won't leave area     */
#define ACT_WIMPY               (Flags::H)
#define ACT_PET                 (Flags::I)             /* Auto set for pets    */
#define ACT_TRAIN               (Flags::J)             /* Can train PC's       */
#define ACT_PRACTICE            (Flags::K)             /* Can practice PC's    */
#define ACT_STAY                (Flags::L)
#define ACT_AGGR_ALIGN          (Flags::M)             /* Fights opp. align    */
#define ACT_SUPERMOB            (Flags::N)             /* complete invis       */
#define ACT_UNDEAD              (Flags::O)

#define ACT_CLERIC              (Flags::Q)
#define ACT_MAGE                (Flags::R)
#define ACT_THIEF               (Flags::S)
#define ACT_WARRIOR             (Flags::T)
#define ACT_NOALIGN             (Flags::U)
#define ACT_NOPURGE             (Flags::V)
#define ACT_OUTDOORS            (Flags::W)
#define ACT_INDOORS             (Flags::Y)
#define ACT_NOPUSH              (Flags::Z)
#define ACT_IS_HEALER           (Flags::a)
#define ACT_GAIN                (Flags::b)
#define ACT_NOMORPH             (Flags::c)
#define ACT_IS_CHANGER          (Flags::d)


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
#define DAM_WOOD                21
#define DAM_SILVER              22
#define DAM_IRON                23

/* OFF bits for mobiles */
#define OFF_AREA_ATTACK         (Flags::A)
#define OFF_BACKSTAB            (Flags::B)
#define OFF_BASH                (Flags::C)
#define OFF_BERSERK             (Flags::D)
#define OFF_DISARM              (Flags::E)
#define OFF_DODGE               (Flags::F)
#define OFF_FADE                (Flags::G)
#define OFF_FAST                (Flags::H)
#define OFF_KICK                (Flags::I)
#define OFF_KICK_DIRT           (Flags::J)
#define OFF_PARRY               (Flags::K)
#define OFF_RESCUE              (Flags::L)
#define OFF_TAIL                (Flags::M)
#define OFF_TRIP                (Flags::N)
#define OFF_CRUSH               (Flags::O)
#define ASSIST_ALL              (Flags::P)
#define ASSIST_ALIGN            (Flags::Q)
#define ASSIST_RACE             (Flags::R)
#define ASSIST_PLAYERS          (Flags::S)
#define ASSIST_GUARD            (Flags::T)
#define ASSIST_VNUM             (Flags::U)

/* return values for check_imm */
#define IS_VULNERABLE           0
#define IS_NORMAL               1
#define IS_RESISTANT            2
#define IS_IMMUNE               3
#define IS_ABSORBING		4

/* IMM bits for mobs */
//#define IMM_SUMMON              (Flags::A)
#define IMM_CHARM               (Flags::B)
#define IMM_MAGIC               (Flags::C)
#define IMM_WEAPON              (Flags::D)
#define IMM_BASH                (Flags::E)
#define IMM_PIERCE              (Flags::F)
#define IMM_SLASH               (Flags::G)
#define IMM_FIRE                (Flags::H)
#define IMM_COLD                (Flags::I)
#define IMM_ELECTRICITY         (Flags::J)
#define IMM_ACID                (Flags::K)
#define IMM_POISON              (Flags::L)
#define IMM_NEGATIVE            (Flags::M)
#define IMM_HOLY                (Flags::N)
#define IMM_ENERGY              (Flags::O)
#define IMM_MENTAL              (Flags::P)
#define IMM_DISEASE             (Flags::Q)
#define IMM_DROWNING            (Flags::R)
#define IMM_LIGHT               (Flags::S)
#define IMM_SOUND               (Flags::T)
#define IMM_WOOD                (Flags::X)
#define IMM_SILVER              (Flags::Y)
#define IMM_IRON                (Flags::Z)

/* RES bits for mobs */
#define RES_CHARM               (Flags::B)
#define RES_MAGIC               (Flags::C)
#define RES_WEAPON              (Flags::D)
#define RES_BASH                (Flags::E)
#define RES_PIERCE              (Flags::F)
#define RES_SLASH               (Flags::G)
#define RES_FIRE                (Flags::H)
#define RES_COLD                (Flags::I)
#define RES_ELECTRICITY         (Flags::J)
#define RES_ACID                (Flags::K)
#define RES_POISON              (Flags::L)
#define RES_NEGATIVE            (Flags::M)
#define RES_HOLY                (Flags::N)
#define RES_ENERGY              (Flags::O)
#define RES_MENTAL              (Flags::P)
#define RES_DISEASE             (Flags::Q)
#define RES_DROWNING            (Flags::R)
#define RES_LIGHT               (Flags::S)
#define RES_SOUND               (Flags::T)
#define RES_WOOD                (Flags::X)
#define RES_SILVER              (Flags::Y)
#define RES_IRON                (Flags::Z)

/* VULN bits for mobs */
#define VULN_CHARM              (Flags::B)
#define VULN_MAGIC              (Flags::C)
#define VULN_WEAPON             (Flags::D)
#define VULN_BASH               (Flags::E)
#define VULN_PIERCE             (Flags::F)
#define VULN_SLASH              (Flags::G)
#define VULN_FIRE               (Flags::H)
#define VULN_COLD               (Flags::I)
#define VULN_ELECTRICITY        (Flags::J)
#define VULN_ACID               (Flags::K)
#define VULN_POISON             (Flags::L)
#define VULN_NEGATIVE           (Flags::M)
#define VULN_HOLY               (Flags::N)
#define VULN_ENERGY             (Flags::O)
#define VULN_MENTAL             (Flags::P)
#define VULN_DISEASE            (Flags::Q)
#define VULN_DROWNING           (Flags::R)
#define VULN_LIGHT              (Flags::S)
#define VULN_SOUND              (Flags::T)
#define VULN_WOOD               (Flags::X)
#define VULN_SILVER             (Flags::Y)
#define VULN_IRON               (Flags::Z)

/* body form */
#define FORM_EDIBLE             (Flags::A)
#define FORM_POISON             (Flags::B)
#define FORM_MAGICAL            (Flags::C)
#define FORM_INSTANT_DECAY      (Flags::D)
#define FORM_OTHER              (Flags::E)  /* defined by material bit */

/* actual form */
#define FORM_ANIMAL             (Flags::G)
#define FORM_SENTIENT           (Flags::H)
#define FORM_UNDEAD             (Flags::I)
#define FORM_CONSTRUCT          (Flags::J)
#define FORM_MIST               (Flags::K)
#define FORM_INTANGIBLE         (Flags::L)

#define FORM_BIPED              (Flags::M)
#define FORM_CENTAUR            (Flags::N)
#define FORM_INSECT             (Flags::O)
#define FORM_SPIDER             (Flags::P)
#define FORM_CRUSTACEAN         (Flags::Q)
#define FORM_WORM               (Flags::R)
#define FORM_BLOB               (Flags::S)

#define FORM_MAMMAL             (Flags::V)
#define FORM_BIRD               (Flags::W)
#define FORM_REPTILE            (Flags::X)
#define FORM_SNAKE              (Flags::Y)
#define FORM_DRAGON             (Flags::Z)
#define FORM_AMPHIBIAN          (Flags::a)
#define FORM_FISH               (Flags::b)
#define FORM_COLD_BLOOD         (Flags::c)

/* body parts */
#define PART_HEAD               (Flags::A)
#define PART_ARMS               (Flags::B)
#define PART_LEGS               (Flags::C)
#define PART_HEART              (Flags::D)
#define PART_BRAINS             (Flags::E)
#define PART_GUTS               (Flags::F)
#define PART_HANDS              (Flags::G)
#define PART_FEET               (Flags::H)
#define PART_FINGERS            (Flags::I)
#define PART_EAR                (Flags::J)
#define PART_EYE                (Flags::K)
#define PART_LONG_TONGUE        (Flags::L)
#define PART_EYESTALKS          (Flags::M)
#define PART_TENTACLES          (Flags::N)
#define PART_FINS               (Flags::O)
#define PART_WINGS              (Flags::P)
#define PART_TAIL               (Flags::Q)
/* for combat */
#define PART_CLAWS              (Flags::U)
#define PART_FANGS              (Flags::V)
#define PART_HORNS              (Flags::W)
#define PART_SCALES             (Flags::X)
#define PART_TUSKS              (Flags::Y)


/*
 * Bits for 'affect_bits'.
 * Used in #MOBILES.
 */
#define AFF_BLIND               (Flags::A)
#define AFF_INVISIBLE           (Flags::B)
#define AFF_DETECT_EVIL         (Flags::C)
#define AFF_DETECT_INVIS        (Flags::D)
#define AFF_DETECT_MAGIC        (Flags::E)
#define AFF_DETECT_HIDDEN       (Flags::F)
#define AFF_DETECT_GOOD         (Flags::G)
#define AFF_SANCTUARY           (Flags::H)
#define AFF_FAERIE_FIRE         (Flags::I)
#define AFF_INFRARED            (Flags::J)
#define AFF_CURSE               (Flags::K)
#define AFF_FEAR                (Flags::L)
#define AFF_POISON              (Flags::M)
#define AFF_PROTECT_EVIL        (Flags::N)
#define AFF_PROTECT_GOOD        (Flags::O)
#define AFF_SNEAK               (Flags::P)
#define AFF_HIDE                (Flags::Q)
#define AFF_NIGHT_VISION        (Flags::R)
#define AFF_CHARM               (Flags::S)
#define AFF_FLYING              (Flags::T)
#define AFF_PASS_DOOR           (Flags::U)
#define AFF_HASTE               (Flags::V)
#define AFF_CALM                (Flags::W)
#define AFF_PLAGUE              (Flags::X)
#define AFF_STEEL               (Flags::Y)
#define AFF_DIVINEREGEN         (Flags::Z)
#define AFF_BERSERK             (Flags::a)
#define AFF_FLAMESHIELD         (Flags::b)
#define AFF_REGENERATION        (Flags::c)
#define AFF_SLOW                (Flags::d)
#define AFF_TALON               (Flags::e)


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
//#define ITEM_ROOM_KEY                31
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
#define ITEM_WARP_CRYSTAL            42
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
#define ITEM_GLOW               (Flags::A)
#define ITEM_HUM                (Flags::B)
#define ITEM_COMPARTMENT        (Flags::C)	/* replaced ITEM_DARK */
#define ITEM_TRANSPARENT        (Flags::D)	/* replaced ITEM_LOCK */
#define ITEM_EVIL               (Flags::E)
#define ITEM_INVIS              (Flags::F)
#define ITEM_MAGIC              (Flags::G)
#define ITEM_NODROP             (Flags::H)
#define ITEM_BLESS              (Flags::I)
#define ITEM_ANTI_GOOD          (Flags::J)
#define ITEM_ANTI_EVIL          (Flags::K)
#define ITEM_ANTI_NEUTRAL       (Flags::L)
#define ITEM_NOREMOVE           (Flags::M)
#define ITEM_INVENTORY          (Flags::N)
#define ITEM_NOPURGE            (Flags::O)
#define ITEM_ROT_DEATH          (Flags::P)
#define ITEM_VIS_DEATH          (Flags::Q)
#define ITEM_NOSAC              (Flags::R)
#define ITEM_NONMETAL           (Flags::S)
#define ITEM_NOLOCATE           (Flags::T)
#define ITEM_MELT_DROP          (Flags::U)
//#define ITEM_HAD_TIMER          (Flags::V)
#define ITEM_SELL_EXTRACT       (Flags::W)
#define ITEM_BURN_PROOF         (Flags::Y)
#define ITEM_NOUNCURSE          (Flags::Z)
#define ITEM_QUESTSELL          (Flags::a)


/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE               (Flags::A)
#define ITEM_WEAR_FINGER        (Flags::B)
#define ITEM_WEAR_NECK          (Flags::C)
#define ITEM_WEAR_BODY          (Flags::D)
#define ITEM_WEAR_HEAD          (Flags::E)
#define ITEM_WEAR_LEGS          (Flags::F)
#define ITEM_WEAR_FEET          (Flags::G)
#define ITEM_WEAR_HANDS         (Flags::H)
#define ITEM_WEAR_ARMS          (Flags::I)
#define ITEM_WEAR_SHIELD        (Flags::J)
#define ITEM_WEAR_ABOUT         (Flags::K)
#define ITEM_WEAR_WAIST         (Flags::L)
#define ITEM_WEAR_WRIST         (Flags::M)
#define ITEM_WIELD              (Flags::N)
#define ITEM_HOLD               (Flags::O)
#define ITEM_NO_SAC             (Flags::P)
#define ITEM_WEAR_FLOAT         (Flags::Q)
#define ITEM_WEAR_WEDDINGRING	(Flags::R)

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
#define WEAPON_FLAMING          (Flags::A)
#define WEAPON_FROST            (Flags::B)
#define WEAPON_VAMPIRIC         (Flags::C)
#define WEAPON_SHARP            (Flags::D)
#define WEAPON_VORPAL           (Flags::E)
#define WEAPON_TWO_HANDS        (Flags::F)
#define WEAPON_SHOCKING         (Flags::G)
#define WEAPON_POISON           (Flags::H)
#define WEAPON_ACIDIC           (Flags::I)

/* gate flags */
#define GATE_NORMAL_EXIT        (Flags::A)
#define GATE_NOCURSE            (Flags::B)
#define GATE_GOWITH             (Flags::C)
#define GATE_BUGGY              (Flags::D)
#define GATE_RANDOM             (Flags::E)

/* furniture flags */
#define STAND_AT                (Flags::A)
#define STAND_ON                (Flags::B)
#define STAND_IN                (Flags::C)
#define SIT_AT                  (Flags::D)
#define SIT_ON                  (Flags::E)
#define SIT_IN                  (Flags::F)
#define REST_AT                 (Flags::G)
#define REST_ON                 (Flags::H)
#define REST_IN                 (Flags::I)
#define SLEEP_AT                (Flags::J)
#define SLEEP_ON                (Flags::K)
#define SLEEP_IN                (Flags::L)
#define PUT_AT                  (Flags::M)
#define PUT_ON                  (Flags::N)
#define PUT_IN                  (Flags::O)
#define PUT_INSIDE              (Flags::P)

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
#define APPLY_EXP_PCT                16
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
#define APPLY_QUESTPOINTS			27
#define APPLY_SKILLPOINTS			28
#define APPLY_MANA_COST_PCT			29
#define APPLY_STAM_COST_PCT			30
#define APPLY_WPN_DAMAGE_PCT		31
#define APPLY_SPELL_DAMAGE_PCT		32
#define APPLY_VAMP_BONUS_PCT		33
#define APPLY_HP_BLOCK_PCT			34
#define APPLY_MANA_BLOCK_PCT		35
#define APPLY_FORGE_UNIQUE			36
#define APPLY_PRIESTESS_UNIQUE		37
#define APPLY_BREW_UNIQUE			38
#define APPLY_SCRIBE_UNIQUE			39
#define APPLY_TANK_UNIQUE			40
#define APPLY_NECRO_PIERCE_UNIQUE	41
#define APPLY_LIDDA_AURA_UNIQUE		42
#define SET_MAGE_INVOKER			43


/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE                (Flags::A)
#define CONT_PICKPROOF                (Flags::B)
#define CONT_CLOSED                   (Flags::C)
#define CONT_LOCKED                   (Flags::D)
#define CONT_PUT_ON                   (Flags::E)



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
#define ROOM_VNUM_WEST_GATE        4310


/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK               (Flags::A)
#define ROOM_NOLIGHT		(Flags::B)
#define ROOM_NO_MOB             (Flags::C)
#define ROOM_INDOORS            (Flags::D)
#define ROOM_LOCKER             (Flags::E)
#define ROOM_FEMALE_ONLY        (Flags::F)
#define ROOM_MALE_ONLY          (Flags::G)
#define ROOM_NOSLEEP		(Flags::H)
#define ROOM_NOVISION		(Flags::I)
#define ROOM_PRIVATE            (Flags::J)
#define ROOM_SAFE               (Flags::K)
#define ROOM_SOLITARY           (Flags::L)
#define ROOM_PET_SHOP           (Flags::M)
#define ROOM_NO_RECALL          (Flags::N)
#define ROOM_IMP_ONLY           (Flags::O)
#define ROOM_GODS_ONLY          (Flags::P)
#define ROOM_HEROES_ONLY        (Flags::Q)
#define ROOM_NEWBIES_ONLY       (Flags::R)
#define ROOM_LAW                (Flags::S)
#define ROOM_NOWHERE            (Flags::T)
#define ROOM_BANK               (Flags::U)
#define ROOM_LEADER_ONLY        (Flags::V)
#define ROOM_TELEPORT           (Flags::W)
#define ROOM_UNDER_WATER        (Flags::X)
#define ROOM_NOPORTAL           (Flags::Y)
#define ROOM_REMORT_ONLY        (Flags::Z)
#define ROOM_NOQUEST		(Flags::a)
#define ROOM_SILENT		(Flags::b)
#define ROOM_NORANDOMRESET	(Flags::c)

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
#define EX_ISDOOR                     (Flags::A)
#define EX_CLOSED                     (Flags::B)
#define EX_LOCKED                     (Flags::C)
#define EX_PICKPROOF                  (Flags::F)
#define EX_NOPASS                     (Flags::G)
#define EX_EASY                       (Flags::H)
#define EX_HARD                       (Flags::I)
#define EX_INFURIATING                (Flags::J)
#define EX_NOCLOSE                    (Flags::K)
#define EX_NOLOCK                     (Flags::L)



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
//#define PLR_IS_NPC              (Flags::A)             /* Don't EVER set.      */

/* RT auto flags */
#define PLR_NOSUMMON            (Flags::A)
#define PLR_LOOKINPIT           (Flags::B)
#define PLR_AUTOASSIST          (Flags::C)
#define PLR_AUTOEXIT            (Flags::D)
#define PLR_AUTOLOOT            (Flags::E)
#define PLR_AUTOSAC             (Flags::F)
#define PLR_AUTOGOLD            (Flags::G)
#define PLR_AUTOSPLIT           (Flags::H)
#define PLR_DEFENSIVE           (Flags::I)
#define PLR_WIMPY               (Flags::J)

/* RT personal flags */
#define PLR_COLOR2              (Flags::L)
#define PLR_VT100               (Flags::M)
#define PLR_MAKEBAG             (Flags::O)
#define PLR_CANLOOT             (Flags::P)
//#define PLR_NOSUMMON            (Flags::Q)  moved to A to match NPC flag
#define PLR_NOFOLLOW            (Flags::R)
#define PLR_COLOR               (Flags::S)

/* penalty flags */
#define PLR_PERMIT              (Flags::U)
#define PLR_CLOSED              (Flags::V)
#define PLR_LOG                 (Flags::W)
#define PLR_FREEZE              (Flags::Y)
#define PLR_THIEF               (Flags::Z)
#define PLR_KILLER              (Flags::a)
#define PLR_SUPERWIZ            (Flags::b)
#define PLR_TICKS               (Flags::c)
#define PLR_NOPK                (Flags::d)

/* Lotus - Extra PLR flags for only Players in pcdata->plr */
#define PLR_OOC                 (Flags::A)
#define PLR_CHATMODE            (Flags::B)
#define PLR_PRIVATE             (Flags::C)
#define PLR_STOPCRASH           (Flags::D)
#define PLR_PK                  (Flags::E)
#define PLR_QUESTOR             (Flags::F)
#define PLR_SHOWEMAIL           (Flags::G)
#define PLR_LINK_DEAD           (Flags::H)
#define PLR_PAINT               (Flags::I)
#define PLR_SNOOP_PROOF         (Flags::J)
#define PLR_NOSHOWLAST          (Flags::K)
#define PLR_NONOTIFY            (Flags::L) /* Comm flags full, pc only */
#define PLR_AUTOPEEK            (Flags::M)
#define PLR_HEEDNAME		(Flags::N)
#define PLR_SHOWLOST            (Flags::O)
#define PLR_SHOWRAFF            (Flags::Q)
#define PLR_MARRIED		(Flags::R)
#define PLR_SQUESTOR		(Flags::S)	/* Montrey */
#define PLR_DUEL_IGNORE		(Flags::T)	/* Montrey */
//#define PLR_NEWSCORE		(Flags::U)	/* Montrey */

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
#define COMM_QUIET              (Flags::A)
#define COMM_DEAF               (Flags::B)
#define COMM_NOWIZ              (Flags::C)
#define COMM_NOAUCTION          (Flags::D)
#define COMM_NOGOSSIP           (Flags::E)
#define COMM_NOQUESTION         (Flags::F)
#define COMM_NOMUSIC            (Flags::G)
#define COMM_NOCLAN             (Flags::H)
#define COMM_NOIC               (Flags::I)
#define COMM_NOANNOUNCE         (Flags::K)
#define COMM_COMPACT            (Flags::L)
#define COMM_BRIEF              (Flags::M)
#define COMM_PROMPT             (Flags::N)
#define COMM_COMBINE            (Flags::O)
#define COMM_NOFLAME            (Flags::P)
#define COMM_SHOW_AFFECTS       (Flags::Q)
#define COMM_NOGRATS            (Flags::R)
#define COMM_NOEMOTE            (Flags::T)
#define COMM_NOCHANNELS          (Flags::W) // this isn't even for players, is it needed? Montrey
#define COMM_NOSOCIAL           (Flags::X)
#define COMM_NOQUERY            (Flags::Y)
#define COMM_AFK                (Flags::Z)
#define COMM_NOQWEST            (Flags::a)
#define COMM_NOPAGE             (Flags::b)
#define COMM_NOPRAY             (Flags::c)
#define COMM_RPRAY		(Flags::d) /* Defunct - replaced by REVOKE_PRAY */
#define COMM_ATBPROMPT          (Flags::e)

/* NOchannel flags */
#define REVOKE_NOCHANNELS	(Flags::A)
#define REVOKE_FLAMEONLY	(Flags::B)
#define REVOKE_GOSSIP		(Flags::C)
#define REVOKE_FLAME            (Flags::D)
#define REVOKE_QWEST            (Flags::F)
#define REVOKE_PRAY		(Flags::G)
#define REVOKE_AUCTION          (Flags::H)
#define REVOKE_CLAN             (Flags::I)
#define REVOKE_MUSIC            (Flags::J)
#define REVOKE_QA               (Flags::K)
#define REVOKE_SOCIAL           (Flags::L)
#define REVOKE_IC               (Flags::M)
#define REVOKE_GRATS            (Flags::N)
#define REVOKE_PAGE             (Flags::Q)
#define REVOKE_QTELL            (Flags::R)
#define REVOKE_TELL		(Flags::S)
#define REVOKE_EMOTE		(Flags::T)
#define REVOKE_SAY		(Flags::U)
#define REVOKE_EXP		(Flags::V)
#define REVOKE_NOTE		(Flags::W)
#define REVOKE_NULL		(Flags::e)	/* for use with new channels code -- Montrey */

/* Command Group flags */
/* Command Group flags */
// A
#define GROUP_GEN		(Flags::B)
#define GROUP_QUEST		(Flags::C)
// D
#define GROUP_BUILD		(Flags::E)
#define GROUP_CODE		(Flags::F)
#define GROUP_SECURE	(Flags::G)
// H
#define GROUP_PLAYER	(Flags::I)		/* player only, for interp table (not set in cgroup) */
#define GROUP_MOBILE	(Flags::J)		/* mob only, for interp table (not set in cgroup) */
#define GROUP_CLAN		(Flags::K)		/* clan commands */
#define GROUP_AVATAR	(Flags::L)		/* level 80 stuff like scon */
#define GROUP_HERO		(Flags::M)		/* hero only stuff like herochat */
#define GROUP_DEPUTY	(Flags::N)		/* stuff for deputies and imm heads, makes them a deputy/head */
#define GROUP_LEADER	(Flags::O)		/* stuff for leaders and imm imps, makes them a leader/imp */

/* WIZnet flags */
#define WIZ_ON                  (Flags::A)
#define WIZ_PURGE		(Flags::B)
#define WIZ_LOGINS              (Flags::C)
#define WIZ_SITES               (Flags::D)
#define WIZ_LINKS               (Flags::E)
#define WIZ_DEATHS              (Flags::F)
#define WIZ_RESETS              (Flags::G)
#define WIZ_MOBDEATHS           (Flags::H)
#define WIZ_FLAGS               (Flags::I)
#define WIZ_PENALTIES           (Flags::J)
#define WIZ_SACCING             (Flags::K)
#define WIZ_LEVELS              (Flags::L)
#define WIZ_SECURE              (Flags::M)
#define WIZ_SWITCHES            (Flags::N)
#define WIZ_SNOOPS              (Flags::O)
#define WIZ_RESTORE             (Flags::P)
#define WIZ_LOAD                (Flags::Q)
#define WIZ_NEWBIE              (Flags::R)
#define WIZ_PREFIX              (Flags::S)
#define WIZ_SPAM                (Flags::T)
#define WIZ_MISC                (Flags::U)
#define WIZ_BUGS                (Flags::V)
#define WIZ_PROGDEBUG           (Flags::W)
// #define WIZ_OPEN                (Flags::X)
// #define WIZ_OPEN                (Flags::Y)
#define WIZ_CHEAT               (Flags::Z)
#define WIZ_MAIL                (Flags::a)
#define WIZ_AUCTION             (Flags::b)
#define WIZ_QUEST               (Flags::c)
#define WIZ_MALLOC              (Flags::d)


/* New censor flags to replace COMM_SWEARON -- Montrey */
#define CENSOR_CHAN		(Flags::A)
#define CENSOR_SPAM		(Flags::C)






/*
 * Video mode flag bits
 */
#define VIDEO_FLASH_OFF         (Flags::A)
#define VIDEO_FLASH_LINE        (Flags::B)
/* spare                        (Flags::C) */
#define VIDEO_DARK_MOD          (Flags::D)
/* spare                        (Flags::E) */
#define VIDEO_VT100             (Flags::F)
/* spare                        (Flags::G) */
#define VIDEO_CODES_SHOW        (Flags::H)


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

#define QUEST_DIR       "../quests/"
#define QUEST_LIST      QUEST_DIR "quest.lst"

#define AREA_DIR        "../area/"
#define AREA_LIST       "area.lst"  /* List of areas*/
#define CHVNUM_FILE     AREA_DIR "chvnum.txt"	/* list of changed object vnums -- Montrey (in progress) */
#define MOB_DIR         AREA_DIR "MOBProgs/"     /* MOBProg files                */
#define WORLDMAP_DIR    AREA_DIR "worldmap/"
#define WORLDMAP_IMAGE_FILE  WORLDMAP_DIR "worldmap.png"

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
