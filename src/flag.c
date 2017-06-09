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

#include "Game.hpp"
#include "Area.hpp"
#include "find.h"
#include "merc.h"
#include "tables.h"
#include "recycle.h"
#include "Format.hpp"

void do_flag(Character *ch, String argument)
{
	char what[MIL];
	Character *victim = nullptr;
	Object *object;
	RoomPrototype *room;
	Flags *flag, old, nw, marked;
	int pos, fieldptr;
	char type;

	String arg1, arg2, arg3, word;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);
	type = argument[0];

	if (type == '=' || type == '-' || type == '+')
		argument = one_argument(argument, word);

	if (arg1.empty()) {
		stc("Syntax:\n"
		    "  flag char   <name> <field> {c(+,-,=){x <flags>\n"
		    "  flag mob    <name> <field> {c(+,-,=){x <flags>\n"
		    "  flag player <name> <field> {c(+,-,=){x <flags>\n"
		    "  flag obj    <name> <field> {c(+,-,=){x <flags>\n"
		    "  flag room   <vnum> <field> {c(+,-,=){x <flags>\n"
		    "\n"
		    "  +: add flag, -: remove flag, = remove all except\n"
		    "  Leave out to toggle the flags listed.\n"
		    "\n"
		    "  Use 'flaglist' to see possible fields and flags.\n"
		    "  Use 'flagsearch' to find things with certain flags.\n", ch);
		return;
	}

	if (arg2.empty()) {
		stc("What do you wish to set flags on?\n", ch);
		return;
	}

	if (arg3.empty()) {
		stc("You need to specify a field to modify.\n", ch);
		return;
	}

	for (fieldptr = 0; fieldptr < flag_fields.size(); fieldptr++)
		if (arg3.is_prefix_of(flag_fields[fieldptr].name))
			break;

	if (fieldptr == flag_fields.size()) {
		stc("That is not a valid flag field.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("Which flags do you wish to change?\n", ch);
		return;
	}

	if (arg1.is_prefix_of("mobile") || arg1.is_prefix_of("character") || arg1.is_prefix_of("player")) {
		if (flag_fields[fieldptr].cand != CAND_CHAR
		    && flag_fields[fieldptr].cand != CAND_MOB
		    && flag_fields[fieldptr].cand != CAND_PLAYER) {
			stc("That is not a character field.\n", ch);
			return;
		}

		if (arg1.is_prefix_of("mobile"))
			victim = get_mob_world(ch, arg2, VIS_CHAR);
		else if (arg1.is_prefix_of("player"))
			victim = get_player_world(ch, arg2, VIS_PLR);
		else
			victim = get_char_world(ch, arg2, VIS_CHAR);

		if (victim == nullptr) {
			stc("You can't find them.\n", ch);
			return;
		}

		if (IS_NPC(victim)) {
			if (flag_fields[fieldptr].cand == CAND_PLAYER) {
				stc("That field may not be changed on mobiles.\n", ch);
				return;
			}

			if (GET_RANK(ch) < flag_fields[fieldptr].mod_mob) {
				stc("You are not high enough level to change that field on mobiles.\n", ch);
				return;
			}

			switch (fieldptr) {
			case FIELD_ACT:         flag = &victim->act_flags;            break;
			case FIELD_OFF:         flag = &victim->off_flags;      break;
			case FIELD_FORM:        flag = &victim->form_flags;           break;
			case FIELD_PART:        flag = &victim->parts_flags;          break;
			case FIELD_COMM:        flag = &victim->comm_flags;           break;
			case FIELD_CENSOR:      flag = &victim->censor_flags;         break;
			case FIELD_REVOKE:      flag = &victim->revoke_flags;         break;

			default:
				ptc(ch, "That is not an acceptable %s flag.\n", arg1);
				return;
			}
		}
		else {
			if (flag_fields[fieldptr].cand == CAND_MOB) {
				stc("That field may not be changed on players.\n", ch);
				return;
			}

			if (GET_RANK(ch) < flag_fields[fieldptr].mod_plr) {
				stc("You are not high enough level to change that field on players.\n", ch);
				return;
			}

			switch (fieldptr) {
			case FIELD_PLAYER:      flag = &victim->act_flags;            break;
			case FIELD_PCDATA:      flag = &victim->pcdata->plr_flags;    break;
			case FIELD_WIZNET:      flag = &victim->wiznet_flags;         break;
			case FIELD_CGROUP:      flag = &victim->pcdata->cgroup_flags; break;
			case FIELD_COMM:        flag = &victim->comm_flags;           break;
			case FIELD_CENSOR:      flag = &victim->censor_flags;         break;
			case FIELD_REVOKE:      flag = &victim->revoke_flags;         break;

			default:
				ptc(ch, "That is not an acceptable %s flag.\n", arg1);
				return;
			}
		}

		Format::sprintf(what, "%s", PERS(victim, ch, VIS_PLR));
	}
	else if (arg1.is_prefix_of("obj")) {
		if (flag_fields[fieldptr].cand != CAND_OBJ) {
			stc("That is not an object field.\n", ch);
			return;
		}

		if (GET_RANK(ch) < flag_fields[fieldptr].mod_mob) {
			stc("You are not high enough level to change that field.\n", ch);
			return;
		}

		if ((object = get_obj_world(ch, arg2)) == nullptr) {
			stc("You can't find it.\n", ch);
			return;
		}

		Format::sprintf(what, "%s", object->name);

		switch (fieldptr) {
		case FIELD_EXTRA:       flag = &object->extra_flags;    break;

		case FIELD_WEAR:        flag = &object->wear_flags;     break;
/* temporarily removed until I can figure out how I want to do it
		case FIELD_WEAPON:
			if (object->item_type != ITEM_WEAPON) {
				stc("That is not a weapon.\n", ch);
				return;
			}

			flag = (unsigned long *) & (object->value[4]);            break;
*/
		default:
			stc("That's not an acceptable object flag.\n", ch);
			return;
		}
	}
	else if (arg1.is_prefix_of("room")) {
		if (flag_fields[fieldptr].cand != CAND_ROOM) {
			stc("That is not a room field.\n", ch);
			return;
		}

		if (GET_RANK(ch) < flag_fields[fieldptr].mod_mob) {
			stc("You are not high enough level to change that field.\n", ch);
			return;
		}

		if (!arg2.is_number()) {
			stc("Use the room's vnum.\n", ch);
			return;
		}

		if ((room = get_room_index(atoi(arg2))) == nullptr) {
			ptc(ch, "Room %d does not exist.\n", atoi(arg1));
			return;
		}

		Format::sprintf(what, "%s", room->name);

		if (fieldptr == FIELD_ROOM)     flag = &room->room_flags;
		else {
			stc("That's not an acceptable room flag.\n", ch);
			return;
		}
	}
	else {
		stc("Please specify : mob, char, player, obj or room.\n", ch);
		return;
	}

	old = *flag;
	const std::vector<flag_type>& flag_table = flag_fields[fieldptr].flag_table;

	if (type != '=')
		nw = old;

	/* mark the words */
	for (; ;) {
		argument = one_argument(argument, word);

		if (word.empty())
			break;

		if (word.size() == 1) { // alpha flag
			Flags flag(word);

			if (flag.empty()) {
				stc("That flag doesn't exist!\n", ch);
				return;
			}

			marked += flag;
		}

		pos = flag_index_lookup(word, flag_table);

		if (pos == -1) {
			stc("That flag doesn't exist!\n", ch);
			return;
		}
		else
			marked += flag_table[pos].bit;
	}

	for (pos = 0; pos < flag_table.size(); pos++) {
		if (!flag_table[pos].settable && old.has(flag_table[pos].bit)) {
			nw += flag_table[pos].bit;
			continue;
		}

		if (marked.has(flag_table[pos].bit)) {
			switch (type) {
			case '=':
			case '+':
				nw += flag_table[pos].bit;
				ptc(ch, "%s %s bit set on %s.\n",
				    flag_table[pos].name, arg3, what);
				break;

			case '-':
				nw -= flag_table[pos].bit;
				ptc(ch, "%s %s bit removed from %s.\n",
				    flag_table[pos].name, arg3, what);
				break;

			default:
				if (nw.has(flag_table[pos].bit)) {
					nw -= flag_table[pos].bit;
					ptc(ch, "%s %s bit removed from %s.\n",
					    flag_table[pos].name, arg3, what);
				}
				else {
					nw += flag_table[pos].bit;
					ptc(ch, "%s %s bit set on %s.\n",
					    flag_table[pos].name, arg3, what);
				}

				break;
			}
		}
	}

	*flag = nw;
}

