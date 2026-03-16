#pragma once

#include "Flags.hh"



/** 
 * @defgroup GameParameters Game Parameters
 * @brief Constants representing various game parameters and settings.
 * 
 * These constants define the limits, levels, and other configurations for the game.
 * @{
 */
constexpr int DIZZYPORT = 3000;

constexpr int MAX_STRING_LENGTH = 4608;
constexpr int MAX_INPUT_LENGTH = 256;
constexpr int MSL = MAX_STRING_LENGTH;
constexpr int MIL = MAX_INPUT_LENGTH;

constexpr int MAX_STATS = 6;
constexpr int MAX_RAFFECTS = 54;
constexpr int MAX_RAFFECT_SLOTS = 10;
constexpr int MAX_EXTRACLASS_SLOTS = 5;
constexpr int EXTRACLASS_SLOT_LEVELS = 10; /**< number of remorts per extra extraclass slot */
constexpr int MAX_THIEF = 250;
constexpr int MAX_KILLER = 250;
constexpr int MAX_QUERY = 20;
constexpr int MAX_LEVEL = 100;
constexpr int MAX_DONATED = 8000; /**< max number of items in the pit, affects do_pit performance.  max items in show_list_to_char is 8000 */
constexpr int LEVEL_AVATAR = MAX_LEVEL - 30;    /**< 80 */
constexpr int LEVEL_HERO = MAX_LEVEL - 9;       /**< 91 */
constexpr int LEVEL_IMMORTAL = MAX_LEVEL - 8;   /**< 92 */
constexpr int QPS_PER_PRAC = 20;
constexpr int QPS_PER_TRAIN = 200;
constexpr int MAX_BOW_DISTANCE = 3;
constexpr int NEW_LAY_COUNTER = 12; /**< hours it takes to recharge lay on hands */

constexpr int MAX_COLORS = 100; /**< 1 + Total Used ColorItems in do_color */

constexpr bool HALLOWEEN = true; /**< Halloween special event active */

constexpr const char* KEYWD_OWNER = "owned by"; /**< for personally owned equipment */

constexpr int LIQ_WATER         = 0;

constexpr int MAX_EQSOCKETS  = 6;

constexpr int PKTIME = 10;       /**< that's x3 seconds, 30 currently */

/** @} */

/** 
 * @defgroup ColorSlotTypes Color slot constants for player customizable stuff
 * @brief Color slot constants for player customizable stuff -- Elrac (Expanded by Xenith)
 * @{
 */
constexpr int CSLOT_CHAN_GOSSIP = 0; /**< default: GREEN, NOBOLD */
constexpr int CSLOT_CHAN_FLAME = 1; /**< default: RED, BOLD */
constexpr int CSLOT_CHAN_QWEST = 2; /**< default: YELLOW, BOLD */
constexpr int CSLOT_CHAN_PRAY = 3;      /**< default: RED, NOBOLD */
constexpr int CSLOT_CHAN_AUCTION = 4; /**< default: BROWN, NOBOLD */
constexpr int CSLOT_CHAN_ANNOUNCE = 5; /**< default: WHITE, BOLD */
constexpr int CSLOT_CHAN_MUSIC = 6; /**< default: BLUE, BOLD */
constexpr int CSLOT_CHAN_QA = 7; /**< default: CYAN, BOLD */
constexpr int CSLOT_CHAN_SOCIAL = 8; /**< default: GREEN, NOBOLD */
constexpr int CSLOT_CHAN_GRATS = 9; /**< default: PURPLE, BOLD */
constexpr int CSLOT_CHAN_IMM = 10; /**< default: RED, BOLD */
constexpr int CSLOT_CHAN_PAGE = 11; /**< default: RED, NOBOLD */
constexpr int CSLOT_CHAN_TELL = 12; /**< default: CYAN, BOLD */
constexpr int CSLOT_CHAN_CLAN = 13; /**< default: RED, NOBOLD */
constexpr int CSLOT_CHAN_IC = 14; /**< default: WHITE, BOLD */
constexpr int CSLOT_CHAN_SAY = 15; /**< default: WHITE, NOBOLD */
constexpr int CSLOT_CHAN_GTELL = 16; /**< default: PURPLE, BOLD */
constexpr int CSLOT_CHAN_QTELL = 17; /**< default: PURPLE, BOLD */
constexpr int CSLOT_CHAN_SPOUSE = 18; /**< default: YELLOW, BOLD */

constexpr int CSLOT_OLDSCORE_NAME = 30; /**< default: CYAN, BOLD */
constexpr int CSLOT_OLDSCORE_PKILL = 31; /**< default: PURPLE, NOBOLD */
constexpr int CSLOT_OLDSCORE_AKILL = 32; /**< default: PURPLE, BOLD */
constexpr int CSLOT_OLDSCORE_CLASS = 33; /**< default: CYAN, NOBOLD */
constexpr int CSLOT_OLDSCORE_STAT = 34; /**< default: RED, BOLD */
constexpr int CSLOT_OLDSCORE_ARMOR = 35; /**< default: BLUE, BOLD */
constexpr int CSLOT_OLDSCORE_POINTS = 36; /**< default: YELLOW, BOLD */
constexpr int CSLOT_OLDSCORE_WEIGHT = 37; /**< default: GREEN, NOBOLD */
constexpr int CSLOT_OLDSCORE_GAIN = 38; /**< default: YELLOW, NOBOLD */
constexpr int CSLOT_OLDSCORE_ALIGN = 39; /**< default: WHITE, BOLD */
constexpr int CSLOT_OLDSCORE_MONEY = 40; /**< default: RED, NOBOLD */
constexpr int CSLOT_OLDSCORE_DICEROLL = 41; /**< default: GREEN, BOLD */
constexpr int CSLOT_OLDSCORE_QP = 42; /**< default: YELLOW, BOLD */
constexpr int CSLOT_OLDSCORE_SP = 43; /**< default: GREEN, BOLD */
constexpr int CSLOT_OLDSCORE_RPP = 44; /**< default: PURPLE, BOLD */
/* MAX 49 */

constexpr int CSLOT_SCORE_TORCH = 50; /**< default: YELLOW, NOBOLD */
constexpr int CSLOT_SCORE_FLAME = 51; /**< default: RED, BOLD */
constexpr int CSLOT_SCORE_BORDER = 52; /**< default: YELLOW, NOBOLD */
constexpr int CSLOT_SCORE_NAME = 53; /**< default: CYAN, BOLD */
constexpr int CSLOT_SCORE_TITLE = 54; /**< default: WHITE, BOLD */
constexpr int CSLOT_SCORE_CLASS = 55; /**< default: CYAN, NOBOLD */
constexpr int CSLOT_SCORE_LEVEL = 56; /**< default: CYAN, NOBOLD */
constexpr int CSLOT_SCORE_STAT = 57; /**< default: RED, BOLD */
constexpr int CSLOT_SCORE_MAXSTAT = 58; /**< default: RED, NOBOLD */
constexpr int CSLOT_SCORE_HEALTHNAME = 59; /**< default: BLACK, BOLD */
constexpr int CSLOT_SCORE_HEALTHNUM = 60; /**< default: WHITE, NOBOLD */
constexpr int CSLOT_SCORE_WEALTH = 61; /**< default: CYAN, BOLD */
constexpr int CSLOT_SCORE_ENCUMB = 62; /**< default: GREEN, NOBOLD */
constexpr int CSLOT_SCORE_ARMOR = 63; /**< default: PURPLE, BOLD */
constexpr int CSLOT_SCORE_DICENAME = 64; /**< default: GREEN, BOLD */
constexpr int CSLOT_SCORE_DICENUM = 65; /**< default: YELLOW, BOLD */
constexpr int CSLOT_SCORE_POINTNAME = 66; /**< default: BLUE, BOLD */
constexpr int CSLOT_SCORE_POINTNUM = 67; /**< default: CYAN, BOLD */
constexpr int CSLOT_SCORE_ALIGN = 68; /**< default: GREY, BOLD */
constexpr int CSLOT_SCORE_XPNAME = 69; /**< default: WHITE, BOLD */
constexpr int CSLOT_SCORE_XPNUM = 70; /**< default: GREY, BOLD */
constexpr int CSLOT_SCORE_POSITION = 71; /**< default: GREEN, NOBOLD */
constexpr int CSLOT_SCORE_PKRECORD = 72; /**< default: RED, NOBOLD */
constexpr int CSLOT_SCORE_PKRANK = 73; /**< default: WHITE, BOLD */
/* MAX 79 */

constexpr int CSLOT_MISC_ROOM = 80; /**< default: RED, BOLD */
constexpr int CSLOT_MISC_MOBILES = 81; /**< default: GREEN, BOLD */
constexpr int CSLOT_MISC_OBJECTS = 82; /**< default: YELLOW, NOBOLD */
constexpr int CSLOT_MISC_PLAYERS = 83; /**< default: PURPLE, BOLD */
constexpr int CSLOT_MISC_INV = 84; /**< default: BROWN, NOBOLD */
/* MAX 99 */
/** @} */

/** 
 * @defgroup PulseConstants Pulse and timing constants
 * @brief Pulse and timing constants for game events and timers.
 * @{
 */
constexpr int PULSE_PER_SECOND = 4;
constexpr int PULSE_VIOLENCE = 3 * PULSE_PER_SECOND;
constexpr int PULSE_MOBILE = 4 * PULSE_PER_SECOND;
constexpr int PULSE_MUSIC = 6 * PULSE_PER_SECOND;
constexpr int PULSE_TICK = 45 * PULSE_PER_SECOND;
constexpr int PULSE_AREA = 120 * PULSE_PER_SECOND;
constexpr int PULSE_QUEST = 60 * PULSE_PER_SECOND;
constexpr int PULSE_AUCTION = 10 * PULSE_PER_SECOND;
constexpr int PULSE_TELEPORT = 20 * PULSE_PER_SECOND;
constexpr int PULSE_UNDERWATER = 15 * PULSE_PER_SECOND;
constexpr int PULSE_JANITOR = 300 * PULSE_PER_SECOND;
constexpr int PULSE_MYSQL_UPD = 10 * PULSE_PER_SECOND;

/* Timers for the tour coaches. */
constexpr int PULSE_TOURHALFSTEP	= 6 * PULSE_PER_SECOND;
constexpr int PULSE_TOURSTART		= 120 * PULSE_PER_SECOND;
constexpr int PULSE_TOURBOARD		= 30 * PULSE_PER_SECOND;
/** @} */


/** 
 * @defgroup VisibilityTypes Visibility Types
 * @brief Constants defining types of visibility for characters.
 * 
 * There are 2 types of invisibility:
 * - All-encompassing (wizi, superwiz, lurk), which are outside gameplay.
 * - Gameplay invisibility (invis, hide, sneak, midnight), which are for in-game use only.
 * 
 * These constants are used in `get_char` functions to return victims the character can see in either case. -- Montrey
 * @{
 */
constexpr int VIS_ALL = 0;
constexpr int VIS_CHAR = 1;
constexpr int VIS_PLR = 2;
/** @} */



/** 
 * @defgroup ActMessageTypes Act Message Types
 * @brief Constants defining types of message targets for the `act` function.
 * 
 * These constants are used to specify the target audience for messages generated by the `act` function.
 * @{
 */
