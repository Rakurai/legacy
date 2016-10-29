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
#include "sql.h"
#include "music.h"
#include "affect.h"

/* command procedures needed */
DECLARE_DO_FUN(do_quit);
DECLARE_DO_FUN(do_afk);
DECLARE_DO_FUN(do_hgratz);
DECLARE_DO_FUN(talk_auction);
DECLARE_DO_FUN(do_look);

extern void new_day(void);

extern void     squestmob_found args((CHAR_DATA *ch, CHAR_DATA *mob));
extern           time_t                  reboot_time;

/*
 * Local functions.
 */
int     hit_gain        args((CHAR_DATA *ch));
int     mana_gain       args((CHAR_DATA *ch));
int     stam_gain       args((CHAR_DATA *ch));
void    mobile_update   args((void));
void    weather_update  args((void));
void    char_update     args((void));
void    descrip_update  args((void));
void    obj_update      args((void));
void    room_update     args((void));
void    aggr_update     args((void));
void    quest_update    args((void));
void    underwater_update    args((void));
void    janitor_update  args((void));

/* used for saving */

int     save_number = 0;

void demote_level(CHAR_DATA *ch)
{
	int sub_hp, sub_mana, sub_stam, sub_prac, sub_train = 1;
	ch->pcdata->last_level = get_play_hours(ch);
	sub_hp          = UMAX(1, con_app[get_curr_stat(ch, STAT_CON)].hitp + number_range(
	                               class_table[ch->class].hp_min, class_table[ch->class].hp_max));
	sub_mana        = UMAX(1, int_app[get_curr_stat(ch, STAT_INT)].manap + number_range(
	                               class_table[ch->class].mana_min, class_table[ch->class].mana_max));
	sub_stam        = UMAX(1, str_app[get_curr_stat(ch, STAT_STR)].stp + number_range(
	                               class_table[ch->class].stam_min, class_table[ch->class].stam_max));
	sub_prac        = wis_app[get_curr_stat(ch, STAT_WIS)].practice;

	if (chr_app[get_curr_stat(ch, STAT_CHR)].chance >= number_percent())
		sub_train = 2;

	ch->max_hit             -= sub_hp;
	ch->max_mana            -= sub_mana;
	ch->max_stam            -= sub_stam;
	ch->practice            -= sub_prac;
	ch->train               -= sub_train;
	ch->pcdata->perm_hit    -= sub_hp;
	ch->pcdata->perm_mana   -= sub_mana;
	ch->pcdata->perm_stam   -= sub_stam;
	ptc(ch, "{RYour loss is: %d/%d hp, %d/%d ma, %d/%d stm, %d/%d prac, %d/%d train.{x\n",
	    sub_hp,         ch->max_hit,
	    sub_mana,       ch->max_mana,
	    sub_stam,       ch->max_stam,
	    sub_prac,       ch->practice,
	    sub_train,      ch->train);

	/* don't let stats go below 20/100/100 */
	if (ch->max_hit < 20)                   ch->max_hit = 20;

	if (ch->max_mana < 100)                 ch->max_mana = 100;

	if (ch->max_stam < 100)                 ch->max_stam = 100;

	if (ch->pcdata->perm_hit < 20)          ch->pcdata->perm_hit = 20;

	if (ch->pcdata->perm_mana < 100)        ch->pcdata->perm_mana = 100;

	if (ch->pcdata->perm_stam < 100)        ch->pcdata->perm_stam = 100;
}

void advance_level(CHAR_DATA *ch)
{
	int add_hp, add_mana, add_stam, add_prac, add_train = 1;
	ch->pcdata->last_level = get_play_hours(ch);
	add_hp          = UMAX(1, con_app[get_curr_stat(ch, STAT_CON)].hitp + number_range(
	                               class_table[ch->class].hp_min, class_table[ch->class].hp_max));
	add_mana        = UMAX(1, int_app[get_curr_stat(ch, STAT_INT)].manap + number_range(
	                               class_table[ch->class].mana_min, class_table[ch->class].mana_max));
	add_stam        = UMAX(1, str_app[get_curr_stat(ch, STAT_STR)].stp + number_range(
	                               class_table[ch->class].stam_min, class_table[ch->class].stam_max));
	add_prac        = wis_app[get_curr_stat(ch, STAT_WIS)].practice;

	if (chr_app[get_curr_stat(ch, STAT_CHR)].chance >= number_percent())
		add_train = 2;

	/* old calculations:
	   add_mana    = (2*get_curr_stat(ch,STAT_INT) + get_curr_stat(ch,STAT_WIS)) / 5;
	   add_move    = number_range(1, (get_curr_stat(ch,STAT_CON) + get_curr_stat(ch,STAT_DEX))/6 );
	*/
	ch->max_hit             += add_hp;
	ch->max_mana            += add_mana;
	ch->max_stam            += add_stam;
	ch->practice            += add_prac;
	ch->train               += add_train;
	ch->pcdata->perm_hit    += add_hp;
	ch->pcdata->perm_mana   += add_mana;
	ch->pcdata->perm_stam   += add_stam;
	ptc(ch, "Your gain is: %d/%d hp, %d/%d ma, %d/%d stm, %d/%d prac, %d/%d train.\n",
	    add_hp,         ch->max_hit,
	    add_mana,       ch->max_mana,
	    add_stam,       ch->max_stam,
	    add_prac,       ch->practice,
	    add_train,      ch->train);
}

/*
This function advances a NPC. This is to advance a pet's
level. This function is likely to be called directly after advance_level().
-- Outsider
*/
void npc_advance_level(CHAR_DATA *ch)
{
	int add_hit, add_mana, add_stam;

	if (! IS_NPC(ch))
		return;

	add_hit = get_curr_stat(ch, STAT_CON);
	add_mana = get_curr_stat(ch, STAT_INT);
	add_stam = get_curr_stat(ch, STAT_STR);
	ch->max_hit += add_hit;
	ch->max_mana += add_mana;
	ch->max_stam += add_stam;
}

void gain_exp(CHAR_DATA *ch, int gain)
{
	char buf[MSL];

	if (IS_NPC(ch))
		return;

	if (ch->level >= LEVEL_HERO)
		return;

	ch->exp = UMAX(exp_per_level(ch, ch->pcdata->points), ch->exp + gain);

	while ((ch->exp >= exp_per_level(ch, ch->pcdata->points) * (ch->level + 1))
	       && (ch->level < LEVEL_HERO)) {
		stc("{PG{RR{YA{bT{GZ{H!  {BWay {Nto {Vlevel{M!!{x\n", ch);
		/* up the player's level. If hero, add fireworks. */
		ch->level++;

		if (ch->level == LEVEL_HERO) {
			DESCRIPTOR_DATA *d;
			CHAR_DATA *victim;
			static char *msg =
			        "{PGGGGGG  RRRRRR    AAAA   TTTTTTT  ZZZZZZ\n"
			        "{YGG  GG  RR  RRR  AAAAAA  TTTTTTT  ZZZZZZ\n"
			        "{GGG      RR  RRR  AA  AA    TTT      ZZZ \n"
			        "{CGG GGG  RRRRRR   AAAAAA    TTT     ZZZ  \n"
			        "{BGG  GG  RR  RRR  AA  AA    TTT    ZZZZZZ\n"
			        "{VGGGGGG  RR   RR  AA  AA    TTT    ZZZZZZ{x\n";
			extern void restore_char(CHAR_DATA *, CHAR_DATA *);
			stc("{MGratz from all the Immortals of Legacy!{x\n\n", ch);

			for (d = descriptor_list; d; d = d->next)
				if (IS_PLAYING(d)) {
					victim = d->original ? d->original : d->character;
					stc(msg, victim);
					restore_char(ch, victim);
				}

			SET_CGROUP(ch, GROUP_HERO);
		}       /* end of if hero */

		/* add avatar group if level 80 */
		if (ch->level == LEVEL_AVATAR)
			SET_CGROUP(ch, GROUP_AVATAR);

		sprintf(buf, "%s has attained level %d!", ch->name, ch->level);
		wiznet(buf, ch, NULL, WIZ_LEVELS, 0, 0);
		do_send_announce(ch, buf);
		advance_level(ch);
		// Do this below -- save_char_obj(ch);

		/* If character has a pet, see if the pet should gain a level.
		   Pets gain a level for every two levels gained by the PC.
		   -- Outsider
		*/
		if (ch->pet) {
			if ((ch->level > ch->pet->level) && (ch->level % 2)) {
				ch->pet->level++;
				npc_advance_level(ch->pet);
				stc("Your pet gains a level!\n", ch);
			}
		}    /* end of we have a pet */

		save_char_obj(ch);
	}        /* end of while leveling */

	tail_chain();
}

