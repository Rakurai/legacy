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

#include "merc.h"
#include "interp.h"
#include "tables.h"
#include "vt100.h" /* VT100 Stuff */
#include "sql.h"
#include "affect.h"

extern void     goto_line    args((CHAR_DATA *ch, int row, int column));
extern void     set_window   args((CHAR_DATA *ch, int top, int bottom));
extern void     slog_file    args((CHAR_DATA *ch, char *file, char *str));

extern char *dv_where;
char dv_command[MAX_INPUT_LENGTH];
char logline[MAX_STRING_LENGTH] = " "; /* extern for debug */
char dv_guard[] = "55555";             /* check this to see if logline overwritten */

/* Disabled Commands Stuff */
bool    check_disabled(const struct cmd_type *command);
DISABLED_DATA *disabled_first;
#define END_MARKER      "END" /* for load_disabled() and save_disabled() */

/* Malloc Stuff */
extern int nAllocString;
extern int nAllocPerm;

/*
 * Command logging types.
 */
#define LOG_NORMAL      0
#define LOG_ALWAYS      1
#define LOG_NEVER       2

/*
 * Log-all switch.
 */
bool                            fLogAll         = FALSE;

/*
 * Command table.
 */
const   struct  cmd_type        cmd_table       [] = {
	/*
	 * Abbreviations
	 */
	{ "'",                          do_say,                 POS_RESTING,    LOG_NORMAL,     0,      0                       },
	{ "-",                          do_emote,               POS_RESTING,    LOG_NORMAL,     0,      0                       },
	{ ".",                          do_gossip,              POS_SLEEPING,   LOG_NORMAL,     0,      0                       },
	{ "/",                          do_recall,              POS_FIGHTING,   LOG_NORMAL,     0,      0                       },
	{ ":",                          do_immtalk,             POS_DEAD,               LOG_NORMAL,     0,      GWG                     },
	{ ";",                          do_gtell,               POS_DEAD,               LOG_NORMAL,     0,      0                       },
	{ "@",                          do_qwest,               POS_DEAD,               LOG_NORMAL,     0,      0                       },
	/*
	 * 0 = No Print, 1 = Communication, 2 = Combat, 3 = Informational,
	 * 4 = Object Manipulation, 5 = Wiz, 6 = Custom Settings, 7 = Magical
	 * 8 = Character Manipulation, 9 = Shop Commands
	 */

	/*
	 * Direction commands at the top for easy access
	 */
	{ "east",                       do_east,                POS_STANDING,   LOG_NEVER,      8,      0                       },
	{ "west",                       do_west,                POS_STANDING,   LOG_NEVER,      8,      0                       },
	{ "down",                       do_down,                POS_STANDING,   LOG_NEVER,      8,      0                       },
	{ "up",                         do_up,                  POS_STANDING,   LOG_NEVER,      8,      0                       },
	{ "north",                      do_north,               POS_STANDING,   LOG_NEVER,      8,      0                       },
	{ "south",                      do_south,               POS_STANDING,   LOG_NEVER,      8,      0                       },

	{ "accept",                     do_accept,              POS_RESTING,    LOG_NORMAL,     8,      0                       },
	{ "addapply",           do_addapply,    POS_DEAD,               LOG_ALWAYS,     5,      GD | GWQ        },
	{ "addexit",            do_addexit,             POS_DEAD,               LOG_ALWAYS,     5,      GWC                     },
	{ "adjust",                     do_adjust,              POS_DEAD,               LOG_ALWAYS,     5,      GD | GWC        },
	{ "advance",            do_advance,             POS_DEAD,               LOG_ALWAYS,     5,      GL | GWC        },
	{ "affects",            do_affects,             POS_SLEEPING,   LOG_NORMAL,     3,      0                       },
	{ "afk",                        do_afk,                 POS_SLEEPING,   LOG_NORMAL,     1,      0                       },
	{ "alia",                       do_alia,                POS_DEAD,               LOG_NORMAL,     0,      0                       },
	{ "alias",                      do_alias,               POS_DEAD,               LOG_NORMAL,     6,      0                       },
//	{ "alight",                     do_alight,              POS_SLEEPING,   LOG_NORMAL,     8,      0                       },
	{ "align",                      do_align,               POS_SLEEPING,   LOG_NORMAL,     8,      0                       },
	{ "allow",                      do_allow,               POS_DEAD,               LOG_ALWAYS,     5,      GWS                     },
	{ "allsave",            do_allsave,             POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "alternate",          do_alternate,   POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "announce",           do_announce,    POS_SLEEPING,   LOG_NORMAL,     1,      0                       },
	{ "answer",                     do_question,    POS_SLEEPING,   LOG_NORMAL,     1,      0                       },
	{ "areas",                      do_areas,               POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "argue",                      do_flame,               POS_DEAD,               LOG_NORMAL,     0,      0                       },
	{ "at",                         do_at,                  POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "auction",            do_auction,             POS_SLEEPING,   LOG_NORMAL,     1,      0                       },
	{ "aura",                       do_aura,                POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "autolist",           do_autolist,    POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "autoassist",         do_autoassist,  POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "autoexit",           do_autoexit,    POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "autogold",           do_autogold,    POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "autoloot",           do_autoloot,    POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "autopeek",           do_autopeek,    POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "autoreboot",         do_autoboot,    POS_DEAD,               LOG_ALWAYS,     5,      GL | GWC        },
	{ "autorecall",         do_autorecall,  POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "autosac",            do_autosac,             POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "autosplit",          do_autosplit,   POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "autotick",           do_autotick,    POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "autograph",          do_autograph,   POS_DEAD,               LOG_NEVER,      0,      0                       },
	{ "buy",                        do_buy,                 POS_RESTING,    LOG_NORMAL,     9,      0                       },
	{ "backstab",           do_backstab,    POS_FIGHTING,   LOG_NORMAL,     2,      0                       },
	{ "backup",                     do_backup,              POS_DEAD,               LOG_NORMAL,     8,      0                       },
	{ "balance",            do_balance,             POS_STANDING,   LOG_NORMAL,     3,      0                       },
	{ "ban",                        do_ban,                 POS_DEAD,               LOG_ALWAYS,     5,      GWS                     },
	{ "bash",                       do_bash,                POS_FIGHTING,   LOG_NORMAL,     2,      0                       },
	{ "battle",                     do_battle,              POS_FIGHTING,   LOG_NORMAL,     2,      0                       },
	{ "berserk",            do_berserk,             POS_FIGHTING,   LOG_NORMAL,     2,      0                       },
	{ "brandish",           do_brandish,    POS_RESTING,    LOG_NORMAL,     7,      0                       },
	{ "breakup",            do_breakup,             POS_STANDING,   LOG_NORMAL,     8,      0                       },
	{ "brew",                       do_brew,                POS_STANDING,   LOG_NORMAL,     7,      0                       },
//	{ "board",                      do_board,               POS_STANDING,   LOG_NORMAL,     8,      0                       },
	{ "ranged",                     do_bow,                 POS_STANDING,   LOG_NORMAL,     2,      0                       },
	{ "brief",                      do_brief,               POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "bug",                        do_bug,                 POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "cast",                       do_cast,                POS_FIGHTING,   LOG_NORMAL,     7,      0                       },
	{ "cedit",                      do_cedit,               POS_DEAD,               LOG_ALWAYS,     5,      GD | GWC        },
	{ "censor",                     do_censor,              POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "channels",           do_channels,    POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "changes",            do_changes,             POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "chatmode",           do_chatmode,    POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "check",                      do_check,               POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "chown",                      do_chown,               POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "circle",                     do_circle,              POS_FIGHTING,   LOG_NORMAL,     2,      0                       },
	{ "clantalk",           do_clantalk,    POS_SLEEPING,   LOG_NORMAL,     1,      GC                      },
	{ "clandeposit",        do_clandeposit, POS_RESTING,    LOG_NORMAL,     8,      GC                      },
	{ "clanwithdraw",       do_clanwithdraw, POS_RESTING,    LOG_NORMAL,     8,      GC                      },
	{ "clanlist",           do_clanlist,    POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "clanpower",          do_clanpower,   POS_SLEEPING,   LOG_NORMAL,     3,      0                       },
	{ "clanqp",                     do_clanqp,              POS_SLEEPING,   LOG_NORMAL,     8,      GC                      },
	{ "claninfo",           do_claninfo,    POS_SLEEPING,   LOG_NORMAL,     3,      GC                      },
	{ "clanrecall",         do_clan_recall, POS_FIGHTING,   LOG_NORMAL,     8,      GC                      },
	{ "clear",                      do_clear,               POS_DEAD,               LOG_NORMAL,     1,      0                       },
	{ "clone",                      do_clone,               POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "close",                      do_close,               POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "color",                      do_color,               POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "combine",            do_combine,             POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "commands",           do_commands,    POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "compare",            do_compare,             POS_RESTING,    LOG_NORMAL,     3,      0                       },
	{ "compact",            do_compact,             POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "copyove",            do_copyove,             POS_DEAD,               LOG_ALWAYS,     5,      0                       },
	{ "copyover",           do_copyover,    POS_DEAD,               LOG_ALWAYS,     5,      GL | GWC        },
	{ "consider",           do_consider,    POS_RESTING,    LOG_NORMAL,     3,      0                       },
	{ "config",                     do_config,              POS_SLEEPING,   LOG_NORMAL,     6,      0                       },
	{ "convert",            do_convert,             POS_SLEEPING,   LOG_NORMAL,     8,      0                       },
	{ "count",                      do_count,               POS_SLEEPING,   LOG_NORMAL,     3,      0                       },
	{ "credits",            do_credits,             POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "critical",           do_critical_blow, POS_FIGHTING,  LOG_NORMAL,     2,      0                       },
	{ "create",                     do_create,              POS_DEAD,               LOG_NORMAL,     5,      GD | GWQ        },
	{ "crush",                      do_crush,               POS_FIGHTING,   LOG_NORMAL,     2,      0                       },
	{ "ctest",                      do_ctest,               POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "drink",                      do_drink,               POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "debug",                      do_debug,               POS_DEAD,               LOG_ALWAYS,     5,      GWC                     },
	{ "deaf",                       do_deaf,                POS_DEAD,               LOG_NORMAL,     1,      0                       },
	{ "defensive",          do_defensive,   POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "delet",                      do_delet,               POS_DEAD,               LOG_ALWAYS,     0,      0                       },
	{ "delete",                     do_delete,              POS_STANDING,   LOG_ALWAYS,     8,      0                       },
	{ "deny",                       do_deny,                POS_DEAD,               LOG_ALWAYS,     5,      GWS                     },
	{ "deposit",            do_deposit,             POS_STANDING,   LOG_NORMAL,     9,      0                       },
	{ "departed",           do_departed,    POS_STANDING,   LOG_NORMAL,     3,      0                       },
	{ "departedlist",       do_departedlist, POS_DEAD,               LOG_ALWAYS,     5,      GD | GWG        },
	{ "deputize",           do_deputize,    POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "description",        do_description, POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "despell",            do_despell,             POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "dirt",                       do_dirt,                POS_FIGHTING,   LOG_NORMAL,     2,      0                       },
	{ "disable",            do_disable,             POS_DEAD,               LOG_ALWAYS,     5,      GL | GWG        },
	{ "disarm",                     do_disarm,              POS_FIGHTING,   LOG_NORMAL,     2,      0                       },
	{ "disconnect",         do_disconnect,  POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "divorce",            do_divorce,             POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "donate",                     do_donate,              POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "doas",                       do_doas,                POS_DEAD,               LOG_ALWAYS,     5,      GL | GWG        },
	{ "drag",                       do_drag,                POS_STANDING,   LOG_NORMAL,     8,      0                       },
	{ "drop",                       do_drop,                POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "duel",                       do_duel,                POS_SLEEPING,   LOG_NORMAL,     6,      0                       },
	{ "dump",                       do_dump,                POS_DEAD,               LOG_ALWAYS,     5,      GWC                     },
	{ "eat",                        do_eat,                 POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "echo",                       do_recho,               POS_DEAD,               LOG_NORMAL,     5,      GWQ                     },
	{ "edit",                       do_edit,                POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "emote",                      do_emote,               POS_RESTING,    LOG_NORMAL,     1,      0                       },
	{ "email",                      do_email,               POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "enter",                      do_enter,               POS_FIGHTING,   LOG_NORMAL,     8,      0                       },
	{ "engrave",            do_engrave,             POS_RESTING,    LOG_NORMAL,     3,      0                       },
	{ "envenom",            do_envenom,             POS_RESTING,    LOG_NORMAL,     2,      0                       },
	{ "equipment",          do_equipment,   POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "evolve",                     do_evolve,              POS_SLEEPING,   LOG_ALWAYS, 8,  0                       },
	{ "exits",                      do_exits,               POS_RESTING,    LOG_NORMAL,     3,      0                       },
	{ "examine",            do_examine,             POS_RESTING,    LOG_NORMAL,     3,      0                       },
	{ "exitlist",           do_exlist,              POS_DEAD,               LOG_NORMAL,     5,      GWB                     },
	{ "extraclass",         do_eremort,             POS_RESTING,    LOG_NORMAL,     0,      0                       },
	{ "familiar",           do_familiar,    POS_RESTING,    LOG_NORMAL,     7,      0                       },
	{ "file",                       do_file,                POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "fill",                       do_fill,                POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "finger",                     do_finger,              POS_SLEEPING,   LOG_NORMAL,     3,      0                       },
	{ "fingerinfo",         do_fingerinfo , POS_SLEEPING,   LOG_NORMAL,     3,      0                       },
	{ "firebuilding",       do_firebuilding, POS_STANDING,   LOG_NORMAL,     4,      0                       },
	{ "flag",                       do_flag,                POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "flaglist",           do_flaglist,    POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "flagsearch",         do_flagsearch,  POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "flame",                      do_flame,               POS_DEAD,               LOG_NORMAL,     1,      0                       },
	{ "flee",                       do_flee,                POS_FIGHTING,   LOG_NORMAL,     8,      0                       },
	{ "fly",                        do_fly,                 POS_SLEEPING,   LOG_NORMAL,     8,      0                       },
	{ "fod",                        do_fod,                 POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "follow",                     do_follow,              POS_RESTING,    LOG_NORMAL,     8,      0                       },
	{ "followerlist",       do_followerlist, POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "for",                        do_for,                 POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "force",                      do_force,               POS_DEAD,               LOG_ALWAYS,     5,      GWS                     },
	{ "forge",                      do_forge,               POS_STANDING,   LOG_NORMAL,     4,      0                       },
	{ "freeze",                     do_freeze,              POS_DEAD,               LOG_ALWAYS,     5,      GWS                     },
	{ "fry",                        do_fry,                 POS_DEAD,               LOG_ALWAYS,     5,      GD | GWS        },
	{ "fyi",                        do_fyi,                 POS_DEAD,               LOG_ALWAYS,     5,      GD | GWG        },
	{ "get",                        do_get,                 POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "gain",                       do_gain,                POS_STANDING,   LOG_NORMAL,     8,      0                       },
	{ "gamein",                     do_gamein,              POS_SLEEPING,   LOG_NORMAL,     3,      0                       },
	{ "gameout",            do_gameout,             POS_SLEEPING,   LOG_NORMAL,     3,      0                       },
	{ "gecho",                      do_echo,                POS_DEAD,               LOG_NORMAL,     5,      GWQ                     },
	{ "give",                       do_give,                POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "goto",                       do_goto,                POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "gossip",                     do_gossip,              POS_SLEEPING,   LOG_NORMAL,     1,      0                       },
	{ "group",                      do_group,               POS_SLEEPING,   LOG_NORMAL,     8,      0                       },
	{ "grant",                      do_grant,               POS_DEAD,               LOG_NORMAL,     5,      GL | GWG        },
	{ "grantlist",          do_grantlist,   POS_DEAD,               LOG_NORMAL,     5,      0                       },
	{ "grats",                      do_grats,               POS_SLEEPING,   LOG_NORMAL,     1,      0                       },
	{ "grouplist",          do_grouplist,   POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "groups",                     do_groups,              POS_SLEEPING,   LOG_NORMAL,     3,      0                       },
	{ "gtell",                      do_gtell,               POS_DEAD,               LOG_NORMAL,     1,      0                       },
	{ "guild",                      do_guild,               POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "heal",                       do_heal,                POS_RESTING,    LOG_NORMAL,     9,      0                       },
	{ "hammerstrike",       do_hammerstrike, POS_FIGHTING,   LOG_NORMAL,     2,      0                       },
	{ "heed",                       do_heed,                POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "hedit",                      do_hedit,               POS_DEAD,               LOG_ALWAYS,     5,      GWB                     },
	{ "help",                       do_help,                POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "hide",                       do_hide,                POS_RESTING,    LOG_NORMAL,     8,      0                       },
	{ "hit",                        do_kill,                POS_FIGHTING,   LOG_NORMAL,     0,      0                       },
	{ "hold",                       do_wear,                POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "hone",                       do_hone,                POS_STANDING,   LOG_NORMAL,     4,      0                       },
	{ "hunt",                       do_hunt,                POS_STANDING,   LOG_NORMAL,     2,      0                       },
	{ "hbi",                        do_hbi,                 POS_DEAD,               LOG_ALWAYS,     5,      GWC                     },
	{ "hbb",                        do_hbb,                 POS_DEAD,               LOG_ALWAYS,     5,      GWB                     },
	{ "inventory",          do_inventory,   POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "ic",                         do_ic,                  POS_DEAD,               LOG_NORMAL,     1,      0                       },
	{ "iclan",                      do_iclantalk,   POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "identify",           do_identify,    POS_STANDING,   LOG_NORMAL,     9,      0                       },
	{ "idea",                       do_idea,                POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "ignore",                     do_ignore,              POS_DEAD,               LOG_NORMAL,     1,      0                       },
	{ "immtalk",            do_immtalk,             POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "immquest",           do_immquest,    POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "immname",            do_immname,             POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "immapp",                     do_immapp,              POS_DEAD,               LOG_NORMAL,     5,      GD | GWG        },
	{ "imotd",                      do_imotd,               POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "info",                       do_groups,              POS_SLEEPING,   LOG_NORMAL,     3,      0                       },
	{ "inset",                       do_inset,                POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "invite",                     do_invite,              POS_DEAD,               LOG_ALWAYS,     8,      0                       },
	{ "join",                       do_join,                POS_RESTING,    LOG_ALWAYS,     8,      GC                      },
	{ "junk",                       do_junk,                POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "kill",                       do_kill,                POS_FIGHTING,   LOG_NORMAL,     2,      0                       },
	{ "kick",                       do_kick,                POS_FIGHTING,   LOG_NORMAL,     2,      0                       },
	{ "look",                       do_look,                POS_RESTING,    LOG_NORMAL,     3,      0                       },
	{ "land",                       do_land,                POS_FIGHTING,               LOG_NORMAL,     8,      0                       },
	{ "lay",                        do_lay_on_hands, POS_RESTING,    LOG_NORMAL,     7,      0                       },
	{ "leader",                     do_leader,              POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "levels",                     do_levels,              POS_SLEEPING,   LOG_NORMAL,     3,      0                       },
	{ "linkload",           do_linkload,    POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "list",                       do_list,                POS_RESTING,    LOG_NORMAL,     9,      0                       },
	{ "load",                       do_load,                POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "loadhelps",          do_loadhelps,   POS_DEAD,               LOG_ALWAYS,     5,      GD | GWG        },
	{ "lock",                       do_lock,                POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "locker",                     do_locker,              POS_DEAD,               LOG_ALWAYS,     5,      GWS                     },
	{ "log",                        do_log,                 POS_DEAD,               LOG_ALWAYS,     5,      GD | GWS        },
	{ "lore",                       do_lore,                POS_RESTING,    LOG_NORMAL,     3,      0                       },
	{ "lower",                      do_lower,               POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "lurk",                       do_lurk,                POS_DEAD,               LOG_NORMAL,     8,      GWG                     },
	{ "mark",                       do_mark,                POS_STANDING,   LOG_NORMAL,     8,      0                       },
	{ "marry",                      do_marry,               POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "master",                     do_master,              POS_DEAD,               LOG_NORMAL,     5,      GWQ                     },
	{ "memory",                     do_memory,              POS_DEAD,               LOG_NORMAL,     5,      GWC                     },
	{ "mlevel",                     do_mlevel,              POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "morph",                      do_morph,               POS_DEAD,               LOG_ALWAYS,     5,      GWQ                     },
	{ "motd",                       do_motd,                POS_DEAD,               LOG_NORMAL,     5,      GD | GWG        },
	{ "mpasound",           do_mpasound,    POS_DEAD,               LOG_NORMAL,     0,      GM                      },
	{ "mpat",                       do_mpat,                POS_DEAD,               LOG_NORMAL,     0,      GM                      },
	{ "mpcast",                     do_mpcast,              POS_DEAD,               LOG_NORMAL,     0,      GM                      },
	{ "mpclearmoney",       do_mpclearmoney, POS_DEAD,               LOG_NORMAL,     0,      GM                      },
	{ "mpecho",                     do_mpecho,              POS_DEAD,               LOG_NORMAL,     0,      GM                      },
	{ "mpechoaround",       do_mpechoaround, POS_DEAD,               LOG_NORMAL,     0,      GM                      },
	{ "mpechoat",           do_mpechoat,    POS_DEAD,               LOG_NORMAL,     0,      GM                      },
	{ "mpforce",            do_mpforce,             POS_DEAD,               LOG_NORMAL,     0,      GM                      },
	{ "mpgoto",                     do_mpgoto,              POS_DEAD,               LOG_NORMAL,     0,      GM                      },
	{ "mpjunk",                     do_mpjunk,              POS_DEAD,               LOG_NORMAL,     0,      GM                      },
	{ "mpkill",                     do_mpkill,              POS_DEAD,               LOG_NORMAL,     0,      GM                      },
	{ "mpmload",            do_mpmload,             POS_DEAD,               LOG_NORMAL,     0,      GM                      },
	{ "mpoload",            do_mpoload,             POS_DEAD,               LOG_NORMAL,     0,      GM                      },
	{ "mppurge",            do_mppurge,             POS_DEAD,               LOG_NORMAL,     0,      GM                      },
	{ "mpstat",                     do_mpstat,              POS_DEAD,               LOG_NORMAL,     5,      GWB                     },
	{ "mptransfer",         do_mptransfer,  POS_DEAD,               LOG_NORMAL,     0,      GM                      },
	{ "music",                      do_music,               POS_SLEEPING,   LOG_NORMAL,     1,      0                       },
	{ "mwhere",                     do_mwhere,              POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
//	{ "mypipe",                     do_mypipe,              POS_DEAD,               LOG_ALWAYS,     5,      GD | GWC        },
	{ "newpassword",        do_newpasswd,   POS_DEAD,               LOG_ALWAYS,     5,      GWS                     },
	{ "newbiekit",          do_newbiekit,   POS_RESTING,    LOG_ALWAYS,     5,      0                       },
	{ "newbiemaker",        do_canmakebag,  POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "newlock",            do_newlock,             POS_DEAD,               LOG_ALWAYS,     5,      GWS                     },
	{ "next",                       do_next,                POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "nofollow",           do_nofollow,    POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "noloot",                     do_noloot,              POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "noreply",            do_noreply,             POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "nosummon",           do_nosummon,    POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "note",                       do_note,                POS_SLEEPING,   LOG_NORMAL,     3,      0                       },
	{ "notify",                     do_notify,              POS_SLEEPING,   LOG_NORMAL,     3,      0                       },
	{ "oclone",                     do_oclone,              POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "order",                      do_order,               POS_RESTING,    LOG_NORMAL,     8,      0                       },
	{ "olevel",                     do_olevel,              POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "ooc",                        do_ooc,                 POS_SLEEPING,   LOG_NORMAL,     6,      0                       },
	{ "open",                       do_open,                POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "outfit",                     do_outfit,              POS_RESTING,    LOG_NORMAL,     8,      0                       },
	{ "owhere",                     do_owhere,              POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "owner",                      do_owner,               POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "page",                       do_page,                POS_DEAD,               LOG_NORMAL,     1,      0                       },
	{ "paintball",          do_paintball,   POS_SLEEPING,   LOG_NORMAL,     6,      0                       },
	{ "pardon",                     do_pardon,              POS_DEAD,               LOG_ALWAYS,     5,      GWS                     },
	{ "password",           do_password,    POS_DEAD,               LOG_NEVER,      6,      0                       },
	{ "peace",                      do_peace,               POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "pecho",                      do_pecho,               POS_DEAD,               LOG_NORMAL,     5,      GWQ                     },
	{ "peek",                       do_peek,                POS_RESTING,    LOG_NORMAL,     3,      0                       },
	{ "personal",           do_personal,    POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "permit",                     do_permit,              POS_DEAD,               LOG_ALWAYS,     5,      GWS                     },
	{ "pet",                        do_pet,                 POS_RESTING,    LOG_NORMAL,     8,      0                       },
	{ "pick",                       do_pick,                POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "pit",                        do_pit,                 POS_RESTING,    LOG_NORMAL,     3,      0                       },
	//{ "pipe",                     do_pipe,                POS_DEAD,               LOG_ALWAYS,     5,      GD | GWC        },
	{ "pk",                         do_pk,                  POS_SLEEPING,   LOG_NORMAL,     6,      0                       },
	{ "play",                       do_play,                POS_RESTING,    LOG_NORMAL,     1,      0                       },
	{ "pmote",                      do_pmote,               POS_RESTING,    LOG_NORMAL,     1,      0                       },
	{ "pocket",                     do_pocket,              POS_DEAD,               LOG_NORMAL,     5,      GWB                     },
	{ "poofin",                     do_bamfin,              POS_DEAD,               LOG_NORMAL,     5,      0                       },
	{ "poofout",            do_bamfout,             POS_DEAD,               LOG_NORMAL,     5,      0                       },
	{ "pose",                       do_pose,                POS_RESTING,    LOG_NORMAL,     1,      0                       },
	{ "pour",                       do_pour,                POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "practice",           do_practice,    POS_SLEEPING,   LOG_NORMAL,     8,      0                       },
	{ "pray",                       do_pray,                POS_DEAD,               LOG_NORMAL,     1,      0                       },
	{ "prefi",                      do_prefi,               POS_DEAD,               LOG_NORMAL,     0,      0                       },
	{ "prefix",                     do_prefix,              POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "printhelps",         do_printhelps,  POS_DEAD,               LOG_ALWAYS,     5,      GD | GWG        },
	{ "private",            do_private,             POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "prompt",                     do_prompt,              POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "propose",            do_propose,             POS_STANDING,   LOG_NORMAL,     8,      0                       },
	{ "protect",            do_protect,             POS_DEAD,               LOG_ALWAYS,     5,      GWS                     },
	{ "pstat",                      do_pstat,               POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "put",                        do_put,                 POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "punish",                     do_punish,              POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "purge",                      do_purge,               POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "push",                       do_push,                POS_STANDING,   LOG_NORMAL,     8,      0                       },
	{ "qpconv",                     do_qpconv,              POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "qtell",                      do_qtell,               POS_DEAD,               LOG_NORMAL,     1,      0                       },
	{ "quaff",                      do_quaff,               POS_RESTING,    LOG_NORMAL,     7,      0                       },
	{ "query",                      do_query,               POS_DEAD,               LOG_NORMAL,     1,      0                       },
	{ "quest",                      do_quest,               POS_SLEEPING,   LOG_NORMAL,     9,      0                       },
	{ "question",           do_question,    POS_SLEEPING,   LOG_NORMAL,     1,      0                       },
	{ "qui",                        do_qui,                 POS_DEAD,               LOG_NORMAL,     0,      0                       },
	{ "quiet",                      do_quiet,               POS_SLEEPING,   LOG_NORMAL,     1,      0                       },
	{ "quit",                       do_quit,                POS_DEAD,               LOG_NORMAL,     8,      0                       },
	{ "qwest",                      do_qwest,               POS_DEAD,               LOG_NORMAL,     1,      0                       },
	{ "rest",                       do_rest,                POS_SLEEPING,   LOG_NORMAL,     8,      0                       },
	{ "rage",                       do_rage,                POS_FIGHTING,   LOG_NORMAL,     2,      0                       },
	{ "rank",                       do_rank,                POS_DEAD,               LOG_NORMAL,     8,      GC                      },
	{ "read",                       do_look,                POS_RESTING,    LOG_NORMAL,     3,      0                       },
	{ "reboo",                      do_reboo,               POS_DEAD,               LOG_NORMAL,     0,      0                       },
	{ "reboot",                     do_reboot,              POS_DEAD,               LOG_ALWAYS,     5,      GL | GWC        },
	{ "recall",                     do_recall,              POS_FIGHTING,   LOG_NORMAL,     8,      0                       },
	{ "recite",                     do_recite,              POS_RESTING,    LOG_NORMAL,     7,      0                       },
	{ "reject",                     do_reject,              POS_RESTING,    LOG_NORMAL,     8,      0                       },
	{ "reload",                     do_reload,              POS_RESTING,    LOG_NORMAL,     2,      0                       },
	{ "relocate",           do_relocate,    POS_FIGHTING,   LOG_NORMAL,     8,      0                       },
	{ "relevel",            do_relevel,             POS_DEAD,               LOG_ALWAYS,     1,      0                       },
	{ "remove",                     do_remove,              POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "remort",                     do_remort,              POS_DEAD,               LOG_ALWAYS,     5,      GH                      },
	{ "remexit",            do_remexit,             POS_DEAD,               LOG_ALWAYS,     5,      GWC                     },
	{ "rename",                     do_rename,              POS_RESTING,    LOG_NORMAL,     8,      0                       },
	{ "report",                     do_report,              POS_RESTING,    LOG_NORMAL,     3,      0                       },
	{ "repair",                     do_repair,              POS_STANDING,   LOG_NORMAL,     9,      0                       },
	{ "reply",                      do_reply,               POS_DEAD,               LOG_NORMAL,     1,      0                       },
	{ "replay",                     do_replay,              POS_SLEEPING,   LOG_NORMAL,     1,      0                       },
	{ "rescue",                     do_rescue,              POS_FIGHTING,   LOG_NORMAL,     2,      0                       },
	{ "restore",            do_restore,             POS_DEAD,               LOG_ALWAYS,     5,      GL | GWG        },
	{ "return",                     do_return,              POS_DEAD,               LOG_NORMAL,     8,      0                       },
	{ "revoke",                     do_revoke,              POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "ridea",                      do_ridea,               POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "rlist",                      do_roomlist,    POS_DEAD,               LOG_NORMAL,     5,      GWB                     },
	{ "roleplay",           do_roleplay,    POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "roomexits",          do_roomexits,   POS_DEAD,               LOG_NORMAL,     5,      GWB                     },
	{ "rotate",                     do_rotate,              POS_DEAD,               LOG_NORMAL,     2,      0                       },
	{ "rppaward",           do_rppaward,    POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "rules",                      do_rules,               POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "rwhere",                     do_rwhere,              POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "say",                        do_say,                 POS_RESTING,    LOG_NORMAL,     1,      0                       },
	{ "sacrifice",          do_sacrifice,   POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "save",                       do_save,                POS_DEAD,               LOG_NORMAL,     8,      0                       },
	{ "score",                      do_score,               POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "scan",                       do_scan,                POS_RESTING,    LOG_NORMAL,     3,      0                       },
	{ "scan2",                      do_scan2,               POS_RESTING,    LOG_NORMAL,     3,      0                       },
	{ "scatter",            do_scatter,             POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "scon",                       do_scon,                POS_RESTING,    LOG_NORMAL,     3,      GAV                     },
	{ "scribe",                     do_scribe,              POS_STANDING,   LOG_NORMAL,     7,      0                       },
	{ "scroll",                     do_scroll,              POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "second",                     do_second,              POS_RESTING,    LOG_NORMAL,     2,      0                       },
	{ "secure",                     do_secure,              POS_DEAD,               LOG_NORMAL,     5,      GD | GWG        },
	{ "sectchange",         do_sectchange,  POS_DEAD,               LOG_ALWAYS,     5,      GWC                     },
	{ "sedit",                      do_sedit,               POS_DEAD,               LOG_ALWAYS,     5,      GWB                     },
	{ "sell",                       do_sell,                POS_RESTING,    LOG_NORMAL,     9,      0                       },
	{ "semote",                     do_smote,               POS_DEAD,               LOG_NORMAL,     1,      0                       },
	{ "set",                        do_set,                 POS_DEAD,               LOG_ALWAYS,     5,      GWQ                     },
	{ "setgamein",          do_setgamein,   POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "setgameout",         do_setgameout,  POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "shadowform",         do_shadow,              POS_FIGHTING,   LOG_NORMAL,     2,      0                       },
	{ "shoot",                      do_shoot,               POS_RESTING,    LOG_NORMAL,     2,      0                       },
	{ "shortwho",           do_swho,                POS_DEAD,               LOG_NORMAL,     3,      GWG                     },
	{ "show",                       do_show,                POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "showflags",          do_showflags,   POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "showlast",           do_showlast,    POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "showlost",           do_showlost,    POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "showraff",           do_showraff,    POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "shutdow",            do_shutdow,             POS_DEAD,               LOG_NORMAL,     0,      GWC                     },
	{ "shutdown",           do_shutdown,    POS_DEAD,               LOG_ALWAYS,     5,      GL | GWC        },
	{ "sing",                       do_sing,                POS_STANDING,   LOG_NORMAL,     4,      0                       },
	{ "sit",                        do_sit,                 POS_SLEEPING,   LOG_NORMAL,     8,      0                       },
	{ "skills",                     do_skills,              POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "skillpoint",         do_skillpoint,  POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "sla",                        do_sla,                 POS_DEAD,               LOG_NORMAL,     0,      GWS                     },
	{ "slay",                       do_slay,                POS_DEAD,               LOG_ALWAYS,     5,      GL | GWS        },
	{ "sleep",                      do_sleep,               POS_SLEEPING,   LOG_NORMAL,     8,      0                       },
	{ "slookup",            do_slookup,             POS_DEAD,               LOG_NORMAL,     5,      GWC                     },
//	{ "smite",                   do_smite,               POS_DEAD,               LOG_ALWAYS,     5,      GWS                     },
	{ "sneak",                      do_sneak,               POS_STANDING,   LOG_NORMAL,     8,      0                       },
	{ "snoop",                      do_snoop,               POS_DEAD,               LOG_ALWAYS,     5,      GWS                     },
	{ "social",                     do_globalsocial, POS_SLEEPING,   LOG_NORMAL,     1,      0                       },
	{ "socials",            do_socials,             POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "sockets",            do_sockets,             POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "spells",                     do_spells,              POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "spousetalk",         do_spousetalk,  POS_SLEEPING,   LOG_NORMAL,     1,      0                       },
	{ "spousegate",         do_spousegate,  POS_STANDING,   LOG_NORMAL,     8,      0                       },
	{ "split",                      do_split,               POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "stand",                      do_stand,               POS_SLEEPING,   LOG_NORMAL,     8,      0                       },
	{ "stat",                       do_stat,                POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "steal",                      do_steal,               POS_STANDING,   LOG_NORMAL,     4,      0                       },
	{ "storage",            do_storage,             POS_DEAD,               LOG_ALWAYS,     5,      GD | GWG        },
	{ "story",                      do_story,               POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "string",                     do_string,              POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "strongbox",          do_strongbox,   POS_DEAD,               LOG_ALWAYS,     5,      GWS                     },
	{ "superwiz",           do_superwiz,    POS_DEAD,               LOG_NORMAL,     5,      GD | GWG        },
	{ "swho",                       do_swho,                POS_DEAD,               LOG_NORMAL,     3,      GWG                     },
	{ "switch",                     do_switch,              POS_DEAD,               LOG_ALWAYS,     5,      GWQ                     },
	{ "tell",                       do_tell,                POS_DEAD,               LOG_NORMAL,     1,      0                       },
	{ "tail",                       do_tail,                POS_SLEEPING,   LOG_ALWAYS,     5,      GWS                     },
	{ "take",                       do_get,                 POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "tap",                        do_sacrifice,   POS_RESTING,    LOG_NORMAL,     0,      0                       },
	{ "testpose",           do_testpose,    POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "time",                       do_time,                POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "title",                      do_title,               POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "train",                      do_train,               POS_RESTING,    LOG_NORMAL,     8,      0                       },
	{ "trade",                      do_trade,               POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "transfer",           do_transfer,    POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "trip",                       do_trip,                POS_FIGHTING,   LOG_NORMAL,     2,      0                       },
	{ "typelist",           do_typelist,    POS_DEAD,               LOG_ALWAYS,     5,      GWG                     },
	{ "typo",                       do_typo,                POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "unalias",            do_unalias,             POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "undeny",                     do_undeny,              POS_DEAD,               LOG_ALWAYS,     5,      GWS                     },
	{ "unjoin",                     do_unjoin,              POS_RESTING,    LOG_ALWAYS,     8,      GC                      },
	{ "unlock",                     do_unlock,              POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "unread",                     do_unread,              POS_SLEEPING,   LOG_NORMAL,     3,      0                       },
	{ "value",                      do_value,               POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "video",                      do_video,               POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "violate",            do_violate,             POS_DEAD,               LOG_ALWAYS,     5,      GD | GWG        },
	{ "visible",            do_visible,             POS_SLEEPING,   LOG_NORMAL,     8,      0                       },
	{ "vlist",                      do_vlist,               POS_DEAD,               LOG_NORMAL,     5,      GWB                     },
	{ "vnum",                       do_vnum,                POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "wake",                       do_wake,                POS_SLEEPING,   LOG_NORMAL,     8,      0                       },
	{ "war",                        do_war,                 POS_SLEEPING,   LOG_NORMAL,     6,      0                       },
	{ "wbb",                        do_wbb,                 POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "wbi",                        do_wbi,                 POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "wear",                       do_wear,                POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "weather",            do_weather,             POS_RESTING,    LOG_NORMAL,     3,      0                       },
	{ "weddingring",        do_weddingring, POS_RESTING,    LOG_NORMAL,     9,      0                       },
	{ "where",                      do_where,               POS_RESTING,    LOG_NORMAL,     3,      0                       },
	{ "whisper",            do_whisper,             POS_SLEEPING,   LOG_NEVER,      1,      0                       },
	{ "who",                        do_who,                 POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "whois",                      do_whois,               POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "wield",                      do_wear,                POS_RESTING,    LOG_NORMAL,     4,      0                       },
	{ "wimpy",                      do_wimpy,               POS_DEAD,               LOG_NORMAL,     6,      0                       },
	{ "withdraw",           do_withdraw,    POS_STANDING,   LOG_NORMAL,     9,      0                       },
	{ "wizhelp",            do_wizhelp,             POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "wizgroup",           do_wizgroup,    POS_DEAD,               LOG_ALWAYS,     5,      GL | GWG        },
	{ "wizinvis",           do_invis,               POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "wizify",                     do_wizify,              POS_DEAD,               LOG_ALWAYS,     5,      GL | GWG        },
	{ "wizlist",            do_wizlist,             POS_DEAD,               LOG_NORMAL,     3,      0                       },
	{ "wizlock",            do_wizlock,             POS_DEAD,               LOG_ALWAYS,     5,      GWC                     },
	{ "wiznet",                     do_wiznet,              POS_DEAD,               LOG_NORMAL,     5,      GWG                     },
	{ "worth",                      do_worth,               POS_SLEEPING,   LOG_NORMAL,     3,      0                       },
	{ "work",                       do_work,                POS_DEAD,               LOG_NORMAL,     5,      GWC                     },
	{ "yell",                       do_yell,                POS_RESTING,    LOG_NORMAL,     1,      0                       },
	{ "zap",                        do_zap,                 POS_RESTING,    LOG_NORMAL,     7,      0                       },
	{ "zecho",                      do_zecho,               POS_DEAD,               LOG_NORMAL,     5,      GWQ                     },
	/*
	* End of list.
	*/
	{ "",                           0,                              POS_DEAD,               LOG_NORMAL,     0,      0                       }
};

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret(CHAR_DATA *ch, const char *argument)
{
	char command[MAX_STRING_LENGTH];
	int cmd;
	bool found;
	/* Stuff for Malloc */
	int string_count = nAllocString;
	int perm_count = nAllocPerm;
	char cmd_copy[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	/* End Stuff for Malloc */

	/*
	 * Strip leading spaces.
	 */
	while (isspace(*argument))
		argument++;

	if (argument[0] == '\0') {
		/* VT100 Stuff */
		if (ch->pcdata
		    && IS_SET(ch->pcdata->video, VIDEO_VT100)) {
			goto_line(ch, ch->lines, 1);
			stc(VT_CLEAR_LINE, ch);
		}

		return;
	}

	/*
	 * Implement freeze command.
	 */
	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE)) {
		sprintf(command, "You try to {G%-100.100s{x\n", argument);
		stc(command, ch);
		stc("A powerful force slams you up against the nearest object. YOU'RE FROZEN!\n", ch);
		return;
	}

	/*
	 * Grab the command word.
	 * Special parsing so ' can be a command,
	 *   also no spaces needed after punctuation.
	 */
	strcpy(logline, argument);
	strcpy(cmd_copy, argument);

	if (!isalpha(argument[0]) && !isdigit(argument[0])) {
		command[0] = argument[0];
		command[1] = '\0';
		argument++;

		while (isspace(*argument))
			argument++;
	}
	else
		argument = one_argument(argument, command);

	/*
	 * Look for command in command table.
	 */
	found = FALSE;

	/* temp give them a deputy group for command checking */
	if (HAS_CGROUP(ch, GROUP_LEADER))
		SET_BIT(ch->pcdata->cgroup, GROUP_DEPUTY);

	for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++) {
		/* if it's a mob only command, there should be no multiple cgroup flags */
		if (IS_NPC(ch)) {
			if (cmd_table[cmd].group > 0
			    && cmd_table[cmd].group != GROUP_MOBILE)
				continue;
		}
		else {
			/* players can't do any kind of mob only command */
			if (IS_SET(cmd_table[cmd].group, GROUP_MOBILE))
				continue;

			/* check for ALL cgroups being present */
			if (!HAS_CGROUP(ch, cmd_table[cmd].group) && !Is_Granted(ch, cmd_table[cmd].name))
				continue;
		}

		if (!str_prefix1(command, cmd_table[cmd].name)) {
			found = TRUE;
			strcpy(dv_command, command);
			break;
		}
	}

	/* now remove it, can't have both groups */
	if (HAS_CGROUP(ch, GROUP_LEADER))
		REMOVE_BIT(ch->pcdata->cgroup, GROUP_DEPUTY);

	/*
	 * Log and snoop.
	 */
	if (cmd_table[cmd].log == LOG_NEVER)
		strcpy(logline, "");

	if ((!IS_NPC(ch) && IS_SET(ch->act, PLR_LOG))
	    ||   fLogAll
	    ||   cmd_table[cmd].log == LOG_ALWAYS) {
		sprintf(log_buf, "Log %s: %s", ch->name, logline);

		if (cmd_table[cmd].log == LOG_ALWAYS)
			wiznet(log_buf, ch, NULL, WIZ_SECURE, 0, GET_RANK(ch));

		if (IS_SET(ch->act, PLR_LOG)) {
			char *strtime;
			char tmp[MAX_STRING_LENGTH];
			strtime = ctime(&current_time);
			strtime[strlen(strtime) - 1] = '\0';
			sprintf(tmp, "%s :: %s\n", strtime, log_buf);
			slog_file(ch, SLOG_FILE, tmp);
		}
		else
			log_string(log_buf);
	}

	if (ch->desc != NULL && ch->desc->snoop_by != NULL) {
		write_to_buffer(ch->desc->snoop_by, "% ",    2);
		write_to_buffer(ch->desc->snoop_by, logline, 0);
		write_to_buffer(ch->desc->snoop_by, "\n",  2);
	}

	if (!found) {
		/*
		 * Look for command in socials table.
		 */
		if (!check_social(ch, command, argument))
			do_huh(ch);

		return;
	}
	else /* a normal valid command.. check if it is disabled */
		if (check_disabled(&cmd_table[cmd])) {
			stc("This command has been temporarily disabled.\n", ch);
			return;
		}

	/* Character not in position for command? */
	if (!IS_IMMORTAL(ch) && get_position(ch) < cmd_table[cmd].position) {
		switch (get_position(ch)) {
		case POS_DEAD:
			stc("This is NOT the night of the living DEAD!\n", ch);
			break;

		case POS_MORTAL:
		case POS_INCAP:
			stc("You are hurt far too bad for that.\n", ch);
			break;

		case POS_STUNNED:
			stc("You are too stunned to do that.\n", ch);
			break;

		case POS_SLEEPING:
			stc("In your dreams, or what?\n", ch);
			break;

		case POS_RESTING:
			stc("Nah... You feel too relaxed...\n", ch);
			break;

		case POS_SITTING:
			stc("Better stand up first.\n", ch);
			break;

		case POS_FIGHTING:
			stc("No way!  You are still fighting!\n", ch);
			break;

		case POS_STANDING:
			stc("You can't do that from the ground.\n", ch);
			break;
		}

		return;
	}

	/*
	 * Dispatch the command.
	 */
	(*cmd_table[cmd].do_fun)(ch, argument);

	/* VT100 Stuff */
	if (ch && ch->pcdata && IS_SET(ch->pcdata->video, VIDEO_VT100)) {
		goto_line(ch, ch->lines, 1);
		stc(VT_CLEAR_LINE, ch);
	}

	if (string_count < nAllocString) {
		sprintf(buf,
		        "{PMALLOC{x Increase in strings :: %s : %s", ch->name, cmd_copy) ;
		wiznet(buf, NULL, NULL, WIZ_MALLOC, 0, 0) ;
	}

	if (perm_count < nAllocPerm) {
		sprintf(buf,
		        "{PMALLOC{x Increase in perms :: %s : %s", ch->name, cmd_copy) ;
		wiznet(buf, NULL, NULL, WIZ_MALLOC, 0, 0) ;
	}

	tail_chain();
	return;
}

