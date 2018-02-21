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

#include <vector>

#include "act.hh"
#include "argument.hh"
#include "affect/Affect.hh"
#include "Area.hh"
#include "Battle.hh"
#include "channels.hh"
#include "Character.hh"
#include "Clan.hh"
#include "declare.hh"
#include "dispel.hh"
#include "Descriptor.hh"
#include "event/event.hh"
#include "Exit.hh"
#include "find.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "GameTime.hh"
#include "interp.hh"
#include "Logging.hh"
#include "macros.hh"
#include "magic.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "Object.hh"
#include "ObjectValue.hh"
#include "Player.hh"
#include "QuestArea.hh"
#include "random.hh"
#include "RoomPrototype.hh"
#include "skill/skill.hh"
#include "String.hh"
#include "Weather.hh"
#include "World.hh"

#define PKTIME 10       /* that's x3 seconds, 30 currently */

/* Maybe this will help me compile. -- Outsider */
void    wear_obj        args((Character *ch, Object *obj, bool fReplace));

/*
 * Local functions.
 */
void    check_assist    args((Character *ch, Character *victim));
void    check_cond      args((Character *ch, Object *obj));
void    check_all_cond  args((Character *ch));
void    check_killer    args((Character *ch, Character *victim));
bool    check_dodge     args((Character *ch, Character *victim, skill::type attack_skill, int attack_type));
bool    check_blur      args((Character *ch, Character *victim, skill::type attack_skill, int attack_type));
bool    check_shblock   args((Character *ch, Character *victim, skill::type attack_skill, int attack_type));
bool    check_parry     args((Character *ch, Character *victim, skill::type attack_skill, int attack_type));
bool    check_dual_parry args((Character *ch, Character *victim, skill::type attack_skill, int attack_type));    /* not a skill, evo dual wield */
void    do_riposte      args((Character *ch, Character *victim));
void    dam_message     args((Character *ch, Character *victim, int dam, skill::type attack_skill, int attack_type, bool immune, bool sanc_immune));
void    death_cry       args((Character *ch));
void    group_gain      args((Character *ch, Character *victim));
int     xp_compute      args((Character *gch, Character *victim, int total_levels, int diff_classes));
bool    is_safe         args((Character *ch, Character *victim, bool showmsg));
void    make_corpse     args((Character *ch));
bool    check_pulse     args((Character *victim));
void    kill_off        args((Character *ch, Character *victim));
void    one_hit         args((Character *ch, Character *victim, skill::type attack_skill, bool secondary));
void    mob_hit         args((Character *ch, Character *victim, skill::type attack_skill));
void    raw_kill        args((Character *victim));
void    set_fighting    args((Character *ch, Character *victim));
void    combat_regen    args((Character *ch));
void    noncombat_regen    args((Character *ch));
void    do_lay_on_hands       args((Character *ch, const char *argument));

/* Global XP */
int gxp;

/* nasty global focus boolean -- Montrey */
bool focus = TRUE;
bool global_quick = FALSE;

/* Control the fights going on.
   Called periodically by update_handler. */
void violence_update(void)
{
	Character *ch;
	Character *victim;

	// go through first and make sure everybody is fighting who should be fighting
	for (ch = char_list; ch != nullptr; ch = ch->next)
		if (ch->fighting)
			check_assist(ch, ch->fighting);

	for (ch = char_list; ch != nullptr; ch = ch->next) {
		if (ch->in_room == nullptr)
			continue;

		// see if anything needs to change because attributes modified
		// this is just a convenient place to do this with the appropriate
		// timing frequency.  can't go in the affect modifiers, because
		// we don't want them to drop a weapon when only half of the affects
		// have been loaded on the character
		extern void attribute_check(Character *ch);
		attribute_check(ch);

		/* Hunting mobs */
		if (IS_NPC(ch)
		    && ch->fighting == nullptr
		    && IS_AWAKE(ch)
		    && ch->hunting != nullptr) {
			hunt_victim(ch);
			continue;
		}

		if ((victim = ch->fighting) == nullptr) {
			/* parasite pk timer off of violence_update.  don't forget it's 3 seconds -- Montrey */

			if (!IS_NPC(ch)) {
				if (ch->pcdata->pktimer
				 && --ch->pcdata->pktimer == 0
				 && affect::exists_on_char(ch, affect::type::shadow_form))
					affect::remove_type_from_char(ch, affect::type::shadow_form);

				if (ch->pcdata->combattimer > 0)
					ch->pcdata->combattimer--;
				else
					noncombat_regen(ch);
			}

			continue;
		}

		// this is also set in multi_hit, to get the one-shot kills
                if (!IS_NPC(ch))
                        ch->pcdata->combattimer = 5;

		combat_regen(ch);

		/* this is moved from the damage function cause i don't want you to
		   actually have to be getting hit to stand up -- Montrey */
		if (ch->wait == 0 && get_position(ch) < POS_FIGHTING) {
			act("You clamber to your feet.", ch, nullptr, nullptr, TO_CHAR);
			act("$n clambers to $s feet.", ch, nullptr, nullptr, TO_ROOM);

			/* prevent spam bashing -- Montrey */
			if (IS_NPC(ch))
				ch->wait = 1;
			else {
				Character *rch;

				for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
					if (IS_NPC(rch)
					    && rch->fighting == ch
					    && get_position(rch) == POS_FIGHTING)
						rch->wait = UMAX(rch->wait, 1);
			}

			ch->position = POS_STANDING;

			if (ch->start_pos == POS_FLYING && CAN_FLY(ch))
				do_fly(ch, "");
		}

		if (IS_AWAKE(ch) && ch->in_room == victim->in_room)
			multi_hit(ch, victim, skill::type::unknown);
		else
			stop_fighting(ch, FALSE);

		if (ch == nullptr || ch->fighting == nullptr)
			continue;

//		view_room_hpbar(ch);

		if (ch->wait > 0)
			continue;

		/* Wimp out? */
		if (IS_NPC(ch)) {
			if ((ch->act_flags.has(ACT_WIMPY)
			     && number_bits(2) == 0
			     && ch->hit < GET_MAX_HIT(ch) / 5)
			    || (affect::exists_on_char(ch, affect::type::charm_person)
			        && ch->master != nullptr
			        && ch->master->in_room != ch->in_room))
				do_flee(ch, "");
		}
		else if (ch->hit > 0 && ch->hit <= ch->wimpy)
			do_flee(ch, "");

		if (ch->fighting == nullptr)
			continue;

		if (affect::exists_on_char(ch, affect::type::fear))
			do_flee(ch, "");

		if ((victim = ch->fighting) == nullptr)
			continue;

		/* Mobs switch to master, no longer do multihits every round :P -- Montrey */
		if (IS_NPC(ch)
		    && IS_NPC(victim)
		    && affect::exists_on_char(victim, affect::type::charm_person)
		    && victim->master != nullptr
		    && victim->master->in_room == ch->in_room
		    && chance(15)) {
			if (!can_see_char(ch, victim->master)) {
				if (chance(50))
					do_flee(ch, "");
			}
			else {
				act("$n changes the focus of $s attacks.", ch, nullptr, nullptr, TO_ROOM);
				stop_fighting(ch, FALSE);
				set_fighting(ch, victim->master);
				continue;
			}
		}

		if ((victim = ch->fighting) == nullptr)
			continue;

		mprog_hitprcnt_trigger(ch, victim);
		mprog_fight_trigger(ch, victim);
	}
} /* end violence_update */

void noncombat_regen(Character *ch) {
	int hitgain = GET_MAX_HIT(ch)/10;
	int managain = GET_MAX_MANA(ch)/10;
	int stamgain = GET_MAX_STAM(ch)/10;

	if (ch->hit < GET_MAX_HIT(ch)
	 || ch->mana < GET_MAX_MANA(ch)
	 || ch->stam < GET_MAX_STAM(ch)) {
	    ch->hit = UMIN(GET_MAX_HIT(ch), ch->hit + hitgain);
		ch->mana = UMIN(GET_MAX_MANA(ch), ch->mana + managain);
		ch->stam = UMIN(GET_MAX_STAM(ch), ch->stam + stamgain);
	}

	if (ch->hit == GET_MAX_HIT(ch)
	 && ch->mana == GET_MAX_MANA(ch)
	 && ch->stam == GET_MAX_STAM(ch)
	 && ch->pcdata->combattimer == 0) {
		stc("You have fully recovered from combat.\n", ch);
		ch->pcdata->combattimer = -1; // start again next combat
	}
}

void combat_regen(Character *ch)
{
	int hitgain = 0, managain = 0, stamgain = 0;

	/* for real vampires, regen or damage */
	if (ch->race == 6 && !IS_NPC(ch)) {
		int sun_damage;
		GameTime::Sun sun = ch->in_room->area->world.time.sunlight;
		Weather::Sky sky = ch->in_room->area->world.weather.sky;

		/* handle the regen first */
		if (GET_ROOM_FLAGS(ch->in_room).has_any_of(ROOM_DARK | ROOM_INDOORS)
		    || ch->in_room->sector_type == SECT_INSIDE
		    || sun == GameTime::Night)
			hitgain += (ch->level / 10) + 1;
		/* now damage from exposure to the sun */
		else {
			sun_damage = (ch->level / 20) + 1;

			/* 75% damage if it's not fully light out */
			if (sun == GameTime::Sunrise
			    || sun == GameTime::Sunset)
				sun_damage = (sun_damage * 3) / 4;

			/* 80% damage if it's cloudy :)  -poor vamps-  */
			if (sky != Weather::Cloudless)
				sun_damage = (sun_damage * 4) / 5;

			if (sun_damage < 0)
				sun_damage = 1;

			sun_damage -= sun_damage * GET_DEFENSE_MOD(ch, DAM_LIGHT) / 100;

			if (sun_damage > 0) {
				damage(ch->fighting, ch, sun_damage, skill::type::unknown, -1, DAM_NONE, FALSE, TRUE);

				if (ch == nullptr)
					return;
			}
		}
	}

	/* remort affect - mage regen */
	if (HAS_RAFF(ch, RAFF_MAGEREGEN) && ch->mana < GET_MAX_MANA(ch))
		managain += (ch->level / 20) + 1;

	/* remort affect - vampire regen */
	if (HAS_RAFF(ch, RAFF_VAMPREGEN) && ch->hit < GET_MAX_HIT(ch))
		hitgain += (ch->level / 20) + 1;

	if (affect::exists_on_char(ch, affect::type::regeneration) && ch->stam < GET_MAX_STAM(ch))
		switch (get_affect_evolution(ch, affect::type::regeneration)) {
		case 2: stamgain += ch->level / 30 + 2;   break;

		case 3: stamgain += ch->level / 15 + 3;   break;

		case 4: stamgain += ch->level / 10 + 4;   break;

		default:                                break;
		}

	if (get_skill_level(ch, skill::type::meditation) && ch->mana < GET_MAX_MANA(ch))
		switch (get_evolution(ch, skill::type::meditation)) {
		case 2: managain += ch->level / 30 + 2;   break;

		case 3: managain += ch->level / 15 + 3;   break;

		case 4: managain += ch->level / 10 + 4;   break;

		default:                                break;
		}

	if (get_skill_level(ch, skill::type::fast_healing) && ch->hit < GET_MAX_HIT(ch))
		switch (get_evolution(ch, skill::type::fast_healing)) {
		case 2: hitgain += ch->level / 30 + 2;    break;

		case 3: hitgain += ch->level / 15 + 3;    break;

		case 4: hitgain += ch->level / 10 + 4;    break;

		default:                                break;
		}

	if (affect::exists_on_char(ch, affect::type::divine_regeneration)) {
		int gain = 0;

		switch (get_affect_evolution(ch, affect::type::divine_regeneration)) {
		case 2: gain = ch->level / 30;            break;

		case 3: gain = ch->level / 15;            break;

		case 4: gain = ch->level / 10;            break;

		default:                                break;
		}

		hitgain += gain;
		managain += gain;
		stamgain += gain;
	}

	if (ch->hit < GET_MAX_HIT(ch))      ch->hit = UMIN(GET_MAX_HIT(ch), ch->hit + hitgain);

	if (ch->mana < GET_MAX_MANA(ch))    ch->mana = UMIN(GET_MAX_MANA(ch), ch->mana + managain);

	if (ch->stam < GET_MAX_STAM(ch))    ch->stam = UMIN(GET_MAX_STAM(ch), ch->stam + stamgain);
}

void check_all_cond(Character *ch)
{
	int iWear;
	Object *obj;

	if (IS_NPC(ch) || IS_IMMORTAL(ch))
		return;

	if (affect::exists_on_char(ch, affect::type::sheen))
		return;

	for (iWear = 0; iWear < MAX_WEAR; iWear++) {
		if ((obj = get_eq_char(ch, iWear)) == nullptr || obj->condition == -1)
			continue;

		check_cond(ch, obj);
	}
} /* end check_all_cond */

void check_cond(Character *ch, Object *obj)
{
	if (IS_IMMORTAL(ch))
		return;

	if (obj->condition == -1)
		return;

	/* sheen protects absolutely */
	if (affect::exists_on_char(ch, affect::type::sheen))
		return;

	if ((number_range(0, 500)) != 100)
		return;

	if (affect::exists_on_char(ch, affect::type::steel_mist))
		obj->condition -= number_range(1, 4);
	else
		obj->condition -= number_range(1, 8);

	if (obj->condition <= 0) {
		act("{W$p{x has been {Wdestroyed{x in combat!", ch, obj, nullptr, TO_CHAR);

		if (ch->in_room->sector_type != SECT_ARENA && !char_in_darena_room(ch)) {
			if (obj->contains) { /* dump contents */
				act("$p scatters it's contents on the ground.", ch, obj, nullptr, TO_CHAR);
				act("$n's $p breaks, scattering it's contents on the ground.", ch, obj, nullptr, TO_ROOM);
			}
			if (obj->gems) { /* dump contents */
				act("$p scatters it's gems on the ground.", ch, obj, nullptr, TO_CHAR);
				act("$n's $p breaks, scattering it's gems on the ground.", ch, obj, nullptr, TO_ROOM);
			}
		}

		destroy_obj(obj);
	}
	else
		act("{W$p{x has been {Wdamaged{x in combat!", ch, obj, nullptr, TO_CHAR);
} /* end check_cond */

/* for auto assisting */
void check_assist(Character *ch, Character *victim)
{
	Character *rch, *rch_next;

	for (rch = ch->in_room->people; rch != nullptr; rch = rch_next) {
		rch_next = rch->next_in_room;

		if (!IS_AWAKE(rch))
			continue;

		if (rch->fighting != nullptr)
			continue;

		Character *target = nullptr;

		if (IS_NPC(ch)) {
			if (IS_NPC(rch)) {
				// BOTH NPC

				// NPC assisting NPC is ok if all are charmed (is this necessary?)
				if (affect::exists_on_char(ch, affect::type::charm_person)) {
					if (affect::exists_on_char(rch, affect::type::charm_person)
					 && is_same_group(ch, rch))
						target = victim;
				}
				else {
					if (rch->off_flags.has(ASSIST_ALL)
					 || is_same_group(ch, rch)
					 || (rch->off_flags.has(ASSIST_RACE) && rch->race == ch->race)
					 || (rch->off_flags.has(ASSIST_ALIGN)
					  && ((IS_GOOD(rch) && IS_GOOD(ch))
					   || (IS_EVIL(rch) && IS_EVIL(ch))
					   || (IS_NEUTRAL(rch) && IS_NEUTRAL(ch))))
					  || (rch->off_flags.has(ASSIST_VNUM) && rch->pIndexData == ch->pIndexData)) {
						int number = 0;

						if (number_bits(1) == 0)
							continue;

						for (Character *vch = ch->in_room->people; vch; vch = vch->next) {
							if (can_see_char(rch, vch)
							    && is_same_group(vch, victim)
							    && number_range(0, number) == 0) {
								target = vch;
								number++;
							}
						}
					}
				}
			}
			else { // ch is a PC
				if (rch->act_flags.has(PLR_AUTOASSIST)
				 && affect::exists_on_char(ch, affect::type::charm_person)
				 && is_same_group(ch, rch))
					target = victim;
			}
		}
		else { // rch is a PC
			if (IS_NPC(rch)) {
				if (rch->off_flags.has(ASSIST_PLAYERS)
				 && rch->level + 6 > victim->level) {
					target = victim;
				}
				else if (affect::exists_on_char(rch, affect::type::charm_person)
				 && is_same_group(ch, rch))
					target = victim;
			}
			else { // ch is a PC
				// BOTH PC

				if (rch->act_flags.has(PLR_AUTOASSIST)
				 && is_same_group(ch, rch))
					target = victim;
			}
		}

		if (target == nullptr)
			continue;

		if (is_safe(rch, target, FALSE))
			continue;

		if (IS_NPC(rch))
			do_emote(rch, "screams and attacks!");

		multi_hit(rch, target, skill::type::unknown);
	}
} /* end check_assist */

void check_protection_aura(Character *ch, Character *victim) {

	if ((IS_EVIL(ch)
	  && affect::exists_on_char(victim, affect::type::protection_evil))
	 || (IS_GOOD(ch)
	  && affect::exists_on_char(victim, affect::type::protection_good))) {
		const affect::Affect *paf = IS_EVIL(ch) ?
			affect::find_on_char(victim, affect::type::protection_evil) :
			affect::find_on_char(victim, affect::type::protection_good);

		if (paf != nullptr) {
			if (paf->evolution >= 2) {
				if (number_range(1,100) <= paf->evolution * 3) {
					char buf[MSL];
					DAZE_STATE(ch, 2 * PULSE_VIOLENCE);

					Format::sprintf(buf, "The impact with $N's %s aura sends a numbing shock through your arm!",
						IS_EVIL(ch) ? "holy" : "unholy");
					act(buf, ch, nullptr, victim, TO_CHAR);
					Format::sprintf(buf, "Your %s aura shocks $n's arm!",
						IS_EVIL(ch) ? "holy" : "unholy");
					act(buf, ch, nullptr, victim, TO_VICT);
					Format::sprintf(buf, "$N's %s aura shocks $n's arm!",
						IS_EVIL(ch) ? "holy" : "unholy");
					act(buf, ch, nullptr, victim, TO_NOTVICT);
				}
			}

			if (paf->evolution >= 3) {
				if (number_range(1,100) <= paf->evolution) {
					char buf[MSL];
					WAIT_STATE(ch, 2 * PULSE_VIOLENCE);

					Format::sprintf(buf, "$N's %s aura knocks you back, momentarily stunning you!",
						IS_EVIL(ch) ? "holy" : "unholy");
					act(buf, ch, nullptr, victim, TO_CHAR);
					Format::sprintf(buf, "Your %s aura knocks $n back, momentarily stunning $s!",
						IS_EVIL(ch) ? "holy" : "unholy");
					act(buf, ch, nullptr, victim, TO_VICT);
					Format::sprintf(buf, "$N's %s aura knocks $n back, momentarily stunning $n!",
						IS_EVIL(ch) ? "holy" : "unholy");
					act(buf, ch, nullptr, victim, TO_NOTVICT);
				}
			}
		}
	}
}

void multi_hit(Character *ch, Character *victim, skill::type attack_skill)
{
	Object *obj;
	int chance;

	// is this the best place to reset combat timer?  hanging it on the violence
	// update alone sometimes didn't work for one-shot kills.  multi-hit appears to be
	// called everywhere that initiates violence
	if (!IS_NPC(ch))
		ch->pcdata->combattimer = 5;

	/* no attacks for stunnies -- just a check */
	if (get_position(ch) < POS_RESTING)
		return;

	if (IS_NPC(ch)) {
		mob_hit(ch, victim, attack_skill);
		return;
	}

	one_hit(ch, victim, attack_skill, FALSE);

	if (!ch->fighting)
		return;

	if (get_eq_char(ch, WEAR_SECONDARY)) {
		chance = get_skill_level(ch, skill::type::dual_wield);

		if (CAN_USE_RSKILL(ch, skill::type::dual_second))
			chance += get_skill_level(ch, skill::type::dual_second) / 10;

		chance += ((get_evolution(ch, skill::type::dual_wield) - 1) * 5);

		if (chance(chance)) {
			one_hit(ch, victim, attack_skill, TRUE);
			check_improve(ch, skill::type::dual_wield, TRUE, 6);

			if (!ch->fighting)
				return;
		}
		else
			check_improve(ch, skill::type::dual_wield, FALSE, 6);
	}

	if (affect::exists_on_char(ch, affect::type::haste))
		one_hit(ch, victim, attack_skill, FALSE);

	if (!ch->fighting || attack_skill == skill::type::backstab)
		return;

	chance = get_skill_level(ch, skill::type::second_attack) / 2;

	if (CAN_USE_RSKILL(ch, skill::type::fourth_attack))
		chance += get_skill_level(ch, skill::type::fourth_attack) / 10;

	if (affect::exists_on_char(ch, affect::type::slow))
		chance /= 2;

	if (chance(chance)) {
		one_hit(ch, victim, attack_skill, FALSE);
		check_improve(ch, skill::type::second_attack, TRUE, 5);

		if (!ch->fighting)
			return;
	}

	chance = get_skill_level(ch, skill::type::third_attack) / 4;

	if (CAN_USE_RSKILL(ch, skill::type::fourth_attack))
		chance += get_skill_level(ch, skill::type::fourth_attack) / 10;

	if (affect::exists_on_char(ch, affect::type::slow))
		chance = 0;

	if (chance(chance)) {
		one_hit(ch, victim, attack_skill, FALSE);
		check_improve(ch, skill::type::third_attack, TRUE, 6);

		if (!ch->fighting)
			return;
	}

	chance = get_skill_level(ch, skill::type::fourth_attack) / 2;

	if (affect::exists_on_char(ch, affect::type::slow) || !CAN_USE_RSKILL(ch, skill::type::fourth_attack))
		chance = 0;

	if (chance(chance)) {
		one_hit(ch, victim, attack_skill, FALSE);
		check_improve(ch, skill::type::fourth_attack, TRUE, 6);

		if (!ch->fighting)
			return;
	}

	if (get_eq_char(ch, WEAR_SECONDARY)) {
		chance = get_skill_level(ch, skill::type::dual_second) / 2;

		if (affect::exists_on_char(ch, affect::type::slow) || !CAN_USE_RSKILL(ch, skill::type::dual_second))
			chance = 0;

		chance += ((get_evolution(ch, skill::type::dual_wield) - 1) * 5);

		if (chance(chance)) {
			one_hit(ch, victim, attack_skill, TRUE);
			check_improve(ch, skill::type::dual_second, TRUE, 6);

			if (!ch->fighting)
				return;
		}
		else
			check_improve(ch, skill::type::dual_second, FALSE, 6);
	}

	check_protection_aura(ch, victim);

	if (attack_skill == skill::type::shadow_form || attack_skill == skill::type::circle)
		return;

	/* remort affect - clumsiness */
	if (HAS_RAFF(ch, RAFF_CLUMSY) && get_position(ch) > POS_RESTING) {
		if (number_range(1, 100) == 1) {
			act("In a spectacular display of clumsiness, $n falls down!", ch, nullptr, nullptr, TO_ROOM);
			stc("You lose your footing avoiding the next strike, and fall down!  DOH!!\n", ch);
			DAZE_STATE(ch, 3 * PULSE_VIOLENCE);
			WAIT_STATE(ch, 3 * PULSE_VIOLENCE);
			ch->position = POS_RESTING;
		}
	}

	/* remort affect - weak grip */
	if ((obj = get_eq_char(ch, WEAR_WIELD)) != nullptr
	    && !affect::exists_on_char(ch, affect::type::talon)
	    && !IS_OBJ_STAT(obj, ITEM_NOREMOVE)
	    && HAS_RAFF(ch, RAFF_WEAKGRIP)) {
		if (number_range(1, 100) == 1) {
			act("$n goes to starts to attack, but $s weapon flies from $s grip!", ch, nullptr, nullptr, TO_ROOM);
			stc("{PCurses!  You've dropped your weapon again!{x\n", ch);
			obj_from_char(obj);

			if (IS_OBJ_STAT(obj, ITEM_NODROP)
			    || IS_OBJ_STAT(obj, ITEM_INVENTORY)
			    || ch->in_room == nullptr
			    || ch->in_room->sector_type == SECT_ARENA
			    || char_in_darena_room(ch))
				obj_to_char(obj, ch);
			else
				obj_to_room(obj, ch->in_room);
		}
	}
} /* end multi_hit */

