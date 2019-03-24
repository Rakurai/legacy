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
#include "lootv2.hh"
#include "magic.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobProg.hh"
#include "MobilePrototype.hh"
#include "Object.hh"
#include "ObjectValue.hh"
#include "Player.hh"
#include "QuestArea.hh"
#include "random.hh"
#include "Room.hh"
#include "skill/skill.hh"
#include "String.hh"
#include "tables.hh"
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
//void    group_gain      args((Character *ch, Character *victim));
int    	group_gain      args((Character *ch, Character *victim));
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
bool 	defense_heal	args((Character *victim, int chance, int percent));


/* Global XP */
int gxp;

/* nasty global focus boolean -- Montrey */
bool focus = true;
bool global_quick = false;

/* Control the fights going on.
   Called periodically by update_handler. */
void violence_update(void)
{
	Character *victim;

	// go through first and make sure everybody is fighting who should be fighting
	for (auto ch : Game::world().char_list)
		if (ch->fighting)
			check_assist(ch, ch->fighting);

	for (auto ch : Game::world().char_list) {
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
		if (ch->is_npc()
		    && ch->fighting == nullptr
		    && IS_AWAKE(ch)
		    && ch->hunting != nullptr) {
			hunt_victim(ch);
			continue;
		}

		if ((victim = ch->fighting) == nullptr) {
			/* parasite pk timer off of violence_update.  don't forget it's 3 seconds -- Montrey */

			if (!ch->is_npc()) {
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
                if (!ch->is_npc())
                        ch->pcdata->combattimer = 5;

		combat_regen(ch);

		/* this is moved from the damage function cause i don't want you to
		   actually have to be getting hit to stand up -- Montrey */
		if (ch->wait == 0 && get_position(ch) < POS_FIGHTING) {
			act("You clamber to your feet.", ch, nullptr, nullptr, TO_CHAR);
			act("$n clambers to $s feet.", ch, nullptr, nullptr, TO_ROOM);

			/* prevent spam bashing -- Montrey */
			if (ch->is_npc())
				ch->wait = 1;
			else {
				Character *rch;

				for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
					if (rch->is_npc()
					    && rch->fighting == ch
					    && get_position(rch) == POS_FIGHTING)
						rch->wait = std::max(rch->wait, 1);
			}

			ch->position = POS_STANDING;

			if (ch->start_pos == POS_FLYING && CAN_FLY(ch))
				do_fly(ch, "");
		}

		if (IS_AWAKE(ch) && ch->in_room == victim->in_room)
			multi_hit(ch, victim, skill::type::unknown);
		else
			stop_fighting(ch, false);

		if (ch == nullptr || ch->fighting == nullptr)
			continue;

//		view_room_hpbar(ch);

		if (ch->wait > 0)
			continue;

		/* Wimp out? */
		if (ch->is_npc()) {
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
		if (ch->is_npc()
		    && victim->is_npc()
		    && affect::exists_on_char(victim, affect::type::charm_person)
		    && victim->master != nullptr
		    && victim->master->in_room == ch->in_room
		    && roll_chance(15)) {
			if (!can_see_char(ch, victim->master)) {
				if (roll_chance(50))
					do_flee(ch, "");
			}
			else {
				act("$n changes the focus of $s attacks.", ch, nullptr, nullptr, TO_ROOM);
				stop_fighting(ch, false);
				set_fighting(ch, victim->master);
				continue;
			}
		}

		if ((victim = ch->fighting) == nullptr)
			continue;

		mprog_hitprcnt_trigger(ch, victim);

		if (ch->is_garbage() || victim->is_garbage())
			continue;

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
	    ch->hit = std::min(GET_MAX_HIT(ch), ch->hit + hitgain);
		ch->mana = std::min(GET_MAX_MANA(ch), ch->mana + managain);
		ch->stam = std::min(GET_MAX_STAM(ch), ch->stam + stamgain);
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
	if (ch->race == 6 && !ch->is_npc()) {
		int sun_damage;
		GameTime::Sun sun = ch->in_room->area().world.time.sunlight;
		Weather::Sky sky = ch->in_room->area().world.weather.sky;

		/* handle the regen first */
		if (ch->in_room->flags().has_any_of(ROOM_DARK | ROOM_INDOORS)
		    || ch->in_room->sector_type() == Sector::inside
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
				damage(ch->fighting, ch, sun_damage, skill::type::unknown, -1, DAM_NONE, false, true);

				if (ch->is_garbage())
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
	
	/* beserker warrior 3pc bonus 
		5% hp healed per round (pending balance) */
	if (GET_ATTR(ch, SET_WARRIOR_BESERKER) >= 3)
		hitgain += (GET_MAX_HIT(ch) * 5 / 100);
	
	/* Cutpurse thief 3pc bonus
		5% stam healed per round (pending balance) */
	if (GET_ATTR(ch, SET_THIEF_CUTPURSE) >= 3)
		stamgain += (GET_MAX_STAM(ch) * 5 / 100);
		
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

	if (ch->hit < GET_MAX_HIT(ch))      ch->hit = std::min(GET_MAX_HIT(ch), ch->hit + hitgain);

	if (ch->mana < GET_MAX_MANA(ch))    ch->mana = std::min(GET_MAX_MANA(ch), ch->mana + managain);

	if (ch->stam < GET_MAX_STAM(ch))    ch->stam = std::min(GET_MAX_STAM(ch), ch->stam + stamgain);
}

void check_all_cond(Character *ch)
{
	int iWear;
	Object *obj;

	if (ch->is_npc() || IS_IMMORTAL(ch))
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

		if (ch->in_room->sector_type() != Sector::arena && !char_in_darena_room(ch)) {
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

		if (ch->is_npc()) {
			if (rch->is_npc()) {
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

						for (Character *vch = ch->in_room->people; vch; vch = vch->next_in_room) {
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
			if (rch->is_npc()) {
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

		if (is_safe(rch, target, false))
			continue;

		if (rch->is_npc())
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
	if (!ch->is_npc())
		ch->pcdata->combattimer = 5;

	/* no attacks for stunnies -- just a check */
	if (get_position(ch) < POS_RESTING)
		return;

	if (ch->is_npc()) {
		mob_hit(ch, victim, attack_skill);
		return;
	}

	one_hit(ch, victim, attack_skill, false);

	if (!ch->fighting)
		return;

	if (get_eq_char(ch, WEAR_SECONDARY)) {
		chance = get_skill_level(ch, skill::type::dual_wield);

		if (CAN_USE_RSKILL(ch, skill::type::dual_second))
			chance += get_skill_level(ch, skill::type::dual_second) / 10;

		chance += ((get_evolution(ch, skill::type::dual_wield) - 1) * 5);

		if (roll_chance(chance)) {
			one_hit(ch, victim, attack_skill, true);
			check_improve(ch, skill::type::dual_wield, true, 6);

			if (!ch->fighting)
				return;
		}
		else
			check_improve(ch, skill::type::dual_wield, false, 6);
	}

	if (affect::exists_on_char(ch, affect::type::haste))
		one_hit(ch, victim, attack_skill, false);

	if (!ch->fighting || attack_skill == skill::type::backstab)
		return;

	chance = get_skill_level(ch, skill::type::second_attack) / 2;

	if (CAN_USE_RSKILL(ch, skill::type::fourth_attack))
		chance += get_skill_level(ch, skill::type::fourth_attack) / 10;

	if (affect::exists_on_char(ch, affect::type::slow))
		chance /= 2;

	if (roll_chance(chance)) {
		one_hit(ch, victim, attack_skill, false);
		check_improve(ch, skill::type::second_attack, true, 5);

		if (!ch->fighting)
			return;
	}

	chance = get_skill_level(ch, skill::type::third_attack) / 4;

	if (CAN_USE_RSKILL(ch, skill::type::fourth_attack))
		chance += get_skill_level(ch, skill::type::fourth_attack) / 10;

	if (affect::exists_on_char(ch, affect::type::slow))
		chance = 0;

	if (roll_chance(chance)) {
		one_hit(ch, victim, attack_skill, false);
		check_improve(ch, skill::type::third_attack, true, 6);

		if (!ch->fighting)
			return;
	}

	chance = get_skill_level(ch, skill::type::fourth_attack) / 2;

	if (affect::exists_on_char(ch, affect::type::slow) || !CAN_USE_RSKILL(ch, skill::type::fourth_attack))
		chance = 0;

	if (roll_chance(chance)) {
		one_hit(ch, victim, attack_skill, false);
		check_improve(ch, skill::type::fourth_attack, true, 6);

		if (!ch->fighting)
			return;
	}

	if (get_eq_char(ch, WEAR_SECONDARY)) {
		chance = get_skill_level(ch, skill::type::dual_second) / 2;

		if (affect::exists_on_char(ch, affect::type::slow) || !CAN_USE_RSKILL(ch, skill::type::dual_second))
			chance = 0;

		chance += ((get_evolution(ch, skill::type::dual_wield) - 1) * 5);

		if (roll_chance(chance)) {
			one_hit(ch, victim, attack_skill, true);
			check_improve(ch, skill::type::dual_second, true, 6);

			if (!ch->fighting)
				return;
		}
		else
			check_improve(ch, skill::type::dual_second, false, 6);
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
			    || ch->in_room->sector_type() == Sector::arena
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

	one_hit(ch, victim, attack_skill, false);

	if (!ch->fighting)
		return;

	if (get_eq_char(ch, WEAR_SECONDARY) && (number_percent() < (ch->level * 2))) {
		one_hit(ch, victim, attack_skill, true);

		if (!ch->fighting)
			return;
	}

	/* Area attack -- BALLS nasty! */
	if (ch->off_flags.has(OFF_AREA_ATTACK)) {
		for (vch = ch->in_room->people; vch != nullptr; vch = vch_next) {
			vch_next = vch->next_in_room;

			if ((vch != victim && vch->fighting == ch && roll_chance(33)))
				one_hit(ch, vch, attack_skill, false);
		}
	}

	if (affect::exists_on_char(ch, affect::type::haste)
	    || (ch->off_flags.has(OFF_FAST) && !affect::exists_on_char(ch, affect::type::slow)))
		one_hit(ch, victim, attack_skill, false);

	if (!ch->fighting || attack_skill == skill::type::backstab)
		return;

	chance = get_skill_level(ch, skill::type::second_attack) / 2;

	if (affect::exists_on_char(ch, affect::type::slow) && !ch->off_flags.has(OFF_FAST))
		chance /= 2;

	if (roll_chance(chance)) {
		one_hit(ch, victim, attack_skill, false);

		if (!ch->fighting)
			return;
	}

	chance = get_skill_level(ch, skill::type::third_attack) / 4;

	if (affect::exists_on_char(ch, affect::type::slow) && !ch->off_flags.has(OFF_FAST))
		chance = 0;

	if (roll_chance(chance)) {
		one_hit(ch, victim, attack_skill, false);

		if (!ch->fighting)
			return;
	}

	chance = get_skill_level(ch, skill::type::fourth_attack) / 6;

	if (affect::exists_on_char(ch, affect::type::slow) && !ch->off_flags.has(OFF_FAST))
		chance = 0;

	if (roll_chance(chance)) {
		one_hit(ch, victim, attack_skill, false);

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
		    || (get_weapon_skill(ch, false) != skill::type::hand_to_hand
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
	bool riposte = false;
	bool shadow = false;
	bool no_weapon = false;

	/* just in case */
	if (victim == ch || ch == nullptr || victim == nullptr)
		return;

	/* Can't beat a dead char!  Guard against weird room leavings. */
	if (get_position(victim) == POS_DEAD || ch->in_room != victim->in_room)
		return;

	if (attack_skill == skill::type::riposte)
		riposte = true;

	if (attack_skill == skill::type::shadow_form) {
		shadow = true;
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
		no_weapon = true;     /* check if unarmed -- Outsider */
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
		check_improve(ch, skill::type::unarmed, true, 1);
	}

	/* Calculate to-hit-armor-class-0 versus armor. */
	if (ch->is_npc()) {
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
		int index = ch->guild == Guild::none ? Guild::warrior : ch->guild;
		thac0_00 = guild_table[index].thac0_00;
		thac0_32 = guild_table[index].thac0_32;
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
			damage(ch, victim, 0, attack_skill, attack_type, dam_type, true, false);
			return;
		}
	}

	/* Hit.  Calc damage. */
	if (ch->is_npc() && wield == nullptr)
		dam = dice(ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);
	else {
		if (sn != skill::type::unknown)
			check_improve(ch, sn, true, 5);

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
		check_improve(ch, skill::type::enhanced_damage, true, 6);
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
		/* 5 pc thief cutpurse bonus */
		if (GET_ATTR (ch, SET_THIEF_CUTPURSE) >= 5 )
			dam += dam * 20 / 100;
	}

	if (attack_skill == skill::type::circle && wield != nullptr) {
		if (wield->value[0] != 2)
			dam *= 2 + ((ch->level - 30) / 30);
		else
			dam *= 2 + ((ch->level - 30) / 24);   /* daggers do more */
		
		/* 5 pc thief cutpurse bonus */
		if (GET_ATTR (ch, SET_THIEF_CUTPURSE) >= 5 )
			dam += dam * 20 / 100;
	}

	if (attack_skill == skill::type::rage && wield != nullptr) {
		if (wield->value[0] != 1
		    && wield->value[0] != 5)
			dam *= 2 + ((ch->level - 30) / 30);
		else
			dam *= 2 + ((ch->level - 30) / 24);   /* swords and axes do more */
	}

	dam += GET_ATTR_DAMROLL(ch) * std::min(100, skill) / 100;

	if (dam <= 0)
		dam = 1;

	/* 5 pc thief cutpurse shadowform bonus */
	if (GET_ATTR (ch, SET_THIEF_CUTPURSE) >= 5 )
		bonus += bonus * 20 / 100;
	
	dam += bonus;  /* Shadow Form Bonus */
	result = damage(ch, victim, dam, attack_skill, attack_type, dam_type, true, false);

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
					false
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

			if (ch->in_room->sector_type() != Sector::arena
			    && ch->in_room->sector_type() != Sector::clanarena
			    && (ch->in_room->area() != Game::world().quest.area() || !Game::world().quest.pk))
				gain_exp(victim, 0 - number_range(ch->level / 20, 3 * ch->level / 20));

			if (ch->in_room->sector_type() != Sector::arena
			    && ch->in_room->sector_type() != Sector::clanarena
			    && (ch->in_room->area() != Game::world().quest.area() || !Game::world().quest.pk)
			    && ch->guild != Guild::paladin) /* Paladins */
				ch->alignment = std::max(-1000, ch->alignment - 1);

			act("$p draws life from $n.", victim, wield, nullptr, TO_ROOM);
			act("You feel $p drawing your life away.", victim, wield, nullptr, TO_CHAR);
			damage(ch, victim, dam, skill::type::unknown, -1, DAM_NEGATIVE, false, true);
			
			/*suffix
			 * placeholder for Hexxing (HEXXING)
			 * bonus 10% to blood blade leech effect.
			*/ 
			 
			// draining effect adds hp
			int gain = (dam / 2);
			gain += gain * GET_ATTR(ch, APPLY_VAMP_BONUS_PCT) / 100;
			ch->hit += gain;
		}

		if (ch->fighting == victim
		 && (weaponaff = affect::find_on_obj(wield, affect::type::weapon_vorpal)) != nullptr) {
			dam = number_range(1, wield->level / 4 + 1);
			act("$n is impaled by $p.", victim, wield, nullptr, TO_ROOM);
			act("$p impales your body.", victim, wield, nullptr, TO_CHAR);
			damage(ch, victim, dam, skill::type::unknown, -1, DAM_PIERCE, false, false);
		}

		if (ch->fighting == victim
		 && (weaponaff = affect::find_on_obj(wield, affect::type::weapon_acidic)) != nullptr) {
			evolution = weaponaff->evolution;

			dam = number_range(1, wield->level / 4 + 2);
			act("$n is burned by the acid $p.", victim, wield, nullptr, TO_ROOM);
			act("The acid on $p starts to burn your flesh.", victim, wield, nullptr, TO_CHAR);

			if (victim->in_room->sector_type() != Sector::arena
			    && ch->in_room->sector_type() != Sector::clanarena
			    && (ch->in_room->area() != Game::world().quest.area() || !Game::world().quest.pk))
				acid_effect((void *) victim, wield->level / 2, dam, TARGET_CHAR, evolution);

			damage(ch, victim, dam, skill::type::unknown, -1, DAM_ACID, false, false);
		}

		if (ch->fighting == victim
		 && (weaponaff = affect::find_on_obj(wield, affect::type::weapon_flaming)) != nullptr) {
			evolution = weaponaff->evolution;

			dam = number_range(1, wield->level / 4 + 1);
			act("$n is burned by $p.", victim, wield, nullptr, TO_ROOM);
			act("$p sears your flesh.", victim, wield, nullptr, TO_CHAR);

			if (victim->in_room->sector_type() != Sector::arena
			    && ch->in_room->sector_type() != Sector::clanarena
			    && (ch->in_room->area() != Game::world().quest.area() || !Game::world().quest.pk))
				fire_effect((void *) victim, wield->level / 2, dam, TARGET_CHAR, evolution);

			damage(ch, victim, dam, skill::type::unknown, -1, DAM_FIRE, false, false);
		}

		if (ch->fighting == victim
		 && (weaponaff = affect::find_on_obj(wield, affect::type::weapon_frost)) != nullptr) {
			evolution = weaponaff->evolution;

			dam = number_range(1, wield->level / 6 + 2);
			act("$p freezes $n.", victim, wield, nullptr, TO_ROOM);
			act("The cold touch of $p surrounds you with ice.", victim, wield, nullptr, TO_CHAR);

			if (victim->in_room->sector_type() != Sector::arena
			    && ch->in_room->sector_type() != Sector::clanarena
			    && (ch->in_room->area() != Game::world().quest.area() || !Game::world().quest.pk))
				cold_effect(victim, wield->level / 2, dam, TARGET_CHAR, evolution);

			damage(ch, victim, dam, skill::type::unknown, -1, DAM_COLD, false, false);
		}

		if (ch->fighting == victim
		 && (weaponaff = affect::find_on_obj(wield, affect::type::weapon_shocking)) != nullptr) {
			evolution = weaponaff->evolution;

			dam = number_range(1, wield->level / 5 + 2);
			act("$n is struck by lightning from $p.", victim, wield, nullptr, TO_ROOM);
			act("You are shocked by $p.", victim, wield, nullptr, TO_CHAR);

			if (victim->in_room->sector_type() != Sector::arena
			    && ch->in_room->sector_type() != Sector::clanarena
			    && (ch->in_room->area() != Game::world().quest.area() || !Game::world().quest.pk))
				shock_effect(victim, wield->level / 2, dam, TARGET_CHAR, evolution);

			damage(ch, victim, dam, skill::type::unknown, -1, DAM_ELECTRICITY, false, false);
		}

		if (ch->fighting == victim
		 && GET_ATTR(ch, SET_THIEF_CUTPURSE) >= 5){
			if (attack_skill == skill::type::circle) {
				if (number_percent() > 90) {
					switch(number_range (1,5)){
						case 1:	//blindness
							stc("You strike at your opponent's eyes!\n", ch);
							stc("Your opponent strikes at your eyes!!\n", victim);
							spell_blindness(skill::type::blindness,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
						break;
						case 2:	//curse
							stc("You strike at your opponents chakras!\n", ch);
							stc("Your opponent strikes at your chakras!!\n", victim);
							spell_curse(skill::type::curse,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
						break;
						case 3:	//plague
							stc("Your weapon glows with a green aura!\n", ch);
							stc("Your opponents weapon glows green!\n", victim);
							spell_plague(skill::type::plague,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
						break;
						case 4: //slow
							stc("You strike at your opponent's ankles!\n", ch);
							stc("Your opponent strikes at your ankles!!\n", victim);
							spell_slow(skill::type::slow,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
						break;
						case 5:	//weaken
							stc("You strike at your opponent's nervous system!\n", ch);
							stc("Your opponent strikes at your nervous system!!\n", victim);
							spell_weaken(skill::type::weaken,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
						break;
					}
				}
			}
			if (attack_skill == skill::type::backstab){
				if (number_percent() > 75) {
					switch(number_range (1,5)){
						case 1:	//blindness
							stc("You strike at your opponent's eyes!\n", ch);
							stc("Your opponent strikes at your eyes!!\n", victim);
							spell_blindness(skill::type::blindness,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
						break;
						case 2:	//curse
							stc("You strike at your opponents chakras!\n", ch);
							stc("Your opponent strikes at your chakras!!\n", victim);
							spell_curse(skill::type::curse,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
						break;
						case 3:	//plague
							stc("Your weapon glows with a green aura!\n", ch);
							stc("Your opponents weapon glows green!\n", victim);
							spell_plague(skill::type::plague,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
						break;
						case 4: //slow
							stc("You strike at your opponent's ankles!\n", ch);
							stc("Your opponent strikes at your ankles!!\n", victim);
							spell_slow(skill::type::slow,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
						break;
						case 5:	//weaken
							stc("You strike at your opponent's nervous system!\n", ch);
							stc("Your opponent strikes at your nervous system!!\n", victim);
							spell_weaken(skill::type::weaken,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
						break;
					}
				}
			}
			if (shadow){
				if (number_percent() > 65) {
					switch(number_range (1,5)){
						case 1:	//blindness
							stc("You strike at your opponent's eyes!\n", ch);
							stc("Your opponent strikes at your eyes!!\n", victim);
							spell_blindness(skill::type::blindness,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
						break;
						case 2:	//curse
							stc("You strike at your opponents chakras!\n", ch);
							stc("Your opponent strikes at your chakras!!\n", victim);
							spell_curse(skill::type::curse,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
						break;
						case 3:	//plague
							stc("Your weapon glows with a green aura!\n", ch);
							stc("Your opponents weapon glows green!\n", victim);
							spell_plague(skill::type::plague,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
						break;
						case 4: //slow
							stc("You strike at your opponent's ankles!\n", ch);
							stc("Your opponent strikes at your ankles!!\n", victim);
							spell_slow(skill::type::slow,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
						break;
						case 5:	//weaken
							stc("You strike at your opponent's nervous system!\n", ch);
							stc("Your opponent strikes at your nervous system!!\n", victim);
							spell_weaken(skill::type::weaken,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
						break;
					}
				}
			}
		}
	}
	/* 	This section is for any on hit affects from unique items
		Each will need to be coded individually per item.
		-- Vegita 2018
	*/
	//Pierce of necromancer unique dagger. 5% chance on hit to inflict random maladiction.
	// 9 total maladictions (excluding energy drain)
	if (result && (GET_ATTR(ch, APPLY_NECRO_PIERCE_UNIQUE)!= 0)) {
		int unique_chance = GET_ATTR(ch, APPLY_NECRO_PIERCE_UNIQUE);
		if ( number_percent() < unique_chance ){
			switch(number_range (1,9)){
				default:
					act("$p glows dark towards $n {Rbut then brightens up{x.", victim, wield, nullptr, TO_ROOM);
					act("{RYou see $p {Rglow dark but then it subsides.{x", victim, wield, nullptr, TO_CHAR);
					break;
			
				case 1:	//blindness
					act("$p {Remits dark rays at $n{R's eyes.{x", victim, wield, nullptr, TO_ROOM);
					act("{RYou see $p {Remit dark rays.{x", victim, wield, nullptr, TO_CHAR);
					spell_blindness(skill::type::blindness,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
					break;
				case 2: //weaken
					act("$p {Rgrows dark tendrils that extend towards $n.{x", victim, wield, nullptr, TO_ROOM);
					act("{RYou see $p grow dark tendrils that come after you.{x", victim, wield, nullptr, TO_CHAR);
					spell_weaken(skill::type::weaken,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
					break;
				case 3: //poison
					act("$p {Rstarts emitting a greenish gas towards $n.{x", victim, wield, nullptr, TO_ROOM);
					act("{RYou see $p {Rbecome covered in a greenish gas.{x", victim, wield, nullptr, TO_CHAR);
					spell_poison(skill::type::poison,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
					break;
				case 4: //curse
					act("$p {Rstarts chanting ancient mutterings at $n.{x", victim, wield, nullptr, TO_ROOM);
					act("{RYou begin hearing dark chanting from $p.{x", victim, wield, nullptr, TO_CHAR);
					spell_curse(skill::type::curse,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
					break;
				case 5: //plague
					act("$p {Rstarts spitting out black death at $n.{x", victim, wield, nullptr, TO_ROOM);
					act("{RYou see $p {Rbegin to spew black death.{x", victim, wield, nullptr, TO_CHAR);
					spell_plague(skill::type::plague,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
					break;
				case 6: //slow
					act("$p {Rdark glow forms a clock who's hands are slowing down while pointing at $n.{x", victim, wield, nullptr, TO_ROOM);
					act("{RYou see $p's {Rglow form a clock who's hands are slowing down.{x", victim, wield, nullptr, TO_CHAR);
					spell_slow(skill::type::slow,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
					break;
				case 7: //starve
					act("$p {Remits a high pitch towards $n.{x", victim, wield, nullptr, TO_ROOM);
					act("{RYou hear $p {Rhumming a high pitch.{x", victim, wield, nullptr, TO_CHAR);
					spell_starve(skill::type::starve,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
					break;
				case 8: //age
					act("$p {Rforms a dark glowing clock, its hands speeding up while pointing at $n.{x", victim, wield, nullptr, TO_ROOM);
					act("{RYou see $p {Rform a dark glowing clock, its hands speeding up.{x", victim, wield, nullptr, TO_CHAR);
					spell_age(skill::type::age,   wield->level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
					break;
			}
		}
	}
	
	/*
	 * Beserker warrior 5 pc bonus
	 * Unblockable Furious strike
	 * 10% chance on each hit to trigger.
	 * damage will be currently figured out dam from the calculations above
	 * halved and then half of the chars level flat added to it
	 */
	 if (GET_ATTR(ch, SET_WARRIOR_BESERKER) >= 5){
		 if (number_percent() < 11){
			 int fstrikedam = dam -= dam / 2;
			 fstrikedam += (ch->level / 2);
			 stc("{GYou lash out at your victim{x.\n", victim);
			 damage(ch, victim, fstrikedam, skill::type::unknown, 46, DAM_SLASH, true, false);
		 }
	 }

		
	
} /* end one_hit */

// called on a hit from bone wall
int affect_callback_weaken_bonewall(affect::Affect *node, void *null) {
	if (node->type == affect::type::bone_wall) {
		node->duration = std::max(0, node->duration - 1);

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
	/*
	Object *wield; //vegita
	wield = get_eq_char(ch, WEAR_WIELD);//vegita
	Object *wield2;
	wield2 = get_eq_char(ch, WEAR_SECONDARY);
	*/

	if (get_position(victim) == POS_DEAD)
		return false;

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
		//Object *wield; //vegita
		//wield = get_eq_char(ch, WEAR_WIELD);//vegita
		//Object *wield2;
		//wield2 = get_eq_char(ch, WEAR_SECONDARY);
		
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
		
		/* suffix
		 * placeholder for Arcane Power (ARCANEPOWER) and Mystical Power (MYSTICALPOWER)
		 * ARCANEPOWER is +5% bonus spell damage
		 * MYSTICALPOWER is +10% bonus spell damage
		 *
		 */
		 dam += dam * GET_ATTR(ch, APPLY_SPELL_DAMAGE_PCT) / 100;
		 
		/* Invoker mage set 2pc bonus
		 */
		 if (GET_ATTR(ch, SET_MAGE_INVOKER) >= 2)
			 dam += dam * 10 / 100;
	}

	/* damage reduction */
	if (dam > 35)
		dam = (dam - 35) / 2 + 35;

	if (dam > 80)
		dam = (dam - 80) / 2 + 80;

	if (victim != ch) {
		/* Certain attacks are forbidden.  Most other attacks are returned. */
		if (is_safe(ch, victim, true))
			return false;

		check_killer(ch, victim);

		if (get_position(victim) > POS_STUNNED) {
			if (!ch->is_npc() && !victim->is_npc() && ch->fighting == nullptr) {
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
	if (!victim->is_npc()) {
		if (dam > 1 && victim->pcdata->condition[COND_DRUNK] > 10)
			dam = 9 * dam / 10;

		if (dam > 1 && victim->guild == Guild::paladin) /* enhanced protection for paladins */
			if ((IS_GOOD(victim) && IS_EVIL(ch))
			    || (IS_EVIL(victim) && IS_GOOD(ch)))
				dam -= dam / 4;
	}

	/* BARRIER reduces damage by (currently) 25% -- Elrac */
	if (dam > 1 && affect::exists_on_char(victim, affect::type::barrier))
		dam -= dam / 4;
	
	/*Shield's Chestplate unique effect (25% damage reduction) */
	if (dam > 1 && GET_ATTR(victim, APPLY_TANK_UNIQUE) != 0)
		dam -= dam / 4;
	
	/* Paladin Montrey's Grace 2pc bonus  (10% damage reduction)*/
	if (dam > 1 && GET_ATTR(victim, SET_PALADIN_GRACE) >= 2)
		dam -= dam * 10 / 100;
		
	sanc_immune = false;

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
					sanc_immune = true;
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
	
	/*Beserker warrior 2 pc bonus */
	if (GET_ATTR(ch, SET_WARRIOR_BESERKER) >= 2)
		dam += dam * 10 / 100;
	
	/*Cutpurse thief 2 pc bonus */
	if (GET_ATTR(ch, SET_THIEF_CUTPURSE) >= 2)
		dam += dam * 10 / 100;

	immune = false;

	if (affect::exists_on_char(victim, affect::type::force_shield) && (dam % 4 == 0) && !sanc_immune) {
		immune = true;
		dam = 0;
	}

	/* Check for parry, blur, shield block, and dodge. */
	// the only things that should get through here are weapon hits, so anything coded with
	// an attack_skill or an attack_type of -1 will bypass this section
	if (attack_skill == skill::type::unknown 
						&& attack_type >= 0 
						&& attack_type != 42 	//hack to make elemental auras 
						&& attack_type != 43	//bypass
						&& attack_type != 44
						&& attack_type != 45
						&& attack_type != 46	//berserker set 5pc furious strike
						&& ch != victim) {
		if (IS_AWAKE(victim) && !global_quick) {
			if (check_dodge(ch, victim, attack_skill, attack_type))
				return false;

			// any of these checks could have side effects that kill ch or victim!
			if (ch->is_garbage() || victim->is_garbage())
				return false;

			if (check_blur(ch, victim, attack_skill, attack_type))
				return false;

			if (ch->is_garbage() || victim->is_garbage())
				return false;

			if (check_shblock(ch, victim, attack_skill, attack_type))
				return false;

			if (ch->is_garbage() || victim->is_garbage())
				return false;

			if (check_parry(ch, victim, attack_skill, attack_type))
				return false;

			if (ch->is_garbage() || victim->is_garbage())
				return false;

			if (check_dual_parry(ch, victim, attack_skill, attack_type))
				return false;

			if (ch->is_garbage() || victim->is_garbage())
				return false;
		}

		// defenses that could injure the attacker
		if (!spell) {
			if (get_eq_char(ch, WEAR_WIELD) != nullptr)
				check_cond(ch, get_eq_char(ch, WEAR_WIELD));

			/* Elemental aura of Goddess Lidda unique affect
			   cold, water, fire, lightning
			 */
			if (GET_ATTR(victim, APPLY_LIDDA_AURA_UNIQUE) > 1){
				int damm = (number_range((victim->level / 2) , (victim->level / 4 + 3))); 
				if (number_percent() < GET_ATTR(victim, APPLY_LIDDA_AURA_UNIQUE)){
					switch (number_range(0, 3)){
						case 0: //cold
							stc("{GYour {ga{Wur{ga {Gflashes to an {Ci{Wc{Cy {Gappearance{x.\n", victim);
							//if (!saves_spell(victim->level, ch, DAM_COLD))
								damage(victim, ch, damm, skill::type::unknown, 42, DAM_COLD, true, false);
							break;
						case 1: //water
							stc("{GYour {ga{Wur{ga {Gtakes on a {Nfl{Bu{Nid {Gappearance{x.\n", victim);
							//if (!saves_spell(victim->level, ch, DAM_WATER))
								damage(victim, ch, damm, skill::type::unknown, 45, DAM_WATER, true, false);
							break;
						case 2: //fire
							stc("{GYour {ga{Wur{ga {Gbursts into a glorious {Pf{Rl{ba{Rm{Pe{x.\n", victim);
							//if (!saves_spell(victim->level, ch, DAM_FIRE))
								damage(victim, ch, damm, skill::type::unknown, 43, DAM_FIRE, true, false);
							break;
						case 3: //lightning
							stc("{GYour {ga{Wur{ga {Gbegins to crackle with raw {Ye{bn{Ter{bg{Yy{x.\n", victim);
							//if (!saves_spell(victim->level, ch, DAM_ELECTRICITY))
								damage(victim, ch, damm, skill::type::unknown, 44, DAM_ELECTRICITY, true, false);
							break;
					}
				}
			}
			
			if (affect::exists_on_char(victim, affect::type::flameshield) 
				&& !saves_spell(victim->level, ch, DAM_FIRE)
				&& attack_type != 42 //ignores elemental aura
				&& attack_type != 43 //attack_types as they shouldn't trigger
				&& attack_type != 44 //flameshield
				&& attack_type != 45
				&& attack_type != 46){ //beserker 5pc furious strike
				damage(victim, ch, 5, skill::type::flameshield, -1, DAM_FIRE, true, true);
			}

			if (ch->is_garbage())
				return false;

			if (affect::exists_on_char(victim, affect::type::sanctuary)
			    && get_affect_evolution(victim, affect::type::sanctuary) >= 3
			    && !saves_spell(victim->level, ch, DAM_HOLY)
				&& attack_type != 42 //ignores elemental aura
				&& attack_type != 43 //attack_types as they shouldn't trigger
				&& attack_type != 44 //sanctuary
				&& attack_type != 45
				&& attack_type != 46)//beserker 5pc furious strike
				damage(victim, ch, 5, skill::type::sanctuary, -1, DAM_HOLY, true, true);

			if (ch->is_garbage())
				return false;
			
			const affect::Affect *paf;
			if ((paf = affect::find_on_char(victim, affect::type::bone_wall)) != nullptr
			    && !saves_spell(paf->level, ch, DAM_PIERCE)
			 && !victim->is_garbage()) {
				damage(victim, ch,
				       std::max(number_range(paf->level * 3 / 4, paf->level * 5 / 4), 5),
				       skill::type::bone_wall, -1, DAM_PIERCE, true, true);

				affect::iterate_over_char(victim, affect_callback_weaken_bonewall, nullptr);
			}

			if (ch->is_garbage())
				return false;
		}

		// did the defense kill the attacker?
		if (!ch->fighting)
			return false;
	}

	int def_mod = GET_DEFENSE_MOD(victim, dam_type);

	if (def_mod >= 100)
		immune = true;

	dam -= dam * def_mod / 100;

	/* new damage modification by armor -- Elrac and Sharra */
	if (!victim->is_npc() && dam > 0 && !spell) {
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

	/* suffix
	 * placeholder for Devastation (DEVASTATION) and Annihilation (ANNIHILATION)
	 * DEVASTATION is +5% bonus to damage.
	 * ANNIHILATION is +10% bonus to damage.
	 */
	dam += dam * GET_ATTR(ch, APPLY_WPN_DAMAGE_PCT) / 100;
	
	if (show)
		dam_message(ch, victim, dam, attack_skill, attack_type, immune, sanc_immune);
	
//	if (dam == 0)
//		return false;

	if (!spell)
		check_all_cond(victim);

	/* Hurt the victim.  Inform the victim of his new state. */
	victim->hit -= dam;
	
	/* Cleric Divine Set 5pc bonus
	 * chance to heal for a portion of the damage recieved.
	 * will be a lower heal amount due to it having the ability to
	 * trigger on every damage recieved
	 */
	 if (GET_ATTR(victim, SET_CLERIC_DIVINE) >= 5){
		 if (number_percent() < 15){
			 int dsetheal = dam * 10 / 100;
			 ptc(victim, "{BDivine powers convert some of the damage to healing!! {H[{Y%d{H]{x,\n", dsetheal);
			 ptc(ch, "{BA divine aura flashes from {Y%s{x \n", ch->name);
			 victim->hit += dsetheal;
		}
	}

	if (dam > GET_MAX_HIT(victim) / 4)
		stc("{PThat really did HURT!{x\n", victim);

	if (victim->hit < GET_MAX_HIT(victim) / 4)
		stc("{PYou sure are BLEEDING!{x\n", victim);

	/* are they dead yet? */
	if (!check_pulse(victim)) {
		kill_off(ch, victim);
		return true;
	}

	/* Sleep spells and extremely wounded folks. */
	if (!IS_AWAKE(victim))
		stop_fighting(victim, false);

	if (victim == ch)
		return true;

	/* Take care of link dead people. */
	if (!victim->is_npc() && victim->desc == nullptr) {
		if (number_range(0, victim->wait) == 0) {
			do_recall(victim, "");
			return true;
		}
	}

	return true;
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
		if (roll_chance(die_hard_skill)) {
			/* they have to be dying for this to kick in */
			victim->hit += (GET_ATTR_CON(victim) / 10) * (die_hard_skill / 10);

			if (victim->hit > GET_MAX_HIT(victim))
				victim->hit = GET_MAX_HIT(victim);

			stc("You make an effort to pull yourself together!\n", victim);
			act("$n pulls themself together!\n", victim, nullptr, nullptr, TO_ROOM);
			check_improve(victim, skill::type::die_hard, true, 2);
		}
		else
			check_improve(victim, skill::type::die_hard, false, 2);
	}  /* end of die hard */

	update_pos(victim);

	switch (get_position(victim)) {
	case POS_MORTAL:
		act("$n is mortally wounded, and will die soon, if not aided.", victim, nullptr, nullptr, TO_ROOM);
		stc("You are mortally wounded, and will die soon, if not aided.\n", victim);
		return true;

	case POS_INCAP:
		act("$n is incapacitated and will slowly die, if not aided.", victim, nullptr, nullptr, TO_ROOM);
		stc("You are incapacitated and will slowly die, if not aided.\n", victim);
		return true;

	case POS_STUNNED:
		act("$n is stunned, but will probably recover.", victim, nullptr, nullptr, TO_ROOM);
		stc("You are stunned, but will probably recover.\n", victim);
		return true;

	case POS_DEAD:
		act("$n is DEAD!!", victim, nullptr, nullptr, TO_ROOM);
		stc("You have been KILLED!!\n\n", victim);

		if (victim->level < 50)
			stc("Your corpse can be located in the Morgue, down from the pit.\n", victim);

		return false;

	default:
		return true;
	}
} /* end check_pulse */

void kill_off(Character *ch, Character *victim)
{
	char buf[MAX_STRING_LENGTH];

	// announcements
	if (!victim->is_npc()) {
		Logging::logf("%s killed by %s at %s", victim->name,
		        (ch->is_npc() ? ch->short_descr : ch->name), victim->in_room->location.to_string());
		String log_buf = Format::format("<PK> %s was slain by %s at [{W%s{x] [{W%d Exp{x]",
		        victim->name, (ch->is_npc() ? ch->short_descr : ch->name),
		        ch->in_room->location.to_string(), ch->is_npc() ? 0 : gxp);
		wiznet(log_buf, nullptr, nullptr, WIZ_DEATHS, 0, 0);
		Format::sprintf(buf, "%s has been slain by %s.",  victim->name, (ch->is_npc() ? ch->short_descr : ch->name));
		do_send_announce(victim, buf);
	}
	else {
		String log_buf = Format::format("%s got ToAsTeD by %s at [{W%s{x] [{W%d Exp{x]",
		        (victim->is_npc() ? victim->short_descr : victim->name),
		        (ch->is_npc() ? ch->short_descr : ch->name), ch->in_room->location.to_string(), gxp);
		wiznet(log_buf, nullptr, nullptr, WIZ_MOBDEATHS, 0, 0);
	}

	// award exp
	int least_exp_awarded = group_gain(ch, victim); 
	

	// raw_kill will extract an NPC, so don't refer to NPC victims after this point
	bool was_NPC = victim->is_npc();
	bool is_suicide = (ch == victim);

	// make them die
	raw_kill(victim);

	// suicides?
	if (is_suicide)
		return;

	if ((ch->in_room->sector_type() == Sector::arena) && !was_NPC && (battle.start))
		deduct_cost(ch, -battle.fee);

	// looting NPC corpse that was just made in raw_kill
	if (!ch->is_npc() && was_NPC) {
		Object *corpse, *obj, *obj_next;

		if ((corpse = get_obj_list(ch, "corpse", ch->in_room->contents)) == nullptr
		    || !can_see_obj(ch, corpse))
			return;
		
		if (roll_chance(30) 
			&& least_exp_awarded > 0
			&& ch->act_flags.has(PLR_AUTOLOOT)){ //loot system roll chance (30% right now)
			obj = generate_eq(ch->level);
			if (obj){
				obj_to_obj(obj, corpse);
				get_obj(ch, obj, corpse);
			}
		}
		
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
	    && victim->in_room->sector_type() != Sector::arena
	    && victim->in_room->sector_type() != Sector::clanarena
	    && (ch->in_room->area() != Game::world().quest.area() || !Game::world().quest.pk))
		gain_exp(victim,
		         (2 * (exp_per_level(victim, victim->pcdata->points)*victim->level - victim->exp) / 3));

	if (!ch->is_npc()) {

		if (ch->in_room->sector_type() == Sector::arena
		    || ch->in_room->sector_type() == Sector::clanarena
		    || (ch->in_room->area() == Game::world().quest.area() && Game::world().quest.pk)) {
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
		return false;

	if (victim->fighting == ch || victim == ch)
		return false;

	/* killing mobiles */
	if (victim->is_npc()) {
		if (victim->pIndexData->pShop != nullptr) {
			if (showmsg)
				stc("But then who would you buy supplies from?!\n", ch);

			return true;
		}

		/* no killing healers, trainers, etc */
		if (victim->act_flags.has(ACT_TRAIN)
		    || victim->act_flags.has(ACT_PRACTICE)
		    || victim->act_flags.has(ACT_IS_HEALER)
		    || victim->act_flags.has(ACT_IS_CHANGER)) {
			if (showmsg)
				stc("Have you no moral fibre whatsoever?!\n", ch);

			return true;
		}

		if (!ch->is_npc()) {
			/* no pets */
			if (victim->act_flags.has(ACT_PET) && affect::exists_on_char(victim, affect::type::charm_person)) {
				if (showmsg)
					act("But $N looks so cute and cuddly.", ch, nullptr, victim, TO_CHAR);

				return true;
			}

			/* no charmed creatures unless owner */
			if (affect::exists_on_char(victim, affect::type::charm_person) && ch != victim->master) {
				if (showmsg)
					stc("That is not your charmed creature!\n", ch);

				return true;
			}
		}
		else {
			/* mob killing mob */
			if (affect::exists_on_char(victim, affect::type::charm_person)
			    && ch->master != nullptr && victim->master != nullptr
			    && !ch->master->is_npc() && !victim->master->is_npc()
			    && ch->master != victim->master
			    && is_safe_char(ch->master, victim->master, false)) {
				if (showmsg)
					stc("Their master would consider that an unfriendly act.\n", ch);

				return true;
			}
		}
	}
	/* killing players */
	else {
		/* NPC doing the killing */
		if (ch->is_npc()) {
			/* charmed mobs and pets cannot attack players while owned */
			if (affect::exists_on_char(ch, affect::type::charm_person) && ch->master != nullptr
			    && ch->master->fighting != victim) {
				if (showmsg)
					stc("Players are your friends!\n", ch);

				return true;
			}
		}
		/* player doing the killing */
		else {
			if (victim->act_flags.has(PLR_KILLER) || victim->act_flags.has(PLR_THIEF))
				return false;

			if (victim->level > ch->level + 8 || ch->level > victim->level + 8) {
				if (showmsg)
					stc("Pick on someone your own size.\n", ch);

				return true;
			}

			if (char_opponents(ch, victim))
				return false;

			if (!victim->pcdata->plr_flags.has(PLR_PK)) {
				if (showmsg)
					stc("They are not in the mood to PK right now.\n", ch);

				return true;
			}

			if (!ch->pcdata->plr_flags.has(PLR_PK)) {
				if (showmsg)
					stc("You are not in the mood to PK right now.\n", ch);

				return true;
			}
		}
	}

	return false;
}

bool is_safe(Character *ch, Character *victim, bool showmsg)
{
	if (victim->in_room == nullptr || ch->in_room == nullptr)
		return true;

//	if (ch->on != nullptr && ch->on->pIndexData->item_type == ITEM_COACH)
//		return true;

	/* safe room? */
	if (victim->in_room->flags().has(ROOM_SAFE)) {
		if (showmsg)
			stc("Oddly enough, in this room you feel peaceful.\n", ch);

		return true;
	}

	if (victim->in_room->sector_type() == Sector::arena
	    || victim->in_room->sector_type() == Sector::clanarena
	    || char_in_darena_room(victim))
		return false;

	/* almost anything goes in the quest area if UPK is on */
	if (Game::world().quest.pk
	    && victim->in_room->area() == Game::world().quest.area()
	    && ch->in_room->area() == Game::world().quest.area())
		return false;

	return is_safe_char(ch, victim, showmsg);
}

bool is_safe_spell(Character *ch, Character *victim, bool area)
{
	if (victim->in_room == nullptr || ch->in_room == nullptr)
		return true;

//	if (ch->on != nullptr && ch->on->pIndexData->item_type == ITEM_COACH)
//		return true;

	if (IS_IMMORTAL(ch) && !area)
		return false;

	if (ch->in_room->flags().has(ROOM_SAFE))
		return true;

	if (victim == ch && area)
		return true;

	if (victim->fighting == ch || victim == ch)
		return false;

	if (!IS_IMMORTAL(ch) && IS_IMMORTAL(victim))
		return true;

	if (!IS_IMMORTAL(ch) && victim->invis_level > ch->level)
		return true;

	if ((victim->in_room->sector_type() == Sector::arena
	     || victim->in_room->sector_type() == Sector::clanarena)
	    && (battle.start))
		return false;

	if (char_in_darena_room(victim))
		return false;

	if (ch->is_npc() && ch->act_flags.has(ACT_MORPH) && !victim->is_npc())
		return true;

	if (affect::exists_on_char(ch, affect::type::fear))
		return true;

	/* killing mobiles */
	if (victim->is_npc()) {
		/* safe room? */
		if (victim->in_room->flags().has(ROOM_SAFE))
			return true;

		if (victim->pIndexData->pShop != nullptr)
			return true;

		/* no killing healers, trainers, etc */
		if (victim->act_flags.has(ACT_TRAIN)
		    || victim->act_flags.has(ACT_PRACTICE)
		    || victim->act_flags.has(ACT_IS_HEALER)
		    || victim->act_flags.has(ACT_IS_CHANGER))
			return true;

		if (!ch->is_npc()) {
			/* no pets */
			if (victim->act_flags.has(ACT_PET))
				return true;

			/* no charmed creatures unless owner */
			if (affect::exists_on_char(victim, affect::type::charm_person) && (area || ch != victim->master))
				return true;

			/* legal kill? -- cannot hit mob fighting non-group member */
			if (victim->fighting != nullptr && !is_same_group(ch, victim->fighting))
				return true;
		}
		/* area effect spells do not hit other mobs */
		else if (area && !is_same_group(victim, ch->fighting))
			return true;
	}
	/* killing players */
	else {
		if (area && IS_IMMORTAL(victim))
			return true;

		/* NPC doing the killing */
		if (ch->is_npc()) {
			/* charmed mobs and pets cannot attack players while owned */
			if (affect::exists_on_char(ch, affect::type::charm_person) && ch->master != nullptr
			    && ch->master->fighting != victim)
				return true;

			/* safe room? */
			if (victim->in_room->flags().has(ROOM_SAFE))
				return true;

			/* legal kill? -- mobs only hit players grouped with opponent*/
			if (ch->fighting != nullptr && !is_same_group(ch->fighting, victim))
				return true;
		}
		/* player doing the killing */
		else {
			if (victim->in_room->flags().has(ROOM_SAFE))
				return true;

			/* almost anything goes in questland if UPK is up */
			if (Game::world().quest.pk
			    && ch->in_room->area() == Game::world().quest.area()
			    && victim->in_room->area() == Game::world().quest.area())
				return false;

			if (victim->act_flags.has(PLR_KILLER) || victim->act_flags.has(PLR_THIEF))
				return false;

			if (ch->level > victim->level + 8)
				return true;

			if (char_opponents(ch, victim))
				return false;

			if (!victim->pcdata->plr_flags.has(PLR_PK)
			    || !ch->pcdata->plr_flags.has(PLR_PK))
				return true;
		}
	}

	return false;
} /* end is_safe_spell */

/* See if an attack justifies a KILLER flag. */
void check_killer(Character *ch, Character *victim)
{
	/* Follow charm thread to responsible character.  Attacking someone's charmed char is hostile!
	   Beware, this will cause a loop if master->pet->master - Lotus */
	while (affect::exists_on_char(victim, affect::type::charm_person) && victim->master != nullptr)
		victim = victim->master;

	/* NPC's are fair game.  So are killers and thieves. */
	if (victim->is_npc()
	    || victim->act_flags.has(PLR_KILLER)
	    || victim->act_flags.has(PLR_THIEF)
	    || victim->in_room->sector_type() == Sector::arena
	    || victim->in_room->sector_type() == Sector::clanarena
	    || char_in_darena(victim))
		return;

	/* if in questlands and UPK flag is up, all is fair */
	if (Game::world().quest.pk
	    && victim->in_room != nullptr
	    && victim->in_room->area() == Game::world().quest.area()
	    && ch->in_room != nullptr
	    && ch->in_room->area() == Game::world().quest.area())
		return;

	/* all's fair in war */
	if (char_opponents(ch, victim))
		return;

	/* Charm-o-rama, you can attack your charmed player */
	if (affect::exists_on_char(ch, affect::type::charm_person)) {
		if (ch->master == nullptr) {
			char buf[MAX_STRING_LENGTH];
			Format::sprintf(buf, "Check_killer: %s charmed with no master", ch->is_npc() ? ch->short_descr : ch->name);
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
	if (ch->is_npc()
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

	if (affect::exists_on_char(victim, affect::type::paralyze)) {
		if (roll_chance(30))
			return false; // don't even check improve
	}

	if (!get_skill_level(victim, skill::type::parry))
		return false;

	chance = get_skill_level(victim, skill::type::parry) * 2 / 3;

	if (get_eq_char(victim, WEAR_WIELD) == nullptr) {
		if (victim->is_npc())
			chance /= 2;
		else
			return false;
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

		default:                skill = std::min(100, victim->level * 3);   break;
		}

		chance -= (100 - skill) / 4;
	}

	if (!can_see_char(victim, ch))
		chance /= 2;

	chance += victim->level - ch->level;
	
#ifdef DEBUG_CHANCE
	ptc(ch, "(parry %d%%)", chance);
	ptc(victim, "(parry %d%%)", chance);
#endif

	if (number_percent() >= chance) {
		check_improve(victim, skill::type::parry, false, 10);
		return false;
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

	check_improve(victim, skill::type::parry, true, 6);

	/* for remorts, do riposte */
	if (CAN_USE_RSKILL(victim, skill::type::riposte))
		do_riposte(victim, ch);
	
	/* Montrey's Grace 5pc Bonus */
	if (GET_ATTR(victim, SET_PALADIN_GRACE) >= 5){
		
		if (!victim->act_flags.has(PLR_DEFENSIVE) && defense_heal(victim, 15, 20)) {
			stc("{BMontrey's Grace heals you!!!!{x\n", victim);
		
			if (!ch->act_flags.has(PLR_DEFENSIVE)) {
				stc("{BYour opponent glows with a {Wwhite ligh{Bt!!!!{x\n", ch);
			}
		}
	}
	
	return true;
} /* end check_parry */

/* Check for parry from the off hand. */
bool check_dual_parry(Character *ch, Character *victim, skill::type attack_skill, int attack_type)
{
	Object *obj;
	char buf[MAX_STRING_LENGTH];
	int chance;
	String attack;

	if (affect::exists_on_char(victim, affect::type::paralyze)) {
		if (roll_chance(30))
			return false; // don't even check improve
	}

	if (!get_skill_level(victim, skill::type::dual_wield)
	    || !get_skill_level(victim, skill::type::parry)
	    || !get_eq_char(victim, WEAR_SECONDARY)
	    || get_evolution(victim, skill::type::dual_wield) < 2
	    || (attack_skill == skill::type::riposte && get_evolution(victim, skill::type::dual_wield) < 3))
		return false;

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

		default:                skill = std::min(100, victim->level * 3);   break;
		}

		chance -= (100 - skill) / 5;
	}

	if (!can_see_char(victim, ch))
		chance /= 2;

	chance += victim->level - ch->level;
		
#ifdef DEBUG_CHANCE
	ptc(ch, "(dlparry %d%%)", chance);
	ptc(victim, "(dlparry %d%%)", chance);
#endif

	if (!roll_chance(chance)) {
		check_improve(victim, skill::type::dual_wield, false, 10);
		return false;
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
	check_improve(victim, skill::type::dual_wield, true, 10);

	/* hilt strike! */
	if (get_evolution(victim, skill::type::dual_wield) < 3
	    || (attack_skill == skill::type::riposte && get_evolution(victim, skill::type::dual_wield) < 4))
		return true;

	chance = ((get_skill_level(victim, skill::type::dual_wield) + get_skill_level(victim, skill::type::hand_to_hand)) / 3);

	if (!can_see_char(victim, ch))
		chance /= 2;

	if (!roll_chance(chance + victim->level - ch->level))
		return true; /* for dual parry */

	// get our hilt strike type, probably at the end so go backwards
	for (attack_type = attack_table.size() - 1; attack_type >= 0; attack_type--)
		if (attack_table[attack_type].name == "hstrike")
			break;

	if (attack_type < 0) {
		Logging::bug("check_dual_parry: unable to find hilt strike damage type", 0);
		return true;
	}

	/* gonna riposte, last check for dodging/blurring/shield blocking it */
	if (check_dodge(victim, ch, skill::type::unknown, attack_type))
		return true;

	if (check_blur(victim, ch, skill::type::unknown, attack_type))
		return true;

	if (check_shblock(victim, ch, skill::type::unknown, attack_type))
		return true;

	damage(victim, ch, (number_range(1, victim->level) + GET_ATTR_DAMROLL(victim)) / 2,
	       skill::type::unknown, attack_type, DAM_BASH, true, false);
	check_improve(victim, skill::type::hand_to_hand, true, 8);
	
	/* Montrey's Grace 5pc Bonus */
	if (GET_ATTR(victim, SET_PALADIN_GRACE) >= 5){
		
		if (!victim->act_flags.has(PLR_DEFENSIVE) && defense_heal(victim, 15, 20)) {
			stc("{BMontrey's Grace heals you!!!!{x\n", victim);
		
			if (!ch->act_flags.has(PLR_DEFENSIVE)) {
				stc("{BYour opponent glows with a {Wwhite ligh{Bt!!!!{x\n", ch);
			}
		}
	}
	
	return true;
} /* end check_dual_parry */

/* Check for shield block. */
bool check_shblock(Character *ch, Character *victim, skill::type attack_skill, int attack_type)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	Object *obj = (get_eq_char(victim, WEAR_SHIELD));
	
	int chance;
	String attack;

	if (affect::exists_on_char(victim, affect::type::paralyze)) {
		if (roll_chance(30))
			return false; // don't even check improve
	}

	if (!get_skill_level(victim, skill::type::shield_block))
		return false;

	if (get_eq_char(victim, WEAR_SHIELD) == nullptr)
		return false;

	chance = get_skill_level(victim, skill::type::shield_block) * 2 / 5;

	chance += (victim->level - ch->level);
	
		
#ifdef DEBUG_CHANCE
	ptc(ch, "(shblock %d%%)", chance);
	ptc(victim, "(shblock %d%%)", chance);
#endif

	if (number_percent() >= chance) {
		check_improve(victim, skill::type::shield_block, false, 10);
		return false;
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
	
	/* Shield block specific suffixes */
	if (GET_ATTR(victim, APPLY_HP_BLOCK_PCT) != 0){
		int life_restored =  (number_range(1, obj->level / 2) + GET_ATTR(victim, APPLY_HP_BLOCK_PCT));
		victim->hit += life_restored;
		Format::sprintf(buf2, "{BYour block restores %d of your life.{x\n", life_restored);
		stc(buf2, victim);
	}
	if (GET_ATTR(victim, APPLY_MANA_BLOCK_PCT) != 0){
		int mana_restored = (number_range(1, obj->level / 2) + GET_ATTR(victim, APPLY_MANA_BLOCK_PCT));
		victim->mana += mana_restored;
		Format::sprintf(buf2, "{BYour block restores %d of your mana.{x\n", mana_restored);
		stc(buf2, victim);
	}

	if (!ch->act_flags.has(PLR_DEFENSIVE)) {
		Format::sprintf(buf, "{R$N{R blocks your %s with a shield.{x", attack);
		act(buf, ch, nullptr, victim, TO_CHAR);
	}

	check_cond(victim, get_eq_char(victim, WEAR_SHIELD));
	check_improve(victim, skill::type::shield_block, true, 6);
	
	/* Montrey's Grace 5pc Bonus */
	if (GET_ATTR(victim, SET_PALADIN_GRACE) >= 5){
		
		if (!victim->act_flags.has(PLR_DEFENSIVE) && defense_heal(victim, 15, 20)) {
			stc("{BMontrey's Grace heals you!!!!{x\n", victim);
		
			if (!ch->act_flags.has(PLR_DEFENSIVE)) {
				stc("{BYour opponent glows with a {Wwhite ligh{Bt!!!!{x\n", ch);
			}
		}
	}
	
	return true;
} /* end check_shblock */

/* Check for dodge. */
bool check_dodge(Character *ch, Character *victim, skill::type attack_skill, int attack_type)
{
	char buf[MAX_STRING_LENGTH];
	int chance;
	String attack;

	if (affect::exists_on_char(victim, affect::type::paralyze)) {
		if (roll_chance(30))
			return false; // don't even check improve
	}

	if (!get_skill_level(victim, skill::type::dodge))
		return false;

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
	
		

#ifdef DEBUG_CHANCE
	ptc(ch, "(dodge %d%%)", chance);
	ptc(victim, "(dodge %d%%)", chance);
#endif
	/*Moderate the result*/
	chance = URANGE(5, chance, 95);

//for testing   chance = 0;

	if (number_percent() >= chance) {
		check_improve(victim, skill::type::dodge, false, 10);
		return false;
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
	check_improve(victim, skill::type::dodge, true, 6);
	
	/* Montrey's Grace 5pc Bonus */
	if (GET_ATTR(victim, SET_PALADIN_GRACE) >= 5){
		
		if (!victim->act_flags.has(PLR_DEFENSIVE) && defense_heal(victim, 15, 20)) {
			stc("{BMontrey's Grace heals you!!!!{x\n", victim);
		
			if (!ch->act_flags.has(PLR_DEFENSIVE)) {
				stc("{BYour opponent glows with a {Wwhite ligh{Bt!!!!{x\n", ch);
			}
		}
	}
	
	return true;
} /* end check_dodge */

/* Check for Blur - Montrey */
bool check_blur(Character *ch, Character *victim, skill::type attack_skill, int attack_type)
{
	char buf[MAX_STRING_LENGTH];
	int chance;
	String attack;

	if (affect::exists_on_char(victim, affect::type::paralyze)) {
		if (roll_chance(30))
			return false; // don't even check improve
	}

	if (!CAN_USE_RSKILL(victim, skill::type::blur))
		return false;

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
	
		

#ifdef DEBUG_CHANCE
	ptc(ch, "(blur %d%%)", chance);
	ptc(victim, "(blur %d%%)", chance);
#endif
	/*Moderate the result*/
	chance = URANGE(5, chance, 95);

	if (number_percent() >= chance) {
		check_improve(victim, skill::type::blur, false, 10);
		return false;
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
	check_improve(victim, skill::type::blur, true, 6);
	
	/* Montrey's Grace 5pc Bonus */
	if (GET_ATTR(victim, SET_PALADIN_GRACE) >= 5){
		
		if (!victim->act_flags.has(PLR_DEFENSIVE) && defense_heal(victim, 15, 20)) {
			stc("{BMontrey's Grace heals you!!!!{x\n", victim);
		
			if (!ch->act_flags.has(PLR_DEFENSIVE)) {
				stc("{BYour opponent glows with a {Wwhite ligh{Bt!!!!{x\n", ch);
			}
		}
	}
	
	return true;
}  /* end check_blur */

/* Defensive heal function. created mainly for
   Montrey's Grace 5pc set bonus, could be used for other bonuses
   chance is there for usability with other skills/bonuses
   and percent is the percent you want to heal, again for usability
*/
bool defense_heal(Character *victim, int chance, int percent)
{
	int mod_hp 		= 0;
	int mod_mana	= 0;
	int mod_stam	= 0;
	
	//debug stuff
	int d1 = (GET_MAX_HIT(victim));
	int d2 = (GET_MAX_MANA(victim));
	int d3 = (GET_MAX_STAM(victim));
	
	mod_hp 			+= (GET_MAX_HIT(victim) * percent / 100);
	mod_mana		+= (GET_MAX_MANA(victim) * percent / 100);
	mod_stam		+= (GET_MAX_STAM(victim) * percent / 100);
	
	if (roll_chance(chance)){
		Logging::bug("Debug: mod_hp: %d ",mod_hp);
		Logging::bug("MAX %d",d1);
		Logging::bug("Debug: mod_mana: %d",mod_mana);
		Logging::bug("MAX: %d",d2);
		Logging::bug("Debug: mod_stam: %d",mod_stam);
		Logging::bug("MAX %d",d3);
		
		victim->hit 	+= mod_hp;
		victim->mana 	+= mod_mana;
		victim->stam 	+= mod_stam;
		
		return true;
	}
	return false;
}

/* Set position of a victim. */
void update_pos(Character *victim)
{
	if (victim->hit > 0) {
		if (victim->position <= POS_STUNNED)
			victim->position = POS_STANDING;
	}
	else if ((victim->is_npc() && victim->hit < 1)
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
	for (auto fch : Game::world().char_list) {
		if (fch == ch || (fBoth && fch->fighting == ch)) {
			fch->fighting = nullptr;
			fch->position = fch->is_npc() ? fch->default_pos : fch->start_pos;
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

	if (ch->is_npc()) {
		name          = ch->short_descr;
		corpse        = create_object(Game::world().get_obj_prototype(OBJ_VNUM_CORPSE_NPC), 0);

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

		corpse->cost = std::max(1, (ch->level * 3));
	}
	else {
		name            = ch->name;
		corpse          = create_object(Game::world().get_obj_prototype(OBJ_VNUM_CORPSE_PC), 0);

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
		bool floating = false;
		obj_next = obj->next_content;

		if (obj->wear_loc == WEAR_FLOAT)
			floating = true;

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
		obj_to_room(corpse, Game::world().get_room(ch->clan->recall));
	else if (ch->is_npc() || ch->level >= 50)
		obj_to_room(corpse, ch->in_room);
	else
		obj_to_room(corpse, Game::world().get_room(Location(Vnum(ROOM_VNUM_MORGUE))));
} /* end make_corpse */

/* Improved Death_cry contributed by Diavolo. */
void death_cry(Character *ch)
{
	Room *was_in_room;
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
		name       = ch->is_npc() ? ch->short_descr : ch->name;
		obj        = create_object(Game::world().get_obj_prototype(vnum), 0);

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

	if (ch->is_npc())
		msg = "Nearby, you hear someone's death cry.";
	else
		msg = "You hear the death cry of a fellow player, maybe friend.";

	was_in_room = ch->in_room;

	for (door = 0; door <= 5; door++) {
		Exit *pexit;

		if ((pexit = was_in_room->exit[door]) != nullptr
		    && pexit->to_room != nullptr
		    && pexit->to_room != was_in_room) {
			ch->in_room = pexit->to_room;
			act(msg, ch, nullptr, nullptr, TO_NOTVIEW);
		}
	}

	ch->in_room = was_in_room;
} /* end death_cry */

void raw_kill(Character *victim)
{
	stop_fighting(victim, true);
	mprog_death_trigger(victim);

	if (victim->is_garbage())
		return;

	if (victim->in_room->sector_type() != Sector::arena
	    && victim->in_room->sector_type() != Sector::clanarena
	    && (victim->in_room->area() != Game::world().quest.area() || !Game::world().quest.pk)
	    && !char_in_duel_room(victim))
		make_corpse(victim);

	if (victim->is_npc()) {
		victim->pIndexData->killed++;
		extract_char(victim, true);
		return;
	}

	// only PCs from here down

	victim->pcdata->pktimer = 0;

	affect::remove_all_from_char(victim, false);

	if (victim->in_room->sector_type() != Sector::arena
	    && victim->in_room->sector_type() != Sector::clanarena
	    && (victim->in_room->area() != Game::world().quest.area() || !Game::world().quest.pk)
	    && !char_in_duel_room(victim)) {
		extract_char(victim, false);

		for (int i = 0; i < 4; i++) {
			victim->armor_base[i] = 100;
		}
	}
	else {
		if (char_in_duel(victim))
			duel_kill(victim);
		else {
			char_from_room(victim);
			char_to_room(victim, Game::world().get_room(Location(Vnum(ROOM_VNUM_ALTAR))));
		}
	}

	victim->position    = POS_RESTING;
	victim->hit         = std::max(1, victim->hit);
	victim->mana        = std::max(1, victim->mana);
	victim->stam        = std::max(1, victim->stam);

	save_char_obj(victim);

	/* force a save of items on the ground, should fix it
	   for crashes after a player dies -- Montrey */
	objstate_save_items();

} /* end raw_kill */

//void 
int group_gain(Character *ch, Character *victim)
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

	// hack to pass back the lowest exp gain to see if anyone gets generated eq
	int lowest_xp = 1000000;

	/* Monsters don't get kill xp's or alignment changes.
	   P-killing doesn't help either.
	   Dying of mortal wounds or poison doesn't give xp to anyone! */

	if (victim == ch){
		xp = 0;
		lowest_xp = 0;
	}

	if (ch->guild != Guild::none)
		vary_int += vary_bit[ch->guild];

	/* calculate number of group members present and the sum of their levels */
	for (gch = ch->in_room->people; gch != nullptr; gch = gch->next_in_room) {
		if (is_same_group(gch, ch)) {
			members++;
			group_levels += gch->is_npc() ? gch->level / 2 : gch->level;

			/* figure out how varied the group is -- Montrey */
			if (gch->guild != Guild::none
			 && !vary_int.has(vary_bit[gch->guild])) {
				vary_int += vary_bit[gch->guild];
				diff_classes++;
			}
		}
	}

	if (members == 0) {
		Logging::bug("Group_gain: members.", members);
		members = 1;
		group_levels = ch->level;
	}

	/* Determine the highest level of a present group member */
	for (lch = ch->in_room->people; lch != nullptr; lch = lch->next_in_room) {
		if (!is_same_group(lch, ch) || lch->is_npc())
			continue;

		if (lch->level > highestlevel)
			highestlevel = lch->level;
	}

	/* distribute exp among the remaining deserving */
	for (gch = ch->in_room->people; gch != nullptr; gch = gch->next_in_room) {
		Object *obj;
		Object *obj_next;

		if (!is_same_group(gch, ch) || gch->is_npc())
			continue;

		if (gch->level - highestlevel <= -7) {
			stc("Your powers would be useless to such an advanced group of adventurers.\n", gch);
			continue;
		}

		xp = xp_compute(gch, victim, group_levels, diff_classes);
		gxp = xp;                                   /* Bad bad global*/

		if (ch->in_room->sector_type() == Sector::arena
		    || ch->in_room->sector_type() == Sector::clanarena
		    || (ch->in_room->area() == Game::world().quest.area() && Game::world().quest.pk))
			xp = 0;

		// at this point xp is real
		if (xp < lowest_xp)
			lowest_xp = xp;

		/*suffix
		 *placeholder for suffix affecting exp
		 *of The Experienced			+40 to exp gain
		 */
		xp += xp * GET_ATTR(gch, APPLY_EXP_PCT) / 100;

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

		if (IS_QUESTOR(gch) && victim->is_npc()) {
			if (gch->pcdata->questmob == victim->pIndexData->vnum) {
				stc("{YYou have almost completed your QUEST!{x\n", gch);
				stc("{YReturn to the questmaster before your time runs out!{x\n", gch);
				gch->pcdata->questmob = -1;
				Format::sprintf(buf, "{Y:QUEST: {x$N has slain %s", victim->short_descr);
				wiznet(buf, gch, nullptr, WIZ_QUEST, 0, 0);
			}
		}
	}

	return lowest_xp;
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
	    && victim->in_room->sector_type() != Sector::arena
	    && victim->in_room->sector_type() != Sector::clanarena
	    && (victim->in_room->area() != Game::world().quest.area() || !Game::world().quest.pk)
	    && gch->guild != Guild::paladin) /* Paladins */
	{
		/* do alignment computations */
		int align = victim->alignment - gch->alignment;
		int change;

		if (align > 500) { /* monster is more good than slayer */
			change = (align - 500) * base_exp / 500 * gch->level / total_levels;
			change = std::max(1, change);
			gch->alignment -= change;
		}
		else if (align < -500) { /* monster is more evil than slayer */
			change = (-1 * align - 500) * base_exp / 500 * gch->level / total_levels;
			change = std::max(1, change);
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
	                time_per_level = std::max(time_per_level,(15 - gch->level));

	        xp = xp * time_per_level / 12;
	} */
	/* new to replace above, reduce by up to 1/3 if you're young -- Montrey */
	int years = get_play_seconds(gch) / (MUD_YEAR * MUD_MONTH * MUD_DAY * MUD_HOUR)
	                        + GET_ATTR_MOD(gch, APPLY_AGE);
	xp = xp - (xp / 3) + ((xp / 3) * (URANGE(1, years, 50) / 50));

	/* remort affect - favor of the gods */
	if (HAS_RAFF(gch, RAFF_FAVORGOD) && roll_chance(25)) {
		stc("The gods smile upon you.\n", gch);
		xp = (roll_chance(25) ? (roll_chance(25) ? (xp * 4) : (xp * 3)) : (xp * 2));
	}

	/* remort affect - laughter of the gods */
	if (HAS_RAFF(gch, RAFF_LAUGHTERGOD) && roll_chance(25)) {
		stc("The laughter of the gods falls upon your ears.\n", gch);
		xp = (roll_chance(25) ? (roll_chance(25) ? (xp / 4) : (xp / 3)) : (xp / 2));
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
	xp = xp * gch->level / (std::max(1, total_levels - 1));

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
		ch->hit = std::min(ch->hit, GET_MAX_HIT(ch));
		stc("Your pulse races as you are consumed by rage!\n", ch);
		act("$n gets a wild look in $s eyes.", ch, nullptr, nullptr, TO_ROOM);
		check_improve(ch, skill::type::berserk, true, 2);

		affect::add_type_to_char(ch,
			affect::type::berserk,
			ch->level,
			number_fuzzy(ch->level / 8),
			get_evolution(ch, skill::type::berserk),
			false
		);
	}
	else {
		WAIT_STATE(ch, 3 * PULSE_VIOLENCE);
		stc("Your pulse speeds up, but nothing happens.\n", ch);
		check_improve(ch, skill::type::berserk, false, 2);
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

	if (is_safe(ch, victim, true)) {
		act("It is not a safe target!", ch, victim, nullptr, TO_CHAR);
		return;
	}

	if (victim->is_npc()
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
		check_improve(ch, skill::type::bash, false, 1);
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
	chance = chance * GET_MAX_STAM(victim) / std::max(victim->stam, 1);
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
		check_improve(ch, skill::type::bash, true, 1);
		DAZE_STATE(victim, kdtime);
		WAIT_STATE(victim, kdtime);
		WAIT_STATE(ch, skill::lookup(skill::type::bash).beats);
		damage(ch, victim, number_range(ch->level * 2 / 3, (ch->level * 3 / 2) + chance / 10), skill::type::bash, -1, DAM_BASH, true, false);

		if (!victim->is_garbage()) {
			victim->position = POS_RESTING;

			if (CAN_USE_RSKILL(victim, skill::type::standfast))
				check_improve(victim, skill::type::standfast, false, 1);
		}
	}
	else {
		if (ch->fighting == nullptr) set_fighting(ch, victim);

		act("$n crashes into you, but fails to accomplish anything!", ch, nullptr, victim, TO_VICT);
		act("You throw yourself into $N, but fail to knock them down!", ch, nullptr, victim, TO_CHAR);
		act("$n slams into $N, but neither of them loses their footing.", ch, nullptr, victim, TO_NOTVICT);
		check_improve(ch, skill::type::bash, true, 1);
		DAZE_STATE(victim, PULSE_VIOLENCE);
		WAIT_STATE(ch, skill::lookup(skill::type::bash).beats);

		if (CAN_USE_RSKILL(victim, skill::type::standfast))
			check_improve(victim, skill::type::standfast, true, 1);
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

	if (is_safe(ch, victim, true)) {
		stc("That doesn't seem to work on this opponent.\n", ch);
		return;
	}

	if (victim->is_npc()
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
	switch (ch->in_room->sector_type()) {
	case (Sector::inside):              chance -= 20;   break;
	case (Sector::city):                chance -= 10;   break;
	case (Sector::field):               chance +=  5;   break;
	case (Sector::mountain):            chance -= 10;   break;
	case (Sector::water_swim):          chance  =  0;   break;
	case (Sector::water_noswim):        chance  =  0;   break;
	case (Sector::air):                 chance  =  0;   break;
	case (Sector::desert):              chance += 10;   break;
	default:
		break;
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
			damage(ch, victim, number_range(2, 5), skill::type::dirt_kicking, -1, DAM_NONE, false, false);

			if (!victim->is_garbage()) {
				stc("You can't see a thing!\n", victim);

				affect::add_type_to_char(victim,
					affect::type::dirt_kicking,
					ch->level,
					0,
					get_evolution(ch, skill::type::dirt_kicking),
					false
				);
			}
		}

		check_improve(ch, skill::type::dirt_kicking, true, 2);
		WAIT_STATE(ch, skill::lookup(skill::type::dirt_kicking).beats);
	}
	else {
		act("Your kicked dirt MISSES $N!", ch, nullptr, victim, TO_CHAR);
		check_improve(ch, skill::type::dirt_kicking, false, 2);
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
		damage(ch, victim, number_range(2, 2 +  2 * victim->size), attack_skill, -1, DAM_BASH, true, false);

		if (!victim->is_garbage()) {
			if (CAN_USE_RSKILL(victim, skill::type::standfast))
				check_improve(victim, skill::type::standfast, false, 1);
		}

		return true;
	}
	else {
		damage(ch, victim, 0, attack_skill, -1, DAM_BASH, true, false);

		if (!victim->is_garbage()) {
			if (CAN_USE_RSKILL(victim, skill::type::standfast))
				check_improve(victim, skill::type::standfast, true, 1);
		}

		return false;
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

	if (is_safe(ch, victim, true))
		return;

	if (victim->is_npc()
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
		check_improve(ch, skill::type::trip, true, 1);
		WAIT_STATE(ch, skill::lookup(skill::type::trip).beats);
	}
	else {
		check_improve(ch, skill::type::trip, false, 1);
		WAIT_STATE(ch, skill::lookup(skill::type::trip).beats * 2 / 3);
	}
} /* end do_trip */

bool check_attack_ok(Character *ch, Character *victim) {
	if ((ch->in_room->sector_type() == Sector::arena) && (!battle.start)) {
		stc("Hold your horses, the battle hasn't begun yet!\n", ch);
		return false;
	}

	if (ch->is_npc() && ch->act_flags.has(ACT_MORPH) && !victim->is_npc()) {
		stc("Morphed players cannot attack PC's.\n", ch);
		wiznet("$N is attempting PK while morphed.", ch, nullptr, WIZ_CHEAT, 0, GET_RANK(ch));
		return false;
	}

	if (ch->is_npc() && ch->act_flags.has(ACT_MORPH) && victim->act_flags.has(ACT_PET)) {
		stc("Morphed players cannot attack pets.\n", ch);
		wiznet("$N is attempting to kill a pet while morphed.", ch, nullptr, WIZ_CHEAT, 0, GET_RANK(ch));
		return false;
	}

	if (affect::exists_on_char(ch, affect::type::fear)) {
		stc("But they would beat the stuffing out of you!!\n", ch);
		return false;
	}

	if (victim == ch) {
		stc("I believe you are suffering from a mild case of schizophrenia.\n", ch);
		return false;
	}

	if (is_safe(ch, victim, true))
		return false;

	if (victim->fighting != nullptr
	    && !is_same_group(ch, victim->fighting)) {
		stc("Kill stealing is not permitted.\n", ch);
		return false;
	}

	if (affect::exists_on_char(ch, affect::type::charm_person) && ch->master == victim) {
		act("$N is your beloved master.", ch, nullptr, victim, TO_CHAR);
		return false;
	}

	return true;
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
	Room *location;
	Descriptor *d;
	Character *ach;

	if (ch->is_npc())
		return;

	if (IS_IMMORTAL(ch)) {
		String arg1, arg2, arg3;
		argument = one_argument(argument, arg1);
		argument = one_argument(argument, arg2);
		argument = one_argument(argument, arg3);

		if (!strcmp(arg1, "start")) {
			if (battle.issued) {
				stc("The battle begins!\n", ch);
				battle.start = true;
				Format::sprintf(buf, "[FYI] $n has closed the arena and battle has begun.\n");
				global_act(ch, buf, true, RED, COMM_QUIET | COMM_NOANNOUNCE);
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
				battle.start  = false;
				battle.issued = false;
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

				if (d->is_playing()
				    && !ach->is_npc()
				    && ach->in_room != nullptr
				    && ach->in_room->sector_type() == Sector::arena
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

		if ((location = Game::world().get_room(Location(Vnum(ROOM_VNUM_ARENACENTER)))) == nullptr) {
			stc("The arena is missing.\n", ch);
			return;
		}

		int count = location->area().num_players();
		if (count > 0) {
			ptc(ch, "You need to clear %d character from the arena first.\n", count);
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
		battle.issued = true;
		ptc(ch, "You open the arena to levels %d to %d, fee %d silver\n",
		    battle.low, battle.high, battle.fee);
		Format::sprintf(buf, "[FYI] $n has opened the arena to levels %d - %d for a %d silver fee.\n",
		        battle.low, battle.high, battle.fee);
		global_act(ch, buf, true, RED, COMM_QUIET | COMM_NOANNOUNCE);
	}
	else {
		if (char_in_duel(ch)) {
			stc("Deal with this battle first.\n", ch);
			return;
		}

		if (ch->in_room->sector_type() == Sector::arena) {
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

		if ((location = Game::world().get_room(Location(Vnum(ROOM_VNUM_ARENACENTER)))) == nullptr) {
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

	if (is_safe(ch, victim, true))
		return;

	if (victim->in_room->flags().has(ROOM_LAW)) {
		stc("The mayor does not approve of your playing style.\n", ch);
		return;
	}

	if (victim == ch) {
		stc("You are an excellant musician, Bravo!!\n", ch);
		return;
	}

	if (!victim->is_npc()) {
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

	if (!ch->is_npc() && ch->guild == Guild::bard)      /* bards */
		singchance += singchance / 3;

	singchance -= singchance * GET_DEFENSE_MOD(victim, DAM_CHARM) / 100;

	/*Moderate the final chance*/
	singchance = URANGE(0, singchance, (101 - (victim->level / 2)));

	/*Final calculation*/
	if (!roll_chance(singchance)) {
		/*Chance for something bad in the future, based on victim level, ch's CHR*/
		check_improve(ch, skill::type::sing, false, 8);
		return;
	}

	check_improve(ch, skill::type::sing, true, 8);

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
		false
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

	if (is_safe(ch, victim, true))
		return;

	if (victim->is_npc()
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

	if (ch->is_npc() && ch->act_flags.has(ACT_MORPH) && !victim->is_npc()) {
		stc("Morphed players cannot backstab PC's.\n", ch);
		wiznet("$N is attempting PK while morphed.", ch, nullptr, WIZ_CHEAT, 0, GET_RANK(ch));
		return;
	}

	if (ch->is_npc() && ch->act_flags.has(ACT_MORPH) && victim->act_flags.has(ACT_PET)) {
		stc("Morphed players cannot backstab pets.\n", ch);
		wiznet("$N is attempting to kill a pet while morphed.", ch, nullptr, WIZ_CHEAT, 0, GET_RANK(ch));
		return;
	}

	if (!deduct_stamina(ch, skill::type::backstab))
		return;

	if (ch->is_npc()) {
		mob_hit(ch, victim, skill::type::backstab);
		return;
	}

	check_killer(ch, victim);
	WAIT_STATE(ch, skill::lookup(skill::type::backstab).beats);

	if (number_percent() < get_skill_level(ch, skill::type::backstab)
	    || (get_skill_level(ch, skill::type::backstab) >= 2 && !IS_AWAKE(victim))) {
		check_improve(ch, skill::type::backstab, true, 4);
		multi_hit(ch, victim, skill::type::backstab);
		if (evo >=3 && roll_chance(50)) { /*vegita - 15% chance to put paralyze on target at evo 3*/
			//multi_hit(ch, victim, skill::type::backstab);
			stc("{YYour skillful blow strikes a nerve on your opponent!{x\n", ch);
			int level = (ch->level);
			affect::add_type_to_char(victim,
				affect::type::paralyze,
				level,
				2,
				1,
				false
			);
		}
	}
	else {
		check_improve(ch, skill::type::backstab, false, 4);
		damage(ch, victim, 0, skill::type::backstab, -1, DAM_NONE, true, false);
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
		check_improve(ch, skill::type::shadow_form, true, 1);
		multi_hit(ch, victim, skill::type::shadow_form);
	}
	else {
		stc("You enter shadow form, but your target avoids your strike.\n", ch);
		check_improve(ch, skill::type::shadow_form, false, 1);
		damage(ch, victim, 0, skill::type::shadow_form, -1, DAM_NONE, true, false);
	}

	affect::add_type_to_char(victim,
		affect::type::shadow_form,
		ch->level,
		-1,
		get_evolution(ch, skill::type::shadow_form),
		false
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
		check_improve(ch, skill::type::circle, true, 1);
		multi_hit(ch, victim, skill::type::circle);
	}
	else {
		stc("You circle your opponent, but your hasty strike misses.\n", ch);
		check_improve(ch, skill::type::circle, false, 1);
		damage(ch, victim, 0, skill::type::circle, -1, DAM_NONE, true, false);
	}
} /* end do_circle */

void do_flee(Character *ch, String argument)
{
	Exit *pexit;
	Room *was_in;
	Room *now_in;
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
		    || pexit->to_room == nullptr
		    || !can_see_room(ch, pexit->to_room)
		    || (pexit->exit_flags.has(EX_CLOSED)
		        && (!affect::exists_on_char(ch, affect::type::pass_door)
		            || pexit->exit_flags.has(EX_NOPASS)))
		    || (ch->is_npc() && pexit->to_room->flags().has(ROOM_NO_MOB)))
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
		    || pexit->to_room == nullptr
		    || !can_see_room(ch, pexit->to_room)
		    || (pexit->exit_flags.has(EX_CLOSED)
		        && (!affect::exists_on_char(ch, affect::type::pass_door)
		            || pexit->exit_flags.has(EX_NOPASS)))
		    || (ch->is_npc() && pexit->to_room->flags().has(ROOM_NO_MOB)))
			continue;

		if (!roll_chance(chance))
			continue;

		move_char(ch, dir, false);

		if ((now_in = ch->in_room) == was_in)
			continue;

		stop_fighting(ch, true);
		ch->in_room = was_in;
		act("$n has fled!", ch, nullptr, nullptr, TO_ROOM);
		ch->in_room = now_in;

		if (!ch->is_npc()) {
			act("You flee $T from combat!", ch, nullptr, Exit::dir_name(dir), TO_CHAR);

			if (ch->guild == Guild::thief)
				stc("You snuck away safely.\n", ch);
			else {
				if (ch->guild == Guild::paladin) { /* Paladins */
					stc("You lose 50 exp.\n", ch);
					gain_exp(ch, -50);
				}
				else {
					stc("You lose 10 exp.\n", ch);
					gain_exp(ch, -10);
				}
			}
		}

		if (victim->is_npc() && (victim->act_flags.has(ACT_AGGRESSIVE))) {
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
		check_improve(ch, skill::type::rescue, false, 1);
		return;
	}

	act("You rescue $N!",  ch, nullptr, victim, TO_CHAR);
	act("$n rescues you!", ch, nullptr, victim, TO_VICT);
	act("$n rescues $N!",  ch, nullptr, victim, TO_NOTVICT);
	check_improve(ch, skill::type::rescue, true, 1);
	stop_fighting(fch, false);
	stop_fighting(victim, false);
	stop_fighting(ch, false);
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

		damage(ch, victim, amount, skill::type::kick, -1, DAM_BASH, true, false);

		if (victim->is_garbage())
			return;

		check_improve(ch, skill::type::kick, true, 1);
		int evo = get_evolution(ch, skill::type::kick);

		if (evo >= 2) {
			if (get_position(victim) == POS_FIGHTING
			    && roll_chance((evo - 1) * 20 + 10)) { // evo 2: 30, evo 3, 50
				stc("You bring your foot around for a second hit.\n", ch);
				damage(ch, victim, amount, skill::type::roundhouse, -1, DAM_BASH, true, false);
				check_improve(ch, skill::type::kick, true, 1);
			}
		}

		if (victim->is_garbage())
			return;

		if (evo >= 3) {
			if (get_position(victim) == POS_FIGHTING
			    && !IS_FLYING(victim)
			    && roll_chance(30)) {
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
		damage(ch, victim, 0, skill::type::kick, -1, DAM_BASH, true, false);
		check_improve(ch, skill::type::kick, false, 1);
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
		damage(ch, victim, number_range(3, 3 * ch->level), skill::type::crush, -1, DAM_BASH, true, false);
		check_improve(ch, skill::type::crush, true, 1);
	}
	else {
		damage(ch, victim, 0, skill::type::crush, -1, DAM_BASH, true, false);
		check_improve(ch, skill::type::crush, false, 1);
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

	if (is_safe(ch, victim, true))
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

	damage(ch, victim, 0, skill::type::disarm, -1, DAM_BASH, false, false);
	WAIT_STATE(ch, skill::lookup(skill::type::disarm).beats);

	/* evo 1 talon give 60% save, 70% at 2, 80% at 3, 90% at 4 */
	if (affect::exists_on_char(victim, affect::type::talon)) {
		int talonchance = 75;

		switch (get_affect_evolution(victim, affect::type::talon)) {
		case 2: talonchance += 10;      break;

		case 3: talonchance += 25;      break;

		default:                        break;
		}

		if (roll_chance(talonchance)) {
			act("$N's vice-like grip prevents you from disarming $M!", ch, nullptr, victim, TO_CHAR);
			act("$N's vice-like grip prevents $M from being disarmed!", ch, nullptr, victim, TO_NOTVICT);
			act("Your vice-like grip prevents $n from disarming you!", ch, nullptr, victim, TO_VICT);
			check_improve(ch, skill::type::disarm, false, 1);
			return;
		}
	}

	/* noremove saves 100% at evo 1, 90% at 2, 80% at 3, 70% at 4 */
	if (IS_OBJ_STAT(weapon, ITEM_NOREMOVE)) {
		if (!roll_chance(10 * (evo - 1))) {
			act("$S weapon won't budge!", ch, nullptr, victim, TO_CHAR);
			act("$n tries to disarm you, but your weapon won't budge!", ch, nullptr, victim, TO_VICT);
			act("$n tries to disarm $N, but fails.", ch, nullptr, victim, TO_NOTVICT);
			check_improve(ch, skill::type::disarm, false, 1);
			return;
		}
	}

	// base disarm chance of 70% of disarm skill
	int chance = get_skill_level(ch, skill::type::disarm) * 7 / 10;

	/* find weapon skills */
	int ch_weapon_skill = get_weapon_learned(ch, get_weapon_skill(ch, false)); // your skill with your weapon
	int vict_weapon_skill = get_weapon_learned(victim, get_weapon_skill(victim, false)); // victim's skill
	int ch_vict_weapon_skill = get_weapon_learned(ch, get_weapon_skill(victim, false)); // your skill with victim's weapon

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
	if (roll_chance(chance)) {
		Room *next_room = nullptr;
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
		    || victim->in_room->sector_type() == Sector::arena
		    || victim->in_room->sector_type() == Sector::clanarena)
			obj_to_char(weapon, victim);
		else {
			/* knock the weapon into the next room! */
			if ((evo == 2 && roll_chance(5))
			    || (evo == 3 && roll_chance(15))
			    || (evo == 4 && roll_chance(25))) {
				door = number_range(0, 5);

				if ((pexit = victim->in_room->exit[door]) != 0
				    && (next_room = pexit->to_room) != nullptr
				    && can_see_room(victim, next_room)) {
					if (pexit->exit_flags.has(EX_CLOSED)) {
						Format::sprintf(buf, "$p slams against the $d and clatters to the %s!",
						        victim->in_room->sector_type() == Sector::inside ? "floor" : "ground");
						act(buf, ch, weapon, pexit->keyword(), TO_CHAR);
						act(buf, ch, weapon, pexit->keyword(), TO_ROOM);
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
							        next_room->sector_type() == Sector::inside ? "floor" : "ground");
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
						        victim->in_room->sector_type() == Sector::inside ? "floor" : "ground");
					else if (door < 5)
						Format::sprintf(buf, "$p clatters to the %s.",
						        victim->in_room->sector_type() == Sector::inside ? "floor" : "ground");
					else {
						if (victim->in_room->sector_type() == Sector::inside)
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
			if (victim->is_npc() && victim->wait == 0 && can_see_obj(victim, weapon)) {
				get_obj(victim, weapon, nullptr);
				wear_obj(victim, weapon, false);   /* added here...will remove in mob_update */
			}
		}

		check_improve(ch, skill::type::disarm, true, 1);
	}
	else {
		WAIT_STATE(ch, skill::lookup(skill::type::disarm).beats);
		act("You fail to disarm $N.", ch, nullptr, victim, TO_CHAR);
		act("$n tries to disarm you, but fails.", ch, nullptr, victim, TO_VICT);
		act("$n tries to disarm $N, but fails.", ch, nullptr, victim, TO_NOTVICT);
		check_improve(ch, skill::type::disarm, false, 1);
	}

	if (is_blinded(ch) && (blind_fight_skill > 0))
		check_improve(ch, skill::type::blind_fight, false, 1);
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
	if (!victim->is_npc()) {
		Format::sprintf(buf, "%s has been slain by %s.", victim->name, (ch->is_npc() ? ch->short_descr : ch->name));
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

	if (is_safe(ch, victim, true))
		return;

	if (!deduct_stamina(ch, skill::type::rotate))
		return;

	check_killer(ch, victim);
	WAIT_STATE(ch, skill::lookup(skill::type::rotate).beats);

	if (number_percent() < get_skill_level(ch, skill::type::rotate)) {
		stc("You deftly shift the focus of your blows.\n", ch);
		check_improve(ch, skill::type::rotate, true, 4);
		ch->fighting = victim;
	}
	else {
		check_improve(ch, skill::type::rotate, false, 4);
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
		check_improve(ch, skill::type::hammerstrike, true, 2);

		affect::add_type_to_char(ch,
			affect::type::hammerstrike,
			ch->level,
			number_fuzzy(ch->level/15),
			get_evolution(ch, skill::type::hammerstrike),
			false
		);
	}
	else {
		WAIT_STATE(ch, 3 * PULSE_VIOLENCE);
		ch->stam = ch->stam / 3;
		stc("You call for power from the gods, but you get no answer.\n", ch);
		check_improve(ch, skill::type::hammerstrike, false, 2);
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

	if (!ch->fighting->is_npc() && IS_IMMORTAL(ch->fighting)) {
		stc("You fail miserably.\n", ch);
		return;
	}

	if (number_percent() > get_skill_level(ch, skill::type::critical_blow)) {
		check_improve(ch, skill::type::critical_blow, false, 2);
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
		check_improve(ch, skill::type::critical_blow, true, 2);
	}
	else {
		ptc(ch, "You try to give %s the blow of death, but you fail.\n",
		    ch->fighting->short_descr);
		check_improve(ch, skill::type::critical_blow, false, 2);

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

	one_hit(ch, ch->fighting, skill::type::critical_blow, false);
} /* end do_critical_blow */

/* Riposte, originally by Elrac */
void do_riposte(Character *victim, Character *ch)
{
	int chance = (get_skill_level(victim, skill::type::riposte));

	if (!chance)
		return;

	chance += (victim->level - ch->level);

	if (number_percent() > chance) {
		check_improve(victim, skill::type::riposte, false, 6);
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
	one_hit(victim, ch, skill::type::riposte, false);
	check_improve(victim, skill::type::riposte, true, 6);
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

	if (ch->in_room->flags().has(ROOM_SAFE) && !IS_IMMORTAL(ch)) {
		stc("Oddly enough, in this room you feel peaceful.", ch);
		return;
	}

	if (!deduct_stamina(ch, skill::type::rage))
		return;

	WAIT_STATE(ch, skill::lookup(skill::type::rage).beats);

	if (number_percent() > get_skill_level(ch, skill::type::rage)) {
		stc("You scream a battlecry but fail to unleash your inner rage.\n", ch);
		act("$n starts into a wild series of attacks, but $s timing is off.", ch, nullptr, nullptr, TO_ROOM);
		check_improve(ch, skill::type::rage, false, 2);
		return;
	}

	/* Let's dance */
	act("You scream a battle cry and unleash your rage!", ch, nullptr, nullptr, TO_CHAR);
	act("$n screams a battle cry, and goes into a wild series of attacks!", ch, nullptr, nullptr, TO_ROOM);

	for (vch = ch->in_room->people; vch != nullptr; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (ch == vch)
			continue;

		if (is_same_group(ch, vch))
			continue;

		if (is_safe_spell(ch, vch, true))
			continue;

		check_killer(ch, vch);
		multi_hit(ch, vch, skill::type::rage);

		if (++pplhit > 4)
			break;
	}

	check_improve(ch, skill::type::rage, true, 2);
}

void do_lay_on_hands(Character *ch, String argument)
{
	int heal, skill;
	Character *victim;

	if (ch->is_npc())
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
	if ( victim->is_npc() )
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
	victim->hit = std::min(victim->hit + heal, GET_MAX_HIT(victim));
	update_pos(victim);
	stc("You feel better.\n", victim);
	stc("Your hands glow softly as a sense of divine power travels through you.\n", ch);
	check_improve(ch, skill::type::lay_on_hands, true, 1);
	return;
}

void do_shoot(Character *ch, String argument)
{
	Character *victim = nullptr;

	if (ch->is_npc())
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
		Room *target_room = nullptr;
		int distance = MAX_BOW_DISTANCE;
		bool nearest = false;

		// north = 1, north; 1.north = 1, north; 2.north = 2, north; etc
		if (isdigit(dir_arg[0]))
			distance = std::min(number_argument(dir_arg, dir_str), MAX_BOW_DISTANCE);
		else {
			nearest = true;
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

			Room *room = ch->in_room;
			Room *to_room;
			Exit *pexit;

			if (nearest) {
				// find the nearest room with that target.  separated from below search code
				// because of different messages to the archer
				for (int i = 0; i < MAX_BOW_DISTANCE; i++) {
					if ((pexit = room->exit[dir]) == nullptr
			         || (to_room = pexit->to_room) == nullptr
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
			         || (to_room = pexit->to_room) == nullptr
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
	Room *old_room = ch->in_room;
	bool was_superwiz = ch->act_flags.has(PLR_SUPERWIZ);

	if (old_room != victim->in_room) {
		char_from_room(ch);
		char_to_room(ch, victim->in_room);
		ch->act_flags += PLR_SUPERWIZ;
	}

	// do the hit
	one_hit(ch, victim, skill::type::unknown, false);

	// move them back
	if (old_room != victim->in_room) {
		char_from_room(ch);
		char_to_room(ch, old_room);

		if (!was_superwiz)
			ch->act_flags -= PLR_SUPERWIZ;

		/* if the target is NPC, then make it hunt the shooter */
		if (victim->is_npc()) {
			victim->hunting = ch;
			hunt_victim(victim);
		}
	}

	WAIT_STATE(ch, skill::lookup(skill::type::backstab).beats);
	check_improve(ch, skill::type::archery, true, 5); /* change added for gains on shooting now damnit leave it*/
}   /* end of do bow */