bool check_social(CHAR_DATA *ch, const char *command, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	struct social_type *iterator;
	bool found;
	found  = FALSE;

	for (iterator = social_table_head->next; iterator != social_table_tail; iterator = iterator->next) {
		if (!str_prefix1(command, iterator->name)) {
			found = TRUE;
			break;
		}
	}

	if (!found)
		return FALSE;

	if (!IS_NPC(ch) && IS_SET(ch->revoke, REVOKE_EMOTE)) {
		stc("You are anti-social!\n", ch);
		return TRUE;
	}

	switch (get_position(ch)) {
	case POS_DEAD:
		stc("This is NOT the night of the living DEAD.\n", ch);
		return TRUE;

	case POS_INCAP:
	case POS_MORTAL:
		stc("You are hurt far too bad for that.\n", ch);
		return TRUE;

	case POS_STUNNED:
		stc("You are too stunned to do that.\n", ch);
		return TRUE;

	case POS_SLEEPING:

		/* I just know this is the path to a 12" 'if' statement.  :(
		   But two players asked for it already!  -- Furey */
		if (!str_cmp(iterator->name, "snore"))
			break;

		stc("Stop fidgeting and get some sleep!\n", ch);
		return TRUE;
	}

	one_argument(argument, arg);
	victim = NULL;

	if (arg[0] == '\0') {
		act(iterator->others_no_arg, ch, NULL, victim, TO_ROOM);
		act(iterator->char_no_arg,   ch, NULL, victim, TO_CHAR);
	}
	else if ((victim = get_char_here(ch, arg, VIS_CHAR)) == NULL)
		stc("They are not here.\n", ch);
	else if (victim == ch) {
		act(iterator->others_auto,   ch, NULL, victim, TO_ROOM);
		act(iterator->char_auto,     ch, NULL, victim, TO_CHAR);
	}
	else {
		act(iterator->others_found,  ch, NULL, victim, TO_NOTVICT);
		act(iterator->char_found,    ch, NULL, victim, TO_CHAR);
		act(iterator->vict_found,    ch, NULL, victim, TO_VICT);

		if (!IS_NPC(ch) && IS_NPC(victim)
		    &&   !affect_exists_on_char(victim, gsn_charm_person)
		    &&   IS_AWAKE(victim)
		    &&   victim->desc == NULL
		    && (!IS_SET(victim->pIndexData->progtypes, ACT_PROG))) {
			switch (number_bits(4)) {
			case 0:
			case 1: case 2: case 3: case 4:
			case 5: case 6: case 7: case 8:
				act(iterator->others_found,
				    victim, NULL, ch, TO_NOTVICT);
				act(iterator->char_found,
				    victim, NULL, ch, TO_CHAR);
				act(iterator->vict_found,
				    victim, NULL, ch, TO_VICT);
				break;

			case 9: case 10: case 11: case 12:
				act("$n raises an eyebrow at $N.", victim, NULL, ch, TO_NOTVICT);
				act("You raise an eyebrow at $N.", victim, NULL, ch, TO_CHAR);
				act("$n raises an eyebrow at you.", victim, NULL, ch, TO_VICT);
				break;
			}
		}
	}

	/* VT100 Stuff */
	if (ch->pcdata && IS_SET(ch->pcdata->video, VIDEO_VT100)) {
		goto_line(ch, ch->lines, 1);
		stc(VT_CLEAR_LINE, ch);
	}

	return TRUE;
}

