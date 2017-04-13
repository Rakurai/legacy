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

#include "merc.h"
#include "tables.h"
#include "recycle.h"

extern AREA_DATA *area_first;

void do_flag(CHAR_DATA *ch, const char *argument)
{
	char arg1[MIL], arg2[MIL], arg3[MIL];
	char word[MIL], what[MIL];
	CHAR_DATA *victim = NULL;
	OBJ_DATA *object;
	ROOM_INDEX_DATA *room;
	unsigned long *flag;
	int old = 0, new = 0, marked = 0, pos, fieldptr, length;
	char type;
	const struct flag_type *flag_table;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);
	type = argument[0];

	if (type == '=' || type == '-' || type == '+')
		argument = one_argument(argument, word);

	if (arg1[0] == '\0') {
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

	if (arg2[0] == '\0') {
		stc("What do you wish to set flags on?\n", ch);
		return;
	}

	if (arg3[0] == '\0') {
		stc("You need to specify a field to modify.\n", ch);
		return;
	}

	for (fieldptr = 0; flag_fields[fieldptr].name != NULL; fieldptr++)
		if (!str_prefix1(arg3, flag_fields[fieldptr].name))
			break;

	if (flag_fields[fieldptr].name == NULL) {
		stc("That is not a valid flag field.\n", ch);
		return;
	}

	if (argument[0] == '\0') {
		stc("Which flags do you wish to change?\n", ch);
		return;
	}

	if (!str_prefix1(arg1, "mobile") || !str_prefix1(arg1, "character") || !str_prefix1(arg1, "player")) {
		if (flag_fields[fieldptr].cand != CAND_CHAR
		    && flag_fields[fieldptr].cand != CAND_MOB
		    && flag_fields[fieldptr].cand != CAND_PLAYER) {
			stc("That is not a character field.\n", ch);
			return;
		}

		if (!str_prefix1(arg1, "mobile"))
			victim = get_mob_world(ch, arg2, VIS_CHAR);
		else if (!str_prefix1(arg1, "player"))
			victim = get_player_world(ch, arg2, VIS_PLR);
		else
			victim = get_char_world(ch, arg2, VIS_CHAR);

		if (victim == NULL) {
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
			case FIELD_ACT:         flag = &victim->act;            break;
			case FIELD_OFF:         flag = &victim->off_flags;      break;
			case FIELD_FORM:        flag = &victim->form;           break;
			case FIELD_PART:        flag = &victim->parts;          break;
			case FIELD_COMM:        flag = &victim->comm;           break;
			case FIELD_CENSOR:      flag = &victim->censor;         break;
			case FIELD_REVOKE:      flag = &victim->revoke;         break;

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
			case FIELD_PLAYER:      flag = &victim->act;            break;

			case FIELD_PCDATA:      flag = &victim->pcdata->plr;    break;

			case FIELD_WIZNET:      flag = &victim->wiznet;         break;

			case FIELD_CGROUP:      flag = &victim->pcdata->cgroup; break;

			case FIELD_COMM:        flag = &victim->comm;           break;

			case FIELD_CENSOR:      flag = &victim->censor;         break;

			case FIELD_REVOKE:      flag = &victim->revoke;         break;

			default:
				ptc(ch, "That is not an acceptable %s flag.\n", arg1);
				return;
			}
		}

		sprintf(what, "%s", PERS(victim, ch, VIS_PLR));
	}
	else if (!str_prefix1(arg1, "obj")) {
		if (flag_fields[fieldptr].cand != CAND_OBJ) {
			stc("That is not an object field.\n", ch);
			return;
		}

		if (GET_RANK(ch) < flag_fields[fieldptr].mod_mob) {
			stc("You are not high enough level to change that field.\n", ch);
			return;
		}

		if ((object = get_obj_world(ch, arg2)) == NULL) {
			stc("You can't find it.\n", ch);
			return;
		}

		sprintf(what, "%s", object->name);

		switch (fieldptr) {
		case FIELD_EXTRA:       flag = &object->extra_flags;    break;

		case FIELD_WEAR:        flag = &object->wear_flags;     break;

		case FIELD_WEAPON:
			if (object->item_type != ITEM_WEAPON) {
				stc("That is not a weapon.\n", ch);
				return;
			}

			flag = (unsigned long *) & (object->value[4]);            break;
		default:
			stc("That's not an acceptable object flag.\n", ch);
			return;
		}
	}
	else if (!str_prefix1(arg1, "room")) {
		if (flag_fields[fieldptr].cand != CAND_ROOM) {
			stc("That is not a room field.\n", ch);
			return;
		}

		if (GET_RANK(ch) < flag_fields[fieldptr].mod_mob) {
			stc("You are not high enough level to change that field.\n", ch);
			return;
		}

		if (!is_number(arg2)) {
			stc("Use the room's vnum.\n", ch);
			return;
		}

		if ((room = get_room_index(atoi(arg2))) == NULL) {
			ptc(ch, "Room %d does not exist.\n", atoi(arg1));
			return;
		}

		sprintf(what, "%s", room->name);

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
	flag_table = flag_fields[fieldptr].flag_table;

	if (type != '=')
		new = old;

	/* mark the words */
	for (; ;) {
		argument = one_argument(argument, word);

		if (word[0] == '\0')
			break;

		length = strlen(word);

		if (length <= 2) {              /* alpha flag? */
			char letter;

			if (length == 1) {
				letter = UPPER(word[0]);
				SET_BIT(marked, flag_convert(letter));
				continue;
			}
			else {
				letter = LOWER(word[0]);

				if (letter == LOWER(word[1])
				    && letter <= 'f'
				    && letter >= 'a') {
					SET_BIT(marked, flag_convert(letter));
					continue;
				}
			}
		}

		pos = flag_lookup(word, flag_table);

		if (pos == -1) {
			stc("That flag doesn't exist!\n", ch);
			return;
		}
		else
			SET_BIT(marked, flag_table[pos].bit);
	}

	for (pos = 0; flag_table[pos].name != NULL; pos++) {
		if (!flag_table[pos].settable && IS_SET(old, flag_table[pos].bit)) {
			SET_BIT(new, flag_table[pos].bit);
			continue;
		}

		if (IS_SET(marked, flag_table[pos].bit)) {
			switch (type) {
			case '=':
			case '+':
				SET_BIT(new, flag_table[pos].bit);
				ptc(ch, "%s %s bit set on %s.\n",
				    flag_table[pos].name, arg3, what);
				break;

			case '-':
				REMOVE_BIT(new, flag_table[pos].bit);
				ptc(ch, "%s %s bit removed from %s.\n",
				    flag_table[pos].name, arg3, what);
				break;

			default:
				if (IS_SET(new, flag_table[pos].bit)) {
					REMOVE_BIT(new, flag_table[pos].bit);
					ptc(ch, "%s %s bit removed from %s.\n",
					    flag_table[pos].name, arg3, what);
				}
				else {
					SET_BIT(new, flag_table[pos].bit);
					ptc(ch, "%s %s bit set on %s.\n",
					    flag_table[pos].name, arg3, what);
				}

				break;
			}
		}
	}

	*flag = new;
}

