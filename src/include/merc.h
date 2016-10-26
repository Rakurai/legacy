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

/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )                    ( )
#define DECLARE_DO_FUN( fun )           void fun( )
#define DECLARE_SPEC_FUN( fun )         bool fun( )
#define DECLARE_SPELL_FUN( fun )        void fun( )
#else
#define args( list )                    list
#define DECLARE_DO_FUN( fun )           DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )         SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )        SPELL_FUN fun
#endif

/* system calls */
int unlink();
int system();



/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if     !defined(FALSE)
#define FALSE    0
#endif

#if     !defined(TRUE)
#define TRUE     1
#endif

#if     defined(_AIX)
#if     !defined(const)
#define const
#endif
typedef int                             sh_int;
typedef int                             bool;
#define unix
#else
typedef short   int                     sh_int;
typedef unsigned char                   bool;
#endif

#define DIZZYPORT 3000

/* global access to our port number, set in comm.c */
extern int port;


/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH             1024
#define MAX_STRING_LENGTH        4608
#define MAX_INPUT_LENGTH          256
#define PAGELEN                    22

/*
 * Structure types.
 */
typedef struct  affect_data             AFFECT_DATA;
typedef struct  area_data               AREA_DATA;
typedef struct  buf_type                BUFFER;
typedef struct  char_data               CHAR_DATA;
typedef struct  descriptor_data         DESCRIPTOR_DATA;
typedef struct  exit_data               EXIT_DATA;
typedef struct  extra_descr_data        EXTRA_DESCR_DATA;
typedef struct  kill_data               KILL_DATA;
typedef struct  mem_data                MEM_DATA;
typedef struct  mob_index_data          MOB_INDEX_DATA;
typedef struct	semiperm_data			SEMIPERM;
typedef struct  note_data               NOTE_DATA;
typedef struct  obj_data                OBJ_DATA;
typedef struct  obj_index_data          OBJ_INDEX_DATA;
typedef struct  pc_data                 PC_DATA;
typedef struct  gen_data                GEN_DATA;
typedef struct  reset_data              RESET_DATA;
typedef struct  room_index_data         ROOM_INDEX_DATA;
typedef struct  shop_data               SHOP_DATA;
typedef struct  time_info_data          TIME_INFO_DATA;
typedef struct  weather_data            WEATHER_DATA;
typedef struct  battle_data             BATTLE_DATA;
typedef struct  mob_prog_data           MPROG_DATA;
typedef struct  mob_prog_act_list       MPROG_ACT_LIST;
typedef struct  auction_data            AUCTION_DATA; /* auction data */
typedef struct  disabled_data           DISABLED_DATA;
typedef struct  tail_data               TAIL_DATA;      /* -- Elrac */
typedef struct  edit_data               EDIT_DATA;      /* -- Elrac */
typedef struct  clan_type		CLAN_DATA;	/* -- Clerve */
typedef struct  departed_type		DEPARTED_DATA;	/* -- Clerve */
typedef struct  invite_type		INVITE_DATA;	/* -- Clerve */
typedef struct  storage_type		STORAGE_DATA;	/* -- Clerve */

/* War data structures -- Montrey */
typedef struct	war_data		WAR_DATA;
typedef struct	opp_data		OPP_DATA;
typedef struct	event_data		EVENT_DATA;
typedef struct	merc_data		MERC_DATA;
typedef struct	offer_data		OFFER_DATA;
typedef struct	arena_data		ARENA_DATA;
typedef struct	duel_data		DUEL_DATA;

/*
 * Function types.
 */
typedef void DO_FUN     args( ( CHAR_DATA *ch, const char *argument ) );
typedef bool SPEC_FUN   args( ( CHAR_DATA *ch ) );
typedef void SPELL_FUN  args( ( int sn, int level, CHAR_DATA *ch, void *vo,
                                int target, int evolution) );


void	bugf	(const char * fmt, ...)			__attribute__	((format(printf, 1, 2)));
//void	logf	(char * fmt, ...)			__attribute__	((format(printf, 1, 2)));
void	ptc	args((CHAR_DATA *ch, const char *fmt, ...))	__attribute__	((format(printf, 2, 3)));
int	ptb	args((BUFFER *buffer, const char *fmt, ...))	__attribute__	((format(printf, 2, 3)));


/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_RAFFECTS	           54
#define MAX_RAFFECT_SLOTS          10
#define MAX_EXTRACLASS_SLOTS        5
#define MAX_THIEF		  250
#define MAX_KILLER		  250
#define MAX_SKILL                 233   /* added critical blow */
#define MAX_GROUP                  39
#define MAX_IN_GROUP               15
#define MAX_ALIAS                  30
#define MAX_QUERY                  20
#define MAX_IGNORE                 10
#define MAX_CLASS                   8
#define MAX_PC_RACE                14
#define MAX_CLAN                   13
#define MAX_LEVEL                 100
#define MAX_EVOLUTION		    3
#define LEVEL_AVATAR				(MAX_LEVEL - 30)	/* 80 */
#define LEVEL_HERO                 (MAX_LEVEL - 9)  /*  91 */
#define LEVEL_IMMORTAL             (MAX_LEVEL - 8)  /*  92 */
#define MAX_GRANT                  32   /* number of granted commands a player can have */
#define QPS_PER_PRAC		   20
#define QPS_PER_TRAIN	           200

#define SILVER_PER_PLAYER	500000

#define MAX_DRINK                   36
#define MAX_WEAPON                  39

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


#define TITLEBLOCK                  14
#define MAX_SPELL                   10   /* Max Spells on an Object */

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

/* BAN flags */
#define BAN_SUFFIX              A
#define BAN_PREFIX              B
#define BAN_NEWBIES             C
#define BAN_ALL                 D
#define BAN_PERMIT              E

struct buf_type
{
    BUFFER *    next;
    bool        valid;
    sh_int      state;  /* error state of the buffer */
    long      size;   /* size in k */
    char *      string; /* buffer's string */
};


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


/* hours it takes to recharge lay on hands */
#define NEW_LAY_COUNTER 12


/*
 * Time and weather stuff.
 */
#define MUD_YEAR		17	/* months */
#define MUD_MONTH		35	/* days */
#define MUD_DAY			24	/* hours */
#define MUD_HOUR		45	/* seconds */

#define SUN_DARK                    0
#define SUN_RISE                    1
#define SUN_LIGHT                   2
#define SUN_SET                     3

#define SKY_CLOUDLESS               0
#define SKY_CLOUDY                  1
#define SKY_RAINING                 2
#define SKY_LIGHTNING               3

/* help file type defines */
/* here, and not in the table, is where order is important.  helps are sorted
   by precedence of their type, so imm helps will follow mortal helps, etc.
   if any changes to the order need to be made, make the change, then wipe the
   table and reload fresh, because loadhelps() will not wipe the table properly
   if numbers don't match up.  same goes for if any categories are removed */
#define	HELP_INFO		A
#define	HELP_CLAN		B
#define	HELP_SKILL		C
#define	HELP_SPELL		D
#define	HELP_RACE		E
#define	HELP_CLASS		F
#define	HELP_REMORT		G
#define	HELP_MOVE		H
#define	HELP_OBJECT		I
#define	HELP_COMM		J
#define	HELP_COMBAT		K
#define HELP_MISC		L
#define	HELP_WIZQUEST		M
#define	HELP_WIZSECURE		N
#define	HELP_WIZCODE		O
#define	HELP_WIZGEN		P
#define	HELP_WIZBUILD		Q
#define	HELP_WIZHELP		R


/* there are 2 types of invisibility, all encompassing (wizi, superwiz, lurk)
   which are outside gameplay, and gameplay invis (invis, hide, sneak, midnight)
   which are for in game use only.  The following are used in get_char functions
   to return victims the character can see in either case -- Montrey */
#define VIS_ALL		0
#define VIS_CHAR	1
#define VIS_PLR		2


/*
 * Drunk struct
 */
struct struckdrunk
{
        int     min_drunk_level;

        int     number_of_rep;
        char    *replacement[11];
};

struct  time_info_data
{
    int         hour;
    int         day;
    int         month;
    int         year;
    char *      motd;
};

struct disabled_data
{
    DISABLED_DATA *next;             /* pointer to next node */
    struct cmd_type const *command;  /* pointer to the command struct*/
    char *disabled_by;               /* name of disabler */
    char *reason;
};

struct  auction_data
{
    OBJ_DATA  * item;   /* a pointer to the item */
    CHAR_DATA * seller; /* a pointer to the seller - which may NOT quit */
    CHAR_DATA * buyer;  /* a pointer to the buyer - which may NOT quit */
    int         bet;    /* last bet - or 0 if noone has bet anything */
    sh_int      going;  /* 1,2, sold */
    sh_int      pulse;  /* how many pulses (.25 sec) until another call-out ? */
    int         min;    /* Minimum bid */
};

struct  battle_data
{
    int         low;
    int         high;
    int         fee;
    bool        issued;
    bool        start;
};

struct  weather_data
{
    int         mmhg;
    int         change;
    int         sky;
    int         sunlight;
};


/*
 * Tail (Snoop) flags
 */
#define TAIL_NONE                        0
#define TAIL_ACT                         A
#define TAIL_ALL                         TAIL_ACT

struct tail_data
{
    bool      valid;
    CHAR_DATA *tailed_by;
    char      *tailer_name;
    int       flags;
    TAIL_DATA *next;
};

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
 * Descriptor (channel) structure.
 */
struct  descriptor_data
{
    DESCRIPTOR_DATA *   next;
    DESCRIPTOR_DATA *   snoop_by;
    CHAR_DATA *         character;
    CHAR_DATA *         original;
    bool                valid;
    long                hostaddr;       /* numeric IP addr -- Elrac */
    char *              host;           /* text addr */
    sh_int              descriptor;
    sh_int              connected;
    bool                fcommand;
    char                inbuf           [4 * MAX_INPUT_LENGTH];
    char                incomm          [MAX_INPUT_LENGTH];
    char                inlast          [MAX_INPUT_LENGTH];
    int                 repeat;
    char *              outbuf;
    int                 outsize;
    int                 outtop;
    int                 port;
    int                 ip;
    char *              showstr_head;
    char *              showstr_point;
    sh_int              timer;
};



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
 * Shop types.
 */
#define MAX_TRADE        5

struct  shop_data
{
    SHOP_DATA * next;                   /* Next shop in list            */
    sh_int	version;		/* Shop versioning -- Montrey */
    sh_int      keeper;                 /* Vnum of shop keeper mob      */
    sh_int      buy_type [MAX_TRADE];   /* Item types shop will buy     */
    sh_int      profit_buy;             /* Cost multiplier for buying   */
    sh_int      profit_sell;            /* Cost multiplier for selling  */
    sh_int      open_hour;              /* First opening hour           */
    sh_int      close_hour;             /* First closing hour           */
};

/*
 * Per-class stuff.
 */

#define MAX_STATS       6
#define STAT_STR        0
#define STAT_INT        1
#define STAT_WIS        2
#define STAT_DEX        3
#define STAT_CON        4
#define STAT_CHR        5

struct deity_type
{
    char *      name;
    char *      align;
    int         value;
};

struct  class_type
{
    char *      name;                   /* the full name of the class */
    char        who_name        [4];    /* Three-letter name for 'who'  */
    sh_int      attr_prime;             /* Prime attribute              */
    sh_int      weapon;                 /* First weapon                 */
    sh_int      skill_adept;            /* Maximum skill level          */
    sh_int      thac0_00;               /* Thac0 for level  0           */
    sh_int      thac0_32;               /* Thac0 for level 32           */
    char *      base_group;             /* base skills gained           */
    char *      default_group;          /* default skills gained        */
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
    char *      name;
};

struct weapon_type
{
    char *      name;
    sh_int      vnum;
    sh_int      type;
    sh_int      *gsn;
};

struct wiznet_type
{
    char *      name;
    long        flag;
    int         level;
    char *      desc;
};

struct attack_type
{
    char *      name;                   /* name */
    char *      noun;                   /* message */
    int         damage;                 /* damage class */
};

struct race_type
{
    char *      name;                   /* call name of the race */
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
    char *      name;                   /* MUST be in race_type */
    char        who_name[4];		/* first 3 characters are used in 'who' */
    sh_int	base_age;		/* the base age for the race */
    sh_int      points;                 /* cost in points of the race */
    sh_int      class_mult[MAX_CLASS];  /* exp multiplier for class, * 100 */
    sh_int      stats[MAX_STATS];       /* starting stats */
    sh_int      max_stats[MAX_STATS];   /* maximum stats */
    sh_int      size;                   /* aff bits for the race */
    int         remort_level;           /* remort level of race */
    char *      skills[5];              /* bonus skills for the race */
};


struct spec_type
{
    char *      name;                   /* special function name */
    SPEC_FUN *  function;               /* the function */
};

struct semiperm_data
{
	SEMIPERM	*next;
	bool		valid;
	char		*string;
};

/*
 * Data structure for notes.
 */

#define NOTE_NOTE       0
#define NOTE_IDEA       1
#define NOTE_ROLEPLAY   2
#define NOTE_IMMQUEST   3
#define NOTE_CHANGES    4
#define NOTE_PERSONAL   5
#define NOTE_TRADE      6

struct  note_data
{
    NOTE_DATA * next;
    bool        valid;
    sh_int      type;
    char *      sender;
    char *      date;
    char *      to_list;
    char *      subject;
    char *      text;
    time_t      date_stamp;
};

