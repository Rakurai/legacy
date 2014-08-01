/*************************************************
*                                                *
*               The Age of Legacy                *
*                                                *
* Based originally on ROM 2.4, tested, enhanced, *
* and maintained by the Legacy Team.  If that    *
* doesn't mean you, and you're stealing our      *
* code, at least tell us and boost our egos ;)   *
*************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "interp.h"


const struct helpfile_table_type helpfile_table[] =
{
	{	"wizhelp",	HELP_WIZHELP	},
	{	"wizquest",	HELP_WIZQUEST	},
	{	"wizsecure",	HELP_WIZSECURE	},
	{	"wizcode",	HELP_WIZCODE	},
	{	"wizgen",	HELP_WIZGEN	},
	{	"wizbuild",	HELP_WIZBUILD	},
	{	"skill",	HELP_SKILL	},
	{	"spell",	HELP_SPELL	},
	{	"race",		HELP_RACE	},
	{	"class",	HELP_CLASS	},
	{	"remort",	HELP_REMORT	},
	{	"clan",		HELP_CLAN	},
	{	"move",		HELP_MOVE	},
	{	"object",	HELP_OBJECT	},
	{	"comm",		HELP_COMM	},
	{	"info",		HELP_INFO	},
	{	"combat",	HELP_COMBAT	},
	{	"misc",		HELP_MISC	},
	{	NULL,		0		}
};


/* random name generation tables, male and female syllable sets 1, 2, 3 */
const char *Msyl1[] =
{
	"A",	"Ab",	"Ac",	"Ad",	"Af",	"Agr",	"Ast",	"As",	"Al",	"Adw",
	"Adr",	"Ar",	"B",	"Br",	"C",	"C",	"C",	"Cr",	"Ch",	"Cad",
	"D",	"Dr",	"Dw",	"Ed",	"Eth",	"Et",	"Er",	"El",	"Eow",	"F",
	"Fr",	"G",	"Gr",	"Gw",	"Gw",	"Gal",	"Gl",	"H",	"Ha",	"Ib",
	"J",	"Jer",	"K",	"Ka",	"Ked",	"L",	"Loth",	"Lar",	"Leg",	"M",
	"Mir",	"N",	"Nyd",	"Ol",	"Oc",	"On",	"P",	"Pr",	"Q",	"R",
	"Rh",	"S",	"Sev",	"T",	"Tr",	"Th",	"Th",	"Ul",	"Um",	"Un",
	"V",	"Y",	"Yb",	"Z",	"W",	"W",	"Wic"
};

const char *Msyl2[] =
{
	"a",	"ae",	"ae",	"au",	"ao",	"are",	"ale",	"ale",	"ay",	"ardo",
	"e",	"edri",	"ei",	"ea",	"ea",	"eri",	"era",	"ela",	"enda",	"erra",
	"i",	"ia",	"ie",	"ira",	"ila",	"ili",	"ira",	"ire",	"igo",	"o",
	"oha",	"oma",	"oa",	"oe",	"oi",	"ore",	"u",	"y"
};

const char *Msyl3[] =
{
	"a",	"and",	"b",	"bwyn",	"baen",	"bard",	"c",	"ch",	"can",	"d",
	"dan",	"don",	"der",	"dric",	"dus",	"f",	"g",	"gord",	"gan",	"han",
	"har",	"jar",	"jan",	"k",	"kin",	"kith",	"kath",	"koth",	"kor",	"kon",
	"l",	"li",	"lin",	"lith",	"lath",	"loth",	"ld",	"ldan",	"m",	"mas",
	"mos",	"mar",	"mond",	"n",	"nydd",	"nidd",	"nnon",	"nwan",	"nyth",	"nad",
	"nn",	"nnor",	"nd",	"p",	"r",	"red",	"ric",	"rid",	"rin",	"ron",
	"rd",	"s",	"sh",	"seth",	"sean",	"t",	"th",	"th",	"tha",	"tlan",
	"trem",	"tram",	"v",	"vudd",	"w",	"wan",	"win",	"win",	"wyn",	"wyn",
	"wyr",	"wyr",	"wyth"
};

const char *Fsyl1[] =
{
	"A",	"Ab",	"Ac",	"Ad",	"Af",	"Agr",	"Ast",	"As",	"Al",	"Adw",
	"Adr",	"Ar",	"B",	"Br",	"C",	"C",	"C",	"Cr",	"Ch",	"Cad",
	"D",	"Dr",	"Dw",	"Ed",	"Eth",	"Et",	"Er",	"El",	"Eow",	"F",
	"Fr",	"G",	"Gr",	"Gw",	"Gw",	"Gal",	"Gl",	"H",	"Ha",	"Ib",
	"Jer",	"K",	"Ka",	"Ked",	"L",	"Loth",	"Lar",	"Leg",	"M",	"Mir",
	"N",	"Nyd",	"Ol",	"Oc",	"On",	"P",	"Pr",	"Q",	"R",	"Rh",
	"S",	"Sev",	"T",	"Tr",	"Th",	"Th",	"Ul",	"Um",	"Un",	"V",
	"W",	"W",	"Wic",	"Y",	"Yb",	"Z"
};

const char *Fsyl2[] =
{
	"a",	"a",	"a",	"ae",	"ae",	"au",	"ao",	"are",	"ale",	"ali",
	"ay",	"ardo",	"e",	"e",	"e",	"ei",	"ea",	"ea",	"eri",	"era",
	"ela",	"eli",	"enda",	"erra",	"i",	"i",	"i",	"ia",	"ie",	"ire",
	"ira",	"ila",	"ili",	"ira",	"igo",	"o",	"oa",	"oi",	"oe",	"ore",
	"u",	"y"
};

const char *Fsyl3[] =
{
	"beth",	"cia",	"cien",	"clya",	"de",	"dia",	"dda",	"dien",	"dith",	"dia",
	"lind",	"lith",	"lia",	"lian",	"lla",	"llan",	"lle",	"ma",	"mma",	"mwen",
	"meth",	"n",	"n",	"n",	"nna",	"ndra",	"ng",	"ni",	"nia",	"niel",
	"rith",	"rien",	"ria",	"ri",	"rwen",	"sa",	"sien",	"ssa",	"ssi",	"swen",
	"thien","thiel","viel",	"via",	"ven",	"veth",	"wen",	"wen",	"wen",	"wen",
	"wia",	"weth",	"wien",	"wiel"
};


/* random mob title generation, based on basic class */
const struct MagT_type MagT_table[] =
{
	{	"wizard",		"wizardess"		},
	{	"spellcaster",		"spellcaster"		},
	{	"prestidigitator",	"prestidigitator"	},
	{	"evoker",		"evoker"		},
	{	"conjuror",		"conjuress"		},
	{	"abjurer",		"abjuress"		},
	{	"summoner",		"summoner"		},
	{	"theurge",		"theurge"		},
	{	"thaumaturge",		"thaumaturge"		},
	{	"magician",		"witch"			},
	{	"enchanter",		"enchantress"		},
	{	"warlock",		"war witch"		},
	{	"sorcerer",		"sorceress"		},
	{	"mage",			"mage"			},
	{	"scribe",		"scribess"		},
	{	"seer",			"seeress"		},
	{	"sage",			"sage"			},
	{	"illusionist",		"illusionist"		},
	{	"invoker",		"invoker"		},
	{	"savant",		"savant"		},
	{	"magus",		"craftess"		},
	{	NULL,			NULL			}
};

