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

/* Added sys/time to avoid error from
   gettimeofday() function call.
   -- Outsider
*/
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "magic.h"
#include "lookup.h"
#include "affect.h"
#include "buffer.h"
#include "Format.hpp"
#include "c_string.h"

/* command procedures needed */
DECLARE_DO_FUN(do_split);
DECLARE_DO_FUN(do_yell);
DECLARE_DO_FUN(do_say);
DECLARE_DO_FUN(talk_auction);

extern  void    channel_who     args((CHAR_DATA *ch, const char *channelname, int channel, int custom));

/*
 * Local functions.
 */
bool       remove_obj      args((CHAR_DATA *ch, int iWear, bool fReplace));
void       wear_obj        args((CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace));
CHAR_DATA *find_keeper     args((CHAR_DATA *ch));
int        get_cost        args((CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy));
void       obj_to_keeper   args((OBJ_DATA *obj, CHAR_DATA *ch));
OBJ_DATA  *get_obj_keeper  args((CHAR_DATA *ch, CHAR_DATA *keeper, const char *argument));

/* Convert a number to an ordinal string -- Elrac
   The string may come from a static buffer, so it should be copied
   before someone else gets a chance to call this function.
*/
char *ordinal_string(int n)
{
	static char buf[20];

	if (n == 1)
		return "first";
	else if (n == 2)
		return "second";
	else if (n == 3)
		return "third";
	else if (n % 10 == 1)
		sprintf(buf, "%dst", n);
	else if (n % 10 == 2)
		sprintf(buf, "%dnd", n);
	else if (n % 10 == 3)
		sprintf(buf, "%drd", n);
	else
		sprintf(buf, "%dth", n);

	return buf;
}

/* Check to see if an object is non-holder clan equipment -- Elrac
   This code depends on the clan equipment being coded into the clan areas
   and the starting/ending vnums of the areas being coded into clan_table.
   This function returns TRUE if <ch> is allowed to do stuff with <obj>
   based on clan ownership and leadership. If not, it tells the player
   he can't do that and the object is destroyed. Also, FALSE is returned.
*/
bool clan_eq_ok(CHAR_DATA *ch, OBJ_DATA *obj, char *action)
{
	if (IS_IMMORTAL(ch))
		return TRUE;

	CLAN_DATA *jclan;
	if ((jclan = clan_vnum_lookup(obj->pIndexData->vnum)) == NULL)
		return TRUE;

	if (ch->clan != jclan) {
		if (action != NULL && *action != '\0') {
			ptc(ch, "You attempt to %s %s, which belongs to %s.\n",
			        action, obj->short_descr, jclan->clanname);
		}

		ptc(ch, "%s {Yexplodes violently{x, leaving only a cloud of {gsmoke{x.\n",
		        obj->short_descr);
		stc("You are lucky you weren't {Phurt!{x\n", ch);
		extract_obj(obj);
		return FALSE;
	}

	if (is_name("leadereq", obj->name) && !HAS_CGROUP(ch, GROUP_LEADER)) {
		if (action != NULL && *action != '\0') {
			ptc(ch, "You attempt to %s your Leader's %s.\n",
			        action, obj->short_descr);
		}

		ptc(ch, "%s flies from your grasp, presumably into your Clan's coffers.\n",
		        obj->short_descr);
		stc("Wait until the Leaders hear of this!\n", ch);
		extract_obj(obj);
		return FALSE;
	}

	return TRUE;
}

/* Check for illegitimate use of personal equipment -- Elrac
   This code checks to make sure the given object is not personalized
   to someone else. If all is OK, return TRUE, otherwise the player
   gets a denial message with the intended action in it, and the
   function returns FALSE.
*/
bool pers_eq_ok(CHAR_DATA *ch, OBJ_DATA *obj, char *action)
{
	const char *pdesc;
	char owner[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];

	if (ch == NULL) {
		bug("pers_eq_ok: NULL character", 0);
		return TRUE;
	}

	if (obj == NULL) {
		bug("pers_eq_ok: NULL object", 0);
		return TRUE;
	}

	if (obj->pIndexData == NULL) {
		bug("pers_eq_ok: NULL object->pIndexData", 0);
		return TRUE;
	}

	if ((pdesc = get_extra_descr(KEYWD_OWNER, obj->extra_descr)) == NULL
	    && (pdesc = get_extra_descr(KEYWD_OWNER, obj->pIndexData->extra_descr)) == NULL)
		return TRUE;

	sscanf(pdesc, "%[^\n]", owner);

	if (is_name(ch->name, owner))
		return TRUE;

	if (obj->short_descr == NULL || obj->short_descr[0] == '\0') {
		bug("clan_eq_ok: object %d has no short_descr", obj->pIndexData->vnum);
		obj->short_descr = str_dup("something");
	}

	if (action != NULL && *action != '\0') {
		sprintf(buf, "You attempt to %s %s.\n", action, obj->short_descr);
		stc(buf, ch);
		sprintf(buf, "   This item belongs to %s!\n", owner);
		stc(buf, ch);

		if (IS_IMMORTAL(ch)) {
			stc("   As an Immortal, you have the right.\n", ch);
			return TRUE;
		}
	}

	return FALSE;
} /* end pers_eq_ok() */

/* RT part of the corpse looting code */

bool can_loot(CHAR_DATA *ch, OBJ_DATA *obj)
{
	CHAR_DATA *owner = NULL, *wch;

	if (IS_IMMORTAL(ch))
		return TRUE;

	if (!obj->owner || obj->owner == NULL)
		return TRUE;

	for (wch = char_list; wch != NULL ; wch = wch->next)
		if (!str_cmp(wch->name, obj->owner))
			owner = wch;

	if (owner == NULL)
		return TRUE;

	if (!str_cmp(ch->name, owner->name))
		return TRUE;

	if (!IS_NPC(owner) && IS_SET(owner->act, PLR_CANLOOT))
		return TRUE;

	if (is_same_group(ch, owner))
		return TRUE;

	return FALSE;
}

void do_second(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;

	if (!get_skill(ch, gsn_dual_wield) && !IS_NPC(ch)) {
		stc("You are not able to wield two weapons.\n", ch);
		return;
	}

	if (argument[0] == '\0') {
		stc("Which weapon do you want to wield in your off-hand?\n", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, argument)) == NULL) {
		stc("You do not have that weapon.\n", ch);
		return;
	}

	if (obj->item_type != ITEM_WEAPON) {
		stc("That is not a weapon.\n", ch);
		return;
	}

	if (!IS_IMMORTAL(ch)) {
		if (IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS) && ch->size < SIZE_HUGE) {
			stc("You can not dual-wield a two handed weapon.\n", ch);
			return;
		}

		if (get_eq_char(ch, WEAR_WIELD) != NULL
		    && IS_WEAPON_STAT(get_eq_char(ch, WEAR_WIELD), WEAPON_TWO_HANDS)
		    && ch->size < SIZE_LARGE) {
			stc("Your primary weapon is taking both of your hands.\n", ch);
			return;
		}

		if (get_eq_char(ch, WEAR_SHIELD) != NULL
		    || get_eq_char(ch, WEAR_HOLD)   != NULL) {
			stc("You need both hands free to wield two weapons.\n", ch);
			return;
		}

		if (get_obj_weight(obj) > (str_app[GET_ATTR_STR(ch)].wield * 5)) {
			stc("This weapon is too heavy to be used in your off-hand.\n", ch);
			return;
		}

		if (obj->level > get_usable_level(ch)) {
			if (obj->level < LEVEL_IMMORTAL) {
				ptc(ch, "You must be level %d(%d) to use this object.\n",
				    ch->level + (obj->level - get_usable_level(ch)), obj->level);
				act("$n tries to use $p, but is too inexperienced.", ch, obj, NULL, TO_ROOM);
				return;
			}
			else {
				stc("This item must be lowered for mortal use.\n", ch);
				act("$n tries to use $p, but is too bound to the mortal plane.", ch, obj, NULL, TO_ROOM);
				return;
			}
		}
	}

	if (!remove_obj(ch, WEAR_SECONDARY, TRUE))
		return;

	if (!clan_eq_ok(ch, obj, "equip yourself with"))
		return;

	if (!pers_eq_ok(ch, obj, "equip yourself with"))
		return;

	act("$n wields $p{x in $s off-hand.", ch, obj, NULL, TO_ROOM);
	act("You wield $p{x in your off-hand.", ch, obj, NULL, TO_CHAR);
	equip_char(ch, obj, WEAR_SECONDARY);
} /* end do_second() */

void get_obj(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container)
{
	/* variables for AUTOSPLIT */
	CHAR_DATA *gch;
	int members;
	char buffer[100];

	if (!CAN_WEAR(obj, ITEM_TAKE)) {
		stc("You can't take that!\n", ch);
		return;
	}

	if (container && container->carried_by && container->carried_by == ch) {
		/* from held container to inventory -- no change in number */
		;
	}
	else if (get_carry_number(ch) + get_obj_number(obj) > can_carry_n(ch)) {
		act("$d: you can't carry that many items.", ch, NULL, obj->name, TO_CHAR);
		return;
	}

	if (get_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch)) {
		act("$d: you can't carry that much weight.", ch, NULL, obj->name, TO_CHAR);
		return;
	}

	if (!can_loot(ch, obj)) {
		act("HEY!  No corpse looting ya scavenger!", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (obj->in_room != NULL) {
		for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
			if (gch->on == obj) {
				act("$N appears to be using $p.", ch, obj, gch, TO_CHAR);
				return;
			}
	}

	if (container != NULL) {
		if (container == donation_pit) {
			if (!IS_HEROIC(ch)
		    && ((obj->level >= LEVEL_IMMORTAL && obj->level > get_holdable_level(ch))
		        || (obj->level < LEVEL_IMMORTAL && obj->level > get_usable_level(ch)))) {
				stc("You are not powerful enough to use it.\n", ch);
				return;
			}

			obj->donated = 0;
		}

		act("You get $p from $P.", ch, obj, container, TO_CHAR);
		act("$n gets $p from $P.", ch, obj, container, TO_ROOM);
		obj_from_obj(obj);
	}
	else {
		act("You get $p.", ch, obj, container, TO_CHAR);
		act("$n gets $p.", ch, obj, container, TO_ROOM);
		obj_from_room(obj);
	}

	if (obj->item_type == ITEM_MONEY) {
		ch->silver += obj->value[0];
		ch->gold += obj->value[1];

		if (IS_SET(ch->act, PLR_AUTOSPLIT)) {
			/* AUTOSPLIT code */
			members = 0;

			for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
				if (is_same_group(gch, ch)
				    && !IS_NPC(gch))       /* don't split with mobs :P */
					members++;

			if (members > 1 && (obj->value[0] > 1 || obj->value[1])) {
				sprintf(buffer, "%d %d", obj->value[0], obj->value[1]);
				do_split(ch, buffer);
			}
		}

		extract_obj(obj);
	}
	else {
		obj_to_char(obj, ch);

		if (!IS_NPC(ch)) {
			/* Did they pick up their quest item? */
			if (IS_SET(ch->act, PLR_QUESTOR)) {
				if (ch->questobj == obj->pIndexData->vnum && ch->questobf != -1) {
					char buf[MAX_STRING_LENGTH];
					stc("{YYou have almost completed your QUEST!{x\n", ch);
					stc("{YReturn to the questmaster before your time runs out!{x\n", ch);
					ch->questobf = -1;
					sprintf(buf, "{Y:QUEST: {x$N has found %s", obj->short_descr);
					wiznet(buf, ch, NULL, WIZ_QUEST, 0, 0);
				}
			}

			/* or skill quest item? */
			if (IS_SET(ch->pcdata->plr, PLR_SQUESTOR)) {
				if (ch->pcdata->squestobj == obj && !ch->pcdata->squestobjf) {
					char buf[MAX_STRING_LENGTH];

					if (ch->pcdata->squestmob == NULL) {
						stc("{YYou have almost completed your {VSKILL QUEST!{x\n", ch);
						stc("{YReturn to the questmistress before your time runs out!{x\n", ch);
					}
					else {
						stc("{YYou have completed part of your {VSKILL QUEST!{x\n", ch);
						sprintf(buf, "{YTake the artifact to %s while there is still time!{x\n",
						        ch->pcdata->squestmob->short_descr);
						stc(buf, ch);
					}

					ch->pcdata->squestobjf = TRUE;
					sprintf(buf, "{Y:SKILL QUEST: {x$N has found the %s", obj->short_descr);
					wiznet(buf, ch, NULL, WIZ_QUEST, 0, 0);
				}
			}
		}
	}

	return;
}

bool from_box_ok(CHAR_DATA *ch, OBJ_DATA *obj, char *box_type)
{
	if (obj == NULL) {
		ptc(ch, "You do not see that in your %s.\n", box_type);
		return FALSE;
	}

	if (!CAN_WEAR(obj, ITEM_TAKE)) {
		stc("You can't take that!\n", ch);
		bugf("Item without ITEM_TAKE in %s's %s.", ch->name, box_type);
		return FALSE;
	}

	if (get_carry_number(ch) + get_obj_number(obj) > can_carry_n(ch)) {
		ptc(ch, "%s: you can't carry that many items.\n", obj->name);
		return FALSE;
	}

	if (get_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch)) {
		ptc(ch, "%s: you can't carry that much weight.\n", obj->name);
		return FALSE;
	}

	return TRUE;
}

void do_get(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj, *obj_next, *container;
	bool found;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (!str_cmp(arg2, "from"))
		argument = one_argument(argument, arg2);

	/* Get type. */
	if (arg1[0] == '\0') {
		stc("Get what?\n", ch);
		return;
	}

	if (arg2[0] == '\0') {
		if (str_cmp(arg1, "all") && str_prefix1("all.", arg1)) {
			/* 'get obj' */
			obj = get_obj_list(ch, arg1, ch->in_room->contents);

			if (obj == NULL) {
				ptc(ch, "You don't see a %s here.\n", arg1);
				return;
			}

			get_obj(ch, obj, NULL);
		}
		else {
			/* 'get all' or 'get all.obj' */
			found = FALSE;

			for (obj = ch->in_room->contents; obj != NULL; obj = obj_next) {
				obj_next = obj->next_content;

				if ((arg1[3] == '\0' || is_name(&arg1[4], obj->name)) && can_see_obj(ch, obj)) {
					found = TRUE;
					get_obj(ch, obj, NULL);
				}
			}

			if (!found) {
				if (arg1[3] == '\0')
					stc("Get what?\n", ch);
				else
					ptc(ch, "You see no %s here.\n", &arg1[4]);
			}
		}
	}
	else {
		/* 'get ... container' */
		if (!str_cmp(arg2, "all") || !str_prefix1("all.", arg2)) {
			stc("You can't do that.\n", ch);
			return;
		}

		if (!str_prefix1(arg2, "locker") && !IS_NPC(ch)) {
			if (IS_SET(GET_ROOM_FLAGS(ch->in_room), ROOM_LOCKER)) {
				if (IS_SET(ch->act, PLR_CLOSED)) {
					int number = get_locker_number(ch);

					if (deduct_cost(ch, number * 10)) {
						ptc(ch, "%d silver has been deducted for your locker.\n",
						    number * 10);
						REMOVE_BIT(ch->act, PLR_CLOSED);
					}
					else {
						stc("Your locker is closed.\n", ch);
						return;
					}
				}

				if (str_cmp(arg1, "all") && str_prefix1("all.", arg1)) {
					obj = get_obj_list(ch, arg1, ch->pcdata->locker);

					if (!from_box_ok(ch, obj, "locker"))
						return;

					obj_from_locker(obj);
					obj_to_char(obj, ch);
					act("$n gets $p from $s locker.", ch, obj, NULL, TO_ROOM);
					act("You get $p from your locker.", ch, obj, NULL, TO_CHAR);
				}
				else {
					found = FALSE;

					for (obj = ch->pcdata->locker; obj != NULL; obj = obj_next) {
						obj_next = obj->next_content;

						if ((arg1[3] == '\0' || is_name(&arg1[4], obj->name)) && can_see_obj(ch, obj)) {
							found = TRUE;

							if (!from_box_ok(ch, obj, "locker"))
								continue;

							obj_from_locker(obj);
							obj_to_char(obj, ch);
							act("$n gets $p from $s locker.", ch, obj, NULL, TO_ROOM);
							act("You get $p from your locker.", ch, obj, NULL, TO_CHAR);
						}
					}

					if (!found) {
						if (arg1[3] == '\0')
							stc("You see nothing in the locker.\n", ch);
						else
							stc("You see nothing like that in the locker.\n", ch);
					}
				}
			}
			else
				stc("You do not see a locker access here.\n", ch);

			return;
		}

		/* Strongbox stuff -- Elrac */
		if (!IS_NPC(ch) && !str_prefix1(arg2, "strongbox")) {
			if (!IS_HEROIC(ch)) {
				stc("Only heroes and former heroes have strongboxes.\n", ch);
				return;
			}

			if (!ch->in_room || ch->in_room->vnum != ROOM_VNUM_STRONGBOX) {
				stc("You do not see your strongbox here.\n", ch);
				return;
			}

			if (str_cmp(arg1, "all") && str_prefix1("all.", arg1)) {
				obj = get_obj_list(ch, arg1, ch->pcdata->strongbox);

				if (!from_box_ok(ch, obj, "strongbox"))
					return;

				obj_from_strongbox(obj);
				obj_to_char(obj, ch);
				act("$n gets $p from $s strongbox.", ch, obj, NULL, TO_ROOM);
				act("You get $p from your strongbox.", ch, obj, NULL, TO_CHAR);
				return;
			}

			/* get all or all.something from strongbox */
			found = FALSE;

			for (obj = ch->pcdata->strongbox; obj != NULL; obj = obj_next) {
				obj_next = obj->next_content;

				if ((arg1[3] == '\0' || is_name(&arg1[4], obj->name)) && can_see_obj(ch, obj)) {
					found = TRUE;

					if (!from_box_ok(ch, obj, "strongbox"))
						continue;

					obj_from_strongbox(obj);
					obj_to_char(obj, ch);
					act("$n gets $p from $s strongbox.", ch, obj, NULL, TO_ROOM);
					act("You get $p from your strongbox.", ch, obj, NULL, TO_CHAR);
				}
			}

			if (!found) {
				if (arg1[3] == '\0')
					stc("You see nothing in your strongbox.\n", ch);
				else
					stc("You see nothing like that in your strongbox.\n", ch);
			}

			return;
		}

		if ((container = get_obj_here(ch, arg2)) == NULL) {
			ptc(ch, "You see no %s here.\n", arg2);
			return;
		}

		switch (container->item_type) {
		default:
			if (!IS_OBJ_STAT(container, ITEM_COMPARTMENT)) {
				if (container->contains)
					stc("You can't get it out.\n", ch);
				else
					stc("That's not a container.\n", ch);

				return;
			}

			break;

		case ITEM_CONTAINER:
		case ITEM_CORPSE_NPC:
			break;

		case ITEM_CORPSE_PC:
			if (!can_loot(ch, container)) {
				stc("You can't do that.\n", ch);
				return;
			}

			break;
		}

		if (IS_SET(container->value[1], CONT_CLOSED) && !IS_OBJ_STAT(container, ITEM_COMPARTMENT)) {
			ptc(ch, "The %s is closed.\n", container->short_descr);
			return;
		}

		if (str_cmp(arg1, "all") && str_prefix1("all.", arg1)) {
			/* 'get obj container' */
			obj = get_obj_list(ch, arg1, container->contains);

			if (obj == NULL) {
				ptc(ch, "You see nothing like that in %s.\n", container->short_descr);
				return;
			}

			get_obj(ch, obj, container);
		}
		else {
			/* 'get all container' or 'get all.obj container' */
			found = FALSE;

			for (obj = container->contains; obj != NULL; obj = obj_next) {
				obj_next = obj->next_content;

				if ((arg1[3] == '\0' || is_name(&arg1[4], obj->name)) && can_see_obj(ch, obj)) {
					found = TRUE;

					if (container == donation_pit && !IS_IMMORTAL(ch)) {
						stc("Don't be so greedy!\n", ch);
						return;
					}

					get_obj(ch, obj, container);
				}
			}

			if (!found) {
				if (arg1[3] == '\0')
					ptc(ch, "You see nothing in %s.\n", container->short_descr);
				else
					ptc(ch, "You see nothing like that in %s.\n", container->short_descr);
			}
		}
	}
}

/* I really hate this, come up with something better someday.  Since compartment items
   don't have max weights, we're basing it off of level, unless the item isn't takeable,
   in which it gets a pretty big capacity.  Hopefully someday we'll put in a volume
   system to replace this ridiculous number of items and max weight capacities system.
                                                                -- Montrey */
bool will_fit(OBJ_DATA *obj, OBJ_DATA *container)
{
	int obj_weight, container_max_total, container_max_single;
	obj_weight = get_obj_weight(obj);

	if (container->item_type == ITEM_CONTAINER) {
		container_max_single = container->value[3] * 10;
		container_max_total  = container->value[0] * 10;
	}
	else {
		if (CAN_WEAR(container, ITEM_TAKE)) {
			container_max_single = (container->level + 1) * 30;
			container_max_total  = ((container->level / 3) + 1) * 10;
		}
		else {
			container_max_single = 1000;
			container_max_total  = 10000;
		}
	}

	if (obj_weight > container_max_single)
		return FALSE;

	if (obj_weight + get_obj_weight(container) > container_max_total)
		return FALSE;

	return TRUE;
}