/*
 * Regeneration stuff.
 */
int hit_gain(CHAR_DATA *ch)
{
	int gain;
	int number;

	if (ch->in_room == NULL)
		return 0;

	if (IS_NPC(ch)) {
		gain =  5 + ch->level;

		if (affect_flag_on_char(ch, AFF_REGENERATION))
			gain *= 2;

		switch (get_position(ch)) {
		default :           gain /= 2;                      break;

		case POS_SLEEPING:  gain = 3 * gain / 2;              break;

		case POS_RESTING:                                   break;

		case POS_FIGHTING:  gain /= 3;                      break;
		}
	}
	else {
		gain = UMAX(3, get_curr_stat(ch, STAT_CON) - 3 + ch->level / 2);
		gain += class_table[ch->class].hp_max - 10;
		number = number_percent();

		if (number < get_skill(ch, gsn_fast_healing)) {
			gain += number * gain / 100;

			if (ch->hit < ch->max_hit)
				check_improve(ch, gsn_fast_healing, TRUE, 8);
		}

		switch (get_position(ch)) {
		default:            gain /= 4;                      break;

		case POS_SLEEPING:                                  break;

		case POS_RESTING:   gain /= 2;                      break;

		case POS_FIGHTING:  gain /= 6;                      break;
		}

		if (ch->pcdata->condition[COND_HUNGER] == 0)
			gain /= 2;

		if (ch->pcdata->condition[COND_THIRST] == 0)
			gain /= 2;
	}

	gain = gain * ch->in_room->heal_rate / 100;

	if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
		gain = gain * ch->on->value[3] / 100;

	if (affect_flag_on_char(ch, AFF_POISON))
		gain /= 4;

	if (affect_flag_on_char(ch, AFF_PLAGUE))
		gain /= 8;

	if (affect_flag_on_char(ch, AFF_HASTE) && ch->race != 8) // faeries, ugly hack, fix later -- Montrey (2014)
		gain /= 2 ;

	if (affect_flag_on_char(ch, AFF_SLOW))
		gain *= 2 ;

	if (affect_flag_on_char(ch, AFF_REGENERATION))
		gain *= 2;

	if (affect_flag_on_char(ch, AFF_DIVINEREGEN))
		gain *= 4;

	return UMIN(gain, ch->max_hit - ch->hit);
}

int mana_gain(CHAR_DATA *ch)
{
	int gain, number;

	if (ch->in_room == NULL)
		return 0;

	if (IS_NPC(ch)) {
		gain = 5 + ch->level;

		switch (get_position(ch)) {
		default:                gain /= 2;              break;

		case POS_SLEEPING:      gain = 3 * gain / 2;      break;

		case POS_RESTING:                               break;

		case POS_FIGHTING:      gain /= 3;              break;
		}
	}
	else {
		gain = (get_curr_stat(ch, STAT_WIS) + get_curr_stat(ch, STAT_INT) + ch->level) / 2;
		number = number_percent();

		if (number < get_skill(ch, gsn_meditation)) {
			gain += number * gain / 100;

			if (ch->mana < ch->max_mana)
				check_improve(ch, gsn_meditation, TRUE, 8);
		}

		/* compare to mages mana regen, mages get full (class 0) */
		gain -= gain * (class_table[0].mana_max - class_table[ch->class].mana_max)
		        / class_table[0].mana_max;

		switch (get_position(ch)) {
		default:                gain /= 4;      break;

		case POS_SLEEPING:                      break;

		case POS_RESTING:       gain /= 2;      break;

		case POS_FIGHTING:      gain /= 6;      break;
		}

		if (ch->pcdata->condition[COND_HUNGER] == 0)
			gain /= 2;

		if (ch->pcdata->condition[COND_THIRST] == 0)
			gain /= 2;
	}

	gain = gain * ch->in_room->mana_rate / 100;

	if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
		gain = gain * ch->on->value[4] / 100;

	if (affect_flag_on_char(ch, AFF_POISON))
		gain /= 4;

	if (affect_flag_on_char(ch, AFF_PLAGUE))
		gain /= 8;

	if (affect_flag_on_char(ch, AFF_HASTE) && ch->race != 8) // faeries, ugly hack, fix later -- Montrey (2014)
		gain /= 2;

	if (affect_flag_on_char(ch, AFF_SLOW))
		gain *= 2;

	if (affect_flag_on_char(ch, AFF_DIVINEREGEN))
		gain *= 2;

	return UMIN(gain, ch->max_mana - ch->mana);
}

int stam_gain(CHAR_DATA *ch)
{
	int gain;

	if (ch->in_room == NULL)
		return 0;

	if (IS_NPC(ch)) {
		gain = 5 + ch->level;

		switch (get_position(ch)) {
		default:                gain /= 2;              break;

		case POS_SLEEPING:      gain = 3 * gain / 2;      break;

		case POS_RESTING:                               break;

		case POS_FIGHTING:      gain /= 3;              break;
		}
	}
	else {
		gain = get_curr_stat(ch, STAT_CON) + get_curr_stat(ch, STAT_DEX) + (ch->level / 2);
		/* compare to warrior stamina regen, warriors get full (class 3) */
		gain -= gain * (class_table[3].stam_max - class_table[ch->class].stam_max)
		        / class_table[3].stam_max;

		switch (get_position(ch)) {
		default:                gain /= 4;      break;

		case POS_SLEEPING:                      break;

		case POS_RESTING:       gain /= 2;      break;

		case POS_FIGHTING:      gain /= 6;      break;
		}

		if (ch->pcdata->condition[COND_HUNGER] == 0)
			gain /= 2;

		if (ch->pcdata->condition[COND_THIRST] == 0)
			gain /= 2;
	}

	gain = gain * ch->in_room->heal_rate / 100;

	if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
		gain = gain * ch->on->value[3] / 100;

	if (affect_flag_on_char(ch, AFF_POISON))
		gain /= 4;

	if (affect_flag_on_char(ch, AFF_PLAGUE))
		gain /= 8;

	if (affect_flag_on_char(ch, AFF_HASTE) && ch->race != 8) // faeries, ugly hack, fix later -- Montrey (2014)
		gain /= 3;

	if (affect_flag_on_char(ch, AFF_SLOW))
		gain *= 2;

	if (affect_flag_on_char(ch, AFF_REGENERATION))
		gain *= 2;

	if (affect_flag_on_char(ch, AFF_DIVINEREGEN))
		gain *= 2;

	return UMIN(gain, ch->max_stam - ch->stam);
}