/* procedure for all mobile attacks */
void mob_hit(Character *ch, Character *victim, skill::type attack_skill)
{
	Character *vch, *vch_next;
	int chance, number;

	// this is a weird way to make mobs with OFF_BACKSTAB start fights with do_backstab.
	// we call do_backstab here, which in turn calls mob_hit (this function).  the
	// difference is that do_backstab will call with attack_skill == skill::type::backstab, and we avoid
	// a loop.
	if (attack_skill != skill::type::backstab // avoid loop
		&& ch->fighting == nullptr
	    && victim->hit == GET_MAX_HIT(victim)
	    && (get_eq_char(ch, WEAR_WIELD) != nullptr)
	    && ch->off_flags.has(OFF_BACKSTAB)
	    && get_skill_level(ch, skill::type::backstab)) {
		do_backstab(ch, victim->name);
		return;
	}

	one_hit(ch, victim, attack_skill, FALSE);

	if (!ch->fighting)
		return;

	if (get_eq_char(ch, WEAR_SECONDARY) && (number_percent() < (ch->level * 2))) {
		one_hit(ch, victim, attack_skill, TRUE);

		if (!ch->fighting)
			return;
	}

	/* Area attack -- BALLS nasty! */
	if (ch->off_flags.has(OFF_AREA_ATTACK)) {
		for (vch = ch->in_room->people; vch != nullptr; vch = vch_next) {
			vch_next = vch->next;

			if ((vch != victim && vch->fighting == ch && chance(33)))
				one_hit(ch, vch, attack_skill, FALSE);
		}
	}

	if (affect::exists_on_char(ch, affect::type::haste)
	    || (ch->off_flags.has(OFF_FAST) && !affect::exists_on_char(ch, affect::type::slow)))
		one_hit(ch, victim, attack_skill, FALSE);

	if (!ch->fighting || attack_skill == skill::type::backstab)
		return;

	chance = get_skill_level(ch, skill::type::second_attack) / 2;

	if (affect::exists_on_char(ch, affect::type::slow) && !ch->off_flags.has(OFF_FAST))
		chance /= 2;

	if (chance(chance)) {
		one_hit(ch, victim, attack_skill, FALSE);

		if (!ch->fighting)
			return;
	}

	chance = get_skill_level(ch, skill::type::third_attack) / 4;

	if (affect::exists_on_char(ch, affect::type::slow) && !ch->off_flags.has(OFF_FAST))
		chance = 0;

	if (chance(chance)) {
		one_hit(ch, victim, attack_skill, FALSE);

		if (!ch->fighting)
			return;
	}

	chance = get_skill_level(ch, skill::type::fourth_attack) / 6;

	if (affect::exists_on_char(ch, affect::type::slow) && !ch->off_flags.has(OFF_FAST))
		chance = 0;

	if (chance(chance)) {
		one_hit(ch, victim, attack_skill, FALSE);

		if (!ch->fighting)
			return;
	}

	check_protection_aura(ch, victim);

	if (ch->wait > 0 || get_position(ch) < POS_FIGHTING)
		return;

	/* now for the skills */
	number = number_range(0, 6);

	switch (number) {
	case (0) :
		if (ch->off_flags.has(OFF_BASH))
			do_bash(ch, "");

		break;

	case (1) :
		if (ch->off_flags.has(OFF_BERSERK) && !affect::exists_on_char(ch, affect::type::berserk))
			do_berserk(ch, "");

		break;

	case (2) :
		if (ch->off_flags.has(OFF_DISARM)
		    || (get_weapon_skill(ch, FALSE) != skill::type::hand_to_hand
		        && (ch->act_flags.has(ACT_WARRIOR)
		            ||  ch->act_flags.has(ACT_THIEF))))
			do_disarm(ch, "");

		break;

	case (3) :
		if (ch->off_flags.has(OFF_KICK))
			do_kick(ch, "");

		break;

	case (4) :
		if (ch->off_flags.has(OFF_KICK_DIRT))
			do_dirt(ch, "");

		break;

	case (5) :
		if (ch->off_flags.has(OFF_TRIP))
			do_trip(ch, "");

		break;

	case (6) :
		if (ch->off_flags.has(OFF_CRUSH))
			do_crush(ch, "");

		break;
	}
} /* end mob_hit */

/* Hit one guy once */
void one_hit(Character *ch, Character *victim, skill::type attack_skill, bool secondary)
{
	Object *wield;
	int victim_ac;
	int thac0;
	int thac0_00;
	int thac0_32;
	int dam;
	int diceroll;
	int skill;
	int bonus = 0;
	bool result;
	bool riposte = FALSE;
	bool shadow = FALSE;
	bool no_weapon = FALSE;

	/* just in case */
	if (victim == ch || ch == nullptr || victim == nullptr)
		return;

	/* Can't beat a dead char!  Guard against weird room leavings. */
	if (get_position(victim) == POS_DEAD || ch->in_room != victim->in_room)
		return;

	if (attack_skill == skill::type::riposte)
		riposte = TRUE;

	if (attack_skill == skill::type::shadow_form) {
		shadow = TRUE;
		bonus = ((ch->level * 2) + (victim->level * 2));
		attack_skill = skill::type::backstab;
	}

	/* Figure out the type of damage message. */
	if (!secondary)
		wield = get_eq_char(ch, WEAR_WIELD);
	else
		wield = get_eq_char(ch, WEAR_SECONDARY);

	/* reworked the old confusing way of overloading the 'dt' variable to represent
	   either a skill (<1000) or an attack table lookup (1000-1030).  The skill is
	   already set, it could be skill::type::unknown which is just a regular hit.
	   Next we need to figure out the index into the attack table, which will give
	   us a damage noun (if no skill) and a damage type.
	 */

	int attack_type = 0; // index into attack table, where we get a damage noun and damage type

	if (wield != nullptr && wield->item_type == ITEM_WEAPON)
		attack_type = wield->value[3];
	else {
		attack_type = ch->dam_type;
		no_weapon = TRUE;     /* check if unarmed -- Outsider */
	}

	// constrain to the table size
	if (attack_type < 0 || (unsigned int)attack_type > attack_table.size()) {
		Logging::bugf("damage(): unknown attack type %d", attack_type);
		attack_type = 0;
	}

	int dam_type = attack_table[attack_type].damage;

	/* get the weapon skill */
	skill::type sn = get_weapon_skill(ch, secondary);
	skill = 20 + get_weapon_learned(ch, sn);

	/* If the char is unarmed and has the Unarmed skill, add that in.
	   -- Outsider
	*/
	if ((no_weapon) && (get_skill_level(ch, skill::type::unarmed) > 1)) {
		skill += get_skill_level(ch, skill::type::unarmed);
		check_improve(ch, skill::type::unarmed, TRUE, 1);
	}

	/* Calculate to-hit-armor-class-0 versus armor. */
	if (IS_NPC(ch)) {
		thac0_00 = 20;
		thac0_32 = -4;   /* as good as a thief */

		if (ch->act_flags.has(ACT_WARRIOR))
			thac0_32 = -10;
		else if (ch->act_flags.has(ACT_THIEF))
			thac0_32 = -4;
		else if (ch->act_flags.has(ACT_CLERIC))
			thac0_32 = 2;
		else if (ch->act_flags.has(ACT_MAGE))
			thac0_32 = 6;
	}
	else {
		thac0_00 = class_table[ch->cls].thac0_00;
		thac0_32 = class_table[ch->cls].thac0_32;
	}

	thac0 = interpolate(ch->level, thac0_00, thac0_32);

	if (thac0 < 0)
		thac0 = thac0 / 2;

	if (thac0 < -5)
		thac0 = -5 + (thac0 + 5) / 2;

	thac0 -= (GET_ATTR_HITROLL(ch) * skill) / 100;
	thac0 += (5 * (100 - skill)) / 100;

	if (attack_skill == skill::type::backstab)
		thac0 -= 10 * (100 - get_skill_level(ch, skill::type::backstab));

	if (attack_skill == skill::type::circle)
		thac0 -= 10 * (100 - get_skill_level(ch, skill::type::circle));

	if (attack_skill == skill::type::rage)
		thac0 -= 10 * (100 - get_skill_level(ch, skill::type::rage));

	switch (dam_type) {
	case (DAM_PIERCE):       victim_ac = GET_AC(victim, AC_PIERCE) / 10;        break;

	case (DAM_BASH):         victim_ac = GET_AC(victim, AC_BASH) / 10;          break;

	case (DAM_SLASH):        victim_ac = GET_AC(victim, AC_SLASH) / 10;         break;

	default:                victim_ac = GET_AC(victim, AC_EXOTIC) / 10;        break;
	};

	if (victim_ac < -15)
		victim_ac = (victim_ac + 15) / 5 - 15;

	if (!can_see_char(ch, victim))
		victim_ac -= 4;

	if (get_position(victim) < POS_FIGHTING)
		victim_ac += 4;

	if (get_position(victim) < POS_RESTING)
		victim_ac += 6;

	/* The moment of excitement! */
	while ((diceroll = number_bits(5)) >= 20)
		;

	/* Avoid the probability calculation if riposting or shadow forming, they do their own checks */
	if (!riposte && !shadow) {
		if (diceroll == 0 || (diceroll != 19 && diceroll < thac0 - victim_ac)) {
			/* Miss. */
			damage(ch, victim, 0, attack_skill, attack_type, dam_type, TRUE, FALSE);
			tail_chain();
			return;
		}
	}

	/* Hit.  Calc damage. */
	if (IS_NPC(ch) && wield == nullptr)
		dam = dice(ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);
	else {
		if (sn != skill::type::unknown)
			check_improve(ch, sn, TRUE, 5);

		if (wield != nullptr) {
			dam = (1L * dice(wield->value[1], wield->value[2]) * skill) / 100L;

			if (get_eq_char(ch, WEAR_SHIELD) == nullptr) /* no shield = more */
				dam = dam * 11 / 10;

			/* sharpness! */
			if (affect::exists_on_obj(wield, affect::type::weapon_sharp)) {
				int percent;

				if ((percent = number_percent()) <= (skill / 8))
					dam = 2 * dam + (dam * 2 * percent / 100);
			}
		}
		else
			dam = number_range(1 + 4 * skill / 100, 2 * ch->level / 3 * skill / 100);
	}

	/* Bonuses. */
	if (get_skill_level(ch, skill::type::enhanced_damage)
	    && (diceroll = number_percent()) <= get_skill_level(ch, skill::type::enhanced_damage)) {
		check_improve(ch, skill::type::enhanced_damage, TRUE, 6);
		dam += 2 * (dam * diceroll / 300);
	}

	if (!IS_AWAKE(victim))
		dam *= 2;
	else if (get_position(victim) < POS_FIGHTING)
		dam = dam * 3 / 2;

	if (attack_skill == skill::type::backstab && wield != nullptr) {
		if (wield->value[0] != 2)
			dam *= 2 + (ch->level / 10);
		else
			dam *= 2 + (ch->level / 8);           /* daggers do more */
		/*Vegita - evolved backstab damage bonuses*/
		switch (get_evolution(ch, skill::type::backstab)) {
			case 1:							//no bonus
				break;
			
			case 2:
				dam += ((10 * dam ) / 100); //%10 bonus
				break;
			
			case 3:
				dam += ((20 * dam ) / 100); //%20 bonus
				break;
				
			default:
				break;
		}
	}

	if (attack_skill == skill::type::circle && wield != nullptr) {
		if (wield->value[0] != 2)
			dam *= 2 + ((ch->level - 30) / 30);
		else
			dam *= 2 + ((ch->level - 30) / 24);   /* daggers do more */
	}

	if (attack_skill == skill::type::rage && wield != nullptr) {
		if (wield->value[0] != 1
		    && wield->value[0] != 5)
			dam *= 2 + ((ch->level - 30) / 30);
		else
			dam *= 2 + ((ch->level - 30) / 24);   /* swords and axes do more */
	}

	dam += GET_ATTR_DAMROLL(ch) * UMIN(100, skill) / 100;

	if (dam <= 0)
		dam = 1;

	dam += bonus;  /* Shadow Form Bonus */
	result = damage(ch, victim, dam, attack_skill, attack_type, dam_type, TRUE, FALSE);

	/* but do we have a funky weapon? */
	if (result && wield != nullptr) {
		const affect::Affect *weaponaff;
		int dam, level, evolution;

		if (ch->fighting == victim
		 && (weaponaff = affect::find_on_obj(wield, affect::type::poison)) != nullptr) {
			level = weaponaff->level;
			evolution = weaponaff->evolution;

			if (!saves_spell(level / 2, victim, DAM_POISON)) {
				stc("You feel poison coursing through your veins.\n", victim);
				act("$n is poisoned by the venom on $p.", victim, wield, nullptr, TO_ROOM);

				affect::add_type_to_char(victim,
					affect::type::poison,
					level,
					level / 2,
					evolution,
					FALSE
				);
			}

			/* weaken the poison if it's temporary */
			if (weaponaff != nullptr) {
				affect::fn_data_container_type container = { affect::type::poison };
				affect::iterate_over_obj(
					wield,
					affect::fn_fade_spell,
					&container
				);

				if (weaponaff->duration == 0)
					act("The poison on $p has worn off.", ch, wield, nullptr, TO_CHAR);
			}
		}

		if (ch->fighting == victim
		 && (weaponaff = affect::find_on_obj(wield, affect::type::weapon_vampiric)) != nullptr) {
			evolution = weaponaff->evolution;

			dam = number_range(1, wield->level / 5 + 1);

			if (ch->in_room->sector_type != SECT_ARENA
			    && ch->in_room->sector_type != SECT_CLANARENA
			    && (ch->in_room->area != Game::world().quest.area || !Game::world().quest.pk))
				gain_exp(victim, 0 - number_range(ch->level / 20, 3 * ch->level / 20));

			if (ch->in_room->sector_type != SECT_ARENA
			    && ch->in_room->sector_type != SECT_CLANARENA
			    && (ch->in_room->area != Game::world().quest.area || !Game::world().quest.pk)
			    && ch->cls != PALADIN_CLASS) /* Paladins */
				ch->alignment = UMAX(-1000, ch->alignment - 1);

			act("$p draws life from $n.", victim, wield, nullptr, TO_ROOM);
			act("You feel $p drawing your life away.", victim, wield, nullptr, TO_CHAR);
			damage(ch, victim, dam, skill::type::unknown, -1, DAM_NEGATIVE, FALSE, TRUE);
			ch->hit += dam / 2;
		}

		if (ch->fighting == victim
		 && (weaponaff = affect::find_on_obj(wield, affect::type::weapon_vorpal)) != nullptr) {
			dam = number_range(1, wield->level / 4 + 1);
			act("$n is impaled by $p.", victim, wield, nullptr, TO_ROOM);
			act("$p impales your body.", victim, wield, nullptr, TO_CHAR);
			damage(ch, victim, dam, skill::type::unknown, -1, DAM_PIERCE, FALSE, FALSE);
		}

		if (ch->fighting == victim
		 && (weaponaff = affect::find_on_obj(wield, affect::type::weapon_acidic)) != nullptr) {
			evolution = weaponaff->evolution;

			dam = number_range(1, wield->level / 4 + 2);
			act("$n is burned by the acid $p.", victim, wield, nullptr, TO_ROOM);
			act("The acid on $p starts to burn your flesh.", victim, wield, nullptr, TO_CHAR);

			if (victim->in_room->sector_type != SECT_ARENA
			    && ch->in_room->sector_type != SECT_CLANARENA
			    && (ch->in_room->area != Game::world().quest.area || !Game::world().quest.pk))
				acid_effect((void *) victim, wield->level / 2, dam, TARGET_CHAR, evolution);

			damage(ch, victim, dam, skill::type::unknown, -1, DAM_ACID, FALSE, FALSE);
		}

		if (ch->fighting == victim
		 && (weaponaff = affect::find_on_obj(wield, affect::type::weapon_flaming)) != nullptr) {
			evolution = weaponaff->evolution;

			dam = number_range(1, wield->level / 4 + 1);
			act("$n is burned by $p.", victim, wield, nullptr, TO_ROOM);
			act("$p sears your flesh.", victim, wield, nullptr, TO_CHAR);

			if (victim->in_room->sector_type != SECT_ARENA
			    && ch->in_room->sector_type != SECT_CLANARENA
			    && (ch->in_room->area != Game::world().quest.area || !Game::world().quest.pk))
				fire_effect((void *) victim, wield->level / 2, dam, TARGET_CHAR, evolution);

			damage(ch, victim, dam, skill::type::unknown, -1, DAM_FIRE, FALSE, FALSE);
		}

		if (ch->fighting == victim
		 && (weaponaff = affect::find_on_obj(wield, affect::type::weapon_frost)) != nullptr) {
			evolution = weaponaff->evolution;

			dam = number_range(1, wield->level / 6 + 2);
			act("$p freezes $n.", victim, wield, nullptr, TO_ROOM);
			act("The cold touch of $p surrounds you with ice.", victim, wield, nullptr, TO_CHAR);

			if (victim->in_room->sector_type != SECT_ARENA
			    && ch->in_room->sector_type != SECT_CLANARENA
			    && (ch->in_room->area != Game::world().quest.area || !Game::world().quest.pk))
				cold_effect(victim, wield->level / 2, dam, TARGET_CHAR, evolution);

			damage(ch, victim, dam, skill::type::unknown, -1, DAM_COLD, FALSE, FALSE);
		}

		if (ch->fighting == victim
		 && (weaponaff = affect::find_on_obj(wield, affect::type::weapon_shocking)) != nullptr) {
			evolution = weaponaff->evolution;

			dam = number_range(1, wield->level / 5 + 2);
			act("$n is struck by lightning from $p.", victim, wield, nullptr, TO_ROOM);
			act("You are shocked by $p.", victim, wield, nullptr, TO_CHAR);

			if (victim->in_room->sector_type != SECT_ARENA
			    && ch->in_room->sector_type != SECT_CLANARENA
			    && (ch->in_room->area != Game::world().quest.area || !Game::world().quest.pk))
				shock_effect(victim, wield->level / 2, dam, TARGET_CHAR, evolution);

			damage(ch, victim, dam, skill::type::unknown, -1, DAM_ELECTRICITY, FALSE, FALSE);
		}
	}

	tail_chain();
} /* end one_hit */

// called on a hit from bone wall
int affect_callback_weaken_bonewall(affect::Affect *node, void *null) {
	if (node->type == affect::type::bone_wall) {
		node->duration = UMAX(0, node->duration - 1);

		if (node->level > 5)
			node->level--;
	}
	return 1; // quit now, only one bonewall affect
}


/* Inflict damage from a hit.
   damage and damage consolidated, bool added to determine whether it's a magic spell or not -- Montrey */