void do_put(CHAR_DATA *ch, const char *argument)
{
	char arg1[MIL], arg2[MIL];
	OBJ_DATA *container, *obj, *obj_next;
	int weight;
	bool found = FALSE, tooheavy = TRUE;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (!str_cmp(arg2, "in") || !str_cmp(arg2, "on"))
		argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		stc("Put what in what?\n", ch);
		return;
	}

	if (!str_cmp(arg2, "all") || !str_prefix1("all.", arg2)) {
		stc("You can't do that.\n", ch);
		return;
	}

	/* locker stuff */
	if (!IS_NPC(ch) && !str_prefix1(arg2, "locker")) {
		if (!IS_SET(GET_ROOM_FLAGS(ch->in_room), ROOM_LOCKER)) {
			stc("You do not see a locker in this room.\n", ch);
			return;
		}

		if (str_cmp(arg1, "all") && str_prefix1("all.", arg1)) {
			/* 'put obj locker' */
			if ((obj = get_obj_carry(ch, arg1)) == NULL) {
				stc("You do not have that item.\n", ch);
				return;
			}

			if (!can_drop_obj(ch, obj)) {
				stc("You can't let go of it.\n", ch);
				return;
			}

			weight = get_obj_weight(obj);

			if (weight + get_locker_weight(ch) > 10000
			    || weight > 1000
			    || get_obj_number(obj) + get_locker_number(ch) > 250) {
				stc("It won't fit.\n", ch);
				return;
			}

			obj_from_char(obj);
			obj_to_locker(obj, ch);
			act("$n puts $p in $s locker.", ch, obj, NULL, TO_ROOM);
			act("You put $p in your locker.", ch, obj, NULL, TO_CHAR);
			return;
		}

		/* 'put all locker' or 'put all.obj locker' */
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			weight = get_obj_weight(obj);

			if ((arg1[3] == '\0' || is_name(&arg1[4], obj->name))
			    && can_see_obj(ch, obj)
			    && obj->wear_loc == WEAR_NONE
			    && can_drop_obj(ch, obj)
			    && weight + get_locker_weight(ch) <= 10000
			    && weight <= 1000
			    && get_obj_number(obj) + get_locker_number(ch) <= 250) {
				obj_from_char(obj);
				obj_to_locker(obj, ch);
				act("$n puts $p in $s locker.", ch, obj, NULL, TO_ROOM);
				act("You put $p in your locker.", ch, obj, NULL, TO_CHAR);
				found = TRUE;
			}
		}

		if (!found)
			stc("You do not have those.\n", ch);

		return;
	}

	/* strongbox stuff -- Elrac */
	if (!IS_NPC(ch) && !str_prefix1(arg2, "strongbox")) {
		if (!IS_HEROIC(ch)) {
			stc("Only heroes and former heroes have strongboxes.\n", ch);
			return;
		}

		if (!ch->in_room || ch->in_room->vnum != ROOM_VNUM_STRONGBOX) {
			stc("There is no strongbox here.\n", ch);
			return;
		}

		if (str_cmp(arg1, "all") && str_prefix1("all.", arg1)) {
			/* 'put obj strongbox' */
			if ((obj = get_obj_carry(ch, arg1)) == NULL) {
				stc("You do not have that item.\n", ch);
				return;
			}

			if (!can_drop_obj(ch, obj)) {
				stc("You can't let go of it.\n", ch);
				return;
			}
/*
			if (get_obj_number(obj) + get_strongbox_number(ch) > 45 + ch->pcdata->remort_count) {
				stc("It won't fit.\n", ch);
				return;
			}
*/
			obj_from_char(obj);
			obj_to_strongbox(obj, ch);
			act("$n puts $p in $s strongbox.", ch, obj, NULL, TO_ROOM);
			act("You put $p in your strongbox.", ch, obj, NULL, TO_CHAR);
			return;
		}

		/* 'put all strongbox' or 'put all.obj strongbox' */
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;

			if ((arg1[3] == '\0' || is_name(&arg1[4], obj->name))
			    && can_see_obj(ch, obj)
			    && obj->wear_loc == WEAR_NONE
			    && can_drop_obj(ch, obj)) {
//			    && get_obj_number(obj) + get_strongbox_number(ch) <= 45 + ch->pcdata->remort_count) {
				obj_from_char(obj);
				obj_to_strongbox(obj, ch);
				act("$n puts $p in $s strongbox.", ch, obj, NULL, TO_ROOM);
				act("You put $p in your strongbox.", ch, obj, NULL, TO_CHAR);
				found = TRUE;
			}
		}

		if (!found)
			stc("You do not have those.\n", ch);

		return;
	}

	if ((container = get_obj_here(ch, arg2)) == NULL) {
		act("There's no $T here.", ch, NULL, arg2, TO_CHAR);
		return;
	}

	if (container->item_type != ITEM_CONTAINER) {
		if (!IS_OBJ_STAT(container, ITEM_COMPARTMENT)) {
			if (container->contains)
				stc("You cannot find an opening.\n", ch);
			else
				stc("That's not a container.\n", ch);

			return;
		}
	}

	if (IS_SET(container->value[1], CONT_CLOSED)
	    && !IS_OBJ_STAT(container, ITEM_COMPARTMENT)) {
		act("The $d is closed.", ch, NULL, container->name, TO_CHAR);
		return;
	}

	if (str_cmp(arg1, "all") && str_prefix1("all.", arg1)) {
		/* 'put obj container' */
		if ((obj = get_obj_carry(ch, arg1)) == NULL) {
			stc("You do not have that item.\n", ch);
			return;
		}

		if (obj == container) {
			stc("You can't fold it into itself.\n", ch);
			return;
		}

		if (!can_drop_obj(ch, obj)) {
			stc("You can't let go of it.\n", ch);
			return;
		}

		/*              if ((get_obj_weight(obj) + (get_true_weight(container) - container->weight)
		                 > ((container->item_type == ITEM_CONTAINER) ? (container->value[0] * 10) :
		                   (CAN_WEAR(container, ITEM_TAKE) ? (((container->level/3)+1)*10) : 1000)))
		                 || (get_obj_weight(obj)
		                 > ((container->item_type == ITEM_CONTAINER) ? (container->value[3] * 10) :
		                   (CAN_WEAR(container, ITEM_TAKE) ? ((container->level+1)*30) : 10000))))*/

		if (!will_fit(obj, container)) {
			stc("It won't fit.\n", ch);
			return;
		}

		obj_from_char(obj);
		obj_to_obj(obj, container);

		if (container->item_type == ITEM_CONTAINER && IS_SET(container->value[1], CONT_PUT_ON)) {
			act("$n puts $p on $P.", ch, obj, container, TO_ROOM);
			act("You put $p on $P.", ch, obj, container, TO_CHAR);
		}
		else {
			act("$n puts $p in $P.", ch, obj, container, TO_ROOM);
			act("You put $p in $P.", ch, obj, container, TO_CHAR);
		}

		return;
	}

	/* 'put all container' or 'put all.obj container' */
	for (obj = ch->carrying; obj != NULL; obj = obj_next) {
		obj_next = obj->next_content;

		if ((arg1[3] == '\0' || is_name(&arg1[4], obj->name))
		    && can_see_obj(ch, obj)
		    && obj->wear_loc == WEAR_NONE
		    && obj != container
		    && can_drop_obj(ch, obj)) {
			found = TRUE;

			if (!will_fit(obj, container))
				continue;

			tooheavy = FALSE;
			obj_from_char(obj);
			obj_to_obj(obj, container);

			if (container->item_type == ITEM_CONTAINER && IS_SET(container->value[1], CONT_PUT_ON)) {
				act("$n puts $p on $P.", ch, obj, container, TO_ROOM);
				act("You put $p on $P.", ch, obj, container, TO_CHAR);
			}
			else {
				act("$n puts $p in $P.", ch, obj, container, TO_ROOM);
				act("You put $p in $P.", ch, obj, container, TO_CHAR);
			}
		}
	}

	if (!found)
		stc("You do not have those.\n", ch);
	else if (tooheavy)
		stc("They won't fit.\n", ch);
}


void do_drop(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	bool found;
	char obj_name[MAX_INPUT_LENGTH];
	int number;
	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Drop what?\n", ch);
		return;
	}

	if (is_number(arg)) {
		/* 'drop NNNN coins' */
		int amount, gold = 0, silver = 0;
		amount   = atoi(arg);
		argument = one_argument(argument, arg);

		if (amount <= 0
		    || (str_prefix1(arg, "coins") && str_prefix1(arg, "coin") &&
		        str_prefix1(arg, "gold") && str_prefix1(arg, "silver"))) {
			stc("Sorry, you can't do that.\n", ch);
			return;
		}

		if (!str_prefix1(arg, "coins") || !str_prefix1(arg, "coin")
		    ||   !str_prefix1(arg, "silver")) {
			if (ch->silver < amount) {
				stc("You don't have that much silver.\n", ch);
				return;
			}

			ch->silver -= amount;
			silver = amount;
		}
		else {
			if (ch->gold < amount) {
				stc("You don't have that much gold.\n", ch);
				return;
			}

			ch->gold -= amount;
			gold = amount;
		}

		/* combine the money dropped with any already in the room */
		for (obj = ch->in_room->contents; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;

			if (obj->pIndexData->vnum == GEN_OBJ_MONEY
			    && is_name(obj->name, "gcash")) {
				silver  += obj->value[0];
				gold    += obj->value[1];
				extract_obj(obj);
			}
		}

		act("$n lets some coins tumble to the floor.", ch, NULL, NULL, TO_ROOM);
		stc("You flip some coins to the ground.\n", ch);
		obj_to_room(create_money(gold, silver), ch->in_room);
		return;
	}

	if (!str_cmp(arg, "all") || !str_prefix1("all.", arg)) {
		/* 'drop all' or 'drop all.obj' */
		found = FALSE;

		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;

			if ((arg[3] == '\0' || is_name(&arg[4], obj->name))
			    &&   can_see_obj(ch, obj)
			    &&   obj->wear_loc == WEAR_NONE) {
				found = TRUE;

				if (can_drop_obj(ch, obj)) {
					obj_from_char(obj);
					act("$n drops $p.", ch, obj, NULL, TO_ROOM);
					act("You drop $p.", ch, obj, NULL, TO_CHAR);
					obj_to_room(obj, ch->in_room);

					if (IS_OBJ_STAT(obj, ITEM_MELT_DROP)) {
						act("$p crumbles into dust.", ch, obj, NULL, TO_ROOM);
						act("$p crumbles into dust.", ch, obj, NULL, TO_CHAR);
						extract_obj(obj);
					}
				}
				else
					act("You can't seem to let go of $p.", ch, obj, NULL, TO_CHAR);
			}
		}

		if (!found) {
			if (arg[3] == '\0')
				act("You are not carrying anything.",
				    ch, NULL, arg, TO_CHAR);
			else
				act("You do not have a $T.",
				    ch, NULL, &arg[4], TO_CHAR);
		}

		return;
	}

	/* 'drop [n*]obj' */
	number = mult_argument(arg, obj_name);

	if (number == -1) {
		sprintf(arg, "You patiently wait for something called "
		        "'%s' to drop on you from the sky.\n", obj_name);
		stc(arg, ch);
		return;
	}

	if (number < 0) {
		sprintf(arg, "You patiently wait for %d things called "
		        "'%s' to drop on you from the sky.\n", -number, obj_name);
		stc(arg, ch);
		return;
	}

	if ((obj = get_obj_carry(ch, obj_name)) == NULL) {
		stc("You do not have that item.\n", ch);
		return;
	}

	if (!can_drop_obj(ch, obj)) {
		stc("You can't seem to let go of it.\n", ch);
		return;
	}

	if (number == 0) {
		stc("You drop 0 items and end up with what you had before.\n", ch);
		return;
	}

	if (number == 1) {
		/* single object, simple processing. */
		obj_from_char(obj);
		act("$n drops $p.", ch, obj, NULL, TO_ROOM);
		act("You drop $p.", ch, obj, NULL, TO_CHAR);
		obj_to_room(obj, ch->in_room);

		if (IS_OBJ_STAT(obj, ITEM_MELT_DROP)) {
			act("$p dissolves into smoke.", ch, obj, NULL, TO_ROOM);
			act("$p dissolves into smoke.", ch, obj, NULL, TO_CHAR);
			extract_obj(obj);
		}

		return;
	}
	else {
		sh_int obj_vnum;
		OBJ_DATA *op, *obj_next;
		int count = 1;
		/* drop <number> items, where <number> > 1. */
		/* obtain vnum for the first object and count how many more of the
		   same vnum the character has available and is able to drop. */
		obj_vnum = obj->pIndexData->vnum;

		for (op = obj->next_content; op; op = op->next_content) {
			if (op->pIndexData->vnum != obj_vnum)
				continue;

			count++;

			if (!can_drop_obj(ch, op)) {
				act("The $T one sticks to you, preventing "
				    "you from dropping that many.\n",
				    ch, NULL, ordinal_string(count), TO_CHAR);
				return;
			}

			if (count >= number)
				break;
		}

		sprintf(arg, "%d", count);

		if (count < number) {
			act("But you only have $T of those!", ch, NULL, arg, TO_CHAR);
			return;
		}

		/* must do visible messages before dropping */
		act("$n drops $p[$T].", ch, obj, arg, TO_ROOM);
		act("You drop $p[$T].", ch, obj, arg, TO_CHAR);

		if (IS_OBJ_STAT(obj, ITEM_MELT_DROP)) {
			act("$p[$T] dissolve into smoke.", ch, obj, arg, TO_ROOM);
			act("$p[$T] dissolve into smoke.", ch, obj, arg, TO_CHAR);
		}

		/* Loop through again, dropping <number> objects. */
		count = 0;

		for (op = obj; op; op = obj_next) {
			obj_next = op->next_content;

			if (op->pIndexData->vnum != obj_vnum)
				continue;

			count++;
			obj_from_char(op);

			if (IS_OBJ_STAT(op, ITEM_MELT_DROP))
				extract_obj(op);
			else
				obj_to_room(op, ch->in_room);

			if (count >= number)
				break;
		}
	}
} /* end do_drop() */

void do_give(CHAR_DATA *ch, const char *argument)
{
	char arg1[MIL], arg2[MIL], buf[MSL];
	CHAR_DATA *victim;
	OBJ_DATA *obj, *op, *obj_next;
	int number, count = 0, item_number = 0, item_weight = 0;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		stc("Give what to whom?\n", ch);
		return;
	}

	if (is_number(arg1)) {
		/* 'give NNNN coins victim' */
		int amount;
		bool silver;
		amount = atoi(arg1);

		if (amount <= 0) {
			stc("You try to hand out imaginary money.  Cheapskate.\n", ch);
			return;
		}

		if (str_cmp(arg2, "coins")
		    && str_cmp(arg2, "coin")
		    && str_cmp(arg2, "gold")
		    && str_cmp(arg2, "silver")) {
			stc("Use give <number> <gold|silver> <person>.\n", ch);
			return;
		}

		silver = str_cmp(arg2, "gold");
		argument = one_argument(argument, arg2);

		if (arg2[0] == '\0') {
			stc("Give the money to whom?\n", ch);
			return;
		}

		if ((victim = get_char_here(ch, arg2, VIS_CHAR)) == NULL) {
			stc("They seem to have left.\n", ch);
			return;
		}

		if (victim == ch) {
			stc("Indignant, you tell yourself that you don't need any help.\n", ch);
			return;
		}

		if ((!silver && ch->gold < amount)
		    || (silver && ch->silver < amount)) {
			stc("You aren't that rich.\n", ch);
			return;
		}

		if (!IS_NPC(victim)) {
			if (!IS_IMMORTAL(ch) && IS_SET(victim->pcdata->plr, PLR_LINK_DEAD)) {
				sprintf(buf, "$N is trying to give an object to the linkdead character %s.", victim->name);
				wiznet(buf, ch, NULL, WIZ_CHEAT, 0, GET_RANK(ch));
				stc("Your recipient cannot receive objects in their current state.\n", ch);
				return;
			}

			if ((!silver && (get_carry_weight(victim) + gold_weight(amount) > can_carry_w(victim)))
			    || (silver && (get_carry_weight(victim) + silver_weight(amount) > can_carry_w(victim)))) {
				stc("They can't carry that much weight.", ch);
				return;
			}
		}

		if (IS_NPC(victim) && IS_SET(victim->act, ACT_IS_CHANGER)) {
			int change;
			change = (silver ? 95 * amount / 10000 : 95 * amount);

			if (IS_NPC(ch) && IS_SET(ch->act, ACT_IS_CHANGER)) {
				stc("You don't need more money.\n", ch);
				return;
			}

			if ((!silver
			     && (get_carry_weight(ch)
			         + gold_weight(change)
			         - silver_weight(amount) > can_carry_w(ch)))
			    || (silver
			        && (get_carry_weight(ch)
			            + silver_weight(change)
			            - gold_weight(amount) > can_carry_w(ch)))) {
				stc("But you wouldn't be able to carry that much change!\n", ch);
				return;
			}
		}

		if (silver) {
			ch->silver -= amount;
			victim->silver += amount;
		}
		else {
			ch->gold -= amount;
			victim->gold += amount;
		}

		if (amount == 1)
			sprintf(buf, "$n gives you one %s coin.", silver ? "silver" : "gold");
		else
			sprintf(buf, "$n gives you %d %s.", amount, silver ? "silver" : "gold");

		act(buf, ch, NULL, victim, TO_VICT);
		sprintf(buf, "$n gives $N %s %s coin%s.  Must be nice to be rich.",
		        amount == 1 ? "one" : "some",
		        silver ? "silver" : "gold",
		        amount == 1 ? "" : "s");
		act(buf, ch, NULL, victim, TO_NOTVICT);

		if (amount == 1)
			sprintf(buf, "You give one %s coin to $N.", silver ? "silver" : "gold");
		else
			sprintf(buf, "You give %d %s to $N.", amount, silver ? "silver" : "gold");

		act(buf, ch, NULL, victim, TO_CHAR);
		mprog_bribe_trigger(victim, ch, silver ? amount : amount * 100);

		if (IS_NPC(victim) && IS_SET(victim->act, ACT_IS_CHANGER)) {
			int change;
			change = (silver ? 95 * amount / 10000 : 95 * amount);

			if (!silver && change > victim->silver)
				victim->silver += change;

			if (silver && change > victim->gold)
				victim->gold += change;

			if (!can_see_char(victim, ch)) {
				act("$n looks around for whoever could be so generous.", victim, NULL, NULL, TO_ROOM);
				return;
			}

			if (change < 1) {
				act("$n tells you 'I'm sorry, you did not give me enough to change.'", victim, NULL, ch, TO_VICT);
				strcpy(ch->reply, victim->name);
				sprintf(buf, "%d %s %s", amount, silver ? "silver" : "gold", ch->name);
				do_give(victim, buf);
				return;
			}

			sprintf(buf, "%d %s %s", change, silver ? "gold" : "silver", ch->name);
			do_give(victim, buf);

			if (silver) {
				sprintf(buf, "%d silver %s", (95 * amount / 100 - change * 100), ch->name);
				do_give(victim, buf);
			}

			act("$n tells you 'What do you think I am?  Made of money?!'", victim, NULL, ch, TO_VICT);
			strcpy(ch->reply, victim->name);
		}

		return;
	}

	/* 'normal' give [n*]<object> <victim> */
	number = mult_argument(arg1, buf);
	strcpy(arg1, buf);

	if (number < 0) {
		stc("If you want more than you had, try stealing!\n", ch);
		return;
	}

	if (number == 0) {
		stc("You give away 0 items, ending up with what you had before.\n", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, arg1)) == NULL) {
		stc("You do not have that item.\n", ch);
		return;
	}

	if (obj->wear_loc != WEAR_NONE) {
		stc("You must remove it first.\n", ch);
		return;
	}

	/* count number of objects of given vnum available */
	for (op = obj; op; op = op->next_content) {
		if (op->pIndexData->vnum != obj->pIndexData->vnum)
			continue;

		count++;
		item_number += get_obj_number(op);
		item_weight += get_obj_weight(op);

		if (!can_drop_obj(ch, op)) {
			if (number == 1)
				stc("You cannot seem to let go of it.\n", ch);
			else
				ptc(ch, "You cannot seem to let go of the %s one.", ordinal_string(count));

			return;
		}

		if (count >= number)
			break;
	}

	if (count < number) {
		ptc(ch, "But you only have %d of those!", count);
		return;
	}

	if ((victim = get_char_here(ch, arg2, VIS_CHAR)) == NULL) {
		stc("They are not here.\n", ch);
		return;
	}

	if (victim == ch) {
		stc("Indignant, you tell yourself that you don't need any help.\n", ch);
		return;
	}

	if (!IS_NPC(victim) && !IS_IMMORTAL(ch) && IS_SET(victim->pcdata->plr, PLR_LINK_DEAD)) {
		sprintf(buf, "$N is trying to give an object to the linkdead character %s.", victim->name);
		wiznet(buf, ch, NULL, WIZ_CHEAT, 0, GET_RANK(ch));
		stc("Your recipient cannot receive objects in their current state.\n", ch);
		return;
	}

	if (IS_NPC(victim) && victim->pIndexData->pShop != NULL) {
		act("$N tells you 'Sorry, you'll have to sell that.'", ch, NULL, victim, TO_CHAR);
		strcpy(ch->reply, victim->name);
		return;
	}

	if (!can_drop_obj(ch, obj)) {
		stc("You can't let go of it.\n", ch);
		return;
	}

	if (!can_see_obj(victim, obj)) {
		/* hack so questmobs can take questobjs, easier than a bunch of reverse lookups in can_see_obj */
		if (!(!IS_NPC(ch) && IS_NPC(victim)
		      && IS_SQUESTOR(ch) && ch->pcdata->squestobj != NULL && ch->pcdata->squestmob != NULL
		      && obj == ch->pcdata->squestobj && victim == ch->pcdata->squestmob)) {
			act("$N can't see it.", ch, NULL, victim, TO_CHAR);
			return;
		}
	}

	if (get_carry_number(victim) + item_number > can_carry_n(victim)) {
		act("$N is already carrying too much.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (get_carry_weight(victim) + item_weight > can_carry_w(victim)) {
		act("$N buckles under that much weight.", ch, NULL, victim, TO_CHAR);
		return;
	}

	/* transfer <number> objects */
	for (op = obj, count = 0; op; op = obj_next) {
		obj_next = op->next_content;

		if (op->pIndexData->vnum != obj->pIndexData->vnum)
			continue;

		obj_from_char(op);
		obj_to_char(op, victim);
		count++;

		if (count >= number)
			break;
	}

	if (number == 1) {
		act("$n gives $p to $N.", ch, obj, victim, TO_NOTVICT);
		act("$n gives you $p.",   ch, obj, victim, TO_VICT);
		act("You give $p to $N.", ch, obj, victim, TO_CHAR);
	}
	else {
		sprintf(buf, "$n gives $p[%d] to $N.", number);
		act(buf, ch, obj, victim, TO_NOTVICT);
		sprintf(buf, "$n gives you $p[%d].", number);
		act(buf, ch, obj, victim, TO_VICT);
		sprintf(buf, "You give $p[%d] to $N.", number);
		act(buf, ch, obj, victim, TO_CHAR);
	}

	if (!IS_NPC(ch) && IS_SET(ch->pcdata->plr, PLR_SQUESTOR)
	    && ch->pcdata->squestmob != NULL && ch->pcdata->squestobj != NULL) {
		if (obj == ch->pcdata->squestobj && victim == ch->pcdata->squestmob) {
			extern void squestobj_to_squestmob args((CHAR_DATA * ch, OBJ_DATA * obj, CHAR_DATA * mob));

			if (!ch->pcdata->squestobjf) {
				bug("At give sqobj to sqmob without sqobj found, continuing...", 0);
				ch->pcdata->squestobjf = TRUE;
			}

			squestobj_to_squestmob(ch, obj, victim);
		}
	}

	/* This will trigger only once. We don't want anything to explode. */
	/* If they want to achieve a given effect multiple times, they need */
	/* to use the single form of give. */
	mprog_give_trigger(victim, ch, obj);
}