void do_typelist(CHAR_DATA *ch, const char *argument)
{
	int x;

	if (argument[0] == '\0') {
		stc("Valid lists: liquid, attack\n", ch);
		return;
	}

	if (!str_prefix1(argument, "liquid")) {
		for (x = 0; liq_table[x].liq_name != NULL; x++)
			ptc(ch, "[%2d][%20s][%20s]\n", x, liq_table[x].liq_name, liq_table[x].liq_color);
	}
	else if (!str_prefix1(argument, "attack")) {
		for (x = 0; attack_table[x].name != NULL; x++)
			ptc(ch, "[%2d][%20s][%20s]\n", x, attack_table[x].name, attack_table[x].noun);
	}
	else
		stc("Valid lists: liquid, attack\n", ch);
}

char *flag_to_alpha(long flag)
{
	int i;

	for (i = 0; ftoa_table[i].alpha != NULL; i++)
		if (flag == ftoa_table[i].flag)
			return ftoa_table[i].alpha;

	return NULL;
}

void do_flaglist(CHAR_DATA *ch, const char *argument)
{
	int x;
	const struct flag_type *flag_table;

	if (argument[0] == '\0') {
		stc("Flag fields are:\n", ch);

		for (x = 0; flag_fields[x].name != NULL; x++)
			if (GET_RANK(ch) >= flag_fields[x].see_mob
			    || GET_RANK(ch) >= flag_fields[x].see_plr)
				ptc(ch, "%-30s%s\n", flag_fields[x].name,
				    field_cand[flag_fields[x].cand]);

		return;
	}

	for (x = 0; flag_fields[x].name != NULL; x++)
		if (GET_RANK(ch) >= flag_fields[x].see_mob
		    || GET_RANK(ch) >= flag_fields[x].see_plr)
			if (!str_prefix1(argument, flag_fields[x].name))
				break;

	if (flag_fields[x].name == NULL) {
		stc("There is no such field.\n", ch);
		return;
	}

	flag_table = flag_fields[x].flag_table;
	ptc(ch, "Flags in the %s field:\n", flag_fields[x].name);

	for (x = 0; flag_table[x].name != NULL; x++)
		ptc(ch, "[%2s] %s\n", flag_to_alpha(flag_table[x].bit), flag_table[x].name);
}

