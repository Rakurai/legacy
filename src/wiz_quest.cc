/*************************************************
*                                                *
*               The Age of Legacy                *
*                                                *
* Based originally on ROM 2.4, tested, enhanced, *
* and maintained by the Legacy Team.  If that    *
* doesn't mean you, and you're stealing our      *
* code, at least tell us and boost our egos ;)   *
*************************************************/

/*************************************************
* IMM questor commands and related functions.    *
* Most of these are in the quest command group.  *
*************************************************/

#include <vector>

#include "act.hh"
#include "argument.hh"
#include "affect/Affect.hh"
#include "Area.hh"
#include "channels.hh"
#include "Character.hh"
#include "declare.hh"
#include "Descriptor.hh"
#include "ExtraDescr.hh"
#include "find.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "interp.hh"
#include "Logging.hh"
#include "macros.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "Object.hh"
#include "Player.hh"
#include "QuestArea.hh"
#include "random.hh"
#include "RoomPrototype.hh"
#include "String.hh"

void do_addapply(Character *ch, String argument)
{
	Object *obj;
	int affect_modify = 1, enchant_type, duration = -1;

	String arg1, arg2, arg3, arg4;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);
	argument = one_argument(argument, arg4);

	if (arg1.empty() || arg2.empty() || arg3.empty()) {
		stc("Syntax: addapply <object> <apply> <value> [duration]\n", ch);
		stc("  Applies can be of:\n", ch);
		stc("    hp mana stamina str dex int wis con chr\n", ch);
		stc("    age ac hitroll damroll saves\n", ch);
		return;
	}

	obj = get_obj_world(ch, arg1);

	if (obj == nullptr) {
		stc("No such object exists!\n", ch);
		return;
	}

	if (arg2 == "hp")          enchant_type = APPLY_HIT;
	else if (arg2 == "mana")        enchant_type = APPLY_MANA;
	else if (arg2 == "stamina")     enchant_type = APPLY_STAM;
	else if (arg2 == "str")         enchant_type = APPLY_STR;
	else if (arg2 == "dex")         enchant_type = APPLY_DEX;
	else if (arg2 == "int")         enchant_type = APPLY_INT;
	else if (arg2 == "wis")         enchant_type = APPLY_WIS;
	else if (arg2 == "con")         enchant_type = APPLY_CON;
	else if (arg2 == "chr")         enchant_type = APPLY_CHR;
	else if (arg2 == "sex")         enchant_type = APPLY_SEX;
	else if (arg2 == "age")         enchant_type = APPLY_AGE;
	else if (arg2 == "ac")          enchant_type = APPLY_AC;
	else if (arg2 == "hitroll")     enchant_type = APPLY_HITROLL;
	else if (arg2 == "damroll")     enchant_type = APPLY_DAMROLL;
	else if (arg2 == "saves")       enchant_type = APPLY_SAVES;
	else {
		stc("That apply is not possible!\n", ch);
		return;
	}

	if (!arg3.is_number()) {
		stc("Applies require a value.\n", ch);
		return;
	}

	if (atoi(arg3) < -30000 || atoi(arg3) > 30000) {
		stc("Value must be between -30000 and 30000.\n", ch);
		return;
	}

	affect_modify = atoi(arg3);

	if (arg4.empty())
		duration = -1;
	else if (!arg4.is_number())
		duration = -1;
	else if (atoi(arg4) < -1 || atoi(arg4) > 998)
		duration = -1;
	else
		duration = atoi(arg4);

	stc("Ok.\n", ch);

	affect::Affect af;
	af.where      = TO_OBJECT;
	af.type       = affect::none;
	af.level      = ch->level;
	af.duration   = duration;
	af.location   = enchant_type;
	af.modifier   = affect_modify;
	af.bitvector(0);
	af.evolution  = 1;
	affect::join_to_obj(obj, &af);
}