/*
 * Return true if an argument is completely numeric.
 */
bool is_number(const char *arg)
{
	if (*arg == '\0')
		return FALSE;

	if (*arg == '+' || *arg == '-')
		arg++;

	for (; *arg != '\0'; arg++) {
		if (!isdigit(*arg))
			return FALSE;
	}

	return TRUE;
}

/*
 * Given a string like 14.foo, return 14 and 'foo'
 *
 * It is most certainly NOT ok for argument to be identical to arg!
 * The strcpy at the end sometimes throws a SIGABRT on overlap! -- Montrey
 */
int number_argument(const char *argument, char *arg)
{
	/* Check for leading digit. Saves time */
	if (*argument >= '0' && *argument <= '9') {
		char *pdot;
		/* Check for dot */
		pdot = strstr(argument, ".");

		if (pdot != NULL) {
			char tmp_buf[MAX_INPUT_LENGTH];
			strcpy(tmp_buf, argument);
			pdot = &tmp_buf[pdot - argument];
			*pdot = '\0';

			/* Check for good numeric */
			if (is_number(tmp_buf)) {
				/* strip out number, return numeric value */
				strcpy(arg, pdot + 1);
				return atoi(tmp_buf);
			}
		}
	}

	strcpy(arg, argument);
	return 1;
}