struct board_index_struct
{
    char *board_hdr;
    NOTE_DATA **board_list;
    char *board_short;
    char *board_plural;
    char *board_long;
};


/* data structure for EDIT -- Elrac */

#define EDIT_TYPE_NONE   0
#define EDIT_TYPE_NOTE   1
#define EDIT_TYPE_DESC   2
#define EDIT_TYPE_ROOM   3
#define EDIT_TYPE_HELP   4

struct edit_data
{
    int edit_type;
    int edit_line;
    int edit_nlines;
    bool edit_undo_ok;
    char edit_string[MAX_STRING_LENGTH];
    char edit_backup[MAX_STRING_LENGTH];
    int edit_id;
};


/*
 * An affect.
 */
struct  affect_data
{
    AFFECT_DATA *       next;
    bool                valid;
    sh_int              where;
    sh_int              type;
    sh_int              level;
    sh_int              duration;
    sh_int              location;
    sh_int              modifier;
    int                 bitvector;
    sh_int		evolution;
};

/* where definitions */
#define TO_AFFECTS      0
#define TO_OBJECT       1
#define TO_IMMUNE       2
#define TO_RESIST       3
#define TO_VULN         4
#define TO_WEAPON       5
/* new definitions for room affects -- Montrey */
#define TO_ROOMFLAGS	6
#define TO_HPREGEN	7
#define TO_MPREGEN	8
/* blah, hate adding something out of order */
#define TO_DRAIN	9


/*
 * A kill structure (indexed by level).
 */
struct  kill_data
{
    sh_int              number;
    sh_int              killed;
};


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

#define CHARTYPE_TEST(chx)     (IS_NPC(chx) ? ENTITY_M : ENTITY_P)
#define CHARTYPE_MATCH(chx,ct) ((CHARTYPE_TEST(chx) & ct) != 0)


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

#define A                       1
#define B                       2
#define C                       4
#define D                       8
#define E                       16
#define F                       32
#define G                       64
#define H                       128

#define I                       256
#define J                       512
#define K                       1024
#define L                       2048
#define M                       4096
#define N                       8192
#define O                       16384
#define P                       32768

#define Q                       65536
#define R                       131072
#define S                       262144
#define T                       524288
#define U                       1048576
#define V                       2097152
#define W                       4194304
#define X                       8388608

#define Y                       16777216
#define Z                       33554432
#define aa                      67108864        /* doubled due to conflicts */
#define bb                      134217728
#define cc                      268435456
#define dd                      536870912
#define ee                      1073741824
#define ff                      2147483648
#define gg                      4294967296

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC              (A)             /* Auto set for mobs    */
#define ACT_SENTINEL            (B)             /* Stays in one room    */
#define ACT_SCAVENGER           (C)             /* Picks up objects     */
#define ACT_WHOKNOWS1           (D)             /* Unknown flag         */
#define ACT_MORPH               (E)             /* For Morphed PC's     */
#define ACT_AGGRESSIVE          (F)             /* Attacks PC's         */
#define ACT_STAY_AREA           (G)             /* Won't leave area     */
#define ACT_WIMPY               (H)
#define ACT_PET                 (I)             /* Auto set for pets    */
#define ACT_TRAIN               (J)             /* Can train PC's       */
#define ACT_PRACTICE            (K)             /* Can practice PC's    */
#define ACT_STAY                (L)
#define ACT_AGGR_ALIGN          (M)             /* Fights opp. align    */
#define ACT_SUPERMOB            (N)             /* complete invis       */
#define ACT_UNDEAD              (O)

#define ACT_CLERIC              (Q)
#define ACT_MAGE                (R)
#define ACT_THIEF               (S)
#define ACT_WARRIOR             (T)
#define ACT_NOALIGN             (U)
#define ACT_NOPURGE             (V)
#define ACT_OUTDOORS            (W)
#define ACT_INDOORS             (Y)
#define ACT_NOPUSH              (Z)
#define ACT_IS_HEALER           (aa)
#define ACT_GAIN                (bb)
#define ACT_NOMORPH             (cc)
#define ACT_IS_CHANGER          (dd)


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
#define OFF_AREA_ATTACK         (A)
#define OFF_BACKSTAB            (B)
#define OFF_BASH                (C)
#define OFF_BERSERK             (D)
#define OFF_DISARM              (E)
#define OFF_DODGE               (F)
#define OFF_FADE                (G)
#define OFF_FAST                (H)
#define OFF_KICK                (I)
#define OFF_KICK_DIRT           (J)
#define OFF_PARRY               (K)
#define OFF_RESCUE              (L)
#define OFF_TAIL                (M)
#define OFF_TRIP                (N)
#define OFF_CRUSH               (O)
#define ASSIST_ALL              (P)
#define ASSIST_ALIGN            (Q)
#define ASSIST_RACE             (R)
#define ASSIST_PLAYERS          (S)
#define ASSIST_GUARD            (T)
#define ASSIST_VNUM             (U)

/* return values for check_imm */
#define IS_VULNERABLE           0
#define IS_NORMAL               1
#define IS_RESISTANT            2
#define IS_IMMUNE               3
#define IS_DRAINING		4

/* DRAIN bits */
#define DRAIN_SUMMON              (A)
#define DRAIN_CHARM               (B)
#define DRAIN_MAGIC               (C)
#define DRAIN_WEAPON              (D)
#define DRAIN_BASH                (E)
#define DRAIN_PIERCE              (F)
#define DRAIN_SLASH               (G)
#define DRAIN_FIRE                (H)
#define DRAIN_COLD                (I)
#define DRAIN_ELECTRICITY         (J)
#define DRAIN_ACID                (K)
#define DRAIN_POISON              (L)
#define DRAIN_NEGATIVE            (M)
#define DRAIN_HOLY                (N)
#define DRAIN_ENERGY              (O)
#define DRAIN_MENTAL              (P)
#define DRAIN_DISEASE             (Q)
#define DRAIN_DROWNING            (R)
#define DRAIN_LIGHT               (S)
#define DRAIN_SOUND               (T)
#define DRAIN_SHADOW              (U)             /* Immune to Shadow Form */
#define DRAIN_WOOD                (X)
#define DRAIN_SILVER              (Y)
#define DRAIN_IRON                (Z)

/* IMM bits for mobs */
#define IMM_SUMMON              (A)
#define IMM_CHARM               (B)
#define IMM_MAGIC               (C)
#define IMM_WEAPON              (D)
#define IMM_BASH                (E)
#define IMM_PIERCE              (F)
#define IMM_SLASH               (G)
#define IMM_FIRE                (H)
#define IMM_COLD                (I)
#define IMM_ELECTRICITY         (J)
#define IMM_ACID                (K)
#define IMM_POISON              (L)
#define IMM_NEGATIVE            (M)
#define IMM_HOLY                (N)
#define IMM_ENERGY              (O)
#define IMM_MENTAL              (P)
#define IMM_DISEASE             (Q)
#define IMM_DROWNING            (R)
#define IMM_LIGHT               (S)
#define IMM_SOUND               (T)
#define IMM_SHADOW              (U)             /* Immune to Shadow Form */
#define IMM_WOOD                (X)
#define IMM_SILVER              (Y)
#define IMM_IRON                (Z)

/* RES bits for mobs */
#define RES_SUMMON              (A)
#define RES_CHARM               (B)
#define RES_MAGIC               (C)
#define RES_WEAPON              (D)
#define RES_BASH                (E)
#define RES_PIERCE              (F)
#define RES_SLASH               (G)
#define RES_FIRE                (H)
#define RES_COLD                (I)
#define RES_ELECTRICITY         (J)
#define RES_ACID                (K)
#define RES_POISON              (L)
#define RES_NEGATIVE            (M)
#define RES_HOLY                (N)
#define RES_ENERGY              (O)
#define RES_MENTAL              (P)
#define RES_DISEASE             (Q)
#define RES_DROWNING            (R)
#define RES_LIGHT               (S)
#define RES_SOUND               (T)
#define RES_WOOD                (X)
#define RES_SILVER              (Y)
#define RES_IRON                (Z)

/* VULN bits for mobs */
#define VULN_SUMMON             (A)
#define VULN_CHARM              (B)
#define VULN_MAGIC              (C)
#define VULN_WEAPON             (D)
#define VULN_BASH               (E)
#define VULN_PIERCE             (F)
#define VULN_SLASH              (G)
#define VULN_FIRE               (H)
#define VULN_COLD               (I)
#define VULN_ELECTRICITY        (J)
#define VULN_ACID               (K)
#define VULN_POISON             (L)
#define VULN_NEGATIVE           (M)
#define VULN_HOLY               (N)
#define VULN_ENERGY             (O)
#define VULN_MENTAL             (P)
#define VULN_DISEASE            (Q)
#define VULN_DROWNING           (R)
#define VULN_LIGHT              (S)
#define VULN_SOUND              (T)
#define VULN_WOOD               (X)
#define VULN_SILVER             (Y)
#define VULN_IRON               (Z)

/* body form */
#define FORM_EDIBLE             (A)
#define FORM_POISON             (B)
#define FORM_MAGICAL            (C)
#define FORM_INSTANT_DECAY      (D)
#define FORM_OTHER              (E)  /* defined by material bit */

/* actual form */
#define FORM_ANIMAL             (G)
#define FORM_SENTIENT           (H)
#define FORM_UNDEAD             (I)
#define FORM_CONSTRUCT          (J)
#define FORM_MIST               (K)
#define FORM_INTANGIBLE         (L)

#define FORM_BIPED              (M)
#define FORM_CENTAUR            (N)
#define FORM_INSECT             (O)
#define FORM_SPIDER             (P)
#define FORM_CRUSTACEAN         (Q)
#define FORM_WORM               (R)
#define FORM_BLOB               (S)

#define FORM_MAMMAL             (V)
#define FORM_BIRD               (W)
#define FORM_REPTILE            (X)
#define FORM_SNAKE              (Y)
#define FORM_DRAGON             (Z)
#define FORM_AMPHIBIAN          (aa)
#define FORM_FISH               (bb)
#define FORM_COLD_BLOOD         (cc)

/* body parts */
#define PART_HEAD               (A)
#define PART_ARMS               (B)
#define PART_LEGS               (C)
#define PART_HEART              (D)
#define PART_BRAINS             (E)
#define PART_GUTS               (F)
#define PART_HANDS              (G)
#define PART_FEET               (H)
#define PART_FINGERS            (I)
#define PART_EAR                (J)
#define PART_EYE                (K)
#define PART_LONG_TONGUE        (L)
#define PART_EYESTALKS          (M)
#define PART_TENTACLES          (N)
#define PART_FINS               (O)
#define PART_WINGS              (P)
#define PART_TAIL               (Q)
/* for combat */
#define PART_CLAWS              (U)
#define PART_FANGS              (V)
#define PART_HORNS              (W)
#define PART_SCALES             (X)
#define PART_TUSKS              (Y)


/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND               (A)
#define AFF_INVISIBLE           (B)
#define AFF_DETECT_EVIL         (C)
#define AFF_DETECT_INVIS        (D)
#define AFF_DETECT_MAGIC        (E)
#define AFF_DETECT_HIDDEN       (F)
#define AFF_DETECT_GOOD         (G)
#define AFF_SANCTUARY           (H)
#define AFF_FAERIE_FIRE         (I)
#define AFF_INFRARED            (J)
#define AFF_CURSE               (K)
#define AFF_FEAR                (L)
#define AFF_POISON              (M)
#define AFF_PROTECT_EVIL        (N)
#define AFF_PROTECT_GOOD        (O)
#define AFF_SNEAK               (P)
#define AFF_HIDE                (Q)
#define AFF_NIGHT_VISION        (R)
#define AFF_CHARM               (S)
#define AFF_FLYING              (T)
#define AFF_PASS_DOOR           (U)
#define AFF_HASTE               (V)
#define AFF_CALM                (W)
#define AFF_PLAGUE              (X)
#define AFF_STEEL               (Y)
#define AFF_DIVINEREGEN         (Z)
#define AFF_BERSERK             (aa)
#define AFF_FLAMESHIELD         (bb)
#define AFF_REGENERATION        (cc)
#define AFF_SLOW                (dd)
#define AFF_TALON               (ee)


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

#define IS_QUESTSHOPKEEPER(mob) ((mob->pIndexData->pShop != NULL) && (mob->pIndexData->pShop->buy_type[0] == ITEM_QUESTSHOP))

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW               (A)
#define ITEM_HUM                (B)
#define ITEM_COMPARTMENT        (C)	/* replaced ITEM_DARK */
#define ITEM_TRANSPARENT        (D)	/* replaced ITEM_LOCK */
#define ITEM_EVIL               (E)
#define ITEM_INVIS              (F)
#define ITEM_MAGIC              (G)
#define ITEM_NODROP             (H)
#define ITEM_BLESS              (I)
#define ITEM_ANTI_GOOD          (J)
#define ITEM_ANTI_EVIL          (K)
#define ITEM_ANTI_NEUTRAL       (L)
#define ITEM_NOREMOVE           (M)
#define ITEM_INVENTORY          (N)
#define ITEM_NOPURGE            (O)
#define ITEM_ROT_DEATH          (P)
#define ITEM_VIS_DEATH          (Q)
#define ITEM_NOSAC              (R)
#define ITEM_NONMETAL           (S)
#define ITEM_NOLOCATE           (T)
#define ITEM_MELT_DROP          (U)
//#define ITEM_HAD_TIMER          (V)
#define ITEM_SELL_EXTRACT       (W)
#define ITEM_BURN_PROOF         (Y)
#define ITEM_NOUNCURSE          (Z)
#define ITEM_QUESTSELL          (aa)