/*** FLAG SEARCHING ***/

int fsearch_player(CHAR_DATA *ch, int fieldptr, long marked)
{
	char buf[MSL];
	BUFFER *output;
	CHAR_DATA *victim;
	PC_DATA *vpc;
	int count = 0;
	long flag;
	output = new_buf();
	add_buf(output, "{VCount {YRoom{x\n");

	for (vpc = pc_list; vpc != NULL; vpc = vpc->next) {
		if ((victim = vpc->ch) == NULL
		    || IS_NPC(victim)
		    || victim->in_room == NULL
		    || !can_see(ch, victim)
		    || !can_see_room(ch, victim->in_room))
			continue;

		/* take care of flag comparison first */
		switch (fieldptr) {
		case FIELD_PLAYER:      flag = victim->act;             break;

		case FIELD_PCDATA:      flag = victim->pcdata->plr;     break;

		case FIELD_WIZNET:      flag = victim->wiznet;          break;

		case FIELD_CGROUP:      flag = victim->pcdata->cgroup;  break;

		case FIELD_COMM:        flag = victim->comm;            break;

		case FIELD_CENSOR:      flag = victim->censor;          break;

		case FIELD_REVOKE:      flag = victim->revoke;          break;

		default:                                                return 0;
		}

		if ((marked & flag) != marked)
			continue;

		if (++count > 500)
			continue;

		sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{x %s{x.\n",
		        count,
		        victim->in_room->vnum,
		        victim->short_descr);
		add_buf(output, buf);
	}

	if (count > 0)
		page_to_char(buf_string(output), ch);

	free_buf(output);
	return count;
}

int fsearch_mobile(CHAR_DATA *ch, int fieldptr, long marked)
{
	char buf[MSL];
	BUFFER *output;
	CHAR_DATA *victim;
	int count = 0;
	long flag;
	output = new_buf();
	add_buf(output, "{VCount  {YRoom   {GMob{x\n");

	for (victim = char_list; victim != NULL; victim = victim->next) {
		if (!IS_NPC(victim)
		    || victim->in_room == NULL
		    || !can_see(ch, victim)
		    || !can_see_room(ch, victim->in_room))
			continue;

		/* take care of flag comparison first */
		switch (fieldptr) {
		case FIELD_ACT:         flag = victim->act;             break;

		case FIELD_OFF:         flag = victim->off_flags;       break;

		case FIELD_FORM:        flag = victim->form;            break;

		case FIELD_PART:        flag = victim->parts;           break;

		case FIELD_COMM:        flag = victim->comm;            break;

		case FIELD_CENSOR:      flag = victim->censor;          break;

		case FIELD_REVOKE:      flag = victim->revoke;          break;

		default:                                                return 0;
		}

		if ((marked & flag) != marked)
			continue;

		if (++count > 500)
			continue;

		sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{H[{G%5d{H]{x %s{x.\n",
		        count,
		        victim->in_room->vnum,
		        victim->pIndexData->vnum,
		        victim->short_descr);
		add_buf(output, buf);
	}

	if (count > 0)
		page_to_char(buf_string(output), ch);

	free_buf(output);
	return count;
}

void fsearch_char(CHAR_DATA *ch, int fieldptr, long marked, bool mobile, bool player)
{
	char buf[MSL];
	BUFFER *output;
	int mobilecount = 0, playercount = 0;

	if (mobile)     mobilecount = fsearch_mobile(ch, fieldptr, marked);

	if (player)     playercount = fsearch_player(ch, fieldptr, marked);

	output = new_buf();

	if (mobile) {
		if (mobilecount == 0)
			sprintf(buf, "You found no mobiles matching your search criteria.\n");
		else
			sprintf(buf, "You found %d matching mobile%s%s.\n",
			        mobilecount,
			        mobilecount > 1 ? "s" : "",
			        mobilecount > 500 ? ", of which 500 are shown" : "");

		add_buf(output, buf);
	}

	if (player) {
		if (playercount == 0)
			sprintf(buf, "You found no players matching your search criteria.\n");
		else
			sprintf(buf, "You found %d matching player%s%s.\n",
			        playercount,
			        playercount > 1 ? "s" : "",
			        playercount > 500 ? ", of which 500 are shown" : "");

		add_buf(output, buf);
	}

	page_to_char(buf_string(output), ch);
	free_buf(output);
}

void fsearch_room(CHAR_DATA *ch, int fieldptr, long marked)
{
	char buf[MSL];
	BUFFER *output;
	AREA_DATA *area;
	ROOM_INDEX_DATA *room;
	int count = 0, vnum;
	long flag;
	output = new_buf();
	add_buf(output, "{VCount {GVnum{x\n");

	for (area = area_first; area; area = area->next) {
		for (vnum = area->min_vnum; vnum <= area->max_vnum; vnum++) {
			if ((room = get_room_index(vnum)) == NULL
			    || !can_see_room(ch, room))
				continue;

			switch (fieldptr) {
			case FIELD_ROOM:        flag = GET_ROOM_FLAGS(room);        break;

			default:                                                return;
			}

			if ((marked & flag) != marked)
				continue;

			if (++count > 500)
				continue;

			sprintf(buf, "{M[{V%3d{M]{H[{G%5d{H]{x %s{x.\n",
			        count,
			        vnum,
			        room->name);
			add_buf(output, buf);
		}
	}

	if (count == 0)
		stc("You found no rooms matching your search criteria.\n", ch);
	else {
		sprintf(buf, "You found %d matching room%s%s.\n",
		        count,
		        count > 1 ? "s" : "",
		        count > 500 ? ", of which 500 are shown" : "");
		add_buf(output, buf);
		page_to_char(buf_string(output), ch);
	}

	free_buf(output);
}

void fsearch_obj(CHAR_DATA *ch, int fieldptr, long marked)
{
	char buf[MSL];
	BUFFER *output;
	OBJ_DATA *obj, *in_obj;
	int count = 1;
	long flag;
	output = new_buf();
	add_buf(output, "{VCount {YRoom  {GObject{x\n");

	/* cut off list at 400 objects, to prevent spamming out your link */
	for (obj = object_list; obj != NULL; obj = obj->next) {
		/* take care of flag comparison first */
		switch (fieldptr) {
		case FIELD_EXTRA:       flag = obj->extra_flags;        break;

		case FIELD_WEAR:        flag = obj->wear_flags;         break;

		case FIELD_WEAPON:      flag = obj->value[0];           break;

		default:                                                return;
		}

		if ((marked & flag) != marked)
			continue;

		for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj)
			;

		if (in_obj->carried_by) {
			if (in_obj->carried_by->in_room == NULL
			    || !can_see_room(ch, in_obj->carried_by->in_room)
			    || !can_see(ch, in_obj->carried_by))
				continue;

			sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{H[{G%5d{H]{x %s{x is carried by %s.\n",
			        count,
			        in_obj->carried_by->in_room->vnum,
			        obj->pIndexData->vnum,
			        obj->short_descr,
			        PERS(in_obj->carried_by, ch, VIS_PLR));
		}
		else if (in_obj->in_locker) {
			if (in_obj->in_locker->in_room == NULL
			    || !can_see_room(ch, in_obj->in_locker->in_room)
			    || !can_see(ch, in_obj->in_locker))
				continue;

			sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{H[{G%5d{H]{x %s{x is in %s's locker.\n",
			        count,
			        in_obj->in_locker->in_room->vnum,
			        obj->pIndexData->vnum,
			        obj->short_descr,
			        PERS(in_obj->in_locker, ch, VIS_PLR));
		}
		else if (in_obj->in_strongbox) {
			if (in_obj->in_strongbox->in_room == NULL
			    || !can_see_room(ch, in_obj->in_strongbox->in_room)
			    || !can_see(ch, in_obj->in_strongbox))
				continue;

			sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{H[{G%5d{H]{x %s{x is in %s's strongbox.\n",
			        count,
			        in_obj->in_strongbox->in_room->vnum,
			        obj->pIndexData->vnum,
			        obj->short_descr,
			        PERS(in_obj->in_strongbox, ch, VIS_PLR));
		}
		else if (in_obj->in_room) {
			if (!can_see_room(ch, in_obj->in_room))
				continue;

			sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{H[{G%5d{H]{x %s{x in %s.\n",
			        count,
			        in_obj->in_room->vnum,
			        obj->pIndexData->vnum,
			        obj->short_descr,
			        in_obj->in_room->name);
		}
		else    /* what's left? */
			continue;

		if (++count <= 500)     /* count stays one ahead of actual number found */
			add_buf(output, buf);
	}

	if (--count == 0)
		stc("You found no items matching your search criteria.\n", ch);
	else {
		sprintf(buf, "You found %d matching item%s%s.\n",
		        count,
		        count > 1 ? "s" : "",
		        count > 500 ? ", of which 500 are shown" : "");
		add_buf(output, buf);
		page_to_char(buf_string(output), ch);
	}

	free_buf(output);
}

void do_flagsearch(CHAR_DATA *ch, const char *argument)
{
	char arg1[MIL], arg2[MIL], word[MIL];
	int fieldptr, length;
	long marked = 0, pos;
	const struct flag_type *flag_table;
	bool player = TRUE, mobile = TRUE, toolowmobile = FALSE, toolowplayer = FALSE;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0') {
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

	if (arg2[0] == '\0') {
		stc("You must specify a field of flags to search for.\n", ch);
		return;
	}

	for (fieldptr = 0; flag_fields[fieldptr].name != NULL; fieldptr++)
		if (!str_prefix1(arg2, flag_fields[fieldptr].name))
			break;

	if (flag_fields[fieldptr].name == NULL) {
		stc("That is not a valid flag field.\n", ch);
		return;
	}

	if (argument[0] == '\0') {
		stc("You must specify at least one flag to search for.\n", ch);
		return;
	}

	if (!str_prefix1(arg1, "mobile")
	    || !str_prefix1(arg1, "player")
	    || !str_prefix1(arg1, "character")) {
		if (flag_fields[fieldptr].cand != CAND_CHAR
		    && flag_fields[fieldptr].cand != CAND_MOB
		    && flag_fields[fieldptr].cand != CAND_PLAYER) {
			stc("That is not a character field.\n", ch);
			return;
		}

		if (!str_prefix1(arg1, "mobile"))                       player = FALSE;

		if (!str_prefix1(arg1, "player"))                       mobile = FALSE;

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
	else if (!str_prefix1(arg1, "obj")) {
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
	else if (!str_prefix1(arg1, "room")) {
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

	flag_table = flag_fields[fieldptr].flag_table;

	/* turn the argument into flags */
	for (; ;) {
		argument = one_argument(argument, word);

		if (word[0] == '\0')
			break;

		length = strlen(word);

		if (length <= 2) {              /* alpha flag? */
			char letter;

			if (length == 1) {
				letter = UPPER(word[0]);
				SET_BIT(marked, flag_convert(letter));
				continue;
			}
			else {
				letter = LOWER(word[0]);

				if (letter == LOWER(word[1])
				    && letter <= 'f'
				    && letter >= 'a') {
					SET_BIT(marked, flag_convert(letter));
					continue;
				}
			}
		}

		pos = flag_lookup(word, flag_table);

		if (pos == -1) {
			ptc(ch, "That is not an acceptable %s flag.\n", flag_fields[fieldptr].name);
			return;
		}
		else
			SET_BIT(marked, flag_table[pos].bit);
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