constexpr int TO_ROOM = 0;          /**< Message sent to everyone in the room. */
constexpr int TO_NOTVICT = 1;       /**< Message sent to everyone except the victim. */
constexpr int TO_VICT = 2;          /**< Message sent only to the victim. */
constexpr int TO_CHAR = 3;          /**< Message sent only to the character. */
constexpr int TO_ALL = 4;           /**< Message sent to everyone, regardless of location. */
constexpr int TO_WORLD = 5;         /**< Message sent globally to the world. */
constexpr int TO_VIEW = 6;          /**< Message sent to the viewing room for special cases. */
constexpr int TO_NOTVIEW = 7;       /**< Message explicitly excluded from viewing rooms. */
constexpr int TO_VICT_CHANNEL = 8; /**< Special case for channel visibility, modified later. */
/** @} */


/** 
 * @defgroup StatTypes Stat Types
 * @brief Constants representing character stats.
 * 
 * These constants define the various stats that characters can have in the game.
 * @{
 */
constexpr int STAT_STR = 0;
constexpr int STAT_INT = 1;
constexpr int STAT_WIS = 2;
constexpr int STAT_DEX = 3;
constexpr int STAT_CON = 4;
constexpr int STAT_CHR = 5;
/** @} */


/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/** 
 * @defgroup MobVnums Mob Virtual Numbers
 * @brief Constants representing well-known mob virtual numbers and group identifiers.
 * 
 * These constants are used to define specific mob types and groups in the game.
 * @{
 */
constexpr int MOB_VNUM_FIDO = 3090;
constexpr int MOB_VNUM_HELGA = 3011;
constexpr int MOB_VNUM_CITYGUARD = 3060;
constexpr int MOB_VNUM_VAMPIRE = 3404;

constexpr int MOB_VNUM_PATROLMAN = 2106;

constexpr int MOB_VNUM_SKELETON = 2;
constexpr int MOB_VNUM_ZOMBIE = 3;
constexpr int MOB_VNUM_WRAITH = 4;
constexpr int MOB_VNUM_GARGOYLE = 5;
constexpr int MOB_VNUM_RESZOMBIE = 6;

constexpr int MOB_VNUM_SQUESTMOB = 85;
/** @} */

/** 
 * @defgroup GangLandFlags Gang Land Flags
 * @brief Flags to enable some mob behavior in the gang land area.
 * @{
 */
constexpr Flags::Bit GROUP_VNUM_TROLLS  = Flags::A; /**< 2100 */
constexpr Flags::Bit GROUP_VNUM_OGRES  = Flags::B; /**< 2101 */
/** @} */

/** 
 * @defgroup ColorConstants Color Constants
 * @brief Constants representing color codes and attributes for text formatting.
 * 
 * The first number (bold = 0,1) gives you dim or BRIGHT colors.
 * The second number determines the colors as follows:
 * - 30 = grey
 * - 31 = red
 * - 32 = green
 * - 33 = yellow
 * - 34 = blue
 * - 35 = purple
 * - 36 = cyan
 * - 37 = white
 * 
 * Special notes:
 * - On a Mac, the colors may be screwy.
 * - 0,30 gives you black.
 * - 0,34 seems to be REALLY dark so it doesn't show up too well.
 * @{
 */
constexpr int NOBOLD = 0;
constexpr int BOLD = 1;
constexpr int BLINK = 5;
constexpr int REVERSE = 7;
constexpr int UNDERLINE = 4;

constexpr int GREY = 30;
constexpr int RED = 31;
constexpr int GREEN = 32;
constexpr int YELLOW = 33;
constexpr int BLUE = 34;
constexpr int PURPLE = 35;
constexpr int CYAN = 36;
constexpr int WHITE = 37;
/** @} */

/** 
 * @defgroup ColorEscapeConstants Color Escape Constants
 * @brief Constants representing ANSI escape codes for text formatting.
 * 
 * Colour definitions by Lope of Loping Through The MUD.
 * These constants are used to define ANSI escape sequences for various text formatting options, such as colors and styles.
 * @{
 */
constexpr const char* C_UNDERLINE =       "\033[4m";
constexpr const char* C_FLASH =           "\033[5m";
constexpr const char* C_REVERSE =         "\033[7m";
constexpr const char* CLEAR =           "\033[0m";
constexpr const char* C_BLACK =         "\033[0;30m";
constexpr const char* C_RED =           "\033[0;31m";
constexpr const char* C_GREEN =         "\033[0;32m";
constexpr const char* C_YELLOW =        "\033[0;33m";
constexpr const char* C_BLUE =          "\033[0;34m";
constexpr const char* C_MAGENTA =       "\033[0;35m";
constexpr const char* C_CYAN =          "\033[0;36m";
constexpr const char* C_WHITE =         "\033[0;37m";
constexpr const char* C_B_GREY =        "\033[1;30m";
constexpr const char* C_B_RED =         "\033[1;31m";
constexpr const char* C_B_GREEN =       "\033[1;32m";
constexpr const char* C_B_YELLOW =      "\033[1;33m";
constexpr const char* C_B_BLUE =        "\033[1;34m";
constexpr const char* C_B_MAGENTA =     "\033[1;35m";
constexpr const char* C_B_CYAN =        "\033[1;36m";
constexpr const char* C_B_WHITE =       "\033[1;37m";
constexpr const char* B_BLACK =         "\033[40m";
constexpr const char* B_RED =           "\033[41m";
constexpr const char* B_GREEN =         "\033[42m";
constexpr const char* B_YELLOW =        "\033[43m";
constexpr const char* B_BLUE =          "\033[44m";
constexpr const char* B_MAGENTA =       "\033[45m";
constexpr const char* B_CYAN =          "\033[46m";
constexpr const char* B_GREY =          "\033[47m";
/** @} */


/** 
 * @defgroup BanFlags Ban Flags
 * @brief Constants representing flags for banning players or connections.
 * 
 * These flags are used to specify the type of ban applied to players or connections.
 * @{
 */
constexpr Flags::Bit BAN_SUFFIX  = Flags::A;
constexpr Flags::Bit BAN_PREFIX  = Flags::B;
constexpr Flags::Bit BAN_NEWBIES = Flags::C;
constexpr Flags::Bit BAN_ALL     = Flags::D;
constexpr Flags::Bit BAN_PERMIT  = Flags::E;
/** @} */

/** 
 * @defgroup MobActFlags Mob ACT Flags
 * @brief Constants representing behavior flags for mobs.
 * 
 * These flags define various behaviors and attributes for mobs in the game.
 * @{
 */
//constexpr Flags::Bit ACT_IS_NPC                = Flags::A; /**< Auto set for mobs */
constexpr Flags::Bit ACT_NOSUMMON  = Flags::A; /**< now matches plr_nosummon */
constexpr Flags::Bit ACT_SENTINEL  = Flags::B; /**< Stays in one room */
constexpr Flags::Bit ACT_SCAVENGER  = Flags::C; /**< Picks up objects */
constexpr Flags::Bit ACT_WHOKNOWS1  = Flags::D; /**< Unknown flag */
constexpr Flags::Bit ACT_MORPH  = Flags::E; /**< For Morphed PC's */
constexpr Flags::Bit ACT_AGGRESSIVE  = Flags::F; /**< Attacks PC's */
constexpr Flags::Bit ACT_STAY_AREA  = Flags::G; /**< Won't leave area */
constexpr Flags::Bit ACT_WIMPY  = Flags::H;
constexpr Flags::Bit ACT_PET  = Flags::I; /**< Auto set for pets */
constexpr Flags::Bit ACT_TRAIN  = Flags::J; /**< Can train PC's */
constexpr Flags::Bit ACT_PRACTICE  = Flags::K; /**< Can practice PC's */
constexpr Flags::Bit ACT_STAY  = Flags::L;
constexpr Flags::Bit ACT_AGGR_ALIGN  = Flags::M; /**< Fights opp. align */
constexpr Flags::Bit ACT_SUPERMOB  = Flags::N; /**< complete invis */
constexpr Flags::Bit ACT_UNDEAD  = Flags::O;

constexpr Flags::Bit ACT_CLERIC  = Flags::Q;
constexpr Flags::Bit ACT_MAGE  = Flags::R;
constexpr Flags::Bit ACT_THIEF  = Flags::S;
constexpr Flags::Bit ACT_WARRIOR  = Flags::T;
constexpr Flags::Bit ACT_NOALIGN  = Flags::U;
constexpr Flags::Bit ACT_NOPURGE  = Flags::V;
constexpr Flags::Bit ACT_OUTDOORS  = Flags::W;
constexpr Flags::Bit ACT_INDOORS  = Flags::Y;
constexpr Flags::Bit ACT_NOPUSH  = Flags::Z;
constexpr Flags::Bit ACT_IS_HEALER  = Flags::a;
constexpr Flags::Bit ACT_GAIN  = Flags::b;
constexpr Flags::Bit ACT_NOMORPH  = Flags::c;
constexpr Flags::Bit ACT_IS_CHANGER  = Flags::d;
/** @} */


/** 
 * @defgroup DamageTypeConstants Damage Type Constants
 * @brief Constants representing various types of damage in the game.
 * 
 * These constants define the damage classes used for attacks, spells, and other effects.
 * @{
 */
constexpr int DAM_NONE = 0;
constexpr int DAM_BASH = 1;
constexpr int DAM_PIERCE = 2;
constexpr int DAM_SLASH = 3;
constexpr int DAM_FIRE = 4;
constexpr int DAM_COLD = 5;
constexpr int DAM_ELECTRICITY = 6;
constexpr int DAM_ACID = 7;
constexpr int DAM_POISON = 8;
constexpr int DAM_NEGATIVE = 9;
constexpr int DAM_HOLY = 10;
constexpr int DAM_ENERGY = 11;
constexpr int DAM_MENTAL = 12;
constexpr int DAM_DISEASE = 13;
constexpr int DAM_DROWNING = 14;
constexpr int DAM_LIGHT = 15;
constexpr int DAM_OTHER = 16;
constexpr int DAM_HARM = 17;
constexpr int DAM_CHARM = 18;
constexpr int DAM_SOUND = 19;
constexpr int DAM_WATER = 20;
constexpr int DAM_WOOD = 21;
constexpr int DAM_SILVER = 22;
constexpr int DAM_IRON = 23;
/** @} */

/** 
 * @defgroup MobileOffensiveFlags Mobile Offensive Flags
 * @brief Constants representing offensive abilities and assist behaviors for mobiles.
 * 
 * These flags define various offensive actions and assist behaviors that mobiles can perform in the game.
 * @{
 */
