/*************************************************
*                                                *
*               The Age of Legacy                *
*                                                *
* Based originally on ROM 2.4, tested, enhanced, *
* and maintained by the Legacy Team.  If that    *
* doesn't mean you, and you're stealing our      *
* code, at least tell us and boost our egos ;)   *
*************************************************/

#include "tables.hh"

#include <vector>

#include "Flags.hh"
#include "merc.hh"
#include "String.hh"

/* random name generation tables, male and female syllable sets 1, 2, 3 */
const String Msyl1[] = {
	"A",    "Ab",   "Ac",   "Ad",   "Af",   "Agr",  "Ast",  "As",   "Al",   "Adw",
	"Adr",  "Ar",   "B",    "Br",   "C",    "C",    "C",    "Cr",   "Ch",   "Cad",
	"D",    "Dr",   "Dw",   "Ed",   "Eth",  "Et",   "Er",   "El",   "Eow",  "F",
	"Fr",   "G",    "Gr",   "Gw",   "Gw",   "Gal",  "Gl",   "H",    "Ha",   "Ib",
	"J",    "Jer",  "K",    "Ka",   "Ked",  "L",    "Loth", "Lar",  "Leg",  "M",
	"Mir",  "N",    "Nyd",  "Ol",   "Oc",   "On",   "P",    "Pr",   "Q",    "R",
	"Rh",   "S",    "Sev",  "T",    "Tr",   "Th",   "Th",   "Ul",   "Um",   "Un",
	"V",    "Y",    "Yb",   "Z",    "W",    "W",    "Wic"
};

const String Msyl2[] = {
	"a",    "ae",   "ae",   "au",   "ao",   "are",  "ale",  "ale",  "ay",   "ardo",
	"e",    "edri", "ei",   "ea",   "ea",   "eri",  "era",  "ela",  "enda", "erra",
	"i",    "ia",   "ie",   "ira",  "ila",  "ili",  "ira",  "ire",  "igo",  "o",
	"oha",  "oma",  "oa",   "oe",   "oi",   "ore",  "u",    "y"
};

const String Msyl3[] = {
	"a",    "and",  "b",    "bwyn", "baen", "bard", "c",    "ch",   "can",  "d",
	"dan",  "don",  "der",  "dric", "dus",  "f",    "g",    "gord", "gan",  "han",
	"har",  "jar",  "jan",  "k",    "kin",  "kith", "kath", "koth", "kor",  "kon",
	"l",    "li",   "lin",  "lith", "lath", "loth", "ld",   "ldan", "m",    "mas",
	"mos",  "mar",  "mond", "n",    "nydd", "nidd", "nnon", "nwan", "nyth", "nad",
	"nn",   "nnor", "nd",   "p",    "r",    "red",  "ric",  "rid",  "rin",  "ron",
	"rd",   "s",    "sh",   "seth", "sean", "t",    "th",   "th",   "tha",  "tlan",
	"trem", "tram", "v",    "vudd", "w",    "wan",  "win",  "win",  "wyn",  "wyn",
	"wyr",  "wyr",  "wyth"
};

const String Fsyl1[] = {
	"A",    "Ab",   "Ac",   "Ad",   "Af",   "Agr",  "Ast",  "As",   "Al",   "Adw",
	"Adr",  "Ar",   "B",    "Br",   "C",    "C",    "C",    "Cr",   "Ch",   "Cad",
	"D",    "Dr",   "Dw",   "Ed",   "Eth",  "Et",   "Er",   "El",   "Eow",  "F",
	"Fr",   "G",    "Gr",   "Gw",   "Gw",   "Gal",  "Gl",   "H",    "Ha",   "Ib",
	"Jer",  "K",    "Ka",   "Ked",  "L",    "Loth", "Lar",  "Leg",  "M",    "Mir",
	"N",    "Nyd",  "Ol",   "Oc",   "On",   "P",    "Pr",   "Q",    "R",    "Rh",
	"S",    "Sev",  "T",    "Tr",   "Th",   "Th",   "Ul",   "Um",   "Un",   "V",
	"W",    "W",    "Wic",  "Y",    "Yb",   "Z"
};

const String Fsyl2[] = {
	"a",    "a",    "a",    "ae",   "ae",   "au",   "ao",   "are",  "ale",  "ali",
	"ay",   "ardo", "e",    "e",    "e",    "ei",   "ea",   "ea",   "eri",  "era",
	"ela",  "eli",  "enda", "erra", "i",    "i",    "i",    "ia",   "ie",   "ire",
	"ira",  "ila",  "ili",  "ira",  "igo",  "o",    "oa",   "oi",   "oe",   "ore",
	"u",    "y"
};

const String Fsyl3[] = {
	"beth", "cia",  "cien", "clya", "de",   "dia",  "dda",  "dien", "dith", "dia",
	"lind", "lith", "lia",  "lian", "lla",  "llan", "lle",  "ma",   "mma",  "mwen",
	"meth", "n",    "n",    "n",    "nna",  "ndra", "ng",   "ni",   "nia",  "niel",
	"rith", "rien", "ria",  "ri",   "rwen", "sa",   "sien", "ssa",  "ssi",  "swen",
	"thien", "thiel", "viel", "via",  "ven",  "veth", "wen",  "wen",  "wen",  "wen",
	"wia",  "weth", "wien", "wiel"
};

/* random mob title generation, based on basic class */
const std::vector<MagT_type> MagT_table = {
	{       "wizard",               "wizardess"             },
	{       "spellcaster",          "spellcaster"           },
	{       "prestidigitator",      "prestidigitator"       },
	{       "evoker",               "evoker"                },
	{       "conjuror",             "conjuress"             },
	{       "abjurer",              "abjuress"              },
	{       "summoner",             "summoner"              },
	{       "theurge",              "theurge"               },
	{       "thaumaturge",          "thaumaturge"           },
	{       "magician",             "witch"                 },
	{       "enchanter",            "enchantress"           },
	{       "warlock",              "war witch"             },
	{       "sorcerer",             "sorceress"             },
	{       "mage",                 "mage"                  },
	{       "scribe",               "scribess"              },
	{       "seer",                 "seeress"               },
	{       "sage",                 "sage"                  },
	{       "illusionist",          "illusionist"           },
	{       "invoker",              "invoker"               },
	{       "savant",               "savant"                },
	{       "magus",                "craftess"              },
};