/* for poisoning weapons and food/drink */
void do_envenom(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	int percent, skill;

	/* find out what */
	if (argument[0] == '\0') {
		stc("Envenom what item?\n", ch);
		return;
	}

	obj =  get_obj_list(ch, argument, ch->carrying);

	if (obj == NULL) {
		stc("You do not have that item.\n", ch);
		return;
	}

	if ((skill = get_skill(ch, gsn_envenom)) < 1) {
		stc("Are you crazy? You'd poison yourself!\n", ch);
		return;
	}

	if (!deduct_stamina(ch, gsn_envenom))
		return;

	if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON) {
		if (IS_OBJ_STAT(obj, ITEM_BLESS) || IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
			act("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
			return;
		}

		if (number_percent() < skill) { /* success! */
			act("$n treats $p with deadly poison.", ch, obj, NULL, TO_ROOM);
			act("You treat $p with deadly poison.", ch, obj, NULL, TO_CHAR);

			if (!obj->value[3]) {
				obj->value[3] = 1;
				check_improve(ch, gsn_envenom, TRUE, 4);
			}

			WAIT_STATE(ch, skill_table[gsn_envenom].beats);
			return;
		}

		act("You fail to poison $p.", ch, obj, NULL, TO_CHAR);

		if (!obj->value[3])
			check_improve(ch, gsn_envenom, FALSE, 4);

		WAIT_STATE(ch, skill_table[gsn_envenom].beats);
		return;
	}

	if (obj->item_type == ITEM_WEAPON) {
		/* Lets let them envenom special weapons - Lotus
		                if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
		                ||  IS_WEAPON_STAT(obj,WEAPON_FROST)
		                ||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
		                ||  IS_WEAPON_STAT(obj,WEAPON_SHARP)
		                ||  IS_WEAPON_STAT(obj,WEAPON_VORPAL)
		                ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
		                ||  IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
		                {
		                        act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
		                        return;
		                }
		*/
		if (obj->value[3] < 0
		    ||  attack_table[obj->value[3]].damage == DAM_BASH) {
			stc("You can only envenom edged weapons.\n", ch);
			return;
		}

		if (IS_WEAPON_STAT(obj, WEAPON_POISON)) {
			act("$p is already envenomed.", ch, obj, NULL, TO_CHAR);
			return;
		}

		percent = number_percent();

		if (percent < skill) {
			AFFECT_DATA af = (AFFECT_DATA){0};
			af.where     = TO_WEAPON;
			af.type      = gsn_poison;
			af.level     = ch->level;
			af.duration  = ch->level * 5;
			af.location  = 0;
			af.modifier  = 0;
			af.bitvector = WEAPON_POISON;
			af.evolution = get_evolution(ch, gsn_envenom);
			affect_copy_to_obj(obj, &af);
			act("$n coats $p with deadly venom.", ch, obj, NULL, TO_ROOM);
			act("You coat $p with venom.", ch, obj, NULL, TO_CHAR);
			check_improve(ch, gsn_envenom, TRUE, 3);
			WAIT_STATE(ch, skill_table[gsn_envenom].beats);
			return;
		}
		else {
			act("You fail to envenom $p.", ch, obj, NULL, TO_CHAR);
			check_improve(ch, gsn_envenom, FALSE, 3);
			WAIT_STATE(ch, skill_table[gsn_envenom].beats);
			return;
		}
	}

	act("You can't poison $p.", ch, obj, NULL, TO_CHAR);
	return;
}
/* Firebuilding by Lotus */
void do_firebuilding(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *torch;

	if (get_carry_number(ch) + 1 > can_carry_n(ch)) {
		act("You can't carry any more items.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (get_carry_weight(ch) >= can_carry_w(ch)) {
		act("You can't carry any more weight.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (!get_skill(ch, gsn_firebuilding)) {
		stc("You lack the knowledge it takes to make a torch!\n", ch);
		return;
	}

	if (!deduct_stamina(ch, gsn_firebuilding))
		return;

	if ((ch->in_room->sector_type == SECT_CITY)
	    || (ch->in_room->sector_type == SECT_WATER_SWIM)
	    || (ch->in_room->sector_type == SECT_WATER_NOSWIM)
	    || (ch->in_room->sector_type == SECT_DESERT)
	    || (ch->in_room->sector_type == SECT_AIR)) {
		stc("You cannot find any twigs to make a torch.\n", ch);
		return;
	}

	WAIT_STATE(ch, skill_table[gsn_firebuilding].beats);

	if (number_percent() > get_skill(ch, gsn_firebuilding)) {
		stc("You burn yourself.\n", ch);
		check_improve(ch, gsn_firebuilding, FALSE, 8);
		return;
	}

	torch = create_object(get_obj_index(OBJ_VNUM_TORCH), 0);

	if (! torch) {
		bug("Error creating a torch in firebuilding.", 0);
		stc("You were unable to make a torch.\n", ch);
		return;
	}

	torch->cost = 0;
	obj_to_char(torch, ch);
	act("$n gathers some twigs and creates $p.", ch, torch, NULL, TO_ROOM);
	act("You gather some twigs and create $p.", ch, torch, NULL, TO_CHAR);
	torch->value[2] = -1;
	check_improve(ch, gsn_firebuilding, TRUE, 8);
	return;
}

void do_fill(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *first_fountain = NULL;
	OBJ_DATA *wanted_fountain = NULL;
	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Fill what?\n", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		stc("You do not have that item.\n", ch);
		return;
	}

	if (obj->item_type != ITEM_DRINK_CON) {
		stc("You can't fill that.\n", ch);
		return;
	}

	for (first_fountain = ch->in_room->contents; first_fountain != NULL;
	     first_fountain = first_fountain->next_content)
		if (first_fountain->item_type == ITEM_FOUNTAIN)
			break;

	if (first_fountain == NULL) {
		stc("From what?\n", ch);
		return;
	}

	wanted_fountain = first_fountain;
	/* check for a user requested OTHER fountain -- Elrac */
	argument = one_argument(argument, arg);

	if (arg[0] != '\0') {
		if (!str_prefix1(arg, "from"))
			argument = one_argument(argument, arg);

		if (arg[0] != '\0') {
			wanted_fountain = get_obj_here(ch, arg);

			if (wanted_fountain == NULL) {
				stc("You don't see that here.\n", ch);
				return;
			}

			if (wanted_fountain->item_type != ITEM_FOUNTAIN) {
				stc("That's not a fountain.\n", ch);
				return;
			}
		}
	}

	if (obj->value[1] != 0 && obj->value[2] != wanted_fountain->value[2]) {
		stc("There is already another liquid in it.\n", ch);
		return;
	}

	if (obj->value[1] >= obj->value[0]) {
		stc("Your container brims over with liquid.\n", ch);
		return;
	}

	sprintf(buf, "You fill $p with %s from $P.",
	        liq_table[wanted_fountain->value[2]].liq_name);
	act(buf, ch, obj, wanted_fountain, TO_CHAR);
	sprintf(buf, "$n fills $p with %s from $P.",
	        liq_table[wanted_fountain->value[2]].liq_name);
	act(buf, ch, obj, wanted_fountain, TO_ROOM);
	obj->value[2] = wanted_fountain->value[2];
	obj->value[1] = obj->value[0];
	obj->value[3] = wanted_fountain->value[3];      /* poison the drink */
} /* end do_fill() */

void do_pour(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
	OBJ_DATA *out, *in;
	CHAR_DATA *vch = NULL;
	int amount;
	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0') {
		stc("Pour what into what?\n", ch);
		return;
	}

	if ((out = get_obj_carry(ch, arg)) == NULL) {
		stc("You do not have that item.\n", ch);
		return;
	}

	if (out->item_type != ITEM_DRINK_CON) {
		stc("That's not a drink container.\n", ch);
		return;
	}

	if (!str_cmp(argument, "out")) {
		if (out->value[1] == 0) {
			stc("It's already empty.\n", ch);
			return;
		}

		out->value[1] = 0;
		out->value[3] = 0;      /* no more poison */
		sprintf(buf, "You invert $p, spilling %s all over the ground.",
		        liq_table[out->value[2]].liq_name);
		act(buf, ch, out, NULL, TO_CHAR);
		sprintf(buf, "$n inverts $p, spilling %s all over the ground.",
		        liq_table[out->value[2]].liq_name);
		act(buf, ch, out, NULL, TO_ROOM);
		return;
	}

	if ((in = get_obj_here(ch, argument)) == NULL) {
		vch = get_char_here(ch, argument, VIS_CHAR);

		if (vch == NULL) {
			stc("Pour into what?\n", ch);
			return;
		}

		in = get_eq_char(vch, WEAR_HOLD);

		if (in == NULL) {
			stc("They aren't holding anything.", ch);
			return;
		}
	}

	if (in->item_type != ITEM_DRINK_CON) {
		stc("You can only pour into other drink containers.\n", ch);
		return;
	}

	if (in == out) {
		stc("You cannot change the laws of physics!\n", ch);
		return;
	}

	if (in->value[1] != 0 && in->value[2] != out->value[2]) {
		stc("They don't hold the same liquid.\n", ch);
		return;
	}

	if (out->value[1] == 0) {
		act("There's nothing in $p to pour.", ch, out, NULL, TO_CHAR);
		return;
	}

	if (in->value[1] >= in->value[0]) {
		act("$p is already filled to the top.", ch, in, NULL, TO_CHAR);
		return;
	}

	amount = UMIN(out->value[1], in->value[0] - in->value[1]);
	in->value[1] += amount;
	out->value[1] -= amount;
	in->value[2] = out->value[2];
	in->value[3] = out->value[3];   /* transfer the poison */

	if (vch == NULL) {
		sprintf(buf, "You pour %s from $p into $P.",
		        liq_table[out->value[2]].liq_name);
		act(buf, ch, out, in, TO_CHAR);
		sprintf(buf, "$n pours %s from $p into $P.",
		        liq_table[out->value[2]].liq_name);
		act(buf, ch, out, in, TO_ROOM);
	}
	else {
		sprintf(buf, "You pour some %s for $N.",
		        liq_table[out->value[2]].liq_name);
		act(buf, ch, NULL, vch, TO_CHAR);
		sprintf(buf, "$n pours you some %s.",
		        liq_table[out->value[2]].liq_name);
		act(buf, ch, NULL, vch, TO_VICT);
		sprintf(buf, "$n pours some %s for $N.",
		        liq_table[out->value[2]].liq_name);
		act(buf, ch, NULL, vch, TO_NOTVICT);
	}
}

void do_drink(CHAR_DATA *ch, const char *argument)
{
	char arg[MIL];
	OBJ_DATA *obj;
	int amount, liquid, liqvalue;
	argument = one_argument(argument, arg);

	if (arg[0] != '\0' && argument[0] != '\0' && !str_cmp(arg, "from"))
		one_argument(argument, arg);

	if (arg[0] == '\0') {
		for (obj = ch->in_room->contents; obj; obj = obj->next_content)
			if (obj->item_type == ITEM_FOUNTAIN)
				break;

		if (obj == NULL)
			for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
				if (obj->item_type == ITEM_DRINK_CON)
					break;

		if (obj == NULL) {
			stc("Drink from what?\n", ch);
			return;
		}
	}
	else {
		if ((obj = get_obj_here(ch, arg)) == NULL) {
			stc("You can't find it.\n", ch);
			return;
		}
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10) {
		stc("The container fails to reach your mouth.  *Hic*\n", ch);
		return;
	}

	switch (obj->item_type) {
	default:
		stc("You can't drink from that.\n", ch);
		return;

	case ITEM_FOUNTAIN:
		if ((liquid = obj->value[2]) < 0) {
			bug("Do_drink: bad liquid number %d.", liquid);
			liquid = obj->value[2] = 0;
		}

		amount = liq_table[liquid].liq_affect[4] * 3;
		break;

	case ITEM_DRINK_CON:
		if (obj->value[1] <= 0) {
			stc("Your container is empty.\n", ch);
			return;
		}

		if ((liquid = obj->value[2]) < 0) {
			bug("Do_drink: bad liquid number %d.", liquid);
			liquid = obj->value[2] = 0;
		}

		amount = liq_table[liquid].liq_affect[4];
		amount = UMIN(amount, obj->value[1]);
		break;
	}

	if (!IS_NPC(ch) && !IS_IMMORTAL(ch)
	    && ch->pcdata->condition[COND_FULL] > 45) {
		stc("You're too full to drink more.\n", ch);
		return;
	}

	act("$n drinks $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_ROOM);
	act("You drink $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_CHAR);
	gain_condition(ch, COND_DRUNK,  amount * liq_table[liquid].liq_affect[COND_DRUNK]  / 36);
	gain_condition(ch, COND_FULL,   amount * liq_table[liquid].liq_affect[COND_FULL]   /  4);
	gain_condition(ch, COND_HUNGER, amount * liq_table[liquid].liq_affect[COND_HUNGER] /  2);

	if (ch->race == 6 && liquid == drink_lookup("blood"))   /* vampires */
		liqvalue = 10;          /* same as water */
	else
		liqvalue = liq_table[liquid].liq_affect[COND_THIRST];

	gain_condition(ch, COND_THIRST, amount * liqvalue / 10);

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
		stc("You're toasted. The room begins to spin!\n", ch);

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL]  > 40)
		stc("You are full.\n", ch);

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 40)
		stc("Your thirst is quenched.\n", ch);

	if (obj->value[3] != 0) {
		/* The drink was poisoned! */
		act("$n turns six shades of green and collapses.", ch, NULL, NULL, TO_ROOM);
		stc("You turn six shades of green and collapse.\n", ch);

		affect_add_sn_to_char(ch,
			gsn_poison,
			number_fuzzy(amount),
			amount * 3,
			1,
			FALSE
		);
	}

	if (obj->value[0] > 0) {
		obj->value[1] = UMAX(obj->value[1] - amount, 0);

		/* if container is now empty, remove the poison */
		if (obj->value[1] == 0)
			obj->value[3] = 0;
	}
}

void do_eat(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj, *op, *obj_next;
	int number, count;
	sh_int obj_vnum;
	bool fFull = FALSE, fNoLongerHungry = FALSE, fPoisoned = FALSE, found = FALSE;
	OBJ_DATA *to_extract = NULL;
	number = mult_argument(argument, buf);
	one_argument(buf, arg);

	if (number < 0) {
		stc("Nice try, but that's not the way to diet.\n", ch);
		return;
	}

	if (number == 0) {
		stc("OK, you eat nothing. Your waistline slims slightly.\n", ch);
		return;
	}

	if (arg[0] == '\0') {
		for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
			if (obj->item_type == ITEM_FOOD) {
				found = TRUE;
				break;
			}
		}

		if (!found) {
			stc("Eat what?\n", ch);
			return;
		}
	}
	else if ((obj = get_obj_carry(ch, arg)) == NULL) {
		stc("You do not have that item.\n", ch);
		return;
	}

	if (!IS_IMMORTAL(ch)) {
		if (obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL) {
			stc("You can't eat that!\n", ch);
			return;
		}

		if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 40) {
			stc("You are too full to eat more.\n", ch);
			return;
		}
	}

	/* see if they have that much */
	count = 0;

	for (op = obj; op; op = op->next_content) {
		if (op->pIndexData->vnum != obj->pIndexData->vnum)
			continue;

		count++;

		if (count >= number)
			break;
	}

	sprintf(buf, "%d", count);

	if (count < number) {
		act("But you only have $T of those!", ch, NULL, buf, TO_CHAR);
		return;
	}

	if (number > 10) {
		stc("You couldn't possibly fit more than ten of those in your mouth.\n", ch);
		return;
	}

	/* They have enough. Loop thru <number> items with that vnum and gobble. */
	obj_vnum = obj->pIndexData->vnum;
	count = 0;

	for (op = obj; op && !fFull; op = obj_next) {
		obj_next = op->next_content;

		if (op->pIndexData->vnum != obj_vnum)
			continue;

		if (op->item_type == ITEM_PILL) {
			if (ch->level < op->level && !IS_IMMORTAL(ch)) {
				stc("This pill is too powerful for you to eat.\n", ch);
				return;
			}

			act("$n eats $p.", ch, op, NULL, TO_ROOM);
			act("You eat $p.", ch, op, NULL, TO_CHAR);
			obj_cast_spell(op->value[1], op->value[0], ch, ch, NULL);
			obj_cast_spell(op->value[2], op->value[0], ch, ch, NULL);
			obj_cast_spell(op->value[3], op->value[0], ch, ch, NULL);
			obj_cast_spell(op->value[4], op->value[0], ch, ch, NULL);
		}
		else if (op->item_type == ITEM_FOOD) {
			if (ch->level < op->level && !IS_IMMORTAL(ch)) {
				stc("This food is too rich for you to eat.\n", ch);
				return;
			}

			if (op->value[3] != 0) {
				/* The food was poisoned! */
				affect_add_sn_to_char(ch,
					gsn_poison,
					number_fuzzy(op->level),
					op->level,
					1,
					FALSE
				);

				fPoisoned    = TRUE;
			}
		}

		/* make it so you can get full on pills, 4 gives 11 or 12 pills on an empty stomach */
		if (!IS_NPC(ch)) {
			int condition;
			condition = ch->pcdata->condition[COND_HUNGER];
			gain_condition(ch, COND_FULL, op->item_type == ITEM_PILL ? 4 : obj->value[0]);
			gain_condition(ch, COND_HUNGER, op->item_type == ITEM_PILL ? 2 : obj->value[1]);

			if (condition == 0 && ch->pcdata->condition[COND_HUNGER] > 0)
				fNoLongerHungry = TRUE;
			else if (ch->pcdata->condition[COND_FULL] > 40)
				fFull = TRUE;
		}

		/* delay extraction long enough to produce a valid message */
		if (to_extract != NULL)
			extract_obj(to_extract);

		to_extract = op;

		if (++count >= number)
			break;
	} /* end for */

	if (count == 1)
		strcpy(buf, "");
	else
		sprintf(buf, "[%d]", count);

	if (obj->item_type != ITEM_PILL) {
		act("$n eats $p$T.", ch, to_extract, buf, TO_ROOM);
		act("You eat $p$T.", ch, to_extract, buf, TO_CHAR);
	}

	if (fNoLongerHungry)
		stc("You are no longer hungry.\n", ch);

	if (fFull && !IS_IMMORTAL(ch))
		stc("You are full.\n", ch);

	if (fPoisoned) {
		act("$n turns six shades of green and collapses.", ch, 0, 0, TO_ROOM);
		stc("You turn six shades of green and collapse.\n", ch);
	}

	if (to_extract != NULL)
		extract_obj(to_extract);
} /* end do_eat() */

/*
 * Remove an object.
 */