/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE               (A)
#define ITEM_WEAR_FINGER        (B)
#define ITEM_WEAR_NECK          (C)
#define ITEM_WEAR_BODY          (D)
#define ITEM_WEAR_HEAD          (E)
#define ITEM_WEAR_LEGS          (F)
#define ITEM_WEAR_FEET          (G)
#define ITEM_WEAR_HANDS         (H)
#define ITEM_WEAR_ARMS          (I)
#define ITEM_WEAR_SHIELD        (J)
#define ITEM_WEAR_ABOUT         (K)
#define ITEM_WEAR_WAIST         (L)
#define ITEM_WEAR_WRIST         (M)
#define ITEM_WIELD              (N)
#define ITEM_HOLD               (O)
#define ITEM_NO_SAC             (P)
#define ITEM_WEAR_FLOAT         (Q)
#define ITEM_WEAR_WEDDINGRING	(R)

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
#define WEAPON_FLAMING          (A)
#define WEAPON_FROST            (B)
#define WEAPON_VAMPIRIC         (C)
#define WEAPON_SHARP            (D)
#define WEAPON_VORPAL           (E)
#define WEAPON_TWO_HANDS        (F)
#define WEAPON_SHOCKING         (G)
#define WEAPON_POISON           (H)

/* gate flags */
#define GATE_NORMAL_EXIT        (A)
#define GATE_NOCURSE            (B)
#define GATE_GOWITH             (C)
#define GATE_BUGGY              (D)
#define GATE_RANDOM             (E)

/* furniture flags */
#define STAND_AT                (A)
#define STAND_ON                (B)
#define STAND_IN                (C)
#define SIT_AT                  (D)
#define SIT_ON                  (E)
#define SIT_IN                  (F)
#define REST_AT                 (G)
#define REST_ON                 (H)
#define REST_IN                 (I)
#define SLEEP_AT                (J)
#define SLEEP_ON                (K)
#define SLEEP_IN                (L)
#define PUT_AT                  (M)
#define PUT_ON                  (N)
#define PUT_IN                  (O)
#define PUT_INSIDE              (P)

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE                    0
#define APPLY_STR                     1
#define APPLY_DEX                     2
#define APPLY_INT                     3
#define APPLY_WIS                     4
#define APPLY_CON                     5
#define APPLY_SEX                     6
#define APPLY_CLASS                   7
#define APPLY_LEVEL                   8
#define APPLY_AGE                     9
#define APPLY_HEIGHT                 10
#define APPLY_WEIGHT                 11
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
#define APPLY_SPELL_AFFECT           25
#define APPLY_CHR                    26
#define APPLY_SHEEN                  27
#define APPLY_BARRIER                28
#define APPLY_FOCUS                  29


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
#define ROOM_DARK               (A)
#define ROOM_NOLIGHT		(B)
#define ROOM_NO_MOB             (C)
#define ROOM_INDOORS            (D)
#define ROOM_LOCKER             (E)
#define ROOM_FEMALE_ONLY        (F)
#define ROOM_MALE_ONLY          (G)
#define ROOM_NOSLEEP		(H)
#define ROOM_NOVISION		(I)
#define ROOM_PRIVATE            (J)
#define ROOM_SAFE               (K)
#define ROOM_SOLITARY           (L)
#define ROOM_PET_SHOP           (M)
#define ROOM_NO_RECALL          (N)
#define ROOM_IMP_ONLY           (O)
#define ROOM_GODS_ONLY          (P)
#define ROOM_HEROES_ONLY        (Q)
#define ROOM_NEWBIES_ONLY       (R)
#define ROOM_LAW                (S)
#define ROOM_NOWHERE            (T)
#define ROOM_BANK               (U)
#define ROOM_LEADER_ONLY        (V)
#define ROOM_TELEPORT           (W)
#define ROOM_UNDER_WATER        (X)
#define ROOM_NOPORTAL           (Y)
#define ROOM_REMORT_ONLY        (Z)
#define ROOM_NOQUEST		(aa)
#define ROOM_SILENT		(bb)
#define ROOM_NORANDOMRESET	(cc)

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
#define EX_ISDOOR                     (A)
#define EX_CLOSED                     (B)
#define EX_LOCKED                     (C)
#define EX_PICKPROOF                  (F)
#define EX_NOPASS                     (G)
#define EX_EASY                       (H)
#define EX_HARD                       (I)
#define EX_INFURIATING                (J)
#define EX_NOCLOSE                    (K)
#define EX_NOLOCK                     (L)



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
#define POS_SNEAK                     9		/* used only for movement act messages */



/*
 * ACT bits for players.
 */
#define PLR_IS_NPC              (A)             /* Don't EVER set.      */

/* RT auto flags */
#define PLR_LOOKINPIT           (B)
#define PLR_AUTOASSIST          (C)
#define PLR_AUTOEXIT            (D)
#define PLR_AUTOLOOT            (E)
#define PLR_AUTOSAC             (F)
#define PLR_AUTOGOLD            (G)
#define PLR_AUTOSPLIT           (H)
#define PLR_DEFENSIVE           (I)
#define PLR_WIMPY               (J)
#define PLR_QUESTOR             (K)

/* RT personal flags */
#define PLR_COLOR2              (L)
#define PLR_VT100               (M)
#define PLR_MAKEBAG             (O)
#define PLR_CANLOOT             (P)
#define PLR_NOSUMMON            (Q)
#define PLR_NOFOLLOW            (R)
#define PLR_COLOR               (S)

/* penalty flags */
#define PLR_PERMIT              (U)
#define PLR_CLOSED              (V)
#define PLR_LOG                 (W)
#define PLR_FREEZE              (Y)
#define PLR_THIEF               (Z)
#define PLR_KILLER              (aa)
#define PLR_SUPERWIZ            (bb)
#define PLR_TICKS               (cc)
#define PLR_NOPK                (dd)

/* Lotus - Extra PLR flags for only Players in pcdata->plr */
#define PLR_OOC                 (A)
#define PLR_CHATMODE            (B)
#define PLR_PRIVATE             (C)
#define PLR_STOPCRASH           (D)
#define PLR_PK                  (E)
#define PLR_SHOWEMAIL           (G)
#define PLR_LINK_DEAD           (H)
#define PLR_PAINT               (I)
#define PLR_SNOOP_PROOF         (J)
#define PLR_NOSHOWLAST          (K)
#define PLR_NONOTIFY            (L) /* Comm flags full, pc only */
#define PLR_AUTOPEEK            (M)
#define PLR_HEEDNAME		(N)
#define PLR_SHOWLOST            (O)
#define PLR_SHOWRAFF            (Q)
#define PLR_MARRIED		(R)
#define PLR_SQUESTOR		(S)	/* Montrey */
#define PLR_DUEL_IGNORE		(T)	/* Montrey */
//#define PLR_NEWSCORE		(U)	/* Montrey */

/* channel flags, for new channel code -- Montrey */
#define CHAN_GOSSIP		0
#define CHAN_FLAME		1
#define CHAN_QWEST		2
#define CHAN_PRAY		3
#define CHAN_CLAN		4
#define CHAN_MUSIC		5
#define CHAN_IC			6
#define CHAN_GRATS		7
#define CHAN_IMMTALK		8
#define CHAN_QA			9

/* RT comm flags -- may be used on both mobs and chars */
#define COMM_QUIET              (A)
#define COMM_DEAF               (B)
#define COMM_NOWIZ              (C)
#define COMM_NOAUCTION          (D)
#define COMM_NOGOSSIP           (E)
#define COMM_NOQUESTION         (F)
#define COMM_NOMUSIC            (G)
#define COMM_NOCLAN             (H)
#define COMM_NOIC               (I)
#define COMM_NOANNOUNCE         (K)
#define COMM_COMPACT            (L)
#define COMM_BRIEF              (M)
#define COMM_PROMPT             (N)
#define COMM_COMBINE            (O)
#define COMM_NOFLAME            (P)
#define COMM_SHOW_AFFECTS       (Q)
#define COMM_NOGRATS            (R)
#define COMM_NOEMOTE            (T)
#define COMM_NOCHANNELS          (W) // this isn't even for players, is it needed? Montrey
#define COMM_NOSOCIAL           (X)
#define COMM_NOQUERY            (Y)
#define COMM_AFK                (Z)
#define COMM_NOQWEST            (aa)
#define COMM_NOPAGE             (bb)
#define COMM_NOPRAY             (cc)
#define COMM_RPRAY		(dd) /* Defunct - replaced by REVOKE_PRAY */
#define COMM_ATBPROMPT          (ee)

/* NOchannel flags */
#define REVOKE_NOCHANNELS	(A)
#define REVOKE_FLAMEONLY	(B)
#define REVOKE_GOSSIP		(C)
#define REVOKE_FLAME            (D)
#define REVOKE_QWEST            (F)
#define REVOKE_PRAY		(G)
#define REVOKE_AUCTION          (H)
#define REVOKE_CLAN             (I)
#define REVOKE_MUSIC            (J)
#define REVOKE_QA               (K)
#define REVOKE_SOCIAL           (L)
#define REVOKE_IC               (M)
#define REVOKE_GRATS            (N)
#define REVOKE_PAGE             (Q)
#define REVOKE_QTELL            (R)
#define REVOKE_TELL		(S)
#define REVOKE_EMOTE		(T)
#define REVOKE_SAY		(U)
#define REVOKE_EXP		(V)
#define REVOKE_NOTE		(W)
#define REVOKE_NULL		(ee)	/* for use with new channels code -- Montrey */

/* Command Group flags */
/* Command Group flags */
// A
#define GROUP_GEN		(B)
#define GROUP_QUEST		(C)
// D
#define GROUP_BUILD		(E)
#define GROUP_CODE		(F)
#define GROUP_SECURE	(G)
// H
#define GROUP_PLAYER	(I)		/* player only, for interp table (not set in cgroup) */
#define GROUP_MOBILE	(J)		/* mob only, for interp table (not set in cgroup) */
#define GROUP_CLAN		(K)		/* clan commands */
#define GROUP_AVATAR	(L)		/* level 80 stuff like scon */
#define GROUP_HERO		(M)		/* hero only stuff like herochat */
#define GROUP_DEPUTY	(N)		/* stuff for deputies and imm heads, makes them a deputy/head */
#define GROUP_LEADER	(O)		/* stuff for leaders and imm imps, makes them a leader/imp */

/* WIZnet flags */
#define WIZ_ON                  (A)
#define WIZ_PURGE		(B)
#define WIZ_LOGINS              (C)
#define WIZ_SITES               (D)
#define WIZ_LINKS               (E)
#define WIZ_DEATHS              (F)
#define WIZ_RESETS              (G)
#define WIZ_MOBDEATHS           (H)
#define WIZ_FLAGS               (I)
#define WIZ_PENALTIES           (J)
#define WIZ_SACCING             (K)
#define WIZ_LEVELS              (L)
#define WIZ_SECURE              (M)
#define WIZ_SWITCHES            (N)
#define WIZ_SNOOPS              (O)
#define WIZ_RESTORE             (P)
#define WIZ_LOAD                (Q)
#define WIZ_NEWBIE              (R)
#define WIZ_PREFIX              (S)
#define WIZ_SPAM                (T)
#define WIZ_MISC                (U)
#define WIZ_BUGS                (V) /* W,X,Y open */
#define WIZ_CHEAT               (Z)
#define WIZ_MAIL                (aa)
#define WIZ_AUCTION             (bb)
#define WIZ_QUEST               (cc)
#define WIZ_MALLOC              (dd)


/* New censor flags to replace COMM_SWEARON -- Montrey */
#define CENSOR_CHAN		(A)
#define CENSOR_SPAM		(C)


/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct  mob_index_data
{
    MOB_INDEX_DATA *    next;
    SPEC_FUN *          spec_fun;
    SHOP_DATA *         pShop;
    sh_int              vnum;
    sh_int              group;
    sh_int		version;	/* Mob versioning -- Montrey */
    sh_int              count;
    sh_int              killed;
    char *              player_name;
    char *              short_descr;
    char *              long_descr;
    char *              description;
    long                act;
    long                affected_by;
    sh_int              alignment;
    sh_int              level;
    sh_int              hitroll;
    sh_int              hit[3];
    sh_int              mana[3];
    sh_int              damage[3];
    sh_int              ac[4];
    sh_int              dam_type;
    long                off_flags;
    long		drain_flags;
    long                imm_flags;
    long                res_flags;
    long                vuln_flags;
    sh_int              start_pos;
    sh_int              default_pos;
    sh_int              sex;
    sh_int              race;
    long                wealth;
    long                form;
    long                parts;
    sh_int              size;
    char *              material;
    MPROG_DATA *        mobprogs;
    int                 progtypes;
};

struct  mob_prog_act_list
{
    MPROG_ACT_LIST * next;
    char *           buf;
    CHAR_DATA *      ch;
    OBJ_DATA *       obj;
    void *           vo;
};