constexpr Flags::Bit OFF_AREA_ATTACK  = Flags::A;
constexpr Flags::Bit OFF_BACKSTAB     = Flags::B;
constexpr Flags::Bit OFF_BASH         = Flags::C;
constexpr Flags::Bit OFF_BERSERK      = Flags::D;
constexpr Flags::Bit OFF_DISARM       = Flags::E;
constexpr Flags::Bit OFF_DODGE        = Flags::F;
constexpr Flags::Bit OFF_FADE         = Flags::G;
constexpr Flags::Bit OFF_FAST         = Flags::H;
constexpr Flags::Bit OFF_KICK         = Flags::I;
constexpr Flags::Bit OFF_KICK_DIRT    = Flags::J;
constexpr Flags::Bit OFF_PARRY        = Flags::K;
constexpr Flags::Bit OFF_RESCUE       = Flags::L;
constexpr Flags::Bit OFF_TAIL         = Flags::M;
constexpr Flags::Bit OFF_TRIP         = Flags::N;
constexpr Flags::Bit OFF_CRUSH        = Flags::O;
constexpr Flags::Bit ASSIST_ALL       = Flags::P;
constexpr Flags::Bit ASSIST_ALIGN     = Flags::Q;
constexpr Flags::Bit ASSIST_RACE      = Flags::R;
constexpr Flags::Bit ASSIST_PLAYERS   = Flags::S;
constexpr Flags::Bit ASSIST_GUARD     = Flags::T;
constexpr Flags::Bit ASSIST_VNUM      = Flags::U;
/** @} */

/** 
 * @defgroup CheckImmReturnValues Check Imm Return Values
 * @brief Constants representing return values for the `check_imm` function.
 * 
 * These constants define the possible outcomes of immunity checks for characters or mobs.
 * @{
 */
constexpr int IS_VULNERABLE = 0;
constexpr int IS_NORMAL = 1;
constexpr int IS_RESISTANT = 2;
constexpr int IS_IMMUNE = 3;
constexpr int IS_ABSORBING = 4;
/** @} */

/** 
 * @defgroup DamageTypeImmunityFlags Damage Type Immunity Flags
 * @brief Constants representing immunity to various types of damage.
 * 
 * These constants define the types of damage that mobs can be immune to in the game.
 * @{
 */
//constexpr Flags::Bit IMM_SUMMON                = Flags::A;
constexpr Flags::Bit IMM_CHARM        = Flags::B;
constexpr Flags::Bit IMM_MAGIC        = Flags::C;
constexpr Flags::Bit IMM_WEAPON       = Flags::D;
constexpr Flags::Bit IMM_BASH         = Flags::E;
constexpr Flags::Bit IMM_PIERCE       = Flags::F;
constexpr Flags::Bit IMM_SLASH        = Flags::G;
constexpr Flags::Bit IMM_FIRE         = Flags::H;
constexpr Flags::Bit IMM_COLD         = Flags::I;
constexpr Flags::Bit IMM_ELECTRICITY  = Flags::J;
constexpr Flags::Bit IMM_ACID         = Flags::K;
constexpr Flags::Bit IMM_POISON       = Flags::L;
constexpr Flags::Bit IMM_NEGATIVE     = Flags::M;
constexpr Flags::Bit IMM_HOLY         = Flags::N;
constexpr Flags::Bit IMM_ENERGY       = Flags::O;
constexpr Flags::Bit IMM_MENTAL       = Flags::P;
constexpr Flags::Bit IMM_DISEASE      = Flags::Q;
constexpr Flags::Bit IMM_DROWNING     = Flags::R;
constexpr Flags::Bit IMM_LIGHT        = Flags::S;
constexpr Flags::Bit IMM_SOUND        = Flags::T;
constexpr Flags::Bit IMM_WOOD         = Flags::X;
constexpr Flags::Bit IMM_SILVER       = Flags::Y;
constexpr Flags::Bit IMM_IRON         = Flags::Z;
/** @} */

/** 
 * @defgroup DamageTypeResistanceFlags Damage Type Resistance Flags
 * @brief Constants representing resistance to various types of damage.
 * 
 * These constants define the types of damage that mobs can resist in the game.
 * @{
 */
constexpr Flags::Bit RES_CHARM        = Flags::B;
constexpr Flags::Bit RES_MAGIC        = Flags::C;
constexpr Flags::Bit RES_WEAPON       = Flags::D;
constexpr Flags::Bit RES_BASH         = Flags::E;
constexpr Flags::Bit RES_PIERCE       = Flags::F;
constexpr Flags::Bit RES_SLASH        = Flags::G;
constexpr Flags::Bit RES_FIRE         = Flags::H;
constexpr Flags::Bit RES_COLD         = Flags::I;
constexpr Flags::Bit RES_ELECTRICITY  = Flags::J;
constexpr Flags::Bit RES_ACID         = Flags::K;
constexpr Flags::Bit RES_POISON       = Flags::L;
constexpr Flags::Bit RES_NEGATIVE     = Flags::M;
constexpr Flags::Bit RES_HOLY         = Flags::N;
constexpr Flags::Bit RES_ENERGY       = Flags::O;
constexpr Flags::Bit RES_MENTAL       = Flags::P;
constexpr Flags::Bit RES_DISEASE      = Flags::Q;
constexpr Flags::Bit RES_DROWNING     = Flags::R;
constexpr Flags::Bit RES_LIGHT        = Flags::S;
constexpr Flags::Bit RES_SOUND        = Flags::T;
constexpr Flags::Bit RES_WOOD         = Flags::X;
constexpr Flags::Bit RES_SILVER       = Flags::Y;
constexpr Flags::Bit RES_IRON         = Flags::Z;
/** @} */

/** 
 * @defgroup DamageTypeVulnerabilityFlags Damage Type Vulnerability Flags
 * @brief Constants representing vulnerabilities to various types of damage.
 * 
 * These constants define the types of damage that mobs are vulnerable to in the game.
 * @{
 */
constexpr Flags::Bit VULN_CHARM        = Flags::B;
constexpr Flags::Bit VULN_MAGIC        = Flags::C;
constexpr Flags::Bit VULN_WEAPON       = Flags::D;
constexpr Flags::Bit VULN_BASH         = Flags::E;
constexpr Flags::Bit VULN_PIERCE       = Flags::F;
constexpr Flags::Bit VULN_SLASH        = Flags::G;
constexpr Flags::Bit VULN_FIRE         = Flags::H;
constexpr Flags::Bit VULN_COLD         = Flags::I;
constexpr Flags::Bit VULN_ELECTRICITY  = Flags::J;
constexpr Flags::Bit VULN_ACID         = Flags::K;
constexpr Flags::Bit VULN_POISON       = Flags::L;
constexpr Flags::Bit VULN_NEGATIVE     = Flags::M;
constexpr Flags::Bit VULN_HOLY         = Flags::N;
constexpr Flags::Bit VULN_ENERGY       = Flags::O;
constexpr Flags::Bit VULN_MENTAL       = Flags::P;
constexpr Flags::Bit VULN_DISEASE      = Flags::Q;
constexpr Flags::Bit VULN_DROWNING     = Flags::R;
constexpr Flags::Bit VULN_LIGHT        = Flags::S;
constexpr Flags::Bit VULN_SOUND        = Flags::T;
constexpr Flags::Bit VULN_WOOD         = Flags::X;
constexpr Flags::Bit VULN_SILVER       = Flags::Y;
constexpr Flags::Bit VULN_IRON         = Flags::Z;
/** @} */

/** 
 * @defgroup BodyFormFlags Body Form Flags
 * @brief Constants representing various body forms and attributes for mobs.
 * 
 * These constants define the physical and magical properties of mob bodies in the game.
 * @{
 */
constexpr Flags::Bit FORM_EDIBLE        = Flags::A;
constexpr Flags::Bit FORM_POISON        = Flags::B;
constexpr Flags::Bit FORM_MAGICAL       = Flags::C;
constexpr Flags::Bit FORM_INSTANT_DECAY = Flags::D;
constexpr Flags::Bit FORM_OTHER         = Flags::E; /**< Defined by material bit */

/* actual form */
constexpr Flags::Bit FORM_ANIMAL  = Flags::G;
constexpr Flags::Bit FORM_SENTIENT  = Flags::H;
constexpr Flags::Bit FORM_UNDEAD  = Flags::I;
constexpr Flags::Bit FORM_CONSTRUCT  = Flags::J;
constexpr Flags::Bit FORM_MIST  = Flags::K;
constexpr Flags::Bit FORM_INTANGIBLE  = Flags::L;

constexpr Flags::Bit FORM_BIPED  = Flags::M;
constexpr Flags::Bit FORM_CENTAUR  = Flags::N;
constexpr Flags::Bit FORM_INSECT  = Flags::O;
constexpr Flags::Bit FORM_SPIDER  = Flags::P;
constexpr Flags::Bit FORM_CRUSTACEAN  = Flags::Q;
constexpr Flags::Bit FORM_WORM  = Flags::R;
constexpr Flags::Bit FORM_BLOB  = Flags::S;

constexpr Flags::Bit FORM_MAMMAL  = Flags::V;
constexpr Flags::Bit FORM_BIRD  = Flags::W;
constexpr Flags::Bit FORM_REPTILE  = Flags::X;
constexpr Flags::Bit FORM_SNAKE  = Flags::Y;
constexpr Flags::Bit FORM_DRAGON  = Flags::Z;
constexpr Flags::Bit FORM_AMPHIBIAN  = Flags::a;
constexpr Flags::Bit FORM_FISH  = Flags::b;
constexpr Flags::Bit FORM_COLD_BLOOD  = Flags::c;
/** @} */

/** 
 * @defgroup BodyPartFlags Body Part Flags
 * @brief Constants representing various body parts and combat-related attributes for mobs.
 * 
 * These constants define the physical body parts and combat-related features of mobs in the game.
 * @{
 */
/* Body parts */
constexpr Flags::Bit PART_HEAD        = Flags::A;
constexpr Flags::Bit PART_ARMS        = Flags::B;
constexpr Flags::Bit PART_LEGS        = Flags::C;
constexpr Flags::Bit PART_HEART       = Flags::D;
constexpr Flags::Bit PART_BRAINS      = Flags::E;
constexpr Flags::Bit PART_GUTS        = Flags::F;
constexpr Flags::Bit PART_HANDS       = Flags::G;
constexpr Flags::Bit PART_FEET        = Flags::H;
constexpr Flags::Bit PART_FINGERS     = Flags::I;
constexpr Flags::Bit PART_EAR         = Flags::J;
constexpr Flags::Bit PART_EYE         = Flags::K;
constexpr Flags::Bit PART_LONG_TONGUE = Flags::L;
constexpr Flags::Bit PART_EYESTALKS   = Flags::M;
constexpr Flags::Bit PART_TENTACLES   = Flags::N;
constexpr Flags::Bit PART_FINS        = Flags::O;
constexpr Flags::Bit PART_WINGS       = Flags::P;
constexpr Flags::Bit PART_TAIL        = Flags::Q;

/* Combat-related body parts */
constexpr Flags::Bit PART_CLAWS       = Flags::U;
constexpr Flags::Bit PART_FANGS       = Flags::V;
constexpr Flags::Bit PART_HORNS       = Flags::W;
constexpr Flags::Bit PART_SCALES      = Flags::X;
constexpr Flags::Bit PART_TUSKS       = Flags::Y;
/** @} */


/** 
 * @defgroup MobileAffectFlags Mobile Affect Flags
 * @brief Constants representing various affect flags for mobs and characters.
 * 
 * These constants define the effects and statuses that can be applied to mobs and characters in the game.
 * @{
 */