bool remove_obj(CHAR_DATA *ch, int iWear, bool fReplace)
{
	OBJ_DATA *obj;

	if ((obj = get_eq_char(ch, iWear)) == NULL)
		return TRUE;

	if (!fReplace)
		return FALSE;

	if (IS_OBJ_STAT(obj, ITEM_NOREMOVE)) {
		act("You can't seem to remove $p.", ch, obj, NULL, TO_CHAR);
		return FALSE;
	}

	unequip_char(ch, obj);
	act("$n stops using $p.", ch, obj, NULL, TO_ROOM);
	act("You stop using $p.", ch, obj, NULL, TO_CHAR);
	return TRUE;
}

/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj(CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace)
{
	/* check for clan equipment -- Elrac */
	if (!clan_eq_ok(ch, obj, "equip yourself with"))
		return;

	/* check for personal equipment -- Elrac */
	if (!pers_eq_ok(ch, obj, "equip yourself with"))
		return;

	if (!IS_IMMORTAL(ch)) {
		if (obj->level > get_usable_level(ch)) {
			if (obj->level < LEVEL_IMMORTAL) {
				ptc(ch, "You must be level %d(%d) to use this object.\n",
				    ch->level + (obj->level - get_usable_level(ch)), obj->level);
				act("$n tries to use $p, but is too inexperienced.", ch, obj, NULL, TO_ROOM);
				return;
			}
			else {
				stc("This item must be lowered for mortal use.\n", ch);
				act("$n tries to use $p, but is too bound to the mortal plane.", ch, obj, NULL, TO_ROOM);
				return;
			}
		}
	}

	if (!IS_IMMORTAL(ch)) {
		if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch))
	    || (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch))
	    || (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
			act("You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR);
			act("$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM);
			obj_from_char(obj);
			obj_to_room(obj, ch->in_room);
			return;
			}
	}

	if (obj->item_type == ITEM_WEDDINGRING) {
		if (!remove_obj(ch, WEAR_WEDDINGRING, fReplace))
			return;

		act("$n wears $p on $s ringfinger.", ch, obj, NULL , TO_ROOM);
		act("You wear $p on your ringfinger.", ch, obj, NULL, TO_CHAR);
		equip_char(ch, obj, WEAR_WEDDINGRING);
		return;
	}

	if (obj->item_type == ITEM_LIGHT) {
		if (!remove_obj(ch, WEAR_LIGHT, fReplace))
			return;

		act("$n lights $p{x and holds it.", ch, obj, NULL, TO_ROOM);
		act("You light $p{x and hold it.",  ch, obj, NULL, TO_CHAR);
		equip_char(ch, obj, WEAR_LIGHT);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_FINGER)) {
		if (get_eq_char(ch, WEAR_FINGER_L) != NULL
		    &&   get_eq_char(ch, WEAR_FINGER_R) != NULL
		    &&   !remove_obj(ch, WEAR_FINGER_L, fReplace)
		    &&   !remove_obj(ch, WEAR_FINGER_R, fReplace))
			return;

		if (get_eq_char(ch, WEAR_FINGER_L) == NULL) {
			act("$n wears $p{x on $s left finger.",    ch, obj, NULL, TO_ROOM);
			act("You wear $p{x on your left finger.",  ch, obj, NULL, TO_CHAR);
			equip_char(ch, obj, WEAR_FINGER_L);
			return;
		}

		if (get_eq_char(ch, WEAR_FINGER_R) == NULL) {
			act("$n wears $p{x on $s right finger.",   ch, obj, NULL, TO_ROOM);
			act("You wear $p{x on your right finger.", ch, obj, NULL, TO_CHAR);
			equip_char(ch, obj, WEAR_FINGER_R);
			return;
		}

		bug("Wear_obj: no free finger.", 0);
		stc("You already wear two rings.\n", ch);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_NECK)) {
		if (get_eq_char(ch, WEAR_NECK_1) != NULL
		    &&   get_eq_char(ch, WEAR_NECK_2) != NULL
		    &&   !remove_obj(ch, WEAR_NECK_1, fReplace)
		    &&   !remove_obj(ch, WEAR_NECK_2, fReplace))
			return;

		if (get_eq_char(ch, WEAR_NECK_1) == NULL) {
			act("$n wears $p{x around $s neck.",   ch, obj, NULL, TO_ROOM);
			act("You wear $p{x around your neck.", ch, obj, NULL, TO_CHAR);
			equip_char(ch, obj, WEAR_NECK_1);
			return;
		}

		if (get_eq_char(ch, WEAR_NECK_2) == NULL) {
			act("$n wears $p{x around $s neck.",   ch, obj, NULL, TO_ROOM);
			act("You wear $p{x around your neck.", ch, obj, NULL, TO_CHAR);
			equip_char(ch, obj, WEAR_NECK_2);
			return;
		}

		bug("Wear_obj: no free neck.", 0);
		stc("You already wear two neck items.\n", ch);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_BODY)) {
		if (!remove_obj(ch, WEAR_BODY, fReplace))
			return;

		act("$n wears $p{x on $s torso.",   ch, obj, NULL, TO_ROOM);
		act("You wear $p{x on your torso.", ch, obj, NULL, TO_CHAR);
		equip_char(ch, obj, WEAR_BODY);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_HEAD)) {
		if (!remove_obj(ch, WEAR_HEAD, fReplace))
			return;

		act("$n wears $p{x on $s head.",   ch, obj, NULL, TO_ROOM);
		act("You wear $p{x on your head.", ch, obj, NULL, TO_CHAR);
		equip_char(ch, obj, WEAR_HEAD);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_LEGS)) {
		if (!remove_obj(ch, WEAR_LEGS, fReplace))
			return;

		act("$n wears $p{x on $s legs.",   ch, obj, NULL, TO_ROOM);
		act("You wear $p{x on your legs.", ch, obj, NULL, TO_CHAR);
		equip_char(ch, obj, WEAR_LEGS);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_FEET)) {
		if (!remove_obj(ch, WEAR_FEET, fReplace))
			return;

		act("$n wears $p{x on $s feet.",   ch, obj, NULL, TO_ROOM);
		act("You wear $p{x on your feet.", ch, obj, NULL, TO_CHAR);
		equip_char(ch, obj, WEAR_FEET);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_HANDS)) {
		if (!remove_obj(ch, WEAR_HANDS, fReplace))
			return;

		act("$n wears $p{x on $s hands.",   ch, obj, NULL, TO_ROOM);
		act("You wear $p{x on your hands.", ch, obj, NULL, TO_CHAR);
		equip_char(ch, obj, WEAR_HANDS);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_ARMS)) {
		if (!remove_obj(ch, WEAR_ARMS, fReplace))
			return;

		act("$n wears $p{x on $s arms.",   ch, obj, NULL, TO_ROOM);
		act("You wear $p{x on your arms.", ch, obj, NULL, TO_CHAR);
		equip_char(ch, obj, WEAR_ARMS);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_ABOUT)) {
		if (!remove_obj(ch, WEAR_ABOUT, fReplace))
			return;

		act("$n wears $p{x about $s body.",   ch, obj, NULL, TO_ROOM);
		act("You wear $p{x about your body.", ch, obj, NULL, TO_CHAR);
		equip_char(ch, obj, WEAR_ABOUT);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_WAIST)) {
		if (!remove_obj(ch, WEAR_WAIST, fReplace))
			return;

		act("$n wears $p{x about $s waist.",   ch, obj, NULL, TO_ROOM);
		act("You wear $p{x about your waist.", ch, obj, NULL, TO_CHAR);
		equip_char(ch, obj, WEAR_WAIST);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_WRIST)) {
		if (get_eq_char(ch, WEAR_WRIST_L) != NULL
		    &&   get_eq_char(ch, WEAR_WRIST_R) != NULL
		    &&   !remove_obj(ch, WEAR_WRIST_L, fReplace)
		    &&   !remove_obj(ch, WEAR_WRIST_R, fReplace))
			return;

		if (get_eq_char(ch, WEAR_WRIST_L) == NULL) {
			act("$n wears $p{x around $s left wrist.",
			    ch, obj, NULL, TO_ROOM);
			act("You wear $p{x around your left wrist.",
			    ch, obj, NULL, TO_CHAR);
			equip_char(ch, obj, WEAR_WRIST_L);
			return;
		}

		if (get_eq_char(ch, WEAR_WRIST_R) == NULL) {
			act("$n wears $p{x around $s right wrist.",
			    ch, obj, NULL, TO_ROOM);
			act("You wear $p{x around your right wrist.",
			    ch, obj, NULL, TO_CHAR);
			equip_char(ch, obj, WEAR_WRIST_R);
			return;
		}

		bug("Wear_obj: no free wrist.", 0);
		stc("You already wear two wrist items.\n", ch);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_SHIELD)) {
		OBJ_DATA *weapon;

		if (get_eq_char(ch, WEAR_SECONDARY) != NULL && !IS_IMMORTAL(ch)) {
			stc("You cannot use a shield while using two weapons.\n", ch);
			return;
		}

		if (!remove_obj(ch, WEAR_SHIELD, fReplace))
			return;

		weapon = get_eq_char(ch, WEAR_WIELD);

		if (weapon != NULL && ch->size < SIZE_LARGE
		    &&  IS_WEAPON_STAT(weapon, WEAPON_TWO_HANDS)) {
			stc("You need another hand free to do that.\n", ch);
			return;
		}

		act("$n wears $p{x as a shield.", ch, obj, NULL, TO_ROOM);
		act("You wear $p{x as a shield.", ch, obj, NULL, TO_CHAR);
		equip_char(ch, obj, WEAR_SHIELD);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WIELD)) {
		int sn, skill;

		if (!remove_obj(ch, WEAR_WIELD, fReplace))
			return;

		if (!IS_NPC(ch)
		    && get_obj_weight(obj) > (str_app[GET_ATTR_STR(ch)].wield
		                              * 10)) {
			stc("It's too heavy for you to pick up.\n", ch);
			return;
		}

		if (!IS_NPC(ch) && ch->size < SIZE_LARGE
		    &&  IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS)
		    &&  get_eq_char(ch, WEAR_SHIELD) != NULL) {
			stc("You need two hands free for that weapon.\n", ch);
			return;
		}

		if (!IS_NPC(ch) && IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS) &&
		    get_eq_char(ch, WEAR_SECONDARY) != NULL) {
			stc("You can not use a two handed weapon when dual-wielding.\n", ch);
			return;
		}

		act("$n wields $p{x.", ch, obj, NULL, TO_ROOM);
		act("You wield $p{x.", ch, obj, NULL, TO_CHAR);
		equip_char(ch, obj, WEAR_WIELD);
		sn = get_weapon_sn(ch, FALSE);

		if (sn == gsn_hand_to_hand)
			return;

		skill = get_weapon_skill(ch, sn);

		if (skill >= 100)
			act("You feel at one with $p{x.", ch, obj, NULL, TO_CHAR);
		else if (skill > 85)
			act("You feel quite confident with $p{x.", ch, obj, NULL, TO_CHAR);
		else if (skill > 70)
			act("You are skilled with $p{x.", ch, obj, NULL, TO_CHAR);
		else if (skill > 50)
			act("Your skill with $p is adequate{x.", ch, obj, NULL, TO_CHAR);
		else if (skill > 25)
			act("$p feels a little clumsy in your hands{x.", ch, obj, NULL, TO_CHAR);
		else if (skill > 1)
			act("You fumble and almost drop $p{x.", ch, obj, NULL, TO_CHAR);
		else
			act("You don't even know which end is up on $p{x.",
			    ch, obj, NULL, TO_CHAR);

		return;
	}

	if (CAN_WEAR(obj, ITEM_HOLD)) {
		if (!remove_obj(ch, WEAR_HOLD, fReplace))
			return;

		if (get_eq_char(ch, WEAR_SECONDARY) != NULL && !IS_IMMORTAL(ch)) {
			stc("You cannot hold an item while using two weapons.\n", ch);
			return;
		}

		act("$n holds $p{x in $s hand.",   ch, obj, NULL, TO_ROOM);
		act("You hold $p{x in your hand.", ch, obj, NULL, TO_CHAR);
		equip_char(ch, obj, WEAR_HOLD);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_FLOAT)) {
		if (!remove_obj(ch, WEAR_FLOAT, fReplace))
			return;

		act("$n releases $p{x to float next to $m.", ch, obj, NULL, TO_ROOM);
		act("You release $p{x and it floats next to you.", ch, obj, NULL, TO_CHAR);
		equip_char(ch, obj, WEAR_FLOAT);
		return;
	}

	if (fReplace)
		stc("You can't wear, wield, or hold that.\n", ch);
} /* end wear_obj() */

void do_wear(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Wear, wield, or hold what?\n", ch);
		return;
	}

	if (!str_cmp(arg, "all")) {
		OBJ_DATA *obj_next;

		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;

			if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj))
				wear_obj(ch, obj, FALSE);
		}

		return;
	}
	else {
		if ((obj = get_obj_carry(ch, arg)) == NULL) {
			stc("You do not have that item.\n", ch);
			return;
		}

		wear_obj(ch, obj, TRUE);
	}

	return;
}

void do_remove(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Remove what?\n", ch);
		return;
	}

	/* Remove all...from miscellanous donation code */
	if (!strcmp(arg, "all")) {
		int x;

		for (x = 0; x < 20; x++)
			remove_obj(ch, x, TRUE);

		return;
	}

	if ((obj = get_obj_wear(ch, arg)) == NULL) {
		stc("You do not have that item.\n", ch);
		return;
	}

	remove_obj(ch, obj->wear_loc, TRUE);
	return;
}

/* Donate by Lotus */
void do_donate(CHAR_DATA *ch, const char *argument)
{
	char arg[MIL];
	OBJ_DATA *item;

	/* try to find the pit */
	if (donation_pit == NULL)
		for (donation_pit = object_list; donation_pit != NULL; donation_pit = donation_pit->next)
			if (donation_pit->pIndexData->vnum == OBJ_VNUM_PIT)
				break;

	/* can't find it?  make one */
	if (donation_pit == NULL) {
		donation_pit = create_object(get_obj_index(OBJ_VNUM_PIT), 0);

		if (! donation_pit) {
			bug("Error creating donation pit in do_donate.", 0);
			stc("There is no donation pit.\n", ch);
			return;
		}

		obj_to_room(donation_pit, get_room_index(ROOM_VNUM_ALTAR));
	}

	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Donate what?\n", ch);
		return;
	}

	if ((item = get_obj_carry(ch, arg)) == NULL) {
		stc("You do not have that item.\n", ch);
		return;
	}

	/* check for clan equipment */
	if (!clan_eq_ok(ch, item, "donate"))
		return;

	if (!can_drop_obj(ch, item)) {
		stc("You can't seem to let go of it.\n", ch);
		return;
	}

	act("$n donates $p to the donation pit.", ch, item, NULL, TO_ROOM);
	act("You donate $p to the donation pit.", ch, item, 0, TO_CHAR);
	obj_from_char(item);
	obj_to_obj(item, donation_pit);
}

/* Junk by Lotus */
void do_junk(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Junk what?\n", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		stc("You do not have that item.\n", ch);
		return;
	}

	if (!can_drop_obj(ch, obj)) {
		stc("You can't seem to let go of it.\n", ch);
		return;
	}

	act("$n junks $p.", ch, obj, NULL, TO_ROOM);
	act("You junk $p.", ch, obj, 0, TO_CHAR);
	extract_obj(obj);
	return;
}

bool acceptable_sac(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (!can_see_obj(ch, obj))
		return FALSE;

	if (obj->item_type == ITEM_CORPSE_PC) {
		if (obj->contains) {
			stc("The powers that be wouldn't like that.\n", ch);
			return FALSE;
		}
	}

	if (!CAN_WEAR(obj, ITEM_TAKE) || CAN_WEAR(obj, ITEM_NO_SAC) || IS_OBJ_STAT(obj, ITEM_NOSAC)) {
		ptc(ch, "%s is not an acceptable sacrifice.\n", capitalize(obj->short_descr));
		return FALSE;
	}

	return TRUE;
}

/* sacrifice all by Montrey */
void do_sacrifice(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj, *obj_next;
	bool found = FALSE;
	int silver = 0;
	CHAR_DATA *person;
	bool being_used;
	/* variables for AUTOSPLIT */
	CHAR_DATA *gch;
	int members;
	char buffer[100];
	one_argument(argument, arg);

	if (arg[0] == '\0' || !str_cmp(arg, ch->name)) {
		act("$n flings $mself at the feet of the Gods...sad.", ch, NULL, NULL, TO_ROOM);
		stc("Please refrain from bloodying the altar.\n", ch);
		return;
	}

	if (!str_cmp(arg, "all")) {
		for (obj = ch->in_room->contents; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;

			if (!acceptable_sac(ch, obj))
				continue;

			/* Make sure no one is resting on the item. -- Outsider */
			person = char_list;
			being_used = FALSE;

			while ((person) && (! being_used)) {
				if (person->on == obj)
					being_used = TRUE;
				else
					person = person->next;
			}

			if (!IS_NPC(ch) || !IS_IMMORTAL(ch))
				silver += URANGE(1, obj->cost, (obj->level * 3));

			act("$n sacrifices $p to the Gods.", ch, obj, NULL, TO_ROOM);
			extract_obj(obj);
			found = TRUE;
		}

		if (!found) {
			stc("There is nothing to sacrifice here.\n", ch);
			return;
		}

		if (IS_NPC(ch) || IS_IMMORTAL(ch)) {
			stc("You sacrifice everything in the room.\n", ch);
			return;
		}

		if (silver == 1) {
			sprintf(buf, "%s gives you one silver coin for your sacrifices.\n", ch->pcdata->deity);
			stc(buf, ch);
		}
		else {
			sprintf(buf, "%s gives you %d silver coins for your sacrifices.\n", ch->pcdata->deity, silver);
			stc(buf, ch);
		}

		wiznet("$N sends up a grateful sacrifice.", ch, NULL, WIZ_SACCING, 0, 0);
	}
	else {
		obj = get_obj_list(ch, arg, ch->in_room->contents);

		if (obj == NULL) {
			stc("You can't find it.\n", ch);
			return;
		}

		if (!acceptable_sac(ch, obj))
			return;

		/* Make sure no one is sleeping on/in the item. -- Outsider */
		person = char_list;

		while (person) {
			if (person->on == obj) {
				stc("Someone is using that.\n", ch);
				return;
			}

			person = person->next;
		}

		if (IS_NPC(ch) || IS_IMMORTAL(ch))
			stc("You make a loyal sacrifice.\n", ch);
		else {
			silver = URANGE(1, obj->cost, (obj->level * 3));

			if (silver == 1) {
				sprintf(buf, "%s gives you one silver coin for your sacrifice.\n", ch->pcdata->deity);
				stc(buf, ch);
			}
			else {
				sprintf(buf, "%s gives you %d silver coins for your sacrifice.\n", ch->pcdata->deity, silver);
				stc(buf, ch);
			}
		}

		act("$n sacrifices $p to the Gods.", ch, obj, NULL, TO_ROOM);
		wiznet("$N sends up $p as a grateful sacrifice.", ch, obj, WIZ_SACCING, 0, 0);
		extract_obj(obj);
	}

	ch->silver += silver;

	if (IS_SET(ch->act, PLR_AUTOSPLIT)) {
		/* AUTOSPLIT code */
		members = 0;

		for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
			if (is_same_group(gch, ch)
			    && !IS_NPC(gch))
				members++;

		if (members > 1 && silver > 1) {
			sprintf(buffer, "%d", silver);
			do_split(ch, buffer);
		}
	}
}

void do_quaff(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Quaff what?\n", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		if ((obj = get_eq_char(ch, WEAR_HOLD)) == NULL) {
			stc("You do not have that potion.\n", ch);
			return;
		}
	}

	if (obj->item_type != ITEM_POTION) {
		stc("You can quaff only potions.\n", ch);
		return;
	}

	if (ch->level < obj->level) {
		stc("This liquid is too powerful for you to drink.\n", ch);
		return;
	}

	if (!IS_NPC(ch)) {
		if (ch->pcdata->condition[COND_FULL] > 45) {
			stc("You are too full to quaff this.\n", ch);
			return;
		}

		gain_condition(ch, COND_FULL, 4);
	}

	act("$n quaffs $p.", ch, obj, NULL, TO_ROOM);
	act("You quaff $p.", ch, obj, NULL, TO_CHAR);
	obj_cast_spell(obj->value[1], obj->value[0], ch, ch, NULL);
	obj_cast_spell(obj->value[2], obj->value[0], ch, ch, NULL);
	obj_cast_spell(obj->value[3], obj->value[0], ch, ch, NULL);
	obj_cast_spell(obj->value[4], obj->value[0], ch, ch, NULL);
	extract_obj(obj);
}

/* Global for scrolls that want that extra oomph */
extern const char *target_name;