bool damage(Character *ch, Character *victim, int dam, skill::type attack_skill, int attack_type, int dam_type, bool show, bool spell)
{
	bool immune, sanc_immune;

	if (get_position(victim) == POS_DEAD)
		return FALSE;

	/* Stop up any residual loopholes.
	if (dam > 2400 && dt >= TYPE_HIT)
	{
	        Logging::bug("Damage: %d: more than 2400 points!",dam);
	        dam = 2400;

	        if (!IS_IMMORTAL(ch))
	        {
	                Object *obj;
	                obj = get_eq_char(ch,WEAR_WIELD);
	                stc("You really shouldn't cheat.\n",ch);

	                if (obj != nullptr)
	                        extract_obj(obj);
	        }
	} */

	if (spell) {
		int damroll = get_unspelled_damroll(ch); // don't add berserk, frenzy, etc
		
		if (attack_skill == skill::type::magic_missile) {
			dam += damroll;
		}
		else if (
			attack_skill == skill::type::chain_lightning
		 || attack_skill == skill::type::blizzard
		 || attack_skill == skill::type::acid_rain
		 || attack_skill == skill::type::firestorm) {
			dam += damroll/5;
		}
		else {
				dam += damroll;
		}

		if (focus && affect::exists_on_char(ch, affect::type::focus))
			dam += number_range((dam / 4), (dam * 5 / 4));
	}

	/* damage reduction */
	if (dam > 35)
		dam = (dam - 35) / 2 + 35;

	if (dam > 80)
		dam = (dam - 80) / 2 + 80;

	if (victim != ch) {
		/* Certain attacks are forbidden.  Most other attacks are returned. */
		if (is_safe(ch, victim, TRUE))
			return FALSE;

		check_killer(ch, victim);

		if (get_position(victim) > POS_STUNNED) {
			if (!IS_NPC(ch) && !IS_NPC(victim) && ch->fighting == nullptr) {
				char buf[MAX_STRING_LENGTH];
				victim->act_flags -= PLR_NOPK;
				Format::sprintf(buf, "%s is out for blood - En guarde, %s!", ch->name, victim->name);
				do_send_announce(ch, buf);
				Format::sprintf(buf, "$N is attempting to murder %s", victim->name);
				wiznet(buf, ch, nullptr, WIZ_FLAGS, 0, 0);

				if (!victim->pcdata->pktimer)
					victim->pcdata->pktimer = PKTIME;

				ch->pcdata->pktimer = PKTIME;
			}

			if (victim->wait == 0 && get_position(victim) < POS_FIGHTING) {
				act("You clamber to your feet.", victim, nullptr, nullptr, TO_CHAR);
				act("$n clambers to $s feet.", victim, nullptr, nullptr, TO_ROOM);

				victim->position = POS_STANDING;

				if (victim->start_pos == POS_FLYING && CAN_FLY(victim))
					do_fly(victim, "");
			}

			if (victim->fighting == nullptr)
				set_fighting(victim, ch);

			if (ch->fighting == nullptr)
				set_fighting(ch, victim);
		}

		/* More charm stuff. */
		if (victim->master == ch)
			stop_follower(victim);
	}

	/* Inviso attacks ... not. */
	if (affect::exists_on_char(ch, affect::type::invis)
	 || affect::exists_on_char(ch, affect::type::midnight)) {
		affect::remove_type_from_char(ch, affect::type::invis);
		affect::remove_type_from_char(ch, affect::type::midnight);
		act("$n fades into existence.", ch, nullptr, nullptr, TO_ROOM);
	}

	/* Damage modifiers. */
	if (!IS_NPC(victim)) {
		if (dam > 1 && victim->pcdata->condition[COND_DRUNK] > 10)
			dam = 9 * dam / 10;

		if (dam > 1 && victim->cls == 5) /* enhanced protection for paladins */
			if ((IS_GOOD(victim) && IS_EVIL(ch))
			    || (IS_EVIL(victim) && IS_GOOD(ch)))
				dam -= dam / 4;
	}

	/* BARRIER reduces damage by (currently) 25% -- Elrac */
	if (dam > 1 && affect::exists_on_char(victim, affect::type::barrier))
		dam -= dam / 4;

	sanc_immune = FALSE;

	if (dam > 1 && affect::exists_on_char(victim, affect::type::sanctuary)) {
		switch (get_affect_evolution(victim, affect::type::sanctuary)) {
		case 1:
			dam = (dam * 60) / 100;
			break;

		case 2:
			if (affect::exists_on_char(ch, affect::type::curse))
				dam = (dam * 45) / 100;
			else
				dam = (dam * 55) / 100;

			break;

		case 3:
			if (affect::exists_on_char(ch, affect::type::curse))
				dam = (dam * 40) / 100;
			else
				dam = (dam * 50) / 100;

			break;

		case 4:
			if (affect::exists_on_char(ch, affect::type::curse)) {
				dam = (dam * 35) / 100;

				if (dam % 10 == 0) {
					sanc_immune = TRUE;
					dam = 0;
				}
			}
			else
				dam = (dam * 45) / 100;

			break;
		}
	}

	if ((affect::exists_on_char(victim, affect::type::protection_evil) && IS_EVIL(ch))
	    || (affect::exists_on_char(victim, affect::type::protection_good) && IS_GOOD(ch)))
		dam -= dam / 4;

	/* remort affect - more damage */
	if (HAS_RAFF(ch, RAFF_MOREDAMAGE))
		dam += dam / 20;

	/* remort affect - less damage */
	if (HAS_RAFF(ch, RAFF_LESSDAMAGE))
		dam -= dam / 20;

	immune = FALSE;

	if (affect::exists_on_char(victim, affect::type::force_shield) && (dam % 4 == 0) && !sanc_immune) {
		immune = TRUE;
		dam = 0;
	}

	/* Check for parry, blur, shield block, and dodge. */
	// the only things that should get through here are weapon hits, so anything coded with
	// an attack_skill or an attack_type of -1 will bypass this section
	if (attack_skill == skill::type::unknown && attack_type >= 0 && ch != victim) {
		if (IS_AWAKE(victim) && !global_quick) {
			if (check_dodge(ch, victim, attack_skill, attack_type))
				return FALSE;

			if (check_blur(ch, victim, attack_skill, attack_type))
				return FALSE;

			if (check_shblock(ch, victim, attack_skill, attack_type))
				return FALSE;

			if (check_parry(ch, victim, attack_skill, attack_type))
				return FALSE;

			if (check_dual_parry(ch, victim, attack_skill, attack_type))
				return FALSE;
		}

		if (!spell) {
			if (get_eq_char(ch, WEAR_WIELD) != nullptr)
				check_cond(ch, get_eq_char(ch, WEAR_WIELD));

			if (affect::exists_on_char(victim, affect::type::flameshield) && !saves_spell(victim->level, ch, DAM_FIRE)) {
				damage(victim, ch, 5, skill::type::flameshield, -1, DAM_FIRE, TRUE, TRUE);
			}

			if (affect::exists_on_char(victim, affect::type::sanctuary)
			    && get_affect_evolution(victim, affect::type::sanctuary) >= 3
			    && !saves_spell(victim->level, ch, DAM_HOLY))
				damage(victim, ch, 5, skill::type::sanctuary, -1, DAM_HOLY, TRUE, TRUE);

			const affect::Affect *paf;
			if ((paf = affect::find_on_char(victim, affect::type::bone_wall)) != nullptr
			    && !saves_spell(paf->level, ch, DAM_PIERCE)) {
				damage(victim, ch,
				       UMAX(number_range(paf->level * 3 / 4, paf->level * 5 / 4), 5),
				       skill::type::bone_wall, -1, DAM_PIERCE, TRUE, TRUE);

				affect::iterate_over_char(ch, affect_callback_weaken_bonewall, nullptr);
			}
		}
	}

	int def_mod = GET_DEFENSE_MOD(victim, dam_type);

	if (def_mod >= 100)
		immune = TRUE;

	dam -= dam * def_mod / 100;

	/* new damage modification by armor -- Elrac and Sharra */
	if (!IS_NPC(victim) && dam > 0 && !spell) {
		int std_ac, vict_ac;
		long factor, ldam = dam;

		switch (dam_type) {
		case AC_PIERCE:
			vict_ac = GET_AC(victim, AC_PIERCE);
			std_ac = -5 * victim->level;
			break;

		case AC_BASH:
			vict_ac = GET_AC(victim, AC_BASH);
			std_ac = -5 * victim->level;
			break;

		case AC_SLASH:
			vict_ac = GET_AC(victim, AC_SLASH);
			std_ac = -5 * victim->level;
			break;

		default:
			vict_ac = GET_AC(victim, AC_EXOTIC);
			std_ac = (-9 * victim->level) / 2;
			break;
		}

		if (vict_ac < 0) {
			factor =  200L * (std_ac - 101) / (std_ac + vict_ac - 202);

			if (factor == 0L) factor = 1L;

			ldam = (ldam * factor * factor) / 10000L;
			dam = ldam;
		}
	}

	if (show)
		dam_message(ch, victim, dam, attack_skill, attack_type, immune, sanc_immune);

//	if (dam == 0)
//		return FALSE;

	if (!spell)
		check_all_cond(victim);

	/* Hurt the victim.  Inform the victim of his new state. */
	victim->hit -= dam;

	if (dam > GET_MAX_HIT(victim) / 4)
		stc("{PThat really did HURT!{x\n", victim);

	if (victim->hit < GET_MAX_HIT(victim) / 4)
		stc("{PYou sure are BLEEDING!{x\n", victim);

	/* are they dead yet? */
	if (!check_pulse(victim)) {
		kill_off(ch, victim);
		return TRUE;
	}

	/* Sleep spells and extremely wounded folks. */
	if (!IS_AWAKE(victim))
		stop_fighting(victim, FALSE);

	if (victim == ch)
		return TRUE;

	/* Take care of link dead people. */
	if (!IS_NPC(victim) && victim->desc == nullptr) {
		if (number_range(0, victim->wait) == 0) {
			do_recall(victim, "");
			return TRUE;
		}
	}

	tail_chain();
	return TRUE;
} /* end damage() */

bool check_pulse(Character *victim)
{
	if (IS_IMMORTAL(victim) && victim->hit < 1)
		victim->hit = 1;

	/* If the character has the Die Hard skill, then give them
	   a chance to recover a little.
	   -- Outsider
	*/
	int die_hard_skill = get_skill_level(victim, skill::type::die_hard);

	if (die_hard_skill >= 1 && victim->hit < 1) {
		if (chance(die_hard_skill)) {
			/* they have to be dying for this to kick in */
			victim->hit += (GET_ATTR_CON(victim) / 10) * (die_hard_skill / 10);

			if (victim->hit > GET_MAX_HIT(victim))
				victim->hit = GET_MAX_HIT(victim);

			stc("You make an effort to pull yourself together!\n", victim);
			act("$n pulls themself together!\n", victim, nullptr, nullptr, TO_ROOM);
			check_improve(victim, skill::type::die_hard, TRUE, 2);
		}
		else
			check_improve(victim, skill::type::die_hard, FALSE, 2);
	}  /* end of die hard */

	update_pos(victim);

	switch (get_position(victim)) {
	case POS_MORTAL:
		act("$n is mortally wounded, and will die soon, if not aided.", victim, nullptr, nullptr, TO_ROOM);
		stc("You are mortally wounded, and will die soon, if not aided.\n", victim);
		return TRUE;

	case POS_INCAP:
		act("$n is incapacitated and will slowly die, if not aided.", victim, nullptr, nullptr, TO_ROOM);
		stc("You are incapacitated and will slowly die, if not aided.\n", victim);
		return TRUE;

	case POS_STUNNED:
		act("$n is stunned, but will probably recover.", victim, nullptr, nullptr, TO_ROOM);
		stc("You are stunned, but will probably recover.\n", victim);
		return TRUE;

	case POS_DEAD:
		act("$n is DEAD!!", victim, nullptr, nullptr, TO_ROOM);
		stc("You have been KILLED!!\n\n", victim);

		if (victim->level < 50)
			stc("Your corpse can be located in the Morgue, down from the pit.\n", victim);

		return FALSE;

	default:
		return TRUE;
	}
} /* end check_pulse */

void kill_off(Character *ch, Character *victim)
{
	char buf[MAX_STRING_LENGTH];

	// announcements
	if (!IS_NPC(victim)) {
		Format::sprintf(log_buf, "%s killed by %s at %d", victim->name,
		        (IS_NPC(ch) ? ch->short_descr : ch->name), victim->in_room->vnum);
		Logging::log(log_buf);
		Format::sprintf(log_buf, "<PK> %s was slain by %s at [{W%d{x] [{W%d Exp{x]",
		        victim->name, (IS_NPC(ch) ? ch->short_descr : ch->name),
		        ch->in_room->vnum, IS_NPC(ch) ? 0 : gxp);
		wiznet(log_buf, nullptr, nullptr, WIZ_DEATHS, 0, 0);
		Format::sprintf(buf, "%s has been slain by %s.",  victim->name, (IS_NPC(ch) ? ch->short_descr : ch->name));
		do_send_announce(victim, buf);
	}
	else {
		Format::sprintf(log_buf, "%s got ToAsTeD by %s at [{W%d{x] [{W%d Exp{x]",
		        (IS_NPC(victim) ? victim->short_descr : victim->name),
		        (IS_NPC(ch) ? ch->short_descr : ch->name), ch->in_room->vnum, gxp);
		wiznet(log_buf, nullptr, nullptr, WIZ_MOBDEATHS, 0, 0);
	}

	// award exp
	group_gain(ch, victim);

	// raw_kill will extract an NPC, so don't refer to NPC victims after this point
	bool was_NPC = IS_NPC(victim);
	bool is_suicide = (ch == victim);

	// make them die
	raw_kill(victim);

	// suicides?
	if (is_suicide)
		return;

	if ((ch->in_room->sector_type == SECT_ARENA) && !was_NPC && (battle.start))
		deduct_cost(ch, -battle.fee);

	// looting NPC corpse that was just made in raw_kill
	if (!IS_NPC(ch) && was_NPC) {
		Object *corpse, *obj, *obj_next;

		if ((corpse = get_obj_list(ch, "corpse", ch->in_room->contents)) == nullptr
		    || !can_see_obj(ch, corpse))
			return;

		/* we don't use do_get here to eliminate messages on not finding stuff -- Montrey */
		if (corpse->contains) {
			for (obj = corpse->contains; obj; obj = obj_next) {
				obj_next = obj->next_content;

				if (obj->name.has_words("gcash")
				 && !ch->act_flags.has(PLR_AUTOGOLD))
					continue;
				else if (!ch->act_flags.has(PLR_AUTOLOOT))
					continue;

				if (can_see_obj(ch, obj))
					get_obj(ch, obj, corpse);       /* get it */
			}
		}

		if (ch->act_flags.has(PLR_AUTOSAC)) {
			if (ch->act_flags.has_any_of(PLR_AUTOLOOT | PLR_AUTOGOLD) && corpse->contains)
				return;
			else
				do_sacrifice(ch, "corpse");   /* leave if corpse has treasure */
		}
	}

	if (was_NPC)
		return;

	// PC victims only from here, safe to use pointer

	/* 2/3 of the way back to previous level */
	if (victim->exp > exp_per_level(victim, victim->pcdata->points) * victim->level
	    && victim->in_room->sector_type != SECT_ARENA
	    && victim->in_room->sector_type != SECT_CLANARENA
	    && (ch->in_room->area != Game::world().quest.area || !Game::world().quest.pk))
		gain_exp(victim,
		         (2 * (exp_per_level(victim, victim->pcdata->points)*victim->level - victim->exp) / 3));

	if (!IS_NPC(ch)) {

		if (ch->in_room->sector_type == SECT_ARENA
		    || ch->in_room->sector_type == SECT_CLANARENA
		    || (ch->in_room->area == Game::world().quest.area && Game::world().quest.pk)) {
			ch->pcdata->arenakills++;

			if (!IS_IMMORTAL(ch))
				victim->pcdata->arenakilled++;
		}
		else {
			/* Make sure victim PK flag is dropped when char dies. -- Outsider 
			if ((victim->pcdata->flag_killer) && (victim->act_flags.has(PLR_KILLER))) {
				victim->act_flags -= PLR_KILLER;
				victim->act_flags -= PLR_NOPK;
			}
			what? why? not the point of killer flags -- Montrey */

			if (char_opponents(ch, victim))
				war_kill(ch, victim);

			ch->pcdata->pckills++;
			victim->pcdata->pckilled++;

			if (victim->pcdata->plr_flags.has(PLR_PK))
				victim->pcdata->plr_flags -= PLR_PK;

			if (victim->pcdata->pkrank >= ch->pcdata->pkrank) {
				if (ch->pcdata->pkrank < 5)
					ch->pcdata->pkrank++;

				if (victim->pcdata->pkrank > 0)
					victim->pcdata->pkrank--;
			}
		}
	}
} /* end kill_off */

/* character only safety, rooms are not accounted for */
bool is_safe_char(Character *ch, Character *victim, bool showmsg)
{
	if (IS_IMMORTAL(ch))
		return FALSE;

	if (victim->fighting == ch || victim == ch)
		return FALSE;

	/* killing mobiles */
	if (IS_NPC(victim)) {
		if (victim->pIndexData->pShop != nullptr) {
			if (showmsg)
				stc("But then who would you buy supplies from?!\n", ch);

			return TRUE;
		}

		/* no killing healers, trainers, etc */
		if (victim->act_flags.has(ACT_TRAIN)
		    || victim->act_flags.has(ACT_PRACTICE)
		    || victim->act_flags.has(ACT_IS_HEALER)
		    || victim->act_flags.has(ACT_IS_CHANGER)) {
			if (showmsg)
				stc("Have you no moral fibre whatsoever?!\n", ch);

			return TRUE;
		}

		if (!IS_NPC(ch)) {
			/* no pets */
			if (victim->act_flags.has(ACT_PET) && affect::exists_on_char(victim, affect::type::charm_person)) {
				if (showmsg)
					act("But $N looks so cute and cuddly.", ch, nullptr, victim, TO_CHAR);

				return TRUE;
			}

			/* no charmed creatures unless owner */
			if (affect::exists_on_char(victim, affect::type::charm_person) && ch != victim->master) {
				if (showmsg)
					stc("That is not your charmed creature!\n", ch);

				return TRUE;
			}
		}
		else {
			/* mob killing mob */
			if (affect::exists_on_char(victim, affect::type::charm_person)
			    && ch->master != nullptr && victim->master != nullptr
			    && !IS_NPC(ch->master) && !IS_NPC(victim->master)
			    && ch->master != victim->master
			    && is_safe_char(ch->master, victim->master, FALSE)) {
				if (showmsg)
					stc("Their master would consider that an unfriendly act.\n", ch);

				return TRUE;
			}
		}
	}
	/* killing players */
	else {
		/* NPC doing the killing */
		if (IS_NPC(ch)) {
			/* charmed mobs and pets cannot attack players while owned */
			if (affect::exists_on_char(ch, affect::type::charm_person) && ch->master != nullptr
			    && ch->master->fighting != victim) {
				if (showmsg)
					stc("Players are your friends!\n", ch);

				return TRUE;
			}
		}
		/* player doing the killing */
		else {
			if (victim->act_flags.has(PLR_KILLER) || victim->act_flags.has(PLR_THIEF))
				return FALSE;

			if (victim->level > ch->level + 8 || ch->level > victim->level + 8) {
				if (showmsg)
					stc("Pick on someone your own size.\n", ch);

				return TRUE;
			}

			if (char_opponents(ch, victim))
				return FALSE;

			if (!victim->pcdata->plr_flags.has(PLR_PK)) {
				if (showmsg)
					stc("They are not in the mood to PK right now.\n", ch);

				return TRUE;
			}

			if (!ch->pcdata->plr_flags.has(PLR_PK)) {
				if (showmsg)
					stc("You are not in the mood to PK right now.\n", ch);

				return TRUE;
			}
		}
	}

	return FALSE;
}

bool is_safe(Character *ch, Character *victim, bool showmsg)
{
	if (victim->in_room == nullptr || ch->in_room == nullptr)
		return TRUE;

//	if (ch->on != nullptr && ch->on->pIndexData->item_type == ITEM_COACH)
//		return TRUE;

	/* safe room? */
	if (GET_ROOM_FLAGS(victim->in_room).has(ROOM_SAFE)) {
		if (showmsg)
			stc("Oddly enough, in this room you feel peaceful.\n", ch);

		return TRUE;
	}

	if (victim->in_room->sector_type == SECT_ARENA
	    || victim->in_room->sector_type == SECT_CLANARENA
	    || char_in_darena_room(victim))
		return FALSE;

	/* almost anything goes in the quest area if UPK is on */
	if (Game::world().quest.pk
	    && victim->in_room->area == Game::world().quest.area
	    && ch->in_room->area == Game::world().quest.area)
		return FALSE;

	return is_safe_char(ch, victim, showmsg);
}

bool is_safe_spell(Character *ch, Character *victim, bool area)
{
	if (victim->in_room == nullptr || ch->in_room == nullptr)
		return TRUE;

//	if (ch->on != nullptr && ch->on->pIndexData->item_type == ITEM_COACH)
//		return TRUE;

	if (IS_IMMORTAL(ch) && !area)
		return FALSE;

	if (GET_ROOM_FLAGS(ch->in_room).has(ROOM_SAFE))
		return TRUE;

	if (victim == ch && area)
		return TRUE;

	if (victim->fighting == ch || victim == ch)
		return FALSE;

	if (!IS_IMMORTAL(ch) && IS_IMMORTAL(victim))
		return TRUE;

	if (!IS_IMMORTAL(ch) && victim->invis_level > ch->level)
		return TRUE;

	if ((victim->in_room->sector_type == SECT_ARENA
	     || victim->in_room->sector_type == SECT_CLANARENA)
	    && (battle.start))
		return FALSE;

	if (char_in_darena_room(victim))
		return FALSE;

	if (IS_NPC(ch) && ch->act_flags.has(ACT_MORPH) && !IS_NPC(victim))
		return TRUE;

	if (affect::exists_on_char(ch, affect::type::fear))
		return TRUE;

	/* killing mobiles */
	if (IS_NPC(victim)) {
		/* safe room? */
		if (GET_ROOM_FLAGS(victim->in_room).has(ROOM_SAFE))
			return TRUE;

		if (victim->pIndexData->pShop != nullptr)
			return TRUE;

		/* no killing healers, trainers, etc */
		if (victim->act_flags.has(ACT_TRAIN)
		    || victim->act_flags.has(ACT_PRACTICE)
		    || victim->act_flags.has(ACT_IS_HEALER)
		    || victim->act_flags.has(ACT_IS_CHANGER))
			return TRUE;

		if (!IS_NPC(ch)) {
			/* no pets */
			if (victim->act_flags.has(ACT_PET))
				return TRUE;

			/* no charmed creatures unless owner */
			if (affect::exists_on_char(victim, affect::type::charm_person) && (area || ch != victim->master))
				return TRUE;

			/* legal kill? -- cannot hit mob fighting non-group member */
			if (victim->fighting != nullptr && !is_same_group(ch, victim->fighting))
				return TRUE;
		}
		/* area effect spells do not hit other mobs */
		else if (area && !is_same_group(victim, ch->fighting))
			return TRUE;
	}
	/* killing players */
	else {
		if (area && IS_IMMORTAL(victim))
			return TRUE;

		/* NPC doing the killing */
		if (IS_NPC(ch)) {
			/* charmed mobs and pets cannot attack players while owned */
			if (affect::exists_on_char(ch, affect::type::charm_person) && ch->master != nullptr
			    && ch->master->fighting != victim)
				return TRUE;

			/* safe room? */
			if (GET_ROOM_FLAGS(victim->in_room).has(ROOM_SAFE))
				return TRUE;

			/* legal kill? -- mobs only hit players grouped with opponent*/
			if (ch->fighting != nullptr && !is_same_group(ch->fighting, victim))
				return TRUE;
		}
		/* player doing the killing */
		else {
			if (GET_ROOM_FLAGS(victim->in_room).has(ROOM_SAFE))
				return TRUE;

			/* almost anything goes in questland if UPK is up */
			if (Game::world().quest.pk
			    && ch->in_room->area == Game::world().quest.area
			    && victim->in_room->area == Game::world().quest.area)
				return FALSE;

			if (victim->act_flags.has(PLR_KILLER) || victim->act_flags.has(PLR_THIEF))
				return FALSE;

			if (ch->level > victim->level + 8)
				return TRUE;

			if (char_opponents(ch, victim))
				return FALSE;

			if (!victim->pcdata->plr_flags.has(PLR_PK)
			    || !ch->pcdata->plr_flags.has(PLR_PK))
				return TRUE;
		}
	}

	return FALSE;
} /* end is_safe_spell */