const std::vector<CleT_type> CleT_table = {
	{       "cleric",               "cleric"                },
	{       "shaman",               "shaman"                },
	{       "acolyte",              "acolyte"               },
	{       "adept",                "adept"                 },
	{       "friar",                "friaress"              },
	{       "missionary",           "missionary"            },
	{       "deacon",               "deaconess"             },
	{       "vicar",                "vicaress"              },
	{       "priest",               "priestess"             },
	{       "curate",               "curess"                },
	{       "monk",                 "nun"                   },
	{       "healer",               "healess"               },
	{       "chaplain",             "chaplain"              },
	{       "expositor",            "expositress"           },
	{       "bishop",               "bishop"                },
	{       "patriarch",            "matriarch"             },
	{       "demon killer",         "demon killer"          },
	{       "evangelist",           "evangelist"            },
};

const std::vector<ThiT_type> ThiT_table = {
	{       "thief",                "thief"                 },
	{       "cutpurse",             "cutpurse"              },
	{       "treasure hunter",      "treasure hunter"       },
	{       "footpad",              "footpad"               },
	{       "larcenist",            "larcenist"             },
	{       "scout",                "scout"                 },
	{       "filcher",              "filcheress"            },
	{       "burglar",              "burglaress"            },
	{       "highwayman",           "highwaywoman"          },
	{       "robber",               "robber"                },
	{       "pickpocket",           "pickpocket"            },
	{       "sharper",              "sharpress"             },
	{       "rogue",                "rogue"                 },
	{       "magsman",              "magswoman"             },
	{       "knifer",               "knifer"                },
	{       "murderer",             "murderess"             },
	{       "quickblade",           "quickblade"            },
	{       "brigand",              "brigand"               },
	{       "bravo",                "bravo"                 },
	{       "cutthroat",            "cutthroat"             },
	{       "assassin",             "assassin"              },
	{       "thug",                 "thug"                  },
};

const std::vector<WarT_type> WarT_table = {
	{       "sentry",               "sentress"              },
	{       "fighter",              "fighter"               },
	{       "soldier",              "soldier"               },
	{       "warrior",              "warrior"               },
	{       "veteran",              "veteran"               },
	{       "swordsman",            "swordswoman"           },
	{       "fencer",               "fenceress"             },
	{       "hero",                 "heroine"               },
	{       "myrmidon",             "myrmidon"              },
	{       "swashbuckler",         "swashbuckleress"       },
	{       "mercenary",            "mercenaress"           },
	{       "swordmaster",          "swordmistress"         },
	{       "champion",             "champion"              },
	{       "cavalier",             "cavalier"              },
	{       "knight",               "lady knight"           },
	{       "paladin",              "paladin"               },
	{       "demon slayer",         "demon slayer"          },
	{       "weaponsmaster",        "weaponsmistress"       },
	{       "battledancer",         "battledancer"          },
	{       "barbarian",            "barbarian"             },
	{       "knight terminus",      "lady knight terminus"  },
	{       "rogue knight",         "rogue knight"          },
};

const std::vector<chan_type> chan_table = {
	{
		"gossip",       "You gossip",   "$n{x gossips '$t{x'",
		CSLOT_CHAN_GOSSIP,      COMM_NOGOSSIP,  REVOKE_GOSSIP
	},
	{
		"flame",        "You *FlAmE*",  "$n{x *FlAmEs* '$t{x'",
		CSLOT_CHAN_FLAME,       COMM_NOFLAME,   REVOKE_FLAME
	},
	{
		"qwest",        "You qwest",    "($n{x) QWEST: $t{x",
		CSLOT_CHAN_QWEST,       COMM_NOQWEST,   REVOKE_QWEST
	},
	{
		"pray",         "You implore the gods:",        "$n{x implores the gods: \"$t{x\"",
		CSLOT_CHAN_PRAY,        COMM_NOPRAY,    REVOKE_PRAY
	},
	{
		"clantalk",     "You clan",     "$n{x clans '$t{x'",
		CSLOT_CHAN_CLAN,        COMM_NOCLAN,    REVOKE_CLAN
	},
	{
		"music",        "You MUSIC:",   "$n{x MUSIC: '$t{x'",
		CSLOT_CHAN_MUSIC,       COMM_NOMUSIC,   REVOKE_MUSIC
	},
	{
		"IC",           "{G*{T({CIC{T){G*{x",   "{T({CIC{T) {G*{W$n{G*{x '$t{x'",
		CSLOT_CHAN_IC,          COMM_NOIC,      REVOKE_IC
	},
	{
		"grats",        "You grat",     "$n{x grats '$t{x'",
		CSLOT_CHAN_GRATS,       COMM_NOGRATS,   REVOKE_GRATS
	},
	{
		"immtalk",      "none",         "none",
		CSLOT_CHAN_IMM,         COMM_NOWIZ,     REVOKE_NULL
	},
	{
		"qa",           "You Q/A",      "[Q/A] $n{x '$t{x'",
		CSLOT_CHAN_QA,          COMM_NOQUESTION, REVOKE_QA
	},
	{
		"auction",		"",				"",
		CSLOT_CHAN_AUCTION,		COMM_NOAUCTION,	REVOKE_AUCTION
	},
};

const std::vector<revoke_type> revoke_table = {
	/* name, bit, message */
	{ "flameonly",  REVOKE_FLAMEONLY,       "ability to use channels other than FLAME"      },
	{ "nochannel",  REVOKE_NOCHANNELS,      "channel priviledges"                           },
	{ "pray",       REVOKE_PRAY,            "ability to PRAY"                               },
	{ "gossip",     REVOKE_GOSSIP,          "ability to GOSSIP"                             },
	{ "flame",      REVOKE_FLAME,           "ability to FLAME"                              },
	{ "qwest",      REVOKE_QWEST,           "ability to QWEST"                              },
	{ "auction",    REVOKE_AUCTION,         "ability to AUCTION"                            },
	{ "clantalk",   REVOKE_CLAN,            "ability to CLANTALK"                           },
	{ "music",      REVOKE_MUSIC,           "ability to MUSIC"                              },
	{ "question",   REVOKE_QA,              "ability to QUESTION and ANSWER"                },
	{ "answer",     REVOKE_QA,              "ability to QUESTION and ANSWER"                },
	{ "ic",         REVOKE_IC,              "ability to talk IN CHARACTER"                  },
	{ "social",     REVOKE_SOCIAL,          "ability to SOCIALize"                          },
	{ "grats",      REVOKE_GRATS,           "ability to offer GRATS"                        },
	{ "page",       REVOKE_PAGE,            "ability to PAGE people"                        },
	{ "qtell",      REVOKE_QTELL,           "ability to talk to your QUERY"                 },
	{ "say",        REVOKE_SAY,             "ability to SAY anything"                       },
	{ "tell",       REVOKE_TELL,            "ability to send TELLs"                         },
	{ "emote",      REVOKE_EMOTE,           "ability to EMOTE"                              },
	{ "exp",        REVOKE_EXP,             "ability to learn from your EXPeriences"        },
	{ "note",       REVOKE_NOTE,            "ability to write NOTEs"                        },
};

