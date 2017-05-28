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

#include "merc.h"
#include "affect.h"
#include "Format.hpp"

/*
 * Local functions.
 */

char                   *mprog_type_to_name      args((int type));

/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. It allows the words to show up in mpstat to
 *  make it just a hair bit easier to see what a mob should be doing.
 */

char *mprog_type_to_name(int type)
{
	switch (type) {
	case IN_FILE_PROG:          return "in_file_prog";

	case ACT_PROG:              return "act_prog";

	case SPEECH_PROG:           return "speech_prog";

	case RAND_PROG:             return "rand_prog";

	case BOOT_PROG:             return "boot_prog";

	case FIGHT_PROG:            return "fight_prog";

	case BUY_PROG:              return "buy_prog";

	case HITPRCNT_PROG:         return "hitprcnt_prog";

	case DEATH_PROG:            return "death_prog";

	case ENTRY_PROG:            return "entry_prog";

	case GREET_PROG:            return "greet_prog";

	case ALL_GREET_PROG:        return "all_greet_prog";

	case GIVE_PROG:             return "give_prog";

	case BRIBE_PROG:            return "bribe_prog";

	case TICK_PROG:             return "tick_prog";

	default:                    return "ERROR_PROG";
	}
}

/* A trivial rehack of do_mstat.  This doesnt show all the data, but just
 * enough to identify the mob and give its basic condition.  It does however,
 * show the MOBprograms which are set.
 */

void do_mpstat(CHAR_DATA *ch, const char *argument)
{
	char        buf[ MAX_STRING_LENGTH ];
	MPROG_DATA *mprg;
	CHAR_DATA  *victim;

	String arg;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("MobProg stat whom?\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) == NULL) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		stc("Only Mobiles can have Programs!\n", ch);
		return;
	}

	if (!(victim->pIndexData->progtypes)) {
		stc("That Mobile has no Programs set.\n", ch);
		return;
	}

	Format::sprintf(buf, "Name: %s.  Vnum: %d.\n",
	        victim->name, victim->pIndexData->vnum);
	stc(buf, ch);
	Format::sprintf(buf, "Short description: %s.\nLong  description: %s",
	        victim->short_descr,
	        victim->long_descr[0] != '\0' ?
	        victim->long_descr : "(none).\n");
	stc(buf, ch);
	Format::sprintf(buf, "Hp: %d/%d.  Mana: %d/%d.  Stamina: %d/%d. \n",
	        victim->hit,         GET_MAX_HIT(victim),
	        victim->mana,        GET_MAX_MANA(victim),
	        victim->stam,        GET_MAX_STAM(victim));
	stc(buf, ch);
	Format::sprintf(buf,
	        "Lv: %d.  Class: %d.  Align: %d.  AC: %d.  Gold: %ld.  Exp: %d.\n",
	        victim->level,       victim->cls,        victim->alignment,
	        GET_AC(victim, AC_PIERCE),    victim->gold,         victim->exp);
	stc(buf, ch);

	for (mprg = victim->pIndexData->mobprogs; mprg != NULL;
	     mprg = mprg->next) {
		Format::sprintf(buf, ">%s %s\n%s\n",
		        mprog_type_to_name(mprg->type),
		        mprg->arglist,
		        mprg->comlist);
		stc(buf, ch);
	}

	return;
}

/* prints the argument to all the rooms aroud the mobile */