/* Morph Command by Lotus */
void do_morph(Character *ch, String argument)
{
	Character *victim;
	Character *morph;
	Character *mobile;

	String arg1;
	argument = one_argument(argument, arg1);

	if (arg1.empty()) {
		stc("Morph Whom?\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg1, VIS_PLR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (victim->desc == nullptr) {
		stc("You can only morph connected players.\n", ch);
		return;
	}

	if (victim->desc->original != nullptr) {
		stc("They are currently morphed already.\n", ch);
		return;
	}

	String arg2;
	argument = one_argument(argument, arg2);

	if (arg2.empty()) {
		stc("Morph them into what?\n", ch);
		return;
	}

	if ((morph = get_mob_world(ch, arg2, VIS_CHAR)) == nullptr) {
		stc("That mobile does not exist.\n", ch);
		return;
	}

	mobile = create_mobile(get_mob_index(morph->pIndexData->vnum));

	if (!mobile) {  /* Make sure it works. -- Outsider */
		Logging::bug("Memory error creating mob in do_morph().", 0);
		stc("Could not morph.\n", ch);
		return;
	}

	mobile->act_flags -= ACT_AGGRESSIVE;
	mobile->act_flags += PLR_COLOR;
	mobile->act_flags += ACT_MORPH;
	char_to_room(mobile, victim->in_room);
	do_switch(victim, mobile->name);
	char_from_room(victim);
	char_to_room(victim, get_room_index(ROOM_VNUM_LIMBO));
	stc("Successful Morph!\n", ch);
}

void do_rppaward(Character *ch, String argument)
{
	Character *victim;
	char buf[MAX_STRING_LENGTH];
	String output;
	Descriptor *d;
	int rppoint;

	if (argument.empty()) {
		stc("Name            RPPs\n", ch);
		stc("--------------------\n", ch);

		for (d = descriptor_list; d != nullptr; d = d->next) {
			if (!IS_PLAYING(d) || !can_see_char(ch, d->character))
				continue;

			victim = (d->original != nullptr) ? d->original : d->character;

			if (!can_see_char(ch, victim))
				continue;

			Format::sprintf(buf, "%-14s {V%5d{x\n", victim->name, victim->pcdata->rolepoints);
			output += buf;
		}

		page_to_char(output, ch);
		return;
	}

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1.empty() || arg2.empty() || argument.empty()) {
		stc("Syntax:\n", ch);
		stc("  rpp <award|deduct> <victim> <amount>\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg2, VIS_PLR)) == nullptr) {
		stc("Player not found.\n", ch);
		return;
	}

	if (!argument.is_number()) {
		stc("Value must be numeric.\n", ch);
		return;
	}

	rppoint = atoi(argument);

	if (rppoint < 0 || rppoint > 50) {
		stc("Value must be between 0 and 50.\n", ch);
		return;
	}

	if (arg1 == "award") {
		victim->pcdata->rolepoints += rppoint;
		ptc(ch, "You have awarded them %d role playing points.\n", rppoint);
		ptc(victim, "You have been awarded %d role playing points.\n", rppoint);
		Format::sprintf(buf, "%s awarded %d rpp to %s.", ch->name, rppoint, victim->name);
		wiznet(buf, ch, nullptr, 0, 0, 0);
		return;
	}

	if (arg1 == "deduct") {
		if (rppoint > victim->pcdata->rolepoints) {
			stc("They do not have enough role playing points for that.\n", ch);
			return;
		}

		victim->pcdata->rolepoints -= rppoint;
		Format::sprintf(buf, "%d role playing points have been deducted, %d remaining.\n",
		        rppoint, victim->pcdata->rolepoints);
		stc(buf, ch);
		stc(buf, victim);
		Format::sprintf(buf, "%s deducted %d rpp from %s [%d remaining].",
		        ch->name, rppoint, victim->name, victim->pcdata->rolepoints);
		wiznet(buf, ch, nullptr, WIZ_SECURE, WIZ_QUEST, GET_RANK(ch));
		return;
	}

	stc("Use 'deduct' or 'award'.\n", ch);
}

RoomPrototype *get_scatter_room(Character *ch)
{
	RoomPrototype *room;

	for (; ;) {
		room = get_room_index(number_range(0, 32767));

		if (room == nullptr
		    || room == ch->in_room
		    || !can_see_room(ch, room)
		    || room->area == Game::world().quest.area
		    || (room->area->min_vnum >= 24000      /* clanhall vnum ranges */
		        && room->area->min_vnum <= 26999)
		    || room->guild
		    || room->area->name == "Playpen"
		    || room->area->name == "IMM-Zone"
		    || room->area->name == "Limbo"
		    || room->area->name == "Eilyndrae"     /* hack to make eilyndrae and torayna cri unquestable */
		    || room->area->name == "Torayna Cri"
		    || room->area->name == "Battle Arenas"
		    || room->sector_type == SECT_ARENA
		    || GET_ROOM_FLAGS(room).has_any_of(
		              ROOM_MALE_ONLY
		              | ROOM_FEMALE_ONLY
		              | ROOM_PRIVATE
		              | ROOM_SOLITARY
		              | ROOM_NOQUEST
		              | ROOM_PET_SHOP))
			continue;

		return room;
	}

	return nullptr;
}

/* scatter all items in a room -- Montrey */
void do_scatter(Character *ch, String argument)
{
	Object *obj, *obj_next;
	RoomPrototype *room;
	bool scattered = FALSE;

	if (ch->in_room == nullptr)
		return;

	for (obj = ch->in_room->contents; obj != nullptr; obj = obj_next) {
		obj_next = obj->next_content;

		if (!CAN_WEAR(obj, ITEM_TAKE))
			continue;

		room = get_scatter_room(ch);
		obj_from_room(obj);
		obj_to_room(obj, room);
		scattered = TRUE;
	}

	if (!scattered)
		stc("You see nothing to scatter.\n", ch);
	else
		stc("Done.\n", ch);
}

void do_string(Character *ch, String argument)
{
	String buf;
	Character *victim;
	Object *obj;
	RoomPrototype *room;

	String type, arg1, arg2, arg3;
	argument = one_argument(argument, type);
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	arg3 = argument;

	if (type.empty() || arg1.empty() || arg2.empty() || arg3.empty()) {
		stc("Syntax:\n", ch);
		stc("  string char <name> <field> <string>\n", ch);
		stc("    fields: name short long desc title spec deity status spouse\n", ch);
		stc("  string obj  <name> <field> <string>\n", ch);
		stc("    fields: name short long material extended more\n", ch);
		stc("  string room <number> <field> <string>\n", ch);
		stc("    fields: name\n", ch);
		return;
	}

	if (type.is_prefix_of("character") || type.is_prefix_of("mobile")) {
		if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == nullptr) {
			stc("They aren't here.\n", ch);
			return;
		}

		if (IS_IMP(victim) && !IS_IMP(ch)) {
			stc("They wouln't like that.\n", ch);
			return;
		}

		/* string something */

		if (arg2.is_prefix_of("spouse")) {
			if (IS_NPC(victim)) {
				stc("Not a good idea.\n", ch);
				return;
			}

			if (arg3 == "none") {
				victim->pcdata->spouse.erase();
			}
			else {
				victim->pcdata->spouse = arg3;
			}

			Format::sprintf(buf, "%s's spouse has been changed to %s.\n", victim->name, arg3);
			stc(buf, ch);
			return;
		}

		if (arg2.is_prefix_of("name")) {
			if (!IS_NPC(victim)) {
				stc("You can't change a player's name!\n", ch);
				return;
			}

			Format::sprintf(buf, "%s's name has been changed to %s.\n", victim->name, arg3);
			victim->name = arg3;
			stc(buf, ch);
			return;
		}

		if (arg2.is_prefix_of("description")) {
			victim->description = arg3;
			Format::sprintf(buf, "%s's description has been changed to %s.\n", victim->name, arg3);
			stc(buf, ch);
			return;
		}

		if (arg2.is_prefix_of("short")) {
			victim->short_descr = arg3;
			Format::sprintf(buf, "%s's short description has been changed to %s.\n", victim->name, arg3);
			stc(buf, ch);
			return;
		}

		if (arg2.is_prefix_of("long")) {
			arg3 += "\n";
			victim->long_descr = arg3;
			Format::sprintf(buf, "%s's long description has been changed to %s", victim->name, arg3);
			stc(buf, ch);
			return;
		}

		if (arg2.is_prefix_of("title")) {
			if (IS_NPC(victim)) {
				stc("Mobiles don't have a title, silly!\n", ch);
				return;
			}

			set_title(victim, arg3);
			Format::sprintf(buf, "%s's title has been changed to %s.\n", victim->name, arg3);
			stc(buf, ch);
			return;
		}

		if (arg2.is_prefix_of("spec")) {
			if (!IS_NPC(victim)) {
				stc("Players don't have specfuns, silly!\n", ch);
				return;
			}

			if ((victim->spec_fun = spec_lookup(arg3)) == 0) {
				Format::sprintf(buf, "%s is not a valid spec fun.\n", arg3);
				stc(buf, ch);
				return;
			}

			Format::sprintf(buf, "%s's spec fun value set to %s.\n", victim->name, arg3);
			stc(buf, ch);
			return;
		}

		if (arg2.is_prefix_of("deity")) {
			if (IS_NPC(victim)) {
				stc("Mobiles are all atheists!\n", ch);
				return;
			}

			victim->pcdata->deity = arg3;
			Format::sprintf(buf, "%s's deity string has been changed to %s.\n", victim->name, arg3);
			stc(buf, ch);
			return;
		}

		if (arg2.is_prefix_of("status")) {
			if (IS_NPC(victim)) {
				stc("A mobile has no status to change!\n", ch);
				return;
			}

			victim->pcdata->status = arg3;
			Format::sprintf(buf, "%s's status string has been changed to %s.\n", victim->name, arg3);
			stc(buf, ch);
			return;
		}
	}

	if (type.is_prefix_of("object")) {
		/* string an obj */
		if ((obj = get_obj_carry(ch, arg1)) == nullptr
		    && (obj = get_obj_wear(ch, arg1)) == nullptr
		    && (obj = get_obj_list(ch, arg1, ch->in_room->contents)) == nullptr) {
			Format::sprintf(buf, "After searching the whole mud, you could not find %s.\n", arg1);
			stc(buf, ch);
			return;
		}

		if (arg2.is_prefix_of("name")) {
			Format::sprintf(buf, "%s is now known as %s.\n", obj->name, arg3);
			obj->name = arg3;
			stc(buf, ch);
			return;
		}

		if (arg2.is_prefix_of("short")) {
			Format::sprintf(buf, "%s's short description has been changed to %s.\n", obj->short_descr, arg3);
			obj->short_descr = arg3;
			stc(buf, ch);
			return;
		}

		if (arg2.is_prefix_of("long")) {
			Format::sprintf(buf, "%s's long description has been changed to %s.\n", obj->short_descr, arg3);
			obj->description = arg3;
			stc(buf, ch);
			return;
		}

		if (arg2.is_prefix_of("material")) {
			Format::sprintf(buf, "%s suddenly warps into %s as %s glows brightly.\n",
			        obj->material, arg3, obj->short_descr);
			obj->material = arg3;
			stc(buf, ch);
			return;
		}

		if (arg2 == "ed" || arg2.is_prefix_of("extended")) {
			ExtraDescr *ed;
			argument = one_argument(argument, arg3);

			if (argument.empty()) {
				stc("Syntax: set obj <object> ed <keyword> <string>\n", ch);
				return;
			}

			if (obj->extra_descr != nullptr) {
				ExtraDescr *ed_next;
				ExtraDescr *ed_prev = nullptr;

				for (ed = obj->extra_descr; ed != nullptr; ed = ed_next) {
					ed_next = ed->next;

					if (ed->keyword == arg3) {
						if (ed == obj->extra_descr)
							obj->extra_descr = ed_next;
						else
							ed_prev->next = ed_next;

						delete ed;
					}
					else
						ed_prev = ed;
				}
			}

			if (argument == "none")
				return;

			char desc[MIL];
			Format::sprintf(desc, "%s\n", argument);

			ed = new ExtraDescr(arg3, desc);
			ed->next            = obj->extra_descr;
			obj->extra_descr    = ed;
			ptc(ch, "Extended description set to:\n%s\n", arg3);
			return;
		}

		if (arg2.is_prefix_of("more")) {
			ExtraDescr *ed;
			buf.erase();
			argument = one_argument(argument, arg3);

			if (argument.empty()) {
				stc("Syntax: string obj <obj> more <keyword> <string>\n", ch);
				return;
			}

			if (obj->extra_descr != nullptr) {
				ExtraDescr *ed_next;
				ExtraDescr *ed_prev = nullptr;

				for (ed = obj->extra_descr; ed != nullptr; ed = ed_next) {
					ed_next = ed->next;

					if (ed->keyword == arg3) {
						buf = ed->description;

						if (ed == obj->extra_descr)
							obj->extra_descr = ed_next;
						else
							ed_prev->next = ed_next;

						delete ed;
					}
					else
						ed_prev = ed;
				}
			}

			buf += argument;
			buf += "\n";
			ed = new ExtraDescr(arg3, buf);
			ed->next            = obj->extra_descr;
			obj->extra_descr    = ed;
			ptc(ch, "Added to extended description:\n%s\n", arg3);
			return;
		}
	}

	/* Room Strings by Lotus */
	if (type.is_prefix_of("room")) {
		if ((room = get_room_index(atoi(arg1))) == nullptr) {
			Format::sprintf(buf, "Room %d does not exist.\n", atoi(arg1));
			stc(buf, ch);
			return;
		}

		if (arg2.is_prefix_of("name")) {
			room->name = arg3;
			Format::sprintf(buf, "Room %d's name has been changed to %s.\n",
			        atoi(arg1), arg3);
			stc(buf, ch);
			return;
		}
	}

	/* echo bad use message */
	do_string(ch, "");
} /* end do_string() */