const std::vector<flag_type> cgroup_flags = {
	{       "General",      GROUP_GEN,              true    },
	{       "Quest",        GROUP_QUEST,    true    },
	{       "Building",     GROUP_BUILD,    true    },
	{       "Coder",        GROUP_CODE,             true    },
	{       "Security",     GROUP_SECURE,   true    },
	{       "Player",       GROUP_PLAYER,   true    },
	{       "Mobile",       GROUP_MOBILE,   true    },
	{       "Clan",         GROUP_CLAN,             true    },
	{       "Avatar",       GROUP_AVATAR,   true    },
	{       "Hero",         GROUP_HERO,             true    },
	{       "Deputy",       GROUP_DEPUTY,   true    },
	{       "Leader",       GROUP_LEADER,   true    },
};

/* wiznet table and prototype for future flag setting */
/* must be added in order of level for config to work */
const std::vector<wiznet_type> wiznet_table = {
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
};

const struct raffects raffects[MAX_RAFFECTS] = {
	/* id - raffect id #
	   group - group #.  code won't pick twice from the same group of affects.  raffects in 0 group are exempt
	   chance - percentage chance of getting that raffect
	   description - what shows up when remorting and in stat
	   added - added vuln/res in remorting

	        "description"
	        "short name"    group,  id,     chance, added

	*/
	{
		"NULL REMORT AFFECT",   /* Problems if someone gets this */
		"NULLRAFF",     0,      1,      100,    0
	},

	/* "Good" Remort Affects (2-99) */

	{
		"Light Feet",           /* Movement cost is halved */
		"lightfeet",    0,      2,      80,     0
	},
	{
		"Magic Sensitivity",    /* Casting lag reduced by 1/4 */
		"fastcast",     1,      3,      70,     0
	},
	{
		"Energy Absorption",    /* get back level/10+1 mana per combat round */
		"mageregen",    2,      4,      50,     0
	},
	{
		"Gift of the Troll",    /* get back level/20+1 hp per combat round */
		"vampregen",    2,      5,      50,     0
	},
	{
		"Careful Learner",      /* +10% exp per kill */
		"moreexp",      3,      6,      80,     0
	},
	{
		"Intimidating Presence",/* +5% damage */
		"moredam",      4,      7,      80,     0
	},
	{
		"Natural Channeling",   /* Mana cost reduced by 10% */
		"cheapspell",   5,      8,      70,     0
	},
	{
		"Immunity to Hunger",   /* No need to eat */
		"nohunger",     6,      9,      50,     0
	},
	{
		"Immunity to Thirst",   /* No need to drink */
		"nothirst",     7,      10,     50,     0
	},
	{
		"Favor of the Heavens", /* Chance of getting up to 4x experience after a fight */
		"favor",        8,      11,     50,     0
	},
	/*    { "Free Portals",         holding a warp stone for nexus/portal is unnecessary
	        "freeport",     0,      14,     70,     0               },
	    {   "Sixth Sense",          Chance of knowing when someone enters the room sneaking
	        "sixthsense",   0,      24,     70,     0               },
	    {   "Farsight",             Can see two rooms in all directions on scanning
	        "farsight",     0,      25,     70,     0               }, */

	/* "Bad" Remort Affects (100-199) */

	{
		"Poor Memory",          /* Recall has a chance of random teleport */
		"buggyrec",     0,      100,    60,     0
	},
	{
		"Leaden Feet",          /* 1/4 second lag on walking */
		"slowwalk",     0,      102,    80,     0
	},
	{
		"Clumsiness",           /* Randomly fall down in combat */
		"clumsy",       0,      103,    60,     0
	},
	{
		"Weak Grip",            /* Randomly drop your weapon in combat */
		"weakgrip",     0,      104,    50,     0
	},
	{
		"Magic Unresponsiveness",/* Casting lag increased by 1/4 */
		"slowcast",     1,      105,    70,     0
	},
	{
		"Oblivious Learner",    /* -10% exp per kill */
		"lessexp",      3,      106,    80,     0
	},
	{
		"Laughable Presence",   /* -5% damage */
		"lessdam",      4,      107,    80,     0
	},
	{
		"Poor Magic Channeling",/* Mana cost increased by 10% */
		"costspell",    5,      108,    70,     0
	},
	{
		"Fast Metabolism",      /* Get hungry twice as fast */
		"exhungry",     6,      109,    70,     0
	},
	{
		"Abnormal Dehydration", /* Get thirsty twice as fast */
		"exthirsty",    7,      110,    70,     0
	},
	{
		"Jest of the Heavens",  /* Chance of getting up to 1/4 experience after a fight */
		"jest",         8,      111,    50,     0
	},
	/*    { "Bad Direction Sense",  Randomly walk the wrong direction
	        "baddir",       0,      23,     50,     0               }, */

	/* Vulnerabilities (900-949) */

	{
		"Natural Vulnerability to Slash",
		"VSlash",       950,    900,    15,     VULN_SLASH
	},
	{
		"Natural Vulnerability to Pierce",
		"VPierce",      901,    901,    15,     VULN_PIERCE
	},
	{
		"Natural Vulnerability to Blunt",
		"VBlunt",       902,    902,    15,     VULN_BASH
	},
	{
		"Natural Vulnerability to Fire",
		"VFire",        903,    903,    15,     VULN_FIRE
	},
	{
		"Natural Vulnerability to Cold",
		"VCold",        904,    904,    15,     VULN_COLD
	},
	{
		"Natural Vulnerability to Electricity",
		"VElectric",    905,    905,    15,     VULN_ELECTRICITY
	},
	{
		"Natural Vulnerability to Acid",
		"VAcid",        906,    906,    15,     VULN_ACID
	},
	{
		"Natural Vulnerability to Poison",
		"VPoison",      907,    907,    30,     VULN_POISON
	},
	{
		"Natural Vulnerability to Disease",
		"VDisease",     908,    908,    30,     VULN_DISEASE
	},
	{
		"Natural Vulnerability to Negative",
		"VNegative",    909,    909,    30,     VULN_NEGATIVE
	},
	{
		"Natural Vulnerability to Holy",
		"VHoly",        910,    910,    30,     VULN_HOLY
	},
	{
		"Natural Vulnerability to Energy",
		"VEnergy",      911,    911,    30,     VULN_ENERGY
	},
	{
		"Natural Vulnerability to Mental",
		"VMental",      912,    912,    30,     VULN_MENTAL
	},
	{
		"Natural Vulnerability to Drowning",
		"VDrowning",    913,    913,    30,     VULN_DROWNING
	},
	{
		"Natural Vulnerability to Light",
		"VLight",       914,    914,    30,     VULN_LIGHT
	},
	{
		"Natural Vulnerability to Sound",
		"VSound",       915,    915,    30,     VULN_SOUND
	},

	/* Resistances (950-999) */

	{
		"Natural Resistance to Slash",
		"RSlash",       900,    950,    15,     RES_SLASH
	},
	{
		"Natural Resistance to Pierce",
		"RPierce",      901,    951,    15,     RES_PIERCE
	},
	{
		"Natural Resistance to Blunt",
		"RBlunt",       902,    952,    15,     RES_BASH
	},
	{
		"Natural Resistance to Fire",
		"RFire",        903,    953,    15,     RES_FIRE
	},
	{
		"Natural Resistance to Cold",
		"RCold",        904,    954,    15,     RES_COLD
	},
	{
		"Natural Resistance to Electricity",
		"RElectric",    905,    955,    15,     RES_ELECTRICITY
	},
	{
		"Natural Resistance to Acid",
		"RAcid",        906,    956,    15,     RES_ACID
	},
	{
		"Natural Resistance to Poison",
		"RPoison",      907,    957,    30,     RES_POISON
	},
	{
		"Natural Resistance to Disease",
		"RDisease",     908,    958,    30,     RES_DISEASE
	},
	{
		"Natural Resistance to Negative",
		"RNegative",    909,    959,    30,     RES_NEGATIVE
	},
	{
		"Natural Resistance to Holy",
		"RHoly",        910,    960,    30,     RES_HOLY
	},
	{
		"Natural Resistance to Energy",
		"REnergy",      911,    961,    30,     RES_ENERGY
	},
	{
		"Natural Resistance to Mental",
		"RMental",      912,    962,    30,     RES_MENTAL
	},
	{
		"Natural Resistance to Drowning",
		"RDrowning",    913,    963,    30,     RES_DROWNING
	},
	{
		"Natural Resistance to Light",
		"RLight",       914,    964,    30,     RES_LIGHT
	},
	{
		"Natural Resistance to Sound",
		"RSound",       915,    965,    30,     RES_SOUND
	}
};