void do_mpasound(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *was_in_room;
	int              door;
	bool save_mobtrigger;

	if (!IS_NPC(ch) || IS_SET(ch->act, ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	if (argument[0] == '\0') {
		bug("Mpasound - No argument: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	was_in_room = ch->in_room;

	for (door = 0; door <= 5; door++) {
		EXIT_DATA       *pexit;

		if ((pexit = was_in_room->exit[door]) != NULL
		    &&   pexit->u1.to_room != NULL
		    &&   pexit->u1.to_room != was_in_room) {
			ch->in_room = pexit->u1.to_room;
			save_mobtrigger = MOBtrigger;
			MOBtrigger  = FALSE;
			act(argument, ch, NULL, NULL, TO_ROOM);
			MOBtrigger = save_mobtrigger;
		}
	}

	ch->in_room = was_in_room;
	return;
}

/* lets the mobile kill any player or mobile without murder*/

void do_mpkill(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;

	if (!IS_NPC(ch) || IS_SET(ch->act, ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		bug("MpKill - no argument: vnum %d.",
		    ch->pIndexData->vnum);
		return;
	}

	if ((victim = get_char_here(ch, arg, VIS_CHAR)) == NULL) {
		bug("MpKill - Victim not in room: vnum %d.",
		    ch->pIndexData->vnum);
		return;
	}

	if (victim == ch) {
		bug("MpKill - Bad victim to attack: vnum %d.",
		    ch->pIndexData->vnum);
		return;
	}

	if (affect_exists_on_char(ch, gsn_charm_person) && ch->master == victim) {
		bug("MpKill - Charmed mob attacking master: vnum %d.",
		    ch->pIndexData->vnum);
		return;
	}

	if (ch->fighting) {
		bug("MpKill - Already fighting: vnum %d",
		    ch->pIndexData->vnum);
		return;
	}

	multi_hit(ch, victim, TYPE_UNDEFINED);
	return;
}

/* lets the mobile destroy an object in its inventory
   it can also destroy a worn object and it can destroy
   items using all.xxxxx or just plain all of them */

void do_mpjunk(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if (!IS_NPC(ch) || IS_SET(ch->act, ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		bug("Mpjunk - No argument: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	if (str_cmp(arg, "all") && str_prefix1("all.", arg)) {
		if ((obj = get_obj_wear(ch, arg)) != NULL) {
			unequip_char(ch, obj);
			extract_obj(obj);
			return;
		}

		if ((obj = get_obj_carry(ch, arg)) == NULL)
			return;

		extract_obj(obj);
	}
	else
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;

			if (arg[3] == '\0' || is_name(&arg[4], obj->name)) {
				if (obj->wear_loc != WEAR_NONE)
					unequip_char(ch, obj);

				extract_obj(obj);
			}
		}

	return;
}

/* prints the message to everyone in the room other than the mob and victim */

void do_mpechoaround(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;

	if (!IS_NPC(ch) || IS_SET(ch->act, ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		bug("Mpechoaround - No argument:  vnum %d.", ch->pIndexData->vnum);
		return;
	}

	if (!(victim = get_char_here(ch, arg, VIS_CHAR))) {
		bug("Mpechoaround - victim does not exist: vnum %d.",
		    ch->pIndexData->vnum);
		return;
	}

	act(argument, ch, NULL, victim, TO_NOTVICT);
	return;
}

/* prints the message to only the victim */

void do_mpechoat(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;

	if (!IS_NPC(ch) || IS_SET(ch->act, ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0') {
		bug("Mpechoat - No argument:  vnum %d.",
		    ch->pIndexData->vnum);
		return;
	}

	if (!(victim = get_char_here(ch, arg, VIS_CHAR))) {
		bug("Mpechoat - victim does not exist: vnum %d.",
		    ch->pIndexData->vnum);
		return;
	}

	act(argument, ch, NULL, victim, TO_VICT);
	return;
}

/* prints the message to the room at large */

void do_mpecho(CHAR_DATA *ch, const char *argument)
{
	if (!IS_NPC(ch) || IS_SET(ch->act, ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	if (argument[0] == '\0') {
		bug("Mpecho - called w/o argument: vnum %d.",
		    ch->pIndexData->vnum);
		return;
	}

	act(argument, ch, NULL, NULL, TO_ROOM);
	return;
}

void do_mpclearmoney(CHAR_DATA *ch, const char *argument)
{
	if (!IS_NPC(ch) || IS_SET(ch->act, ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	ch->gold = 0;
	ch->silver = 0;
	return;
}

/* lets the mobile load an item or mobile.  All items
are loaded into inventory.  you can specify a level with
the load object portion as well. */

void do_mpmload(CHAR_DATA *ch, const char *argument)
{
	MOB_INDEX_DATA *pMobIndex;
	CHAR_DATA      *victim;

	if (!IS_NPC(ch) || IS_SET(ch->act, ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg[0] == '\0' || !is_number(arg)) {
		bug("Mpmload - Bad vnum as arg: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	if ((pMobIndex = get_mob_index(atoi(arg))) == NULL) {
		bug("Mpmload - Bad mob vnum: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	victim = create_mobile(pMobIndex);

	if (! victim) {  /* Check for memory error. -- Outsider */
		bug("Memory error in do_mpmload().", 0);
		return;
	}

	char_to_room(victim, ch->in_room);
	return;
}

void do_mpoload(CHAR_DATA *ch, const char *argument)
{
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA       *obj;

	if (!IS_NPC(ch) || IS_SET(ch->act, ACT_MORPH)) {
		do_huh(ch);
		return;
	}

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || !is_number(arg1)) {
		bug("Mpoload - Bad syntax: vnum %d.",
		    ch->pIndexData->vnum);
		return;
	}

	if ((pObjIndex = get_obj_index(atoi(arg1))) == NULL) {
		bug("Mpoload - Bad vnum arg: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	obj = create_object(pObjIndex, 0);

	if (! obj) {
		bug("Memory error creating object in mpoload.", 0);
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

void do_mppurge(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA  *obj;

	if (!IS_NPC(ch) || IS_SET(ch->act, ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		/* 'purge' */
		CHAR_DATA *vnext;
		OBJ_DATA  *obj_next;

		for (victim = ch->in_room->people; victim != NULL; victim = vnext) {
			vnext = victim->next_in_room;

			if (IS_NPC(victim) && victim != ch)
				extract_char(victim, TRUE);
		}

		for (obj = ch->in_room->contents; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			extract_obj(obj);
		}

		return;
	}

	if ((victim = get_char_here(ch, arg, VIS_CHAR)) != NULL) {
		if (!IS_NPC(victim)) {
			bug("Mppurge - Purging a PC: vnum %d.", ch->pIndexData->vnum);
			return;
		}

		extract_char(victim, TRUE);
		return;
	}
	else if ((obj = get_obj_here(ch, arg)) != NULL)
		extract_obj(obj);
	else
		bug("Mppurge - Bad argument: vnum %d.", ch->pIndexData->vnum);
} /* end do_mppurge() */

/* lets the mobile goto any location it wishes that is not private */

void do_mpgoto(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *location;

	if (!IS_NPC(ch) || IS_SET(ch->act, ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		bug("Mpgoto - No argument: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	if ((location = find_location(ch, arg)) == NULL) {
		bug("Mpgoto - No such location: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	if (ch->fighting != NULL)
		stop_fighting(ch, TRUE);

	char_from_room(ch);
	char_to_room(ch, location);
	return;
}

/* lets the mobile do a command at another location. Very useful */

void do_mpat(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *location;
	ROOM_INDEX_DATA *original;
	CHAR_DATA       *wch;

	if (!IS_NPC(ch) || IS_SET(ch->act, ACT_MORPH)) {
		stc("Huh?\n", ch);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0') {
		bug("Mpat - Bad argument: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	if ((location = find_location(ch, arg)) == NULL) {
		bug("Mpat - No such location: vnum %d.", ch->pIndexData->vnum);
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
	for (wch = char_list; wch != NULL; wch = wch->next) {
		if (wch == ch) {
			char_from_room(ch);
			char_to_room(ch, original);
			break;
		}
	}

	return;
}

/* lets the mobile transfer people.  the all argument transfers
   everyone in the current room to the specified location */

void do_mptransfer(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *location;
//	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;

	if (!IS_NPC(ch) || IS_SET(ch->act, ACT_MORPH)) {
		do_huh(ch);
		return;
	}

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0') {
		bug("Mptransfer - Bad syntax: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	/* eh, let's go ahead and NOT let mobs transfer all :P  -- Montrey */
	/*      if (!str_cmp(arg1, "all"))
	        {
	                for (d = descriptor_list; d != NULL; d = d->next)
	                {
	                        if (IS_PLAYING(d)
	                         && d->character != ch
	                         && d->character->in_room != NULL
	                         && can_see_char(ch, d->character))
	                        {
	                                char buf[MSL];

	                                Format::sprintf(buf, "%s %s", d->character->name, arg2);
	                                do_transfer(ch, buf);
	                        }
	                }

	                return;
	        } */

	if (arg2[0] == '\0')
		location = ch->in_room;
	else {
		if ((location = find_location(ch, arg2)) == NULL) {
			bug("Mptransfer - No such location: vnum %d.", ch->pIndexData->vnum);
			return;
		}

		if (room_is_private(location)) {
			bug("Mptransfer - Private room: vnum %d.", ch->pIndexData->vnum);
			return;
		}
	}

	for (victim = char_list; victim != NULL; victim = victim->next)
		if (is_name(arg1, victim->name))
			break;

	if (victim == NULL) {
		bug("Mptransfer - No such person: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	if (victim->in_room == NULL) {
		bug("Mptransfer - Victim in NULL room: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	if (victim->fighting != NULL)
		stop_fighting(victim, TRUE);

	char_from_room(victim);
	char_to_room(victim, location);
	return;
}

/* lets the mobile force someone to do something.  must be mortal level
   and the all argument only affects those in the room with the mobile */
void do_mpforce(CHAR_DATA *ch, const char *argument)
{
	if (!IS_NPC(ch) || IS_SET(ch->act, ACT_MORPH)) {
		do_huh(ch);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0') {
		bug("Mpforce - Bad syntax: vnum %d.", ch->pIndexData->vnum);
		return;
	}

	if (!str_cmp(arg, "all")) {
		CHAR_DATA *vch, *vch_next;

		for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
			vch_next = vch->next_in_room;

			if (vch->in_room == ch->in_room
			    && !IS_IMMORTAL(vch))
//			 && can_see_char(ch, vch))  who cares about whether the mob can see? -- Montrey
				interpret(vch, argument);
		}
	}
	else {
		CHAR_DATA *victim;

		for (victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room)
			if (is_name(arg, victim->name))
				break;

		if (victim == NULL) {
			bug("Mpforce - No such victim: vnum %d.", ch->pIndexData->vnum);
			return;
		}

		if (victim == ch) {
			bug("Mpforce - Forcing oneself: vnum %d.", ch->pIndexData->vnum);
			return;
		}

		interpret(victim, argument);
	}
}
