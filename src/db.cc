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

#include "file.hh"
#include "Game.hh"
#include "Area.hh"
#include "channels.hh"
#include "merc.hh"
#include "memory.hh"
#include "recycle.hh"
#include "sql.hh"
#include "lookup.hh"
#include "music.hh"
#include "Affect.hh"
#include "affect_list.hh"
#include "Auction.hh"
#include "Format.hh"
#include "GameTime.hh"
#include "Weather.hh"
#include "Shop.hh"
#include "MobProg.hh"
#include "Reset.hh"
#include "Disabled.hh"

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

/* quest_open, if true, says that the quest area is open.
   quest_min, quest_max are defined only if quest_open is true. */
/* quest_upk is restricted pk in quest area or not, quest_double is double qp time */
bool                    quest_open;
bool                    quest_upk;
long                    quest_double = 0;
int                     quest_min, quest_max;
/* startroom and area are initialized by quest_init() */
RoomPrototype         *quest_startroom;

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
Area *area_last; // currently loading area

/*
 * MOBprogram locals
*/

void boot_bug(const String& str, int param);

//int             mprog_name_to_type      args((const String& name));
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
			if (skill_table[sn].pgsn != nullptr)
				*skill_table[sn].pgsn = sn;
	}
	/* Read in all the area files */
	{
		FILE *fpList;

		if ((fpList = fopen(AREA_LIST, "r")) == nullptr) {
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

				if ((fpArea = fopen(abuf, "r")) == nullptr) {
					perror(strArea);
					exit(1);
				}
			}

			Format::printf("Now loading area: %s\n", strArea);

			for (; ;) {
				if (fread_letter(fpArea) != '#') {
					boot_bug("Boot_db: # not found.", 0);
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
					boot_bug("Boot_db: bad section name.", 0);
					exit(1);
				}
			}

			if (fpArea != stdin)
				fclose(fpArea);

			fpArea = nullptr;
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
//	area_update();
	Game::world().update();
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
 * Load an AREA section (only a header, really)
 */
void load_area(FILE *fp)
{
	area_last = new Area(fp);
	Game::world().areas.push_back(area_last);
}

/*
 * Snarf a reset section.
 */
void load_resets(FILE *fp)
{
	char letter;

	if (area_last == nullptr) {
		boot_bug("Load_resets: no #AREA seen yet.", 0);
		exit(1);
	}

	for (; ;) {
		if ((letter = fread_letter(fp)) == 'S')
			break;

		if (letter == '*') {
			fread_to_eol(fp);
			continue;
		}

		ungetc(letter, fp);
		Reset *pReset = new Reset(fp);
		area_last->resets.push_back(pReset);
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
			boot_bug("Load_mobiles: # not found.", 0);
			exit(1);
		}

		vnum                            = fread_number(fp);

		if (vnum == 0)
			break;

		if (vnum < area_last->min_vnum ||
		    vnum > area_last->max_vnum)
			boot_bug("mobile vnum %d out of range.", vnum);

		fBootDb = FALSE;

		if (get_mob_index(vnum) != nullptr) {
			boot_bug("Load_mobiles: vnum %d duplicated.", vnum);
			exit(1);
		}

		fBootDb = TRUE;
		pMobIndex = new MobilePrototype(fp, vnum);

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
			boot_bug("Load_objects: # not found.", 0);
			exit(1);
		}

		vnum                            = fread_number(fp);

		if (vnum == 0)
			break;

		if (vnum < area_last->min_vnum ||
		    vnum > area_last->max_vnum)
			boot_bug("object vnum %d out of range.", vnum);

		fBootDb = FALSE;

		if (get_obj_index(vnum) != nullptr) {
			boot_bug("Load_objects: vnum %d duplicated.", vnum);
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


		int val = 0; // prevent accidents in altering below switches
		switch (pObjIndex->item_type) {
		case ITEM_WEAPON:
			pObjIndex->value[val]         = ObjectValue(get_weapon_type(fread_word(fp)));
			break;

		case ITEM_KEY:
			pObjIndex->value[val]         = ObjectValue(fread_flag(fp));
			break;

		default:
			pObjIndex->value[val]         = ObjectValue(fread_number(fp));
			break;
		}


		val = 1;
		switch (pObjIndex->item_type) {
		case ITEM_CONTAINER:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
		case ITEM_PORTAL:
			pObjIndex->value[val]         = ObjectValue(fread_flag(fp));
			break;

		case ITEM_POTION:
		case ITEM_PILL:
		case ITEM_SCROLL:
			pObjIndex->value[val]         = ObjectValue(skill_lookup(fread_word(fp)));
			break;

		default:
			pObjIndex->value[val]         = ObjectValue(fread_number(fp));
			break;
		}


		val = 2;
		switch (pObjIndex->item_type) {
		case ITEM_DRINK_CON:
		case ITEM_FOUNTAIN:
			pObjIndex->value[val]         = ObjectValue(liq_lookup(fread_word(fp)));

			if (pObjIndex->value[val] == -1) {
				pObjIndex->value[val] = 0;
				boot_bug("Unknown liquid type", 0);
			}

			break;

		case ITEM_POTION:
		case ITEM_PILL:
		case ITEM_SCROLL:
			pObjIndex->value[val]         = ObjectValue(skill_lookup(fread_word(fp)));
			break;

		case ITEM_FURNITURE:
		case ITEM_PORTAL:
		case ITEM_ANVIL:
			pObjIndex->value[val]         = ObjectValue(fread_flag(fp));
			break;

		default:
			pObjIndex->value[val]         = ObjectValue(fread_number(fp));
			break;
		}


		val = 3;
		switch (pObjIndex->item_type) {
		case ITEM_WEAPON:
			pObjIndex->value[val]         = ObjectValue(attack_lookup(fread_word(fp)));
			break;

		case ITEM_WAND:
		case ITEM_STAFF:
		case ITEM_POTION:
		case ITEM_PILL:
		case ITEM_SCROLL:
			pObjIndex->value[val]         = ObjectValue(skill_lookup(fread_word(fp)));
			break;

		case ITEM_DRINK_CON:
		case ITEM_FOUNTAIN:
		case ITEM_FOOD:
			pObjIndex->value[val]         = ObjectValue(fread_flag(fp));
			break;

		default:
			pObjIndex->value[val]         = ObjectValue(fread_number(fp));
			break;
		}


		val = 4;
		switch (pObjIndex->item_type) {
		case ITEM_WEAPON:
			pObjIndex->value[val]         = ObjectValue(fread_flag(fp));
			break;

		case ITEM_POTION:
		case ITEM_PILL:
		case ITEM_SCROLL:
			pObjIndex->value[val]         = ObjectValue(skill_lookup(fread_word(fp)));
			break;

		default:
			pObjIndex->value[val]         = ObjectValue(fread_number(fp));
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
				af.bitvector(0);
				af.permanent          = TRUE;

				Flags bitvector = 0;
				if (affect_parse_flags('O', &af, bitvector)) {
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

				Flags bitvector    = fread_flag(fp);

				// do at least once even if no bitvector
				do {
					if (affect_parse_flags(letter, &af, bitvector)) {
						affect_copy_to_list(&pObjIndex->affected, &af); 
						top_affect++;

						// don't multiply the modifier, just apply to the first bit
						af.location = 0;
						af.modifier = 0;
					}

					af.type = 0; // reset every time
				} while (!bitvector.empty());
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

	if (area_last == nullptr) {
		boot_bug("Load_resets: no #AREA seen yet.", 0);
		exit(1);
	}

	for (; ;) {
		letter                          = fread_letter(fp);

		if (letter != '#') {
			boot_bug("Load_rooms: # not found.", 0);
			exit(1);
		}

		vnum                            = fread_number(fp);

		if (vnum == 0)
			break;

		if (vnum < area_last->min_vnum ||
		    vnum > area_last->max_vnum)
			boot_bug("room   vnum %d out of range.", vnum);

		fBootDb = FALSE;

		if (get_room_index(vnum) != nullptr) {
			boot_bug("Load_rooms: vnum %d duplicated.", vnum);
			exit(1);
		}

		fBootDb = TRUE;
		pRoomIndex = new RoomPrototype;
		pRoomIndex->version             = aVersion;
		pRoomIndex->people              = nullptr;
		pRoomIndex->contents            = nullptr;
		pRoomIndex->extra_descr         = nullptr;
		pRoomIndex->area                = area_last;
		pRoomIndex->vnum                = vnum;
		pRoomIndex->name                = fread_string(fp);
		pRoomIndex->description         = fread_string(fp);
		pRoomIndex->tele_dest           = fread_number(fp);
		pRoomIndex->room_flags          = fread_flag(fp);

		/* horrible hack */
		if (3000 <= vnum && vnum < 3400)
			pRoomIndex->room_flags += ROOM_LAW;

		pRoomIndex->sector_type         = fread_number(fp);
		pRoomIndex->light               = 0;

		for (door = 0; door <= 5; door++)
			pRoomIndex->exit[door] = nullptr;

		if (GET_ROOM_FLAGS(pRoomIndex).has(ROOM_FEMALE_ONLY)) {
			Format::sprintf(log_buf, "Room %d is FEMALE_ONLY", pRoomIndex->vnum);
			log_string(log_buf);
		}

		if (GET_ROOM_FLAGS(pRoomIndex).has(ROOM_MALE_ONLY)) {
			Format::sprintf(log_buf, "Room %d is MALE_ONLY", pRoomIndex->vnum);
			log_string(log_buf);
		}

		if (GET_ROOM_FLAGS(pRoomIndex).has(ROOM_LOCKER)) {
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
					boot_bug("Load_rooms: invalid class in guild", 0);
					exit(1);
				}

				break;

			case 'D':       /* door */
				door = fread_number(fp);

				if (door < 0 || door > 5) {
					boot_bug("Fread_rooms: vnum %d has bad door number.", vnum);
					exit(1);
				}

				pexit = new Exit;
				pexit->description      = fread_string(fp);
				pexit->keyword          = fread_string(fp);
				pexit->exit_flags        = Flags::none;
				locks                   = fread_number(fp);
				pexit->key              = fread_number(fp);
				pexit->u1.vnum          = fread_number(fp);

				switch (locks) {
				case 1: pexit->exit_flags = EX_ISDOOR;                   break;

				case 2: pexit->exit_flags = EX_ISDOOR | EX_PICKPROOF;      break;

				case 3: pexit->exit_flags = EX_ISDOOR | EX_NOPASS;         break;

				case 4: pexit->exit_flags = EX_ISDOOR | EX_NOPASS | EX_PICKPROOF; break;
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
				boot_bug("Load_rooms: vnum %d has flag not 'CDEHMOS'.", vnum);
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
		pShop->next             = nullptr;
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

		if (shop_first == nullptr)
			shop_first = pShop;

		if (shop_last  != nullptr)
			shop_last->next = pShop;

		shop_last       = pShop;
		pShop->next     = nullptr;
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
			boot_bug("Load_specials: letter '%c' not *MS.", letter);
			exit(1);

		case 'S':
			return;

		case '*':
			break;

		case 'M':
			pMobIndex           = get_mob_index(fread_number(fp));
			pMobIndex->spec_fun = spec_lookup(fread_word(fp));

			if (pMobIndex->spec_fun == 0) {
				boot_bug("Load_specials: 'M': vnum %d.", pMobIndex->vnum);
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
//    char buf[MAX_STRING_LENGTH];
	RoomPrototype *pRoomIndex;
//    RoomPrototype *to_room;
	Exit *pexit;
//    Exit *pexit_rev;
	int iHash;
	int door;

	for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
		for (pRoomIndex  = room_index_hash[iHash];
		     pRoomIndex != nullptr;
		     pRoomIndex  = pRoomIndex->next) {
			bool fexit;
			fexit = FALSE;

			for (door = 0; door <= 5; door++) {
				if ((pexit = pRoomIndex->exit[door]) != nullptr) {
					if (pexit->u1.vnum <= 0
					    || get_room_index(pexit->u1.vnum) == nullptr)
						pexit->u1.to_room = nullptr;
					else {
						fexit = TRUE;
						pexit->u1.to_room = get_room_index(pexit->u1.vnum);
					}
				}
			}

			if (!fexit)
				pRoomIndex->room_flags += ROOM_NO_MOB;
		}
	}

	/* nobody cares about the Fix_exits() messages -- Elrac
	  for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	  {
	      for ( pRoomIndex  = room_index_hash[iHash];
	            pRoomIndex != nullptr;
	            pRoomIndex  = pRoomIndex->next )
	      {
	          for ( door = 0; door <= 5; door++ )
	          {
	              if ( ( pexit     = pRoomIndex->exit[door]       ) != nullptr
	              &&   ( to_room   = pexit->u1.to_room            ) != nullptr
	              &&   ( pexit_rev = to_room->exit[Exit::rev_dir(door)] ) != nullptr
	              &&   pexit_rev->u1.to_room != pRoomIndex
	              &&   (pRoomIndex->vnum < 1200 || pRoomIndex->vnum > 1299))
	              {
	                  Format::sprintf( buf, "Fix_exits: %d:%d -> %d:%d -> %d.",
	                      pRoomIndex->vnum, door,
	                      to_room->vnum,    Exit::rev_dir(door),
	                      (pexit_rev->u1.to_room == nullptr)
	                          ? 0 : pexit_rev->u1.to_room->vnum );
	                  boot_bug( buf, 0 );
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
void boot_bug(const String& str, int param)
{
	if (fpArea != nullptr) {
		int iLine = 0;
		int iChar = 0;

		if (fpArea != stdin) {
			iChar = ftell(fpArea);
			fseek(fpArea, 0, 0);

			for (iLine = 0; ftell(fpArea) < iChar; iLine++) {
				while (getc(fpArea) != '\n')
					;
			}

			fseek(fpArea, iChar, 0);
		}

		Logging::bugf("[*****] FILE: %s LINE: %d", strArea, iLine);
	}

	Logging::bugf(str, param);
}

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

Flags::Bit mprog_name_to_type(const String& name)
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
		boot_bug("Mob: %d couldnt open mobprog file", pMobIndex->vnum);
		exit(1);
	}

	mprg2 = mprg;

	switch (letter = fread_letter(progfile)) {
	case '>':
		break;

	case '|':
		boot_bug("empty mobprog file.", 0);
		exit(1);
		break;

	default:
		boot_bug("in mobprog file syntax error.", 0);
		exit(1);
		break;
	}

	while (!done) {
		mprg2->type = mprog_name_to_type(fread_word(progfile));

		switch (mprg2->type) {
		case ERROR_PROG:
			boot_bug("mobprog file type error", 0);
			exit(1);
			break;

		case IN_FILE_PROG:
			boot_bug("mprog file contains a call to file.", 0);
			exit(1);
			break;

		default:
			pMobIndex->progtype_flags = pMobIndex->progtype_flags + mprg2->type;
			mprg2->arglist       = fread_string(progfile);
			mprg2->comlist       = fread_string(progfile);

			switch (letter = fread_letter(progfile)) {
			case '>':
				mprg2->next = new MobProg;
				mprg2       = mprg2->next;
				mprg2->next = nullptr;
				break;

			case '|':
				done = TRUE;
				break;

			default:
				boot_bug("in mobprog file syntax error.", 0);
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
		boot_bug("Load_mobiles: vnum %d MOBPROG char", pMobIndex->vnum);
		exit(1);
	}

	pMobIndex->mobprogs = new MobProg;
	mprg = pMobIndex->mobprogs;

	while (!done) {
		mprg->type = mprog_name_to_type(fread_word(fp));

		switch (mprg->type) {
		case ERROR_PROG:
			boot_bug("Load_mobiles: vnum %d MOBPROG type.", pMobIndex->vnum);
			exit(1);
			break;

		case IN_FILE_PROG:
			mprg = mprog_file_read(fread_string(fp), mprg, pMobIndex);
			fread_to_eol(fp);

			switch (letter = fread_letter(fp)) {
			case '>':
				mprg->next = new MobProg;
				mprg       = mprg->next;
				mprg->next = nullptr;
				break;

			case '|':
				mprg->next = nullptr;
				fread_to_eol(fp);
				done = TRUE;
				break;

			default:
				boot_bug("Load_mobiles: vnum %d bad MOBPROG.", pMobIndex->vnum);
				exit(1);
				break;
			}

			break;

		default:
			pMobIndex->progtype_flags = pMobIndex->progtype_flags | mprg->type;
			mprg->arglist        = fread_string(fp);
			fread_to_eol(fp);
			mprg->comlist        = fread_string(fp);
			fread_to_eol(fp);

			switch (letter = fread_letter(fp)) {
			case '>':
				mprg->next = new MobProg;
				mprg       = mprg->next;
				mprg->next = nullptr;
				break;

			case '|':
				mprg->next = nullptr;
				fread_to_eol(fp);
				done = TRUE;
				break;

			default:
				boot_bug("Load_mobiles: vnum %d bad MOBPROG.", pMobIndex->vnum);
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

