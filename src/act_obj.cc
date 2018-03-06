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
#include <sys/time.h>

#include "act.hh"
#include "argument.hh"
#include "affect/Affect.hh"
#include "Area.hh"
#include "channels.hh"
#include "Character.hh"
#include "Clan.hh"
#include "declare.hh"
#include "Descriptor.hh"
#include "event/event.hh"
#include "ExtraDescr.hh"
#include "find.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "GameTime.hh"
#include "interp.hh"
#include "lookup.hh"
#include "Logging.hh"
#include "macros.hh"
#include "magic.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"
#include "ObjectValue.hh"
#include "Player.hh"
#include "QuestArea.hh"
#include "random.hh"
#include "Room.hh"
#include "Shop.hh"
#include "skill/skill.hh"
#include "String.hh"
#include "tables.hh"
#include "World.hh"

extern  void    channel_who     args((Character *ch, const char *channelname, int channel, int custom));

/*
 * Local functions.
 */
bool       remove_obj      args((Character *ch, int iWear, bool fReplace));
void       wear_obj        args((Character *ch, Object *obj, bool fReplace));
Character *find_keeper     args((Character *ch));
int        get_cost        args((Character *keeper, Object *obj, bool fBuy));
void       obj_to_keeper   args((Object *obj, Character *ch));
Object  *get_obj_keeper  args((Character *ch, Character *keeper, const String& argument));

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
		Format::sprintf(buf, "%dst", n);
	else if (n % 10 == 2)
		Format::sprintf(buf, "%dnd", n);
	else if (n % 10 == 3)
		Format::sprintf(buf, "%drd", n);
	else
		Format::sprintf(buf, "%dth", n);

	return buf;
}

