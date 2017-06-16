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

#include "Game.hh"
#include "Area.hh"
#include "find.hh"
#include "merc.hh"
#include "interp.hh"
#include "Affect.hh"
#include "Format.hh"
#include "Battle.hh"

DECLARE_SPEC_FUN(spec_clanguard);

const   sh_int  stamina_loss   [SECT_MAX]      = {
	1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6
};

/*
 * Local functions.
 */
int     find_door       args((Character *ch, char *arg));
bool    has_key         args((Character *ch, int key));
int     find_exit       args((Character *ch, const String& arg));

void move_char(Character *ch, int door, bool follow)
{
	Character *fch, *fch_next;
	RoomPrototype *in_room, *to_room;
	Exit *pexit;
	int cost;
	char dir_buf[128];

	if (door < 0 || door > 5) {
		bug("Do_move: bad door %d.", door);
		return;
	}

	in_room = ch->in_room;

	if ((pexit   = in_room->exit[door]) == nullptr
	    || (to_room = pexit->u1.to_room) == nullptr
	    || !can_see_room(ch, pexit->u1.to_room)) {
		stc("Alas, you cannot go that way.\n", ch);
		return;
	}

	if (GET_ROOM_FLAGS(to_room).has(ROOM_LAW)
	    && (IS_NPC(ch) && ch->act_flags.has(ACT_AGGRESSIVE))) {
		stc("They don't seem to want your 'type' here.", ch);
		return;
	}

	if (pexit->exit_flags.has(EX_CLOSED)
	    && (!affect_exists_on_char(ch, gsn_pass_door) || pexit->exit_flags.has(EX_NOPASS))
	    &&  !IS_IMMORTAL(ch)) {
		act("The $d is closed.", ch, nullptr, pexit->keyword, TO_CHAR);
		return;
	}

	/*
	It seems sort of silly that a master cannot send their
	pet away. You can't order other people's pets/charms around,
	anyway. So, I'm taking this out to allow a master to send
	his pet "away" or "home" or otherwise out of the room.
	-- Outsider

	if (affect_exists_on_char(ch, gsn_charm_person)
	 && ch->master != nullptr
	 && in_room == ch->master->in_room)
	{
	     stc("What? And leave your beloved master?\n", ch);
	     return;
	}
	*/

	if (!is_room_owner(ch, to_room) && room_is_private(to_room)) {
		stc("Sorry, that room is private.\n", ch);
		return;
	}

	if (!IS_NPC(ch)) {
		if (to_room->guild && to_room->guild != ch->cls + 1 && !IS_IMMORTAL(ch)) {
			stc("You must be a guild member to enter.\n", ch);
			return;
		}

		/* don't care about mobs getting messages */
		if (GET_ROOM_FLAGS(in_room).has(ROOM_UNDER_WATER)) {
			if (GET_ROOM_FLAGS(to_room).has(ROOM_UNDER_WATER))
				stc("{CYou continue to hold your breath...{x\n", ch);
			else
				stc("{CYou gasp for air!{x\n", ch);
		}
		else if (GET_ROOM_FLAGS(to_room).has(ROOM_UNDER_WATER))
			stc("{CYou begin to hold your breath.{x\n", ch);
	}

	if ((in_room->sector_type == SECT_AIR
	  || to_room->sector_type == SECT_AIR)
	 && !IS_FLYING(ch) 
	 && !IS_IMMORTAL(ch)) {
		if (CAN_FLY(ch))
			do_fly(ch, ""); // try to take off
		else
			stc("You cannot fly!\n", ch);

		if (!IS_FLYING(ch)) // still not flying?
			return;
	}

	if ((in_room->sector_type == SECT_WATER_NOSWIM
	  || to_room->sector_type == SECT_WATER_NOSWIM)
	 && !IS_FLYING(ch)
	 && !IS_IMMORTAL(ch)
	 && !get_skill(ch, gsn_swimming)) {
		// try to find a boat first
		bool found = FALSE;

		for (Object *obj = ch->carrying; obj != nullptr; obj = obj->next_content)
			if (obj->item_type == ITEM_BOAT) {
				found = TRUE;
				break;
			}

		if (!found) {
			// no boat?  try to fly then
			if (CAN_FLY(ch))
				do_fly(ch, "");

			// flying now?
			if (!IS_FLYING(ch)) {
				stc("You don't float well enough to walk on water.\n", ch);
				return;
			}
		}
	}

	cost = (stamina_loss[UMIN(SECT_MAX - 1, in_room->sector_type)]
	        + stamina_loss[UMIN(SECT_MAX - 1, to_room->sector_type)]) / 2;

	/* conditional effects */
	if (IS_FLYING(ch) || affect_exists_on_char(ch, gsn_haste))
		cost /= 2;

	if (affect_exists_on_char(ch, gsn_slow))
		cost *= 2;

	/* remort affect - light feet */
	if (HAS_RAFF(ch, RAFF_LIGHTFEET))
		cost /= 2;

	if (ch->stam < cost) {
		stc("You are too exhausted to continue.\n", ch);
		return;
	}

	/* remort affect - slow walk */
	if (HAS_RAFF(ch, RAFF_SLOWWALK))
		WAIT_STATE(ch, 3);
	else
		WAIT_STATE(ch, 1);

	ch->stam -= cost;

	if (affect_exists_on_char(ch, gsn_sneak) || ch->invis_level
	    || (!IS_NPC(ch) && ch->act_flags.has(PLR_SUPERWIZ)))
		act("$n leaves $T.", ch, nullptr, Exit::dir_name(door), TO_NOTVIEW, POS_SNEAK, FALSE);
	else
		act("$n leaves $T.", ch, nullptr, Exit::dir_name(door), TO_NOTVIEW);

	char_from_room(ch);
	char_to_room(ch, to_room);

	if (Exit::rev_dir(door) == 5)
		Format::sprintf(dir_buf, "%s", "below");
	else if (Exit::rev_dir(door) == 4)
		Format::sprintf(dir_buf, "%s", "above");
	else
		Format::sprintf(dir_buf, "the %s", Exit::dir_name(door, true));

	if (affect_exists_on_char(ch, gsn_sneak) || ch->invis_level
	    || (!IS_NPC(ch) && ch->act_flags.has(PLR_SUPERWIZ)))
		act("$n has arrived from $T.", ch, nullptr, dir_buf, TO_NOTVIEW, POS_SNEAK, FALSE);
	else
		act("$n has arrived from $T.", ch, nullptr, dir_buf, TO_NOTVIEW);

	do_look(ch, "auto");

	for (fch = to_room->people; fch != nullptr; fch = fch_next) {
		fch_next = fch->next_in_room;

		if (IS_NPC(fch) && fch->spec_fun && fch->spec_fun == spec_lookup("spec_clanguard"))
			spec_clanguard(fch);
	}

	if (in_room == to_room) /* no circular follows */
		return;

	for (fch = in_room->people; fch != nullptr; fch = fch_next) {
		fch_next = fch->next_in_room;

		if (fch->master == ch && affect_exists_on_char(fch, gsn_charm_person) && get_position(fch) < POS_STANDING) {
			if (fch->start_pos == POS_FLYING && CAN_FLY(fch))
				do_fly(fch, "");
			else
				do_stand(fch, "");
		}

		if (fch->master == ch && get_position(fch) >= POS_STANDING && can_see_room(fch, to_room)) {
			if (IS_NPC(fch) && fch->act_flags.has(ACT_STAY))
				continue;

			if (GET_ROOM_FLAGS(ch->in_room).has(ROOM_LAW) && (IS_NPC(fch)
			                && fch->act_flags.has(ACT_AGGRESSIVE))) {
				act("You can't bring $N into the city.", ch, nullptr, fch, TO_CHAR);
				act("They don't seem to want your 'type' here.", fch, nullptr, nullptr, TO_CHAR);
				continue;
			}

			act("You follow $N.", fch, nullptr, ch, TO_CHAR);
			move_char(fch, door, TRUE);
		}
	}

	mprog_entry_trigger(ch);
	mprog_greet_trigger(ch);
}

void do_north(Character *ch, String argument)
{
	move_char(ch, DIR_NORTH, FALSE);
	return;
}

void do_east(Character *ch, String argument)
{
	move_char(ch, DIR_EAST, FALSE);
	return;
}

void do_south(Character *ch, String argument)
{
	move_char(ch, DIR_SOUTH, FALSE);
	return;
}

void do_west(Character *ch, String argument)
{
	move_char(ch, DIR_WEST, FALSE);
	return;
}

void do_up(Character *ch, String argument)
{
	move_char(ch, DIR_UP, FALSE);
	return;
}

void do_down(Character *ch, String argument)
{
	move_char(ch, DIR_DOWN, FALSE);
	return;
}

int find_door(Character *ch, const String& arg)
{
	Exit *pexit;
	int door;

	if (arg.is_prefix_of("north"))    door = 0;
	else if (arg.is_prefix_of("east"))    door = 1;
	else if (arg.is_prefix_of("south"))    door = 2;
	else if (arg.is_prefix_of("west"))    door = 3;
	else if (arg.is_prefix_of("up"))    door = 4;
	else if (arg.is_prefix_of("down"))    door = 5;
	else {
		for (door = 0; door <= 5; door++) {
			if ((pexit = ch->in_room->exit[door]) != nullptr
			    && pexit->exit_flags.has(EX_ISDOOR)
			    && pexit->keyword.has_words(arg))
				return door;
		}

		act("You see no $T here.", ch, nullptr, arg, TO_CHAR);
		return -1;
	}

	if ((pexit = ch->in_room->exit[door]) == nullptr) {
		act("There's no door $T here.", ch, nullptr, arg, TO_CHAR);
		return -1;
	}

	if (!pexit->exit_flags.has(EX_ISDOOR)) {
		stc("You can't do that.\n", ch);
		return -1;
	}

	return door;
}

/* This is for Smokescreen */
int find_exit(Character *ch, const String& arg)
{
	Exit *pexit;
	int door;

	if (arg == "n" || arg == "north") door = 0;
	else if (arg == "e" || arg == "east") door = 1;
	else if (arg == "s" || arg == "south") door = 2;
	else if (arg == "w" || arg == "west") door = 3;
	else if (arg == "u" || arg == "up") door = 4;
	else if (arg == "d" || arg == "down") door = 5;
	else {
		for (door = 0; door <= 5; door++) {
			if ((pexit = ch->in_room->exit[door]) != nullptr
			    &&   pexit->exit_flags.has(EX_ISDOOR)
			    &&   pexit->keyword.has_words(arg))
				return door;
		}

		act("I see no $T here.", ch, nullptr, arg, TO_CHAR);
		return -1;
	}

	if ((pexit = ch->in_room->exit[door]) == nullptr) {
		act("I see no door $T here.", ch, nullptr, arg, TO_CHAR);
		return -1;
	}

	return door;
}