struct  mob_prog_data
{
    MPROG_DATA *next;
    int         type;
    char *      arglist;
    char *      comlist;
};

bool    MOBtrigger;

#define ERROR_PROG        -1
#define IN_FILE_PROG       0
#define ACT_PROG           1
#define SPEECH_PROG        2
#define RAND_PROG          4
#define FIGHT_PROG         8
#define DEATH_PROG        16
#define HITPRCNT_PROG     32
#define ENTRY_PROG        64
#define GREET_PROG       128
#define ALL_GREET_PROG   256
#define GIVE_PROG        512
#define BRIBE_PROG      1024
#define BUY_PROG        2048
#define TICK_PROG       4096
#define BOOT_PROG	8192



/* memory settings */
#define MEM_CUSTOMER    A
#define MEM_SELLER      B
#define MEM_HOSTILE     C
#define MEM_AFRAID      D

/* memory for mobs */
struct mem_data
{
    MEM_DATA    *next;
    bool        valid;
    int         id;
    int         reaction;
    time_t      when;
};


/*
 * One character (PC or NPC).
 */
struct  char_data
{
    CHAR_DATA *         next;
    CHAR_DATA *         next_in_room;
    CHAR_DATA *         master;
    CHAR_DATA *         leader;
    CHAR_DATA *         fighting;
    /* CHAR_DATA *         reply; -- Changing this to a string, holding the player's name. */
    char                reply[64];
    RESET_DATA *	reset;		/* let's make it keep track of what reset it */
    bool		replylock;	   /* <--- made this a bool to simplify */
    CHAR_DATA *         pet;			/* not saving at quit - Montrey */
    MEM_DATA *          memory;
    SPEC_FUN *          spec_fun;
    MOB_INDEX_DATA *    pIndexData;
    DESCRIPTOR_DATA *   desc;
    AFFECT_DATA *       affected;
    NOTE_DATA *         pnote;
    OBJ_DATA *          carrying;
    OBJ_DATA *          on;
    ROOM_INDEX_DATA *   in_room;
    ROOM_INDEX_DATA *   was_in_room;
    AREA_DATA *         zone;
    PC_DATA *           pcdata;
    GEN_DATA *          gen_data;
    bool                valid;
    char *              name;
    long                id;
    char *              short_descr;
    char *              long_descr;
    char *              description;
    char *              prompt;
    char *              prefix;
    sh_int              group;
    CLAN_DATA *         clan;
    CLAN_DATA *		inviters;
    bool		invitation_accepted;
    sh_int              sex;
    sh_int              class;
    sh_int              race;
    sh_int              level;
    int                 lines;  /* for the pager */
    time_t              logon;
    time_t              last_bank;  /* For Bank, duh */
    sh_int              timer;  /* Main timer is now part of descriptor */
    sh_int              wait;
    sh_int              daze;
    sh_int              fightpulse;
    sh_int              hit;
    sh_int              max_hit;
    sh_int              mana;
    sh_int              max_mana;
    sh_int              stam;
    sh_int              max_stam;
    long                gold;
    long                silver;
    long                gold_in_bank;
    long                silver_in_bank;
    int                 exp;
    long                act;
    long                comm;   /* RT added to pad the vector */
    long                revoke;  /* New Revoke stuff */
    long                wiznet; /* wiz stuff */
    long		censor;			/* New censor flags -- Montrey */
    long		drain_flags;
    long                imm_flags;
    long                res_flags;
    long                vuln_flags;
    sh_int              invis_level;
    sh_int              lurk_level;
    long                affected_by;
    sh_int              position;
    sh_int              practice;
    sh_int              train;
    sh_int              saving_throw;
    sh_int              alignment;
    sh_int              hitroll;
    sh_int              damroll;
	sh_int		armor_a[4];
	sh_int		armor_m[4];
    sh_int              wimpy;
    /* stats */
    sh_int              perm_stat[MAX_STATS];
    sh_int              mod_stat[MAX_STATS];
    /* parts stuff */
    long                form;
    long                parts;
    sh_int              size;
    char*               material;
    /* mobile stuff */
    long                off_flags;
    sh_int              damage[3];
    sh_int              dam_type;
    sh_int              start_pos;
    sh_int              default_pos;
    CHAR_DATA *         hunting;
    sh_int              nectimer;
    sh_int              secure_level;
    MPROG_ACT_LIST *    mpact;
    int                 mpactnum;
    int                 quest_giver;  /* Elrac */
    int                 questpoints;  /* Vassago */
    int			questpoints_donated; /* Clerve */
    long		gold_donated; /* Montrey */
    sh_int              nextquest;    /* Vassago */
    sh_int              countdown;    /* Vassago */
    sh_int              questobj;     /* Vassago */
    sh_int              questmob;     /* Vassago */
    sh_int              questloc;     /* -- Elrac */
    sh_int              questobf;     /* Lotus */
    TAIL_DATA *         tail;         /* -- Elrac */
    EDIT_DATA *         edit;         /* -- Elrac */
};



/*
 * Video mode flag bits
 */
#define VIDEO_FLASH_OFF         A
#define VIDEO_FLASH_LINE        B
/* spare                        C */
#define VIDEO_DARK_MOD          D
/* spare                        E */
#define VIDEO_VT100             F
/* spare                        G */
#define VIDEO_CODES_SHOW        H


/* mudding experience for newbies */
#define MEXP_TOTAL_NEWBIE	0
#define MEXP_LEGACY_NEWBIE	1
#define MEXP_LEGACY_OLDBIE	2

/*
 * Data which only PC's have.
 */
struct  pc_data
{
	PC_DATA *           next;
	CHAR_DATA *	ch;	/* i may be missing something, but this seems like a 'duh',
				   make it point backwards to the character -- Montrey */

	BUFFER *            buffer;
	bool                valid;
	char *              pwd;
	char *              bamfin;
	char *              bamfout;
	char *              gamein;
	char *              gameout;
	char *              afk;
	char *              title;
	char *              immname;   /* Immortal Name */
    char *              immprefix; // immtalk prefix
	time_t			last_note;
	time_t			last_idea;
	time_t			last_roleplay;
	time_t			last_immquest;
	time_t			last_changes;
	time_t			last_personal;
	time_t			last_trade;
	sh_int			perm_hit;
	sh_int			perm_mana;
	sh_int			perm_stam;
	sh_int			trains_to_hit;			/* Montrey */
	sh_int			trains_to_mana;			/* Montrey */
	sh_int			trains_to_stam;			/* Montrey */
	sh_int			mud_exp;			/* Montrey */
	sh_int              true_sex;
	int                 pckills;
	int                 pckilled;
	sh_int                 arenakills;			/* Tarrant */
	sh_int                 arenakilled;			/* Tarrant */
	int			pkrank;				/* Montrey */
	int                 last_level;
	int			last_logoff;			/* Montrey */
	int                 played;
	int                 backup;
	sh_int              condition       [4];
	sh_int              learned         [MAX_SKILL];
	sh_int		evolution	[MAX_SKILL];	/* Montrey */
	int			skillpoints;			/* Montrey */
	int                 squest_giver;			/* Montrey */
	sh_int              nextsquest;			/* Montrey */
	sh_int              sqcountdown;			/* Montrey */
	OBJ_DATA  *         squestobj;			/* Montrey */
	CHAR_DATA *         squestmob;			/* Montrey */
	bool                squestobjf;			/* Montrey */
	bool		squestmobf;			/* Montrey */
	int			squestloc1;	/* obj */	/* Montrey */
	int			squestloc2;	/* mob */	/* Montrey */
	bool                group_known     [MAX_GROUP];
	int			rolepoints;			/* Montrey */
	sh_int              points;
	sh_int              confirm_delete;
	char *              alias[MAX_ALIAS];
	char *              alias_sub[MAX_ALIAS];
	CHAR_DATA *         skeleton;			/* Lotus */
	CHAR_DATA *         zombie;				/* Lotus */
	CHAR_DATA *         wraith;				/* Lotus */
	CHAR_DATA *         gargoyle;			/* Lotus */
	sh_int              color         [MAX_COLORS];
	sh_int              bold          [MAX_COLORS];
	long                  cgroup;  /* Command Groups - Xenith */
   long                plr; /* Extra PLR flags */
   char *              rank;
   sh_int              lastcolor     [2];
   sh_int              pktimer;
   char *              status;
   char *              deity;
   sh_int              remort_count;
   int			extraclass[MAX_EXTRACLASS_SLOTS];			/* Endo */
   int			raffect[MAX_RAFFECT_SLOTS];			/* Endo */
    OBJ_DATA *          locker;
   OBJ_DATA *          strongbox;			/* Elrac */
   char *              email;
   time_t              last_ltime;
   time_t              last_saved;
   sh_int			flag_thief;
   sh_int			flag_killer;
   char *              last_lsite;
   char *              fingerinfo;
   char *              query[MAX_QUERY];
   char *              spouse;
   char *              propose;
   char *              whisper;			/* Montrey */
   long              video;				/* Elrac */
   sh_int              tailing;			/* Elrac */
   int                 mark_room;			/* Elrac */
   char *	        ignore[MAX_IGNORE];
   char *              aura;   /* For Gimic Aura Command */
   DUEL_DATA *	duel;				/* Montrey */
   sh_int      lays;     /* times we can lay on hands */
   sh_int      next_lay_countdown;   /* time before we get power back */
   sh_int      familiar;          /* PCs can have a familiar -- Outsider */
   char        granted_commands[MAX_GRANT][32];    /* granted commands */
};


/* Data for generating characters -- only used during generation */
struct gen_data
{
    GEN_DATA    *next;
    bool        valid;
    bool        skill_chosen[MAX_SKILL];
    bool        group_chosen[MAX_GROUP];
    int         points_chosen;
};



/*
 * Liquids.
 */
#define LIQ_WATER        0

struct  liq_type
{
    char *      liq_name;
    char *      liq_color;
    sh_int      liq_affect[5];
};



/*
 * Extra description data for a room or object.
 */
struct  extra_descr_data
{
    EXTRA_DESCR_DATA *next;     /* Next in list                     */
    bool valid;
    char *keyword;              /* Keyword in look/examine          */
    char *description;          /* What to see                      */
};


#define MAX_EQSOCKETS 6
typedef struct eqsocket_data {
    char quality;
    char type;
} EQSOCKET_DATA;

/*
 * Prototype for an object.
 */
struct  obj_index_data
{
    OBJ_INDEX_DATA *    next;
    EXTRA_DESCR_DATA *  extra_descr;
    AFFECT_DATA *       affected;
    char *              name;
    char *              short_descr;
    char *              description;
    sh_int              vnum;
    sh_int              reset_num;
    sh_int		version;	/* Object versioning -- Montrey */
    char *              material;
    sh_int              item_type;
    long                extra_flags; /* Formerly INT */
    long                wear_flags;  /* Formerly INT */
    sh_int              level;
    sh_int              condition;
    sh_int              count;
    sh_int              weight;
    int                 cost;
    int                 value[5];

    int                 num_settings; // for socketed gems
};



/*
 * One object.
 */
struct obj_data
{
	OBJ_INDEX_DATA *	pIndexData;
	RESET_DATA *		reset;		/* let's make it keep track of what reset it */
	OBJ_DATA *		next;
	OBJ_DATA *		next_content;
	OBJ_DATA *		in_obj;
	OBJ_DATA *		on;
	ROOM_INDEX_DATA *	in_room;
	CHAR_DATA *		carried_by;
	CHAR_DATA *		in_locker;
	CHAR_DATA *		in_strongbox;
	OBJ_DATA *		contains;

	char *			name;
	char *			short_descr;
	char *			description;
	char *			material;
	char *			owner;
	EXTRA_DESCR_DATA *	extra_descr;
	sh_int			item_type;
	long			extra_flags;
	long			wear_flags;
	int			value[5];
	sh_int			weight;
	int			cost;
	sh_int			level;
	sh_int			condition;
	sh_int			wear_loc;
	sh_int			timer;
	sh_int			clean_timer;		/* Montrey */
	sh_int			spell[MAX_SPELL];
	sh_int			spell_lev[MAX_SPELL];
	bool			valid;

    /* ugly way to do this: rather than everywhere cycling through the affects given by
       the object's index data separately from the affects given by inset gems, we
       compile a list of affects whenever one of those changes (rare event). -- Montrey */
    bool            enchanted; // have the affects for this object been modified from the index?  only for saving to file
    AFFECT_DATA *   perm_affected; // initially identical to the index, can be changed by enchants and addapply
    AFFECT_DATA *   affected; // the compiled list, never shown in 'stat' or 'lore', so it can be deduped.

    char            num_settings;
    OBJ_DATA *      gems; // gems in settings
    AFFECT_DATA *   gem_affected;
};



/*
 * Exit data.
 */
