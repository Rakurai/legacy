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
#include "recycle.h"
#include "tables.h"
#include "magic.h"
#include "sql.h"
#include "lookup.h"
#include "gem.h"

extern AREA_DATA *area_first;

/* command procedures needed */
DECLARE_DO_FUN(do_exits);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_play);

extern void     email_file    args((CHAR_DATA *ch, const char *file, const char *str));
extern void do_config args((CHAR_DATA *ch, const char *buf));

char   *const   where_name      [] = {
	"<used as light>     ",
	"<worn on finger>    ",
	"<worn on finger>    ",
	"<worn around neck>  ",
	"<worn around neck>  ",
	"<worn on torso>     ",
	"<worn on head>      ",
	"<worn on legs>      ",
	"<worn on feet>      ",
	"<worn on hands>     ",
	"<worn on arms>      ",
	"<worn as shield>    ",
	"<worn about body>   ",
	"<worn about waist>  ",
	"<worn around wrist> ",
	"<worn around wrist> ",
	"<wielded>           ",
	"<held>              ",
	"<floating nearby>   ",
	"<secondary weapon>  ",
	"<wedding ring>      ",
};

char   *const   day_name        [] = {
	"Regeneration",
	"Endeavor",
	"The Sun",
	"The Great Gods",
	"The Blue Moon",
	"The Lesser Moon",
	"Omens"
};

char   *const   month_name      [] = {
	"Abundance", "Perseverance", "Challenge",
	"Sacrifice", "Continuity", "Reverence",
	"Harmony", "Strife", "Peace",
	"Futility", "Courtship", "Awakening",
	"Long Shadows", "Silence", "Ancient Darkness",
	"Endings", "Rapture"
};

/*
 * Local functions.
 */
char   *format_obj_to_char      args((OBJ_DATA *obj, CHAR_DATA *ch,
                                      bool fShort));
void    show_list_to_char       args((OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing, bool insidecont));
void    show_char_to_char_0     args((CHAR_DATA *victim, CHAR_DATA *ch));
void    show_char_to_char_1     args((CHAR_DATA *victim, CHAR_DATA *ch));
void    show_char_to_char       args((CHAR_DATA *list, CHAR_DATA *ch));
bool    check_blind             args((CHAR_DATA *ch));

char *format_obj_to_char(OBJ_DATA *obj, CHAR_DATA *ch, bool fShort)
{
	static char buf[MSL];
	int diff;
	AFFECT_DATA *paf;
	buf[0] = '\0';

	if ((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
	    || (obj->description == NULL || obj->description[0] == '\0'))
		return buf;

	if (obj->num_settings > 0) {
		strcat(buf, get_gem_short_string(obj));
		strcat(buf, " ");
	}

	/* Color additions by Lotus */
	if (!IS_NPC(ch)
	    && ((ch->questobj > 0 && obj->pIndexData->vnum == ch->questobj)
	        || (ch->pcdata->squestobj != NULL && ch->pcdata->squestobj == obj)))
		strcat(buf, "{f{R[TARGET] {x");

	if (IS_OBJ_STAT(obj, ITEM_INVIS))
		strcat(buf, "{W(Invis) ");

	if (IS_AFFECTED(ch, AFF_DETECT_EVIL)
	    && IS_OBJ_STAT(obj, ITEM_EVIL))
		strcat(buf, "{R(Red Aura) ");

	if (IS_AFFECTED(ch, AFF_DETECT_GOOD)
	    && IS_OBJ_STAT(obj, ITEM_BLESS))
		strcat(buf, "{B(Blue Aura) ");

	if (IS_AFFECTED(ch, AFF_DETECT_MAGIC)
	    && IS_OBJ_STAT(obj, ITEM_MAGIC))
		strcat(buf, "{G(Magical) ");

	if (IS_OBJ_STAT(obj, ITEM_GLOW))
		strcat(buf, "{Y(Glowing) ");

	if (IS_OBJ_STAT(obj, ITEM_HUM))
		strcat(buf, "{C(Humming) ");

	/* flags for temp weapon affects -- Elrac */
	if (obj->item_type == ITEM_WEAPON) {
		for (paf = obj->affected; paf; paf = paf->next) {
			if (!paf->duration)     continue;

			if (paf->bitvector & WEAPON_FLAMING)    strcat(buf, "{Y(Fl) ");

			if (paf->bitvector & WEAPON_FROST)      strcat(buf, "{C(Fr) ");

			if (paf->bitvector & WEAPON_VAMPIRIC)   strcat(buf, "{P(Bl) ");

			if (paf->bitvector & WEAPON_SHOCKING)   strcat(buf, "{V(Sh) ");

			if (paf->bitvector & WEAPON_POISON)     strcat(buf, "{G(Po) ");
		}
	}

	/* flags for temp weapon affects and dazzling light -- Elrac */
	if (get_affect(obj->affected, gsn_dazzling_light))
		strcat(buf, "{W{f(Dazzling){x ");

	if (obj->condition <= 9 && obj->condition >= 0)         strcat(buf, "{b(Ruined) ");

	if (obj->condition >= 10 && obj->condition <= 24)       strcat(buf, "{b(Broken) ");

	strcat(buf, "{x");

	if (fShort) {
		if (obj->short_descr != NULL)
			strcat(buf, obj->short_descr);
	}
	else {
		if (obj->description != NULL)
			strcat(buf, obj->description);
	}

	strcat(buf, "{x");

	if (IS_SET(ch->act, PLR_LOOKINPIT) && !IS_NPC(ch)) {
		diff = get_usable_level(ch) - obj->level;

		if (obj->level >= LEVEL_IMMORTAL)  strcat(buf, " {V(Immortal Item)");
		else if (diff < -50)                    strcat(buf, " {R(You wish)");
		else if (diff < -25)                    strcat(buf, " {R(In your dreams)");
		else if (diff < -10)                    strcat(buf, " {P(Powerful Item)");
		else if (diff <   0)                    strcat(buf, " {P(Almost there)");
		else if (diff <  10)                    strcat(buf, " {G(Just your size)");
		else if (diff <  25)                    strcat(buf, " {G(Still useful)");
		else if (diff <  50)                    strcat(buf, " {H(Weak Item)");
		else                                    strcat(buf, " {H(Tis but a toy)");

		strcat(buf, "{x");
	}

	return buf;
}

void show_affect_to_char(AFFECT_DATA *paf, CHAR_DATA *ch)
{
	if (IS_IMMORTAL(ch))
		ptc(ch, "Affects %s by %d, level %d", affect_loc_name(paf->location), paf->modifier, paf->level);
	else
		ptc(ch, "Affects %s by %d\n", affect_loc_name(paf->location), paf->modifier);

	if (paf->duration > -1)
		ptc(ch, ", %d hours.\n", paf->duration);
	else
		ptc(ch, ".\n");

	if (paf->bitvector) {
		switch (paf->where) {
		case TO_AFFECTS:
			ptc(ch, "Adds %s affect.\n", affect_bit_name(paf->bitvector));
			break;

		case TO_OBJECT:
			ptc(ch, "Adds %s object flag.\n", extra_bit_name(paf->bitvector));
			break;

		case TO_WEAPON:
			ptc(ch, "Adds %s weapon flags.\n", weapon_bit_name(paf->bitvector));
			break;

		case TO_DRAIN:
			ptc(ch, "Drains %s.\n", imm_bit_name(paf->bitvector));
			break;

		case TO_IMMUNE:
			ptc(ch, "Adds immunity to %s.\n", imm_bit_name(paf->bitvector));
			break;

		case TO_RESIST:
			ptc(ch, "Adds resistance to %s.\n", imm_bit_name(paf->bitvector));
			break;

		case TO_VULN:
			ptc(ch, "Adds vulnerability to %s.\n", imm_bit_name(paf->bitvector));
			break;

		default:
			ptc(ch, "Unknown bit %d: %d\n", paf->where, paf->bitvector);
			break;
		}
	}
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char(OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing, bool insidecont)
{
	OBJ_DATA *obj;
	BUFFER *output;
	char buf[MAX_STRING_LENGTH];
	char **prgpstrShow, *pstrShow;
	int *prgnShow, nShow = 0, iShow, count = 0;
	bool fCombine, foundcont = FALSE;

	if (ch->desc == NULL)
		return;

	/* Alloc space for output lines */
	output = new_buf();

	for (obj = list; obj != NULL; obj = obj->next_content)
		count++;

	if (count > 8000) {
		stc("There are too many items to display.\n", ch);
		return;
	}

	prgpstrShow = alloc_mem(count * sizeof(char *));
	prgnShow    = alloc_mem(count * sizeof(int));

	/* Format the list of objects */
	for (obj = list; obj != NULL; obj = obj->next_content) {
		if (!can_see_obj(ch, obj))
			continue;

		if (insidecont && obj->contains)
			foundcont = TRUE;

		if (obj->wear_loc == WEAR_NONE) {
			pstrShow = format_obj_to_char(obj, ch, fShort);
			fCombine = FALSE;

			if (IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE)) {
				/* Look for duplicates, case sensitive.
				   Matches tend to be near end so run loop backwords. */
				for (iShow = nShow - 1; iShow >= 0; iShow--)
					if (!strcmp(prgpstrShow[iShow], pstrShow)) {
						prgnShow[iShow]++;
						fCombine = TRUE;
						break;
					}
			}

			/* Couldn't combine, or didn't want to */
			if (!fCombine) {
				prgpstrShow [nShow] = str_dup(pstrShow);
				prgnShow    [nShow] = 1;
				nShow++;
			}
		}
	}

	/* Output the formatted list */
	for (iShow = 0; iShow < nShow; iShow++) {
		if (prgpstrShow[iShow][0] == '\0') {
			free_string(prgpstrShow[iShow]);
			continue;
		}

		if (IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE)) {
			if (prgnShow[iShow] != 1) {
				sprintf(buf, "(%2d) ", prgnShow[iShow]);
				add_buf(output, buf);
			}
			else
				add_buf(output, "     ");
		}

		add_buf(output, prgpstrShow[iShow]);
		add_buf(output, "\n");
		free_string(prgpstrShow[iShow]);

		if (output->size > 15500) {
			if (IS_SET(ch->comm, COMM_COMBINE))
				add_buf(output, "     (More stuff not shown)\n");
			else
				add_buf(output, "     (More stuff not shown -- COMBINE would help)\n");

			break;
		}
	}

	if (fShowNothing && nShow == 0) {
		if (IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE))
			stc("     ", ch);

		stc("Nothing.\n", ch);
	}

	page_to_char(buf_string(output), ch);
	/* Clean up */
	free_buf(output);
	free_mem(prgpstrShow, count * sizeof(char *));
	free_mem(prgnShow,    count * sizeof(int));

	if (foundcont)  /* for evolved peek, list items in containers, but not twice nested */
		for (obj = list; obj != NULL; obj = obj->next_content)
			if (can_see_obj(ch, obj) && obj->contains) {
				ptc(ch, "\n  Inside %s{x:\n", obj->short_descr);
				show_list_to_char(obj->contains, ch, TRUE, TRUE, FALSE);
			}
} /* end show_list_to_char() */

void do_peek(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	bool all = FALSE;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (IS_NPC(ch) || !get_skill(ch, gsn_peek)) {
		stc("You are not skilled at peeking.\n", ch);
		return;
	}

	if (arg1[0] == '\0') {
		stc("Peek at whom?\n", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg1, VIS_CHAR)) == NULL) {
		stc("You see no one by that name in the room.\n", ch);
		return;
	}

	if (victim == ch) {
		stc("Are you that paranoid?!?\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) {
		stc("They probably wouldn't like that.\n", ch);
		return;
	}

	if (!deduct_stamina(ch, gsn_peek))
		return;

	if (!chance(get_skill(ch, gsn_peek))) {
		stc("You can't seem to find a good angle...\n", ch);
		return;
	}

	if (arg2[0] != '\0' && get_evolution(ch, gsn_peek) > 1) {
		if (!str_cmp(arg2, "lore")) {
			OBJ_DATA *obj;

			if (!get_skill(ch, gsn_lore)) {
				stc("You aren't trained in the lore of items.\n", ch);
				return;
			}

			if (argument[0] == '\0') {
				stc("What item in their inventory do you want to lore?\n", ch);
				return;
			}

			if ((obj = get_obj_carry(victim, argument)) == NULL) {
				stc("They aren't carrying that.\n", ch);
				return;
			}

			WAIT_STATE(ch, skill_table[gsn_lore].beats);

			if (!IS_NPC(ch) && !chance(get_skill(ch, gsn_lore)))
				act("You look at $p, but you can't find out any additional information.",
				    ch, obj, NULL, TO_CHAR);
			else {
				spell_identify(gsn_lore, (4 * obj->level) / 3, ch, obj,
				               TARGET_OBJ, get_evolution(ch, gsn_lore));
				check_improve(ch, gsn_lore, TRUE, 4);
			}

			return;
		}
		else if (!str_cmp(arg2, "all") && get_evolution(ch, gsn_peek) > 2)
			all = TRUE;
	}

	act("You peek at $S inventory:", ch, NULL, victim, TO_CHAR);
	show_list_to_char(victim->carrying, ch, TRUE, TRUE, all);
	check_improve(ch, gsn_peek, TRUE, 4);
}

void show_char_to_char_0(CHAR_DATA *victim, CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH],
	     message[MAX_STRING_LENGTH];
	buf[0] = '\0';

	/* Pretty little link dead by Demonfire */
	if (!IS_NPC(victim)) {
		if (IS_SET(victim->pcdata->plr, PLR_LINK_DEAD))
			strcat(buf, "{G(LinkDead) ");
	}

	if (IS_NPC(victim)
	    && ((ch->questmob > 0 && victim->pIndexData->vnum == ch->questmob)
	        || (!ch->desc->original && ch->pcdata->squestmob != NULL && victim == ch->pcdata->squestmob)))
		strcat(buf, "{f{R[TARGET] {x");

	if (IS_SET(victim->comm, COMM_AFK))
		strcat(buf, "{b[AFK] ");

	if (IS_AFFECTED(victim, AFF_INVISIBLE))
		strcat(buf, "{C(Invis) ");

	if (get_affect(victim->affected, gsn_midnight))
		strcat(buf, "{c(Shadowy) ");

	if (get_affect(victim->affected, gsn_hex))
		strcat(buf, "{c(Dark Aura) ");

	if (victim->invis_level)
		strcat(buf, "{T(Wizi) ");

	if (victim->lurk_level)
		strcat(buf, "{H(Lurk) ");

	if (!IS_NPC(victim) && (victim->pcdata->aura[0] != '\0')) {
		char string[MAX_INPUT_LENGTH];
		sprintf(string, "{W(%s{W) ", victim->pcdata->aura);
		strcat(buf, string);
	}

	if (IS_AFFECTED(victim, AFF_HIDE)) strcat(buf,
		                "{B(Hide) ");

	if (IS_AFFECTED(victim, AFF_CHARM)) strcat(buf,
		                "{M(Charmed) ");

	if (IS_AFFECTED(victim, AFF_PASS_DOOR)) strcat(buf,
		                "{c(Translucent) ");

	if (IS_AFFECTED(victim, AFF_FAERIE_FIRE)) strcat(buf,
		                "{P(Pink Aura) ");

	if (IS_AFFECTED(victim, AFF_FLAMESHIELD)) strcat(buf,
		                "{b(Flaming Aura) ");

	if (IS_EVIL(victim) && IS_AFFECTED(ch, AFF_DETECT_EVIL)) strcat(buf,
		                "{R(Red Aura) ");

	if (IS_GOOD(victim) && IS_AFFECTED(ch, AFF_DETECT_GOOD)) strcat(buf,
		                "{Y(Golden Aura) ");

	if (IS_AFFECTED(victim, AFF_SANCTUARY)) strcat(buf,
		                "{W(White Aura) ");

	if (!IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER))
		strcat(buf, "{R(KILLER) ");

	if (!IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF))
		strcat(buf, "{B(THIEF) ");

	strcat(buf, "{x");

	if (get_position(victim) == victim->start_pos && victim->long_descr[0] != '\0') {
		if (IS_NPC(victim))
			new_color(ch, CSLOT_MISC_MOBILES);
		else
			new_color(ch, CSLOT_MISC_PLAYERS);

		strcat(buf, victim->long_descr);
		stc(buf, ch);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	/* took PERS out of this line, with both show_char_to_char and PERS checking can_see,
	   means that sometimes it shows players (and mobs) as 'someone' in the room list -- Montrey */
	strcat(buf, (IS_NPC(victim) ? victim->short_descr : victim->name));

	if (!IS_NPC(victim) && !IS_SET(ch->comm, COMM_BRIEF)
	    &&   get_position(victim) == POS_STANDING && ch->on == NULL) {
		new_color(ch, CSLOT_MISC_PLAYERS);
		strcat(buf, victim->pcdata->title);
		stc("{x{a", ch);
		set_color(ch, WHITE, NOBOLD);
	}

	switch (get_position(victim)) {
	case POS_DEAD:     strcat(buf, " is DEAD!!");              break;

	case POS_MORTAL:   strcat(buf, " is mortally wounded.");   break;

	case POS_INCAP:    strcat(buf, " is incapacitated.");      break;

	case POS_STUNNED:  strcat(buf, " is lying here stunned."); break;

	case POS_SLEEPING:
		if (victim->on != NULL) {
			if (IS_SET(victim->on->value[2], SLEEP_AT)) {
				sprintf(message, " is sleeping at %s.",
				        victim->on->short_descr);
				strcat(buf, message);
			}
			else if (IS_SET(victim->on->value[2], SLEEP_ON)) {
				sprintf(message, " is sleeping on %s.",
				        victim->on->short_descr);
				strcat(buf, message);
			}
			else {
				sprintf(message, " is sleeping in %s.",
				        victim->on->short_descr);
				strcat(buf, message);
			}
		}
		else
			strcat(buf, " is sleeping here.");

		break;

	case POS_RESTING:
		if (victim->on != NULL) {
			if (IS_SET(victim->on->value[2], REST_AT)) {
				sprintf(message, " is resting at %s.",
				        victim->on->short_descr);
				strcat(buf, message);
			}
			else if (IS_SET(victim->on->value[2], REST_ON)) {
				sprintf(message, " is resting on %s.",
				        victim->on->short_descr);
				strcat(buf, message);
			}
			else {
				sprintf(message, " is resting in %s.",
				        victim->on->short_descr);
				strcat(buf, message);
			}
		}
		else
			strcat(buf, " is resting here.");

		break;

	case POS_SITTING:
		if (victim->on != NULL) {
			if (IS_SET(victim->on->value[2], SIT_AT)) {
				sprintf(message, " is sitting at %s.",
				        victim->on->short_descr);
				strcat(buf, message);
			}
			else if (IS_SET(victim->on->value[2], SIT_ON)) {
				sprintf(message, " is sitting on %s.",
				        victim->on->short_descr);
				strcat(buf, message);
			}
			else {
				sprintf(message, " is sitting in %s.",
				        victim->on->short_descr);
				strcat(buf, message);
			}
		}
		else
			strcat(buf, " is sitting here.");

		break;

	case POS_STANDING:
		if (victim->on != NULL) {
			if (IS_SET(victim->on->value[2], STAND_AT)) {
				sprintf(message, " is standing at %s.",
				        victim->on->short_descr);
				strcat(buf, message);
			}
			else if (IS_SET(victim->on->value[2], STAND_ON)) {
				sprintf(message, " is standing on %s.",
				        victim->on->short_descr);
				strcat(buf, message);
			}
			else {
				sprintf(message, " is standing in %s.",
				        victim->on->short_descr);
				strcat(buf, message);
			}
		}
		else
			strcat(buf, " is here.");

		break;

	case POS_FIGHTING:
		strcat(buf, " is here, fighting ");

		if (victim->fighting == NULL)
			strcat(buf, "thin air??");
		else if (victim->fighting == ch)
			strcat(buf, "YOU! DOH!");
		else if (victim->in_room == victim->fighting->in_room) {
			strcat(buf, PERS(victim->fighting, ch, VIS_CHAR));
			strcat(buf, ".");
		}
		else
			strcat(buf, "with shadow-boxers??");

		break;
	}

	strcat(buf, "\n");
	buf[0] = UPPER(buf[0]);
	new_color(ch, IS_NPC(victim) ? CSLOT_MISC_MOBILES : CSLOT_MISC_PLAYERS);
	stc(buf, ch);
	set_color(ch, WHITE, NOBOLD);
	return;
}

void show_char_to_char_1(CHAR_DATA *victim, CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	int iWear;
	int percent;
	bool found;

	if (can_see(victim, ch)) {
		if (ch == victim)
			act("$n looks at $mself.", ch, NULL, NULL, TO_ROOM);
		else {
			act("$n looks at you.", ch, NULL, victim, TO_VICT);
			act("$n looks at $N.",  ch, NULL, victim, TO_NOTVICT);
		}
	}

	if (victim->description[0] != '\0') {
		set_color(ch, PURPLE, NOBOLD);
		stc(victim->description, ch);
		set_color(ch, WHITE, NOBOLD);
	}
	else {
		set_color(ch, PURPLE, NOBOLD);
		act("Nothing Special.", ch, NULL, victim, TO_CHAR);
		set_color(ch, WHITE, NOBOLD);
	}

	if (victim->max_hit > 0)
		percent = (100 * victim->hit) / victim->max_hit;
	else
		percent = -1;

	strcpy(buf, PERS(victim, ch, VIS_CHAR));

	if (percent >= 100)
		strcat(buf, " is in excellent condition.\n");
	else if (percent >= 90)
		strcat(buf, " has a few scratches.\n");
	else if (percent >= 75)
		strcat(buf, " has some small wounds and bruises.\n");
	else if (percent >=  50)
		strcat(buf, " has quite a few wounds.\n");
	else if (percent >= 30)
		strcat(buf, " has some big nasty wounds and scratches.\n");
	else if (percent >= 15)
		strcat(buf, " looks pretty hurt.\n");
	else if (percent >= 1)
		strcat(buf, " is in awful condition.\n");
	else if (percent >= 0)
		strcat(buf, " will soon be toast!!!\n");
	else
		strcat(buf, " is in need of ***SERIOUS*** medical attention!!!\n");

	buf[0] = UPPER(buf[0]);
	set_color(ch, CYAN, NOBOLD);
	stc(buf, ch);
	set_color(ch, WHITE, NOBOLD);
	found = FALSE;

	if (!IS_NPC(victim)) {
		set_color(ch, RED, BOLD);
		sprintf(buf, "%s has killed %d players and has been killed by %d players.\n",
		        GET_SEX(victim) == SEX_NEUTRAL ? "It" : GET_SEX(victim) == SEX_MALE ? "He" : "She"
		        , victim->pcdata->pckills, victim->pcdata->pckilled);
		stc(buf, ch);
		set_color(ch, WHITE, NOBOLD);
	}

	if (!IS_NPC(victim)) {
		set_color(ch, PURPLE, BOLD);
		sprintf(buf, "%s has killed %d players in the arena and has been defeated by %d players in the arena.\n",
		        GET_SEX(victim) == SEX_NEUTRAL ? "It" : GET_SEX(victim) == SEX_MALE ? "He" : "She"
		        , victim->pcdata->arenakills, victim->pcdata->arenakilled);
		stc(buf, ch);
		set_color(ch, WHITE, NOBOLD);
	}

	if (!IS_NPC(victim) && !IS_IMMORTAL(victim)) {
		sprintf(buf, "%s is a devout follower of %s.\n", victim->name,
		        victim->pcdata->deity);
		set_color(ch, CYAN, BOLD);
		stc(buf, ch);
		set_color(ch, WHITE, NOBOLD);
	}

	if (!IS_NPC(victim)) {
		set_color(ch, BLUE, BOLD);
		sprintf(buf, "%s is about %d years old.\n",
		        GET_SEX(victim) == SEX_NEUTRAL ? "It" : GET_SEX(victim) == SEX_MALE ? "He" : "She"
		        , get_age(victim));
		stc(buf, ch);
		set_color(ch, WHITE, NOBOLD);
	}

	for (iWear = 0; iWear < MAX_WEAR; iWear++) {
		if (!IS_NPC(victim) && victim->pcdata->spouse == NULL && iWear == WEAR_WEDDINGRING
		    && !IS_IMMORTAL(ch)) /* so imms can see weddingrings on unmarried ppl */
			continue;

		if ((obj = get_eq_char(victim, iWear)) != NULL
		    &&   can_see_obj(ch, obj)) {
			set_color(ch, GREEN, NOBOLD);

			if (!found) {
				stc("\n", ch);
				act("$N is using:", ch, NULL, victim, TO_CHAR);
				found = TRUE;
			}

			stc(where_name[iWear], ch);
			stc(format_obj_to_char(obj, ch, TRUE), ch);
			stc("\n", ch);
			set_color(ch, WHITE, NOBOLD);
		}
	}

	if (victim != ch
	    &&   !IS_NPC(ch)
	    &&   IS_SET(ch->pcdata->plr, PLR_AUTOPEEK)
	    && (!IS_IMMORTAL(victim) || IS_IMMORTAL(ch))
	    &&   number_percent() < get_skill(ch, gsn_peek)) {
		set_color(ch, YELLOW, NOBOLD);
		stc("\nYou peek at the inventory:\n", ch);
		show_list_to_char(victim->carrying, ch, TRUE, TRUE, FALSE);
		check_improve(ch, gsn_peek, TRUE, 4);
		/* Don't forget to deduct stamina for peeking. -- Outsider */
		deduct_stamina(ch, gsn_peek);
	}

	set_color(ch, WHITE, NOBOLD);
	return;
}