void gain_condition(CHAR_DATA *ch, int iCond, int value)
{
	int condition;

	if (value == 0 || IS_NPC(ch) || ch->pcdata == NULL)
		return;

	if (IS_IMMORTAL(ch)) {
		if (iCond == COND_HUNGER)
			ch->pcdata->condition[iCond] = 100;

		if (iCond == COND_THIRST)
			ch->pcdata->condition[iCond] = 100;

		return;
	}

	if ((condition = ch->pcdata->condition[iCond]) == -1)
		return;

	ch->pcdata->condition[iCond] = URANGE(0, condition + value, 48);

	if (!IS_SET(ch->pcdata->plr, PLR_CHATMODE)) {
		if (ch->pcdata->condition[iCond] == 0) {
			switch (iCond) {
			case COND_HUNGER:
				stc("You are hungry.\n", ch);
				break;

			case COND_THIRST:
				stc("You are thirsty.\n", ch);
				break;

			case COND_DRUNK:
				if (condition != 0)
					stc("You are sober.\n", ch);

				break;
			}
		}         /* end of condition == 0 */
	}   /* end of player in chat mode */
}

/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update(void)
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;
	EXIT_DATA *pexit;
	int door;

	/* Examine all mobs. */
	for (ch = char_list; ch != NULL; ch = ch_next) {
		ch_next = ch->next;

		if (!IS_NPC(ch) || ch->in_room == NULL || affect_flag_on_char(ch, AFF_CHARM))
			continue;

		if (get_position(ch) <= POS_SITTING)
			continue;

		/* Why check for resting mobiles? */

		if (ch->in_room->area->empty)
			/* && !IS_SET(ch->act,ACT_UPDATE_ALWAYS)) */
			continue;

		/* Examine call for special procedure */
		if (ch->spec_fun != 0) {
			if ((*ch->spec_fun)(ch))
				continue;
		}

		if (ch->pIndexData->pShop != NULL)
			if ((ch->gold * 100 + ch->silver) < ch->pIndexData->wealth) {
				ch->gold += ch->pIndexData->wealth * number_range(1, 20) / 5000000;
				ch->silver += ch->pIndexData->wealth * number_range(1, 20) / 50000;
			}

		/* MOBprogram random trigger */
		if (ch->in_room->area->nplayer > 0) {
			mprog_random_trigger(ch);

			/* If ch dies or changes
			position due to it's random
			trigger, continue - Kahn */
			if (get_position(ch) < POS_STANDING)
				continue;
		}

		/* That's all for sleeping / busy monster, and empty zones */
		if (get_position(ch) != POS_STANDING)
			continue;

		/* Scavenge */
		if (IS_SET(ch->act, ACT_SCAVENGER)
		    && ch->in_room->contents != NULL
		    && number_bits(6) == 0) {
			CHAR_DATA *gch;
			OBJ_DATA *obj;
			OBJ_DATA *obj_best = 0;
			bool not_used;
			int max = 1;

			for (obj = ch->in_room->contents; obj; obj = obj->next_content) {
				not_used = TRUE;

				for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
					if (gch->on == obj)
						not_used = FALSE;

				if (CAN_WEAR(obj, ITEM_TAKE) && can_loot(ch, obj) && obj->cost > max && not_used) {
					obj_best = obj;
					max = obj->cost;
				}
			}

			if (obj_best) {
				obj_from_room(obj_best);
				obj_to_char(obj_best, ch);
				act("$n gets $p.", ch, obj_best, NULL, TO_ROOM);
			}
		}

		/* Wander */
		if (!IS_SET(ch->act, ACT_SENTINEL)
		    && number_bits(3) == 0
		    && (door = number_bits(5)) <= 5
		    && (pexit = ch->in_room->exit[door]) != NULL
		    &&   pexit->u1.to_room != NULL
		    &&   !IS_SET(pexit->exit_info, EX_CLOSED)
		    &&   !IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB)
		    && (!IS_SET(ch->act, ACT_STAY_AREA)
		        ||   pexit->u1.to_room->area == ch->in_room->area)
		    && (!IS_SET(ch->act, ACT_OUTDOORS)
		        ||   !IS_SET(pexit->u1.to_room->room_flags, ROOM_INDOORS))
		    && (!IS_SET(ch->act, ACT_INDOORS)
		        ||   IS_SET(pexit->u1.to_room->room_flags, ROOM_INDOORS))) {
			move_char(ch, door, FALSE);

			/* If ch changes position due
			to it's or someother mob's
			movement via MOBProgs,
			continue - Kahn */
			if (get_position(ch) < POS_STANDING)
				continue;
		}
	}

	return;
}

/*
 * Update the weather.
 */
void weather_update(void)
{
	char buf[MSL];
	CHAR_DATA *ch;
	int diff;
	buf[0] = '\0';

	switch (++time_info.hour) {
	case  5:
		new_day();
		weather_info.sunlight = SUN_LIGHT;
		strcat(buf, "The Clocktower Bell rings as another day begins.\n");
		break;

	case  6:
		weather_info.sunlight = SUN_RISE;
		strcat(buf, "The sun rises in the east.\n");
		break;

	case 12:
		strcat(buf, "The Clocktower Bell signals the midday.\n");
		break;

	case 19:
		weather_info.sunlight = SUN_SET;
		strcat(buf, "The sun slowly disappears in the west.\n");
		break;

	case 20:
		weather_info.sunlight = SUN_DARK;
		strcat(buf, "The night has begun.\n");
		break;

	case 24:
		strcat(buf, "The Clocktower Bell tolls, declaring midnight.\n");
		time_info.hour = 0;
		time_info.day++;
		break;
	}

	if (time_info.day >= MUD_MONTH) {
		time_info.day = 0;
		time_info.month++;
	}

	if (time_info.month >= MUD_YEAR) {
		time_info.month = 0;
		time_info.year++;
	}

	/*
	 * Weather change.
	 */
	if (time_info.month >= 9 && time_info.month <= 16) diff = weather_info.mmhg >  985 ? -2 : 2;
	else diff = weather_info.mmhg > 1015 ? -2 : 2;

	weather_info.change     += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
	weather_info.change     = UMAX(weather_info.change, -12);
	weather_info.change     = UMIN(weather_info.change, 12);
	weather_info.mmhg       += weather_info.change;
	weather_info.mmhg       = UMAX(weather_info.mmhg, 960);
	weather_info.mmhg       = UMIN(weather_info.mmhg, 1040);

	switch (weather_info.sky) {
	default:
		bug("Weather_update: bad sky %d.", weather_info.sky);
		weather_info.sky = SKY_CLOUDLESS;
		break;

	case SKY_CLOUDLESS:
		if (weather_info.mmhg < 990 || (weather_info.mmhg < 1010 && number_bits(2) == 0)) {
			strcat(buf, "The sky grows dark with rolling grey clouds.\n");
			weather_info.sky = SKY_CLOUDY;
		}

		break;

	case SKY_CLOUDY:
		if (weather_info.mmhg < 970 || (weather_info.mmhg < 990 && number_bits(2) == 0)) {
#ifdef SEASON_CHRISTMAS
			strcat(buf, "Snow starts to fall from the sky.\n");
#else
			strcat(buf, "It starts to rain a heavy downpour.\n");
#endif
			weather_info.sky = SKY_RAINING;
		}

		if (weather_info.mmhg > 1030 && number_bits(2) == 0) {
			strcat(buf, "Shafts of light cut through the dense clouds above.\n");
			weather_info.sky = SKY_CLOUDLESS;
		}

		break;

	case SKY_RAINING:
		if (weather_info.mmhg < 970 && number_bits(2) == 0) {
			strcat(buf, "Lightning flashes in the sky.\n");
			weather_info.sky = SKY_LIGHTNING;
		}

		if (weather_info.mmhg > 1030 || (weather_info.mmhg > 1010 && number_bits(2) == 0)) {
#ifdef SEASON_CHRISTMAS
			strcat(buf, "The snow slows to a few flakes, and finally stops.\n");
#else
			strcat(buf, "The rain stopped.\n");
#endif
			weather_info.sky = SKY_CLOUDY;
		}

		break;

	case SKY_LIGHTNING:
		if (weather_info.mmhg > 1010 || (weather_info.mmhg >  990 && number_bits(2) == 0)) {
			strcat(buf, "The lightning has stopped.\n");
			weather_info.sky = SKY_RAINING;
			break;
		}

		break;
	}

	if (buf[0] != '\0') {
		for (ch = char_list; ch != NULL; ch = ch->next)

			/* why send it to mobs? */
			if (!IS_NPC(ch) && IS_OUTSIDE(ch) && IS_AWAKE(ch) && IS_SET(ch->act, PLR_TICKS))
				stc(buf, ch);
	}
}

