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

#include <list>

#include "act.hh"
#include "affect/Affect.hh"
#include "Area.hh"
#include "Auction.hh"
#include "channels.hh"
#include "Character.hh"
#include "declare.hh"
#include "Descriptor.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "GameTime.hh"
#include "interp.hh"
#include "Logging.hh"
#include "merc.hh"
#include "music.hh"
#include "MobilePrototype.hh"
#include "progs/triggers.hh"
#include "progs/MobProgActList.hh"
#include "progs/MobProgContext.hh"
#include "Object.hh"
#include "ObjectValue.hh"
#include "Player.hh"
#include "random.hh"
#include "Reset.hh"
#include "Room.hh"
#include "String.hh"
#include "Weather.hh"
#include "World.hh"
#include "comm.hh"

extern void     squestmob_found args((Character *ch, Character *mob));
extern           time_t                  reboot_time;

/*
 * Local functions.
 */
int     hit_gain        args((Character *ch));
int     mana_gain       args((Character *ch));
int     stam_gain       args((Character *ch));
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

void demote_level(Character *ch)
{
	int sub_hp, sub_mana, sub_stam, sub_prac, sub_train = 1;
	ch->pcdata->last_level = get_play_hours(ch);
	sub_hp          = std::max(1, con_app[GET_ATTR_CON(ch)].hitp + number_range(
	                               guild_table[ch->guild].hp_min, guild_table[ch->guild].hp_max));
	sub_mana        = std::max(1, int_app[GET_ATTR_INT(ch)].manap + number_range(
	                               guild_table[ch->guild].mana_min, guild_table[ch->guild].mana_max));
	sub_stam        = std::max(1, str_app[GET_ATTR_STR(ch)].stp + number_range(
	                               guild_table[ch->guild].stam_min, guild_table[ch->guild].stam_max));
	sub_prac        = wis_app[GET_ATTR_WIS(ch)].practice;

	if (chr_app[GET_ATTR_CHR(ch)].chance >= number_percent())
		sub_train = 2;

	ch->practice            -= sub_prac;
	ch->train               -= sub_train;
	ATTR_BASE(ch, APPLY_HIT)    = std::max(20, ATTR_BASE(ch, APPLY_HIT) - sub_hp);
	ATTR_BASE(ch, APPLY_MANA)   = std::max(100, ATTR_BASE(ch, APPLY_MANA) - sub_mana);
	ATTR_BASE(ch, APPLY_STAM)   = std::max(100, ATTR_BASE(ch, APPLY_STAM) - sub_stam);

	ptc(ch, "{RYour loss is: %d/%d hp, %d/%d ma, %d/%d stm, %d/%d prac, %d/%d train.{x\n",
	    sub_hp,         GET_MAX_HIT(ch),
	    sub_mana,       GET_MAX_MANA(ch),
	    sub_stam,       GET_MAX_STAM(ch),
	    sub_prac,       ch->practice,
	    sub_train,      ch->train);
}

void advance_level(Character *ch)
{
	int add_hp, add_mana, add_stam, add_prac, add_train = 1;
	ch->pcdata->last_level = get_play_hours(ch);
	add_hp          = std::max(1, con_app[GET_ATTR_CON(ch)].hitp + number_range(
	                               guild_table[ch->guild].hp_min, guild_table[ch->guild].hp_max));
	add_mana        = std::max(1, int_app[GET_ATTR_INT(ch)].manap + number_range(
	                               guild_table[ch->guild].mana_min, guild_table[ch->guild].mana_max));
	add_stam        = std::max(1, str_app[GET_ATTR_STR(ch)].stp + number_range(
	                               guild_table[ch->guild].stam_min, guild_table[ch->guild].stam_max));
	add_prac        = wis_app[GET_ATTR_WIS(ch)].practice;

	if (chr_app[GET_ATTR_CHR(ch)].chance >= number_percent())
		add_train = 2;

	/* old calculations:
	   add_mana    = (2*get_curr_stat(ch,STAT_INT) + get_curr_stat(ch,STAT_WIS)) / 5;
	   add_move    = number_range(1, (get_curr_stat(ch,STAT_CON) + get_curr_stat(ch,STAT_DEX))/6 );
	*/
	ATTR_BASE(ch, APPLY_HIT)             += add_hp;
	ATTR_BASE(ch, APPLY_MANA)            += add_mana;
	ATTR_BASE(ch, APPLY_STAM)            += add_stam;
	ch->practice            += add_prac;
	ch->train               += add_train;
	ptc(ch, "Your gain is: %d/%d hp, %d/%d ma, %d/%d stm, %d/%d prac, %d/%d train.\n",
	    add_hp,         GET_MAX_HIT(ch),
	    add_mana,       GET_MAX_MANA(ch),
	    add_stam,       GET_MAX_STAM(ch),
	    add_prac,       ch->practice,
	    add_train,      ch->train);
}

/*
This function advances a NPC. This is to advance a pet's
level. This function is likely to be called directly after advance_level().
-- Outsider
*/
void npc_advance_level(Character *ch)
{
	int add_hit, add_mana, add_stam;

	if (! ch->is_npc())
		return;

	add_hit = GET_ATTR_CON(ch);
	add_mana = GET_ATTR_INT(ch);
	add_stam = GET_ATTR_STR(ch);
	ATTR_BASE(ch, APPLY_HIT)  += add_hit;
	ATTR_BASE(ch, APPLY_MANA) += add_mana;
	ATTR_BASE(ch, APPLY_STAM) += add_stam;
}