struct  exit_data
{
    union
    {
        ROOM_INDEX_DATA *       to_room;
        sh_int                  vnum;
    } u1;
    sh_int              exit_info;
    sh_int              key;
    char *              keyword;
    char *              description;
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct  reset_data
{
    RESET_DATA *        next;
    sh_int		version;	/* Reset versioning -- Montrey */
    char                command;
    sh_int              arg1;
    sh_int              arg2;
    sh_int              arg3;
    sh_int              arg4;
};


#define AREA_TYPE_ALL   'A'
#define AREA_TYPE_CLAN  'C'
#define AREA_TYPE_IMMS  'I'
#define AREA_TYPE_HERO  'H'
#define AREA_TYPE_ARENA 'R'
#define AREA_TYPE_XXX   'X'
#define AREA_TYPE_NORM  ' '


/*
 * Area definition.
 */
struct  area_data
{
    AREA_DATA *         next;
    RESET_DATA *        reset_first;
    RESET_DATA *        reset_last;
    sh_int		version;	/* Area file versioning -- Montrey */
    char *              file_name;
    char *              name;
    char *              credits;
    sh_int              age;
    sh_int              nplayer;
    sh_int              low_range;
    sh_int              high_range;
    sh_int              min_vnum;
    sh_int              max_vnum;
    bool                empty;
    char                area_type;  /* -- Elrac */
    char *              author;     /* -- Elrac */
    char *              title;      /* -- Elrac */
    char *              keywords;   /* -- Elrac */
};



/* Room type */
struct room_index_data
{
	ROOM_INDEX_DATA *	next;
	CHAR_DATA *		people;
	OBJ_DATA *		contents;
	EXTRA_DESCR_DATA *	extra_descr;
	AREA_DATA *		area;
	EXIT_DATA *		exit    [6];
	EXIT_DATA *		old_exit[6];
	char *			name;
	char *			description;
	char *			owner;
	sh_int			vnum;
	sh_int			version;	/* Room versioning -- Montrey */
	AFFECT_DATA *		affected;		/* Montrey */
	long			original_flags;		/* Montrey */
	long			room_flags;
	sh_int			light;
	sh_int			sector_type;
	sh_int			heal_rate;
	sh_int			mana_rate;
	CLAN_DATA *		clan;
	sh_int			guild;		/* guild room, class number+1, 0 none -- Montrey */
	sh_int			tele_dest;
	sh_int			hunt_id;  /* Unique ID for current hunt */
	ROOM_INDEX_DATA *	hunt_next;  /* next room in search circle */
	ROOM_INDEX_DATA *	hunt_back;  /* pointer back toward origin */
};

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

#define TARGET_CHAR                 0
#define TARGET_OBJ                  1
#define TARGET_ROOM                 2
#define TARGET_NONE                 3



/*
 * Skills include spells as a particular case.
 */
struct  skill_type
{
    char *      name;                   /* Name of skill                */
    sh_int      skill_level[MAX_CLASS]; /* Level needed by class        */
    sh_int      rating[MAX_CLASS];      /* How hard it is to learn      */
    SPELL_FUN * spell_fun;              /* Spell pointer (for spells)   */
    sh_int      target;                 /* Legal targets                */
    sh_int      minimum_position;       /* Position for caster / user   */
    sh_int *    pgsn;                   /* Pointer to associated gsn    */
    sh_int      slot;                   /* Slot for #OBJECT loading     */
    sh_int      min_mana;               /* Minimum mana used            */
    sh_int      beats;                  /* Waiting time after use       */
    char *      noun_damage;            /* Damage message               */
    char *      msg_off;                /* Wear off message             */
    char *      msg_obj;                /* Wear off message for obects  */
    int         remort_class;           /* Required remort level or 0   */
    int		evocost_sec[MAX_CLASS];	/* Class cost to evolve to 2    */
    int		evocost_pri[MAX_CLASS];	/* Class cost to evolve to 3    */
};

struct  group_type
{
    char *      name;
    sh_int      rating[MAX_CLASS];
    char *      spells[MAX_IN_GROUP];
};



/*
 * These are skill_lookup return values for common skills and spells.
 */
extern sh_int	gsn_reserved;
extern sh_int	gsn_acid_blast;
extern sh_int	gsn_acid_breath;
extern sh_int	gsn_acid_rain;
extern sh_int	gsn_age;
extern sh_int	gsn_animate_skeleton;
extern sh_int	gsn_animate_wraith;
extern sh_int	gsn_animate_gargoyle;
extern sh_int	gsn_animate_zombie;
extern sh_int	gsn_armor;
extern sh_int	gsn_bless;
extern sh_int	gsn_blindness;
extern sh_int	gsn_blizzard;
extern sh_int	gsn_blood_blade;
extern sh_int	gsn_blood_moon;
extern sh_int	gsn_burning_hands;
extern sh_int	gsn_call_lightning;
extern sh_int	gsn_calm;
extern sh_int	gsn_cancellation;
extern sh_int	gsn_cause_light;
extern sh_int	gsn_cause_serious;
extern sh_int	gsn_cause_critical;
extern sh_int	gsn_chain_lightning;
extern sh_int	gsn_change_sex;
extern sh_int	gsn_channel;
extern sh_int	gsn_charm_person;
extern sh_int	gsn_chill_touch;
extern sh_int	gsn_colour_spray;
extern sh_int	gsn_continual_light;
extern sh_int	gsn_control_weather;
extern sh_int	gsn_create_food;
extern sh_int	gsn_create_parchment;
extern sh_int	gsn_create_rose;
extern sh_int	gsn_create_sign;
extern sh_int	gsn_create_spring;
extern sh_int	gsn_create_vial;
extern sh_int	gsn_create_water;
extern sh_int	gsn_cure_blindness;
extern sh_int	gsn_cure_critical;
extern sh_int	gsn_cure_disease;
extern sh_int	gsn_cure_light;
extern sh_int	gsn_cure_poison;
extern sh_int	gsn_cure_serious;
extern sh_int	gsn_curse;
extern sh_int	gsn_darkness;
extern sh_int	gsn_dazzling_light;
extern sh_int	gsn_demonfire;
extern sh_int	gsn_detect_evil;
extern sh_int	gsn_detect_good;
extern sh_int	gsn_detect_hidden;
extern sh_int	gsn_detect_invis;
extern sh_int	gsn_detect_magic;
extern sh_int	gsn_detect_poison;
extern sh_int	gsn_dispel_evil;
extern sh_int	gsn_dispel_good;
extern sh_int	gsn_dispel_magic;
extern sh_int	gsn_divine_healing;
extern sh_int	gsn_divine_regeneration;
extern sh_int	gsn_earthquake;
extern sh_int	gsn_encampment;
extern sh_int	gsn_enchant_armor;
extern sh_int	gsn_enchant_weapon;
extern sh_int	gsn_energy_drain;
extern sh_int	gsn_faerie_fire;
extern sh_int	gsn_faerie_fog;
extern sh_int	gsn_farsight;
extern sh_int	gsn_fear;
extern sh_int	gsn_fire_breath;
extern sh_int	gsn_fireball;
extern sh_int	gsn_fireproof;
extern sh_int	gsn_firestorm;
extern sh_int	gsn_flame_blade;
extern sh_int	gsn_flameshield;
extern sh_int	gsn_flamestrike;
extern sh_int	gsn_fly;
extern sh_int	gsn_floating_disc;
extern sh_int	gsn_frenzy;
extern sh_int	gsn_frost_blade;
extern sh_int	gsn_frost_breath;
extern sh_int	gsn_gas_breath;
extern sh_int	gsn_gate;
extern sh_int	gsn_general_purpose;
extern sh_int	gsn_giant_strength;
extern sh_int	gsn_harm;
extern sh_int	gsn_haste;
extern sh_int	gsn_heal;
extern sh_int	gsn_heat_metal;
extern sh_int	gsn_high_explosive;
extern sh_int	gsn_holy_word;
extern sh_int	gsn_identify;
extern sh_int	gsn_infravision;
extern sh_int	gsn_invis;
extern sh_int	gsn_know_alignment;
extern sh_int	gsn_light_of_truth;
extern sh_int	gsn_lightning_bolt;
extern sh_int	gsn_lightning_breath;
extern sh_int	gsn_locate_life;
extern sh_int	gsn_locate_object;
extern sh_int	gsn_magic_missile;
extern sh_int	gsn_mass_healing;
extern sh_int	gsn_mass_invis;
extern sh_int	gsn_nexus;
extern sh_int	gsn_pass_door;
extern sh_int	gsn_plague;
extern sh_int	gsn_poison;
extern sh_int	gsn_power_word;
extern sh_int	gsn_polymorph;
extern sh_int	gsn_portal;
extern sh_int	gsn_protect_container;
extern sh_int	gsn_protection_evil;
extern sh_int	gsn_protection_good;
extern sh_int	gsn_ray_of_truth;
extern sh_int	gsn_recharge;
extern sh_int	gsn_refresh;
extern sh_int	gsn_resurrect;
extern sh_int	gsn_regeneration;
extern sh_int	gsn_remove_alignment;
extern sh_int	gsn_remove_invis;
extern sh_int	gsn_remove_curse;
extern sh_int	gsn_sanctuary;
extern sh_int   gsn_scry;
extern sh_int	gsn_shield;
extern sh_int	gsn_shock_blade;
extern sh_int	gsn_shocking_grasp;
extern sh_int	gsn_shrink;
extern sh_int	gsn_sleep;
extern sh_int	gsn_slow;
extern sh_int	gsn_smokescreen;
extern sh_int   gsn_starve;
extern sh_int	gsn_steel_mist;
extern sh_int	gsn_stone_skin;
extern sh_int	gsn_summon;
extern sh_int	gsn_summon_object;
extern sh_int	gsn_sunray;
extern sh_int	gsn_talon;
extern sh_int	gsn_teleport;
extern sh_int	gsn_teleport_object;
extern sh_int	gsn_undo_spell;
extern sh_int	gsn_ventriloquate;
extern sh_int	gsn_vision;
extern sh_int	gsn_weaken;
extern sh_int	gsn_word_of_recall;
extern sh_int	gsn_wrath;
extern sh_int	gsn_axe;
extern sh_int	gsn_dagger;
extern sh_int	gsn_flail;
extern sh_int	gsn_mace;
extern sh_int	gsn_polearm;
extern sh_int	gsn_spear;
extern sh_int	gsn_sword;
extern sh_int	gsn_whip;
extern sh_int   gsn_bow;
extern sh_int	gsn_shield_block;
extern sh_int	gsn_brew;
extern sh_int	gsn_scribe;
extern sh_int	gsn_backstab;
extern sh_int	gsn_bash;
extern sh_int	gsn_berserk;
extern sh_int	gsn_circle;
extern sh_int	gsn_crush;
extern sh_int	gsn_dirt_kicking;
extern sh_int	gsn_disarm;
extern sh_int	gsn_dodge;
extern sh_int	gsn_enhanced_damage;
extern sh_int	gsn_envenom;
extern sh_int	gsn_hand_to_hand;
extern sh_int	gsn_kick;
extern sh_int   gsn_roundhouse; /*wchange added for evo 2+'s second hit*/
extern sh_int   gsn_footsweep;
extern sh_int gsn_necromancy;
extern sh_int	gsn_parry;
extern sh_int	gsn_rescue;
extern sh_int	gsn_trip;
extern sh_int	gsn_second_attack;
extern sh_int	gsn_third_attack;
extern sh_int	gsn_dual_wield;
extern sh_int	gsn_hunt;
extern sh_int  gsn_unarmed;
extern sh_int	gsn_swimming;
extern sh_int	gsn_fast_healing;
extern sh_int	gsn_firebuilding;
extern sh_int	gsn_forge;
extern sh_int	gsn_repair;
extern sh_int	gsn_rotate;
extern sh_int	gsn_languages;
extern sh_int	gsn_haggle;
extern sh_int	gsn_hide;
extern sh_int	gsn_lore;
extern sh_int	gsn_meditation;
extern sh_int	gsn_peek;
extern sh_int	gsn_pick_lock;
extern sh_int	gsn_scan;
extern sh_int	gsn_sneak;
extern sh_int	gsn_steal;
extern sh_int	gsn_sing;
extern sh_int	gsn_scrolls;
extern sh_int   gsn_spousegate;
extern sh_int	gsn_staves;
extern sh_int	gsn_wands;
extern sh_int	gsn_recall;
extern sh_int  gsn_lay_on_hands;
extern sh_int  gsn_familiar;
extern sh_int  gsn_die_hard;
extern sh_int	gsn_sheen;
extern sh_int	gsn_focus;
extern sh_int	gsn_paralyze;
extern sh_int	gsn_barrier;
extern sh_int	gsn_dazzle;
extern sh_int	gsn_full_heal;
extern sh_int	gsn_midnight;
extern sh_int	gsn_shadow_form;
extern sh_int	gsn_hone;
extern sh_int	gsn_riposte;
extern sh_int	gsn_fourth_attack;
extern sh_int	gsn_rage;
extern sh_int  gsn_blind_fight;
extern sh_int	gsn_sap;
extern sh_int	gsn_pain;
extern sh_int	gsn_hex;
extern sh_int	gsn_bone_wall;
extern sh_int	gsn_hammerstrike;
extern sh_int	gsn_force_shield;
extern sh_int	gsn_holy_sword;
extern sh_int	gsn_align;
extern sh_int	gsn_blur;
extern sh_int	gsn_dual_second;
extern sh_int	gsn_quick;
extern sh_int	gsn_standfast;
extern sh_int	gsn_mark;
extern sh_int	gsn_critical_blow;

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

/*
 * Utility macros.
 */
#define IS_VALID(data)          ((data) != NULL && (data)->valid)
#define VALIDATE(data)          ((data)->valid = TRUE)
#define INVALIDATE(data)        ((data)->valid = FALSE)
#define UMIN(a, b)              ((a) < (b) ? (a) : (b))
#define UMAX(a, b)              ((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)         ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)                ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)                ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)       ((flag) & (bit))
#define SET_BIT(var, bit)       ((var) |= (bit))
#define REMOVE_BIT(var, bit)    ((var) &= ~(bit))
#define chance(prc)		(((number_range(1,100)) <= (prc)))
#define rounddown(x, inc)	((inc > x) ? 0 : (x - (x % inc)))
#define roundup(x, inc)		((inc > x) ? inc : (x - ((x % inc) == 0 ? inc : (x % inc)) + inc))

/*
 * Character macros.
 */
#define IS_NPC(ch)              (IS_SET((ch)->act, ACT_IS_NPC))
#define IS_PLAYING(d)		(d && d->connected == CON_PLAYING && d->character)
#define IS_AFFECTED(ch, sn)     (IS_SET((ch)->affected_by, (sn)))

/* permission checking stuff */
#define IS_HERO(ch)         (!IS_NPC(ch) && ch->level >= LEVEL_HERO)
#define IS_REMORT(ch)		(!IS_NPC(ch) && ch->pcdata->remort_count > 0)
#define IS_HEROIC(ch)		(IS_HERO(ch) || IS_REMORT(ch))
#define IS_IMM_GROUP(bit)	(IS_SET(bit, GROUP_GEN|GROUP_QUEST|GROUP_BUILD|GROUP_CODE|GROUP_SECURE))
#define HAS_CGROUP(ch, bit)	(!IS_NPC(ch) && (((ch->pcdata->cgroup & bit) == bit)))
#define RANK(flags)			(IS_IMM_GROUP(flags) ?												\
							(IS_SET(flags, GROUP_LEADER) ?	RANK_IMP	:					\
							(IS_SET(flags, GROUP_DEPUTY) ?	RANK_HEAD 	: RANK_IMM))	:	\
							(IS_SET(flags, GROUP_PLAYER) ?	RANK_MORTAL : RANK_MOBILE))