void do_recite(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *scroll, *obj = NULL;
	target_name = one_argument(argument, arg1);
	one_argument(target_name, arg2);

	if ((scroll = get_obj_carry(ch, arg1)) == NULL) {
		if ((scroll = get_eq_char(ch, WEAR_HOLD)) == NULL) {
			stc("You do not have that scroll.\n", ch);
			return;
		}
	}

	if (scroll->item_type != ITEM_SCROLL) {
		stc("You can recite only scrolls.\n", ch);
		return;
	}

	/* Avoid reciting blank scrolls, thus destroying them. -- Outsider */
	if (scroll->value[1] <= 0) {
		stc("The scroll is blank.\n", ch);
		return;
	}

	if (get_usable_level(ch) < scroll->level) {
		stc("This scroll is too complex for you to comprehend.\n", ch);
		return;
	}

	if (!deduct_stamina(ch, gsn_scrolls))
		return;

	if (arg2[0] == '\0') {
		/* Smart Targetting -- Montrey */
		/* target according to first spell on the scroll */
		if (skill_table[scroll->value[1]].target == TAR_CHAR_OFFENSIVE
		    || skill_table[scroll->value[1]].target == TAR_OBJ_CHAR_OFF) {
			if (ch->fighting != NULL)
				victim = ch->fighting;
			else {
				stc("Recite the scroll on whom or what?\n", ch);
				return;
			}
		}
		else
			victim = ch;
	}
	else {
		if ((victim = get_char_here(ch, arg2, VIS_CHAR)) == NULL
		    && (obj    = get_obj_here(ch, arg2)) == NULL) {
			stc("You can't find it.\n", ch);
			return;
		}
	}

	act("$n recites $p.", ch, scroll, NULL, TO_ROOM);
	act("You recite $p.", ch, scroll, NULL, TO_CHAR);

	if (number_percent() >= 20 + get_skill(ch, gsn_scrolls) * 4 / 5) {
		stc("You mispronounce a syllable.\n", ch);
		check_improve(ch, gsn_scrolls, FALSE, 2);
	}
	else {
		obj_cast_spell(scroll->value[1], scroll->value[0], ch, victim, obj);
		obj_cast_spell(scroll->value[2], scroll->value[0], ch, victim, obj);
		obj_cast_spell(scroll->value[3], scroll->value[0], ch, victim, obj);
		obj_cast_spell(scroll->value[4], scroll->value[0], ch, victim, obj);
		check_improve(ch, gsn_scrolls, TRUE, 2);
	}

	/* delay on scrolls -- Elrac */
	WAIT_STATE(ch, (ch->cls == 0 || ch->cls == 1 || ch->cls == 4)
	           ? (skill_table[gsn_scrolls].beats * 4) / 5
	           : skill_table[gsn_scrolls].beats);
	extract_obj(scroll);
}

void do_brandish(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	OBJ_DATA *staff;
	int sn;

	if ((staff = get_eq_char(ch, WEAR_HOLD)) == NULL) {
		stc("You hold nothing in your hand.\n", ch);
		return;
	}

	if (staff->item_type != ITEM_STAFF) {
		stc("You can brandish only with a staff.\n", ch);
		return;
	}

	if ((sn = staff->value[3]) < 0
	    ||   sn >= MAX_SKILL
	    ||   skill_table[sn].spell_fun == 0) {
		bug("Do_brandish: bad sn %d.", sn);
		return;
	}

	if (!deduct_stamina(ch, gsn_staves))
		return;

	WAIT_STATE(ch, skill_table[gsn_staves].beats);

	if (staff->value[2] > 0) {
		act("$n lifts up and brandishes $p.", ch, staff, NULL, TO_ROOM);
		act("You brandish $p.",  ch, staff, NULL, TO_CHAR);

		if (get_usable_level(ch) < staff->level
		    ||   number_percent() >= 20 + get_skill(ch, gsn_staves) * 4 / 5) {
			act("You fail to invoke $p.", ch, staff, NULL, TO_CHAR);
			act("...and nothing happens.", ch, NULL, NULL, TO_ROOM);
			check_improve(ch, gsn_staves, FALSE, 2);
		}
		else for (vch = ch->in_room->people; vch; vch = vch_next) {
				vch_next    = vch->next_in_room;

				switch (skill_table[sn].target) {
				default:
					bug("Do_brandish: bad target for sn %d.", sn);
					return;

				case TAR_IGNORE:
					if (vch != ch)
						continue;

					break;

				case TAR_CHAR_OFFENSIVE:
					if (IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch))
						continue;

					break;

				case TAR_CHAR_DEFENSIVE:
					if (IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch))
						continue;

					break;

				case TAR_CHAR_SELF:
					if (vch != ch)
						continue;

					break;
				}

				obj_cast_spell(staff->value[3], staff->value[0], ch, vch, NULL);
				check_improve(ch, gsn_staves, TRUE, 2);
			}
	}

	if (--staff->value[2] <= 0) {
		act("$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM);
		act("Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR);
		extract_obj(staff);
	}

	return;
}

void do_zap(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *wand;
	OBJ_DATA *obj;
	one_argument(argument, arg);

	if (arg[0] == '\0' && ch->fighting == NULL) {
		stc("Zap whom or what?\n", ch);
		return;
	}

	if ((wand = get_eq_char(ch, WEAR_HOLD)) == NULL) {
		stc("You are holding nothing in your hand.\n", ch);
		return;
	}

	if (wand->item_type != ITEM_WAND) {
		stc("You can zap only with a wand.\n", ch);
		return;
	}

	obj = NULL;

	if (arg[0] == '\0') {   /* no target given */
		/* check for default target */
		if (ch->fighting != NULL)
			victim = ch->fighting;
		else {
			stc("Zap whom or what?\n", ch);
			return;
		}
	}
	else {  /* target given, see if it is valid */
		if ((victim = get_char_here(ch, arg, VIS_CHAR)) == NULL
		    && (obj    = get_obj_here(ch, arg)) == NULL
		    && (wand->value[3] != gsn_summon || (victim = get_char_world(ch, arg, VIS_CHAR)) == NULL)) {
			stc("You can't find it.\n", ch);
			return;
		}
	}

	/* added this cause it'd prolly crash if someone tried :P -- Montrey */
	if (victim == NULL && obj == wand) {
		stc("You can't zap a wand with itself.\n", ch);
		return;
	}

	if (!deduct_stamina(ch, gsn_wands))
		return;

	WAIT_STATE(ch, skill_table[gsn_wands].beats);

	if (wand->value[2] > 0) {
		if (victim != NULL) {
			if (victim == ch) {
				act("$n zaps $mself with $p.", ch, wand, victim, TO_ROOM);
				act("You zap yourself with $p.", ch, wand, victim, TO_CHAR);
			}
			else {
				act("$n zaps $N with $p.", ch, wand, victim, TO_ROOM);
				act("You zap $N with $p.", ch, wand, victim, TO_CHAR);
			}
		}
		else { /* victim is NULL, must be an object */
			act("$n zaps $P with $p.", ch, wand, obj, TO_ROOM);
			act("You zap $P with $p.", ch, wand, obj, TO_CHAR);
		}

		if (get_usable_level(ch) < wand->level
		    ||  number_percent() >= 20 + get_skill(ch, gsn_wands) * 4 / 5) {
			act("Your efforts with $p produce only sparks and smoke.",
			    ch, wand, NULL, TO_CHAR);
			act("$n's efforts with $p produce only sparks and smoke.",
			    ch, wand, NULL, TO_ROOM);
			check_improve(ch, gsn_wands, FALSE, 2);
		}
		else {
			/* Wand does not work on target, unless we set target_name.
			   target_name is a global variable in magic.c --Outsider */
			target_name = arg;
			obj_cast_spell(wand->value[3], wand->value[0], ch, victim, obj);
			check_improve(ch, gsn_wands, TRUE, 2);
		}
	}

	if (--wand->value[2] <= 0) {
		act("$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM);
		act("Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR);
		extract_obj(wand);
	}

	return;
}

void do_brew(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int sn;
	int target_level = 0;    /* what level should we brew at? */

	if (!IS_NPC(ch)
	    && ch->level < skill_table[gsn_brew].skill_level[ch->cls]) {
		stc("You do not know how to brew potions.\n", ch);
		return;
	}

	if (IS_NPC(ch)) {
		stc("Mobiles can't brew!.\n", ch);
		return;
	}

	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Brew what spell?\n", ch);
		return;
	}

	/* Do we have a vial to brew potions? */
	for (obj = ch->carrying; obj; obj = obj->next_content) {
		if (obj->item_type == ITEM_POTION && obj->wear_loc == WEAR_HOLD)
			break;
	}

	/* Interesting ... Most scrolls/potions in the mud have no hold
	   flag; so, the problem with players running around making scrolls
	   with 3 heals or 3 gas breath from pre-existing scrolls has been
	   severely reduced. Still, I like the idea of 80% success rate for
	   first spell imprint, 25% for 2nd, and 10% for 3rd. I don't like the
	   idea of a scroll with 3 ultrablast spells; although, I have limited
	   its applicability when I reduced the spell->level to 1/3 and 1/4
	   of ch->level for scrolls and potions respectively. --- JH */

	/* I will just then make two items, an empty vial and a parchment available
	   in midgaard shops with holdable flags and -1 for each of the 3 spell
	   slots. Need to update the midgaard.are files --- JH */

	if (!obj) {
		stc("You are not holding a vial.\n", ch);
		return;
	}

	if ((sn = skill_lookup(arg))  < 0) {
		stc("There is no such spell.\n", ch);
		return;
	}

	if (!get_skill(ch, sn)) {
		stc("You don't know any spells by that name.\n", ch);
		return;
	}

	/* preventing potions of gas breath, acid blast, etc.; doesn't make sense
	   when you quaff a gas breath potion, and then the mobs in the room are
	   hurt. Those TAR_IGNORE spells are a mixed blessing. - JH */

	if (((skill_table[sn].target != TAR_CHAR_DEFENSIVE) &&
	     (skill_table[sn].target != TAR_CHAR_SELF) &&
	     (skill_table[sn].target != TAR_OBJ_CHAR_DEF))
	    ||
	    (skill_table[sn].remort_class > 0)) {
		stc("You cannot brew that spell.\n", ch);
		return;
	}

	/* We shall let casters brew spells at a lower level, provided
	   the level is high enough for their class to cast that spell.
	   Example: Fireball cannot be brewed as a 1st level spell.
	   Re-using "arg" here. We seem to be done with it. -- Outsider
	*/
	arg[0] = '\0';    /* just making sure */
	argument = one_argument(argument, arg);

	if (arg[0] != '\0') {  /* we got a new level set */
		target_level = atoi(arg);

		/* make sure the new level is high enough */
		if (target_level < skill_table[sn].skill_level[ch->cls]) {
			stc("You cannot brew the spell at that low a level.\n", ch);
			return;
		}

		/* Keep level from being too high. */
		if (target_level > ch->level) {
			stc("You have not yet reached that level!\n", ch);
			return;
		}
	}

	if (!deduct_stamina(ch, gsn_brew))
		return;

	act("$n begins preparing a potion.", ch, obj, NULL, TO_ROOM);
	check_improve(ch, gsn_brew, TRUE, 2);
	WAIT_STATE(ch, skill_table[gsn_brew].beats);

	/* Check the skill percentage, fcn(wis,int,skill) */
	if (!IS_NPC(ch)
	    && (number_percent() > ch->pcdata->learned[gsn_brew] ||
	        number_percent() > ((GET_ATTR_INT(ch) - 13) * 5 +
	                            (GET_ATTR_WIS(ch) - 13) * 3))) {
		act("$p explodes violently!", ch, obj, NULL, TO_CHAR);
		act("$p explodes violently!", ch, obj, NULL, TO_ROOM);
		check_improve(ch, gsn_brew, FALSE, 2);
		extract_obj(obj);
		return;
	}

	/* took this outside of imprint codes, so I can make do_brew differs from
	   do_scribe; basically, setting potion level and spell level --- JH */

	if (target_level) {
		obj->level = (sh_int) target_level;
		obj->value[0] = (sh_int) target_level;
	}
	else {
		obj->level = ch->level;
		obj->value[0] = ch->level;
	}

	spell_imprint(sn, ch->level, ch, obj);
}

void do_scribe(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int sn;
	int target_level = 0;   /* let caster make items of lower level */

	if (!IS_NPC(ch)
	    && ch->level < skill_table[gsn_scribe].skill_level[ch->cls]) {
		stc("You do not know how to scribe scrolls.\n", ch);
		return;
	}

	if (IS_NPC(ch)) {
		stc("Mobiles can't scribe!.\n", ch);
		return;
	}

	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Scribe what spell?\n", ch);
		return;
	}

	/* Do we have a parchment to scribe spells? */
	for (obj = ch->carrying; obj; obj = obj->next_content) {
		if (obj->item_type == ITEM_SCROLL && obj->wear_loc == WEAR_HOLD)
			break;
	}

	if (!obj) {
		stc("You are not holding a parchment.\n", ch);
		return;
	}

	if ((sn = skill_lookup(arg))  < 0) {
		stc("There is no such spell.\n", ch);
		return;
	}

	if (!get_skill(ch, sn)) {
		stc("You don't know any spells by that name.\n", ch);
		return;
	}

	if (skill_table[sn].remort_class > 0) {
		stc("You cannot scribe that spell.\n", ch);
		return;
	}

	/* Check to see if caster is trying to make the scroll at a lower
	   level. Do not let the caster make scrolls of a lower level than
	   normally required to cast the spell. Example: No 1st level fireballs.
	   Re-using "Arg" here. We seem to be done with it.
	   -- Outsider
	*/
	arg[0] = '\0';    /* make sure it is reset */
	argument = one_argument(argument, arg);

	if (arg[0] != '\0') {  /* we got some data */
		target_level = atoi(arg);

		/* Keep the spell level from dropping too low. */
		if (target_level < skill_table[sn].skill_level[ch->cls]) {
			stc("You cannot scribe that spell at that level.", ch);
			return;
		}

		/* Keep the spell level from going too high. */
		if (target_level > ch->level) {
			stc("You are not yet at that level!\n", ch);
			return;
		}
	}

	if (!deduct_stamina(ch, gsn_scribe))
		return;

	act("$n begins writing a scroll.", ch, obj, NULL, TO_ROOM);
	check_improve(ch, gsn_scribe, TRUE, 2);
	WAIT_STATE(ch, skill_table[gsn_scribe].beats);

	/* Check the skill percentage, fcn(int,wis,skill) */
	if (!IS_NPC(ch)
	    && (number_percent() > ch->pcdata->learned[gsn_scribe] ||
	        number_percent() > ((GET_ATTR_INT(ch) - 13) * 5 +
	                            (GET_ATTR_WIS(ch) - 13) * 3))) {
		act("$p bursts in flames!", ch, obj, NULL, TO_CHAR);
		act("$p bursts in flames!", ch, obj, NULL, TO_ROOM);
		check_improve(ch, gsn_scribe, FALSE, 2);
		extract_obj(obj);
		return;
	}

	/* basically, making scrolls more potent than potions; also, scrolls
	   are not limited in the choice of spells, i.e. scroll of enchant weapon
	   has no analogs in potion forms --- JH */

	if (target_level) {
		obj->level = (sh_int) target_level;
		obj->value[0] = (sh_int) target_level;
	}
	else {
		obj->level = ch->level;
		obj->value[0] = ch->level;
	}

	spell_imprint(sn, ch->level, ch, obj);
}

void do_steal(CHAR_DATA *ch, const char *argument)
{
	char buf  [MAX_STRING_LENGTH];
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int percent;

	if (get_skill(ch, gsn_steal) == 0) {
		stc("You don't know how to steal.\n", ch);
		return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		stc("Steal what from whom?\n", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg2, VIS_CHAR)) == NULL) {
		stc("Can't steal from people who are not here.\n", ch);
		return;
	}

	if (victim == ch) {
		stc("A little schizophrenia going on here?\n", ch);
		return;
	}

	if (victim->in_room->sector_type == SECT_ARENA
	    || victim->in_room->sector_type == SECT_CLANARENA
	    || (victim->in_room->area == quest_area && !quest_upk)
	    || char_in_duel_room(victim)) {
		stc("You are here to do battle, not to steal!\n", ch);
		return;
	}

	if (IS_NPC(ch) && IS_SET(ch->act, ACT_MORPH) && !IS_NPC(victim)) {
		stc("Morphed players cannot attack PC's.\n", ch);
		return;
	}

	if (!IS_NPC(victim) && IS_IMMORTAL(victim)) {
		stc("I'm sorry, but the gods wouldn't like that!\n", ch);
		return;
	}

	if (is_safe(ch, victim, TRUE))
		return;

	if (!deduct_stamina(ch, gsn_steal))
		return;

	if (IS_NPC(victim) && victim->fighting) {
		stc("I don't think that is a good idea.\n", ch);
		return;
	}

	WAIT_STATE(ch, skill_table[gsn_steal].beats);
	percent  = number_percent();

	if (get_skill(ch, gsn_steal) >= 1)
		percent  += (IS_AWAKE(victim) ? 10 : -50);

	if ((!IS_NPC(victim) && !IS_NPC(ch) && !IS_IMMORTAL(ch))
	    && ((ch->level + 8 < victim->level || ch->level - 8 > victim->level)
	        || (!is_clan(victim) || !is_clan(ch)))) {
		stc("You cannot steal from them.\n", ch);
		return;
	}

	if (!IS_NPC(victim) && !IS_IMMORTAL(ch)) {
		if (!IS_SET(victim->pcdata->plr, PLR_PK)) {
			stc("They are not in the mood to PSteal.\n", ch);
			return;
		}

		if (!IS_SET(ch->pcdata->plr, PLR_PK)) {
			stc("You are not in the mood to PSteal.\n", ch);
			return;
		}
	}

	if (!IS_NPC(ch) && percent > get_skill(ch, gsn_steal)) {
		/*
		 * Failure.
		 */
		stc("Uh-oh!  WhOops.\n", ch);
		act("$n tried to steal from you.\n", ch, NULL, victim, TO_VICT);
		act("$n bungles a robbery attempt on $N.\n",  ch, NULL, victim, TO_NOTVICT);

		switch (number_range(0, 3)) {
		case 0 :
			sprintf(buf, "%s is a bum mugger!", ch->name);
			break;

		case 1 :
			sprintf(buf, "%s couldn't rob %s way out of a paper bag!",
			        ch->name, GET_ATTR_SEX(ch) == SEX_NEUTRAL ? "its" : GET_ATTR_SEX(ch) == SEX_MALE ? "his" : "her");
			break;

		case 2 :
			sprintf(buf, "%s tried to rob me!", ch->name);
			break;

		case 3 :
			sprintf(buf, "Keep your hands out of there, %s! Pervert!", ch->name);
			break;
		}

		do_yell(victim, buf);

		if (!IS_NPC(ch)) {
			if (IS_NPC(victim)) {
				check_improve(ch, gsn_steal, FALSE, 2);
				multi_hit(victim, ch, TYPE_UNDEFINED);
			}
			else {
				sprintf(buf, "$N tried to steal from %s.", victim->name);
				wiznet(buf, ch, NULL, WIZ_FLAGS, 0, 0);

				if (!IS_SET(ch->act, PLR_THIEF)) {
					SET_BIT(ch->act, PLR_THIEF);
					SET_BIT(ch->pcdata->plr, PLR_NOPK);
					set_color(ch, BLUE, NOBOLD);
					stc("*** You are now a THIEF!! ***\n", ch);
					set_color(ch, WHITE, NOBOLD);
					ch->pcdata->flag_thief = MAX_THIEF;
					/* Stable, Lotus :save_char_obj( ch ); */
				}
			}
		}

		return;
	}

	if (!str_cmp(arg1, "coin")
	    ||   !str_cmp(arg1, "coins")
	    ||   !str_cmp(arg1, "gold")
	    ||   !str_cmp(arg1, "silver")) {
		int gold, silver;
		gold = victim->gold * number_range(1, ch->level) / MAX_LEVEL;
		silver = victim->silver * number_range(1, ch->level) / MAX_LEVEL;

		if (gold <= 0 && silver <= 0) {
			stc("You didn't get anything. Try someone richer.\n", ch);
			return;
		}

		ch->gold        += gold;
		ch->silver      += silver;
		victim->silver  -= silver;
		victim->gold    -= gold;

		if (silver <= 0)
			sprintf(buf, "Bingo!  You got %d gold coin%s.\n", gold,
			        gold == 1 ? "" : "s");
		else if (gold <= 0)
			sprintf(buf, "Bingo!  You got %d silver coin%s.\n", silver,
			        silver == 1 ? "" : "s");
		else
			sprintf(buf, "Bingo!  You got %d silver and %d gold coins.\n",
			        silver, gold);

		stc(buf, ch);
		check_improve(ch, gsn_steal, TRUE, 2);
		return;
	}

	for (obj = victim->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->wear_loc == WEAR_NONE
		    && can_see_obj(ch, obj)
		    && is_name(arg1, obj->name))
			break;
	}

	if (obj == NULL) {
		stc("You feel around, but dont get anything but lint.\n", ch);
		return;
	}

	if (!can_drop_obj(ch, obj)
	    ||   IS_OBJ_STAT(obj, ITEM_INVENTORY)
	    ||   obj->level > ch->level) {
		stc("You can't pry it away.\n", ch);
		return;
	}

	if (get_carry_number(ch) + get_obj_number(obj) > can_carry_n(ch)) {
		stc("You have your hands full.\n", ch);
		return;
	}

	if (get_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch)) {
		stc("You can't carry that much weight.\n", ch);
		return;
	}

	obj_from_char(obj);
	obj_to_char(obj, ch);
	check_improve(ch, gsn_steal, TRUE, 2);
	stc("Got it!\n", ch);

	/* Did they pick up their quest item? */
	if (IS_SET(ch->act, PLR_QUESTOR)) {
		if (ch->questobj == obj->pIndexData->vnum && ch->questobf != -1) {
			char buf[MAX_STRING_LENGTH];
			stc("{YYou have almost completed your QUEST!{x\n", ch);
			stc("{YReturn to the questmaster before your time runs out!{x\n", ch);
			ch->questobf = -1;
			sprintf(buf, "{Y:QUEST: {x$N has found %s", obj->short_descr);
			wiznet(buf, ch, NULL, WIZ_QUEST, 0, 0);
		}
	}

	/* or skill quest item? */
	if (!IS_NPC(ch) && IS_SET(ch->pcdata->plr, PLR_SQUESTOR)) {
		if (ch->pcdata->squestobj == obj && !ch->pcdata->squestobjf) {
			char buf[MAX_STRING_LENGTH];

			if (ch->pcdata->squestmob == NULL) {
				stc("{YYou have almost completed your {VSKILL QUEST!{x\n", ch);
				stc("{YReturn to the questmistress before your time runs out!{x\n", ch);
			}
			else {
				stc("{YYou have completed part of your {VSKILL QUEST!{x\n", ch);
				ptc(ch, "{YTake the artifact to %s while there is still time!{x\n",
				    ch->pcdata->squestmob->short_descr);
			}

			ch->pcdata->squestobjf = TRUE;
			sprintf(buf, "{Y:SKILL QUEST: {x$N has found the %s", obj->short_descr);
			wiznet(buf, ch, NULL, WIZ_QUEST, 0, 0);
		}
	}
}