/* Bits for 'affect_bits'. Used in #MOBILES. */
constexpr Flags::Bit AFF_BLIND         = Flags::A;
constexpr Flags::Bit AFF_INVISIBLE     = Flags::B;
constexpr Flags::Bit AFF_DETECT_EVIL   = Flags::C;
constexpr Flags::Bit AFF_DETECT_INVIS  = Flags::D;
constexpr Flags::Bit AFF_DETECT_MAGIC  = Flags::E;
constexpr Flags::Bit AFF_DETECT_HIDDEN = Flags::F;
constexpr Flags::Bit AFF_DETECT_GOOD   = Flags::G;
constexpr Flags::Bit AFF_SANCTUARY     = Flags::H;
constexpr Flags::Bit AFF_FAERIE_FIRE   = Flags::I;
constexpr Flags::Bit AFF_INFRARED      = Flags::J;
constexpr Flags::Bit AFF_CURSE         = Flags::K;
constexpr Flags::Bit AFF_FEAR          = Flags::L;
constexpr Flags::Bit AFF_POISON        = Flags::M;
constexpr Flags::Bit AFF_PROTECT_EVIL  = Flags::N;
constexpr Flags::Bit AFF_PROTECT_GOOD  = Flags::O;
constexpr Flags::Bit AFF_SNEAK         = Flags::P;
constexpr Flags::Bit AFF_HIDE          = Flags::Q;
constexpr Flags::Bit AFF_NIGHT_VISION  = Flags::R;
constexpr Flags::Bit AFF_CHARM         = Flags::S;
constexpr Flags::Bit AFF_FLYING        = Flags::T;
constexpr Flags::Bit AFF_PASS_DOOR     = Flags::U;
constexpr Flags::Bit AFF_HASTE         = Flags::V;
constexpr Flags::Bit AFF_CALM          = Flags::W;
constexpr Flags::Bit AFF_PLAGUE        = Flags::X;
constexpr Flags::Bit AFF_STEEL         = Flags::Y;
constexpr Flags::Bit AFF_DIVINEREGEN   = Flags::Z;
constexpr Flags::Bit AFF_BERSERK       = Flags::a;
constexpr Flags::Bit AFF_FLAMESHIELD   = Flags::b;
constexpr Flags::Bit AFF_REGENERATION  = Flags::c;
constexpr Flags::Bit AFF_SLOW          = Flags::d;
constexpr Flags::Bit AFF_TALON         = Flags::e;
/** @} */


/** 
 * @defgroup SexConstants Sex Constants
 * @brief Constants representing the sexes of characters and mobs.
 * 
 * These constants define the possible sexes for characters and mobs in the game.
 * @{
 */
constexpr int SEX_NEUTRAL = 0;
constexpr int SEX_MALE = 1;
constexpr int SEX_FEMALE = 2;
/** @} */

/** 
 * @defgroup ArmorClassTypes Armor Class Types
 * @brief Constants representing types of armor class.
 * 
 * These constants define the types of armor class used for damage mitigation in the game.
 * @{
 */
constexpr int AC_PIERCE = 0;
constexpr int AC_BASH = 1;
constexpr int AC_SLASH = 2;
constexpr int AC_EXOTIC = 3;
/** @} */

/** 
 * @defgroup DiceConstants Dice Constants
 * @brief Constants representing dice attributes for rolls.
 * 
 * These constants define the attributes of dice rolls used in the game mechanics.
 * @{
 */
constexpr int DICE_NUMBER = 0;
constexpr int DICE_TYPE = 1;
constexpr int DICE_BONUS = 2;
/** @} */

/** 
 * @defgroup SizeConstants Size Constants
 * @brief Constants representing the sizes of characters and mobs.
 * 
 * These constants define the possible sizes for characters and mobs in the game.
 * @{
 */
constexpr int SIZE_TINY = 0;
constexpr int SIZE_SMALL = 1;
constexpr int SIZE_MEDIUM = 2;
constexpr int SIZE_LARGE = 3;
constexpr int SIZE_HUGE = 4;
constexpr int SIZE_GIANT = 5;
/** @} */

/** 
 * @defgroup GenericObjectTypes Generic Object Types
 * @brief Constants representing generic object types for creation.
 * 
 * These constants define the types of generic objects that can be created in the game.
 * @{
 */
constexpr int GEN_OBJ_TREASURE = 108;
constexpr int GEN_OBJ_FOOD = 119;
constexpr int GEN_OBJ_MONEY = 120;
constexpr int GEN_OBJ_TOKEN = 141;
/** @} */

/** 
 * @defgroup ObjectVnums Object Virtual Numbers
 * @brief Constants representing well-known object virtual numbers.
 * 
 * These constants define specific object types and their virtual numbers used in the game.
 * @{
 */
constexpr int OBJ_VNUM_CORPSE_NPC = 10;
constexpr int OBJ_VNUM_CORPSE_PC = 11;
constexpr int OBJ_VNUM_SEVERED_HEAD = 12;
constexpr int OBJ_VNUM_TORN_HEART = 13;
constexpr int OBJ_VNUM_SLICED_ARM = 14;
constexpr int OBJ_VNUM_SLICED_LEG = 15;
constexpr int OBJ_VNUM_GUTS = 16;
constexpr int OBJ_VNUM_BRAINS = 17;
constexpr int OBJ_VNUM_DOO = 71;

constexpr int OBJ_VNUM_LIGHT_BALL = 21;
constexpr int OBJ_VNUM_SPRING = 22;
constexpr int OBJ_VNUM_DISC = 23;
constexpr int OBJ_VNUM_PORTAL = 25;
constexpr int OBJ_VNUM_TORCH = 3030;
constexpr int OBJ_VNUM_NEWBIEBAG = 72;
constexpr int OBJ_VNUM_WEAPON = 1004;
constexpr int OBJ_VNUM_HOLYSWORD = 18;

constexpr int OBJ_VNUM_SIGN = 1003;
constexpr int OBJ_VNUM_CAMP = 1002;
constexpr int OBJ_VNUM_VIAL = 3381;
constexpr int OBJ_VNUM_PARCH = 3380;

constexpr int OBJ_VNUM_PIT = 3010;

constexpr int OBJ_VNUM_SCHOOL_MACE = 3700;
constexpr int OBJ_VNUM_SCHOOL_DAGGER = 3701;
constexpr int OBJ_VNUM_SCHOOL_SWORD = 3702;
constexpr int OBJ_VNUM_SCHOOL_SPEAR = 3717;
constexpr int OBJ_VNUM_SCHOOL_STAFF = 3718;
constexpr int OBJ_VNUM_SCHOOL_AXE = 3719;
constexpr int OBJ_VNUM_SCHOOL_FLAIL = 3720;
constexpr int OBJ_VNUM_SCHOOL_WHIP = 3721;
constexpr int OBJ_VNUM_SCHOOL_POLEARM = 3722;
constexpr int OBJ_VNUM_SCHOOL_BOW = 3727;

constexpr int OBJ_VNUM_SCHOOL_VEST = 3703;
constexpr int OBJ_VNUM_SCHOOL_SHIELD = 3704;
constexpr int OBJ_VNUM_SCHOOL_BANNER = 3716;
constexpr int OBJ_VNUM_MAP = 3162;
constexpr int OBJ_VNUM_TOKEN = 3379;

constexpr int OBJ_VNUM_WHISTLE = 2116;

constexpr int OBJ_VNUM_SQUESTOBJ = 85;
/** @} */

/** 
 * @defgroup ItemTypes Item Types
 * @brief Constants representing various item types in the game.
 * 
 * These constants define the types of items that can exist in the game, such as weapons, armor, and consumables.
 * @{
 */
constexpr int ITEM_LIGHT = 1;
constexpr int ITEM_SCROLL = 2;
constexpr int ITEM_WAND = 3;
constexpr int ITEM_STAFF = 4;
constexpr int ITEM_WEAPON = 5;
constexpr int ITEM_TREASURE = 8;
constexpr int ITEM_ARMOR = 9;
constexpr int ITEM_POTION = 10;
constexpr int ITEM_CLOTHING = 11;
constexpr int ITEM_FURNITURE = 12;
constexpr int ITEM_TRASH = 13;
constexpr int ITEM_CONTAINER = 15;
constexpr int ITEM_DRINK_CON = 17;
constexpr int ITEM_KEY = 18;
constexpr int ITEM_FOOD = 19;
constexpr int ITEM_MONEY = 20;
constexpr int ITEM_BOAT = 22;
constexpr int ITEM_CORPSE_NPC = 23;
constexpr int ITEM_CORPSE_PC = 24;
constexpr int ITEM_FOUNTAIN = 25;
constexpr int ITEM_PILL = 26;
constexpr int ITEM_MAP = 28;
constexpr int ITEM_PORTAL = 29;
constexpr int ITEM_WARP_STONE = 30;
//constexpr int ITEM_ROOM_KEY = 31;
constexpr int ITEM_GEM = 32;
constexpr int ITEM_JEWELRY = 33;
constexpr int ITEM_JUKEBOX = 34;
constexpr int ITEM_PBTUBE = 35;
constexpr int ITEM_PBGUN = 36;
constexpr int ITEM_MATERIAL = 37;
constexpr int ITEM_ANVIL = 38;
//constexpr int ITEM_COACH = 39;
constexpr int ITEM_WEDDINGRING = 40;
constexpr int ITEM_TOKEN = 41;
constexpr int ITEM_WARP_CRYSTAL = 42;
constexpr int ITEM_QUESTSHOP = 999; /**< pseudo item type for quest shop -- Elrac */
/** @} */


/** 
 * @defgroup ExtraItemFlags Extra Item Flags
 * @brief Constants representing additional attributes for items.
 * 
 * These flags define special properties and behaviors for items in the game.
 * @{
 */
constexpr Flags::Bit ITEM_GLOW                 = Flags::A;
constexpr Flags::Bit ITEM_HUM                  = Flags::B;
constexpr Flags::Bit ITEM_COMPARTMENT          = Flags::C; /**< replaced ITEM_DARK */
constexpr Flags::Bit ITEM_TRANSPARENT          = Flags::D; /**< replaced ITEM_LOCK */
constexpr Flags::Bit ITEM_EVIL                 = Flags::E;
constexpr Flags::Bit ITEM_INVIS                = Flags::F;
constexpr Flags::Bit ITEM_MAGIC                = Flags::G;
constexpr Flags::Bit ITEM_NODROP               = Flags::H;
constexpr Flags::Bit ITEM_BLESS                = Flags::I;
constexpr Flags::Bit ITEM_ANTI_GOOD            = Flags::J;
constexpr Flags::Bit ITEM_ANTI_EVIL            = Flags::K;
constexpr Flags::Bit ITEM_ANTI_NEUTRAL         = Flags::L;
constexpr Flags::Bit ITEM_NOREMOVE             = Flags::M;
constexpr Flags::Bit ITEM_INVENTORY            = Flags::N;
constexpr Flags::Bit ITEM_NOPURGE              = Flags::O;
constexpr Flags::Bit ITEM_ROT_DEATH            = Flags::P;
constexpr Flags::Bit ITEM_VIS_DEATH            = Flags::Q;
constexpr Flags::Bit ITEM_NOSAC                = Flags::R;
constexpr Flags::Bit ITEM_NONMETAL             = Flags::S;
constexpr Flags::Bit ITEM_NOLOCATE             = Flags::T;
constexpr Flags::Bit ITEM_MELT_DROP            = Flags::U;
//constexpr Flags::Bit ITEM_HAD_TIMER            = Flags::V;
constexpr Flags::Bit ITEM_SELL_EXTRACT         = Flags::W;
constexpr Flags::Bit ITEM_BURN_PROOF           = Flags::Y;
constexpr Flags::Bit ITEM_NOUNCURSE            = Flags::Z;
constexpr Flags::Bit ITEM_QUESTSELL            = Flags::a;
/** @} */