#define	GET_RANK(ch)		(IS_NPC(ch) ? RANK_MOBILE : RANK(ch->pcdata->cgroup))
#define IS_IMMORTAL(ch)		(GET_RANK(ch) >= RANK_IMM)
#define IS_IMP(ch)			(GET_RANK(ch) == RANK_IMP)
#define IS_HEAD(ch)			(GET_RANK(ch) >= RANK_HEAD)
#define OUTRANKS(ch, victim)	(GET_RANK(ch) > GET_RANK(victim))

#define SET_CGROUP(ch, bit)	(SET_BIT(ch->pcdata->cgroup, bit))
#define REM_CGROUP(ch, bit)	(REMOVE_BIT(ch->pcdata->cgroup, bit))

/* other shortcuts */
#define IS_GOOD(ch)             (ch->alignment >= 350)
#define IS_EVIL(ch)             (ch->alignment <= -350)
#define IS_NEUTRAL(ch)          (!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)            (ch->position > POS_SLEEPING)
#define GET_AC(ch,type)         ((ch)->armor_a[type] + (ch)->armor_m[type]    \
                        + ( IS_AWAKE(ch)                                      \
                        ? dex_app[get_curr_stat(ch,STAT_DEX)].defensive : 0 ) \
                        - (( !IS_NPC(ch) && ch->pcdata->remort_count > 0 )    \
                        ? (((ch->pcdata->remort_count * ch->level) / 50)) : 0 )) /* should give -1 per 10 levels,
                                                                                   -1 per 5 remorts -- Montrey */
#define GET_SEX(ch)		(URANGE(0, ch->sex, 2))
#define GET_HITROLL(ch) \
                ((ch)->hitroll+str_app[get_curr_stat(ch,STAT_STR)].tohit)
#define GET_DAMROLL(ch) \
                ((ch)->damroll+str_app[get_curr_stat(ch,STAT_STR)].todam)
#define IS_OUTSIDE(ch)          (!IS_SET((ch)->in_room->room_flags, ROOM_INDOORS))

/*
#define WAIT_STATE(ch, npulse)  ((ch)->wait = UMAX((ch)->wait, (npulse)))
#define DAZE_STATE(ch, npulse)  ((ch)->daze = UMAX((ch)->daze, (npulse)))
*/
#define WAIT_STATE(ch, npulse)  (ch->level < LEVEL_IMMORTAL ? \
    (ch->wait = UMAX(ch->wait, npulse)) : (ch->wait = 0))
#define DAZE_STATE(ch, npulse)  (ch->level < LEVEL_IMMORTAL ? \
    (ch->daze = UMAX(ch->daze, npulse)) : (ch->daze = 0))
#define gold_weight(amount)  ((amount) * 2 / 5)
#define silver_weight(amount) ((amount)/ 10)
#define IS_QUESTOR(ch)     (IS_SET((ch)->act, PLR_QUESTOR))
#define IS_SQUESTOR(ch)    (!IS_NPC(ch) && IS_SET((ch)->pcdata->plr, PLR_SQUESTOR))
#define IS_KILLER(ch)		(IS_SET((ch)->act, PLR_KILLER))
#define IS_THIEF(ch)		(IS_SET((ch)->act, PLR_THIEF))

/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)     (IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)  (IS_SET((obj)->extra_flags, (stat)))
#define IS_WEAPON_STAT(obj,stat)(IS_SET((obj)->value[4],(stat)))
#define WEIGHT_MULT(obj)        ((obj)->item_type == ITEM_CONTAINER ? \
        (obj)->value[4] : 100)


/*
 * Identd stuff
 */

#define replace_string( pstr, nstr ) \
          { free_string( (pstr) ); pstr=str_dup( (nstr) ); }
#define IS_NULLSTR(str)       ((str)==NULL || (str)[0]=='\0')
#define CH(d)         ((d)->original ? (d)->original : (d)->character )


/*
 * Description macros.
 */
#define PERS(ch, looker, vis)   (  (vis == VIS_ALL					\
				|| (vis == VIS_CHAR && can_see(looker, ch))		\
				|| (vis == VIS_PLR && can_see_who(looker, ch))) ?	\
				IS_NPC(ch) ? ch->short_descr : ch->name : "someone")

/*
 * Structure for a social in the socials table.
 */
struct  social_type
{
    char      name[20];
    char *    char_no_arg;
    char *    others_no_arg;
    char *    char_found;
    char *    others_found;
    char *    vict_found;
/*
    char *    char_not_found;
*/
    char *      char_auto;
    char *      others_auto;
    struct social_type * previous;
    struct social_type * next;
};

/*
 * Structure for a clan in the clan table.
 */

struct clan_type
{
	char *		name;
	char *		who_name;
	sh_int		hall;
	sh_int		area_minvnum;
	sh_int		area_maxvnum;
	bool		independent; /* true for loners and imm clans */
	long		clanqp;
	long		gold_balance;
	char *		clanname;
	char *		creator;
	int		score;		/* points remaining to defeat in current war */
	int		warcpmod;	/* how much their power is modified by wars */

	struct clan_type * previous;
	struct clan_type * next;
};

struct storage_type
{	char *name;
	char *by_who;
	char *date;

	struct storage_type *next;
	struct storage_type *previous;
};

struct departed_type
{	char *name;
	struct departed_type *next;
	struct departed_type *previous;
};


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

struct war_data
{
	OPP_DATA *	chal[4];
	OPP_DATA *	def[4];
	EVENT_DATA *	events;

	bool		ongoing;

	bool		valid;
	WAR_DATA *	previous;
	WAR_DATA *	next;
};

struct opp_data
{
	char *		name;
	char *		clanname;
	bool		inwar;
	sh_int		start_score;
	sh_int		final_score;

	bool		valid;
	OPP_DATA *	next;
};

struct event_data
{
	sh_int		type;
	char *		astr;
	char *		bstr;
	sh_int		number;
	time_t		time;

	bool		valid;
	EVENT_DATA *	next;
};

struct merc_data
{
	char *		name;
	int		minimum;

	OFFER_DATA * 	offer;
	sh_int		num_offers;
	char *		employer;

	bool		valid;
	MERC_DATA *	previous;
	MERC_DATA *	next;
};

struct offer_data
{
	char *		name;
	int		amount;

	bool		valid;
	OFFER_DATA *	next;
};

struct arena_data
{
	char *		keyword;
	char *		name;
	char *		desc;

	int		minvnum;	/* actual arena, not prep rooms or view room */
	int		maxvnum;
	ROOM_INDEX_DATA *chalprep;
	ROOM_INDEX_DATA *defprep;
	ROOM_INDEX_DATA *viewroom;

	ARENA_DATA *	previous;
	ARENA_DATA *	next;
};

struct duel_data
{
	CHAR_DATA *	challenger;
	CHAR_DATA *	defender;
	ARENA_DATA *	arena;
	int		accept_timer;
	int		prep_timer;