/* Update all descriptors, handles login timer */
void descrip_update(void)
{
	DESCRIPTOR_DATA *d, *d_next;
	CHAR_DATA *ch;

	for (d = descriptor_list; d != NULL; d = d_next) {
		d_next = d->next;
		++d->timer;

		if (IS_PLAYING(d)) {
			ch = d->character;

			if (IS_IMMORTAL(ch) || get_duel(ch)) {
				if (d->timer > 99)
					d->timer = 99;

				continue;
			}

			if (d->timer >= 12) {
				if (ch->was_in_room == NULL && ch->in_room != NULL) {
					ch->was_in_room = ch->in_room;

					if (ch->fighting != NULL)
						stop_fighting(ch, TRUE);

					act("$n disappears into the void...", ch, NULL, NULL, TO_ROOM);
					stc("You disappear into the void.\n", ch);

					if (ch->level > 1)
						save_char_obj(ch);

					char_from_room(ch);
					char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO));
				}
			}

			if (d->timer == 7) {
				if (!IS_SET(ch->comm, COMM_AFK)) {
					act("$n is set to auto-afk...", ch, NULL, NULL, TO_ROOM);
					do_afk(ch, "{CA{Tuto-{CA{Tfk by {BL{Ce{gg{Wa{Cc{By{x");
				}
			}
		}

		if (d->timer > 30) {
			close_socket(d);
			continue;
		}
	}
}

