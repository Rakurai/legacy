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

#include "channels.h"
#include "merc.h"
#include "memory.h"
#include "recycle.h"
#include "sql.h"
#include "lookup.h"
#include "music.h"
#include "Affect.hpp"
#include "affect_list.h"
#include "Auction.hpp"
#include "Format.hpp"
#include "GameTime.hpp"

extern  int     _filbuf         args((FILE *));
extern void          affect_copy_to_list         args(( Affect **list_head, const Affect *paf ));

/*
 * Globals.
 */
Shop              *shop_first;
Shop              *shop_last;


char                    bug_buf         [2 * MAX_INPUT_LENGTH];
Character              *char_list;
Player                *pc_list;        /* should probably go somewhere else *shrug* -- Montrey */
String                  help_greeting;
char                    log_buf         [2 * MAX_INPUT_LENGTH];
Object               *object_list;
String                  default_prompt = "%CW<%CC%h%CThp %CG%m%CHma %CB%v%CNst%CW> ";

Object               *donation_pit;

/* records */
unsigned long   record_logins = 0;
int             record_players = 0;
int             record_players_since_boot = 0;

/* skill table global skill numbers.  skill_lookup will return a sn of a
   skill from it's name, but these are fast :) */
sh_int  gsn_reserved;
sh_int  gsn_acid_blast;
sh_int  gsn_acid_breath;
sh_int  gsn_acid_rain;
sh_int  gsn_age;
sh_int  gsn_animate_skeleton;
sh_int  gsn_animate_wraith;
sh_int  gsn_animate_gargoyle;
sh_int  gsn_animate_zombie;
sh_int  gsn_armor;
sh_int  gsn_bless;
sh_int  gsn_blind_fight;
sh_int  gsn_blindness;
sh_int  gsn_blizzard;
sh_int  gsn_blood_blade;
sh_int  gsn_blood_moon;
sh_int  gsn_burning_hands;
sh_int  gsn_call_lightning;
sh_int  gsn_calm;
sh_int  gsn_cancellation;
sh_int  gsn_cause_light;
sh_int  gsn_cause_serious;
sh_int  gsn_cause_critical;
sh_int  gsn_chain_lightning;
sh_int  gsn_change_sex;
sh_int  gsn_channel;
sh_int  gsn_charm_person;
sh_int  gsn_chill_touch;
sh_int  gsn_colour_spray;
sh_int  gsn_continual_light;
sh_int  gsn_control_weather;
sh_int  gsn_create_food;
sh_int  gsn_create_parchment;
sh_int  gsn_create_rose;
sh_int  gsn_create_sign;
sh_int  gsn_create_spring;
sh_int  gsn_create_vial;
sh_int  gsn_create_water;
sh_int  gsn_cure_blindness;
sh_int  gsn_cure_critical;
sh_int  gsn_cure_disease;
sh_int  gsn_cure_light;
sh_int  gsn_cure_poison;
sh_int  gsn_cure_serious;
sh_int  gsn_curse;
sh_int  gsn_darkness;
sh_int  gsn_dazzling_light;
sh_int  gsn_demonfire;
sh_int  gsn_detect_evil;
sh_int  gsn_detect_good;
sh_int  gsn_detect_hidden;
sh_int  gsn_detect_invis;
sh_int  gsn_detect_magic;
sh_int  gsn_detect_poison;
sh_int  gsn_dispel_evil;
sh_int  gsn_dispel_good;
sh_int  gsn_dispel_magic;
sh_int  gsn_divine_healing;
sh_int  gsn_divine_regeneration;
sh_int  gsn_earthquake;
sh_int  gsn_encampment;
sh_int  gsn_enchant_armor;
sh_int  gsn_enchant_weapon;
sh_int  gsn_energy_drain;
sh_int  gsn_faerie_fire;
sh_int  gsn_faerie_fog;
sh_int  gsn_farsight;
sh_int  gsn_fear;
sh_int  gsn_fire_breath;
sh_int  gsn_fireball;
sh_int  gsn_fireproof;
sh_int  gsn_firestorm;
sh_int  gsn_flame_blade;
sh_int  gsn_flameshield;
sh_int  gsn_flamestrike;
sh_int  gsn_fly;
sh_int  gsn_floating_disc;
sh_int  gsn_frenzy;
sh_int  gsn_frost_blade;
sh_int  gsn_frost_breath;
sh_int  gsn_gas_breath;
sh_int  gsn_gate;
sh_int  gsn_general_purpose;
sh_int  gsn_giant_strength;
sh_int  gsn_harm;
sh_int  gsn_haste;
sh_int  gsn_heal;
sh_int  gsn_heat_metal;
sh_int  gsn_high_explosive;
sh_int  gsn_holy_word;
sh_int  gsn_identify;
sh_int  gsn_night_vision;
sh_int  gsn_invis;
//sh_int  gsn_ironskin;
sh_int  gsn_know_alignment;
sh_int  gsn_light_of_truth;
sh_int  gsn_lightning_bolt;
sh_int  gsn_lightning_breath;
sh_int  gsn_locate_life;
sh_int  gsn_locate_object;
sh_int  gsn_magic_missile;
sh_int  gsn_mass_healing;
sh_int  gsn_mass_invis;
sh_int  gsn_nexus;
sh_int  gsn_pass_door;
sh_int  gsn_plague;
sh_int  gsn_poison;
sh_int  gsn_power_word;
sh_int  gsn_polymorph;
sh_int  gsn_portal;
sh_int  gsn_protect_container;
sh_int  gsn_protection_evil;
sh_int  gsn_protection_good;
sh_int  gsn_ray_of_truth;
sh_int  gsn_recharge;
sh_int  gsn_refresh;
sh_int  gsn_resurrect;
sh_int  gsn_regeneration;
sh_int  gsn_remove_alignment;
sh_int  gsn_remove_invis;
sh_int  gsn_remove_curse;
sh_int  gsn_sanctuary;
sh_int   gsn_scry;
sh_int  gsn_shield;
sh_int  gsn_shock_blade;
sh_int  gsn_shocking_grasp;
sh_int  gsn_shrink;
sh_int  gsn_sleep;
sh_int  gsn_slow;
sh_int  gsn_smokescreen;
sh_int  gsn_starve;
sh_int  gsn_steel_mist;
sh_int  gsn_stone_skin;
sh_int  gsn_summon;
sh_int  gsn_summon_object;
sh_int  gsn_sunray;
sh_int  gsn_talon;
sh_int  gsn_teleport;
sh_int  gsn_teleport_object;
sh_int  gsn_undo_spell;
sh_int  gsn_ventriloquate;
sh_int  gsn_vision;
sh_int  gsn_weaken;
sh_int  gsn_word_of_recall;
sh_int  gsn_wrath;
sh_int  gsn_axe;
sh_int  gsn_dagger;
sh_int  gsn_flail;
sh_int  gsn_mace;
sh_int  gsn_polearm;
sh_int  gsn_spear;
sh_int  gsn_sword;
sh_int  gsn_whip;
sh_int  gsn_archery;
sh_int  gsn_shield_block;
sh_int  gsn_brew;
sh_int  gsn_scribe;
sh_int  gsn_backstab;
sh_int  gsn_bash;
sh_int  gsn_berserk;
sh_int  gsn_circle;
sh_int  gsn_crush;
sh_int  gsn_dirt_kicking;
sh_int  gsn_disarm;
sh_int  gsn_dodge;
sh_int  gsn_enhanced_damage;
sh_int  gsn_envenom;
sh_int  gsn_hand_to_hand;
sh_int  gsn_kick;
sh_int  gsn_roundhouse; /*evo 2+ kick*/
sh_int  gsn_footsweep;
sh_int  gsn_necromancy;
sh_int  gsn_parry;
sh_int  gsn_rescue;
sh_int  gsn_trip;
sh_int  gsn_second_attack;
sh_int  gsn_third_attack;
sh_int  gsn_dual_wield;
sh_int  gsn_hunt;
sh_int   gsn_unarmed;
sh_int  gsn_swimming;
sh_int  gsn_fast_healing;
sh_int  gsn_firebuilding;
sh_int  gsn_forge;
sh_int  gsn_repair;
sh_int  gsn_rotate;
sh_int  gsn_languages;
sh_int  gsn_haggle;
sh_int  gsn_hide;
sh_int  gsn_lore;
sh_int  gsn_meditation;
sh_int  gsn_peek;
sh_int  gsn_pick_lock;
sh_int  gsn_scan;
sh_int  gsn_sneak;
sh_int  gsn_steal;
sh_int  gsn_sing;
sh_int  gsn_scrolls;
sh_int  gsn_spousegate;
sh_int  gsn_staves;
sh_int  gsn_wands;
sh_int  gsn_recall;
sh_int   gsn_lay_on_hands;
sh_int   gsn_familiar;
sh_int   gsn_die_hard;
sh_int  gsn_sheen;
sh_int  gsn_focus;
sh_int  gsn_paralyze;
sh_int  gsn_barrier;
sh_int  gsn_dazzle;
sh_int  gsn_full_heal;
sh_int  gsn_midnight;
sh_int  gsn_shadow_form;
sh_int  gsn_hone;
sh_int  gsn_riposte;
sh_int  gsn_fourth_attack;
sh_int  gsn_rage;
sh_int  gsn_sap;
sh_int  gsn_pain;
sh_int  gsn_hex;
sh_int  gsn_bone_wall;
sh_int  gsn_hammerstrike;
sh_int  gsn_force_shield;
sh_int  gsn_holy_sword;
sh_int  gsn_align;
sh_int  gsn_blur;
sh_int  gsn_dual_second;
sh_int  gsn_quick;
sh_int  gsn_standfast;
sh_int  gsn_mark;
sh_int  gsn_critical_blow;