	bool		valid;
	DUEL_DATA *previous;
	DUEL_DATA *next;
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

extern  const   struct  class_type      class_table     [MAX_CLASS];
extern  const   struct  deity_type      deity_table     [];
extern  const   struct  weapon_type     weapon_table    [];
extern  const   struct  item_type       item_table      [];
extern  const   struct  wiznet_type     wiznet_table    [];
extern  const   struct  attack_type     attack_table    [];
extern  const   struct  race_type       race_table      [];
extern  const   struct  pc_race_type    pc_race_table   [];
extern  const   struct  spec_type       spec_table      [];
extern  const   struct  liq_type        liq_table       [];
extern  const   struct  skill_type      skill_table     [MAX_SKILL];
extern  const   struct  group_type      group_table     [MAX_GROUP];

/* new social system by Clerve */
extern          struct social_type      *social_table_head;
extern          struct social_type      *social_table_tail;

/* new clan system by Clerve */
extern 		struct clan_type	*clan_table_head;
extern 		struct clan_type	*clan_table_tail;

/* storage list */
extern		STORAGE_DATA		*storage_list_head;
extern		STORAGE_DATA		*storage_list_tail;

/* departed list */
extern		DEPARTED_DATA		*departed_list_head;
extern		DEPARTED_DATA		*departed_list_tail;
extern		char 			*departed_list_line;

/* War stuff */
extern 		struct war_data		*war_table_head;
extern 		struct war_data		*war_table_tail;
extern		struct merc_data	*merc_table_head;
extern		struct merc_data	*merc_table_tail;
extern		struct arena_data	*arena_table_head;
extern		struct arena_data	*arena_table_tail;
extern		struct duel_data	*duel_table_head;
extern		struct duel_data	*duel_table_tail;

/*
 * Global variables.
 */
extern          SHOP_DATA         *     shop_first;

extern          CHAR_DATA         *     char_list;
extern          DESCRIPTOR_DATA   *     descriptor_list;
extern          OBJ_DATA          *     object_list;

extern		PC_DATA *		pc_list;	/* Montrey */

extern          char                    bug_buf         [];
extern          time_t                  current_time;
extern          bool                    fLogAll;
extern          KILL_DATA               kill_table      [];
extern          char                    log_buf         [];
extern          TIME_INFO_DATA          time_info;
extern          WEATHER_DATA            weather_info;
extern          BATTLE_DATA             battle;
extern          AUCTION_DATA      *     auction;
extern          DISABLED_DATA     *     disabled_first;
extern		OBJ_DATA	  *	donation_pit;

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
extern ROOM_INDEX_DATA *quest_startroom;
extern AREA_DATA       *quest_area;

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if     defined(_AIX)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(apollo)
int     atoi            args( ( const char *string ) );
void *  calloc          args( ( unsigned nelem, size_t size ) );
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(hpux)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(linux)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(MIPS_OS)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(NeXT)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(sequent)
char *  crypt           args( ( const char *key, const char *salt ) );
int     fclose          args( ( FILE *stream ) );
int     fprintf         args( ( FILE *stream, const char *format, ... ) );
int     fread           args( ( void *ptr, int size, int n, FILE *stream ) );
int     fseek           args( ( FILE *stream, long offset, int ptrname ) );
void    perror          args( ( const char *s ) );
int     ungetc          args( ( int c, FILE *stream ) );
#endif

#if     defined(sun)
char *  crypt           args( ( const char *key, const char *salt ) );
int     fclose          args( ( FILE *stream ) );
int     fprintf         args( ( FILE *stream, const char *format, ... ) );
#if     defined(SYSV)
siz_t   fread           args( ( void *ptr, size_t size, size_t n,
                            FILE *stream) );
#else
int     fread           args( ( void *ptr, int size, int n, FILE *stream ) );
#endif
int     fseek           args( ( FILE *stream, long offset, int ptrname ) );
void    perror          args( ( const char *s ) );
int     ungetc          args( ( int c, FILE *stream ) );
#endif

#if     defined(ultrix)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif



/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if     defined(NOCRYPT)
#define crypt(s1, s2)   (s1)
#endif



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
#define CD      CHAR_DATA
#define MID     MOB_INDEX_DATA
#define OD      OBJ_DATA
#define OID     OBJ_INDEX_DATA
#define RID     ROOM_INDEX_DATA
#define SF      SPEC_FUN
#define AD      AFFECT_DATA

#define MSL MAX_STRING_LENGTH
#define MIL MAX_INPUT_LENGTH

/* act_comm.c */
//void    check_sex       args( ( CHAR_DATA *ch) );
void    add_follower    args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void    stop_follower   args( ( CHAR_DATA *ch ) );
void    nuke_pets       args( ( CHAR_DATA *ch ) );
void    die_follower    args( ( CHAR_DATA *ch ) );
bool    is_same_group   args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
void    do_say          args( ( CHAR_DATA *ch, const char *argument ) );
void    do_autopeek     args( ( CHAR_DATA *ch, const char *argument ) );
void    do_save         args( ( CHAR_DATA *ch, const char *argument ) );
void    do_testpose     args( ( CHAR_DATA *ch, const char *argument ) );
void    do_video        args( ( CHAR_DATA *ch, const char *argument ) );
void    do_align        args( ( CHAR_DATA *ch, const char *argument ) );
void    send_to_clan    args( ( CHAR_DATA *ch, CLAN_DATA *target, const char *text ) );
void wiznet             args( (const char *string, CHAR_DATA *ch, OBJ_DATA *obj,
                               long flag, long flag_skip, int min_rank ) );

/* channel.c */
void    global_act      args( ( CHAR_DATA *ch, const char *message, \
                                int despite_invis, int color, \
                                long nocomm_bits ) );
void    do_fyi          args( ( CHAR_DATA *ch, const char *argument ) );

/* social-edit.c */
void load_social_table();
void save_social_table();
int count_socials();

/* clan-edit.c */
int	count_clan_members	args((CLAN_DATA *clan, int bit));
void load_clan_table();
void save_clan_table();
int count_clans();
int calc_cp(CLAN_DATA *clan, bool curve);

/* war.c */
void	load_war_table();
void	load_war_events();
void	save_war_table();
void	save_war_events();
void	war_kill		args((CHAR_DATA *ch, CHAR_DATA *victim));
bool	char_at_war		args((CHAR_DATA *ch));
bool	clan_at_war		args((CLAN_DATA *clan));
bool	char_opponents		args((CHAR_DATA *charA, CHAR_DATA *charB));
bool	clan_opponents		args((CLAN_DATA *clanA, CLAN_DATA *clanB));

/* duel.c */
void	duel_update();
void	load_arena_table();
void	view_room_hpbar();
void	duel_kill		args((CHAR_DATA *victim));
bool	char_in_darena_room	args((CHAR_DATA *ch));
bool	char_in_duel_room	args((CHAR_DATA *ch));
bool	char_in_darena		args((CHAR_DATA *ch));
bool	char_in_duel		args((CHAR_DATA *ch));
DUEL_DATA *get_duel		args((CHAR_DATA *ch));

/* storage.c */
void load_storage_list();
void save_storage_list();
int count_stored_characters();
void insert_storagedata(STORAGE_DATA *);
void remove_storagedata(STORAGE_DATA *);
STORAGE_DATA *lookup_storage_data(const char *);

/* departed.c */
void load_departed_list();
void save_departed_list();

void remove_departed(const char *);
void insert_departed(const char *);
bool has_departed(const char *);



/* act_info.c */
void    set_title       args( ( CHAR_DATA *ch, const char *title ) );
void    do_at           args( ( CHAR_DATA *ch, const char *argument ) );
int     color_strlen    args( ( const char *argument ) );
void    do_look         args( ( CHAR_DATA *ch, const char *argument ) );
void   do_send_announce args( ( CHAR_DATA *ch, const char *argument ) );
void    do_peek         args( ( CHAR_DATA *ch, const char *argument ) );
void    do_gamein       args( ( CHAR_DATA *ch, const char *argument ) );
void    do_gameout      args( ( CHAR_DATA *ch, const char *argument ) );
void    do_pit          args( ( CHAR_DATA *ch, const char *argument ) );
void	set_color	args((CHAR_DATA *ch, int color, int bold));
void	new_color	args((CHAR_DATA *ch, int custom));
void    show_affect_to_char  args((AFFECT_DATA *paf, CHAR_DATA *ch));

/* act_move.c */
void    move_char       args( ( CHAR_DATA *ch, int door, bool follow ) );
void    do_mark         args( ( CHAR_DATA *ch, const char *argument ) );
void    do_relocate     args( ( CHAR_DATA *ch, const char *argument ) );
void    recall		args( ( CHAR_DATA *ch, bool clan ) );
RID  *get_random_room   args( ( CHAR_DATA *ch ) );
void    do_land         args( ( CHAR_DATA *ch, const char *argument ) );
void    do_fly          args( ( CHAR_DATA *ch, const char *argument ) );

/* act_obj.c */
bool can_loot           args( (CHAR_DATA *ch, OBJ_DATA *obj) );
void    get_obj         args( ( CHAR_DATA *ch, OBJ_DATA *obj,
                            OBJ_DATA *container ) );
void    do_second       args( (CHAR_DATA *ch, const char *argument) );
void    do_engrave      args( (CHAR_DATA *ch, const char *argument) );

/* act_wiz.c */
void do_allsave         args( (CHAR_DATA *ch, const char *argument) );
RID *find_location      args( (CHAR_DATA *ch, const char *argument) );
void do_fod             args( (CHAR_DATA *ch, const char *argument) );
void do_grouplist       args( (CHAR_DATA *ch, const char *argument) );
void do_restore         args( (CHAR_DATA *ch, const char *argument) );
void do_rwhere          args( (CHAR_DATA *ch, const char *argument) );
void do_setgamein       args( (CHAR_DATA *ch, const char *argument) );
void do_setgameout      args( (CHAR_DATA *ch, const char *argument) );
void do_tail            args( (CHAR_DATA *ch, const char *argument) );
int  set_tail           args( (CHAR_DATA *ch, CHAR_DATA *victim, int tail_flags) );
void do_owner           args( (CHAR_DATA *ch, const char *argument) );

/* alias.c */
void    substitute_alias args( (DESCRIPTOR_DATA *d, const char *input) );

/* bank.c */
void    find_money      args( ( CHAR_DATA *ch ) );

/* comm.c */
void    show_string     args( ( struct descriptor_data *d, const char *input) );
void    close_socket    args( ( DESCRIPTOR_DATA *dclose ) );
void	cwtb		args((DESCRIPTOR_DATA *d, const char *txt));
void    write_to_buffer args( ( DESCRIPTOR_DATA *d, const char *txt,
                            int length ) );
void    stc    args( ( const char *txt, CHAR_DATA *ch ) );
void    page_to_char    args( ( char *txt, CHAR_DATA *ch ) );
void    xact             args( ( const char *format, CHAR_DATA *ch,
                                const void *arg1, const void *arg2, int type ) );
void    act             args( ( const char *format, CHAR_DATA *ch,
                                const void *arg1, const void *arg2, int type ) );
void    act_new         args( ( const char *format, CHAR_DATA *ch,
                            const void *arg1, const void *arg2, int type,
                            int min_pos, bool censor) );

/* nanny.c */
void	update_pc_index		args((CHAR_DATA *ch, bool remove));

/* db.c */
const char *  print_flags     args( ( int flag ));
void    boot_db         args( ( void ) );
void    clear_char      args( ( CHAR_DATA *ch ) );
void    bug             args( ( const char *str, int param ) );
void    log_string      args( ( const char *str ) );
void    tail_chain      args( ( void ) );
void    load_disabled   args( ( void ) );
void    save_disabled   args( ( void ) );
void    do_oldareas     args( ( CHAR_DATA *ch, const char *argument ) );


/* area_handler.c */
void    area_update     args( ( void ) );
CD *    create_mobile   args( ( MOB_INDEX_DATA *pMobIndex ) );
void    clone_mobile    args( ( CHAR_DATA *parent, CHAR_DATA *clone) );
OD *    create_object   args( ( OBJ_INDEX_DATA *pObjIndex, int level ) );
void    clone_object    args( ( OBJ_DATA *parent, OBJ_DATA *clone ) );
MID *   get_mob_index   args( ( int vnum ) );
OID *   get_obj_index   args( ( int vnum ) );
RID *   get_room_index  args( ( int vnum ) );
int	get_location_ac args( (CHAR_DATA *ch, int wear, int type) );


/* file.c */
char	fread_letter		args((FILE *fp));
int	fread_number		args((FILE *fp));
long	fread_flag		args((FILE *fp));
char *	fread_string		args((FILE *fp));
char *	fread_string_eol	args((FILE *fp));
void	fread_to_eol		args((FILE *fp));
char *	fread_word		args((FILE *fp));
void	fappend			args((const char *file, const char *str));


/* memory.c */
void *	alloc_mem		args((long sMem));
void *	alloc_perm		args((long sMem));
void *	alloc_perm2		args((long sMem, const char *message));
void	free_mem        	args((void *pMem, long sMem));


/* random.c */
int	number_fuzzy		args((int number));
int	number_range		args((int from, int to));
int	number_percent		args((void));
int	number_door		args((void));
int	number_bits		args((int width));
long	number_mm		args((void));
int	dice			args((int number, int size));

/* load_config.c */
int     load_config     args((const char *filename));

/* help.c */
void	help		args((CHAR_DATA *ch, const char *argument));

/* string.c */
char *	str_dup_semiperm	args((const char *string));
char *  str_dup         args( ( const char *str ) );
void    free_string     args( ( char *pstr ) );
bool    str_cmp         args( ( const char *astr, const char *bstr ) );
bool    str_prefix      args( ( const char *astr, const char *bstr ) );
bool    str_prefix1     args( ( const char *astr, const char *bstr ) );
bool    str_infix       args( ( const char *astr, const char *bstr ) );
bool    str_suffix      args( ( const char *astr, const char *bstr ) );
const char *  capitalize      args( ( const char *str ) );
const char *  smash_bracket   args( ( const char *str ) );
const char *  smash_tilde     args( ( const char *str ) );
const char *	ignore_apostrophe args((const char *str));
void	strcut		args((char *str, int length));
const char *	strcenter	args((const char *string, int space));
const char *	strrpc		args((const char *replace, const char *with, const char *in));
const char *    strins          args((const char *string, const char *ins, int place));
const char *  center_string_in_whitespace args((const char *string, int length));


/* effect.c */
void    acid_effect     args( (void *vo, int level, int dam, int target, int evolution) );
void    cold_effect     args( (void *vo, int level, int dam, int target, int evolution) );
void    fire_effect     args( (void *vo, int level, int dam, int target, int evolution) );
void    poison_effect   args( (void *vo, int level, int dam, int target, int evolution) );
void    shock_effect    args( (void *vo, int level, int dam, int target, int evolution) );


/* fight.c */
void    check_killer    args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
bool    damage          args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
                                int dt, int class, bool show, bool spell ) );
void    death_cry       args( ( CHAR_DATA *ch ) );
bool    is_safe         args( (CHAR_DATA *ch, CHAR_DATA *victim, bool showmsg ) );
bool    is_safe_spell   args( (CHAR_DATA *ch, CHAR_DATA *victim, bool area ) );
bool	is_safe_char	args((CHAR_DATA *ch, CHAR_DATA *victim, bool showmsg));		/* Montrey */
void    multi_hit       args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void    raw_kill        args( ( CHAR_DATA *victim ) );
void    stop_fighting   args( ( CHAR_DATA *ch, bool fBoth ) );
void    update_pos      args( ( CHAR_DATA *victim ) );
void    violence_update args( ( void ) );

/* mob_prog.c */
#ifdef DUNNO_STRSTR
char *  strstr                  args ( (const char *s1, const char *s2 ) );
#endif

void    mprog_wordlist_check    args ( ( const char * arg, CHAR_DATA *mob,
                                        CHAR_DATA* actor, OBJ_DATA* object,
                                        void* vo, int type ) );
void    mprog_percent_check     args ( ( CHAR_DATA *mob, CHAR_DATA* actor,
                                        OBJ_DATA* object, void* vo,
                                        int type ) );
void    mprog_act_trigger       args ( ( const char* buf, CHAR_DATA* mob,
                                        CHAR_DATA* ch, OBJ_DATA* obj,
                                        void* vo ) );
void    mprog_bribe_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch,
                                        int amount ) );
void    mprog_entry_trigger     args ( ( CHAR_DATA* mob ) );
void    mprog_give_trigger      args ( ( CHAR_DATA* mob, CHAR_DATA* ch,
                                        OBJ_DATA* obj ) );
void    mprog_greet_trigger     args ( ( CHAR_DATA* mob ) );
void    mprog_fight_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_buy_trigger       args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_hitprcnt_trigger  args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_death_trigger     args ( ( CHAR_DATA* mob ) );
void    mprog_random_trigger    args ( ( CHAR_DATA* mob ) );
void    mprog_tick_trigger      args ( ( CHAR_DATA* mob ) );
void 	mprog_boot_trigger	args ( ( CHAR_DATA* mob	) );
void    mprog_speech_trigger    args ( ( const char* txt, CHAR_DATA* mob ) );