/** 
 * @defgroup WearFlags Wear Flags
 * @brief Constants representing wearable locations for items.
 * 
 * These flags define the locations on a character where items can be worn or equipped.
 * @{
 */
constexpr Flags::Bit ITEM_TAKE                 = Flags::A;
constexpr Flags::Bit ITEM_WEAR_FINGER          = Flags::B;
constexpr Flags::Bit ITEM_WEAR_NECK            = Flags::C;
constexpr Flags::Bit ITEM_WEAR_BODY            = Flags::D;
constexpr Flags::Bit ITEM_WEAR_HEAD            = Flags::E;
constexpr Flags::Bit ITEM_WEAR_LEGS            = Flags::F;
constexpr Flags::Bit ITEM_WEAR_FEET            = Flags::G;
constexpr Flags::Bit ITEM_WEAR_HANDS           = Flags::H;
constexpr Flags::Bit ITEM_WEAR_ARMS            = Flags::I;
constexpr Flags::Bit ITEM_WEAR_SHIELD          = Flags::J;
constexpr Flags::Bit ITEM_WEAR_ABOUT           = Flags::K;
constexpr Flags::Bit ITEM_WEAR_WAIST           = Flags::L;
constexpr Flags::Bit ITEM_WEAR_WRIST           = Flags::M;
constexpr Flags::Bit ITEM_WIELD                = Flags::N;
constexpr Flags::Bit ITEM_HOLD                 = Flags::O;
constexpr Flags::Bit ITEM_NO_SAC               = Flags::P;
constexpr Flags::Bit ITEM_WEAR_FLOAT           = Flags::Q;
constexpr Flags::Bit ITEM_WEAR_WEDDINGRING	  = Flags::R;
/** @} */

/** 
 * @defgroup WeaponClassConstants Weapon Class Constants
 * @brief Constants representing various weapon classes in the game.
 * 
 * These constants define the types of weapons that can be used by characters and mobs.
 * @{
 */
constexpr int WEAPON_EXOTIC            = 0;
constexpr int WEAPON_SWORD             = 1;
constexpr int WEAPON_DAGGER            = 2;
constexpr int WEAPON_SPEAR             = 3;
constexpr int WEAPON_MACE              = 4;
constexpr int WEAPON_AXE               = 5;
constexpr int WEAPON_FLAIL             = 6;
constexpr int WEAPON_WHIP              = 7;
constexpr int WEAPON_POLEARM           = 8;
constexpr int WEAPON_BOW               = 9;
/** @} */

/** 
 * @defgroup WeaponFlags Weapon Flags
 * @brief Constants representing special properties of weapons.
 * 
 * These flags define unique attributes and effects that weapons can have in the game.
 * @{
 */
constexpr Flags::Bit WEAPON_FLAMING            = Flags::A;
constexpr Flags::Bit WEAPON_FROST              = Flags::B;
constexpr Flags::Bit WEAPON_VAMPIRIC           = Flags::C;
constexpr Flags::Bit WEAPON_SHARP              = Flags::D;
constexpr Flags::Bit WEAPON_VORPAL             = Flags::E;
constexpr Flags::Bit WEAPON_TWO_HANDS          = Flags::F;
constexpr Flags::Bit WEAPON_SHOCKING           = Flags::G;
constexpr Flags::Bit WEAPON_POISON             = Flags::H;
constexpr Flags::Bit WEAPON_ACIDIC             = Flags::I;
/** @} */

/** 
 * @defgroup GateFlags Gate Flags
 * @brief Constants representing special properties of gates and portals.
 * 
 * These flags define the behaviors and attributes of gates and portals in the game.
 * @{
 */
constexpr Flags::Bit GATE_NORMAL_EXIT          = Flags::A;
constexpr Flags::Bit GATE_NOCURSE              = Flags::B;
constexpr Flags::Bit GATE_GOWITH               = Flags::C;
constexpr Flags::Bit GATE_BUGGY                = Flags::D;
constexpr Flags::Bit GATE_RANDOM               = Flags::E;
/** @} */

/** 
 * @defgroup FurnitureFlags Furniture Flags
 * @brief Constants representing interaction options for furniture items.
 * 
 * These flags define the ways characters can interact with furniture in the game.
 * @{
 */
constexpr Flags::Bit STAND_AT                  = Flags::A;
constexpr Flags::Bit STAND_ON                  = Flags::B;
constexpr Flags::Bit STAND_IN                  = Flags::C;
constexpr Flags::Bit SIT_AT                    = Flags::D;
constexpr Flags::Bit SIT_ON                    = Flags::E;
constexpr Flags::Bit SIT_IN                    = Flags::F;
constexpr Flags::Bit REST_AT                   = Flags::G;
constexpr Flags::Bit REST_ON                   = Flags::H;
constexpr Flags::Bit REST_IN                   = Flags::I;
constexpr Flags::Bit SLEEP_AT                  = Flags::J;
constexpr Flags::Bit SLEEP_ON                  = Flags::K;
constexpr Flags::Bit SLEEP_IN                  = Flags::L;
constexpr Flags::Bit PUT_AT                    = Flags::M;
constexpr Flags::Bit PUT_ON                    = Flags::N;
constexpr Flags::Bit PUT_IN                    = Flags::O;
constexpr Flags::Bit PUT_INSIDE                = Flags::P;
/** @} */

/** 
 * @defgroup ApplyTypes Apply Types
 * @brief Constants representing types of modifications for affects.
 * 
 * These constants define the attributes that can be modified by affects in the game, such as stats, hit points, and damage.
 * 2016 revamp of stats.  These values are now used as the things that affects
 * can modify, and make up the bonus vectors that determine bonuses to hp, mana,
 * hitroll, etc.  Changing these numbers can have drastic affects on both player
 * files and area files. -- Montrey
 * @{
 */
constexpr int APPLY_NONE                     = 0; /**< wasted in bonus vector, but oh well */
constexpr int APPLY_STR                      = 1;
constexpr int APPLY_DEX                      = 2;
constexpr int APPLY_INT                      = 3;
constexpr int APPLY_WIS                      = 4;
constexpr int APPLY_CON                      = 5;
constexpr int APPLY_SEX                      = 6;
//constexpr int APPLY_CLASS                    = 7;
//constexpr int APPLY_LEVEL                    = 8;
constexpr int APPLY_AGE                      = 9;
//constexpr int APPLY_HEIGHT                  = 10;
//constexpr int APPLY_WEIGHT                  = 11;
constexpr int APPLY_MANA                    = 12;
constexpr int APPLY_HIT                     = 13;
constexpr int APPLY_STAM                    = 14;
constexpr int APPLY_GOLD                    = 15;
constexpr int APPLY_EXP_PCT                 = 16;
constexpr int APPLY_AC                      = 17;
constexpr int APPLY_HITROLL                 = 18;
constexpr int APPLY_DAMROLL                 = 19;
constexpr int APPLY_SAVES                   = 20;
constexpr int APPLY_SAVING_PARA             = 20;
constexpr int APPLY_SAVING_ROD              = 21;
constexpr int APPLY_SAVING_PETRI            = 22;
constexpr int APPLY_SAVING_BREATH           = 23;
constexpr int APPLY_SAVING_SPELL            = 24;
//constexpr int APPLY_SPELL_AFFECT            = 25;
constexpr int APPLY_CHR                     = 26;
constexpr int APPLY_QUESTPOINTS			 = 27;
constexpr int APPLY_SKILLPOINTS			 = 28;
constexpr int APPLY_MANA_COST_PCT			 = 29;
constexpr int APPLY_STAM_COST_PCT			 = 30;
constexpr int APPLY_WPN_DAMAGE_PCT		 = 31;
constexpr int APPLY_SPELL_DAMAGE_PCT		 = 32;
constexpr int APPLY_VAMP_BONUS_PCT		 = 33;
constexpr int APPLY_HP_BLOCK_PCT			 = 34;
constexpr int APPLY_MANA_BLOCK_PCT		 = 35;
constexpr int APPLY_FORGE_UNIQUE			 = 36;
constexpr int APPLY_PRIESTESS_UNIQUE		 = 37;
constexpr int APPLY_BREW_UNIQUE			 = 38;
constexpr int APPLY_SCRIBE_UNIQUE			 = 39;
constexpr int APPLY_TANK_UNIQUE			 = 40;
constexpr int APPLY_NECRO_PIERCE_UNIQUE	 = 41;
constexpr int APPLY_LIDDA_AURA_UNIQUE		 = 42;
/** @} */

/** 
 * @defgroup ClassGearSets Class Gear Sets
 * @brief Constants representing class-specific gear sets.
 * 
 * These constants define unique gear sets tailored for specific classes in the game.
 * Values 43-63 reserved for class gear sets.
 * @{
 */
constexpr int SET_MAGE_INVOKER			 = 43;
constexpr int SET_WARRIOR_BESERKER		 = 44;
constexpr int SET_CLERIC_DIVINE			 = 45;
constexpr int SET_THIEF_CUTPURSE			 = 46;
constexpr int SET_PALADIN_GRACE			 = 47;
/** @} */

/** 
 * @defgroup ContainerFlags Container Flags
 * @brief Constants representing attributes and behaviors for containers.
 * 
 * These flags define the properties of containers, such as whether they can be closed, locked, or put on.
 * @{
 */
constexpr Flags::Bit CONT_CLOSEABLE  = Flags::A;
constexpr Flags::Bit CONT_PICKPROOF  = Flags::B;
constexpr Flags::Bit CONT_CLOSED     = Flags::C;
constexpr Flags::Bit CONT_LOCKED     = Flags::D;
constexpr Flags::Bit CONT_PUT_ON     = Flags::E;
/** @} */



/** 
 * @defgroup RoomVnums Room Virtual Numbers
 * @brief Constants representing well-known room virtual numbers.
 * 
 * These constants define specific room types and their virtual numbers used in the game.
 * @{
 */
constexpr int ROOM_VNUM_LIMBO = 2;
constexpr int ROOM_VNUM_CHAT = 1200;
constexpr int ROOM_VNUM_TEMPLE = 3001;
constexpr int ROOM_VNUM_PARK = 3105;
constexpr int ROOM_VNUM_ALTAR = 3054;
constexpr int ROOM_VNUM_MORGUE = 1251;
constexpr int ROOM_VNUM_SCHOOL = 3700;
constexpr int ROOM_VNUM_BALANCE = 4500;
constexpr int ROOM_VNUM_CIRCLE = 4400;
constexpr int ROOM_VNUM_DEMISE = 4201;
constexpr int ROOM_VNUM_HONOR = 4300;
constexpr int ROOM_VNUM_ARENACENTER = 700;
constexpr int ROOM_VNUM_STRONGBOX = 1269;
constexpr int ROOM_VNUM_ARENATICKET = 3368;
constexpr int ROOM_VNUM_TICKETBACKROOM = 3369;
/** @} */