/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper(CHAR_DATA *ch)
{
	/*char buf[MAX_STRING_LENGTH];*/
	CHAR_DATA *keeper;
	SHOP_DATA *pShop;
	pShop = NULL;

	for (keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room) {
		if (IS_SET(keeper->act, ACT_MORPH))
			continue;

		if (IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL)
			break;
	}

	if (pShop == NULL) {
		stc("This isn't a store...you might have noticed.\n", ch);
		return NULL;
	}

	/*
	 * Undesirables.
	 * REWORK PK, lets let them shop, Lotus
	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_KILLER) )
	{
	        do_say( keeper, "Killers are not welcome!" );
	        sprintf( buf, "%s the psycho KILLER is over here!\n", ch->name );
	        do_yell( keeper, buf );
	        return NULL;
	}

	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_THIEF) )
	{
	        do_say( keeper, "Thieves are not welcome!" );
	        sprintf( buf, "%s the gutless THIEF is over here!\n", ch->name );
	        do_yell( keeper, buf );
	        return NULL;
	}
	        */
	/*
	 * Shop hours.
	 */
	if (time_info.hour < pShop->open_hour) {
		do_say(keeper, "Sorry, I am closed.");
		return NULL;
	}

	if (time_info.hour > pShop->close_hour) {
		do_say(keeper, "Sorry, I am closed.");
		return NULL;
	}

	/*
	 * Invisible or hidden people.
	 */
	if (!can_see_char(keeper, ch)) {
		do_say(keeper, "Umm, Sorry, If I could see you, we could trade!");
		return NULL;
	}

	return keeper;
}

/* insert an object at the right spot for the keeper */
void obj_to_keeper(OBJ_DATA *obj, CHAR_DATA *ch)
{
	OBJ_DATA *t_obj, *t_obj_next;

	/* see if any duplicates are found */
	for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next) {
		t_obj_next = t_obj->next_content;

		if (obj->pIndexData == t_obj->pIndexData
		    && !str_cmp(obj->short_descr, t_obj->short_descr)) {
			/* if this is an unlimited item, destroy the new one */
			if (IS_OBJ_STAT(t_obj, ITEM_INVENTORY)) {
				extract_obj(obj);
				return;
			}

			obj->cost = t_obj->cost; /* keep it standard */
			break;
		}
	}

	if (t_obj == NULL) {
		obj->next_content = ch->carrying;
		ch->carrying = obj;
	}
	else {
		obj->next_content = t_obj->next_content;
		t_obj->next_content = obj;
	}

	obj->carried_by      = ch;
	obj->in_room         = NULL;
	obj->in_obj          = NULL;
}

/* get an object from a shopkeeper's list */
OBJ_DATA *get_obj_keeper(CHAR_DATA *ch, CHAR_DATA *keeper, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int number;
	int count;
	number = number_argument(argument, arg);
	count  = 0;

	for (obj = keeper->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->wear_loc == WEAR_NONE
		    &&  can_see_obj(keeper, obj)
		    &&  can_see_obj(ch, obj)
		    &&  is_name(arg, obj->name)) {
			if (++count == number)
				return obj;

			/* skip other objects of the same name */
			while (obj->next_content != NULL
			       && obj->pIndexData == obj->next_content->pIndexData
			       && !str_cmp(obj->short_descr, obj->next_content->short_descr))
				obj = obj->next_content;
		}
	}

	return NULL;
}

int get_cost(CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy)
{
	SHOP_DATA *pShop;
	int cost;

	if (obj == NULL || (pShop = keeper->pIndexData->pShop) == NULL)
		return 0;

	if (fBuy)
		cost = obj->cost * pShop->profit_buy  / 100;
	else if (IS_OBJ_STAT(obj, ITEM_QUESTSELL)) {
		if (pShop->buy_type[0] == ITEM_QUESTSHOP) {
			cost = obj->cost * pShop->profit_sell / 100;
			cost = (cost * obj->condition) / 100;
		}
		else
			cost = 0;
	}
	else {
		OBJ_DATA *obj2;
		int itype;
		cost = 0;

		for (itype = 0; itype < MAX_TRADE; itype++) {
			if (obj->item_type == pShop->buy_type[itype]) {
				cost = obj->cost * pShop->profit_sell / 100;
				break;
			}
		}

		if (!IS_OBJ_STAT(obj, ITEM_SELL_EXTRACT))
			for (obj2 = keeper->carrying; obj2; obj2 = obj2->next_content) {
				if (obj->pIndexData == obj2->pIndexData
				    &&   !str_cmp(obj->short_descr, obj2->short_descr)) {
					if (IS_OBJ_STAT(obj2, ITEM_INVENTORY))
						cost /= 2;
					else
						cost = cost * 3 / 4;
				}
			}
	}

	if (obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND) {
		if (obj->value[1] == 0)
			cost /= 4;
		else
			cost = cost * obj->value[2] / obj->value[1];
	}

	return cost;
} /* end get_cost() */

void make_pet(CHAR_DATA *ch, CHAR_DATA *pet) {
	SET_BIT(pet->act, ACT_PET);
	affect_add_perm_to_char(pet, gsn_charm_person);
	pet->comm = COMM_NOCHANNELS;
	add_follower(pet, ch);
	pet->leader = ch;
	ch->pet = pet;
}

void do_buy(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	int cost, roll;
	// char *x; /* for WIZ_CHEAT */
	int level_buyable;
	int cash_on_char, money_in_bank; /* these together is all our money */
	int need_bank_credit = FALSE;    /* whether we need money from the bank */
	int total_cost;              /* cost of multiple items */
	/* figure out how much moola we have -- Outsider */
	cash_on_char = ch->silver + (ch->gold * 100);
	money_in_bank = ch->silver_in_bank + (ch->gold_in_bank * 100);

	if (argument[0] == '\0') {
		stc("What do you want to buy?\n", ch);
		return;
	}

	if (IS_SET(GET_ROOM_FLAGS(ch->in_room), ROOM_PET_SHOP)) {
		/* PETS */
		char arg[MAX_INPUT_LENGTH];
		char buf[MAX_STRING_LENGTH];
		CHAR_DATA *pet;
		ROOM_INDEX_DATA *pRoomIndexNext;
		ROOM_INDEX_DATA *in_room;

		if (IS_NPC(ch))
			return;

		argument = one_argument(argument, arg);

		/* hack to make new thalos pets work */
		if (ch->in_room->vnum == 9621)
			pRoomIndexNext = get_room_index(9706);
		else
			pRoomIndexNext = get_room_index(ch->in_room->vnum + 1);

		if (pRoomIndexNext == NULL) {
			bug("Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum);
			stc("Sorry, we don't sell those.  If you'd like to see my manager...\n", ch);
			return;
		}

		in_room     = ch->in_room;
		ch->in_room = pRoomIndexNext;
		pet         = get_char_here(ch, arg, VIS_CHAR);
		ch->in_room = in_room;

		if (ch->in_room->guild && ch->in_room->guild != ch->cls + 1 && !IS_IMMORTAL(ch)) {
			stc("Sorry, members only.\n", ch);
			return;
		}

		if (pet == NULL || !IS_SET(pet->act, ACT_PET)) {
			stc("Sorry, we don't sell those.  If you'd like to see my manager...\n", ch);
			return;
		}

		if (ch->pet != NULL) {
			stc("You've got a pet already!\n", ch);
			return;
		}

		if (ch->pcdata->familiar) {
			stc("You are still mourning the loss of your familiar.\n", ch);
			return;
		}

		cost = 10 * pet->level * pet->level;

		/* I'm going to make this more interesting -- Outsider
		if ( (ch->silver + (100 * ch->gold)) < cost )
		{
		        stc( "You are too poor to buy that.\n", ch );
		        return;
		}
		*/

		/* give us a chance to use bank credit */
		if (cash_on_char < cost) {
			need_bank_credit = TRUE;
			/* when using the bank, add 10% fee */
			cost += cost / 10;

			if ((cash_on_char + money_in_bank) < cost) {
				stc("You are too poor to buy that.\n", ch);
				return;
			}
		}

		if (ch->level < pet->level) {
			stc("This thing would eat you for breakfast!\n", ch);
			return;
		}

		/* haggle */
		roll = number_percent();

		if (number_percent() < (GET_ATTR_CHR(ch) * 3))
			roll -= 15;

		if (roll < 1) roll = 1;

		if (roll < get_skill(ch, gsn_haggle)) {
			cost -= cost / 2 * roll / 100;
			ptc(ch, "You haggle the price down to %d coins.\n", cost);
			check_improve(ch, gsn_haggle, TRUE, 4);
		}

		/* need to replace this now that we can use bank credit -- Outsider
		deduct_cost(ch,cost);
		*/
		if (need_bank_credit) {
			cost -= cash_on_char;
			ch->silver = ch->gold = 0;  /* we're penniless now */

			if (ch->silver_in_bank >= cost) /* we have enough silver to cover it */
				ch->silver_in_bank -= cost;
			else { /* use all the silver and take gold */
				cost -= ch->silver_in_bank;
				ch->silver_in_bank = 0;
				ch->gold_in_bank -= (cost / 100) + 1;
				ch->silver_in_bank = 100 - (cost % 100);  /* get change back */
			}
		}
		else /* we have enough cash */
			deduct_cost(ch, cost);

		pet = create_mobile(pet->pIndexData);

		/* Check for memory error. -- Outsider */
		if (! pet) {
			bug("Memory error from create_mobile() in do_buy().", 0);
			stc("You were unable to take your pet! You get a refund.\n", ch);
			ch->silver += cost;
		}

		argument = one_argument(argument, arg);

		if (arg[0] != '\0') {
			if (strchr(arg, '~'))
				wiznet("$N is attempting to use the tilde in pet name cheat.",
				       ch, NULL, WIZ_CHEAT, 0, GET_RANK(ch));

			sprintf(buf, "%s %s", pet->name, smash_tilde(arg));
			pet->name = buf;
		}

		sprintf(buf, "%sA collar around its neck says 'I belong to %s'.\n",
		        pet->description, ch->name);
		free_string(pet->description);
		pet->description = str_dup(buf);
		char_to_room(pet, ch->in_room);

		make_pet(ch, pet);

		stc("Enjoy your pet.  Watch out, they bite!\n", ch);
		act("$n purchased $N as a pet.", ch, NULL, pet, TO_ROOM);
		return;
	}
	else {
		/* non-pet shop */
		CHAR_DATA *keeper;
		OBJ_DATA *obj, *t_obj;
		char arg[MAX_INPUT_LENGTH];
		int number, count = 1;

		if ((keeper = find_keeper(ch)) == NULL)
			return;

		if (keeper->in_room->guild && keeper->in_room->guild != ch->cls + 1 && !IS_IMMORTAL(ch)) {
			act("$n tells you 'Sorry, members only.'", keeper, NULL, ch, TO_VICT);
			return;
		}

		number = mult_argument(argument, arg);
		obj  = get_obj_keeper(ch, keeper, arg);
		cost = get_cost(keeper, obj, TRUE);

		if (cost <= 0 || !can_see_obj(ch, obj)) {
			act("$n tells you 'I don't sell that -- try 'list''.",
			    keeper, NULL, ch, TO_VICT);
			strcpy(ch->reply, keeper->name);
			return;
		}

		/* Negative buy fix and large buy fix */
		if (number <= 0 || number >= 100) {
			stc("Sorry, I don't have that many.\n", ch);
			return;
		}

		if (!IS_OBJ_STAT(obj, ITEM_INVENTORY)) {
			for (t_obj = obj->next_content;
			     count < number && t_obj != NULL;
			     t_obj = t_obj->next_content) {
				if (t_obj->pIndexData == obj->pIndexData
				    &&  !str_cmp(t_obj->short_descr, obj->short_descr))
					count++;
				else
					break;
			}

			if (count < number) {
				act("$n tells you 'I don't have that many in stock.",
				    keeper, NULL, ch, TO_VICT);
				strcpy(ch->reply, keeper->name);
				return;
			}
		}

		if (IS_QUESTSHOPKEEPER(keeper)) {
			if (ch->questpoints < cost * number) {
				act("$n tell you 'Sorry, but you don't have enough quest points!'",
				    keeper, NULL, ch, TO_VICT);
				strcpy(ch->reply, keeper->name);
				return;
			}
		}
		else if (cash_on_char < (cost * number)) {
			/* give us the chance to use bank credit -- Outsider */
			need_bank_credit = TRUE;

			/* since we use the bank, add service charge of 10% */
			if ((cash_on_char + money_in_bank) < ((cost * number) + (cost * number / 10))) {
				if (number > 1)
					act("$n tells you 'You can't afford to buy that many'.",
					    keeper, obj, ch, TO_VICT);
				else
					act("$n tells you 'You can't afford to buy $p'.",
					    keeper, obj, ch, TO_VICT);

				strcpy(ch->reply, keeper->name);
				return;
			}
		}

		if (obj->pIndexData->item_type == ITEM_PILL || obj->pIndexData->item_type == ITEM_POTION)
			level_buyable = ch->level;
		else
			level_buyable = get_usable_level(ch);

		if (obj->level > level_buyable) {
			act("$n tells you 'You can't use $p yet'.",
			    keeper, obj, ch, TO_VICT);
			strcpy(ch->reply, keeper->name);
			return;
		}

		if (get_carry_number(ch) +  number * get_obj_number(obj) > can_carry_n(ch)) {
			stc("You can't carry that many items.\n", ch);
			return;
		}

		if (get_carry_weight(ch) + number * get_obj_weight(obj) > can_carry_w(ch)) {
			stc("You can't carry that much weight.\n", ch);
			return;
		}

		/* haggle and sell for silver - unless it's a quest shop keeper */
		if (!IS_QUESTSHOPKEEPER(keeper)) {
			roll = number_percent();

			if (number_percent() < (GET_ATTR_CHR(ch) * 3))
				roll -= 15;

			if (roll < 1) roll = 1;

			if (!IS_OBJ_STAT(obj, ITEM_SELL_EXTRACT)
			    && roll < get_skill(ch, gsn_haggle)
			    && obj->pIndexData->item_type != ITEM_MONEY) { /* to prevent buying money for less than value */
				cost -= obj->cost / 2 * roll / 100;
				act("You haggle with $N.", ch, NULL, keeper, TO_CHAR);
				check_improve(ch, gsn_haggle, TRUE, 4);
			}

			if (number > 1) {
				sprintf(buf, "$n buys $p[%d].", number);
				act(buf, ch, obj, NULL, TO_ROOM);
				sprintf(buf, "You buy $p[%d] for %d silver.", number, cost * number);
				act(buf, ch, obj, NULL, TO_CHAR);
			}
			else {
				act("$n buys $p.", ch, obj, NULL, TO_ROOM);
				sprintf(buf, "You buy $p for %d silver.", cost);
				act(buf, ch, obj, NULL, TO_CHAR);
			}

			/* replace this with bank + cash code -- Outsider
			deduct_cost(ch,cost * number);
			*/
			total_cost = cost * number;

			if (need_bank_credit) {
				total_cost += total_cost / 10;   /* 10% bank fee */
				/* use all of the char's cash */
				total_cost -= cash_on_char;
				ch->silver = ch->gold = 0;

				/* try using gold from bank first */
				if (ch->silver_in_bank >= total_cost) /* we have enough silver to cover it */
					ch->silver_in_bank -= total_cost;
				else {  /* we need gold too, and all our silver */
					total_cost -= ch->silver_in_bank;
					ch->silver_in_bank = 0;
					ch->gold_in_bank -= (cost / 100) + 1;
					ch->silver_in_bank = 100 - (cost % 100);  /* get change back */
				}
			}
			else    /* we have the cash on us */
				deduct_cost(ch, total_cost);

			keeper->gold += cost * number / 100;
			keeper->silver += cost * number - (cost * number / 100) * 100;
		}
		else {
			/* quest shop keeper. sale is not for money. */
			if (number > 1) {
				sprintf(buf, "$n buys $p[%d].", number);
				act(buf, ch, obj, NULL, TO_ROOM);
				sprintf(buf, "You buy $p[%d] for %d Quest Points.",
				        number, cost * number);
				act(buf, ch, obj, NULL, TO_CHAR);
			}
			else {
				act("$n buys $p.", ch, obj, NULL, TO_ROOM);
				sprintf(buf, "You buy $p for %d Quest Points.", cost);
				act(buf, ch, obj, NULL, TO_CHAR);
			}

			ch->questpoints -= cost;
		}

		mprog_buy_trigger(keeper, ch);

		for (count = 0; count < number; count++) {
			if (IS_OBJ_STAT(obj, ITEM_INVENTORY)) {
				t_obj = create_object(obj->pIndexData, obj->level);

				if (! t_obj) {
					bug("Memory error creating object in do_buy.", 0);
					return;
				}
			}
			else {
				t_obj = obj;
				obj = obj->next_content;
				obj_from_char(t_obj);
			}

			/* owner items bought in guild rooms to purchaser -- Montrey */
			if (keeper->in_room->guild) {
				char owner[MSL];
				EXTRA_DESCR_DATA *ed;
				bool foundold = FALSE;

				/* loop through and find previous owner, if any, and change */
				if (t_obj->extra_descr != NULL) {
					for (ed = t_obj->extra_descr; ed != NULL; ed = ed->next)
						if (!str_cmp(ed->keyword, KEYWD_OWNER)) {
							free_string(ed->description);
							strcpy(owner, ch->name);
							ed->description = str_dup(owner);
							foundold = TRUE;
						}
				}

				if (!foundold) {
					strcpy(owner, ch->name);
					ed                      = new_extra_descr();
					ed->keyword             = str_dup(KEYWD_OWNER);
					ed->description         = str_dup(owner);
					ed->next                = t_obj->extra_descr;
					t_obj->extra_descr      = ed;
				}
			}

			t_obj->timer = 0;

			if (cost < t_obj->cost)
				t_obj->cost = cost;

			obj_to_char(t_obj, ch);
		}
	}
} /* end do_buy() */

void do_list(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];

	if (IS_SET(GET_ROOM_FLAGS(ch->in_room), ROOM_PET_SHOP)) {
		ROOM_INDEX_DATA *pRoomIndexNext;
		CHAR_DATA *pet;
		bool found;

		/* hack to make new thalos pets work */
		if (ch->in_room->vnum == 9621)
			pRoomIndexNext = get_room_index(9706);
		else
			pRoomIndexNext = get_room_index(ch->in_room->vnum + 1);

		if (pRoomIndexNext == NULL) {
			bug("Do_list: bad pet shop at vnum %d.", ch->in_room->vnum);
			stc("You can't do that here.\n", ch);
			return;
		}

		found = FALSE;

		for (pet = pRoomIndexNext->people; pet; pet = pet->next_in_room) {
			if (IS_SET(pet->act, ACT_PET)) {
				if (!found) {
					found = TRUE;
					stc("Pets and Exotic Companions for sale:\n", ch);
				}

				sprintf(buf, "[%2d] %8d - %s\n",
				        pet->level,
				        10 * pet->level * pet->level,
				        pet->short_descr);
				stc(buf, ch);
			}
		}

		if (!found)
			stc("Sorry, we're out of pets right now.\n", ch);

		return;
	}
	else {
		/* non-pet shop */
		CHAR_DATA *keeper;
		OBJ_DATA *obj;
		int cost, count;
		bool found;
		char arg[MAX_INPUT_LENGTH];

		if ((keeper = find_keeper(ch)) == NULL)
			return;

		one_argument(argument, arg);
		found = FALSE;

		for (obj = keeper->carrying; obj; obj = obj->next_content) {
			if (obj->wear_loc == WEAR_NONE
			    &&   can_see_obj(ch, obj)
			    && (cost = get_cost(keeper, obj, TRUE)) > 0
			    && (arg[0] == '\0'
			        ||  is_name(arg, obj->name))) {
				if (!found) {
					found = TRUE;

					if (IS_QUESTSHOPKEEPER(keeper))
						stc("{PAll prices in QUEST POINTS!{x\n", ch);

					stc("[Lv Price Qty] Item\n", ch);
				}

				if (IS_OBJ_STAT(obj, ITEM_INVENTORY))
					sprintf(buf, "[%2d %5d -- ] %s\n",
					        obj->level, cost, obj->short_descr);
				else {
					count = 1;

					while (obj->next_content != NULL
					       && obj->pIndexData == obj->next_content->pIndexData
					       && !str_cmp(obj->short_descr,
					                   obj->next_content->short_descr)) {
						obj = obj->next_content;
						count++;
					}

					sprintf(buf, "[%2d %5d %2d ] %s\n",
					        obj->level, cost, count, obj->short_descr);
				}

				stc(buf, ch);
			}
		}

		if (!found)
			stc("You can't buy anything here.\n", ch);
		else {
			if (IS_QUESTSHOPKEEPER(keeper)) {
				stc("---------------------------\n", ch);
				stc("{PAll prices in QUEST POINTS!{x\n", ch);
			}
		}

		return;
	}
} /* end do_list() */