void do_open(Character *ch, String argument)
{
	Object *obj;
	int door;

	if (argument.empty()) {
		stc("Open what?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (!arg.is_prefix_of("north") /* hack so players can refer to a direction if */
	    && !arg.is_prefix_of("east")  /* they have an item of that name -- Montrey */
	    && !arg.is_prefix_of("south")
	    && !arg.is_prefix_of("west")
	    && !arg.is_prefix_of("up")
	    && !arg.is_prefix_of("down")
	    && (obj = get_obj_here(ch, arg)) != nullptr) {
		/* open portal */
		if (obj->item_type == ITEM_PORTAL) {
			if (!obj->value[1].flags().has(EX_ISDOOR)) {
				stc("You can't do that.\n", ch);
				return;
			}

			if (!obj->value[1].flags().has(EX_CLOSED)) {
				stc("It's already open.\n", ch);
				return;
			}

			if (obj->value[1].flags().has(EX_LOCKED)) {
				stc("It's locked.\n", ch);
				return;
			}

			obj->value[1] -= EX_CLOSED;
			act("You open $p and a bright unholy light dawns upon you.", ch, obj, nullptr, TO_CHAR);
			act("$n opens $p and is overtaken by an unholy light.", ch, obj, nullptr, TO_ROOM);
			return;
		}

		/* 'open object' */
		if (obj->item_type != ITEM_CONTAINER) {
			if (IS_OBJ_STAT(obj, ITEM_COMPARTMENT))
				stc("It's already open.\n", ch);
			else if (obj->contains)
				stc("You can't open it.\n", ch);
			else
				stc("That's not a container.\n", ch);

			return;
		}

		if (!obj->value[1].flags().has(CONT_CLOSED)) {
			stc("It's already open.\n", ch);
			return;
		}

		if (!obj->value[1].flags().has(CONT_CLOSEABLE)) {
			stc("You can't do that.\n", ch);
			return;
		}

		if (obj->value[1].flags().has(CONT_LOCKED)) {
			stc("It's locked.\n", ch);
			return;
		}

		obj->value[1] -= CONT_CLOSED;
		act("You open $p.", ch, obj, nullptr, TO_CHAR);
		act("$n opens $p.", ch, obj, nullptr, TO_ROOM);
		return;
	}

	if ((door = find_door(ch, arg)) >= 0) {
		/* 'open door' */
		RoomPrototype *to_room;
		Exit *pexit;
		Exit *pexit_rev;
		pexit = ch->in_room->exit[door];

		if (!pexit->exit_flags.has(EX_CLOSED))
		{ stc("It's already open.\n",      ch); return; }

		if (pexit->exit_flags.has(EX_LOCKED))
		{ stc("It's locked.\n",            ch); return; }

		pexit->exit_flags -= EX_CLOSED;
		act("$n opens the $d.", ch, nullptr, pexit->keyword, TO_ROOM);
		stc("You open it.\n", ch);

		/* open the other side */
		if ((to_room   = pexit->u1.to_room) != nullptr
		    && (pexit_rev = to_room->exit[Exit::rev_dir(door)]) != nullptr
		    &&   pexit_rev->u1.to_room == ch->in_room) {
			Character *rch;
			pexit_rev->exit_flags -= EX_CLOSED;

			for (rch = to_room->people; rch != nullptr; rch = rch->next_in_room)
				act("The $d opens.", rch, nullptr, pexit_rev->keyword, TO_CHAR);
		}
	}

	return;
}

void do_close(Character *ch, String argument)
{
	Object *obj;
	int door;

	if (argument.empty()) {
		stc("Close what?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (!arg.is_prefix_of("north") /* hack so players can refer to a direction if */
	    && !arg.is_prefix_of("east")  /* they have an item of that name -- Montrey */
	    && !arg.is_prefix_of("south")
	    && !arg.is_prefix_of("west")
	    && !arg.is_prefix_of("up")
	    && !arg.is_prefix_of("down")
	    && (obj = get_obj_here(ch, arg)) != nullptr) {
		/* portal stuff */
		if (obj->item_type == ITEM_PORTAL) {
			if (!obj->value[1].flags().has(EX_ISDOOR)
			    || obj->value[1].flags().has(EX_NOCLOSE)) {
				stc("You can't do that.\n", ch);
				return;
			}

			if (obj->value[1].flags().has(EX_CLOSED)) {
				stc("It's already closed.\n", ch);
				return;
			}

			obj->value[1] += EX_CLOSED;
			act("You close $p.", ch, obj, nullptr, TO_CHAR);
			act("$n closes $p.", ch, obj, nullptr, TO_ROOM);
			return;
		}

		/* 'close object' */
		if (obj->item_type != ITEM_CONTAINER) {
			if (IS_OBJ_STAT(obj, ITEM_COMPARTMENT))
				stc("You can't close it.\n", ch);
			else if (obj->contains)
				stc("There is no apparent opening.\n", ch);
			else
				stc("That's not a container.\n", ch);

			return;
		}

		if (obj->value[1].flags().has(CONT_CLOSED)) {
			stc("It's already closed.\n", ch);
			return;
		}

		if (!obj->value[1].flags().has(CONT_CLOSEABLE)) {
			stc("You can't do that.\n", ch);
			return;
		}

		obj->value[1] += CONT_CLOSED;
		act("You close $p.", ch, obj, nullptr, TO_CHAR);
		act("$n closes $p.", ch, obj, nullptr, TO_ROOM);
		return;
	}

	if ((door = find_door(ch, arg)) >= 0) {
		/* 'close door' */
		RoomPrototype *to_room;
		Exit *pexit;
		Exit *pexit_rev;
		pexit = ch->in_room->exit[door];

		if (pexit->exit_flags.has(EX_CLOSED)) {
			stc("It's already closed.\n", ch);
			return;
		}

		pexit->exit_flags += EX_CLOSED;
		act("$n closes the $d.", ch, nullptr, pexit->keyword, TO_ROOM);
		stc("You close it.\n", ch);

		/* close the other side */
		if ((to_room  = pexit->u1.to_room) != nullptr
		    && (pexit_rev = to_room->exit[Exit::rev_dir(door)]) != 0
		    && pexit_rev->u1.to_room == ch->in_room) {
			Character *rch;
			pexit_rev->exit_flags += EX_CLOSED;

			for (rch = to_room->people; rch != nullptr; rch = rch->next_in_room)
				act("The $d closes.", rch, nullptr, pexit_rev->keyword, TO_CHAR);
		}
	}
}

bool has_key(Character *ch, int key)
{
	Object *obj;

	for (obj = ch->carrying; obj != nullptr; obj = obj->next_content) {
		if (obj->pIndexData->vnum == key)
			return TRUE;
	}

	return FALSE;
}

void do_lock(Character *ch, String argument)
{
	Object *obj;
	int door;

	if (argument.empty()) {
		stc("Lock what?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (!arg.is_prefix_of("north") /* hack so players can refer to a direction if */
	    && !arg.is_prefix_of("east")  /* they have an item of that name -- Montrey */
	    && !arg.is_prefix_of("south")
	    && !arg.is_prefix_of("west")
	    && !arg.is_prefix_of("up")
	    && !arg.is_prefix_of("down")
	    && (obj = get_obj_here(ch, arg)) != nullptr) {
		/* portal stuff */
		if (obj->item_type == ITEM_PORTAL) {
			if (!obj->value[1].flags().has(EX_ISDOOR)
			    ||  obj->value[1].flags().has(EX_NOCLOSE)) {
				stc("You can't do that.\n", ch);
				return;
			}

			if (!obj->value[1].flags().has(EX_CLOSED)) {
				stc("It's not closed.\n", ch);
				return;
			}

			if (obj->value[4] < 0 || obj->value[1].flags().has(EX_NOLOCK)) {
				stc("It can't be locked.\n", ch);
				return;
			}

			if (!has_key(ch, obj->value[4])) {
				stc("You lack the key.\n", ch);
				return;
			}

			if (obj->value[1].flags().has(EX_LOCKED)) {
				stc("It's already locked.\n", ch);
				return;
			}

			obj->value[1] += EX_LOCKED;
			act("You lock $p.", ch, obj, nullptr, TO_CHAR);
			act("$n locks $p and carefully hides the key.", ch, obj, nullptr, TO_ROOM);
			return;
		}

		/* 'lock object' */
		if (obj->item_type != ITEM_CONTAINER) {
			if (IS_OBJ_STAT(obj, ITEM_COMPARTMENT))
				stc("You can't even close it, let alone lock it.\n", ch);
			else if (obj->contains)
				stc("You can't find a keyhole.\n", ch);
			else
				stc("That's not a container.\n", ch);

			return;
		}

		if (!obj->value[1].flags().has(CONT_CLOSED)) {
			stc("It's not closed.\n", ch);
			return;
		}

		if (obj->value[2] < 0) {
			stc("It can't be locked.\n", ch);
			return;
		}

		if (!has_key(ch, obj->value[2])) {
			stc("You lack the key.\n", ch);
			return;
		}

		if (obj->value[1].flags().has(CONT_LOCKED)) {
			stc("It's already locked.\n", ch);
			return;
		}

		obj->value[1] += CONT_LOCKED;
		act("You lock $p.", ch, obj, nullptr, TO_CHAR);
		act("$n locks $p and carefully hides the key.", ch, obj, nullptr, TO_ROOM);
		return;
	}

	if ((door = find_door(ch, arg)) >= 0) {
		/* 'lock door' */
		RoomPrototype *to_room;
		Exit *pexit;
		Exit *pexit_rev;
		pexit = ch->in_room->exit[door];

		if (!pexit->exit_flags.has(EX_CLOSED)) {
			stc("It's not closed.\n", ch);
			return;
		}

		if (pexit->key < 0) {
			stc("It can't be locked.\n", ch);
			return;
		}

		if (!has_key(ch, pexit->key)) {
			stc("You lack the key.\n", ch);
			return;
		}

		if (pexit->exit_flags.has(EX_LOCKED)) {
			stc("It's already locked.\n", ch);
			return;
		}

		pexit->exit_flags += EX_LOCKED;
		stc("You lock the door.\n", ch);
		act("$n locks the $d.", ch, nullptr, pexit->keyword, TO_ROOM);

		/* lock the other side */
		if ((to_room = pexit->u1.to_room) != nullptr
		    && (pexit_rev = to_room->exit[Exit::rev_dir(door)]) != 0
		    && pexit_rev->u1.to_room == ch->in_room)
			pexit_rev->exit_flags += EX_LOCKED;
	}
}

void do_unlock(Character *ch, String argument)
{
	Object *obj;
	int door;

	if (argument.empty()) {
		stc("Unlock what?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (!arg.is_prefix_of("north") /* hack so players can refer to a direction if */
	    && !arg.is_prefix_of("east")  /* they have an item of that name -- Montrey */
	    && !arg.is_prefix_of("south")
	    && !arg.is_prefix_of("west")
	    && !arg.is_prefix_of("up")
	    && !arg.is_prefix_of("down")
	    && (obj = get_obj_here(ch, arg)) != nullptr) {
		/* portal stuff */
		if (obj->item_type == ITEM_PORTAL) {
			if (obj->value[1].flags().has(EX_ISDOOR)) {
				stc("You can't do that.\n", ch);
				return;
			}

			if (!obj->value[1].flags().has(EX_CLOSED)) {
				stc("It's not closed.\n", ch);
				return;
			}

			if (obj->value[4] < 0) {
				stc("It can't be unlocked.\n", ch);
				return;
			}

			if (!has_key(ch, obj->value[4])) {
				stc("You lack the key.\n", ch);
				return;
			}

			if (!obj->value[1].flags().has(EX_LOCKED)) {
				stc("It's already unlocked.\n", ch);
				return;
			}

			obj->value[1] -= EX_LOCKED;
			act("You unlock $p.", ch, obj, nullptr, TO_CHAR);
			act("$n unlocks $p.", ch, obj, nullptr, TO_ROOM);
			return;
		}

		/* 'unlock object' */
		if (obj->item_type != ITEM_CONTAINER) {
			if (IS_OBJ_STAT(obj, ITEM_COMPARTMENT))
				stc("It's not even closed.\n", ch);
			else if (obj->contains)
				stc("You can't unlock that.\n", ch);
			else
				stc("That's not a container.\n", ch);

			return;
		}

		if (!obj->value[1].flags().has(CONT_CLOSED)) {
			stc("It's not closed.\n", ch);
			return;
		}

		if (obj->value[2] < 0) {
			stc("It can't be unlocked.\n", ch);
			return;
		}

		if (!has_key(ch, obj->value[2])) {
			stc("You lack the key.\n", ch);
			return;
		}

		if (!obj->value[1].flags().has(CONT_LOCKED)) {
			stc("It's already unlocked.\n", ch);
			return;
		}

		obj->value[1] -= CONT_LOCKED;
		act("You unlock $p.", ch, obj, nullptr, TO_CHAR);
		act("$n unlocks $p.", ch, obj, nullptr, TO_ROOM);
		return;
	}

	if ((door = find_door(ch, arg)) >= 0) {
		/* 'unlock door' */
		RoomPrototype *to_room;
		Exit *pexit;
		Exit *pexit_rev;
		pexit = ch->in_room->exit[door];

		if (!pexit->exit_flags.has(EX_CLOSED)) {
			stc("It's not closed.\n", ch);
			return;
		}

		if (pexit->key < 0) {
			stc("It can't be unlocked.\n", ch);
			return;
		}

		if (!has_key(ch, pexit->key)) {
			stc("You lack the key.\n", ch);
			return;
		}

		if (!pexit->exit_flags.has(EX_LOCKED)) {
			stc("It's already unlocked.\n", ch);
			return;
		}

		pexit->exit_flags -= EX_LOCKED;
		stc("You unlock it.\n", ch);
		act("$n unlocks the $d.", ch, nullptr, pexit->keyword, TO_ROOM);

		/* unlock the other side */
		if ((to_room = pexit->u1.to_room) != nullptr
		    && (pexit_rev = to_room->exit[Exit::rev_dir(door)]) != nullptr
		    && pexit_rev->u1.to_room == ch->in_room)
			pexit_rev->exit_flags -= EX_LOCKED;
	}
}

void do_pick(Character *ch, String argument)
{
	Character *gch;
	Object *obj;
	int door;

	if (argument.empty()) {
		stc("Pick what?  Your nose!?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	WAIT_STATE(ch, skill_table[gsn_pick_lock].beats);

	/* look for guards */
	for (gch = ch->in_room->people; gch; gch = gch->next_in_room) {
		if (IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level) {
			act("$N appears to be protecting it.",
			    ch, nullptr, gch, TO_CHAR);
			return;
		}
	}

	if (!get_skill(ch, gsn_pick_lock)) {
		stc("Hmm. You seem to lack the knowledge on how to picklocks.\n", ch);
		return;
	}

	if (!deduct_stamina(ch, gsn_pick_lock))
		return;

	if (!IS_NPC(ch) && number_percent() > get_skill(ch, gsn_pick_lock)) {
		stc("You failed.\n", ch);
		check_improve(ch, gsn_pick_lock, FALSE, 2);
		return;
	}

	if ((obj = get_obj_here(ch, arg)) != nullptr) {
		/* portal stuff */
		if (obj->item_type == ITEM_PORTAL) {
			if (!obj->value[1].flags().has(EX_ISDOOR)) {
				stc("You can't do that.\n", ch);
				return;
			}

			if (!obj->value[1].flags().has(EX_CLOSED)) {
				stc("It's not closed.\n", ch);
				return;
			}

			if (obj->value[4] < 0) {
				stc("It can't be unlocked.\n", ch);
				return;
			}

			if (obj->value[1].flags().has(EX_PICKPROOF)) {
				stc("You failed.\n", ch);
				return;
			}

			obj->value[1] -= EX_LOCKED;
			act("You pick the lock on $p.", ch, obj, nullptr, TO_CHAR);
			act("$n picks the lock on $p.", ch, obj, nullptr, TO_ROOM);
			check_improve(ch, gsn_pick_lock, TRUE, 2);
			return;
		}

		/* 'pick object' */
		if (obj->item_type != ITEM_CONTAINER) {
			if (IS_OBJ_STAT(obj, ITEM_COMPARTMENT))
				stc("It's not even closed.\n", ch);
			else if (obj->contains)
				stc("It doesn't seem to have a lock.\n", ch);
			else
				stc("That's not a container.\n", ch);

			return;
		}

		if (!obj->value[1].flags().has(CONT_CLOSED))
		{ stc("It's not closed.\n",        ch); return; }

		if (obj->value[2] < 0)
		{ stc("It can't be unlocked.\n",   ch); return; }

		if (!obj->value[1].flags().has(CONT_LOCKED))
		{ stc("It's already unlocked.\n",  ch); return; }

		if (obj->value[1].flags().has(CONT_PICKPROOF))
		{ stc("Unfortunately, your skills prove to be lacking.\n", ch); return; }

		obj->value[1] -= CONT_LOCKED;
		act("You pick the lock on $p.", ch, obj, nullptr, TO_CHAR);
		act("$n picks the lock on $p.", ch, obj, nullptr, TO_ROOM);
		check_improve(ch, gsn_pick_lock, TRUE, 2);
		return;
	}

	if ((door = find_door(ch, arg)) >= 0) {
		/* 'pick door' */
		RoomPrototype *to_room;
		Exit *pexit;
		Exit *pexit_rev;
		pexit = ch->in_room->exit[door];

		if (!pexit->exit_flags.has(EX_CLOSED) && !IS_IMMORTAL(ch))
		{ stc("It's not closed.\n",        ch); return; }

		if (pexit->key < 0 && !IS_IMMORTAL(ch))
		{ stc("It can't be picked.\n",     ch); return; }

		if (!pexit->exit_flags.has(EX_LOCKED))
		{ stc("It's already unlocked.\n",  ch); return; }

		if (pexit->exit_flags.has(EX_PICKPROOF) && !IS_IMMORTAL(ch))
		{ stc("Unfortunately, your skills prove to be lacking.\n",             ch); return; }

		pexit->exit_flags -= EX_LOCKED;
		stc("You pick it!!\n", ch);
		act("$n picks the $d.", ch, nullptr, pexit->keyword, TO_ROOM);
		check_improve(ch, gsn_pick_lock, TRUE, 2);

		/* pick the other side */
		if ((to_room   = pexit->u1.to_room) != nullptr
		    && (pexit_rev = to_room->exit[Exit::rev_dir(door)]) != nullptr
		    &&   pexit_rev->u1.to_room == ch->in_room)
			pexit_rev->exit_flags -= EX_LOCKED;
	}

	return;
}

void do_stand(Character *ch, String argument)
{
	Object *obj = nullptr;

//	if (ch->on && ch->on->pIndexData->item_type == ITEM_COACH) {
//		stc("There is no room to stand up inside the coach.\n"
//		    "If you want to leave the coach, type 'alight'.\n", ch);
//		return;
//	}

	if (!argument.empty()) {
		if (get_position(ch) == POS_FIGHTING) {
			stc("This is already a standup fight!\n", ch);
			return;
		}

		obj = get_obj_list(ch, argument, ch->in_room->contents);

		if (obj == nullptr) {
			if (get_char_here(ch, argument, VIS_CHAR) != nullptr)
				stc("Is that how you treat your friends?\n", ch);
			else
				stc("You don't see that here.\n", ch);

			return;
		}

		if (obj->item_type != ITEM_FURNITURE
		    || (!obj->value[2].flags().has(STAND_AT)
		        &&   !obj->value[2].flags().has(STAND_ON)
		        &&   !obj->value[2].flags().has(STAND_IN))) {
			stc("You can't seem to find a place to stand.\n", ch);
			return;
		}

		if (ch->on != obj && count_users(obj) >= obj->value[0]) {
			act("There's no room to stand on $p.",
			        ch, obj, nullptr, TO_ROOM, POS_DEAD, FALSE);
			return;
		}
	}

	switch (get_position(ch)) {
	case POS_SLEEPING:
		if (affect_exists_on_char(ch, gsn_sleep)) {
			stc("You don't seem to want to wake up!\n", ch);
			return;
		}

		if (obj == nullptr) {
			stc("You wake and stand up.\n", ch);
			act("$n wakes and stands up.", ch, nullptr, nullptr,
			    TO_ROOM);
			ch->on = nullptr;
		}
		else if (obj->value[2].flags().has(STAND_AT)) {
			act("You wake and stand at $p.", ch, obj, nullptr, TO_CHAR, POS_DEAD, FALSE);
			act("$n wakes and stands at $p.", ch, obj, nullptr, TO_ROOM);
		}
		else if (obj->value[2].flags().has(STAND_ON)) {
			act("You wake and stand on $p.", ch, obj, nullptr, TO_CHAR, POS_DEAD, FALSE);
			act("$n wakes and stands on $p.", ch, obj, nullptr, TO_ROOM);
		}
		else {
			act("You wake and stand in $p.", ch, obj, nullptr, TO_CHAR, POS_DEAD, FALSE);
			act("$n wakes and stands in $p.", ch, obj, nullptr, TO_ROOM);
		}

		ch->position = POS_STANDING;
		do_look(ch, "auto");
		break;

	case POS_RESTING: case POS_SITTING:
		if (obj == nullptr) {
			stc("You stand up.\n", ch);
			act("$n stands up.", ch, nullptr, nullptr, TO_ROOM);
			ch->on = nullptr;
		}
		else if (obj->value[2].flags().has(STAND_AT)) {
			act("You stand at $p.", ch, obj, nullptr, TO_CHAR);
			act("$n stands at $p.", ch, obj, nullptr, TO_ROOM);
		}
		else if (obj->value[2].flags().has(STAND_ON)) {
			act("You stand on $p.", ch, obj, nullptr, TO_CHAR);
			act("$n stands on $p.", ch, obj, nullptr, TO_ROOM);
		}
		else {
			act("You stand in $p.", ch, obj, nullptr, TO_CHAR);
			act("$n stands on $p.", ch, obj, nullptr, TO_ROOM);
		}

		ch->position = POS_STANDING;
		break;

	case POS_FLYING:
		do_land(ch, "");
		return; // landing will set POS_STANDING (or not)

	case POS_STANDING:
		stc("You are already standing.\n", ch);
		break;

	case POS_FIGHTING:
		stc("This is already a standup fight!\n", ch);
		break;
	}

	ch->start_pos = POS_STANDING;
	return;
}

void do_rest(Character *ch, String argument)
{
	Object *obj = nullptr;

	if (ch->fighting) {
		stc("HEY! NO laying down on the JOB!!\n", ch);
		return;
	}

	/* okay, now that we know we can rest, find an object to rest on */
	if (!argument.empty()) {
		obj = get_obj_list(ch, argument, ch->in_room->contents);

		if (obj == nullptr) {
			if (get_char_here(ch, argument, VIS_CHAR) != nullptr)
				stc("Is that how you treat your friends?\n", ch);
			else
				stc("You don't see that here.\n", ch);

			return;
		}
	}
	else obj = ch->on;

	if (obj != nullptr) {
		if (obj->item_type != ITEM_FURNITURE
		    || (!obj->value[2].flags().has(REST_ON)
		        &&   !obj->value[2].flags().has(REST_IN)
		        &&   !obj->value[2].flags().has(REST_AT))) {
			stc("You can't rest on that.\n", ch);
			return;
		}

		if (obj != nullptr && ch->on != obj && count_users(obj) >= obj->value[0]) {
			act("There's no more room on $p.", ch, obj, nullptr, TO_CHAR, POS_DEAD, FALSE);
			return;
		}

		ch->on = obj;
	}

	switch (get_position(ch)) {
	case POS_SLEEPING:
		if (affect_exists_on_char(ch, gsn_sleep)) {
			stc("You don't seem to want to wake up!\n", ch);
			return;
		}

		if (obj == nullptr) {
			stc("You wake up and start resting.\n", ch);
			act("$n wakes up and starts resting.", ch, nullptr, nullptr, TO_ROOM);
		}
		else if (obj->value[2].flags().has(REST_AT)) {
			act("You wake up and rest at $p.",
			        ch, obj, nullptr, TO_CHAR, POS_SLEEPING, FALSE);
			act("$n wakes up and rests at $p.", ch, obj, nullptr, TO_ROOM);
		}
		else if (obj->value[2].flags().has(REST_ON)) {
			act("You wake up and rest on $p.",
			        ch, obj, nullptr, TO_CHAR, POS_SLEEPING, FALSE);
			act("$n wakes up and rests on $p.", ch, obj, nullptr, TO_ROOM);
		}
		else {
			act("You wake up and rest in $p.",
			        ch, obj, nullptr, TO_CHAR, POS_SLEEPING, FALSE);
			act("$n wakes up and rests in $p.", ch, obj, nullptr, TO_ROOM);
		}

		ch->position = POS_RESTING;
		break;

	case POS_RESTING:
		stc("You are already resting.\n", ch);
		break;

	case POS_STANDING:
		if (obj == nullptr) {
			stc("You sit down and rest.\n", ch);
			act("$n sits down and rests.", ch, nullptr, nullptr, TO_ROOM);
		}
		else if (obj->value[2].flags().has(REST_AT)) {
			act("You sit down at $p and rest.", ch, obj, nullptr, TO_CHAR);
			act("$n sits down at $p and rests.", ch, obj, nullptr, TO_ROOM);
		}
		else if (obj->value[2].flags().has(REST_ON)) {
			act("You sit on $p and rest.", ch, obj, nullptr, TO_CHAR);
			act("$n sits on $p and rests.", ch, obj, nullptr, TO_ROOM);
		}
		else {
			act("You rest in $p.", ch, obj, nullptr, TO_CHAR);
			act("$n rests in $p.", ch, obj, nullptr, TO_ROOM);
		}

		ch->position = POS_RESTING;
		break;

	case POS_SITTING:
		if (obj == nullptr) {
			stc("You lay back and rest.\n", ch);
			act("$n lies back and rests.", ch, nullptr, nullptr, TO_ROOM);
		}
		else if (obj->value[2].flags().has(REST_AT)) {
			act("You rest at $p.", ch, obj, nullptr, TO_CHAR);
			act("$n rests at $p.", ch, obj, nullptr, TO_ROOM);
		}
		else if (obj->value[2].flags().has(REST_ON)) {
			act("You rest on $p.", ch, obj, nullptr, TO_CHAR);
			act("$n rests on $p.", ch, obj, nullptr, TO_ROOM);
		}
		else {
			act("You rest in $p.", ch, obj, nullptr, TO_CHAR);
			act("$n rests in $p.", ch, obj, nullptr, TO_ROOM);
		}

		ch->position = POS_RESTING;
		break;

	case POS_FLYING:
		do_land(ch, "");

		if (ch->position == POS_FLYING)
			return; // land failed

		ch->start_pos = POS_FLYING;
		do_rest(ch, argument);
		break;
	}

	return;
}

void do_sit(Character *ch, String argument)
{
	Object *obj = nullptr;

	if (affect_exists_on_char(ch, gsn_sleep)) {
		stc("You don't seem to want to wake up!\n", ch);
		return;
	}

	if (ch->fighting) {
		stc("No sitting down during potentially terminal fights!\n", ch);
		return;
	}

	/* okay, now that we know we can sit, find an object to sit on */
	if (!argument.empty()) {
		obj = get_obj_list(ch, argument, ch->in_room->contents);

		if (obj == nullptr) {
			if (get_char_here(ch, argument, VIS_CHAR) != nullptr)
				stc("Is that how you treat your friends?\n", ch);
			else
				stc("You don't see that here.\n", ch);

			return;
		}
	}
	else obj = ch->on;

	if (obj != nullptr) {
		if (obj->item_type != ITEM_FURNITURE
		    || (!obj->value[2].flags().has(SIT_ON)
		        &&   !obj->value[2].flags().has(SIT_IN)
		        &&   !obj->value[2].flags().has(SIT_AT))) {
			stc("You can't sit on that.\n", ch);
			return;
		}

		if (obj != nullptr && ch->on != obj && count_users(obj) >= obj->value[0]) {
			act("There's no more room on $p.", ch, obj, nullptr, TO_CHAR, POS_DEAD, FALSE);
			return;
		}

		ch->on = obj;
	}

	switch (get_position(ch)) {
	case POS_SLEEPING:
		if (obj == nullptr) {
			stc("You wake and sit up.\n", ch);
			act("$n wakes and sits up.", ch, nullptr, nullptr, TO_ROOM);
		}
		else if (obj->value[2].flags().has(SIT_AT)) {
			act("You wake and sit at $p.", ch, obj, nullptr, TO_CHAR, POS_DEAD, FALSE);
			act("$n wakes and sits at $p.", ch, obj, nullptr, TO_ROOM);
		}
		else if (obj->value[2].flags().has(SIT_ON)) {
			act("You wake and sit on $p.", ch, obj, nullptr, TO_CHAR, POS_DEAD, FALSE);
			act("$n wakes and sits at $p.", ch, obj, nullptr, TO_ROOM);
		}
		else {
			act("You wake and sit in $p.", ch, obj, nullptr, TO_CHAR, POS_DEAD, FALSE);
			act("$n wakes and sits in $p.", ch, obj, nullptr, TO_ROOM);
		}

		ch->position = POS_SITTING;
		break;

	case POS_RESTING:
		if (obj == nullptr)
			stc("You stop resting.\n", ch);
		else if (obj->value[2].flags().has(SIT_AT)) {
			act("You sit at $p.", ch, obj, nullptr, TO_CHAR);
			act("$n sits at $p.", ch, obj, nullptr, TO_ROOM);
		}
		else if (obj->value[2].flags().has(SIT_ON)) {
			act("You sit on $p.", ch, obj, nullptr, TO_CHAR);
			act("$n sits on $p.", ch, obj, nullptr, TO_ROOM);
		}

		ch->position = POS_SITTING;
		break;

	case POS_SITTING:
		stc("You are already sitting.\n", ch);
		break;

	case POS_STANDING:
		if (obj == nullptr) {
			stc("You sit down.\n", ch);
			act("$n sits down.", ch, nullptr, nullptr, TO_ROOM);
		}
		else if (obj->value[2].flags().has(SIT_AT)) {
			act("You sit down at $p.", ch, obj, nullptr, TO_CHAR);
			act("$n sits down at $p.", ch, obj, nullptr, TO_ROOM);
		}
		else if (obj->value[2].flags().has(SIT_ON)) {
			act("You sit on $p.", ch, obj, nullptr, TO_CHAR);
			act("$n sits on $p.", ch, obj, nullptr, TO_ROOM);
		}
		else {
			act("You sit down in $p.", ch, obj, nullptr, TO_CHAR);
			act("$n sits down in $p.", ch, obj, nullptr, TO_ROOM);
		}

		ch->position = POS_SITTING;
		break;

	case POS_FLYING:
		do_land(ch, "");

		if (ch->position == POS_FLYING)
			return; // land failed

		do_sit(ch, argument);
		ch->start_pos = POS_FLYING;
		break;
	}

	return;
}

void do_sleep(Character *ch, String argument)
{
	Object *obj = nullptr;

	if (GET_ROOM_FLAGS(ch->in_room).has(ROOM_NOSLEEP)) {
		stc("Hmmm...you can't seem to fall asleep in this room.\n", ch);
		return;
	}

	switch (get_position(ch)) {
	case POS_SLEEPING:
		stc("You are already asleep.\n", ch);
		break;

	case POS_RESTING:
	case POS_SITTING:
	case POS_STANDING:
		if (argument.empty() && ch->on == nullptr) {
			stc("You go to sleep.\n", ch);
			act("$n goes to sleep.", ch, nullptr, nullptr, TO_ROOM);
			ch->position = POS_SLEEPING;
		}
		else { /* find an object and sleep on it */
			if (argument.empty())
				obj = ch->on;
			else
				obj = get_obj_list(ch, argument,  ch->in_room->contents);

			if (obj == nullptr) {
				if (get_char_here(ch, argument, VIS_CHAR) != nullptr)
					stc("Is that how you treat your friends?\n", ch);
				else
					stc("You don't see that here.\n", ch);

				return;
			}

			if (obj->item_type != ITEM_FURNITURE || // && obj->item_type != ITEM_COACH) ||
			   (!obj->value[2].flags().has(SLEEP_ON)
			        &&   !obj->value[2].flags().has(SLEEP_IN)
			        &&   !obj->value[2].flags().has(SLEEP_AT))) {
				stc("You can't sleep on that!\n", ch);
				return;
			}

			if (ch->on != obj && count_users(obj) >= obj->value[0]) {
				act("There is no room on $p for you.",
				        ch, obj, nullptr, TO_CHAR, POS_DEAD, FALSE);
				return;
			}

			ch->on = obj;

			if (obj->value[2].flags().has(SLEEP_AT)) {
				act("You go to sleep at $p.", ch, obj, nullptr, TO_CHAR);
				act("$n goes to sleep at $p.", ch, obj, nullptr, TO_ROOM);
			}
			else if (obj->value[2].flags().has(SLEEP_ON)) {
				act("You go to sleep on $p.", ch, obj, nullptr, TO_CHAR);
				act("$n goes to sleep on $p.", ch, obj, nullptr, TO_ROOM);
			}
			else {
				act("You go to sleep in $p.", ch, obj, nullptr, TO_CHAR);
				act("$n goes to sleep in $p.", ch, obj, nullptr, TO_ROOM);
			}

			ch->position = POS_SLEEPING;
		}

		break;

	case POS_FIGHTING:
		stc("I doubt this fight is THAT boring!\n", ch);
		break;

	case POS_FLYING:
		do_land(ch, "");

		if (ch->position == POS_FLYING)
			return; // land failed

		do_sleep(ch, argument);
		ch->start_pos = POS_FLYING;
		break;
	}

	return;
}

void do_wake(Character *ch, String argument)
{
	Character *victim;

	if (argument.empty()) {
//		if (ch->on && ch->on->pIndexData->item_type == ITEM_COACH)
//			do_sit(ch, "");
//		else

		if (ch->start_pos == POS_FLYING && CAN_FLY(ch))
			do_fly(ch, "");
		else
			do_stand(ch, "");

		return;
	}

	String arg;
	one_argument(argument, arg);

	if (!IS_AWAKE(ch))
	{ stc("In your sleep?\n",       ch); return; }

	if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr)
	{ stc("Can't wake 'em if they are not here!\n", ch); return; }

	if (IS_AWAKE(victim))
	{ act("$N is as awake as you are.", ch, nullptr, victim, TO_CHAR); return; }

	if (affect_exists_on_char(victim, gsn_sleep))
	{ act("$E doesn't seem to WANT to wake up!",   ch, nullptr, victim, TO_CHAR);  return; }

	act("$n rudely awakes you from your peaceful slumber.",
	        ch, nullptr, victim, TO_VICT, POS_SLEEPING, FALSE);

	if (victim->start_pos == POS_FLYING && CAN_FLY(victim))
		do_fly(victim, "");
	else
		do_stand(victim, "");
}

void do_sneak(Character *ch, String argument)
{
	if (affect_exists_on_char(ch, gsn_sneak)) {
		stc("You already surpass the wind in stealth.\n", ch);
		return;
	}

	WAIT_STATE(ch, skill_table[gsn_sneak].beats);

	if (!get_skill(ch, gsn_sneak)) {
		stc("But you don't know how to sneak!\n", ch);
		return;
	}

	if (!deduct_stamina(ch, gsn_sneak))
		return;

	if (number_percent() < get_skill(ch, gsn_sneak)) {
		affect_add_sn_to_char(ch,
			gsn_sneak,
			ch->level,
			ch->level,
			get_evolution(ch, gsn_sneak),
			FALSE
		);
		stc("You feel more stealthy.\n", ch);
		check_improve(ch, gsn_sneak, TRUE, 3);
	}
	else {
		stc("You feel like a klutz.\n", ch);
		check_improve(ch, gsn_sneak, FALSE, 3);
	}
}

void do_hide(Character *ch, String argument)
{
	if (affect_exists_on_char(ch, gsn_hide)) {
		stc("You find an even better hiding place.\n", ch);
		return;
	}

	WAIT_STATE(ch, skill_table[gsn_hide].beats);

	if (!get_skill(ch, gsn_hide)) {
		stc("But you don't know how to hide!\n", ch);
		return;
	}

	if (!deduct_stamina(ch, gsn_hide))
		return;

	if (number_percent() < get_skill(ch, gsn_hide)) {
		affect_add_sn_to_char(ch,
			gsn_hide,
			ch->level,
			ch->level,
			get_evolution(ch, gsn_hide),
			FALSE
		);
		stc("You blend into the surroundings.\n", ch);
		check_improve(ch, gsn_hide, TRUE, 3);
	}
	else {
		stc("You attempt to be inconspicuous.\n", ch);
		check_improve(ch, gsn_hide, FALSE, 3);
	}
}

/*
 * Contributed by Alander.
 */
void do_visible(Character *ch, String argument)
{
	affect_remove_sn_from_char(ch, gsn_invis);
	affect_remove_sn_from_char(ch, gsn_sneak);
	affect_remove_sn_from_char(ch, gsn_hide);
	affect_remove_sn_from_char(ch, gsn_midnight);
	ch->act_flags -= PLR_SUPERWIZ;
	ch->invis_level = 0;
	ch->lurk_level = 0;
	stc("You are now visible.\n", ch);
	return;
}

void do_recall(Character *ch, String argument)
{
	recall(ch, FALSE);
}

void do_clan_recall(Character *ch, String argument)
{
	/* This looks really ugly, so I'm re-writing it. -- Outsider
	if ((!is_clan(ch) && !ch->act_flags.has(ACT_PET)) || ch->clan == nullptr)
	{
	     stc("You do not belong to a clan.\n",ch);
	     return;
	}
	*/

	/* Make sure we belong to a clan OR we are a pet. */
	if (is_clan(ch) || ch->act_flags.has(ACT_PET)) {
		/* Make sure we have a valid clan OR we are a pet */
		if (ch->clan || ch->act_flags.has(ACT_PET)) {
			recall(ch, TRUE);
			return;
		}
	}

	/* We either aren't a clan memeber or not a pet. */
	stc("You do not belong to a clan.\n", ch);
	return;
	/* recall(ch, TRUE); */
}

void recall(Character *ch, bool clan)
{
	RoomPrototype *location;
	bool pet = FALSE, combat = FALSE;
	int lose = 0;

	if (ch->in_room == nullptr)
		return;

	if (IS_NPC(ch)) {
		if (ch->act_flags.has(ACT_PET) && ch->master != nullptr)
			pet = TRUE;
		else {
			ptc(ch, "Only players and pets can %srecall.\n", clan ? "clan" : "");
			return;
		}
	}

	act("$n prays for divine transportation...", ch, nullptr, nullptr, TO_ROOM);

	if (char_in_duel(ch)) {
		ptc(ch, "You cannot %srecall from the duel arena.\n", clan ? "clan" : "");
		return;
	}

	if (!IS_NPC(ch))
		if (ch->pcdata->pktimer) {
			stc("The gods laugh at your cowardice...\n", ch);
			return;
		}

	/* default locations, can be changed in the following ifs */
	if (clan) {
		if (pet)
			location = get_room_index(ch->master->clan->hall);
		else
			location = get_room_index(ch->clan->hall);
	}
	else
		location = get_room_index(ROOM_VNUM_TEMPLE);

	/* Recall from Clan's Arena */
	if (ch->in_room->sector_type == SECT_CLANARENA && !clan && ch->fighting) {
		if (pet)
			location = get_room_index(ch->master->clan->hall);
		else
			location = get_room_index(ch->clan->hall);

		if (location == nullptr)
			location = get_room_index(ROOM_VNUM_TEMPLE);
	}
	else if (ch->in_room->sector_type == SECT_ARENA) {
		Descriptor *d;
		bool empty = TRUE;

		for (d = descriptor_list; d != nullptr; d = d->next)
			if (IS_PLAYING(d)
			    && d->character != ch
			    && d->character != ch->pet     /* just in case */
			    && !IS_IMMORTAL(d->character)  /* exclude imms */
			    && d->character->in_room != nullptr
			    && d->character->in_room->area == ch->in_room->area)
				empty = FALSE;

		if (empty && ch->fighting == nullptr && (battle.start || !battle.issued)) { /* last person in arena? */
			if (location == nullptr) {
				stc("You are completely lost.\n", ch);
				return;
			}

			stc("\n{YAfter winning the great battle,\n"
			    "you call upon the power of the gods!{x\n\n", ch);
			act("$n disappears in a victory cloud!", ch, nullptr, nullptr, TO_ROOM);    /* for imms in arena */
			char_from_room(ch);
			char_to_room(ch, location);
			act("$n arrives from a victory cloud!", ch, nullptr, nullptr, TO_ROOM);
			do_look(ch, "auto");

			/* Bring the pet along */
			if (ch->pet != nullptr && ch->pet != ch->master)   /* avoid a nasty loop */
				do_recall(ch->pet, "");

			return;
		}

		location = get_room_index(ROOM_VNUM_ARENACENTER);
	}

	if (location == nullptr) {
		stc("You are completely lost.\n", ch);
		return;
	}

	if (ch->in_room == location) {
		stc("You are already there.\n", ch);
		return;
	}

	if ((!IS_IMMORTAL(ch) && GET_ROOM_FLAGS(ch->in_room).has(ROOM_NO_RECALL)) || affect_exists_on_char(ch, gsn_curse)) {
		stc("Unsympathetic laughter of the Gods plays upon your ears.\n", ch);
		return;
	}

	if (ch->fighting != nullptr) {
		if (number_percent() < 80 * get_skill(ch, gsn_recall) / 100) {
			check_improve(ch, gsn_recall, FALSE, 6);
			WAIT_STATE(ch, 4);
			stc("The Gods ignore your hasty prayers.\n", ch);
			return;
		}

		if (ch->in_room->sector_type != SECT_ARENA
		    && ch->in_room->sector_type != SECT_CLANARENA)
			lose = (ch->desc != nullptr ? 25 : (clan ? 50 : 25));

		combat = TRUE;
		gain_exp(ch, 0 - lose);
		check_improve(ch, gsn_recall, TRUE, 4);
		stop_fighting(ch, TRUE);
	}

	/* remort affect - buggy recall */
	if (HAS_RAFF(ch, RAFF_BUGGYREC) && chance(5))
		location = get_random_room(ch);

	if (!ch->in_room->clan || ch->in_room->clan != ch->clan)
		ch->stam = (ch->stam * 3) / 4;

	if (clan)
		act("$n disappears.", ch, nullptr, nullptr, TO_ROOM);
	else
		act("$n disappears in a mushroom cloud.", ch, nullptr, nullptr, TO_ROOM);

	char_from_room(ch);
	char_to_room(ch, location);
	do_look(ch, "auto");

	if (combat) { /* delayed to put message after transfer */
		ptc(ch, "You %sRECALL from combat!", clan ? "CLAN" : "");

		if (lose) ptc(ch, "  You lose %d experience.", lose);

		stc("\n", ch);
	}

	if (clan)
		act("$n appears in the room.", ch, nullptr, nullptr, TO_ROOM);
	else
		act("$n arrives from a mushroom cloud.", ch, nullptr, nullptr, TO_ROOM);

	if (ch->pet != nullptr && ch->pet != ch->master) {
		if (clan)
			do_clan_recall(ch->pet, "");
		else
			do_recall(ch->pet, "");
	}
}

void do_train(Character *ch, String argument)
{
	String buf;
	Character *mob;
	sh_int stat = - 1;
	char *pOutput = nullptr;
	int cost, add;

	if (IS_NPC(ch))
		return;

	/*
	 * Check for trainer.
	 */
	for (mob = ch->in_room->people; mob; mob = mob->next_in_room) {
		if (IS_NPC(mob) && mob->act_flags.has(ACT_TRAIN))
			break;
	}

	if (mob == nullptr) {
		stc("You can't do that here.\n", ch);
		return;
	}

	if (argument.empty()) {
		Format::sprintf(buf, "You have %d training sessions.\n", ch->train);
		stc(buf, ch);
		argument = "foo";
	}

	cost = 1;

	if (argument == "str") {
		if (class_table[ch->cls].stat_prime == STAT_STR)
			cost    = 1;

		stat        = STAT_STR;
		pOutput     = "strength";
	}
	else if (argument == "int") {
		if (class_table[ch->cls].stat_prime == STAT_INT)
			cost    = 1;

		stat        = STAT_INT;
		pOutput     = "intelligence";
	}
	else if (argument == "wis") {
		if (class_table[ch->cls].stat_prime == STAT_WIS)
			cost    = 1;

		stat        = STAT_WIS;
		pOutput     = "wisdom";
	}
	else if (argument == "dex") {
		if (class_table[ch->cls].stat_prime == STAT_DEX)
			cost    = 1;

		stat        = STAT_DEX;
		pOutput     = "dexterity";
	}
	else if (argument == "con") {
		if (class_table[ch->cls].stat_prime == STAT_CON)
			cost    = 1;

		stat        = STAT_CON;
		pOutput     = "constitution";
	}
	else if (argument == "chr") {
		if (class_table[ch->cls].stat_prime == STAT_CHR)
			cost    = 1;

		stat        = STAT_CHR;
		pOutput     = "charisma";
	}
	else if (argument == "hp")
		cost = 1;
	else if (argument == "mana")
		cost = 1;
	else if (argument == "stamina")
		cost = 1;
	else {
		buf = "You can train:";

		if (ATTR_BASE(ch, APPLY_STR) < get_max_train(ch, STAT_STR))
			buf += " str";

		if (ATTR_BASE(ch, APPLY_INT) < get_max_train(ch, STAT_INT))
			buf += " int";

		if (ATTR_BASE(ch, APPLY_WIS) < get_max_train(ch, STAT_WIS))
			buf += " wis";

		if (ATTR_BASE(ch, APPLY_DEX) < get_max_train(ch, STAT_DEX))
			buf += " dex";

		if (ATTR_BASE(ch, APPLY_CON) < get_max_train(ch, STAT_CON))
			buf += " con";

		if (ATTR_BASE(ch, APPLY_CHR) < get_max_train(ch, STAT_CHR))
			buf += " chr";

		buf += " hp mana stamina.\n";
		stc(buf, ch);
		return;
	}

	if (argument == "hp") {
		if (ch->pcdata->trains_to_hit < 50)             add = 10;
		else if (ch->pcdata->trains_to_hit < 100)       add = 9;
		else if (ch->pcdata->trains_to_hit < 150)       add = 8;
		else if (ch->pcdata->trains_to_hit < 200)       add = 7;
		else if (ch->pcdata->trains_to_hit < 250)       add = 6;
		else if (ch->pcdata->trains_to_hit < 300)       add = 5;
		else if (ch->pcdata->trains_to_hit < 350)       add = 4;
		else if (ch->pcdata->trains_to_hit < 400)       add = 3;
		else if (ch->pcdata->trains_to_hit < 450)       add = 2;
		else                                            add = 1;

		if (cost > ch->train) {
			stc("You don't have enough training sessions.\n", ch);
			return;
		}

		ch->train -= cost;
		ch->pcdata->trains_to_hit += 1;
		ATTR_BASE(ch, APPLY_HIT) += add;
		ch->hit += add;
		act("Your durability increases!", ch, nullptr, nullptr, TO_CHAR);
		act("$n's durability increases!", ch, nullptr, nullptr, TO_ROOM);
		return;
	}

	if (argument == "mana") {
		if (ch->pcdata->trains_to_mana < 50)            add = 10;
		else if (ch->pcdata->trains_to_mana < 100)      add = 9;
		else if (ch->pcdata->trains_to_mana < 150)      add = 8;
		else if (ch->pcdata->trains_to_mana < 200)      add = 7;
		else if (ch->pcdata->trains_to_mana < 250)      add = 6;
		else if (ch->pcdata->trains_to_mana < 300)      add = 5;
		else if (ch->pcdata->trains_to_mana < 350)      add = 4;
		else if (ch->pcdata->trains_to_mana < 400)      add = 3;
		else if (ch->pcdata->trains_to_mana < 450)      add = 2;
		else                                            add = 1;

		if (cost > ch->train) {
			stc("You don't have enough training sessions.\n", ch);
			return;
		}

		ch->train -= cost;
		ch->pcdata->trains_to_mana += 1;
		ATTR_BASE(ch, APPLY_MANA) += add;
		ch->mana += add;
		act("Your power increases!", ch, nullptr, nullptr, TO_CHAR);
		act("$n's power increases!", ch, nullptr, nullptr, TO_ROOM);
		return;
	}

	if (argument == "stamina") {
		if (ch->pcdata->trains_to_stam < 50)            add = 10;
		else if (ch->pcdata->trains_to_stam < 100)      add = 9;
		else if (ch->pcdata->trains_to_stam < 150)      add = 8;
		else if (ch->pcdata->trains_to_stam < 200)      add = 7;
		else if (ch->pcdata->trains_to_stam < 250)      add = 6;
		else if (ch->pcdata->trains_to_stam < 300)      add = 5;
		else if (ch->pcdata->trains_to_stam < 350)      add = 4;
		else if (ch->pcdata->trains_to_stam < 400)      add = 3;
		else if (ch->pcdata->trains_to_stam < 450)      add = 2;
		else                                            add = 1;

		if (cost > ch->train) {
			stc("You don't have enough training sessions.\n", ch);
			return;
		}

		ch->train -= cost;
		ch->pcdata->trains_to_stam += 1;
		ATTR_BASE(ch, APPLY_STAM) += add;
		ch->stam += add;
		act("Your energy increases!", ch, nullptr, nullptr, TO_CHAR);
		act("$n's energy increases!", ch, nullptr, nullptr, TO_ROOM);
		return;
	}

	if (ATTR_BASE(ch, stat_to_attr(stat)) >= get_max_train(ch, stat)) {
		act("Your $T is already at maximum.", ch, nullptr, pOutput, TO_CHAR);
		return;
	}

	if (cost > ch->train) {
		stc("You don't have enough training sessions.\n", ch);
		return;
	}

	ch->train            -= cost;
	ATTR_BASE(ch, stat_to_attr(stat)) += 1;
	act("Your $T increases!", ch, nullptr, pOutput, TO_CHAR);
	act("$n's $T increases!", ch, nullptr, pOutput, TO_ROOM);
	return;
}

/* function for checking legality of push/drag */
bool is_safe_drag(Character *ch, Character *victim)
{
	if (victim->in_room == nullptr || ch->in_room == nullptr)
		return TRUE;

	if (ch->fighting || victim->fighting) {
		stc("Wait for the fight to finish!\n", ch);
		return TRUE;
	}

	if (ch == victim) {
		stc("You like playing with yourself?\n", ch);
		return TRUE;
	}

	if (IS_IMMORTAL(victim)) {
		stc("You cannot do that to immortals.\n", ch);
		return TRUE;
	}

	if (IS_IMMORTAL(ch))
		return FALSE;

	/* safe room? */
	if (GET_ROOM_FLAGS(victim->in_room).has(ROOM_SAFE)
	    && (IS_NPC(victim) || victim->pcdata->pktimer <= 0)) {
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

	return is_safe_char(ch, victim, TRUE);
}

void do_push(Character *ch, String argument)
{
	char buf[MIL], dir_buf[MSL];
	RoomPrototype *to_room;
	Exit *pexit;
	Character *victim;
	int dir;

	if (argument.empty()) {
		stc("Whom do you want to push?\n", ch);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (is_safe_drag(ch, victim))
		return;

	if (!IS_AWAKE(victim)) {
		act("$N is asleep. Try dragging $M.", ch, nullptr, victim, TO_CHAR);
		return;
	}

	if (get_position(victim) <= POS_RESTING) {
		act("$N is resting. Try dragging $M.", ch, nullptr, victim, TO_CHAR);
		return;
	}

	/* determine if its a valid direction, we'll deal with exits below */
	if (argument.is_prefix_of("north"))       dir = 0;
	else if (argument.is_prefix_of("east"))        dir = 1;
	else if (argument.is_prefix_of("south"))       dir = 2;
	else if (argument.is_prefix_of("west"))        dir = 3;
	else if (argument.is_prefix_of("up"))          dir = 4;
	else if (argument.is_prefix_of("down"))        dir = 5;
	else {
		stc("No such direction.\n", ch);
		return;
	}

	/* here's the chance of failure */
	if (!IS_IMMORTAL(ch)) {
		int chance = 75;
		chance += (GET_ATTR_STR(ch) - GET_ATTR_STR(victim)) * 5;
		chance = URANGE(5, chance, 95);

		if (!chance(chance)) {
			act("$n tries unsuccessfully to push $N.", ch, nullptr, victim, TO_NOTVICT);
			act("$n tries to push you.", ch, nullptr, victim, TO_VICT);
			act("$N looks at you with contempt and ignores you.", ch, nullptr, victim, TO_CHAR);

			if (!GET_ROOM_FLAGS(victim->in_room).has(ROOM_SAFE))
				multi_hit(victim, ch, TYPE_UNDEFINED);

			return;
		}
	}

	/* no exit from the room? */
	if ((pexit   = ch->in_room->exit[dir]) == nullptr
	    || (to_room = pexit->u1.to_room) == nullptr
	    || !can_see_room(ch, pexit->u1.to_room)) {
		act("You shove $M up against a wall and threaten $M.", ch, nullptr, victim, TO_CHAR);
		act("$n shoves you up against a wall and threatens you.", ch, nullptr, victim, TO_VICT);
		act("$n shoves $N up against a wall and threatens $M.", ch, nullptr, victim, TO_NOTVICT);
		return;
	}

	if (GET_ROOM_FLAGS(to_room).has(ROOM_LAW)
	    && (IS_NPC(victim) && victim->act_flags.has(ACT_AGGRESSIVE))) {
		stc("They are too ill-tempered to have in the city.\n", ch);
		return;
	}

	if (!IS_NPC(ch)
	    && to_room->guild
	    && to_room->guild != victim->cls + 1) {
		stc("They are not a member, they cannot enter.\n", ch);
		return;
	}

	/* exit is impassible? */
	if (pexit->exit_flags.has(EX_CLOSED)
	    && (!affect_exists_on_char(victim, gsn_pass_door)
	        || pexit->exit_flags.has(EX_NOPASS))) {
		Format::sprintf(buf, "You shove $M up against the %s and threaten $M.", pexit->keyword);
		act(buf, ch, nullptr, victim, TO_CHAR);
		Format::sprintf(buf, "$n shoves you up against the %s and threatens you.", pexit->keyword);
		act(buf, ch, nullptr, victim, TO_VICT);
		Format::sprintf(buf, "$n shoves $N up against the %s and threatens $M.", pexit->keyword);
		act(buf, ch, nullptr, victim, TO_NOTVICT);
		return;
	}

	if (room_is_private(to_room)) {
		stc("That room is private right now.\n", ch);
		return;
	}

	WAIT_STATE(ch, 3);
	Format::sprintf(buf, "$n pushes you %s!\n", Exit::dir_name(dir));
	act(buf, ch, nullptr, victim, TO_VICT);
	Format::sprintf(buf, "$n pushes $N %s!", Exit::dir_name(dir));
	act(buf, ch, nullptr, victim, TO_NOTVICT);
	Format::sprintf(buf, "You push $N %s.", Exit::dir_name(dir));
	act(buf, ch, nullptr, victim, TO_CHAR);
	char_from_room(victim);
	char_to_room(victim, to_room);

	if (Exit::rev_dir(dir) == 5)
		Format::sprintf(dir_buf, "%s", "below");
	else if (Exit::rev_dir(dir) == 4)
		Format::sprintf(dir_buf, "%s", "above");
	else
		Format::sprintf(dir_buf, "the %s", Exit::dir_name(dir, true));

	if (GET_ROOM_FLAGS(ch->in_room).has(ROOM_UNDER_WATER)
	    && !GET_ROOM_FLAGS(victim->in_room).has(ROOM_UNDER_WATER))
		stc("{CYou gasp for air!{x\n", victim);

	if (GET_ROOM_FLAGS(victim->in_room).has(ROOM_UNDER_WATER)) {
		if (GET_ROOM_FLAGS(ch->in_room).has(ROOM_UNDER_WATER)) {
			stc("{CYou continue to hold your breath...{x\n", victim);
			Format::sprintf(buf, "$N floats in from %s.", dir_buf);
			act(buf, ch, nullptr, victim, TO_NOTVICT);
		}
		else {
			stc("{CYou begin to hold your breath.{x\n", victim);
			Format::sprintf(buf, "$N spashes in from %s.", dir_buf);
			act(buf, ch, nullptr, victim, TO_NOTVICT);
		}
	}
	else if (victim->in_room->sector_type == SECT_AIR) {
		if (!IS_FLYING(victim)
		    && victim->in_room->exit[DIR_DOWN]) {

			Format::sprintf(buf, "$n stumbles into the emptiness from %s.", dir_buf);
			act(buf, victim, nullptr, nullptr, TO_ROOM);

			// try to fly
			if (CAN_FLY(victim))
				do_fly(victim, "");

			if (!IS_FLYING(victim)) {
				int count = 0;  /* just to prevent an infinite loop */
				long brief = victim->comm_flags.has(COMM_BRIEF);
				victim->comm_flags += COMM_BRIEF;

				while (victim->in_room->sector_type == SECT_AIR
				       && !GET_ROOM_FLAGS(victim->in_room).has(ROOM_UNDER_WATER)
				       && victim->in_room->exit[DIR_DOWN]
				       && (to_room = victim->in_room->exit[DIR_DOWN]->u1.to_room)
				       && count++ < 10) {
					RoomPrototype *around, *old = victim->in_room;
					act("$n screams and falls down...", victim, nullptr, nullptr, TO_ROOM);
					do_look(victim, "auto");
					char_from_room(victim);

					/* echo in the cardinal directions */
					for (dir = 0; dir < 4; dir++) {
						if (old->exit[dir] == nullptr
						    || (around = old->exit[dir]->u1.to_room) == nullptr
						    || around->exit[Exit::rev_dir(dir)] == nullptr
						    || around->exit[Exit::rev_dir(dir)]->u1.to_room != old)
							continue;

						char_to_room(victim, old->exit[dir]->u1.to_room);
						Format::sprintf(buf, "You hear a scream from the %s, as if someone were falling...",
						        Exit::dir_name(dir, true));
						act(buf, victim, nullptr, nullptr, TO_ROOM);
						char_from_room(victim);
					}

					char_to_room(victim, old);

					if (count == 1)
						stc("\nYou scream as you realize there is air under your feet, and fall down!\n\n", victim);
					else
						stc("\nYou keep falling....\n\n", victim);

					char_from_room(victim);
					char_to_room(victim, to_room);
					act("You hear a scream, and look up to see $n hurtling in from above!",
					    victim, nullptr, nullptr, TO_ROOM);
				}

				if (!brief)
					victim->comm_flags -= COMM_BRIEF;

				if (victim->in_room->sector_type == SECT_WATER_NOSWIM
				    || victim->in_room->sector_type == SECT_WATER_SWIM
				    || GET_ROOM_FLAGS(victim->in_room).has(ROOM_UNDER_WATER)) {
					stc("You spash down HARD in the water.  OW!!\n\n", victim);
					act("$n spashes down HARD in the water.", victim, nullptr, nullptr, TO_ROOM);

					if (GET_ROOM_FLAGS(victim->in_room).has(ROOM_UNDER_WATER))
						stc("{CYou begin to hold your breath.{x\n", victim);
				}
				else {
					stc("The ground finally breaks your fall.  OW!!\n\n", victim);
					act("$n crash lands HARD on the ground.", victim, nullptr, nullptr, TO_ROOM);
				}
			}
		}
		else {
			Format::sprintf(buf, "$n floats in from %s.", dir_buf);
			act(buf, victim, nullptr, nullptr, TO_ROOM);
		}
	}
	else {
		Format::sprintf(buf, "$n is stumbles in from %s.", dir_buf);
		act(buf, victim, nullptr, nullptr, TO_ROOM);
	}

	do_look(victim, "auto");
	mprog_entry_trigger(victim);
	mprog_greet_trigger(victim);
}

void do_drag(Character *ch, String argument)
{
	char buf[MIL], dir_buf[MSL];
	RoomPrototype *to_room, *from_room;
	Exit *pexit;
	Character *victim;
	int dir, cost;

	if (argument.empty()) {
		stc("Whom do you want to drag?\n", ch);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (is_safe_drag(ch, victim))
		return;

	if (get_position(victim) > POS_RESTING) {
		act("$N is standing. Try pushing $M.", ch, nullptr, victim, TO_CHAR);
		return;
	}

	if (affect_exists_on_char(ch, gsn_charm_person)
	    && ch->master != nullptr
	    && victim->in_room == ch->master->in_room) {
		stc("What?  And leave your beloved master?\n", ch);
		return;
	}

	/* determine if its a valid direction, we'll deal with exits below */
	if (argument.is_prefix_of("north"))       dir = 0;
	else if (argument.is_prefix_of("east"))        dir = 1;
	else if (argument.is_prefix_of("south"))       dir = 2;
	else if (argument.is_prefix_of("west"))        dir = 3;
	else if (argument.is_prefix_of("up"))          dir = 4;
	else if (argument.is_prefix_of("down"))        dir = 5;
	else {
		stc("No such direction.\n", ch);
		return;
	}

	/* here's the chance of failure */
	if (!IS_IMMORTAL(ch)) {
		int chance = GET_ATTR_STR(ch) * 4;
		chance -= (victim->size - ch->size) * 30;
		chance -= get_carry_weight(victim) / 60;
		/* unlike push, it is possible for there to be *no* chance of dragging */
		chance = URANGE(0, chance, 95);

		if (!chance(chance)) {
			act("You strain your muscles, but fail to move $N!", ch, nullptr, victim, TO_CHAR);
			act("$n strains $s muscles, but can't seem to drag $N.", ch, nullptr, victim, TO_NOTVICT);

			if (IS_AWAKE(victim)) {
				act("$n tries to drag you, but is not strong enough.", ch, nullptr, victim, TO_VICT);

				if (!GET_ROOM_FLAGS(victim->in_room).has(ROOM_SAFE))
					multi_hit(victim, ch, TYPE_UNDEFINED);
			}

			return;
		}
	}

	/* no exit from the room? */
	if ((pexit   = ch->in_room->exit[dir]) == nullptr
	    || (to_room = pexit->u1.to_room) == nullptr
	    || !can_see_room(ch, pexit->u1.to_room)) {
		act("You drag $M around the room.", ch, nullptr, victim, TO_CHAR);
		act("$n drags $N around the room!", ch, nullptr, victim, TO_NOTVICT);

		if (IS_AWAKE(victim))
			act("$n drags you around the room!", ch, nullptr, victim, TO_VICT);
		else
			stc("You have nightmares about your head being slammed into a wall.\n", victim);

		return;
	}

	if (room_is_private(to_room)) {
		stc("Sorry, that room is private.\n", ch);
		return;
	}

	if (to_room->guild) {
		stc("You cannot drag people into a guild room.\n", ch);
		return;
	}

	if (to_room->sector_type == SECT_AIR
	 && !IS_FLYING(ch)
	 && !IS_IMMORTAL(ch)) {
		if (CAN_FLY(ch))
			do_fly(ch, "");
		else
			stc("You cannot fly!\n", ch);

		if (!IS_FLYING(ch))
			return;
	}

	if (GET_ROOM_FLAGS(to_room).has(ROOM_LAW)) {
		if (IS_NPC(ch) && ch->act_flags.has(ACT_AGGRESSIVE)) {
			stc("They don't want your 'type' in there.\n", ch);
			return;
		}

		if (IS_NPC(victim) && victim->act_flags.has(ACT_AGGRESSIVE)) {
			stc("They are too ill-tempered to have in the city.\n", ch);
			return;
		}
	}

	from_room = ch->in_room;
	cost = (stamina_loss[UMIN(SECT_MAX - 1, from_room->sector_type)]
	        + stamina_loss[UMIN(SECT_MAX - 1, to_room->sector_type)]);

	/* conditional effects */
	if (IS_FLYING(ch) || affect_exists_on_char(ch, gsn_haste))
		cost /= 2;

	if (affect_exists_on_char(ch, gsn_slow))
		cost *= 2;

	/* remort affect - light feet */
	if (HAS_RAFF(ch, RAFF_LIGHTFEET))
		cost /= 2;

	if (ch->stam < cost) {
		stc("You are too exhausted for that.\n", ch);
		return;
	}

	ch->stam -= cost;

	if (!IS_AWAKE(victim))
		stc("You dream about rugburns.\n", victim);

	/* exit is impassible? */
	if (pexit->exit_flags.has(EX_CLOSED)) {
		if (!affect_exists_on_char(ch, gsn_pass_door)
		    || pexit->exit_flags.has(EX_NOPASS)) {
			ptc(ch, "You back into the %s.\n", pexit->keyword);
			Format::sprintf(buf, "$n tries to drag $N, but backs into the %s.", pexit->keyword);
			act(buf, ch, nullptr, victim, TO_NOTVICT);

			if (IS_AWAKE(victim)) {
				Format::sprintf(buf, "$n tries to drag you, but backs into the %s.", pexit->keyword);
				act(buf, ch, nullptr, victim, TO_VICT);
			}

			return;
		}

		if (!affect_exists_on_char(victim, gsn_pass_door)
		    || pexit->exit_flags.has(EX_NOPASS)) {
			ptc(ch, "You try to drag them through the %s, but they are too solid.\n", pexit->keyword);
			Format::sprintf(buf, "$n tries to drag $N, but $E bangs against the %s.", pexit->keyword);
			act(buf, ch, nullptr, victim, TO_NOTVICT);

			if (IS_AWAKE(victim)) {
				Format::sprintf(buf, "$n tries to drag you, but you are too solid to pass through the %s.", pexit->keyword);
				act(buf, ch, nullptr, victim, TO_VICT);
			}
			else
				stc("You have nightmares about your head banging against walls.\n", victim);

			return;
		}
	}

	if (room_is_private(to_room)) {
		stc("That room is private right now.\n", ch);
		return;
	}

	WAIT_STATE(ch, 3);
	/* act should default to resting position minimum */
	Format::sprintf(buf, "$n drags you %s!", Exit::dir_name(dir));
	act(buf, ch, nullptr, victim, TO_VICT);
	Format::sprintf(buf, "$n drags $N %s!", Exit::dir_name(dir));
	act(buf, ch, nullptr, victim, TO_NOTVICT);
	Format::sprintf(buf, "You drag $N %s.", Exit::dir_name(dir));
	act(buf, ch, nullptr, victim, TO_CHAR);
	char_from_room(victim);
	char_from_room(ch);
	char_to_room(victim, to_room);
	char_to_room(ch, to_room);

	if (Exit::rev_dir(dir) == 5)
		Format::sprintf(dir_buf, "%s", "below");
	else if (Exit::rev_dir(dir) == 4)
		Format::sprintf(dir_buf, "%s", "above");
	else
		Format::sprintf(dir_buf, "the %s", Exit::dir_name(dir, true));

	if (GET_ROOM_FLAGS(from_room).has(ROOM_UNDER_WATER)
	    && !GET_ROOM_FLAGS(victim->in_room).has(ROOM_UNDER_WATER)) {
		act("{CYou gasp for air!{x\n", ch, nullptr, victim, TO_CHAR);
		act("{CYou gasp for air!{x\n", ch, nullptr, victim, TO_VICT);
	}

	if (GET_ROOM_FLAGS(victim->in_room).has(ROOM_UNDER_WATER)) {
		if (GET_ROOM_FLAGS(from_room).has(ROOM_UNDER_WATER)) {
			stc("{CYou continue to hold your breath...{x\n", ch);
			act("{CYou continue to hold your breath...{x", victim, nullptr, nullptr, TO_CHAR);
			Format::sprintf(buf, "$n swims in from %s, dragging $N behind.", dir_buf);
			act(buf, ch, nullptr, victim, TO_NOTVICT);
		}
		else {
			stc("{CYou begin to hold your breath.{x\n", ch);
			Format::sprintf(buf, "$n spashes in from %s, dragging $N behind.", dir_buf);
			act(buf, ch, nullptr, victim, TO_NOTVICT);

			if (!IS_AWAKE(victim)) {
				if (affect_exists_on_char(victim, gsn_sleep)) {
					if (chance(40)) {
						affect_remove_sn_from_char(victim, gsn_sleep);
						victim->position = POS_STANDING;
					}
				}
				else
					victim->position = POS_STANDING;

				/* this won't show if they're still asleep */
				act("A cool rush of water awakens you.  You are underwater!\n", victim, nullptr, nullptr, TO_CHAR);
				act("$n wakes at being dragged into the water.", victim, nullptr, nullptr, TO_ROOM);
			}

			act("{CYou begin to hold your breath.{x", victim, nullptr, nullptr, TO_CHAR);
		}
	}
	else if (victim->in_room->sector_type == SECT_AIR) {
		if (!IS_FLYING(victim)
		 && victim->in_room->exit[DIR_DOWN]) {

			Format::sprintf(buf, "$n drags $N into the emptiness from %s.", dir_buf);
			act(buf, ch, nullptr, victim, TO_NOTVICT);

			if (CAN_FLY(victim)
			 && IS_AWAKE(victim))
				do_fly(victim, "");

			if (!IS_FLYING(victim)) {
				int count = 0;  /* just to prevent an infinite loop */
				long brief = victim->comm_flags.has(COMM_BRIEF);
				victim->comm_flags += COMM_BRIEF;

				while (victim->in_room->sector_type == SECT_AIR
				       && !GET_ROOM_FLAGS(victim->in_room).has(ROOM_UNDER_WATER)
				       && victim->in_room->exit[DIR_DOWN]
				       && (to_room = victim->in_room->exit[DIR_DOWN]->u1.to_room)
				       && count++ < 10) {
					if (IS_AWAKE(victim)) {
						RoomPrototype *around, *old = victim->in_room;
						act("$n screams and falls down...", victim, nullptr, nullptr, TO_ROOM);
						do_look(victim, "auto");
						char_from_room(victim);

						/* echo in the cardinal directions */
						for (dir = 0; dir < 4; dir++) {
							if (old->exit[dir] == nullptr
							    || (around = old->exit[dir]->u1.to_room) == nullptr
							    || around->exit[Exit::rev_dir(dir)] == nullptr
							    || around->exit[Exit::rev_dir(dir)]->u1.to_room != old)
								continue;

							char_to_room(victim, old->exit[dir]->u1.to_room);
							Format::sprintf(buf, "You hear a scream from the %s, as if someone were falling...",
							        Exit::dir_name(dir, true));
							act(buf, victim, nullptr, nullptr, TO_ROOM);
							char_from_room(victim);
						}

						char_to_room(victim, old);

						if (count == 1)
							stc("\nYou scream as you realize there is air under your feet, and fall down!\n\n", victim);
						else
							stc("\nYou keep falling....\n\n", victim);
					}
					else {
						act("$N falls down...", ch, nullptr, victim, TO_NOTVICT);

						if (count == 1)
							stc("You have an unsettling dream about falling...\n", victim);
					}

					to_room = victim->in_room->exit[DIR_DOWN]->u1.to_room;
					char_from_room(victim);
					char_to_room(victim, to_room);

					if (IS_AWAKE(victim))
						act("You hear a scream, and look up to see $n hurtling in from above!",
						    victim, nullptr, nullptr, TO_ROOM);
					else
						act("$n falls in from above.", victim, nullptr, nullptr, TO_ROOM);
				}

				if (!brief)
					victim->comm_flags -= COMM_BRIEF;

				if (victim->in_room->sector_type == SECT_WATER_NOSWIM
				    || victim->in_room->sector_type == SECT_WATER_SWIM
				    || GET_ROOM_FLAGS(victim->in_room).has(ROOM_UNDER_WATER)) {
					if (IS_AWAKE(victim))
						stc("You spash down HARD in the water.  OW!!\n\n", victim);
					else
						stc("You are awakened by a mind numbing slap of water as you splash down!\n\n", victim);

					act("$n spashes down HARD in the water.", victim, nullptr, nullptr, TO_ROOM);
				}
				else {
					if (IS_AWAKE(victim))
						stc("The ground finally breaks your fall.  OW!!\n\n", victim);
					else
						stc("You are awakened as you crash HARD into the ground!\n\n", victim);

					act("$n crash lands HARD on the ground.", victim, nullptr, nullptr, TO_ROOM);
				}

				affect_remove_sn_from_char(victim, gsn_sleep); // removes a sleep spell
				victim->position = POS_STANDING;
			}
		}
		else {
			Format::sprintf(buf, "$n flies in from %s, dragging $N behind.", dir_buf);
			act(buf, ch, nullptr, victim, TO_NOTVICT);
		}
	}
	else {
		Format::sprintf(buf, "$n drags $N in from %s.", dir_buf);
		act(buf, ch, nullptr, victim, TO_NOTVICT);
	}

	if (IS_AWAKE(victim))
		do_look(victim, "auto");

	do_look(ch, "auto");
	mprog_entry_trigger(ch);
	mprog_greet_trigger(ch);
	mprog_entry_trigger(victim);
	mprog_greet_trigger(victim);
}

/* MARK: remember the current location for RELOCATE - Elrac */
void do_mark(Character *ch, String argument)
{
	if (IS_NPC(ch)) {
		stc("You feel so at home here, there is no need to MARK.\n", ch);
		return;
	}

	if (!CAN_USE_RSKILL(ch, gsn_mark)) {
		stc("Huh?\n", ch);
		return;
	}

	if (ch->in_room == nullptr || ch->in_room->vnum == 0) {
		stc("You are lost! You would not want to return here.\n", ch);
		return;
	}

	if (ch->in_room->sector_type == SECT_ARENA
	    || ch->in_room->area == Game::world().quest.area
	    || (GET_ROOM_FLAGS(ch->in_room).has(ROOM_GODS_ONLY) && !IS_IMMORTAL(ch))
	    || char_in_duel_room(ch)) {
		stc("Access to this room must be gained anew each time!\n", ch);
		return;
	}

	if (!deduct_stamina(ch, gsn_mark))
		return;

	ch->pcdata->mark_room = ch->in_room->vnum;
	stc("You mark this location. RELOCATE will get you back here.\n", ch);
} /* end do_mark() */

/* RELOCATE: return to previously MARKed location - Elrac */
void do_relocate(Character *ch, String argument)
{
	RoomPrototype *target_room;

	if (IS_NPC(ch)) {
		stc("It's a nice day out, you would rather walk.\n", ch);
		return;
	}

	if (!CAN_USE_RSKILL(ch, gsn_mark)) {
		stc("Huh?\n", ch);
		return;
	}

	if (ch->pcdata->mark_room == 0) {
		stc("You do not remember marking any room.\n", ch);
		return;
	}

	target_room = get_room_index(ch->pcdata->mark_room);

	if (target_room == nullptr) {
		stc("The way back to the room you marked has been lost.\n", ch);
		return;
	}

	/* Hack to prevent players from relocating out of 1212 */
	if (ch->in_room->vnum == 1212) {
		stc("You cannot relocate out of this room.\n", ch);
		return;
	}

	if (ch->fighting) {
		stc("You cannot relocate out of combat.\n", ch);
		return;
	}

	if (char_in_duel_room(ch)) {
		stc("You cannot relocate from the duel arena.\n", ch);
		return;
	}

	if (ch->pcdata->pktimer) {
		stc("You cannot relocate so soon after combat.\n", ch);
		return;
	}

	if (!deduct_stamina(ch, gsn_mark))
		return;

	if (number_percent() > get_skill(ch, gsn_mark)) {
		stc("You fail to relocate.\n", ch);
		check_improve(ch, gsn_mark, FALSE, 4);
		return;
	}

	if ((ch->in_room->sector_type == SECT_ARENA)
	    || (ch->in_room->area == Game::world().quest.area)) {
		stc("You aren't getting out of here that easily!\n", ch);
		return;
	}

	if (ch->in_room == target_room) {
		stc("Noticing you are already there, you don't bother.\n", ch);
		return;
	}

	if (ch->in_room != nullptr) {
		act("$n disappears in a flash of blinding light.", ch, nullptr, nullptr, TO_ROOM);
		char_from_room(ch);
	}

	char_to_room(ch, target_room);
	act("$n appears in a puff of smoke.", ch, nullptr, nullptr, TO_ROOM);
	check_improve(ch, gsn_mark, TRUE, 4);
	do_look(ch, "auto");
	WAIT_STATE(ch, skill_table[gsn_mark].beats);
} /* end do_relocate() */

/* random room generation procedure */
RoomPrototype *get_random_room(Character *ch)
{
	RoomPrototype *room, *prev;

	for (; ;) {
		room = get_room_index(number_range(0, 32767));

		if (room == nullptr
		    || !can_see_room(ch, room)
		    || room->area == Game::world().quest.area
		    || room->clan
		    || room->guild
		    || room->area->name == "Playpen"
		    || room->area->name == "IMM-Zone"
		    || room->area->name == "Limbo"
		    || room->area->name == "Eilyndrae"     /* hack to make eilyndrae and torayna cri unquestable */
		    || room->area->name == "Torayna Cri"
		    || GET_ROOM_FLAGS(room).has_any_of(ROOM_PRIVATE | ROOM_SOLITARY)
		    || (IS_NPC(ch) && GET_ROOM_FLAGS(room).has(ROOM_LAW) && ch->act_flags.has(ACT_AGGRESSIVE))
		    || room->sector_type == SECT_ARENA)
			continue;

		/* no pet shops */
		if ((prev = get_room_index(room->vnum - 1)) != nullptr)
			if (GET_ROOM_FLAGS(prev).has(ROOM_PET_SHOP))
				continue;

		return room;
	}
}

void do_enter(Character *ch, String argument)
{
	RoomPrototype *location;

	/* nifty portal stuff */
	if (!argument.empty()) {
		RoomPrototype *old_room;
		Object *portal;
		Character *fch, *fch_next;
		bool fighting = FALSE;
		int dex, chance, topp = 0;
		old_room = ch->in_room;

		if ((portal = get_obj_list(ch, argument, ch->in_room->contents)) == nullptr) {
			stc("You don't see that here.\n", ch);
			return;
		}

		if (portal->item_type != ITEM_PORTAL || (portal->value[1].flags().has(EX_CLOSED) && !IS_IMMORTAL(ch))) {
			stc("You don't see any sort of entrance...\n", ch);
			return;
		}

		if (!IS_IMMORTAL(ch)) {
			if (ch->in_room->sector_type == SECT_ARENA || char_in_duel_room(ch)) {
				stc("The gods have restricted the use of portals in the arena.\n", ch);
				return;
			}

			/* Added by Lotus 6-22-98 */
			/* make it so you can't use portable portals to get out of norecall areas -- Montrey */
			if (GET_ROOM_FLAGS(ch->in_room).has(ROOM_NOPORTAL)
			    || (GET_ROOM_FLAGS(ch->in_room).has(ROOM_NO_RECALL) && CAN_WEAR(portal, ITEM_TAKE))) {
				stc("The Lord of Evil has denied you access to your portal...muahahaha...\n", ch);
				return;
			}

			if (affect_exists_on_char(ch, gsn_curse)
			    && (portal->value[2].flags().has(GATE_NOCURSE) || CAN_WEAR(portal, ITEM_TAKE))) {
				stc("You step through and are spat violently back out.  Hmmm..\n", ch);
				return;
			}
		}

		if (portal->value[2].flags().has(GATE_RANDOM) || portal->value[3] == -1) {
			location = get_random_room(ch);
			portal->value[3] = location->vnum; /* for record keeping :) */
		}
		else if (portal->value[2].flags().has(GATE_BUGGY) && (number_percent() < 5))
			location = get_random_room(ch);
		else
			location = get_room_index(portal->value[3]);

		if (location == nullptr
		    || location == old_room
		    || !can_see_room(ch, location)
		    || (room_is_private(location) && !IS_IMP(ch))) {
			act("$p opens into a solid looking brick wall.", ch, portal, nullptr, TO_CHAR);
			return;
		}

		if (IS_NPC(ch) && GET_ROOM_FLAGS(location).has(ROOM_LAW) && ch->act_flags.has(ACT_AGGRESSIVE)) {
			stc("As soon as you enter, you are spat violently out again.\n", ch);
			return;
		}

		if (ch->fighting) {
			if (CAN_WEAR(portal, ITEM_TAKE)) {
				stc("The Lord of Evil has denied you access to your portal...muahahaha...\n", ch);
				return;
			}

			/* figure out our chance to flee */
			dex = GET_ATTR_DEX(ch);
			chance = (dex - 9) * 6;

			for (fch = ch->in_room->people; fch != nullptr; fch = fch->next_in_room) {
				if (fch->fighting != ch)
					continue;

				chance += (dex - GET_ATTR_DEX(fch)) * 5;

				if (topp)
					chance -= 10; /* -15% per opponent after the first */

				topp++;
			}

			chance = URANGE(10, chance, 90);
			WAIT_STATE(ch, PULSE_VIOLENCE);

			if (!chance(chance)) {
				stc("You can't get close enough to jump in!\n", ch);
				return;
			}

			fighting = TRUE;
			stop_fighting(ch, TRUE);
			act("$n jumps into $p!", ch, portal, nullptr, TO_ROOM);
		}
		else
			act("$n steps into $p.", ch, portal, nullptr, TO_ROOM);

		if (fighting) {
			if (portal->value[2].flags().has(GATE_NORMAL_EXIT))
				act("You jump through $p!", ch, portal, nullptr, TO_CHAR);
			else
				act("You jump through $p and into a bright light beyond...", ch, portal, nullptr, TO_CHAR);
		}
		else {
			if (portal->value[2].flags().has(GATE_NORMAL_EXIT))
				act("You step through $p.", ch, portal, nullptr, TO_CHAR);
			else
				act("You walk through $p and into a bright light beyond...", ch, portal, nullptr, TO_CHAR);
		}

		char_from_room(ch);
		char_to_room(ch, location);

		if (portal->value[2].flags().has(GATE_GOWITH)) { /* take the gate along */
			obj_from_room(portal);
			obj_to_room(portal, location);
		}

		if (fighting) {
			if (portal->value[2].flags().has(GATE_NORMAL_EXIT))
				act("$n jumps out of $p, looking somewhat battered.", ch, portal, nullptr, TO_ROOM);
			else
				act("In a blinding flash, $n arrives through $p.", ch, portal, nullptr, TO_ROOM);
		}
		else {
			if (portal->value[2].flags().has(GATE_NORMAL_EXIT))
				act("$n saunters in.", ch, portal, nullptr, TO_ROOM);
			else
				act("In a blinding flash, $n arrives through $p.", ch, portal, nullptr, TO_ROOM);
		}

		do_look(ch, "auto");

		if (!IS_NPC(ch) && fighting) {
			if (ch->cls != 2) {
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

		/* charges */
		if (portal->value[0] > 0) {
			portal->value[0] -= 1;

			if (portal->value[0] == 0)
				portal->value[0] = -1;
		}

		/* protect against circular follows */
		if (old_room == location)
			return;

		for (fch = old_room->people; fch != nullptr; fch = fch_next) {
			fch_next = fch->next_in_room;

			/* no following through dead portals */
			if (portal == nullptr || portal->value[0] == -1)
				continue;

			if (fch->master == ch && affect_exists_on_char(fch, gsn_charm_person) && get_position(fch) < POS_STANDING) {
				if (fch->start_pos == POS_FLYING && CAN_FLY(fch))
					do_fly(fch, "");
				else
					do_stand(fch, "");
			}

			if (fch->master == ch && get_position(fch) == POS_STANDING) {
				if (GET_ROOM_FLAGS(ch->in_room).has(ROOM_LAW)
				    && (IS_NPC(fch) && fch->act_flags.has(ACT_AGGRESSIVE))) {
					act("You can't bring $N into the city!! Are you DAFT?!", ch, nullptr, fch, TO_CHAR);
					act("Get yer aggressive butt outta town buddy...", fch, nullptr, nullptr, TO_CHAR);
					continue;
				}

				act("You chase after $N.", fch, nullptr, ch, TO_CHAR);
				do_enter(fch, argument);
			}
		}

		if (portal != nullptr && portal->value[0] == -1) {
			char buf[MAX_STRING_LENGTH];
			act("$p explodes outwards and vanishes.", ch, portal, nullptr, TO_CHAR);
			Format::sprintf(buf, "$p disappears with an inrush of air and a 'Pop!'");

			if (ch->in_room == old_room)
				act(buf, ch, portal, nullptr, TO_ROOM);
			else if (old_room->people != nullptr) {
				act(buf, old_room->people, portal, nullptr, TO_CHAR);
				act(buf, old_room->people, portal, nullptr, TO_ROOM);
			}

			extract_obj(portal);
		}

		return;
	}

	stc("Nope, can't do it.\n", ch);
}

void do_land(Character *ch, String argument)
{
	if (ch->in_room->sector_type == SECT_AIR) {
		stc("There is nowhere to put your feet!\n", ch);
		return;
	}

	if (!IS_FLYING(ch)) {
		stc("You are already on the ground.\n", ch);
		return;
	}

	if (ch->in_room->sector_type == SECT_WATER_SWIM
	 || ch->in_room->sector_type == SECT_WATER_NOSWIM) {
		stc("You land in the water with a big {B*{CS{BP{CL{BA{CS{BH{C*{x!\n", ch);
		act("$n lands in the water with a big {B*{CS{BP{CL{BA{CS{BH{C*{x!\n",
			ch, nullptr, nullptr, TO_ROOM);
	} else {
		stc("You land gracefully on both feet.\n", ch);
		act("$n gracefully lands on both feet.", ch, nullptr, nullptr, TO_ROOM);
	}

	ch->position = POS_STANDING;
	ch->start_pos = POS_STANDING; // preferred position after bash, rest, sleep, etc
}

void do_fly(Character *ch, String argument)
{
	if (IS_FLYING(ch)) {
		stc("You are already flying!\n", ch);
		return;
	}

	if (!CAN_FLY(ch)) {
		stc("You attempt to grow some wings, but fail.\n", ch);
		return;
	}

	// if sitting/sleeping/whatever, stand/wake first.  defer checks to there
	if (ch->position <= POS_SITTING)
		do_stand(ch, "");

	// if still not standing, must have failed
	if (ch->position <= POS_SITTING)
		return;

	ch->position = POS_FLYING;
	ch->start_pos = POS_FLYING; // preferred position after bash, rest, sleep, etc
	ch->on = nullptr;

	stc("You take to the air.\n", ch);
	act("$n takes to the air.", ch, nullptr, nullptr, TO_ROOM);
}

/*
This function attempts to transport a player from
their current location to the room in which their
spouse is located. A player may not use spouse gate
if they are not married, are effected by curse or not standing,
or in a non-teleport/recall area.
-- Outsider
*/
void do_spousegate(Character *ch, String argument)
{
	Character *victim;   /* the spouse in question */
	bool gate_pet = FALSE;   /* take pet with you */

	if (IS_NPC(ch))
		return;

	if (ch->in_room == nullptr)
		return;

	/* We should make sure the character has this skill. -- Outsider */
	if (get_skill(ch, gsn_spousegate) < 50) {
		stc("You do not know how to gate to your spouse.\n", ch);
		return;
	}

	if (ch->in_room->area == Game::world().quest.area) {
		stc("You may not gate in the quest area.\n", ch);
		return;
	}

	if ((ch->in_room->sector_type == SECT_ARENA) || (char_in_duel_room(ch))) {
		stc("You may not gate while in the arena.\n", ch);
		return;
	}

	if (! ch->pcdata->plr_flags.has(PLR_MARRIED)) {
		stc("You are not married.\n", ch);
		return;
	}

	if (ch->fighting) {
		stc("You are too busy right now!\n", ch);
		return;
	}

	victim = get_char_world(ch, ch->pcdata->spouse, VIS_CHAR);

	if (! victim) {
		stc("Your beloved does not seem to be in this world.\n", ch);
		return;
	}

	if (GET_ROOM_FLAGS(ch->in_room).has(ROOM_NO_RECALL)
	    || victim == ch
	    || victim->in_room == nullptr
	    || !can_see_room(ch, victim->in_room)
	    || GET_ROOM_FLAGS(victim->in_room).has_any_of(ROOM_SAFE | ROOM_PRIVATE | ROOM_SOLITARY | ROOM_NO_RECALL)
	    || victim->in_room->sector_type == SECT_ARENA
	    || victim->in_room->area == Game::world().quest.area
	    || char_in_duel_room(victim)
	    || victim->in_room->clan
	    || victim->in_room->guild
	    || (IS_NPC(victim))) {
		stc("You failed.\n", ch);
		return;
	}

	/* check for pet */
	if ((ch->pet) && (ch->in_room == ch->pet->in_room) &&
	    (! ch->pet->act_flags.has(ACT_STAY)))
		gate_pet = TRUE;

	/* transfer person and (perhaps) pet */
	act("$n steps through a gate and vanishes.", ch, nullptr, nullptr, TO_ROOM);
	stc("You step through a gate and vanish.\n", ch);
	char_from_room(ch);
	char_to_room(ch, victim->in_room);
	act("$n has arrived through a gate.", ch, nullptr, nullptr, TO_ROOM);
	do_look(ch, "auto");

	if (gate_pet) {
		act("$n steps through a gate and vanishes.", ch->pet, nullptr, nullptr, TO_ROOM);
		stc("You step through a gate and vanish.\n", ch->pet);
		char_from_room(ch->pet);
		char_to_room(ch->pet, victim->in_room);
		act("$n has arrived through a gate.", ch->pet, nullptr, nullptr, TO_ROOM);
		do_look(ch->pet, "auto");
	}
}