/*
 * scan a 'p.' or 'm.' or 'o.' or 'r.' out of an argument
 * before or after a <number>'.' .
 *
 * 'p.'{<number>'.'}<name> -> ENTITY_P and {<number>'.'}<name>
 * 'm.'{<number>'.'}<name> -> ENTITY_M and {<number>'.'}<name>
 * <number>'.p.'<name> -> ENTITY_P and <number>'.'<name>
 * <number>'.m.'<name> -> ENTITY_M and <number>'.'<name>
 *
 * - It's OK for argument to be identical to arg, but arg MUST be a
 * character array rather than a str_dup()'d buffer, as its length
 * may change!
 *
 * It is most certainly NOT ok for argument to be identical to arg!
 * The strcpy at the end sometimes throws a SIGABRT on overlap! -- Montrey
 */
int entity_argument(const char *argument, char *arg)
{
	char tmp_buf[MAX_STRING_LENGTH];
	char *ap;   /* pointer to argument */
	int  number;
	int  etype = 0;
	/* check for and isolate leading numeric */
	number = number_argument(argument, tmp_buf);
	ap = tmp_buf;

	if (ap[0] != '\0' && ap[1] == '.') {
		switch (LOWER(ap[0])) {
		case 'p':
			etype = ENTITY_P;  ap += 2;
			break;

		case 'm':
			etype = ENTITY_M;  ap += 2;
			break;

		case 'o':
			etype = ENTITY_O;  ap += 2;
			break;

		case 'r':
			etype = ENTITY_R;  ap += 2;
			break;
		}
	}
	else if (ap[0] != '\0' && ap[1] == '#') {
		switch (LOWER(ap[0])) {
		case 'm':
			etype = ENTITY_VM;  ap += 2;
			break;

		case 'o':
			etype = ENTITY_VO;  ap += 2;
			break;

		case 'r':
			etype = ENTITY_VR;  ap += 2;
			break;
		}
	}

	/* recombine number (if any) and remaining arg. */
	if (number == 1)
		strcpy(arg, ap);
	else
		sprintf(arg, "%d.%s", number, ap);

	return etype;
}

