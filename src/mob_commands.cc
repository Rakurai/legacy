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
 *  The MOBprograms have been contributed by N'Atas-ha.  Any support for   *
 *  these routines should not be expected from Merc Industries.  However,  *
 *  under no circumstances should the blame for bugs, etc be placed on     *
 *  Merc Industries.  They are not guaranteed to work on all systems due   *
 *  to their frequent use of strxxx functions.  They are also not the most *
 *  efficient way to perform their tasks, but hopefully should be in the   *
 *  easiest possible way to install and begin using. Documentation for     *
 *  such installation can be found in INSTALL.  Enjoy........    N'Atas-Ha *
 ***************************************************************************/

#include "act.hh"
#include "argument.hh"
#include "affect/Affect.hh"
#include "Character.hh"
#include "declare.hh"
#include "Exit.hh"
#include "find.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "Logging.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "MobProg.hh"
#include "Object.hh"
#include "Player.hh"
#include "Room.hh"
#include "String.hh"
#include "World.hh"


/* A trivial rehack of do_mstat.  This doesnt show all the data, but just
 * enough to identify the mob and give its basic condition.  It does however,
 * show the MOBprograms which are set.
 */

void do_mpstat(Character *ch, String argument)
{
	char        buf[ MAX_STRING_LENGTH ];
	Character  *victim;

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		stc("MobProg stat whom?\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (!victim->is_npc()) {
		stc("Only Mobiles can have Programs!\n", ch);
		return;
	}

	if (victim->pIndexData->progtype_flags.empty()) {
		stc("That Mobile has no Programs set.\n", ch);
		return;
	}

	Format::sprintf(buf, "Name: %s.  Vnum: %d.\n",
	        victim->name, victim->pIndexData->vnum);
	stc(buf, ch);
	Format::sprintf(buf, "Short description: %s.\nLong  description: %s",
	        victim->short_descr,
	        !victim->long_descr.empty() ?
	        victim->long_descr : "(none).\n");
	stc(buf, ch);
	Format::sprintf(buf, "Hp: %d/%d.  Mana: %d/%d.  Stamina: %d/%d. \n",
	        victim->hit,         GET_MAX_HIT(victim),
	        victim->mana,        GET_MAX_MANA(victim),
	        victim->stam,        GET_MAX_STAM(victim));
	stc(buf, ch);
	Format::sprintf(buf,
	        "Lv: %d.  Class: %d.  Align: %d.  AC: %d.  Gold: %ld.  Exp: %d.\n",
	        victim->level,       victim->guild,        victim->alignment,
	        GET_AC(victim, AC_PIERCE),    victim->gold,         victim->exp);
	stc(buf, ch);

	for (const auto mprg : victim->pIndexData->mobprogs) {
		Format::sprintf(buf, ">%s %s\n%s\n",
		        MobProg::type_to_name(mprg->type),
		        mprg->arglist,
		        mprg->comlist);
		stc(buf, ch);
	}

	return;
}

/* prints the argument to all the rooms aroud the mobile */