void do_sell(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost, roll;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Sell what?\n", ch);
		return;
	}

	if ((keeper = find_keeper(ch)) == NULL)
		return;

	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		act("$n tells you 'You don't have that item'.",
		    keeper, NULL, ch, TO_VICT);
		strcpy(ch->reply, keeper->name);
		return;
	}

	if (!can_drop_obj(ch, obj)) {
		stc("You can't let go of it.\n", ch);
		return;
	}

	if (!can_see_obj(keeper, obj)) {
		act("$n doesn't see what you are offering.", keeper, NULL, ch, TO_VICT);
		return;
	}

	if ((cost = get_cost(keeper, obj, FALSE)) <= 0) {
		act("$n laughs at $p and suggests a novel use for it.", keeper, obj, ch, TO_VICT);
		return;
	}

	if (!IS_QUESTSHOPKEEPER(keeper) && cost > (keeper->silver + 100 * keeper->gold)) {
		act("$n tells you 'The shop has fallen upon hard times. I cant afford $p'",
		    keeper, obj, ch, TO_VICT);
		return;
	}

	act("$n sells $p.", ch, obj, NULL, TO_ROOM);

	if (!IS_QUESTSHOPKEEPER(keeper)) {
		/* haggle */
		roll = number_percent();

		if (number_percent() < (GET_ATTR_CHR(ch) * 3))
			roll -= 15;

		if (roll < 1) roll = 1;

		if (!IS_OBJ_STAT(obj, ITEM_SELL_EXTRACT)
		    && roll < get_skill(ch, gsn_haggle)
		    && obj->pIndexData->item_type != ITEM_MONEY) {
			stc("You haggle with the shopkeeper.\n", ch);
			cost += obj->cost / 2 * roll / 100;
			cost = UMIN(cost, 95 * get_cost(keeper, obj, TRUE) / 100);
			cost = UMIN(cost, (keeper->silver + 100 * keeper->gold));
			check_improve(ch, gsn_haggle, TRUE, 4);
		}

		sprintf(buf, "You sell $p for %d silver and %d gold piece%s.",
		        cost - (cost / 100) * 100, cost / 100, cost == 1 ? "" : "s");
		act(buf, ch, obj, NULL, TO_CHAR);
		ch->gold     += cost / 100;
		ch->silver   += cost - (cost / 100) * 100;
		deduct_cost(keeper, cost);

		if (keeper->gold < 0)
			keeper->gold = 0;

		if (keeper->silver < 0)
			keeper->silver = 0;
	}
	else {
		sprintf(buf, "You sell $p for %d Quest Point%s.",
		        cost, cost == 1 ? "" : "s");
		act(buf, ch, obj, NULL, TO_CHAR);
		ch->questpoints += cost;
	}

	if (obj->item_type == ITEM_TRASH
	    || IS_OBJ_STAT(obj, ITEM_SELL_EXTRACT)
	    || IS_QUESTSHOPKEEPER(keeper))
		extract_obj(obj);
	else {
		obj_from_char(obj);
		obj->timer = number_range(50, 100);
		obj_to_keeper(obj, keeper);
	}

	return;
} /* end do_sell() */

void do_value(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Value what?\n", ch);
		return;
	}

	if ((keeper = find_keeper(ch)) == NULL)
		return;

	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		act("$n tells you 'You don't have that item'.",
		    keeper, NULL, ch, TO_VICT);
		strcpy(ch->reply, keeper->name);
		return;
	}

	if (!can_see_obj(keeper, obj)) {
		act("$n doesn't see what you are offering.", keeper, NULL, ch, TO_VICT);
		return;
	}

	if (!can_drop_obj(ch, obj)) {
		stc("You can't let go of it.\n", ch);
		return;
	}

	if ((cost = get_cost(keeper, obj, FALSE)) <= 0) {
		act("$n laughs at $p.", keeper, obj, ch, TO_VICT);
		return;
	}

	if (IS_QUESTSHOPKEEPER(keeper))
		sprintf(buf, "$n tells you 'I'll give you %d quest points for $p'.", cost);
	else {
		sprintf(buf,
		        "$n tells you 'I'll give you %d silver and %d gold coins for $p'.",
		        cost - (cost / 100) * 100, cost / 100);
	}

	act(buf, keeper, obj, ch, TO_VICT);
	strcpy(ch->reply, keeper->name);
	return;
}

