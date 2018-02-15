/* remort magic */

#include "act.hh"
#include "affect/Affect.hh"
#include "Area.hh"
#include "Character.hh"
#include "declare.hh"
#include "dispel.hh"
#include "Flags.hh"
#include "GameTime.hh"
#include "Logging.hh"
#include "macros.hh"
#include "magic.hh"
#include "merc.hh"
#include "Object.hh"
#include "ObjectValue.hh"
#include "Player.hh"
#include "random.hh"
#include "RoomPrototype.hh"
#include "String.hh"
#include "World.hh"

/* needed functions */
void    wear_obj        args((Character *ch, Object *obj, bool fReplace));

extern bool     global_quick;

/*** MAGE ***/

void spell_sheen(skill::Type sn, int level, Character *ch, void *vo, int target, int evolution)
{
	Character *victim = (Character *) vo;

	if (affect::exists_on_char(victim, affect::sheen)) {
		stc("Your armor is already coated with magical steel.\n", ch);
		return;
	}

	affect::add_type_to_char(victim,
		affect::sheen,
		level,
		level,
		evolution,
		FALSE
	);

	stc("A protective sheen covers your armor.\n", victim);
}

void spell_focus(skill::Type sn, int level, Character *ch, void *vo, int target, int evolution)
{
	Character *victim = (Character *) vo;

	if (affect::exists_on_char(victim, affect::focus)) {
		stc("Your spells are already focused.\n", ch);
		return;
	}

	affect::add_type_to_char(victim,
		affect::focus,
		level,
		level / 3,
		evolution,
		FALSE
	);

	stc("You focus on your magic -- you feel more deadly!\n", victim);
}

void spell_paralyze(skill::Type sn, int level, Character *ch, void *vo, int target, int evolution)
{
	Character *victim = (Character *) vo;

	if (ch == victim) {
		stc("That wouldn't be very smart now, would it?\n", ch);
		return;
	}

	if (affect::exists_on_char(victim, affect::paralyze)) {
		act("$N is already paralyzed.", ch, nullptr, victim, TO_CHAR);
		return;
	}

	if (!IS_NPC(victim) && saves_spell(level, victim, DAM_MENTAL)) {
		act("$N slows, but only momentarily.  Your paralysis has failed.", ch, nullptr, victim, TO_CHAR);
		return;
	}

	affect::add_type_to_char(victim,
		affect::paralyze,
		level,
		level / 20,
		evolution,
		FALSE
	);

	stc("You can't move anymore!\n", victim);
	act("$n seems paralyzed!", victim, nullptr, nullptr, TO_ROOM);
}

void spell_ironskin(skill::Type sn, int level, Character *ch, void *vo, int target, int evolution)
{
	Character *victim = (Character *) vo;

	if (affect::exists_on_char(victim, affect::ironskin)) {
		stc("Your skin is already hard as iron.\n", ch);
		return;
	}

	affect::add_type_to_char(victim,
		affect::ironskin,
		level,
		level,
		evolution,
		FALSE
	);

	stc("Your skin takes on the consistency of iron.\n", victim);
}

/*** CLERIC ***/

void spell_barrier(skill::Type sn, int level, Character *ch, void *vo, int target, int evolution)
{
	Character *victim = (Character *) vo;

	if (affect::exists_on_char(victim, affect::barrier)) {
		stc("You are already surrounded by a barrier.\n", ch);
		return;
	}

	affect::add_type_to_char(victim,
		affect::barrier,
		level,
		level / 5,
		evolution,
		FALSE
	);

	stc("You are surrounded by a protective barrier.\n", victim);
}

/* Dazzle by Montrey */
void spell_dazzle(skill::Type sn, int level, Character *ch, void *vo, int target, int evolution)
{
	Character *victim = (Character *) vo;
	int chance;

	if (ch == victim && ch->fighting != nullptr)
		victim = ch->fighting;

	if (is_blinded(victim)) {
		stc("They can't see, what good would it do?\n", ch);
		return;
	}

	if (ch == victim) {
		stc("And just who are you trying to dazzle?\n", ch);
		return;
	}

	/* basic chances, works better on mobs than normal saves */
	chance = 70 - (victim->level - level) * 2 + GET_ATTR_SAVES(victim);

	/* berserking isn't as good as normal saves */
	if (affect::exists_on_char(victim, affect::berserk))
		chance -= victim->level / 4;

	/* better chance if it's dark out */
	if (ch->in_room->sector_type != SECT_INSIDE
	    && ch->in_room->sector_type != SECT_CITY) {
		if (GET_ROOM_FLAGS(ch->in_room).has(ROOM_DARK))
			chance += 25;
		else if (ch->in_room->area->world.time.sunlight == GameTime::Night)
			chance += 20;
		else if (ch->in_room->area->world.time.sunlight == GameTime::Sunset)
			chance += 15;
	}

	int def = GET_DEFENSE_MOD(victim, DAM_LIGHT);

	if (def >= 100)
		chance = 0;
	else
		chance -= 25 * def / 100; // negative def is vuln

	/* Oh god, my eyes!! */
	if (number_percent() < chance) {
		act("A brilliant light flashes from $n's fingertips, and $s blinded opponent stops fighting.",
		    ch, nullptr, victim, TO_NOTVICT);
		act("Your flash dazzles $N's eyes, and the fight stops.", ch, nullptr, victim, TO_CHAR);
		act("A brilliant flash of light dazzles your eyes, and the fighting stops.",
		    ch, nullptr, victim, TO_VICT);

		affect::add_type_to_char(victim,
			affect::dazzle,
			level,
			0,
			evolution,
			FALSE
		);

		stop_fighting(ch, TRUE);
		return;
	}

	act("A brilliant flash of light bursts from $n's fingertips, but nothing happens.",
	    ch, nullptr, victim, TO_NOTVICT);
	act("Your flash of light has no effect.", ch, nullptr, victim, TO_CHAR);
	act("A brilliant flash of light bursts from $n's fingertips, but you close your eyes in time.",
	    ch, nullptr, victim, TO_VICT);
	return;
}