const struct CleT_type CleT_table[] =
{
	{	"cleric",		"cleric"		},
	{	"shaman",		"shaman"		},
	{	"acolyte",		"acolyte"		},
	{	"adept",		"adept"			},
	{	"friar",		"friaress"		},
	{	"missionary",		"missionary"		},
	{	"deacon",		"deaconess"		},
	{	"vicar",		"vicaress"		},
	{	"priest",		"priestess"		},
	{	"curate",		"curess"		},
	{	"monk",			"nun"			},
	{	"healer",		"healess"		},
	{	"chaplain",		"chaplain"		},
	{	"expositor",		"expositress"		},
	{	"bishop",		"bishop"		},
	{	"patriarch",		"matriarch"		},
	{	"demon killer",		"demon killer"		},
	{	"evangelist",		"evangelist"		},
	{	NULL,			NULL			}
};

const struct ThiT_type ThiT_table[] =
{
	{	"thief",		"thief"			},
	{	"cutpurse",		"cutpurse"		},
	{	"treasure hunter",	"treasure hunter"	},
	{	"footpad",		"footpad"		},
	{	"larcenist",		"larcenist"		},
	{	"scout",		"scout"			},
	{	"filcher",		"filcheress"		},
	{	"burglar",		"burglaress"		},
	{	"highwayman",		"highwaywoman"		},
	{	"robber",		"robber"		},
	{	"pickpocket",		"pickpocket"		},
	{	"sharper",		"sharpress"		},
	{	"rogue",		"rogue"			},
	{	"magsman",		"magswoman"		},
	{	"knifer",		"knifer"		},
	{	"murderer",		"murderess"		},
	{	"quickblade",		"quickblade"		},
	{	"brigand",		"brigand"		},
	{	"bravo",		"bravo"			},
	{	"cutthroat",		"cutthroat"		},
	{	"assassin",		"assassin"		},
	{	"thug",			"thug"			},
	{	NULL,			NULL			}
};

const struct WarT_type WarT_table[] =
{
	{	"sentry",		"sentress"		},
	{	"fighter",		"fighter"		},
	{	"soldier",		"soldier"		},
	{	"warrior",		"warrior"		},
	{	"veteran",		"veteran"		},
	{	"swordsman",		"swordswoman"		},
	{	"fencer",		"fenceress"		},
	{	"hero",			"heroine"		},
	{	"myrmidon",		"myrmidon"		},
	{	"swashbuckler",		"swashbuckleress"	},
	{	"mercenary",		"mercenaress"		},
	{	"swordmaster",		"swordmistress"		},
	{	"champion",		"champion"		},
	{	"cavalier",		"cavalier"		},
	{	"knight",		"lady knight"		},
	{	"paladin",		"paladin"		},
	{	"demon slayer",		"demon slayer"		},
	{	"weaponsmaster",	"weaponsmistress"	},
	{	"battledancer",		"battledancer"		},
	{	"barbarian",		"barbarian"		},
	{	"knight terminus",	"lady knight terminus"	},
	{	"rogue knight",		"rogue knight"		},
	{	NULL,			NULL			}
};


const struct chan_type chan_table[] =
{
	{	"gossip",	"You gossip",	"$n{x gossips '$t{x'",
		CSLOT_CHAN_GOSSIP,	COMM_NOGOSSIP,	REVOKE_GOSSIP	},
	{	"flame",	"You *FlAmE*",	"$n{x *FlAmEs* '$t{x'",
		CSLOT_CHAN_FLAME,	COMM_NOFLAME,	REVOKE_FLAME	},
	{	"qwest",	"You qwest",	"($n{x) QWEST: $t{x",
		CSLOT_CHAN_QWEST,	COMM_NOQWEST,	REVOKE_QWEST	},
	{	"pray",		"You implore the gods:",	"$n{x implores the gods: \"$t{x\"",
		CSLOT_CHAN_PRAY,	COMM_NOPRAY,	REVOKE_PRAY	},
	{	"clantalk",	"You clan",	"$n{x clans '$t{x'",
		CSLOT_CHAN_CLAN,	COMM_NOCLAN,	REVOKE_CLAN	},
	{	"music",	"You MUSIC:",	"$n{x MUSIC: '$t{x'",
		CSLOT_CHAN_MUSIC,	COMM_NOMUSIC,	REVOKE_MUSIC	},
	{	"IC",		"{G*{T({CIC{T){G*{x",	"{T({CIC{T) {G*{W$n{G*{x '$t{x'",
		CSLOT_CHAN_IC,		COMM_NOIC,	REVOKE_IC	},
	{	"grats",	"You grat",	"$n{x grats '$t{x'",
		CSLOT_CHAN_GRATS,	COMM_NOGRATS,	REVOKE_GRATS	},
	{	"immtalk",	"none",		"none",
		CSLOT_CHAN_IMM,		COMM_NOWIZ,	REVOKE_NULL	},
	{	"qa",		"You Q/A",	"[Q/A] $n{x '$t{x'",
		CSLOT_CHAN_QA,		COMM_NOQUESTION,REVOKE_QA	},
	{	NULL,		NULL,			NULL,
		0,			0,		0		}
};


const struct revoke_type revoke_table[] =
{
	/* name, bit, message */
	{ "flameonly",	REVOKE_FLAMEONLY,	"ability to use channels other than FLAME"	},
	{ "nochannel",	REVOKE_NOCHANNELS,	"channel priviledges"				},
	{ "pray",	REVOKE_PRAY,		"ability to PRAY"				},
	{ "gossip",	REVOKE_GOSSIP,		"ability to GOSSIP"				},
	{ "flame",	REVOKE_FLAME,		"ability to FLAME"				},
	{ "qwest",	REVOKE_QWEST,		"ability to QWEST"				},
	{ "auction",	REVOKE_AUCTION,		"ability to AUCTION"				},
	{ "clantalk",	REVOKE_CLAN,		"ability to CLANTALK"				},
	{ "music",	REVOKE_MUSIC,		"ability to MUSIC"				},
	{ "question",	REVOKE_QA,		"ability to QUESTION and ANSWER"		},
	{ "answer",	REVOKE_QA,		"ability to QUESTION and ANSWER"		},
	{ "ic",		REVOKE_IC,		"ability to talk IN CHARACTER"			},
	{ "social",	REVOKE_SOCIAL,		"ability to SOCIALize"				},
	{ "grats",	REVOKE_GRATS,		"ability to offer GRATS"			},
	{ "page",	REVOKE_PAGE,		"ability to PAGE people"			},
	{ "qtell",	REVOKE_QTELL,		"ability to talk to your QUERY"			},
	{ "say",	REVOKE_SAY,		"ability to SAY anything"			},
	{ "tell",	REVOKE_TELL,		"ability to send TELLs"				},
	{ "emote",	REVOKE_EMOTE,		"ability to EMOTE"				},
	{ "exp",	REVOKE_EXP,		"ability to learn from your EXPeriences"	},
	{ "note",	REVOKE_NOTE,		"ability to write NOTEs"			},
	{ NULL,		0,			NULL						}
};