/*
 * Locals.
 */
MobilePrototype         *mob_index_hash          [MAX_KEY_HASH];
ObjectPrototype         *obj_index_hash          [MAX_KEY_HASH];
RoomPrototype        *room_index_hash         [MAX_KEY_HASH];

Area              *area_first;
Area              *area_last;

/* quest_open, if true, says that the quest area is open.
   quest_min, quest_max are defined only if quest_open is true. */
/* quest_upk is restricted pk in quest area or not, quest_double is double qp time */
bool                    quest_open;
bool                    quest_upk;
long                    quest_double = 0;
int                     quest_min, quest_max;
/* startroom and area are initialized by quest_init() */
RoomPrototype         *quest_startroom;
Area               *quest_area;

/*
 * Credits defines stuff
 */

#define RANGE_OK   0
#define RANGE_ALL  1
#define RANGE_CLAN 2
#define RANGE_IMM  3

/*
 * Semi-locals.
 */
bool                    fBootDb;
FILE                   *fpArea;
char                    strArea[MAX_INPUT_LENGTH];
sh_int                  aVersion = 1;
/*
 * MOBprogram locals
*/

int             mprog_name_to_type      args((const String& name));
MobProg     *mprog_file_read         args((const String& f, MobProg *mprg,
                MobilePrototype *pMobIndex));
void            mprog_read_programs     args((FILE *fp,
                MobilePrototype *pMobIndex));

/*
 * Local booting procedures.
*/
void    init_mm         args((void));
void    load_area       args((FILE *fp));
void    load_mobiles    args((FILE *fp));
void    load_objects    args((FILE *fp));
void    load_resets     args((FILE *fp));
void    load_rooms      args((FILE *fp));
void    load_shops      args((FILE *fp));
void    load_specials   args((FILE *fp));
void    load_notes      args((void));
void    fix_exits       args((void));