void show_char_to_char(CHAR_DATA *list, CHAR_DATA *ch)
{
	CHAR_DATA *rch;

	for (rch = list; rch != NULL; rch = rch->next_in_room) {
		if (rch == ch)
			continue;

		if (can_see(ch, rch))
			show_char_to_char_0(rch, ch);
		else if (room_is_dark(ch->in_room)
		         && !room_is_very_dark(ch->in_room)
		         && IS_AFFECTED(rch, AFF_INFRARED))
			stc("You see glowing red eyes watching YOU!\n", ch);
	}
}

bool check_blind(CHAR_DATA *ch)
{
	if (IS_IMMORTAL(ch))
		return TRUE;

	if (IS_AFFECTED(ch, AFF_BLIND)) {
		stc("You can't see a thing!\n", ch);
		return FALSE;
	}

	return TRUE;
}

/*
 * the first number bold = 0,1 gives you dim or BRIGHT colors
 * the second number determines the colors as follows:
 * 30 = grey, 31 = red, 32 = green, 33 = yellow, 34 = blue, 35 = purple
 * 36 = cyan, 37 = white
 * special notes: on a MAC the colors may be screwy, 0,30 gives you black
 * 0,34 seems to be REALLY dark so it doesn't show up too well...
 */
void set_color(CHAR_DATA *ch, int color, int bold)
{
	char buf[MAX_INPUT_LENGTH];

	if (IS_SET(ch->act, PLR_COLOR)) {
		sprintf(buf, "\033[%d;%dm", bold, color);
		stc(buf, ch);
	}

	if (!IS_NPC(ch)) {
		ch->pcdata->lastcolor[0] = color;
		ch->pcdata->lastcolor[1] = bold;
	}
}

void new_color(CHAR_DATA *ch, int slot)
{
	if (!IS_NPC(ch) && ch->pcdata->color[slot] != 0)
		set_color(ch, ch->pcdata->color[slot], ch->pcdata->bold[slot]);
	else
		set_color(ch, csetting_table[slot].color, csetting_table[slot].bold);
}

void goto_line(CHAR_DATA *ch, int row, int column)
{
	char buf[MAX_INPUT_LENGTH];
	sprintf(buf, "\033[%d;%dH", row, column);
	stc(buf, ch);
}

void set_window(CHAR_DATA *ch, int top, int bottom)
{
	char buf[MAX_INPUT_LENGTH];
	sprintf(buf, "\033[%d;%dr", top, bottom);
	stc(buf, ch);
}

/* changes your scroll */
void do_scroll(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[100];
	int lines;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		sprintf(buf, "You currently display %d lines per page.\n",
		        ch->lines);
		stc(buf, ch);
		return;
	}

	if (!is_number(arg)) {
		stc("You must provide a number.\n", ch);
		return;
	}

	lines = atoi(arg);

	if (lines < 10 || lines > 100) {
		stc("You must provide a reasonable number.\n", ch);
		return;
	}

	sprintf(buf, "Scroll set to %d lines.\n", lines);
	stc(buf, ch);
	ch->lines = lines;
} /* end set_scroll() */

void do_socials(CHAR_DATA *ch, const char *argument)
{
	BUFFER *dbuf;
	char buf[MAX_STRING_LENGTH];
	struct social_type *iterator;
	int col;
	col = 0;
	dbuf = new_buf();

	for (iterator = social_table_head->next; iterator != social_table_tail; iterator = iterator->next) {
		sprintf(buf, "%-12s", iterator->name);
		add_buf(dbuf, buf);

		if (++col % 6 == 0)
			add_buf(dbuf, "\n");
	}

	if (col % 6 != 0)
		add_buf(dbuf, "\n");

	page_to_char(buf_string(dbuf), ch);
	free_buf(dbuf);
} /* end do_socials() */

/* RT Commands to replace news, motd, imotd, etc from ROM */
void do_imotd(CHAR_DATA *ch, const char *argument)
{
	help(ch, "imotd");
}

void do_rules(CHAR_DATA *ch, const char *argument)
{
	help(ch, "rules");
}

void do_story(CHAR_DATA *ch, const char *argument)
{
	help(ch, "story");
}

void do_departed(CHAR_DATA *ch, const char *argument)
{
	DEPARTED_DATA *iterator;
	char name_buf[20];
	int i = 0;
	stc("\n", ch);
	stc(departed_list_line, ch);

	for (iterator = departed_list_head->next; iterator != departed_list_tail; iterator = iterator->next) {
		sprintf(name_buf, "%-18s", iterator->name);
		stc(name_buf, ch);
		i++;

		if (i % 2 == 0) {
			iterator = iterator->next;

			if (iterator == departed_list_tail) {
				stc("\n", ch);
				i = 0;
				break;
			}

			stc(iterator->name, ch);
			stc("\n", ch);
			stc(departed_list_line, ch);
		}
	}

	if (i != 0)
		stc("\n", ch);
}

void do_wizlist(CHAR_DATA *ch, const char *argument)
{
	help(ch, "wizlist");
}

/* Alphabetizing stuff. Fun. */
int compare_clans(const void *p1, const void *p2)
{
	return strcmp((*(CLAN_DATA **)p1)->name, (*(CLAN_DATA **)p2)->name);
}

void do_clanlist(CHAR_DATA *ch, const char *argument)
{
	char lblock[MIL], rblock[MIL];
	CLAN_DATA *clan;
	BUFFER *buffer;
	int count = 0, l = 0, r = 0, i = 0, highremort = 0, highlevel = 0;
	struct clan_clan {
		char    name[20];
		char    title[150];
		char    rank[20];
		sh_int  level;
		sh_int  remort;
		sh_int  status;
		bool    printed;
	};
	struct clan_clan clan_list[500];

	if (argument[0] == '\0') {
		CLAN_DATA *aClan[count_clans()], zero_clan;
		int index = 0;
		zero_clan.name = "zzzzzzzz";
		count = count_clans();

		while (index < count) {
			aClan[index] = &zero_clan;
			index++;
		}

		clan = clan_table_head->next;
		count = 0;

		while (clan != clan_table_tail) {
			if (clan == NULL)
				continue;

			if (!clan->independent)
				aClan[count] = clan;

			clan = clan->next;
			count++;
		}

		qsort(aClan, count, sizeof(aClan[0]), compare_clans);
		count = 0;
		set_color(ch, CYAN, BOLD);
		stc("Current Clans of Legacy:\n\n", ch);
		stc("Clan Name                         Mail Name      Logo     Creator\n", ch);
		stc("----------------------------------------------------------------------\n", ch);
		set_color(ch, CYAN, NOBOLD);

		while (strcmp(aClan[count]->name, "zzzzzzzz")) {
			ptc(ch, "%-34s{x%-14s{x%-9s{x %s{x\n",
			    aClan[count]->clanname,
			    aClan[count]->name,
			    aClan[count]->who_name,
			    aClan[count]->creator);
			count++;
		}

		set_color(ch, WHITE, NOBOLD);
		return;
	}

	/* argument given: list clan membership */
	if (!(clan = clan_lookup(argument))) {
		stc("That is not a clan.\n", ch);
		return;
	}

	if (db_queryf("do_clanlist",
	    "SELECT name, title, rank, level, remort, cgroup FROM pc_index WHERE clan LIKE '%s'", db_esc(clan->name)) != SQL_OK)
		return;

	while (db_next_row() == SQL_OK) {
		int cgroup = db_get_column_int(5);
		strcpy(clan_list[count].name, db_get_column_str(0));
		strcpy(clan_list[count].title, db_get_column_str(1));
		strcpy(clan_list[count].rank, db_get_column_str(2));
		clan_list[count].level  = db_get_column_int(3);
		clan_list[count].remort = db_get_column_int(4);
		clan_list[count].status = IS_SET(cgroup, GROUP_LEADER) ? 2 : IS_SET(cgroup, GROUP_DEPUTY) ? 1 : 0;
		clan_list[count].printed       = FALSE;

		if (clan_list[count].remort > highremort)
			highremort = clan_list[count].remort;

		if (clan_list[count].level > highlevel)
			highlevel = clan_list[count].level;

		count++;
	}

	buffer = new_buf();
	ptb(buffer, "{VMembers of %s:\n\n", clan->clanname);
	sprintf(rblock, "{x");
	sprintf(lblock, "{x");

	if (clan->independent)  /* we sort the imm clan by leader-deputy-imm ranks, not level */
		for (r = 2; r > -1; r--) {
			for (i = 0; i < count; i++) {
				if (clan_list[i].printed || clan_list[i].status != r)
					continue;

				ptb(buffer, "%s[{G%3s%s] {W[ {CIMM {W] %s%s{x\n",
				    r == 2 ? "{Y" : r == 1 ? "{B" : "{W",
				    clan_list[i].rank,
				    r == 2 ? "{Y" : r == 1 ? "{B" : "{W",
				    clan_list[i].name,
				    clan_list[i].title);
				clan_list[i].printed = TRUE;
			}
		}
	else
		for (r = highremort; r > -1; r--) {
			for (l = highlevel; l > 0; l--) {
				for (i = 0; i < count; i++) {
					if (clan_list[i].printed == TRUE
					    || clan_list[i].level != l
					    || (clan_list[i].remort != r
					        && clan_list[i].level < LEVEL_IMMORTAL))
						continue;

					if (l < LEVEL_IMMORTAL) {
						if (r)
							sprintf(rblock, "{G%2d{c/", r);
						else
							sprintf(rblock, "   ");

						sprintf(lblock, "{W[%s{B%2d{W]", rblock, l);
					}
					else
						sprintf(lblock, "{W[ {CIMM {W]");

					ptb(buffer, "%s[{G%3s%s] %s %s%s{x\n",
					    clan_list[i].status == 2 ? "{Y" :
					    clan_list[i].status == 1 ? "{B" : "{W",
					    clan_list[i].rank,
					    clan_list[i].status == 2 ? "{Y" :
					    clan_list[i].status == 1 ? "{B" : "{W",
					    lblock,
					    clan_list[i].name,
					    clan_list[i].title);
					clan_list[i].printed = TRUE;
				}
			}
		}

	if (count == 0)
		stc("- none -\n", ch);
	else {
		ptb(buffer, "\n{WThere are %d members of %s{x.\n", count, clan->clanname);
		page_to_char(buf_string(buffer), ch);
	}

	free_buf(buffer);
} /* end do_clanlist() */

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

void do_autolist(CHAR_DATA *ch, const char *argument)
{
	/* lists most player flags */
	if (IS_NPC(ch))
		return;

	stc("   action     status\n", ch);
	stc("---------------------\n", ch);
	stc("autoassist     ", ch);

	if (IS_SET(ch->act, PLR_AUTOASSIST))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("autoexit       ", ch);

	if (IS_SET(ch->act, PLR_AUTOEXIT))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("autogold       ", ch);

	if (IS_SET(ch->act, PLR_AUTOGOLD))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("autoloot       ", ch);

	if (IS_SET(ch->act, PLR_AUTOLOOT))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("autosac        ", ch);

	if (IS_SET(ch->act, PLR_AUTOSAC))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("autosplit      ", ch);

	if (IS_SET(ch->act, PLR_AUTOSPLIT))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("autotick       ", ch);

	if (IS_SET(ch->act, PLR_TICKS))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("autorecall     ", ch);

	if (IS_SET(ch->act, PLR_WIMPY))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("chatmode       ", ch);

	if (IS_SET(ch->pcdata->plr, PLR_CHATMODE))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("compact mode   ", ch);

	if (IS_SET(ch->comm, COMM_COMPACT))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("prompt         ", ch);

	if (IS_SET(ch->comm, COMM_PROMPT))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("show defensive ", ch);

	if (IS_SET(ch->act, PLR_DEFENSIVE))
		stc("OFF\n", ch);
	else
		stc("ON\n", ch);

	stc("show RAffects  ", ch);

	if (IS_SET(ch->pcdata->plr, PLR_SHOWRAFF))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("combine items  ", ch);

	if (IS_SET(ch->comm, COMM_COMBINE))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	stc("autopeek       ", ch);

	if (IS_SET(ch->pcdata->plr, PLR_AUTOPEEK))
		stc("ON\n", ch);
	else
		stc("OFF\n", ch);

	if (ch->pcdata) {
		stc("showlost       ", ch);

		if (IS_SET(ch->pcdata->plr, PLR_SHOWLOST))
			stc("ON\n", ch);
		else
			stc("OFF\n", ch);

		stc("flash mode     ", ch);

		if (IS_SET(ch->pcdata->video, VIDEO_FLASH_LINE))
			stc("UNDERLINE\n", ch);
		else if (IS_SET(ch->pcdata->video, VIDEO_FLASH_OFF))
			stc("OFF\n", ch);
		else
			stc("ON\n", ch);
	}

	if (!IS_SET(ch->act, PLR_CANLOOT))
		stc("Your corpse is safe from thieves.\n", ch);
	else
		stc("Your corpse may be looted.\n", ch);

	if (IS_SET(ch->act, PLR_NOSUMMON))
		stc("You cannot be summoned.\n", ch);
	else
		stc("You can be summoned.\n", ch);

	if (IS_SET(ch->act, PLR_NOFOLLOW))
		stc("You do not welcome followers.\n", ch);
	else
		stc("You accept followers.\n", ch);

	if (ch->pcdata) {
		if (IS_SET(ch->pcdata->plr, PLR_SHOWLOST))
			stc("You are notified about lost items when quiting.\n", ch);
		else
			stc("You are not notified about lost items when quiting.\n", ch);
	}
}

/* Color testing by Lotus */
void do_ctest(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act, PLR_COLOR)) {
		stc("Colors currently used at Legacy:\n", ch);
		stc("\n", ch);
		stc("{WWHITE     (W)  {gGREY      (g)  {e{WBG GREY      (e){a\n", ch);
		stc("{PPINK      (P)  {RRED       (R)  {r{PBG RED       (r){a\n", ch);
		stc("{YYELLOW    (Y)  {bBROWN     (b)  {y{YBG BROWN     (y){a\n", ch);
		stc("{GGREEN     (G)  {HHUNTER    (H)  {h{GBG HUNTER    (h){a\n", ch);
		stc("{CCYAN      (C)  {TTURQUOISE (T)  {t{CBG TURQUOISE (t){a\n", ch);
		stc("{BBLUE      (B)  {NNAVY      (N)  {n{BBG NAVY      (n){a\n", ch);
		stc("{VVIOLET    (V)  {MMAROON    (M)  {m{VBG MAROON    (m){a\n", ch);
		stc("{cCHARCOAL  (c)  {kBLACK     (k)  {a{cBG BLACK     (a){a\n", ch);
		stc("{PD{YE{GF{BA{MU{VL{WT   {x(x)  {fFLASHING  (f){x  {sREVERSE      (s){x\n", ch);
	}
	else
		stc("You must turn color on to see this.\n", ch);
}

void do_autoassist(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act, PLR_AUTOASSIST)) {
		stc("Autoassist removed.\n", ch);
		REMOVE_BIT(ch->act, PLR_AUTOASSIST);
	}
	else {
		stc("You will now assist when needed.\n", ch);
		SET_BIT(ch->act, PLR_AUTOASSIST);
	}
}

void do_defensive(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act, PLR_DEFENSIVE)) {
		stc("You will now see all blocks.\n", ch);
		REMOVE_BIT(ch->act, PLR_DEFENSIVE);
	}
	else {
		stc("You will no longer see defensive blocks.\n", ch);
		SET_BIT(ch->act, PLR_DEFENSIVE);
	}
}

void do_autoexit(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act, PLR_AUTOEXIT)) {
		stc("Exits will no longer be displayed.\n", ch);
		REMOVE_BIT(ch->act, PLR_AUTOEXIT);
	}
	else {
		stc("Exits will now be displayed.\n", ch);
		SET_BIT(ch->act, PLR_AUTOEXIT);
	}
}

void do_autogold(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act, PLR_AUTOGOLD)) {
		stc("Autogold removed.\n", ch);
		REMOVE_BIT(ch->act, PLR_AUTOGOLD);
	}
	else {
		stc("Automatic gold looting set.\n", ch);
		SET_BIT(ch->act, PLR_AUTOGOLD);
	}
}

void do_autoloot(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act, PLR_AUTOLOOT)) {
		stc("Autolooting removed.\n", ch);
		REMOVE_BIT(ch->act, PLR_AUTOLOOT);
	}
	else {
		stc("Automatic corpse looting set.\n", ch);
		SET_BIT(ch->act, PLR_AUTOLOOT);
	}
}

void do_autosac(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act, PLR_AUTOSAC)) {
		stc("Autosacrificing removed.\n", ch);
		REMOVE_BIT(ch->act, PLR_AUTOSAC);
	}
	else {
		stc("Automatic corpse sacrificing set.\n", ch);
		SET_BIT(ch->act, PLR_AUTOSAC);
	}
}

void do_autosplit(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act, PLR_AUTOSPLIT)) {
		stc("Autosplitting removed.\n", ch);
		REMOVE_BIT(ch->act, PLR_AUTOSPLIT);
	}
	else {
		stc("Automatic gold splitting set.\n", ch);
		SET_BIT(ch->act, PLR_AUTOSPLIT);
	}
}

void do_brief(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->comm, COMM_BRIEF)) {
		stc("Full descriptions activated.\n", ch);
		REMOVE_BIT(ch->comm, COMM_BRIEF);
	}
	else {
		stc("Short descriptions activated.\n", ch);
		SET_BIT(ch->comm, COMM_BRIEF);
	}
}

void do_compact(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->comm, COMM_COMPACT)) {
		stc("Compact mode removed.\n", ch);
		REMOVE_BIT(ch->comm, COMM_COMPACT);
	}
	else {
		stc("Compact mode set.\n", ch);
		SET_BIT(ch->comm, COMM_COMPACT);
	}
}

void do_showflags(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	victim = ch;

	if (IS_IMMORTAL(ch) && argument[0] != '\0') {
		if ((victim = get_char_world(ch, argument, VIS_PLR)) == NULL) {
			stc("You cannot locate that character.\n", ch);
			return;
		}
	}

	set_color(ch, YELLOW, NOBOLD);
	sprintf(buf, "Act  : %s\n", act_bit_name(victim->act));
	stc(buf, ch);

	if (!IS_NPC(victim)) {
		sprintf(buf, "Plr  : %s\n", plr_bit_name(victim->pcdata->plr));
		stc(buf, ch);
	}

	if (IS_NPC(victim)) {
		sprintf(buf, "Off  : %s\n", off_bit_name(victim->off_flags));
		stc(buf, ch);
	}

	if (!IS_NPC(victim)) {
		sprintf(buf, "Wiz  : %s\n", wiz_bit_name(victim->wiznet));
		stc(buf, ch);
	}

	sprintf(buf, "Aff  : %s\n", affect_bit_name(victim->affected_by));
	stc(buf, ch);
	ptc(ch, "Drn  : %s\n", imm_bit_name(victim->drain_flags));
	ptc(ch, "Imm  : %s\n", imm_bit_name(victim->imm_flags));
	ptc(ch, "Res  : %s\n", imm_bit_name(victim->res_flags));
	ptc(ch, "Vuln : %s\n", imm_bit_name(victim->vuln_flags));
	sprintf(buf, "Form : %s\nParts: %s\n",
	        form_bit_name(victim->form), part_bit_name(victim->parts));
	stc(buf, ch);
	set_color(ch, WHITE, NOBOLD);
	return;
}

void do_show(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->comm, COMM_SHOW_AFFECTS)) {
		stc("Affects will no longer be shown in score.\n", ch);
		REMOVE_BIT(ch->comm, COMM_SHOW_AFFECTS);
	}
	else {
		stc("Affects will now be shown in score.\n", ch);
		SET_BIT(ch->comm, COMM_SHOW_AFFECTS);
	}
}

void do_prompt(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];

	if (argument[0] == '\0') {
		if (IS_SET(ch->comm, COMM_PROMPT)) {
			stc("You will no longer see prompts.\n", ch);
			REMOVE_BIT(ch->comm, COMM_PROMPT);
		}
		else {
			stc("You will now see prompts.\n", ch);
			SET_BIT(ch->comm, COMM_PROMPT);
		}

		return;
	}

	if (!strcmp(argument, "default"))
		strcpy(buf, "{W<{C%h{Thp {G%m{Hma {B%v{Nst {W[{g%e{W] {R%X{W>{x ");
	else if (!strcmp(argument, "short"))
		strcpy(buf, "{W<{C%h{Thp {G%m{Hma {B%v{Nst{W>{x ");
	else if (!strcmp(argument, "long"))
		strcpy(buf, "{W<{C%h{g/{C%H{Thp {G%m{g/{G%M{Hma {B%v{Nst {W[{c%e{W] {Y%g{C/{g%s {W( {g%z {W) {R%X{W>{x ");
	else if (!strcmp(argument, "riches"))
		strcpy(buf, "{W<{C%h{g/{C%H{Thp {G%m{g/{G%M{Hma {B%v{Nst {W[{g%e{W] {Y%g{bg {W%s{gs{W>{x ");
	else if (!strcmp(argument, "immortal"))
		strcpy(buf, "{W<{Y({W%R{Y) {T-{C%r{T- {H[{G%e{H] {W%z {N[{B%w{N]{W>{x ");
	else if (!strcmp(argument, "align"))
		strcpy(buf, "{W<{C%h{g/{C%H{Thp {G%m{g/{G%M{Hma {B%v{Nst {W[{c%e{W] {R%X {V({M%a{V){W>{x ");
	else if (!strcmp(argument, "quest"))
		strcpy(buf, "{W<{C%h{g/{C%H{Thp {G%m{g/{G%M{Hma {B%v{Nst {Y({b%Q %q{Y){W>{x ");
	else {
//		if (strlen(argument) > 100)
//			argument[100] = '\0';

		strncpy(buf, argument, 100);

		if (str_suffix("%c", buf))
			strcat(buf, " ");
	}

	free_string(ch->prompt);
	ch->prompt = str_dup(buf);
	sprintf(buf, "Prompt set to %s\n", ch->prompt);
	stc(buf, ch);
}