/* Full Heal by Montrey */
void spell_full_heal(skill::Type sn, int level, Character *ch, void *vo, int target, int evolution)
{
	Character *victim = (Character *) vo;
	int mana, mana_cost;
	mana = get_skill_cost(ch, sn);
	mana_cost = (GET_MAX_HIT(ch) / 4) - mana;

	if (HAS_RAFF(ch, RAFF_COSTLYSPELLS))
		mana_cost += mana_cost / 5;

	if (HAS_RAFF(ch, RAFF_CHEAPSPELLS))
		mana_cost -= mana_cost / 5;

	if (ch->mana < mana_cost) {
		stc("You don't have enough mana.\n", ch);
		ch->mana += mana;
		return;
	}

	if (victim->fighting) {
		if (victim == ch)
			stc("The fury of battle prevents a full healing.\n", ch);
		else
			act("$N cannot be fully healed while in combat.", ch, nullptr, victim, TO_CHAR);

		return;
	}

	if (!IS_NPC(ch))
		if (ch->pcdata->pktimer > 0) {
			stc("You can't cast this so soon after combat.\n", ch);
			return;
		}

	victim->hit = GET_MAX_HIT(victim);
	ch->mana -= mana_cost;
	act("$n looks revived as $s wounds vanish completely.", victim, nullptr, nullptr, TO_ROOM);
	stc("Your wounds vanish completely.\n", victim);
}

/*** THIEF ***/

void spell_midnight(skill::Type sn, int level, Character *ch, void *vo, int target, int evolution)
{
	Character *victim = (Character *) vo;

	if (affect::exists_on_char(victim, affect::midnight) && (victim == ch)) {
		stc("You fail to invade the shadows further.\n", ch);
		return;
	}

	stc("You blend into the night.\n", victim);
	act("$n vanishes into the shadows.", victim, nullptr, nullptr, TO_ROOM);

	affect::add_type_to_char(victim,
		affect::midnight,
		level,
		2,
		evolution,
		FALSE
	);
}

/*** NECRO ***/

void spell_sap(skill::Type sn, int level, Character *ch, void *vo, int target, int evolution)
{
	Character *victim = (Character *) vo;
	int dam, mult;
	dam = dice(level, 22);
	mult = ((100 - (((ch->hit * 100) / GET_MAX_HIT(ch)) * 2)) * 2);
	dam = UMAX(dam, dam + ((dam * mult) / 100));

	if (ch->hit < 31000) {
		int def_mod = GET_DEFENSE_MOD(victim, DAM_NEGATIVE);

		if (def_mod < 100) {
			int gain = dam;
			gain -= gain * def_mod / 100;
			ch->hit += (gain / 10);
		}
	}

	if (dam != 0) {
		stc("You feel frail and weak.\n", victim);
		stc("You are instantly revitalized!\n", ch);
	}

	damage(ch, victim, dam, sn, DAM_NEGATIVE, TRUE, TRUE);
}