/* Big mama top level function */
void boot_db()
{
	fBootDb = TRUE;

	/* Init random number generator */
	{
		init_mm();
	}

	/* Load the clan info, needs to be done before the areas due to clanrooms */
	{
		load_clan_table();
		Format::printf("survived load_clan_table\n");
	}
	/* Set time and weather */
	{
		long lhour, lday, lmonth;
		lhour           = (current_time - 650336715) / (PULSE_TICK / PULSE_PER_SECOND);
		time_info.hour  = lhour  % MUD_DAY;
		lday            = lhour  / MUD_DAY;
		time_info.day   = lday   % MUD_MONTH;
		lmonth          = lday   / MUD_MONTH;
		time_info.month = lmonth % MUD_YEAR;
		time_info.year  = lmonth / MUD_YEAR;
		time_info.motd  = "";

		if (time_info.hour <  5)   weather_info.sunlight = SUN_DARK;
		else if (time_info.hour <  6)   weather_info.sunlight = SUN_RISE;
		else if (time_info.hour < 19)   weather_info.sunlight = SUN_LIGHT;
		else if (time_info.hour < 20)   weather_info.sunlight = SUN_SET;
		else                            weather_info.sunlight = SUN_DARK;

		weather_info.change     = 0;
		weather_info.mmhg       = 960;

		if (time_info.month >= 7 && time_info.month <= 12)
			weather_info.mmhg += number_range(1, 50);
		else
			weather_info.mmhg += number_range(1, 80);

		if (weather_info.mmhg <=  980)     weather_info.sky = SKY_LIGHTNING;
		else if (weather_info.mmhg <= 1000)     weather_info.sky = SKY_RAINING;
		else if (weather_info.mmhg <= 1020)     weather_info.sky = SKY_CLOUDY;
		else                                    weather_info.sky = SKY_CLOUDLESS;
	}
	/* initialize auction */
	{
		auction.init();
	}
	/* Assign gsn's for skills which have them */
	{
		int sn;

		for (sn = 0; sn < skill_table.size(); sn++)
			if (skill_table[sn].pgsn != NULL)
				*skill_table[sn].pgsn = sn;
	}
	/* Read in all the area files */
	{
		FILE *fpList;

		if ((fpList = fopen(AREA_LIST, "r")) == NULL) {
			perror(AREA_LIST);
			exit(1);
		}

		for (; ;) {
			strcpy(strArea, fread_word(fpList));

			if (strArea[0] == '$')
				break;

			if (strArea[0] == '#')
				continue;

			if (strArea[0] == '-')
				fpArea = stdin;
			else {
				char abuf[MSL];
				Format::sprintf(abuf, "%s%s", AREA_DIR, strArea);

				if ((fpArea = fopen(abuf, "r")) == NULL) {
					perror(strArea);
					exit(1);
				}
			}

			Format::printf("Now loading area: %s\n", strArea);

			for (; ;) {
				if (fread_letter(fpArea) != '#') {
					bug("Boot_db: # not found.", 0);
					exit(1);
				}

				String word = fread_word(fpArea);

				if (word[0] == '$')  break;
				else if (word == "AREA")  load_area(fpArea);
				else if (word == "MOBILES")  load_mobiles(fpArea);
				else if (word == "OBJECTS")  load_objects(fpArea);
				else if (word == "RESETS")  load_resets(fpArea);
				else if (word == "ROOMS")  load_rooms(fpArea);
				else if (word == "SHOPS")  load_shops(fpArea);
				else if (word == "SPECIALS")  load_specials(fpArea);
//				else if (word == "TOURSTARTS")  load_tourstarts(fpArea);
//				else if (word == "TOURROUTES")  load_tourroutes(fpArea);
				else {
					bug("Boot_db: bad section name.", 0);
					exit(1);
				}
			}

			if (fpArea != stdin)
				fclose(fpArea);

			fpArea = NULL;
		}

		fclose(fpList);
	}
	/* initialize quest stuff after areas loaded, maybe areas are needed */
	quest_init();

	int itemsloaded = objstate_load_items();   /* load our list of items from disk, before resets! */
	Format::printf("survived objstate_load_items (%d)\n", itemsloaded);

	/* Perform various loading procedures, reset all areas once, fix up exits */
	fix_exits();
	Format::printf("survived fix_exits\n");
	load_war_table();
	Format::printf("survived load_war_table\n");
	load_war_events();
	Format::printf("survived load_war_events\n");
	load_arena_table();
	Format::printf("survived load_arena_table\n");
	fBootDb = FALSE;
	area_update();
	Format::printf("survived area_update\n");
	load_notes();
	Format::printf("survived load_notes\n");
	load_disabled();
	Format::printf("survived load_disabled\n");
	MOBtrigger = TRUE;
	load_songs();
	Format::printf("survived load_songs\n");
	load_social_table();
	Format::printf("survived load_social_table\n");
	load_storage_list();
	Format::printf("survived load_storage_list\n");
	load_departed_list();
	Format::printf("survived load_departed_list\n");

	/* read in our record players and record logins */
	if (db_query("boot_db", "SELECT logins, players FROM records") == SQL_OK) {
		if (db_next_row() == SQL_OK) {
			record_logins = db_get_column_int(0);
			record_players = db_get_column_int(1);
		}
		else
			bug("boot_db: failed to fetch record logins and players", 0);
	}

	/* load our greeting */
	if (db_query("boot_db", "SELECT text FROM helps WHERE keywords='GREETING'") == SQL_OK) {
		if (db_next_row() == SQL_OK)
			help_greeting = db_get_column_str(0);
		else {
			bug("boot_db: failed to fetch greeting", 0);
			exit(1);
		}
	}

	if (help_greeting.empty())
		help_greeting = "need a greeting! enter your name: ";
} /* end boot_db() */

/*
 * Pull the level ranges from the area credits field -- Elrac
 * Assumed credit line format example:
 * {H{{ 5 25} {MFinn    {TThe Fourth Tower
 * - Elrac
 */
int  scan_credits(Area *pArea)
{
	char line[MIL], buf[MIL];
	String keywords;
	char *p, *levels, *author, *title;
	int nblanks;

	if (pArea->credits.empty()) {
		log_string("scan_credits: No credits available.\n");
		return -1;
	}

	/* default area type/levels */
	pArea->area_type  = AREA_TYPE_NORM;
	pArea->low_range  = 1;
	pArea->high_range = LEVEL_HERO;

	/* credit line gets mangled in scanning. copy for safekeeping. */
	strcpy(line, pArea->credits);
	/*** scan low/high level range numbers ***/
	/* terminate level range at first closing brace. */
	p = strchr(line, '}');

	if (p == NULL) {
		log_string("Missing '}' in credits line\n");
		return -1;
	}

	*p = '\0';
	author = p + 1;
	/* find the first nonblank after the last opening brace char in levels. */
	levels = line;

	while ((p = strchr(levels, '{')) != NULL) levels = p + 1;

	while (isascii(*levels) && isspace(*levels)) levels++;

	/* truncate level range at last non-blank ascii char */
	nblanks = 0;

	for (p = levels; *p; p++) {
		if (!isascii(*p))
			nblanks = 0;
		else if (isspace(*p))
			nblanks++;
		else {
			*p = toupper(*p);
			nblanks = 0;
		}
	}

	*(p -= nblanks) = '\0';

	if (*levels == '\0') {
		log_string("scan_credits: Empty level range string\n");
		return -2;
	}

	if (!strcmp(levels, "ALL")) {
		pArea->area_type  = AREA_TYPE_ALL;
		keywords = "ALL ";
	}
	else if (!strcmp(levels, "CLANS") || !strcmp(levels, "CLAN")) {
		pArea->low_range  = 15; /* min for clan membership */
		pArea->area_type  = AREA_TYPE_CLAN;
		keywords = "CLAN ";
	}
	else if (!strcmp(levels, "IMM") || !strcmp(levels, "IMMS")) {
		pArea->low_range  = LEVEL_IMMORTAL;
		pArea->high_range = MAX_LEVEL;
		pArea->area_type  = AREA_TYPE_IMMS;
		keywords = "IMMS ";
	}
	else if (!strcmp(levels, "HRO") || !strcmp(levels, "HERO")) {
		pArea->low_range  = LEVEL_HERO;
		pArea->high_range = MAX_LEVEL;
		pArea->area_type  = AREA_TYPE_HERO;
		keywords = "HRO HERO ";
	}
	else if (!strcmp(levels, "ARENA")) {
		pArea->low_range  = 1;
		pArea->high_range = MAX_LEVEL;
		pArea->area_type  = AREA_TYPE_ARENA;
		keywords = "ARENA ";
	}
	else if (!strcmp(levels, "XXX")) {
		pArea->low_range  = MAX_LEVEL;
		pArea->high_range = MAX_LEVEL;
		pArea->area_type  = AREA_TYPE_XXX;
		keywords = "XXX ";
	}
	else if (!isascii(*levels) || !isdigit(*levels)) {
		Format::sprintf(buf, "scan_credits: Unrecognized level range: '%s'\n", levels);
		log_string(buf);
		return -3;
	}
	else {
		int ilow, ihigh;
		pArea->area_type = AREA_TYPE_NORM;
		ilow = atoi(levels);

		if (ilow < 0) {
			Format::sprintf(buf, "scan_credits: Bad start level: %d\n", ilow);
			log_string(buf);
			return -4;
		}

		pArea->low_range = ilow;

		/* skip digits and spaces. */
		while (isascii(*levels) && isdigit(*levels)) levels++;

		while (isascii(*levels) && isspace(*levels)) levels++;

		if (!isascii(*levels) || !isdigit(*levels)) {
			log_string("scan_credits: Missing second number of range\n");
			return -5;
		}

		ihigh = atoi(levels);

		if (ihigh < ilow || ihigh > 100) {
			Format::sprintf(buf, "scan_credits: Bad ending level : low : %d High : %d\n", ilow, ihigh);
			log_string(buf);
			return -6;
		}

		pArea->high_range = ihigh;
	}

	/*** scan author out of credits line (we assume it's one string) ***/

	while (*author == ' ') author++;

	for (p = author; *p && *p != ' '; p++);

	if (*p == '\0') { /* only one string for author & title */
		title = author;
		author = "anonymous";
	}
	else {
		*p = '\0';

		for (title = p + 1; *title == ' '; title++);
	}

	pArea->author = author;
	keywords += String(author).uncolor();
	keywords += " ";
	/*** title is the remainder ***/
	pArea->title = title;
	keywords += String(title).uncolor();

	for (auto it = keywords.begin(); it != keywords.end(); it++) *it = LOWER(*it);

	pArea->keywords = keywords;
	return pArea->area_type;
} /* end scan_credits() */