/** 
 * @defgroup RoomFlags Room Flags
 * @brief Constants representing attributes and behaviors for rooms.
 * 
 * These flags define the properties and restrictions of rooms in the game.
 * @{
 */
/* Room flags. Used in #ROOMS. */
constexpr Flags::Bit ROOM_DARK          = Flags::A;
constexpr Flags::Bit ROOM_NOLIGHT       = Flags::B;
constexpr Flags::Bit ROOM_NO_MOB        = Flags::C;
constexpr Flags::Bit ROOM_INDOORS       = Flags::D;
constexpr Flags::Bit ROOM_LOCKER        = Flags::E;
constexpr Flags::Bit ROOM_FEMALE_ONLY   = Flags::F;
constexpr Flags::Bit ROOM_MALE_ONLY     = Flags::G;
constexpr Flags::Bit ROOM_NOSLEEP       = Flags::H;
constexpr Flags::Bit ROOM_NOVISION      = Flags::I;
constexpr Flags::Bit ROOM_PRIVATE       = Flags::J;
constexpr Flags::Bit ROOM_SAFE          = Flags::K;
constexpr Flags::Bit ROOM_SOLITARY      = Flags::L;
constexpr Flags::Bit ROOM_PET_SHOP      = Flags::M;
constexpr Flags::Bit ROOM_NO_RECALL     = Flags::N;
constexpr Flags::Bit ROOM_IMP_ONLY      = Flags::O;
constexpr Flags::Bit ROOM_GODS_ONLY     = Flags::P;
constexpr Flags::Bit ROOM_HEROES_ONLY   = Flags::Q;
constexpr Flags::Bit ROOM_NEWBIES_ONLY  = Flags::R;
constexpr Flags::Bit ROOM_LAW           = Flags::S;
constexpr Flags::Bit ROOM_NOWHERE       = Flags::T;
constexpr Flags::Bit ROOM_BANK          = Flags::U;
constexpr Flags::Bit ROOM_LEADER_ONLY   = Flags::V;
constexpr Flags::Bit ROOM_TELEPORT      = Flags::W;
constexpr Flags::Bit ROOM_UNDER_WATER   = Flags::X;
constexpr Flags::Bit ROOM_NOPORTAL      = Flags::Y;
constexpr Flags::Bit ROOM_REMORT_ONLY   = Flags::Z;
constexpr Flags::Bit ROOM_NOQUEST       = Flags::a;
constexpr Flags::Bit ROOM_SILENT        = Flags::b;
constexpr Flags::Bit ROOM_NORANDOMRESET = Flags::c;
/** @} */

/** 
 * @defgroup DirectionTypes Direction Types
 * @brief Constants representing cardinal directions used in the game.
 * 
 * These constants define the directions that characters and mobs can move within rooms.
 * @{
 */
/* Directions. Used in #ROOMS. */
constexpr int DIR_NORTH = 0;
constexpr int DIR_EAST = 1;
constexpr int DIR_SOUTH = 2;
constexpr int DIR_WEST = 3;
constexpr int DIR_UP = 4;
constexpr int DIR_DOWN = 5;
/** @} */



/** 
 * @defgroup ExitFlags Exit Flags
 * @brief Constants representing attributes and behaviors for room exits.
 * 
 * These flags define the properties of exits in rooms, such as whether they are doors, locked, or passable.
 * @{
 */
/* Exit flags. Used in #ROOMS. */
constexpr Flags::Bit EX_ISDOOR       = Flags::A;
constexpr Flags::Bit EX_CLOSED       = Flags::B;
constexpr Flags::Bit EX_LOCKED       = Flags::C;
constexpr Flags::Bit EX_PICKPROOF    = Flags::F;
constexpr Flags::Bit EX_NOPASS       = Flags::G;
constexpr Flags::Bit EX_EASY         = Flags::H;
constexpr Flags::Bit EX_HARD         = Flags::I;
constexpr Flags::Bit EX_INFURIATING  = Flags::J;
constexpr Flags::Bit EX_NOCLOSE      = Flags::K;
constexpr Flags::Bit EX_NOLOCK       = Flags::L;
/** @} */



/** 
 * @defgroup EquipmentWearLocations Equipment Wear Locations
 * @brief Constants representing locations where equipment can be worn.
 * 
 * These constants define the possible locations on a character where items can be equipped.
 * @{
 */
/* Equipment wear locations. Used in #RESETS. */
constexpr int WEAR_NONE        = -1;
constexpr int WEAR_LIGHT       = 0;
constexpr int WEAR_FINGER_L    = 1;
constexpr int WEAR_FINGER_R    = 2;
constexpr int WEAR_NECK_1      = 3;
constexpr int WEAR_NECK_2      = 4;
constexpr int WEAR_BODY        = 5;
constexpr int WEAR_HEAD        = 6;
constexpr int WEAR_LEGS        = 7;
constexpr int WEAR_FEET        = 8;
constexpr int WEAR_HANDS       = 9;
constexpr int WEAR_ARMS        = 10;
constexpr int WEAR_SHIELD      = 11;
constexpr int WEAR_ABOUT       = 12;
constexpr int WEAR_WAIST       = 13;
constexpr int WEAR_WRIST_L     = 14;
constexpr int WEAR_WRIST_R     = 15;
constexpr int WEAR_WIELD       = 16;
constexpr int WEAR_HOLD        = 17;
constexpr int WEAR_FLOAT       = 18;
constexpr int WEAR_SECONDARY   = 19;
constexpr int WEAR_WEDDINGRING = 20;
constexpr int MAX_WEAR         = 21;
/** @} */



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/** 
 * @defgroup Conditions Conditions
 * @brief Constants representing various character conditions.
 * 
 * These constants define the states of characters, such as hunger, thirst, and drunkenness.
 * @{
 */
constexpr int COND_DRUNK = 0;
constexpr int COND_FULL = 1;
constexpr int COND_THIRST = 2;
constexpr int COND_HUNGER = 3;
/** @} */



/** 
 * @defgroup Positions Positions
 * @brief Constants representing character positions.
 * 
 * These constants define the physical states or actions of characters, such as standing, sitting, or fighting.
 * @{
 */
constexpr int POS_DEAD = 0;
constexpr int POS_MORTAL = 1;
constexpr int POS_INCAP = 2;
constexpr int POS_STUNNED = 3;
constexpr int POS_SLEEPING = 4;
constexpr int POS_RESTING = 5;
constexpr int POS_SITTING = 6;
constexpr int POS_FIGHTING = 7; /**< used only on the interp table */
constexpr int POS_STANDING = 8;
constexpr int POS_FLYING = 9;

constexpr int POS_SNEAK = 19; /**< used only for movement act messages */
/** @} */


/** 
 * @defgroup PlayerFlags Player Flags
 * @brief Constants representing various act flags for players.
 * 
 * These flags define automatic behaviors, personal preferences, penalties, and other attributes for players in the game.
 * @{
 */
//constexpr Flags::Bit PLR_IS_NPC                = Flags::A; /**< Don't EVER set. */

/* RT auto flags */
constexpr Flags::Bit PLR_NOSUMMON  = Flags::A;
constexpr Flags::Bit PLR_LOOKINPIT  = Flags::B;
constexpr Flags::Bit PLR_AUTOASSIST  = Flags::C;
constexpr Flags::Bit PLR_AUTOEXIT  = Flags::D;
constexpr Flags::Bit PLR_AUTOLOOT  = Flags::E;
constexpr Flags::Bit PLR_AUTOSAC  = Flags::F;
constexpr Flags::Bit PLR_AUTOGOLD  = Flags::G;
constexpr Flags::Bit PLR_AUTOSPLIT  = Flags::H;
constexpr Flags::Bit PLR_DEFENSIVE  = Flags::I;
constexpr Flags::Bit PLR_WIMPY  = Flags::J;

/* RT personal flags */
constexpr Flags::Bit PLR_COLOR2  = Flags::L;
constexpr Flags::Bit PLR_VT100  = Flags::M;
constexpr Flags::Bit PLR_MAKEBAG  = Flags::O;
constexpr Flags::Bit PLR_CANLOOT  = Flags::P;
//constexpr Flags::Bit PLR_NOSUMMON              = Flags::Q;  moved to A to match NPC flag
constexpr Flags::Bit PLR_NOFOLLOW  = Flags::R;
constexpr Flags::Bit PLR_COLOR  = Flags::S;

/* penalty flags */
constexpr Flags::Bit PLR_PERMIT  = Flags::U;
constexpr Flags::Bit PLR_CLOSED  = Flags::V;
constexpr Flags::Bit PLR_LOG  = Flags::W;
constexpr Flags::Bit PLR_FREEZE  = Flags::Y;
constexpr Flags::Bit PLR_THIEF  = Flags::Z;
constexpr Flags::Bit PLR_KILLER  = Flags::a;
constexpr Flags::Bit PLR_SUPERWIZ  = Flags::b;
constexpr Flags::Bit PLR_TICKS  = Flags::c;
constexpr Flags::Bit PLR_NOPK  = Flags::d;

/* Lotus - Extra PLR flags for only Players in pcdata->plr */
constexpr Flags::Bit PLR_OOC  = Flags::A;
constexpr Flags::Bit PLR_CHATMODE  = Flags::B;
constexpr Flags::Bit PLR_PRIVATE  = Flags::C;
constexpr Flags::Bit PLR_STOPCRASH  = Flags::D;
constexpr Flags::Bit PLR_PK  = Flags::E;
constexpr Flags::Bit PLR_QUESTOR  = Flags::F;
constexpr Flags::Bit PLR_SHOWEMAIL  = Flags::G;
constexpr Flags::Bit PLR_LINK_DEAD  = Flags::H;
constexpr Flags::Bit PLR_PAINT  = Flags::I;
constexpr Flags::Bit PLR_SNOOP_PROOF  = Flags::J;
constexpr Flags::Bit PLR_NOSHOWLAST  = Flags::K;
constexpr Flags::Bit PLR_NONOTIFY  = Flags::L; /**< Comm flags full, pc only */
constexpr Flags::Bit PLR_AUTOPEEK  = Flags::M;
constexpr Flags::Bit PLR_HEEDNAME  = Flags::N;
constexpr Flags::Bit PLR_SHOWLOST  = Flags::O;
constexpr Flags::Bit PLR_SHOWRAFF  = Flags::Q;
constexpr Flags::Bit PLR_MARRIED  = Flags::R;
constexpr Flags::Bit PLR_SQUESTOR  = Flags::S; /**< Montrey */
constexpr Flags::Bit PLR_DUEL_IGNORE  = Flags::T; /**< Montrey */
//constexpr Flags::Bit PLR_NEWSCORE		  = Flags::U; /**< Montrey */
/** @} */

/** 
 * @defgroup ChannelFlags Channel Flags
 * @brief Constants representing various flags for communication channels.
 * 
 * These flags define the behavior and restrictions for different communication channels in the game.
 * @{
 */