void do_combine(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->comm, COMM_COMBINE)) {
		stc("Long inventory selected.\n", ch);
		REMOVE_BIT(ch->comm, COMM_COMBINE);
	}
	else {
		stc("Combined inventory selected.\n", ch);
		SET_BIT(ch->comm, COMM_COMBINE);
	}
}

void do_showlost(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->pcdata->plr, PLR_SHOWLOST)) {
		stc("You will no longer be notified of lost items.\n", ch);
		REMOVE_BIT(ch->pcdata->plr, PLR_SHOWLOST);
	}
	else {
		stc("You will now be notified of lost items.\n", ch);
		SET_BIT(ch->pcdata->plr, PLR_SHOWLOST);
	}
}

void do_noloot(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act, PLR_CANLOOT)) {
		stc("Your corpse is now safe from thieves.\n", ch);
		REMOVE_BIT(ch->act, PLR_CANLOOT);
	}
	else {
		stc("Your corpse may now be looted.\n", ch);
		SET_BIT(ch->act, PLR_CANLOOT);
	}
}

void do_nofollow(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act, PLR_NOFOLLOW)) {
		stc("You now accept followers.\n", ch);
		REMOVE_BIT(ch->act, PLR_NOFOLLOW);
	}
	else {
		stc("You no longer accept followers.\n", ch);
		SET_BIT(ch->act, PLR_NOFOLLOW);
		die_follower(ch);
	}
}

void do_nosummon(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		if (IS_SET(ch->imm_flags, IMM_SUMMON)) {
			stc("You are no longer immune to summon.\n", ch);
			REMOVE_BIT(ch->imm_flags, IMM_SUMMON);
		}
		else {
			stc("You are now immune to summoning.\n", ch);
			SET_BIT(ch->imm_flags, IMM_SUMMON);
		}
	}
	else {
		if (IS_SET(ch->act, PLR_NOSUMMON)) {
			stc("You are no longer immune to summon.\n", ch);
			REMOVE_BIT(ch->act, PLR_NOSUMMON);
		}
		else {
			stc("You are now immune to summoning.\n", ch);
			SET_BIT(ch->act, PLR_NOSUMMON);
		}
	}
}

void do_look(CHAR_DATA *ch, const char *argument)
{
	char buf  [MAX_STRING_LENGTH];
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	EXIT_DATA *pexit;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	const char *pdesc;
	int door;
	int number, count;

	if (ch->desc == NULL)
		return;

	if (get_position(ch) < POS_SLEEPING) {
		stc("You can't see anything!\n", ch);
		return;
	}

	if (get_position(ch) == POS_SLEEPING) {
		stc("You can't see anything, you're sleeping!\n", ch);
		return;
	}

	if (!check_blind(ch))
		return;

	if (!IS_IMMORTAL(ch)
	    &&   room_is_dark(ch->in_room)) {
		stc("It is pitch black ... \n", ch);
		show_char_to_char(ch->in_room->people, ch);
		return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	number = number_argument(arg1, arg3);
	count = 0;

	if (arg1[0] == '\0' || !str_cmp(arg1, "auto")) {
		if (ch->in_room == NULL)
			return;

		/* 'look' or 'look auto' */
		new_color(ch, CSLOT_MISC_ROOM);
		stc(ch->in_room->name, ch);
		set_color(ch, WHITE, NOBOLD);

		if (IS_IMMORTAL(ch)) {
			sprintf(buf, " [%d] (%s) (%d)",
			        ch->in_room->vnum,
			        ch->in_room->area->name,
			        ch->in_room->area->nplayer);
			set_color(ch, WHITE, BOLD);
			stc(buf, ch);
			set_color(ch, WHITE, NOBOLD);
		}

		stc("\n", ch);

		if (arg1[0] == '\0'
		    || (!IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF))) {
			stc("  ", ch);
			stc(ch->in_room->description, ch);
		}

		if (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT)) {
			stc("\n", ch);
			do_exits(ch, "auto");
		}

		new_color(ch, CSLOT_MISC_OBJECTS);
		show_list_to_char(ch->in_room->contents, ch, FALSE, FALSE, FALSE);
		set_color(ch, WHITE, NOBOLD);
		show_char_to_char(ch->in_room->people,   ch);
		return;
	}

	/* viewing room stuff */
	{
		DUEL_DATA *duel = duel_table_head->next;

		while (duel != duel_table_tail) {
			if (duel->accept_timer == 0 && ch->in_room == duel->arena->viewroom)
				break;

			duel = duel->next;
		}

		if (duel != duel_table_tail
		    && duel->challenger != NULL
		    && duel->defender != NULL
		    && ch != duel->challenger
		    && ch != duel->defender) {
			if (can_see(ch, duel->challenger)
			    && !IS_SET(duel->challenger->in_room->room_flags, ROOM_NOWHERE)
			    && !str_prefix1(arg1, duel->challenger->name)) {
				char_from_room(ch);
				char_to_room(ch, duel->challenger->in_room);
				do_look(ch, "");
				char_from_room(ch);
				char_to_room(ch, duel->arena->viewroom);
				return;
			}

			if (can_see(ch, duel->defender)
			    && !IS_SET(duel->defender->in_room->room_flags, ROOM_NOWHERE)
			    && !str_prefix1(arg1, duel->defender->name)) {
				char_from_room(ch);
				char_to_room(ch, duel->defender->in_room);
				do_look(ch, "");
				char_from_room(ch);
				char_to_room(ch, duel->arena->viewroom);
				return;
			}
		}
	}

	if (!str_cmp(arg1, "i") || !str_cmp(arg1, "in") || !str_cmp(arg1, "on")) {
		ROOM_INDEX_DATA *location;

		/* 'look in' */
		if (arg2[0] == '\0') {
			stc("Look in what?\n", ch);
			return;
		}

		/* Stuff for Lockers */
		if (!str_prefix1(arg2, "locker") && !IS_NPC(ch)) {
			if (IS_SET(ch->in_room->room_flags, ROOM_LOCKER)) {
				stc("Your locker contains:\n", ch);
				show_list_to_char(ch->pcdata->locker, ch, TRUE, TRUE, FALSE);
			}
			else
				stc("You do not see that here.\n", ch);

			return;
		}

		/* Stuff for strongboxes -- Elrac */
		if (!IS_NPC(ch) && !str_prefix1(arg2, "strongbox")) {
			if (!IS_HEROIC(ch)) {
				stc("Only heroes and former heroes have a strongbox.\n", ch);
				return;
			}

			if (ch->in_room && ch->in_room->vnum == ROOM_VNUM_STRONGBOX) {
				stc("Your strongbox contains:\n", ch);
				show_list_to_char(ch->pcdata->strongbox, ch, TRUE, TRUE, FALSE);
			}
			else
				stc("You do not see that here.\n", ch);

			return;
		}

		if ((obj = get_obj_here(ch, arg2)) == NULL) {
			stc("You do not see that here.\n", ch);
			return;
		}

		/*
		Some of the reasoning for checking contents AND whether the item is ITEM_COMPARTMENT
		may seem irrational, but it's actually so you can see the contents of an object that
		you can't get objects from.  Some uses might include glass cases in shops, a scroll
		sealed inside a crystal shard, etc.  These would have an object reset inside them,
		or be flagged ITEM_COMPARTMENT long enough to receive an item, and then the object
		inside could be completely ungettable, or gotten out through destroying the item.
		                                                        -- Montrey
		*/
		switch (obj->item_type) {
		default:
			if (!IS_OBJ_STAT(obj, ITEM_COMPARTMENT) && !obj->contains)
				stc("That is not a container.\n", ch);

			break;

		case ITEM_DRINK_CON:
			if (obj->value[1] <= 0 && !obj->contains) {
				stc("It is empty.\n", ch);
				break;
			}

			ptc(ch, "It's %sfilled with  a %s liquid.\n",
			    obj->value[1] < obj->value[0] / 4 ?     "less than half-" :
			    obj->value[1] < 3 * obj->value[0] / 4 ? "about half-" :
			    "more than half-",
			    liq_table[obj->value[2]].liq_color);
			break;

		case ITEM_CONTAINER:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			if (IS_SET(obj->value[1], CONT_CLOSED)) {
				stc("It is closed.\n", ch);
				break;
			}

			if (obj == donation_pit)
				SET_BIT(ch->act, PLR_LOOKINPIT);

			new_color(ch, CSLOT_MISC_OBJECTS);
			act("$p holds:", ch, obj, NULL, TO_CHAR);
			show_list_to_char(obj->contains, ch, TRUE, TRUE, FALSE);
			set_color(ch, WHITE, NOBOLD);

			if (obj == donation_pit)
				REMOVE_BIT(ch->act, PLR_LOOKINPIT);

			break;

		case ITEM_PORTAL:
			if ((location = get_room_index(obj->value[3])) == NULL) {
				if (!obj->contains)
					stc("It looks very empty..\n", ch);

				break;
			}

			stc("\n", ch);
			new_color(ch, CSLOT_MISC_ROOM);
			stc(location->name, ch);
			set_color(ch, WHITE, NOBOLD);

			if (IS_IMMORTAL(ch)) {
				sprintf(buf, " [Room %d]", location->vnum);
				set_color(ch, RED, NOBOLD);
				stc(buf, ch);
				set_color(ch, WHITE, NOBOLD);
			}

			stc("\n", ch);

			if (!IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF)) {
				stc("  ", ch);
				stc(location->description, ch);
			}

			stc("\n", ch);

			if (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT)) {
				char showexit[100];
				sprintf(showexit, "%d exits auto", location->vnum);
				do_at(ch, showexit);
			}

			new_color(ch, CSLOT_MISC_OBJECTS);
			show_list_to_char(location->contents, ch, FALSE, FALSE, FALSE);
			set_color(ch, WHITE, NOBOLD);
			show_char_to_char(location->people, ch);
			break;
		}

		if (obj->item_type != ITEM_CONTAINER    /* these 3 are containers regardless */
		    && obj->item_type != ITEM_CORPSE_NPC
		    && obj->item_type != ITEM_CORPSE_PC
		    && (IS_OBJ_STAT(obj, ITEM_COMPARTMENT) || obj->contains)) {
			if (!IS_OBJ_STAT(obj, ITEM_COMPARTMENT) && !IS_OBJ_STAT(obj, ITEM_TRANSPARENT))
				stc("The surface is opaque.\n", ch);
			else {
				new_color(ch, CSLOT_MISC_OBJECTS);
				act("$p holds:", ch, obj, NULL, TO_CHAR);
				show_list_to_char(obj->contains, ch, TRUE, TRUE, FALSE);
				set_color(ch, WHITE, NOBOLD);
			}
		}

		return;
	}

	if ((victim = get_char_here(ch, arg1, VIS_CHAR)) != NULL) {
		show_char_to_char_1(victim, ch);
		return;
	}

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (can_see_obj(ch, obj)) {
			/* player can see object */
			pdesc = get_extra_descr(arg3, obj->extra_descr);

			if (pdesc != NULL) {
				if (++count == number) {
					stc(pdesc, ch);
					return;
				}
				else continue;
			}

			pdesc = get_extra_descr(arg3, obj->pIndexData->extra_descr);

			if (pdesc != NULL) {
				if (++count == number) {
					stc(pdesc, ch);
					return;
				}
				else continue;
			}

			if (is_name(arg3, obj->name)) {
				if (++count == number) {
					stc(obj->description, ch);
					stc("\n", ch);
					return;
				}
			}
		}
	}

	for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content) {
		if (can_see_obj(ch, obj)) {
			pdesc = get_extra_descr(arg3, obj->extra_descr);

			if (pdesc != NULL) {
				if (++count == number) {
					stc(pdesc, ch);
					return;
				}
				else
					continue;
			}

			pdesc = get_extra_descr(arg3, obj->pIndexData->extra_descr);

			if (pdesc != NULL) {
				if (++count == number) {
					stc(pdesc, ch);
					return;
				}
				else
					continue;
			}
		}

		if (is_name(arg3, obj->name))
			if (++count == number) {
				stc(obj->description, ch);
				stc("\n", ch);
				return;
			}
	}

	pdesc = get_extra_descr(arg3, ch->in_room->extra_descr);

	if (pdesc != NULL) {
		if (++count == number) {
			stc(pdesc, ch);
			return;
		}
	}

	if (count > 0 && count != number) {
		if (count == 1)
			sprintf(buf, "You only see one %s here.\n", arg3);
		else
			sprintf(buf, "You only see %d of those here.\n", count);

		stc(buf, ch);
		return;
	}

	if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north")) door = 0;
	else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east")) door = 1;
	else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south")) door = 2;
	else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west")) door = 3;
	else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up")) door = 4;
	else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down")) door = 5;
	else {
		stc("You do not see that here.\n", ch);
		return;
	}

	/* 'look direction' */
	if ((pexit = ch->in_room->exit[door]) == NULL) {
		stc("Nothing special there.\n", ch);
		return;
	}

	if (pexit->description != NULL && pexit->description[0] != '\0')
		stc(pexit->description, ch);
	else
		stc("Nothing special there.\n", ch);

	if (pexit->keyword    != NULL
	    &&   pexit->keyword[0] != '\0'
	    &&   pexit->keyword[0] != ' ') {
		if (IS_SET(pexit->exit_info, EX_CLOSED))
			act("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
		else if (IS_SET(pexit->exit_info, EX_ISDOOR))
			act("The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR);
	}

	return;
}

void do_examine(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Examine what?\n", ch);
		return;
	}

	/* enable "examine locker" -- Elrac */
	if (!str_prefix1(argument, "locker"))
		strcpy(arg, "in locker");

	if (!str_prefix1(argument, "strongbox"))
		strcpy(arg, "in strongbox");

	do_look(ch, arg);

	if ((obj = get_obj_here(ch, arg)) != NULL) {
		switch (obj->item_type) {
		default:
			if (IS_OBJ_STAT(obj, ITEM_COMPARTMENT) || obj->contains) {
				sprintf(buf, "in %s", argument);
				do_look(ch, buf);
			}

			break;

		case ITEM_JUKEBOX:
			do_play(ch, "list");
			break;

		case ITEM_MONEY:
			if (obj->value[0] == 0) {
				if (obj->value[1] == 0)
					sprintf(buf, "Odd...there's no coins in the pile.\n");
				else if (obj->value[1] == 1)
					sprintf(buf, "Wow. One gold coin.\n");
				else
					sprintf(buf, "There are %d gold coins in the pile.\n", obj->value[1]);
			}
			else if (obj->value[1] == 0) {
				if (obj->value[0] == 1)
					sprintf(buf, "Wow. One silver coin.\n");
				else
					sprintf(buf, "There are %d silver coins in the pile.\n", obj->value[0]);
			}
			else
				sprintf(buf, "There are %d gold and %d silver coins in the pile.\n",
				        obj->value[1], obj->value[0]);

			stc(buf, ch);
			break;

		case ITEM_PORTAL:
		case ITEM_DRINK_CON:
		case ITEM_CONTAINER:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			sprintf(buf, "in %s", argument);
			do_look(ch, buf);
			break;
		}
	}
}

/*
** Show the rooms && portals which have exits leading INTO the current room
** -DF ( 04-06-98 )
*/
void exits_in(CHAR_DATA *ch)
{
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *room;
	EXIT_DATA *exit;
	OBJ_DATA *obj;
	BUFFER *output = NULL;
	char buf[1024];
	int vnum, in_room_vnum, i;
	char *dir_name[] = { "North", "East", "South", "West", "Up", "Down" };
	bool found = FALSE;

	if (ch->in_room == NULL)
		return;

	output = new_buf();
	in_room_vnum = ch->in_room->vnum;                                   /* Save our current rooms vnum */

	for (pArea = area_first; pArea; pArea = pArea->next) {              /* Every area */
		for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) { /* Every vnum in the area */
			room = get_room_index(vnum);

			if (room != NULL) {
				for (i = 0; i < 6; i++) {                               /* Every exit in the current room */
					exit = room->exit[i];

					if (exit != NULL) {
						if (exit->u1.to_room == NULL)
							continue;

						if (exit->u1.to_room->vnum == in_room_vnum) {   /* Does the exit lead to our room? */
							found = TRUE;
							sprintf(buf, "( %-6.6s ) from %s (%d) in (%s)\n",
							        dir_name[i],
							        room->name,
							        room->vnum,
							        room->area->name);
							add_buf(output, buf);
						} /* End if */
					} /* End else */
				} /* End for */
			} /* End if */
		} /* End for */
	} /* End for */

	if (!found)
		add_buf(output, "No rooms lead into this one.\n");

	found = FALSE;

	for (obj = object_list; obj != NULL; obj = obj->next) {
		if (obj->item_type == ITEM_PORTAL) {
			if (obj->in_room == NULL)
				continue;

			if (obj->value[3] == in_room_vnum) {
				found = TRUE;
				sprintf(buf, "( Portal ) %s in %s (%d) in (%s)\n",
				        obj->name,
				        obj->in_room->name,
				        obj->in_room->vnum,
				        obj->in_room->area->name);
				add_buf(output, buf);
			}                                                          /* End if */
		}
	}

	if (!found)
		add_buf(output, "No portals found that lead into this room.\n");

	page_to_char(buf_string(output), ch);
	free_buf(output);
} /* End do_exits_in */

/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits(CHAR_DATA *ch, const char *argument)
{
	extern char *const dir_name[];
	char buf[MAX_STRING_LENGTH];
	EXIT_DATA *pexit;
	bool found;
	bool fAuto;
	int door;

	if (!check_blind(ch))
		return;

	if (!str_cmp(argument, "in") && IS_IMMORTAL(ch)) {
		exits_in(ch);
		return;
	}

	fAuto  = !str_cmp(argument, "auto");

	if (fAuto)
		sprintf(buf, "[Exits:");
	else if (IS_IMMORTAL(ch))
		sprintf(buf, "Obvious exits from room %d:\n", ch->in_room->vnum);
	else
		sprintf(buf, "Obvious exits:\n");

	found = FALSE;

	for (door = 0; door <= 5; door++) {
		if ((pexit = ch->in_room->exit[door]) != NULL
		    &&   pexit->u1.to_room != NULL
		    &&   can_see_room(ch, pexit->u1.to_room)
		    &&   !IS_SET(pexit->exit_info, EX_CLOSED)) {
			found = TRUE;

			if (fAuto) {
				strcat(buf, " ");
				strcat(buf, dir_name[door]);
			}
			else {
				sprintf(buf + strlen(buf), "%-5s - %s",
				        capitalize(dir_name[door]),
				        room_is_dark(pexit->u1.to_room)
				        ?  "Too dark to tell"
				        : pexit->u1.to_room->name
				       );

				if (IS_IMMORTAL(ch))
					sprintf(buf + strlen(buf),
					        " (room %d)\n", pexit->u1.to_room->vnum);
				else
					sprintf(buf + strlen(buf), "\n");
			}
		}
	}

	if (!found)
		strcat(buf, fAuto ? " none" : "None.\n");

	if (fAuto)
		strcat(buf, "]\n");

	set_color(ch, WHITE, BOLD);
	stc(buf, ch);
	set_color(ch, WHITE, NOBOLD);
	return;
}

void do_worth(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		ptc(ch, "You have %ld gold and %ld silver.\n", ch->gold, ch->silver);
		return;
	}

	if (ch->level < LEVEL_HERO)
		ptc(ch, "You have %ld gold, %ld silver, and %d experience (%ld exp to level).\n",
		    ch->gold, ch->silver, ch->exp, (ch->level + 1) * exp_per_level(ch, ch->pcdata->points) - ch->exp);
	else
		ptc(ch, "You have %ld gold, %ld silver, and %d experience.\n", ch->gold, ch->silver, ch->exp);

	ptc(ch, "You have %ld gold and %ld silver in the bank.\n", ch->gold_in_bank, ch->silver_in_bank);
	ptc(ch, "You have earned %d quest points, %d skill points, %d roleplay points,\n",
	    ch->questpoints, ch->pcdata->skillpoints, ch->pcdata->rolepoints);
	ptc(ch, "%d trains, and %d practices.\n", ch->train, ch->practice);
	return;
}

char *day_number_suffix(int day)
{
	if (day > 4 && day < 20) return "th";

	if (day % 10 == 1)       return "st";

	if (day % 10 == 2)       return "nd";

	if (day % 10 == 3)       return "rd";

	return "th";
} /* end day_number_suffix() */

void new_day(void)
{
	char *suf;
	int day;
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;
	day = time_info.day + 1;
	suf = day_number_suffix(day);

	for (d = descriptor_list; d != NULL; d = d->next) {
		victim = d->original ? d->original : d->character;

		if (IS_PLAYING(d) && !IS_SET(victim->comm, COMM_NOANNOUNCE | COMM_QUIET))
			ptc(victim, "{BThera wakes to the Day of %s,\n%d%s of the Month of %s.{x\n",
			    day_name[day % 7], day, suf, month_name[time_info.month]);
	}
} /* end new_day() */