/*
 * Given a string like 14*foo, return 14 and 'foo'
*/
int mult_argument(const char *argument, char *arg)
{
	char *pstar;
	int number;
	pstar = strchr(argument, '*');

	if (pstar == NULL) {
		/* no star: return 1 */
		strcpy(arg, argument);
		return 1;
	}

	*pstar = '\0';
	number = atoi(argument);
	*pstar = '*';
	strcpy(arg, pstar + 1);
	return number;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
const char *one_argument(const char *argument, char *arg_first)
{
	char cEnd;

	while (isspace(*argument))
		argument++;

	cEnd = ' ';

	if (*argument == '\'' || *argument == '"')
		cEnd = *argument++;

	while (*argument != '\0') {
		if (*argument == cEnd) {
			argument++;
			break;
		}

		*arg_first = /*LOWER(*/*argument/*)*/;
		arg_first++;
		argument++;
	}

	*arg_first = '\0';

	while (isspace(*argument))
		argument++;

	return argument;
}

/*
 * Contributed by Alander, modified by Lotus
 */
void do_commands(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	int cmd;
	int col;
	int x;
	int arg;
	int counter;
	struct field_struct {
		char  *name;
		int  number;
	} fields[] = {
		{ "Not Printed",              0 },
		{ "Communications",           1 },
		{ "Combat Related",           2 },
		{ "Informational",            3 },
		{ "Object Manipulation",      4 },
		{ "Immortal Wizstuff",        5 },
		{ "Custom Settings",          6 },
		{ "Magic Related",            7 },
		{ "Character Manipulation",   8 },
		{ "Shop Manipulation",        9 },
		{ "\n",                       0 }
	};

	if (argument[0] == '\0') {
		stc("Command Structure:\n\n", ch);

		for (x = 1, counter = 1; fields[x].number; x++) {
			sprintf(buf, "[%d] %s\n", counter, fields[x].name);
			stc(buf, ch);
			counter++;
		}

		return;
	}

	if (is_number(argument)) {
		arg = atoi(argument);

		for (x = 1; fields[x].number != 0; x++)
			if (fields[x].number == arg)
				break;
	}
	else {
		for (x = 1; fields[x].number != 0; x++)
			if (!str_prefix1(argument, fields[x].name))
				break;
	}

	if (*(fields[x].name) == '\n') {
		stc("Invalid Option.\n", ch);
		return;
	}

	col = 0;

	for (cmd = 0, counter = 1; cmd_table[cmd].name[0] != '\0'; cmd++) {
		if (IS_IMM_GROUP(cmd_table[cmd].group))
			continue;

		/* if it's a mob only command, there should be no multiple cgroup flags */
		if (IS_NPC(ch)) {
			if (cmd_table[cmd].group > 0
			    && cmd_table[cmd].group != GROUP_MOBILE)
				continue;
		}
		else {
			/* players can't do any kind of mob only command */
			if (IS_SET(cmd_table[cmd].group, GROUP_MOBILE))
				continue;

			/* check for ALL cgroups being present */
			if (!HAS_CGROUP(ch, cmd_table[cmd].group))
				continue;
		}

		if (cmd_table[cmd].show == fields[x].number) {
			sprintf(buf, "[%2d] %-14s", counter, cmd_table[cmd].name);
			stc(buf, ch);

			if (++col % 4 == 0)
				stc("\n", ch);

			counter++;
		}
	}

	if (col % 4 != 0)
		stc("\n", ch);

	return;
}

void do_huh(CHAR_DATA *ch)
{
	char   *const   message        [] = {
		"Huh?\n",
		"Pardon?\n",
		"Excuse me?\n",
		"What?\n",
		"Say again.\n",
		"Eh?\n",
		"Can you repeat that?\n",
		"You want what?\n",
		"Ummm...\n",
		"Syntax Error\n",
		"Come again?\n",
		"Bad command or file name\n"
	};
	stc(message[number_range(0, 11)], ch);
	return;
}

void do_wizhelp(CHAR_DATA *ch, const char *argument)
{
	int cmd, col, i;

	/* temp give them a deputy group */
	if (HAS_CGROUP(ch, GROUP_LEADER))
		SET_BIT(ch->pcdata->cgroup, GROUP_DEPUTY);

	for (i = 0; cgroup_flags[i].name != NULL; i++) {
		if (!IS_IMM_GROUP(cgroup_flags[i].bit))
			continue;

		if (!HAS_CGROUP(ch, cgroup_flags[i].bit))
			continue;

		ptc(ch, "%s{G%s{x\n", i > 0 ? "\n" : "", cgroup_flags[i].name);
		col = 0;

		for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++) {
			if (IS_SET(cmd_table[cmd].group, cgroup_flags[i].bit)
			    && (HAS_CGROUP(ch, cmd_table[cmd].group)
			        || Is_Granted(ch, cmd_table[cmd].name))
			    && cmd_table[cmd].show) {
				if (col % 5 == 0)
					stc("        ", ch);

				ptc(ch, "%s%-14s{x",
				    IS_SET(cmd_table[cmd].group, GROUP_LEADER) ? "{Y" :
				    IS_SET(cmd_table[cmd].group, GROUP_DEPUTY) ? "{C" : "",
				    cmd_table[cmd].name);

				if (++col % 5 == 0)
					stc("\n", ch);
			}
		}

		if (col % 5 != 0)
			stc("\n", ch);
	}

	/* now remove it */
	if (HAS_CGROUP(ch, GROUP_LEADER))
		REMOVE_BIT(ch->pcdata->cgroup, GROUP_DEPUTY);
}