void do_typelist(Character *ch, String argument)
{
	int x;

	if (argument.empty()) {
		stc("Valid lists: liquid, attack\n", ch);
		return;
	}

	if (argument.is_prefix_of("liquid")) {
		for (x = 0; x < liq_table.size(); x++)
			ptc(ch, "[%2d][%20s][%20s]\n", x, liq_table[x].name, liq_table[x].color);
	}
	else if (argument.is_prefix_of("attack")) {
		for (x = 0; x < attack_table.size(); x++)
			ptc(ch, "[%2d][%20s][%20s]\n", x, attack_table[x].name, attack_table[x].noun);
	}
	else
		stc("Valid lists: liquid, attack\n", ch);
}

void do_flaglist(Character *ch, String argument)
{
	int x;
	
	if (argument.empty()) {
		stc("Flag fields are:\n", ch);

		for (x = 0; x < flag_fields.size(); x++)
			if (GET_RANK(ch) >= flag_fields[x].see_mob
			    || GET_RANK(ch) >= flag_fields[x].see_plr)
				ptc(ch, "%-30s%s\n", flag_fields[x].name,
				    field_cand[flag_fields[x].cand]);

		return;
	}

	for (x = 0; x < flag_fields.size(); x++)
		if (GET_RANK(ch) >= flag_fields[x].see_mob
		    || GET_RANK(ch) >= flag_fields[x].see_plr)
			if (argument.is_prefix_of(flag_fields[x].name))
				break;

	if (x >= flag_fields.size()) {
		stc("There is no such field.\n", ch);
		return;
	}

	ptc(ch, "Flags in the %s field:\n", flag_fields[x].name);

	const std::vector<flag_type>& flag_table = flag_fields[x].flag_table;

	for (x = 0; x < flag_table.size(); x++)
		ptc(ch, "[%2s] %s\n", Flags(flag_table[x].bit).to_string(), flag_table[x].name);
}