void do_time(CHAR_DATA *ch, const char *argument)
{
	extern char str_boot_time[];
	char buf[MAX_STRING_LENGTH];
	char *suf;
	int day;
	day     = time_info.day + 1;
	suf = day_number_suffix(day);
	sprintf(buf,
	        "{WIt is %d o'clock %s, Day of %s, %d%s of the Month of %s.{x\n",
	        (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
	        time_info.hour >= 12 ? "pm" : "am",
	        day_name[day % 7],
	        day, suf,
	        month_name[time_info.month]);
	stc(buf, ch);

	if (quest_double) {
		stc("{gQuest points are currently being doubled!", ch);

		if (IS_IMMORTAL(ch)) {
			int hours       = quest_double / 3600,
			    minutes     = (quest_double - (hours * 3600)) / 60,
			    seconds     = (quest_double - (hours * 3600)) - (minutes * 60);

			if (quest_double > 1200)
				stc("{Y", ch);

			stc(" (", ch);
			ptc(ch, "%2d:", hours);

			if (minutes > 9)
				ptc(ch, "%d:", minutes);
			else
				ptc(ch, "0%d:", minutes);

			if (seconds > 9)
				ptc(ch, "%d)", seconds);
			else
				ptc(ch, "0%d)", seconds);
		}

		stc("{x\n", ch);
	}

	sprintf(buf, "{gLegacy started up %s\rThe system time is: %s{x\r",
	        str_boot_time,
	        (char *) ctime(&current_time)
	       );
	stc(buf, ch);
	return;
}

void do_weather(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	static char *const sky_look[4] = {
		"bright and sunny",
		"cloudy",
#ifdef SEASON_CHRISTMAS
		"white with snow",
#else
		"raining with heavy droplets of water",
#endif
		"on fire with lightning"
	};

	if (!IS_OUTSIDE(ch)) {
		stc("You are inside and cannot see outdoors.\n", ch);
		return;
	}

	sprintf(buf, "The sky is %s and %s.\n",
	        sky_look[weather_info.sky],
	        weather_info.change >= 0
#ifdef SEASON_CHRISTMAS
	        ? "a cold southerly breeze blows"
	        : "a freezing northern gust blows"
#else
	        ? "a warm southerly breeze blows"
	        : "a cold northern gust blows"
#endif
	       );
	stc(buf, ch);
	return;
}

/* new whois by Montrey */
void do_whois(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	char block[MAX_INPUT_LENGTH];
	char clan[MAX_STRING_LENGTH];
	char *remort, *rank;
	BUFFER *output;
	CHAR_DATA *victim;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("You must provide a name.\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg, VIS_PLR)) == NULL) {
		stc("No one of that name is playing.\n", ch);
		return;
	}

	output = new_buf();
	/* first block "(RP)(PB)(NH) " */
	sprintf(block, "%s%s%s",
	        IS_SET(victim->pcdata->plr, PLR_OOC) ? "{W({YRP{W){x" : "",
	        IS_SET(victim->pcdata->plr, PLR_PAINT) ? "{W({VPB{W){x" : "",
	        IS_SET(victim->act, PLR_MAKEBAG) ? "{W({CNH{W){x" : "");
	add_buf(output, block);

	if (block[0] != '\0')
		add_buf(output, " ");

	/* second block "[99 100 Sup Pdn]"*/
	if (IS_REMORT(victim)) {
		sprintf(buf, "{G%2d{x", victim->pcdata->remort_count);
		remort = buf;
	}
	else
		remort = "  ";

	sprintf(block, "%s%s {B%d {P%3s {C%3s%s ",
	        (IS_SET(victim->pcdata->plr, PLR_PK)) ? "{P[{x" : "{g[{x",
	        remort,
	        victim->level,
	        pc_race_table[victim->race].who_name,
	        class_table[victim->class].who_name,
	        (IS_SET(victim->pcdata->plr, PLR_PK)) ? "{P]{x" : "{g]{x");
	add_buf(output, block);
	/* third block "name title" */
	sprintf(block, "%s%s%s{W%s{x %s{x\n",
	        (IS_SET(victim->comm, COMM_AFK)) ? "{b[AFK]{x " : "",
	        (IS_SET(victim->act, PLR_KILLER)) ? "{R(KILLER){x " : "",
	        (IS_SET(victim->act, PLR_THIEF)) ? "{B(THIEF){x " : "",
	        victim->name,
	        victim->pcdata->title);
	add_buf(output, block);
	/* second line */
	sprintf(clan, "{x"); /* ugly, do something better someday */

	if (victim->clan != NULL) {
		if (victim->pcdata->rank != NULL)
			rank = victim->pcdata->rank;
		else if (IS_SET(victim->pcdata->cgroup, GROUP_LEADER))
			rank = "Leader";
		else if (IS_SET(victim->pcdata->cgroup, GROUP_DEPUTY))
			rank = "Deputy";
		else
			rank = "Member";

		sprintf(clan, "%s{x of %s{x, ", rank, victim->clan->clanname);
	}

	if (IS_REMORT(victim) && victim->pcdata->status != NULL && victim->pcdata->status[0] != '\0')
		sprintf(remort, "%s{x, ", victim->pcdata->status);
	else
		remort = "";

	sprintf(block, "%s%s{PRating: %d{x\n", clan, remort, victim->pcdata->pkrank);
	add_buf(output, block);

	if (IS_SET(victim->comm, COMM_AFK))
		add_buf(output, victim->pcdata->afk);

	page_to_char(buf_string(output), ch);
	free_buf(output);
}

char *count_players(CHAR_DATA *ch)
{
	static char buf[100];
	int count = 0;
	DESCRIPTOR_DATA *d;

	for (d = descriptor_list; d != NULL; d = d->next)
		if (IS_PLAYING(d) && can_see_who(ch, d->character))
			count++;

	if (record_players_since_boot == count)
		sprintf(buf, "There are %d characters on, the most since last reboot.\n", count);
	else
		sprintf(buf, "There are %d characters on, the most since last reboot is %d.\n",
		        count, record_players_since_boot);

	return buf;
}

void do_count(CHAR_DATA *ch, const char *argument)
{
	set_color(ch, WHITE, BOLD);
	stc(count_players(ch), ch);
	ptc(ch, "The most players ever on at once is %d.\n", record_players);
	set_color(ch, WHITE, NOBOLD);
}

void do_who(CHAR_DATA *ch, const char *argument)
{
	struct s_charitem {
		CHAR_DATA *pch;
		long levelkey;
	}
	*charitems, tmp_charitem;
	char buf[MIL], buf2[MIL];
	char block1[MIL];  /* [level race class] */
	char block2[MIL];  /* [rank clan] */
	char rbuf[32];
	BUFFER *output;
	DESCRIPTOR_DATA *d;
	int iClass, iRace, iLevelLower = 0, iLevelUpper = MAX_LEVEL;
	int nNumber = 0, nMatch = 0, ndesc = 0;
	int j1, j2;
	bool rgfClass[MAX_CLASS], rgfRace[MAX_PC_RACE];
	bool fClassRestrict = FALSE, fClanRestrict = FALSE, fRaceRestrict = FALSE, fImmortalOnly = FALSE;
	bool fClan = FALSE;
	bool fPK = FALSE;
	CHAR_DATA *wch;
	CLAN_DATA *cch = NULL;
	const char *class;
	char *rank, *p, *q, *lbrk, *rbrk, *remort;

	/* Set default arguments. */
	for (iClass = 0; iClass < MAX_CLASS; iClass++)
		rgfClass[iClass] = FALSE;

	for (iRace = 0; iRace < MAX_PC_RACE; iRace++)
		rgfRace[iRace] = FALSE;

	/* Parse arguments. */
	for (; ;) {
		char arg[MIL];
		argument = one_argument(argument, arg);

		if (arg[0] == '\0')
			break;

		if (is_number(arg)) {
			switch (++nNumber) {
			case 1:         iLevelLower = atoi(arg);        break;

			case 2:         iLevelUpper = atoi(arg);        break;

			default:
				stc("Find players between what two levels?\n", ch);
				return;
			}
		}
		else {
			/* Look for classes to turn on. */
			if (arg[0] == 'i')
				fImmortalOnly = TRUE;
			else if (!str_prefix1(arg, "pk"))
				fPK = TRUE;
			else if (!str_cmp(arg, "clan"))
				fClanRestrict = TRUE;
			else {
				iClass = class_lookup(arg);

				if (iClass == -1) {
					iRace = race_lookup(arg);

					if (iRace == 0 || iRace >= MAX_PC_RACE) {
						if ((cch = clan_lookup(arg)) != NULL)
							fClan = TRUE;
						else {
							stc("That's not a valid race, class, clan, or way to crash this mud.\n", ch);
							return;
						}
					}
					else {
						fRaceRestrict = TRUE;
						rgfRace[iRace] = TRUE;
					}
				}
				else {
					fClassRestrict = TRUE;
					rgfClass[iClass] = TRUE;
				}
			}
		}
	}

	/* Count up descriptors to get max # of players */
	for (d = descriptor_list; d != NULL; d = d->next)
		ndesc++;

	charitems = alloc_mem(ndesc * sizeof(struct s_charitem));
	/* Now show matching chars. */
	buf[0] = '\0';
	output = new_buf();

	for (d = descriptor_list; d != NULL; d = d->next) {
		/* Check for match against restrictions. */
		if (!IS_PLAYING(d) || !can_see_who(ch, d->character))
			continue;

		wch = d->original ? d->original : d->character;

		if (!can_see_who(ch, wch))
			continue;

		if (wch->level < iLevelLower
		    || wch->level > iLevelUpper
		    || (fImmortalOnly && wch->level < LEVEL_IMMORTAL)
		    || (fPK && !IS_SET(wch->pcdata->plr, PLR_PK))
		    || (fClassRestrict && !rgfClass[wch->class])
		    || (fRaceRestrict && !rgfRace[wch->race])
		    || (fClan && wch->clan != cch)
		    || (fClanRestrict && !wch->clan))
			continue;

		charitems[nMatch].pch = wch;

		/* add in a fudge factor to force sort order for major groups */
		if (wch->level >= LEVEL_IMMORTAL) {
			charitems[nMatch].levelkey = 99000 + wch->level;

			if (IS_SET(wch->pcdata->cgroup, GROUP_LEADER))
				charitems[nMatch].levelkey += 2000;
			else if (IS_SET(wch->pcdata->cgroup, GROUP_DEPUTY))
				charitems[nMatch].levelkey += 1000;
		}
		else
			charitems[nMatch].levelkey = 1000 * wch->pcdata->remort_count + wch->level;

		nMatch++;
	}

	/* Sort the players on level. */
	for (j1 = 0; j1 < nMatch - 1; j1++) {
		for (j2 = j1 + 1; j2 < nMatch; j2++) {
			if (charitems[j2].levelkey > charitems[j1].levelkey) {
				tmp_charitem = charitems[j1];
				charitems[j1] = charitems[j2];
				charitems[j2] = tmp_charitem;
			}
		}
	}

	/* Print sorted player list. */
	for (j1 = 0; j1 < nMatch; j1++) {
		wch = charitems[j1].pch;
		/*** Block 1 stuff ***/
		/* Imm:    Immname */
		/* Mortal: [lvl race class] */
		class = class_table[wch->class].who_name;

		if (IS_REMORT(wch)) {
			sprintf(rbuf, "{G%2d{x", wch->pcdata->remort_count);
			remort = rbuf;
		}
		else
			remort = "  ";

		if (wch->level >= LEVEL_IMMORTAL && wch->pcdata->immname[0]) {
			/* copy immname without the brackets */
			for (q = block1, *q++ = ' ', p = wch->pcdata->immname; *p; p++)
				if (*p != '[' && *p != ']')
					*q++ = *p;

			*q = '\0';
		}
		else {
			if (char_at_war(wch)) {
				lbrk = "<";
				rbrk = ">";
			}
			else {
				lbrk = "[";
				rbrk = "]";
			}

			sprintf(block1, "%s%s%s {B%2d {P%3s {C%3s%s%s{x",
			        IS_SET(wch->pcdata->plr, PLR_PK) ? "{P" : "{W",
			        lbrk,
			        remort,
			        wch->level,
			        pc_race_table[wch->race].who_name,
			        class,
			        IS_SET(wch->pcdata->plr, PLR_PK) ? "{P" : "{W",
			        rbrk);
		}

		/*** Block 2 stuff ***/
		/* Unclanned: "          "      */
		/* Leader:    [clanname *rank*]  */
		/* Other:     [clanname rank]  */

		if (wch->clan || wch->level >= LEVEL_IMMORTAL) {
			sprintf(rbuf, "%s{x%3s{x",
			        IS_SET(wch->pcdata->cgroup, GROUP_LEADER) ? "{Y~" :
			        IS_SET(wch->pcdata->cgroup, GROUP_DEPUTY) ? "{B~" : " ",
			        wch->pcdata->rank[0] ? wch->pcdata->rank :
			        wch->level >= LEVEL_IMMORTAL ? "{WIMM" : "   ");
			rank = rbuf;
		}
		else
			rank = "    ";

		sprintf(block2, "%s %s{a{x %s%s%s",
		        rank,
		        wch->clan ? wch->clan->who_name : "       ",
		        IS_SET(wch->act, PLR_MAKEBAG)       ? "{CH{x" : "{T-{x",
		        IS_SET(wch->pcdata->plr, PLR_OOC)   ? "{YR{x" : "{b-{x",
		        IS_SET(wch->pcdata->plr, PLR_PAINT) ? "{V*{x" : "{M-{x");

		/* Format it up. */
		/* new format -- Elrac */
		if (IS_SET(wch->act, PLR_SUPERWIZ)) {
			lbrk = "{P({R";
			rbrk = "{P)";
		}
		else if (wch->invis_level && wch->lurk_level) {
			lbrk = "{T{{";
			rbrk = "{T}";
		}
		else if (wch->invis_level) {
			lbrk = "{H<";
			rbrk = "{H>";
		}
		else if (wch->lurk_level) {
			lbrk = "{G(";
			rbrk = "{G)";
		}
		else {
			lbrk = "{W";
			rbrk = "";
		}

		sprintf(buf, "%s %s ", block1, block2);

		if (IS_SET(wch->comm, COMM_AFK))        strcat(buf, "{b[AFK]{x ");

		if (IS_SET(wch->act, PLR_KILLER))       strcat(buf, "{R(KILLER){x ");

		if (IS_SET(wch->act, PLR_THIEF))        strcat(buf, "{B(THIEF){x ");

		strcat(buf, lbrk);
		strcat(buf, wch->name);
		strcat(buf, rbrk);
		strcat(buf, "{x");
		strcat(buf, wch->pcdata->title);
		strcat(buf, "{x{a{W \n");
		add_buf(output, buf);
	}

	free_mem(charitems, ndesc * sizeof(struct s_charitem));
	stc("\n                             {C*{W*{C* {BL{Ce{gg{Wa{Cc{By {C*{W*{C*{x\n\n", ch);
	sprintf(buf2, "\n");
	add_buf(output, buf2);
	add_buf(output, count_players(ch));
	add_buf(output, "\n");
	set_color(ch, WHITE, BOLD);
	page_to_char(buf_string(output), ch);
	set_color(ch, WHITE, NOBOLD);
	free_buf(output);

} /* do_who() */

/* Short Who by Lotus */
void do_swho(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char roombuf[MAX_STRING_LENGTH];
	BUFFER *output;
	DESCRIPTOR_DATA *d;
	output = new_buf();
	add_buf(output, "{WRP PK NH PB QW SQ NT  Name          Pos'n   Room Name{x\n");
	add_buf(output, "{W----------------------------------------------------------------------------{x\n");

	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *wch;
		char const *position = "Drooling";

		if (!IS_PLAYING(d) || !can_see_who(ch, d->character))
			continue;

		wch   = (d->original != NULL) ? d->original : d->character;

		if (!can_see_who(ch, wch))
			continue;

		switch (get_position(wch)) {
		case POS_DEAD:     position = "{WDead!";              break;

		case POS_MORTAL:   position = "{WWound";              break;

		case POS_INCAP:    position = "{WIncap";              break;

		case POS_STUNNED:  position = "{WStun ";              break;

		case POS_SLEEPING: position = "{gSleep";              break;

		case POS_RESTING:  position = "{bRest ";              break;

		case POS_SITTING:  position =
//			        (wch->on && wch->on->pIndexData->item_type == ITEM_COACH) ? "{YRide " : 
				"Sit  ";    break;

		case POS_FIGHTING: position = "{WFight";              break;

		case POS_STANDING: position = "Stand";                break;
		}

		/* Imms can now see rooms of privated mortals -- Elrac */
		if (!IS_SET(wch->pcdata->plr, PLR_PRIVATE)) {
			int l;
			strcpy(roombuf, wch->in_room->name);
			l = strlen(roombuf);

			while (color_strlen(roombuf) > 37)
				roombuf[--l] = '\0';
		}
		else if (!IS_IMMORTAL(ch) || IS_IMMORTAL(wch))
			sprintf(roombuf, "Private");
		else {
			int l;
			strcpy(roombuf + 1, wch->in_room->name);
			roombuf[0] = '~';
			l = strlen(roombuf);

			while (color_strlen(roombuf) > 36)
				roombuf[--l] = '\0';

			strcpy(roombuf + l, "{V~");
		}

		sprintf(buf, "{b[%s][%s][%s][%s][%s][%s][%s{b] {W%-12s {R({P%s{R) {M<{V%s{M>\n",
		        IS_SET(wch->pcdata->plr, PLR_OOC) ? "{Y*{b" : " ",
		        IS_SET(wch->pcdata->plr, PLR_PK) ? "{G*{b" : " ",
		        IS_SET(wch->act, PLR_MAKEBAG) ? "{T*{b" : " ",
		        IS_SET(wch->pcdata->plr, PLR_PAINT) ? "{P*{b" : " ",
		        IS_SET(wch->act, PLR_QUESTOR) ? "{V*{b" : " ",
		        IS_SET(wch->pcdata->plr, PLR_SQUESTOR) ? "{B*{b" : " ",
		        (wch->pnote != NULL) ?
		        wch->pnote->type == NOTE_NOTE           ? "{P*" :
		        wch->pnote->type == NOTE_IDEA           ? "{Y*" :
		        wch->pnote->type == NOTE_ROLEPLAY       ? "{V*" :
		        wch->pnote->type == NOTE_IMMQUEST       ? "{B*" :
		        wch->pnote->type == NOTE_CHANGES        ? "{G*" :
		        wch->pnote->type == NOTE_PERSONAL       ? "{C*" :
		        wch->pnote->type == NOTE_TRADE          ? "{b*" :
		        "{c?" :
		        " ",
		        wch->name,
		        position,
		        roombuf);
		add_buf(output, buf);
	} /* end for */

	add_buf(output, "{W----------------------------------------------------------------------------{x\n");
	page_to_char(buf_string(output), ch);
	free_buf(output);
} /* end do_swho() */

/* New container capable inventory - Lotus */
void do_inventory(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim = NULL;
	char arg1[MAX_INPUT_LENGTH];

	if (argument[0] == '\0') {
		victim = ch;
		new_color(ch, CSLOT_MISC_INV);
		stc("You are carrying:\n", ch);
	}
	else {
		argument = one_argument(argument, arg1);

		if (!str_cmp(arg1, "char") && IS_IMMORTAL(ch)) {
			victim = get_char_world(ch, argument, VIS_PLR);

			if (victim == NULL) {
				stc("They aren't in the game.\n", ch);
				return;
			}

			new_color(ch, CSLOT_MISC_INV);
			act_new("$N is carrying:", ch, NULL, victim, TO_CHAR, POS_SLEEPING, FALSE);
		}
		else if (!str_cmp(arg1, "all")) {
			victim = ch;
			new_color(ch, CSLOT_MISC_INV);
			stc("You are carrying:\n", ch);
			show_list_to_char(victim->carrying, ch, TRUE, TRUE, TRUE);
			set_color(ch, WHITE, NOBOLD);
			return;
		}
		else {
			stc("Syntax: Inventory or Inventory all\n", ch);

			if (IS_IMMORTAL(ch))
				stc("      : Inventory char <victim>\n", ch);

			return;
		}
	}

	show_list_to_char(victim->carrying, ch, TRUE, TRUE, FALSE);
	set_color(ch, WHITE, NOBOLD);
}

void do_equipment(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	int iWear;
	char buf[MSL];
	CHAR_DATA *victim;

	if (IS_IMMORTAL(ch) && argument[0]) {
		if ((victim = get_player_world(ch, argument, VIS_PLR)) == NULL)
			if ((victim = get_char_world(ch, argument, VIS_PLR)) == NULL) {
				stc("They're not in the game.\n", ch);
				return;
			}

		set_color(ch, GREEN, NOBOLD);
		act_new("$N is using:", ch, NULL, victim, TO_CHAR, POS_SLEEPING, FALSE);
	}
	else {
		victim = ch;
		set_color(ch, GREEN, NOBOLD);
		stc("You are using:\n", ch);
	}

	for (iWear = 0; iWear < MAX_WEAR; iWear++) {
		if (ch->pcdata)
			if (iWear == WEAR_WEDDINGRING && ch->pcdata->spouse == NULL && !IS_IMMORTAL(ch))
				continue;

		strcpy(buf, where_name[iWear]);

		if ((obj = get_eq_char(victim, iWear)) == NULL) {
			if (!IS_IMMORTAL(victim)) {
				/* mortals can not hold shield & held AND secondary wpn */
				if ((iWear == WEAR_SHIELD || iWear == WEAR_HOLD)
				    && get_eq_char(victim, WEAR_SECONDARY) != NULL)
					continue;

				if (iWear == WEAR_SECONDARY)
					continue;
			}

			strcat(buf, "       ({W- - -{x)");
		}
		else if (can_see_obj(ch, obj)) {
			int spaces;

			if (obj->num_settings == 0)
				spaces = MAX_GEM_SETTINGS + 3;
			else
				spaces = (MAX_GEM_SETTINGS - obj->num_settings);

			while (spaces-- > 0)
				strcat(buf, " ");

			strcat(buf, format_obj_to_char(obj, ch, TRUE));
		}
		else
			strcat(buf, "       (something)");

		strcat(buf, "\n");
		stc(buf, ch);
	}

	set_color(ch, WHITE, NOBOLD);
}

void do_compare(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj1;
	OBJ_DATA *obj2;
	int value1;
	int value2;
	char *msg;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0') {
		stc("Compare what to what?\n", ch);
		return;
	}

	if ((obj1 = get_obj_carry(ch, arg1)) == NULL) {
		stc("You do not have one in your inventory.\n", ch);
		return;
	}

	if (arg2[0] == '\0') {
		for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content) {
			if (obj2->wear_loc != WEAR_NONE
			    &&  can_see_obj(ch, obj2)
			    &&  obj1->item_type == obj2->item_type
			    && (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0)
				break;
		}

		if (obj2 == NULL) {
			stc("You aren't wearing anything comparable.\n", ch);
			return;
		}
	}
	else if ((obj2 = get_obj_carry(ch, arg2)) == NULL) {
		stc("You do not have one in your inventory.\n", ch);
		return;
	}

	msg         = NULL;
	value1      = 0;
	value2      = 0;

	if (obj1 == obj2)
		msg = "You compare $p to itself.  It looks about the same.";
	else if (obj1->item_type != obj2->item_type)
		msg = "You can't compare $p and $P.";
	else {
		switch (obj1->item_type) {
		default:
			msg = "You can't compare $p and $P.";
			break;

		case ITEM_ARMOR:
			value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
			value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
			break;

		case ITEM_WEAPON:
			value1 = (1 + obj1->value[2]) * obj1->value[1];
			value2 = (1 + obj2->value[2]) * obj2->value[1];
			break;
		}
	}

	if (msg == NULL) {
		if (value1 == value2) msg = "$p and $P look about the same.";
		else if (value1  > value2) msg = "$p looks better than $P.";
		else                         msg = "$p looks worse than $P.";
	}

	act(msg, ch, obj1, obj2, TO_CHAR);
	return;
}

void do_credits(CHAR_DATA *ch, const char *argument)
{
	help(ch, "diku");
	help(ch, "credit");
	return;
}

void do_where(CHAR_DATA *ch, const char *argument)
{
	ARENA_DATA *arena = arena_table_head->next;
	char arg[MIL];
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;
	bool found = FALSE;
	one_argument(argument, arg);

	if (ch->in_room == NULL)
		return;

	while (arena != arena_table_tail) {
		if (ch->in_room->vnum >= arena->minvnum
		    && ch->in_room->vnum <= arena->maxvnum)
			break;

		arena = arena->next;
	}

	if (arena != arena_table_tail) {
		for (victim = char_list; victim != NULL; victim = victim->next) {
			if (victim->in_room != NULL
			    && victim->in_room->vnum >= arena->minvnum
			    && victim->in_room->vnum <= arena->maxvnum
			    && !IS_SET(victim->in_room->room_flags, ROOM_NOWHERE)
			    && can_see(ch, victim)) {
				found = TRUE;
				ptc(ch, "%-28s{x %s{x\n", PERS(victim, ch, VIS_CHAR), victim->in_room->name);
			}
		}

		if (!found)
			stc("You can't see anyone nearby...\n", ch);

		return;
	}

	if (arg[0] == '\0') {
		stc("Nearby you see:\n", ch);

		for (d = descriptor_list; d; d = d->next) {
			victim = d->character;

			if (IS_PLAYING(d)
			    && victim->in_room != NULL
			    && !IS_SET(victim->in_room->room_flags, ROOM_NOWHERE)
			    && victim->in_room->area == ch->in_room->area
			    && can_see(ch, victim)) {
				found = TRUE;
				ptc(ch, "%-28s{x %s{x\n", victim->name, victim->in_room->name);
			}
		}

		if (!found)
			stc("None.\n", ch);
	}
	else {
		for (victim = char_list; victim != NULL; victim = victim->next) {
			if (victim->in_room != NULL
			    && victim->in_room->area == ch->in_room->area
			    && !IS_AFFECTED(victim, AFF_HIDE)
			    && can_see(ch, victim)
			    && is_name(arg, victim->name)) {
				found = TRUE;
				ptc(ch, "%-28s{x %s{x\n", PERS(victim, ch, VIS_CHAR), victim->in_room->name);
				break;
			}
		}

		if (!found)
			act("You didn't find any $T.", ch, NULL, arg, TO_CHAR);
	}
}