/* See if an attack justifies a KILLER flag. */
void check_killer(Character *ch, Character *victim)
{
	/* Follow charm thread to responsible character.  Attacking someone's charmed char is hostile!
	   Beware, this will cause a loop if master->pet->master - Lotus */
	while (affect::exists_on_char(victim, affect::type::charm_person) && victim->master != nullptr)
		victim = victim->master;

	/* NPC's are fair game.  So are killers and thieves. */
	if (IS_NPC(victim)
	    || victim->act_flags.has(PLR_KILLER)
	    || victim->act_flags.has(PLR_THIEF)
	    || victim->in_room->sector_type == SECT_ARENA
	    || victim->in_room->sector_type == SECT_CLANARENA
	    || char_in_darena(victim))
		return;

	/* if in questlands and UPK flag is up, all is fair */
	if (Game::world().quest.pk
	    && victim->in_room != nullptr
	    && victim->in_room->area == Game::world().quest.area
	    && ch->in_room != nullptr
	    && ch->in_room->area == Game::world().quest.area)
		return;

	/* all's fair in war */
	if (char_opponents(ch, victim))
		return;

	/* Charm-o-rama, you can attack your charmed player */
	if (affect::exists_on_char(ch, affect::type::charm_person)) {
		if (ch->master == nullptr) {
			char buf[MAX_STRING_LENGTH];
			Format::sprintf(buf, "Check_killer: %s charmed with no master", IS_NPC(ch) ? ch->short_descr : ch->name);
			Logging::bug(buf, 0);
			affect::remove_type_from_char(ch, affect::type::charm_person);
			return;
		}

		stop_follower(ch);
		return;
	}

	/* NPC's are cool of course (as long as not charmed).
	   Hitting yourself is cool too (bleeding).
	   So is being immortal (Alander's idea).
	   And current killers stay as they are. */
	if (IS_NPC(ch)
	    || ch == victim
	    || IS_IMMORTAL(ch)
	    || ch->act_flags.has(PLR_KILLER)
	    || ch->fighting  == victim)
		return;

	/* Attacking players under a pktimer is okay */
	if (victim->pcdata->pktimer != 0)
		return;

	/* It's okay unless they were sleeping and haven't been attacked recently */
	if ((get_position(victim) >= POS_RESTING) || (affect::exists_on_char(ch, affect::type::sleep)))
		return;

	stc("{P*** You are now a KILLER!! ***{x\n", ch);
	ch->act_flags += PLR_KILLER;
	ch->act_flags += PLR_NOPK;
	ch->pcdata->flag_killer = MAX_KILLER;
	save_char_obj(ch);
} /* end check_killer */

/* Check for parry. */
bool check_parry(Character *ch, Character *victim, skill::type attack_skill, int attack_type)
{
	Object *obj;
	char buf[MAX_STRING_LENGTH];
	int chance;
	String attack;

	if (!get_skill_level(victim, skill::type::parry))
		return FALSE;

	chance = get_skill_level(victim, skill::type::parry) * 2 / 3;

	if (get_eq_char(victim, WEAR_WIELD) == nullptr) {
		if (IS_NPC(victim))
			chance /= 2;
		else
			return FALSE;
	}

	/* skill with your opponent's weapon */
	if ((obj = get_eq_char(ch, WEAR_WIELD)) != nullptr) {
		int skill;

		switch (obj->value[0]) {
		case WEAPON_SWORD:      skill = get_skill_level(victim, skill::type::sword);   break;

		case WEAPON_DAGGER:     skill = get_skill_level(victim, skill::type::dagger);  break;

		case WEAPON_SPEAR:      skill = get_skill_level(victim, skill::type::spear);   break;

		case WEAPON_MACE:       skill = get_skill_level(victim, skill::type::mace);    break;

		case WEAPON_AXE:        skill = get_skill_level(victim, skill::type::axe);     break;

		case WEAPON_FLAIL:      skill = get_skill_level(victim, skill::type::flail);   break;

		case WEAPON_WHIP:       skill = get_skill_level(victim, skill::type::whip);    break;

		case WEAPON_POLEARM:    skill = get_skill_level(victim, skill::type::polearm); break;

		case WEAPON_BOW:        skill = get_skill_level(victim, skill::type::archery);     break;

		default:                skill = UMIN(100, victim->level * 3);   break;
		}

		chance -= (100 - skill) / 4;
	}

	if (!can_see_char(victim, ch))
		chance /= 2;

	if (affect::exists_on_char(victim, affect::type::paralyze))
		chance /= 2;

	chance += victim->level - ch->level;
#ifdef DEBUG_CHANCE
	ptc(ch, "(parry %d%%)", chance);
	ptc(victim, "(parry %d%%)", chance);
#endif

	if (number_percent() >= chance) {
		check_improve(victim, skill::type::parry, FALSE, 10);
		return FALSE;
	}

	/* parry is good, let's figure out the damage message */
	if (attack_skill != skill::type::unknown)
		attack = skill::lookup(attack_skill).noun_damage;
	else {
		if (attack_type < 0 || (unsigned int)attack_type >= attack_table.size()) {
			Logging::bug("Dam_message: bad attack_type %d.", attack_type);
			attack_type = 0;
		}

		attack = attack_table[attack_type].noun;
	}

	if (!victim->act_flags.has(PLR_DEFENSIVE)) {
		Format::sprintf(buf, "{BYou parry $n's {B%s.{x", attack);
		act(buf, ch, nullptr, victim, TO_VICT);
	}

	if (!ch->act_flags.has(PLR_DEFENSIVE)) {
		Format::sprintf(buf, "{R$N{R parries your %s.{x", attack);
		act(buf, ch, nullptr, victim, TO_CHAR);
	}

	if (get_eq_char(victim, WEAR_WIELD) != nullptr)
		check_cond(victim, get_eq_char(victim, WEAR_WIELD));

	check_improve(victim, skill::type::parry, TRUE, 6);

	/* for remorts, do riposte */
	if (CAN_USE_RSKILL(victim, skill::type::riposte))
		do_riposte(victim, ch);

	return TRUE;
} /* end check_parry */

/* Check for parry from the off hand. */
bool check_dual_parry(Character *ch, Character *victim, skill::type attack_skill, int attack_type)
{
	Object *obj;
	char buf[MAX_STRING_LENGTH];
	int chance;
	String attack;

	if (!get_skill_level(victim, skill::type::dual_wield)
	    || !get_skill_level(victim, skill::type::parry)
	    || !get_eq_char(victim, WEAR_SECONDARY)
	    || get_evolution(victim, skill::type::dual_wield) < 2
	    || (attack_skill == skill::type::riposte && get_evolution(victim, skill::type::dual_wield) < 3))
		return FALSE;

	chance = get_skill_level(victim, skill::type::dual_wield) / 3;

	/* skill with your opponent's weapon */
	if ((obj = get_eq_char(ch, WEAR_WIELD)) != nullptr) {
		int skill;

		switch (obj->value[0]) {
		case WEAPON_SWORD:      skill = get_skill_level(victim, skill::type::sword);   break;

		case WEAPON_DAGGER:     skill = get_skill_level(victim, skill::type::dagger);  break;

		case WEAPON_SPEAR:      skill = get_skill_level(victim, skill::type::spear);   break;

		case WEAPON_MACE:       skill = get_skill_level(victim, skill::type::mace);    break;

		case WEAPON_AXE:        skill = get_skill_level(victim, skill::type::axe);     break;

		case WEAPON_FLAIL:      skill = get_skill_level(victim, skill::type::flail);   break;

		case WEAPON_WHIP:       skill = get_skill_level(victim, skill::type::whip);    break;

		case WEAPON_POLEARM:    skill = get_skill_level(victim, skill::type::polearm); break;

		case WEAPON_BOW:        skill = get_skill_level(victim, skill::type::archery);     break;

		default:                skill = UMIN(100, victim->level * 3);   break;
		}

		chance -= (100 - skill) / 5;
	}

	if (!can_see_char(victim, ch))
		chance /= 2;

	if (affect::exists_on_char(victim, affect::type::paralyze))
		chance /= 2;

	chance += victim->level - ch->level;
#ifdef DEBUG_CHANCE
	ptc(ch, "(dlparry %d%%)", chance);
	ptc(victim, "(dlparry %d%%)", chance);
#endif

	if (!chance(chance)) {
		check_improve(victim, skill::type::dual_wield, FALSE, 10);
		return FALSE;
	}

	/* parry is good, let's figure out the damage message */
	if (attack_skill != skill::type::unknown)
		attack = skill::lookup(attack_skill).noun_damage;
	else {
		if (attack_type < 0 || (unsigned int)attack_type >= attack_table.size()) {
			Logging::bug("Dam_message: bad attack_type %d.", attack_type);
			attack_type = 0;
		}

		attack = attack_table[attack_type].noun;
	}

	if (!victim->act_flags.has(PLR_DEFENSIVE)) {
		Format::sprintf(buf, "{BYou parry $n's {B%s with your second weapon!{x", attack);
		act(buf, ch, nullptr, victim, TO_VICT);
	}

	if (!ch->act_flags.has(PLR_DEFENSIVE)) {
		Format::sprintf(buf, "{R$N{R parries your %s with $S second weapon!{x", attack);
		act(buf, ch, nullptr, victim, TO_CHAR);
	}

	check_cond(victim, get_eq_char(victim, WEAR_SECONDARY));
	check_improve(victim, skill::type::dual_wield, TRUE, 10);

	/* hilt strike! */
	if (get_evolution(victim, skill::type::dual_wield) < 3
	    || (attack_skill == skill::type::riposte && get_evolution(victim, skill::type::dual_wield) < 4))
		return TRUE;

	chance = ((get_skill_level(victim, skill::type::dual_wield) + get_skill_level(victim, skill::type::hand_to_hand)) / 3);

	if (!can_see_char(victim, ch))
		chance /= 2;

	if (!chance(chance + victim->level - ch->level))
		return TRUE; /* for dual parry */

	// get our hilt strike type, probably at the end so go backwards
	for (attack_type = attack_table.size() - 1; attack_type >= 0; attack_type--)
		if (attack_table[attack_type].name == "hstrike")
			break;

	if (attack_type < 0) {
		Logging::bug("check_dual_parry: unable to find hilt strike damage type", 0);
		return TRUE;
	}

	/* gonna riposte, last check for dodging/blurring/shield blocking it */
	if (check_dodge(victim, ch, skill::type::unknown, attack_type))
		return TRUE;

	if (check_blur(victim, ch, skill::type::unknown, attack_type))
		return TRUE;

	if (check_shblock(victim, ch, skill::type::unknown, attack_type))
		return TRUE;

	damage(victim, ch, (number_range(1, victim->level) + GET_ATTR_DAMROLL(victim)) / 2,
	       skill::type::unknown, attack_type, DAM_BASH, TRUE, FALSE);
	check_improve(victim, skill::type::hand_to_hand, TRUE, 8);
	return TRUE;
} /* end check_dual_parry */

/* Check for shield block. */
bool check_shblock(Character *ch, Character *victim, skill::type attack_skill, int attack_type)
{
	char buf[MAX_STRING_LENGTH];
	int chance;
	String attack;

	if (!get_skill_level(victim, skill::type::shield_block))
		return FALSE;

	if (get_eq_char(victim, WEAR_SHIELD) == nullptr)
		return FALSE;

	chance = get_skill_level(victim, skill::type::shield_block) * 2 / 5;

	if (affect::exists_on_char(victim, affect::type::paralyze))
		chance /= 2;

	chance += (victim->level - ch->level);
#ifdef DEBUG_CHANCE
	ptc(ch, "(shblock %d%%)", chance);
	ptc(victim, "(shblock %d%%)", chance);
#endif

	if (number_percent() >= chance) {
		check_improve(victim, skill::type::shield_block, FALSE, 10);
		return FALSE;
	}

	/* shield block is good, let's figure out the damage message */
	if (attack_skill != skill::type::unknown)
		attack = skill::lookup(attack_skill).noun_damage;
	else {
		if (attack_type < 0 || (unsigned int)attack_type >= attack_table.size()) {
			Logging::bug("Dam_message: bad attack_type %d.", attack_type);
			attack_type = 0;
		}

		attack = attack_table[attack_type].noun;
	}

	if (!victim->act_flags.has(PLR_DEFENSIVE)) {
		Format::sprintf(buf, "{BYou block $n's {B%s with your shield.{x", attack);
		act(buf, ch, nullptr, victim, TO_VICT);
	}

	if (!ch->act_flags.has(PLR_DEFENSIVE)) {
		Format::sprintf(buf, "{R$N{R blocks your %s with a shield.{x", attack);
		act(buf, ch, nullptr, victim, TO_CHAR);
	}

	check_cond(victim, get_eq_char(victim, WEAR_SHIELD));
	check_improve(victim, skill::type::shield_block, TRUE, 6);
	return TRUE;
} /* end check_shblock */

/* Check for dodge. */
bool check_dodge(Character *ch, Character *victim, skill::type attack_skill, int attack_type)
{
	char buf[MAX_STRING_LENGTH];
	int chance;
	String attack;

	if (!get_skill_level(victim, skill::type::dodge))
		return FALSE;

	chance = get_skill_level(victim, skill::type::dodge) / 2;
	// size affects dodge rate - Montrey (2014)
	chance -= (victim->size - SIZE_MEDIUM) * 5;  // bonus 10% for tiny, -15% for giant
	// evasion checks moved to general dodge/blur - Montrey (2014)
	// stats
	chance += 3 * ((GET_ATTR_DEX(victim)) - (GET_ATTR_DEX(ch)));

	// speed and spells
	if (victim->off_flags.has(OFF_FAST) || affect::exists_on_char(victim, affect::type::haste))
		chance += 15;

	if (ch->off_flags.has(OFF_FAST) || affect::exists_on_char(ch, affect::type::haste))
		chance -= 15;

	if (affect::exists_on_char(victim, affect::type::slow))
		chance -= 15;

	if (affect::exists_on_char(ch, affect::type::slow))
		chance += 15;

	if (!can_see_char(victim, ch))
		chance -= 20;

	if (!can_see_char(ch, victim))
		chance += 20;

//	if (!can_see_char(victim,ch))
//		chance /= 2;
	chance += (victim->level - ch->level) * 2;

	if (affect::exists_on_char(victim, affect::type::paralyze))
		chance /= 2;

#ifdef DEBUG_CHANCE
	ptc(ch, "(dodge %d%%)", chance);
	ptc(victim, "(dodge %d%%)", chance);
#endif
	/*Moderate the result*/
	chance = URANGE(5, chance, 95);

//for testing   chance = 0;

	if (number_percent() >= chance) {
		check_improve(victim, skill::type::dodge, FALSE, 10);
		return FALSE;
	}

	/* dodge is good, let's figure out the damage message */
	if (attack_skill != skill::type::unknown)
		attack = skill::lookup(attack_skill).noun_damage;
	else {
		if (attack_type < 0 || (unsigned int)attack_type >= attack_table.size()) {
			Logging::bug("Dam_message: bad attack_type %d.", attack_type);
			attack_type = 0;
		}

		attack = attack_table[attack_type].noun;
	}

	if (!victim->act_flags.has(PLR_DEFENSIVE)) {
		Format::sprintf(buf, "{BYou dodge $n's {B%s.{x", attack);
		act(buf, ch, nullptr, victim, TO_VICT);
	}

	if (!ch->act_flags.has(PLR_DEFENSIVE)) {
		Format::sprintf(buf, "{R$N{R dodges your %s.{x", attack);
		act(buf, ch, nullptr, victim, TO_CHAR);
	}

//	}
	check_improve(victim, skill::type::dodge, TRUE, 6);
	return TRUE;
} /* end check_dodge */

/* Check for Blur - Montrey */
bool check_blur(Character *ch, Character *victim, skill::type attack_skill, int attack_type)
{
	char buf[MAX_STRING_LENGTH];
	int chance;
	String attack;

	if (!CAN_USE_RSKILL(victim, skill::type::blur))
		return FALSE;

	chance = get_skill_level(victim, skill::type::blur) / 3;
	// size affects dodge rate - Montrey (2014)
	chance -= (victim->size - SIZE_MEDIUM) * 5;  // bonus 10% for tiny, -15% for giant
	// evasion checks moved to general dodge/blur - Montrey (2014)
	// stats
	chance += 3 * ((GET_ATTR_DEX(victim)) - (GET_ATTR_DEX(ch)));

	// speed and spells
	if (victim->off_flags.has(OFF_FAST) || affect::exists_on_char(victim, affect::type::haste))
		chance += 10;

	if (ch->off_flags.has(OFF_FAST) || affect::exists_on_char(ch, affect::type::haste))
		chance -= 10;

	if (affect::exists_on_char(victim, affect::type::slow))
		chance -= 10;

	if (affect::exists_on_char(ch, affect::type::slow))
		chance += 10;

	if (!can_see_char(victim, ch))
		chance -= 20;

	if (!can_see_char(ch, victim))
		chance += 20;

//	if (!can_see_char(victim,ch))
//		chance /= 2;
	chance += (victim->level - ch->level) * 2;

	if (affect::exists_on_char(victim, affect::type::paralyze))
		chance /= 2;

#ifdef DEBUG_CHANCE
	ptc(ch, "(blur %d%%)", chance);
	ptc(victim, "(blur %d%%)", chance);
#endif
	/*Moderate the result*/
	chance = URANGE(5, chance, 95);

	if (number_percent() >= chance) {
		check_improve(victim, skill::type::blur, FALSE, 10);
		return FALSE;
	}

	/* blur is good, let's figure out the damage message */
	if (attack_skill != skill::type::unknown)
		attack = skill::lookup(attack_skill).noun_damage;
	else {
		if (attack_type < 0 || (unsigned int)attack_type >= attack_table.size()) {
			Logging::bug("Dam_message: bad attack_type %d.", attack_type);
			attack_type = 0;
		}

		attack = attack_table[attack_type].noun;
	}

	if (!victim->act_flags.has(PLR_DEFENSIVE)) {
		Format::sprintf(buf, "{V$n's {V%s is no match for your speed.{x", attack);
		act(buf, ch, nullptr, victim, TO_VICT);
	}

	if (!ch->act_flags.has(PLR_DEFENSIVE)) {
		Format::sprintf(buf, "{M$N {Mblurs with speed as $E evades your %s.{x", attack);
		act(buf, ch, nullptr, victim, TO_CHAR);
	}

//	}
	check_improve(victim, skill::type::blur, TRUE, 6);
	return TRUE;
}  /* end check_blur */

/* Set position of a victim. */
void update_pos(Character *victim)
{
	if (victim->hit > 0) {
		if (victim->position <= POS_STUNNED)
			victim->position = POS_STANDING;
	}
	else if ((IS_NPC(victim) && victim->hit < 1)
	         || (victim->hit <= -11))
		victim->position = POS_DEAD;
	else if (victim->hit <= -6)
		victim->position = POS_MORTAL;
	else if (victim->hit <= -3)
		victim->position = POS_INCAP;
	else
		victim->position = POS_STUNNED;
} /* end update_pos */

/* Start fights. */
void set_fighting(Character *ch, Character *victim)
{
	if (ch->fighting != nullptr) {
		Logging::bug("Set_fighting: already fighting", 0);
		return;
	}

	if (affect::exists_on_char(ch, affect::type::sleep))
		affect::remove_type_from_char(ch, affect::type::sleep);

	ch->fighting = victim;

	if (ch->wait == 0 && ch->position < POS_STANDING) {
		ch->position = POS_STANDING;

		if (ch->start_pos == POS_FLYING && CAN_FLY(ch))
			ch->position = POS_FLYING;
	}
} /* end set_fighting */

/* Stop fights. */
void stop_fighting(Character *ch, bool fBoth)
{
	Character *fch;

	for (fch = char_list; fch != nullptr; fch = fch->next) {
		if (fch == ch || (fBoth && fch->fighting == ch)) {
			fch->fighting = nullptr;
			fch->position = IS_NPC(fch) ? fch->default_pos : fch->start_pos;
			update_pos(fch);
		}
	}
} /* end stop_fighting */

/* Make a corpse out of a character. */
void make_corpse(Character *ch)
{
	Object *corpse;
	Object *obj;
	Object *obj_next;
	String name;

	if (IS_NPC(ch)) {
		name          = ch->short_descr;
		corpse        = create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);

		if (! corpse) {
			Logging::bug("Error creating corpse in make_corpse.", 0);
			return;
		}

		corpse->timer = number_range(3, 6);

		if (ch->gold > 0) {
			obj_to_obj(create_money(ch->gold, ch->silver), corpse);
			ch->gold = 0;
			ch->silver = 0;
		}

		corpse->cost = UMAX(1, (ch->level * 3));
	}
	else {
		name            = ch->name;
		corpse          = create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);

		if (! corpse) {
			Logging::bug("Memory error making corpse in make_corpse.", 0);
			return;
		}

		corpse->timer   = number_range(25, 40);
		ch->act_flags -= PLR_CANLOOT;
		corpse->owner = ch->name;
		/* Corpse Looting - Taken Out
		corpse->owner = nullptr;

		if (ch->gold > 1 || ch->silver > 1)
		{
		        obj_to_obj(create_money(ch->gold / 2, ch->silver/2), corpse);
		        ch->gold -= ch->gold/2;
		        ch->silver -= ch->silver/2;
		}*/
		corpse->cost = 1;
	}

	corpse->level = ch->level;
	corpse->short_descr = Format::format(corpse->short_descr, name);
	corpse->description = Format::format(corpse->description, name);

	/* We crashed today in this:  a mobile had just high enough strength to
	   wield his sword, with a piece of eq that gave a strength bonus.  When
	   he died, this part removed that piece of eq first, causing him to lose
	   the strength and drop the sword, which fubared the whole deal.  So,
	   as a temp fix, going to make this function remove their weapon and put
	   it in their inventory first.  In theory, though, there could be other
	   problems... but I don't have a good way of handling it yet :)  -- Montrey */

	if ((obj = get_eq_char(ch, WEAR_WIELD)) != nullptr)
		unequip_char(ch, obj);

	for (obj = ch->carrying; obj != nullptr; obj = obj_next) {
		bool floating = FALSE;
		obj_next = obj->next_content;

		if (obj->wear_loc == WEAR_FLOAT)
			floating = TRUE;

		obj_from_char(obj);

		if (obj->item_type == ITEM_POTION)
			obj->timer = number_range(500, 1000);

		if (obj->item_type == ITEM_SCROLL)
			obj->timer = number_range(1000, 2500);

		if (IS_OBJ_STAT(obj, ITEM_ROT_DEATH) && !floating) {
			obj->timer = number_range(5, 10);
			obj->extra_flags -= ITEM_ROT_DEATH;
		}

		obj->extra_flags -= ITEM_VIS_DEATH;

		if (IS_OBJ_STAT(obj, ITEM_INVENTORY))
			extract_obj(obj);
		else if (floating) {
			if (IS_OBJ_STAT(obj, ITEM_ROT_DEATH)) { /* get rid of it! */
				if (obj->contains != nullptr) {
					act("$p decays, scattering its contents.", ch, obj, nullptr, TO_ROOM);
				}
				else
					act("$p decays.", ch, obj, nullptr, TO_ROOM);

				destroy_obj(obj);
			}
			else {
				act("$p falls to the floor.", ch, obj, nullptr, TO_ROOM);
				obj_to_room(obj, ch->in_room);
			}
		}
		else
			obj_to_obj(obj, corpse);
	}

	if (char_at_war(ch))
		obj_to_room(corpse, get_room_index(ch->clan->hall));
	else if (IS_NPC(ch) || ch->level >= 50)
		obj_to_room(corpse, ch->in_room);
	else
		obj_to_room(corpse, get_room_index(ROOM_VNUM_MORGUE));
} /* end make_corpse */