constexpr int CHAN_GOSSIP = 0;
constexpr int CHAN_FLAME = 1;
constexpr int CHAN_QWEST = 2;
constexpr int CHAN_PRAY = 3;
constexpr int CHAN_CLAN = 4;
constexpr int CHAN_MUSIC = 5;
constexpr int CHAN_IC = 6;
constexpr int CHAN_GRATS = 7;
constexpr int CHAN_IMMTALK = 8;
constexpr int CHAN_QA = 9;
constexpr int CHAN_AUCTION = 10;
/** @} */

/** 
 * @defgroup CommunicationFlags Communication Flags
 * @brief Constants representing communication preferences and restrictions.
 * 
 * These flags define the communication settings for players and mobs, such as muting channels or enabling compact mode.
 * @{
 */
constexpr Flags::Bit COMM_QUIET  = Flags::A;
constexpr Flags::Bit COMM_DEAF  = Flags::B;
constexpr Flags::Bit COMM_NOWIZ  = Flags::C;
constexpr Flags::Bit COMM_NOAUCTION  = Flags::D;
constexpr Flags::Bit COMM_NOGOSSIP  = Flags::E;
constexpr Flags::Bit COMM_NOQUESTION  = Flags::F;
constexpr Flags::Bit COMM_NOMUSIC  = Flags::G;
constexpr Flags::Bit COMM_NOCLAN  = Flags::H;
constexpr Flags::Bit COMM_NOIC  = Flags::I;
constexpr Flags::Bit COMM_NOANNOUNCE  = Flags::K;
constexpr Flags::Bit COMM_COMPACT  = Flags::L;
constexpr Flags::Bit COMM_BRIEF  = Flags::M;
constexpr Flags::Bit COMM_PROMPT  = Flags::N;
constexpr Flags::Bit COMM_COMBINE  = Flags::O;
constexpr Flags::Bit COMM_NOFLAME  = Flags::P;
constexpr Flags::Bit COMM_SHOW_AFFECTS  = Flags::Q;
constexpr Flags::Bit COMM_NOGRATS  = Flags::R;
constexpr Flags::Bit COMM_NOEMOTE  = Flags::T;
constexpr Flags::Bit COMM_NOCHANNELS  = Flags::W; /**< this isn't even for players, is it needed? Montrey */
constexpr Flags::Bit COMM_NOSOCIAL  = Flags::X;
constexpr Flags::Bit COMM_NOQUERY  = Flags::Y;
constexpr Flags::Bit COMM_AFK  = Flags::Z;
constexpr Flags::Bit COMM_NOQWEST  = Flags::a;
constexpr Flags::Bit COMM_NOPAGE  = Flags::b;
constexpr Flags::Bit COMM_NOPRAY  = Flags::c;
constexpr Flags::Bit COMM_RPRAY  = Flags::d; /**< Defunct - replaced by REVOKE_PRAY */
constexpr Flags::Bit COMM_ATBPROMPT  = Flags::e;
/** @} */

/** 
 * @defgroup RevokedChannelFlags Revoked Channel Flags
 * @brief Constants representing restrictions on communication channels.
 * 
 * These flags define the channels that players or mobs are restricted from using.
 * @{
 */
/* NOchannel flags */
constexpr Flags::Bit REVOKE_NOCHANNELS	  = Flags::A;
constexpr Flags::Bit REVOKE_FLAMEONLY	  = Flags::B;
constexpr Flags::Bit REVOKE_GOSSIP		  = Flags::C;
constexpr Flags::Bit REVOKE_FLAME              = Flags::D;
constexpr Flags::Bit REVOKE_QWEST              = Flags::F;
constexpr Flags::Bit REVOKE_PRAY		  = Flags::G;
constexpr Flags::Bit REVOKE_AUCTION            = Flags::H;
constexpr Flags::Bit REVOKE_CLAN               = Flags::I;
constexpr Flags::Bit REVOKE_MUSIC              = Flags::J;
constexpr Flags::Bit REVOKE_QA                 = Flags::K;
constexpr Flags::Bit REVOKE_SOCIAL             = Flags::L;
constexpr Flags::Bit REVOKE_IC                 = Flags::M;
constexpr Flags::Bit REVOKE_GRATS              = Flags::N;
constexpr Flags::Bit REVOKE_PAGE               = Flags::Q;
constexpr Flags::Bit REVOKE_QTELL              = Flags::R;
constexpr Flags::Bit REVOKE_TELL		  = Flags::S;
constexpr Flags::Bit REVOKE_EMOTE		  = Flags::T;
constexpr Flags::Bit REVOKE_SAY		  = Flags::U;
constexpr Flags::Bit REVOKE_EXP		  = Flags::V;
constexpr Flags::Bit REVOKE_NOTE		  = Flags::W;
constexpr Flags::Bit REVOKE_NULL		  = Flags::e; /**< for use with new channels code -- Montrey */
/** @} */

/** 
 * @defgroup CommandGroupFlags Command Group Flags
 * @brief Constants representing command group flags for various roles and permissions.
 * 
 * These flags define the grouping of commands based on roles, permissions, and functionality in the game.
 * @{
 */
// A
constexpr Flags::Bit GROUP_GEN		  = Flags::B;
constexpr Flags::Bit GROUP_QUEST		  = Flags::C;
// D
constexpr Flags::Bit GROUP_BUILD		  = Flags::E;
constexpr Flags::Bit GROUP_CODE		  = Flags::F;
constexpr Flags::Bit GROUP_SECURE	  = Flags::G;
// H
constexpr Flags::Bit GROUP_PLAYER	  = Flags::I; /**< player only, for interp table (not set in cgroup) */
constexpr Flags::Bit GROUP_MOBILE	  = Flags::J; /**< mob only, for interp table (not set in cgroup) */
constexpr Flags::Bit GROUP_CLAN		  = Flags::K; /**< clan commands */
constexpr Flags::Bit GROUP_AVATAR	  = Flags::L; /**< level 80 stuff like scon */
constexpr Flags::Bit GROUP_HERO		  = Flags::M; /**< hero only stuff like herochat */
constexpr Flags::Bit GROUP_DEPUTY	  = Flags::N; /**< stuff for deputies and imm heads, makes them a deputy/head */
constexpr Flags::Bit GROUP_LEADER	  = Flags::O; /**< stuff for leaders and imm imps, makes them a leader/imp */
/** @} */

/** 
 * @defgroup WIZnetFlags WIZnet Flags
 * @brief Constants representing WIZnet flags for administrative notifications.
 * 
 * These flags define the types of notifications and messages that administrators can receive through WIZnet.
 * @{
 */
constexpr Flags::Bit WIZ_ON                    = Flags::A;
constexpr Flags::Bit WIZ_PURGE		  = Flags::B;
constexpr Flags::Bit WIZ_LOGINS                = Flags::C;
constexpr Flags::Bit WIZ_SITES                 = Flags::D;
constexpr Flags::Bit WIZ_LINKS                 = Flags::E;
constexpr Flags::Bit WIZ_DEATHS                = Flags::F;
constexpr Flags::Bit WIZ_RESETS                = Flags::G;
constexpr Flags::Bit WIZ_MOBDEATHS             = Flags::H;
constexpr Flags::Bit WIZ_FLAGS                 = Flags::I;
constexpr Flags::Bit WIZ_PENALTIES             = Flags::J;
constexpr Flags::Bit WIZ_SACCING               = Flags::K;
constexpr Flags::Bit WIZ_LEVELS                = Flags::L;
constexpr Flags::Bit WIZ_SECURE                = Flags::M;
constexpr Flags::Bit WIZ_SWITCHES              = Flags::N;
constexpr Flags::Bit WIZ_SNOOPS                = Flags::O;
constexpr Flags::Bit WIZ_RESTORE               = Flags::P;
constexpr Flags::Bit WIZ_LOAD                  = Flags::Q;
constexpr Flags::Bit WIZ_NEWBIE                = Flags::R;
constexpr Flags::Bit WIZ_PREFIX                = Flags::S;
constexpr Flags::Bit WIZ_SPAM                  = Flags::T;
constexpr Flags::Bit WIZ_MISC                  = Flags::U;
constexpr Flags::Bit WIZ_BUGS                  = Flags::V;
/* W,X,Y open */
constexpr Flags::Bit WIZ_CHEAT                 = Flags::Z;
constexpr Flags::Bit WIZ_MAIL                  = Flags::a;
constexpr Flags::Bit WIZ_AUCTION               = Flags::b;
constexpr Flags::Bit WIZ_QUEST                 = Flags::c;
constexpr Flags::Bit WIZ_MALLOC                = Flags::d;
/** @} */

/** 
 * @defgroup CensorFlags Censor Flags
 * @brief Constants representing censorship flags for communication channels and spam control.
 * 
 * These flags define the censorship settings for channels and spam filtering in the game.
 * @{
 */
/* New censor flags to replace COMM_SWEARON -- Montrey */
constexpr Flags::Bit CENSOR_CHAN		  = Flags::A;
constexpr Flags::Bit CENSOR_SPAM		  = Flags::C;
/** @} */






/** 
 * @defgroup VideoModeFlags Video Mode Flags
 * @brief Constants representing video mode flag bits.
 * 
 * These flags define various video mode settings for the game, such as flash effects and display options.
 * @{
 */
constexpr Flags::Bit VIDEO_FLASH_OFF           = Flags::A;
constexpr Flags::Bit VIDEO_FLASH_LINE          = Flags::B;
/* spare                        (Flags::C) */
constexpr Flags::Bit VIDEO_DARK_MOD            = Flags::D;
/* spare                        (Flags::E) */
constexpr Flags::Bit VIDEO_VT100             = Flags::F;
/* spare                        (Flags::G) */
constexpr Flags::Bit VIDEO_CODES_SHOW          = Flags::H;
/** @} */



/** 
 * @defgroup AreaTypes Area Types
 * @brief Constants representing types of areas in the game.
 * 
 * These constants define the classification of areas based on their purpose or accessibility.
 * @{
 */
constexpr const char AREA_TYPE_ALL    = 'A';
constexpr const char AREA_TYPE_CLAN   = 'C';
constexpr const char AREA_TYPE_IMMS   = 'I';
constexpr const char AREA_TYPE_HERO   = 'H';
constexpr const char AREA_TYPE_ARENA  = 'R';
constexpr const char AREA_TYPE_XXX    = 'X';
constexpr const char AREA_TYPE_NORM   = ' ';
/** @} */

/** 
 * @defgroup TargetTypes Target Types
 * @brief Constants representing types of targets for spells and abilities.
 * 
 * These constants define the valid targets for spells, abilities, and other actions in the game.
 * @{
 */
constexpr int TAR_IGNORE                   = 0;
constexpr int TAR_CHAR_OFFENSIVE           = 1;
constexpr int TAR_CHAR_DEFENSIVE           = 2;
constexpr int TAR_CHAR_SELF                = 3;
constexpr int TAR_OBJ_INV                  = 4;
constexpr int TAR_OBJ_CHAR_DEF             = 5;
constexpr int TAR_OBJ_CHAR_OFF             = 6;
constexpr int TAR_OBJ_HERE                 = 7; /**< new, in inventory or room */

constexpr int TARGET_CHAR                  = 0;
constexpr int TARGET_OBJ                   = 1;
constexpr int TARGET_ROOM                  = 2;
constexpr int TARGET_NONE                  = 3;
/** @} */