void slog_file(CHAR_DATA *ch, char *file, char *str)
{
	FILE *fp;

	if (IS_NPC(ch) || str[0] == '\0')
		return;

	if ((fp = fopen(file, "a")) == NULL) {
		perror(file);
		stc("Could not open the file!\n", ch);
	}
	else {
		fprintf(fp, "%s\n", str);
		fclose(fp);
	}

	return;
}

/* Check if that command is disabled
   Note that we check for equivalence of the do_fun pointers; this means
   that disabling 'chat' will also disable the '.' command
*/
bool check_disabled(const struct cmd_type *command)
{
	DISABLED_DATA *p;

	for (p = disabled_first; p ; p = p->next)
		if (p->command->do_fun == command->do_fun)
			return TRUE;

	return FALSE;
}

void load_disabled()
{
	DISABLED_DATA *p;
	int i;
	disabled_first = NULL;

	if (db_query("load_disabled", "SELECT command, immortal, reason FROM disabled") != SQL_OK)
		return;

	while (db_next_row() == SQL_OK) {
		for (i = 0; cmd_table[i].name[0]; i++)
			if (!str_cmp(cmd_table[i].name, db_get_column_str(0)))
				break;

		if (!cmd_table[i].name[0]) {
			bug("load_disabled: skipping uknown command", 0);
			continue;
		}

		p = alloc_mem(sizeof(DISABLED_DATA));
		p->command = &cmd_table[i];
		p->disabled_by = str_dup(db_get_column_str(1));
		p->reason = str_dup(db_get_column_str(2));
		p->next = disabled_first;
		disabled_first = p;
	}
}