/* New short consider by Lotus */
void do_scon(CHAR_DATA *ch, const char *argument)
{
	char arg[MIL];
	CHAR_DATA *victim;
	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		if ((victim = ch->fighting) == NULL) {
			stc("Which way did he go?!?\n", ch);
			return;
		}
	}
	else if ((victim = get_char_here(ch, arg, VIS_CHAR)) == NULL) {
		stc("They're not here.\n", ch);
		return;
	}

	ptc(ch, "{GN{H[{G%s{H] {CL{T[{C%d{T]\n"
	    "{CHp{T[{C%d{T/{C%d{T] {GMa{H[{G%d{H/{G%d{H] {BSt{N[{B%d{N/{B%d{N]{x\n",
	    victim->name, victim->level,
	    victim->hit, victim->max_hit,
	    victim->mana, victim->max_mana,
	    victim->stam, victim->max_stam);

	if (!strcmp(argument, "more")) {
		set_color(ch, WHITE, BOLD);
		ptc(ch, "Align[%d] Hit[%d] Dam[%d] AC[%d %d %d %d]\n",
		    victim->alignment, GET_HITROLL(victim), GET_DAMROLL(victim),
		    GET_AC(victim, AC_PIERCE), GET_AC(victim, AC_BASH),
		    GET_AC(victim, AC_SLASH),  GET_AC(victim, AC_EXOTIC));

		if (victim->drain_flags)        ptc(ch, " Drain:  %s\n", imm_bit_name(victim->drain_flags));

		if (victim->imm_flags)          ptc(ch, " Immune: %s\n", imm_bit_name(victim->imm_flags));

		if (victim->res_flags)          ptc(ch, " Resist: %s\n", imm_bit_name(victim->res_flags));

		if (victim->vuln_flags)         ptc(ch, " Vuln:   %s\n", imm_bit_name(victim->vuln_flags));

		if (victim->affected_by)
			ptc(ch, "Affects: %s\n", affect_bit_name(victim->affected_by));

		set_color(ch, WHITE, NOBOLD);
	}
}

void do_consider(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	AFFECT_DATA *paf;
	CHAR_DATA *victim;
	int diff, percent;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Consider killing whom?\n", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg, VIS_CHAR)) == NULL) {
		stc("They're not here.\n", ch);
		return;
	}

	/* New Consider by Clerve */
	if (HAS_CGROUP(ch, GROUP_AVATAR)) {
		ptc(ch, "{CYou see %s (level %d)\n", victim->name, victim->level);
		ptc(ch, "{TRace: %s  Sex: %s  Class: %s  Size: %s\n",
		    race_table[victim->race].name,
		    sex_table[GET_SEX(victim)].name,
		    IS_NPC(victim) ? "mobile" : class_table[victim->class].name,
		    size_table[victim->size].name);
		ptc(ch, "{PStr: %-2d(%-2d)\t{BAC Pierce : %-10d{YHit Points: %d/%d\n",
		    victim->perm_stat[STAT_STR], get_curr_stat(victim, STAT_STR),
		    GET_AC(victim, AC_PIERCE), victim->hit, victim->max_hit);
		ptc(ch, "{PInt: %-2d(%-2d)\t{BAC Bash   : %-10d{YMana      : %d/%d\n",
		    victim->perm_stat[STAT_INT], get_curr_stat(victim, STAT_INT),
		    GET_AC(victim, AC_BASH), victim->mana, victim->max_mana);
		ptc(ch, "{PWis: %-2d(%-2d)\t{BAC Slash  : %-10d{YStamina   : %d/%d\n",
		    victim->perm_stat[STAT_WIS], get_curr_stat(victim, STAT_WIS),
		    GET_AC(victim, AC_SLASH), victim->stam, victim->max_stam);
		ptc(ch, "{PDex: %-2d(%-2d)\t{BAC Magic  : %-10d{GHit Roll  : %d\n",
		    victim->perm_stat[STAT_DEX], get_curr_stat(victim, STAT_DEX),
		    GET_AC(victim, AC_EXOTIC), GET_HITROLL(victim));
		ptc(ch, "{PCon: %-2d(%-2d)\t\t\t      {GDam Roll  : %d\n",
		    victim->perm_stat[STAT_CON], get_curr_stat(victim, STAT_CON),
		    GET_DAMROLL(victim));
		ptc(ch, "{PChr: %-2d(%-2d)\t{WSaves     : %-10dAlignment : %d\n",
		    victim->perm_stat[STAT_CHR], get_curr_stat(victim, STAT_CHR),
		    victim->saving_throw, victim->alignment);

		if (IS_NPC(victim)) {
			ptc(ch, "\t\t{WDamage    : %2dd%-2d     Message   : %s\n",
			    victim->damage[DICE_NUMBER], victim->damage[DICE_TYPE],
			    attack_table[victim->dam_type].noun);
			ptc(ch, "\t\t{WCount     : %-10dKilled    : %d\n",
			    victim->pIndexData->count, victim->pIndexData->killed);
		}

		ptc(ch, "\n{gAct: %s\n", act_bit_name(victim->act));

		if (!IS_NPC(victim))
			ptc(ch, "{gPlr: %s\n", plr_bit_name(victim->pcdata->plr));

		if (IS_NPC(victim) && victim->off_flags)
			ptc(ch, "{gOff: %s\n", off_bit_name(victim->off_flags));

		if (victim->drain_flags)        ptc(ch, "{gDrn: %s\n", imm_bit_name(victim->drain_flags));

		if (victim->imm_flags)          ptc(ch, "{gImm: %s\n", imm_bit_name(victim->imm_flags));

		if (victim->res_flags)          ptc(ch, "{gRes: %s\n", imm_bit_name(victim->res_flags));

		if (victim->vuln_flags)         ptc(ch, "{gVul: %s\n", imm_bit_name(victim->vuln_flags));

		ptc(ch, "{gForm: %s\n{gParts: %s\n",
		    form_bit_name(victim->form), part_bit_name(victim->parts));
		ptc(ch, "{gMaster: %s  {gLeader: %s  {gPet: %s\n",
		    victim->master ? victim->master->name : "(none)",
		    victim->leader ? victim->leader->name : "(none)",
		    victim->pet   ? victim->pet->name   : "(none)");

		if (IS_NPC(victim) && victim->spec_fun != 0)
			ptc(ch, "{gMobile has special procedure %s.\n", spec_name(victim->spec_fun));

		if (victim->affected_by)
			ptc(ch, "{gAffected by: %s\n", affect_bit_name(victim->affected_by));

		for (paf = victim->affected; paf != NULL; paf = paf->next)
			ptc(ch, "{bSpell: '%s' modifies %s by %d for %d hours with bits %s, level %d, evolve %d.\n",
			    skill_table[(int) paf->type].name, affect_loc_name(paf->location),
			    paf->modifier, paf->duration, affect_bit_name(paf->bitvector),
			    paf->level, paf->evolution);
	}

	diff = victim->level - ch->level;

	if (diff <= -10)   strcpy(buf, "You can kill $N naked and weaponless.");
	else if (diff <=  -5)   strcpy(buf, "$N is no match for you.");
	else if (diff <=  -2)   strcpy(buf, "$N looks like an easy kill.");
	else if (diff <=   1)   strcpy(buf, "The perfect match!");
	else if (diff <=   4)   strcpy(buf, "$N says 'Do you feel lucky, punk?'.");
	else if (diff <=   9)   strcpy(buf, "$N laughs at you mercilessly..");
	else if (diff <=  30)   strcpy(buf, "Death will thank you for your gift.");
	else                    strcpy(buf, "$N could slay you in one blow.");

	set_color(ch, WHITE, BOLD);
	act(buf, ch, NULL, victim, TO_CHAR);
	set_color(ch, WHITE, NOBOLD);

	if (victim->max_hit > 0)
		percent = (100 * victim->hit) / victim->max_hit;
	else
		percent = -1;

	if (percent >= 100) strcpy(buf, "$N is in excellent condition.");
	else if (percent >=  90) strcpy(buf, "$N has a few scratches.");
	else if (percent >=  75) strcpy(buf, "$N has some small wounds and bruises.");
	else if (percent >=  50) strcpy(buf, "$N has quite a few wounds.");
	else if (percent >=  30) strcpy(buf, "$N has some big nasty wounds and scratches.");
	else if (percent >=  15) strcpy(buf, "$N looks pretty hurt.");
	else if (percent >=   1) strcpy(buf, "$N is in awful condition.");
	else if (percent >=   0) strcpy(buf, "$N will soon be toast!!!");
	else                     strcpy(buf, "$N is in need of ***SERIOUS*** medical attention!!!");

	act(buf, ch, NULL, victim, TO_CHAR);
	set_color(ch, WHITE, NOBOLD);
}

void set_title(CHAR_DATA *ch, const char *title)
{
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC(ch)) {
		bug("Set_title: NPC.", 0);
		return;
	}

	if (title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?') {
		buf[0] = ' ';
		strcpy(buf + 1, title);
	}
	else
		strcpy(buf, title);

	free_string(ch->pcdata->title);
	ch->pcdata->title = str_dup(buf);
	return;
}

void do_title(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (argument[0] == '\0') {
		stc("Change your title to what?\n", ch);
		return;
	}

	if (!str_cmp(argument, "none")) {
		set_title(ch, "");
		stc("Title removed.\n", ch);
		return;
	}

	char buf[150];
	buf[0] = '\0';
	strncat(buf, argument, 150);

	if (color_strlen(buf) > 45) {
		stc("Titles can't be longer than 45 printed characters.\n", ch);
		return;
	}

	set_title(ch, buf);
	ptc(ch, "Title changed to: %s\n", ch->pcdata->title);
}

/*
 * Immname by Demonfire - Modified by Lotus
 * Added 11.25.1996
 */

void do_immname(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	char buf[MIL];
	sprintf(buf, "immortal immname %s", argument);
	do_config(ch, buf);
/*
	char block[MAX_STRING_LENGTH];
	char test[MAX_STRING_LENGTH];

	if (argument[0] == '\0') {
		stc("Change your immname to what?\n", ch);
		return;
	}

	if (!str_prefix1(argument, "none")) {
		stc("Your immname has been removed.\n", ch);
		free_string(ch->pcdata->immname);
		ch->pcdata->immname = str_dup("");
		return;
	}

	if (color_strlen(argument) > TITLEBLOCK) {
		stc("Too Long: Limited to 13 printed characters\n", ch);
		return;
	}

	free_string(ch->pcdata->immname);
	block[0]  = '\0';
	strcat(block, "{W[{x");
	strcat(block, set_colorname(argument, TITLEBLOCK));
	strcat(block, "{W]{x");
	ch->pcdata->immname = str_dup(block);
	sprintf(test, "Your new immname is %s.\n",
	        ch->pcdata->immname);
	stc(test, ch);
	return;
*/
}

void do_description(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];

	if (argument[0] != '\0') {
		buf[0] = '\0';

		if (argument[0] == '-') {
			int len;
			bool found = FALSE;

			if (ch->description == NULL || ch->description[0] == '\0') {
				stc("No lines left to remove.\n", ch);
				return;
			}

			strcpy(buf, ch->description);

			for (len = strlen(buf); len > 0; len--) {
				if (buf[len] == '\r') {
					if (!found) { /* back it up */
						if (len > 0)
							len--;

						found = TRUE;
					}
					else { /* found the second one */
						buf[len + 1] = '\0';
						free_string(ch->description);
						ch->description = str_dup(buf);
						set_color(ch, PURPLE, NOBOLD);
						stc("Your description is:\n", ch);
						stc(ch->description ? ch->description :
						    "(None).\n", ch);
						set_color(ch, WHITE, NOBOLD);
						return;
					}
				}
			}

			buf[0] = '\0';
			free_string(ch->description);
			ch->description = str_dup(buf);
			stc("Description cleared.\n", ch);
			return;
		}

		if (argument[0] == '+') {
			if (ch->description != NULL)
				strcat(buf, ch->description);

			argument++;

			while (isspace(*argument))
				argument++;
		}

		if (strlen(buf) + strlen(argument) >= MAX_STRING_LENGTH - 2) {
			stc("Description too long.\n", ch);
			return;
		}

		strcat(buf, argument);
		strcat(buf, "\n");
		free_string(ch->description);
		ch->description = str_dup(buf);
	}

	set_color(ch, PURPLE, NOBOLD);
	stc("Your description is:\n", ch);
	stc(ch->description ? ch->description : "(None).\n", ch);
	set_color(ch, WHITE, NOBOLD);
	return;
}

/* Duplicate of Description for Finger Info */
void do_fingerinfo(CHAR_DATA *ch, const char *argument)
{
	char buf[MSL];

	if (IS_NPC(ch))
		return;

	if (argument[0] != '\0') {
		buf[0] = '\0';

		if (!str_cmp(argument, "clear")) {
			free_string(ch->pcdata->fingerinfo);
			ch->pcdata->fingerinfo = str_dup("");
			stc("Fingerinfo cleared.\n", ch);
			return;
		}
		else if (argument[0] == '-') {
			int len;
			bool found = FALSE;

			if (ch->pcdata->fingerinfo == NULL
			    || ch->pcdata->fingerinfo[0] == '\0') {
				stc("No lines left to remove.\n", ch);
				return;
			}

			strcpy(buf, ch->pcdata->fingerinfo);

			for (len = strlen(buf); len > 0; len--) {
				if (buf[len] == '\r') {
					if (!found) { /* back it up */
						if (len > 0)
							len--;

						found = TRUE;
					}
					else { /* found the second one */
						buf[len + 1] = '\0';
						free_string(ch->pcdata->fingerinfo);
						ch->pcdata->fingerinfo = str_dup(buf);
						set_color(ch, CYAN, NOBOLD);
						ptc(ch, "Your finger info is:\n"
						    "%s",
						    ch->pcdata->fingerinfo ?
						    ch->pcdata->fingerinfo :
						    "(None).\n");
						set_color(ch, WHITE, NOBOLD);
						return;
					}
				}
			}

			buf[0] = '\0';
			free_string(ch->pcdata->fingerinfo);
			ch->pcdata->fingerinfo = str_dup(buf);
			stc("Finger Info cleared.\n", ch);
		}
		else if (argument[0] == '+') {
			if (ch->pcdata->fingerinfo != NULL)
				strcat(buf, ch->pcdata->fingerinfo);

			argument++;

			while (isspace(*argument))
				argument++;
		}
		else {
			stc("Syntax:\n"
			    "  fingerinfo           (shows your fingerinfo)\n"
			    "  fingerinfo -         (removes a line of text)\n"
			    "  fingerinfo + <text>  (adds a line of text)\n"
			    "  fingerinfo clear     (clears your fingerinfo)\n", ch);
			return;
		}

		if (strlen(buf) + strlen(argument) >= MAX_STRING_LENGTH - 2) {
			stc("Finger info too long.\n", ch);
			return;
		}

		strcat(buf, argument);
		strcat(buf, "\n");
		free_string(ch->pcdata->fingerinfo);
		ch->pcdata->fingerinfo = str_dup(buf);
	}

	set_color(ch, CYAN, NOBOLD);
	ptc(ch, "Your finger info is:\n"
	    "%s",
	    ch->pcdata->fingerinfo ?
	    ch->pcdata->fingerinfo :
	    "(None).\n");
	set_color(ch, WHITE, NOBOLD);
}

void do_report(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf, *paf_last = NULL;
	one_argument(argument, arg);
	sprintf(buf,
	        "You say 'I have %d/%d hp %d/%d mana %d/%d st %d xp.'\n",
	        ch->hit,  ch->max_hit,
	        ch->mana, ch->max_mana,
	        ch->stam, ch->max_stam,
	        ch->exp);
	stc(buf, ch);
	sprintf(buf, "$n says 'I have %d/%d hp %d/%d mana %d/%d st %d xp.'",
	        ch->hit,  ch->max_hit,
	        ch->mana, ch->max_mana,
	        ch->stam, ch->max_stam,
	        ch->exp);
	act(buf, ch, NULL, NULL, TO_ROOM);

	if (!strcmp(arg, "all")) {
		if (ch->affected != NULL) {
			stc("You say 'I am affected by the following spells:'\n", ch);
			act("$n says 'I am affected by the following spells:'", ch, NULL, NULL, TO_ROOM);

			for (paf = ch->affected; paf != NULL; paf = paf->next) {
				if (paf_last != NULL && paf->type == paf_last->type)
					continue;
				else {
					sprintf(buf, "You say 'Spell: %-15s'", skill_table[paf->type].name);
					sprintf(buf2, "$n says 'Spell: %-15s'", skill_table[paf->type].name);
				}

				stc(buf, ch);
				act(buf2, ch, NULL, NULL, TO_ROOM);
				stc("\n", ch);
				paf_last = paf;
			}
		}
		else {
			stc("You say 'I am not affected by any spells.'\n", ch);
			act("$n says 'I am not affected by any spells.'", ch, NULL, NULL, TO_ROOM);
		}
	}

	return;
}

/* PRACTICE list of skills and spells, sorted by group -- Elrac */
void prac_by_group(CHAR_DATA *ch, const char *argument)
{
	int gt;
	const struct group_type *gp;
	int js;
	bool group_first;
	int sn;
	char buf[50];
	char line[160];
	int line_cols = 0;  /* number of filled data columns (19 char each) */
	BUFFER *output;
	output = new_buf();

	for (gt = 0; group_table[gt].name; gt++) { /* loop thru groups */
		if (!ch->pcdata->group_known[gt])   /* ignore groups the player doesn't have */
			continue;

		gp = &group_table[gt];

		if (gp->rating[ch->class] < 0)      /* ignore things not in class */
			continue;

		if (argument && argument[0] && str_prefix1(argument, gp->name))
			continue;

		group_first = TRUE;

		for (js = 0; js < MAX_IN_GROUP; js++) { /* loop thru skills/spells */
			if (gp->spells[js] == NULL)
				break;

			sn = skill_lookup(gp->spells[js]);

			if (sn == -1)
				continue;

			if (skill_table[sn].skill_level[ch->class] > ch->level)
				continue; /* skill beyond player's level */

			if (ch->pcdata->learned[sn] <= 0)
				continue;

			/* finally, a skill he knows. */
			if (group_first) {  /* first in group get a new header */
				sprintf(buf, "%s:", gp->name);
				/* Header, like all columns, formatted as 25 char + 1 sp */
				sprintf(line, "{G%-25.25s{x ", buf);
				add_buf(output, line);
				add_buf(output, "\n");
				strcpy(line, "");
				line_cols = 0;
				group_first = FALSE;
			}

			if (line_cols >= 3) {
				add_buf(output, line);
				add_buf(output, "\n");
				strcpy(line, "");
				line_cols = 0;
			}

			sprintf(buf, "%3d%% %-20.20s ", ch->pcdata->learned[sn],
			        gp->spells[js]);
			strcat(line, buf);
			line_cols++;
		} /* end for skills */

		if (line_cols > 0) { /* print last incomplete line */
			add_buf(output, line);
			add_buf(output, "\n");
			strcpy(line, "");
			line_cols = 0;
		}
	} /* end for groups */

	sprintf(buf, "You have %d practice sessions left.\n", ch->practice);
	add_buf(output, buf);
	page_to_char(buf_string(output), ch);
	free_buf(output);
} /* end prac_by_group() */

/* PRACTICE list of skills and spells, sorted by percentage and/or name -- Elrac */
void prac_by_key(CHAR_DATA *ch, char *key, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int sn;             /* skill number */
	int slist[MAX_SKILL];
	int nskills = 0;
	int js;
	int ip;
	int adept;
	char buf[50];
	int line_cols = 0;  /* number of filled data columns (19 char each) */
	BUFFER *output;
	output = new_buf();
	argument = one_argument(argument, arg);

	/* loop thru all skills */
	for (sn = 0; sn < MAX_SKILL && skill_table[sn].name; sn++) {
		if (skill_table[sn].remort_class > 0)
			if (!CAN_USE_RSKILL(ch, sn))
				continue;

		if (skill_table[sn].skill_level[ch->class] > ch->level)
			continue; /* skill beyond player's level */

		if (ch->pcdata->learned[sn] <= 0)
			continue; /* player doesn't know skill */

		if (arg[0] != '\0' && str_prefix1(arg, skill_table[sn].name))
			continue;

		/* skill is known. Insertion sort into slist by appropriate key. */
		ip = nskills;

		if (key[0] == '%') {
			while (ip > 0 &&
			       (ch->pcdata->learned[slist[ip - 1]] <
			        ch->pcdata->learned[sn] ||
			        (ch->pcdata->learned[slist[ip - 1]] ==
			         ch->pcdata->learned[sn] &&
			         strcmp(skill_table[slist[ip - 1]].name,
			                skill_table[sn].name) > 0))) {
				slist[ip] = slist[ip - 1];
				ip--;
			}
		}
		else {
			while (ip > 0 && strcmp(skill_table[slist[ip - 1]].name,
			                        skill_table[sn].name) > 0) {
				slist[ip] = slist[ip - 1];
				ip--;
			}
		}

		slist[ip] = sn;
		nskills++;
	}

	/* slist has a sorted list of skills. Output in columns. */
	line_cols = 0;

	for (js = 0; js < nskills; js++) {
		if (skill_table[slist[js]].remort_class > 0) {
			if (skill_table[slist[js]].remort_class == ch->class + 1)
				adept = 65;
			else
				adept = 50;
		}
		else
			adept = class_table[ch->class].skill_adept;

		ptb(output, "%s%3d%% %-20.20s{x ",
		    ch->pcdata->learned[slist[js]] >= adept ? "{g" : "{C",
		    ch->pcdata->learned[slist[js]],
		    skill_table[slist[js]].name);
		line_cols++;

		if (line_cols >= 3) {
			add_buf(output, "\n");
			line_cols = 0;
		}
	}

	if (line_cols > 0)
		add_buf(output, "\n");

	sprintf(buf, "You have %d practice sessions left.\n", ch->practice);
	add_buf(output, buf);
	page_to_char(buf_string(output), ch);
	free_buf(output);
} /* end prac_by_key() */