/* for position */      /* shared with AEDIT */
const std::vector<position_type> position_table = {
	{       "dead",                 "dead"  },
	{       "mortally wounded",     "mort"  },
	{       "incapacitated",        "incap" },
	{       "stunned",              "stun"  },
	{       "sleeping",             "sleep" },
	{       "resting",              "rest"  },
	{       "sitting",              "sit"   },
	{       "fighting",             "fight" },
	{       "standing",             "stand" },
	{       "flying",               "fly"   },
};

/* for sex */           /* shared with AEDIT */
const std::vector<sex_type> sex_table = {
	{"none" },      {"male" },      {"female"},     {"either"},
};

/* for sizes */         /* shared with AEDIT */
const std::vector<size_type> size_table = {
	{"tiny"},       {"small"},      {"medium"},     {"large"},
	{"huge"},       {"giant"},
};

/* Below is stuff that is mostly used for the flag related commands */

const String field_cand[] = {
	"mobiles and players",
	"mobiles only",
	"players only",
	"objects",
	"rooms"
};

/* Organization of field types for the flag commands.  Note:  you must keep these
   in order with the FIELD definitions in tables.h!  This table will also see use
   in 'consider' and 'stat', to determine who can see what flag fields. */
const std::vector<field_type> flag_fields = {
	/* name                 flag_table      cand            see_mob see_plr mod_mob mod_plr */
	{ "player",             plr_flags,      CAND_PLAYER,    999,    IMM,    999,    HED     },
	{ "pcdata",             pcdata_flags,   CAND_PLAYER,    999,    IMM,    999,    HED     },
	{ "cgroup",             cgroup_flags,   CAND_PLAYER,    999,    HED,    999,    IMP     },
	{ "wiznet",             wiz_flags,      CAND_PLAYER,    999,    HED,    999,    IMP     },
	{ "revoke",             revoke_flags,   CAND_CHAR,      IMM,    IMM,    IMM,    HED     },
	{ "censor",             censor_flags,   CAND_CHAR,      IMM,    IMM,    IMM,    IMP     },
	{ "communications",     comm_flags,     CAND_CHAR,      IMM,    IMM,    IMM,    HED     },
	{ "act",                act_flags,      CAND_MOB,       MTL,    999,    IMM,    999     },
	{ "offensive",          off_flags,      CAND_MOB,       MTL,    999,    IMM,    999     },
	{ "form",               form_flags,     CAND_MOB,       MTL,    MTL,    IMM,    999     },
	{ "parts",              part_flags,     CAND_MOB,       MTL,    MTL,    IMM,    999     },
	{ "group",              group_flags,    CAND_MOB,       MTL,    999,    IMM,    999     },
	{ "extra",              extra_flags,    CAND_OBJ,       MTL,    999,    IMM,    999     },
//	{ "EQ",              	eq_flags,    	CAND_OBJ,       MTL,    999,    IMM,    999     },
	{ "wear",               wear_flags,     CAND_OBJ,       MTL,    999,    IMM,    999     },
//	{ "weapon",             weapon_flags,   CAND_OBJ,       MTL,    999,    IMM,    999     },
	{ "room",               room_flags,     CAND_ROOM,      IMM,    999,    IMM,    999     },
};