/*
 * Load an AREA section (only a header, really)
 */
void load_area(FILE *fp)
{
	Area *pArea = new Area;
	pArea->reset_first      = NULL;
	pArea->reset_last       = NULL;
	pArea->file_name        = fread_string(fp);
	String line = pArea->file_name;

	String num;
	line = one_argument(line, num);

	if (num.is_number()) {
		aVersion = atoi(num);
		pArea->file_name = line;
	}
	else
		aVersion = 1;

	pArea->version          = aVersion;
	pArea->name             = fread_string(fp);
	pArea->credits          = fread_string(fp);
	pArea->min_vnum         = fread_number(fp);
	pArea->max_vnum         = fread_number(fp);
	scan_credits(pArea);
	pArea->age              = 15;
	pArea->nplayer          = 0;
	pArea->empty            = FALSE;

	if (area_first == NULL)
		area_first = pArea;

	if (area_last != NULL)
		area_last->next = pArea;

	area_last   = pArea;
	pArea->next = NULL;
	top_area++;
}

/*
 * Snarf a reset section.
 */
void load_resets(FILE *fp)
{
	Reset *pReset;
	RoomPrototype *pRoomIndex;
	Exit *pexit;
	char letter;
	ObjectPrototype *temp_index;

	if (area_last == NULL) {
		bug("Load_resets: no #AREA seen yet.", 0);
		exit(1);
	}

	for (; ;) {
		if ((letter = fread_letter(fp)) == 'S')
			break;

		if (letter == '*') {
			fread_to_eol(fp);
			continue;
		}

		pReset = new Reset;
		pReset->version = aVersion;
		pReset->command = letter;
		/* if_flag */     fread_number(fp);
		pReset->arg1    = fread_number(fp);
		pReset->arg2    = fread_number(fp);
		pReset->arg3    = (letter == 'G' || letter == 'R') ? 0 : fread_number(fp);
		pReset->arg4    = (letter == 'P' || letter == 'M') ? fread_number(fp) : 0;
		fread_to_eol(fp);

		/* Validate parameters.  We're calling the index functions for the side effect. */
		switch (letter) {
		default:
			bug("Load_resets: bad command '%c'.", letter);
			exit(1);
			break;

		/* Mobile resets, worked over to allow for resetting into random rooms -- Montrey */
		case 'M':
			/* make sure the mobile exists */
			get_mob_index(pReset->arg1);

			/* limit the global limit to 20, -1 is max */
			if (pReset->arg2 > 20 || pReset->arg2 < 0)
				pReset->arg2 = 20;

			/* make sure the room exists, vnum 0 means it's a random reset */
			if (pReset->arg3 != 0)
				get_room_index(pReset->arg3);

			/* on a random reset, local limit is percentage chance, limit to 0 to 100 */
			if (pReset->arg3 == 0)
				pReset->arg4 = URANGE(0, pReset->arg4, 100);

			break;

		case 'O':
			temp_index = get_obj_index(pReset->arg1);
			temp_index->reset_num++;
			get_room_index(pReset->arg3);
			break;

		case 'P':
			temp_index = get_obj_index(pReset->arg1);
			temp_index->reset_num++;
			get_obj_index(pReset->arg3);

			if (pReset->arg4 > 20 || pReset->arg4 < 0)
				pReset->arg4 = 20;    /* Lets add a limit - Lotus */

			break;

		case 'G':
		case 'E':
			temp_index = get_obj_index(pReset->arg1);
			temp_index->reset_num++;
			break;

		case 'D':
			pRoomIndex = get_room_index(pReset->arg1);

			if (pReset->arg2 < 0 || pReset->arg2 > 5
			    || (pexit = pRoomIndex->exit[pReset->arg2]) == NULL
			    || !IS_SET(pexit->exit_info, EX_ISDOOR)) {
				bug("Load_resets: 'D': exit %d not door.", pReset->arg2);
				exit(1);
			}

			if (pReset->arg3 < 0 || pReset->arg3 > 2) {
				bug("Load_resets: 'D': bad 'locks': %d.", pReset->arg3);
				exit(1);
			}

			break;

		case 'R':
			pRoomIndex = get_room_index(pReset->arg1);

			if (pReset->arg2 < 0 || pReset->arg2 > 6) {
				bug("Load_resets: 'R': bad exit %d.", pReset->arg2);
				exit(1);
			}

			break;
		}

		if (area_last->reset_first == NULL)
			area_last->reset_first      = pReset;

		if (area_last->reset_last  != NULL)
			area_last->reset_last->next = pReset;

		area_last->reset_last = pReset;
		pReset->next = NULL;
		top_reset++;
	}
}

/*
 * Snarf a mob section.  new style
 */