/*
 * Update all chars, including mobs.
*/
void char_update(void)
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;
	CHAR_DATA *ch_quit;
	ch_quit     = NULL;
	/* update save counter */
	save_number++;

	if (save_number > 29)
		save_number = 0;

	for (ch = char_list; ch != NULL; ch = ch_next) {
		ch_next = ch->next;

		if (!IS_IMMORTAL(ch) && !char_in_duel_room(ch)) {
			if (ch->desc != NULL)
				if (ch->desc->timer > 30)
					ch_quit = ch;

			if (ch->timer > 30 && ch_quit == NULL)
				ch_quit = ch;
		}

		/* Autotick stuff - Lotus */
		if (!IS_NPC(ch) && IS_SET(ch->act, PLR_TICKS))
			stc("{Btick...{x\n", ch);

		if (get_position(ch) >= POS_STUNNED) {
			/* Nectimer for Necromancy spells */
			if (IS_NPC(ch) && (ch->nectimer > 0)) {
				ch->nectimer -= 1;

				if (ch->nectimer <= 0) {
					act("$n vanishes back into the nether void.", ch, NULL, NULL,
					    TO_ROOM);
					extract_char(ch, TRUE);
					continue;
				}
			}

			/* If the player had a familiar and it is gone (probably dead)
			   then the player loses the ability score benefit of the
			   familiar. -- Outsider
			*/
			if (!IS_NPC(ch) && ch->pcdata->familiar) {
				if (! ch->pet) {
					ch->pcdata->familiar -= 1;   /* remove +1 to stat position */
					ch->mod_stat[ch->pcdata->familiar] -= 1;

					/* make sure we don't go below the min ability score */
					if ((ch->perm_stat[ch->pcdata->familiar] + ch->mod_stat[ch->pcdata->familiar]) < 3)
						ch->mod_stat[ch->pcdata->familiar] += 1;

					/* make sure we don't go above the max ability score */
					if ((ch->perm_stat[ch->pcdata->familiar] + ch->mod_stat[ch->pcdata->familiar]) > 25)
						ch->mod_stat[ch->pcdata->familiar] -= 1;

					/* remove familiar */
					ch->pcdata->familiar = 0;
				}
			}    /* end of removed familiar */

			/* If the character is a Paladin and not an NPC
			   then check their Lay on Hands status. -- Outsider */
			if (!IS_NPC(ch) && ch->class == PALADIN_CLASS) {
				/* keep it in the limits */
				if (ch->pcdata->lays > 10) ch->pcdata->lays = 10;

				if (ch->pcdata->lays < 0) ch->pcdata->lays = 0;

				if (ch->pcdata->next_lay_countdown > NEW_LAY_COUNTER)
					ch->pcdata->next_lay_countdown = NEW_LAY_COUNTER;

				ch->pcdata->next_lay_countdown--;

				if (ch->pcdata->next_lay_countdown < 1) {
					ch->pcdata->lays = (ch->level / 10) + 1;
					ch->pcdata->next_lay_countdown = NEW_LAY_COUNTER;
				}
			}

			if (ch->hit  < ch->max_hit)
				ch->hit  += hit_gain(ch);
			else
				ch->hit = ch->max_hit;

			if (ch->mana < ch->max_mana)
				ch->mana += mana_gain(ch);
			else
				ch->mana = ch->max_mana;

			if (ch->stam < ch->max_stam)
				ch->stam += stam_gain(ch);
			else
				ch->stam = ch->max_stam;
		}

		if (get_position(ch) == POS_STUNNED)
			update_pos(ch);

		if (!IS_NPC(ch) && ch->level < LEVEL_IMMORTAL) {
			OBJ_DATA *obj;

			if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL
			    &&   obj->item_type == ITEM_LIGHT
			    &&   obj->value[2] > 0) {
				if (--obj->value[2] == 0 && ch->in_room != NULL) {
					if (ch->in_room->light > 0)
						--ch->in_room->light;

					act("$p goes out.  Hope you're not afraid of the dark.", ch, obj, NULL, TO_ROOM);
					act("$p flickers and goes out.", ch, obj, NULL, TO_CHAR);
					extract_obj(obj);
				}
				else if (obj->value[2] <= 5 && ch->in_room != NULL)
					act("$p hisses and sparks.", ch, obj, NULL, TO_CHAR);
			}

			/* This timer is here in case a player goes linkdead - Lotus */
			if (ch->desc == NULL) {
				ch->timer++;

				if (IS_IMMORTAL(ch) || char_in_duel_room(ch)) {
					if (ch->timer > 99)
						ch->timer = 99;
				}
				else if (ch->timer >= 12 && ch->was_in_room == NULL && ch->in_room != NULL) {
					ch->was_in_room = ch->in_room;

					if (ch->fighting != NULL)
						stop_fighting(ch, TRUE);

					act("$n disappears into the void...", ch, NULL, NULL, TO_ROOM);
					stc("You disappear into the void.\n", ch);

					if (ch->level > 1)
						save_char_obj(ch);

					char_from_room(ch);
					char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO));
				}
			}

			gain_condition(ch, COND_DRUNK,  -1);
			gain_condition(ch, COND_FULL, ch->size > SIZE_MEDIUM ? -4 : -3);

			/* remort affects - extra thirsty, extra hungry, no thirst, no hunger */
			if (HAS_RAFF(ch, RAFF_NOHUNGER))
				gain_condition(ch, COND_HUNGER, 0);
			else if (HAS_RAFF(ch, RAFF_EXTRAHUNGRY))
				gain_condition(ch, COND_HUNGER, ch->size > SIZE_MEDIUM ? -4 : -2);
			else
				gain_condition(ch, COND_HUNGER, ch->size > SIZE_MEDIUM ? -2 : -1);

			if (HAS_RAFF(ch, RAFF_NOTHIRST))
				gain_condition(ch, COND_THIRST, 0);
			else if (HAS_RAFF(ch, RAFF_EXTRATHIRSTY))
				gain_condition(ch, COND_THIRST, -2);
			else
				gain_condition(ch, COND_THIRST, -1);

			/* Check killer flag - Clerve */
			if (ch->pcdata->flag_killer == 0 && IS_SET(ch->act, PLR_KILLER)) {
				REMOVE_BIT(ch->act, PLR_KILLER);
				REMOVE_BIT(ch->act, PLR_NOPK);
				stc("The urge to kill dimishes.\n", ch);
				stc("You are no longer a KILLER.\n", ch);
				save_char_obj(ch);
			}
			else if (ch->pcdata->flag_killer > 0 && IS_SET(ch->act, PLR_KILLER))
				ch->pcdata->flag_killer--;

			/* Check thief flag - Clerve */
			if (ch->pcdata->flag_thief == 0 && IS_SET(ch->act, PLR_THIEF)) {
				REMOVE_BIT(ch->act, PLR_THIEF);
				REMOVE_BIT(ch->act, PLR_NOPK);
				stc("The urge to steal dimishes.\n", ch);
				stc("You are no longer a THIEF.\n", ch);
				save_char_obj(ch);
			}
			else if (ch->pcdata->flag_thief > 0 && IS_SET(ch->act, PLR_THIEF))
				ch->pcdata->flag_thief--;
		}

		// print the affects that are wearing off.  this is complicated because
		// we may have more than one affect that is part of the same group, and
		// we also don't want to print a 'wearing off' message for affects that
		// are duplicated (for some reason).  so, the hackish solution is to sort
		// the list twice: once by duration, and then by skill number.  this
		// should get all the grouped spells together.
		// this will usually already be sorted this way, unless it was sorted by
		// duration for the show_affects player command, so only O(n) hit here.
		affect_sort_char(ch, affect_comparator_duration);
		affect_sort_char(ch, affect_comparator_type);

		for (const AFFECT_DATA *paf = affect_list_char(ch); paf; paf = paf->next) {
			if (paf->duration == 0) {
				if (paf->next == NULL
				 || paf->next->type != paf->type
				 || paf->next->duration > 0) {
					if (paf->type > 0 && skill_table[paf->type].msg_off)
						ptc(ch, "%s\n", skill_table[paf->type].msg_off);
				}
			}
		}

		// now remove spells with duration 0
		AFFECT_DATA pattern;
		pattern.duration = 0;
		affect_remove_matching_from_char(ch, affect_comparator_duration, &pattern);

		// decrement duration and sometimes decrement level.  this is done after
		// the wearing off of spells with duration 0, because we use -1 to mean
		// indefinite and players are used to having spell counters go down to 0
		// before they wear off.
		affect_iterate_over_char(ch, affect_fn_fade_spell, NULL);

		/* MOBprogram tick trigger -- Montrey */
		if (IS_NPC(ch)) {
			mprog_tick_trigger(ch);

			/* If ch dies or changes position
			   due to it's tick trigger, continue */
			if (get_position(ch) <= POS_STUNNED)
				continue;
		}

		/*
		 * Careful with the damages here,
		 *   MUST NOT refer to ch after damage taken,
		 *   as it may be lethal damage (on NPC).
		 */

		if (ch != NULL && affect_flag_on_char(ch, gsn_plague)) {
		 	const AFFECT_DATA *plague = affect_find_in_char(ch, gsn_plague);

			act("$n writhes in agony as plague sores erupt from $s skin.",
			    ch, NULL, NULL, TO_ROOM);
			stc("You writhe in agony from the plague.\n", ch);

			spread_plague(ch->in_room, plague, 4);

			int dam = UMIN(ch->level, plague->level / 5 + 1);
			ch->mana -= dam;
			ch->stam -= dam;
			damage(ch, ch, dam, gsn_plague, DAM_DISEASE, FALSE, TRUE);
		}
		else if (affect_flag_on_char(ch, AFF_POISON) && ch != NULL
		         &&   !affect_flag_on_char(ch, AFF_SLOW)) {
			const AFFECT_DATA *poison;
			poison = affect_find_in_char(ch, gsn_poison);

			if (poison != NULL) {
				act("$n shivers and suffers.", ch, NULL, NULL, TO_ROOM);
				stc("You shiver and suffer.\n", ch);
				damage(ch, ch, poison->level / 10 + 1, gsn_poison,
				       DAM_POISON, FALSE, TRUE);
			}
		}
		else if (get_position(ch) == POS_INCAP && number_range(0, 1) == 0)
			damage(ch->fighting ? ch->fighting : ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, FALSE, FALSE);
		else if (get_position(ch) == POS_MORTAL)
			damage(ch->fighting ? ch->fighting : ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, FALSE, FALSE);
	}

	/*
	 * Autosave and autoquit.
	 * Check that these chars still exist.
	 */
	for (ch = char_list; ch != NULL; ch = ch_next) {
		ch_next = ch->next;

		if (ch->desc != NULL && ch->desc->descriptor % 30 == save_number)
			save_char_obj(ch);

		if (ch == ch_quit)
			do_quit(ch, "now");
	}

	return;
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update(void)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	for (obj = object_list; obj != NULL; obj = obj_next) {
		CHAR_DATA *rch;
		char *message;
		obj_next = obj->next;

		// TODO: this sorting could be eliminated or reduced if we just keep a
		// boolean value that is set true when affects are added or resorted.

		// print the affects that are wearing off.  this is complicated because
		// we may have more than one affect that is part of the same group, and
		// we also don't want to print a 'wearing off' message for affects that
		// are duplicated (for some reason).  so, the hackish solution is to sort
		// the list twice: once by duration, and then by skill number.  this
		// should get all the grouped spells together.
		affect_sort_obj(obj, affect_comparator_duration);
		affect_sort_obj(obj, affect_comparator_type);

		for (const AFFECT_DATA *paf = affect_list_obj(obj); paf; paf = paf->next) {
			if (paf->duration == 0) {
				if (paf->next == NULL
				 || paf->next->type != paf->type
				 || paf->next->duration > 0) {
					/* for addapplied objects with a duration */
					if (paf->type == 0) {
						if (obj->carried_by != NULL) {
							rch = obj->carried_by;
							act("The magic of $p diminishes.", rch, obj, NULL, TO_CHAR);
						}

						if (obj->in_room != NULL && obj->in_room->people != NULL) {
							rch = obj->in_room->people;
							act("The magic of $p diminishes.", rch, obj, NULL, TO_ALL);
						}
					}

					if (paf->type > 0 && skill_table[paf->type].msg_obj) {
						if (obj->carried_by != NULL) {
							rch = obj->carried_by;
							act(skill_table[paf->type].msg_obj, rch, obj, NULL, TO_CHAR);
						}

						if (obj->in_room != NULL && obj->in_room->people != NULL) {
							rch = obj->in_room->people;
							act(skill_table[paf->type].msg_obj, rch, obj, NULL, TO_ALL);
						}
					}
				}
			}
		}

		// now remove spells with duration 0
		AFFECT_DATA pattern;
		pattern.duration = 0;
		affect_remove_matching_from_obj(obj, affect_comparator_duration, &pattern);

		// decrement duration and sometimes decrement level.  this is done after
		// the wearing off of spells with duration 0, because we use -1 to mean
		// indefinite and players are used to having spell counters go down to 0
		// before they wear off.
		affect_iterate_over_obj(obj, affect_fn_fade_spell, NULL);

		/* do not decay items being auctioned -- Elrac */
		if (obj == auction->item)
			continue;

		if (obj->timer <= 0 || --obj->timer > 0)
			if (obj->clean_timer <= 0 || --obj->clean_timer > 0)
				continue;

		switch (obj->item_type) {
		default:                message = "$p crumbles into ashes.";                            break;

		case ITEM_FOUNTAIN:     message = "$p turns to dust at your feet.";                     break;

		case ITEM_CORPSE_NPC:   message = "$p decays into dust.";                               break;

		case ITEM_CORPSE_PC:    message = "$p is carried off by ravenous vultures.";            break;

			if (obj->contains)
				message = "$p is carried off by ravenous vultures, leaving its contents on the ground.";
			else
				message = "$p is carried off by ravenous vultures.";

			break;

		case ITEM_FOOD:         message = "$p decays into nothing.";                            break;

		case ITEM_POTION:       message = "$p shatters, messily spilling its unused contents."; break;

		case ITEM_PORTAL:       message = "$p implodes with a rush of air and is gone.";        break;

		case ITEM_CONTAINER:
			if (CAN_WEAR(obj, ITEM_WEAR_FLOAT))
				message = "$p flickers and vanishes.";
			else
				message = "$p crumbles into dust.";

			break;
		}

		if (obj->carried_by != NULL) {
			if (IS_NPC(obj->carried_by) && obj->carried_by->pIndexData->pShop != NULL)
				obj->carried_by->silver += obj->cost / 5;
			else if (!obj->in_obj) { /* don't send messages if it's in another object */
				act(message, obj->carried_by, obj, NULL, TO_CHAR);

				if (get_eq_char(obj->carried_by, WEAR_FLOAT) == obj) {
					act(message, obj->carried_by, obj, NULL, TO_ROOM);

					if (obj->contains)
						act("$p spills its contents on the ground.", obj->carried_by, obj, NULL, TO_ALL);
				}
			}
		}
		else if (obj->in_room != NULL && (rch = obj->in_room->people) != NULL && !obj->in_obj)
			act(message, rch, obj, NULL, TO_ALL);

		/* save the contents, but not npc corpses */
		if (obj->item_type != ITEM_CORPSE_NPC && obj->contains) {
			OBJ_DATA *t_obj, *next_obj;

			for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj) {
				next_obj = t_obj->next_content;
				obj_from_obj(t_obj);

				if (obj->in_obj)                                /* in another object */
					obj_to_obj(t_obj, obj->in_obj);
				else if (obj->carried_by) {                     /* carried */
					if (get_eq_char(obj->carried_by, WEAR_FLOAT) == obj) {
						if (obj->carried_by->in_room == NULL)
							extract_obj(t_obj);
						else
							obj_to_room(t_obj, obj->carried_by->in_room);
					}
					else
						obj_to_char(t_obj, obj->carried_by);
				}
				else if (obj->in_room == NULL)                  /* destroy it */
					extract_obj(t_obj);
				else                                            /* to a room */
					obj_to_room(t_obj, obj->in_room);
			}
		}

		extract_obj(obj);
	}
}

