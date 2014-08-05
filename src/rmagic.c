/* remort magic */

#include "merc.h"
#include "magic.h"

/* needed functions */
void    wear_obj        args((CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace));

extern bool     global_quick;

/*** MAGE ***/

void spell_sheen(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (get_affect(victim->affected, sn)) {
		stc("Your armor is already coated with magical steel.\n", ch);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level;
	af.modifier  = 0;
	af.bitvector = 0;
	af.location  = APPLY_SHEEN;
	af.evolution = evolution;
	affect_to_char(victim, &af);
	stc("A protective sheen covers your armor.\n", victim);
}

void spell_focus(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (get_affect(victim->affected, sn)) {
		stc("Your spells are already focused.\n", ch);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 3;
	af.modifier  = 0;
	af.location  = APPLY_FOCUS;
	af.bitvector = 0;
	af.evolution = evolution;
	affect_to_char(victim, &af);
	stc("You focus on your magic -- you feel more deadly!\n", victim);
}

void spell_paralyze(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (ch == victim) {
		stc("That wouldn't be very smart now, would it?\n", ch);
		return;
	}

	if (get_affect(victim->affected, sn)) {
		act("$N is already paralyzed.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (!IS_NPC(victim) && saves_spell(level, victim, DAM_MENTAL)) {
		act("$N slows, but only momentarily.  Your paralysis has failed.", ch, NULL, victim, TO_CHAR);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = ch->level;
	af.duration  = level / 20;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = 0;
	af.evolution = evolution;
	affect_to_char(victim, &af);
	stc("You can't move anymore!\n", victim);
	act("$n seems paralyzed!", victim, NULL, NULL, TO_ROOM);
}

void spell_ironskin(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (get_affect(victim->affected, sn)) {
		stc("Your skin is already hard as iron.\n", ch);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level;
	af.modifier  = -100;
	af.location  = APPLY_AC;
	af.bitvector = 0;
	af.evolution = evolution;
	affect_to_char(victim, &af);
	stc("Your skin takes on the consistency of iron.\n", victim);
}

/*** CLERIC ***/

void spell_barrier(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (get_affect(victim->affected, sn)) {
		stc("You are already surrounded by a barrier.\n", ch);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 5;
	af.modifier  = 0;
	af.location  = APPLY_BARRIER;
	af.bitvector = 0;
	af.evolution = evolution;
	affect_to_char(victim, &af);
	stc("You are surrounded by a protective barrier.\n", victim);
}

/* Dazzle by Montrey */
void spell_dazzle(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int chance;

	if (ch == victim && ch->fighting != NULL)
		victim = ch->fighting;

	if (IS_AFFECTED(victim, AFF_BLIND)) {
		stc("They can't see, what good would it do?\n", ch);
		return;
	}

	if (ch == victim) {
		stc("And just who are you trying to dazzle?\n", ch);
		return;
	}

	/* basic chances, works better on mobs than normal saves */
	chance = 70 - (victim->level - level) * 2 + victim->saving_throw;

	/* berserking isn't as good as normal saves */
	if (IS_AFFECTED(victim, AFF_BERSERK))
		chance -= victim->level / 4;

	/* better chance if it's dark out */
	if ((!ch->in_room->sector_type == SECT_INSIDE)
	    && (!ch->in_room->sector_type == SECT_CITY)) {
		if (IS_SET(ch->in_room->room_flags, ROOM_DARK))
			chance += 25;
		else if (weather_info.sunlight == SUN_DARK)
			chance += 20;
		else if (weather_info.sunlight == SUN_SET)
			chance += 15;
	}

	switch (check_immune(victim, DAM_LIGHT)) {
	case IS_IMMUNE:         chance = 0;        break;

	case IS_RESISTANT:      chance -= 25;      break;

	case IS_VULNERABLE:     chance += 25;      break;
	}

	/* Oh god, my eyes!! */
	if (number_percent() < chance) {
		act("A brilliant light flashes from $n's fingertips, and $s blinded opponent stops fighting.",
		    ch, NULL, victim, TO_NOTVICT);
		act("Your flash dazzles $N's eyes, and the fight stops.", ch, NULL, victim, TO_CHAR);
		act("A brilliant flash of light dazzles your eyes, and the fighting stops.",
		    ch, NULL, victim, TO_VICT);
		af.where        = TO_AFFECTS;
		af.type         = gsn_dazzle;
		af.level        = ch->level;
		af.duration     = 0;
		af.location     = APPLY_HITROLL;
		af.modifier     = 0;
		af.bitvector    = AFF_BLIND;
		af.evolution = evolution;
		affect_to_char(victim, &af);
		stop_fighting(ch, TRUE);
		return;
	}

	act("A brilliant flash of light bursts from $n's fingertips, but nothing happens.",
	    ch, NULL, victim, TO_NOTVICT);
	act("Your flash of light has no effect.", ch, NULL, victim, TO_CHAR);
	act("A brilliant flash of light bursts from $n's fingertips, but you close your eyes in time.",
	    ch, NULL, victim, TO_VICT);
	return;
}

/* Full Heal by Montrey */
void spell_full_heal(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int mana, mana_cost;
	mana = get_skill_cost(ch, sn);
	mana_cost = (ch->max_hit / 4) - mana;

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
			act("$N cannot be fully healed while in combat.", ch, NULL, victim, TO_CHAR);

		return;
	}

	if (!IS_NPC(ch))
		if (ch->pcdata->pktimer > 0) {
			stc("You can't cast this so soon after combat.\n", ch);
			return;
		}

	victim->hit = victim->max_hit;
	ch->mana -= mana_cost;
	act("$n looks revived as $s wounds vanish completely.", victim, NULL, NULL, TO_ROOM);
	stc("Your wounds vanish completely.\n", victim);
}

/*** THIEF ***/

void spell_midnight(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (get_affect(victim->affected, sn) && (victim == ch)) {
		stc("You fail to invade the shadows further.\n", ch);
		return;
	}

	stc("You blend into the night.\n", victim);
	act("$n vanishes into the shadows.", victim, NULL, NULL, TO_ROOM);
	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 2;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = 0;
	af.evolution = evolution;
	affect_to_char(victim, &af);
}

/*** NECRO ***/

void spell_sap(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam, mult;
	dam = dice(level, 22);
	mult = ((100 - (((ch->hit * 100) / ch->max_hit) * 2)) * 2);
	dam = UMAX(dam, dam + ((dam * mult) / 100));

	if (ch->hit < 31000)
		ch->hit += (dam / 10);

	if (dam != 0) {
		stc("You feel frail and weak.\n", victim);
		stc("You are instantly revitalized!\n", ch);
	}

	damage(ch, victim, dam, sn, DAM_NEGATIVE, TRUE, TRUE);
}

/* Pain by Montrey */
void spell_pain(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (ch == victim) {
		stc("Despite your masochistic tendencies, you can't do that.\n", ch);
		return;
	}

	act("You bestow pure agony upon $N!", ch, NULL, victim, TO_CHAR);
	act("$N writhes in agony as the pain of $S wounds overtakes $M.", ch, NULL, victim, TO_NOTVICT);
	act("You scream in agony as the pain of your wounds increases.", ch, NULL, victim, TO_VICT);
	dam = (UMIN(victim->hit, (4 * level))) + number_range(1, (level * 2));
	damage(ch, (CHAR_DATA *) vo, dam, sn, DAM_HARM, TRUE, TRUE);

	if (ch->fighting != NULL) {
		spell_slow(gsn_slow,   level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
		spell_weaken(gsn_weaken, level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
		spell_plague(gsn_plague, level, ch, (void *) victim, TARGET_CHAR, get_evolution(ch, sn));
	}

	return;
}

/* Hex by Montrey */
void spell_hex(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (ch == victim) {
		stc("Mortal fear of the dark gods prevents you from hexing yourself.\n", ch);
		return;
	}

	if (get_affect(victim->affected, sn)) {
		act("The dark gods have already cursed $N.", ch, NULL, victim, TO_CHAR);
		return;
	}

	act("Unholy darkness surrounds $N, guiding your attacks.", ch, NULL, victim, TO_CHAR);
	act("$N is surrounded by a dark mist.", ch, NULL, victim, TO_NOTVICT);
	act("An unholy mist surrounds you.", ch, NULL, victim, TO_VICT);
	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 30;
	af.location  = APPLY_AC;
	af.modifier  = 3 * level;
	af.bitvector = 0;
	af.evolution = evolution;
	affect_to_char(victim, &af);
}

/* Bone Wall */
void spell_bone_wall(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	AFFECT_DATA af;
	af.where        = TO_AFFECTS;
	af.type         = sn;
	af.level        = level;
	af.duration     = level;
	af.location     = APPLY_NONE;
	af.modifier     = 0;
	af.bitvector    = 0;
	af.evolution    = evolution;
	affect_to_char(ch, &af);
	stc("Bones lift from the ground and begin to swirl around you.\n", ch);
	act("Bones lift from the ground and begin to swirl around $n.", ch, NULL, NULL, TO_ROOM);
}

/*** PALADIN ***/

void spell_force(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (get_affect(victim->affected, sn)) {
		stc("You are already protected by the force.\n", ch);
		return;
	}

	act("$n is surrounded by a mystical aura.", victim, NULL, NULL, TO_ROOM);
	stc("You are surrounded by a mystical aura.\n", victim);
	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 2;
	af.modifier  = 0;
	af.location  = APPLY_AC;
	af.bitvector = 0;
	af.evolution = evolution;
	affect_to_char(victim, &af);
}

/* Holy Sword by Montrey */
void spell_holy_sword(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	OBJ_DATA *sword, *wielded;
	AFFECT_DATA *saf;

	if (!IS_IMMORTAL(ch)) {
		if (ch->alignment < 1000 && ch->alignment > -1000) {
			stc("Impurity in your heart prevents summoning a godly blade.\n", ch);
			return;
		}
	}

	wielded = get_eq_char(ch, WEAR_WIELD);

	if (wielded != NULL && IS_OBJ_STAT(wielded, ITEM_NOREMOVE)) {
		act("You can't remove $p.", ch, wielded, NULL, TO_CHAR);
		return;
	}

	/* makes a sword, Xd12 damage, adds shocking at 20+, vorpal at 45+, flaming at 70+ */
	sword = create_object(get_obj_index(OBJ_VNUM_HOLYSWORD), 0);

	if (! sword) {
		bug("Memory error while creating holy sword.", 0);
		stc("Unable to create a holy sword.\n", ch);
		return;
	}

	sword->value[1]         = level / 8 + 1;
	sword->timer            = level * 2 - number_range(0, level / 2);
	sword->level            = level;
	sword->enchanted        = TRUE;
	sword->extra_flags      |= ITEM_INVENTORY;    /* so it vapes on death */

	if (sword->level >= 20)
		sword->value[4] |= WEAPON_SHOCKING;

	if (sword->level >= 45)
		sword->value[4] |= WEAPON_VORPAL;

	if (sword->level >= 70)
		sword->value[4] |= WEAPON_FLAMING;

	for (saf = sword->affected; saf != NULL; saf = saf->next) {
		if (saf->location == APPLY_HITROLL)
			saf->modifier = level / 10 + 1;

		if (saf->location == APPLY_DAMROLL)
			saf->modifier = level / 10 + 1;
	}

	if (ch->alignment >= 1) {
		sword->short_descr = str_dup("{Wa Holy Avenger{x");
		SET_BIT(sword->extra_flags, ITEM_BLESS);
		SET_BIT(sword->extra_flags, ITEM_ANTI_EVIL);
	}
	else {
		sword->short_descr = str_dup("{can Unholy Avenger{x");
		SET_BIT(sword->extra_flags, ITEM_EVIL);
		SET_BIT(sword->extra_flags, ITEM_ANTI_GOOD);
	}

	act("$n prays for a moment, and a holy sword materializes in $s hand.", ch, NULL, NULL, TO_ROOM);
	stc("You summon a godly blade for your divine justice.\n", ch);
	obj_to_char(sword, ch);
	wear_obj(ch, sword, TRUE);
	ch->mana = 0;
	return;
}

/*** BARD ***/

/* Quick */
void spell_quick(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	/* how simple could it be? */
	act("$n blazes into a wild flurry of attacks!", ch, NULL, NULL, TO_ROOM);
	stc("You blaze into a wild flurry of attacks!\n", ch);
	global_quick = TRUE;
	multi_hit(ch, ch->fighting, TYPE_UNDEFINED);
	global_quick = FALSE;
}