const std::vector<sector_type> sector_table = {
	{       "indoors",      Sector::inside             },
	{       "city",         Sector::city               },
	{       "road",         Sector::road               },
	{       "field",        Sector::field              },
	{       "sparse forest",Sector::forest_sparse      },
	{       "forest",       Sector::forest_medium      },
	{       "dense forest", Sector::forest_dense       },
	{       "hills",        Sector::hills              },
	{       "mountain",     Sector::mountain   },
	{       "water",        Sector::water_swim },
	{       "ocean",        Sector::water_noswim       },
	{       "air",          Sector::air                },
	{       "desert",       Sector::desert             },
	{       "arena",        Sector::arena              },
	{       "clan arena",   Sector::clanarena  },
	{       "area entrance",Sector::area_entrance      },
};

/* various flag tables */
const std::vector<flag_type> cont_flags = {
	{	"closable",			CONT_CLOSEABLE,	true	},
	{	"pickproof",		CONT_PICKPROOF,	true	},
	{	"closed",			CONT_CLOSED,	true	},
	{	"locked",			CONT_LOCKED,	true	},
};

const std::vector<flag_type> act_flags = {
	{   "nosummon",             Flags::A,      true    },
	{   "sentinel",             Flags::B,      true    },
	{   "scavenger",            Flags::C,      true    },
	{   "morph",                Flags::E,      false   },
	{   "aggressive",           Flags::F,      true    },
	{   "stay_area",            Flags::G,      true    },
	{   "wimpy",                Flags::H,      true    },
	{   "pet",                  Flags::I,      true    },
	{   "train",                Flags::J,      true    },
	{   "practice",             Flags::K,      true    },
	{   "stay",                 Flags::L,      true    },
	{   "undead",               Flags::O,      true    },
	{   "cleric",               Flags::Q,      true    },
	{   "mage",                 Flags::R,      true    },
	{   "thief",                Flags::S,      true    },
	{   "warrior",              Flags::T,      true    },
	{   "noalign",              Flags::U,      true    },
	{   "nopurge",              Flags::V,      true    },
	{   "outdoors",             Flags::W,      true    },
	{   "indoors",              Flags::Y,      true    },
	{   "nopush",               Flags::Z,      true    },
	{   "healer",               Flags::a,     true    },
	{   "gain",                 Flags::b,     true    },
	{   "changer",              Flags::d,     true    },
};

const std::vector<flag_type> wear_flags = {
	{   "take",                 Flags::A,      true    },
	{   "finger",               Flags::B,      true    },
	{   "neck",                 Flags::C,      true    },
	{   "torso",                Flags::D,      true    },
	{   "head",                 Flags::E,      true    },
	{   "legs",                 Flags::F,      true    },
	{   "feet",                 Flags::G,      true    },
	{   "hands",                Flags::H,      true    },
	{   "arms",                 Flags::I,      true    },
	{   "shield",               Flags::J,      true    },
	{   "body",                 Flags::K,      true    },
	{   "waist",                Flags::L,      true    },
	{   "wrist",                Flags::M,      true    },
	{   "wield",                Flags::N,      true    },
	{   "hold",                 Flags::O,      true    },
	{   "nosac",                Flags::P,      true    },
	{   "floating",             Flags::Q,      true    },
	{   "weddingring",          Flags::R,      true    },
};

const std::vector<flag_type> plr_flags = {
	{   "nosummon",             Flags::A,      true    },
	{   "lookinpit",            Flags::B,      true    },
	{   "autoassist",           Flags::C,      true    },
	{   "autoexit",             Flags::D,      true    },
	{   "autoloot",             Flags::E,      true    },
	{   "autosac",              Flags::F,      true    },
	{   "autogold",             Flags::G,      true    },
	{   "autosplit",            Flags::H,      true    },
	{   "showdefense",          Flags::I,      true    },
	{   "autorecall",           Flags::J,      true    },
	{   "questor",              Flags::K,      true    },
	{   "crazycolor",           Flags::L,      true    },
	{   "vt100",                Flags::M,      true    },
	{   "makebag",              Flags::O,      true    },
	{   "can_loot",             Flags::P,      true    },
	{   "none",                 Flags::Q,      false   },
	{   "nofollow",             Flags::R,      false   },
	{   "color",                Flags::S,      true    },
	{   "permit",               Flags::U,      true    },
	{   "closedlocker",         Flags::V,      true    },
	{   "log",                  Flags::W,      true    },
	{   "freeze",               Flags::Y,      true    },
	{   "thief",                Flags::Z,      true    },
	{   "killer",               Flags::a,     true    },
	{   "superwiz",             Flags::b,     false   },
	{   "autotick",             Flags::c,     true    },
	{   "changer",              Flags::d,     true    },
	{   "leader",               Flags::e,     true    },
};

const std::vector<flag_type> pcdata_flags = {
	{   "ooc",                  Flags::A,      true    },
	{   "chatmode",             Flags::B,      true    },
	{   "private",              Flags::C,      true    },
	{   "stopcrash",            Flags::D,      true    },
	{   "pk",                   Flags::E,      true    },
	{   "remort",               Flags::F,      true    },
	{   "showemail",            Flags::G,      true    },
	{   "linkdead",             Flags::H,      true    },
	{   "paintbow",             Flags::I,      true    },
	{   "snoopproof",           Flags::J,      true    },
	{   "showlogin",            Flags::K,      true    },
	{   "autopeek",             Flags::M,      true    },
	{   "heedname",             Flags::N,      true    },
	{   "showlost",             Flags::O,      true    },
	{   "extraskill",           Flags::P,      true    },
	{   "showraff",             Flags::Q,      true    },
	{   "married",              Flags::R,      true    },
	{   "squestor",             Flags::S,      true    },
};

const std::vector<flag_type> wiz_flags = {
	{   "on",                   Flags::A,      true    },
	{   "ticks",                Flags::B,      true    },
	{   "logins",               Flags::C,      true    },
	{   "sites",                Flags::D,      true    },
	{   "links",                Flags::E,      true    },
	{   "deaths",               Flags::F,      true    },
	{   "resets",               Flags::G,      true    },
	{   "mobdeaths",            Flags::H,      true    },
	{   "flags",                Flags::I,      true    },
	{   "penalties",            Flags::J,      true    },
	{   "saccing",              Flags::K,      true    },
	{   "levels",               Flags::L,      true    },
	{   "secure",               Flags::M,      true    },
	{   "switches",             Flags::N,      true    },
	{   "snoops",               Flags::O,      true    },
	{   "restore",              Flags::P,      true    },
	{   "load",                 Flags::Q,      true    },
	{   "newbie",               Flags::R,      true    },
	{   "prefix",               Flags::S,      true    },
	{   "spam",                 Flags::T,      true    },
	{   "misc",                 Flags::U,      true    },
	{   "assist",               Flags::Y,      true    },
	{   "cheat",                Flags::W,      true    },
	{   "mail",                 Flags::a,     true    },
	{   "auction",              Flags::b,     true    },
	{   "quest",                Flags::c,     true    },
	{   "malloc",               Flags::d,     true    },
	{   "bugs",                 Flags::V,      true    },
};