/** 
 * @defgroup RankTypes Rank Types
 * @brief Constants representing general ranks for immortals and players.
 * 
 * These constants define the hierarchy of ranks used for comparisons and command types in the game.
 * @{
 */
/* general ranks, for imm comparisons */
constexpr int RANK_MOBILE		 = 0;
constexpr int RANK_MORTAL		 = 1;
constexpr int RANK_IMM		 = 2;
constexpr int RANK_HEAD		 = 3;
constexpr int RANK_IMP		 = 4;

/* for command types, these are also used in some other locations as abbreviations */
constexpr int IMP = RANK_IMP; /**< implementor */
constexpr int HED = RANK_HEAD; /**< head of department */
constexpr int IMM = RANK_IMM; /**< immortal */
constexpr int MTL = RANK_MORTAL; /**< mortal - not used here, but in flag tables */
/** @} */

/** 
 * @defgroup EventTypes Event Types
 * @brief Constants representing various event types in the game.
 * 
 * These constants define the types of events that can occur, such as wars, kills, and clan actions.
 * @{
 */
constexpr int EVENT_NULL		 = 0;
constexpr int EVENT_WAR_START		 = 1;
constexpr int EVENT_WAR_STOP_WIN	 = 2;
constexpr int EVENT_WAR_STOP_IMM	 = 3;
constexpr int EVENT_WAR_DECLARE	 = 4;
constexpr int EVENT_WAR_JOIN		 = 5;
constexpr int EVENT_KILL		 = 6;
constexpr int EVENT_CLAN_DEFEAT	 = 7;
constexpr int EVENT_CLAN_WIPEOUT	 = 8;
constexpr int EVENT_CLAN_INVADE	 = 9;
constexpr int EVENT_ADJUST_SCORE	 = 10;
constexpr int EVENT_ADJUST_POWER	 = 11;
constexpr int EVENT_ADJUST_CLANQP	 = 12;
constexpr int EVENT_CLAN_SURRENDER	 = 13;
/** @} */

/** 
 * @defgroup DataFilePaths Data File Paths
 * @brief Constants representing file paths used by the server.
 * 
 * These constants define the locations of various data files, including player files, area files, logs, and miscellaneous files.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 * so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 * @{
 */
constexpr const char* PLAYER_DIR      = "../player/"; /**< Player files */
constexpr const char* TEMP_FILE       = "../player/romtmp";
constexpr const char* BACKUP_DIR      = "../player/backup/"; /**< Player files */
constexpr const char* STORAGE_DIR     = "../player/storage/"; /**< Player files in storage */
constexpr const char* STORAGE_FILE    = "../player/storage/player_list.txt"; /**< list of names of stored players */
constexpr const char* OLDCHAR_DIR     = "../player/oldchar/"; /**< Player files wiped out by autocleanup */

constexpr const char* HELP_DIR = "../help/"; /**< Help file temporary storage */

constexpr const char* AREA_DIR = "../area/";
constexpr const char* AREA_LIST = "area.lst";  /**< List of areas to boot */
constexpr const char* CHVNUM_FILE = "../area/chvnum.txt"; /**< list of changed object vnums -- Montrey (in progress) */
constexpr const char* MOB_DIR = "../area/MOBProgs/"; /**< MOBProg files */
constexpr const char* WORLDMAP_DIR = "../area/worldmap/";
constexpr const char* WORLDMAP_IMAGE_FILE = "../area/worldmap/worldmap.png";


constexpr const char* BIN_DIR         = "../bin/";
constexpr const char* EXE_FILE        = "../bin/legacy";

constexpr const char* MISC_DIR        = "../misc/"; /**< Miscellaneous Stuff */
constexpr const char* DB_FILE         = "../misc/database.sl3";
constexpr const char* CONFIG_FILE     = "../misc/config.json"; /**< new config file */
constexpr const char* NULL_FILE       = "../misc/NULL_FILE"; /**< To reserve one stream */
constexpr const char* PID_FILE        = "../misc/legacy.pid"; /**< pid file for the autoreboot script */
constexpr const char* COPYOVER_FILE   = "../misc/copyover.data";
constexpr const char* COPYOVER_LOG    = "../misc/copyover.log"; /**< This is to tell who does copyovers */
constexpr const char* COPYOVER_ITEMS  = "../misc/copyover.item"; /**< saves items through copyovers */
constexpr const char* BUG_FILE        = "../misc/bugs.txt";      /**< For 'bug' and bug() */
constexpr const char* PUNISHMENT_FILE = "../misc/punishment.txt";/**< For punishments */
constexpr const char* TYPO_FILE       = "../misc/typos.txt";     /**< For 'typo' */
constexpr const char* RIDEA_FILE      = "../misc/ridea.txt"; /**< random ideas, perm storage */
constexpr const char* WBI_FILE        = "../misc/wbi.txt"; /**< For 'will be imped' */
constexpr const char* HBI_FILE        = "../misc/hbi.txt"; /**< For 'has been imped' */
constexpr const char* WBB_FILE        = "../misc/wbb.txt"; /**< For 'will be built' */
constexpr const char* HBB_FILE        = "../misc/hbb.txt"; /**< For 'has been built' */
constexpr const char* WORK_FILE       = "../misc/work.txt"; /**< For future projects */
constexpr const char* IMMAPP_FILE     = "../misc/immapp.txt"; /**< For players to be considered for immhood */
constexpr const char* DEPARTED_FILE   = "../misc/departed.txt"; /**< departed imms files */
constexpr const char* SHUTDOWN_FILE   = "../misc/shutdown.txt";  /**< For 'shutdown' */
constexpr const char* BAN_FILE        = "../misc/ban.txt";
constexpr const char* MUSIC_FILE      = "../misc/music.txt";
constexpr const char* EMAIL_FILE      = "../misc/email.txt";
constexpr const char* DISABLED_FILE   = "../misc/disabled.txt"; /**< disabled commands */
constexpr const char* BOOT_FILE       = "../misc/bootfile"; /**< flags incomplete boot */

constexpr const char* CLAN_DIR        = MISC_DIR;
constexpr const char* CLAN_FILE       = "clans.txt";
constexpr const char* ARENA_DIR       = MISC_DIR;
constexpr const char* ARENA_FILE      = "arena.txt";
constexpr const char* SOCIAL_FILE      = "../misc/social.txt";

constexpr const char* WAR_DIR        = "../war/";
constexpr const char* WAR_FILE       = "war.txt";
constexpr const char* EVENT_DIR        = "../war/events/";
constexpr const char* EVENT_TMP       = "eventtmp";

constexpr const char* LOG_DIR = "../log/";
constexpr const char* SLOG_FILE = "../log/slog.txt"; /**< Secure Logs */
/** @} */

/** 
 * @defgroup NoteFilePaths Note File Paths
 * @brief Constants representing file paths for various note categories.
 * @{
 */
constexpr const char* NOTE_DIR = "../notes/";
constexpr const char* NOTE_FILE = "../notes/notes.not"; /**< For 'notes' */
constexpr const char* IDEA_FILE = "../notes/ideas.not";
constexpr const char* ROLEPLAY_FILE = "../notes/roleplay.not";
constexpr const char* IMMQUEST_FILE = "../notes/immquest.not";
constexpr const char* CHANGES_FILE = "../notes/change.not";
constexpr const char* PERSONAL_FILE = "../notes/personal.not";
constexpr const char* TRADE_FILE = "../notes/trade.not";
/** @} */


/** 
 * @defgroup RemortAffectTypes Remort Affect Types
 * @brief Constants for remort system affects, including beneficial and detrimental effects.
 * @{
 */
constexpr int RAFF_TEST			 = 1;

/* The Good... */
constexpr int RAFF_LIGHTFEET			 = 2;
constexpr int RAFF_FASTCAST			 = 3;
constexpr int RAFF_MAGEREGEN			 = 4;
constexpr int RAFF_VAMPREGEN			 = 5;
constexpr int RAFF_MOREEXP			 = 6;
constexpr int RAFF_MOREDAMAGE		 = 7;
constexpr int RAFF_CHEAPSPELLS		 = 8;
constexpr int RAFF_NOHUNGER			 = 9;
constexpr int RAFF_NOTHIRST			 = 10;
constexpr int RAFF_FAVORGOD			 = 11;
/* constexpr int RAFF_FREEPORTALS		 = 14; */
/* constexpr int RAFF_SIXTHSENSE		 = 24; */
/* constexpr int RAFF_FARSIGHT		 = 25; */

/* The Bad... */
constexpr int RAFF_BUGGYREC			 = 100;
/*constexpr int RAFF_BUGGYCLANREC		 = 101; */
constexpr int RAFF_SLOWWALK			 = 102;
constexpr int RAFF_CLUMSY			 = 103;
constexpr int RAFF_WEAKGRIP			 = 104;
constexpr int RAFF_SLOWCAST			 = 105;
constexpr int RAFF_LESSEXP			 = 106;
constexpr int RAFF_LESSDAMAGE		 = 107;
constexpr int RAFF_COSTLYSPELLS		 = 108;
constexpr int RAFF_EXTRAHUNGRY		 = 109;
constexpr int RAFF_EXTRATHIRSTY		 = 110;
constexpr int RAFF_LAUGHTERGOD		 = 111;
/* constexpr int RAFF_BADSENSE		 = 23; */

/* And the... oh.  Vuln/Res */
constexpr int RAFF_VULN_SLASH		 = 900;
constexpr int RAFF_VULN_PIERCE		 = 901;
constexpr int RAFF_VULN_BLUNT		 = 902;
constexpr int RAFF_VULN_FIRE			 = 903;
constexpr int RAFF_VULN_COLD			 = 904;
constexpr int RAFF_VULN_ELECTRICITY		 = 905;
constexpr int RAFF_VULN_ACID			 = 906;
constexpr int RAFF_VULN_POISON		 = 907;
constexpr int RAFF_VULN_DISEASE		 = 908;
constexpr int RAFF_VULN_NEGATIVE		 = 919;
constexpr int RAFF_VULN_HOLY			 = 910;
constexpr int RAFF_VULN_ENERGY		 = 911;
constexpr int RAFF_VULN_MENTAL		 = 912;
constexpr int RAFF_VULN_DROWNING		 = 913;
constexpr int RAFF_VULN_LIGHT		 = 914;
constexpr int RAFF_VULN_SOUND		 = 915;

constexpr int RAFF_RES_SLASH			 = 950;
constexpr int RAFF_RES_PIERCE		 = 951;
constexpr int RAFF_RES_BLUNT			 = 952;
constexpr int RAFF_RES_FIRE			 = 953;
constexpr int RAFF_RES_COLD			 = 954;
constexpr int RAFF_RES_ELECTRICITY		 = 955;
constexpr int RAFF_RES_ACID			 = 956;
constexpr int RAFF_RES_POISON		 = 957;
constexpr int RAFF_RES_DISEASE		 = 958;
constexpr int RAFF_RES_NEGATIVE		 = 959;
constexpr int RAFF_RES_HOLY			 = 960;
constexpr int RAFF_RES_ENERGY		 = 961;
constexpr int RAFF_RES_MENTAL		 = 962;
constexpr int RAFF_RES_DROWNING		 = 963;
constexpr int RAFF_RES_LIGHT			 = 964;
constexpr int RAFF_RES_SOUND			 = 965;
/** @} */