void load_mobiles(FILE *fp)
{
	MobilePrototype *pMobIndex;
	sh_int vnum;
	char letter;
	int iHash;

	for (; ;) {
		letter                          = fread_letter(fp);

		if (letter != '#') {
			bug("Load_mobiles: # not found.", 0);
			exit(1);
		}

		vnum                            = fread_number(fp);

		if (vnum == 0)
			break;

		if (vnum < area_last->min_vnum ||
		    vnum > area_last->max_vnum)
			bug("mobile vnum %d out of range.", vnum);

		fBootDb = FALSE;

		if (get_mob_index(vnum) != NULL) {
			bug("Load_mobiles: vnum %d duplicated.", vnum);
			exit(1);
		}

		fBootDb = TRUE;
		pMobIndex = new MobilePrototype;
		pMobIndex->vnum                 = vnum;
		pMobIndex->version              = aVersion;
		pMobIndex->player_name          = fread_string(fp);
		pMobIndex->short_descr          = fread_string(fp);
		pMobIndex->long_descr           = fread_string(fp);
		pMobIndex->description          = fread_string(fp);
		pMobIndex->race                 = race_lookup(fread_string(fp));
		pMobIndex->long_descr[0]        = UPPER(pMobIndex->long_descr[0]);
		pMobIndex->description[0]       = UPPER(pMobIndex->description[0]);
		pMobIndex->act_flags                  = fread_flag(fp) | race_table[pMobIndex->race].act;

		// affect flags no longer includes flags already on the race affect bitvector
		pMobIndex->affect_flags         = fread_flag(fp) & ~race_table[pMobIndex->race].aff;

		pMobIndex->pShop                = NULL;
		pMobIndex->alignment            = fread_number(fp);
		pMobIndex->group                = fread_flag(fp);
		pMobIndex->level                = fread_number(fp);
		pMobIndex->hitroll              = fread_number(fp);
		/* read hit dice */
		pMobIndex->hit[DICE_NUMBER]     = fread_number(fp);
		/* 'd'          */                fread_letter(fp);
		pMobIndex->hit[DICE_TYPE]       = fread_number(fp);
		/* '+'          */                fread_letter(fp);
		pMobIndex->hit[DICE_BONUS]      = fread_number(fp);
		/* read mana dice */
		pMobIndex->mana[DICE_NUMBER]    = fread_number(fp);
		fread_letter(fp);
		pMobIndex->mana[DICE_TYPE]      = fread_number(fp);
		fread_letter(fp);
		pMobIndex->mana[DICE_BONUS]     = fread_number(fp);
		/* read damage dice */
		pMobIndex->damage[DICE_NUMBER]  = fread_number(fp);
		fread_letter(fp);
		pMobIndex->damage[DICE_TYPE]    = fread_number(fp);
		fread_letter(fp);
		pMobIndex->damage[DICE_BONUS]   = fread_number(fp);
		pMobIndex->dam_type         = attack_lookup(fread_word(fp));
		/* read armor class */
		pMobIndex->ac[AC_PIERCE]        = fread_number(fp) * 10;
		pMobIndex->ac[AC_BASH]          = fread_number(fp) * 10;
		pMobIndex->ac[AC_SLASH]         = fread_number(fp) * 10;
		pMobIndex->ac[AC_EXOTIC]        = fread_number(fp) * 10;
		/* read flags and add in data from the race table */
		pMobIndex->off_flags            = fread_flag(fp)
		                                  | race_table[pMobIndex->race].off;

		// defense flags no longer includes flags already on the race bitvector
		pMobIndex->absorb_flags         = 0; /* fix when we change the area versions */
		pMobIndex->imm_flags            = fread_flag(fp) & ~race_table[pMobIndex->race].imm;
		pMobIndex->res_flags            = fread_flag(fp) & ~race_table[pMobIndex->race].res;
		pMobIndex->vuln_flags           = fread_flag(fp) & ~race_table[pMobIndex->race].vuln;

		// fix old style ACT_IS_NPC (A) flag, changed to ACT_NOSUMMON (A)
		REMOVE_BIT(pMobIndex->act_flags, A);

		// fix old style IMM_SUMMON (A) flag, changed to ACT_NOSUMMON (A)
		if (IS_SET(pMobIndex->imm_flags, A))
			SET_BIT(pMobIndex->act_flags, ACT_NOSUMMON);
		REMOVE_BIT(pMobIndex->imm_flags, A);
		REMOVE_BIT(pMobIndex->res_flags, A);
		REMOVE_BIT(pMobIndex->vuln_flags, A);

		/* vital statistics */
		pMobIndex->start_pos        = position_lookup(fread_word(fp));

		if (pMobIndex->start_pos == POS_STANDING && IS_SET(pMobIndex->affect_flags, AFF_FLYING))
			pMobIndex->start_pos = POS_FLYING;

		pMobIndex->default_pos      = position_lookup(fread_word(fp));

		if (pMobIndex->default_pos == POS_STANDING && IS_SET(pMobIndex->affect_flags, AFF_FLYING))
			pMobIndex->default_pos = POS_FLYING;

		pMobIndex->sex              = sex_lookup(fread_word(fp));

		if (pMobIndex->sex < 0) {
			bug("Load_mobiles: bad sex for vnum %d.", vnum);
			exit(1);
		}

		pMobIndex->wealth               = fread_number(fp);
		pMobIndex->form                 = fread_flag(fp)
		                                  | race_table[pMobIndex->race].form;
		pMobIndex->parts                = fread_flag(fp)
		                                  | race_table[pMobIndex->race].parts;
		/* size */
		pMobIndex->size                 = size_lookup(fread_word(fp));

		if (pMobIndex->size < 0) {
			bug("Load_mobiles: bad size for vnum %d.", vnum);
			exit(1);
		}

		pMobIndex->material             = fread_word(fp);

		for (; ;) {
			letter = fread_letter(fp);

			if (letter == 'F') {
				String word;
				long vector;
				word                    = fread_word(fp);
				vector                  = fread_flag(fp);

				if (word.is_prefix_of("act"))
					REMOVE_BIT(pMobIndex->act_flags, vector);
				else if (word.is_prefix_of("aff"))
					REMOVE_BIT(pMobIndex->affect_flags, vector);
				else if (word.is_prefix_of("off"))
					REMOVE_BIT(pMobIndex->off_flags, vector);
				else if (word.is_prefix_of("drn"))
					REMOVE_BIT(pMobIndex->absorb_flags, vector);
				else if (word.is_prefix_of("imm"))
					REMOVE_BIT(pMobIndex->imm_flags, vector);
				else if (word.is_prefix_of("res"))
					REMOVE_BIT(pMobIndex->res_flags, vector);
				else if (word.is_prefix_of("vul"))
					REMOVE_BIT(pMobIndex->vuln_flags, vector);
				else if (word.is_prefix_of("for"))
					REMOVE_BIT(pMobIndex->form, vector);
				else if (word.is_prefix_of("par"))
					REMOVE_BIT(pMobIndex->parts, vector);
				else {
					bug("Flag remove: flag not found.", 0);
					exit(1);
				}
			}
			else {
				ungetc(letter, fp);
				break;
			}
		}

		letter = fread_letter(fp);

		if (letter == '>') {
			ungetc(letter, fp);
			mprog_read_programs(fp, pMobIndex);
		}
		else ungetc(letter, fp);

		iHash                   = vnum % MAX_KEY_HASH;
		pMobIndex->next         = mob_index_hash[iHash];
		mob_index_hash[iHash]   = pMobIndex;
		top_mob_index++;
	}

	return;
}

/*
 * Snarf an obj section. new style
 */
