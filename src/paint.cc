#include "act.hh"
#include "affect/Affect.hh"
#include "channels.hh"
#include "Character.hh"
#include "declare.hh"
#include "find.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "interp.hh"
#include "macros.hh"
#include "memory.hh"
#include "merc.hh"
#include "Object.hh"
#include "ObjectValue.hh"
#include "Player.hh"
#include "random.hh"
#include "Room.hh"
#include "String.hh"

void do_paintbow(Character *ch, String argument)
{
	if (IS_NPC(ch)) {
		stc("Mobiles don't like to play paintball =).\n", ch);
		return;
	}

	if (ch->pcdata->plr_flags.has(PLR_PAINT)) {
		stc("You pull out the white flag.\n", ch);
		ch->pcdata->plr_flags -= PLR_PAINT;
		wiznet("$N is now in *NPB* mode.", ch, nullptr, WIZ_MISC, 0, 0);
	}
	else {
		stc("You prepare for combat.\n", ch);
		ch->pcdata->plr_flags += PLR_PAINT;
		wiznet("$N is now in *PB* mode.", ch, nullptr, WIZ_MISC, 0, 0);
	}
} /* end do_paintball() */

/* Paintball code by Lotus */
void do_reload(Character *ch)
{
	char buf[MAX_STRING_LENGTH];
	Object *tube;
	Object *gun;

	if ((tube = get_obj_carry(ch, "tube")) == nullptr) {
		stc("You don't have any paint tubes!\n", ch);
		return;
	}

	if (tube->item_type != ITEM_PBTUBE) {
		stc("That is not a paintball tube!\n", ch);
		return;
	}

	if ((gun = get_eq_char(ch, WEAR_HOLD)) == nullptr) {
		stc("Your not holding your paintgun!\n", ch);
		return;
	}

	if (gun->item_type != ITEM_PBGUN) {
		stc("That is not a paintball gun!\n", ch);
		return;
	}

	if ((gun->value[0] + tube->value[0]) > gun->value[1]) {
		Format::sprintf(buf, "Your paint gun can only hold %d paintballs.\n",
		        gun->value[1]);
		stc(buf, ch);
		return;
	}

	extract_obj(tube);
	gun->value[0] += tube->value[0];
	Format::sprintf(buf, "{VYou now have {W[{V%d{W]{V paintballs!{x\n", gun->value[0]);
	stc(buf, ch);
} /* end do_reload() */

void do_splat(Character *ch, String argument)
{
	Object *gun;
	Room *location;
	Character *victim;

	if ((gun = get_eq_char(ch, WEAR_HOLD)) == nullptr) {
		stc("You're not holding your paintgun!\n", ch);
		return;
	}

	if (gun->item_type != ITEM_PBGUN) {
		stc("That is not a paintball gun!\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("Shoot who?\n", ch);
		return;
	}

	if ((victim = get_char_here(ch, argument, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if ((IS_NPC(ch)) || (IS_NPC(victim))) {
		stc("Mobiles can't play paintball.\n", ch);
		return;
	}

	if (!ch->pcdata->plr_flags.has(PLR_PAINT) ||
	    !victim->pcdata->plr_flags.has(PLR_PAINT)) {
		stc("Both players must have their paintball flag on.\n", ch);
		return;
	}

	if (ch->in_room == nullptr || victim->in_room == nullptr)
		return;

	if (victim->in_room->flags().has(ROOM_SAFE)) {
		stc("Oddly enough, in this room you feel peaceful.\n", ch);
		return;
	}

	if (gun->value[0] <= 0) {
		stc("Your gun is empty!  Reload!\n", ch);
		return;
	}

	if (gun->value[3] > 25) {
		stc("Your gun is damaged too badly!\n", ch);
		return;
	}

	if (ch == victim) {
		stc("OUCH!\n", ch);
		gun->value[0] -= 1;
		WAIT_STATE(ch, gun->value[2]);
		return;
	}

	if (number_percent() > (75 - gun->value[4])) {
		if ((location = Game::world().get_room(Location(Vnum(ROOM_VNUM_ALTAR)))) == nullptr)
			return;

		act("{P$n nails $N with a bullseye paintball shot! {HSPLAT!{x",
		    ch, nullptr, victim, TO_NOTVICT);
		act("{PYou blast $N with a perfect shot! {HSPLAT!{x",
		    ch, nullptr, victim, TO_CHAR);
		act("{PA pellet from $n's gun soars through the air and hits you! {HSPLAT!{x",
		    ch, nullptr, victim, TO_VICT);

		if ((victim->in_room->sector_type() != Sector::arena) &&
		    (!victim->in_room->flags().has(ROOM_NO_RECALL)) &&
		    !char_in_duel_room(victim) &&
		    (!affect::exists_on_char(victim, affect::type::curse))) {
			char_from_room(victim);
			char_to_room(victim, location);

			if (victim->pet != nullptr) {
				char_from_room(victim->pet);
				char_to_room(victim->pet, location);
			}

			do_look(victim, "auto");
		}

		gun->value[0] -= 1;
		WAIT_STATE(ch, gun->value[2]);
		return;
	}
	else if (number_percent() > (25 - gun->value[4])) {
		act("{B$n fires at $N, but misses by a mile.{x",
		    ch, nullptr, victim, TO_NOTVICT);
		act("{BYou miss $N!  You couldn't hit the broadside of a barn!{x",
		    ch, nullptr, victim, TO_CHAR);
		act("{B$n shoots at you and misses.  Whew!{x",
		    ch, nullptr, victim, TO_VICT);
		gun->value[0] -= 1;
		WAIT_STATE(ch, gun->value[2]);
		return;
	}
	else {
		stc("{YDo'h, Your gun is jammed!!!{x\n", ch);
		gun->value[0] -= 1;
		WAIT_STATE(ch, gun->value[2] * 2);
		gun->value[3] += 1;
		return;
	}
}