/* Pain by Montrey */
void spell_pain(skill::Type sn, int level, Character *ch, void *vo, int target, int evolution)
{
	Character *victim = (Character *) vo;
	int dam;

	if (ch == victim) {
		stc("Despite your masochistic tendencies, you can't do that.\n", ch);
		return;
	}

	act("You bestow pure agony upon $N!", ch, nullptr, victim, TO_CHAR);
	act("$N writhes in agony as the pain of $S wounds overtakes $M.", ch, nullptr, victim, TO_NOTVICT);
	act("You scream in agony as the pain of your wounds increases.", ch, nullptr, victim, TO_VICT);
	dam = (UMIN(victim->hit, (4 * level))) + number_range(1, (level * 2));
	damage(ch, (Character *) vo, dam, sn, DAM_HARM, TRUE, TRUE);

	if (ch->fighting != nullptr) {
		spell_slow(skill::slow,   level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
		spell_weaken(skill::weaken, level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
		spell_plague(skill::plague, level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
	}

	return;
}

/* Hex by Montrey */
void spell_hex(skill::Type sn, int level, Character *ch, void *vo, int target, int evolution)
{
	Character *victim = (Character *) vo;

	if (ch == victim) {
		stc("Mortal fear of the dark gods prevents you from hexing yourself.\n", ch);
		return;
	}

	if (affect::exists_on_char(victim, affect::hex)) {
		act("The dark gods have already cursed $N.", ch, nullptr, victim, TO_CHAR);
		return;
	}

	act("Unholy darkness surrounds $N, guiding your attacks.", ch, nullptr, victim, TO_CHAR);
	act("$N is surrounded by a dark mist.", ch, nullptr, victim, TO_NOTVICT);
	act("An unholy mist surrounds you.", ch, nullptr, victim, TO_VICT);

	affect::add_type_to_char(victim,
		affect::hex,
		level,
		level / 30,
		evolution,
		FALSE
	);
}

/* Bone Wall */
void spell_bone_wall(skill::Type sn, int level, Character *ch, void *vo, int target, int evolution)
{
	Character *victim = (Character *) vo;

	affect::add_type_to_char(victim,
		affect::bone_wall,
		level,
		level,
		evolution,
		FALSE
	);

	stc("Bones lift from the ground and begin to swirl around you.\n", ch);
	act("Bones lift from the ground and begin to swirl around $n.", ch, nullptr, nullptr, TO_ROOM);
}

/*** PALADIN ***/

void spell_force(skill::Type sn, int level, Character *ch, void *vo, int target, int evolution)
{
	Character *victim = (Character *) vo;

	if (affect::exists_on_char(victim, affect::force_shield)) {
		stc("You are already protected by the force.\n", ch);
		return;
	}

	act("$n is surrounded by a mystical aura.", victim, nullptr, nullptr, TO_ROOM);
	stc("You are surrounded by a mystical aura.\n", victim);

	affect::add_type_to_char(victim,
		affect::force_shield,
		level,
		2,
		evolution,
		FALSE
	);
}

/* Holy Sword by Montrey */
void spell_holy_sword(skill::Type sn, int level, Character *ch, void *vo, int target, int evolution)
{
	Object *sword, *wielded;

	if (!IS_IMMORTAL(ch)) {
		if (ch->alignment < 1000 && ch->alignment > -1000) {
			stc("Impurity in your heart prevents summoning a godly blade.\n", ch);
			return;
		}
	}

	wielded = get_eq_char(ch, WEAR_WIELD);

	if (wielded != nullptr && IS_OBJ_STAT(wielded, ITEM_NOREMOVE)) {
		act("You can't remove $p.", ch, wielded, nullptr, TO_CHAR);
		return;
	}

	/* makes a sword, Xd12 damage, adds shocking at 20+, vorpal at 45+, flaming at 70+ */
	sword = create_object(get_obj_index(OBJ_VNUM_HOLYSWORD), 0);

	if (! sword) {
		Logging::bug("Memory error while creating holy sword.", 0);
		stc("Unable to create a holy sword.\n", ch);
		return;
	}

	sword->value[1]         = level / 8 + 1;
	sword->timer            = level * 2 - number_range(0, level / 2);
	sword->level            = level;
	sword->extra_flags += ITEM_INVENTORY;    /* so it vapes on death */

	if (sword->level >= 20)
		sword->value[4] += WEAPON_SHOCKING;

	if (sword->level >= 45)
		sword->value[4] += WEAPON_VORPAL;

	if (sword->level >= 70)
		sword->value[4] += WEAPON_FLAMING;

	affect::Affect af;
	af.where      = TO_OBJECT;
	af.type       = affect::enchant_weapon;
	af.level      = level;
	af.duration   = -1;
	af.modifier   = level / 10 + 1;
	af.bitvector(0);
	af.evolution  = evolution;

	af.location   = APPLY_HITROLL;
	affect::join_to_obj(sword, &af);

	af.location   = APPLY_DAMROLL;
	affect::join_to_obj(sword, &af);

	if (ch->alignment >= 1) {
		sword->short_descr = "{Wa Holy Avenger{x";
		sword->extra_flags += ITEM_BLESS;
		sword->extra_flags += ITEM_ANTI_EVIL;
	}
	else {
		sword->short_descr = "{can Unholy Avenger{x";
		sword->extra_flags += ITEM_EVIL;
		sword->extra_flags += ITEM_ANTI_GOOD;
	}

	act("$n prays for a moment, and a holy sword materializes in $s hand.", ch, nullptr, nullptr, TO_ROOM);
	stc("You summon a godly blade for your divine justice.\n", ch);
	obj_to_char(sword, ch);
	wear_obj(ch, sword, TRUE);
	ch->mana = 0;
	return;
}

/*** BARD ***/

/* Quick */
void spell_quick(skill::Type sn, int level, Character *ch, void *vo, int target, int evolution)
{
	/* how simple could it be? */
	act("$n blazes into a wild flurry of attacks!", ch, nullptr, nullptr, TO_ROOM);
	stc("You blaze into a wild flurry of attacks!\n", ch);
	global_quick = TRUE;
	multi_hit(ch, ch->fighting, TYPE_UNDEFINED);
	global_quick = FALSE;
}