/* Check to see if an object is non-holder clan equipment -- Elrac
   This code depends on the clan equipment being coded into the clan areas
   and the starting/ending vnums of the areas being coded into clan_table.
   This function returns TRUE if <ch> is allowed to do stuff with <obj>
   based on clan ownership and leadership. If not, it tells the player
   he can't do that and the object is destroyed. Also, FALSE is returned.
*/
bool clan_eq_ok(Character *ch, Object *obj, const String& action)
{
	if (IS_IMMORTAL(ch))
		return TRUE;

	Clan *jclan;
	if ((jclan = clan_vnum_lookup(obj->pIndexData->vnum)) == nullptr)
		return TRUE;

	if (ch->clan != jclan) {
		if (!action.empty()) {
			ptc(ch, "You attempt to %s %s, which belongs to %s.\n",
			        action, obj->short_descr, jclan->clanname);
		}

		ptc(ch, "%s {Yexplodes violently{x, leaving only a cloud of {gsmoke{x.\n",
		        obj->short_descr);
		stc("You are lucky you weren't {Phurt!{x\n", ch);
		destroy_obj(obj);
		return FALSE;
	}

	if (obj->name.has_words("leadereq") && !ch->has_cgroup(GROUP_LEADER)) {
		if (!action.empty()) {
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
bool pers_eq_ok(Character *ch, Object *obj, const String& action)
{
	const ExtraDescr *pdesc;
	char owner[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];

	if (ch == nullptr) {
		Logging::bug("pers_eq_ok: nullptr character", 0);
		return TRUE;
	}

	if (obj == nullptr) {
		Logging::bug("pers_eq_ok: nullptr object", 0);
		return TRUE;
	}

	if (obj->pIndexData == nullptr) {
		Logging::bug("pers_eq_ok: nullptr object->pIndexData", 0);
		return TRUE;
	}

	if ((pdesc = get_extra_descr(KEYWD_OWNER, obj->extra_descr)) == nullptr
	    && (pdesc = get_extra_descr(KEYWD_OWNER, obj->pIndexData->extra_descr)) == nullptr)
		return TRUE;

	sscanf(pdesc->description.c_str(), "%[^\n]", owner);

	if (String(owner).has_words(ch->name))
		return TRUE;

	if (obj->short_descr.empty()) {
		Logging::bugf("clan_eq_ok: object %d has no short_descr", obj->pIndexData->vnum);
		obj->short_descr = "something";
	}

	if (!action.empty()) {
		Format::sprintf(buf, "You attempt to %s %s.\n", action, obj->short_descr);
		stc(buf, ch);
		Format::sprintf(buf, "   This item belongs to %s!\n", owner);
		stc(buf, ch);

		if (IS_IMMORTAL(ch)) {
			stc("   As an Immortal, you have the right.\n", ch);
			return TRUE;
		}
	}

	return FALSE;
} /* end pers_eq_ok() */

/* RT part of the corpse looting code */

bool can_loot(Character *ch, Object *obj)
{
	Character *owner = nullptr, *wch;

	if (IS_IMMORTAL(ch))
		return TRUE;

	if (obj->owner.empty())
		return TRUE;

	for (wch = Game::world().char_list; wch != nullptr ; wch = wch->next)
		if (wch->name == obj->owner)
			owner = wch;

	if (owner == nullptr)
		return TRUE;

	if (ch->name == owner->name)
		return TRUE;

	if (!IS_NPC(owner) && owner->act_flags.has(PLR_CANLOOT))
		return TRUE;

	if (is_same_group(ch, owner))
		return TRUE;

	return FALSE;
}

void do_second(Character *ch, String argument)
{
	Object *obj;

	if (!get_skill_level(ch, skill::type::dual_wield) && !IS_NPC(ch)) {
		stc("You are not able to wield two weapons.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("Which weapon do you want to wield in your off-hand?\n", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, argument)) == nullptr) {
		stc("You do not have that weapon.\n", ch);
		return;
	}

	if (obj->item_type != ITEM_WEAPON) {
		stc("That is not a weapon.\n", ch);
		return;
	}

	if (!IS_IMMORTAL(ch)) {
		if (affect::exists_on_obj(obj, affect::type::weapon_two_hands) && ch->size < SIZE_HUGE) {
			stc("You can not dual-wield a two handed weapon.\n", ch);
			return;
		}

		if (get_eq_char(ch, WEAR_WIELD) != nullptr
		    && affect::exists_on_obj(get_eq_char(ch, WEAR_WIELD), affect::type::weapon_two_hands)
		    && ch->size < SIZE_LARGE) {
			stc("Your primary weapon is taking both of your hands.\n", ch);
			return;
		}

		if (get_eq_char(ch, WEAR_SHIELD) != nullptr
		    || get_eq_char(ch, WEAR_HOLD)   != nullptr) {
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
				act("$n tries to use $p, but is too inexperienced.", ch, obj, nullptr, TO_ROOM);
				return;
			}
			else {
				stc("This item must be lowered for mortal use.\n", ch);
				act("$n tries to use $p, but is too bound to the mortal plane.", ch, obj, nullptr, TO_ROOM);
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

	act("$n wields $p{x in $s off-hand.", ch, obj, nullptr, TO_ROOM);
	act("You wield $p{x in your off-hand.", ch, obj, nullptr, TO_CHAR);
	equip_char(ch, obj, WEAR_SECONDARY);
} /* end do_second() */

void get_obj(Character *ch, Object *obj, Object *container)
{
	/* variables for AUTOSPLIT */
	Character *gch;
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
		act("$d: you can't carry that many items.", ch, nullptr, obj->name, TO_CHAR);
		return;
	}

	if (get_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch)) {
		act("$d: you can't carry that much weight.", ch, nullptr, obj->name, TO_CHAR);
		return;
	}

	if (!can_loot(ch, obj)) {
		act("HEY!  No corpse looting ya scavenger!", ch, nullptr, nullptr, TO_CHAR);
		return;
	}

	if (obj->in_room != nullptr) {
		for (gch = obj->in_room->people; gch != nullptr; gch = gch->next_in_room)
			if (gch->on == obj) {
				act("$N appears to be using $p.", ch, obj, gch, TO_CHAR);
				return;
			}
	}

	if (container != nullptr) {
		if (container == Game::world().donation_pit) {
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

		if (ch->act_flags.has(PLR_AUTOSPLIT)) {
			/* AUTOSPLIT code */
			members = 0;

			for (gch = ch->in_room->people; gch != nullptr; gch = gch->next_in_room)
				if (is_same_group(gch, ch)
				    && !IS_NPC(gch))       /* don't split with mobs :P */
					members++;

			if (members > 1 && (obj->value[0] > 1 || obj->value[1])) {
				Format::sprintf(buffer, "%d %d", obj->value[0], obj->value[1]);
				do_split(ch, buffer);
			}
		}

		extract_obj(obj);
	}
	else {
		obj_to_char(obj, ch);

		if (!IS_NPC(ch)) {
			/* Did they pick up their quest item? */
			if (IS_QUESTOR(ch)) {
				if (ch->pcdata->questobj == obj->pIndexData->vnum && ch->pcdata->questobf != -1) {
					char buf[MAX_STRING_LENGTH];
					stc("{YYou have almost completed your QUEST!{x\n", ch);
					stc("{YReturn to the questmaster before your time runs out!{x\n", ch);
					ch->pcdata->questobf = -1;
					Format::sprintf(buf, "{Y:QUEST: {x$N has found %s", obj->short_descr);
					wiznet(buf, ch, nullptr, WIZ_QUEST, 0, 0);
				}
			}

			/* or skill quest item? */
			if (IS_SQUESTOR(ch)) {
				if (ch->pcdata->squestobj == obj && !ch->pcdata->squestobjf) {
					char buf[MAX_STRING_LENGTH];

					if (ch->pcdata->squestmob == nullptr) {
						stc("{YYou have almost completed your {VSKILL QUEST!{x\n", ch);
						stc("{YReturn to the questmistress before your time runs out!{x\n", ch);
					}
					else {
						stc("{YYou have completed part of your {VSKILL QUEST!{x\n", ch);
						Format::sprintf(buf, "{YTake the artifact to %s while there is still time!{x\n",
						        ch->pcdata->squestmob->short_descr);
						stc(buf, ch);
					}

					ch->pcdata->squestobjf = TRUE;
					Format::sprintf(buf, "{Y:SKILL QUEST: {x$N has found the %s", obj->short_descr);
					wiznet(buf, ch, nullptr, WIZ_QUEST, 0, 0);
				}
			}
		}
	}

	return;
}

bool from_box_ok(Character *ch, Object *obj, char *box_type)
{
	if (obj == nullptr) {
		ptc(ch, "You do not see that in your %s.\n", box_type);
		return FALSE;
	}

	if (!CAN_WEAR(obj, ITEM_TAKE)) {
		stc("You can't take that!\n", ch);
		Logging::bugf("Item without ITEM_TAKE in %s's %s.", ch->name, box_type);
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

void do_touch(Character *ch, String argument) {
	if (argument.empty()) {
		stc("Touch what?\n", ch);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	Object *obj = get_obj_here(ch, arg);

	if (!obj) {
		stc("You don't see that here.\n", ch);
		return;
	}

	if (obj->item_type == ITEM_WARP_CRYSTAL) {
		if (IS_NPC(ch)) {
			stc("The gods have forbidden your kind from touching that.\n", ch);
			return;
		}

		act("You place your hand upon the $p.", ch, obj, nullptr, TO_CHAR);
		act("$n places $s hand upon the $p.", ch, obj, nullptr, TO_ROOM);

		String loc;

		for (ExtraDescr *ed = obj->pIndexData->extra_descr; ed; ed = ed->next)
			if (ed->keyword == "warp_loc")
				loc = ed->description;

		if (loc.empty()) {
			Logging::bug("warp crystal with no location extra description", 0);
			act("The $p seems to be inert.", ch, obj, nullptr, TO_CHAR);
			return;
		}

		if (ch->pcdata->warp_locs.find(loc) != ch->pcdata->warp_locs.end()) {
			act("You have touched this $p already... nothing happens.", ch, obj, nullptr, TO_CHAR);
			return;
		}

		act("Tendrils of blue-white energy momentarily swirl around your wrist.\n"
			"When they subside, you feel a strange connection to the $p.",
			ch, obj, nullptr, TO_CHAR);
		act("Tendrils of blue-white energy momentarily swirl around $n's wrist.",
			ch, obj, nullptr, TO_ROOM);

		ch->pcdata->warp_locs.emplace(loc);
		return;
	}

	stc("You touch it, but nothing happens.\n", ch);
}

void do_get(Character *ch, String argument)
{
	Object *obj, *obj_next, *container;
	bool found;

	/* Get type. */
	if (argument.empty()) {
		stc("Get what?\n", ch);
		return;
	}

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg2 == "from")
		argument = one_argument(argument, arg2);

	if (arg2.empty()) {
		if (arg1 != "all" && !arg1.has_prefix("all.")) {
			/* 'get obj' */
			obj = get_obj_list(ch, arg1, ch->in_room->contents);

			if (obj == nullptr) {
				ptc(ch, "You don't see a %s here.\n", arg1);
				return;
			}

			get_obj(ch, obj, nullptr);
		}
		else {
			/* 'get all' or 'get all.obj' */
			found = FALSE;

			for (obj = ch->in_room->contents; obj != nullptr; obj = obj_next) {
				obj_next = obj->next_content;

				if ((arg1.length() == 3 // 'all '
				 || obj->name.has_words(arg1.substr(4)))
				  && can_see_obj(ch, obj)) {
					found = TRUE;
					get_obj(ch, obj, nullptr);
				}
			}

			if (!found) {
				if (arg1.length() == 3)
					stc("Get what?\n", ch);
				else
					ptc(ch, "You see no %s here.\n", &arg1[4]);
			}
		}
	}
	else {
		/* 'get ... container' */
		if (arg2 == "all" || arg2.has_prefix("all.")) {
			stc("You can't do that.\n", ch);
			return;
		}

		if (arg2.is_prefix_of("locker") && !IS_NPC(ch)) {
			if (ch->in_room->flags().has(ROOM_LOCKER)) {
				if (ch->act_flags.has(PLR_CLOSED)) {
					int number = get_locker_number(ch);

					if (deduct_cost(ch, number * 10)) {
						ptc(ch, "%d silver has been deducted for your locker.\n",
						    number * 10);
						ch->act_flags -= PLR_CLOSED;
					}
					else {
						stc("Your locker is closed.\n", ch);
						return;
					}
				}

				if (arg1 != "all" && !arg1.has_prefix("all.")) {
					obj = get_obj_list(ch, arg1, ch->pcdata->locker);

					if (!from_box_ok(ch, obj, "locker"))
						return;

					obj_from_locker(obj);
					obj_to_char(obj, ch);
					act("$n gets $p from $s locker.", ch, obj, nullptr, TO_ROOM);
					act("You get $p from your locker.", ch, obj, nullptr, TO_CHAR);
				}
				else {
					found = FALSE;

					for (obj = ch->pcdata->locker; obj != nullptr; obj = obj_next) {
						obj_next = obj->next_content;

						if ((arg1.length() == 3
						 || obj->name.has_words(arg1.substr(4))) && can_see_obj(ch, obj)) {
							found = TRUE;

							if (!from_box_ok(ch, obj, "locker"))
								continue;

							obj_from_locker(obj);
							obj_to_char(obj, ch);
							act("$n gets $p from $s locker.", ch, obj, nullptr, TO_ROOM);
							act("You get $p from your locker.", ch, obj, nullptr, TO_CHAR);
						}
					}

					if (!found) {
						if (arg1.length() == 3)
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
		if (!IS_NPC(ch) && arg2.is_prefix_of("strongbox")) {
			if (!IS_HEROIC(ch)) {
				stc("Only heroes and former heroes have strongboxes.\n", ch);
				return;
			}

			if (!ch->in_room || ch->in_room->location != Location(Vnum(ROOM_VNUM_STRONGBOX))) {
				stc("You do not see your strongbox here.\n", ch);
				return;
			}

			if (arg1 != "all" && !arg1.has_prefix("all.")) {
				obj = get_obj_list(ch, arg1, ch->pcdata->strongbox);

				if (!from_box_ok(ch, obj, "strongbox"))
					return;

				obj_from_strongbox(obj);
				obj_to_char(obj, ch);
				act("$n gets $p from $s strongbox.", ch, obj, nullptr, TO_ROOM);
				act("You get $p from your strongbox.", ch, obj, nullptr, TO_CHAR);
				return;
			}

			/* get all or all.something from strongbox */
			found = FALSE;

			for (obj = ch->pcdata->strongbox; obj != nullptr; obj = obj_next) {
				obj_next = obj->next_content;

				if ((arg1.length() == 3 || obj->name.has_words(arg1.substr(4))) && can_see_obj(ch, obj)) {
					found = TRUE;

					if (!from_box_ok(ch, obj, "strongbox"))
						continue;

					obj_from_strongbox(obj);
					obj_to_char(obj, ch);
					act("$n gets $p from $s strongbox.", ch, obj, nullptr, TO_ROOM);
					act("You get $p from your strongbox.", ch, obj, nullptr, TO_CHAR);
				}
			}

			if (!found) {
				if (arg1.length() == 3)
					stc("You see nothing in your strongbox.\n", ch);
				else
					stc("You see nothing like that in your strongbox.\n", ch);
			}

			return;
		}

		if ((container = get_obj_here(ch, arg2)) == nullptr) {
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

		if (container->value[1].flags().has(CONT_CLOSED) && !IS_OBJ_STAT(container, ITEM_COMPARTMENT)) {
			ptc(ch, "The %s is closed.\n", container->short_descr);
			return;
		}

		if (arg1 != "all" && !arg1.has_prefix("all.")) {
			/* 'get obj container' */
			obj = get_obj_list(ch, arg1, container->contains);

			if (obj == nullptr) {
				ptc(ch, "You see nothing like that in %s.\n", container->short_descr);
				return;
			}

			get_obj(ch, obj, container);
		}
		else {
			/* 'get all container' or 'get all.obj container' */
			found = FALSE;

			for (obj = container->contains; obj != nullptr; obj = obj_next) {
				obj_next = obj->next_content;

				if ((arg1.length() == 3 || obj->name.has_words(arg1.substr(4))) && can_see_obj(ch, obj)) {
					found = TRUE;

					if (container == Game::world().donation_pit && !IS_IMMORTAL(ch)) {
						stc("Don't be so greedy!\n", ch);
						return;
					}

					get_obj(ch, obj, container);
				}
			}

			if (!found) {
				if (arg1.length() == 3)
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
bool will_fit(Object *obj, Object *container)
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

void do_put(Character *ch, String argument)
{
	Object *container, *obj, *obj_next;
	int weight;
	bool found = FALSE, tooheavy = TRUE;

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg2 == "in" || arg2 == "on")
		argument = one_argument(argument, arg2);

	if (arg1.empty() || arg2.empty()) {
		stc("Put what in what?\n", ch);
		return;
	}

	if (arg2 == "all" || arg2.has_prefix("all.")) {
		stc("You can't do that.\n", ch);
		return;
	}

	/* locker stuff */
	if (!IS_NPC(ch) && arg2.is_prefix_of("locker")) {
		if (!ch->in_room->flags().has(ROOM_LOCKER)) {
			stc("You do not see a locker in this room.\n", ch);
			return;
		}

		if (arg1 != "all" && !arg1.has_prefix("all.")) {
			/* 'put obj locker' */
			if ((obj = get_obj_carry(ch, arg1)) == nullptr) {
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
			act("$n puts $p in $s locker.", ch, obj, nullptr, TO_ROOM);
			act("You put $p in your locker.", ch, obj, nullptr, TO_CHAR);
			return;
		}

		/* 'put all locker' or 'put all.obj locker' */
		for (obj = ch->carrying; obj != nullptr; obj = obj_next) {
			obj_next = obj->next_content;
			weight = get_obj_weight(obj);

			if ((arg1.length() == 3 || obj->name.has_words(arg1.substr(4)))
			    && can_see_obj(ch, obj)
			    && obj->wear_loc == WEAR_NONE
			    && can_drop_obj(ch, obj)
			    && weight + get_locker_weight(ch) <= 10000
			    && weight <= 1000
			    && get_obj_number(obj) + get_locker_number(ch) <= 250) {
				obj_from_char(obj);
				obj_to_locker(obj, ch);
				act("$n puts $p in $s locker.", ch, obj, nullptr, TO_ROOM);
				act("You put $p in your locker.", ch, obj, nullptr, TO_CHAR);
				found = TRUE;
			}
		}

		if (!found)
			stc("You do not have those.\n", ch);

		return;
	}

	/* strongbox stuff -- Elrac */
	if (!IS_NPC(ch) && arg2.is_prefix_of("strongbox")) {
		if (!IS_HEROIC(ch)) {
			stc("Only heroes and former heroes have strongboxes.\n", ch);
			return;
		}

		if (!ch->in_room || ch->in_room->location != Location(Vnum(ROOM_VNUM_STRONGBOX))) {
			stc("There is no strongbox here.\n", ch);
			return;
		}

		if (arg1 != "all" && !arg1.has_prefix("all.")) {
			/* 'put obj strongbox' */
			if ((obj = get_obj_carry(ch, arg1)) == nullptr) {
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
			act("$n puts $p in $s strongbox.", ch, obj, nullptr, TO_ROOM);
			act("You put $p in your strongbox.", ch, obj, nullptr, TO_CHAR);
			return;
		}

		/* 'put all strongbox' or 'put all.obj strongbox' */
		for (obj = ch->carrying; obj != nullptr; obj = obj_next) {
			obj_next = obj->next_content;

			if ((arg1.length() == 3 || obj->name.has_words(arg1.substr(4)))
			    && can_see_obj(ch, obj)
			    && obj->wear_loc == WEAR_NONE
			    && can_drop_obj(ch, obj)) {
//			    && get_obj_number(obj) + get_strongbox_number(ch) <= 45 + ch->pcdata->remort_count) {
				obj_from_char(obj);
				obj_to_strongbox(obj, ch);
				act("$n puts $p in $s strongbox.", ch, obj, nullptr, TO_ROOM);
				act("You put $p in your strongbox.", ch, obj, nullptr, TO_CHAR);
				found = TRUE;
			}
		}

		if (!found)
			stc("You do not have those.\n", ch);

		return;
	}

	if ((container = get_obj_here(ch, arg2)) == nullptr) {
		act("There's no $T here.", ch, nullptr, arg2, TO_CHAR);
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

	if (container->value[1].flags().has(CONT_CLOSED)
	    && !IS_OBJ_STAT(container, ITEM_COMPARTMENT)) {
		act("The $d is closed.", ch, nullptr, container->name, TO_CHAR);
		return;
	}

	if (arg1 != "all" && !arg1.has_prefix("all.")) {
		/* 'put obj container' */
		if ((obj = get_obj_carry(ch, arg1)) == nullptr) {
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

		if (container->item_type == ITEM_CONTAINER && container->value[1].flags().has(CONT_PUT_ON)) {
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
	for (obj = ch->carrying; obj != nullptr; obj = obj_next) {
		obj_next = obj->next_content;

		if ((arg1.length() == 3 || obj->name.has_words(arg1.substr(4)))
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

			if (container->item_type == ITEM_CONTAINER && container->value[1].flags().has(CONT_PUT_ON)) {
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


void do_drop(Character *ch, String argument)
{
	Object *obj;
	Object *obj_next;
	bool found;
	int number;

	if (argument.empty()) {
		stc("You attempt to drop it like it's hot.\n", ch);
		return;
	}

	String arg, obj_name;
	argument = one_argument(argument, arg);

	if (arg.is_number()) {
		/* 'drop NNNN coins' */
		int amount, gold = 0, silver = 0;
		amount   = atoi(arg);
		argument = one_argument(argument, arg);

		if (amount <= 0
		    || (!arg.is_prefix_of("coins") && !arg.is_prefix_of("coin") &&
		        !arg.is_prefix_of("gold") && !arg.is_prefix_of("silver"))) {
			stc("Sorry, you can't do that.\n", ch);
			return;
		}

		if (arg.is_prefix_of("coins") || arg.is_prefix_of("coin")
		    ||   arg.is_prefix_of("silver")) {
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
		for (obj = ch->in_room->contents; obj != nullptr; obj = obj_next) {
			obj_next = obj->next_content;

			if (obj->pIndexData->vnum == GEN_OBJ_MONEY
			    && obj->name.has_words("gcash")) {
				silver  += obj->value[0];
				gold    += obj->value[1];
				extract_obj(obj);
			}
		}

		act("$n lets some coins tumble to the floor.", ch, nullptr, nullptr, TO_ROOM);
		stc("You flip some coins to the ground.\n", ch);
		obj_to_room(create_money(gold, silver), ch->in_room);
		return;
	}

	if (arg == "all" || arg.has_prefix("all.")) {
		/* 'drop all' or 'drop all.obj' */
		found = FALSE;

		for (obj = ch->carrying; obj != nullptr; obj = obj_next) {
			obj_next = obj->next_content;

			if ((arg.length() == 3 || obj->name.has_words(arg.substr(4)))
			    &&   can_see_obj(ch, obj)
			    &&   obj->wear_loc == WEAR_NONE) {
				found = TRUE;

				if (can_drop_obj(ch, obj)) {
					obj_from_char(obj);
					act("$n drops $p.", ch, obj, nullptr, TO_ROOM);
					act("You drop $p.", ch, obj, nullptr, TO_CHAR);
					obj_to_room(obj, ch->in_room);

					if (IS_OBJ_STAT(obj, ITEM_MELT_DROP)) {
						act("$p crumbles into dust.", ch, obj, nullptr, TO_ROOM);
						act("$p crumbles into dust.", ch, obj, nullptr, TO_CHAR);
						destroy_obj(obj);
					}
				}
				else
					act("You can't seem to let go of $p.", ch, obj, nullptr, TO_CHAR);
			}
		}

		if (!found) {
			if (arg.length() == 3)
				act("You are not carrying anything.",
				    ch, nullptr, arg, TO_CHAR);
			else
				act("You do not have a $T.",
				    ch, nullptr, &arg[4], TO_CHAR);
		}

		return;
	}

	/* 'drop [n*]obj' */
	number = mult_argument(arg, obj_name);

	if (number == -1) {
		Format::sprintf(arg, "You patiently wait for something called "
		        "'%s' to drop on you from the sky.\n", obj_name);
		stc(arg, ch);
		return;
	}

	if (number < 0) {
		Format::sprintf(arg, "You patiently wait for %d things called "
		        "'%s' to drop on you from the sky.\n", -number, obj_name);
		stc(arg, ch);
		return;
	}

	if ((obj = get_obj_carry(ch, obj_name)) == nullptr) {
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
		act("$n drops $p.", ch, obj, nullptr, TO_ROOM);
		act("You drop $p.", ch, obj, nullptr, TO_CHAR);
		obj_to_room(obj, ch->in_room);

		if (IS_OBJ_STAT(obj, ITEM_MELT_DROP)) {
			act("$p dissolves into smoke.", ch, obj, nullptr, TO_ROOM);
			act("$p dissolves into smoke.", ch, obj, nullptr, TO_CHAR);
			destroy_obj(obj);
		}

		return;
	}
	else {
		Object *op, *obj_next;
		int count = 1;
		/* drop <number> items, where <number> > 1. */
		/* obtain vnum for the first object and count how many more of the
		   same vnum the character has available and is able to drop. */
		const Vnum& obj_vnum = obj->pIndexData->vnum;

		for (op = obj->next_content; op; op = op->next_content) {
			if (op->pIndexData->vnum != obj_vnum)
				continue;

			count++;

			if (!can_drop_obj(ch, op)) {
				act("The $T one sticks to you, preventing "
				    "you from dropping that many.\n",
				    ch, nullptr, ordinal_string(count), TO_CHAR);
				return;
			}

			if (count >= number)
				break;
		}

		Format::sprintf(arg, "%d", count);

		if (count < number) {
			act("But you only have $T of those!", ch, nullptr, arg, TO_CHAR);
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
				destroy_obj(op);
			else
				obj_to_room(op, ch->in_room);

			if (count >= number)
				break;
		}
	}
} /* end do_drop() */

void do_give(Character *ch, String argument)
{
	Character *victim;
	Object *obj, *op, *obj_next;
	int number, count = 0, item_number = 0, item_weight = 0;

	String arg1, arg2, buf;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1.empty() || arg2.empty()) {
		stc("Give what to whom?\n", ch);
		return;
	}

	if (arg1.is_number()) {
		/* 'give NNNN coins victim' */
		int amount;
		bool silver;
		amount = atoi(arg1);

		if (amount <= 0) {
			stc("You try to hand out imaginary money.  Cheapskate.\n", ch);
			return;
		}

		if (arg2 != "coins"
		    && arg2 != "coin"
		    && arg2 != "gold"
		    && arg2 != "silver") {
			stc("Use give <number> <gold|silver> <person>.\n", ch);
			return;
		}

		silver = arg2 != "gold";
		argument = one_argument(argument, arg2);

		if (arg2.empty()) {
			stc("Give the money to whom?\n", ch);
			return;
		}

		if ((victim = get_char_here(ch, arg2, VIS_CHAR)) == nullptr) {
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
			if (!IS_IMMORTAL(ch) && victim->pcdata->plr_flags.has(PLR_LINK_DEAD)) {
				Format::sprintf(buf, "$N is trying to give an object to the linkdead character %s.", victim->name);
				wiznet(buf, ch, nullptr, WIZ_CHEAT, 0, GET_RANK(ch));
				stc("Your recipient cannot receive objects in their current state.\n", ch);
				return;
			}

			if ((!silver && (get_carry_weight(victim) + gold_weight(amount) > can_carry_w(victim)))
			    || (silver && (get_carry_weight(victim) + silver_weight(amount) > can_carry_w(victim)))) {
				stc("They can't carry that much weight.", ch);
				return;
			}
		}

		if (IS_NPC(victim) && victim->act_flags.has(ACT_IS_CHANGER)) {
			int change;
			change = (silver ? 95 * amount / 10000 : 95 * amount);

			if (IS_NPC(ch) && ch->act_flags.has(ACT_IS_CHANGER)) {
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
			Format::sprintf(buf, "$n gives you one %s coin.", silver ? "silver" : "gold");
		else
			Format::sprintf(buf, "$n gives you %d %s.", amount, silver ? "silver" : "gold");

		act(buf, ch, nullptr, victim, TO_VICT);
		Format::sprintf(buf, "$n gives $N %s %s coin%s.  Must be nice to be rich.",
		        amount == 1 ? "one" : "some",
		        silver ? "silver" : "gold",
		        amount == 1 ? "" : "s");
		act(buf, ch, nullptr, victim, TO_NOTVICT);

		if (amount == 1)
			Format::sprintf(buf, "You give one %s coin to $N.", silver ? "silver" : "gold");
		else
			Format::sprintf(buf, "You give %d %s to $N.", amount, silver ? "silver" : "gold");

		act(buf, ch, nullptr, victim, TO_CHAR);
		mprog_bribe_trigger(victim, ch, silver ? amount : amount * 100);

		if (IS_NPC(victim) && victim->act_flags.has(ACT_IS_CHANGER)) {
			int change;
			change = (silver ? 95 * amount / 10000 : 95 * amount);

			if (!silver && change > victim->silver)
				victim->silver += change;

			if (silver && change > victim->gold)
				victim->gold += change;

			if (!can_see_char(victim, ch)) {
				act("$n looks around for whoever could be so generous.", victim, nullptr, nullptr, TO_ROOM);
				return;
			}

			if (change < 1) {
				act("$n tells you 'I'm sorry, you did not give me enough to change.'", victim, nullptr, ch, TO_VICT);
				ch->reply = victim->name;
				Format::sprintf(buf, "%d %s %s", amount, silver ? "silver" : "gold", ch->name);
				do_give(victim, buf);
				return;
			}

			Format::sprintf(buf, "%d %s %s", change, silver ? "gold" : "silver", ch->name);
			do_give(victim, buf);

			if (silver) {
				Format::sprintf(buf, "%d silver %s", (95 * amount / 100 - change * 100), ch->name);
				do_give(victim, buf);
			}

			act("$n tells you 'What do you think I am?  Made of money?!'", victim, nullptr, ch, TO_VICT);
			ch->reply = victim->name;
		}

		return;
	}

	/* 'normal' give [n*]<object> <victim> */
	number = mult_argument(arg1, buf);
	arg1 = buf;

	if (number < 0) {
		stc("If you want more than you had, try stealing!\n", ch);
		return;
	}

	if (number == 0) {
		stc("You give away 0 items, ending up with what you had before.\n", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, arg1)) == nullptr) {
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

	if ((victim = get_char_here(ch, arg2, VIS_CHAR)) == nullptr) {
		stc("They are not here.\n", ch);
		return;
	}

	if (victim == ch) {
		stc("Indignant, you tell yourself that you don't need any help.\n", ch);
		return;
	}

	if (!IS_NPC(victim) && !IS_IMMORTAL(ch) && victim->pcdata->plr_flags.has(PLR_LINK_DEAD)) {
		Format::sprintf(buf, "$N is trying to give an object to the linkdead character %s.", victim->name);
		wiznet(buf, ch, nullptr, WIZ_CHEAT, 0, GET_RANK(ch));
		stc("Your recipient cannot receive objects in their current state.\n", ch);
		return;
	}

	if (IS_NPC(victim) && victim->pIndexData->pShop != nullptr) {
		act("$N tells you 'Sorry, you'll have to sell that.'", ch, nullptr, victim, TO_CHAR);
		ch->reply = victim->name;
		return;
	}

	if (!can_drop_obj(ch, obj)) {
		stc("You can't let go of it.\n", ch);
		return;
	}

	if (!can_see_obj(victim, obj)) {
		/* hack so questmobs can take questobjs, easier than a bunch of reverse lookups in can_see_obj */
		if (!(!IS_NPC(ch) && IS_NPC(victim)
		      && IS_SQUESTOR(ch) && ch->pcdata->squestobj != nullptr && ch->pcdata->squestmob != nullptr
		      && obj == ch->pcdata->squestobj && victim == ch->pcdata->squestmob)) {
			act("$N can't see it.", ch, nullptr, victim, TO_CHAR);
			return;
		}
	}

	if (get_carry_number(victim) + item_number > can_carry_n(victim)) {
		act("$N is already carrying too much.", ch, nullptr, victim, TO_CHAR);
		return;
	}

	if (get_carry_weight(victim) + item_weight > can_carry_w(victim)) {
		act("$N buckles under that much weight.", ch, nullptr, victim, TO_CHAR);
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
		Format::sprintf(buf, "$n gives $p[%d] to $N.", number);
		act(buf, ch, obj, victim, TO_NOTVICT);
		Format::sprintf(buf, "$n gives you $p[%d].", number);
		act(buf, ch, obj, victim, TO_VICT);
		Format::sprintf(buf, "You give $p[%d] to $N.", number);
		act(buf, ch, obj, victim, TO_CHAR);
	}

	if (IS_SQUESTOR(ch)
	    && ch->pcdata->squestmob != nullptr && ch->pcdata->squestobj != nullptr) {
		if (obj == ch->pcdata->squestobj && victim == ch->pcdata->squestmob) {
			extern void squestobj_to_squestmob args((Character * ch, Object * obj, Character * mob));

			if (!ch->pcdata->squestobjf) {
				Logging::bug("At give sqobj to sqmob without sqobj found, continuing...", 0);
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
void do_envenom(Character *ch, String argument)
{
	Object *obj;
	int percent, skill;

	/* find out what */
	if (argument.empty()) {
		stc("Envenom what item?\n", ch);
		return;
	}

	obj =  get_obj_list(ch, argument, ch->carrying);

	if (obj == nullptr) {
		stc("You do not have that item.\n", ch);
		return;
	}

	if ((skill = get_skill_level(ch, skill::type::envenom)) < 1) {
		stc("Are you crazy? You'd poison yourself!\n", ch);
		return;
	}

	if (!deduct_stamina(ch, skill::type::envenom))
		return;

	if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON) {
		if (IS_OBJ_STAT(obj, ITEM_BLESS) || IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
			act("You fail to poison $p.", ch, obj, nullptr, TO_CHAR);
			return;
		}

		if (number_percent() < skill) { /* success! */
			act("$n treats $p with deadly poison.", ch, obj, nullptr, TO_ROOM);
			act("You treat $p with deadly poison.", ch, obj, nullptr, TO_CHAR);

			if (!obj->value[3]) {
				obj->value[3] = 1;
				check_improve(ch, skill::type::envenom, TRUE, 4);
			}

			WAIT_STATE(ch, skill::lookup(skill::type::envenom).beats);
			return;
		}

		act("You fail to poison $p.", ch, obj, nullptr, TO_CHAR);

		if (!obj->value[3])
			check_improve(ch, skill::type::envenom, FALSE, 4);

		WAIT_STATE(ch, skill::lookup(skill::type::envenom).beats);
		return;
	}

	if (obj->item_type == ITEM_WEAPON) {
		if (obj->value[3] < 0
		    ||  attack_table[obj->value[3]].damage == DAM_BASH) {
			stc("You can only envenom edged weapons.\n", ch);
			return;
		}

		if (affect::exists_on_obj(obj, affect::type::poison)) {
			act("$p is already envenomed.", ch, obj, nullptr, TO_CHAR);
			return;
		}

		percent = number_percent();

		if (percent < skill) {
			affect::Affect af;
			af.where     = TO_WEAPON;
			af.type      = affect::type::poison;
			af.level     = ch->level;
			af.duration  = ch->level * 5;
			af.location  = 0;
			af.modifier  = 0;
			af.bitvector(0);
			af.evolution = get_evolution(ch, skill::type::envenom);
			affect::copy_to_obj(obj, &af);
			act("$n coats $p with deadly venom.", ch, obj, nullptr, TO_ROOM);
			act("You coat $p with venom.", ch, obj, nullptr, TO_CHAR);
			check_improve(ch, skill::type::envenom, TRUE, 3);
			WAIT_STATE(ch, skill::lookup(skill::type::envenom).beats);
			return;
		}
		else {
			act("You fail to envenom $p.", ch, obj, nullptr, TO_CHAR);
			check_improve(ch, skill::type::envenom, FALSE, 3);
			WAIT_STATE(ch, skill::lookup(skill::type::envenom).beats);
			return;
		}
	}

	act("You can't poison $p.", ch, obj, nullptr, TO_CHAR);
	return;
}
/* Firebuilding by Lotus */
void do_firebuilding(Character *ch, String argument)
{
	Object *torch;

	if (get_carry_number(ch) + 1 > can_carry_n(ch)) {
		act("You can't carry any more items.", ch, nullptr, nullptr, TO_CHAR);
		return;
	}

	if (get_carry_weight(ch) >= can_carry_w(ch)) {
		act("You can't carry any more weight.", ch, nullptr, nullptr, TO_CHAR);
		return;
	}

	if (!get_skill_level(ch, skill::type::firebuilding)) {
		stc("You lack the knowledge it takes to make a torch!\n", ch);
		return;
	}

	if (!deduct_stamina(ch, skill::type::firebuilding))
		return;

	if ((ch->in_room->sector_type() == Sector::city)
	    || (ch->in_room->sector_type() == Sector::water_swim)
	    || (ch->in_room->sector_type() == Sector::water_noswim)
	    || (ch->in_room->sector_type() == Sector::desert)
	    || (ch->in_room->sector_type() == Sector::air)) {
		stc("You cannot find any twigs to make a torch.\n", ch);
		return;
	}

	WAIT_STATE(ch, skill::lookup(skill::type::firebuilding).beats);

	if (number_percent() > get_skill_level(ch, skill::type::firebuilding)) {
		stc("You burn yourself.\n", ch);
		check_improve(ch, skill::type::firebuilding, FALSE, 8);
		return;
	}

	torch = create_object(Game::world().get_obj_prototype(OBJ_VNUM_TORCH), 0);

	if (! torch) {
		Logging::bug("Error creating a torch in firebuilding.", 0);
		stc("You were unable to make a torch.\n", ch);
		return;
	}

	torch->cost = 0;
	obj_to_char(torch, ch);
	act("$n gathers some twigs and creates $p.", ch, torch, nullptr, TO_ROOM);
	act("You gather some twigs and create $p.", ch, torch, nullptr, TO_CHAR);
	torch->value[2] = -1;
	check_improve(ch, skill::type::firebuilding, TRUE, 8);
	return;
}

void do_fill(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Object *obj;
	Object *first_fountain = nullptr;
	Object *wanted_fountain = nullptr;

	if (argument.empty()) {
		stc("Fill what?\n", ch);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	if ((obj = get_obj_carry(ch, arg)) == nullptr) {
		stc("You do not have that item.\n", ch);
		return;
	}

	if (obj->item_type != ITEM_DRINK_CON) {
		stc("You can't fill that.\n", ch);
		return;
	}

	for (first_fountain = ch->in_room->contents; first_fountain != nullptr;
	     first_fountain = first_fountain->next_content)
		if (first_fountain->item_type == ITEM_FOUNTAIN)
			break;

	if (first_fountain == nullptr) {
		stc("From what?\n", ch);
		return;
	}

	wanted_fountain = first_fountain;
	/* check for a user requested OTHER fountain -- Elrac */
	argument = one_argument(argument, arg);

	if (!arg.empty()) {
		if (arg.is_prefix_of("from"))
			argument = one_argument(argument, arg);

		if (!arg.empty()) {
			wanted_fountain = get_obj_here(ch, arg);

			if (wanted_fountain == nullptr) {
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

	Format::sprintf(buf, "You fill $p with %s from $P.",
	        liq_table[wanted_fountain->value[2]].name);
	act(buf, ch, obj, wanted_fountain, TO_CHAR);
	Format::sprintf(buf, "$n fills $p with %s from $P.",
	        liq_table[wanted_fountain->value[2]].name);
	act(buf, ch, obj, wanted_fountain, TO_ROOM);
	obj->value[2] = wanted_fountain->value[2];
	obj->value[1] = obj->value[0];
	obj->value[3] = wanted_fountain->value[3];      /* poison the drink */
} /* end do_fill() */

void do_pour(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Object *out, *in;
	Character *vch = nullptr;
	int amount;

	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty() || argument.empty()) {
		stc("Pour what into what?\n", ch);
		return;
	}

	if ((out = get_obj_carry(ch, arg)) == nullptr) {
		stc("You do not have that item.\n", ch);
		return;
	}

	if (out->item_type != ITEM_DRINK_CON) {
		stc("That's not a drink container.\n", ch);
		return;
	}

	if (argument == "out") {
		if (out->value[1] == 0) {
			stc("It's already empty.\n", ch);
			return;
		}

		out->value[1] = 0;
		out->value[3] = 0;      /* no more poison */
		Format::sprintf(buf, "You invert $p, spilling %s all over the ground.",
		        liq_table[out->value[2]].name);
		act(buf, ch, out, nullptr, TO_CHAR);
		Format::sprintf(buf, "$n inverts $p, spilling %s all over the ground.",
		        liq_table[out->value[2]].name);
		act(buf, ch, out, nullptr, TO_ROOM);
		return;
	}

	if ((in = get_obj_here(ch, argument)) == nullptr) {
		vch = get_char_here(ch, argument, VIS_CHAR);

		if (vch == nullptr) {
			stc("Pour into what?\n", ch);
			return;
		}

		in = get_eq_char(vch, WEAR_HOLD);

		if (in == nullptr) {
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
		act("There's nothing in $p to pour.", ch, out, nullptr, TO_CHAR);
		return;
	}

	if (in->value[1] >= in->value[0]) {
		act("$p is already filled to the top.", ch, in, nullptr, TO_CHAR);
		return;
	}

	amount = UMIN(out->value[1], in->value[0] - in->value[1]);
	in->value[1] += amount;
	out->value[1] -= amount;
	in->value[2] = out->value[2];
	in->value[3] = out->value[3];   /* transfer the poison */

	if (vch == nullptr) {
		Format::sprintf(buf, "You pour %s from $p into $P.",
		        liq_table[out->value[2]].name);
		act(buf, ch, out, in, TO_CHAR);
		Format::sprintf(buf, "$n pours %s from $p into $P.",
		        liq_table[out->value[2]].name);
		act(buf, ch, out, in, TO_ROOM);
	}
	else {
		Format::sprintf(buf, "You pour some %s for $N.",
		        liq_table[out->value[2]].name);
		act(buf, ch, nullptr, vch, TO_CHAR);
		Format::sprintf(buf, "$n pours you some %s.",
		        liq_table[out->value[2]].name);
		act(buf, ch, nullptr, vch, TO_VICT);
		Format::sprintf(buf, "$n pours some %s for $N.",
		        liq_table[out->value[2]].name);
		act(buf, ch, nullptr, vch, TO_NOTVICT);
	}
}

void do_drink(Character *ch, String argument)
{
	Object *obj;
	int amount, liquid, liqvalue;

	String arg;
	argument = one_argument(argument, arg);

	if (!arg.empty() && !argument.empty() && arg == "from")
		one_argument(argument, arg);

	if (arg.empty()) {
		for (obj = ch->in_room->contents; obj; obj = obj->next_content)
			if (obj->item_type == ITEM_FOUNTAIN)
				break;

		if (obj == nullptr)
			for (obj = ch->carrying; obj != nullptr; obj = obj->next_content)
				if (obj->item_type == ITEM_DRINK_CON)
					break;

		if (obj == nullptr) {
			stc("Drink from what?\n", ch);
			return;
		}
	}
	else {
		if ((obj = get_obj_here(ch, arg)) == nullptr) {
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
			Logging::bug("Do_drink: bad liquid number %d.", liquid);
			liquid = obj->value[2] = 0;
		}

		amount = liq_table[liquid].affect[4] * 3;
		break;

	case ITEM_DRINK_CON:
		if (obj->value[1] <= 0) {
			stc("Your container is empty.\n", ch);
			return;
		}

		if ((liquid = obj->value[2]) < 0) {
			Logging::bug("Do_drink: bad liquid number %d.", liquid);
			liquid = obj->value[2] = 0;
		}

		amount = liq_table[liquid].affect[4];
		amount = UMIN(amount, obj->value[1]);
		break;
	}

	if (!IS_NPC(ch) && !IS_IMMORTAL(ch)
	    && ch->pcdata->condition[COND_FULL] > 45) {
		stc("You're too full to drink more.\n", ch);
		return;
	}

	act("$n drinks $T from $p.", ch, obj, liq_table[liquid].name, TO_ROOM);
	act("You drink $T from $p.", ch, obj, liq_table[liquid].name, TO_CHAR);
	gain_condition(ch, COND_DRUNK,  amount * liq_table[liquid].affect[COND_DRUNK]  / 36);
	gain_condition(ch, COND_FULL,   amount * liq_table[liquid].affect[COND_FULL]   /  4);
	gain_condition(ch, COND_HUNGER, amount * liq_table[liquid].affect[COND_HUNGER] /  2);

	if (ch->race == 6 && liquid == liq_lookup("blood"))   /* vampires */
		liqvalue = 10;          /* same as water */
	else
		liqvalue = liq_table[liquid].affect[COND_THIRST];

	gain_condition(ch, COND_THIRST, amount * liqvalue / 10);

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
		stc("You're toasted. The room begins to spin!\n", ch);

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL]  > 40)
		stc("You are full.\n", ch);

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 40)
		stc("Your thirst is quenched.\n", ch);

	if (obj->value[3] != 0) {
		/* The drink was poisoned! */
		act("$n turns six shades of green and collapses.", ch, nullptr, nullptr, TO_ROOM);
		stc("You turn six shades of green and collapse.\n", ch);

		affect::add_type_to_char(ch,
			affect::type::poison,
			number_fuzzy(amount),
			amount * 3,
			1,
			FALSE
		);
	}

	if (obj->value[0] > 0) {
		obj->value[1] = UMAX((int)obj->value[1] - amount, 0);

		/* if container is now empty, remove the poison */
		if (obj->value[1] == 0)
			obj->value[3] = 0;
	}
}

void do_eat(Character *ch, String argument)
{
	Object *obj, *op, *obj_next;
	int number, count;
	bool fFull = FALSE, fNoLongerHungry = FALSE, fPoisoned = FALSE, found = FALSE;
	Object *to_extract = nullptr;

	String buf, arg;
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

	if (arg.empty()) {
		for (obj = ch->carrying; obj != nullptr; obj = obj->next_content) {
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
	else if ((obj = get_obj_carry(ch, arg)) == nullptr) {
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

	Format::sprintf(buf, "%d", count);

	if (count < number) {
		act("But you only have $T of those!", ch, nullptr, buf, TO_CHAR);
		return;
	}

	if (number > 10) {
		stc("You couldn't possibly fit more than ten of those in your mouth.\n", ch);
		return;
	}

	/* They have enough. Loop thru <number> items with that vnum and gobble. */
	const Vnum& obj_vnum = obj->pIndexData->vnum;
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

			act("$n eats $p.", ch, op, nullptr, TO_ROOM);
			act("You eat $p.", ch, op, nullptr, TO_CHAR);
			obj_cast_spell(skill::from_int(op->value[1]), op->value[0], ch, ch, nullptr);
			obj_cast_spell(skill::from_int(op->value[2]), op->value[0], ch, ch, nullptr);
			obj_cast_spell(skill::from_int(op->value[3]), op->value[0], ch, ch, nullptr);
			obj_cast_spell(skill::from_int(op->value[4]), op->value[0], ch, ch, nullptr);
		}
		else if (op->item_type == ITEM_FOOD) {
			if (ch->level < op->level && !IS_IMMORTAL(ch)) {
				stc("This food is too rich for you to eat.\n", ch);
				return;
			}

			if (op->value[3] != 0) {
				/* The food was poisoned! */
				affect::add_type_to_char(ch,
					affect::type::poison,
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
		if (to_extract != nullptr)
			extract_obj(to_extract);

		to_extract = op;

		if (++count >= number)
			break;
	} /* end for */

	if (count == 1)
		buf.erase();
	else
		Format::sprintf(buf, "[%d]", count);

	if (obj->item_type != ITEM_PILL) {
		act("$n eats $p$T.", ch, to_extract, buf, TO_ROOM);
		act("You eat $p$T.", ch, to_extract, buf, TO_CHAR);
	}

	if (fNoLongerHungry)
		stc("You are no longer hungry.\n", ch);

	if (fFull && !IS_IMMORTAL(ch))
		stc("You are full.\n", ch);

	if (fPoisoned) {
		act("$n turns six shades of green and collapses.", ch, nullptr, nullptr, TO_ROOM);
		stc("You turn six shades of green and collapse.\n", ch);
	}

	if (to_extract != nullptr)
		extract_obj(to_extract);
} /* end do_eat() */

/*
 * Remove an object.
 */
bool remove_obj(Character *ch, int iWear, bool fReplace)
{
	Object *obj;

	if ((obj = get_eq_char(ch, iWear)) == nullptr)
		return TRUE;

	if (!fReplace)
		return FALSE;

	if (IS_OBJ_STAT(obj, ITEM_NOREMOVE)) {
		act("You can't seem to remove $p.", ch, obj, nullptr, TO_CHAR);
		return FALSE;
	}

	unequip_char(ch, obj);
	act("$n stops using $p.", ch, obj, nullptr, TO_ROOM);
	act("You stop using $p.", ch, obj, nullptr, TO_CHAR);
	return TRUE;
}

/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj(Character *ch, Object *obj, bool fReplace)
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
				act("$n tries to use $p, but is too inexperienced.", ch, obj, nullptr, TO_ROOM);
				return;
			}
			else {
				stc("This item must be lowered for mortal use.\n", ch);
				act("$n tries to use $p, but is too bound to the mortal plane.", ch, obj, nullptr, TO_ROOM);
				return;
			}
		}
	}

	if (!IS_IMMORTAL(ch)) {
		if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch))
	    || (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch))
	    || (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
			act("You are zapped by $p and drop it.", ch, obj, nullptr, TO_CHAR);
			act("$n is zapped by $p and drops it.",  ch, obj, nullptr, TO_ROOM);
			obj_from_char(obj);
			obj_to_room(obj, ch->in_room);
			return;
			}
	}

	if (obj->item_type == ITEM_WEDDINGRING) {
		if (!remove_obj(ch, WEAR_WEDDINGRING, fReplace))
			return;

		act("$n wears $p on $s ringfinger.", ch, obj, nullptr , TO_ROOM);
		act("You wear $p on your ringfinger.", ch, obj, nullptr, TO_CHAR);
		equip_char(ch, obj, WEAR_WEDDINGRING);
		return;
	}

	if (obj->item_type == ITEM_LIGHT) {
		if (!remove_obj(ch, WEAR_LIGHT, fReplace))
			return;

		act("$n lights $p{x and holds it.", ch, obj, nullptr, TO_ROOM);
		act("You light $p{x and hold it.",  ch, obj, nullptr, TO_CHAR);
		equip_char(ch, obj, WEAR_LIGHT);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_FINGER)) {
		if (get_eq_char(ch, WEAR_FINGER_L) != nullptr
		    &&   get_eq_char(ch, WEAR_FINGER_R) != nullptr
		    &&   !remove_obj(ch, WEAR_FINGER_L, fReplace)
		    &&   !remove_obj(ch, WEAR_FINGER_R, fReplace))
			return;

		if (get_eq_char(ch, WEAR_FINGER_L) == nullptr) {
			act("$n wears $p{x on $s left finger.",    ch, obj, nullptr, TO_ROOM);
			act("You wear $p{x on your left finger.",  ch, obj, nullptr, TO_CHAR);
			equip_char(ch, obj, WEAR_FINGER_L);
			return;
		}

		if (get_eq_char(ch, WEAR_FINGER_R) == nullptr) {
			act("$n wears $p{x on $s right finger.",   ch, obj, nullptr, TO_ROOM);
			act("You wear $p{x on your right finger.", ch, obj, nullptr, TO_CHAR);
			equip_char(ch, obj, WEAR_FINGER_R);
			return;
		}

		Logging::bug("Wear_obj: no free finger.", 0);
		stc("You already wear two rings.\n", ch);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_NECK)) {
		if (get_eq_char(ch, WEAR_NECK_1) != nullptr
		    &&   get_eq_char(ch, WEAR_NECK_2) != nullptr
		    &&   !remove_obj(ch, WEAR_NECK_1, fReplace)
		    &&   !remove_obj(ch, WEAR_NECK_2, fReplace))
			return;

		if (get_eq_char(ch, WEAR_NECK_1) == nullptr) {
			act("$n wears $p{x around $s neck.",   ch, obj, nullptr, TO_ROOM);
			act("You wear $p{x around your neck.", ch, obj, nullptr, TO_CHAR);
			equip_char(ch, obj, WEAR_NECK_1);
			return;
		}

		if (get_eq_char(ch, WEAR_NECK_2) == nullptr) {
			act("$n wears $p{x around $s neck.",   ch, obj, nullptr, TO_ROOM);
			act("You wear $p{x around your neck.", ch, obj, nullptr, TO_CHAR);
			equip_char(ch, obj, WEAR_NECK_2);
			return;
		}

		Logging::bug("Wear_obj: no free neck.", 0);
		stc("You already wear two neck items.\n", ch);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_BODY)) {
		if (!remove_obj(ch, WEAR_BODY, fReplace))
			return;

		act("$n wears $p{x on $s torso.",   ch, obj, nullptr, TO_ROOM);
		act("You wear $p{x on your torso.", ch, obj, nullptr, TO_CHAR);
		equip_char(ch, obj, WEAR_BODY);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_HEAD)) {
		if (!remove_obj(ch, WEAR_HEAD, fReplace))
			return;

		act("$n wears $p{x on $s head.",   ch, obj, nullptr, TO_ROOM);
		act("You wear $p{x on your head.", ch, obj, nullptr, TO_CHAR);
		equip_char(ch, obj, WEAR_HEAD);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_LEGS)) {
		if (!remove_obj(ch, WEAR_LEGS, fReplace))
			return;

		act("$n wears $p{x on $s legs.",   ch, obj, nullptr, TO_ROOM);
		act("You wear $p{x on your legs.", ch, obj, nullptr, TO_CHAR);
		equip_char(ch, obj, WEAR_LEGS);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_FEET)) {
		if (!remove_obj(ch, WEAR_FEET, fReplace))
			return;

		act("$n wears $p{x on $s feet.",   ch, obj, nullptr, TO_ROOM);
		act("You wear $p{x on your feet.", ch, obj, nullptr, TO_CHAR);
		equip_char(ch, obj, WEAR_FEET);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_HANDS)) {
		if (!remove_obj(ch, WEAR_HANDS, fReplace))
			return;

		act("$n wears $p{x on $s hands.",   ch, obj, nullptr, TO_ROOM);
		act("You wear $p{x on your hands.", ch, obj, nullptr, TO_CHAR);
		equip_char(ch, obj, WEAR_HANDS);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_ARMS)) {
		if (!remove_obj(ch, WEAR_ARMS, fReplace))
			return;

		act("$n wears $p{x on $s arms.",   ch, obj, nullptr, TO_ROOM);
		act("You wear $p{x on your arms.", ch, obj, nullptr, TO_CHAR);
		equip_char(ch, obj, WEAR_ARMS);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_ABOUT)) {
		if (!remove_obj(ch, WEAR_ABOUT, fReplace))
			return;

		act("$n wears $p{x about $s body.",   ch, obj, nullptr, TO_ROOM);
		act("You wear $p{x about your body.", ch, obj, nullptr, TO_CHAR);
		equip_char(ch, obj, WEAR_ABOUT);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_WAIST)) {
		if (!remove_obj(ch, WEAR_WAIST, fReplace))
			return;

		act("$n wears $p{x about $s waist.",   ch, obj, nullptr, TO_ROOM);
		act("You wear $p{x about your waist.", ch, obj, nullptr, TO_CHAR);
		equip_char(ch, obj, WEAR_WAIST);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_WRIST)) {
		if (get_eq_char(ch, WEAR_WRIST_L) != nullptr
		    &&   get_eq_char(ch, WEAR_WRIST_R) != nullptr
		    &&   !remove_obj(ch, WEAR_WRIST_L, fReplace)
		    &&   !remove_obj(ch, WEAR_WRIST_R, fReplace))
			return;

		if (get_eq_char(ch, WEAR_WRIST_L) == nullptr) {
			act("$n wears $p{x around $s left wrist.",
			    ch, obj, nullptr, TO_ROOM);
			act("You wear $p{x around your left wrist.",
			    ch, obj, nullptr, TO_CHAR);
			equip_char(ch, obj, WEAR_WRIST_L);
			return;
		}

		if (get_eq_char(ch, WEAR_WRIST_R) == nullptr) {
			act("$n wears $p{x around $s right wrist.",
			    ch, obj, nullptr, TO_ROOM);
			act("You wear $p{x around your right wrist.",
			    ch, obj, nullptr, TO_CHAR);
			equip_char(ch, obj, WEAR_WRIST_R);
			return;
		}

		Logging::bug("Wear_obj: no free wrist.", 0);
		stc("You already wear two wrist items.\n", ch);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_SHIELD)) {
		Object *weapon;

		if (get_eq_char(ch, WEAR_SECONDARY) != nullptr && !IS_IMMORTAL(ch)) {
			stc("You cannot use a shield while using two weapons.\n", ch);
			return;
		}

		if (!remove_obj(ch, WEAR_SHIELD, fReplace))
			return;

		weapon = get_eq_char(ch, WEAR_WIELD);

		if (weapon != nullptr && ch->size < SIZE_LARGE
		    && affect::exists_on_obj(weapon, affect::type::weapon_two_hands)) {
			stc("You need another hand free to do that.\n", ch);
			return;
		}

		act("$n wears $p{x as a shield.", ch, obj, nullptr, TO_ROOM);
		act("You wear $p{x as a shield.", ch, obj, nullptr, TO_CHAR);
		equip_char(ch, obj, WEAR_SHIELD);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WIELD)) {
		skill::type sn;
		int skill;

		if (!remove_obj(ch, WEAR_WIELD, fReplace))
			return;

		if (!IS_NPC(ch)
		    && get_obj_weight(obj) > (str_app[GET_ATTR_STR(ch)].wield
		                              * 10)) {
			stc("It's too heavy for you to pick up.\n", ch);
			return;
		}

		if (!IS_NPC(ch) && ch->size < SIZE_LARGE
		    &&  affect::exists_on_obj(obj, affect::type::weapon_two_hands)
		    &&  get_eq_char(ch, WEAR_SHIELD) != nullptr) {
			stc("You need two hands free for that weapon.\n", ch);
			return;
		}

		if (!IS_NPC(ch) && affect::exists_on_obj(obj, affect::type::weapon_two_hands) &&
		    get_eq_char(ch, WEAR_SECONDARY) != nullptr) {
			stc("You can not use a two handed weapon when dual-wielding.\n", ch);
			return;
		}

		act("$n wields $p{x.", ch, obj, nullptr, TO_ROOM);
		act("You wield $p{x.", ch, obj, nullptr, TO_CHAR);
		equip_char(ch, obj, WEAR_WIELD);
		sn = get_weapon_skill(ch, FALSE);

		if (sn == skill::type::hand_to_hand)
			return;

		skill = get_weapon_learned(ch, sn);

		if (skill >= 100)
			act("You feel at one with $p{x.", ch, obj, nullptr, TO_CHAR);
		else if (skill > 85)
			act("You feel quite confident with $p{x.", ch, obj, nullptr, TO_CHAR);
		else if (skill > 70)
			act("You are skilled with $p{x.", ch, obj, nullptr, TO_CHAR);
		else if (skill > 50)
			act("Your skill with $p is adequate{x.", ch, obj, nullptr, TO_CHAR);
		else if (skill > 25)
			act("$p feels a little clumsy in your hands{x.", ch, obj, nullptr, TO_CHAR);
		else if (skill > 1)
			act("You fumble and almost drop $p{x.", ch, obj, nullptr, TO_CHAR);
		else
			act("You don't even know which end is up on $p{x.",
			    ch, obj, nullptr, TO_CHAR);

		return;
	}

	if (CAN_WEAR(obj, ITEM_HOLD)) {
		if (!remove_obj(ch, WEAR_HOLD, fReplace))
			return;

		if (get_eq_char(ch, WEAR_SECONDARY) != nullptr && !IS_IMMORTAL(ch)) {
			stc("You cannot hold an item while using two weapons.\n", ch);
			return;
		}

		act("$n holds $p{x in $s hand.",   ch, obj, nullptr, TO_ROOM);
		act("You hold $p{x in your hand.", ch, obj, nullptr, TO_CHAR);
		equip_char(ch, obj, WEAR_HOLD);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_FLOAT)) {
		if (!remove_obj(ch, WEAR_FLOAT, fReplace))
			return;

		act("$n releases $p{x to float next to $m.", ch, obj, nullptr, TO_ROOM);
		act("You release $p{x and it floats next to you.", ch, obj, nullptr, TO_CHAR);
		equip_char(ch, obj, WEAR_FLOAT);
		return;
	}

	if (fReplace)
		stc("You can't wear, wield, or hold that.\n", ch);
} /* end wear_obj() */

void do_wear(Character *ch, String argument)
{
	Object *obj;

	if (argument.empty()) {
		stc("Wear, wield, or hold what?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg == "all") {
		Object *obj_next;

		for (obj = ch->carrying; obj != nullptr; obj = obj_next) {
			obj_next = obj->next_content;

			if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj))
				wear_obj(ch, obj, FALSE);
		}

		return;
	}
	else {
		if ((obj = get_obj_carry(ch, arg)) == nullptr) {
			stc("You do not have that item.\n", ch);
			return;
		}

		wear_obj(ch, obj, TRUE);
	}

	return;
}

void do_remove(Character *ch, String argument)
{
	Object *obj;

	if (argument.empty()) {
		stc("Remove what?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	/* Remove all...from miscellanous donation code */
	if (!strcmp(arg, "all")) {
		int x;

		for (x = 0; x < 20; x++)
			remove_obj(ch, x, TRUE);

		return;
	}

	if ((obj = get_obj_wear(ch, arg)) == nullptr) {
		stc("You do not have that item.\n", ch);
		return;
	}

	remove_obj(ch, obj->wear_loc, TRUE);
	return;
}

/* Donate by Lotus */
void do_donate(Character *ch, String argument)
{
	Object *item;

	/* try to find the pit */
	if (Game::world().donation_pit == nullptr)
		for (Game::world().donation_pit = Game::world().object_list; Game::world().donation_pit != nullptr; Game::world().donation_pit = Game::world().donation_pit->next)
			if (Game::world().donation_pit->pIndexData->vnum == OBJ_VNUM_PIT)
				break;

	/* can't find it?  make one */
	if (Game::world().donation_pit == nullptr) {
		Game::world().donation_pit = create_object(Game::world().get_obj_prototype(OBJ_VNUM_PIT), 0);

		if (! Game::world().donation_pit) {
			Logging::bug("Error creating donation pit in do_donate.", 0);
			stc("There is no donation pit.\n", ch);
			return;
		}

		obj_to_room(Game::world().donation_pit, Game::world().get_room(Location(Vnum(ROOM_VNUM_ALTAR))));
	}

	if (argument.empty()) {
		stc("Donate what?\n", ch);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	if ((item = get_obj_carry(ch, arg)) == nullptr) {
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

	act("$n donates $p to the donation pit.", ch, item, nullptr, TO_ROOM);
	act("You donate $p to the donation pit.", ch, item, nullptr, TO_CHAR);
	obj_from_char(item);
	obj_to_obj(item, Game::world().donation_pit);
}

/* Junk by Lotus */
void do_junk(Character *ch, String argument)
{
	Object *obj;

	if (argument.empty()) {
		stc("Junk what?\n", ch);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	if ((obj = get_obj_carry(ch, arg)) == nullptr) {
		stc("You do not have that item.\n", ch);
		return;
	}

	if (!can_drop_obj(ch, obj)) {
		stc("You can't seem to let go of it.\n", ch);
		return;
	}

	act("$n junks $p.", ch, obj, nullptr, TO_ROOM);
	act("You junk $p.", ch, obj, nullptr, TO_CHAR);
	extract_obj(obj);
	return;
}

bool acceptable_sac(Character *ch, Object *obj)
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
		ptc(ch, "%s is not an acceptable sacrifice.\n", obj->short_descr.capitalize());
		return FALSE;
	}

	return TRUE;
}

/* sacrifice all by Montrey */
void do_sacrifice(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Object *obj, *obj_next;
	bool found = FALSE;
	int silver = 0;
	Character *person;
	bool being_used;
	/* variables for AUTOSPLIT */
	Character *gch;
	int members;
	char buffer[100];

	String arg;
	one_argument(argument, arg);

	if (arg.empty() || arg == ch->name) {
		act("$n flings $mself at the feet of the Gods...sad.", ch, nullptr, nullptr, TO_ROOM);
		stc("Please refrain from bloodying the altar.\n", ch);
		return;
	}

	if (arg == "all") {
		for (obj = ch->in_room->contents; obj != nullptr; obj = obj_next) {
			obj_next = obj->next_content;

			if (!acceptable_sac(ch, obj))
				continue;

			/* Make sure no one is resting on the item. -- Outsider */
			person = Game::world().char_list;
			being_used = FALSE;

			while ((person) && (! being_used)) {
				if (person->on == obj)
					being_used = TRUE;
				else
					person = person->next;
			}

			if (!IS_NPC(ch) || !IS_IMMORTAL(ch))
				silver += URANGE(1, obj->cost, (obj->level * 3));

			act("$n sacrifices $p to the Gods.", ch, obj, nullptr, TO_ROOM);
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
			Format::sprintf(buf, "%s gives you one silver coin for your sacrifices.\n", ch->pcdata->deity);
			stc(buf, ch);
		}
		else {
			Format::sprintf(buf, "%s gives you %d silver coins for your sacrifices.\n", ch->pcdata->deity, silver);
			stc(buf, ch);
		}

		wiznet("$N sends up a grateful sacrifice.", ch, nullptr, WIZ_SACCING, 0, 0);
	}
	else {
		obj = get_obj_list(ch, arg, ch->in_room->contents);

		if (obj == nullptr) {
			stc("You can't find it.\n", ch);
			return;
		}

		if (!acceptable_sac(ch, obj))
			return;

		/* Make sure no one is sleeping on/in the item. -- Outsider */
		person = Game::world().char_list;

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
				Format::sprintf(buf, "%s gives you one silver coin for your sacrifice.\n", ch->pcdata->deity);
				stc(buf, ch);
			}
			else {
				Format::sprintf(buf, "%s gives you %d silver coins for your sacrifice.\n", ch->pcdata->deity, silver);
				stc(buf, ch);
			}
		}

		act("$n sacrifices $p to the Gods.", ch, obj, nullptr, TO_ROOM);
		wiznet("$N sends up $p as a grateful sacrifice.", ch, obj, WIZ_SACCING, 0, 0);
		extract_obj(obj);
	}

	ch->silver += silver;

	if (ch->act_flags.has(PLR_AUTOSPLIT)) {
		/* AUTOSPLIT code */
		members = 0;

		for (gch = ch->in_room->people; gch != nullptr; gch = gch->next_in_room)
			if (is_same_group(gch, ch)
			    && !IS_NPC(gch))
				members++;

		if (members > 1 && silver > 1) {
			Format::sprintf(buffer, "%d", silver);
			do_split(ch, buffer);
		}
	}
}

void do_quaff(Character *ch, String argument)
{
	Object *obj;

	if (argument.empty()) {
		stc("Quaff what?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if ((obj = get_obj_carry(ch, arg)) == nullptr) {
		if ((obj = get_eq_char(ch, WEAR_HOLD)) == nullptr) {
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

	act("$n quaffs $p.", ch, obj, nullptr, TO_ROOM);
	act("You quaff $p.", ch, obj, nullptr, TO_CHAR);
	obj_cast_spell(skill::from_int(obj->value[1]), obj->value[0], ch, ch, nullptr);
	obj_cast_spell(skill::from_int(obj->value[2]), obj->value[0], ch, ch, nullptr);
	obj_cast_spell(skill::from_int(obj->value[3]), obj->value[0], ch, ch, nullptr);
	obj_cast_spell(skill::from_int(obj->value[4]), obj->value[0], ch, ch, nullptr);
	extract_obj(obj);
}

/* Global for scrolls that want that extra oomph */
extern String target_name;

void do_recite(Character *ch, String argument)
{
	Character *victim;
	Object *scroll, *obj = nullptr;

	String arg1, arg2;
	target_name = one_argument(argument, arg1);
	one_argument(target_name, arg2);

	if ((scroll = get_obj_carry(ch, arg1)) == nullptr) {
		if ((scroll = get_eq_char(ch, WEAR_HOLD)) == nullptr) {
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

	if (!deduct_stamina(ch, skill::type::scrolls))
		return;

	if (arg2.empty()) {
		/* Smart Targetting -- Montrey */
		/* target according to first spell on the scroll */
		if (skill::lookup(skill::from_int(scroll->value[1])).target == TAR_CHAR_OFFENSIVE
		    || skill::lookup(skill::from_int(scroll->value[1])).target == TAR_OBJ_CHAR_OFF) {
			if (ch->fighting != nullptr)
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
		if ((victim = get_char_here(ch, arg2, VIS_CHAR)) == nullptr
		    && (obj    = get_obj_here(ch, arg2)) == nullptr) {
			stc("You can't find it.\n", ch);
			return;
		}
	}

	act("$n recites $p.", ch, scroll, nullptr, TO_ROOM);
	act("You recite $p.", ch, scroll, nullptr, TO_CHAR);

	if (number_percent() >= 20 + get_skill_level(ch, skill::type::scrolls) * 4 / 5) {
		stc("You mispronounce a syllable.\n", ch);
		check_improve(ch, skill::type::scrolls, FALSE, 2);
	}
	else {
		obj_cast_spell(skill::from_int(scroll->value[1]), scroll->value[0], ch, victim, obj);
		obj_cast_spell(skill::from_int(scroll->value[2]), scroll->value[0], ch, victim, obj);
		obj_cast_spell(skill::from_int(scroll->value[3]), scroll->value[0], ch, victim, obj);
		obj_cast_spell(skill::from_int(scroll->value[4]), scroll->value[0], ch, victim, obj);
		check_improve(ch, skill::type::scrolls, TRUE, 2);
	}

	/* delay on scrolls -- Elrac */
	WAIT_STATE(ch, (ch->guild == Guild::mage || ch->guild == Guild::cleric || ch->guild == Guild::necromancer)
	           ? (skill::lookup(skill::type::scrolls).beats * 4) / 5
	           : skill::lookup(skill::type::scrolls).beats);
	extract_obj(scroll);
}

void do_brandish(Character *ch, String argument)
{
	Character *vch;
	Character *vch_next;
	Object *staff;

	if ((staff = get_eq_char(ch, WEAR_HOLD)) == nullptr) {
		stc("You hold nothing in your hand.\n", ch);
		return;
	}

	if (staff->item_type != ITEM_STAFF) {
		stc("You can brandish only with a staff.\n", ch);
		return;
	}

	skill::type type = skill::from_int(staff->value[3]);

	if (type == skill::type::unknown) {
		Logging::bug("Do_brandish: bad sn %d.", staff->value[3]);
		return;
	}

	const auto& entry = skill::lookup(type);

	if (entry.spell_fun == 0) {
		Logging::bugf("Do_brandish: bad spell '%s'.", entry.name);
		return;
	}

	if (!deduct_stamina(ch, skill::type::staves))
		return;

	WAIT_STATE(ch, skill::lookup(skill::type::staves).beats);

	if (staff->value[2] > 0) {
		act("$n lifts up and brandishes $p.", ch, staff, nullptr, TO_ROOM);
		act("You brandish $p.",  ch, staff, nullptr, TO_CHAR);

		if (get_usable_level(ch) < staff->level
		    ||   number_percent() >= 20 + get_skill_level(ch, skill::type::staves) * 4 / 5) {
			act("You fail to invoke $p.", ch, staff, nullptr, TO_CHAR);
			act("...and nothing happens.", ch, nullptr, nullptr, TO_ROOM);
			check_improve(ch, skill::type::staves, FALSE, 2);
		}
		else for (vch = ch->in_room->people; vch; vch = vch_next) {
				vch_next    = vch->next_in_room;

				switch (entry.target) {
				default:
					Logging::bug("Do_brandish: bad target for sn %d.", staff->value[3]);
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

				obj_cast_spell(type, staff->value[0], ch, vch, nullptr);
				check_improve(ch, skill::type::staves, TRUE, 2);
			}
	}

	if (--staff->value[2] <= 0) {
		act("$n's $p blazes bright and is gone.", ch, staff, nullptr, TO_ROOM);
		act("Your $p blazes bright and is gone.", ch, staff, nullptr, TO_CHAR);
		destroy_obj(staff);
	}

	return;
}

void do_zap(Character *ch, String argument)
{
	Character *victim;
	Object *wand;
	Object *obj = nullptr;

	if (argument.empty() && ch->fighting == nullptr) {
		stc("Zap whom or what?\n", ch);
		return;
	}

	if ((wand = get_eq_char(ch, WEAR_HOLD)) == nullptr) {
		stc("You are holding nothing in your hand.\n", ch);
		return;
	}

	if (wand->item_type != ITEM_WAND) {
		stc("You can zap only with a wand.\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {   /* no target given */
		/* check for default target */
		if (ch->fighting != nullptr)
			victim = ch->fighting;
		else {
			stc("Zap whom or what?\n", ch);
			return;
		}
	}
	else {  /* target given, see if it is valid */
		if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr
		    && (obj    = get_obj_here(ch, arg)) == nullptr
		    && (skill::from_int(wand->value[3]) != skill::type::summon || (victim = get_char_world(ch, arg, VIS_CHAR)) == nullptr)) {
			stc("You can't find it.\n", ch);
			return;
		}
	}

	/* added this cause it'd prolly crash if someone tried :P -- Montrey */
	if (victim == nullptr && obj == wand) {
		stc("You can't zap a wand with itself.\n", ch);
		return;
	}

	if (!deduct_stamina(ch, skill::type::wands))
		return;

	WAIT_STATE(ch, skill::lookup(skill::type::wands).beats);

	if (wand->value[2] > 0) {
		if (victim != nullptr) {
			if (victim == ch) {
				act("$n zaps $mself with $p.", ch, wand, victim, TO_ROOM);
				act("You zap yourself with $p.", ch, wand, victim, TO_CHAR);
			}
			else {
				act("$n zaps $N with $p.", ch, wand, victim, TO_ROOM);
				act("You zap $N with $p.", ch, wand, victim, TO_CHAR);
			}
		}
		else { /* victim is nullptr, must be an object */
			act("$n zaps $P with $p.", ch, wand, obj, TO_ROOM);
			act("You zap $P with $p.", ch, wand, obj, TO_CHAR);
		}

		if (get_usable_level(ch) < wand->level
		    ||  number_percent() >= 20 + get_skill_level(ch, skill::type::wands) * 4 / 5) {
			act("Your efforts with $p produce only sparks and smoke.",
			    ch, wand, nullptr, TO_CHAR);
			act("$n's efforts with $p produce only sparks and smoke.",
			    ch, wand, nullptr, TO_ROOM);
			check_improve(ch, skill::type::wands, FALSE, 2);
		}
		else {
			/* Wand does not work on target, unless we set target_name.
			   target_name is a global variable in magic.c --Outsider */
			target_name = arg;
			obj_cast_spell(skill::from_int(wand->value[3]), wand->value[0], ch, victim, obj);
			check_improve(ch, skill::type::wands, TRUE, 2);
		}
	}

	if (--wand->value[2] <= 0) {
		act("$n's $p explodes into fragments.", ch, wand, nullptr, TO_ROOM);
		act("Your $p explodes into fragments.", ch, wand, nullptr, TO_CHAR);
		destroy_obj(wand);
	}

	return;
}

void do_brew(Character *ch, String argument)
{
	Object *obj;
	skill::type sn;
	int target_level = 0;    /* what level should we brew at? */

	if (!IS_NPC(ch)
	    && ch->level < skill::lookup(skill::type::brew).skill_level[ch->guild]) {
		stc("You do not know how to brew potions.\n", ch);
		return;
	}

	if (IS_NPC(ch)) {
		stc("Mobiles can't brew!.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("Brew what spell?\n", ch);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

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

	if ((sn = skill::lookup(arg)) == skill::type::unknown) {
		stc("There is no such spell.\n", ch);
		return;
	}

	if (!get_skill_level(ch, sn)) {
		stc("You don't know any spells by that name.\n", ch);
		return;
	}

	/* preventing potions of gas breath, acid blast, etc.; doesn't make sense
	   when you quaff a gas breath potion, and then the mobs in the room are
	   hurt. Those TAR_IGNORE spells are a mixed blessing. - JH */

	if (((skill::lookup(sn).target != TAR_CHAR_DEFENSIVE) &&
	     (skill::lookup(sn).target != TAR_CHAR_SELF) &&
	     (skill::lookup(sn).target != TAR_OBJ_CHAR_DEF))
	    ||
	    (skill::lookup(sn).remort_guild > 0)) {
		stc("You cannot brew that spell.\n", ch);
		return;
	}

	/* We shall let casters brew spells at a lower level, provided
	   the level is high enough for their class to cast that spell.
	   Example: Fireball cannot be brewed as a 1st level spell.
	   Re-using "arg" here. We seem to be done with it. -- Outsider
	*/
	argument = one_argument(argument, arg);

	if (!arg.empty()) {  /* we got a new level set */
		target_level = atoi(arg);

		/* make sure the new level is high enough */
		if (target_level < skill::lookup(sn).skill_level[ch->guild]) {
			stc("You cannot brew the spell at that low a level.\n", ch);
			return;
		}

		/* Keep level from being too high. */
		if (target_level > ch->level) {
			stc("You have not yet reached that level!\n", ch);
			return;
		}
	}

	if (!deduct_stamina(ch, skill::type::brew))
		return;

	act("$n begins preparing a potion.", ch, obj, nullptr, TO_ROOM);
	check_improve(ch, skill::type::brew, TRUE, 2);
	WAIT_STATE(ch, skill::lookup(skill::type::brew).beats);

	/* Check the skill percentage, fcn(wis,int,skill) */
	if (!IS_NPC(ch)
	    && (number_percent() > get_skill_level(ch, skill::type::brew) ||
	        number_percent() > ((GET_ATTR_INT(ch) - 13) * 5 +
	                            (GET_ATTR_WIS(ch) - 13) * 3))) {
		act("$p explodes violently!", ch, obj, nullptr, TO_CHAR);
		act("$p explodes violently!", ch, obj, nullptr, TO_ROOM);
		check_improve(ch, skill::type::brew, FALSE, 2);
		destroy_obj(obj);
		return;
	}

	/* took this outside of imprint codes, so I can make do_brew differs from
	   do_scribe; basically, setting potion level and spell level --- JH */

	if (target_level) {
		obj->level = (int) target_level;
		obj->value[0] = (int) target_level;
	}
	else {
		obj->level = ch->level;
		obj->value[0] = ch->level;
	}

	spell_imprint(sn, ch->level, ch, obj);
}

void do_scribe(Character *ch, String argument)
{
	Object *obj;
	skill::type sn;
	int target_level = 0;   /* let caster make items of lower level */

	if (!IS_NPC(ch)
	    && ch->level < skill::lookup(skill::type::scribe).skill_level[ch->guild]) {
		stc("You do not know how to scribe scrolls.\n", ch);
		return;
	}

	if (IS_NPC(ch)) {
		stc("Mobiles can't scribe!.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("Scribe what spell?\n", ch);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	/* Do we have a parchment to scribe spells? */
	for (obj = ch->carrying; obj; obj = obj->next_content) {
		if (obj->item_type == ITEM_SCROLL && obj->wear_loc == WEAR_HOLD)
			break;
	}

	if (!obj) {
		stc("You are not holding a parchment.\n", ch);
		return;
	}

	if ((sn = skill::lookup(arg)) == skill::type::unknown) {
		stc("There is no such spell.\n", ch);
		return;
	}

	if (!get_skill_level(ch, sn)) {
		stc("You don't know any spells by that name.\n", ch);
		return;
	}

	if (skill::lookup(sn).remort_guild > 0) {
		stc("You cannot scribe that spell.\n", ch);
		return;
	}

	/* Check to see if caster is trying to make the scroll at a lower
	   level. Do not let the caster make scrolls of a lower level than
	   normally required to cast the spell. Example: No 1st level fireballs.
	   Re-using "Arg" here. We seem to be done with it.
	   -- Outsider
	*/
	argument = one_argument(argument, arg);

	if (!arg.empty()) {  /* we got some data */
		target_level = atoi(arg);

		/* Keep the spell level from dropping too low. */
		if (target_level < skill::lookup(sn).skill_level[ch->guild]) {
			stc("You cannot scribe that spell at that level.", ch);
			return;
		}

		/* Keep the spell level from going too high. */
		if (target_level > ch->level) {
			stc("You are not yet at that level!\n", ch);
			return;
		}
	}

	if (!deduct_stamina(ch, skill::type::scribe))
		return;

	act("$n begins writing a scroll.", ch, obj, nullptr, TO_ROOM);
	check_improve(ch, skill::type::scribe, TRUE, 2);
	WAIT_STATE(ch, skill::lookup(skill::type::scribe).beats);

	/* Check the skill percentage, fcn(int,wis,skill) */
	if (!IS_NPC(ch)
	    && (number_percent() > get_skill_level(ch, skill::type::scribe) ||
	        number_percent() > ((GET_ATTR_INT(ch) - 13) * 5 +
	                            (GET_ATTR_WIS(ch) - 13) * 3))) {
		act("$p bursts in flames!", ch, obj, nullptr, TO_CHAR);
		act("$p bursts in flames!", ch, obj, nullptr, TO_ROOM);
		check_improve(ch, skill::type::scribe, FALSE, 2);
		destroy_obj(obj);
		return;
	}

	/* basically, making scrolls more potent than potions; also, scrolls
	   are not limited in the choice of spells, i.e. scroll of enchant weapon
	   has no analogs in potion forms --- JH */

	if (target_level) {
		obj->level = (int) target_level;
		obj->value[0] = (int) target_level;
	}
	else {
		obj->level = ch->level;
		obj->value[0] = ch->level;
	}

	spell_imprint(sn, ch->level, ch, obj);
}

void do_steal(Character *ch, String argument)
{
	char buf  [MAX_STRING_LENGTH];
	Character *victim;
	Object *obj;
	int percent;

	if (get_skill_level(ch, skill::type::steal) == 0) {
		stc("You don't know how to steal.\n", ch);
		return;
	}

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1.empty() || arg2.empty()) {
		stc("Steal what from whom?\n", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg2, VIS_CHAR)) == nullptr) {
		stc("Can't steal from people who are not here.\n", ch);
		return;
	}

	if (victim == ch) {
		stc("A little schizophrenia going on here?\n", ch);
		return;
	}

	if (victim->in_room->sector_type() == Sector::arena
	    || victim->in_room->sector_type() == Sector::clanarena
	    || (victim->in_room->area() == Game::world().quest.area() && !Game::world().quest.pk)
	    || char_in_duel_room(victim)) {
		stc("You are here to do battle, not to steal!\n", ch);
		return;
	}

	if (IS_NPC(ch) && ch->act_flags.has(ACT_MORPH) && !IS_NPC(victim)) {
		stc("Morphed players cannot attack PC's.\n", ch);
		return;
	}

	if (!IS_NPC(victim) && IS_IMMORTAL(victim)) {
		stc("I'm sorry, but the gods wouldn't like that!\n", ch);
		return;
	}

	if (is_safe(ch, victim, TRUE))
		return;

	if (!deduct_stamina(ch, skill::type::steal))
		return;

	if (IS_NPC(victim) && victim->fighting) {
		stc("I don't think that is a good idea.\n", ch);
		return;
	}

	WAIT_STATE(ch, skill::lookup(skill::type::steal).beats);
	percent  = number_percent();

	if (get_skill_level(ch, skill::type::steal) >= 1)
		percent  += (IS_AWAKE(victim) ? 10 : -50);

	if ((!IS_NPC(victim) && !IS_NPC(ch) && !IS_IMMORTAL(ch))
	    && ((ch->level + 8 < victim->level || ch->level - 8 > victim->level)
	        || (!is_clan(victim) || !is_clan(ch)))) {
		stc("You cannot steal from them.\n", ch);
		return;
	}

	if (!IS_NPC(victim) && !IS_IMMORTAL(ch)) {
		if (!victim->pcdata->plr_flags.has(PLR_PK)) {
			stc("They are not in the mood to PSteal.\n", ch);
			return;
		}

		if (!ch->pcdata->plr_flags.has(PLR_PK)) {
			stc("You are not in the mood to PSteal.\n", ch);
			return;
		}
	}

	if (!IS_NPC(ch) && percent > get_skill_level(ch, skill::type::steal)) {
		/*
		 * Failure.
		 */
		stc("Uh-oh!  WhOops.\n", ch);
		act("$n tried to steal from you.\n", ch, nullptr, victim, TO_VICT);
		act("$n bungles a robbery attempt on $N.\n",  ch, nullptr, victim, TO_NOTVICT);

		switch (number_range(0, 3)) {
		case 0 :
			Format::sprintf(buf, "%s is a bum mugger!", ch->name);
			break;

		case 1 :
			Format::sprintf(buf, "%s couldn't rob %s way out of a paper bag!",
			        ch->name, GET_ATTR_SEX(ch) == SEX_NEUTRAL ? "its" : GET_ATTR_SEX(ch) == SEX_MALE ? "his" : "her");
			break;

		case 2 :
			Format::sprintf(buf, "%s tried to rob me!", ch->name);
			break;

		case 3 :
			Format::sprintf(buf, "Keep your hands out of there, %s! Pervert!", ch->name);
			break;
		}

		do_yell(victim, buf);

		if (!IS_NPC(ch)) {
			if (IS_NPC(victim)) {
				check_improve(ch, skill::type::steal, FALSE, 2);
				multi_hit(victim, ch, skill::type::unknown);
			}
			else {
				Format::sprintf(buf, "$N tried to steal from %s.", victim->name);
				wiznet(buf, ch, nullptr, WIZ_FLAGS, 0, 0);

				if (!ch->act_flags.has(PLR_THIEF)) {
					ch->act_flags += PLR_THIEF;
					ch->pcdata->plr_flags += PLR_NOPK;
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

	if (arg1 == "coin"
	    ||   arg1 == "coins"
	    ||   arg1 == "gold"
	    ||   arg1 == "silver") {
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
			Format::sprintf(buf, "Bingo!  You got %d gold coin%s.\n", gold,
			        gold == 1 ? "" : "s");
		else if (gold <= 0)
			Format::sprintf(buf, "Bingo!  You got %d silver coin%s.\n", silver,
			        silver == 1 ? "" : "s");
		else
			Format::sprintf(buf, "Bingo!  You got %d silver and %d gold coins.\n",
			        silver, gold);

		stc(buf, ch);
		check_improve(ch, skill::type::steal, TRUE, 2);
		return;
	}

	for (obj = victim->carrying; obj != nullptr; obj = obj->next_content) {
		if (obj->wear_loc == WEAR_NONE
		    && can_see_obj(ch, obj)
		    && obj->name.has_words(arg1))
			break;
	}

	if (obj == nullptr) {
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
	check_improve(ch, skill::type::steal, TRUE, 2);
	stc("Got it!\n", ch);

	/* Did they pick up their quest item? */
	if (IS_QUESTOR(ch)) {
		if (ch->pcdata->questobj == obj->pIndexData->vnum && ch->pcdata->questobf != -1) {
			char buf[MAX_STRING_LENGTH];
			stc("{YYou have almost completed your QUEST!{x\n", ch);
			stc("{YReturn to the questmaster before your time runs out!{x\n", ch);
			ch->pcdata->questobf = -1;
			Format::sprintf(buf, "{Y:QUEST: {x$N has found %s", obj->short_descr);
			wiznet(buf, ch, nullptr, WIZ_QUEST, 0, 0);
		}
	}

	/* or skill quest item? */
	if (IS_SQUESTOR(ch)) {
		if (ch->pcdata->squestobj == obj && !ch->pcdata->squestobjf) {
			char buf[MAX_STRING_LENGTH];

			if (ch->pcdata->squestmob == nullptr) {
				stc("{YYou have almost completed your {VSKILL QUEST!{x\n", ch);
				stc("{YReturn to the questmistress before your time runs out!{x\n", ch);
			}
			else {
				stc("{YYou have completed part of your {VSKILL QUEST!{x\n", ch);
				ptc(ch, "{YTake the artifact to %s while there is still time!{x\n",
				    ch->pcdata->squestmob->short_descr);
			}

			ch->pcdata->squestobjf = TRUE;
			Format::sprintf(buf, "{Y:SKILL QUEST: {x$N has found the %s", obj->short_descr);
			wiznet(buf, ch, nullptr, WIZ_QUEST, 0, 0);
		}
	}
}

/*
 * Shopping commands.
 */
Character *find_keeper(Character *ch)
{
	/*char buf[MAX_STRING_LENGTH];*/
	Character *keeper;
	Shop *pShop;
	pShop = nullptr;

	for (keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room) {
		if (keeper->act_flags.has(ACT_MORPH))
			continue;

		if (IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != nullptr)
			break;
	}

	if (pShop == nullptr) {
		stc("This isn't a store...you might have noticed.\n", ch);
		return nullptr;
	}

	/*
	 * Undesirables.
	 * REWORK PK, lets let them shop, Lotus
	if ( !IS_NPC(ch) && ch->act_flags.has(PLR_KILLER) )
	{
	        do_say( keeper, "Killers are not welcome!" );
	        Format::sprintf( buf, "%s the psycho KILLER is over here!\n", ch->name );
	        do_yell( keeper, buf );
	        return nullptr;
	}

	if ( !IS_NPC(ch) && ch->act_flags.has(PLR_THIEF) )
	{
	        do_say( keeper, "Thieves are not welcome!" );
	        Format::sprintf( buf, "%s the gutless THIEF is over here!\n", ch->name );
	        do_yell( keeper, buf );
	        return nullptr;
	}
	        */
	/*
	 * Shop hours.
	 */
	if (keeper->in_room->area().world.time.hour < pShop->open_hour) {
		do_say(keeper, "Sorry, I am closed.");
		return nullptr;
	}

	if (keeper->in_room->area().world.time.hour > pShop->close_hour) {
		do_say(keeper, "Sorry, I am closed.");
		return nullptr;
	}

	/*
	 * Invisible or hidden people.
	 */
	if (!can_see_char(keeper, ch)) {
		do_say(keeper, "Umm, Sorry, If I could see you, we could trade!");
		return nullptr;
	}

	return keeper;
}

/* insert an object at the right spot for the keeper */
void obj_to_keeper(Object *obj, Character *ch)
{
	Object *t_obj, *t_obj_next;

	/* see if any duplicates are found */
	for (t_obj = ch->carrying; t_obj != nullptr; t_obj = t_obj_next) {
		t_obj_next = t_obj->next_content;

		if (obj->pIndexData == t_obj->pIndexData
		    && obj->short_descr == t_obj->short_descr) {
			/* if this is an unlimited item, destroy the new one */
			if (IS_OBJ_STAT(t_obj, ITEM_INVENTORY)) {
				extract_obj(obj);
				return;
			}

			obj->cost = t_obj->cost; /* keep it standard */
			break;
		}
	}

	if (t_obj == nullptr) {
		obj->next_content = ch->carrying;
		ch->carrying = obj;
	}
	else {
		obj->next_content = t_obj->next_content;
		t_obj->next_content = obj;
	}

	obj->carried_by      = ch;
	obj->in_room         = nullptr;
	obj->in_obj          = nullptr;
}

/* get an object from a shopkeeper's list */
Object *get_obj_keeper(Character *ch, Character *keeper, const String& argument)
{
	Object *obj;
	int number;
	int count;

	String arg;
	number = number_argument(argument, arg);
	count  = 0;

	for (obj = keeper->carrying; obj != nullptr; obj = obj->next_content) {
		if (obj->wear_loc == WEAR_NONE
		    &&  can_see_obj(keeper, obj)
		    &&  can_see_obj(ch, obj)
		    &&  obj->name.has_words(arg)) {
			if (++count == number)
				return obj;

			/* skip other objects of the same name */
			while (obj->next_content != nullptr
			       && obj->pIndexData == obj->next_content->pIndexData
			       && obj->short_descr == obj->next_content->short_descr)
				obj = obj->next_content;
		}
	}

	return nullptr;
}

int get_cost(Character *keeper, Object *obj, bool fBuy)
{
	Shop *pShop;
	int cost;

	if (obj == nullptr || (pShop = keeper->pIndexData->pShop) == nullptr)
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
		Object *obj2;
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
				    &&   obj->short_descr == obj2->short_descr) {
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

void make_pet(Character *ch, Character *pet) {
	pet->act_flags += ACT_PET;
	affect::add_perm_to_char(pet, affect::type::charm_person);
	pet->comm_flags = COMM_NOCHANNELS;
	add_follower(pet, ch);
	pet->leader = ch;
	ch->pet = pet;
}

void do_buy(Character *ch, String argument)
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

	if (argument.empty()) {
		stc("What do you want to buy?\n", ch);
		return;
	}

	if (ch->in_room->flags().has(ROOM_PET_SHOP)) {
		/* PETS */
		char buf[MAX_STRING_LENGTH];
		Character *pet;
		Room *roomNext;
		Room *in_room;

		if (IS_NPC(ch))
			return;

		String arg;
		argument = one_argument(argument, arg);

		/* hack to make new thalos pets work */
		if (ch->in_room->location == Location(Vnum(9621)))
			roomNext = Game::world().get_room(Location(Vnum(9706)));
		else
			roomNext = Game::world().get_room(Location(Vnum(ch->in_room->prototype.vnum.value() + 1)));

		if (roomNext == nullptr) {
			Logging::bugf("Do_buy: bad pet shop at vnum %d.", ch->in_room->prototype.vnum);
			stc("Sorry, we don't sell those.  If you'd like to see my manager...\n", ch);
			return;
		}

		in_room     = ch->in_room;
		ch->in_room = roomNext;
		pet         = get_char_here(ch, arg, VIS_CHAR);
		ch->in_room = in_room;

		if (ch->in_room->guild() && ch->in_room->guild() != ch->guild + 1 && !IS_IMMORTAL(ch)) {
			stc("Sorry, members only.\n", ch);
			return;
		}

		if (pet == nullptr || !pet->act_flags.has(ACT_PET)) {
			stc("Sorry, we don't sell those.  If you'd like to see my manager...\n", ch);
			return;
		}

		if (ch->pet != nullptr) {
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

		if (roll < get_skill_level(ch, skill::type::haggle)) {
			cost -= cost / 2 * roll / 100;
			ptc(ch, "You haggle the price down to %d coins.\n", cost);
			check_improve(ch, skill::type::haggle, TRUE, 4);
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
			Logging::bug("Memory error from create_mobile() in do_buy().", 0);
			stc("You were unable to take your pet! You get a refund.\n", ch);
			ch->silver += cost;
		}

		argument = one_argument(argument, arg);

		if (!arg.empty()) {
			if (strchr(arg, '~'))
				wiznet("$N is attempting to use the tilde in pet name cheat.",
				       ch, nullptr, WIZ_CHEAT, 0, GET_RANK(ch));

			Format::sprintf(buf, "%s %s", pet->name, arg);
			pet->name = buf;
		}

		Format::sprintf(buf, "%sA collar around its neck says 'I belong to %s'.\n",
		        pet->description, ch->name);
		pet->description = buf;
		char_to_room(pet, ch->in_room);

		make_pet(ch, pet);

		stc("Enjoy your pet.  Watch out, they bite!\n", ch);
		act("$n purchased $N as a pet.", ch, nullptr, pet, TO_ROOM);
		return;
	}
	else {
		/* non-pet shop */
		Character *keeper;
		Object *obj, *t_obj;
		int number, count = 1;

		if ((keeper = find_keeper(ch)) == nullptr)
			return;

		if (keeper->in_room->guild() && keeper->in_room->guild() != ch->guild + 1 && !IS_IMMORTAL(ch)) {
			act("$n tells you 'Sorry, members only.'", keeper, nullptr, ch, TO_VICT);
			return;
		}

		String arg;
		number = mult_argument(argument, arg);

		obj  = get_obj_keeper(ch, keeper, arg);
		cost = get_cost(keeper, obj, TRUE);

		if (cost <= 0 || !can_see_obj(ch, obj)) {
			act("$n tells you 'I don't sell that -- try 'list''.",
			    keeper, nullptr, ch, TO_VICT);
			ch->reply = keeper->name;
			return;
		}

		/* Negative buy fix and large buy fix */
		if (number <= 0 || number >= 100) {
			stc("Sorry, I don't have that many.\n", ch);
			return;
		}

		if (!IS_OBJ_STAT(obj, ITEM_INVENTORY)) {
			for (t_obj = obj->next_content;
			     count < number && t_obj != nullptr;
			     t_obj = t_obj->next_content) {
				if (t_obj->pIndexData == obj->pIndexData
				    &&  t_obj->short_descr == obj->short_descr)
					count++;
				else
					break;
			}

			if (count < number) {
				act("$n tells you 'I don't have that many in stock.",
				    keeper, nullptr, ch, TO_VICT);
				ch->reply = keeper->name;
				return;
			}
		}

		if (IS_QUESTSHOPKEEPER(keeper)) {
			if (IS_NPC(ch) || ch->pcdata->questpoints < cost * number) {
				act("$n tell you 'Sorry, but you don't have enough quest points!'",
				    keeper, nullptr, ch, TO_VICT);
				ch->reply = keeper->name;
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

				ch->reply = keeper->name;
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
			ch->reply = keeper->name;
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
			    && roll < get_skill_level(ch, skill::type::haggle)
			    && obj->pIndexData->item_type != ITEM_MONEY) { /* to prevent buying money for less than value */
				cost -= obj->cost / 2 * roll / 100;
				act("You haggle with $N.", ch, nullptr, keeper, TO_CHAR);
				check_improve(ch, skill::type::haggle, TRUE, 4);
			}

			if (number > 1) {
				Format::sprintf(buf, "$n buys $p[%d].", number);
				act(buf, ch, obj, nullptr, TO_ROOM);
				Format::sprintf(buf, "You buy $p[%d] for %d silver.", number, cost * number);
				act(buf, ch, obj, nullptr, TO_CHAR);
			}
			else {
				act("$n buys $p.", ch, obj, nullptr, TO_ROOM);
				Format::sprintf(buf, "You buy $p for %d silver.", cost);
				act(buf, ch, obj, nullptr, TO_CHAR);
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
				Format::sprintf(buf, "$n buys $p[%d].", number);
				act(buf, ch, obj, nullptr, TO_ROOM);
				Format::sprintf(buf, "You buy $p[%d] for %d Quest Points.",
				        number, cost * number);
				act(buf, ch, obj, nullptr, TO_CHAR);
			}
			else {
				act("$n buys $p.", ch, obj, nullptr, TO_ROOM);
				Format::sprintf(buf, "You buy $p for %d Quest Points.", cost);
				act(buf, ch, obj, nullptr, TO_CHAR);
			}

			ch->pcdata->questpoints -= cost;
		}

		mprog_buy_trigger(keeper, ch);

		for (count = 0; count < number; count++) {
			if (IS_OBJ_STAT(obj, ITEM_INVENTORY)) {
				t_obj = create_object(obj->pIndexData, obj->level);

				if (! t_obj) {
					Logging::bug("Memory error creating object in do_buy.", 0);
					return;
				}
			}
			else {
				t_obj = obj;
				obj = obj->next_content;
				obj_from_char(t_obj);
			}

			/* owner items bought in guild rooms to purchaser -- Montrey */
			if (keeper->in_room->guild()) {
				String owner;
				ExtraDescr *ed;
				bool foundold = FALSE;

				/* loop through and find previous owner, if any, and change */
				if (t_obj->extra_descr != nullptr) {
					for (ed = t_obj->extra_descr; ed != nullptr; ed = ed->next)
						if (ed->keyword == KEYWD_OWNER) {
							owner = ch->name;
							ed->description = owner;
							foundold = TRUE;
						}
				}

				if (!foundold) {
					ed                      = new ExtraDescr(KEYWD_OWNER, ch->name);
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

void do_list(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];

	if (ch->in_room->flags().has(ROOM_PET_SHOP)) {
		Room *roomNext;
		Character *pet;
		bool found;

		/* hack to make new thalos pets work */
		if (ch->in_room->location == Location(Vnum(9621)))
			roomNext = Game::world().get_room(Location(Vnum(9706)));
		else
			roomNext = Game::world().get_room(Location(Vnum(ch->in_room->prototype.vnum.value() + 1)));

		if (roomNext == nullptr) {
			Logging::bugf("Do_list: bad pet shop at vnum %d.", ch->in_room->prototype.vnum);
			stc("You can't do that here.\n", ch);
			return;
		}

		found = FALSE;

		for (pet = roomNext->people; pet; pet = pet->next_in_room) {
			if (pet->act_flags.has(ACT_PET)) {
				if (!found) {
					found = TRUE;
					stc("Pets and Exotic Companions for sale:\n", ch);
				}

				Format::sprintf(buf, "[%2d] %8d - %s\n",
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
		Character *keeper;
		Object *obj;
		int cost, count;
		bool found;

		if ((keeper = find_keeper(ch)) == nullptr)
			return;

		String arg;
		one_argument(argument, arg);
		found = FALSE;

		for (obj = keeper->carrying; obj; obj = obj->next_content) {
			if (obj->wear_loc == WEAR_NONE
			    &&   can_see_obj(ch, obj)
			    && (cost = get_cost(keeper, obj, TRUE)) > 0
			    && (arg.empty()
			        ||  obj->name.has_words(arg))) {
				if (!found) {
					found = TRUE;

					if (IS_QUESTSHOPKEEPER(keeper))
						stc("{PAll prices in QUEST POINTS!{x\n", ch);

					stc("[Lv Price Qty] Item\n", ch);
				}

				if (IS_OBJ_STAT(obj, ITEM_INVENTORY))
					Format::sprintf(buf, "[%2d %5d -- ] %s\n",
					        obj->level, cost, obj->short_descr);
				else {
					count = 1;

					while (obj->next_content != nullptr
					       && obj->pIndexData == obj->next_content->pIndexData
					       && obj->short_descr == obj->next_content->short_descr) {
						obj = obj->next_content;
						count++;
					}

					Format::sprintf(buf, "[%2d %5d %2d ] %s\n",
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

void do_sell(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Character *keeper;
	Object *obj;
	int cost, roll;

	if (argument.empty()) {
		stc("Sell what?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if ((keeper = find_keeper(ch)) == nullptr)
		return;

	if ((obj = get_obj_carry(ch, arg)) == nullptr) {
		act("$n tells you 'You don't have that item'.",
		    keeper, nullptr, ch, TO_VICT);
		ch->reply = keeper->name;
		return;
	}

	if (!can_drop_obj(ch, obj)) {
		stc("You can't let go of it.\n", ch);
		return;
	}

	if (!can_see_obj(keeper, obj)) {
		act("$n doesn't see what you are offering.", keeper, nullptr, ch, TO_VICT);
		return;
	}

	if (IS_QUESTSHOPKEEPER(keeper) && IS_NPC(ch)) {
		stc("Mobiles can't sell quest items.\n", ch);
		return;
	}

	if ((cost = get_cost(keeper, obj, FALSE)) <= 0) {
		act("$n laughs at $p and suggests a novel use for it.", keeper, obj, ch, TO_VICT);
		return;
	}

	if (!IS_QUESTSHOPKEEPER(keeper) && cost > (keeper->silver + 100 * keeper->gold)) {
		act("$n tells you 'The shop has fallen upon hard times. I cant afford $p.'",
		    keeper, obj, ch, TO_VICT);
		return;
	}

	act("$n sells $p.", ch, obj, nullptr, TO_ROOM);

	if (!IS_QUESTSHOPKEEPER(keeper)) {
		/* haggle */
		roll = number_percent();

		if (number_percent() < (GET_ATTR_CHR(ch) * 3))
			roll -= 15;

		if (roll < 1) roll = 1;

		if (!IS_OBJ_STAT(obj, ITEM_SELL_EXTRACT)
		    && roll < get_skill_level(ch, skill::type::haggle)
		    && obj->pIndexData->item_type != ITEM_MONEY) {
			stc("You haggle with the shopkeeper.\n", ch);
			cost += obj->cost / 2 * roll / 100;
			cost = UMIN(cost, 95 * get_cost(keeper, obj, TRUE) / 100);
			cost = UMIN(cost, (keeper->silver + 100 * keeper->gold));
			check_improve(ch, skill::type::haggle, TRUE, 4);
		}

		Format::sprintf(buf, "You sell $p for %d silver and %d gold piece%s.",
		        cost - (cost / 100) * 100, cost / 100, cost == 1 ? "" : "s");
		act(buf, ch, obj, nullptr, TO_CHAR);
		ch->gold     += cost / 100;
		ch->silver   += cost - (cost / 100) * 100;
		deduct_cost(keeper, cost);

		if (keeper->gold < 0)
			keeper->gold = 0;

		if (keeper->silver < 0)
			keeper->silver = 0;
	}
	else {
		Format::sprintf(buf, "You sell $p for %d Quest Point%s.",
		        cost, cost == 1 ? "" : "s");
		act(buf, ch, obj, nullptr, TO_CHAR);
		ch->pcdata->questpoints += cost;
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

void do_value(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Character *keeper;
	Object *obj;
	int cost;

	if (argument.empty()) {
		stc("Value what?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if ((keeper = find_keeper(ch)) == nullptr)
		return;

	if ((obj = get_obj_carry(ch, arg)) == nullptr) {
		act("$n tells you 'You don't have that item'.",
		    keeper, nullptr, ch, TO_VICT);
		ch->reply = keeper->name;
		return;
	}

	if (!can_see_obj(keeper, obj)) {
		act("$n doesn't see what you are offering.", keeper, nullptr, ch, TO_VICT);
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
		Format::sprintf(buf, "$n tells you 'I'll give you %d quest points for $p'.", cost);
	else {
		Format::sprintf(buf,
		        "$n tells you 'I'll give you %d silver and %d gold coins for $p'.",
		        cost - (cost / 100) * 100, cost / 100);
	}

	act(buf, keeper, obj, ch, TO_VICT);
	ch->reply = keeper->name;
	return;
}

/* get the name of the anvil's owner */
String anvil_owner_name(Object *anvil)
{
	/* check if private anvil */
	if (anvil->value[2] == 0) return "";

	/* anvil name must begin with "anvil private " */
	if (!anvil->name.has_prefix("anvil private ")) {
		Logging::bugf("anvil_owner_name: anvil %d has a private flag but incorrect name",
		    anvil->pIndexData->vnum);
		return "";
	}

	return anvil->name.substr(strlen("anvil private "));
}

/* is named player the anvil's owner? -- Elrac */
int is_anvil_owner(Character *ch, Object *anvil)
{
	return anvil_owner_name(anvil).has_exact_words(ch->name);
}

/*
 * forge_flag function, recoded by Vegita and Montrey for use with
 * Age of Legacy's Evolution System.
 *
 */
void forge_flag(Character *ch, const String& argument, Object *anvil)
{
	Object *weapon;
	int table_num, flag_count = 0, evo, qpcost;
	evo = get_evolution(ch, skill::type::forge);

	/* are they wielding a weapon? */
	if ((weapon = get_eq_char(ch, WEAR_WIELD)) == nullptr) {
		stc("You must be wielding a weapon to forge flags upon!\n", ch);
		return;
	}

	/* Weapons under level 30 cannot recieve perm flags */
	if (!IS_IMMORTAL(ch) && weapon->level < 30) {
		stc("The weapon you are wielding is not powerful enough to receive enhancement.\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	/* player used a valid flag type? */
	if ((table_num = affect_index_lookup(arg, weapon_affects)) == -1) {
		ptc(ch, "'%s' is not a valid weapon flag name, sorry!\n", arg);
		return;
	}

	affect::type type = weapon_affects[table_num].type;

	/* have to be evo 2 to forge vorpal, otherwise, can forge everything but sharp and poison */
	if (type == affect::type::poison
	 || type == affect::type::weapon_sharp
	 || type == affect::type::weapon_acidic
	 || (type == affect::type::weapon_vorpal && evo < 2)) {
		stc("You cannot forge that flag.\n", ch);
		return;
	}

	/* now we count the already existing flags */
	/* you can always forge vorpal or two handed, no matter how many flags it has,
	   so those are taken care of above.  now we only care about preventing them
	   from forging too many magic flags */
	if (affect::exists_on_obj(weapon, affect::type::weapon_acidic))   flag_count++;
	if (affect::exists_on_obj(weapon, affect::type::weapon_flaming))  flag_count++;
	if (affect::exists_on_obj(weapon, affect::type::weapon_frost))    flag_count++;
	if (affect::exists_on_obj(weapon, affect::type::weapon_vampiric)) flag_count++;
	if (affect::exists_on_obj(weapon, affect::type::weapon_shocking)) flag_count++;

	if ((flag_count >= 2 || (flag_count >= 1 && evo < 3))
	    && type != affect::type::weapon_two_hands
	    && type != affect::type::weapon_vorpal) {
		stc("You cannot forge any more magical flags on that weapon.\n", ch);
		return;
	}

	/* now we check to see if it already has the weapon flag they are trying to forge
	 * to prevent duplicate weapon flags being forged on the same weapon.
	 */
	if (type == affect::type::weapon_two_hands && affect::exists_on_obj(weapon, type)) {
		act("$p is already a two-handed weapon.", ch, weapon, nullptr, TO_CHAR);
		return;
	}

	if (type == affect::type::weapon_acidic && affect::exists_on_obj(weapon, type)) {
		act("$p is already dripping with acid.", ch, weapon, nullptr, TO_CHAR);
		return;
	}

	if (type == affect::type::weapon_flaming && affect::exists_on_obj(weapon, type)) {
		act("$p is already a flaming weapon.", ch, weapon, nullptr, TO_CHAR);
		return;
	}

	if (type == affect::type::weapon_frost && affect::exists_on_obj(weapon, type)) {
		act("$p's is already coated with ice.", ch, weapon, nullptr, TO_CHAR);
		return;
	}

	if (type == affect::type::weapon_vampiric && affect::exists_on_obj(weapon, type)) {
		act("$p already drinks the blood of its victim.", ch, weapon, nullptr, TO_CHAR);
		return;
	}

	if (type == affect::type::weapon_shocking && affect::exists_on_obj(weapon, type)) {
		act("$p is already imbued with lightning.", ch, weapon, nullptr, TO_CHAR);
		return;
	}

	if (type == affect::type::weapon_vorpal && affect::exists_on_obj(weapon, type)) {
		act("$p is already sharp enough to sever limbs.", ch, weapon, nullptr, TO_CHAR);
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
	if (ch->pcdata->questpoints < qpcost) {
		ptc(ch, "You lack the required %d quest points to forge flags on this anvil.\n", qpcost);
		return;
	}

	/* deduct quest points */
	if (qpcost > 0) {
		ptc(ch, "In a hasty prayer, you offer %d points of your questing experience as a sacrifice.\n", qpcost);
		ch->pcdata->questpoints -= qpcost;
	}

	ptc(ch, "You grab the smith hammer next to the anvil, lay %s down upon the anvil, and strike it with a mighty blow! *CLING!*\n",
	    weapon->short_descr);
	act("$n lays $p upon the anvil and strikes it with a mighty blow!", ch, weapon, nullptr, TO_ROOM);

	if (IS_IMMORTAL(ch) || number_percent() < (flag_count >= 1 ? 50 : 30)) {
		stc("Amid sparks and rising smoke, an awesome transformation affects your weapon!\n", ch);
		act("$n's weapon strikes sparks on the anvil, and smoke rises. It is... changed!", ch, nullptr, nullptr, TO_ROOM);

		affect::Affect af;
		af.where        = TO_WEAPON;
		af.type         = type;
		af.level        = ch->level;
		af.duration     = -1;
		af.location     = 0;
		af.modifier     = 0;
		af.bitvector(0);
		af.evolution    = evo;
		affect::copy_to_obj(weapon, &af);

		if (type == affect::type::weapon_two_hands) {
			++weapon->value[1];
			remove_obj(ch, WEAR_WIELD, TRUE);
		}
	}
	else {
		if (flag_count >= 1) {
			stc("Your weapon begins to glow a bright yellow then suddenly explodes!!!!!\n", ch);
			act("$n's weapon glows a bright yellow then suddenly explodes.\n", ch, nullptr, nullptr, TO_ROOM);
			destroy_obj(weapon);
			return;
		}
		else {
			stc("The blow jars your arm, but you notice no change in your weapon.\n", ch);
			act("$n looks disappointed.", ch, nullptr, nullptr, TO_ROOM);
			return;
		}
	}
}

/* Hone by Montrey */
void do_hone(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Object *weapon;
	Object *whetstone;
	whetstone = get_eq_char(ch, WEAR_HOLD);
	weapon = get_eq_char(ch, WEAR_WIELD);

	if (IS_NPC(ch) || (get_skill_level(ch, skill::type::hone) < 1)) {
		stc("You lack the skill to hone weapons.\n", ch);
		return;
	}

	if (weapon == nullptr) {
		stc("Hone what, your hand?  Try wielding it.\n", ch);
		return;
	}

	if (affect::exists_on_obj(weapon, affect::type::weapon_sharp)) {
		act("$p could not possibly be any sharper.", ch, weapon, nullptr, TO_CHAR);
		return;
	}

	if ((attack_table[weapon->value[3]].damage != DAM_SLASH)
	    && (attack_table[weapon->value[3]].damage != DAM_PIERCE)) {
		stc("You can only hone edged weapons.\n", ch);
		return;
	}

	if ((whetstone == nullptr) || (!whetstone->name.has_words("whetstone"))) {
		stc("How do you expect to sharpen a weapon without holding a whetstone?\n", ch);
		return;
	}

	if (!deduct_stamina(ch, skill::type::hone))
		return;

	if (!IS_IMMORTAL(ch)) {
		if (number_percent() > UMIN(get_skill_level(ch, skill::type::hone), 95)) {
			Format::sprintf(buf, "You fail to hone your weapon, and you gouge %s deeply, ruining it.\n",
			        whetstone->short_descr);
			stc(buf, ch);
			act("$n's hand slips, and $e ruins $s whetstone.", ch, nullptr, nullptr, TO_ROOM);
			destroy_obj(whetstone);
			return;
		}
	}

	Format::sprintf(buf, "You skillfully hone %s to a razor edge.\n", weapon->short_descr);
	act("$n skillfully sharpens $p to a razor edge.", ch, weapon, nullptr, TO_ROOM);
	stc(buf, ch);

	affect::Affect af;
	af.where     = TO_WEAPON;
	af.type      = affect::type::weapon_sharp;
	af.level     = ch->level;
	af.duration  = ch->level * 5;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector(0);
	af.evolution = get_evolution(ch, skill::type::hone);
	affect::copy_to_obj(weapon, &af);
	return;
}

void do_forge(Character *ch, String argument)
{
	Object *obj, *anvil = nullptr, *material;
	Character *owner;
	String buf;
	char sdesc[MSL], costbuf[MSL];
	ExtraDescr *ed;
	int is_owner, cost, cost_gold, cost_silver, evo;
	evo = get_evolution(ch, skill::type::forge);
	/* check arguments */

	String type;
	argument = one_argument(argument, type);

	if (argument.empty() || type.empty()) {
		if (get_skill_level(ch, skill::type::forge))
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

	if (anvil == nullptr && !IS_IMMORTAL(ch)) {
		stc("How do you propose to forge without an anvil?\n", ch);
		stc("A public anvil may be found in the smithy below Griswold's Repair Shop.\n", ch);
		return;
	}

	/* check for FORGE FLAG */
	if (type.is_prefix_of("flag") && !IS_NPC(ch)) {
		if (!deduct_stamina(ch, skill::type::forge))
			return;

		forge_flag(ch, argument, anvil);
		return;
	}

	/* not FORGE FLAG, so check skill */
	if (!get_skill_level(ch, skill::type::forge)) {
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

	if (argument.uncolor().size() > 30) {
		stc("Name is limited to 30 printed characters.\n", ch);
		return;
	}

	if ((material = get_eq_char(ch, WEAR_HOLD)) == nullptr) {
		stc("You are not holding any materials to make a weapon!\n", ch);
		return;
	}

	if (material->item_type != ITEM_MATERIAL) {
		stc("That is not a useable material!\n", ch);
		return;
	}

	if (!deduct_stamina(ch, skill::type::forge))
		return;

	WAIT_STATE(ch, skill::lookup(skill::type::forge).beats);

	if (!IS_IMMORTAL(ch) && number_percent() > (get_skill_level(ch, skill::type::forge) + material->value[0])) {
		stc("You fail to forge a useful weapon.\n", ch);
		act("$n tries but fails to forge a useful weapon.\n", ch, nullptr, nullptr, TO_ROOM);
		check_improve(ch, skill::type::forge, FALSE, 1);
		destroy_obj(material);
		return;
	}

	obj = create_object(Game::world().get_obj_prototype(OBJ_VNUM_WEAPON), 0);

	if (!obj) {
		Logging::bug("Memory error in do_forge.", 0);
		stc("An error occured while trying to forge.\n", ch);
		return;
	}

	obj->level = ch->level;
	obj->material = material->material;
	obj->condition = material->condition;

	obj->extra_flags = material->extra_flags;

	for (const affect::Affect *paf = affect::list_obj(material); paf; paf = paf->next)
		affect::copy_to_obj(obj, paf);

	obj->value[0] = get_weapon_type(type);
	Format::sprintf(buf, "%s %s", weapon_table[weapon_lookup(type)].name, argument.uncolor());
	obj->name = buf;
	Format::sprintf(sdesc, "%s{x", argument);
	obj->short_descr = sdesc;
	Format::sprintf(buf, "A %s is here, forged by %s's craftsmanship.", weapon_table[weapon_lookup(type)].name, ch->name);
	obj->description = buf;
	Format::sprintf(buf, "It is a marvellous %s, crafted from the finest %s around.\n"
	        "It was created in the Month of %s by %s %s\n"
	        "named %s.  Legend holds that this %s was a great weaponsmith.\n",
	        weapon_table[weapon_lookup(type)].name, obj->material,
	        ch->in_room->area().world.time.month_name(),
	        (ch->level > LEVEL_HERO) ? "an immortal" : (ch->level > 75) ? "a master" :
	        (ch->level > 50) ? "an experienced" : (ch->level > 25) ? "a young" :
	        "a newbie", guild_table[ch->guild].name, ch->name,
	        race_table[ch->race].name);
	ed = new ExtraDescr(obj->name, buf);
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
	Format::sprintf(buf, "%s %s into a formidable %s\ncalled \"%s\".\n",
	        "You quickly and skillfully forge",
	        material->short_descr,
	        weapon_table[weapon_lookup(type)].name,
	        obj->short_descr);
	stc(buf, ch);
	Format::sprintf(buf, "%s has forged a %s named \"%s\".", ch->name,
	        weapon_table[weapon_lookup(type)].name, obj->short_descr);
	act(buf, ch, obj, nullptr, TO_ROOM);
	destroy_obj(material);
	check_improve(ch, skill::type::forge, TRUE, 1);

	/* Charge player for forging -- Elrac */
	if (cost > 0) {
		if (anvil->value[2] == 0) { /* public */
			Format::sprintf(buf,
			        "You pay the community of Thera %d silver for the use of its anvil.\n",
			        cost);
			stc(buf, ch);
		}
		else {
			Format::sprintf(buf,
			        "You pay %d silver for the use of this private anvil.\n",
			        cost);
			stc(buf, ch);

			if (! is_owner) {
				owner = get_char_world(ch, anvil_owner_name(anvil), VIS_CHAR);

				if (owner != nullptr) {
					cost_gold = cost / 100;
					cost_silver = cost - 100 * cost_gold;
					Format::sprintf(buf, "%s pays you ", ch->name);

					if (cost_gold > 0) {
						Format::sprintf(costbuf, "%d gold ", cost_gold);
						buf += costbuf;

						if (cost_silver > 0) buf += "and ";
					}

					if (cost_silver > 0) {
						Format::sprintf(costbuf, "%d silver ", cost_silver);
						buf += costbuf;
					}

					buf += "for the use of your anvil.\n";
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
void do_engrave(Character *ch, String argument)
{
	Character *jeweler = nullptr;
	Object *weapon;
	ExtraDescr *ed;
	ExtraDescr *eng_desc;
	ExtraDescr *dflt_desc;
	char buf[10 * MAX_INPUT_LENGTH];
	char *pd;           /* ptr into desc text */
	char *pb;           /* ptr into buffer */
	char *l2, *ld, *la; /* line 2, line with dup author, line after dup author */
	int  lines;         /* number of lines */
	char c0;
	String dbuf;
	char *eng_line;

	if (!IS_IMMORTAL(ch) &&
	    (jeweler = get_mob_here(ch, "jeweler", VIS_CHAR)) == nullptr &&
	    (jeweler = get_mob_here(ch, "jeweller", VIS_CHAR)) == nullptr) {
		stc("You need a jeweler's engraving tools to engrave.\n", ch);
		stc("Find a jeweler and try again on his or her premises!\n", ch);
		return;
	}

	if ((weapon = get_eq_char(ch, WEAR_WIELD)) == nullptr) {
		stc("You must {Rwield{x a weapon in order to engrave it.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("What do you want to engrave on your weapon?\n", ch);
		return;
	}

	/* Imms have no length limit. Please don't overdo it! */
	/* If you change this, remember that the jeweler can == nullptr for imms! */
	if (!IS_IMMORTAL(ch) && argument.uncolor().size() > 75) {
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
	eng_desc = nullptr;
	dflt_desc = nullptr;

	String tbuf;
	one_argument(weapon->name, tbuf);
	strcpy(buf, tbuf);

	for (ed = weapon->extra_descr; ed; ed = ed->next) {
		if (dflt_desc == nullptr && ed->keyword.has_words(buf))
			dflt_desc = ed;

		if (eng_desc == nullptr && ed->keyword.has_words("engravings"))
			eng_desc = ed;
	}

	if (eng_desc == nullptr) {
		/* no engravings yet, build an empty extdesc */
		eng_desc = new ExtraDescr("engravings", "\n");
		eng_desc->next = weapon->extra_descr;
		weapon->extra_descr = eng_desc;
	}

	if (dflt_desc == nullptr) {
		/* no extdesc for wpn, build an empty one */
		dflt_desc = new ExtraDescr(
			weapon->name,
			!weapon->description.empty() ?
			  (dbuf += weapon->description + "\n" + eng_line) : eng_line);

		dflt_desc->next = weapon->extra_descr;
		weapon->extra_descr = dflt_desc;
	}
	else if (strstr(dflt_desc->description, eng_line) == nullptr) {
		/* add to existing extdesc */
		dbuf += dflt_desc->description;
		dbuf += eng_line;
		dflt_desc->description = dbuf;
		dbuf.erase();
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
	l2 = nullptr;
	ld = nullptr;
	la = nullptr;
	pb = buf;
	char tdesc[10*MIL];
	strcpy(tdesc, eng_desc->description);

	for (pd = tdesc; *pd;) {
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
		if (ld == nullptr &&
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
		else if (la == nullptr && ld != nullptr)
			la = pd;

		pb = buf;
	} /* end for chars in desc */

	/* assemble 'worthy' parts of the old desc into a buffer. */

	if (ld != nullptr) {
		c0 = *ld;
		*ld = '\0';
		dbuf += eng_desc->description;
		*ld = c0;
		dbuf += la;
	}
	else if (lines > 20)
		dbuf += l2;
	else
		dbuf += eng_desc->description;

	strftime(buf, 9, "[%m/%d] ", localtime(&Game::current_time));
	dbuf += buf;

	if (IS_NPC(ch)) {
		Format::sprintf(buf, "{Y%s{x, {Mcitizen of %s,", ch->short_descr,
		        ch->in_room && !ch->in_room->area().name.empty() ?
		        ch->in_room->area().name : "Thera");
	}
	else {
		Format::sprintf(buf, "{Y%s{W%s{x ", ch->name,
		        ch->pcdata && !ch->pcdata->title.empty() ?
		        ch->pcdata->title : "{M(adventurer of Thera){x");
	}

	dbuf += buf;
	Format::sprintf(buf, "engraved {Ythis{x:\n \"%s\".\n", argument);
	dbuf += buf;
	eng_desc->description = dbuf;
	stc("You have left a mark of duration upon your weapon.\n", ch);
	act("$n solemnly engraves $s weapon.", ch, nullptr, nullptr, TO_ROOM);
} /* end do_engrave() */

/* Change the looks on weddingrings */
void do_weddingring(Character *ch, String argument)
{
	Character *jeweler = nullptr;
	Object *ring;
	int price = 0;

	if (ch->desc->original != nullptr) {
		stc("Switch back to your real form!\n", ch);
		return;
	}

	if (!IS_IMMORTAL(ch) &&
	    (jeweler = get_mob_here(ch, "jeweler", VIS_CHAR)) == nullptr &&
	    (jeweler = get_mob_here(ch, "jeweller", VIS_CHAR)) == nullptr) {
		stc("You need a jeweler's engraving tools to engrave.\n", ch);
		stc("Find a jeweler and try again on his or her premises!\n", ch);
		return;
	}

	if ((ring = get_eq_char(ch, WEAR_WEDDINGRING)) == nullptr) {
		stc("You must {Rwear{x your wedding ring in order to have it changed.\n", ch);
		return;
	}

	String arg1;
	argument = one_argument(argument, arg1);

	if (arg1.empty() || argument.empty())
		goto help;

	if (!IS_IMMORTAL(ch))
		price = 2 * argument.uncolor().size();

	if (price > 100 && !IS_IMMORTAL(ch)) {
		stc("The jeweler exclaims 'That's too long! It'll never fit on such a tiny ring!\n", ch);
		return;
	}

	if ((ch->silver + ch->gold * 100) < price) {
		ptc(ch, "A description like that costs %d gold. And you don't have that much, do you?\n", price);
		return;
	}

	if (arg1.is_prefix_of("long")) {
		ptc(ch, "The long description of your weddingring is now:\n{x'%s{x'.\n", argument);
		ring->description = argument;
	}
	else if (arg1.is_prefix_of("short")) {
		ptc(ch, "The short description of your weddingring is now:\n{x'%s{x'.\n", argument);
		ring->short_descr = argument;
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

void do_lore(Character *ch, String argument)
{
	Object *obj;

	if (!get_skill_level(ch, skill::type::lore)) {
		stc("You aren't trained in the lore of items.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("What do you want to lore?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if ((obj = get_obj_carry(ch, arg)) == nullptr
	 && (obj = get_obj_here(ch, arg)) == nullptr) {
		stc("You do not have that item.\n", ch);
		return;
	}

	if (!prd_chance(&ch->skill_fails, get_skill_level(ch, skill::type::lore))) {
		act("You look at $p, but you can't find out any additional information.", ch, obj, nullptr, TO_CHAR);
		act("$n looks at $p but cannot find out anything.", ch, obj, nullptr, TO_ROOM);
		return;
	}
	else {
		act("$n studies $p, discovering all of its hidden powers.", ch, obj, nullptr, TO_ROOM);
		spell_identify(skill::type::lore, (4 * obj->level) / 3, ch, obj, TARGET_OBJ, get_evolution(ch, skill::type::lore));
		check_improve(ch, skill::type::lore, TRUE, 4);
	}
}

void do_autograph(Character *ch, String argument)
{
	Object *obj;
	char buf[MSL];

	String arg;
	one_argument(argument, arg);

	if ((obj = get_obj_carry(ch, arg)) == nullptr) {
		stc("You are not carrying that.\n", ch);
		return;
	}

	Format::sprintf(buf, "%s {c(autographed){x", obj->short_descr);
	obj->short_descr = buf;
	stc("You sign the card.\n", ch);
}

/*
This function allows a character to rename their pet.
-- Outsider
*/
void do_rename(Character *ch, String argument)
{
	Character *pet;
	char new_name[64];

	/* make sure we have a pet */
	if (! ch->pet) {
		stc("You have no pet.\n", ch);
		return;
	}

	/* We have to name it something.... */
	if (argument.empty()) {
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

	Format::sprintf(new_name, "%s %s", pet->name, argument);
	pet->name = new_name;
	stc("Your pet has now been named ", ch);
	stc(argument, ch);
	stc(".\n", ch);
}