/* Update all rooms -- Montrey */
void room_update(void)
{
	ROOM_INDEX_DATA *room;
	int x;

	for (x = 1; x < 32600; x++) {
		if ((room = get_room_index(x)) == NULL)
			continue;

		// print the affects that are wearing off.  this is complicated because
		// we may have more than one affect that is part of the same group, and
		// we also don't want to print a 'wearing off' message for affects that
		// are duplicated (for some reason).  so, the hackish solution is to sort
		// the list twice: once by duration, and then by skill number.  this
		// should get all the grouped spells together.
		affect_sort_room(room, affect_comparator_duration);
		affect_sort_room(room, affect_comparator_type);

		for (const AFFECT_DATA *paf = affect_list_room(room); paf; paf = paf->next) {
			if (paf->duration == 0) {
				if (paf->next == NULL
				 || paf->next->type != paf->type
				 || paf->next->duration > 0) {
					/* there is no msg_room for spells, so we'll use msg_obj for
					   room affect spells.  might change this later, but i really
					   don't feel like adding another ,"" to all those entries
					   right now :P -- Montrey */
					if (paf->type > 0 && skill_table[paf->type].msg_obj && room->people)
						act(skill_table[paf->type].msg_obj, NULL, NULL, NULL, TO_ALL);
				}
			}
		}

		// now remove spells with duration 0
		AFFECT_DATA pattern;
		pattern.duration = 0;
		affect_remove_matching_from_room(room, affect_comparator_duration, &pattern);

		// decrement duration and sometimes decrement level.  this is done after
		// the wearing off of spells with duration 0, because we use -1 to mean
		// indefinite and players are used to having spell counters go down to 0
		// before they wear off.
		affect_iterate_over_room(room, affect_fn_fade_spell, NULL);
	}
}

/*
 * Aggression -- Elrac
 *
 * This is an attempt at faster, yet fair aggression.
 *
 * We loop thru the descriptor list to build a uniqued
 * list of rooms containing players. Then we loop through
 * the mobs in these rooms to find a random aggressive
 * one and through the players to find a random one
 * susceptible to aggression. Then we hit him :)
 *
 * NOTE: Pursuant to an ancient hack, this code also handles
 *       mobprog updates. Do not forget this.
 */

bool eligible_aggressor(CHAR_DATA *ch)
{
	return (IS_NPC(ch)
	        && IS_AWAKE(ch)
	        && IS_SET(ch->act, ACT_AGGRESSIVE | ACT_AGGR_ALIGN)
	        && ch->fighting == NULL
	        && !affect_flag_on_char(ch, AFF_CALM | AFF_CHARM)
	       );
}

bool eligible_victim(CHAR_DATA *ch)
{
	if (IS_NPC(ch))
		return FALSE;

	if (ch->level >= LEVEL_IMMORTAL)
		return FALSE;

//	if (ch->on && ch->on->pIndexData->item_type == ITEM_COACH)
//		return FALSE;

	return TRUE;
}