const struct flag_type cgroup_flags[] =
{
	{	"General",	GROUP_GEN,		TRUE	},
	{	"Quest",	GROUP_QUEST,	TRUE	},
	{	"Building",	GROUP_BUILD,	TRUE	},
	{	"Coder",	GROUP_CODE,		TRUE	},
	{	"Security",	GROUP_SECURE,	TRUE	},
	{	"Player",	GROUP_PLAYER,	TRUE	},
	{	"Mobile",	GROUP_MOBILE,	TRUE	},
	{	"Clan",		GROUP_CLAN,		TRUE	},
	{	"Avatar",	GROUP_AVATAR,	TRUE	},
	{	"Hero",		GROUP_HERO,		TRUE	},
	{	"Deputy",	GROUP_DEPUTY,	TRUE	},
	{	"Leader",	GROUP_LEADER,	TRUE	},
	{	NULL,		0,				0		}
};


/* wiznet table and prototype for future flag setting */
/* must be added in order of level for config to work */
const struct wiznet_type wiznet_table[] =
{
	{    "wiznet",       WIZ_ON,         IMM, "You will see wiznet messages." },
	{    "prefix",       WIZ_PREFIX,     IMM, "You will see the {G<W{Hizne{Gt>{x prefix." },
	{    "logins",       WIZ_LOGINS,     IMM, "You will be notified when players arrive and depart." },
	{    "links",        WIZ_LINKS,      IMM, "You will be notified when players go linkdead and return." },
	{    "newbies",      WIZ_NEWBIE,     IMM, "You will be notified of new players." },
	{    "spam",         WIZ_SPAM,       IMM, "You will be notified of spam." },
	{    "deaths",       WIZ_DEATHS,     IMM, "You will be notified when a player dies." },
	{    "penalties",    WIZ_PENALTIES,  IMM, "You will be notified of nochannels, bans, freezes, etc." },
	{    "levels",       WIZ_LEVELS,     IMM, "You will be notified when players level." },
	{    "misc",         WIZ_MISC,       IMM, "You will be notified when players change PK or RP status." },
	{    "cheaters",     WIZ_CHEAT,      IMM, "You will be notified when players attempt to exploit the system." },
	{    "mail",         WIZ_MAIL,       IMM, "You will see who is posting mail or changing their email address." },
	{    "auction",      WIZ_AUCTION,    IMM, "You will be notified of auction details." },
	{    "quests",       WIZ_QUEST,      IMM, "You will be notified of quest details." },
	{    "bugs",         WIZ_BUGS,       IMM, "You will see bug error messages." },
	{    "flags",        WIZ_FLAGS,      IMM, "You will be notified of killer/thief flags." },
	{    "resets",       WIZ_RESETS,     IMM, "You will be notified when an area resets." },
	{    "mobdeaths",    WIZ_MOBDEATHS,  IMM, "You will be notified when an NPC dies." },
	{    "saccing",      WIZ_SACCING,    IMM, "You will be notified of sacrifices." },
	{    "restores",     WIZ_RESTORE,    HED, "You will see restore commands." },
	{    "purges",       WIZ_PURGE,      HED, "You will see purge commands." },
	{    "loads",        WIZ_LOAD,       HED, "You will see load commands." },
	{    "switches",     WIZ_SWITCHES,   HED, "You will see morphs." },
	{    "sites",        WIZ_SITES,      HED, "You will see the site address on connects." },
	{    "malloc",       WIZ_MALLOC,     IMP, "You will see memory allocation spam." },
	{    "snoops",       WIZ_SNOOPS,     IMP, "You will see snoop commands." },
	{    "secure",       WIZ_SECURE,     IMP, "The catch all of generic Imm commands." },
	{    NULL,           0,              0,  NULL }
};


