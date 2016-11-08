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

#include "merc.h"
#include "recycle.h"
#include "affect.h"

DECLARE_DO_FUN(do_switch);

void do_addapply(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char arg4[MAX_INPUT_LENGTH];
	int affect_modify = 1, enchant_type, duration = -1;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);
	argument = one_argument(argument, arg4);

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		stc("Syntax: addapply <object> <apply> <value> [duration]\n", ch);
		stc("  Applies can be of:\n", ch);
		stc("    hp mana stamina str dex int wis con chr\n", ch);
		stc("    age ac hitroll damroll saves\n", ch);
		return;
	}

	obj = get_obj_world(ch, arg1);

	if (obj == NULL) {
		stc("No such object exists!\n", ch);
		return;
	}

	if (!str_cmp(arg2, "hp"))          enchant_type = APPLY_HIT;
	else if (!str_cmp(arg2, "mana"))        enchant_type = APPLY_MANA;
	else if (!str_cmp(arg2, "stamina"))     enchant_type = APPLY_STAM;
	else if (!str_cmp(arg2, "str"))         enchant_type = APPLY_STR;
	else if (!str_cmp(arg2, "dex"))         enchant_type = APPLY_DEX;
	else if (!str_cmp(arg2, "int"))         enchant_type = APPLY_INT;
	else if (!str_cmp(arg2, "wis"))         enchant_type = APPLY_WIS;
	else if (!str_cmp(arg2, "con"))         enchant_type = APPLY_CON;
	else if (!str_cmp(arg2, "chr"))         enchant_type = APPLY_CHR;
	else if (!str_cmp(arg2, "sex"))         enchant_type = APPLY_SEX;
	else if (!str_cmp(arg2, "age"))         enchant_type = APPLY_AGE;
	else if (!str_cmp(arg2, "ac"))          enchant_type = APPLY_AC;
	else if (!str_cmp(arg2, "hitroll"))     enchant_type = APPLY_HITROLL;
	else if (!str_cmp(arg2, "damroll"))     enchant_type = APPLY_DAMROLL;
	else if (!str_cmp(arg2, "saves"))       enchant_type = APPLY_SAVES;
	else {
		stc("That apply is not possible!\n", ch);
		return;
	}

	if (!is_number(arg3)) {
		stc("Applies require a value.\n", ch);
		return;
	}

	if (atoi(arg3) < -30000 || atoi(arg3) > 30000) {
		stc("Value must be between -30000 and 30000.\n", ch);
		return;
	}

	affect_modify = atoi(arg3);

	if (arg4[0] == '\0')
		duration = -1;
	else if (!is_number(arg4))
		duration = -1;
	else if (atoi(arg4) < -1 || atoi(arg4) > 998)
		duration = -1;
	else
		duration = atoi(arg4);

	stc("Ok.\n", ch);

	AFFECT_DATA af = (AFFECT_DATA){0};
	af.where      = TO_OBJECT;
	af.type       = 0;
	af.level      = ch->level;
	af.duration   = duration;
	af.location   = enchant_type;
	af.modifier   = affect_modify;
	af.bitvector  = 0;
	af.evolution  = 1;
	affect_join_to_obj(obj, &af);
}