/* Improved Death_cry contributed by Diavolo. */
void death_cry(Character *ch)
{
	RoomPrototype *was_in_room;
	char *msg;
	int door, vnum;
	vnum = 0;
	msg = "You hear $n's death cry.";

	switch (number_bits(4)) {
	case  0:
		msg  = "$n collapses at your feet ... DEAD.";
		break;

	case  1:
		if (ch->material.empty())
			msg = "$n splatters blood all over you, the ground, everything...Eeeeeeew!";

		break;

	case  2:
		if (ch->parts_flags.has(PART_GUTS)) {
			msg = "$n spills $s guts all over the floor.";
			vnum = OBJ_VNUM_GUTS;
		}

		break;

	case  3:
		if (ch->parts_flags.has(PART_HEAD)) {
			msg = "$n's severed head plops on the ground.";
			vnum = OBJ_VNUM_SEVERED_HEAD;
		}

		break;

	case  4:
		if (ch->parts_flags.has(PART_HEART)) {
			msg = "$n's heart is torn from $s chest and lays beating at your feet.";
			vnum = OBJ_VNUM_TORN_HEART;
		}

		break;

	case  5:
		if (ch->parts_flags.has(PART_ARMS)) {
			msg = "$n's arm is sliced from $s dead body.";
			vnum = OBJ_VNUM_SLICED_ARM;
		}

		break;

	case  6:
		if (ch->parts_flags.has(PART_LEGS)) {
			msg = "$n's leg is sliced from $s dead body.";
			vnum = OBJ_VNUM_SLICED_LEG;
		}

		break;

	case  7:
		if (ch->parts_flags.has(PART_BRAINS)) {
			msg = "$n's head is shattered, and $s brains splash all over you.";
			vnum = OBJ_VNUM_BRAINS;
		}
	}

	act(msg, ch, nullptr, nullptr, TO_NOTVIEW);

	if (vnum != 0) {
		Object *obj;
		String name;
		name       = IS_NPC(ch) ? ch->short_descr : ch->name;
		obj        = create_object(get_obj_index(vnum), 0);

		if (! obj) {
			Logging::bug("Memory error creating object in death_cry.", 0);
			return;
		}

		obj->timer = number_range(4, 7);
		obj->short_descr = Format::format(obj->short_descr, name);
		obj->description = Format::format(obj->description, name);

		if (obj->item_type == ITEM_FOOD) {
			if (ch->form_flags.has(FORM_POISON))
				obj->value[3] = 1;
			else if (!ch->form_flags.has(FORM_EDIBLE))
				obj->item_type = ITEM_TRASH;
		}

		obj_to_room(obj, ch->in_room);
	}

	if (IS_NPC(ch))
		msg = "Nearby, you hear someone's death cry.";
	else
		msg = "You hear the death cry of a fellow player, maybe friend.";

	was_in_room = ch->in_room;

	for (door = 0; door <= 5; door++) {
		Exit *pexit;

		if ((pexit = was_in_room->exit[door]) != nullptr
		    && pexit->u1.to_room != nullptr
		    && pexit->u1.to_room != was_in_room) {
			ch->in_room = pexit->u1.to_room;
			act(msg, ch, nullptr, nullptr, TO_NOTVIEW);
		}
	}

	ch->in_room = was_in_room;
} /* end death_cry */

void raw_kill(Character *victim)
{
	stop_fighting(victim, TRUE);
	mprog_death_trigger(victim);

	if (victim->in_room->sector_type != SECT_ARENA
	    && victim->in_room->sector_type != SECT_CLANARENA
	    && (victim->in_room->area != Game::world().quest.area || !Game::world().quest.pk)
	    && !char_in_duel_room(victim))
		make_corpse(victim);

	if (IS_NPC(victim)) {
		victim->pIndexData->killed++;
		extract_char(victim, TRUE);
		return;
	}

	// only PCs from here down

	victim->pcdata->pktimer = 0;

	affect::remove_all_from_char(victim, FALSE);

	if (victim->in_room->sector_type != SECT_ARENA
	    && victim->in_room->sector_type != SECT_CLANARENA
	    && (victim->in_room->area != Game::world().quest.area || !Game::world().quest.pk)
	    && !char_in_duel_room(victim)) {
		extract_char(victim, FALSE);

		for (int i = 0; i < 4; i++) {
			victim->armor_base[i] = 100;
		}
	}
	else {
		if (char_in_duel(victim))
			duel_kill(victim);
		else {
			char_from_room(victim);
			char_to_room(victim, get_room_index(ROOM_VNUM_ALTAR));
		}
	}

	victim->position    = POS_RESTING;
	victim->hit         = UMAX(1, victim->hit);
	victim->mana        = UMAX(1, victim->mana);
	victim->stam        = UMAX(1, victim->stam);

	save_char_obj(victim);

	/* force a save of items on the ground, should fix it
	   for crashes after a player dies -- Montrey */
	objstate_save_items();

} /* end raw_kill */

void group_gain(Character *ch, Character *victim)
{
	char buf[MAX_STRING_LENGTH];
	Character *gch;
	Character *lch;
	int xp;
	int members = 0;
	int group_levels = 0;
	int highestlevel = 0;
	Flags vary_int;
	int diff_classes = 1;
	const Flags::Bit vary_bit [] =
	{ Flags::A, Flags::B, Flags::C, Flags::D, Flags::E, Flags::F, Flags::G, Flags::H };

	/* Monsters don't get kill xp's or alignment changes.
	   P-killing doesn't help either.
	   Dying of mortal wounds or poison doesn't give xp to anyone! */

	if (victim == ch)
		return;

	vary_int += vary_bit[ch->cls];

	/* calculate number of group members present and the sum of their levels */
	for (gch = ch->in_room->people; gch != nullptr; gch = gch->next_in_room) {
		if (is_same_group(gch, ch)) {
			members++;
			group_levels += IS_NPC(gch) ? gch->level / 2 : gch->level;

			/* figure out how varied the group is -- Montrey */
			if (!vary_int.has(vary_bit[gch->cls])) {
				vary_int += vary_bit[gch->cls];
				diff_classes++;
			}
		}
	}

	if (members == 0) {
		Logging::bug("Group_gain: members.", members);
		members = 1;
		group_levels = ch->level ;
	}

	/* Determine the highest level of a present group member */
	for (lch = ch->in_room->people; lch != nullptr; lch = lch->next_in_room) {
		if (!is_same_group(lch, ch) || IS_NPC(lch))
			continue;

		if (lch->level > highestlevel)
			highestlevel = lch->level;
	}

	/* distribute exp among the remaining deserving */
	for (gch = ch->in_room->people; gch != nullptr; gch = gch->next_in_room) {
		Object *obj;
		Object *obj_next;

		if (!is_same_group(gch, ch) || IS_NPC(gch))
			continue;

		if (gch->level - highestlevel <= -7) {
			stc("Your powers would be useless to such an advanced group of adventurers.\n", gch);
			continue;
		}

		xp = xp_compute(gch, victim, group_levels, diff_classes);
		gxp = xp;                                   /* Bad bad global*/

		if (ch->in_room->sector_type == SECT_ARENA
		    || ch->in_room->sector_type == SECT_CLANARENA
		    || (ch->in_room->area == Game::world().quest.area && Game::world().quest.pk))
			xp = 0;

		ptc(gch, "{PYou receive %d experience points.{x\n", xp);

		if (!ch->revoke_flags.has(REVOKE_EXP))
			gain_exp(gch, xp);

		/* check for items becoming unwearable due to alignment changes */
		for (obj = ch->carrying; obj != nullptr; obj = obj_next) {
			obj_next = obj->next_content;

			if (obj->wear_loc == WEAR_NONE)
				continue;

			if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch))
			    || (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch))
			    || (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
				act("You are zapped by $p and hastily drop it.", ch, obj, nullptr, TO_CHAR);
				act("$n is zapped by $p and hastily drops it.", ch, obj, nullptr, TO_ROOM);
				obj_from_char(obj);
				obj_to_room(obj, ch->in_room);
			}
		}

		if (gch->act_flags.has(PLR_QUESTOR) && IS_NPC(victim)) {
			if (gch->questmob == victim->pIndexData->vnum) {
				stc("{YYou have almost completed your QUEST!{x\n", gch);
				stc("{YReturn to the questmaster before your time runs out!{x\n", gch);
				gch->questmob = -1;
				Format::sprintf(buf, "{Y:QUEST: {x$N has slain %s", victim->short_descr);
				wiznet(buf, gch, nullptr, WIZ_QUEST, 0, 0);
			}
		}
	}
} /* end group_gain */

/* Compute xp for a kill, adjust alignment of killer.  Edit this function to change xp computations. */
int xp_compute(Character *gch, Character *victim, int total_levels, int diff_classes)
{
	int level_range = victim->level - gch->level;

	/* force remorts to kill tougher mobs to avoid an exp drop off */
	if (IS_REMORT(gch)) {
		int remort_mod = ((gch->pcdata->remort_count * gch->level) / 100);  /* none until r3 */

		if (level_range < remort_mod)
			level_range -= ((remort_mod / 10) + 1);
	}

	int base_exp = 0;

	/* compute the base exp */
	switch (level_range) {
	case -9 :       base_exp =   1;         break;

	case -8 :       base_exp =   2;         break;

	case -7 :       base_exp =   5;         break;

	case -6 :       base_exp =   9;         break;

	case -5 :       base_exp =  11;         break;

	case -4 :       base_exp =  22;         break;

	case -3 :       base_exp =  33;         break;

	case -2 :       base_exp =  50;         break;

	case -1 :       base_exp =  66;         break;

	case  0 :       base_exp =  83;         break;

	case  1 :       base_exp =  99;         break;

	case  2 :       base_exp = 121;         break;

	case  3 :       base_exp = 143;         break;

	case  4 :       base_exp = 165;         break;
	}

	if (level_range > 4)
		base_exp = 160 + 20 * (level_range - 4);

	// should the character change alignment?
	// should this even be here?  needs to go before alignment checks for exp,
	// but maybe should go in its own function
	if (!victim->act_flags.has(ACT_NOALIGN)
	    && victim->in_room->sector_type != SECT_ARENA
	    && victim->in_room->sector_type != SECT_CLANARENA
	    && (victim->in_room->area != Game::world().quest.area || !Game::world().quest.pk)
	    && gch->cls != 5) /* Paladins */
	{
		/* do alignment computations */
		int align = victim->alignment - gch->alignment;
		int change;

		if (align > 500) { /* monster is more good than slayer */
			change = (align - 500) * base_exp / 500 * gch->level / total_levels;
			change = UMAX(1, change);
			gch->alignment -= change;
		}
		else if (align < -500) { /* monster is more evil than slayer */
			change = (-1 * align - 500) * base_exp / 500 * gch->level / total_levels;
			change = UMAX(1, change);
			gch->alignment += change;
		}
		else { /* improve this someday */
			change =  gch->alignment * base_exp / 500 * gch->level / total_levels;
			gch->alignment -= change;
		}

		gch->alignment = URANGE(-1000, gch->alignment, 1000);
	}

	int xp = base_exp;

	/* calculate exp multiplier */
/*	if (!victim->act_flags.has(ACT_NOALIGN)) {
		if (gch->alignment > 500) { // for goodie two shoes
			     if (victim->alignment < -750)              xp = base_exp * 4 / 3;
			else if (victim->alignment < -500)              xp = base_exp * 5 / 4;
			else if (victim->alignment > 250)               xp = base_exp * 3 / 4;
			else if (victim->alignment > 750)               xp = base_exp * 1 / 4;
			else if (victim->alignment > 500)               xp = base_exp * 1 / 2;
		}
		else if (gch->alignment < -500) { // for baddies
			     if (victim->alignment > 750)               xp = base_exp * 5 / 4;
			else if (victim->alignment > 500)               xp = base_exp * 11/10;
			else if (victim->alignment < -750)              xp = base_exp * 1 / 2;
			else if (victim->alignment < -500)              xp = base_exp * 3 / 4;
			else if (victim->alignment < -250)              xp = base_exp * 9 /10;
		}
		else if (gch->alignment > 200) { // a little good
			     if (victim->alignment < -500)              xp = base_exp * 6 / 5;
			else if (victim->alignment > 750)               xp = base_exp * 1 / 2;
			else if (victim->alignment > 0)                 xp = base_exp * 3 / 4;
		}
		else if (gch->alignment < -200) { // a little bad
			     if (victim->alignment > 500)               xp = base_exp * 6 / 5;
			else if (victim->alignment < -750)              xp = base_exp * 1 / 2;
			else if (victim->alignment < 0)                 xp = base_exp * 3 / 4;
		}
		else { // neutral
			     if (victim->alignment > 500
			      || victim->alignment < -500)              xp = base_exp * 4 / 3;
			else if (victim->alignment < 200
			      && victim->alignment > -200)              xp = base_exp * 1 / 2;
		}
	}
*/
	/* more exp at the low levels */
	if (gch->level < 6)
		xp = 10 * xp / (gch->level + 4);

	/* less at high */
	if (gch->level > 35)
		xp =  15 * xp / (gch->level - 25);

	/* reduce for playing time (... why? -- Montrey )
	{
	         compute quarter-hours per level
	        time_per_level = 4 * (get_play_hours(gch) + get_age(gch) / gch->level;
	        time_per_level = URANGE(8,time_per_level,12);

	        if (gch->level < 15)   make it a curve
	                time_per_level = UMAX(time_per_level,(15 - gch->level));

	        xp = xp * time_per_level / 12;
	} */
	/* new to replace above, reduce by up to 1/3 if you're young -- Montrey */
	int years = get_play_seconds(gch) / (MUD_YEAR * MUD_MONTH * MUD_DAY * MUD_HOUR)
	                        + GET_ATTR_MOD(gch, APPLY_AGE);
	xp = xp - (xp / 3) + ((xp / 3) * (URANGE(1, years, 50) / 50));

	/* remort affect - favor of the gods */
	if (HAS_RAFF(gch, RAFF_FAVORGOD) && chance(25)) {
		stc("The gods smile upon you.\n", gch);
		xp = (chance(25) ? (chance(25) ? (xp * 4) : (xp * 3)) : (xp * 2));
	}

	/* remort affect - laughter of the gods */
	if (HAS_RAFF(gch, RAFF_LAUGHTERGOD) && chance(25)) {
		stc("The laughter of the gods falls upon your ears.\n", gch);
		xp = (chance(25) ? (chance(25) ? (xp / 4) : (xp / 3)) : (xp / 2));
	}

	/* remort affect - more experience */
	if (HAS_RAFF(gch, RAFF_MOREEXP))
		xp += xp / 10;

	/* remort affect - less experience */
	if (HAS_RAFF(gch, RAFF_LESSEXP))
		xp -= xp / 10;

	/* randomize the rewards */
	xp = number_range(xp * 3 / 4, xp * 5 / 4);
	int xp90 = 90 * xp / 100; /* 90% limit for one grouped player   */
	/* tried and true formula for group exp calc */
	xp = xp * gch->level / (UMAX(1, total_levels - 1));

	/* modified for groups of more than 1 -- Elrac */
	if (gch->level != total_levels) {
		/* everyone gets a 30% bonus for grouping, +5% for each different class in the group */
		xp = (130 + (diff_classes * 5)) * xp / 100;

		if (xp > xp90) xp = xp90; /* but no more than the single grouped player limit */
	}

	return xp;
} /* end xp_compute */

// TODO: fix this for defense % modifiers
void dam_message(Character *ch, Character *victim, int dam, skill::type attack_skill, int attack_type, bool immune, bool sanc_immune)
{
	char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH], buf3[MAX_INPUT_LENGTH];
	const char *vp;
	String attack;
	char punct;

	if (ch == nullptr || victim == nullptr)
		return;

	if (dam < 0)          vp = "{Gheals{x";
	else if (dam ==   0)  vp = "{Ymisses{x";                      
	else if (dam <=   4)  vp = "{bscratches{x";                   
	else if (dam <=   8)  vp = "{Ggrazes{x";                      
	else if (dam <=  12)  vp = "{Hhits{x";                        
	else if (dam <=  16)  vp = "{Ninjures{x";                     
	else if (dam <=  20)  vp = "{Cwounds{x";                      
	else if (dam <=  24)  vp = "{Bmauls{x";                       
	else if (dam <=  28)  vp = "{Mdecimates{x";                   
	else if (dam <=  32)  vp = "{Vdevastates{x";                  
	else if (dam <=  36)  vp = "{Pmaims{x";                       
	else if (dam <=  40)  vp = "{PM{cU{PT{cI{PL{cA{PT{cE{PS{x";   
	else if (dam <=  44)  vp = "{BDI{NSE{BMB{NOW{BEL{NS{x";       
	else if (dam <=  48)  vp = "{MDI{YSM{MEM{YBE{MRS{x";          
	else if (dam <=  52)  vp = "{BDE{bCA{BPI{bTA{BTE{bS{x";       
	else if (dam <=  56)  vp = "{c::: {RDEMOLISHES {c:::{x";      
	else if (dam <=  60)  vp = "{T*** {CSTOMPS {T***{x";          
	else if (dam <=  75)  vp = "{M@@@ {VINCINERATES {M@@@{x";     
	else if (dam <= 100)  vp = "{N=== {BOBLITERATES {N==={x";     
	else if (dam <= 125)  vp = "{H>>> {GANNIHILATES {H<<<{x";     
	else if (dam <= 150)  vp = "{b<<< {YERADICATES {b>>>{x";      
	else if (dam <= 170)  vp = "{R[#[ {PVAPORIZES {R]#]{x";       
	else if (dam <= 200) 
		vp = "{cdoes {PUN{RSP{PEA{RKA{PBL{RE {cthings to{x";
	else
		vp = "{Tbrings a touch of {RD{cE{RA{cT{RH {Tto{x";

	punct   = (dam <= 24) ? '.' : '!';

	if (attack_skill != skill::type::unknown)
		attack = skill::lookup(attack_skill).noun_damage;
	else {
		if (attack_type < 0 || (unsigned int)attack_type >= attack_table.size()) {
			Logging::bug("Dam_message: bad attack_type %d.", attack_type);
			attack_type = 0;
		}

		attack = attack_table[attack_type].noun;
	}

	if (immune) {
		if (ch == victim) {
			Format::sprintf(buf1, "$n is unaffected by $s own %s.", attack);
			Format::sprintf(buf2, "Luckily, you are immune to that.");
		}
		else {
			Format::sprintf(buf1, "$N is unaffected by $n's %s!", attack);
			Format::sprintf(buf2, "$N is unaffected by your %s!", attack);
			Format::sprintf(buf3, "$n's %s is powerless against you.", attack);
		}
	}
	else if (sanc_immune) {
		if (ch == victim) {
			Format::sprintf(buf1, "$n's holy aura protects $m from $s own %s.", attack);
			Format::sprintf(buf2, "Your sanctuary protects you from your own %s.", attack);
		}
		else {
			Format::sprintf(buf1, "$N's sanctuary repels $n's cursed %s!", attack);
			Format::sprintf(buf2, "$N's sanctuary repels your cursed %s!", attack);
			Format::sprintf(buf3, "$n's %s fails to penetrate your sanctuary.", attack);
		}
	}
	else {
		if (ch == victim) {
			Format::sprintf(buf1, "$n's %s %s $m%c [%d]" , attack, vp, punct, dam);
			Format::sprintf(buf2, "Your %s %s you%c [%d]", attack, vp, punct, dam);
		}
		else {
			Format::sprintf(buf1, "$n's %s %s $N%c [%d]" , attack, vp, punct, dam);
			Format::sprintf(buf2, "Your %s %s $N%c [%d]" , attack, vp, punct, dam);
			Format::sprintf(buf3, "$n's %s %s you%c [%d]", attack, vp, punct, dam);
		}
	}

	if (ch == victim) {
		act(buf1, ch, nullptr, nullptr, TO_ROOM);
		act(buf2, ch, nullptr, nullptr, TO_CHAR);
	}
	else {
		act(buf1, ch, nullptr, victim, TO_NOTVICT);
		set_color(ch, GREEN, NOBOLD);
		act(buf2, ch, nullptr, victim, TO_CHAR);
		set_color(ch, WHITE, NOBOLD);
		set_color(victim, YELLOW, BOLD);
		act(buf3, ch, nullptr, victim, TO_VICT);
		set_color(victim, WHITE, NOBOLD);
	}
} /* end dam_message */

void do_berserk(Character *ch, String argument)
{
	int chance, hp_percent;

	if ((chance = get_skill_level(ch, skill::type::berserk)) == 0) {
		stc("You turn red in the face, but nothing happens.\n", ch);
		return;
	}

	if (affect::exists_on_char(ch, affect::type::berserk) || affect::exists_on_char(ch, affect::type::berserk) || affect::exists_on_char(ch, affect::type::frenzy)) {
		stc("You get a little madder.\n", ch);
		return;
	}

	if (affect::exists_on_char(ch, affect::type::calm)) {
		stc("You're feeling to mellow to berserk.\n", ch);
		return;
	}

	if (!deduct_stamina(ch, skill::type::berserk))
		return;

	/* modifiers */

	/* fighting */
	if (ch->fighting)
		chance += 10;

	/* damage -- below 50% of hp helps, above hurts */
	hp_percent = 100 * ch->hit / GET_MAX_HIT(ch);
	chance += 25 - hp_percent / 2;

	if (number_percent() < chance) {
		WAIT_STATE(ch, PULSE_VIOLENCE);
		/* heal a little damage */
		ch->hit += ch->level * 2;
		ch->hit = UMIN(ch->hit, GET_MAX_HIT(ch));
		stc("Your pulse races as you are consumed by rage!\n", ch);
		act("$n gets a wild look in $s eyes.", ch, nullptr, nullptr, TO_ROOM);
		check_improve(ch, skill::type::berserk, TRUE, 2);

		affect::add_type_to_char(ch,
			affect::type::berserk,
			ch->level,
			number_fuzzy(ch->level / 8),
			get_evolution(ch, skill::type::berserk),
			FALSE
		);
	}
	else {
		WAIT_STATE(ch, 3 * PULSE_VIOLENCE);
		stc("Your pulse speeds up, but nothing happens.\n", ch);
		check_improve(ch, skill::type::berserk, FALSE, 2);
	}
} /* end do_berserk */