const struct raffects raffects[MAX_RAFFECTS] =
{
/* id - raffect id #
   group - group #.  code won't pick twice from the same group of affects.  raffects in 0 group are exempt
   chance - percentage chance of getting that raffect
   description - what shows up when remorting and in stat
   added - added vuln/res in remorting

	"description"
	"short name"	group,	id,	chance,	added

*/
    {	"NULL REMORT AFFECT",   /* Problems if someone gets this */
    	"NULLRAFF",	0,	1,	100,	0		},

	/* "Good" Remort Affects (2-99) */

    {	"Light Feet",		/* Movement cost is halved */
    	"lightfeet",	0,	2,	80,	0		},
    {	"Magic Sensitivity",	/* Casting lag reduced by 1/4 */
    	"fastcast",	1,	3,	70,	0		},
    {	"Energy Absorption",	/* get back level/10+1 mana per combat round */
    	"mageregen",	2,	4,	50,	0		},
    {	"Gift of the Troll",	/* get back level/20+1 hp per combat round */
    	"vampregen",	2,	5,	50,	0		},
    {	"Careful Learner",	/* +10% exp per kill */
    	"moreexp",	3,	6,	80,	0		},
    {	"Intimidating Presence",/* +5% damage */
    	"moredam",	4,	7,	80,	0		},
    {	"Natural Channeling",	/* Mana cost reduced by 10% */
    	"cheapspell",	5,	8,	70,	0		},
    {	"Immunity to Hunger",	/* No need to eat */
    	"nohunger",	6,	9,	50,	0		},
    {	"Immunity to Thirst",	/* No need to drink */
    	"nothirst",	7,	10,	50,	0		},
    {	"Favor of the Heavens",	/* Chance of getting up to 4x experience after a fight */
    	"favor",	8,	11,	50,	0		},
/*    {	"Free Portals",		holding a warp stone for nexus/portal is unnecessary
    	"freeport",	0,	14,	70,	0		},
    {	"Sixth Sense",		Chance of knowing when someone enters the room sneaking
    	"sixthsense",	0,	24,	70,	0		},
    {	"Farsight",		Can see two rooms in all directions on scanning
    	"farsight",	0,	25,	70,	0		}, */


	/* "Bad" Remort Affects (100-199) */

    {	"Poor Memory",		/* Recall has a chance of random teleport */
    	"buggyrec",	0,	100,	60,	0		},
    {	"Leaden Feet",		/* 1/4 second lag on walking */
    	"slowwalk",	0,	102,	80,	0		},
    {	"Clumsiness",		/* Randomly fall down in combat */
    	"clumsy",	0,	103,	60,	0		},
    {	"Weak Grip",		/* Randomly drop your weapon in combat */
    	"weakgrip",	0,	104,	50,	0		},
    {	"Magic Unresponsiveness",/* Casting lag increased by 1/4 */
    	"slowcast",	1,	105,	70,	0		},
    {	"Oblivious Learner",	/* -10% exp per kill */
    	"lessexp",	3,	106,	80,	0		},
    {	"Laughable Presence",	/* -5% damage */
    	"lessdam",	4,	107,	80,	0		},
    {	"Poor Magic Channeling",/* Mana cost increased by 10% */
    	"costspell",	5,	108,	70,	0		},
    {	"Fast Metabolism",	/* Get hungry twice as fast */
    	"exhungry",	6,	109,	70,	0		},
    {	"Abnormal Dehydration",	/* Get thirsty twice as fast */
    	"exthirsty",	7,	110,	70,	0		},
    {	"Jest of the Heavens",	/* Chance of getting up to 1/4 experience after a fight */
    	"jest",		8,	111,	50,	0		},
/*    {	"Bad Direction Sense",	Randomly walk the wrong direction
    	"baddir",	0,	23,	50,	0		}, */

	/* Vulnerabilities (900-949) */

    {	"Natural Vulnerability to Slash",
    	"VSlash",	950,	900,	15,	VULN_SLASH	},
    {	"Natural Vulnerability to Pierce",
    	"VPierce",	901,	901,	15,	VULN_PIERCE	},
    {	"Natural Vulnerability to Blunt",
    	"VBlunt",	902,	902,	15,	VULN_BASH	},
    {	"Natural Vulnerability to Fire",
    	"VFire",	903,	903,	15,	VULN_FIRE	},
    {	"Natural Vulnerability to Cold",
    	"VCold",	904,	904,	15,	VULN_COLD	},
    {	"Natural Vulnerability to Electricity",
    	"VElectric",	905,	905,	15,	VULN_ELECTRICITY},
    {	"Natural Vulnerability to Acid",
    	"VAcid",	906,	906,	15,	VULN_ACID	},
    {	"Natural Vulnerability to Poison",
    	"VPoison",	907,	907,	30,	VULN_POISON	},
    {	"Natural Vulnerability to Disease",
    	"VDisease",	908,	908,	30,	VULN_DISEASE	},
    {	"Natural Vulnerability to Negative",
    	"VNegative",	909,	909,	30,	VULN_NEGATIVE	},
    {	"Natural Vulnerability to Holy",
    	"VHoly",	910,	910,	30,	VULN_HOLY	},
    {	"Natural Vulnerability to Energy",
    	"VEnergy",	911,	911,	30,	VULN_ENERGY	},
    {	"Natural Vulnerability to Mental",
    	"VMental",	912,	912,	30,	VULN_MENTAL	},
    {	"Natural Vulnerability to Drowning",
    	"VDrowning",	913,	913,	30,	VULN_DROWNING	},
    {	"Natural Vulnerability to Light",
    	"VLight",	914,	914,	30,	VULN_LIGHT	},
    {	"Natural Vulnerability to Sound",
    	"VSound",	915,	915,	30,	VULN_SOUND	},

	/* Resistances (950-999) */

    {	"Natural Resistance to Slash",
    	"RSlash",	900,	950,	15,	RES_SLASH	},
    {	"Natural Resistance to Pierce",
    	"RPierce",	901,	951,	15,	RES_PIERCE	},
    {	"Natural Resistance to Blunt",
    	"RBlunt",	902,	952,	15,	RES_BASH	},
    {	"Natural Resistance to Fire",
    	"RFire",	903,	953,	15,	RES_FIRE	},
    {	"Natural Resistance to Cold",
    	"RCold",	904,	954,	15,	RES_COLD	},
    {	"Natural Resistance to Electricity",
    	"RElectric",	905,	955,	15,	RES_ELECTRICITY	},
    {	"Natural Resistance to Acid",
    	"RAcid",	906,	956,	15,	RES_ACID	},
    {	"Natural Resistance to Poison",
    	"RPoison",	907,	957,	30,	RES_POISON	},
    {	"Natural Resistance to Disease",
    	"RDisease",	908,	958,	30,	RES_DISEASE	},
    {	"Natural Resistance to Negative",
    	"RNegative",	909,	959,	30,	RES_NEGATIVE	},
    {	"Natural Resistance to Holy",
    	"RHoly",	910,	960,	30,	RES_HOLY	},
    {	"Natural Resistance to Energy",
    	"REnergy",	911,	961,	30,	RES_ENERGY	},
    {	"Natural Resistance to Mental",
    	"RMental",	912,	962,	30,	RES_MENTAL	},
    {	"Natural Resistance to Drowning",
    	"RDrowning",	913,	963,	30,	RES_DROWNING	},
    {	"Natural Resistance to Light",
    	"RLight",	914,	964,	30,	RES_LIGHT	},
    {	"Natural Resistance to Sound",
    	"RSound",	915,	965,	30,	RES_SOUND	}
};


/* for position */	/* shared with AEDIT */
const struct position_type position_table[] =
{
	{	"dead",			"dead"	},
	{	"mortally wounded",	"mort"	},
	{	"incapacitated",	"incap"	},
	{	"stunned",		"stun"	},
	{	"sleeping",		"sleep"	},
	{	"resting",		"rest"	},
	{	"sitting",		"sit"	},
	{	"fighting",		"fight"	},
	{	"standing",		"stand"	},
	{	NULL,			NULL	}
};


/* for sex */		/* shared with AEDIT */
const struct sex_type sex_table[] =
{
	{"none"	},	{"male"	},	{"female"},	{"either"},	{NULL}
};


/* for sizes */		/* shared with AEDIT */
const struct size_type size_table[] =
{
	{"tiny"},	{"small"},	{"medium"},	{"large"},
	{"huge"},	{"giant"},	{NULL}
};


/* Below is stuff that is mostly used for the flag related commands */

const char *field_cand[] =
{
	"mobiles and players",
	"mobiles only",
	"players only",
	"objects",
	"rooms"
};

/* Organization of field types for the flag commands.  Note:  you must keep these
   in order with the FIELD definitions in tables.h!  This table will also see use
   in 'consider' and 'stat', to determine who can see what flag fields. */
const struct field_type flag_fields[] =
{
	/* name			flag_table	cand		see_mob	see_plr	mod_mob	mod_plr */
	{ "player",		plr_flags,	CAND_PLAYER,	999,	IMM,	999,	HED	},
	{ "pcdata",		pcdata_flags,	CAND_PLAYER,	999,	IMM,	999,	HED	},
	{ "cgroup",		cgroup_flags,	CAND_PLAYER,	999,	HED,	999,	IMP	},
	{ "wiznet",		wiz_flags,	CAND_PLAYER,	999,	HED,	999,	IMP	},
	{ "revoke",		revoke_flags,	CAND_CHAR,	IMM,	IMM,	IMM,	HED	},
	{ "censor",		censor_flags,	CAND_CHAR,	IMM,	IMM,	IMM,	IMP	},
	{ "affect",		affect_flags,	CAND_CHAR,	MTL,	MTL,	IMM,	HED	},
	{ "drains",		imm_flags,	CAND_CHAR,	MTL,	MTL,	IMM,	HED	},
	{ "immunities",		imm_flags,	CAND_CHAR,	MTL,	MTL,	IMM,	HED	},
	{ "resistances",	imm_flags,	CAND_CHAR,	MTL,	MTL,	IMM,	HED	},
	{ "vulnerabilites",	imm_flags,	CAND_CHAR,	MTL,	MTL,	IMM,	HED	},
	{ "communications",	comm_flags,	CAND_CHAR,	IMM,	IMM,	IMM,	HED	},
	{ "act",		act_flags,	CAND_MOB,	MTL,	999,	IMM,	999	},
	{ "offensive",		off_flags,	CAND_MOB,	MTL,	999,	IMM,	999	},
	{ "form",		form_flags,	CAND_MOB,	MTL,	MTL,	IMM,	999	},
	{ "parts",		part_flags,	CAND_MOB,	MTL,	MTL,	IMM,	999	},
	{ "extra",		extra_flags,	CAND_OBJ,	MTL,	999,	IMM,	999	},
	{ "wear",		wear_flags,	CAND_OBJ,	MTL,	999,	IMM,	999	},
	{ "weapon",		weapon_flags,	CAND_OBJ,	MTL,	999,	IMM,	999	},
	{ "room",		room_flags,	CAND_ROOM,	IMM,	999,	IMM,	999	},
	{ NULL,			NULL,		0,		999,	999,	999,	999	}
};