void do_practice(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_STRING_LENGTH];
	const char *argtail;
	char buf[MAX_STRING_LENGTH];
	int sn;
	CHAR_DATA *mob;
	int adept, increase;

	if (IS_NPC(ch))
		return;

	/*** PRACTICE (the info command) ***/
	argtail = one_argument(argument, arg);

	if (arg[0] == '\0') {
		prac_by_key(ch, "abc", "");
		return;
	}
	else if (!str_prefix1(arg, "groups")) {
		prac_by_group(ch, argtail);
		return;
	}
	else if (!str_prefix1(arg, "abc") ||
	         !str_prefix1(arg, "%")) {
		prac_by_key(ch, arg, argtail);
		return;
	}

	/* none of the above returned, so it must be: */
	/*** PRACTICE <skill> ***/

	if (!IS_AWAKE(ch)) {
		stc("In your dreams, or what?\n", ch);
		return;
	}

	for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room) {
		if (IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE))
			break;
	}

	if (mob == NULL) {
		stc("You can't do that here.\n", ch);
		return;
	}

	if (ch->practice <= 0) {
		stc("You have no practice sessions left.\n", ch);
		return;
	}

	if ((sn = find_spell(ch, argument)) < 0
	    || (!IS_NPC(ch)
	        && (ch->level < skill_table[sn].skill_level[ch->class]
	            ||    ch->pcdata->learned[sn] < 1 /* skill is not known */
	            ||    skill_table[sn].rating[ch->class] == 0))) {
		stc("You can't practice that.\n", ch);
		return;
	}

	if (skill_table[sn].remort_class > 0)
		if (!CAN_USE_RSKILL(ch, sn)) {
			stc("You can't practice that.\n", ch);
			return;
		}

	adept = 0;

	if (skill_table[sn].remort_class > 0) {
		if (skill_table[sn].remort_class == ch->class + 1)
			adept = 65;
		else if (HAS_EXTRACLASS(ch, sn))
			adept = 50;
	}
	else
		adept = IS_NPC(ch) ? 100 : class_table[ch->class].skill_adept;

	if (ch->pcdata->learned[sn] >= adept) {
		sprintf(buf, "You are already learned at %s.\n",
		        skill_table[sn].name);
		stc(buf, ch);
		return;
	}

	ch->practice--;
	increase = (int_app[get_curr_stat(ch, STAT_INT)].learn / skill_table[sn].rating[ch->class]);

	if (increase < 1)
		increase = 1;

	ch->pcdata->learned[sn] += increase;

	if (ch->pcdata->learned[sn] < adept) {
		act("You practice $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR);
		act("$n practices $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM);
	}
	else {
		ch->pcdata->learned[sn] = adept;
		act("You are now learned at $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR);
		act("$n is now learned at $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM);
	}
} /* end do_practice() */

#if 0
void do_old_practice(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	int sn;

	if (IS_NPC(ch))
		return;

	if (argument[0] == '\0') {
		int col;
		col    = 0;

		for (sn = 0; sn < MAX_SKILL; sn++) {
			if (skill_table[sn].name == NULL)
				break;

			if (ch->level < skill_table[sn].skill_level[ch->class]
			    || ch->pcdata->learned[sn] < 1 /* skill is not known */)
				continue;

			sprintf(buf, "%-18s %3d%%  ",
			        skill_table[sn].name, ch->pcdata->learned[sn]);
			stc(buf, ch);

			if (++col % 3 == 0)
				stc("\n", ch);
		}

		if (col % 3 != 0)
			stc("\n", ch);

		sprintf(buf, "You have %d practice sessions left.\n",
		        ch->practice);
		stc(buf, ch);
	}
	else {
		CHAR_DATA *mob;
		int adept;

		if (!IS_AWAKE(ch)) {
			stc("In your dreams, or what?\n", ch);
			return;
		}

		for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room) {
			if (IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE))
				break;
		}

		if (mob == NULL) {
			stc("You can't do that here.\n", ch);
			return;
		}

		if (ch->practice <= 0) {
			stc("You have no practice sessions left.\n", ch);
			return;
		}

		if ((sn = find_spell(ch, argument)) < 0
		    || (!IS_NPC(ch)
		        && (ch->level < skill_table[sn].skill_level[ch->class]
		            ||    ch->pcdata->learned[sn] < 1 /* skill is not known */
		            ||    skill_table[sn].rating[ch->class] == 0))) {
			stc("You can't practice that.\n", ch);
			return;
		}

		adept = IS_NPC(ch) ? 100 : class_table[ch->class].skill_adept;

		if (ch->pcdata->learned[sn] >= adept) {
			sprintf(buf, "You are already learned at %s.\n",
			        skill_table[sn].name);
			stc(buf, ch);
		}
		else {
			ch->practice--;
			ch->pcdata->learned[sn] +=
			        int_app[get_curr_stat(ch, STAT_INT)].learn /
			        skill_table[sn].rating[ch->class];

			if (ch->pcdata->learned[sn] < adept) {
				act("You practice $T.",
				    ch, NULL, skill_table[sn].name, TO_CHAR);
				act("$n practices $T.",
				    ch, NULL, skill_table[sn].name, TO_ROOM);
			}
			else {
				ch->pcdata->learned[sn] = adept;
				act("You are now learned at $T.",
				    ch, NULL, skill_table[sn].name, TO_CHAR);
				act("$n is now learned at $T.",
				    ch, NULL, skill_table[sn].name, TO_ROOM);
			}
		}
	}
}
#endif

/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int wimpy;
	one_argument(argument, arg);

	if (arg[0] == '\0')
		wimpy = ch->max_hit / 5;
	else
		wimpy = atoi(arg);

	if (wimpy < 0) {
		stc("Your courage exceeds your wisdom.\n", ch);
		return;
	}

	if (wimpy > ch->max_hit / 2) {
		stc("CHICKEN!!!!!\n", ch);
		return;
	}

	ch->wimpy   = wimpy;
	sprintf(buf, "Wimpy set to %d hit points.\n", wimpy);
	stc(buf, ch);
	return;
}