void load_objects(FILE *fp)
{
	ObjectPrototype *pObjIndex;
	sh_int vnum;
	char letter;
	int iHash;

	for (; ;) {
		letter                          = fread_letter(fp);

		if (letter != '#') {
			bug("Load_objects: # not found.", 0);
			exit(1);
		}

		vnum                            = fread_number(fp);

		if (vnum == 0)
			break;

		if (vnum < area_last->min_vnum ||
		    vnum > area_last->max_vnum)
			bug("object vnum %d out of range.", vnum);

		fBootDb = FALSE;

		if (get_obj_index(vnum) != NULL) {
			bug("Load_objects: vnum %d duplicated.", vnum);
			exit(1);
		}

		fBootDb = TRUE;
		pObjIndex = new ObjectPrototype;
		pObjIndex->vnum                 = vnum;
		pObjIndex->reset_num            = 0;
		pObjIndex->version              = aVersion;
		pObjIndex->name                 = fread_string(fp);
		pObjIndex->short_descr          = fread_string(fp);
		pObjIndex->description          = fread_string(fp);
		pObjIndex->material             = fread_string(fp);
		pObjIndex->item_type            = item_lookup(fread_word(fp));
		pObjIndex->extra_flags          = fread_flag(fp);
		pObjIndex->wear_flags           = fread_flag(fp);
		pObjIndex->num_settings			= 0;

		switch (pObjIndex->item_type) {
		case ITEM_WEAPON:
			pObjIndex->value[0]         = get_weapon_type(fread_word(fp));
			pObjIndex->value[1]         = fread_number(fp);
			pObjIndex->value[2]         = fread_number(fp);
			pObjIndex->value[3]         = attack_lookup(fread_word(fp));
			pObjIndex->value[4]         = fread_flag(fp);
			break;

		case ITEM_CONTAINER:
			pObjIndex->value[0]         = fread_number(fp);
			pObjIndex->value[1]         = fread_flag(fp);
			pObjIndex->value[2]         = fread_number(fp);
			pObjIndex->value[3]         = fread_number(fp);
			pObjIndex->value[4]         = fread_number(fp);
			break;

		case ITEM_DRINK_CON:
		case ITEM_FOUNTAIN:
			pObjIndex->value[0]         = fread_number(fp);
			pObjIndex->value[1]         = fread_number(fp);
			pObjIndex->value[2]         = liq_lookup(fread_word(fp));

			if (pObjIndex->value[2] == -1) {
				pObjIndex->value[2] = 0;
				bug("Unknown liquid type", 0);
			}

			pObjIndex->value[3]         = fread_number(fp);
			pObjIndex->value[4]         = fread_number(fp);
			break;

		case ITEM_WAND:
		case ITEM_STAFF:
			pObjIndex->value[0]         = fread_number(fp);
			pObjIndex->value[1]         = fread_number(fp);
			pObjIndex->value[2]         = fread_number(fp);
			pObjIndex->value[3]         = skill_lookup(fread_word(fp));
			pObjIndex->value[4]         = fread_number(fp);
			break;

		case ITEM_POTION:
		case ITEM_PILL:
		case ITEM_SCROLL:
			pObjIndex->value[0]         = fread_number(fp);
			pObjIndex->value[1]         = skill_lookup(fread_word(fp));
			pObjIndex->value[2]         = skill_lookup(fread_word(fp));
			pObjIndex->value[3]         = skill_lookup(fread_word(fp));
			pObjIndex->value[4]         = skill_lookup(fread_word(fp));
			break;

		case ITEM_TOKEN:
			pObjIndex->value[0]         = fread_number(fp);
			pObjIndex->value[1]         = fread_number(fp);
			pObjIndex->value[2]         = fread_number(fp);
			pObjIndex->value[3]         = fread_number(fp);
			pObjIndex->value[4]         = fread_number(fp);
			break;

		default:
			pObjIndex->value[0]             = fread_flag(fp);
			pObjIndex->value[1]             = fread_flag(fp);
			pObjIndex->value[2]             = fread_flag(fp);
			pObjIndex->value[3]             = fread_flag(fp);
			pObjIndex->value[4]             = fread_flag(fp);
			break;
		}

		pObjIndex->level                = fread_number(fp);
		pObjIndex->weight               = fread_number(fp);
		pObjIndex->cost                 = fread_number(fp);
		/* condition */
		letter                          = fread_letter(fp);

		switch (letter) {
		case ('P') :                pObjIndex->condition = 100; break;

		case ('G') :                pObjIndex->condition =  90; break;

		case ('A') :                pObjIndex->condition =  75; break;

		case ('W') :                pObjIndex->condition =  50; break;

		case ('D') :                pObjIndex->condition =  25; break;

		case ('B') :                pObjIndex->condition =  10; break;

		case ('R') :                pObjIndex->condition =   5; break;

		case ('I') :                pObjIndex->condition =  -1; break;

		default:                    pObjIndex->condition = 100; break;
		}

		for (; ;) {
			char letter;
			letter = fread_letter(fp);

			if (letter == 'A') { // apply
				Affect af;
				af.type               = 0;
				af.level              = pObjIndex->level;
				af.duration           = -1;
				af.location           = fread_number(fp);
				af.modifier           = fread_number(fp);
				af.evolution          = 1;
				af.bitvector          = 0;
				af.permanent          = TRUE;

				unsigned int bitvector = 0;
				if (affect_parse_flags('O', &af, &bitvector)) {
					affect_copy_to_list(&pObjIndex->affected, &af);
					top_affect++;
				}
			}
			else if (letter == 'F') { // flag, can add bits or do other ->where types
				Affect af;
				af.type               = 0;
				af.level              = pObjIndex->level;
				af.duration           = -1;
				af.evolution          = 1;
				af.permanent          = TRUE;

				letter          = fread_letter(fp);
				af.location     = fread_number(fp);
				af.modifier     = fread_number(fp);

				unsigned int bitvector    = fread_flag(fp);

				// do at least once even if no bitvector
				do {
					if (affect_parse_flags(letter, &af, &bitvector)) {
						affect_copy_to_list(&pObjIndex->affected, &af); 
						top_affect++;

						// don't multiply the modifier, just apply to the first bit
						af.location = 0;
						af.modifier = 0;
					}

					af.type = 0; // reset every time
				} while (bitvector != 0);
			}
			else if (letter == 'E') {
				ExtraDescr *ed = new_extra_descr();
				ed->keyword             = fread_string(fp);
				ed->description         = fread_string(fp);
				ed->next                = pObjIndex->extra_descr;
				pObjIndex->extra_descr  = ed;
				top_ed++;
			}
			else if (letter == 'S') {
				pObjIndex->num_settings = fread_number(fp);
			}
			else {
				ungetc(letter, fp);
				break;
			}
		}

		// affects are immutable, compute the checksum now
		pObjIndex->affect_checksum = affect_checksum_list(&pObjIndex->affected);

		iHash                   = vnum % MAX_KEY_HASH;
		pObjIndex->next         = obj_index_hash[iHash];
		obj_index_hash[iHash]   = pObjIndex;
		top_obj_index++;
	}

	return;
}

/*
 * Snarf a room section.
 */