const struct ftoa_type ftoa_table[] =
{
	{ "A", A },	{ "B", B },	{ "C", C },	{ "D", D },	{ "E", E },
	{ "F", F },	{ "G", G },	{ "H", H },	{ "I", I },	{ "J", J },
	{ "K", K },	{ "L", L },	{ "M", M },	{ "N", N },	{ "O", O },
	{ "P", P },	{ "Q", Q },	{ "R", R },	{ "S", S },	{ "T", T },
	{ "U", U },	{ "V", V },	{ "W", W },	{ "X", X },	{ "Y", Y },
	{ "Z", Z },	{ "aa", aa },	{ "bb", bb },	{ "cc", cc },	{ "dd", dd },
	{ "ee", ee },	{ NULL, 0 }
};

const struct sector_type sector_table [] = 
{
	{	"indoors",	SECT_INSIDE		},
	{	"city",		SECT_CITY		},
	{	"field",	SECT_FIELD		},
	{	"forest",	SECT_FOREST		},
	{	"hills",	SECT_HILLS		},
	{	"mountain",	SECT_MOUNTAIN	},
	{	"water",	SECT_WATER_SWIM	},
	{	"ocean",	SECT_WATER_NOSWIM	},
	{	"unused",	SECT_UNUSED		},
	{	"air",		SECT_AIR		},
	{	"desert",	SECT_DESERT		},
	{	"MAX",		SECT_MAX		},
	{	"arena",	SECT_ARENA		},
	{	"clan arena",	SECT_CLANARENA	},
	{	NULL,		SECT_NULL		}
};

/* various flag tables */
const struct flag_type act_flags[] =
{
    {   "npc",                  A,      FALSE   },
    {   "sentinel",             B,      TRUE    },
    {   "scavenger",            C,      TRUE    },
    {   "morph",                E,      FALSE   },
    {   "aggressive",           F,      TRUE    },
    {   "stay_area",            G,      TRUE    },
    {   "wimpy",                H,      TRUE    },
    {   "pet",                  I,      TRUE    },
    {   "train",                J,      TRUE    },
    {   "practice",             K,      TRUE    },
    {   "stay",                 L,      TRUE    },
    {   "undead",               O,      TRUE    },
    {   "cleric",               Q,      TRUE    },
    {   "mage",                 R,      TRUE    },
    {   "thief",                S,      TRUE    },
    {   "warrior",              T,      TRUE    },
    {   "noalign",              U,      TRUE    },
    {   "nopurge",              V,      TRUE    },
    {   "outdoors",             W,      TRUE    },
    {   "indoors",              Y,      TRUE    },
    {   "nopush",               Z,      TRUE    },
    {   "healer",               aa,     TRUE    },
    {   "gain",                 bb,     TRUE    },
    {   "changer",              dd,     TRUE    },
    {   NULL,                   0,      FALSE   }
};

const struct flag_type wear_flags[] =
{
    {   "take",                 A,      TRUE    },
    {   "finger",               B,      TRUE    },
    {   "neck",                 C,      TRUE    },
    {   "body",                 D,      TRUE    },
    {   "head",                 E,      TRUE    },
    {   "legs",                 F,      TRUE    },
    {   "feet",                 G,      TRUE    },
    {   "hands",                H,      TRUE    },
    {   "arms",                 I,      TRUE    },
    {   "shield",               J,      TRUE    },
    {   "body",                 K,      TRUE    },
    {   "waist",                L,      TRUE    },
    {   "wrist",                M,      TRUE    },
    {   "wield",                N,      TRUE    },
    {   "hold",                 O,      TRUE    },
    {   "nosac",                P,      TRUE    },
    {   "floating",             Q,      TRUE    },
    {   "weddingring",		R,	TRUE	},
    {   NULL,                   0,      FALSE   }
};

const struct flag_type plr_flags[] =
{
    {   "npc",                  A,      FALSE   },
    {   "lookinpit",            B,      TRUE    },
    {   "autoassist",           C,      TRUE    },
    {   "autoexit",             D,      TRUE    },
    {   "autoloot",             E,      TRUE    },
    {   "autosac",              F,      TRUE    },
    {   "autogold",             G,      TRUE    },
    {   "autosplit",            H,      TRUE    },
    {   "showdefense",          I,      TRUE    },
    {   "autorecall",           J,      TRUE    },
    {   "questor",              K,      TRUE    },
    {   "crazycolor",           L,      TRUE    },
    {   "vt100",                M,      TRUE    },
    {   "makebag",              O,      TRUE    },
    {   "can_loot",             P,      TRUE    },
    {   "nosummon",             Q,      TRUE    },
    {   "nofollow",             R,      FALSE   },
    {   "color",                S,      TRUE    },
    {   "permit",               U,      TRUE    },
    {   "closedlocker",         V,      TRUE    },
    {   "log",                  W,      TRUE    },
    {   "freeze",               Y,      TRUE    },
    {   "thief",                Z,      TRUE    },
    {   "killer",               aa,     TRUE    },
    {   "superwiz",             bb,     FALSE   },
    {   "autotick",             cc,     TRUE    },
    {   "changer",              dd,     TRUE    },
    {   "leader",               ee,     TRUE    },
    {   NULL,                   0,      0       }
};

const struct flag_type pcdata_flags[] =
{
    {   "ooc",                  A,      TRUE    },
    {   "chatmode",             B,      TRUE    },
    {   "private",              C,      TRUE    },
    {   "stopcrash",            D,      TRUE    },
    {   "pk",                   E,      TRUE    },
    {   "remort",               F,      TRUE    },
    {   "showemail",            G,      TRUE    },
    {   "linkdead",             H,      TRUE    },
    {   "paintball",            I,      TRUE    },
    {   "snoopproof",           J,      TRUE    },
    {   "showlogin",            K,      TRUE    },
    {   "autopeek",             M,      TRUE    },
    {   "heedname",             N,      TRUE    },
    {   "showlost",             O,      TRUE    },
    {   "extraskill",           P,      TRUE    },
    {   "showraff",             Q,      TRUE    },
    {	"married",		R,	TRUE	},
    {   "squestor",		S,	TRUE	},
    {   NULL,                   0,      0       }
};