void do_password(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char *pArg;
	char *pwdnew;
	char *p;
	char cEnd;

	if (IS_NPC(ch))
		return;

	/*
	 * Can't use one_argument here because it smashes case.
	 * So we just steal all its code.  Bleagh.
	 */
	pArg = arg1;

	while (isspace(*argument))
		argument++;

	cEnd = ' ';

	if (*argument == '\'' || *argument == '"')
		cEnd = *argument++;

	while (*argument != '\0') {
		if (*argument == cEnd) {
			argument++;
			break;
		}

		*pArg++ = *argument++;
	}

	*pArg = '\0';
	pArg = arg2;

	while (isspace(*argument))
		argument++;

	cEnd = ' ';

	if (*argument == '\'' || *argument == '"')
		cEnd = *argument++;

	while (*argument != '\0') {
		if (*argument == cEnd) {
			argument++;
			break;
		}

		*pArg++ = *argument++;
	}

	*pArg = '\0';

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		stc("Syntax: password <old> <new>.\n", ch);
		return;
	}

	/*    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) ) */
	if (strcmp(arg1, ch->pcdata->pwd)) {
		WAIT_STATE(ch, 40);
		stc("Wrong password.  Wait 10 seconds.\n", ch);
		return;
	}

	if (strlen(arg2) < 5) {
		stc(
		        "New password must be at least five characters long.\n", ch);
		return;
	}

	/*
	 * No tilde allowed because of player file format.
	 */
	/*pwdnew = crypt( arg2, ch->name );*/
	pwdnew = arg2;

	for (p = pwdnew; *p != '\0'; p++) {
		if (*p == '~') {
			stc(
			        "New password not acceptable, try again.\n", ch);
			return;
		}
	}

	free_string(ch->pcdata->pwd);
	ch->pcdata->pwd = str_dup(pwdnew);
	save_char_obj(ch);
	stc("Password Changed.\n", ch);
	return;
}
void do_invite(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	bool found = FALSE;

	if (argument[0] == '\0') {
		if (IS_IMMORTAL(ch)) {
			for (d = descriptor_list; d != NULL; d = d->next) {
				if (!IS_PLAYING(d) || !can_see_who(ch, d->character))
					continue;

				victim = (d->original != NULL) ? d->original : d->character;

				if (!can_see_who(ch, victim))
					continue;

				if (victim->inviters != NULL) {
					if (!found) {
						stc("{M[{YName        {M][{YInviting Clan                 {M][{YStatus  {M]\n", ch);
						stc("--------------------------------------------------------{x\n", ch);
					}

					sprintf(buf, "{M[{Y%-12s{M][{Y%-30s{M][{Y%-8s{M]{x\n", victim->name,
					        victim->inviters->clanname,
					        (victim->invitation_accepted == TRUE) ? "accepted" : "invited");
					stc(buf, ch);
					found = TRUE;
				}
			}

			if (!found)
				stc("There are currently no invited players.\n", ch);
		}
		else {
			if (!IS_SET(ch->pcdata->cgroup, (GROUP_LEADER | GROUP_DEPUTY))) {
				stc("Only leaders or deputies of your clan may send out invitations.\n", ch);
				return;
			}
			else
				stc("You must specify a name if you want to invite someone.\n", ch);
		}

		return;
	}

	argument = one_argument(argument, arg1);

	if (!str_cmp(arg1, "terminate")) {
		if (argument[0] == '\0') {
			if ((ch->inviters != NULL)) {
				stc("You have terminated your invitation.\n", ch);
				ch->inviters = NULL;
				ch->invitation_accepted = FALSE;
				return;
			}

			/* if ((ch->invite != 0) && (ch->invite >= INVITED))
			 {
			     stc("Please use invite decline.\n",ch);
			     return;
			 }
			 */
			stc("No one has invited the likes of you!\n", ch);
			return;
		}

		if ((victim = get_char_world(ch, argument, VIS_PLR)) == NULL) {
			stc("The person you want to uninvite is not here.\n", ch);
			return;
		}

		if (victim->inviters != NULL) {
			if (victim->inviters == ch->clan) {
				stc("You have terminated their invitation.\n", ch);
				sprintf(buf, "%s has terminated your invitation.\n", ch->name);
				victim->inviters = NULL;
				victim->invitation_accepted = FALSE;
				stc(buf, victim);
			}
			else
				stc("That player is not invited to your clan!\n", ch);
		}
		else {
			stc("That player is not invited to your clan!\n", ch);
			return;
		}

		stc("That player has not been invited.\n", ch);
		return;
	}

	if (!strcmp(arg1, "accept")) {
		if (ch->inviters == NULL) {
			stc("No one has invited the likes of you!\n", ch);
			return;
		}

		if (ch->invitation_accepted == TRUE) {
			stc("You have already accepted the invitation!\n", ch);
			return;
		}

		if (!deduct_cost(ch, 10000)) { /* 100 gold to accept */
			stc("You must have at least 100 gold to accept an invitation.\n", ch);
			return;
		}

		stc("You have accepted their invitation.\n", ch);
		ch->invitation_accepted = TRUE;

		for (d = descriptor_list; d != NULL; d = d->next) {
			victim = d->original ? d->original : d->character;

			if (IS_PLAYING(d)
			    && d->character != ch
			    && ((ch->inviters == d->character->clan)
			        || IS_IMMORTAL(d->character))
			    && !IS_SET(d->character->comm, COMM_NOCLAN)
			    && !IS_SET(d->character->comm, COMM_QUIET)) {
				new_color(d->character, CSLOT_CHAN_CLAN);

				if (IS_IMMORTAL(d->character)) {
					sprintf(buf, "%s",
					        smash_bracket(ch->inviters->who_name));
					stc(buf, d->character);
				}

				sprintf(buf, "%s has accepted an invitation to enter the clan hall",
				        ch->name);
				act_new("Clan Notice: '$t{x'", ch, buf, d->character, TO_VICT, POS_DEAD, FALSE);
				set_color(d->character, WHITE, NOBOLD);
			}
		}

		return;
	}

	if (!strcmp(arg1, "decline")) {
		if (ch->inviters == NULL) {
			stc("No one has invited the likes of you!\n", ch);
			return;
		}

		if (ch->invitation_accepted == TRUE) {
			stc("You have already accepted the invitation!\n", ch);
			return;
		}

		stc("You have declined their invitation.\n", ch);

		for (d = descriptor_list; d != NULL; d = d->next) {
			victim = d->original ? d->original : d->character;

			if (IS_PLAYING(d)
			    && d->character != ch
			    && ((ch->inviters == d->character->clan)
			        || IS_IMMORTAL(d->character))
			    && !IS_SET(d->character->comm, COMM_NOCLAN)
			    && !IS_SET(d->character->comm, COMM_QUIET)) {
				new_color(d->character, CSLOT_CHAN_CLAN);

				if (IS_IMMORTAL(d->character)) {
					sprintf(buf, "%s",
					        smash_bracket(ch->inviters->who_name));
					stc(buf, d->character);
				}

				sprintf(buf, "%s has declined an invitation to enter the clan hall",
				        ch->name);
				act_new("Clan Notice: '$t{x'", ch, buf, d->character, TO_VICT, POS_DEAD, FALSE);
				set_color(d->character, WHITE, NOBOLD);
			}
		}

		ch->inviters = NULL;
		ch->invitation_accepted = FALSE;
		return;
	}

	if (!IS_SET(ch->pcdata->cgroup, (GROUP_LEADER | GROUP_DEPUTY))) {
		stc("Only leaders or deputies of your clan may send out invitations.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_PLR)) == NULL) {
		stc("The person you want to invite is not here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		stc("You cannot invite mobiles to your clan hall.\n", ch);
		return;
	}

	if (victim->inviters != NULL) {
		if (victim->inviters == ch->clan)
			stc("They already have an invitation to your clan hall.\n", ch);
		else
			stc("They already have an invitation to a different clan hall.\n", ch);

		return;
	}

	if (victim == ch) {
		stc("Feeling lonely?\n", ch);
		return;
	}

	if (victim->clan == ch->clan) {
		stc("They don't need an invitation to enter your clan hall.\n", ch);
		return;
	}

	victim->inviters = ch->clan;
	victim->invitation_accepted = FALSE;
	sprintf(buf, "You have invited %s to enter your clan hall.\n", victim->name);
	stc(buf, ch);
	sprintf(buf, "%s has invited you to enter the clan hall of %s.\n", ch->name,
	        ch->clan->clanname);
	stc(buf, victim);
} /* do_invite */

void do_join(CHAR_DATA *ch, const char *argument)
{
	char arg[MIL];
	CHAR_DATA *victim;
	one_argument(argument, arg);

	if (IS_IMMORTAL(ch)) {
		stc("Use 'guild' instead.\n", ch);
		return;
	}

	if (!ch->clan || !IS_SET(ch->pcdata->cgroup, (GROUP_LEADER | GROUP_DEPUTY))) {
		do_huh(ch);
		return;
	}

	if (arg[0] == '\0') {
		stc("Whom do want to join your clan?\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg, VIS_PLR)) == NULL) {
		stc("They aren't playing.\n", ch);
		return;
	}

	if (victim->level < 15 && !IS_REMORT(victim)) {
		stc("They are too inexperienced to join your clan.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		stc("You cannot join immortals.\n", ch);
		return;
	}

	if (victim->clan) {
		stc("They're already in a clan.\n", ch);
		return;
	}

	if (get_duel(victim)) {
		stc("Wait until they finish their duel.\n", ch);
		return;
	}

	/* remove a leader flag, in case they were a leader of a clan that poofed */
	REM_CGROUP(victim, GROUP_LEADER);
	REM_CGROUP(victim, GROUP_DEPUTY);
	SET_CGROUP(victim, GROUP_CLAN);
	victim->clan = ch->clan;
	victim->questpoints_donated = 0;
	victim->gold_donated = 0;
	save_char_obj(victim);
	ptc(ch, "The character is now a member of %s.\n", ch->clan->clanname);
	ptc(victim, "You are now a member of %s.\n", ch->clan->clanname);
}

void vape_ceq(CHAR_DATA *ch)
{
	CLAN_DATA *clan;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	OBJ_DATA *obj_in_cont;
	OBJ_DATA *obj_in_cont_next;
	clan = ch->clan;

	for (obj = ch->carrying; obj != NULL; obj = obj_next) {
		obj_next = obj->next_content;

		if (obj->contains) {
			for (obj_in_cont = obj->contains; obj_in_cont != NULL; obj_in_cont = obj_in_cont_next) {
				obj_in_cont_next = obj_in_cont->next_content;

				/* clanbag */
				if (obj->pIndexData->vnum >= clan->area_minvnum
				    && obj->pIndexData->vnum <= clan->area_maxvnum) {
					obj_from_obj(obj_in_cont);
					obj_to_char(obj_in_cont, ch);

					if (obj_in_cont->item_type == ITEM_MONEY) {
						ch->silver += obj_in_cont->value[0];
						ch->gold += obj_in_cont->value[1];
						extract_obj(obj_in_cont);
					}
				}

				if (obj_in_cont->pIndexData->vnum >= clan->area_minvnum
				    && obj_in_cont->pIndexData->vnum <= clan->area_maxvnum) {
					ptc(ch,
					    "%s {Yexplodes violently{x, leaving only a cloud of {gsmoke{x.\n",
					    obj->short_descr);
					extract_obj(obj_in_cont);
				}
			}
		}

		if (obj->pIndexData->vnum >= clan->area_minvnum
		    && obj->pIndexData->vnum <= clan->area_maxvnum) {
			ptc(ch,
			    "%s {Yexplodes violently{x, leaving only a cloud of {gsmoke{x.\n",
			    obj->short_descr);
			extract_obj(obj);
		}
	}
}

void do_unjoin(CHAR_DATA *ch, const char *argument)
{
	char arg[MIL];
	CHAR_DATA *victim;
	CLAN_DATA *clan;
	one_argument(argument, arg);

	if (IS_IMMORTAL(ch)) {
		stc("Use 'guild' instead.\n", ch);
		return;
	}

	if (!ch->clan || !HAS_CGROUP(ch, GROUP_LEADER)) {
		do_huh(ch);
		return;
	}

	if (arg[0] == '\0') {
		stc("Whom do want to remove from your clan?\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg, VIS_PLR)) == NULL) {
		stc("They aren't playing.\n", ch);
		return;
	}

	if (victim->clan != ch->clan) {
		stc("They're not in your clan.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		stc("You cannot unjoin immortals.\n", ch);
		return;
	}

	if (get_duel(victim)) {
		stc("Wait until they finish their duel.\n", ch);
		return;
	}

	if (HAS_CGROUP(victim, GROUP_LEADER)) {
		stc("They must be deleadered first.\n", ch);
		return;
	}

	/* Get rid of their clan eq, twice for clanbags (put in a recursive form
	   later, this is stupid) -- Montrey */
	vape_ceq(victim);
	vape_ceq(victim);
	ptc(ch, "The character is no longer a member of: %s\n", ch->clan->clanname);
	ptc(victim, "You are no longer a member of: %s\n", ch->clan->clanname);
	clan = victim->clan;
	victim->clan = NULL;

	/* boot them from the clanhall */
	if (victim->in_room->vnum >= clan->area_minvnum
	    && victim->in_room->vnum <= clan->area_maxvnum) {
		char_from_room(victim);
		char_to_room(victim, get_room_index(ROOM_VNUM_ALTAR));
		do_look(ch, "auto");
		stc("You have been removed from the clanhall.\n", victim);
	}

	/* Remove Leader flag!!!! */
	REM_CGROUP(victim, GROUP_LEADER);
	REM_CGROUP(victim, GROUP_DEPUTY);
	REM_CGROUP(victim, GROUP_CLAN);
	victim->questpoints_donated = 0;
	victim->gold_donated = 0;
	save_char_obj(victim);
}

/* Clan Status by Lotus */
/* Begin modification by Lotus and Slipstream */
void do_rank(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char test[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	if (!IS_SET(ch->pcdata->cgroup, (GROUP_LEADER | GROUP_DEPUTY)) && !IS_IMMORTAL(ch)) {
		stc("Sorry, only a leader can change clan rank.\n", ch);
		return;
	}

	argument = one_argument(argument, arg1);

	if (arg1[0] == '\0') {
		stc("Change whose clan rank?\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg1, VIS_PLR)) == NULL) {
		stc("The player is not logged on.\n", ch);
		return;
	}

	if (victim->clan != ch->clan && !IS_IMMORTAL(ch)) {
		stc("They are not a member of your clan.\n", ch);
		return;
	}

	if (argument[0] == '\0') {
		stc("Change their clan rank to what?\n"
		    "(use 'none' to remove rank)\n", ch);
		return;
	}

	if (!str_prefix1(argument, "none")) {
		stc("Clan rank removed.\n", ch);
		stc("Your clan rank has been removed.\n", victim);
		free_string(victim->pcdata->rank);
		victim->pcdata->rank = str_dup("");
		return;
	}

	if (color_strlen(argument) > 3) {
		stc("String cannot be longer than 3 printed characters.\n", ch);
		return;
	}

	if (strlen(argument) > 25) {
		stc("String cannot be longer than 25 total characters.\n", ch);
		return;
	}

	free_string(victim->pcdata->rank);
	victim->pcdata->rank = str_dup(center_string_in_whitespace(argument, 3));
	sprintf(test, "Your new rank is {W[%s{W]{x.\n",
	        victim->pcdata->rank);
	stc(test, victim);
	stc("Clan rank changed.\n", ch);
	return;
}

/* prefix command: it will put the string typed on each line typed */
void do_prefi(CHAR_DATA *ch, const char *argument)
{
	stc("You cannot abbreviate the prefix command.\n", ch);
	return;
}

void do_prefix(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_INPUT_LENGTH];

	if (argument[0] == '\0') {
		if (ch->prefix[0] == '\0') {
			stc("You have no prefix to clear.\n", ch);
			return;
		}

		stc("Prefix removed.\n", ch);
		free_string(ch->prefix);
		ch->prefix = str_dup("");
		return;
	}

	if (ch->prefix[0] != '\0') {
		sprintf(buf, "Prefix changed to: %s.\n", argument);
		free_string(ch->prefix);
	}
	else
		sprintf(buf, "Prefix set to: %s.\n", argument);

	ch->prefix = str_dup(argument);
} /* end do_prefix() */

void email_file(CHAR_DATA *ch, const char *file, const char *str)
{
	FILE *fp;

	if (IS_NPC(ch) || str[0] == '\0')
		return;

	if ((fp = fopen(file, "a")) == NULL) {
		perror(file);
		stc("Could not open the file!\n", ch);
	}
	else {
		fprintf(fp, "%s", str);
		fclose(fp);
	}

	return;
}

void do_email(CHAR_DATA *ch, const char *argument)
{
	char buf[MIL];

	if (IS_NPC(ch)) {
		do_huh(ch);
		return;
	}

	if (argument[0] == '\0') {
		ptc(ch, "Your current email address is: %s\n", ch->pcdata->email);
		return;
	}

	free_string(ch->pcdata->email);
	strcpy(buf, argument);
	ch->pcdata->email = str_dup(buf);
	ptc(ch, "Your email has been changed to: %s\n", buf);
	sprintf(buf, "\"%s\" <%s>\n", ch->name, ch->pcdata->email);
	email_file(ch, EMAIL_FILE, buf);
	sprintf(buf, "$N has changed their email to '%s'\n", ch->pcdata->email);
	wiznet(buf, ch, NULL, WIZ_MAIL, 0, GET_RANK(ch));
}

/* gameinout: show game entry or exit message -- Elrac */
void gameinout(CHAR_DATA *ch, char *mortal, char *entryexit, char inout)
{
	CHAR_DATA *victim;
	char *msgptr;
	char buf[MAX_INPUT_LENGTH];
	char *p;

	if (IS_NPC(ch)) {
		stc("Mobs don't have entry/exit messages!\n", ch);
		return;
	}

	if (mortal == NULL)
		victim = ch;
	else {
		victim = get_player_world(ch, mortal, VIS_PLR);

		if (victim == NULL) {
			act("No player named '$t' in the game!", ch, mortal, NULL, TO_CHAR);
			return;
		}
	}

	if (inout == 'I')
		msgptr = victim->pcdata->gamein;
	else
		msgptr = victim->pcdata->gameout;

	if (msgptr == NULL || *msgptr == '\0') {
		if (victim == ch)
			act("You don't have a game $t message!",
			    ch, entryexit, NULL, TO_CHAR);
		else
			act("$N does not have a game $t message!",
			    ch, entryexit, victim, TO_CHAR);

		return;
	}

	if (victim == ch)
		act(msgptr, ch, NULL, NULL, TO_CHAR);
	else {
		/* convert $n to $N for 3rd person form of game msg */
		strcpy(buf, msgptr);

		for (p = buf; *p; p++)
			if (*p == '$')
				switch (*(p + 1)) {
				case 'n': p[1] = 'N'; break;

				case 'e': p[1] = 'E'; break;

				case 'm': p[1] = 'M'; break;

				case 's': p[1] = 'S'; break;
				}

		act(buf, ch, NULL, victim, TO_CHAR);
	}
} /* end gameinout() */

void do_gamein(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	one_argument(argument, arg);

	if (!IS_IMMORTAL(ch) || arg[0] == '\0')
		gameinout(ch, NULL, "entry", 'I');
	else
		gameinout(ch, arg, "entry", 'I');
} /* end do_gamein() */

void do_gameout(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	one_argument(argument, arg);

	if (!IS_IMMORTAL(ch) || arg[0] == '\0')
		gameinout(ch, NULL, "exit", 'O');
	else
		gameinout(ch, arg, "exit", 'O');
} /* end do_gameout() */

/* Show contents of the pit, selected by level -- Elrac */
void do_pit(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *pit;
	OBJ_DATA sel_pit; /* a real live container-type object! */
	OBJ_DATA *obj, *next_obj;
	char arg[MAX_INPUT_LENGTH];
	const char *keywords;
	int num1 = -1, num2 = -1;
	int level1 = 0, level2 = 0;
	bool flevel = FALSE, fexplevel = FALSE, fname = FALSE;

	if (!check_blind(ch))
		return;

	if ((pit = get_obj_here(ch, "pit")) == NULL) {
		stc("You do not see a donation pit here.\n", ch);
		return;
	}

	if (pit != donation_pit) {
		stc("You seem to be looking at the wrong pit.\n", ch);
		return;
	}

	if (pit->contains == NULL) {
		stc("The pit is empty at the moment.\n", ch);
		return;
	}

	/* scan [ [level [level] ] keyword(s) ] */
	keywords = argument;
	argument = one_argument(argument, arg);

	if (arg[0] == '\0')
		keywords = "";
	else {
		if (is_number(arg)) {
			num1 = UMAX(atoi(arg), -1);
			keywords = argument;
			argument = one_argument(argument, arg);

			if (arg[0] == '\0')
				keywords = "";
			else {
				if (is_number(arg)) {
					num2 = UMAX(atoi(arg), -1);
					keywords = argument;
					one_argument(argument, arg);

					if (arg[0] == '\0')
						keywords = "";
				}
			}
		}
	}

	/* interpret num1 and num2 into a level range */
	fname = (keywords[0] != '\0');

	if (num1 == -1 && !fname) {
		/* no number, no keyword */
		flevel = TRUE;
		level1 = UMIN(ch->level - 7, 2 * ch->level / 3);
		level1 = UMAX(level1, 0);
		level2 = get_holdable_level(ch);
	}
	else if (num1 == -1) {
		/* keyword only */
		flevel = FALSE;
	}
	else if (num2 == -1) {
		/* level 1 but no level 2, maybe keyword */
		fexplevel = TRUE;
		flevel = TRUE;
		level1 = UMAX(num1 - 10, 0);
		level2 = num1 + 10;
	}
	else {
		/* level 1 and level 2, maybe keyword */
		fexplevel = TRUE;
		flevel = TRUE;
		level1 = UMAX(num1, 0);
		level2 = UMAX(num2, level1);
	}

	/* hastily dummy up an empty clone of the pit */
	/* not a bona fide object, it is on local stack and not in obj list */
	memset(&sel_pit, 0, sizeof(sel_pit));
	sel_pit.pIndexData = pit->pIndexData;
	clone_object(pit, &sel_pit);
	sel_pit.in_room = pit->in_room;

	/* select pit items into 'sel_pit' for looking at */
	for (obj = pit->contains; obj != NULL; obj = next_obj) {
		next_obj = obj->next_content;

		if (flevel &&
		    (obj->level < level1 || obj->level > level2))
			continue;

		if (fname && !is_name(keywords, obj->name))
			continue;

		obj_from_obj(obj);
		obj_to_obj(obj, &sel_pit);
	}

	if (sel_pit.contains == NULL) {
		char buf[MAX_INPUT_LENGTH];

		if (flevel && fname)
			sprintf(buf, "You see no '%s' around that level in the pit.\n",
			        keywords);
		else if (flevel) {
			if (fexplevel)
				sprintf(buf, "You see nothing around that level in the pit.\n");
			else
				sprintf(buf, "You see nothing around your level in the pit.\n");
		}
		else
			sprintf(buf, "You see no '%s' in the pit.\n", keywords);

		stc(buf, ch);
	}
	else {
		/* code copied from do_look(), so should be convincing. */
		SET_BIT(ch->act, PLR_LOOKINPIT);
		new_color(ch, CSLOT_MISC_OBJECTS);
		act("In your selection from $p, you see:", ch, pit, NULL, TO_CHAR);
		show_list_to_char(sel_pit.contains, ch, TRUE, TRUE, FALSE);
		set_color(ch, WHITE, NOBOLD);
		REMOVE_BIT(ch->act, PLR_LOOKINPIT);
	}

	/* stuff goes back into pit. rummaging shuffles contents, that's ok */
	for (obj = sel_pit.contains; obj != NULL; obj = next_obj) {
		next_obj = obj->next_content;
		obj_from_obj(obj);
		obj_to_obj(obj, pit);
	}
} /* end do_pit() */

/* Show information of a clan to a member */
void do_claninfo(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_INPUT_LENGTH];

	if (!IS_IMMORTAL(ch)) {
		if (ch->clan == NULL) {
			stc("You're not even in a clan!\n", ch);
			return;
		}

		if (ch->clan->independent) {
			stc("Your clan doesn't support this feature!\n", ch);
			return;
		}

		sprintf(buf, "You are a member of %s, created by %s.\n",
		        ch->clan->clanname, ch->clan->creator);
		stc(buf, ch);
		sprintf(buf, "To send a note to your clan, send a note to '%s'.\n",
		        ch->clan->name);
		stc(buf, ch);
		sprintf(buf, "The clan has %ld questpoints, of which %d came from you.\n",
		        ch->clan->clanqp, ch->questpoints_donated);
		stc(buf, ch);
		sprintf(buf, "The clan has %ld gold coins, of which %ld came from you.\n",
		        ch->clan->gold_balance, ch->gold_donated);
		stc(buf, ch);
	}
	else {
		CLAN_DATA *iterator;
		stc("Clanname            |QP     |Gold\n", ch);
		stc("--------------------+-------+------\n", ch);

		for (iterator = clan_table_head->next; iterator != clan_table_tail; iterator = iterator->next) {
			if (iterator->independent)
				continue;

			sprintf(buf, "%-20s|%-7ld|%ld\n", iterator->name, iterator->clanqp, iterator->gold_balance);
			stc(buf, ch);
		}

		stc("--------------------+-------+------\n", ch);
	}

	return;
}

char *make_bar(char *bgcolor, char *fgcolor, long info, int numbg, bool imm)
{
	char output[MSL], buf[MSL];

	if (imm)
		sprintf(output, "{%s{%s%-10ld{x", bgcolor, fgcolor, info);
	else
		sprintf(output, "{%s{%s          {x", bgcolor, fgcolor);

	sprintf(buf, "{x{%s", fgcolor);
	return str_dup(strins(output, buf, numbg + 4));
}

void do_clanpower(CHAR_DATA *ch, const char *argument)
{
	CLAN_DATA *clan;
	int count = 0, inc, i, x, j = 0, high, low, negmod = 0, total;
	BUFFER *output;
	struct c_list {
		CLAN_DATA *clan;
		int members;
		int tmembers;
		int qp;
		int gold;
		int warpts;
		bool printed;
	};
	struct c_list clist[count_clans()];
	output = new_buf();

	for (clan = clan_table_head->next; clan != clan_table_tail; clan = clan->next) {
		if (clan->independent)
			continue;

		clist[count].clan       = clan;
		clist[count].tmembers   = count_clan_members(clan, 0);
		clist[count].qp         = clan->clanqp;
		clist[count].gold       = clan->gold_balance;
		clist[count].warpts     = clan->warcpmod;
		clist[count].printed    = FALSE;
		count++;
	}

	/* presorting, make each int a number between 1 and... eh, 10 */
	for (i = 1, high = clist[0].tmembers; i < count; i++)
		high = UMAX(clist[i].tmembers, high);

	for (i = 0; i < count; i++)
		clist[i].members = high ? URANGE(1, ((9 * clist[i].tmembers) / high) + 1, 10) : 1;

	for (i = 1, high = clist[0].qp; i < count; i++)
		high = UMAX(clist[i].qp, high);

	for (i = 0; i < count; i++)
		clist[i].qp = high ? URANGE(1, ((9 * clist[i].qp) / high) + 1, 10) : 1;

	for (i = 0; i < count; i++)
		clist[i].gold = URANGE(1, ((9 * clist[i].gold) / 200000) + 1, 10);

	/* war points can be negative! */
	for (i = 1, high = clist[0].warpts, low = clist[0].warpts; i < count; i++) {
		high = UMAX(clist[i].warpts, high);
		low = UMIN(clist[i].warpts, low);
	}

	if (low < 0)
		negmod = 0 - low;

	high += negmod;
	low += negmod;
	inc = high - low;

	for (i = 0; i < count; i++)
		clist[i].warpts = inc ? URANGE(1, ((9 * (clist[i].warpts + negmod - low)) / inc) + 1, 10) : high ? 10 : 1;

	add_buf(output, "\n                            {WThe {BClans{W of {BL{Ce{gg{Wa{Cc{By{W:\n\n");
	ptb(output, "%s{CSize:      {GFame:      {YWealth:    {PMight:{x\n",
	    IS_IMMORTAL(ch) ? "                                       " :
	    "                                  ");

	for (i = 0; i < count; i++) {
		for (x = 0, high = 0; x < count; x++)
			if (!clist[x].printed) {
				total = calc_cp(clist[x].clan, TRUE);

				if (total >= high) {
					j = x;
					high = total;
				}
			}

		if (IS_IMMORTAL(ch))
			ptb(output, "{G(%2d) ", high);

		clist[j].printed = TRUE;
		ptb(output, "{W%d. %-30s{x%s %s %s %s\n",
		    i + 1,
		    clist[j].clan->clanname,
		    make_bar("t", "C", clist[j].tmembers, clist[j].members, IS_IMMORTAL(ch)),
		    make_bar("h", "G", clist[j].clan->clanqp, clist[j].qp, IS_IMMORTAL(ch)),
		    make_bar("y", "Y", clist[j].clan->gold_balance, clist[j].gold, IS_IMMORTAL(ch)),
		    make_bar("r", "P", clist[j].clan->warcpmod, clist[j].warpts, IS_IMMORTAL(ch)));
	}

	add_buf(output, "\n");
	page_to_char(buf_string(output), ch);
	free_buf(output);
}

void print_old_affects(CHAR_DATA *ch)
{
	AFFECT_DATA *paf, *paf_last = NULL;
	char buf[MAX_STRING_LENGTH];
	const char *buf4;
	char buf3[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	bool found = FALSE;
	long cheat = 0;
	long filter;
	long printme;
	BUFFER *buffer;
	OBJ_DATA *obj;
	int iWear;
	int raff, i;
	buffer = new_buf();
	cheat = ch->affected_by;

	if (!IS_NPC(ch) && ch->pcdata->pktimer)
		ptb(buffer, "{PYour PK timer is at %d seconds and counting!{x\n\n", ch->pcdata->pktimer * 3);

	if (ch->affected != NULL) {
		add_buf(buffer, "{bYou are affected by the following spells:{x\n");

		for (paf = ch->affected; paf != NULL; paf = paf->next) {
			if (paf_last != NULL && paf->type == paf_last->type) {
				if (ch->level >= 20)
					sprintf(buf, "                          ");
				else
					continue;
			}
			else
				sprintf(buf, "{bSpell: %-19s{x", skill_table[paf->type].name);

			add_buf(buffer, buf);

			if (IS_SET(cheat, paf->bitvector))
				cheat -= paf->bitvector;

			if (ch->level >= 20) {
				sprintf(buf,
				        "{b: modifies %s by %d{x ",
				        affect_loc_name(paf->location),
				        paf->modifier);
				add_buf(buffer, buf);

				if (paf->duration == -1)
					sprintf(buf, "{bpermanently{x");
				else
					sprintf(buf, "{bfor %d hours{x", paf->duration + 1);

				add_buf(buffer, buf);
			}

			add_buf(buffer, "\n");
			paf_last = paf;
		}

		found = TRUE;
		add_buf(buffer, "\n");
	}

	if (race_table[ch->race].aff != 0 &&
	    IS_AFFECTED(ch, race_table[ch->race].aff)) {
		add_buf(buffer, "{bYou are affected by the following racial abilities:{x\n");

		if (IS_SET(cheat, race_table[ch->race].aff))
			cheat -= race_table[ch->race].aff;

		strcpy(buf3, affect_bit_name(race_table[ch->race].aff));
		buf4 = buf3;
		buf4 = one_argument(buf4, buf2);

		while (buf2[0]) {
			sprintf(buf, "{bSpell: %-19s{x", buf2);
			add_buf(buffer, buf);
			add_buf(buffer, "\n");
			buf4 = one_argument(buf4, buf2);
		}

		found = TRUE;
		add_buf(buffer, "\n");
	}

	if (ch->affected_by != 0  && (ch->affected_by != race_table[ch->race].aff)) {
		bool print = FALSE;

		for (iWear = 0; iWear < MAX_WEAR; iWear++) {
			if ((obj = get_eq_char(ch, iWear)) != NULL) {
				for (paf = obj->affected; paf != NULL; paf = paf->next) {
					if (!IS_SET(ch->affected_by, paf->bitvector))
						continue;

					if (paf->where != TO_AFFECTS)
						continue;

					filter = paf->bitvector;
					filter &= ch->affected_by;
					printme = filter;
					filter &= cheat;
					cheat -= filter;

					if (!print) {
						add_buf(buffer, "{bYou are affected by the following equipment spells:{x\n");
						print = TRUE;
					}

					strcpy(buf3, affect_bit_name(printme));
					buf4 = buf3;
					buf4 = one_argument(buf4, buf2);

					while (buf2[0]) {
						sprintf(buf, "{bSpell: %-19s:{x %s", buf2, obj->short_descr);
						add_buf(buffer, buf);
						add_buf(buffer, "\n");
						buf4 = one_argument(buf4, buf2);
					}
				}

				if (!obj->enchanted) {
					for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) {
						if (!IS_SET(ch->affected_by, paf->bitvector))
							continue;

						if (paf->where != TO_AFFECTS)
							continue;

						filter = paf->bitvector;
						filter &= ch->affected_by;
						printme = filter;
						filter &= cheat;
						cheat -= filter;

						if (!print) {
							add_buf(buffer, "{bYou are affected by the following equipment spells:{x\n");
							print = TRUE;
						}

						strcpy(buf3, affect_bit_name(printme));
						buf4 = buf3;
						buf4 = one_argument(buf4, buf2);

						while (buf2[0]) {
							sprintf(buf, "{bSpell: %-19s:{x %s", buf2, obj->short_descr);
							add_buf(buffer, buf);
							add_buf(buffer, "\n");
							buf4 = one_argument(buf4, buf2);
						}
					}
				}
			}
		}

		found = TRUE;

		if (print)
			add_buf(buffer, "\n");
	}

	if (cheat != 0) {
		if (!IS_NPC(ch) && !IS_IMMORTAL(ch)) {
			sprintf(buf, "%s has the immortal affect(s) of %s.  Fixing...",
			        ch->name, affect_bit_name(cheat));
			wiznet(buf, NULL, NULL, WIZ_CHEAT, 0, 0);
			log_string(buf);
		}

		add_buf(buffer, "{bYou are affected by the following immortal abilities:{x\n");
		strcpy(buf3, affect_bit_name(cheat));
		/* get rid of the imm affect, needed for racial changes (removing perms from races) -- Montrey */
		ch->affected_by -= cheat;
		buf4 = buf3;
		buf4 = one_argument(buf4, buf2);

		while (buf2[0]) {
			sprintf(buf, "{bSpell: %-19s{x", buf2);
			add_buf(buffer, buf);
			add_buf(buffer, "\n");
			buf4 = one_argument(buf4, buf2);
		}

		found = TRUE;
		add_buf(buffer, "\n");
	}

	if (!IS_NPC(ch)) {
		if ((ch->pcdata->raffect[0] != 0) && (ch->pcdata->remort_count > 0) && IS_SET(ch->pcdata->plr, PLR_SHOWRAFF)) {
			add_buf(buffer, "{bYou are affected by the following remort affects:{x\n");

			for (raff = 0; raff < ch->pcdata->remort_count / 10 + 1; raff++) {
				for (i = 0; i < MAX_RAFFECTS; i++) {
					if (raffects[i].id == ch->pcdata->raffect[raff]) {
						sprintf(buf, "{b%s{x\n", raffects[i].description);
						add_buf(buffer, buf);
					}
				}
			}

			add_buf(buffer, "\n");
			found = TRUE;
		}
	}

	if (!found)
		stc("{bYou are not affected by any spells.{x\n", ch);
	else
		page_to_char(buf_string(buffer), ch);

	free_buf(buffer);
}

void print_new_affects(CHAR_DATA *ch)
{
	char buf[MSL], buf2[MSL], torch[8], border[4], breakline[MSL], *p;
	AFFECT_DATA *paf, *paf_last = NULL;
	BUFFER *buffer;
	OBJ_DATA *obj;
	long cheat = 0;
	bool found = FALSE;
	/* sort varibles */
	bool un_sorted;
	AFFECT_DATA temp_paf;
	AFFECT_DATA *first_pointer, *second_pointer;
	strcpy(border, get_custom_color_code(ch, CSLOT_SCORE_BORDER));
	sprintf(torch, "%s|#|{x", get_custom_color_code(ch, CSLOT_SCORE_TORCH));
	sprintf(breakline, " %s%s----------------------------------------------------------------%s\n", torch, border, torch);
	buffer = new_buf();
	cheat = ch->affected_by;

	if (ch->affected != NULL) {
		ptb(buffer, " %s {bYou are affected by the following spells:                      %s\n",
		    torch, torch);
		add_buf(buffer, breakline);
		/* Someone suggested putting affects in order of duration.
		   To try this, I am going to bubble sort the linked
		   list of affects before we print them.
		   -- Outsider
		*/
		un_sorted = TRUE;

		while (un_sorted) {
			un_sorted = FALSE;

			// go through the list, looking for unsorted items
			for (paf = ch->affected; paf != NULL; paf = paf->next) {
				// check for end of list
				if (! paf->next)
					break;

				// check if item and next one are in order
				if (paf->duration < paf->next->duration) {
					first_pointer = paf->next;
					second_pointer = paf->next->next;
					memcpy(&temp_paf, paf, sizeof(AFFECT_DATA));
					memcpy(paf, paf->next, sizeof(AFFECT_DATA));
					paf->next = first_pointer;
					memcpy(paf->next, &temp_paf, sizeof(AFFECT_DATA));
					paf->next->next = second_pointer;
					un_sorted = TRUE;
				}
			}
		}

		/* End of bubble sort for affects. */

		for (paf = ch->affected; paf != NULL; paf = paf->next) {
			if (paf_last != NULL && paf->type == paf_last->type) {
				if (ch->level >= 20)
					strcpy(buf, "                   ");
				else
					continue;
			}
			else
				sprintf(buf, "{b%-19s", skill_table[paf->type].name);

			if (ch->level >= 20) {
				sprintf(buf2, "{b%s by %d ",
				        affect_loc_name(paf->location), paf->modifier);

				if (paf->duration == -1)
					strcat(buf2, "permanently");
				else {
					char tbuf[MSL];
					sprintf(tbuf, "for %d hours", paf->duration + 1);
					strcat(buf2, tbuf);
				}
			}
			else
				strcpy(buf2, " ");

			ptb(buffer, " %s %-19s %s| %-42s %s\n", torch, buf, border, buf2, torch);
			paf_last = paf;

			if (IS_SET(cheat, paf->bitvector))
				cheat -= paf->bitvector;
		}

		found = TRUE;
	}

	if (race_table[ch->race].aff != 0
	    && IS_AFFECTED(ch, race_table[ch->race].aff)) {
		if (found)
			add_buf(buffer, breakline);

		ptb(buffer, " %s {bYou are affected by the following racial abilities:            %s\n",
		    torch, torch);
		add_buf(buffer, breakline);

		if (IS_SET(cheat, race_table[ch->race].aff))
			cheat -= race_table[ch->race].aff;

		strcpy(buf, affect_bit_name(race_table[ch->race].aff));
		const char *words = buf;

		while (*words) {
			words = one_argument(words, buf2);
			ptb(buffer, " %s {b%-19s                                            %s\n",
			    torch, buf2, torch);
		}

		found = TRUE;
	}

	if (ch->affected_by != 0  && (ch->affected_by != race_table[ch->race].aff)) {
		char objbuf[MSL];
		bool print = FALSE;
		long filter, printme;
		int iWear, len;

		for (iWear = 0; iWear < MAX_WEAR; iWear++) {
			if ((obj = get_eq_char(ch, iWear)) != NULL) {
				for (paf = obj->affected; paf; paf = paf->next) {
					if (paf->where != TO_AFFECTS
					    || !IS_SET(ch->affected_by, paf->bitvector))
						continue;

					filter = paf->bitvector;
					filter &= ch->affected_by;
					printme = filter;
					filter &= cheat;
					cheat -= filter;

					if (!print) {
						if (found)
							add_buf(buffer, breakline);

						ptb(buffer, " %s {bYou are affected by the following equipment spells:            %s\n",
						    torch, torch);
						add_buf(buffer, breakline);
						print = TRUE;
					}

					strcpy(objbuf, smash_bracket(obj->short_descr));

					for (p = objbuf, len = 0; *p; p++)
						if (++len > 38) {
							*p = '\0';
							break;
						}

					strcpy(buf, affect_bit_name(printme));
					const char *words = buf;

					while (*words) {
						words = one_argument(words, buf2);
						ptb(buffer, " %s {b%-19s %s|{x %-42s %s\n",
						    torch, buf2, border, objbuf, torch);
					}
				}

				if (!obj->enchanted) {
					for (paf = obj->pIndexData->affected; paf; paf = paf->next) {
						if (paf->where != TO_AFFECTS
						    || !IS_SET(ch->affected_by, paf->bitvector))
							continue;

						filter = paf->bitvector;
						filter &= ch->affected_by;
						printme = filter;
						filter &= cheat;
						cheat -= filter;

						if (!print) {
							if (found)
								add_buf(buffer, breakline);

							ptb(buffer, " %s {bYou are affected by the following equipment spells:            %s\n",
							    torch, torch);
							add_buf(buffer, breakline);
							print = TRUE;
						}

						strcpy(objbuf, smash_bracket(obj->short_descr));

						for (p = objbuf, len = 0; *p; p++)
							if (++len > 38) {
								*p = '\0';
								break;
							}

						strcpy(buf, affect_bit_name(printme));
						const char *words = buf;

						while (*words) {
							words = one_argument(words, buf2);
							ptb(buffer, " %s {b%-19s %s|{x %-40s %s\n",
							    torch, buf2, border, objbuf, torch);
						}
					}
				}
			}
		}

		if (print)
			found = TRUE;
	}

	if (cheat != 0 && !IS_NPC(ch)) {
		sprintf(buf, "%s has invalid affect(s) of %s.  Fixing...",
		        ch->name, affect_bit_name(cheat));
		wiznet(buf, NULL, NULL, WIZ_CHEAT, 0, 0);
		log_string(buf);
		ch->affected_by -= cheat;
	}

	if (!IS_NPC(ch)
	    && ch->pcdata->remort_count
	    && ch->pcdata->raffect[0]
	    && IS_SET(ch->pcdata->plr, PLR_SHOWRAFF)) {
		int raff, i;

		if (found)
			add_buf(buffer, breakline);

		ptb(buffer, " %s {bYou are affected by the following remort affects:              %s\n",
		    torch, torch);
		add_buf(buffer, breakline);

		for (raff = 0; raff < ch->pcdata->remort_count / 10 + 1; raff++)
			for (i = 0; i < MAX_RAFFECTS; i++)
				if (raffects[i].id == ch->pcdata->raffect[raff])
					ptb(buffer, " %s {b%-62s %s\n",
					    torch, raffects[i].description, torch);

		found = TRUE;
	}

	if (!found)
		ptb(buffer, " %s {bYou are not affected by any spells.                            %s\n",
		    torch, torch);

	ptb(buffer, " %s%s================================================================%s\n", torch, border, torch);
	page_to_char(buf_string(buffer), ch);
	free_buf(buffer);
}

void score_old(CHAR_DATA *ch)
{
	new_color(ch, CSLOT_OLDSCORE_NAME);
	ptc(ch, "You are %s%s\n", ch->name, IS_NPC(ch) ? "" : ch->pcdata->title);
	ptc(ch, "Level %d, %d years old (%d hours).\n", ch->level, get_age(ch), get_play_hours(ch));

	if (!IS_NPC(ch)) {
		new_color(ch, CSLOT_OLDSCORE_PKILL);
		ptc(ch, "You have killed %d players and have been killed by %d players, rating %d.\n",
		    ch->pcdata->pckills, ch->pcdata->pckilled, ch->pcdata->pkrank);
		new_color(ch, CSLOT_OLDSCORE_AKILL);
		ptc(ch, "You have won %d arena fights and have lost %d arena fights.\n",
		    ch->pcdata->arenakills, ch->pcdata->arenakilled);

		if (ch->pcdata->flag_killer > 0 && IS_SET(ch->act, PLR_KILLER))
			ptc(ch,
			    "Your {RKILLER{x flag will remain for %d more ticks.\n",
			    ch->pcdata->flag_killer);

		if (ch->pcdata->flag_thief > 0 && IS_SET(ch->act, PLR_THIEF))
			ptc(ch,
			    "Your {BTHIEF{x flag will remain for %d more ticks.\n",
			    ch->pcdata->flag_thief);
	}

	new_color(ch, CSLOT_OLDSCORE_CLASS);
	ptc(ch, "Race: %s  Sex: %s  Class:  %s\n\n",
	    race_table[ch->race].name,
	    GET_SEX(ch) == SEX_NEUTRAL ? "sexless" : GET_SEX(ch) == SEX_MALE ? "male" : "female",
	    IS_NPC(ch) ? "mobile" : class_table[ch->class].name);
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Str: %-2d(%-2d)", ch->perm_stat[STAT_STR], get_curr_stat(ch, STAT_STR));
	new_color(ch, CSLOT_OLDSCORE_ARMOR);
	ptc(ch, "     AC Pierce: %-5d", GET_AC(ch, AC_PIERCE));
	new_color(ch, CSLOT_OLDSCORE_POINTS);
	ptc(ch, "    Hit Points: %-5d/%-5d\n", ch->hit, ch->max_hit);
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Int: %-2d(%-2d)", ch->perm_stat[STAT_INT], get_curr_stat(ch, STAT_INT));
	new_color(ch, CSLOT_OLDSCORE_ARMOR);
	ptc(ch, "     AC Bash  : %-5d", GET_AC(ch, AC_BASH));
	new_color(ch, CSLOT_OLDSCORE_POINTS);
	ptc(ch, "    Mana      : %-5d/%-5d\n", ch->mana, ch->max_mana);
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Wis: %-2d(%-2d)", ch->perm_stat[STAT_WIS], get_curr_stat(ch, STAT_WIS));
	new_color(ch, CSLOT_OLDSCORE_ARMOR);
	ptc(ch, "     AC Slash : %-5d", GET_AC(ch, AC_SLASH));
	new_color(ch, CSLOT_OLDSCORE_POINTS);
	ptc(ch, "    Stamina   : %-5d/%-5d\n", ch->stam, ch->max_stam);
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Dex: %-2d(%-2d)", ch->perm_stat[STAT_DEX], get_curr_stat(ch, STAT_DEX));
	new_color(ch, CSLOT_OLDSCORE_ARMOR);
	ptc(ch, "     AC Magic : %-5d", GET_AC(ch, AC_EXOTIC));
	new_color(ch, CSLOT_OLDSCORE_WEIGHT);
	ptc(ch, "    Items     : %-5d/%-5d\n", get_carry_number(ch), can_carry_n(ch));
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Con: %-2d(%-2d)", ch->perm_stat[STAT_CON], get_curr_stat(ch, STAT_CON));
	new_color(ch, CSLOT_OLDSCORE_GAIN);
	ptc(ch, "     Practices: %-4d", ch->practice);
	new_color(ch, CSLOT_OLDSCORE_WEIGHT);
	ptc(ch, "     Weight    : %-5d/%-5d\n", get_carry_weight(ch) / 10, can_carry_w(ch) / 10);
	new_color(ch, CSLOT_OLDSCORE_STAT);
	ptc(ch, "Chr: %-2d(%-2d)", ch->perm_stat[STAT_CHR], get_curr_stat(ch, STAT_CHR));
	new_color(ch, CSLOT_OLDSCORE_GAIN);
	ptc(ch, "     Trains   : %-4d", ch->train);
	new_color(ch, CSLOT_OLDSCORE_DICEROLL);
	ptc(ch, "     Hit Roll  : %-5d\n", GET_HITROLL(ch));
	new_color(ch, CSLOT_OLDSCORE_MONEY);
	ptc(ch, "                Gold     : %-8ld", ch->gold);
	new_color(ch, CSLOT_OLDSCORE_DICEROLL);
	ptc(ch, " Dam Roll  : %-5d\n", GET_DAMROLL(ch));

	if (!IS_NPC(ch)) {
		new_color(ch, CSLOT_OLDSCORE_QP);
		ptc(ch, "QP :  %4d", ch->questpoints);
	}
	else
		stc("          ", ch);

	new_color(ch, CSLOT_OLDSCORE_MONEY);
	ptc(ch, "      Silver   : %-8ld", ch->silver);
	new_color(ch, CSLOT_OLDSCORE_ALIGN);
	ptc(ch, " Alignment : %-5d\n", ch->alignment);

	if (!IS_NPC(ch)) {
		new_color(ch, CSLOT_OLDSCORE_SP);
		ptc(ch, "SP :  %4d", ch->pcdata->skillpoints);
	}
	else
		stc("          ", ch);

	new_color(ch, CSLOT_OLDSCORE_ALIGN);
	ptc(ch, "      Saves    : %-6d", ch->saving_throw);
	new_color(ch, CSLOT_OLDSCORE_ALIGN);
	ptc(ch, "   Wimpy     : %-5d\n", ch->wimpy);

	if (!IS_NPC(ch)) {
		new_color(ch, CSLOT_OLDSCORE_RPP);
		ptc(ch, "RPP : %4d", ch->pcdata->rolepoints);
	}
	else
		stc("          ", ch);

	new_color(ch, CSLOT_OLDSCORE_ALIGN);
	ptc(ch, "      Thac0    : %-6d\n",
	    interpolate(ch->level, class_table[ch->class].thac0_00,
	                class_table[ch->class].thac0_32));
	set_color(ch, GREEN, NOBOLD);
	set_color(ch, GREEN, BLINK);

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]   > 10)
		stc("You are drunk.\n", ch);

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] ==  0)
		stc("You are thirsty.\n", ch);

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_HUNGER] ==  0)
		stc("You are hungry.\n", ch);

	set_color(ch, WHITE, NOBOLD);

	switch (get_position(ch)) {
	case POS_DEAD:          stc("You are DEAD!!\n", ch);                  break;

	case POS_MORTAL:        stc("You are mortally wounded.\n", ch);       break;

	case POS_INCAP:         stc("You are incapacitated.\n", ch);          break;

	case POS_STUNNED:       stc("You are stunned.\n", ch);                break;

	case POS_SLEEPING:      stc("You are sleeping.\n", ch);               break;

	case POS_RESTING:       stc("You are resting.\n", ch);                break;

	case POS_STANDING:      stc("You are standing.\n", ch);               break;

	case POS_FIGHTING:      stc("You are fighting.\n", ch);               break;

	case POS_SITTING:       stc("You are sitting.\n", ch);                break;
	}

	if (!IS_NPC(ch) && ch->level < LEVEL_HERO)
		ptc(ch, "You need %ld exp to level.  ",
		    ((ch->level + 1) * exp_per_level(ch, ch->pcdata->points) - ch->exp));

	ptc(ch, "You have scored %d total exp.\n", ch->exp);

	if (ch->invis_level)
		ptc(ch, "  Invisible: level %d", ch->invis_level);

	if (ch->lurk_level)
		ptc(ch, "  Lurk: level %d", ch->lurk_level);

	if (ch->invis_level || ch->lurk_level)
		stc("\n", ch);

	if (IS_SET(ch->comm, COMM_SHOW_AFFECTS))
		print_old_affects(ch);

	if (!IS_NPC(ch) && ch->pcdata->pktimer)
		ptc(ch, "{PYour PK timer is at %d seconds and counting!{x\n\n", ch->pcdata->pktimer * 3);
}