void do_switch(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Character *victim;

	if (IS_NPC(ch) || ch->desc == nullptr) {
		stc("You're not a real live player, you cannot switch.\n", ch);
		return;
	}

	if (ch->desc->original != nullptr) {
		stc("You are currently switched already.\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		stc("Switch into whom?\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		stc("You can only switch into mobiles.\n", ch);
		return;
	}

	if (!is_room_owner(ch, victim->in_room) && ch->in_room != victim->in_room
	    &&  room_is_private(victim->in_room) && !IS_IMP(ch)) {
		stc("That character is in a private room.\n", ch);
		return;
	}

	if (victim->desc != nullptr) {
		stc("Sorry, Character in use.\n", ch);
		return;
	}

	Format::sprintf(buf, "$N has switched into: %s", victim->short_descr);
	/* 0 so it shows spell casters */
	wiznet(buf, ch, nullptr, WIZ_SWITCHES, 0, 0);
	ch->desc->character = victim;
	ch->desc->original  = ch;
	victim->desc        = ch->desc;
	ch->desc            = nullptr;

	/* change communications to match */
	victim->prompt = ch->prompt;

	victim->comm_flags = ch->comm_flags;
	victim->censor_flags = ch->censor_flags;        /* Montrey */
	victim->lines = ch->lines;
	stc("You have been morphed.\n", victim);
	return;
}

void do_return(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	RoomPrototype *location;

	if (ch->desc == nullptr)
		return;

	if (ch->desc->original == nullptr) {
		stc("You are not currently morphed.\n", ch);
		return;
	}

	stc("You return to your original body.\n", ch);

	ch->prompt.erase();

	Format::sprintf(buf, "$N has returned from: %s.", ch->short_descr);
	wiznet(buf, ch->desc->original, 0, WIZ_SWITCHES, WIZ_SECURE, GET_RANK(ch));

	if (ch->desc->character->act_flags.has(ACT_MORPH)) {
		if (ch->desc->character->in_room == nullptr)
			location = get_room_index(ROOM_VNUM_MORGUE);
		else
			location = ch->desc->character->in_room;

		char_from_room(ch->desc->original);
		char_to_room(ch->desc->original, location);
	}

	ch->desc->character       = ch->desc->original;
	ch->desc->original        = nullptr;
	ch->desc->character->desc = ch->desc;

	if (!IS_NPC(ch->desc->character))
		if (!ch->desc->character->pcdata->buffer.empty())
			stc("You have messages: Type 'replay'\n", ch);

	ch->desc                  = nullptr;

	if (ch->act_flags.has(ACT_MORPH) && ch->in_room != nullptr)
		extract_char(ch, TRUE);  /* Only if raw_kill didn't do it */

	return;
}

/* for future use */
bool setup_obj(Character *ch, Object *obj, String argument)
{
	String arg1;
	argument = one_argument(argument, arg1);

	switch (obj->item_type) {
	case ITEM_TOKEN:

	/*              {
	                        int type = 0;

	                        struct token_desc
	                        {
	                                char *keywords;
	                                char *short_desc;
	                        };

	                        static const struct token_desc tdesc_table[5] =
	                        {
	                                {
	                                        "token",
	                                        "a blank token"
	                                },
	                                {
	                                        "wild token",
	                                        "a blank wild token"
	                                },
	                                {
	                                        "trivia token",
	                                        "a blank trivia token"
	                                },
	                                {
	                                        "hidden token",
	                                        "a blank hidden token"
	                                },
	                                {
	                                        "quest token",
	                                        "a blank quest token"
	                                }
	                        };

	                        if (arg1.empty())                    type = TOKEN_NONE;
	                        else if (arg1.is_prefix_of("wild"))    type = TOKEN_WILD;
	                        else if (arg1.is_prefix_of("trivia"))  type = TOKEN_TRIVIA;
	                        else if (arg1.is_prefix_of("hidden"))  type = TOKEN_HIDDEN;
	                        else if (arg1.is_prefix_of("quest"))   type = TOKEN_QUEST;
	                        else
	                        {
	                                stc("Valid token types are quest, hidden, trivia, and wild,\n"
	                                    "or use without an argument for a generic token.\n", ch);
	                                return FALSE;
	                        }

	                        obj->name = tdesc_table[type].keywords;
	                        obj->short_descr = tdesc_table[type].short_desc;
	                        obj->value[0] = type;
	                        break;
	                } */
	case ITEM_LIGHT:
	case ITEM_JUKEBOX:
	case ITEM_SCROLL:
	case ITEM_WAND:
	case ITEM_STAFF:
	case ITEM_WEAPON:
	case ITEM_ARMOR:
	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_MONEY:
//	case ITEM_COACH:
	case ITEM_ANVIL:
	case ITEM_FURNITURE:
	case ITEM_TRASH:
	case ITEM_CONTAINER:
	case ITEM_DRINK_CON:
	case ITEM_KEY:
	case ITEM_FOOD:
	case ITEM_BOAT:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	case ITEM_FOUNTAIN:
	case ITEM_MAP:
	case ITEM_PBTUBE:
	case ITEM_PBGUN:
	case ITEM_MATERIAL:
	case ITEM_CLOTHING:
	case ITEM_PORTAL:
	case ITEM_TREASURE:
	case ITEM_WARP_STONE:
	case ITEM_GEM:
	case ITEM_JEWELRY:
	case ITEM_WEDDINGRING:
	case ITEM_WARP_CRYSTAL:
	default:
		break;
	}

	return TRUE;
}

void do_create(Character *ch, String argument)
{
	char buf[100];
	ObjectPrototype *pObjIndex;
	Object *obj;
	int x;

	if (!IS_IMMORTAL(ch))
		return;

	if (argument.empty()) {
		stc("Syntax:\n", ch);
		stc("  create obj <type>\n", ch);
		return;
	}

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1 == "object") {
		if (arg2.empty()) {
			stc("Valid item types are:\n", ch);

			for (x = 0; x < item_table.size(); x++)
				ptc(ch, "%s\n", item_table[x].name);

			return;
		}

		if (arg2 == "npc_corpse"
		    || arg2 == "pc_corpse") {
//		    || arg2 == "coach"
			stc("You cannot create one of those.\n", ch);
			return;
		}

		for (x = 0; x < item_table.size(); x++) {
			if (arg2 == item_table[x].name) {
				if ((pObjIndex = get_obj_index(item_table[x].type + 100)) == nullptr) {
					Format::sprintf(buf, "[create] Cannot find item vnum %d.\n", item_table[x].type + 100);
					Logging::bug(buf, 0);
					stc("That item seems to be missing.\n", ch);
					return;
				}

				obj = create_object(pObjIndex, 0);

				if (! obj) {
					Logging::bug("Memory error creating object in do_create.", 0);
					return;
				}

				if (!setup_obj(ch, obj, argument)) {
					extract_obj(obj);
					return;
				}

				obj_to_char(obj, ch);
				act("$n has created $p!", ch, obj, nullptr, TO_ROOM);
				stc("Success.\n", ch);
				return;
			}
		}

		/* not found, echo the syntax */
		do_create(ch, "obj");
		return;
	}
	/* no match, echo the syntax */
	do_create(ch, "");
	return;
}