void do_mpasound(Character *ch, String argument)
{
	Room *was_in_room;
	int              door;
	bool save_mobtrigger;

	if (!ch->is_npc() || ch->act_flags.has(ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	if (argument.empty()) {
		Logging::bugf("Mpasound - No argument: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	was_in_room = ch->in_room;

	for (door = 0; door <= 5; door++) {
		Exit       *pexit;

		if ((pexit = was_in_room->exit[door]) != nullptr
		    &&   pexit->to_room != nullptr
		    &&   pexit->to_room != was_in_room) {
			ch->in_room = pexit->to_room;
			save_mobtrigger = MOBtrigger;
			MOBtrigger  = false;
			act(argument, ch, nullptr, nullptr, TO_ROOM);
			MOBtrigger = save_mobtrigger;
		}
	}

	ch->in_room = was_in_room;
	return;
}

/* lets the mobile kill any player or mobile without murder*/

void do_mpkill(Character *ch, String argument)
{
	Character *victim;

	if (!ch->is_npc() || ch->act_flags.has(ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		Logging::bugf("MpKill - no argument: vnum %d.",
		    ch->pIndexData->vnum);
		return;
	}

	if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr) {
		Logging::bugf("MpKill - Victim not in room: vnum %d.",
		    ch->pIndexData->vnum);
		return;
	}

	if (victim == ch) {
		Logging::bugf("MpKill - Bad victim to attack: vnum %d.",
		    ch->pIndexData->vnum);
		return;
	}

	if (affect::exists_on_char(ch, affect::type::charm_person) && ch->master == victim) {
		Logging::bugf("MpKill - Charmed mob attacking master: vnum %d.",
		    ch->pIndexData->vnum);
		return;
	}

	if (ch->fighting) {
		Logging::bugf("MpKill - Already fighting: vnum %d",
		    ch->pIndexData->vnum);
		return;
	}

	multi_hit(ch, victim, skill::type::unknown);
	return;
}

/* lets the mobile destroy an object in its inventory
   it can also destroy a worn object and it can destroy
   items using all.xxxxx or just plain all of them */

void do_mpjunk(Character *ch, String argument)
{
	Object *obj;
	Object *obj_next;

	if (!ch->is_npc() || ch->act_flags.has(ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		Logging::bugf("Mpjunk - No argument: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	if (arg != "all" && !arg.has_prefix("all.")) {
		if ((obj = get_obj_wear(ch, arg)) != nullptr) {
			unequip_char(ch, obj);
			extract_obj(obj);
			return;
		}

		if ((obj = get_obj_carry(ch, arg)) == nullptr)
			return;

		extract_obj(obj);
	}
	else
		for (obj = ch->carrying; obj != nullptr; obj = obj_next) {
			obj_next = obj->next_content;

			if (arg[3] == '\0' || obj->name.has_words(&arg[4])) {
				if (obj->wear_loc != WEAR_NONE)
					unequip_char(ch, obj);

				extract_obj(obj);
			}
		}

	return;
}

/* prints the message to everyone in the room other than the mob and victim */

void do_mpechoaround(Character *ch, String argument)
{
	Character *victim;

	if (!ch->is_npc() || ch->act_flags.has(ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty()) {
		Logging::bugf("Mpechoaround - No argument:  vnum %d.", ch->pIndexData->vnum);
		return;
	}

	if (!(victim = get_char_here(ch, arg, VIS_CHAR))) {
		Logging::bugf("Mpechoaround - victim does not exist: vnum %d.",
		    ch->pIndexData->vnum);
		return;
	}

	act(argument, ch, nullptr, victim, TO_NOTVICT);
	return;
}

/* prints the message to only the victim */

void do_mpechoat(Character *ch, String argument)
{
	Character *victim;

	if (!ch->is_npc() || ch->act_flags.has(ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty() || argument.empty()) {
		Logging::bugf("Mpechoat - No argument:  vnum %d.",
		    ch->pIndexData->vnum);
		return;
	}

	if (!(victim = get_char_here(ch, arg, VIS_CHAR))) {
		Logging::bugf("Mpechoat - victim does not exist: vnum %d.",
		    ch->pIndexData->vnum);
		return;
	}

	act(argument, ch, nullptr, victim, TO_VICT);
	return;
}

/* prints the message to the room at large */

void do_mpecho(Character *ch, String argument)
{
	if (!ch->is_npc() || ch->act_flags.has(ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	if (argument.empty()) {
		Logging::bugf("Mpecho - called w/o argument: vnum %d.",
		    ch->pIndexData->vnum);
		return;
	}

	act(argument, ch, nullptr, nullptr, TO_ROOM);
	return;
}

void do_mpmoney(Character *ch, String argument)
{
	if (!ch->is_npc() || ch->act_flags.has(ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	String gold, silver;
	argument = one_argument(argument, gold);
	argument = one_argument(argument, silver);

	if (gold.empty() || silver.empty() || !gold.is_number() || !silver.is_number()) {
		Logging::bugf("Mpmoney: bad syntax '%s' gold and '%s' silver, vnum %d", gold, silver, ch->pIndexData->vnum);
		return;
	}

	ch->gold = atoi(gold);
	ch->silver = atoi(silver);
	return;
}

/* lets the mobile load an item or mobile.  All items
are loaded into inventory.  you can specify a level with
the load object portion as well. */

void do_mpmload(Character *ch, String argument)
{
	MobilePrototype *pMobIndex;
	Character      *victim;

	if (!ch->is_npc() || ch->act_flags.has(ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg.empty() || !arg.is_number()) {
		Logging::bugf("Mpmload - Bad vnum as arg: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	if ((pMobIndex = Game::world().get_mob_prototype(atoi(arg))) == nullptr) {
		Logging::bugf("Mpmload - Bad mob vnum: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	victim = create_mobile(pMobIndex);

	if (! victim) {  /* Check for memory error. -- Outsider */
		Logging::bug("Memory error in do_mpmload().", 0);
		return;
	}

	char_to_room(victim, ch->in_room);
	return;
}

void do_mpoload(Character *ch, String argument)
{
	ObjectPrototype *pObjIndex;
	Object       *obj;

	if (!ch->is_npc() || ch->act_flags.has(ACT_MORPH)) {
		do_huh(ch);
		return;
	}

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1.empty() || !arg1.is_number()) {
		Logging::bugf("Mpoload - Bad syntax: vnum %d.",
		    ch->pIndexData->vnum);
		return;
	}

	if ((pObjIndex = Game::world().get_obj_prototype(atoi(arg1))) == nullptr) {
		Logging::bugf("Mpoload - Bad vnum arg: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	obj = create_object(pObjIndex, 0);

	if (! obj) {
		Logging::bug("Memory error creating object in mpoload.", 0);
		return;
	}

	if (CAN_WEAR(obj, ITEM_TAKE))
		obj_to_char(obj, ch);
	else
		obj_to_room(obj, ch->in_room);

	return;
}

/* lets the mobile purge all objects and other npcs in the room,
   or purge a specified object or mob in the room.  It can purge
   itself, but this had best be the last command in the MOBprogram
   otherwise ugly stuff will happen */

void do_mppurge(Character *ch, String argument)
{
	Character *victim;
	Object  *obj;

	if (!ch->is_npc() || ch->act_flags.has(ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		/* 'purge' */
		Character *vnext;
		Object  *obj_next;

		for (victim = ch->in_room->people; victim != nullptr; victim = vnext) {
			vnext = victim->next_in_room;

			if (victim->is_npc() && victim != ch)
				extract_char(victim, true);
		}

		for (obj = ch->in_room->contents; obj != nullptr; obj = obj_next) {
			obj_next = obj->next_content;
			extract_obj(obj);
		}

		return;
	}

	if ((victim = get_char_here(ch, arg, VIS_CHAR)) != nullptr) {
		if (!victim->is_npc()) {
			Logging::bugf("Mppurge - Purging a PC: vnum %d.", ch->pIndexData->vnum);
			return;
		}

		extract_char(victim, true);
		return;
	}
	else if ((obj = get_obj_here(ch, arg)) != nullptr)
		extract_obj(obj);
	else
		Logging::bugf("Mppurge - Bad argument: vnum %d.", ch->pIndexData->vnum);
} /* end do_mppurge() */

/* lets the mobile goto any location it wishes that is not private */

void do_mpgoto(Character *ch, String argument)
{
	Room *location;

	if (!ch->is_npc() || ch->act_flags.has(ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		Logging::bugf("Mpgoto - No argument: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	if ((location = find_location(ch, arg)) == nullptr) {
		Logging::bugf("Mpgoto - No such location: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	if (ch->fighting != nullptr)
		stop_fighting(ch, true);

	char_from_room(ch);
	char_to_room(ch, location);
	return;
}

/* lets the mobile do a command at another location. Very useful */

void do_mpat(Character *ch, String argument)
{
	Room *location;
	Room *original;

	if (!ch->is_npc() || ch->act_flags.has(ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty() || argument.empty()) {
		Logging::bugf("Mpat - Bad argument: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	if ((location = find_location(ch, arg)) == nullptr) {
		Logging::bugf("Mpat - No such location: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	original = ch->in_room;
	char_from_room(ch);
	char_to_room(ch, location);
	interpret(ch, argument);

	/*
	 * See if 'ch' still exists before continuing!
	 * Handles 'at XXXX quit' case.
	 */
	if (ch->is_garbage())
		return;

	char_from_room(ch);
	char_to_room(ch, original);
}

/* lets the mobile transfer people.  the all argument transfers
   everyone in the current room to the specified location */

void do_mptransfer(Character *ch, String argument)
{
	Room *location;
//	Descriptor *d;

	if (!ch->is_npc() || ch->act_flags.has(ACT_MORPH)) {
		do_huh(ch);
		return;
	}

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1.empty()) {
		Logging::bugf("Mptransfer - Bad syntax: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	/* eh, let's go ahead and NOT let mobs transfer all :P  -- Montrey */
	/*      if (arg1 == "all")
	        {
	                for (d = descriptor_list; d != nullptr; d = d->next)
	                {
	                        if (d->is_playing()
	                         && d->character != ch
	                         && d->character->in_room != nullptr
	                         && can_see_char(ch, d->character))
	                        {
	                                char buf[MSL];

	                                Format::sprintf(buf, "%s %s", d->character->name, arg2);
	                                do_transfer(ch, buf);
	                        }
	                }

	                return;
	        } */

	if (arg2.empty())
		location = ch->in_room;
	else {
		if ((location = find_location(ch, arg2)) == nullptr) {
			Logging::bugf("Mptransfer - No such location: vnum %d.", ch->pIndexData->vnum);
			return;
		}

		if (location->is_private()) {
			Logging::bugf("Mptransfer - Private room: vnum %d.", ch->pIndexData->vnum);
			return;
		}
	}

	Character *victim = nullptr;
	for (auto vch : Game::world().char_list)
		if (vch->name.has_words(arg1)) {
			victim = vch;
			break;
		}

	if (victim == nullptr) {
		Logging::bugf("Mptransfer - No such person: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	if (victim->in_room == nullptr) {
		Logging::bugf("Mptransfer - Victim in nullptr room: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	if (victim->fighting != nullptr)
		stop_fighting(victim, true);

	char_from_room(victim);
	char_to_room(victim, location);
	return;
}

/* lets the mobile force someone to do something.  must be mortal level
   and the all argument only affects those in the room with the mobile */
void do_mpforce(Character *ch, String argument)
{
	if (!ch->is_npc() || ch->act_flags.has(ACT_MORPH)) {
		do_huh(ch);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty() || argument.empty()) {
		Logging::bugf("Mpforce - Bad syntax: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	if (arg == "all") {
		Character *vch, *vch_next;

		for (vch = ch->in_room->people; vch != nullptr; vch = vch_next) {
			vch_next = vch->next_in_room;

			if (vch->in_room == ch->in_room
			    && !IS_IMMORTAL(vch))
//			 && can_see_char(ch, vch))  who cares about whether the mob can see? -- Montrey
				interpret(vch, argument);
		}
	}
	else {
		Character *victim;

		for (victim = ch->in_room->people; victim != nullptr; victim = victim->next_in_room)
			if (victim->name.has_words(arg))
				break;

		if (victim == nullptr) {
			Logging::bugf("Mpforce - No such victim: vnum %d.", ch->pIndexData->vnum);
			return;
		}

		if (victim == ch) {
			Logging::bugf("Mpforce - Forcing oneself: vnum %d.", ch->pIndexData->vnum);
			return;
		}

		interpret(victim, argument);
	}
}

void do_mpinvis(Character *ch, String argument) {
	if (!ch->is_npc() || ch->act_flags.has(ACT_MORPH)) {
		do_huh(ch);
		return;
	}

	ch->act_flags += ACT_SUPERMOB;
}

void do_mpvis(Character *ch, String argument) {
	if (!ch->is_npc() || ch->act_flags.has(ACT_MORPH)) {
		do_huh(ch);
		return;
	}

	ch->act_flags -= ACT_SUPERMOB;
}

// makes a mob a pet of a player.  doesn't set 'charm', add that if needed outside
void do_mpmake_pet(Character *ch, String argument) {
	if (!ch->is_npc() || ch->act_flags.has(ACT_MORPH)) {
		do_huh(ch);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty()) {
		Logging::bugf("Mpmake_pet - Bad syntax: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	Character *master;

	for (master = ch->in_room->people; master != nullptr; master = master->next_in_room)
		if (!master->is_npc() && master->name.has_exact_words(arg))
			break;

	if (master == nullptr) {
		Logging::bugf("MPmake_pet: no such master '%s': vnum %d.", arg, ch->pIndexData->vnum);
		return;
	}

	Character *pet = ch; // default self

	if (!argument.empty()) {
		one_argument(argument, arg);

		for (pet = ch->in_room->people; pet != nullptr; pet = pet->next_in_room)
			if (pet->is_npc() && pet->name.has_exact_words(arg))
				break;

		if (pet == nullptr) {
			Logging::bugf("MPmake_pet: no such mob '%s': vnum %d.", arg, ch->pIndexData->vnum);
			return;
		}
	}

	// fix up old pet if any
	if (master->pet != nullptr) {
		stop_follower(master->pet);
	}

	make_pet(master, pet);
}

void do_mpcontrol(Character *ch, String argument) {
	if (!ch->is_npc() || ch->act_flags.has(ACT_MORPH)) {
		do_huh(ch);
		return;
	}

	String key, mob_name, target_name;
	argument = one_argument(argument, mob_name);
	target_name = one_argument(argument, key);

	if (mob_name.empty() || key.empty()) {
		Logging::bugf("Mpcontrol - Bad syntax: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	Character *mob = get_char_here(ch, mob_name, VIS_CHAR);

	if (mob == nullptr) {
		Logging::bugf("MPcontrol: no such mob '%s': vnum %d.", mob_name, ch->pIndexData->vnum);
		return;
	}

	Character *target = nullptr;

	// allow target to be nullptr, but only if no name given
	if (!target_name.empty()) {
		target = get_char_here(ch, target_name, VIS_CHAR);

		if (target == nullptr) {
			Logging::bugf("MPcontrol: no such target '%s': vnum %d.", target_name, ch->pIndexData->vnum);
			return;
		}
	}

	mprog_control_trigger(mob, key, target);
}