/* handler.c */
int     count_users     args( (OBJ_DATA *obj) );
bool    deduct_cost     args( (CHAR_DATA *ch, long cost) );
int     check_immune    args( (CHAR_DATA *ch, int dam_type) );
int     liq_lookup      args( ( const char *name) );
int     weapon_lookup   args( ( const char *name) );
int     weapon_type     args( ( const char *name) );
const char    *weapon_name    args( ( int weapon_Type) );
int     item_lookup     args( ( const char *name) );
const char    *item_name      args( ( int item_type) );
int     attack_lookup   args(( const char *name) );
int     race_lookup     args(( const char *name) );
int     class_lookup    args(( const char *name) );
int     deity_lookup    args(( const char *name) );
bool    is_clan         args((CHAR_DATA *ch) );
bool    is_same_clan    args((CHAR_DATA *ch, CHAR_DATA *victim));
int     get_skill       args(( CHAR_DATA *ch, int sn ) );
int     get_weapon_sn   args(( CHAR_DATA *ch, bool secondary ) );
int     get_weapon_skill args(( CHAR_DATA *ch, int sn ) );
void    reset_char      args(( CHAR_DATA *ch )  );
int     get_curr_stat   args(( CHAR_DATA *ch, int stat ) );
int     get_max_train   args(( CHAR_DATA *ch, int stat ) );
int     can_carry_n     args(( CHAR_DATA *ch ) );
int     can_carry_w     args(( CHAR_DATA *ch ) );
bool    is_name         args(( const char *str, const char *namelist ) );
bool    is_exact_name   args(( const char *str, const char *namelist ) );
bool    is_exact_name_color   args(( const char *str, const char *namelist ) );
void    copy_affect_to_char  args(( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    copy_affect_to_obj   args(( OBJ_DATA *obj, AFFECT_DATA *paf ) );
void    copy_affect_to_room  args(( ROOM_INDEX_DATA *room, AFFECT_DATA *paf ) );
void    affect_remove   args(( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    affect_remove_obj args((OBJ_DATA *obj, AFFECT_DATA *paf ) );
void    affect_remove_room args((ROOM_INDEX_DATA *obj, AFFECT_DATA *paf ) );
void    affect_strip    args(( CHAR_DATA *ch, int sn ) );
void    affect_join     args(( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    affect_combine  args(( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    char_from_room  args(( CHAR_DATA *ch ) );
void    char_to_room    args(( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
void    obj_to_char     args(( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    obj_from_char   args(( OBJ_DATA *obj ) );
void    obj_to_locker   args(( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    obj_to_strongbox args(( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    obj_from_locker args(( OBJ_DATA *obj ) );
void    obj_from_strongbox args(( OBJ_DATA *obj) );
int     apply_ac        args(( OBJ_DATA *obj, int iWear, int type ) );
OD *    get_eq_char     args(( CHAR_DATA *ch, int iWear ) );
void    equip_char      args(( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void    unequip_char    args(( CHAR_DATA *ch, OBJ_DATA *obj ) );
int     count_obj_list  args(( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void    obj_from_room   args(( OBJ_DATA *obj ) );
void    obj_to_room     args(( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
void    obj_to_obj      args(( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void    obj_from_obj    args(( OBJ_DATA *obj ) );
void    extract_obj     args(( OBJ_DATA *obj ) );
void    extract_char    args(( CHAR_DATA *ch, bool fPull ) );
OD *    get_obj_type    args(( OBJ_INDEX_DATA *pObjIndexData ) );
OD *    create_money    args(( int gold, int silver ) );
int     get_obj_number  args(( OBJ_DATA *obj ) );
int     get_obj_weight  args(( OBJ_DATA *obj ) );
int     get_true_weight args(( OBJ_DATA *obj ) );
bool    room_is_dark    args(( ROOM_INDEX_DATA *room));
bool    room_is_very_dark args((ROOM_INDEX_DATA *room));
bool    is_room_owner   args(( CHAR_DATA *ch, ROOM_INDEX_DATA *room) );
bool    room_is_private args(( ROOM_INDEX_DATA *pRoomIndex ) );
bool    can_see         args(( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    can_see_who     args(( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    can_see_obj     args(( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool    can_see_room    args(( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex) );
bool    can_drop_obj    args(( CHAR_DATA *ch, OBJ_DATA *obj ) );
const char *  item_type_name  args(( OBJ_DATA *obj ) );
const char *  affect_loc_name args(( int location ) );
const char *  affect_bit_name args(( int vector ) );
const char *  extra_bit_name  args(( int extra_flags ) );
const char *  wiz_bit_name    args(( int wiz_flags ) );
const char *  wear_bit_name   args(( int wear_flags ) );
const char *  act_bit_name    args(( int act_flags ) );
const char *  room_bit_name   args(( int room_flags ));
const char *  plr_bit_name    args(( int plr_flags ) );
const char *  off_bit_name    args(( int off_flags ) );
const char *  imm_bit_name    args(( int flags ) );
const char *  form_bit_name   args(( int form_flags ) );
const char *  part_bit_name   args(( int part_flags ) );
const char *  weapon_bit_name args(( int weapon_flags ) );
const char *  comm_bit_name   args(( int comm_flags ) );
const char *  revoke_bit_name args(( int revoke_flags ) );
const char *  cgroup_bit_name args(( int flags ));
const char *  censor_bit_name args(( int censor_flags ) );
const char *  cont_bit_name   args(( int cont_flags) );
const char *  first_arg       args(( const char *argument, char *arg_first, bool fCase ) );
const char *  get_who_line    args(( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    note_is_name    args(( const char *str, const char *namelist ) );
bool    mob_exists      args(( const char *name ) );
bool    has_slash       args(( const char *str ) );
bool    valid_character args(( CHAR_DATA *cd ) );
bool    valid_object    args(( OBJ_DATA *od ) );
//int	round			args((float fNum, int iInc));	/* below is Montrey's list of */
int	parse_deity		args((const char *dstring));		/* hacks!  Beware! :) */
int	get_usable_level	args((CHAR_DATA *ch));
int	get_holdable_level	args((CHAR_DATA *ch));
CLAN_DATA *clan_vnum_lookup	args((int vnum));
const char *	strins			args((const char *string, const char *ins, int place));
const char *	get_owner		args((CHAR_DATA *ch, OBJ_DATA *obj));
CD *	get_obj_carrier		args((OBJ_DATA *obj));
int	get_true_hitroll	args((CHAR_DATA *ch));
int	get_true_damroll	args((CHAR_DATA *ch));
int	get_armor_ac		args((CHAR_DATA *ch, int type));
int	get_locker_number	args((CHAR_DATA *ch));
int	get_locker_weight	args((CHAR_DATA *ch));
int	get_strongbox_number	args((CHAR_DATA *ch));
int	get_carry_number	args((CHAR_DATA *ch));
int	get_carry_weight	args((CHAR_DATA *ch));
int	get_position		args((CHAR_DATA *ch));
int	get_play_hours		args((CHAR_DATA *ch));
int	get_play_seconds	args((CHAR_DATA *ch));
int	get_age			args((CHAR_DATA *ch));
int	get_age_mod		args((CHAR_DATA *ch));
AD *	get_affect		args((AFFECT_DATA *af, int sn));
int	get_affect_evolution	args((CHAR_DATA *ch, int sn));
const char *	get_color_name		args((int color, int bold));
const char *	get_color_code		args((int color, int bold));
const char *	get_custom_color_name	args((CHAR_DATA *ch, int slot));
const char *	get_custom_color_code	args((CHAR_DATA *ch, int slot));
long	flag_convert		args((char letter));
int	interpolate		args((int level, int value_00, int value_32));
const char *	get_extra_descr		args((const char *name, EXTRA_DESCR_DATA *ed));

/* find.c */
CD *	get_mob_here		args((CHAR_DATA *ch, const char *argument, int vis));
CD *	get_mob_area		args((CHAR_DATA *ch, const char *argument, int vis));
CD *	get_mob_world		args((CHAR_DATA *ch, const char *argument, int vis));
CD *	get_char_here		args((CHAR_DATA *ch, const char *argument, int vis));
CD *    get_char_room       args((CHAR_DATA *ch, ROOM_INDEX_DATA *room, const char *argument, int vis));
CD *	get_char_area		args((CHAR_DATA *ch, const char *argument, int vis));
CD *	get_char_world		args((CHAR_DATA *ch, const char *argument, int vis));
CD *	get_player_here		args((CHAR_DATA *ch, const char *argument, int vis));
CD *	get_player_area		args((CHAR_DATA *ch, const char *argument, int vis));
CD *	get_player_world	args((CHAR_DATA *ch, const char *argument, int vis));
OD *	get_obj_list		args((CHAR_DATA *ch, const char *argument, OBJ_DATA *list));
OD *	get_obj_carry		args((CHAR_DATA *ch, const char *argument));
OD *	get_obj_wear		args((CHAR_DATA *ch, const char *argument));
OD *	get_obj_here		args((CHAR_DATA *ch, const char *argument));
OD *	get_obj_world		args((CHAR_DATA *ch, const char *argument));

/* objstate.c */
int     objstate_load_items		args((void));
int     objstate_save_items		args((void));

/* hunt.c */
void hunt_victim        args( ( CHAR_DATA *ch) );

/* interp.c */
void    interpret       args( ( CHAR_DATA *ch, const char *argument ) );
bool    is_number       args( ( const char *arg ) );
int     number_argument args( ( const char *argument, char *arg ) );
int     entity_argument args( ( const char *argument, char *arg ) );
int     mult_argument   args( ( const char *argument, char *arg) );
const char *  one_argument    args( ( const char *argument, char *arg_first ) );
bool    check_social    args( ( CHAR_DATA *ch, const char *command,
                            const char *argument ) );
void	do_huh		args( ( CHAR_DATA *ch ) );	/* Xenith */
bool    Is_Granted      args( (CHAR_DATA *ch, const char *argument) );


/* magic.c */
int     find_spell      args( ( CHAR_DATA *ch, const char *name) );
int     skill_lookup    args( ( const char *name ) );
int     slot_lookup     args( ( int slot ) );
bool    saves_spell     args( ( int level, CHAR_DATA *victim, int dam_type ) );
void    obj_cast_spell  args( ( int sn, int level, CHAR_DATA *ch,
                                    CHAR_DATA *victim, OBJ_DATA *obj ) );
void spell_imprint      args( ( int sn, int level, CHAR_DATA *ch, void *vo ));

/* note.c */
const char *format_string( const char *oldstring );
const char * string_replace( const char * orig, const char * old, const char * new );

/* quest.c */
void    quest_init       args( (void) );
void    sq_cleanup	 args( ( CHAR_DATA *ch ) );

/* save.c */
void    save_char_obj    args( ( CHAR_DATA *ch ) );
void    backup_char_obj  args( ( CHAR_DATA *ch ) );
bool    load_char_obj    args( ( DESCRIPTOR_DATA *d, const char *name ) );
//void    do_delete_finger args( ( CHAR_DATA *ch) );
//void    do_save_finger   args( ( CHAR_DATA *ch) );
const char    *dizzy_ctime     args( ( time_t *timep ) );
time_t  dizzy_scantime   args( ( const char *ctime ) );

/* skills.c */
bool    parse_gen_groups args( ( CHAR_DATA *ch, const char *argument ) );
void    list_group_costs args( ( CHAR_DATA *ch ) );
void    list_group_known args( ( CHAR_DATA *ch ) );
long    exp_per_level   args( ( CHAR_DATA *ch, int points ) );
void    check_improve   args( ( CHAR_DATA *ch, int sn, bool success,
                                    int multiplier ) );
int     group_lookup    args( (const char *name) );
void    gn_add          args( ( CHAR_DATA *ch, int gn) );
void    gn_remove       args( ( CHAR_DATA *ch, int gn) );
void    group_add       args( ( CHAR_DATA *ch, const char *name, bool deduct) );
void    group_remove    args( ( CHAR_DATA *ch, const char *name) );
int     get_evolution   args( ( CHAR_DATA *ch, int sn ) );
int	get_skill_cost  args( ( CHAR_DATA *ch, int sn ) );
bool	deduct_stamina  args( ( CHAR_DATA *ch, int sn ) );

/* special.c */
SF *    spec_lookup     args( ( const char *name ) );
const char *  spec_name       args( ( SPEC_FUN *function ) );
bool    IS_SPECIAL      args( ( CHAR_DATA *ch) );

/* teleport.c */
RID *   room_by_name    args( ( const char *target, int level, bool error) );

/* tour.c */
//void    do_alight       args( ( CHAR_DATA *ch, const char *argument ) );
//void    do_board        args( ( CHAR_DATA *ch, const char *argument ) );
//void    load_tourstarts args( ( FILE *fp ) );
//void    load_tourroutes args( ( FILE *fp ) );
//void    tour_update     args( ( void ) );

/* update.c */
void    advance_level   args( ( CHAR_DATA *ch ) );
void    npc_advance_level args( ( CHAR_DATA *ch ) );
void    demote_level    args( ( CHAR_DATA *ch ) );
void    gain_exp        args( ( CHAR_DATA *ch, int gain ) );
void    gain_condition  args( ( CHAR_DATA *ch, int iCond, int value ) );
void    update_handler  args( ( void ) );

/* remort.c */
int     raff_lookup     args( ( int index ) );
bool    HAS_RAFF	args( ( CHAR_DATA *ch, int flag ) );
bool    HAS_RAFF_GROUP  args( ( CHAR_DATA *ch, int flag ) );
bool    HAS_EXTRACLASS	args( ( CHAR_DATA *ch, int sn ) );
bool    CAN_USE_RSKILL  args( ( CHAR_DATA *ch, int sn ) );
void    list_extraskill args( ( CHAR_DATA *ch ) );

/* channels.c */


#define MSL MAX_STRING_LENGTH
#undef  CD
#undef  MID
#undef  OD
#undef  OID
#undef  RID
#undef  SF
#undef  AD

struct raffects
{
    char *description;
    char *shortname;
    int group;
    int id;
    int chance;
    long add;
};

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

#endif