void gain_exp(Character *ch, int gain)
{
	char buf[MSL];

	if (ch->is_npc())
		return;

	if (ch->level >= LEVEL_HERO)
		return;

	ch->exp = std::max(exp_per_level(ch, ch->pcdata->points), ch->exp + gain);

	while ((ch->exp >= exp_per_level(ch, ch->pcdata->points) * (ch->level + 1))
	       && (ch->level < LEVEL_HERO)) {
		stc("{PG{RR{YA{bT{GZ{H!  {BWay {Nto {Vlevel{M!!{x\n", ch);
		/* up the player's level. If hero, add fireworks. */
		ch->level++;

		if (ch->level == LEVEL_HERO) {
			Descriptor *d;
			Character *victim;
			static char *msg =
			        "{PGGGGGG  RRRRRR    AAAA   TTTTTTT  ZZZZZZ\n"
			        "{YGG  GG  RR  RRR  AAAAAA  TTTTTTT  ZZZZZZ\n"
			        "{GGG      RR  RRR  AA  AA    TTT      ZZZ \n"
			        "{CGG GGG  RRRRRR   AAAAAA    TTT     ZZZ  \n"
			        "{BGG  GG  RR  RRR  AA  AA    TTT    ZZZZZZ\n"
			        "{VGGGGGG  RR   RR  AA  AA    TTT    ZZZZZZ{x\n";
			extern void restore_char(Character *, Character *);
			stc("{MGratz from all the Immortals of Legacy!{x\n\n", ch);

			for (d = descriptor_list; d; d = d->next)
				if (d->is_playing()) {
					victim = d->original ? d->original : d->character;
					stc(msg, victim);
					restore_char(ch, victim);
				}

			ch->add_cgroup(GROUP_HERO);
		}       /* end of if hero */

		/* add avatar group if level 80 */
		if (ch->level == LEVEL_AVATAR)
			ch->add_cgroup(GROUP_AVATAR);

		Format::sprintf(buf, "%s has attained level %d!", ch->name, ch->level);
		wiznet(buf, ch, nullptr, WIZ_LEVELS, 0, 0);
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
}

/*
 * Regeneration stuff.
 */
int hit_gain(Character *ch)
{
	int gain;
	int number;

	if (ch->in_room == nullptr)
		return 0;

	if (ch->is_npc()) {
		gain =  5 + ch->level;

		if (affect::exists_on_char(ch, affect::type::regeneration))
			gain *= 2;

		switch (get_position(ch)) {
		default :           gain /= 2;                      break;

		case POS_SLEEPING:  gain = 3 * gain / 2;              break;

		case POS_RESTING:                                   break;

		case POS_FIGHTING:  gain /= 3;                      break;
		}
	}
	else {
		gain = std::max(3, GET_ATTR_CON(ch) - 3 + ch->level / 2);
		gain += guild_table[ch->guild].hp_max - 10;
		number = number_percent();

		if (number < get_skill_level(ch, skill::type::fast_healing)) {
			gain += number * gain / 100;

			if (ch->hit < GET_MAX_HIT(ch))
				check_improve(ch, skill::type::fast_healing, true, 8);
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

	gain = gain * ch->in_room->heal_rate() / 100;

	if (ch->on != nullptr && ch->on->item_type == ITEM_FURNITURE)
		gain = gain * ch->on->value[3] / 100;

	if (affect::exists_on_char(ch, affect::type::poison))
		gain /= 4;

	if (affect::exists_on_char(ch, affect::type::plague))
		gain /= 8;

	if (affect::exists_on_char(ch, affect::type::haste) && ch->race != 8) // faeries, ugly hack, fix later -- Montrey (2014)
		gain /= 2 ;

	if (affect::exists_on_char(ch, affect::type::slow))
		gain *= 2 ;

	if (affect::exists_on_char(ch, affect::type::regeneration))
		gain *= 2;

	if (affect::exists_on_char(ch, affect::type::divine_regeneration))
		gain *= 4;

	return std::min(gain, GET_MAX_HIT(ch) - ch->hit);
}

int mana_gain(Character *ch)
{
	int gain, number;

	if (ch->in_room == nullptr)
		return 0;

	if (ch->is_npc()) {
		gain = 5 + ch->level;

		switch (get_position(ch)) {
		default:                gain /= 2;              break;

		case POS_SLEEPING:      gain = 3 * gain / 2;      break;

		case POS_RESTING:                               break;

		case POS_FIGHTING:      gain /= 3;              break;
		}
	}
	else {
		gain = (GET_ATTR_WIS(ch) + GET_ATTR_INT(ch) + ch->level) / 2;
		number = number_percent();

		if (number < get_skill_level(ch, skill::type::meditation)) {
			gain += number * gain / 100;

			if (ch->mana < GET_MAX_MANA(ch))
				check_improve(ch, skill::type::meditation, true, 8);
		}

		/* compare to mages mana regen, mages get full (class 0) */
		gain -= gain * (guild_table[0].mana_max - guild_table[ch->guild].mana_max)
		        / guild_table[0].mana_max;

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

	gain = gain * ch->in_room->mana_rate() / 100;

	if (ch->on != nullptr && ch->on->item_type == ITEM_FURNITURE)
		gain = gain * ch->on->value[4] / 100;

	if (affect::exists_on_char(ch, affect::type::poison))
		gain /= 4;

	if (affect::exists_on_char(ch, affect::type::plague))
		gain /= 8;

	if (affect::exists_on_char(ch, affect::type::haste) && ch->race != 8) // faeries, ugly hack, fix later -- Montrey (2014)
		gain /= 2;

	if (affect::exists_on_char(ch, affect::type::slow))
		gain *= 2;

	if (affect::exists_on_char(ch, affect::type::divine_regeneration))
		gain *= 2;

	return std::min(gain, GET_MAX_MANA(ch) - ch->mana);
}

int stam_gain(Character *ch)
{
	int gain;

	if (ch->in_room == nullptr)
		return 0;

	if (ch->is_npc()) {
		gain = 5 + ch->level;

		switch (get_position(ch)) {
		default:                gain /= 2;              break;

		case POS_SLEEPING:      gain = 3 * gain / 2;      break;

		case POS_RESTING:                               break;

		case POS_FIGHTING:      gain /= 3;              break;
		}
	}
	else {
		gain = GET_ATTR_CON(ch) + GET_ATTR_DEX(ch) + (ch->level / 2);
		/* compare to warrior stamina regen, warriors get full (class 3) */
		gain -= gain * (guild_table[3].stam_max - guild_table[ch->guild].stam_max)
		        / guild_table[3].stam_max;

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

	gain = gain * ch->in_room->heal_rate() / 100;

	if (ch->on != nullptr && ch->on->item_type == ITEM_FURNITURE)
		gain = gain * ch->on->value[3] / 100;

	if (affect::exists_on_char(ch, affect::type::poison))
		gain /= 4;

	if (affect::exists_on_char(ch, affect::type::plague))
		gain /= 8;

	if (affect::exists_on_char(ch, affect::type::haste) && ch->race != 8) // faeries, ugly hack, fix later -- Montrey (2014)
		gain /= 3;

	if (affect::exists_on_char(ch, affect::type::slow))
		gain *= 2;

	if (affect::exists_on_char(ch, affect::type::regeneration))
		gain *= 2;

	if (affect::exists_on_char(ch, affect::type::divine_regeneration))
		gain *= 2;

	return std::min(gain, GET_MAX_STAM(ch) - ch->stam);
}

void gain_condition(Character *ch, int iCond, int value)
{
	int condition;

	if (value == 0 || ch->is_npc() || ch->pcdata == nullptr)
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

	if (!ch->pcdata->plr_flags.has(PLR_CHATMODE)) {
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
	/* Examine all mobs. */
	for (auto ch : Game::world().char_list)
		ch->update();
}


/* Update all descriptors, handles login timer */
void descrip_update(void)
{
	Descriptor *d, *d_next;
	Character *ch;

	for (d = descriptor_list; d != nullptr; d = d_next) {
		d_next = d->next;
		++d->timer;

		if (d->is_playing()) {
			ch = d->character;

			if (IS_IMMORTAL(ch) || get_duel(ch)) {
				if (d->timer > 99)
					d->timer = 99;

				continue;
			}

			if (d->timer >= 12) {
				if (ch->was_in_room == nullptr && ch->in_room != nullptr) {
					ch->was_in_room = ch->in_room;

					if (ch->fighting != nullptr)
						stop_fighting(ch, true);
					
					if (!ch->is_npc()){
						act("$n disappears into the void...", ch, nullptr, nullptr, TO_ROOM);
						stc("You disappear into the void.\n", ch);

						if (ch->level > 1)
							save_char_obj(ch);

						char_from_room(ch);
						char_to_room(ch, Game::world().get_room(Location(Vnum(ROOM_VNUM_LIMBO))));
					}
				}
			}

			if (d->timer == 7) {
				if (!ch->comm_flags.has(COMM_AFK) && !ch->is_npc()) {
					act("$n is set to auto-afk...", ch, nullptr, nullptr, TO_ROOM);
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
	Character *ch_quit;
	ch_quit     = nullptr;
	/* update save counter */
	save_number++;

	if (save_number > 29)
		save_number = 0;

	for (auto ch : Game::world().char_list) {
		if (!IS_IMMORTAL(ch) && !char_in_duel_room(ch)) {
			if (ch->desc != nullptr)
				if (ch->desc->timer > 30)
					ch_quit = ch;

			if (ch->timer > 30 && ch_quit == nullptr)
				ch_quit = ch;
		}

		/* Autotick stuff - Lotus */
		if (!ch->is_npc() && ch->act_flags.has(PLR_TICKS))
			stc("{Btick...{x\n", ch);

		if (get_position(ch) >= POS_STUNNED) {
			/* Nectimer for Necromancy spells */
			if (ch->is_npc() && (ch->nectimer > 0)) {
				ch->nectimer -= 1;

				if (ch->nectimer <= 0) {
					act("$n vanishes back into the nether void.", ch, nullptr, nullptr,
					    TO_ROOM);
					extract_char(ch, true);
					continue;
				}
			}

			/* If the player had a familiar and it is gone (probably dead)
			   then the player loses the ability score benefit of the
			   familiar. -- Outsider
			*/
			if (!ch->is_npc() && ch->pcdata->familiar) {
				if (! ch->pet) {
					ch->pcdata->familiar = false;
				}
			}    /* end of removed familiar */

			/* If the character is a Paladin and not an NPC
			   then check their Lay on Hands status. -- Outsider */
			if (!ch->is_npc() && ch->guild == Guild::paladin) {
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

			if (ch->hit  < GET_MAX_HIT(ch))
				ch->hit  += hit_gain(ch);
			else
				ch->hit = GET_MAX_HIT(ch);

			if (ch->mana < GET_MAX_MANA(ch))
				ch->mana += mana_gain(ch);
			else
				ch->mana = GET_MAX_MANA(ch);

			if (ch->stam < GET_MAX_STAM(ch))
				ch->stam += stam_gain(ch);
			else
				ch->stam = GET_MAX_STAM(ch);
		}

		if (get_position(ch) == POS_STUNNED)
			update_pos(ch);

		if (!ch->is_npc() && !IS_IMMORTAL(ch)) {
			Object *obj;

			if ((obj = get_eq_char(ch, WEAR_LIGHT)) != nullptr
			    &&   obj->item_type == ITEM_LIGHT
			    &&   obj->value[2] > 0) {
				if (--obj->value[2] == 0 && ch->in_room != nullptr) {
					if (ch->in_room->light > 0)
						--ch->in_room->light;

					act("$p goes out.  Hope you're not afraid of the dark.", ch, obj, nullptr, TO_ROOM);
					act("$p flickers and goes out.", ch, obj, nullptr, TO_CHAR);
					destroy_obj(obj);
				}
				else if (obj->value[2] <= 5 && ch->in_room != nullptr)
					act("$p hisses and sparks.", ch, obj, nullptr, TO_CHAR);
			}

			/* This timer is here in case a player goes linkdead - Lotus */
			if (ch->desc == nullptr) {
				ch->timer++;

				if (IS_IMMORTAL(ch) || char_in_duel_room(ch)) {
					if (ch->timer > 99)
						ch->timer = 99;
				}
				else if (ch->timer >= 12 && ch->was_in_room == nullptr && ch->in_room != nullptr) {
					ch->was_in_room = ch->in_room;

					if (ch->fighting != nullptr)
						stop_fighting(ch, true);

					act("$n disappears into the void...", ch, nullptr, nullptr, TO_ROOM);
					stc("You disappear into the void.\n", ch);

					if (ch->level > 1)
						save_char_obj(ch);

					char_from_room(ch);
					char_to_room(ch, Game::world().get_room(Location(Vnum(ROOM_VNUM_LIMBO))));
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
			if (ch->pcdata->flag_killer == 0 && ch->act_flags.has(PLR_KILLER)) {
				ch->act_flags -= PLR_KILLER;
				ch->act_flags -= PLR_NOPK;
				stc("The urge to kill dimishes.\n", ch);
				stc("You are no longer a KILLER.\n", ch);
				save_char_obj(ch);
			}
			else if (ch->pcdata->flag_killer > 0 && ch->act_flags.has(PLR_KILLER))
				ch->pcdata->flag_killer--;

			/* Check thief flag - Clerve */
			if (ch->pcdata->flag_thief == 0 && ch->act_flags.has(PLR_THIEF)) {
				ch->act_flags -= PLR_THIEF;
				ch->act_flags -= PLR_NOPK;
				stc("The urge to steal dimishes.\n", ch);
				stc("You are no longer a THIEF.\n", ch);
				save_char_obj(ch);
			}
			else if (ch->pcdata->flag_thief > 0 && ch->act_flags.has(PLR_THIEF))
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
		affect::sort_char(ch, affect::comparator_duration);
		affect::sort_char(ch, affect::comparator_type);

		for (const affect::Affect *paf = affect::list_char(ch); paf; paf = paf->next) {
			if (paf->duration == 0) {
				if (paf->next == nullptr
				 || paf->next->type != paf->type
				 || paf->next->duration > 0) {
					if (paf->type >= affect::type::first && !affect::lookup(paf->type).msg_off.empty())
						ptc(ch, "%s\n", affect::lookup(paf->type).msg_off);
				}
			}
		}

		// now remove spells with duration 0
		affect::Affect pattern;
		pattern.duration = 0;
		affect::remove_matching_from_char(ch, affect::comparator_duration, &pattern);

		// decrement duration and sometimes decrement level.  this is done after
		// the wearing off of spells with duration 0, because we use -1 to mean
		// indefinite and players are used to having spell counters go down to 0
		// before they wear off.
		affect::iterate_over_char(ch, affect::fn_fade_spell, nullptr);

		/* MOBprogram tick trigger -- Montrey */
		if (ch->is_npc()) {
			progs::tick_trigger(ch);

			/* If ch dies or changes position
			   due to it's tick trigger, continue */
			if (ch->is_garbage())
				continue;
		}

		/*
		 * Careful with the damages here,
		 *   MUST NOT refer to ch after damage taken,
		 *   as it may be lethal damage (on NPC).
		 */

		if (affect::exists_on_char(ch, affect::type::plague)) {
		 	const affect::Affect *plague = affect::find_on_char(ch, affect::type::plague);

			act("$n writhes in agony as plague sores erupt from $s skin.",
			    ch, nullptr, nullptr, TO_ROOM);
			stc("You writhe in agony from the plague.\n", ch);

			spread_plague(ch->in_room, plague, 4);

			// TODO: check for plague being nullptr only applies as long as plague bit exists
			int dam = std::min(ch->level, (plague ? plague->level : ch->level) / 5 + 1);
			ch->mana -= dam;
			ch->stam -= dam;
			damage(ch->fighting ? ch->fighting : ch, ch, dam, skill::type::plague, -1, DAM_DISEASE, false, true);
		}

		if (ch->is_garbage())
			continue;

		if (affect::exists_on_char(ch, affect::type::poison)
		 && !affect::exists_on_char(ch, affect::type::slow)) {
			const affect::Affect *poison = affect::find_on_char(ch, affect::type::poison);

			if (poison != nullptr) {
				act("$n shivers and suffers.", ch, nullptr, nullptr, TO_ROOM);
				stc("You shiver and suffer.\n", ch);
				damage(ch->fighting ? ch->fighting : ch, ch, poison->level / 10 + 1, skill::type::poison,
				       -1, DAM_POISON, false, true);
			}
		}
		
		if (ch->is_garbage())
			continue;

		// bleeding out?
		if (get_position(ch) == POS_INCAP && number_range(0, 1) == 0)
			damage(ch->fighting ? ch->fighting : ch, ch, 1, skill::type::unknown, -1, DAM_NONE, false, false);
		else if (get_position(ch) == POS_MORTAL)
			damage(ch->fighting ? ch->fighting : ch, ch, 1, skill::type::unknown, -1, DAM_NONE, false, false);
	}

	/*
	 * Autosave and autoquit.
	 * Check that these chars still exist.
	 */
	for (auto ch : Game::world().char_list) {
		if (ch->desc != nullptr && ch->desc->descriptor % 30 == save_number)
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
	Object *obj;
	Object *obj_next;

	for (obj = Game::world().object_list; obj != nullptr; obj = obj_next) {
		Character *rch;
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
		affect::sort_obj(obj, affect::comparator_duration);
		affect::sort_obj(obj, affect::comparator_type);

		for (const affect::Affect *paf = affect::list_obj(obj); paf; paf = paf->next) {
			if (paf->duration == 0) {
				if (paf->next == nullptr
				 || paf->next->type != paf->type
				 || paf->next->duration > 0) {
					/* for addapplied objects with a duration */
					if (paf->type == affect::type::none) {
						if (obj->carried_by != nullptr) {
							rch = obj->carried_by;
							act("The magic of $p diminishes.", rch, obj, nullptr, TO_CHAR);
						}

						if (obj->in_room != nullptr && obj->in_room->people != nullptr) {
							rch = obj->in_room->people;
							act("The magic of $p diminishes.", rch, obj, nullptr, TO_ALL);
						}
					}

					String message = affect::lookup(paf->type).msg_obj;

					if (!message.empty()) {
						if (obj->carried_by != nullptr) {
							rch = obj->carried_by;
							act(message, rch, obj, nullptr, TO_CHAR);
						}

						if (obj->in_room != nullptr && obj->in_room->people != nullptr) {
							rch = obj->in_room->people;
							act(message, rch, obj, nullptr, TO_ALL);
						}
					}
				}
			}
		}

		// now remove spells with duration 0
		affect::Affect pattern;
		pattern.duration = 0;
		affect::remove_matching_from_obj(obj, affect::comparator_duration, &pattern);

		// decrement duration and sometimes decrement level.  this is done after
		// the wearing off of spells with duration 0, because we use -1 to mean
		// indefinite and players are used to having spell counters go down to 0
		// before they wear off.
		affect::iterate_over_obj(obj, affect::fn_fade_spell, nullptr);

		/* do not decay items being auctioned -- Elrac */
		if (auction.is_participant(obj))
			continue;

		if (obj->timer <= 0 || --obj->timer > 0)
			if (obj->clean_timer <= 0 || --obj->clean_timer > 0)
			 	continue;

		// past this point the object is going away

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

		if (obj->carried_by != nullptr) {
			if (obj->carried_by->is_npc() && obj->carried_by->pIndexData->pShop != nullptr)
				obj->carried_by->silver += obj->cost / 5;
			else if (!obj->in_obj) { /* don't send messages if it's in another object */
				act(message, obj->carried_by, obj, nullptr, TO_CHAR);

				if (get_eq_char(obj->carried_by, WEAR_FLOAT) == obj) {
					act(message, obj->carried_by, obj, nullptr, TO_ROOM);

					if (obj->contains)
						act("$p spills its contents on the ground.", obj->carried_by, obj, nullptr, TO_ALL);
				}
			}
		}
		else if (obj->in_room != nullptr && (rch = obj->in_room->people) != nullptr && !obj->in_obj)
			act(message, rch, obj, nullptr, TO_ALL);

		/* save the contents, but not npc corpses */
		if (obj->item_type != ITEM_CORPSE_NPC && obj->contains) {
			Object *t_obj, *next_obj;

			for (t_obj = obj->contains; t_obj != nullptr; t_obj = next_obj) {
				next_obj = t_obj->next_content;
				obj_from_obj(t_obj);

				if (obj->in_obj)                                /* in another object */
					obj_to_obj(t_obj, obj->in_obj);
				else if (obj->carried_by) {                     /* carried */
					if (get_eq_char(obj->carried_by, WEAR_FLOAT) == obj) {
						if (obj->carried_by->in_room == nullptr)
							extract_obj(t_obj);
						else
							obj_to_room(t_obj, obj->carried_by->in_room);
					}
					else
						obj_to_char(t_obj, obj->carried_by);
				}
				else if (obj->in_room == nullptr)                  /* destroy it */
					extract_obj(t_obj);
				else                                            /* to a room */
					obj_to_room(t_obj, obj->in_room);
			}
		}

		extract_obj(obj);
	}
}

/* Update all rooms -- Montrey */
void room_update(void) {
	for (auto& area_pair : Game::world().areas) {
		for (auto& pair : area_pair.second->rooms) {
			Room *room = pair.second;

			// print the affects that are wearing off.  this is complicated because
			// we may have more than one affect that is part of the same group, and
			// we also don't want to print a 'wearing off' message for affects that
			// are duplicated (for some reason).  so, the hackish solution is to sort
			// the list twice: once by duration, and then by skill number.  this
			// should get all the grouped spells together.
			affect::sort_room(room, affect::comparator_duration);
			affect::sort_room(room, affect::comparator_type);

			for (const affect::Affect *paf = affect::list_room(room); paf; paf = paf->next) {
				if (paf->duration == 0) {
					if (paf->next == nullptr
					 || paf->next->type != paf->type
					 || paf->next->duration > 0) {

						if (!room->people)
							continue;

						/* there is no msg_room for spells, so we'll use msg_obj for
						   room affect spells.  might change this later, but i really
						   don't feel like adding another ,"" to all those entries
						   right now :P -- Montrey */
						String message = affect::lookup(paf->type).msg_obj;

						if (!message.empty())
							act(message, nullptr, nullptr, nullptr, TO_ALL);
					}
				}
			}

			// now remove spells with duration 0
			affect::Affect pattern;
			pattern.duration = 0;
			affect::remove_matching_from_room(room, affect::comparator_duration, &pattern);

			// decrement duration and sometimes decrement level.  this is done after
			// the wearing off of spells with duration 0, because we use -1 to mean
			// indefinite and players are used to having spell counters go down to 0
			// before they wear off.
			affect::iterate_over_room(room, affect::fn_fade_spell, nullptr);
		}
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

bool eligible_aggressor(Character *ch)
{
	return (ch->is_npc()
	        && IS_AWAKE(ch)
	        && ch->act_flags.has_any_of(ACT_AGGRESSIVE | ACT_AGGR_ALIGN)
	        && ch->fighting == nullptr
	        && !affect::exists_on_char(ch, affect::type::calm)
	        && !affect::exists_on_char(ch, affect::type::charm_person)
	       );
}

bool eligible_victim(Character *ch)
{
	if (ch->is_npc())
		return false;

	if (IS_IMMORTAL(ch))
		return false;

//	if (ch->on && ch->on->pIndexData->item_type == ITEM_COACH)
//		return false;

	return true;
}

void aggr_update(void)
{
	Descriptor *d;
	int player_count;
	int jroom, room_count;
	int jvictim, victim_count, victim_num;
	int jmob, mob_count, mob_num;
	Room *room;
	Character *ch, *plr, *mob, *victim;
	bool duplicate;
	/* Count players. There can't possibly be more player-
	   inhabited rooms than there are players. */
	player_count = 0;

	for (d = descriptor_list; d != nullptr; d = d->next)
		player_count++;

	/* allocate stack memory for pointers to <player_count> rooms */
	Room *room_list[player_count];

	for (jroom = 0; jroom < player_count; jroom++)
		room_list[jroom] = nullptr;

	/* build a list of rooms, no duplicates */
	room_count = 0;

	for (d = descriptor_list; d != nullptr; d = d->next) {
		if (!d->is_playing() ||
		    d->original  != nullptr)
			continue;

		plr = d->character;

		if (plr->is_npc()          ||
		    plr->in_room == nullptr)
			continue;

		duplicate = false;

		for (jroom = 0; jroom < room_count; jroom++)
			if (room_list[jroom] == plr->in_room)
				duplicate = true;

		if (duplicate == false)
			room_list[room_count++] = plr->in_room;
	}

	/* for each room match up an aggressive mob and aggressable player */
	for (jroom = 0; jroom < room_count; jroom++) {
		room = room_list[jroom];

		/* We can parasite skill quest mobs in here, since the player just needs
		   to be in the same room as the mob for it to react.  Done before
		   mobprog and aggression checking to prevent other mobs jumping the
		   player before they can complete their quest, may change for realism, tho -- Montrey */
		for (ch = room->people; ch != nullptr; ch = ch->next_in_room) {
			if (IS_SQUESTOR(ch)
			    && ch->pcdata->squestmob != nullptr
			    && ch->pcdata->squestobj == nullptr) {
				/* look for quest mob */
				for (mob = room->people; mob != nullptr; mob = mob->next_in_room) {
					if (mob->is_npc() && mob == ch->pcdata->squestmob && can_see_char(mob, ch)) {
						squestmob_found(ch, mob);
						break;
					}
				}
			}
		}

		/* no aggression in safe rooms */
		if (room->flags().has(ROOM_SAFE)
		 || room->flags().has(ROOM_LAW))
			continue;

		/* only aggression below this point */
		/* count mobs and players in room for random selection */
		mob_count = 0;
		victim_count = 0;

		for (ch = room->people; ch != nullptr; ch = ch->next_in_room)
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
		mob = nullptr;
		jvictim = 0;
		victim = nullptr;

		for (ch = room->people; ch != nullptr; ch = ch->next_in_room) {
			if (eligible_aggressor(ch)) {
				if (++jmob == mob_num)
					mob = ch;
			}
			else if (eligible_victim(ch)) {
				if (++jvictim == victim_num)
					victim = ch;
			}
		}

		if (mob == nullptr || victim == nullptr) {
			Logging::bug("aggr_update: mob or victim nullptr!", 0);
			return;
		}

		/* final checks before the fight starts */
		if ((mob->act_flags.has(ACT_WIMPY) && IS_AWAKE(victim))
		    || !can_see_char(mob, victim))
			continue;

		if (mob->level < victim->level - 5)
			continue;

		if (mob->act_flags.has(ACT_AGGR_ALIGN)) {
			if (IS_NEUTRAL(victim))
				continue;

			if (IS_GOOD(mob) && !IS_EVIL(victim))
				continue;

			if (IS_EVIL(mob) && !IS_GOOD(victim))
				continue;
		}

		/* I like the idea of charisma coming into play here, but we did a mud-wide
		   lowering of mob stats, and nothing aggresses on players anymore because
		   their charisma is so high compared to mobs.  Removing this and changing
		   to a random chance of aggressing based on victim charisma, but keep in mind
		   this is called once every pulse.  -- Montrey
		if ((GET_ATTR_CHR(victim) + number_range(0, 1))
		    > (GET_ATTR_CHR(mob) + number_range(0, 3)))
			continue;
		*/

		if (number_range(0, 29 - GET_ATTR_CHR(victim)) > 3) // 20% for 25 chr
			continue;

		/* rumble! */
		multi_hit(mob, victim, skill::type::unknown);
	}
} /* end aggr_update() */

void tele_update(void)
{
	Room *room;

	for (auto ch : Game::world().char_list) {
		if (ch->in_room == nullptr)
			continue;

		if (ch->in_room->flags().has(ROOM_TELEPORT)) {
			do_look(ch, "tele");

			if (!ch->in_room->tele_dest().is_valid())
				room = get_random_room(ch);
			else
				room = Game::world().get_room(ch->in_room->tele_dest());

			char_from_room(ch);
			char_to_room(ch, room);
			act("$n slowly fades into existence.\n", ch, nullptr, nullptr, TO_ROOM);
			do_look(ch, "auto");
		}
	}
}

/*
 * All players age by 1 second here. -- Elrac
 * I trust this will be less error-prone than the previous method,
 * which has been known to produce wild fluctuations.
 * Mobs are not aged. It would take a lot of CPU time and contribute
 * nothing significant to realism.
 */
void age_update(void)
{
	Descriptor *d;
	Character *wch;

	/* leech our Game::quest_double counter here too */
	if (Game::quest_double)
		Game::quest_double++;

	for (d = descriptor_list; d; d = d->next) {
		if (!d->is_playing())
			continue;

		wch = (d->original != nullptr) ? d->original : d->character;

		if (!wch->is_npc())
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
	for (auto ch : Game::world().char_list) {
		if (ch->daze > 0)       --ch->daze;

		if (ch->wait > 0)       --ch->wait;
	}
}

/* Mobprog act triggers are different than others in that they are not processed
   immediately, but instead each act trigger adds to a chain that is processed
   once per pulse.  This is probably to avoid some potential looping with
   mobs triggering eachother.  Anyway, this was parasiting on aggr_update,
   which is called every pulse.  However, aggr_update is only called on player-
   inhabited rooms.  Usually this is fine unless you're trying to do some
   crazy things with mobprogs such as helper mobs in utility rooms or, in my
   case, a pet mob that needed to execute an act prog as its master entered a
   portal.  So, putting processing of these progs here, and we'll just see what
   happens.  Technically this only needs to be called for player-occupied areas,
   but maybe if we change from a game-wide character list to area-owned lists
   it will make sense to do so. -- Montrey
 */

/* Slight problem: there are ways a mob can extract itself or other characters
   in a mobprog, which can potentially trash the list we're trying to iterate
   over.  Additionally, act_progs can result in new act_progs on characters that
   we either iterated over already, are about to iterate over, or even the current
   character.  I think the easiest solution is to build a list of characters
   and progs that we'll execute in this pulse, and let the results be handled
   next pulse.  Also check for extraction every step of the way.  -- Montrey
 */
void act_update(void) {
	struct ch_act {
		Character *ch;
		progs::MobProgActList *act_list;
	};

	std::list<ch_act> ch_acts;

	// build a list of all characters with acts, set up their act lists for new content
	for (auto ch : Game::world().char_list)
		if (ch->is_npc() && ch->mpact != nullptr) {
			ch_acts.push_back({ch, ch->mpact});
			ch->mpact = nullptr; // take ownership of the list
		}

	// execute all acts
	for (auto entry : ch_acts) {
		Character *ch = entry.ch;
		progs::MobProgActList *start = entry.act_list;

		for (progs::MobProgActList *tmp_act = start; tmp_act != nullptr; tmp_act = tmp_act->next) {
			// did the mob get extracted?
			if (ch->is_garbage())
				break;

			progs::wordlist_check(
				tmp_act->buf,
				ch->pIndexData->mobprogs,
				tmp_act->context,
				progs::Prog::Type::ACT_PROG
			);
		}

		// delete the list
		while (start != nullptr) {
			progs::MobProgActList *tmp_act = start->next;
			delete start;
			start = tmp_act;
		}
	}
}

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler(void)
{
	static int      pulse_quest,
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

	Game::world().update();

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
		Game::world().time.update();
		Game::world().weather.update();
		char_update();
		descrip_update();
		obj_update();
		room_update();

		if (reboot_time != 0) {
			if (Game::current_time == reboot_time) {
				int count = 0;
				Descriptor *d;

				for (d = descriptor_list; d != nullptr; d = d->next)
					if (d->is_playing())
						count++;

				if (count == 0) {
					Descriptor *d_next;
					extern bool merc_down;
					Logging::log("AUTO-REBOOT");
//					do_sysinfo("The system is going down for auto-reboot NOW.\n");
					merc_down = true;

					for (d = descriptor_list; d != nullptr; d = d_next) {
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
	auction.update();
	act_update();
	aggr_update();
} /* end update_handler() */

/* worldwide cleanup of objects */
void janitor_update()
{
	Character *rch;
	Object *obj;

	if (Game::port != DIZZYPORT)
		return;

	for (obj = Game::world().object_list; obj; obj = obj->next) {
		if (!obj->in_room
		    || obj->contains
		    || obj->timer
		    || obj->clean_timer)
			continue;

		if (obj->reset && obj->reset->command == 'O')
			if (Location((int)obj->reset->arg3) == obj->in_room->location)
				continue;

		for (rch = obj->in_room->people; rch; rch = rch->next_in_room)
			if (!rch->is_npc())
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

		if (roll_chance(10))
			obj->clean_timer = number_range(80, 240);       /* 1 to 3 hours */
	}

//	Logging::bugf("janitor_update: %d items marked for cleanup", count);
	objstate_save_items();
}

void underwater_update(void)
{
	int skill, dam;

	for (auto ch : Game::world().char_list) {
		if (!ch->is_npc() && ch->in_room->flags().has(ROOM_UNDER_WATER)) {
			skill = get_skill_level(ch, skill::type::swimming);

			if (skill == 100)
				stc("You would be {Cdrowning{x if not for your underwater breathing skill.\n", ch);
			else {
				/* a drink of water */
				gain_condition(ch, COND_THIRST,
				               liq_table[0].affect[4] * liq_table[0].affect[COND_THIRST] / 10);
				gain_condition(ch, COND_FULL,
				               liq_table[0].affect[4] * liq_table[0].affect[COND_FULL] / 4);
				dam = (ch->hit * (100 - skill)) / 400;

				if (ch->hit > 100) {
					stc("{CYou are drowning!!!{x\n", ch);

					if (skill > 0) {
						stc("{HYour skill helps slow your drowning.{x\n", ch);
						check_improve(ch, skill::type::swimming, true, 1);
					}

					damage(ch->fighting ? ch->fighting : ch, ch, dam, skill::type::swimming, -1, DAM_WATER, false, true);
				}
				else {
					stc("{PYou cannot hold your breath any more!{x\n", ch);
					stc("{CYour lungs fill with water and you lose consciousness...{x\n", ch);
					damage(ch->fighting ? ch->fighting : ch, ch, ch->hit + 15, skill::type::swimming, -1, DAM_WATER, false, true);
				}
			}
		}
	}
}

void quest_update(void)
{
	Descriptor *d;
	Character *ch;

	for (d = descriptor_list; d != nullptr; d = d->next) {
		if (d->is_playing()) {
			ch = d->character;

			if (ch->is_npc())
				continue;

			if (ch->pcdata->nextquest > 0) {
				ch->pcdata->nextquest--;

				if (ch->pcdata->nextquest == 0)
					stc("You may now quest again.\n", ch);
			}
			else if (IS_QUESTOR(ch)) {
				if (--ch->pcdata->countdown <= 0) {
					ch->pcdata->nextquest = 0;
					stc("You have run out of time for your quest!\nYou may now quest again.\n", ch);
					quest_cleanup(ch);
				}

				if (ch->pcdata->countdown > 0 && ch->pcdata->countdown < 6)
					stc("Better hurry, you're almost out of time for your quest!\n", ch);
			}

			/* parasite skill quest timer off of quest_update */
			if (ch->pcdata->nextsquest > 0) {
				ch->pcdata->nextsquest--;

				if (ch->pcdata->nextsquest == 0)
					stc("You may now skill quest again.\n", ch);
			}
			else if (IS_SQUESTOR(ch)) {
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