void score_new(CHAR_DATA *ch)
{
	char buf[MSL], border[4], torch[4], flame[4];
	strcpy(border, get_custom_color_code(ch, CSLOT_SCORE_BORDER));
	strcpy(torch, get_custom_color_code(ch, CSLOT_SCORE_TORCH));
	strcpy(flame, get_custom_color_code(ch, CSLOT_SCORE_FLAME));
//	line  1:   ,                                                                  ,
	ptc(ch, "  %s,                                                                  ,{x\n", flame);
//	line  2:  '`,                Kazander, Lover of Freyja's Soul                '`,
	/* center the name and title */
	new_color(ch, CSLOT_SCORE_TITLE);
	sprintf(buf, "%s%s{x%s", get_custom_color_code(ch, CSLOT_SCORE_NAME), ch->name, ch->pcdata->title);
	ptc(ch, " %s'`,{x %s %s'`,{x\n", flame, strcenter(buf, 62), flame);
//	line  3:  `,                                                                 `,
	ptc(ch, " %s`,                                                                 `,{x\n", flame);
//	line  4:  .:.                        Male Dragon Mage                        .:.
	sprintf(buf, "%s ", GET_SEX(ch) == SEX_NEUTRAL ? "Sexless" : GET_SEX(ch) == SEX_MALE ? "Male" : "Female");
	strcat(buf, capitalize(race_table[ch->race].name));
	strcat(buf, " ");
	strcat(buf, capitalize(class_table[ch->class].name));
	new_color(ch, CSLOT_SCORE_CLASS);
	ptc(ch, " {Y.:.{x %s {Y.:.{x\n", strcenter(buf, 62));
//	line  5:  )X(           Level 99 (Remort 0)     Age: 17 (130 Hours)          )X(
	ptc(ch, " %s)X({x ", torch);
	new_color(ch, CSLOT_SCORE_LEVEL);
	sprintf(buf, "Level{B:{x %d {B({xRemort %d{B){x     Age{B:{x %d {B({x%d Hours{B){x",
	        ch->level,
	        ch->pcdata->remort_count,
	        get_age(ch),
	        get_play_hours(ch));
	stc(strcenter(buf, 62), ch);
	ptc(ch, " %s)X({x\n", torch);
//	line  6: [%%%]==============================================================[%%%]
	ptc(ch, "%s", torch);
	stc("[%%%]", ch);
	ptc(ch, "%s==============================================================%s", border, torch);
	stc("[%%%]{x\n", ch);
//	line  7:  |#| Strength     25/25 |      Hp 30000/30000  |     Gold     17001 |#|
	new_color(ch, CSLOT_SCORE_STAT);
	ptc(ch, " %s|#|{x     Strength %2d/", torch, get_curr_stat(ch, STAT_STR));
	new_color(ch, CSLOT_SCORE_MAXSTAT);
	ptc(ch, "%2d %s|{x", ch->perm_stat[STAT_STR], border);
	new_color(ch, CSLOT_SCORE_HEALTHNAME);
	stc("      Hp ", ch);
	new_color(ch, CSLOT_SCORE_HEALTHNUM);
	ptc(ch, "%5d/%5d  %s|{x", ch->hit, ch->max_hit, border);
	new_color(ch, CSLOT_SCORE_WEALTH);
	ptc(ch, "     Gold %9ld %s|#|{x\n", ch->gold, torch);
//	line  8:  |#| Intelligence 25/25 |    Mana 30000/30000  |   Silver       958 |#|
	new_color(ch, CSLOT_SCORE_STAT);
	ptc(ch, " %s|#|{x Intelligence %2d/", torch, get_curr_stat(ch, STAT_INT));
	new_color(ch, CSLOT_SCORE_MAXSTAT);
	ptc(ch, "%2d %s|{x", ch->perm_stat[STAT_INT], border);
	new_color(ch, CSLOT_SCORE_HEALTHNAME);
	stc("    Mana ", ch);
	new_color(ch, CSLOT_SCORE_HEALTHNUM);
	ptc(ch, "%5d/%5d  %s|{x", ch->mana, ch->max_mana, border);
	new_color(ch, CSLOT_SCORE_WEALTH);
	ptc(ch, "   Silver %9ld %s|#|{x\n", ch->silver, torch);
//	line  9:  |#| Wisdom       25/25 | Stamina 30000/30000  |    Items   22/1000 |#|
	new_color(ch, CSLOT_SCORE_STAT);
	ptc(ch, " %s|#|{x       Wisdom %2d/", torch, get_curr_stat(ch, STAT_WIS));
	new_color(ch, CSLOT_SCORE_MAXSTAT);
	ptc(ch, "%2d %s|{x", ch->perm_stat[STAT_WIS], border);
	new_color(ch, CSLOT_SCORE_HEALTHNAME);
	stc(" Stamina ", ch);
	new_color(ch, CSLOT_SCORE_HEALTHNUM);
	ptc(ch, "%5d/%5d  %s|{x", ch->stam, ch->max_stam, border);
	new_color(ch, CSLOT_SCORE_ENCUMB);
	ptc(ch, "    Items %4d/%4d %s|#|{x\n", get_carry_number(ch), can_carry_n(ch), torch);
//	line 10:  |#| Dexterity    25/25 |                      |   Weight   53/1000 |#|
	new_color(ch, CSLOT_SCORE_STAT);
	ptc(ch, " %s|#|{x    Dexterity %2d/", torch, get_curr_stat(ch, STAT_DEX));
	new_color(ch, CSLOT_SCORE_MAXSTAT);
	ptc(ch, "%2d %s|                      |{x", ch->perm_stat[STAT_DEX], border);
	new_color(ch, CSLOT_SCORE_ENCUMB);
	ptc(ch, "   Weight %4d/%4d %s|#|{x\n",
	    get_carry_weight(ch),
	    IS_IMMORTAL(ch) ? 9999 : can_carry_w(ch),
	    torch);
//	line 11:  |#| Constitution 25/25 |  Armor vs            |                    |#|
	new_color(ch, CSLOT_SCORE_STAT);
	ptc(ch, " %s|#|{x Constitution %2d/", torch, get_curr_stat(ch, STAT_CON));
	new_color(ch, CSLOT_SCORE_MAXSTAT);
	ptc(ch, "%2d %s|{x", ch->perm_stat[STAT_CON], border);
	new_color(ch, CSLOT_SCORE_ARMOR);
	ptc(ch, "  Armor vs            %s|                    %s|#|{x\n", border, torch);
//	line 12:  |#| Charisma     25/25 |    Pierce      -600  |  Hitroll       312 |#|
	new_color(ch, CSLOT_SCORE_STAT);
	ptc(ch, " %s|#|{x     Charisma %2d/", torch, get_curr_stat(ch, STAT_CHR));
	new_color(ch, CSLOT_SCORE_MAXSTAT);
	ptc(ch, "%2d %s|{x", ch->perm_stat[STAT_CHR], border);
	new_color(ch, CSLOT_SCORE_ARMOR);
	ptc(ch, "    Pierce    %6d  %s|{x", GET_AC(ch, AC_PIERCE), border);
	new_color(ch, CSLOT_SCORE_DICENAME);
	stc("  Hitroll     ", ch);
	new_color(ch, CSLOT_SCORE_DICENUM);
	ptc(ch, "%5d %s|#|{x\n", GET_HITROLL(ch), torch);
//	line 13:  |#|                    |      Bash      -603  |  Damroll       334 |#|
	new_color(ch, CSLOT_SCORE_ARMOR);
	ptc(ch, " %s|#|{x                    %s|{x      Bash    %6d  %s|{x",
	    torch, border, GET_AC(ch, AC_BASH), border);
	new_color(ch, CSLOT_SCORE_DICENAME);
	stc("  Damroll     ", ch);
	new_color(ch, CSLOT_SCORE_DICENUM);
	ptc(ch, "%5d %s|#|{x\n", GET_DAMROLL(ch), torch);
//	line 14:  |#| QuestPoints   4012 |     Slash      -596  |    Saves       -23 |#|
	new_color(ch, CSLOT_SCORE_POINTNAME);
	ptc(ch, " %s|#|{x QuestPoints  ", torch);
	new_color(ch, CSLOT_SCORE_POINTNUM);
	ptc(ch, "%5d %s|{x", ch->questpoints, border);
	new_color(ch, CSLOT_SCORE_ARMOR);
	ptc(ch, "     Slash    %6d  %s|{x", GET_AC(ch, AC_SLASH), border);
	new_color(ch, CSLOT_SCORE_DICENAME);
	stc("    Saves     ", ch);
	new_color(ch, CSLOT_SCORE_DICENUM);
	ptc(ch, "%5d %s|#|{x\n", ch->saving_throw, torch);
//	line 15:  |#| SkillPoints      0 |     Magic      -601  |    Wimpy       200 |#|
	new_color(ch, CSLOT_SCORE_POINTNAME);
	ptc(ch, " %s|#|{x SkillPoints  ", torch);
	new_color(ch, CSLOT_SCORE_POINTNUM);
	ptc(ch, "%5d %s|{x", ch->pcdata->skillpoints, border);
	new_color(ch, CSLOT_SCORE_ARMOR);
	ptc(ch, "     Magic    %6d  %s|{x", GET_AC(ch, AC_EXOTIC), border);
	new_color(ch, CSLOT_SCORE_DICENAME);
	stc("    Wimpy     ", ch);
	new_color(ch, CSLOT_SCORE_DICENUM);
	ptc(ch, "%5d %s|#|{x\n", ch->wimpy, torch);
//	line 16:  |#|    RPPoints      0 |                      |                    |#|
	new_color(ch, CSLOT_SCORE_POINTNAME);
	ptc(ch, " %s|#|{x    RPPoints  ", torch);
	new_color(ch, CSLOT_SCORE_POINTNUM);
	ptc(ch, "%5d %s|                      |                    %s|#|{x\n",
	    ch->pcdata->rolepoints, border, torch);
//	line 17:  |#|      Trains      4 |  Alignment    -1000  | Experience         |#|
	new_color(ch, CSLOT_SCORE_POINTNAME);
	ptc(ch, " %s|#|{x      Trains  ", torch);
	new_color(ch, CSLOT_SCORE_POINTNUM);
	ptc(ch, "%5d %s|{x", ch->train, border);
	new_color(ch, CSLOT_SCORE_ALIGN);
	ptc(ch, "  Alignment    %s%5d  %s|{x",
	    ch->alignment > 350 ? "{W" : ch->alignment < -350 ? "{R" : "{c", ch->alignment, border);
	new_color(ch, CSLOT_SCORE_XPNAME);
	ptc(ch, " Experience         %s|#|{x\n", torch);
//	line 18:  |#|   Practices     32 |----------------------|    To Level   1000 |#|
	new_color(ch, CSLOT_SCORE_POINTNAME);
	ptc(ch, " %s|#|{x   Practices  ", torch);
	new_color(ch, CSLOT_SCORE_POINTNUM);
	ptc(ch, "%5d %s|----------------------|{x", ch->practice, border);
	new_color(ch, CSLOT_SCORE_XPNUM);
	ptc(ch, "    To Level %6ld %s|#|{x\n",
	    UMIN((ch->level + 1) * exp_per_level(ch, ch->pcdata->points) - ch->exp, 999999), torch);
//	line 19:  |#|                    |   You are standing   |       Total  42055 |#|
	sprintf(buf, "You are ");

	switch (get_position(ch)) {
	case POS_DEAD:          strcat(buf, "DEAD!!");          break;

	case POS_INCAP:
	case POS_MORTAL:        strcat(buf, "dying");           break;

	case POS_STUNNED:       strcat(buf, "stunned");         break;

	case POS_SLEEPING:      strcat(buf, "sleeping");        break;

	case POS_RESTING:       strcat(buf, "resting");         break;

	case POS_STANDING:      strcat(buf, "standing");        break;

	case POS_FIGHTING:      strcat(buf, "fighting");        break;

	case POS_SITTING:       strcat(buf, "sitting");         break;
	}

	ptc(ch, " %s|#|                    %s|{x", torch, border);
	new_color(ch, CSLOT_SCORE_POSITION);
	ptc(ch, "%s%s|{x", strcenter(buf, 22), border);
	new_color(ch, CSLOT_SCORE_XPNUM);
	ptc(ch, "       Total %6d %s|#|{x\n", UMIN(ch->exp, 999999), torch);
//	line 20:  |#|----------------------------------------------------------------|#|
	ptc(ch, " %s|#|%s----------------------------------------------------------------%s|#|{x\n",
	    torch, border, torch);
//	line 21:  |#| PK: 000 Wins, 000 Losses  Rank: 0  Arena: 000 Wins, 000 Losses |#|
	new_color(ch, CSLOT_SCORE_PKRECORD);
	sprintf(buf, "PK: %d Wins, %d Losses  %sRank: %d{x  Arena: %d Wins, %d Losses",
	        ch->pcdata->pckills, ch->pcdata->pckilled,
	        get_custom_color_code(ch, CSLOT_SCORE_PKRANK), ch->pcdata->pkrank,
	        ch->pcdata->arenakills, ch->pcdata->arenakilled);
	ptc(ch, " %s|#|{x %s %s|#|{x\n", torch, strcenter(buf, 62), torch);
//	line 22:  |#|================================================================|#|
	ptc(ch, " %s|#|%s================================================================%s|#|{x\n",
	    torch, border, torch);

	if (IS_SET(ch->comm, COMM_SHOW_AFFECTS))
		print_new_affects(ch);

//	line 23:  |#|                                                                |#|
	ptc(ch, " %s|#|                                                                |#|{x\n", torch);
//	line 24: [[|]]                                                              [[|]]
	ptc(ch, "%s[[|]]                                                              [[|]]{x\n", torch);
}

void do_affects(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch) || !IS_SET(ch->pcdata->plr, PLR_NEWSCORE)) {
		/* Avoid getting ordered to show affects.
		This was causing a crash. Since pets don't talk, they
		won't show affects anyway. We can just return.
		-- Outsider */
		if (IS_AFFECTED(ch, AFF_CHARM))
			return;

		print_old_affects(ch);
		return;
	}

	new_color(ch, CSLOT_SCORE_FLAME);
	stc("  ,                                                                  ,\n"
	    " '`,                                                                '`,\n"
	    " `,                                                                 `,\n"
	    " {Y.:.                                                                .:.{x\n", ch);
	new_color(ch, CSLOT_SCORE_TORCH);
	stc(" )X(                                                                )X(\n"
	    "[%%%]", ch);
	new_color(ch, CSLOT_SCORE_BORDER);
	stc("==============================================================", ch);
	new_color(ch, CSLOT_SCORE_TORCH);
	stc("[%%%]\n", ch);
	set_color(ch, WHITE, NOBOLD);
	print_new_affects(ch);
	new_color(ch, CSLOT_SCORE_TORCH);
	stc(" |#|                                                                |#|\n"
	    "[[|]]                                                              [[|]]\n", ch);
	set_color(ch, WHITE, NOBOLD);
}

void do_score(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch) || !IS_SET(ch->pcdata->plr, PLR_NEWSCORE))
		score_old(ch);
	else
		score_new(ch);
}

/*
The scry spell lets the caster view the room in which his/her
pet is currently standing in. The room may, however, be
dark.
-- Outsider
*/
void spell_scry(int sn, int level, CHAR_DATA *ch, void *vo, int target, int evolution)
{
	CHAR_DATA *pet;

	/* make sure we have a pet */
	if (! ch->pet) {
		stc("You do not have a pet.\n", ch);
		return;
	}

	pet = ch->pet;

	/* check to see if room is dark */
	if (room_is_dark(pet->in_room)) {
		stc("The room your pet is in is dark.\n", ch);
		return;
	}
	else {  /* we can see */
		stc("You see...\n  ", ch);
		stc(pet->in_room->description, ch);
		show_list_to_char(pet->in_room->contents, ch, FALSE, FALSE, FALSE);
		show_char_to_char(pet->in_room->people, ch);
		return;
	}

	return;
}   /* spell_scry end */