void aggr_update(void)
{
	DESCRIPTOR_DATA *d;
	int player_count;
	int jroom, room_count;
	int jvictim, victim_count, victim_num;
	int jmob, mob_count, mob_num;
	ROOM_INDEX_DATA **room_list;
	ROOM_INDEX_DATA *room;
	CHAR_DATA *ch, *plr, *mob, *victim;
	bool duplicate;
	/* Count players. There can't possibly be more player-
	   inhabited rooms than there are players. */
	player_count = 0;

	for (d = descriptor_list; d != NULL; d = d->next)
		player_count++;

	/* allocate memory for pointers to <player_count> rooms */
	room_list = alloc_mem(player_count * sizeof(ROOM_INDEX_DATA *));

	for (jroom = 0; jroom < player_count; jroom++)
		room_list[jroom] = NULL;

	/* build a list of rooms, no duplicates */
	room_count = 0;

	for (d = descriptor_list; d != NULL; d = d->next) {
		if (!IS_PLAYING(d) ||
		    d->original  != NULL)
			continue;

		plr = d->character;

		if (IS_NPC(plr)          ||
		    plr->in_room == NULL)
			continue;

		duplicate = FALSE;

		for (jroom = 0; jroom < room_count; jroom++)
			if (room_list[jroom] == plr->in_room)
				duplicate = TRUE;

		if (duplicate == FALSE)
			room_list[room_count++] = plr->in_room;
	}

	/* for each room match up an aggressive mob and aggressable player */
	for (jroom = 0; jroom < room_count; jroom++) {
		room = room_list[jroom];

		/* We can parasite skill quest mobs in here, since the player just needs
		   to be in the same room as the mob for it to react.  Done before
		   mobprog and aggression checking to prevent other mobs jumping the
		   player before they can complete their quest, may change for realism, tho -- Montrey */
		for (ch = room->people; ch != NULL; ch = ch->next_in_room) {
			if (!IS_NPC(ch)
			    && IS_SET(ch->pcdata->plr, PLR_SQUESTOR)
			    && ch->pcdata->squestmob != NULL
			    && ch->pcdata->squestobj == NULL) {
				/* look for quest mob */
				for (mob = room->people; mob != NULL; mob = mob->next_in_room) {
					if (IS_NPC(mob) && mob == ch->pcdata->squestmob && can_see(mob, ch)) {
						squestmob_found(ch, mob);
						break;
					}
				}
			}
		}

		/* do mobprog updates where required.
		   This has nothing to do with aggression but parasitizes on our
		   list of player-inhabited rooms and the loop thereover. */
		for (ch = room->people; ch != NULL; ch = ch->next_in_room) {
			if (IS_NPC(ch) && ch->mpactnum > 0) {
				MPROG_ACT_LIST *tmp_act, *tmp2_act;

				for (tmp_act = ch->mpact; tmp_act != NULL;
				     tmp_act = tmp_act->next) {
					mprog_wordlist_check(tmp_act->buf, ch, tmp_act->ch,
					                     tmp_act->obj, tmp_act->vo, ACT_PROG);
					free_string(tmp_act->buf);
				}

				for (tmp_act = ch->mpact; tmp_act != NULL; tmp_act = tmp2_act) {
					tmp2_act = tmp_act->next;
					free_mem(tmp_act, sizeof(MPROG_ACT_LIST));
				}

				ch->mpactnum = 0;
				ch->mpact    = NULL;
			}
		}

		/* no aggression in safe rooms */
		if (IS_SET(room->room_flags, ROOM_SAFE) || IS_SET(room->room_flags, ROOM_LAW))
			continue;

		/* only aggression below this point */
		/* count mobs and players in room for random selection */
		mob_count = 0;
		victim_count = 0;

		for (ch = room->people; ch != NULL; ch = ch->next_in_room)
			if (eligible_aggressor(ch))
				mob_count++;
			else if (eligible_victim(ch))
				victim_count++;

		/* any chance of aggression? */
		if (mob_count == 0 || victim_count == 0)
			continue;

		/* select a random aggressor/victim pair */
		mob_num = number_range(1, mob_count);
		victim_num = number_range(1, victim_count);
		/* find the (numerically) selected mob and victim */
		jmob = 0;
		mob = NULL;
		jvictim = 0;
		victim = NULL;

		for (ch = room->people; ch != NULL; ch = ch->next_in_room) {
			if (eligible_aggressor(ch)) {
				if (++jmob == mob_num)
					mob = ch;
			}
			else if (eligible_victim(ch)) {
				if (++jvictim == victim_num)
					victim = ch;
			}
		}

		if (mob == NULL || victim == NULL) {
			bug("aggr_update: mob or victim NULL!", 0);
			return;
		}

		/* final checks before the fight starts */
		if ((IS_SET(mob->act, ACT_WIMPY) && IS_AWAKE(victim))
		    || !can_see(mob, victim))
			continue;

		if (mob->level < victim->level - 5)
			continue;

		if (IS_SET(mob->act, ACT_AGGR_ALIGN)) {
			if (IS_NEUTRAL(victim))
				continue;

			if (IS_GOOD(mob) && !IS_EVIL(victim))
				continue;

			if (IS_EVIL(mob) && !IS_GOOD(victim))
				continue;
		}

		if ((get_curr_stat(victim, STAT_CHR) + number_range(0, 1))
		    > (get_curr_stat(mob, STAT_CHR) + number_range(0, 3)))
			continue;

		/* rumble! */
		multi_hit(mob, victim, TYPE_UNDEFINED);
	}

	/* release temp memory */
	free_mem(room_list, player_count * sizeof(ROOM_INDEX_DATA *));
} /* end aggr_update() */

void tele_update(void)
{
	CHAR_DATA *ch, *ch_next;
	ROOM_INDEX_DATA *pRoomIndex;

	for (ch = char_list; ch != NULL; ch = ch_next) {
		ch_next = ch->next;

		if (ch->in_room == NULL)
			continue;

		if (IS_SET(ch->in_room->room_flags, ROOM_TELEPORT)) {
			do_look(ch, "tele");

			if (ch->in_room->tele_dest == 0)
				pRoomIndex = get_random_room(ch);
			else
				pRoomIndex = get_room_index(ch->in_room->tele_dest);

			char_from_room(ch);
			char_to_room(ch, pRoomIndex);
			act("$n slowly fades into existence.\n", ch, NULL, NULL, TO_ROOM);
			do_look(ch, "auto");
		}
	}
}

void auction_update(void)
{
	char buf[MAX_STRING_LENGTH];

	if (auction->item == NULL) {
		/* no auction in progress -- return doing nothing */
		return;
	}

	if (--auction->pulse <= 0) { /* decrease pulse */
		auction->pulse = PULSE_AUCTION;

		switch (++auction->going) { /* increase the going state */
		case 1 : /* going once */
		case 2 : /* going twice */
			if (auction->bet > 0)
				sprintf(buf, "%s: going %s for %d gold.\n",
				        auction->item->short_descr,
				        ((auction->going == 1) ? "once" : "twice"), auction->bet);
			else
				sprintf(buf, "%s: going %s (no bet received yet).\n",
				        auction->item->short_descr,
				        ((auction->going == 1) ? "once" : "twice"));

			talk_auction(NULL, buf);
			break;

		case 3 : /* SOLD! */
			if (auction->bet > 0) {
				sprintf(buf, "AUCTION: %s sold to $n for %d gold.\n",
				        auction->item->short_descr, auction->bet);
				global_act(auction->buyer, buf, TRUE,
				           YELLOW, COMM_NOAUCTION | COMM_QUIET);
				sprintf(buf, "AUCTION: %s sold to $N for %d gold.\n",
				        auction->item->short_descr, auction->bet);
				wiznet(buf, auction->buyer, NULL, WIZ_AUCTION, 0,
				       GET_RANK(auction->buyer));
				obj_to_char(auction->item, auction->buyer);
				act("The auctioneer appears before you in a puff of smoke and hands you $p.",
				    auction->buyer, auction->item, NULL, TO_CHAR);
				act("The auctioneer appears before $n, and hands $m $p",
				    auction->buyer, auction->item, NULL, TO_ROOM);
				/* deduct_cost(auction->seller,-auction->bet); */ /* give him the money */
				auction->seller->gold += auction->bet;
				auction->item = NULL; /* reset item */
			}
			else { /* not sold */
				sprintf(buf,
				        "No bets received for %s - object has been removed.\n",
				        auction->item->short_descr);
				talk_auction(NULL, buf);
				act("The auctioneer appears before you to return $p to you.",
				    auction->seller, auction->item, NULL, TO_CHAR);
				act("The auctioneer appears before $n to return $p to $m.",
				    auction->seller, auction->item, NULL, TO_ROOM);
				obj_to_char(auction->item, auction->seller);
				auction->item = NULL; /* clear auction */
			} /* else */
		} /* switch */
	} /* if */
} /* auction_update() */

/*
 * All players age by 1 second here. -- Elrac
 * I trust this will be less error-prone than the previous method,
 * which has been known to produce wild fluctuations.
 * Mobs are not aged. It would take a lot of CPU time and contribute
 * nothing significant to realism.
 */
void age_update(void)
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *wch;

	/* leech our quest_double counter here too */
	if (quest_double)
		quest_double++;

	for (d = descriptor_list; d; d = d->next) {
		if (!IS_PLAYING(d))
			continue;

		wch = (d->original != NULL) ? d->original : d->character;

		if (!IS_NPC(wch))
			wch->pcdata->played++;

		/* Mud has crashed on the above line before - 8-28-98 */
		/* hopefully fixed, IS_PLAYING checks for both connected and null character -- Montrey */
	}
}

/* Okay, this is odd.  I don't know why, but all of a sudden mobiles stopped
   updating their wait timers.  I looked all over, and found where it's
   decremented, in game_loop_unix inside the descriptor loop.  Now, mobiles
   do not have descriptors.  I searched an old copy of the code and found the
   same, just in case I had changed it and forgotten.  No luck.  I don't see
   why it ever worked, I don't know why it changed suddenly, I must have done
   something wrong and can't figure out what.  Now mobs don't get up from
   bashing, and can't do *anything* once their wait is set.  This is the
   solution:  update_handler is called every pulse from game_loop_unix.
   I'm taking the wait and daze decrements out of the decriptor update in
   there and moving them here, loop over all characters and decrement.  This
   function is called from update_handler.  Maybe someday I'll figure out what
   went wrong, until then, this seems to be the best course of action.
                                                -- Montrey */