const struct flag_type wiz_flags[] =
{
    {   "on",                   A,      TRUE    },
    {   "ticks",                B,      TRUE    },
    {   "logins",               C,      TRUE    },
    {   "sites",                D,      TRUE    },
    {   "links",                E,      TRUE    },
    {   "deaths",               F,      TRUE    },
    {   "resets",               G,      TRUE    },
    {   "mobdeaths",            H,      TRUE    },
    {   "flags",                I,      TRUE    },
    {   "penalties",            J,      TRUE    },
    {   "saccing",              K,      TRUE    },
    {   "levels",               L,      TRUE    },
    {   "secure",               M,      TRUE    },
    {   "switches",             N,      TRUE    },
    {   "snoops",               O,      TRUE    },
    {   "restore",              P,      TRUE    },
    {   "load",                 Q,      TRUE    },
    {   "newbie",               R,      TRUE    },
    {   "prefix",               S,      TRUE    },
    {   "spam",                 T,      TRUE    },
    {   "misc",                 U,      TRUE    },
    {   "assist",               Y,      TRUE    },
    {   "cheat",                W,      TRUE    },
    {   "mail",                 aa,     TRUE    },
    {   "auction",              bb,     TRUE    },
    {   "quest",                cc,     TRUE    },
    {   "malloc",               dd,     TRUE    },
    {   "bugs",                 V,      TRUE    },
    {   NULL,                   0,      0       }
};

const struct flag_type extra_flags[] =
{
    {   "glowing",              A,      TRUE    },
    {   "humming",              B,      TRUE    },
    {   "compartment",		C,	TRUE	},
    {   "transparent",          D,      TRUE    },
    {   "evil",                 E,      TRUE    },
    {   "invis",                F,      TRUE    },
    {   "magic",                G,      TRUE    },
    {   "nodrop",               H,      TRUE    },
    {   "bless",                I,      TRUE    },
    {   "antigood",             J,      TRUE    },
    {   "antievil",             K,      TRUE    },
    {   "antineutral",          L,      TRUE    },
    {   "noremove",             M,      TRUE    },
    {   "inventory",            N,      TRUE    },
    {   "nopurge",              O,      TRUE    },
    {   "rotdeath",             P,      TRUE    },
    {   "visdeath",             Q,      TRUE    },
    {   "nosac",                R,      TRUE    },
    {   "nonmetal",             S,      TRUE    },
    {   "nolocate",             T,      TRUE    },
    {   "meltdrop",             U,      TRUE    },
    {   "hadtimer",             V,      TRUE    },
    {   "sellextract",          W,      TRUE    },
    {   "burnproof",            Y,      TRUE    },
    {   "nouncurse",            Z,      TRUE    },
    {   NULL,                   0,      0       }
};

const struct flag_type room_flags [] =
{
	{	"dark",		A,	TRUE	},
	{	"nolight",	B,	TRUE	},
	{	"nomob",	C,	TRUE	},
	{	"indoors",	D,	TRUE	},
	{	"locker",	E,	TRUE	},
	{	"femaleonly",	F,	TRUE	},
	{	"maleonly",	G,	TRUE	},
	{	"nosleep",	H,	TRUE	},
	{	"novision",	I,	TRUE	},
	{	"private",	J,	TRUE	},
	{	"safe",		K,	TRUE	},
	{	"solitary",	L,	TRUE	},
	{	"petshop",	M,	FALSE	},
	{	"norecall",	N,	TRUE	},
	{	"imponly",	O,	TRUE	},
	{	"godsonly",	P,	TRUE	},
	{	"heroesonly",	Q,	TRUE	},
	{	"newbiesonly",	R,	TRUE	},
	{	"law",		S,	TRUE	},
	{	"nowhere",	T,	TRUE	},
	{	"bank",		U,	TRUE	},
	{	"leaderonly",	V,	TRUE	},
	{	"teleport",	W,	TRUE	},
	{	"underwater",	X,	TRUE	},
	{	"noportal",	Y,	TRUE	},
	{	"remortsonly",	Z,	TRUE	},
	{	"noquest",	aa,	TRUE	},
	{	"silent",	bb,	TRUE	},
	{	NULL,		0,	0	}
};

const struct flag_type affect_flags[] =
{
    {   "blind",                A,      TRUE    },
    {   "invisible",            B,      TRUE    },
    {   "detect_evil",          C,      TRUE    },
    {   "detect_invis",         D,      TRUE    },
    {   "detect_magic",         E,      TRUE    },
    {   "detect_hidden",        F,      TRUE    },
    {   "detect_good",          G,      TRUE    },
    {   "sanctuary",            H,      TRUE    },
    {   "faerie_fire",          I,      TRUE    },
    {   "infrared",             J,      TRUE    },
    {   "curse",                K,      TRUE    },
    {   "fear",                 L,      TRUE    },
    {   "poison",               M,      TRUE    },
    {   "protect_evil",         N,      TRUE    },
    {   "protect_good",         O,      TRUE    },
    {   "sneak",                P,      TRUE    },
    {   "hide",                 Q,      TRUE    },
    {   "night_vision",         R,      TRUE    },
    {   "charm",                S,      TRUE    },
    {   "flying",               T,      TRUE    },
    {   "pass_door",            U,      TRUE    },
    {   "haste",                V,      TRUE    },
    {   "calm",                 W,      TRUE    },
    {   "plague",               X,      TRUE    },
    {   "steel_mist",           Y,      TRUE    },
    {   "divineregen",          Z,      TRUE    },
    {   "berserk",              aa,     TRUE    },
    {   "flameshield",          bb,     TRUE    },
    {   "regeneration",         cc,     TRUE    },
    {   "slow",                 dd,     TRUE    },
    {   "talon",                ee,     TRUE    },
    {   NULL,                   0,      0       }
};

const struct flag_type off_flags[] =
{
    {   "area_attack",          A,      TRUE    },
    {   "backstab",             B,      TRUE    },
    {   "bash",                 C,      TRUE    },
    {   "berserk",              D,      TRUE    },
    {   "disarm",               E,      TRUE    },
    {   "dodge",                F,      TRUE    },
    {   "fast",                 H,      TRUE    },
    {   "kick",                 I,      TRUE    },
    {   "dirt_kick",            J,      TRUE    },
    {   "parry",                K,      TRUE    },
    {   "rescue",               L,      TRUE    },
    {   "trip",                 N,      TRUE    },
    {   "crush",                O,      TRUE    },
    {   "assist_all",           P,      TRUE    },
    {   "assist_align",         Q,      TRUE    },
    {   "assist_race",          R,      TRUE    },
    {   "assist_players",       S,      TRUE    },
    {   "assist_guard",         T,      TRUE    },
    {   "assist_vnum",          U,      TRUE    },
    {   NULL,                   0,      0       }
};