void do_disable(CHAR_DATA *ch, const char *argument)
{
	char cmd[MIL];
	DISABLED_DATA *p;
	int i;
	argument = one_argument(argument, cmd);

	if (cmd[0] == '\0') {
		if (!disabled_first) { /* Any disabled at all ? */
			stc("There are no commands disabled.\n", ch);
			return;
		}

		stc("Command             {T|{xDisabled by    {T|{xReason\n"
		    "{T--------------------|---------------|-----------------------------------------------{x\n", ch);

		for (p = disabled_first; p; p = p->next)
			ptc(ch, "%-20s{T|{x%-15s{T|{x%s\n", p->command->name, p->disabled_by, p->reason);

		return;
	}

	/* First check if it is one of the disabled commands */
	for (p = disabled_first; p ; p = p->next)
		if (!str_cmp(cmd, p->command->name))
			break;

	if (p) { /* found the command, enable it */
		if (disabled_first == p)
			disabled_first = p->next;
		else {
			DISABLED_DATA *q;

			for (q = disabled_first; q && q->next; q = q->next)
				if (q->next == p)
					q->next = p->next;
		}

		/* remove it from the database */
		db_commandf("do_disable", "DELETE FROM disabled WHERE command LIKE '%s'", db_esc(p->command->name));
		free_string(p->disabled_by);
		free_string(p->reason);
		free_mem(p, sizeof(DISABLED_DATA));
		stc("Command enabled.\n", ch);
		return;
	}

	/* IQ test */
	if (!str_cmp(cmd, "disable")) {
		stc("You cannot disable the disable command.\n", ch);
		return;
	}

	if (argument[0] == '\0') {
		stc("You must provide a reason.\n", ch);
		return;
	}

	if (strlen(argument) > 45) {
		stc("Please limit the reason to 45 characters or less.\n", ch);
		return;
	}

	for (i = 0; cmd_table[i].name[0] != '\0'; i++)
		if (!str_cmp(cmd_table[i].name, cmd))
			break;

	if (cmd_table[i].name[0] == '\0') {
		stc("No such command.\n", ch);
		return;
	}

	/* maybe a command group check here? thinking about it */
	p = alloc_mem(sizeof(DISABLED_DATA));
	p->command      = &cmd_table[i];
	p->disabled_by  = str_dup(ch->name);
	p->reason       = str_dup(argument);
	strcut(p->disabled_by, 25);
	strcut(p->reason, 100);
	p->next = disabled_first;
	disabled_first = p;
	/* add it to the database */
	db_commandf("do_disable", "INSERT INTO disabled VALUES('%s','%s','%s')",
	            db_esc(p->command->name), db_esc(p->disabled_by), db_esc(p->reason));
	stc("Command disabled.\n", ch);
}

/*
This function checks to see if a given commands
has been granted to the character.
-- Outsider
*/
bool Is_Granted(CHAR_DATA *ch, const char *argument)
{
	int index = 0;
	bool found = FALSE;

	if (IS_NPC(ch))
		return FALSE;

	while ((index < MAX_GRANT) && (! found)) {
		if (! strcmp(ch->pcdata->granted_commands[index], argument))
			found = TRUE;
		else
			index++;
	}

	return found;
}