void wait_update(void)
{
	CHAR_DATA *ch;

	for (ch = char_list; ch != NULL; ch = ch->next) {
		if (ch->daze > 0)       --ch->daze;

		if (ch->wait > 0)       --ch->wait;
	}
}

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler(void)
{
	static int      pulse_area,
	       pulse_quest,
	       pulse_mobile,
	       pulse_violence,
	       pulse_point,
	       pulse_music,
	       pulse_tele,
	       pulse_underwater,
	       pulse_age,
//	       pulse_tour,
	       pulse_duel,
	       pulse_janitor;
//	       pulse_mysql_upd;

	if (--pulse_area        <= 0)   { pulse_area    = PULSE_AREA;           area_update();  }

	if (--pulse_quest       <= 0)   { pulse_quest   = PULSE_QUEST;          quest_update(); }

	if (--pulse_music       <= 0)   { pulse_music   = PULSE_MUSIC;          song_update();  }

//	if (--pulse_tour        <= 0)   { pulse_tour    = PULSE_TOURHALFSTEP;   tour_update();  }

	if (--pulse_mobile      <= 0)   { pulse_mobile  = PULSE_MOBILE;         mobile_update();}

	if (--pulse_violence    <= 0)   { pulse_violence = PULSE_VIOLENCE;       violence_update();}

	if (--pulse_duel        <= 0)   { pulse_duel    = PULSE_PER_SECOND;     duel_update();}

	if (--pulse_tele        <= 0)   { pulse_tele    = PULSE_TELEPORT;       tele_update();  }

	if (--pulse_underwater  <= 0)   { pulse_underwater = PULSE_UNDERWATER;    underwater_update();}

	if (--pulse_age         <= 0)   { pulse_age     = PULSE_PER_SECOND;     age_update();   }

	if (--pulse_janitor     <= 0)   { pulse_janitor = PULSE_JANITOR;        janitor_update();}

//	if (--pulse_mysql_upd <= 0) { pulse_mysql_upd = PULSE_MYSQL_UPD; db_update();}

	if (--pulse_point       <= 0) {
		pulse_point = PULSE_TICK;
		weather_update();
		char_update();
		descrip_update();
		obj_update();
		room_update();

		if (reboot_time != 0) {
			if (current_time == reboot_time) {
				int count = 0;
				DESCRIPTOR_DATA *d;

				for (d = descriptor_list; d != NULL; d = d->next)
					if (IS_PLAYING(d))
						count++;

				if (count == 0) {
					DESCRIPTOR_DATA *d_next;
					extern bool merc_down;
					log_string("AUTO-REBOOT");
//					do_sysinfo("The system is going down for auto-reboot NOW.\n");
					merc_down = TRUE;

					for (d = descriptor_list; d != NULL; d = d_next) {
						d_next = d->next;
						save_char_obj(d->character);
						close_socket(d);
					}
				}
				else
					reboot_time += 300;     /* try again in 5 minutes */
			}
		}
	}

	wait_update();
	auction_update();
	aggr_update();
	tail_chain();
} /* end update_handler() */

/* worldwide cleanup of objects */
void janitor_update()
{
	CHAR_DATA *rch;
	OBJ_DATA *obj;

	if (port != DIZZYPORT)
		return;

	for (obj = object_list; obj; obj = obj->next) {
		if (!obj->in_room
		    || obj->contains
		    || obj->timer
		    || obj->clean_timer)
			continue;

		if (obj->reset && obj->reset->command == 'O')
			if (obj->reset->arg3 == obj->in_room->vnum)
				continue;

		for (rch = obj->in_room->people; rch; rch = rch->next_in_room)
			if (!IS_NPC(rch))
				break;

		if (rch)        /* found a player in the room */
			continue;

		switch (obj->item_type) {       /* leave these types alone */
		case ITEM_FURNITURE:
		case ITEM_MONEY:
		case ITEM_CORPSE_PC:
		case ITEM_FOUNTAIN:
		case ITEM_PORTAL:
		case ITEM_JUKEBOX:
		case ITEM_ANVIL:
//		case ITEM_COACH:
		case ITEM_WEDDINGRING:
		case ITEM_TOKEN:
			continue;
		}

		if (chance(10))
			obj->clean_timer = number_range(80, 240);       /* 1 to 3 hours */
	}

//	bugf("janitor_update: %d items marked for cleanup", count);
	save_items();
}

void underwater_update(void)
{
	CHAR_DATA *ch, *ch_next;
	int skill, dam;

	for (ch = char_list; ch != NULL; ch = ch_next) {
		ch_next = ch->next;

		if (!IS_NPC(ch) && IS_SET(ch->in_room->room_flags, ROOM_UNDER_WATER)) {
			skill = get_skill(ch, gsn_swimming);

			if (skill == 100)
				stc("You would be {Cdrowning{x if not for your underwater breathing skill.\n", ch);
			else {
				/* a drink of water */
				gain_condition(ch, COND_THIRST,
				               liq_table[0].liq_affect[4] * liq_table[0].liq_affect[COND_THIRST] / 10);
				gain_condition(ch, COND_FULL,
				               liq_table[0].liq_affect[4] * liq_table[0].liq_affect[COND_FULL] / 4);
				dam = (ch->hit * (100 - skill)) / 400;

				if (ch->hit > 100) {
					stc("{CYou are drowning!!!{x\n", ch);

					if (skill > 0) {
						stc("{HYour skill helps slow your drowning.{x\n", ch);
						check_improve(ch, gsn_swimming, TRUE, 1);
					}

					damage(ch->fighting ? ch->fighting : ch, ch, dam, gsn_swimming, DAM_WATER, FALSE, TRUE);
				}
				else {
					stc("{PYou cannot hold your breath any more!{x\n", ch);
					stc("{CYour lungs fill with water and you lose consciousness...{x\n", ch);
					damage(ch->fighting ? ch->fighting : ch, ch, ch->hit + 15, gsn_swimming, DAM_WATER, FALSE, TRUE);
				}
			}
		}
	}
}

void quest_update(void)
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *ch;

	for (d = descriptor_list; d != NULL; d = d->next) {
		if (IS_PLAYING(d)) {
			ch = d->character;

			if (IS_NPC(ch))
				continue;

			if (ch->nextquest > 0) {
				ch->nextquest--;

				if (ch->nextquest == 0)
					stc("You may now quest again.\n", ch);
			}
			else if (IS_SET(ch->act, PLR_QUESTOR)) {
				if (--ch->countdown <= 0) {
					ch->nextquest = 0;
					stc("You have run out of time for your quest!\nYou may now quest again.\n", ch);
					REMOVE_BIT(ch->act, PLR_QUESTOR);
					ch->quest_giver = 0;
					ch->countdown = 0;
					ch->questmob = 0;
					ch->questloc = 0;
				}

				if (ch->countdown > 0 && ch->countdown < 6)
					stc("Better hurry, you're almost out of time for your quest!\n", ch);
			}

			/* parasite skill quest timer off of quest_update */
			if (ch->pcdata->nextsquest > 0) {
				ch->pcdata->nextsquest--;

				if (ch->pcdata->nextsquest == 0)
					stc("You may now skill quest again.\n", ch);
			}
			else if (IS_SET(ch->pcdata->plr, PLR_SQUESTOR)) {
				if (--ch->pcdata->sqcountdown <= 0) {
					ch->pcdata->nextsquest = 0;
					stc("You have run out of time for your skill quest!\n"
					    "You may now skill quest again.\n", ch);
					sq_cleanup(ch);
				}

				if (ch->pcdata->sqcountdown > 0 && ch->pcdata->sqcountdown < 6)
					stc("Better hurry, you're almost out of time for your skill quest!\n", ch);
			}
		}
	}
} /* end quest_update() */