/* Morph Command by Lotus */
void do_morph(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *morph;
	CHAR_DATA *mobile;
	argument = one_argument(argument, arg1);

	if (arg1[0] == '\0') {
		stc("Morph Whom?\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg1, VIS_PLR)) == NULL) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (victim->desc == NULL) {
		stc("You can only morph connected players.\n", ch);
		return;
	}

	if (victim->desc->original != NULL) {
		stc("They are currently morphed already.\n", ch);
		return;
	}

	argument = one_argument(argument, arg2);

	if (arg2[0] == '\0') {
		stc("Morph them into what?\n", ch);
		return;
	}

	if ((morph = get_mob_world(ch, arg2, VIS_CHAR)) == NULL) {
		stc("That mobile does not exist.\n", ch);
		return;
	}

	mobile = create_mobile(get_mob_index(morph->pIndexData->vnum));

	if (!mobile) {  /* Make sure it works. -- Outsider */
		bug("Memory error creating mob in do_morph().", 0);
		stc("Could not morph.\n", ch);
		return;
	}

	REMOVE_BIT(mobile->act, ACT_AGGRESSIVE);
	SET_BIT(mobile->act, PLR_COLOR);
	SET_BIT(mobile->act, ACT_MORPH);
	char_to_room(mobile, victim->in_room);
	do_switch(victim, mobile->name);
	char_from_room(victim);
	char_to_room(victim, get_room_index(ROOM_VNUM_LIMBO));
	stc("Successful Morph!\n", ch);
}

void do_rppaward(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	BUFFER *output;
	DESCRIPTOR_DATA *d;
	int rppoint;

	if (argument[0] == '\0') {
		output = new_buf();
		stc("Name            RPPs\n", ch);
		stc("--------------------\n", ch);

		for (d = descriptor_list; d != NULL; d = d->next) {
			if (!IS_PLAYING(d) || !can_see(ch, d->character))
				continue;

			victim = (d->original != NULL) ? d->original : d->character;

			if (!can_see(ch, victim))
				continue;

			sprintf(buf, "%-14s {V%5d{x\n", victim->name, victim->pcdata->rolepoints);
			add_buf(output, buf);
		}

		page_to_char(buf_string(output), ch);
		free_buf(output);
		return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0') {
		stc("Syntax:\n", ch);
		stc("  rpp <award|deduct> <victim> <amount>\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg2, VIS_PLR)) == NULL) {
		stc("Player not found.\n", ch);
		return;
	}

	if (!is_number(argument)) {
		stc("Value must be numeric.\n", ch);
		return;
	}

	rppoint = atoi(argument);

	if (rppoint < 0 || rppoint > 50) {
		stc("Value must be between 0 and 50.\n", ch);
		return;
	}

	if (!str_cmp(arg1, "award")) {
		victim->pcdata->rolepoints += rppoint;
		ptc(ch, "You have awarded them %d role playing points.\n", rppoint);
		ptc(victim, "You have been awarded %d role playing points.\n", rppoint);
		sprintf(buf, "%s awarded %d rpp to %s.", ch->name, rppoint, victim->name);
		wiznet(buf, ch, NULL, 0, 0, 0);
		return;
	}

	if (!str_cmp(arg1, "deduct")) {
		if (rppoint > victim->pcdata->rolepoints) {
			stc("They do not have enough role playing points for that.\n", ch);
			return;
		}

		victim->pcdata->rolepoints -= rppoint;
		sprintf(buf, "%d role playing points have been deducted, %d remaining.\n",
		        rppoint, victim->pcdata->rolepoints);
		stc(buf, ch);
		stc(buf, victim);
		sprintf(buf, "%s deducted %d rpp from %s [%d remaining].",
		        ch->name, rppoint, victim->name, victim->pcdata->rolepoints);
		wiznet(buf, ch, NULL, WIZ_SECURE, WIZ_QUEST, GET_RANK(ch));
		return;
	}

	stc("Use 'deduct' or 'award'.\n", ch);
}

ROOM_INDEX_DATA *get_scatter_room(CHAR_DATA *ch)
{
	ROOM_INDEX_DATA *room;

	for (; ;) {
		room = get_room_index(number_range(0, 32767));

		if (room == NULL
		    || room == ch->in_room
		    || !can_see_room(ch, room)
		    || room->area == quest_area
		    || (room->area->min_vnum >= 24000      /* clanhall vnum ranges */
		        && room->area->min_vnum <= 26999)
		    || room->guild
		    || !str_cmp(room->area->name, "Playpen")
		    || !str_cmp(room->area->name, "IMM-Zone")
		    || !str_cmp(room->area->name, "Limbo")
		    || !str_cmp(room->area->name, "Eilyndrae")     /* hack to make eilyndrae and torayna cri unquestable */
		    || !str_cmp(room->area->name, "Torayna Cri")
		    || !str_cmp(room->area->name, "Battle Arenas")
		    || room->sector_type == SECT_ARENA
		    || IS_SET(GET_ROOM_FLAGS(room),
		              ROOM_MALE_ONLY
		              | ROOM_FEMALE_ONLY
		              | ROOM_PRIVATE
		              | ROOM_SOLITARY
		              | ROOM_NOQUEST
		              | ROOM_PET_SHOP))
			continue;

		return room;
	}

	return NULL;
}

/* scatter all items in a room -- Montrey */
void do_scatter(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj, *obj_next;
	ROOM_INDEX_DATA *room;
	bool scattered = FALSE;

	if (ch->in_room == NULL)
		return;

	for (obj = ch->in_room->contents; obj != NULL; obj = obj_next) {
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

void do_string(CHAR_DATA *ch, const char *argument)
{
	char type [MAX_INPUT_LENGTH];
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	char buf [MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *room;
	argument = one_argument(argument, type);
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	strcpy(arg3, smash_tilde(argument));

	if (type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		stc("Syntax:\n", ch);
		stc("  string char <name> <field> <string>\n", ch);
		stc("    fields: name short long desc title spec deity status spouse\n", ch);
		stc("  string obj  <name> <field> <string>\n", ch);
		stc("    fields: name short long material extended more\n", ch);
		stc("  string room <number> <field> <string>\n", ch);
		stc("    fields: name\n", ch);
		return;
	}

	if (!str_prefix1(type, "character") || !str_prefix1(type, "mobile")) {
		if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == NULL) {
			stc("They aren't here.\n", ch);
			return;
		}

		if (IS_IMP(victim) && !IS_IMP(ch)) {
			stc("They wouln't like that.\n", ch);
			return;
		}

		/* string something */

		if (!str_prefix1(arg2, "spouse")) {
			if (IS_NPC(victim)) {
				stc("Not a good idea.\n", ch);
				return;
			}

			if (!str_cmp(arg3, "none")) {
				free_string(victim->pcdata->spouse);
				victim->pcdata->spouse = str_dup("");
			}
			else {
				free_string(victim->pcdata->spouse);
				victim->pcdata->spouse = str_dup(arg3);
			}

			sprintf(buf, "%s's spouse has been changed to %s.\n", victim->name, arg3);
			stc(buf, ch);
			return;
		}

		if (!str_prefix1(arg2, "name")) {
			if (!IS_NPC(victim)) {
				stc("You can't change a player's name!\n", ch);
				return;
			}

			sprintf(buf, "%s's name has been changed to %s.\n", victim->name, arg3);
			free_string(victim->name);
			victim->name = str_dup(arg3);
			stc(buf, ch);
			return;
		}

		if (!str_prefix1(arg2, "description")) {
			free_string(victim->description);
			victim->description = str_dup(arg3);
			sprintf(buf, "%s's description has been changed to %s.\n", victim->name, arg3);
			stc(buf, ch);
			return;
		}

		if (!str_prefix1(arg2, "short")) {
			free_string(victim->short_descr);
			victim->short_descr = str_dup(arg3);
			sprintf(buf, "%s's short description has been changed to %s.\n", victim->name, arg3);
			stc(buf, ch);
			return;
		}

		if (!str_prefix1(arg2, "long")) {
			free_string(victim->long_descr);
			strcat(arg3, "\n");
			victim->long_descr = str_dup(arg3);
			sprintf(buf, "%s's long description has been changed to %s", victim->name, arg3);
			stc(buf, ch);
			return;
		}

		if (!str_prefix1(arg2, "title")) {
			if (IS_NPC(victim)) {
				stc("Mobiles don't have a title, silly!\n", ch);
				return;
			}

			set_title(victim, arg3);
			sprintf(buf, "%s's title has been changed to %s.\n", victim->name, arg3);
			stc(buf, ch);
			return;
		}

		if (!str_prefix1(arg2, "spec")) {
			if (!IS_NPC(victim)) {
				stc("Players don't have specfuns, silly!\n", ch);
				return;
			}

			if ((victim->spec_fun = spec_lookup(arg3)) == 0) {
				sprintf(buf, "%s is not a valid spec fun.\n", arg3);
				stc(buf, ch);
				return;
			}

			sprintf(buf, "%s's spec fun value set to %s.\n", victim->name, arg3);
			stc(buf, ch);
			return;
		}

		if (!str_prefix1(arg2, "deity")) {
			if (IS_NPC(victim)) {
				stc("Mobiles are all atheists!\n", ch);
				return;
			}

			free_string(victim->pcdata->deity);
			victim->pcdata->deity = str_dup(arg3);
			sprintf(buf, "%s's deity string has been changed to %s.\n", victim->name, arg3);
			stc(buf, ch);
			return;
		}

		if (!str_prefix1(arg2, "status")) {
			if (IS_NPC(victim)) {
				stc("A mobile has no status to change!\n", ch);
				return;
			}

			free_string(victim->pcdata->status);
			victim->pcdata->status = str_dup(arg3);
			sprintf(buf, "%s's status string has been changed to %s.\n", victim->name, arg3);
			stc(buf, ch);
			return;
		}
	}

	if (!str_prefix1(type, "object")) {
		/* string an obj */
		if ((obj = get_obj_carry(ch, arg1)) == NULL
		    && (obj = get_obj_wear(ch, arg1)) == NULL
		    && (obj = get_obj_list(ch, arg1, ch->in_room->contents)) == NULL) {
			sprintf(buf, "After searching the whole mud, you could not find %s.\n", arg1);
			stc(buf, ch);
			return;
		}

		if (!str_prefix1(arg2, "name")) {
			sprintf(buf, "%s is now known as %s.\n", obj->name, arg3);
			free_string(obj->name);
			obj->name = str_dup(arg3);
			stc(buf, ch);
			return;
		}

		if (!str_prefix1(arg2, "short")) {
			sprintf(buf, "%s's short description has been changed to %s.\n", obj->short_descr, arg3);
			free_string(obj->short_descr);
			obj->short_descr = str_dup(arg3);
			stc(buf, ch);
			return;
		}

		if (!str_prefix1(arg2, "long")) {
			sprintf(buf, "%s's long description has been changed to %s.\n", obj->short_descr, arg3);
			free_string(obj->description);
			obj->description = str_dup(arg3);
			stc(buf, ch);
			return;
		}

		if (!str_prefix1(arg2, "material")) {
			sprintf(buf, "%s suddenly warps into %s as %s glows brightly.\n",
			        obj->material, arg3, obj->short_descr);
			free_string(obj->material);
			obj->material = str_dup(arg3);
			stc(buf, ch);
			return;
		}

		if (!str_cmp(arg2, "ed") || !str_prefix1(arg2, "extended")) {
			EXTRA_DESCR_DATA *ed;
			argument = one_argument(argument, arg3);

			if (argument == NULL) {
				stc("Syntax: set obj <object> ed <keyword> <string>\n", ch);
				return;
			}

			if (obj->extra_descr != NULL) {
				EXTRA_DESCR_DATA *ed_next;
				EXTRA_DESCR_DATA *ed_prev = NULL;

				for (ed = obj->extra_descr; ed != NULL; ed = ed_next) {
					ed_next = ed->next;

					if (!str_cmp(ed->keyword, arg3)) {
						if (ed == obj->extra_descr)
							obj->extra_descr = ed_next;
						else
							ed_prev->next = ed_next;

						free_extra_descr(ed);
					}
					else
						ed_prev = ed;
				}
			}

			if (!str_cmp(argument, "none"))
				return;

			char desc[MIL];
			sprintf(desc, "%s\n", argument);

			ed = new_extra_descr();
			ed->keyword         = str_dup(arg3);
			ed->description     = str_dup(desc);
			ed->next            = obj->extra_descr;
			obj->extra_descr    = ed;
			ptc(ch, "Extended description set to:\n%s\n", arg3);
			return;
		}

		if (!str_prefix1(arg2, "more")) {
			EXTRA_DESCR_DATA *ed;
			char buf[MAX_STRING_LENGTH];
			buf[0] = '\0';
			argument = one_argument(argument, arg3);

			if (argument == NULL) {
				stc("Syntax: string obj <obj> more <keyword> <string>\n", ch);
				return;
			}

			if (obj->extra_descr != NULL) {
				EXTRA_DESCR_DATA *ed_next;
				EXTRA_DESCR_DATA *ed_prev = NULL;

				for (ed = obj->extra_descr; ed != NULL; ed = ed_next) {
					ed_next = ed->next;

					if (!str_cmp(ed->keyword, arg3)) {
						strcpy(buf, ed->description);

						if (ed == obj->extra_descr)
							obj->extra_descr = ed_next;
						else
							ed_prev->next = ed_next;

						free_extra_descr(ed);
					}
					else
						ed_prev = ed;
				}
			}

			strcat(buf, argument);
			strcat(buf, "\n");
			ed = new_extra_descr();
			ed->keyword         = str_dup(arg3);
			ed->description     = str_dup(buf);
			ed->next            = obj->extra_descr;
			obj->extra_descr    = ed;
			ptc(ch, "Added to extended description:\n%s\n", arg3);
			return;
		}
	}

	/* Room Strings by Lotus */
	if (!str_prefix1(type, "room")) {
		if ((room = get_room_index(atoi(arg1))) == NULL) {
			sprintf(buf, "Room %d does not exist.\n", atoi(arg1));
			stc(buf, ch);
			return;
		}

		if (!str_prefix1(arg2, "name")) {
			free_string(room->name);
			room->name = str_dup(arg3);
			sprintf(buf, "Room %d's name has been changed to %s.\n",
			        atoi(arg1), arg3);
			stc(buf, ch);
			return;
		}
	}

	/* echo bad use message */
	do_string(ch, "");
} /* end do_string() */

void do_switch(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	if (IS_NPC(ch) || ch->desc == NULL) {
		stc("You're not a real live player, you cannot switch.\n", ch);
		return;
	}

	if (ch->desc->original != NULL) {
		stc("You are currently switched already.\n", ch);
		return;
	}

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Switch into whom?\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) == NULL) {
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

	if (victim->desc != NULL) {
		stc("Sorry, Character in use.\n", ch);
		return;
	}

	sprintf(buf, "$N has switched into: %s", victim->short_descr);
	/* 0 so it shows spell casters */
	wiznet(buf, ch, NULL, WIZ_SWITCHES, 0, 0);
	ch->desc->character = victim;
	ch->desc->original  = ch;
	victim->desc        = ch->desc;
	ch->desc            = NULL;

	/* change communications to match */
	free_string(victim->prompt);
	victim->prompt = str_dup(ch->prompt);

	victim->comm = ch->comm;
	victim->censor = ch->censor;        /* Montrey */
	victim->lines = ch->lines;
	stc("You have been morphed.\n", victim);
	return;
}

void do_return(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	ROOM_INDEX_DATA *location;

	if (ch->desc == NULL)
		return;

	if (ch->desc->original == NULL) {
		stc("You are not currently morphed.\n", ch);
		return;
	}

	stc("You return to your original body.\n", ch);

	free_string(ch->prompt);
	ch->prompt = str_dup("");

	sprintf(buf, "$N has returned from: %s.", ch->short_descr);
	wiznet(buf, ch->desc->original, 0, WIZ_SWITCHES, WIZ_SECURE, GET_RANK(ch));

	if (IS_SET(ch->desc->character->act, ACT_MORPH)) {
		if (ch->desc->character->in_room == NULL)
			location = get_room_index(ROOM_VNUM_MORGUE);
		else
			location = ch->desc->character->in_room;

		char_from_room(ch->desc->original);
		char_to_room(ch->desc->original, location);
	}

	ch->desc->character       = ch->desc->original;
	ch->desc->original        = NULL;
	ch->desc->character->desc = ch->desc;

	if (!IS_NPC(ch->desc->character))
		if (ch->desc->character->pcdata->buffer->string[0] != '\0')
			stc("You have messages: Type 'replay'\n", ch);

	ch->desc                  = NULL;

	if (IS_SET(ch->act, ACT_MORPH) && ch->in_room != NULL)
		extract_char(ch, TRUE);  /* Only if raw_kill didn't do it */

	return;
}

/* for future use */
bool setup_obj(CHAR_DATA *ch, OBJ_DATA *obj, const char *argument)
{
	char arg1[MIL];
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

	                        if (arg1[0] == '\0')                    type = TOKEN_NONE;
	                        else if (!str_prefix1(arg1, "wild"))    type = TOKEN_WILD;
	                        else if (!str_prefix1(arg1, "trivia"))  type = TOKEN_TRIVIA;
	                        else if (!str_prefix1(arg1, "hidden"))  type = TOKEN_HIDDEN;
	                        else if (!str_prefix1(arg1, "quest"))   type = TOKEN_QUEST;
	                        else
	                        {
	                                stc("Valid token types are quest, hidden, trivia, and wild,\n"
	                                    "or use without an argument for a generic token.\n", ch);
	                                return FALSE;
	                        }

	                        free_string(obj->name);
	                        free_string (obj->short_descr);
	                        obj->name = str_dup(tdesc_table[type].keywords);
	                        obj->short_descr = str_dup(tdesc_table[type].short_desc);
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
	case ITEM_ROOM_KEY:
	case ITEM_GEM:
	case ITEM_JEWELRY:
	case ITEM_WEDDINGRING:
	default:
		break;
	}

	return TRUE;
}

void do_create(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	char buf[100];
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *obj;
	int x;

	if (!IS_IMMORTAL(ch))
		return;

	if (argument[0] == '\0') {
		stc("Syntax:\n", ch);
		stc("  create obj <type>\n", ch);
		return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (!str_cmp(arg1, "object")) {
		if (arg2[0] == '\0') {
			stc("Valid item types are:\n", ch);

			for (x = 0; item_table[x].name != NULL; x++)
				ptc(ch, "%s\n", item_table[x].name);

			return;
		}

		if (!str_cmp(arg2, "npc_corpse")
		    || !str_cmp(arg2, "pc_corpse")) {
//		    || !str_cmp(arg2, "coach")
			stc("You cannot create one of those.\n", ch);
			return;
		}

		for (x = 0; item_table[x].name != NULL; x++) {
			if (!str_cmp(arg2, item_table[x].name)) {
				if ((pObjIndex = get_obj_index(item_table[x].type + 100)) == NULL) {
					sprintf(buf, "[create] Cannot find item vnum %d.\n", item_table[x].type + 100);
					bug(buf, 0);
					stc("That item seems to be missing.\n", ch);
					return;
				}

				obj = create_object(pObjIndex, 0);

				if (! obj) {
					bug("Memory error creating object in do_create.", 0);
					return;
				}

				if (!setup_obj(ch, obj, argument)) {
					extract_obj(obj);
					return;
				}

				obj_to_char(obj, ch);
				act("$n has created $p!", ch, obj, NULL, TO_ROOM);
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
void do_skillpoint(CHAR_DATA *ch, const char *argument)
{
	char char_name[MAX_INPUT_LENGTH];
	char give_or_take[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	sh_int new_points;
	char buffer[MAX_INPUT_LENGTH];
	argument = one_argument(argument, give_or_take);
	argument = one_argument(argument, char_name);

	if ((give_or_take[0] == '\0') || (char_name[0] == '\0') || (argument[0] == '\0')) {
		stc("Syntax: skillpoint <award|deduct> <player> <amount>\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, char_name, VIS_PLR)) == NULL) {
		stc("Player not found.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		stc("Immortals can handle their own skill points.\n", ch);
		return;
	}

	if (!is_number(argument)) {
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
		sprintf(buffer, "%d skill points awarded to %s. Total: %d.\n",
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
			sprintf(buffer, "%d skill points deducted from %s. %d remaining.\n",
			        new_points, victim->name, victim->pcdata->skillpoints);
			stc(buffer, ch);
			stc(buffer, victim);
		}
	}
	/* Must have mis-typed command. */
	else
		stc("Syntax: skillpoint <award|deduct> <player> <amount>\n", ch);
}