const std::vector<flag_type> extra_flags = {
	{   "glowing",              Flags::A,      true    },
	{   "humming",              Flags::B,      true    },
	{   "compartment",          Flags::C,      true    },
	{   "transparent",          Flags::D,      true    },
	{   "evil",                 Flags::E,      true    },
	{   "invis",                Flags::F,      true    },
	{   "magic",                Flags::G,      true    },
	{   "nodrop",               Flags::H,      true    },
	{   "bless",                Flags::I,      true    },
	{   "antigood",             Flags::J,      true    },
	{   "antievil",             Flags::K,      true    },
	{   "antineutral",          Flags::L,      true    },
	{   "noremove",             Flags::M,      true    },
	{   "inventory",            Flags::N,      true    },
	{   "nopurge",              Flags::O,      true    },
	{   "rotdeath",             Flags::P,      true    },
	{   "visdeath",             Flags::Q,      true    },
	{   "nosac",                Flags::R,      true    },
	{   "nonmetal",             Flags::S,      true    },
	{   "nolocate",             Flags::T,      true    },
	{   "meltdrop",             Flags::U,      true    },
	{   "hadtimer",             Flags::V,      true    },
	{   "sellextract",          Flags::W,      true    },
	{   "burnproof",            Flags::Y,      true    },
	{   "nouncurse",            Flags::Z,      true    },
};

const std::vector<flag_type> room_flags = {
	{       "dark",         Flags::A,      true    },
	{       "nolight",      Flags::B,      true    },
	{       "nomob",        Flags::C,      true    },
	{       "indoors",      Flags::D,      true    },
	{       "locker",       Flags::E,      true    },
	{       "femaleonly",   Flags::F,      true    },
	{       "maleonly",     Flags::G,      true    },
	{       "nosleep",      Flags::H,      true    },
	{       "novision",     Flags::I,      true    },
	{       "private",      Flags::J,      true    },
	{       "safe",         Flags::K,      true    },
	{       "solitary",     Flags::L,      true    },
	{       "petshop",      Flags::M,      false   },
	{       "norecall",     Flags::N,      true    },
	{       "imponly",      Flags::O,      true    },
	{       "godsonly",     Flags::P,      true    },
	{       "heroesonly",   Flags::Q,      true    },
	{       "newbiesonly",  Flags::R,      true    },
	{       "law",          Flags::S,      true    },
	{       "nowhere",      Flags::T,      true    },
	{       "bank",         Flags::U,      true    },
	{       "leaderonly",   Flags::V,      true    },
	{       "teleport",     Flags::W,      true    },
	{       "underwater",   Flags::X,      true    },
	{       "noportal",     Flags::Y,      true    },
	{       "remortsonly",  Flags::Z,      true    },
	{       "noquest",      Flags::a,     true    },
	{       "silent",       Flags::b,     true    },
};

const std::vector<flag_type> affect_flags = {
	{   "blind",                Flags::A,      true    },
	{   "invisible",            Flags::B,      true    },
	{   "detect_evil",          Flags::C,      true    },
	{   "detect_invis",         Flags::D,      true    },
	{   "detect_magic",         Flags::E,      true    },
	{   "detect_hidden",        Flags::F,      true    },
	{   "detect_good",          Flags::G,      true    },
	{   "sanctuary",            Flags::H,      true    },
	{   "faerie_fire",          Flags::I,      true    },
	{   "infrared",             Flags::J,      true    },
	{   "curse",                Flags::K,      true    },
	{   "fear",                 Flags::L,      true    },
	{   "poison",               Flags::M,      true    },
	{   "protect_evil",         Flags::N,      true    },
	{   "protect_good",         Flags::O,      true    },
	{   "sneak",                Flags::P,      true    },
	{   "hide",                 Flags::Q,      true    },
	{   "night_vision",         Flags::R,      true    },
	{   "charm",                Flags::S,      true    },
	{   "flying",               Flags::T,      true    },
	{   "pass_door",            Flags::U,      true    },
	{   "haste",                Flags::V,      true    },
	{   "calm",                 Flags::W,      true    },
	{   "plague",               Flags::X,      true    },
	{   "steel_mist",           Flags::Y,      true    },
	{   "divineregen",          Flags::Z,      true    },
	{   "berserk",              Flags::a,     true    },
	{   "flameshield",          Flags::b,     true    },
	{   "regeneration",         Flags::c,     true    },
	{   "slow",                 Flags::d,     true    },
	{   "talon",                Flags::e,     true    },
};

const std::vector<flag_type> off_flags = {
	{   "area_attack",          Flags::A,      true    },
	{   "backstab",             Flags::B,      true    },
	{   "bash",                 Flags::C,      true    },
	{   "berserk",              Flags::D,      true    },
	{   "disarm",               Flags::E,      true    },
	{   "dodge",                Flags::F,      true    },
	{   "fast",                 Flags::H,      true    },
	{   "kick",                 Flags::I,      true    },
	{   "dirt_kick",            Flags::J,      true    },
	{   "parry",                Flags::K,      true    },
	{   "rescue",               Flags::L,      true    },
	{   "trip",                 Flags::N,      true    },
	{   "crush",                Flags::O,      true    },
	{   "assist_all",           Flags::P,      true    },
	{   "assist_align",         Flags::Q,      true    },
	{   "assist_race",          Flags::R,      true    },
	{   "assist_players",       Flags::S,      true    },
	{   "assist_guard",         Flags::T,      true    },
	{   "assist_vnum",          Flags::U,      true    },
};