void load_rooms(FILE *fp)
{
	RoomPrototype *pRoomIndex;
	ExtraDescr *ed;
	Exit *pexit;
	int locks;
	char log_buf[MAX_STRING_LENGTH];
	sh_int vnum;
	char letter;
	int door;
	int iHash;

	if (area_last == NULL) {
		bug("Load_resets: no #AREA seen yet.", 0);
		exit(1);
	}

	for (; ;) {
		letter                          = fread_letter(fp);

		if (letter != '#') {
			bug("Load_rooms: # not found.", 0);
			exit(1);
		}

		vnum                            = fread_number(fp);

		if (vnum == 0)
			break;

		if (vnum < area_last->min_vnum ||
		    vnum > area_last->max_vnum)
			bug("room   vnum %d out of range.", vnum);

		fBootDb = FALSE;

		if (get_room_index(vnum) != NULL) {
			bug("Load_rooms: vnum %d duplicated.", vnum);
			exit(1);
		}

		fBootDb = TRUE;
		pRoomIndex = new RoomPrototype;
		pRoomIndex->version             = aVersion;
		pRoomIndex->people              = NULL;
		pRoomIndex->contents            = NULL;
		pRoomIndex->extra_descr         = NULL;
		pRoomIndex->area                = area_last;
		pRoomIndex->vnum                = vnum;
		pRoomIndex->name                = fread_string(fp);
		pRoomIndex->description         = fread_string(fp);
		pRoomIndex->tele_dest           = fread_number(fp);
		pRoomIndex->room_flags          = fread_flag(fp);

		/* horrible hack */
		if (3000 <= vnum && vnum < 3400)
			SET_BIT(pRoomIndex->room_flags, ROOM_LAW);

		pRoomIndex->sector_type         = fread_number(fp);
		pRoomIndex->light               = 0;

		for (door = 0; door <= 5; door++)
			pRoomIndex->exit[door] = NULL;

		if (IS_SET(GET_ROOM_FLAGS(pRoomIndex), ROOM_FEMALE_ONLY)) {
			Format::sprintf(log_buf, "Room %d is FEMALE_ONLY", pRoomIndex->vnum);
			log_string(log_buf);
		}

		if (IS_SET(GET_ROOM_FLAGS(pRoomIndex), ROOM_MALE_ONLY)) {
			Format::sprintf(log_buf, "Room %d is MALE_ONLY", pRoomIndex->vnum);
			log_string(log_buf);
		}

		if (IS_SET(GET_ROOM_FLAGS(pRoomIndex), ROOM_LOCKER)) {
			Format::sprintf(log_buf, "Room %d is LOCKER", pRoomIndex->vnum);
			log_string(log_buf);
		}

		/* defaults */
		pRoomIndex->heal_rate = 100;
		pRoomIndex->mana_rate = 100;
		pRoomIndex->guild = 0;

		for (; ;) {
			letter = fread_letter(fp);

			if (letter == 'S')
				break;

			switch (letter) {
			case 'H':       /* healing room */
				pRoomIndex->heal_rate = fread_number(fp);
				break;

			case 'M':       /* mana room */
				pRoomIndex->mana_rate = fread_number(fp);
				break;

			case 'C':       /* clan */
				pRoomIndex->clan = clan_lookup(fread_string(fp));
				break;

			case 'G':       /* guild */
				if (!(pRoomIndex->guild = class_lookup(fread_string(fp)) + 1)) {
					bug("Load_rooms: invalid class in guild", 0);
					exit(1);
				}

				break;

			case 'D':       /* door */
				door = fread_number(fp);

				if (door < 0 || door > 5) {
					bug("Fread_rooms: vnum %d has bad door number.", vnum);
					exit(1);
				}

				pexit = new Exit;
				pexit->description      = fread_string(fp);
				pexit->keyword          = fread_string(fp);
				pexit->exit_info        = 0;
				locks                   = fread_number(fp);
				pexit->key              = fread_number(fp);
				pexit->u1.vnum          = fread_number(fp);

				switch (locks) {
				case 1: pexit->exit_info = EX_ISDOOR;                   break;

				case 2: pexit->exit_info = EX_ISDOOR | EX_PICKPROOF;      break;

				case 3: pexit->exit_info = EX_ISDOOR | EX_NOPASS;         break;

				case 4: pexit->exit_info = EX_ISDOOR | EX_NOPASS | EX_PICKPROOF; break;
				}

				pRoomIndex->exit[door] = pexit;
				pRoomIndex->old_exit[door] = pexit;
				top_exit++;
				break;

			case 'E':       /* extended desc */
				ed = new_extra_descr();
				ed->keyword             = fread_string(fp);
				ed->description         = fread_string(fp);
				ed->next                = pRoomIndex->extra_descr;
				pRoomIndex->extra_descr = ed;
				top_ed++;
				break;

			case 'O':
				pRoomIndex->owner = fread_string(fp);
				break;

			default:
				bug("Load_rooms: vnum %d has flag not 'CDEHMOS'.", vnum);
				exit(1);
			}
		}

		iHash                   = vnum % MAX_KEY_HASH;
		pRoomIndex->next        = room_index_hash[iHash];
		room_index_hash[iHash]  = pRoomIndex;
		top_room++;
	}

	return;
}

/*
 * Snarf a shop section.
 */
void load_shops(FILE *fp)
{
	Shop *pShop;
	MobilePrototype *pMobIndex;
	int shopkeeper;
	int iTrade;

	for (; ;) {
		if ((shopkeeper = fread_number(fp)) == 0)
			break;

		pShop = new Shop;
		pShop->next             = NULL;
		pShop->version          = aVersion;
		pShop->keeper           = shopkeeper;

		for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
			pShop->buy_type[iTrade]     = fread_number(fp);

		pShop->profit_buy       = fread_number(fp);
		pShop->profit_sell      = fread_number(fp);
		pShop->open_hour        = fread_number(fp);
		pShop->close_hour       = fread_number(fp);
		fread_to_eol(fp);
		pMobIndex               = get_mob_index(pShop->keeper);
		pMobIndex->pShop        = pShop;

		if (shop_first == NULL)
			shop_first = pShop;

		if (shop_last  != NULL)
			shop_last->next = pShop;

		shop_last       = pShop;
		pShop->next     = NULL;
		top_shop++;
	}

	return;
}

/*
 * Snarf spec proc declarations.
 */
void load_specials(FILE *fp)
{
	MobilePrototype *pMobIndex;
	char letter;

	for (; ;) {
		switch (letter = fread_letter(fp)) {
		default:
			bug("Load_specials: letter '%c' not *MS.", letter);
			exit(1);

		case 'S':
			return;

		case '*':
			break;

		case 'M':
			pMobIndex           = get_mob_index(fread_number(fp));
			pMobIndex->spec_fun = spec_lookup(fread_word(fp));

			if (pMobIndex->spec_fun == 0) {
				bug("Load_specials: 'M': vnum %d.", pMobIndex->vnum);
				exit(1);
			}

			break;
		}

		fread_to_eol(fp);
	}
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits(void)
{
//    extern const sh_int rev_dir [];
//    char buf[MAX_STRING_LENGTH];
	RoomPrototype *pRoomIndex;
//    RoomPrototype *to_room;
	Exit *pexit;
//    Exit *pexit_rev;
	int iHash;
	int door;

	for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
		for (pRoomIndex  = room_index_hash[iHash];
		     pRoomIndex != NULL;
		     pRoomIndex  = pRoomIndex->next) {
			bool fexit;
			fexit = FALSE;

			for (door = 0; door <= 5; door++) {
				if ((pexit = pRoomIndex->exit[door]) != NULL) {
					if (pexit->u1.vnum <= 0
					    || get_room_index(pexit->u1.vnum) == NULL)
						pexit->u1.to_room = NULL;
					else {
						fexit = TRUE;
						pexit->u1.to_room = get_room_index(pexit->u1.vnum);
					}
				}
			}

			if (!fexit)
				SET_BIT(pRoomIndex->room_flags, ROOM_NO_MOB);
		}
	}

	/* nobody cares about the Fix_exits() messages -- Elrac
	  for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	  {
	      for ( pRoomIndex  = room_index_hash[iHash];
	            pRoomIndex != NULL;
	            pRoomIndex  = pRoomIndex->next )
	      {
	          for ( door = 0; door <= 5; door++ )
	          {
	              if ( ( pexit     = pRoomIndex->exit[door]       ) != NULL
	              &&   ( to_room   = pexit->u1.to_room            ) != NULL
	              &&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	              &&   pexit_rev->u1.to_room != pRoomIndex
	              &&   (pRoomIndex->vnum < 1200 || pRoomIndex->vnum > 1299))
	              {
	                  Format::sprintf( buf, "Fix_exits: %d:%d -> %d:%d -> %d.",
	                      pRoomIndex->vnum, door,
	                      to_room->vnum,    rev_dir[door],
	                      (pexit_rev->u1.to_room == NULL)
	                          ? 0 : pexit_rev->u1.to_room->vnum );
	                  bug( buf, 0 );
	              }
	          }
	      }
	  }
	*/
	return;
}