const struct flag_type imm_flags[] =
{
	{	"summon",	IMM_SUMMON,	TRUE	},
	{	"charm",	IMM_CHARM,	TRUE	},
	{	"magic",	IMM_MAGIC,	TRUE	},
	{	"weapon",	IMM_WEAPON,	TRUE	},
	{	"bash",		IMM_BASH,	TRUE	},
	{	"pierce",	IMM_PIERCE,	TRUE	},
	{	"slash",	IMM_SLASH,	TRUE	},
	{	"fire",		IMM_FIRE,	TRUE	},
	{	"cold",		IMM_COLD,	TRUE	},
	{	"electricity",	IMM_ELECTRICITY,TRUE	},
	{	"acid",		IMM_ACID,	TRUE	},
	{	"poison",	IMM_POISON,	TRUE	},
	{	"negative",	IMM_NEGATIVE,	TRUE	},
	{	"holy",		IMM_HOLY,	TRUE	},
	{	"energy",	IMM_ENERGY,	TRUE	},
	{	"mental",	IMM_MENTAL,	TRUE	},
	{	"disease",	IMM_DISEASE,	TRUE	},
	{	"drowning",	IMM_DROWNING,	TRUE	},
	{	"light",	IMM_LIGHT,	TRUE	},
	{	"sound",	IMM_SOUND,	TRUE	},
	{	"shadowform",	IMM_SHADOW,	TRUE	},
	{	"wood",		IMM_WOOD,	TRUE	},
	{	"silver",	IMM_SILVER,	TRUE	},
	{	"iron",		IMM_IRON,	TRUE	},
	{	NULL,		0,		0	}
};

const struct flag_type form_flags[] =
{
    {   "edible",               FORM_EDIBLE,            TRUE    },
    {   "poison",               FORM_POISON,            TRUE    },
    {   "magical",              FORM_MAGICAL,           TRUE    },
    {   "instant_decay",        FORM_INSTANT_DECAY,     TRUE    },
    {   "other",                FORM_OTHER,             TRUE    },
    {   "animal",               FORM_ANIMAL,            TRUE    },
    {   "sentient",             FORM_SENTIENT,          TRUE    },
    {   "undead",               FORM_UNDEAD,            TRUE    },
    {   "construct",            FORM_CONSTRUCT,         TRUE    },
    {   "mist",                 FORM_MIST,              TRUE    },
    {   "intangible",           FORM_INTANGIBLE,        TRUE    },
    {   "biped",                FORM_BIPED,             TRUE    },
    {   "centaur",              FORM_CENTAUR,           TRUE    },
    {   "insect",               FORM_INSECT,            TRUE    },
    {   "spider",               FORM_SPIDER,            TRUE    },
    {   "crustacean",           FORM_CRUSTACEAN,        TRUE    },
    {   "worm",                 FORM_WORM,              TRUE    },
    {   "blob",                 FORM_BLOB,              TRUE    },
    {   "mammal",               FORM_MAMMAL,            TRUE    },
    {   "bird",                 FORM_BIRD,              TRUE    },
    {   "reptile",              FORM_REPTILE,           TRUE    },
    {   "snake",                FORM_SNAKE,             TRUE    },
    {   "dragon",               FORM_DRAGON,            TRUE    },
    {   "amphibian",            FORM_AMPHIBIAN,         TRUE    },
    {   "fish",                 FORM_FISH ,             TRUE    },
    {   "cold_blood",           FORM_COLD_BLOOD,        TRUE    },
    {   NULL,                   0,                      0       }
};

const struct flag_type part_flags[] =
{
    {   "head",                 PART_HEAD,              TRUE    },
    {   "arms",                 PART_ARMS,              TRUE    },
    {   "legs",                 PART_LEGS,              TRUE    },
    {   "heart",                PART_HEART,             TRUE    },
    {   "brains",               PART_BRAINS,            TRUE    },
    {   "guts",                 PART_GUTS,              TRUE    },
    {   "hands",                PART_HANDS,             TRUE    },
    {   "feet",                 PART_FEET,              TRUE    },
    {   "fingers",              PART_FINGERS,           TRUE    },
    {   "ear",                  PART_EAR,               TRUE    },
    {   "eye",                  PART_EYE,               TRUE    },
    {   "long_tongue",          PART_LONG_TONGUE,       TRUE    },
    {   "eyestalks",            PART_EYESTALKS,         TRUE    },
    {   "tentacles",            PART_TENTACLES,         TRUE    },
    {   "fins",                 PART_FINS,              TRUE    },
    {   "wings",                PART_WINGS,             TRUE    },
    {   "tail",                 PART_TAIL,              TRUE    },
    {   "claws",                PART_CLAWS,             TRUE    },
    {   "fangs",                PART_FANGS,             TRUE    },
    {   "horns",                PART_HORNS,             TRUE    },
    {   "scales",               PART_SCALES,            TRUE    },
    {   "tusks",                PART_TUSKS,             TRUE    },
    {   NULL,                   0,                      0       }
};

const struct flag_type comm_flags[] =
{
    {   "quiet",                COMM_QUIET,             TRUE    },
    {   "deaf",                 COMM_DEAF,              TRUE    },
    {   "nowiz",                COMM_NOWIZ,             TRUE    },
    {   "nopray",               COMM_NOPRAY,            TRUE    },
    {   "rpray",                COMM_RPRAY,             TRUE    },
    {   "noauction",            COMM_NOAUCTION,         TRUE    },
    {   "nogossip",             COMM_NOGOSSIP,          TRUE    },
    {   "noflame",              COMM_NOFLAME,           TRUE    },
    {   "noquestion",           COMM_NOQUESTION,        TRUE    },
    {   "nomusic",              COMM_NOMUSIC,           TRUE    },
    {   "noclan",               COMM_NOCLAN,            TRUE    },
    {   "compact",              COMM_COMPACT,           TRUE    },
    {   "brief",                COMM_BRIEF,             TRUE    },
    {   "prompt",               COMM_PROMPT,            TRUE    },
    {   "combine",              COMM_COMBINE,           TRUE    },
    {   "show_affects",         COMM_SHOW_AFFECTS,      TRUE    },
    {   "nograts",              COMM_NOGRATS,           TRUE    },
    {   "noannounce",           COMM_NOANNOUNCE,        TRUE    },
    {   "nosocial",             COMM_NOSOCIAL,          TRUE    },
    {   "noquery",              COMM_NOQUERY,           TRUE    },
    {   "noqwest",              COMM_NOQWEST,           TRUE    },
    {   "nopage",               COMM_NOPAGE,            TRUE    },
    {   "nochannels",           COMM_NOCHANNELS,        TRUE    },
    {   "afk",                  COMM_AFK,               TRUE    },
    {   "atbprompt",            COMM_ATBPROMPT,         TRUE    },
    {   NULL,                   0,                      0       }
};

const struct flag_type censor_flags[] =
{
	{	"channels",		CENSOR_CHAN,		TRUE	},
	{	"xsocials",		CENSOR_XSOC,		TRUE	},
	{	"spam",			CENSOR_SPAM,		TRUE	},
	{	NULL,			0,			0,	}
};