const std::vector<flag_type> form_flags = {
	{   "edible",               FORM_EDIBLE,            true    },
	{   "poison",               FORM_POISON,            true    },
	{   "magical",              FORM_MAGICAL,           true    },
	{   "instant_decay",        FORM_INSTANT_DECAY,     true    },
	{   "other",                FORM_OTHER,             true    },
	{   "animal",               FORM_ANIMAL,            true    },
	{   "sentient",             FORM_SENTIENT,          true    },
	{   "undead",               FORM_UNDEAD,            true    },
	{   "construct",            FORM_CONSTRUCT,         true    },
	{   "mist",                 FORM_MIST,              true    },
	{   "intangible",           FORM_INTANGIBLE,        true    },
	{   "biped",                FORM_BIPED,             true    },
	{   "centaur",              FORM_CENTAUR,           true    },
	{   "insect",               FORM_INSECT,            true    },
	{   "spider",               FORM_SPIDER,            true    },
	{   "crustacean",           FORM_CRUSTACEAN,        true    },
	{   "worm",                 FORM_WORM,              true    },
	{   "blob",                 FORM_BLOB,              true    },
	{   "mammal",               FORM_MAMMAL,            true    },
	{   "bird",                 FORM_BIRD,              true    },
	{   "reptile",              FORM_REPTILE,           true    },
	{   "snake",                FORM_SNAKE,             true    },
	{   "dragon",               FORM_DRAGON,            true    },
	{   "amphibian",            FORM_AMPHIBIAN,         true    },
	{   "fish",                 FORM_FISH ,             true    },
	{   "cold_blood",           FORM_COLD_BLOOD,        true    },
};

const std::vector<flag_type> part_flags = {
	{   "head",                 PART_HEAD,              true    },
	{   "arms",                 PART_ARMS,              true    },
	{   "legs",                 PART_LEGS,              true    },
	{   "heart",                PART_HEART,             true    },
	{   "brains",               PART_BRAINS,            true    },
	{   "guts",                 PART_GUTS,              true    },
	{   "hands",                PART_HANDS,             true    },
	{   "feet",                 PART_FEET,              true    },
	{   "fingers",              PART_FINGERS,           true    },
	{   "ear",                  PART_EAR,               true    },
	{   "eye",                  PART_EYE,               true    },
	{   "long_tongue",          PART_LONG_TONGUE,       true    },
	{   "eyestalks",            PART_EYESTALKS,         true    },
	{   "tentacles",            PART_TENTACLES,         true    },
	{   "fins",                 PART_FINS,              true    },
	{   "wings",                PART_WINGS,             true    },
	{   "tail",                 PART_TAIL,              true    },
	{   "claws",                PART_CLAWS,             true    },
	{   "fangs",                PART_FANGS,             true    },
	{   "horns",                PART_HORNS,             true    },
	{   "scales",               PART_SCALES,            true    },
	{   "tusks",                PART_TUSKS,             true    },
};

const std::vector<flag_type> group_flags = {
	{   "trolls",               GROUP_VNUM_TROLLS,      true    },
	{   "ogres",                GROUP_VNUM_OGRES,       true    },
};

const std::vector<flag_type> comm_flags = {
	{   "quiet",                COMM_QUIET,             true    },
	{   "deaf",                 COMM_DEAF,              true    },
	{   "nowiz",                COMM_NOWIZ,             true    },
	{   "nopray",               COMM_NOPRAY,            true    },
	{   "rpray",                COMM_RPRAY,             true    },
	{   "noauction",            COMM_NOAUCTION,         true    },
	{   "nogossip",             COMM_NOGOSSIP,          true    },
	{   "noflame",              COMM_NOFLAME,           true    },
	{   "noquestion",           COMM_NOQUESTION,        true    },
	{   "nomusic",              COMM_NOMUSIC,           true    },
	{   "noclan",               COMM_NOCLAN,            true    },
	{   "compact",              COMM_COMPACT,           true    },
	{   "brief",                COMM_BRIEF,             true    },
	{   "prompt",               COMM_PROMPT,            true    },
	{   "combine",              COMM_COMBINE,           true    },
	{   "show_affects",         COMM_SHOW_AFFECTS,      true    },
	{   "nograts",              COMM_NOGRATS,           true    },
	{   "noannounce",           COMM_NOANNOUNCE,        true    },
	{   "nosocial",             COMM_NOSOCIAL,          true    },
	{   "noquery",              COMM_NOQUERY,           true    },
	{   "noqwest",              COMM_NOQWEST,           true    },
	{   "nopage",               COMM_NOPAGE,            true    },
	{   "nochannels",           COMM_NOCHANNELS,        true    },
	{   "afk",                  COMM_AFK,               true    },
	{   "atbprompt",            COMM_ATBPROMPT,         true    },
};

const std::vector<flag_type> censor_flags = {
	{       "channels",             CENSOR_CHAN,            true    },
	{       "spam",                 CENSOR_SPAM,            true    },
};

const std::vector<flag_type> revoke_flags = {
	{   "nochannels",          REVOKE_NOCHANNELS,       true    },
	{   "flame_only",          REVOKE_FLAMEONLY,        true    },
	{   "pray",                REVOKE_PRAY,             true    },
	{   "gossip",              REVOKE_GOSSIP,           true    },
	{   "flame",               REVOKE_FLAME,            true    },
	{   "qwest",               REVOKE_QWEST,            true    },
	{   "auction",             REVOKE_AUCTION,          true    },
	{   "clan",                REVOKE_CLAN,             true    },
	{   "ic",                  REVOKE_IC,               true    },
	{   "music",               REVOKE_MUSIC,            true    },
	{   "q/a",                 REVOKE_QA,               true    },
	{   "social",              REVOKE_SOCIAL,           true    },
	{   "grats",               REVOKE_GRATS,            true    },
	{   "page",                REVOKE_PAGE,             true    },
	{   "qtell",               REVOKE_QTELL,            true    },
};

const std::vector<affect_table_type> weapon_affects = {
	{   "acidic",              affect::type::weapon_acidic,   true    },
	{   "flaming",             affect::type::weapon_flaming,  true    },
	{   "frost",               affect::type::weapon_frost,    true    },
	{   "vampiric",            affect::type::weapon_vampiric, true    },
	{   "sharp",               affect::type::weapon_sharp,    true    },
	{   "vorpal",              affect::type::weapon_vorpal,   true    },
	{   "twohanded",           affect::type::weapon_two_hands,true    },
	{   "shocking",            affect::type::weapon_shocking, true    },
	{   "poison",              affect::type::poison,   true    },
};