/*
 * Reports a bug.
 */
void bug(const String& str, int param)
{
	char buf[MAX_STRING_LENGTH];

	if (fpArea != NULL) {
		int iLine;
		int iChar;

		if (fpArea == stdin)
			iLine = 0;
		else {
			iChar = ftell(fpArea);
			fseek(fpArea, 0, 0);

			for (iLine = 0; ftell(fpArea) < iChar; iLine++) {
				while (getc(fpArea) != '\n')
					;
			}

			fseek(fpArea, iChar, 0);
		}

		Format::sprintf(buf, "[*****] FILE: %s LINE: %d", strArea, iLine);
		log_string(buf);
		/* RT removed because we don't want bugs shutting the mud
		        if ( ( fp = fopen( "shutdown.txt", "a" ) ) != NULL )
		        {
		            Format::fprintf( fp, "[*****] %s\n", buf );
		            fclose( fp );
		        }
		*/
	}

	strcpy(buf, "[*****] BUG: ");
	Format::sprintf(buf + strlen(buf), str.c_str(), param);
	log_string(buf);
	/* RT removed due to bug-file spamming
	    if ( ( fp = fopen( BUG_FILE, "a" ) ) != NULL )
	    {
	        Format::fprintf( fp, "%s\n", buf );
	        fclose( fp );
	    }
	*/
	wiznet(buf, NULL, NULL, WIZ_BUGS, 0, 0);
	return;
}

/*
 * Writes a string to the log.
 */
void log_string(const String& str)
{
	char *strtime;
	strtime                    = ctime(&current_time);
	strtime[strlen(strtime) - 1] = '\0';
	Format::fprintf(stderr, "%s :: %s\n", strtime, str);
	return;
}

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

int mprog_name_to_type(const String& name)
{
	if (name == "in_file_prog")    return IN_FILE_PROG;

	if (name == "act_prog")    return ACT_PROG;

	if (name == "speech_prog")    return SPEECH_PROG;

	if (name == "rand_prog")    return RAND_PROG;

	if (name == "boot_prog")    return BOOT_PROG;

	if (name == "fight_prog")    return FIGHT_PROG;

	if (name == "buy_prog")    return BUY_PROG;

	if (name == "hitprcnt_prog")    return HITPRCNT_PROG;

	if (name == "death_prog")    return DEATH_PROG;

	if (name == "entry_prog")    return ENTRY_PROG;

	if (name == "greet_prog")    return GREET_PROG;

	if (name == "all_greet_prog")    return ALL_GREET_PROG;

	if (name == "give_prog")    return GIVE_PROG;

	if (name == "bribe_prog")    return BRIBE_PROG;

	if (name == "tick_prog")    return TICK_PROG;

	return (ERROR_PROG);
}
/* This routine reads in scripts of MOBprograms from a file */

MobProg *mprog_file_read(const String& f, MobProg *mprg,
                            MobilePrototype *pMobIndex)
{
	char        MOBProgfile[ MAX_INPUT_LENGTH ];
	MobProg *mprg2;
	FILE       *progfile;
	char        letter;
	bool        done = FALSE;
	Format::sprintf(MOBProgfile, "%s%s", MOB_DIR, f);
	progfile = fopen(MOBProgfile, "r");

	if (!progfile) {
		bug("Mob: %d couldnt open mobprog file", pMobIndex->vnum);
		exit(1);
	}

	mprg2 = mprg;

	switch (letter = fread_letter(progfile)) {
	case '>':
		break;

	case '|':
		bug("empty mobprog file.", 0);
		exit(1);
		break;

	default:
		bug("in mobprog file syntax error.", 0);
		exit(1);
		break;
	}

	while (!done) {
		mprg2->type = mprog_name_to_type(fread_word(progfile));

		switch (mprg2->type) {
		case ERROR_PROG:
			bug("mobprog file type error", 0);
			exit(1);
			break;

		case IN_FILE_PROG:
			bug("mprog file contains a call to file.", 0);
			exit(1);
			break;

		default:
			pMobIndex->progtypes = pMobIndex->progtypes | mprg2->type;
			mprg2->arglist       = fread_string(progfile);
			mprg2->comlist       = fread_string(progfile);

			switch (letter = fread_letter(progfile)) {
			case '>':
				mprg2->next = new MobProg;
				mprg2       = mprg2->next;
				mprg2->next = NULL;
				break;

			case '|':
				done = TRUE;
				break;

			default:
				bug("in mobprog file syntax error.", 0);
				exit(1);
				break;
			}

			break;
		}
	}

	fclose(progfile);
	return mprg2;
}

/* This procedure is responsible for reading any in_file MOBprograms.
 */

void mprog_read_programs(FILE *fp, MobilePrototype *pMobIndex)
{
	MobProg *mprg;
	char        letter;
	bool        done = FALSE;

	if ((letter = fread_letter(fp)) != '>') {
		bug("Load_mobiles: vnum %d MOBPROG char", pMobIndex->vnum);
		exit(1);
	}

	pMobIndex->mobprogs = new MobProg;
	mprg = pMobIndex->mobprogs;

	while (!done) {
		mprg->type = mprog_name_to_type(fread_word(fp));

		switch (mprg->type) {
		case ERROR_PROG:
			bug("Load_mobiles: vnum %d MOBPROG type.", pMobIndex->vnum);
			exit(1);
			break;

		case IN_FILE_PROG:
			mprg = mprog_file_read(fread_string(fp), mprg, pMobIndex);
			fread_to_eol(fp);

			switch (letter = fread_letter(fp)) {
			case '>':
				mprg->next = new MobProg;
				mprg       = mprg->next;
				mprg->next = NULL;
				break;

			case '|':
				mprg->next = NULL;
				fread_to_eol(fp);
				done = TRUE;
				break;

			default:
				bug("Load_mobiles: vnum %d bad MOBPROG.", pMobIndex->vnum);
				exit(1);
				break;
			}

			break;

		default:
			pMobIndex->progtypes = pMobIndex->progtypes | mprg->type;
			mprg->arglist        = fread_string(fp);
			fread_to_eol(fp);
			mprg->comlist        = fread_string(fp);
			fread_to_eol(fp);

			switch (letter = fread_letter(fp)) {
			case '>':
				mprg->next = new MobProg;
				mprg       = mprg->next;
				mprg->next = NULL;
				break;

			case '|':
				mprg->next = NULL;
				fread_to_eol(fp);
				done = TRUE;
				break;

			default:
				bug("Load_mobiles: vnum %d bad MOBPROG.", pMobIndex->vnum);
				exit(1);
				break;
			}

			break;
		}
	}

	return;
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain(void)
{
	return;
}