const struct flag_type revoke_flags[] =
{
    {   "nochannels",          REVOKE_NOCHANNELS,       TRUE    },
    {   "flame_only",          REVOKE_FLAMEONLY,        TRUE    },
    {   "pray",                REVOKE_PRAY,             TRUE    },
    {   "gossip",              REVOKE_GOSSIP,           TRUE    },
    {   "flame",               REVOKE_FLAME,            TRUE    },
    {   "qwest",               REVOKE_QWEST,            TRUE    },
    {   "auction",             REVOKE_AUCTION,          TRUE    },
    {   "clan",                REVOKE_CLAN,             TRUE    },
    {   "ic",                  REVOKE_IC,               TRUE    },
    {   "music",               REVOKE_MUSIC,            TRUE    },
    {   "q/a",                 REVOKE_QA,               TRUE    },
    {   "social",              REVOKE_SOCIAL,           TRUE    },
    {   "grats",               REVOKE_GRATS,            TRUE    },
    {   "page",                REVOKE_PAGE,             TRUE    },
    {   "qtell",               REVOKE_QTELL,            TRUE    },
    {   NULL,                  0,                       0       },
};

const struct flag_type weapon_flags[] =
{
    {   "flaming",              WEAPON_FLAMING,      TRUE    },
    {   "frost",                WEAPON_FROST,      TRUE    },
    {   "vampiric",             WEAPON_VAMPIRIC,      TRUE    },
    {   "sharp",                WEAPON_SHARP,      TRUE    },
    {   "vorpal",               WEAPON_VORPAL,      TRUE    },
    {   "twohanded",            WEAPON_TWO_HANDS,      TRUE    },
    {   "shocking",             WEAPON_SHOCKING,      TRUE    },
    {   "poison",               WEAPON_POISON,      TRUE    },
    {   NULL,                   0,      0       }
};


const struct color_type	color_table[] =
{
	{	"black",	"{k",	GREY,	NOBOLD	},
	{	"charcoal",	"{c",	GREY,	BOLD	},
	{	"red",		"{R",	RED,	NOBOLD	},
	{	"pink",		"{P",	RED,	BOLD	},
	{	"hunter",	"{H",	GREEN,	NOBOLD	},
	{	"green",	"{G",	GREEN,	BOLD	},
	{	"brown",	"{b",	YELLOW,	NOBOLD	},
	{	"yellow",	"{Y",	YELLOW,	BOLD	},
	{	"navy",		"{N",	BLUE,	NOBOLD	},
	{	"blue",		"{B",	BLUE,	BOLD	},
	{	"maroon",	"{M",	PURPLE,	NOBOLD	},
	{	"violet",	"{V",	PURPLE,	BOLD	},
	{	"turquoise",	"{T",	CYAN,	NOBOLD	},
	{	"cyan",		"{C",	CYAN,	BOLD	},
	{	"grey",		"{g",	WHITE,	NOBOLD	},
	{	"white",	"{W",	WHITE,	BOLD	},
	{	NULL,		NULL,	0,	0	}
};


/* default color settings table -- Montrey */
/* sections must be separated by a NULL line, entries must match the
   definitions in merc.h for CSLOTs */
const struct csetting_type csetting_table[] =
{
//		color	bold	name
	{	GREEN,	NOBOLD, "Gossip"	},	/*  0 */
	{	RED,	BOLD,	"Flame"		},
	{	YELLOW,	BOLD,	"Qwest"		},
	{	RED,	NOBOLD,	"Pray"		},
	{	YELLOW, NOBOLD,	"Auction"	},
	{	WHITE,	BOLD,	"Announcements"	},	/*  5 */
	{	BLUE,	BOLD,	"Music"		},
	{	CYAN,	BOLD,	"QA"		},
	{	GREEN,	NOBOLD,	"Socials"	},
	{	PURPLE,	BOLD,	"Grats"		},
	{	RED,	BOLD,	"Immtalk"	},	/* 10 */
	{	RED,	NOBOLD,	"Page"		},
	{	CYAN,	BOLD,	"Tell"		},
	{	RED,	NOBOLD,	"Clantalk"	},
	{	WHITE,	BOLD,	"IC"		},
	{	WHITE,	NOBOLD,	"Say"		},	/* 15 */
	{	PURPLE,	BOLD,	"GTell"		},
	{	PURPLE,	BOLD,	"QTell"		},
	{	YELLOW,	BOLD,	"Spousetalk"	},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},	/* 20 */
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},	/* 25 */
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	CYAN,	BOLD,	"Name"		},	/* 30 */
	{	PURPLE,	NOBOLD,	"PKills"	},
	{	PURPLE,	BOLD,	"AKills"	},
	{	CYAN,	NOBOLD,	"Class"		},
	{	RED,	BOLD,	"Stats"		},
	{	BLUE,	BOLD,	"Armor"		},	/* 35 */
	{	YELLOW,	BOLD,	"Points"	},
	{	GREEN,	NOBOLD,	"Weight"	},
	{	YELLOW,	NOBOLD,	"Gain"		},
	{	WHITE,	BOLD,	"Alignment"	},
	{	RED,	NOBOLD,	"Money"		},	/* 40 */
	{	GREEN,	BOLD,	"Diceroll"	},
	{	YELLOW,	BOLD,	"QP"		},
	{	GREEN,	BOLD,	"SP"		},
	{	PURPLE,	BOLD,	"RPP"		},
	{	0,	0,	NULL		},	/* 45 */
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	YELLOW,	NOBOLD,	"Torch"		},	/* 50 */
	{	RED,	BOLD,	"Flame"		},
	{	YELLOW,	NOBOLD,	"Border"	},
	{	CYAN,	BOLD,	"Name"		},
	{	WHITE,	BOLD,	"Title"		},
	{	CYAN,	NOBOLD,	"Class"		},	/* 55 */
	{	CYAN,	NOBOLD,	"Level"		},
	{	RED,	BOLD,	"Stats"		},
	{	RED,	NOBOLD,	"Trained"	},
	{	GREY,	BOLD,	"Healthname"	},
	{	WHITE,	NOBOLD,	"Healthnum"	},	/* 60 */
	{	CYAN,	BOLD,	"Money"		},
	{	GREEN,	NOBOLD,	"Weight"	},
	{	PURPLE,	BOLD,	"Armor"		},
	{	GREEN,	BOLD,	"Dicename"	},
	{	YELLOW,	BOLD,	"Dicenum"	},	/* 65 */
	{	BLUE,	BOLD,	"Pointname"	},
	{	CYAN,	BOLD,	"Pointnum"	},
	{	GREY,	BOLD,	"Alignment"	},
	{	WHITE,	BOLD,	"XPName"	},
	{	GREY,	BOLD,	"XPNum"		},	/* 70 */
	{	GREEN,	NOBOLD,	"Position"	},
	{	RED,	NOBOLD,	"PKRecord"	},
	{	WHITE,	BOLD,	"PKRank"	},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},	/* 75 */
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	RED,	BOLD,	"Rooms"		},	/* 80 */
	{	GREEN,	BOLD,	"Mobiles"	},
	{	YELLOW,	NOBOLD,	"Objects"	},
	{	PURPLE,	BOLD,	"Players"	},
	{	YELLOW, NOBOLD,	"Inventory"	},
	{	0,	0,	NULL		},	/* 85 */
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},	/* 90 */
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},	/* 95 */
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		},
	{	0,	0,	NULL		}	/* MAX_COLOR */
};