/* put an item on auction, or see the stats on the current item or bet */
void do_auction(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	char buf[MSL], arg1[MIL], arg2[MIL];
	CLAN_DATA *clan;
	int min = 0;

	/* NPC can be extracted at any time and thus can't auction! */
	if (IS_NPC(ch)) {
		stc("Mobiles can't auction!\n", ch);
		return;
	}

	if (argument[0] == '\0') {
		if (IS_SET(ch->comm, COMM_NOAUCTION)) {
			new_color(ch, CSLOT_CHAN_AUCTION);
			stc("Auction channel is now ON.\n", ch);
			REMOVE_BIT(ch->comm, COMM_NOAUCTION);
			set_color(ch, WHITE, NOBOLD);
		}
		else {
			new_color(ch, CSLOT_CHAN_AUCTION);
			stc("Auction channel is now OFF.\n", ch);
			SET_BIT(ch->comm, COMM_NOAUCTION);
			set_color(ch, WHITE, NOBOLD);
		}

		return;
	}

	/* at least one argument given */
	if (IS_SET(ch->revoke, REVOKE_AUCTION)) {
		stc("The gods have revoked your ability to auction.\n", ch);
		return;
	}

	if (IS_SET(ch->comm, COMM_QUIET)) {
		new_color(ch, CSLOT_CHAN_AUCTION);
		stc("You must turn off quiet mode first.\n", ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	argument = one_argument(argument, arg1);

	if (!str_cmp(arg1, "who")) {
		channel_who(ch, "Auction", COMM_NOAUCTION, CSLOT_CHAN_AUCTION);
		return;
	}

	if (!str_cmp(arg1, "status")) {
		if (auction->item == NULL) {
			stc("No item is up for auction.\n", ch);
			return;
		}

		ptc(ch, "Name (%s)   Type (%s)   Wear (%s)\n"
		    "Level (%d)   Cost (%d)   Condition  (%d)\n",
		    auction->item->short_descr,
		    item_type_name(auction->item),
		    wear_bit_name(auction->item->wear_flags),
		    auction->item->level,
		    auction->item->cost,
		    auction->item->condition);

		if (auction->bet > 0)
			ptc(ch, "Current bid on this item is %d gold.\n", auction->bet);
		else
			stc("No bids on this item have been received.\n", ch);

		if (auction->min > 0)
			ptc(ch, "The minimum bid on this item is %d gold.\n", auction->min);
		else
			stc("There is no minimum bid on this item.\n", ch);

		if (IS_IMMORTAL(ch))
			ptc(ch, "Seller: %s\n"
			    "Current Bidder: %s\n",
			    (auction->seller == NULL) ? "(None)" : auction->seller->name,
			    (auction->buyer == NULL)  ? "(None)" : auction->buyer->name);

		return;
	}

	if (IS_IMMORTAL(ch) && !str_cmp(arg1, "stop")) {
		if (auction->item == NULL) {
			stc("There is no auction going on you can stop.\n", ch);
			return;
		}

		sprintf(buf, "Sale of %s has been stopped by the Imms. Item confiscated.\n",
		        auction->item->short_descr);
		talk_auction(NULL, buf);
		sprintf(buf, "%s has stopped the auction of %s.", ch->name, auction->item->short_descr);
		wiznet(buf, ch, NULL, WIZ_AUCTION, 0, GET_RANK(ch));
		obj_to_char(auction->item, ch);
		auction->item = NULL;

		if (auction->buyer != NULL) { /* return money to the buyer */
			auction->buyer->gold += auction->bet;
			stc("Your money has been returned.\n", auction->buyer);
		}

		return;
	}

	if (!str_cmp(arg1, "bet") || !str_cmp(arg1, "bid")) {
		int newbet;

		if (auction->item == NULL) {
			stc("There isn't anything being auctioned right now.\n", ch);
			return;
		}

		if (auction->seller == ch) {
			stc("You may not bid on your own items.\n", ch);
			return;
		}

		if (auction->buyer == ch) {
			stc("You have already bid on it!\n", ch);
			return;
		}

		if (IS_IMMORTAL(ch)) {
			stc("Immortals may not bid on auctions.\n", ch);
			return;
		}

		/* Took this out cause it seems to have started to malfunction - Lotus
		                newbet = parsebet (auction->bet, argument);
		*/
		if (argument[0] == '\0')
			newbet = auction->bet + UMAX(auction->bet / 10, 1);
		else {
			if (!is_number(argument)) {
				stc("Your bid must be numeric.\n", ch);
				return;
			}

			newbet = atoi(argument);

			if ((newbet - auction->bet) < UMAX(auction->bet / 10, 1)) {
				stc("You must bid at least 10% over the current bid.\n", ch);
				return;
			}
		}

		if ((newbet < auction->min) && (auction->min > 0)) {
			ptc(ch, "The minimum bid for that item is %d gold.\n", auction->min);
			return;
		}

		if (!deduct_cost(ch, newbet * 100)) { /* subtracts the gold */
			stc("You don't have that much money!\n", ch);
			return;
		}

		/* return the gold to the last buyer, if one exists */
		if (auction->buyer != NULL)
			deduct_cost(auction->buyer, -(auction->bet * 100));

		/* players spam bidding without having to type their bid! :P */
		if (argument[0] == '\0')
			WAIT_STATE(ch, PULSE_PER_SECOND);

		auction->buyer = ch;
		auction->bet   = newbet;
		auction->going = 0;
		auction->pulse = PULSE_AUCTION; /* start the auction over again */
		sprintf(buf, "A bid of %d gold has been received on %s.\n",
		        newbet, auction->item->short_descr);
		talk_auction(NULL, buf);
		sprintf(buf, "%s has bid %d gold.", ch->name, newbet);
		wiznet(buf, ch, NULL, WIZ_AUCTION, 0, GET_RANK(ch));
		return;
	} /* end bid */

	if (auction->item != NULL) {
		act("Try again later - $p is being auctioned right now!", ch, auction->item, NULL, TO_CHAR);
		return;
	}

	/* auc was not followed by an auc command keyword, so we auc an object */
	/* Added minimum bid - Lotus */
	argument = one_argument(argument, arg2);

	/* changed from obj_list to obj_carry so as not to auc worn EQ -- Elrac */
	if ((obj = get_obj_carry(ch, arg1)) == NULL) { /* does char have the item ? */
		stc("You aren't carrying that.\n", ch);
		return;
	}

	/* can't auction food made with create food */
	if (obj->pIndexData->vnum == GEN_OBJ_FOOD) {
		stc("It looks too delicious to just auction away. :)\n", ch);
		return;
	}

	/* Cannot auction any clan items */
	clan = clan_table_head->next;

	while (clan != clan_table_tail) {
		if (obj->pIndexData->vnum >= clan->area_minvnum
		    && obj->pIndexData->vnum <= clan->area_maxvnum) {
			ptc(ch, "You attempt to auction %s, which belongs to %s.\n"
			    "%s {Yexplodes violently{x, leaving only a cloud of {gsmoke{x.\n"
			    "You are lucky you weren't {Phurt!{x\n",
			    obj->short_descr,
			    clan->clanname,
			    obj->short_descr);
			obj_from_char(obj);
			extract_obj(obj);
			return;
		}

		clan = clan->next;
	}

	if (!can_drop_obj(ch, obj)) {
		stc("Hah, you can't get rid of it that easily!\n", ch);
		return;
	}

	if (obj->pIndexData->vnum == OBJ_VNUM_NEWBIEBAG
	    || obj->pIndexData->vnum == OBJ_VNUM_TOKEN) {
		stc("I'm sorry, you can't auction newbie items.\n", ch);
		return;
	}

	if (arg2[0] != '\0') {
		if (!is_number(arg2)) {
			stc("Minimum bid value must be numeric.\n", ch);
			return;
		}

		min = atoi(arg2);

		if (min < 1 || min > 10000) {
			stc("The minimum bid must be between 1 and 10000 gold.\n", ch);
			return;
		}
	}

	switch (obj->item_type) {
	default:
		act("You cannot auction $Ts.", ch, NULL, item_type_name(obj), TO_CHAR);
		return;

	case ITEM_WEAPON:
	case ITEM_LIGHT:
	case ITEM_ARMOR:
	case ITEM_WAND:
	case ITEM_FOOD:
	case ITEM_STAFF:
	case ITEM_POTION:
	case ITEM_CONTAINER:
	case ITEM_DRINK_CON:
	case ITEM_BOAT:
	case ITEM_PILL:
	case ITEM_MAP:
	case ITEM_WARP_STONE:
	case ITEM_JEWELRY:
	case ITEM_SCROLL:
	case ITEM_TOKEN:
		obj_from_char(obj);
		auction->item   = obj;
		auction->bet    = 0;
		auction->buyer  = NULL;
		auction->seller = ch;
		auction->pulse  = PULSE_AUCTION;
		auction->going  = 0;
		auction->min    = min;

		if (auction->min > 0)
			sprintf(buf, "The auctioneer receives %s and announces a minimum bid of %d gold.\n",
			        obj->short_descr, auction->min);
		else
			sprintf(buf, "The auctioneer receives %s and places it on the auction block.\n",
			        obj->short_descr);

		talk_auction(NULL, buf);
		sprintf(buf, "%s is auctioning %s.", ch->name, obj->short_descr);
		wiznet(buf, ch, NULL, WIZ_AUCTION, 0, GET_RANK(ch));
		return;
	}
}

/* get the name of the anvil's owner */
char *anvil_owner_name(OBJ_DATA *anvil)
{
	/* check if private anvil */
	if (anvil->value[2] == 0) return NULL;

	/* anvil name must begin with "anvil private " */
	if (str_prefix1("anvil private ", anvil->name)) {
		bug("anvil_owner_name: anvil %d has a private flag but incorrect name",
		    anvil->pIndexData->vnum);
		return NULL;
	}

	return anvil->name + strlen("anvil private ");
}

/* is named player the anvil's owner? -- Elrac */
int is_anvil_owner(CHAR_DATA *ch, OBJ_DATA *anvil)
{
	return is_exact_name(ch->name, anvil_owner_name(anvil));
}

/*
 * forge_flag function, recoded by Vegita and Montrey for use with
 * Age of Legacy's Evolution System.
 *
 */
void forge_flag(CHAR_DATA *ch, const char *argument, OBJ_DATA *anvil)
{
	char arg[MIL];
	OBJ_DATA *weapon;
	int flag_table_num, flag, flag_count = 0, evo, qpcost;
	evo = get_evolution(ch, gsn_forge);

	/* are they wielding a weapon? */
	if ((weapon = get_eq_char(ch, WEAR_WIELD)) == NULL) {
		stc("You must be wielding a weapon to forge flags upon!\n", ch);
		return;
	}

	/* Weapons under level 30 cannot recieve perm flags */
	if (!IS_IMMORTAL(ch) && weapon->level < 30) {
		stc("The weapon you are wielding is not powerful enough to receive enhancement.\n", ch);
		return;
	}

	argument = one_argument(argument, arg);

	/* player used a valid flag type? */
	if ((flag_table_num = flag_lookup(arg, weapon_flags)) == -1) {
		ptc(ch, "'%s' is not a valid weapon flag name, sorry!\n", arg);
		return;
	}

	flag = weapon_flags[flag_table_num].bit;

	/* have to be evo 2 to forge vorpal, otherwise, can forge everything but sharp and poison */
	if (flag == WEAPON_POISON
	    || flag == WEAPON_SHARP
	    || (flag == WEAPON_VORPAL
	        && evo < 2)) {
		stc("You cannot forge that flag.\n", ch);
		return;
	}

	/* now we count the already existing flags */
	/* you can always forge vorpal or two handed, no matter how many flags it has,
	   so those are taken care of above.  now we only care about preventing them
	   from forging too many magic flags */
	if (IS_WEAPON_STAT(weapon, WEAPON_FLAMING))             flag_count++;

	if (IS_WEAPON_STAT(weapon, WEAPON_FROST))               flag_count++;

	if (IS_WEAPON_STAT(weapon, WEAPON_VAMPIRIC))    flag_count++;

	if (IS_WEAPON_STAT(weapon, WEAPON_SHOCKING))    flag_count++;

	if ((flag_count >= 2 || (flag_count >= 1 && evo < 3))
	    && flag != WEAPON_TWO_HANDS
	    && flag != WEAPON_VORPAL) {
		stc("You cannot forge any more magical flags on that weapon.\n", ch);
		return;
	}

	/* now we check to see if it already has the weapon flag they are trying to forge
	 * to prevent duplicate weapon flags being forged on the same weapon.
	 */
	if (flag == WEAPON_TWO_HANDS && IS_WEAPON_STAT(weapon, WEAPON_TWO_HANDS)) {
		act("$p is already a two-handed weapon.", ch, weapon, NULL, TO_CHAR);
		return;
	}

	if (flag == WEAPON_FLAMING && IS_WEAPON_STAT(weapon, WEAPON_FLAMING)) {
		act("$p is already a flaming weapon.", ch, weapon, NULL, TO_CHAR);
		return;
	}

	if (flag == WEAPON_FROST && IS_WEAPON_STAT(weapon, WEAPON_FROST)) {
		act("$p's is already coated with ice.", ch, weapon, NULL, TO_CHAR);
		return;
	}

	if (flag == WEAPON_VAMPIRIC && IS_WEAPON_STAT(weapon, WEAPON_VAMPIRIC)) {
		act("$p already drinks the blood of its victim.", ch, weapon, NULL, TO_CHAR);
		return;
	}

	if (flag == WEAPON_SHOCKING && IS_WEAPON_STAT(weapon, WEAPON_SHOCKING)) {
		act("$p is already imbued with lightning.", ch, weapon, NULL, TO_CHAR);
		return;
	}

	if (flag == WEAPON_VORPAL && IS_WEAPON_STAT(weapon, WEAPON_VORPAL)) {
		act("$p is already sharp enough to sever limbs.", ch, weapon, NULL, TO_CHAR);
		return;
	}

	/*setup qpcosts*/
	if (!IS_IMMORTAL(ch)) {
		qpcost = anvil->value[1];   /* default to public cost */

		if (is_anvil_owner(ch, anvil))
			qpcost = anvil->value[4];

		if (flag_count > 0)
			qpcost *= 5;
	}
	else
		qpcost = 0;

	/* check for required QP */
	if (ch->questpoints < qpcost) {
		ptc(ch, "You lack the required %d quest points to forge flags on this anvil.\n", qpcost);
		return;
	}

	/* deduct quest points */
	if (qpcost > 0) {
		ptc(ch, "In a hasty prayer, you offer %d points of your questing experience as a sacrifice.\n", qpcost);
		ch->questpoints -= qpcost;
	}

	ptc(ch, "You grab the smith hammer next to the anvil, lay %s down upon the anvil, and strike it with a mighty blow! *CLING!*\n",
	    weapon->short_descr);
	act("$n lays $p upon the anvil and strikes it with a mighty blow!", ch, weapon, NULL, TO_ROOM);

	if (IS_IMMORTAL(ch) || number_percent() < (flag_count >= 1 ? 50 : 30)) {
		stc("Amid sparks and rising smoke, an awesome transformation affects your weapon!\n", ch);
		act("$n's weapon strikes sparks on the anvil, and smoke rises. It is... changed!", ch, NULL, NULL, TO_ROOM);
		SET_BIT(weapon->value[4], flag);

		if (flag == WEAPON_TWO_HANDS) {
			weapon->value[1]++;
			remove_obj(ch, WEAR_WIELD, TRUE);
		}
	}
	else {
		if (flag_count >= 1) {
			stc("Your weapon begins to glow a bright yellow then suddenly explodes!!!!!\n", ch);
			act("$n's weapon glows a bright yellow then suddenly explodes.\n", ch, NULL, NULL, TO_ROOM);
			extract_obj(weapon);
			return;
		}
		else {
			stc("The blow jars your arm, but you notice no change in your weapon.\n", ch);
			act("$n looks disappointed.", ch, NULL, NULL, TO_ROOM);
			return;
		}
	}
}

/* Hone by Montrey */
void do_hone(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *weapon;
	OBJ_DATA *whetstone;
	whetstone = get_eq_char(ch, WEAR_HOLD);
	weapon = get_eq_char(ch, WEAR_WIELD);

	if (IS_NPC(ch) || (get_skill(ch, gsn_hone) < 1)) {
		stc("You lack the skill to hone weapons.\n", ch);
		return;
	}

	if (weapon == NULL) {
		stc("Hone what, your hand?  Try wielding it.\n", ch);
		return;
	}

	if (IS_WEAPON_STAT(weapon, WEAPON_SHARP)) {
		act("$p could not possibly be any sharper.", ch, weapon, NULL, TO_CHAR);
		return;
	}

	if ((attack_table[weapon->value[3]].damage != DAM_SLASH)
	    && (attack_table[weapon->value[3]].damage != DAM_PIERCE)) {
		stc("You can only hone edged weapons.\n", ch);
		return;
	}

	if ((whetstone == NULL) || (!is_name("whetstone", whetstone->name))) {
		stc("How do you expect to sharpen a weapon without holding a whetstone?\n", ch);
		return;
	}

	if (!deduct_stamina(ch, gsn_hone))
		return;

	if (!IS_IMMORTAL(ch)) {
		if (number_percent() > UMIN(get_skill(ch, gsn_hone), 95)) {
			sprintf(buf, "You fail to hone your weapon, and you gouge %s deeply, ruining it.\n",
			        whetstone->short_descr);
			stc(buf, ch);
			act("$n's hand slips, and $e ruins $s whetstone.", ch, NULL, NULL, TO_ROOM);
			extract_obj(whetstone);
			return;
		}
	}

	sprintf(buf, "You skillfully hone %s to a razor edge.\n", weapon->short_descr);
	act("$n skillfully sharpens $p to a razor edge.", ch, weapon, NULL, TO_ROOM);
	stc(buf, ch);
	SET_BIT(weapon->value[4], WEAPON_SHARP);
	return;
}

void do_forge(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj, *anvil = NULL, *material;
	CHAR_DATA *owner;
	char type[MSL], sdesc[MSL], buf[MSL], costbuf[MSL];
	EXTRA_DESCR_DATA *ed;
	extern char *const month_name[];
	int is_owner, cost, cost_gold, cost_silver, evo;
	evo = get_evolution(ch, gsn_forge);
	/* check arguments */
	argument = one_argument(argument, type);

	if (argument[0] == '\0' || type[0] == '\0') {
		if (get_skill(ch, gsn_forge))
			stc("Syntax: {Rforge{x <weapon type> <weapon name>\n"
			    "        {Rforge flag{x <flag type>\n", ch);
		else
			stc("Syntax: {Rforge flag{x <flag type>\n", ch);

		stc("Possible flag types are: flaming, frost, shocking, vampiric, vorpal, or twohanded.\n", ch);
		stc("(see HELP FORGE for details)\n", ch);
		return;
	}

	/* Check for anvil in room -- Elrac */
	for (obj = ch->in_room->contents; obj; obj = obj->next_content) {
		if (obj->item_type == ITEM_ANVIL) {
			anvil = obj;
			break;
		}
	}

	if (anvil == NULL && !IS_IMMORTAL(ch)) {
		stc("How do you propose to forge without an anvil?\n", ch);
		stc("A public anvil may be found in the smithy below Griswold's Repair Shop.\n", ch);
		return;
	}

	/* check for FORGE FLAG */
	if (!str_prefix1(type, "flag")) {
		if (!deduct_stamina(ch, gsn_forge))
			return;

		forge_flag(ch, argument, anvil);
		return;
	}

	/* not FORGE FLAG, so check skill */
	if (!get_skill(ch, gsn_forge)) {
		stc("You stand before the anvil, feeling foolish.\n"
		    "Lacking the skill to forge, you would only ruin the material.\n", ch);
		return;
	}

	if (weapon_lookup(type) == -1) {
		stc("That is not a valid weapon type.\n", ch);
		return;
	}

	/* Check for cost of using anvil, and whether player can afford it -- Elrac */
	if (IS_IMMORTAL(ch)) {
		is_owner = 0;
		cost = 0;
	}
	else {
		is_owner = is_anvil_owner(ch, anvil);
		cost = is_owner ? anvil->value[3] : anvil->value[0];

		if (cost > 0) {
			if (100 * ch->gold + ch->silver < cost) {
				ptc(ch, "It costs %d silver to forge on this anvil, and you don't have that much!\n", cost);
				return;
			}
		}
	}

	if (color_strlen(argument) > 30) {
		stc("Name is limited to 30 printed characters.\n", ch);
		return;
	}

	if ((material = get_eq_char(ch, WEAR_HOLD)) == NULL) {
		stc("You are not holding any materials to make a weapon!\n", ch);
		return;
	}

	if (material->item_type != ITEM_MATERIAL) {
		stc("That is not a useable material!\n", ch);
		return;
	}

	if (!deduct_stamina(ch, gsn_forge))
		return;

	WAIT_STATE(ch, skill_table[gsn_forge].beats);

	if (!IS_IMMORTAL(ch) && number_percent() > (get_skill(ch, gsn_forge) + material->value[0])) {
		stc("You fail to forge a useful weapon.\n", ch);
		act("$n tries but fails to forge a useful weapon.\n", ch, NULL, NULL, TO_ROOM);
		check_improve(ch, gsn_forge, FALSE, 1);
		extract_obj(material);
		return;
	}

	obj = create_object(get_obj_index(OBJ_VNUM_WEAPON), 0);

	if (!obj) {
		bug("Memory error in do_forge.", 0);
		stc("An error occured while trying to forge.\n", ch);
		return;
	}

	const char *name = smash_tilde(argument); // volatile, good until smash_tilde called again

	obj->level = ch->level;
	free_string(obj->material);
	obj->material = str_dup(material->material);
	obj->condition = material->condition;

	obj->extra_flags = material->extra_flags;

	for (const AFFECT_DATA *paf = affect_list_obj(material); paf; paf = paf->next)
		affect_copy_to_obj(obj, paf);

	obj->value[0] = weapon_type(type);
	free_string(obj->name);
	sprintf(buf, "%s %s", weapon_table[weapon_lookup(type)].name, smash_bracket(name));
	obj->name = str_dup(buf);
	sprintf(sdesc, "%s{x", name);
	free_string(obj->short_descr);
	obj->short_descr = str_dup(sdesc);
	sprintf(buf, "A %s is here, forged by %s's craftsmanship.", weapon_table[weapon_lookup(type)].name, ch->name);
	free_string(obj->description);
	obj->description = str_dup(buf);
	ed = new_extra_descr();
	sprintf(buf, "It is a marvellous %s, crafted from the finest %s around.\n"
	        "It was created in the Month of %s by %s %s\n"
	        "named %s.  Legend holds that this %s was a great weaponsmith.\n",
	        weapon_table[weapon_lookup(type)].name, obj->material,
	        month_name[time_info.month],
	        (ch->level > LEVEL_HERO) ? "an immortal" : (ch->level > 75) ? "a master" :
	        (ch->level > 50) ? "an experienced" : (ch->level > 25) ? "a young" :
	        "a newbie", class_table[ch->cls].name, ch->name,
	        race_table[ch->race].name);
	ed->keyword        = str_dup(obj->name);
	ed->description    = str_dup(buf);
	ed->next           = obj->extra_descr;
	obj->extra_descr   = ed;
	obj->value[1] = 5;
	obj->value[2] = number_fuzzy(ch->level / 5);

	if (evo == 2) /*wchange evolution bonuses to damage roll*/
		obj->value[2] += 1;

	if (evo == 3)
		obj->value[2] += 2;

	if (evo == 4)
		obj->value[2] += 3;

	switch (obj->value[0]) {
	case WEAPON_SWORD  : obj->value[3] = attack_lookup("slash");   break;

	case WEAPON_MACE   : obj->value[3] = attack_lookup("pound");   break;

	case WEAPON_DAGGER : obj->value[3] = attack_lookup("pierce");  break;

	case WEAPON_AXE    : obj->value[3] = attack_lookup("chop");    break;

	case WEAPON_SPEAR  : obj->value[3] = attack_lookup("thrust");  break;

	case WEAPON_FLAIL  : obj->value[3] = attack_lookup("crush");   break;

	case WEAPON_WHIP   : obj->value[3] = attack_lookup("whip");    break;

	case WEAPON_POLEARM: obj->value[3] = attack_lookup("beating"); break;
	case WEAPON_BOW    : obj->value[3] = attack_lookup("arrow"); break;
	}

	obj->weight = number_fuzzy(number_range(ch->level / 10, ch->level / 15) * 10);
	obj->cost = ch->level / number_range(1, 10);
	/* Add a few bonuses */
	obj->value[1] += material->value[1];
	obj->value[2] += material->value[2];

	if (ch->level >= 50)
		obj->value[2] += 1;

	obj_to_char(obj, ch);
	/* Nicer message -- Elrac */
	sprintf(buf, "%s %s into a formidable %s\ncalled \"%s\".\n",
	        "You quickly and skillfully forge",
	        material->short_descr,
	        weapon_table[weapon_lookup(type)].name,
	        obj->short_descr);
	stc(buf, ch);
	sprintf(buf, "%s has forged a %s named \"%s\".", ch->name,
	        weapon_table[weapon_lookup(type)].name, obj->short_descr);
	act(buf, ch, obj, NULL, TO_ROOM);
	extract_obj(material);
	check_improve(ch, gsn_forge, TRUE, 1);

	/* Charge player for forging -- Elrac */
	if (cost > 0) {
		if (anvil->value[2] == 0) { /* public */
			sprintf(buf,
			        "You pay the community of Thera %d silver for the use of its anvil.\n",
			        cost);
			stc(buf, ch);
		}
		else {
			sprintf(buf,
			        "You pay %d silver for the use of this private anvil.\n",
			        cost);
			stc(buf, ch);

			if (! is_owner) {
				owner = get_char_world(ch, anvil_owner_name(anvil), VIS_CHAR);

				if (owner != NULL) {
					cost_gold = cost / 100;
					cost_silver = cost - 100 * cost_gold;
					sprintf(buf, "%s pays you ", ch->name);

					if (cost_gold > 0) {
						sprintf(costbuf, "%d gold ", cost_gold);
						strcat(buf, costbuf);

						if (cost_silver > 0) strcat(buf, "and ");
					}

					if (cost_silver > 0) {
						sprintf(costbuf, "%d silver ", cost_silver);
						strcat(buf, costbuf);
					}

					strcat(buf, "for the use of your anvil.\n");
					stc(buf, owner);
					owner->gold += cost_gold;
					owner->silver += cost_silver;
				}
			}
		}

		deduct_cost(ch, cost);
	}
} /* end do_forge() */

/* Engrave a message of historical significance on a weapon -- Elrac */
void do_engrave(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *jeweler = NULL;
	OBJ_DATA *weapon;
	EXTRA_DESCR_DATA *ed;
	EXTRA_DESCR_DATA *eng_desc;
	EXTRA_DESCR_DATA *dflt_desc;
	char buf[10 * MAX_INPUT_LENGTH];
	char *pd;           /* ptr into desc text */
	char *pb;           /* ptr into buffer */
	char *l2, *ld, *la; /* line 2, line with dup author, line after dup author */
	int  lines;         /* number of lines */
	char c0;
	BUFFER *dbuf = NULL;
	char *eng_line;
	struct timeval now_time;
	time_t current_time;

	if (!IS_IMMORTAL(ch) &&
	    (jeweler = get_mob_here(ch, "jeweler", VIS_CHAR)) == NULL &&
	    (jeweler = get_mob_here(ch, "jeweller", VIS_CHAR)) == NULL) {
		stc("You need a jeweler's engraving tools to engrave.\n", ch);
		stc("Find a jeweler and try again on his or her premises!\n", ch);
		return;
	}

	if ((weapon = get_eq_char(ch, WEAR_WIELD)) == NULL) {
		stc("You must {Rwield{x a weapon in order to engrave it.\n", ch);
		return;
	}

	if (!argument[0]) {
		stc("What do you want to engrave on your weapon?\n", ch);
		return;
	}

	/* Imms have no length limit. Please don't overdo it! */
	/* If you change this, remember that the jeweler can == NULL for imms! */
	if (!IS_IMMORTAL(ch) && color_strlen(argument) > 75) {
		check_social(jeweler, "snort", "");
		/* ENGRAVE 123456789012345678901234567890123456789012345678901234567890123456789012345 */
		stc("       \"The blade of a weapon is not suited for the Great Theran Novel!\"\n", ch);
		stc("       \"A profound sentiment, well expressed, will be no longer than this sentence.\"\n", ch);
		return;
	}

	/* OK, we will engrave. */
	/* If there is no long description or if the long description does not
	   mention engravings yet, then add a line to this effect. */
	eng_line = "There are some personal {Cengravings{x on this weapon.\n";
	eng_desc = NULL;
	dflt_desc = NULL;
	one_argument(weapon->name, buf);

	for (ed = weapon->extra_descr; ed; ed = ed->next) {
		if (dflt_desc == NULL && is_name(buf, ed->keyword))
			dflt_desc = ed;

		if (eng_desc == NULL && is_name("engravings", ed->keyword))
			eng_desc = ed;
	}

	if (eng_desc == NULL) {
		/* no engravings yet, build an empty extdesc */
		eng_desc = new_extra_descr();
		eng_desc->keyword = str_dup("engravings");
		eng_desc->description = str_dup("\n");
		eng_desc->next = weapon->extra_descr;
		weapon->extra_descr = eng_desc;
	}

	if (dflt_desc == NULL) {
		/* no extdesc for wpn, build an empty one */
		dflt_desc = new_extra_descr();
		dflt_desc->keyword = str_dup(weapon->name);

		if (weapon->description && weapon->description[0]) {
			dbuf = new_buf();
			add_buf(dbuf, weapon->description);
			add_buf(dbuf, "\n");
			add_buf(dbuf, eng_line);
			dflt_desc->description = str_dup(buf_string(dbuf));
			free_buf(dbuf);
		}
		else
			dflt_desc->description = str_dup(eng_line);

		dflt_desc->next = weapon->extra_descr;
		weapon->extra_descr = dflt_desc;
	}
	else if (strstr(dflt_desc->description, eng_line) == NULL) {
		/* add to existing extdesc */
		dbuf = new_buf();
		add_buf(dbuf, dflt_desc->description);
		add_buf(dbuf, eng_line);
		free_string(dflt_desc->description);
		dflt_desc->description = str_dup(buf_string(dbuf));
		free_buf(dbuf);
		dbuf = NULL;
	}

	/* Split existing extdesc into single lines.
	   Nothing tricky about this except that line ends could be NL and/or CR.
	   Mission goals:
	   - mark line 2, in case a line needs to be deleted; (l2)
	   - mark existing engraving by same author, if any; (ld)
	   - mark first line after engraving by existing author, if any; (la)
	   - count the number of existing lines. (lines)
	*/
	lines = 1;
	l2 = NULL;
	ld = NULL;
	la = NULL;
	pb = buf;

	for (pd = eng_desc->description; *pd;) {
		if (pd[0] == '\n') {
			*pb++ = *pd++;
			*pb = '\0';

			if (++lines == 2 && *pd != '\0')
				l2 = pd;
		}
		else {
			*pb++ = *pd++;
			continue;
		}

		/* there is a complete line in buf, ending in CR and/or LF and NUL. */
		if (ld == NULL &&
		    buf[0] == '[' &&
		    isdigit(buf[1]) &&
		    isdigit(buf[2]) &&
		    isdigit(buf[3]) &&
		    isdigit(buf[4]) &&
		    isdigit(buf[5]) &&
		    isdigit(buf[6]) &&
		    buf[7] == ']' &&
		    buf[8] == ' ' &&
		    buf[9] == '{' &&
		    isalpha(buf[10]) &&
		    !strncmp(ch->name, buf + 11, strlen(ch->name)) &&
		    buf[11 + strlen(ch->name)] == '{')
			ld = pd - strlen(buf);
		else if (la == NULL && ld != NULL)
			la = pd;

		pb = buf;
	} /* end for chars in desc */

	/* assemble 'worthy' parts of the old desc into a buffer. */
	dbuf = new_buf();

	if (ld != NULL) {
		c0 = *ld;
		*ld = '\0';
		add_buf(dbuf, eng_desc->description);
		*ld = c0;
		add_buf(dbuf, la);
	}
	else if (lines > 20)
		add_buf(dbuf, l2);
	else
		add_buf(dbuf, eng_desc->description);

	free_string(eng_desc->description);
	gettimeofday(&now_time, NULL);
	current_time = (time_t) now_time.tv_sec;
	strftime(buf, 9, "[%m/%d] ", localtime(&current_time));
	add_buf(dbuf, buf);

	if (IS_NPC(ch)) {
		sprintf(buf, "{Y%s{x, {Mcitizen of %s,", ch->short_descr,
		        ch->in_room && ch->in_room->area && ch->in_room->area->name ?
		        ch->in_room->area->name : "Thera");
	}
	else {
		sprintf(buf, "{Y%s{W%s{x ", ch->name,
		        ch->pcdata && ch->pcdata->title[0] ?
		        ch->pcdata->title : "{M(adventurer of Thera){x");
	}

	add_buf(dbuf, buf);
	sprintf(buf, "engraved {Ythis{x:\n \"%s\".\n", smash_tilde(argument));
	add_buf(dbuf, buf);
	eng_desc->description = str_dup(buf_string(dbuf));
	free_buf(dbuf);
	stc("You have left a mark of duration upon your weapon.\n", ch);
	act("$n solemnly engraves $s weapon.", ch, NULL, NULL, TO_ROOM);
} /* end do_engrave() */

/* Change the looks on weddingrings */
void do_weddingring(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *jeweler = NULL;
	OBJ_DATA *ring;
	char arg1[MAX_INPUT_LENGTH];
	int price = 0;

	if (ch->desc->original != NULL) {
		stc("Switch back to your real form!\n", ch);
		return;
	}

	if (!IS_IMMORTAL(ch) &&
	    (jeweler = get_mob_here(ch, "jeweler", VIS_CHAR)) == NULL &&
	    (jeweler = get_mob_here(ch, "jeweller", VIS_CHAR)) == NULL) {
		stc("You need a jeweler's engraving tools to engrave.\n", ch);
		stc("Find a jeweler and try again on his or her premises!\n", ch);
		return;
	}

	if ((ring = get_eq_char(ch, WEAR_WEDDINGRING)) == NULL) {
		stc("You must {Rwear{x your wedding ring in order to have it changed.\n", ch);
		return;
	}

	argument = one_argument(argument, arg1);

	if (arg1[0] == '\0' || argument[0] == '\0')
		goto help;

	if (!IS_IMMORTAL(ch))
		price = 2 * color_strlen(argument);

	if (price > 100 && !IS_IMMORTAL(ch)) {
		stc("The jeweler exclaims 'That's too long! It'll never fit on such a tiny ring!\n", ch);
		return;
	}

	if ((ch->silver + ch->gold * 100) < price) {
		ptc(ch, "A description like that costs %d gold. And you don't have that much, do you?\n", price);
		return;
	}

	if (!str_prefix1(arg1, "long")) {
		ptc(ch, "The long description of your weddingring is now:\n{x'%s{x'.\n", argument);
		free_string(ring->description);
		ring->description = str_dup(smash_tilde(argument));
	}
	else if (!str_prefix1(arg1, "short")) {
		ptc(ch, "The short description of your weddingring is now:\n{x'%s{x'.\n", argument);
		free_string(ring->short_descr);
		ring->short_descr = str_dup(smash_tilde(argument));
	}
	else
		goto help;

	deduct_cost(ch, price * 100);
	ptc(ch, "You pay the jeweler %d gold.\n", price);
	return;
help:
	stc("The jeweler explains to you the syntax you should use:\n", ch);
	stc("weddingring short <text>\n", ch);
	stc("weddingring long  <text>\n", ch);
	stc("\nThe price is 2 gold pieces for every character of text.\n", ch);
	return;
}

void do_lore(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;

	if (!get_skill(ch, gsn_lore)) {
		stc("You aren't trained in the lore of items.\n", ch);
		return;
	}

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("What do you want to lore?\n", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, arg)) == NULL
	 && (obj = get_obj_here(ch, arg)) == NULL) {
		stc("You do not have that item.\n", ch);
		return;
	}

	if (!prd_chance(&ch->skill_fails, get_skill(ch, gsn_lore))) {
		act("You look at $p, but you can't find out any additional information.", ch, obj, NULL, TO_CHAR);
		act("$n looks at $p but cannot find out anything.", ch, obj, NULL, TO_ROOM);
		return;
	}
	else {
		act("$n studies $p, discovering all of its hidden powers.", ch, obj, NULL, TO_ROOM);
		spell_identify(gsn_lore, (4 * obj->level) / 3, ch, obj, TARGET_OBJ, get_evolution(ch, gsn_lore));
		check_improve(ch, gsn_lore, TRUE, 4);
	}
}

void do_autograph(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	char arg[MIL], buf[MSL];
	one_argument(argument, arg);

	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		stc("You are not carrying that.\n", ch);
		return;
	}

	sprintf(buf, "%s {c(autographed){x", obj->short_descr);
	free_string(obj->short_descr);
	obj->short_descr = str_dup(buf);
	stc("You sign the card.\n", ch);
}

/*
This function allows a character to rename their pet.
-- Outsider
*/
void do_rename(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *pet;
	char new_name[64];

	/* make sure we have a pet */
	if (! ch->pet) {
		stc("You have no pet.\n", ch);
		return;
	}

	/* We have to name it something.... */
	if (argument[0] == '\0') {
		stc("So...it's a No Name brand?\n", ch);
		return;
	}

	/* check name length */
	if (strlen(argument) > 60) {
		stc("That is a mouth full!\n", ch);
		return;
	}

	pet = ch->pet;

	/* make sure the name has no '~' in it */
	if (strchr(pet->name, '~')) {
		stc("Your pet has an invalid name!\n", ch);
		return;
	}

	if (strchr(argument, '~')) {
		stc("That is not a valid name for your pet!\n", ch);
		return;
	}

	sprintf(new_name, "%s %s", pet->name, argument);
	pet->name = new_name;
	stc("Your pet has now been named ", ch);
	stc(argument, ch);
	stc(".\n", ch);
}