/* This function lets an IMM award or deduct skill points from a player.
The syntax for the command is:
skillpoint <award|deduct> <player> <amount>

-- Outsider
*/
void do_skillpoint(Character *ch, String argument)
{
	Character *victim;
	sh_int new_points;
	char buffer[MAX_INPUT_LENGTH];

	String char_name, give_or_take;
	argument = one_argument(argument, give_or_take);
	argument = one_argument(argument, char_name);

	if ((give_or_take.empty()) || (char_name.empty()) || (argument.empty())) {
		stc("Syntax: skillpoint <award|deduct> <player> <amount>\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, char_name, VIS_PLR)) == nullptr) {
		stc("Player not found.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		stc("Immortals can handle their own skill points.\n", ch);
		return;
	}

	if (!argument.is_number()) {
		stc("Value must be numeric.\n", ch);
		return;
	}

	new_points = atoi(argument);

	/* Make sure we have a reasonable amount. */
	if ((new_points < 1) || (new_points > 50)) {
		stc("Please give a value from 1 to 50.\n", ch);
		return;
	}

	/* First check for adding skill points. */
	if (!strcmp(give_or_take, "award")) {
		victim->pcdata->skillpoints += new_points;
		Format::sprintf(buffer, "%d skill points awarded to %s. Total: %d.\n",
		        new_points, victim->name, victim->pcdata->skillpoints);
		stc(buffer, ch);
		stc(buffer, victim);
	}
	/* Remove skill points from player. */
	else if (!strcmp(give_or_take, "deduct")) {
		if (victim->pcdata->skillpoints < new_points) {
			stc("That player does not have that many skill points.\n", ch);
			return;
		}
		else {
			victim->pcdata->skillpoints -= new_points;
			Format::sprintf(buffer, "%d skill points deducted from %s. %d remaining.\n",
			        new_points, victim->name, victim->pcdata->skillpoints);
			stc(buffer, ch);
			stc(buffer, victim);
		}
	}
	/* Must have mis-typed command. */
	else
		stc("Syntax: skillpoint <award|deduct> <player> <amount>\n", ch);
}