/*** FLAG SEARCHING ***/

int fsearch_player(Character *ch, int fieldptr, const Flags& marked)
{
	char buf[MSL];
	String output;
	Character *victim;
	Player *vpc;
	int count = 0;
	Flags flag;
	output += "{VCount {YRoom{x\n";

	for (vpc = pc_list; vpc != nullptr; vpc = vpc->next) {
		if ((victim = vpc->ch) == nullptr
		    || IS_NPC(victim)
		    || victim->in_room == nullptr
		    || !can_see_char(ch, victim)
		    || !can_see_room(ch, victim->in_room))
			continue;

		/* take care of flag comparison first */
		switch (fieldptr) {
		case FIELD_PLAYER:      flag = victim->act_flags;             break;

		case FIELD_PCDATA:      flag = victim->pcdata->plr_flags;     break;

		case FIELD_WIZNET:      flag = victim->wiznet_flags;          break;

		case FIELD_CGROUP:      flag = victim->pcdata->cgroup_flags;  break;

		case FIELD_COMM:        flag = victim->comm_flags;            break;

		case FIELD_CENSOR:      flag = victim->censor_flags;          break;

		case FIELD_REVOKE:      flag = victim->revoke_flags;          break;

		default:                                                return 0;
		}

		if (!flag.has_all_of(marked))
			continue;

		if (++count > 500)
			continue;

		Format::sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{x %s{x.\n",
		        count,
		        victim->in_room->vnum,
		        victim->short_descr);
		output += buf;
	}

	if (count > 0)
		page_to_char(output, ch);

	return count;
}

int fsearch_mobile(Character *ch, int fieldptr, const Flags& marked)
{
	char buf[MSL];
	String output;
	Character *victim;
	int count = 0;
	Flags flag;
	output += "{VCount  {YRoom   {GMob{x\n";

	for (victim = char_list; victim != nullptr; victim = victim->next) {
		if (!IS_NPC(victim)
		    || victim->in_room == nullptr
		    || !can_see_char(ch, victim)
		    || !can_see_room(ch, victim->in_room))
			continue;

		/* take care of flag comparison first */
		switch (fieldptr) {
		case FIELD_ACT:         flag = victim->act_flags;             break;

		case FIELD_OFF:         flag = victim->off_flags;       break;

		case FIELD_FORM:        flag = victim->form_flags;            break;

		case FIELD_PART:        flag = victim->parts_flags;           break;

		case FIELD_COMM:        flag = victim->comm_flags;            break;

		case FIELD_CENSOR:      flag = victim->censor_flags;          break;

		case FIELD_REVOKE:      flag = victim->revoke_flags;          break;

		default:                                                return 0;
		}

		if (!flag.has_all_of(marked))
			continue;

		if (++count > 500)
			continue;

		Format::sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{H[{G%5d{H]{x %s{x.\n",
		        count,
		        victim->in_room->vnum,
		        victim->pIndexData->vnum,
		        victim->short_descr);
		output += buf;
	}

	if (count > 0)
		page_to_char(output, ch);

	return count;
}

void fsearch_char(Character *ch, int fieldptr, const Flags& marked, bool mobile, bool player)
{
	char buf[MSL];
	String output;
	int mobilecount = 0, playercount = 0;

	if (mobile)     mobilecount = fsearch_mobile(ch, fieldptr, marked);

	if (player)     playercount = fsearch_player(ch, fieldptr, marked);


	if (mobile) {
		if (mobilecount == 0)
			Format::sprintf(buf, "You found no mobiles matching your search criteria.\n");
		else
			Format::sprintf(buf, "You found %d matching mobile%s%s.\n",
			        mobilecount,
			        mobilecount > 1 ? "s" : "",
			        mobilecount > 500 ? ", of which 500 are shown" : "");

		output += buf;
	}

	if (player) {
		if (playercount == 0)
			Format::sprintf(buf, "You found no players matching your search criteria.\n");
		else
			Format::sprintf(buf, "You found %d matching player%s%s.\n",
			        playercount,
			        playercount > 1 ? "s" : "",
			        playercount > 500 ? ", of which 500 are shown" : "");

		output += buf;
	}

	page_to_char(output, ch);
}

void fsearch_room(Character *ch, int fieldptr, const Flags& marked)
{
	char buf[MSL];
	String output;
	RoomPrototype *room;
	int count = 0, vnum;
	Flags flag;
	output += "{VCount {GVnum{x\n";

	for (Area *area: Game::world().areas) {
		for (vnum = area->min_vnum; vnum <= area->max_vnum; vnum++) {
			if ((room = get_room_index(vnum)) == nullptr
			    || !can_see_room(ch, room))
				continue;

			switch (fieldptr) {
			case FIELD_ROOM:        flag = GET_ROOM_FLAGS(room);        break;

			default:                                                return;
			}

			if (!flag.has_all_of(marked))
				continue;

			if (++count > 500)
				continue;

			Format::sprintf(buf, "{M[{V%3d{M]{H[{G%5d{H]{x %s{x.\n",
			        count,
			        vnum,
			        room->name);
			output += buf;
		}
	}

	if (count == 0)
		stc("You found no rooms matching your search criteria.\n", ch);
	else {
		Format::sprintf(buf, "You found %d matching room%s%s.\n",
		        count,
		        count > 1 ? "s" : "",
		        count > 500 ? ", of which 500 are shown" : "");
		output += buf;
		page_to_char(output, ch);
	}

}

void fsearch_obj(Character *ch, int fieldptr, const Flags& marked)
{
	char buf[MSL];
	String output;
	Object *obj, *in_obj;
	int count = 1;
	Flags flag;
	output += "{VCount {YRoom  {GObject{x\n";

	/* cut off list at 400 objects, to prevent spamming out your link */
	for (obj = object_list; obj != nullptr; obj = obj->next) {
		/* take care of flag comparison first */
		switch (fieldptr) {
		case FIELD_EXTRA:       flag = obj->extra_flags;        break;

		case FIELD_WEAR:        flag = obj->wear_flags;         break;

		case FIELD_WEAPON:      flag = obj->value[0];           break;

		default:                                                return;
		}

		if (!flag.has_all_of(marked))
			continue;

		for (in_obj = obj; in_obj->in_obj != nullptr; in_obj = in_obj->in_obj)
			;

		if (in_obj->carried_by) {
			if (in_obj->carried_by->in_room == nullptr
			    || !can_see_room(ch, in_obj->carried_by->in_room)
			    || !can_see_char(ch, in_obj->carried_by))
				continue;

			Format::sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{H[{G%5d{H]{x %s{x is carried by %s.\n",
			        count,
			        in_obj->carried_by->in_room->vnum,
			        obj->pIndexData->vnum,
			        obj->short_descr,
			        PERS(in_obj->carried_by, ch, VIS_PLR));
		}
		else if (in_obj->in_locker) {
			if (in_obj->in_locker->in_room == nullptr
			    || !can_see_room(ch, in_obj->in_locker->in_room)
			    || !can_see_char(ch, in_obj->in_locker))
				continue;

			Format::sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{H[{G%5d{H]{x %s{x is in %s's locker.\n",
			        count,
			        in_obj->in_locker->in_room->vnum,
			        obj->pIndexData->vnum,
			        obj->short_descr,
			        PERS(in_obj->in_locker, ch, VIS_PLR));
		}
		else if (in_obj->in_strongbox) {
			if (in_obj->in_strongbox->in_room == nullptr
			    || !can_see_room(ch, in_obj->in_strongbox->in_room)
			    || !can_see_char(ch, in_obj->in_strongbox))
				continue;

			Format::sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{H[{G%5d{H]{x %s{x is in %s's strongbox.\n",
			        count,
			        in_obj->in_strongbox->in_room->vnum,
			        obj->pIndexData->vnum,
			        obj->short_descr,
			        PERS(in_obj->in_strongbox, ch, VIS_PLR));
		}
		else if (in_obj->in_room) {
			if (!can_see_room(ch, in_obj->in_room))
				continue;

			Format::sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{H[{G%5d{H]{x %s{x in %s.\n",
			        count,
			        in_obj->in_room->vnum,
			        obj->pIndexData->vnum,
			        obj->short_descr,
			        in_obj->in_room->name);
		}
		else    /* what's left? */
			continue;

		if (++count <= 500)     /* count stays one ahead of actual number found */
			output += buf;
	}

	if (--count == 0)
		stc("You found no items matching your search criteria.\n", ch);
	else {
		Format::sprintf(buf, "You found %d matching item%s%s.\n",
		        count,
		        count > 1 ? "s" : "",
		        count > 500 ? ", of which 500 are shown" : "");
		output += buf;
		page_to_char(output, ch);
	}

}

void do_flagsearch(Character *ch, String argument)
{
	int fieldptr;
	Flags marked;
	long pos;
	bool player = TRUE, mobile = TRUE, toolowmobile = FALSE, toolowplayer = FALSE;

	String arg1, arg2, word;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1.empty()) {
		stc("Syntax:\n"
		    "  flagsearch char   <field> <flags>\n"
		    "  flagsearch mob    <field> <flags>\n"
		    "  flagsearch player <field> <flags>\n"
		    "  flagsearch obj    <field> <flags>\n"
		    "  flagsearch room   <field> <flags>\n"
		    "\n"
		    "  Specifying more than one flag returns results\n"
		    "  that have ALL flags set.\n"
		    "\n"
		    "  Use 'flaglist' to see fields and flags.\n"
		    "  Use 'flag' to set flags.\n", ch);
		return;
	}

	if (arg2.empty()) {
		stc("You must specify a field of flags to search for.\n", ch);
		return;
	}

	for (fieldptr = 0; fieldptr < flag_fields.size(); fieldptr++)
		if (arg2.is_prefix_of(flag_fields[fieldptr].name))
			break;

	if (fieldptr >= flag_fields.size()) {
		stc("That is not a valid flag field.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("You must specify at least one flag to search for.\n", ch);
		return;
	}

	if (arg1.is_prefix_of("mobile")
	    || arg1.is_prefix_of("player")
	    || arg1.is_prefix_of("character")) {
		if (flag_fields[fieldptr].cand != CAND_CHAR
		    && flag_fields[fieldptr].cand != CAND_MOB
		    && flag_fields[fieldptr].cand != CAND_PLAYER) {
			stc("That is not a character field.\n", ch);
			return;
		}

		if (arg1.is_prefix_of("mobile"))                       player = FALSE;

		if (arg1.is_prefix_of("player"))                       mobile = FALSE;

		if (GET_RANK(ch) < flag_fields[fieldptr].see_mob)       toolowmobile = TRUE;

		if (GET_RANK(ch) < flag_fields[fieldptr].see_plr)       toolowplayer = TRUE;

		if (toolowmobile && toolowplayer) {
			stc("You are not high enough level to search for that field.\n", ch);
			return;
		}

		if (!player) {          /* mob only */
			if (flag_fields[fieldptr].cand == CAND_PLAYER) {
				stc("Mobiles do not have that field.\n", ch);
				return;
			}

			if (toolowmobile) {
				stc("You are not high enough level to search for that field on mobiles.\n", ch);
				return;
			}
		}
		else if (!mobile) {     /* player only */
			if (flag_fields[fieldptr].cand == CAND_MOB) {
				stc("Players do not have that field.\n", ch);
				return;
			}

			if (toolowplayer) {
				stc("You are not high enough level to search for that field on players.\n", ch);
				return;
			}
		}
		else {                  /* all chars */
			if ((flag_fields[fieldptr].cand == CAND_MOB    && toolowmobile)
			    || (flag_fields[fieldptr].cand == CAND_PLAYER && toolowplayer)) {
				stc("You are not high enough level to search for that field.\n", ch);
				return;
			}
		}

		switch (fieldptr) {
		case FIELD_PLAYER:
		case FIELD_PCDATA:
		case FIELD_CGROUP:
		case FIELD_WIZNET:      mobile = FALSE;         break;

		case FIELD_ACT:
		case FIELD_OFF:
		case FIELD_FORM:
		case FIELD_PART:        player = FALSE;         break;

		case FIELD_REVOKE:
		case FIELD_CENSOR:
		case FIELD_COMM:                                break;

		default:        /* just in case */
			stc("That is not a character field.\n", ch);
			return;
		}
	}
	else if (arg1.is_prefix_of("obj")) {
		if (flag_fields[fieldptr].cand == CAND_OBJ) {
			if (GET_RANK(ch) < flag_fields[fieldptr].see_mob) {
				stc("You are not high enough level to search for that field.\n", ch);
				return;
			}
		}
		else {
			stc("That is not an object field.\n", ch);
			return;
		}

		switch (fieldptr) {
		case FIELD_EXTRA:
		case FIELD_WEAR:
		case FIELD_WEAPON:
			break;

		default:
			stc("That's not an acceptable object flag.\n", ch);
			return;
		}
	}
	else if (arg1.is_prefix_of("room")) {
		if (flag_fields[fieldptr].cand == CAND_ROOM) {
			if (GET_RANK(ch) < flag_fields[fieldptr].see_mob) {
				stc("You are not high enough level to search for that field.\n", ch);
				return;
			}
		}
		else {
			stc("That is not a room field.\n", ch);
			return;
		}

		switch (fieldptr) {
		case FIELD_ROOM:
			break;

		default:
			stc("That's not an acceptable room flag.\n", ch);
			return;
		}
	}
	else {
		stc("Please specify : mob, char, player, obj or room.\n", ch);
		return;
	}

	const std::vector<flag_type>& flag_table = flag_fields[fieldptr].flag_table;

	/* turn the argument into flags */
	for (; ;) {
		argument = one_argument(argument, word);

		if (word.empty())
			break;

		if (word.size() == 1) { // alpha flag
			Flags flag(word);

			if (flag.empty()) {
				stc("That flag doesn't exist!\n", ch);
				return;
			}

			marked += flag;
		}

		pos = flag_index_lookup(word, flag_table);

		if (pos == -1) {
			ptc(ch, "That is not an acceptable %s flag.\n", flag_fields[fieldptr].name);
			return;
		}
		else
			marked += flag_table[pos].bit;
	}

	/* search the mud */
	switch (flag_fields[fieldptr].cand) {
	case CAND_CHAR:
	case CAND_MOB:
	case CAND_PLAYER:       fsearch_char(ch, fieldptr, marked, mobile, player);     break;

	case CAND_OBJ:          fsearch_obj(ch, fieldptr, marked);                      break;

	case CAND_ROOM:         fsearch_room(ch, fieldptr, marked);                     break;

	default:                                                                        break;
	}
}