const std::vector<color_type> color_table = {
	{       "black",        "{k",   GREY,   NOBOLD  },
	{       "charcoal",     "{c",   GREY,   BOLD    },
	{       "red",          "{R",   RED,    NOBOLD  },
	{       "pink",         "{P",   RED,    BOLD    },
	{       "hunter",       "{H",   GREEN,  NOBOLD  },
	{       "green",        "{G",   GREEN,  BOLD    },
	{       "brown",        "{b",   YELLOW, NOBOLD  },
	{       "yellow",       "{Y",   YELLOW, BOLD    },
	{       "navy",         "{N",   BLUE,   NOBOLD  },
	{       "blue",         "{B",   BLUE,   BOLD    },
	{       "maroon",       "{M",   PURPLE, NOBOLD  },
	{       "violet",       "{V",   PURPLE, BOLD    },
	{       "turquoise",    "{T",   CYAN,   NOBOLD  },
	{       "cyan",         "{C",   CYAN,   BOLD    },
	{       "grey",         "{g",   WHITE,  NOBOLD  },
	{       "white",        "{W",   WHITE,  BOLD    },
};

/* default color settings table -- Montrey */
/* sections must be separated by a nullptr line, entries must match the
   definitions in merc.h for CSLOTs */
const std::vector<csetting_type> csetting_table = {
//		color       bold    name
	{       GREEN,  NOBOLD, "Gossip"        },      /*  0 */
	{       RED,    BOLD,   "Flame"         },
	{       YELLOW, BOLD,   "Qwest"         },
	{       RED,    NOBOLD, "Pray"          },
	{       YELLOW, NOBOLD, "Auction"       },
	{       WHITE,  BOLD,   "Announcements" },      /*  5 */
	{       BLUE,   BOLD,   "Music"         },
	{       CYAN,   BOLD,   "QA"            },
	{       GREEN,  NOBOLD, "Socials"       },
	{       PURPLE, BOLD,   "Grats"         },
	{       RED,    BOLD,   "Immtalk"       },      /* 10 */
	{       RED,    NOBOLD, "Page"          },
	{       CYAN,   BOLD,   "Tell"          },
	{       RED,    NOBOLD, "Clantalk"      },
	{       WHITE,  BOLD,   "IC"            },
	{       WHITE,  NOBOLD, "Say"           },      /* 15 */
	{       PURPLE, BOLD,   "GTell"         },
	{       PURPLE, BOLD,   "QTell"         },
	{       YELLOW, BOLD,   "Spousetalk"    },
	{       0,      0,      ""              },
	{       0,      0,      ""              },      /* 20 */
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       0,      0,      ""              },      /* 25 */
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       CYAN,   BOLD,   "Name"          },      /* 30 */
	{       PURPLE, NOBOLD, "PKills"        },
	{       PURPLE, BOLD,   "AKills"        },
	{       CYAN,   NOBOLD, "Class"         },
	{       RED,    BOLD,   "Stats"         },
	{       BLUE,   BOLD,   "Armor"         },      /* 35 */
	{       YELLOW, BOLD,   "Points"        },
	{       GREEN,  NOBOLD, "Weight"        },
	{       YELLOW, NOBOLD, "Gain"          },
	{       WHITE,  BOLD,   "Alignment"     },
	{       RED,    NOBOLD, "Money"         },      /* 40 */
	{       GREEN,  BOLD,   "Diceroll"      },
	{       YELLOW, BOLD,   "QP"            },
	{       GREEN,  BOLD,   "SP"            },
	{       PURPLE, BOLD,   "RPP"           },
	{       0,      0,      ""              },      /* 45 */
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       YELLOW, NOBOLD, "Torch"         },      /* 50 */
	{       RED,    BOLD,   "Flame"         },
	{       YELLOW, NOBOLD, "Border"        },
	{       CYAN,   BOLD,   "Name"          },
	{       WHITE,  BOLD,   "Title"         },
	{       CYAN,   NOBOLD, "Class"         },      /* 55 */
	{       CYAN,   NOBOLD, "Level"         },
	{       RED,    BOLD,   "Stats"         },
	{       RED,    NOBOLD, "Trained"       },
	{       GREY,   BOLD,   "Healthname"    },
	{       WHITE,  NOBOLD, "Healthnum"     },      /* 60 */
	{       CYAN,   BOLD,   "Money"         },
	{       GREEN,  NOBOLD, "Weight"        },
	{       PURPLE, BOLD,   "Armor"         },
	{       GREEN,  BOLD,   "Dicename"      },
	{       YELLOW, BOLD,   "Dicenum"       },      /* 65 */
	{       BLUE,   BOLD,   "Pointname"     },
	{       CYAN,   BOLD,   "Pointnum"      },
	{       GREY,   BOLD,   "Alignment"     },
	{       WHITE,  BOLD,   "XPName"        },
	{       GREY,   BOLD,   "XPNum"         },      /* 70 */
	{       GREEN,  NOBOLD, "Position"      },
	{       RED,    NOBOLD, "PKRecord"      },
	{       WHITE,  BOLD,   "PKRank"        },
	{       0,      0,      ""              },
	{       0,      0,      ""              },      /* 75 */
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       RED,    BOLD,   "Rooms"         },      /* 80 */
	{       GREEN,  BOLD,   "Mobiles"       },
	{       YELLOW, NOBOLD, "Objects"       },
	{       PURPLE, BOLD,   "Players"       },
	{       YELLOW, NOBOLD, "Inventory"     },
	{       0,      0,      ""              },      /* 85 */
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       0,      0,      ""              },      /* 90 */
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       0,      0,      ""              },      /* 95 */
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       0,      0,      ""              },
	{       0,      0,      ""              }       /* MAX_COLOR */
};


int flag_index_lookup(const String& name, const std::vector<flag_type>& flag_table)
{
	for (unsigned int flag = 0; flag < flag_table.size(); flag++) {
		if (tolower(name[0]) == tolower(flag_table[flag].name[0])
		    &&  name.is_prefix_of(flag_table[flag].name))
			return flag;
	}

	return -1;
}

int affect_index_lookup(const String& name, const std::vector<affect_table_type>& affect_table)
{
	for (unsigned int i = 0; i < affect_table.size(); i++) {
		if (tolower(name[0]) == tolower(affect_table[i].name[0])
		    &&  name.is_prefix_of(affect_table[i].name))
			return i;
	}

	return -1;
}