void do_bash(Character *ch, String argument)
{
	Character *victim;
	int chance;
	int kdtime;
	int evolution_level;

	if (get_skill_level(ch, skill::type::bash) == 0) {
		stc("Bashing? What's that?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		victim = ch->fighting;

		if (victim == nullptr) {
			stc("But you aren't fighting anyone!\n", ch);
			return;
		}
	}
	else if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (get_position(ch) < POS_FIGHTING) {
		act("To bash from your current position would be a neat trick!", ch, nullptr, nullptr, TO_CHAR);
		return;
	}

	if (get_position(victim) < POS_FIGHTING) {
		act("You'll have to let $M get back up first.", ch, nullptr, victim, TO_CHAR);
		return;
	}

	if (victim == ch) {
		stc("You try to bash your brains out, but fail.\n", ch);
		return;
	}

	if (is_safe(ch, victim, TRUE)) {
		act("It is not a safe target!", ch, victim, nullptr, TO_CHAR);
		return;
	}

	if (IS_NPC(victim)
	    && victim->fighting != nullptr
	    && !is_same_group(ch, victim->fighting)) {
		stc("Kill stealing is not permitted.\n", ch);
		return;
	}

	if (affect::exists_on_char(ch, affect::type::charm_person) && ch->master == victim) {
		act("But $N is your friend!", ch, nullptr, victim, TO_CHAR);
		return;
	}

	if (!deduct_stamina(ch, skill::type::bash))
		return;

	/* see how much the bash skill was evolved -- Outsider */
	evolution_level = get_evolution(ch, skill::type::bash);
	/* modifiers */
	// new bash mods - Montrey (2014)
	// split now into two checks.  we first check for evasion (dodge/blur), and then
	// for chance of being knocked down
	/* now the attack */
	check_killer(ch, victim);

	if (ch->fighting == nullptr)
		set_fighting(ch, victim);

	/*check for successful hit*/
	if (check_dodge(ch, victim, skill::type::bash, 0)
	    || check_blur(ch, victim, skill::type::bash, 0)) {
		/* We missed. However, if bash is evolved, we keep our footing. -- Outsider */
		if (evolution_level < 2) { /* fail */
			act("You fall flat on your face!", ch, nullptr, victim, TO_CHAR);
			act("$n falls flat on $s face.", ch, nullptr, victim, TO_NOTVICT);
			act("You evade $n's bash, causing $m to fall flat on $s face.", ch, nullptr, victim, TO_VICT);
			ch->position = POS_RESTING;
		}
		else {  /* keep footing */
			act("You miss, but manage to keep your footing.", ch, nullptr, victim, TO_CHAR);
			act("$n misses, but keeps $s footing.", ch, nullptr, victim, TO_NOTVICT);
			act("You evade $n's bash, but $m keeps $s feet.", ch, nullptr, victim, TO_VICT);
		}

		WAIT_STATE(ch, skill::lookup(skill::type::bash).beats * 3 / 2);
		check_improve(ch, skill::type::bash, FALSE, 1);
		return;
	}

	// connect, check to see if knocked down
	chance = get_skill_level(ch, skill::type::bash) / 2;
	// size is a factor twice - here for knockdown, and in dodge/blur for evasion
//	if (ch->size > victim->size)
	chance += (ch->size - victim->size) * 10;
	/* this is intentional!  AC_BASH is armor class vs blunt weapons, gained through
	   thick armors and stuff.  the penalty for it is not a typo, it is supposed to
	   count against you -- Montrey */
	chance -= get_unspelled_ac(victim, AC_BASH) / 50; // armor is negative for better armor
	/* Hitroll matters, maybe in the future */
	/*if (GET_ATTR_HITROLL(ch) <120)
	        chance += (GET_ATTR_HITROLL(ch) / 8);
	else
	{
	        chance += 15;
	        chance += ((GET_ATTR_HITROLL(ch) - 120) / 16);
	}*/
	/* less bashable if translucent -- Elrac */
//	if ( affect::exists_on_char(victim, affect::type::pass_door) )
//		chance -= chance / 3;
	/*Change in chance based on STR and score and stamina*/
	chance += 3 * (GET_ATTR_STR(ch) - GET_ATTR_STR(victim));
	// stamina mod, scale by their remaining stamina
	chance = chance * GET_MAX_STAM(victim) / UMAX(victim->stam, 1);
	/*Change in chance based on carried weight of both involved*/
	// hard to balance this for mobs -- Montrey (2014)
//	chance += (get_carry_weight(ch) - get_carry_weight(victim)) / 300;
	/*Level modifiers*/
	chance += (ch->level - victim->level);
	/*Possibly add a couple influential spells*/
	/* If bash was evolved, then our chances are better. -- Outsider */
	chance += (evolution_level - 1) * 10;

	/* a 100% standfast skill eliminates bashing */
	if (CAN_USE_RSKILL(victim, skill::type::standfast)) {
		chance = chance * (100 - get_skill_level(victim, skill::type::standfast));
		chance /= 100;
	}

#ifdef DEBUG_CHANCE
	ptc(ch, "(bash %d%%)", chance);
	ptc(victim, "(bash %d%%)", chance);
#endif
	/*Moderate the result*/
	chance = URANGE(5, chance, 95);

	/* Check for knockdown*/
	if (number_percent() < chance) {
		kdtime = number_range((PULSE_VIOLENCE + 2), 3 * PULSE_VIOLENCE) + (get_carry_weight(victim) / 400);
		act("$n sends you sprawling with a powerful bash!", ch, nullptr, victim, TO_VICT);
		act("You slam into $N, and send $M flying!", ch, nullptr, victim, TO_CHAR);
		act("$n sends $N sprawling with a powerful bash.", ch, nullptr, victim, TO_NOTVICT);
		check_improve(ch, skill::type::bash, TRUE, 1);
		DAZE_STATE(victim, kdtime);
		WAIT_STATE(victim, kdtime);
		WAIT_STATE(ch, skill::lookup(skill::type::bash).beats);
		damage(ch, victim, number_range(ch->level * 2 / 3, (ch->level * 3 / 2) + chance / 10), skill::type::bash, -1, DAM_BASH, TRUE, FALSE);
		victim->position = POS_RESTING;

		if (CAN_USE_RSKILL(victim, skill::type::standfast))
			check_improve(victim, skill::type::standfast, FALSE, 1);
	}
	else {
		if (ch->fighting == nullptr) set_fighting(ch, victim);

		act("$n crashes into you, but fails to accomplish anything!", ch, nullptr, victim, TO_VICT);
		act("You throw yourself into $N, but fail to knock them down!", ch, nullptr, victim, TO_CHAR);
		act("$n slams into $N, but neither of them loses their footing.", ch, nullptr, victim, TO_NOTVICT);
		check_improve(ch, skill::type::bash, TRUE, 1);
		DAZE_STATE(victim, PULSE_VIOLENCE);
		WAIT_STATE(ch, skill::lookup(skill::type::bash).beats);

		if (CAN_USE_RSKILL(victim, skill::type::standfast))
			check_improve(victim, skill::type::standfast, TRUE, 1);
	}
} /* end do_bash */

void do_dirt(Character *ch, String argument)
{
	Character *victim;
	int chance;

	String arg;
	one_argument(argument, arg);

	if ((chance = get_skill_level(ch, skill::type::dirt_kicking)) == 0) {
		stc("You get your feet dirty.\n", ch);
		return;
	}

	if (arg.empty()) {
		victim = ch->fighting;

		if (victim == nullptr) {
			stc("But you aren't in combat!\n", ch);
			return;
		}
	}
	else if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (is_blinded(victim)) {
		act("$E has already been blinded.", ch, nullptr, victim, TO_CHAR);
		return;
	}

	if (IS_FLYING(ch))
	{
		stc("How do you expect to kick dirt while flying?\n", ch);
		return;
	}

	if (victim == ch) {
		stc("Very funny.\n", ch);
		return;
	}

	if (is_safe(ch, victim, TRUE)) {
		stc("That doesn't seem to work on this opponent.\n", ch);
		return;
	}

	if (IS_NPC(victim)
	    && victim->fighting != nullptr
	    && !is_same_group(ch, victim->fighting)) {
		stc("Kill stealing is not permitted.\n", ch);
		return;
	}

	if (affect::exists_on_char(ch, affect::type::charm_person) && ch->master == victim) {
		act("But $N is such a good friend!", ch, nullptr, victim, TO_CHAR);
		return;
	}

	if (!deduct_stamina(ch, skill::type::dirt_kicking))
		return;

	/* modifiers */
	/* dexterity */
	chance += GET_ATTR_DEX(ch);
	chance -= 2 * GET_ATTR_DEX(victim);

	/* speed  */
	if (ch->off_flags.has(OFF_FAST) || affect::exists_on_char(ch, affect::type::haste))
		chance += 10;

	if (victim->off_flags.has(OFF_FAST) || affect::exists_on_char(victim, affect::type::haste))
		chance -= 25;

	/* level */
	chance += (ch->level - victim->level) * 2;

	/* sloppy hack to prevent false zeroes */
	if (chance % 5 == 0)
		chance += 1;

	/* terrain */
	switch (ch->in_room->sector_type) {
	case (SECT_INSIDE):              chance -= 20;   break;

	case (SECT_CITY):                chance -= 10;   break;

	case (SECT_FIELD):               chance +=  5;   break;

	case (SECT_FOREST):                              break;

	case (SECT_HILLS):                               break;

	case (SECT_MOUNTAIN):            chance -= 10;   break;

	case (SECT_WATER_SWIM):          chance  =  0;   break;

	case (SECT_WATER_NOSWIM):        chance  =  0;   break;

	case (SECT_AIR):                 chance  =  0;   break;

	case (SECT_DESERT):              chance += 10;   break;
	}

	if (chance == 0) {
		stc("There isn't any dirt to kick.\n", ch);
		return;
	}

	/* now the attack */
	check_killer(ch, victim);

	if (number_percent() < chance) {
		if (affect::exists_on_char(victim, affect::type::rayban)) {
			act("You kick dirt in $n's eyes, but it doesn't affect $m!", ch, nullptr, victim, TO_CHAR);
			act("$n kicks dirt in your eyes, but your eye protection saves your vision!", ch, nullptr, victim, TO_VICT);
		}
		else {
			act("$n is blinded by the dirt in $s eyes!", victim, nullptr, nullptr, TO_ROOM);
			act("$n kicks dirt in your eyes!", ch, nullptr, victim, TO_VICT);
			damage(ch, victim, number_range(2, 5), skill::type::dirt_kicking, -1, DAM_NONE, FALSE, FALSE);
			stc("You can't see a thing!\n", victim);

			affect::add_type_to_char(victim,
				affect::type::dirt_kicking,
				ch->level,
				0,
				get_evolution(ch, skill::type::dirt_kicking),
				FALSE
			);
		}

		check_improve(ch, skill::type::dirt_kicking, TRUE, 2);
		WAIT_STATE(ch, skill::lookup(skill::type::dirt_kicking).beats);
	}
	else {
		act("Your kicked dirt MISSES $N!", ch, nullptr, victim, TO_CHAR);
		check_improve(ch, skill::type::dirt_kicking, FALSE, 2);
		WAIT_STATE(ch, skill::lookup(skill::type::dirt_kicking).beats);
	}
} /* end do_dirt */

// pulled out to use for evolved kick as well as do_trip
bool trip(Character *ch, Character *victim, int chance, skill::type attack_skill)
{
	/* modifiers */
	/* size */
	if (ch->size < victim->size)
		chance += (ch->size - victim->size) * 10;  /* bigger = harder to trip */

	/* dex */
	chance += GET_ATTR_DEX(ch);
	chance -= GET_ATTR_DEX(victim) * 3 / 2;

	/* speed */
	if (ch->off_flags.has(OFF_FAST) || affect::exists_on_char(ch, affect::type::haste))
		chance += 10;

	if (victim->off_flags.has(OFF_FAST) || affect::exists_on_char(victim, affect::type::haste))
		chance -= 20;

	/* level */
	chance += (ch->level - victim->level) * 2;
	/* now the attack */
	check_killer(ch, victim);

	/* a 100% standfast skill eliminates tripping */
	if (CAN_USE_RSKILL(victim, skill::type::standfast)) {
		chance = chance * (100 - get_skill_level(victim, skill::type::standfast));
		chance /= 100;
	}

	if (number_percent() < chance) {
		DAZE_STATE(victim, 2 * PULSE_VIOLENCE);
		WAIT_STATE(victim, 2 * PULSE_VIOLENCE);
		victim->position = POS_RESTING;
		damage(ch, victim, number_range(2, 2 +  2 * victim->size), attack_skill, -1, DAM_BASH, TRUE, FALSE);

		if (CAN_USE_RSKILL(victim, skill::type::standfast))
			check_improve(victim, skill::type::standfast, FALSE, 1);

		return TRUE;
	}
	else {
		damage(ch, victim, 0, attack_skill, -1, DAM_BASH, TRUE, FALSE);

		if (CAN_USE_RSKILL(victim, skill::type::standfast))
			check_improve(victim, skill::type::standfast, TRUE, 1);

		return FALSE;
	}
}

void do_trip(Character *ch, String argument)
{
	Character *victim;
	int chance;

	String arg;
	one_argument(argument, arg);

	if ((chance = get_skill_level(ch, skill::type::trip)) == 0) {
		stc("Tripping?  What's that?\n", ch);
		return;
	}

	if (arg.empty()) {
		victim = ch->fighting;

		if (victim == nullptr) {
			stc("But you aren't fighting anyone!\n", ch);
			return;
		}
	}
	else {
		victim = get_char_here(ch, arg, VIS_CHAR);

		if (victim == nullptr) {
			stc("They aren't here.\n", ch);
			return;
		}
	}

	if (is_safe(ch, victim, TRUE))
		return;

	if (IS_NPC(victim)
	    && victim->fighting != nullptr
	    && !is_same_group(ch, victim->fighting)) {
		stc("Kill stealing is not permitted.\n", ch);
		return;
	}

	if (IS_FLYING(victim)) {
		act("$S feet aren't on the ground.", ch, nullptr, victim, TO_CHAR);
		return;
	}

	if (get_position(victim) < POS_FIGHTING) {
		act("$N is already down.", ch, nullptr, victim, TO_CHAR);
		return;
	}

	if (affect::exists_on_char(ch, affect::type::charm_person) && ch->master == victim) {
		act("$N is your beloved master.", ch, nullptr, victim, TO_CHAR);
		return;
	}

	if (victim == ch) {
		stc("You fall flat on your face!\n", ch);
		WAIT_STATE(ch, 2 * skill::lookup(skill::type::trip).beats);
		act("$n trips over $s own feet!", ch, nullptr, nullptr, TO_ROOM);
		return;
	}

	if (!deduct_stamina(ch, skill::type::trip))
		return;

	if (trip(ch, victim, chance, skill::type::trip)) {
		act("$n trips you and you go down!", ch, nullptr, victim, TO_VICT);
		act("You trip $N and $E goes down!", ch, nullptr, victim, TO_CHAR);
		act("$n trips $N, sending $M to the ground.", ch, nullptr, victim, TO_NOTVICT);
		check_improve(ch, skill::type::trip, TRUE, 1);
		WAIT_STATE(ch, skill::lookup(skill::type::trip).beats);
	}
	else {
		check_improve(ch, skill::type::trip, FALSE, 1);
		WAIT_STATE(ch, skill::lookup(skill::type::trip).beats * 2 / 3);
	}
} /* end do_trip */

bool check_attack_ok(Character *ch, Character *victim) {
	if ((ch->in_room->sector_type == SECT_ARENA) && (!battle.start)) {
		stc("Hold your horses, the battle hasn't begun yet!\n", ch);
		return FALSE;
	}

	if (IS_NPC(ch) && ch->act_flags.has(ACT_MORPH) && !IS_NPC(victim)) {
		stc("Morphed players cannot attack PC's.\n", ch);
		wiznet("$N is attempting PK while morphed.", ch, nullptr, WIZ_CHEAT, 0, GET_RANK(ch));
		return FALSE;
	}

	if (IS_NPC(ch) && ch->act_flags.has(ACT_MORPH) && victim->act_flags.has(ACT_PET)) {
		stc("Morphed players cannot attack pets.\n", ch);
		wiznet("$N is attempting to kill a pet while morphed.", ch, nullptr, WIZ_CHEAT, 0, GET_RANK(ch));
		return FALSE;
	}

	if (affect::exists_on_char(ch, affect::type::fear)) {
		stc("But they would beat the stuffing out of you!!\n", ch);
		return FALSE;
	}

	if (victim == ch) {
		stc("I believe you are suffering from a mild case of schizophrenia.\n", ch);
		return FALSE;
	}

	if (is_safe(ch, victim, TRUE))
		return FALSE;

	if (victim->fighting != nullptr
	    && !is_same_group(ch, victim->fighting)) {
		stc("Kill stealing is not permitted.\n", ch);
		return FALSE;
	}

	if (affect::exists_on_char(ch, affect::type::charm_person) && ch->master == victim) {
		act("$N is your beloved master.", ch, nullptr, victim, TO_CHAR);
		return FALSE;
	}

	return TRUE;
}

void do_kill(Character *ch, String argument)
{
	Character *victim;

	if (argument.empty()) {
		stc("Kill whom?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	/* Check to see if we have blind fight -- Outsider */
	if (CAN_USE_RSKILL(ch, skill::type::blind_fight)) {
		victim = get_char_here(ch, arg, 0);

		if (!victim) {
			stc("They aren't here.\n", ch);
			return;
		}
	}
	else {  /* cannot blind fight */
		if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr) {
			stc("They aren't here.\n", ch);
			return;
		}
	}

	if (!check_attack_ok(ch, victim))
		return;

	if (ch->fighting) {
		stc("You do the best you can!\n", ch);
		return;
	}

	WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
	check_killer(ch, victim);
	multi_hit(ch, victim, skill::type::unknown);
} /* end do_kill */

/* Battle/Arena by Lotus */
void do_battle(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	int low, high, fee;
	RoomPrototype *location;
	Descriptor *d;
	Character *ach;

	if (IS_NPC(ch))
		return;

	if (IS_IMMORTAL(ch)) {
		String arg1, arg2, arg3;
		argument = one_argument(argument, arg1);
		argument = one_argument(argument, arg2);
		argument = one_argument(argument, arg3);

		if (!strcmp(arg1, "start")) {
			if (battle.issued) {
				stc("The battle begins!\n", ch);
				battle.start = TRUE;
				Format::sprintf(buf, "[FYI] $n has closed the arena and battle has begun.\n");
				global_act(ch, buf, TRUE, RED, COMM_QUIET | COMM_NOANNOUNCE);
				return;
			}
			else {
				stc("No battle has been issued yet.\n", ch);
				return;
			}
		}

		if (!strcmp(arg1, "stop")) {
			if (battle.issued) {
				stc("The battle ends.  Be sure to teleport the victor out of the arena.\n", ch);
				battle.start  = FALSE;
				battle.issued = FALSE;
				battle.fee    = 0;
				battle.low    = 0;
				battle.high   = 0;
				return;
			}
			else {
				stc("There is no battle to stop yet.\n", ch);
				return;
			}
		}

		if (!strcmp(arg1, "status")) {
			stc("Battle status is as follows:\n", ch);

			if (!battle.issued) {
				stc("No battle in progress.\n", ch);
				return;
			}
			else {
				ptc(ch, "Range: %d - %d\n", battle.low, battle.high);
				ptc(ch, "Fee: %d silver\n", battle.fee);
				stc("A challenge has been issued.\n", ch);

				if (battle.start)
					stc("A battle is in progress.\n", ch);
			}

			stc("Players currently in the Arena:\n", ch);

			for (d = descriptor_list; d; d = d->next) {
				ach = d->character;

				if (IS_PLAYING(d)
				    && !IS_NPC(ach)
				    && ach->in_room != nullptr
				    && ach->in_room->sector_type == SECT_ARENA
				    && can_see_char(ch, ach))
					ptc(ch, "{G[%3d] {P%s{x\n", ach->level, ach->name);
			}

			return;
		}

		if (battle.issued) {
			stc("A battle is already in progress.\n", ch);
			return;
		}

		if (arg1.empty() || arg2.empty() || arg3.empty()) {
			stc("Syntax: <low> <high> <fee>\n", ch);
			return;
		}

		if (!arg1.is_number() || !arg2.is_number() || !arg3.is_number()) {
			stc("Values must be numeric.\n", ch);
			return;
		}

		if ((location = get_room_index(ROOM_VNUM_ARENACENTER)) == nullptr) {
			stc("The arena is missing.\n", ch);
			return;
		}

		if (location->area->nplayer != 0) {
			ptc(ch, "You need to clear %d character from the arena first.\n", location->area->nplayer);
			return;
		}

		low  = atoi(arg1);
		high = atoi(arg2);
		fee  = atoi(arg3);

		if ((low < 0 || low > 100) || (high < 0 || high > 100)) {
			stc("Level ranges are 0 to 100.\n", ch);
			return;
		}

		if (fee < 0 || fee > 100000) {
			stc("Fee ranges is 0 to 10000.\n", ch);
			return;
		}

		battle.low    = low;
		battle.high   = high;
		battle.fee    = fee;
		battle.issued = TRUE;
		ptc(ch, "You open the arena to levels %d to %d, fee %d silver\n",
		    battle.low, battle.high, battle.fee);
		Format::sprintf(buf, "[FYI] $n has opened the arena to levels %d - %d for a %d silver fee.\n",
		        battle.low, battle.high, battle.fee);
		global_act(ch, buf, TRUE, RED, COMM_QUIET | COMM_NOANNOUNCE);
	}
	else {
		if (char_in_duel(ch)) {
			stc("Deal with this battle first.\n", ch);
			return;
		}

		if (ch->in_room->sector_type == SECT_ARENA) {
			stc("You are already within the walls of the arena.\n", ch);
			return;
		}

		if (!battle.issued) {
			stc("No challenge has been issued by an immortal.\n", ch);
			return;
		}

		if (battle.start) {
			stc("The battle has already begun, try again later.\n", ch);
			return;
		}

		if ((ch->level < battle.low) || (ch->level > battle.high)) {
			stc("You are not within the level range for battle.\n", ch);
			return;
		}

		if ((ch->silver + 100 * ch->gold) < battle.fee) {
			stc("You cannot afford to engage in battle.\n", ch);
			return;
		}

		if ((location = get_room_index(ROOM_VNUM_ARENACENTER)) == nullptr) {
			stc("The arena is missing.\n", ch);
			return;
		}

		if (ch->in_room == nullptr)
			return;

		if (ch->fighting != nullptr) {
			stc("But you are already in combat!\n", ch);
			return;
		}

		act("$n cackles as $e is teleported to the arena.", ch, nullptr, nullptr, TO_ROOM);
		char_from_room(ch);
		char_to_room(ch, location);
		deduct_cost(ch, battle.fee);
		act("$n enters the arena and prepares to draw blood.", ch, nullptr, nullptr, TO_ROOM);
		stc("You have been teleported to the arena.\n", ch);
		Format::sprintf(buf, "%s has joined the battle in the arena.\n", ch->name);
		do_send_announce(ch, buf);
		do_look(ch, "auto");
	}
} /* end battle */

/* Singing Skill by Lotus */
void do_sing(Character *ch, String argument)
{
	Character *victim;
	int singchance;

	if (argument.empty()) {
		stc("You sing a simple melody for all to enjoy.\n", ch);
		return;
	}

	if ((victim = get_char_here(ch, argument, VIS_CHAR)) == nullptr) {
		stc("No one by that name is here to sing to.\n", ch);
		return;
	}

	if (is_safe(ch, victim, TRUE))
		return;

	if (GET_ROOM_FLAGS(victim->in_room).has(ROOM_LAW)) {
		stc("The mayor does not approve of your playing style.\n", ch);
		return;
	}

	if (victim == ch) {
		stc("You are an excellant musician, Bravo!!\n", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		stc("They ignore your singing.\n", ch);
		return;
	}

	stc("You sing a beautiful melody.\n", ch);

	if (!get_skill_level(ch, skill::type::sing))
		return;

	if (!deduct_stamina(ch, skill::type::sing))
		return;

	WAIT_STATE(ch, skill::lookup(skill::type::sing).beats);

	if (affect::exists_on_char(victim, affect::type::charm_person)
	    || affect::exists_on_char(ch, affect::type::charm_person))
		return;

	singchance = get_skill_level(ch, skill::type::sing) / 2;

	/* Level modifiers */
	if (ch->level < victim->level)
		singchance -= (victim->level - ch->level) * 4;
	else
		singchance += (ch->level - victim->level) * 2;

	/*bonus for remorts*/
	singchance += ch->pcdata->remort_count / 2;
	/*Apply stat bonuses*/
	singchance += (GET_ATTR_CHR(ch));
	singchance -= (GET_ATTR_INT(victim) + GET_ATTR_WIS(victim)) / 2;

	if (!IS_NPC(ch) && ch->cls == 6)      /* bards */
		singchance += singchance / 3;

	singchance -= singchance * GET_DEFENSE_MOD(victim, DAM_CHARM) / 100;

	/*Moderate the final chance*/
	singchance = URANGE(0, singchance, (101 - (victim->level / 2)));

	/*Final calculation*/
	if (!chance(singchance)) {
		/*Chance for something bad in the future, based on victim level, ch's CHR*/
		check_improve(ch, skill::type::sing, FALSE, 8);
		return;
	}

	check_improve(ch, skill::type::sing, TRUE, 8);

	/*End calculation, sing is successful, now check final legality and apply*/
	if (victim->master)
		stop_follower(victim);

	add_follower(victim, ch);
	victim->leader = ch;

	affect::add_type_to_char(victim,
		affect::type::charm_person,
		ch->level,
		number_fuzzy(ch->level/4),
		get_evolution(ch, skill::type::charm_person),
		FALSE
	);

	act("Isn't $n's music beautiful?", ch, nullptr, victim, TO_VICT);

	if (ch != victim)
		act("$N really loves your music.", ch, nullptr, victim, TO_CHAR);

	return;
} /* end do_sing */

void do_backstab(Character *ch, String argument)
{
	Character *victim;
	Object *obj;
	int evo = get_evolution(ch, skill::type::backstab);

	if (!get_skill_level(ch, skill::type::backstab)) {
		do_huh(ch);
		return;
	}

	if (argument.empty()) {
		stc("Backstab whom?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (ch->fighting != nullptr) {
		stc("You're facing the wrong end.\n", ch);
		return;
	}
	else if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (victim == ch) {
		stc("How can you sneak up on yourself?\n", ch);
		return;
	}

	if (is_safe(ch, victim, TRUE))
		return;

	if (IS_NPC(victim)
	    && victim->fighting != nullptr
	    && !is_same_group(ch, victim->fighting)) {
		stc("Kill stealing is not permitted.\n", ch);
		return;
	}

	if ((obj = get_eq_char(ch, WEAR_WIELD)) == nullptr) {
		stc("You need to wield a weapon to backstab.\n", ch);
		return;
	}

	if (victim->hit < GET_MAX_HIT(victim)) {
		act("$N is hurt and suspicious ... you can't sneak up.", ch, nullptr, victim, TO_CHAR);
		return;
	}

	if (IS_NPC(ch) && ch->act_flags.has(ACT_MORPH) && !IS_NPC(victim)) {
		stc("Morphed players cannot backstab PC's.\n", ch);
		wiznet("$N is attempting PK while morphed.", ch, nullptr, WIZ_CHEAT, 0, GET_RANK(ch));
		return;
	}

	if (IS_NPC(ch) && ch->act_flags.has(ACT_MORPH) && victim->act_flags.has(ACT_PET)) {
		stc("Morphed players cannot backstab pets.\n", ch);
		wiznet("$N is attempting to kill a pet while morphed.", ch, nullptr, WIZ_CHEAT, 0, GET_RANK(ch));
		return;
	}

	if (!deduct_stamina(ch, skill::type::backstab))
		return;

	if (IS_NPC(ch)) {
		mob_hit(ch, victim, skill::type::backstab);
		return;
	}

	check_killer(ch, victim);
	WAIT_STATE(ch, skill::lookup(skill::type::backstab).beats);

	if (number_percent() < get_skill_level(ch, skill::type::backstab)
	    || (get_skill_level(ch, skill::type::backstab) >= 2 && !IS_AWAKE(victim))) {
		check_improve(ch, skill::type::backstab, TRUE, 4);
		multi_hit(ch, victim, skill::type::backstab);
		if (evo >=3 && chance(50)) { /*vegita - 15% chance to put paralyze on target at evo 3*/
			//multi_hit(ch, victim, skill::type::backstab);
			stc("{YYour skillful blow strikes a nerve on your opponent!{x\n", ch);
			int level = (ch->level);
			affect::add_type_to_char(victim,
				affect::type::paralyze,
				level,
				2,
				1,
				FALSE
			);
		}
	}
	else {
		check_improve(ch, skill::type::backstab, FALSE, 4);
		damage(ch, victim, 0, skill::type::backstab, -1, DAM_NONE, TRUE, FALSE);
	}
} /* end do_backstab */

/* Shadow Form for remorts - Lotus */
void do_shadow(Character *ch, String argument)
{
	Character *victim;
	Object *obj;

	if (!CAN_USE_RSKILL(ch, skill::type::shadow_form)) {
		stc("Huh?\n", ch);
		return;
	}

	if (ch->fighting == nullptr) {
		stc("You must be fighting in order to enter shadow form.\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg.empty())
		victim = ch->fighting;
	else {
		if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr) {
			stc("They aren't here.\n", ch);
			return;
		}

		if (victim->fighting == nullptr || !is_same_group(ch, victim->fighting)) {
			stc("You are not fighting that opponent.\n", ch);
			return;
		}
	}

	if (affect::exists_on_char(victim, affect::type::shadow_form)) {
		act("$N has seen shadow form before and could easily avoid the attack.", ch, nullptr, victim, TO_CHAR);
		return;
	}

	if ((obj = get_eq_char(ch, WEAR_WIELD)) == nullptr) {
		stc("You need to wield a weapon for shadow form to do any harm!\n", ch);
		return;
	}

	check_killer(ch, victim);

	if (!deduct_stamina(ch, skill::type::shadow_form))
		return;

	WAIT_STATE(ch, skill::lookup(skill::type::shadow_form).beats);

	if (number_percent() < get_skill_level(ch, skill::type::shadow_form)) {
		check_improve(ch, skill::type::shadow_form, TRUE, 1);
		multi_hit(ch, victim, skill::type::shadow_form);
	}
	else {
		stc("You enter shadow form, but your target avoids your strike.\n", ch);
		check_improve(ch, skill::type::shadow_form, FALSE, 1);
		damage(ch, victim, 0, skill::type::shadow_form, -1, DAM_NONE, TRUE, FALSE);
	}

	affect::add_type_to_char(victim,
		affect::type::shadow_form,
		ch->level,
		-1,
		get_evolution(ch, skill::type::shadow_form),
		FALSE
	);
} /* end do_shadow */

void do_circle(Character *ch, String argument)
{
	Character *victim;
	Object *obj;

	if (!get_skill_level(ch, skill::type::circle)) {
		stc("You twirl around is a circle! wheeee!!!!\n", ch);
		return;
	}

	if (ch->fighting == nullptr) {
		stc("You must be fighting in order to circle.\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg.empty())
		victim = ch->fighting;
	else {
		if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr) {
			stc("They aren't here.\n", ch);
			return;
		}

		if (victim->fighting == nullptr || !is_same_group(ch, victim->fighting)) {
			stc("You are not fighting that opponent.\n", ch);
			return;
		}
	}

	if ((obj = get_eq_char(ch, WEAR_WIELD)) == nullptr) {
		stc("You need to wield a weapon to circle.\n", ch);
		return;
	}

	check_killer(ch, victim);

	if (!deduct_stamina(ch, skill::type::circle))
		return;

	WAIT_STATE(ch, skill::lookup(skill::type::circle).beats);

	if (number_percent() < get_skill_level(ch, skill::type::circle)) {
		check_improve(ch, skill::type::circle, TRUE, 1);
		multi_hit(ch, victim, skill::type::circle);
	}
	else {
		stc("You circle your opponent, but your hasty strike misses.\n", ch);
		check_improve(ch, skill::type::circle, FALSE, 1);
		damage(ch, victim, 0, skill::type::circle, -1, DAM_NONE, TRUE, FALSE);
	}
} /* end do_circle */

void do_flee(Character *ch, String argument)
{
	Exit *pexit;
	RoomPrototype *was_in;
	RoomPrototype *now_in;
	Character *victim, *vch;
	Character *hunted;
	int dex, topp = 0, chance, dir;
	/* some more vars to get the chance to flee */
	int weight, wis;

	if (ch->wait > 0)
		return;

	if ((victim = ch->fighting) == nullptr) {
		stc("You aren't fighting anyone.\n", ch);
		return;
	}

	if (get_position(ch) < POS_FIGHTING) {
		stc("You'll have to get up first!\n", ch);
		return;
	}

	/* figure out our chance to flee */
	/* added weight and wisdom -- Outsider */
	dex = GET_ATTR_DEX(ch);
	wis = GET_ATTR_WIS(ch);
	weight = get_carry_weight(ch);
	/* Get the weight factor and store it in weight again */
	weight = weight / 1000;
	/* Chance is decided by the char's dex, minus their weight.
	   That value is then averaged with the character's wisdom
	   -- Outsider
	*/
	chance = ((dex - 9) * 6) - weight;
	chance += (wis - 9) * 6;
	chance = chance / 2;

	for (vch = ch->in_room->people; vch != nullptr; vch = vch->next_in_room) {
		if (vch->fighting != ch)
			continue;

		/* average dex and wis to chance -- Outsider */
		chance += ((dex - GET_ATTR_DEX(vch)) * 5) +
		          ((wis - GET_ATTR_WIS(vch)) * 5) /
		          2;

		if (topp)
			chance -= 10; /* -10% per opponent after the first */

		topp++;
	}

	topp = 0;
	was_in = ch->in_room;

	/* count the possible exits */
	for (dir = 0; dir < 6; dir++) {
		if ((pexit = was_in->exit[dir]) == 0
		    || pexit->u1.to_room == nullptr
		    || !can_see_room(ch, pexit->u1.to_room)
		    || (pexit->exit_flags.has(EX_CLOSED)
		        && (!affect::exists_on_char(ch, affect::type::pass_door)
		            || pexit->exit_flags.has(EX_NOPASS)))
		    || (IS_NPC(ch) && GET_ROOM_FLAGS(pexit->u1.to_room).has(ROOM_NO_MOB)))
			continue;

		topp++;
	}

	/* divide chance by number of exits */
	if (topp)
		chance /= topp;
	else {  /* might as well eliminate the no exits possibility here */
		stc("There is nowhere to run!\n", ch);
		return;
	}

	chance = URANGE(10, chance, 90);

	for (dir = 0; dir < 6; dir++) {
		if ((pexit = was_in->exit[dir]) == 0
		    || pexit->u1.to_room == nullptr
		    || !can_see_room(ch, pexit->u1.to_room)
		    || (pexit->exit_flags.has(EX_CLOSED)
		        && (!affect::exists_on_char(ch, affect::type::pass_door)
		            || pexit->exit_flags.has(EX_NOPASS)))
		    || (IS_NPC(ch) && GET_ROOM_FLAGS(pexit->u1.to_room).has(ROOM_NO_MOB)))
			continue;

		if (!chance(chance))
			continue;

		move_char(ch, dir, FALSE);

		if ((now_in = ch->in_room) == was_in)
			continue;

		stop_fighting(ch, TRUE);
		ch->in_room = was_in;
		act("$n has fled!", ch, nullptr, nullptr, TO_ROOM);
		ch->in_room = now_in;

		if (!IS_NPC(ch)) {
			act("You flee $T from combat!", ch, nullptr, Exit::dir_name(dir), TO_CHAR);

			if (ch->cls == THIEF_CLASS)
				stc("You snuck away safely.\n", ch);
			else {
				if (ch->cls == PALADIN_CLASS) { /* Paladins */
					stc("You lose 50 exp.\n", ch);
					gain_exp(ch, -50);
				}
				else {
					stc("You lose 10 exp.\n", ch);
					gain_exp(ch, -10);
				}
			}
		}

		if (IS_NPC(victim) && (victim->act_flags.has(ACT_AGGRESSIVE))) {
			if ((hunted = get_char_area(victim, ch->name, VIS_CHAR)) != nullptr) {
				victim->hunting = hunted;
				WAIT_STATE(victim, 3 * PULSE_VIOLENCE);
			}
		}

		return;
	}

	WAIT_STATE(ch, PULSE_VIOLENCE * 2 / 3);
	stc("PANIC! You couldn't escape!\n", ch);
} /* end do_flee */

void do_rescue(Character *ch, String argument)
{
	Character *victim;
	Character *fch;

	if (argument.empty()) {
		stc("Rescue whom?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (victim == ch) {
		stc("What about fleeing instead?\n", ch);
		return;
	}

	if (ch->fighting == victim) {
		stc("You don't need to rescue them from yourself.\n", ch);
		return;
	}

	if (!is_same_group(ch, victim)) {
		stc("Kill stealing is not permitted.\n", ch);
		return;
	}

	if (victim->fighting == nullptr) {
		stc("That person is not fighting right now.\n", ch);
		return;
	}

	// old way was to get the char  that your 'victim' is currently attacking.
	// new way is to get the first opponent that is fighting the victim.
	for (fch = ch->in_room->people; fch != nullptr; fch = fch->next_in_room)
		if (fch->fighting == victim)
			break;

	if (fch == nullptr) {
		stc("No one is attacking that person right now.\n", ch);
		return;
	}

	if (!deduct_stamina(ch, skill::type::rescue))
		return;

	WAIT_STATE(ch, skill::lookup(skill::type::rescue).beats);

	if (number_percent() > get_skill_level(ch, skill::type::rescue)) {
		stc("You fail the rescue.\n", ch);
		check_improve(ch, skill::type::rescue, FALSE, 1);
		return;
	}

	act("You rescue $N!",  ch, nullptr, victim, TO_CHAR);
	act("$n rescues you!", ch, nullptr, victim, TO_VICT);
	act("$n rescues $N!",  ch, nullptr, victim, TO_NOTVICT);
	check_improve(ch, skill::type::rescue, TRUE, 1);
	stop_fighting(fch, FALSE);
	stop_fighting(victim, FALSE);
	stop_fighting(ch, FALSE);
	check_killer(ch, fch);
	set_fighting(ch, fch);
	set_fighting(fch, ch);
} /* end do_rescue */

void do_kick(Character *ch, String argument)
{
	Character *victim;
	int skill, amount;

	if ((skill = get_skill_level(ch, skill::type::kick)) == 0) {
		stc("You'd better leave the martial arts to fighters.\n", ch);
		return;
	}

	if ((victim = ch->fighting) == nullptr) {
		stc("You aren't fighting anyone.\n", ch);
		return;
	}

	if (!deduct_stamina(ch, skill::type::kick))
		return;

	WAIT_STATE(ch, skill::lookup(skill::type::kick).beats);
	check_killer(ch, victim);

	if (skill >= number_percent()) {
		amount = number_range((ch->level / 3), ch->level);
		amount += GET_ATTR_DAMROLL(ch);

		if (get_position(victim) < POS_FIGHTING)
			amount = amount * 5 / 4;

		damage(ch, victim, amount, skill::type::kick, -1, DAM_BASH, TRUE, FALSE);
		check_improve(ch, skill::type::kick, TRUE, 1);
		int evo = get_evolution(ch, skill::type::kick);

		if (evo >= 2) {
			if (get_position(victim) == POS_FIGHTING
			    && chance((evo - 1) * 20 + 10)) { // evo 2: 30, evo 3, 50
				stc("You bring your foot around for a second hit.\n", ch);
				damage(ch, victim, amount, skill::type::roundhouse, -1, DAM_BASH, TRUE, FALSE);
				check_improve(ch, skill::type::kick, TRUE, 1);
			}
		}

		if (evo >= 3) {
			if (get_position(victim) == POS_FIGHTING
			    && !IS_FLYING(victim)
			    && chance(30)) {
				if (trip(ch, victim, skill, skill::type::footsweep)) {
					act("$n sweeps your feet out from under you!", ch, nullptr, victim, TO_VICT);
					act("You sweep $N's feet and $E goes down!", ch, nullptr, victim, TO_CHAR);
					act("$n trips $N, sending $M to the ground.", ch, nullptr, victim, TO_NOTVICT);
					// for now, no extra wait state
				}
				else {
					// for now, no extra wait state
				}
			}
		}
	}
	else {
		// miss for 0
		damage(ch, victim, 0, skill::type::kick, -1, DAM_BASH, TRUE, FALSE);
		check_improve(ch, skill::type::kick, FALSE, 1);
	}
} /* end do_kick */

void do_crush(Character *ch, String argument)
{
	Character *victim;

	if (!get_skill_level(ch, skill::type::crush)) {
		stc("You are not skilled at grappling.\n", ch);
		return;
	}

	if ((victim = ch->fighting) == nullptr) {
		stc("You aren't fighting anyone.\n", ch);
		return;
	}

	WAIT_STATE(ch, skill::lookup(skill::type::crush).beats);

	if (get_skill_level(ch, skill::type::crush) > number_percent()) {
		damage(ch, victim, number_range(3, 3 * ch->level), skill::type::crush, -1, DAM_BASH, TRUE, FALSE);
		check_improve(ch, skill::type::crush, TRUE, 1);
	}
	else {
		damage(ch, victim, 0, skill::type::crush, -1, DAM_BASH, TRUE, FALSE);
		check_improve(ch, skill::type::crush, FALSE, 1);
	}

	check_killer(ch, victim);
} /* end do_crush */

void do_disarm(Character *ch, String argument)
{
	Character *victim;

	if (get_skill_level(ch, skill::type::disarm) <= 0) {
		stc("You don't know how to disarm opponents.\n", ch);
		return;
	}

	if (argument.empty()) {
		if ((victim = ch->fighting) == nullptr) {
			stc("You aren't fighting anyone.\n", ch);
			return;
		}
	}
	else {
		if ((victim = get_char_here(ch, argument, VIS_CHAR)) == nullptr) {
			stc("They aren't here.\n", ch);
			return;
		}

		if (victim == ch) {
			stc("And just how do you plan to do that?\n", ch);
			return;
		}
	}

	if (is_safe(ch, victim, TRUE))
		return;

	int evo = get_evolution(ch, skill::type::disarm);

	/* check to see if we can fight blind */
	int blind_fight_skill = CAN_USE_RSKILL(ch, skill::type::blind_fight) ? get_skill_level(ch, skill::type::blind_fight) : 0;
	int sight_modifier = 0;

	/* if they're not facing you, can't disarm, unless evo 3 or higher.  evo 4 has no penalty */
	if (victim->fighting && victim->fighting != ch) {
		if (!ch->fighting) {
			stc("They are already in combat.\n", ch);
			return;
		}

		if (ch->fighting != victim) {
			stc("Deal with your own fights!\n", ch);
			return;
		}

		if (evo < 3) {
			stc("They do not have a weapon drawn against you.\n", ch);
			return;
		}

		/* additional -20% if you're blind */
		if (!can_see_char(ch, victim))
			sight_modifier -= 20 * (100 - blind_fight_skill) / 100;
	}

	/* if you're blind, can't disarm, unless you're evo 2 or higher */
	if (!can_see_char(ch, victim)) {
		switch (evo) {
		case 1: sight_modifier -= 100 * (100 - blind_fight_skill) / 100; break;
		case 2: sight_modifier -= 60 * (100 - blind_fight_skill) / 100; break;
		case 3: sight_modifier -= 30 * (100 - blind_fight_skill) / 100; break;
		case 4:                 break;
		}
	}

	if (sight_modifier <= -100) {
		stc("You can't see their weapon well enough to disarm them.\n", ch);
		return;
	}

	Object *weapon = get_eq_char(victim, WEAR_WIELD);

	if (weapon == nullptr) {
		stc("Your opponent is not wielding a weapon.\n", ch);
		return;
	}

	/* need a weapon to disarm, unless you're npc or you have skill in hand to hand */
	if (get_eq_char(ch, WEAR_WIELD) == nullptr
	 && get_skill_level(ch, skill::type::hand_to_hand) == 0) { // note: not the same as get_weapon_learned
		stc("You must wield a weapon to disarm.\n", ch);
		return;
	}

	if (!deduct_stamina(ch, skill::type::disarm))
		return;

	damage(ch, victim, 0, skill::type::disarm, -1, DAM_BASH, FALSE, FALSE);
	WAIT_STATE(ch, skill::lookup(skill::type::disarm).beats);

	/* evo 1 talon give 60% save, 70% at 2, 80% at 3, 90% at 4 */
	if (affect::exists_on_char(victim, affect::type::talon)) {
		int talonchance = 75;

		switch (get_affect_evolution(victim, affect::type::talon)) {
		case 2: talonchance += 10;      break;

		case 3: talonchance += 25;      break;

		default:                        break;
		}

		if (chance(talonchance)) {
			act("$N's vice-like grip prevents you from disarming $M!", ch, nullptr, victim, TO_CHAR);
			act("$N's vice-like grip prevents $M from being disarmed!", ch, nullptr, victim, TO_NOTVICT);
			act("Your vice-like grip prevents $n from disarming you!", ch, nullptr, victim, TO_VICT);
			check_improve(ch, skill::type::disarm, FALSE, 1);
			return;
		}
	}

	/* noremove saves 100% at evo 1, 90% at 2, 80% at 3, 70% at 4 */
	if (IS_OBJ_STAT(weapon, ITEM_NOREMOVE)) {
		if (!chance(10 * (evo - 1))) {
			act("$S weapon won't budge!", ch, nullptr, victim, TO_CHAR);
			act("$n tries to disarm you, but your weapon won't budge!", ch, nullptr, victim, TO_VICT);
			act("$n tries to disarm $N, but fails.", ch, nullptr, victim, TO_NOTVICT);
			check_improve(ch, skill::type::disarm, FALSE, 1);
			return;
		}
	}

	// base disarm chance of 70% of disarm skill
	int chance = get_skill_level(ch, skill::type::disarm) * 7 / 10;

	/* find weapon skills */
	int ch_weapon_skill = get_weapon_learned(ch, get_weapon_skill(ch, FALSE)); // your skill with your weapon
	int vict_weapon_skill = get_weapon_learned(victim, get_weapon_skill(victim, FALSE)); // victim's skill
	int ch_vict_weapon_skill = get_weapon_learned(ch, get_weapon_skill(victim, FALSE)); // your skill with victim's weapon

	/* skill */
	if (get_eq_char(ch, WEAR_WIELD) == nullptr)
		chance = chance * ch_weapon_skill / 150; // harder to disarm without a weapon
	else
		chance = chance * ch_weapon_skill / 100;

	// up to 15% mod for higher weapon skill
	chance += (ch_weapon_skill - vict_weapon_skill) / 7;

	// up to 10% mod for being more skilled with their weapon than they are
	chance += (ch_vict_weapon_skill - vict_weapon_skill) / 10;

	/* dex + str vs. 2 x str */
	chance += GET_ATTR_DEX(ch);
	chance += GET_ATTR_STR(ch);
	chance -= 2 * GET_ATTR_STR(victim);

	/* level */
	chance += (ch->level - victim->level);
	chance += sight_modifier / 5; // already passed vis check for connecting, only up to 20% penalty here

	/* and now the attack */
	if (chance(chance)) {
		RoomPrototype *next_room = nullptr;
		Exit *pexit = nullptr;
		char buf[MAX_STRING_LENGTH];
		int door;

		switch (evo) {
		case 1:
			act("$n *** DISARMS *** you and sends your weapon flying!", ch, nullptr, victim, TO_VICT);
			act("You disarm $N!", ch, nullptr, victim, TO_CHAR);
			act("$n disarms $N!", ch, nullptr, victim, TO_NOTVICT);
			break;

		case 2:
			act("With a ringing blow, $n knocks your weapon away from you!", ch, nullptr, victim, TO_VICT);
			act("With a ringing blow, you knock $N's weapon away from $M!", ch, nullptr, victim, TO_CHAR);
			act("With a ringing blow, $n knocks $N's weapon away from $M!", ch, nullptr, victim, TO_NOTVICT);
			break;

		case 3:
			act("In a spectacular coupe, $n knocks your weapon away from you!", ch, nullptr, victim, TO_VICT);
			act("In a spectacular coupe, you knock $N's weapon away from $M!", ch, nullptr, victim, TO_CHAR);
			act("In a spectacular coupe, $n knocks $N's weapon away from $M!", ch, nullptr, victim, TO_NOTVICT);
			break;

		case 4:
			act("$n's devastating cutover rips your weapon from your grasp!", ch, nullptr, victim, TO_VICT);
			act("Your devastating cutover rips $N's weapon from $S grasp!", ch, nullptr, victim, TO_CHAR);
			act("$n's devastating cutover rips $N's weapon from $S grasp!", ch, nullptr, victim, TO_NOTVICT);
			break;
		}

		obj_from_char(weapon);

		if (IS_OBJ_STAT(weapon, ITEM_NODROP)
		    || IS_OBJ_STAT(weapon, ITEM_INVENTORY)
		    || victim->in_room->sector_type == SECT_ARENA
		    || victim->in_room->sector_type == SECT_CLANARENA)
			obj_to_char(weapon, victim);
		else {
			/* knock the weapon into the next room! */
			if ((evo == 2 && chance(5))
			    || (evo == 3 && chance(15))
			    || (evo == 4 && chance(25))) {
				door = number_range(0, 5);

				if ((pexit = victim->in_room->exit[door]) != 0
				    && (next_room = pexit->u1.to_room) != nullptr
				    && can_see_room(victim, next_room)) {
					if (pexit->exit_flags.has(EX_CLOSED)) {
						Format::sprintf(buf, "$p slams against the $d and clatters to the %s!",
						        victim->in_room->sector_type == SECT_INSIDE ? "floor" : "ground");
						act(buf, ch, weapon, pexit->keyword, TO_CHAR);
						act(buf, ch, weapon, pexit->keyword, TO_ROOM);
						obj_to_room(weapon, victim->in_room);
					}
					else {
						Format::sprintf(buf, "$p flies through the air and disappears %s%s!",
						        door < 4 ? "to the " : "", Exit::dir_name(door));
						act(buf, ch, weapon, nullptr, TO_CHAR);
						act(buf, ch, weapon, nullptr, TO_ROOM);

						if (next_room->people != nullptr) {
							Format::sprintf(buf, "$p flies in from %s%s and clatters to the %s!",
							        Exit::rev_dir(door) < 4 ? "the " : "", Exit::dir_name(door, true),
							        next_room->sector_type == SECT_INSIDE ? "floor" : "ground");
							act(buf, next_room->people, weapon, nullptr, TO_CHAR);
							act(buf, next_room->people, weapon, nullptr, TO_ROOM);
						}

						obj_to_room(weapon, next_room);
					}
				}
				else {
					if (door < 4)
						Format::sprintf(buf, "$p slams against the %s wall and clatters to the %s!",
						        Exit::dir_name(door),
						        victim->in_room->sector_type == SECT_INSIDE ? "floor" : "ground");
					else if (door < 5)
						Format::sprintf(buf, "$p clatters to the %s.",
						        victim->in_room->sector_type == SECT_INSIDE ? "floor" : "ground");
					else {
						if (victim->in_room->sector_type == SECT_INSIDE)
							Format::sprintf(buf, "$p flies up and strikes the ceiling, then clatters to the floor!");
						else
							Format::sprintf(buf, "$p flies into the air, and falls hard to the ground!");
					}

					act(buf, ch, weapon, nullptr, TO_CHAR);
					act(buf, ch, weapon, nullptr, TO_ROOM);
					obj_to_room(weapon, victim->in_room);
				}
			}
			else
				obj_to_room(weapon, victim->in_room);

			if (evo > 1)
				WAIT_STATE(victim, (12 + (evo * 6)));

			/* If they pick it up, might as well wear it again too. -- Outsider */
			if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim, weapon)) {
				get_obj(victim, weapon, nullptr);
				wear_obj(victim, weapon, FALSE);   /* added here...will remove in mob_update */
			}
		}

		check_improve(ch, skill::type::disarm, TRUE, 1);
	}
	else {
		WAIT_STATE(ch, skill::lookup(skill::type::disarm).beats);
		act("You fail to disarm $N.", ch, nullptr, victim, TO_CHAR);
		act("$n tries to disarm you, but fails.", ch, nullptr, victim, TO_VICT);
		act("$n tries to disarm $N, but fails.", ch, nullptr, victim, TO_NOTVICT);
		check_improve(ch, skill::type::disarm, FALSE, 1);
	}

	if (is_blinded(ch) && (blind_fight_skill > 0))
		check_improve(ch, skill::type::blind_fight, FALSE, 1);
}

void do_sla(Character *ch, String argument)
{
	stc("If you want to SLAY, spell it out.\n", ch);
} /* end do_sla */

void do_slay(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH]; /* For [FYI] */
	Character *victim;

	if (argument.empty()) {
		stc("Slay whom?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (ch == victim) {
		stc("Suicide is a mortal sin.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		act("Forgetting that $E is immortal, you foolishly attempt to slay $N.", ch, nullptr, victim, TO_CHAR);
		act("$n attempts to slay you in cold blood!", ch, nullptr, victim, TO_VICT);
		act("$n foolishly attempts to slay the immortal, $N.", ch, nullptr, victim, TO_NOTVICT);
		return;
	}

	act("You slay $M in cold blood!",  ch, nullptr, victim, TO_CHAR);
	act("$n slays you in cold blood!", ch, nullptr, victim, TO_VICT);
	act("$n slays $N in cold blood!",  ch, nullptr, victim, TO_NOTVICT);

	/* Add this so it will announce it - Lotus */
	if (!IS_NPC(victim)) {
		Format::sprintf(buf, "%s has been slain by %s.", victim->name, (IS_NPC(ch) ? ch->short_descr : ch->name));
		do_send_announce(victim, buf);
	}

	raw_kill(victim); // not kill_off, don't want to gain exp, update arena kills and such

} /* end do_slay */

void do_rotate(Character *ch, String argument)
{
	Character *victim;

	if (get_skill_level(ch, skill::type::rotate) == 0) {
		stc("Do what?", ch);
		return;
	}

	if (argument.empty()) {
		stc("Rotate your attack to whom?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (ch->fighting == nullptr) {
		stc("You're not in combat, just pick someone!\n", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (victim == ch) {
		stc("Suicide is not an option?\n", ch);
		return;
	}

	if (is_safe(ch, victim, TRUE))
		return;

	if (!deduct_stamina(ch, skill::type::rotate))
		return;

	check_killer(ch, victim);
	WAIT_STATE(ch, skill::lookup(skill::type::rotate).beats);

	if (number_percent() < get_skill_level(ch, skill::type::rotate)) {
		stc("You deftly shift the focus of your blows.\n", ch);
		check_improve(ch, skill::type::rotate, TRUE, 4);
		ch->fighting = victim;
	}
	else {
		check_improve(ch, skill::type::rotate, FALSE, 4);
		stc("You fail to switch dancing partners.\n", ch);
	}
} /* end do_rotate */

void do_hammerstrike(Character *ch, String argument)
{
	int chance;

	if (!CAN_USE_RSKILL(ch, skill::type::hammerstrike)) {
		stc("Huh?\n", ch);
		return;
	}

	chance = get_skill_level(ch, skill::type::hammerstrike);

	if (affect::exists_on_char(ch, affect::type::hammerstrike)) {
		stc("Are you insane?!?\n", ch);
		return;
	}

	if (number_percent() < chance) {
		WAIT_STATE(ch, PULSE_VIOLENCE);
		ch->stam -= ch->stam / 3;
		stc("The gods strike you with a lightning bolt of power!\n", ch);
		act("$n is lit on fire by a blue bolt of godly power.", ch, nullptr, nullptr, TO_ROOM);
		check_improve(ch, skill::type::hammerstrike, TRUE, 2);

		affect::add_type_to_char(ch,
			affect::type::hammerstrike,
			ch->level,
			number_fuzzy(ch->level/15),
			get_evolution(ch, skill::type::hammerstrike),
			FALSE
		);
	}
	else {
		WAIT_STATE(ch, 3 * PULSE_VIOLENCE);
		ch->stam = ch->stam / 3;
		stc("You call for power from the gods, but you get no answer.\n", ch);
		check_improve(ch, skill::type::hammerstrike, FALSE, 2);
	}
} /* end do_hammerstrike */

void do_critical_blow(Character *ch, String argument)
{
	Object *weapon;
	int chance;

	if (!CAN_USE_RSKILL(ch, skill::type::critical_blow)) {
		stc("Huh?\n", ch);
		return;
	}

	if ((weapon = get_eq_char(ch, WEAR_WIELD)) == nullptr) {
		stc("You must wield a weapon to critical blow.\n", ch);
		return;
	}

	if (ch->fighting == nullptr) {
		stc("You're not in combat, just pick someone!\n", ch);
		return;
	}

	if (!deduct_stamina(ch, skill::type::critical_blow))
		return;

	WAIT_STATE(ch, skill::lookup(skill::type::critical_blow).beats);

	if (!IS_NPC(ch->fighting) && IS_IMMORTAL(ch->fighting)) {
		stc("You fail miserably.\n", ch);
		return;
	}

	if (number_percent() > get_skill_level(ch, skill::type::critical_blow)) {
		check_improve(ch, skill::type::critical_blow, FALSE, 2);
		stc("Your opponent spotted your move, and your strike misses.\n", ch);
		WAIT_STATE(ch, skill::lookup(skill::type::critical_blow).beats);
		return;
	}

	chance = 100 - (((ch->fighting->hit * 100) / GET_MAX_HIT(ch->fighting)) * 3);

	if (!chance)
		chance = 1;

	if (number_percent() <= chance) {
		ptc(ch, "You thrust your weapon through %s's chest, killing them instantly!\n",
		    ch->fighting->short_descr);
		ch->fighting->hit = -10;
		check_improve(ch, skill::type::critical_blow, TRUE, 2);
	}
	else {
		ptc(ch, "You try to give %s the blow of death, but you fail.\n",
		    ch->fighting->short_descr);
		check_improve(ch, skill::type::critical_blow, FALSE, 2);

		/* Crit Blow failed, let's damage their weapon */
		if (weapon->condition != -1 && number_range(0, 10) == 5) {
			weapon->condition -= number_range(1, 8);

			if (weapon-> condition <= 0) {
				stc("Your failed attack has {Pdestroyed{x your weapon!\n", ch);
				destroy_obj(weapon);
				WAIT_STATE(ch, skill::lookup(skill::type::critical_blow).beats);
				return;
			}

			stc("Your failed attack has {Wdamaged{x your weapon!\n", ch);
		}
	}

	one_hit(ch, ch->fighting, skill::type::critical_blow, FALSE);
} /* end do_critical_blow */

/* Riposte, originally by Elrac */
void do_riposte(Character *victim, Character *ch)
{
	int chance = (get_skill_level(victim, skill::type::riposte));

	if (!chance)
		return;

	chance += (victim->level - ch->level);

	if (number_percent() > chance) {
		check_improve(victim, skill::type::riposte, FALSE, 6);
		return;
	}

	/* gonna riposte, last check for dodging/blurring/shield blocking it */
	if (check_dodge(victim, ch, skill::type::riposte, 0))
		return;

	if (check_blur(victim, ch, skill::type::riposte, 0))
		return;

	if (check_shblock(victim, ch, skill::type::riposte, 0))
		return;

	if (check_dual_parry(victim, ch, skill::type::riposte, 0))
		return;

	/* success, do the riposte */
	act("{GIn a brilliant riposte, you strike back at $n{G!{x", ch, nullptr, victim, TO_VICT);
	one_hit(victim, ch, skill::type::riposte, FALSE);
	check_improve(victim, skill::type::riposte, TRUE, 6);
} /* end do_riposte */

/* RAGE by Montrey */
void do_rage(Character *ch, String argument)
{
	Character *vch;
	Character *vch_next;
	int pplhit = 0;

	if (!CAN_USE_RSKILL(ch, skill::type::rage)) {
		stc("Your meager skill with weapons prevents it.\n", ch);
		return;
	}

	if (GET_ROOM_FLAGS(ch->in_room).has(ROOM_SAFE) && !IS_IMMORTAL(ch)) {
		stc("Oddly enough, in this room you feel peaceful.", ch);
		return;
	}

	if (!deduct_stamina(ch, skill::type::rage))
		return;

	WAIT_STATE(ch, skill::lookup(skill::type::rage).beats);

	if (number_percent() > get_skill_level(ch, skill::type::rage)) {
		stc("You scream a battlecry but fail to unleash your inner rage.\n", ch);
		act("$n starts into a wild series of attacks, but $s timing is off.", ch, nullptr, nullptr, TO_ROOM);
		check_improve(ch, skill::type::rage, FALSE, 2);
		return;
	}

	/* Let's dance */
	act("You scream a battle cry and unleash your rage!", ch, nullptr, nullptr, TO_CHAR);
	act("$n screams a battle cry, and goes into a wild series of attacks!", ch, nullptr, nullptr, TO_ROOM);

	for (vch = char_list; vch != nullptr; vch = vch_next) {
		vch_next = vch->next;

		if (ch == vch)
			continue;

		if (vch->in_room == nullptr)
			continue;

		if (is_same_group(ch, vch))
			continue;

		if (is_safe_spell(ch, vch, TRUE))
			continue;

		if (vch->in_room == ch->in_room) {
			check_killer(ch, vch);
			multi_hit(ch, vch, skill::type::rage);

			if (++pplhit > 4)
				break;
		}
	}

	check_improve(ch, skill::type::rage, TRUE, 2);
}

void do_lay_on_hands(Character *ch, String argument)
{
	int heal, skill;
	Character *victim;

	if (IS_NPC(ch))
		return;

	skill = get_skill_level(ch, skill::type::lay_on_hands);

	if (skill <= 1) {
		stc("You don't know how to use Lay on Hands.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("Syntax: lay <target>\n"
		    "        lay count\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg.is_prefix_of("count")) {
		ptc(ch, "You may use Lay on Hands %d more times today.\n",
		    ch->pcdata->lays);
		return;
	}

	if (ch->pcdata->lays <= 0) {
		stc("Your power of Lay on Hands has run out.\n", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	/* Sure, why not let it work on mobs. -- Outsider
	if ( IS_NPC(victim) )
	{
	   stc("Lay on Hands doesn't work well for mobs.\n", ch);
	   return;
	}
	*/

	if (victim == ch) {
		stc("To Lay on Hands on yourself is useless.\n", ch);
		return;
	}

	if (!deduct_stamina(ch, skill::type::lay_on_hands))
		return;

	act("$n lays $s hands on $N.", ch, nullptr, victim, TO_NOTVICT);
	act("You lay your hands on $N.", ch, nullptr, victim, TO_CHAR);
	act("$n lays $s hands on you.", ch, nullptr, victim, TO_VICT);
	ch->pcdata->lays--;
	WAIT_STATE(ch, skill::lookup(skill::type::lay_on_hands).beats);
	heal = ch->level;
	heal = (heal * skill) / 100;
	victim->hit = UMIN(victim->hit + heal, GET_MAX_HIT(victim));
	update_pos(victim);
	stc("You feel better.\n", victim);
	stc("Your hands glow softly as a sense of divine power travels through you.\n", ch);
	check_improve(ch, skill::type::lay_on_hands, TRUE, 1);
	return;
}

void do_shoot(Character *ch, String argument)
{
	Character *victim = nullptr;

	if (IS_NPC(ch))
		return;

	/* make sure we are holding a bow */
	Object *wield = get_eq_char(ch, WEAR_WIELD);
	int dir = -1;

	if (wield == nullptr) {
		stc("You are not wielding anything.\n", ch);
		return;
	}

	if (wield->value[0] != WEAPON_BOW) {
		stc("You are not wielding a bow.\n", ch);
		return;
	}

	if (argument.empty()) { // no arguments, can only target current opponent
		victim = ch->fighting; // could be nullptr

		if (victim == nullptr) {
			stc("Whom do you want to shoot?\n", ch);
			return;
		}
	}
	else { // if any arguments, try parsing the first one for a direction
		String dir_str, dir_arg;
		String target_str = one_argument(argument, dir_arg);
		RoomPrototype *target_room = nullptr;
		int distance = MAX_BOW_DISTANCE;
		bool nearest = FALSE;

		// north = 1, north; 1.north = 1, north; 2.north = 2, north; etc
		if (isdigit(dir_arg[0]))
			distance = UMIN(number_argument(dir_arg, dir_str), MAX_BOW_DISTANCE);
		else {
			nearest = TRUE;
			dir_str = dir_arg;
		}

		     if (dir_str.is_prefix_of("north")) dir = DIR_NORTH;
		else if (dir_str.is_prefix_of("east"))  dir = DIR_EAST;
		else if (dir_str.is_prefix_of("south")) dir = DIR_SOUTH;
		else if (dir_str.is_prefix_of("west"))  dir = DIR_WEST;
		else if (dir_str.is_prefix_of("up"))    dir = DIR_UP;
		else if (dir_str.is_prefix_of("down"))  dir = DIR_DOWN;

		// find the target room
		if (dir == -1) {
			target_str = argument; // revert, search here
			target_room = ch->in_room;
		}
		else {
			if (target_str.empty()) {
				stc("Whom do you want to shoot?\n", ch);
				return;
			}

			RoomPrototype *room = ch->in_room;
			RoomPrototype *to_room;
			Exit *pexit;

			if (nearest) {
				// find the nearest room with that target.  separated from below search code
				// because of different messages to the archer
				for (int i = 0; i < MAX_BOW_DISTANCE; i++) {
					if ((pexit = room->exit[dir]) == nullptr
			         || (to_room = pexit->u1.to_room) == nullptr
			         || !can_see_room(ch, to_room)
					 || (pexit->exit_flags.has(EX_ISDOOR)
				      && pexit->exit_flags.has(EX_CLOSED)))
						break; // target room is nullptr

					if (get_char_room(ch, to_room, target_str, VIS_CHAR)) {
						target_room = to_room;
						break;
					}

					room = to_room;
				} // not found?  fall through with target_room is nullptr
			}
			else {
				for (int i = 0; i < distance; i++) {
					if ((pexit = room->exit[dir]) == nullptr
			         || (to_room = pexit->u1.to_room) == nullptr
			         || !can_see_room(ch, to_room)) {
						ptc(ch, "Alas, you cannot shoot%s in that direction.\n", i == 0 ? "" : " that far");
						return;
					}

					/* check for a door in the way */
					if (pexit->exit_flags.has(EX_ISDOOR)
				     && pexit->exit_flags.has(EX_CLOSED)) {
						stc("A door blocks the path of the arrow.\n", ch);
						return;
					}

					room = to_room;
				}

				target_room = room;
			}
		}

		if (target_room != nullptr)
			victim = get_char_room(ch, target_room, target_str, VIS_CHAR);
	}

	if (victim == nullptr) {
		stc("You do not see your target in that direction.\n", ch);
		return;
	}

	if (!check_attack_ok(ch, victim))
		return;

	/* shoot! */
	stc("You let your arrow fly!\n", ch);

	if (can_see_in_room(victim, victim->in_room)) {
		if (victim->in_room == ch->in_room)
			act("$n shoots an arrow at you!", ch, nullptr, victim, TO_VICT);
		else {
			const String dir_name [] =
			{       "the north", "the east", "the south", "the west", "above", "below"      };

			ptc(victim, "An arrow flies at you from %s!", dir_name[Exit::rev_dir(dir)]);
		}
	}
	else
		stc("Someone is shooting at you!\n", victim);

	// temporarily move the shooter to the victim, makes damage messages easier
	// don't give away the shooter to the victim, make them temp superwiz
	RoomPrototype *old_room = ch->in_room;
	bool was_superwiz = ch->act_flags.has(PLR_SUPERWIZ);

	if (old_room != victim->in_room) {
		char_from_room(ch);
		char_to_room(ch, victim->in_room);
		ch->act_flags += PLR_SUPERWIZ;
	}

	// do the hit
	one_hit(ch, victim, skill::type::unknown, FALSE);

	// move them back
	if (old_room != victim->in_room) {
		char_from_room(ch);
		char_to_room(ch, old_room);

		if (!was_superwiz)
			ch->act_flags -= PLR_SUPERWIZ;

		/* if the target is NPC, then make it hunt the shooter */
		if (IS_NPC(victim)) {
			victim->hunting = ch;
			hunt_victim(victim);
		}
	}

	WAIT_STATE(ch, skill::lookup(skill::type::backstab).beats);
	check_improve(ch, skill::type::archery, TRUE, 5); /* change added for gains on shooting now damnit leave it*/
}   /* end of do bow */

