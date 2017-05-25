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
* IMM general commands and related functions.    *
* Most of these are in the general command       *
* group.                                         *
*************************************************/

#include "merc.h"
#include "recycle.h"
#include "lookup.h"
#include "tables.h"
#include "sql.h"
#include "affect.h"
#include "buffer.h"
#include "Format.hpp"

DECLARE_DO_FUN(do_slookup);
DECLARE_DO_FUN(do_claninfo);


extern bool    swearcheck              args((const char *argument));

extern  ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
extern  AREA_DATA       *area_first;

void do_adjust(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	char       buf  [ MAX_STRING_LENGTH ];
	char       arg1 [ MAX_INPUT_LENGTH ];
	char       arg2 [ MAX_INPUT_LENGTH ];
	int      value;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2)) {
		stc("Syntax: Adjust <char> <Exp>.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == NULL) {
		stc("That Player is not here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		stc("Not on NPC's.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		stc("You can't adjust immortals.\n", ch);
		return;
	}

	value = atoi(arg2);

	if (value == 0) {
		stc("The value must not be equal to 0.\n", ch);
		return;
	}

	gain_exp(victim, value);
	sprintf(buf, "You have adjusted the exp of %s by %d experience points.\n",
	        victim->name, value);
	stc(buf, ch);

	if (value > 0) {
		sprintf(buf, "You have been bonused %d experience points.\n", value);
		stc(buf, victim);
	}
	else {
		sprintf(buf, "You have been penalized %d experience points.\n", value);
		stc(buf, victim);
	}

	return;
}

/* save all players, without lag -- Elrac */
void do_allsave(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *wch;
	stc("Saving all players... ", ch);

	for (d = descriptor_list; d; d = d->next) {
		wch = (d->original ? d->original : d->character);
		save_char_obj(wch);
	}

	stc("done.\n", ch);
} /* end allsave */

char *site_to_ssite(char *site)
{
	static char ssite[MSL];
	char *p;
	bool alpha = FALSE;
	int dotcount = 0;

	/* Parse the site, determine type.  For alphanumeric hosts, we
	   match the last three dot sections, for straight numerics we
	   match the first three. */
	for (p = site; *p; p++) {
		if (*p == '.')
			dotcount++;
		else if (!isdigit(*p))
			alpha = TRUE;
	}

	strcpy(ssite, site);
	p = ssite;

	if (alpha) {
		int skippart = 1;

		/* use all but the first part if it's 4 or less parts, 5 or more, all but the first 2 */
		if (dotcount > 3)
			skippart = 2;

		dotcount = 0;

		while (*p != '\0') {
			if (*p == '.') {
				if (++dotcount == skippart) {
					p++;
					break;
				}
			}

			p++;
		}

		if (*p == '\0')
			return site;
	}
	else {
		dotcount = 0;

		while (*p) {
			if (*p == '.' && ++dotcount == 3) {
				*p = '\0';
				break;
			}

			p++;
		}

		p = ssite;
	}

	return p;
}

void do_alternate(CHAR_DATA *ch, const char *argument)
{
	char arg1[MIL], arg2[MIL], query[MSL], colorsite[MSL], *p, *q;
	BUFFER *output;
	int sorted_count = 0, i;
	struct alts {
		char name[20];
		char site[100];
		bool printed;
	};
	struct alts results_sorted[500];
	argument = one_argument(argument, arg1);
	one_argument(argument, arg2);

	if (arg1[0] == '\0' || (!str_cmp(arg1, "site") && arg2[0] == '\0')) {
		stc("Syntax:\n"
		    "  alternate <{Yplayer name{x>\n"
		    "  alternate site <{Ysite{x>\n"
		    "    {c(ex. *.tstc.edu, or 208.238.145.242){x\n", ch);
		return;
	}

	if (!str_cmp(arg1, "site")) {
		char site[MSL];
		bool prefix = FALSE, suffix = FALSE;
		strcpy(site, arg2);
		p = site;
		/* only get sites in the last year.  we don't delete them,
		   saving for some unforseen circumstance :)  date()
		   minus lastlog is 10000 for the same day one year ago,
		   so we'll get that and just cut off the excess.  we do
		   this instead of simply selecting only log entries up to
		   a year old so that if the person has not logged on in the
		   last year, we still get a result.  -- Montrey */
		strcpy(query, "SELECT name, site, (DATE() - lastlog) FROM sites "
		       "WHERE site LIKE '");

		if (*p == '*') {
			prefix = TRUE;
			p++;
		}

		for (q = p; * (q + 1); q++);

		if (*q == '*') {
			suffix = TRUE;
			*q = '\0';
		}

		sprintf(colorsite, "{Y%s{W", p);

		if (prefix)
			strcat(query, "\%");

		strcat(query, p);

		if (suffix)
			strcat(query, "\%");

		strcat(query, "' ORDER BY name, (DATE() - lastlog)");
		bug(query, 0);

		if (db_query("do_alternate", query) != SQL_OK)
			return;

		while (db_next_row() == SQL_OK) {
			if (sorted_count >= 500) {
				bug("do_alternate: WARNING: maximum sorted structure size reached", 0);
				break;
			}

			/* if we're getting results over a year old, break it off, unless
			   we don't have a single result yet */
			if (sorted_count && db_get_column_int(2) >= 10000)
				break;

			strcpy(results_sorted[sorted_count].name, db_get_column_str(0));
			strcpy(results_sorted[sorted_count].site,
			       strrpc(p, colorsite, db_get_column_str(1)));
			sorted_count++;
		}

		if (sorted_count == 0) {
			stc("Site not found - make sure your specified site is not too limiting.\n", ch);
			return;
		}
	}
	else {
		bool old_char = FALSE;
		struct sites {
			char site[100];
			char ssite[100];
		};
		struct sites sitelist[50];
		struct alts results_to_sort[500];
		int x, to_sort_count = 0, sitecount = 0;

		if (db_queryf("do_alternate",
		                        "SELECT site, ssite, (DATE() - lastlog) FROM sites "
		                        "WHERE name LIKE '%s' ORDER BY (DATE() - lastlog)", arg1) != SQL_OK)
			return;

		while (db_next_row() == SQL_OK) {
			if (sitecount >= 50) {
				bug("do_alternate: WARNING: maximum site structure size reached", 0);
				break;
			}

			/* old char? */
			if (db_get_column_int(2) >= 10000) {
				if (!old_char && sitecount)
					break;
				else
					old_char = TRUE;
			}

			strcpy(sitelist[sitecount].site, db_get_column_str(0));
			strcpy(sitelist[sitecount].ssite, db_get_column_str(1));
			sitecount++;
		}

		if (sitecount == 0) {
			stc("Player not found - make sure to search by exact name.\n", ch);
			return;
		}

		for (i = 0; i < sitecount; i++) {
			/* don't get *any* results more than a year old, unless all
			   of the results previously were from an old character */
			sprintf(query, "SELECT name, site FROM sites WHERE ");

			if (!old_char)
				strcat(query, "lastlog >= DATE('now', '-1 year') AND ");

			strcat(query, "ssite='%s'");

			if (db_queryf("do_alternate", query, db_esc(sitelist[i].ssite)) != SQL_OK)
				return;

			sprintf(colorsite, "{Y%s{W", sitelist[i].ssite);

			while (db_next_row() == SQL_OK) {
				const char *name = db_get_column_str(0);

				if (!str_cmp(name, arg1)) {
					if (sorted_count >= 500) {
						bug("do_alternate: WARNING: maximum sorted structure size reached", 0);
						break;
					}

					strcpy(results_sorted[sorted_count].name, name);
					strcpy(results_sorted[sorted_count].site,
					       strrpc(sitelist[i].ssite, colorsite, db_get_column_str(1)));
					results_sorted[sorted_count].printed = FALSE;
					sorted_count++;
				}
				else {
					if (to_sort_count >= 500) {
						bug("do_alternate: WARNING: maximum unsorted structure size reached", 0);
						break;
					}

					strcpy(results_to_sort[to_sort_count].name, name);
					strcpy(results_to_sort[to_sort_count].site,
					       strrpc(sitelist[i].ssite, colorsite, db_get_column_str(1)));
					results_to_sort[to_sort_count].printed = FALSE;
					to_sort_count++;
				}
			}
		}

		for (x = 'A'; x <= 'Z'; x++) {
			for (; ;) {
				int curname = -1;
				bool lettermatch = FALSE;

				for (i = 0; i < to_sort_count; i++) {
					if (results_to_sort[i].printed
					    || results_to_sort[i].name[0] != x)
						continue;

					if (curname == -1)
						curname = i;
					else if (str_cmp(results_to_sort[curname].name, results_to_sort[i].name))
						continue;

					strcpy(results_sorted[sorted_count].name, results_to_sort[i].name);
					strcpy(results_sorted[sorted_count].site, results_to_sort[i].site);
					sorted_count++;
					results_to_sort[i].printed = TRUE;
					lettermatch = TRUE;
				}

				if (!lettermatch)
					break;
			}
		}
	}

	output = new_buf();
	add_buf(output, "{GMatching characters/sites:{x\n");
	add_buf(output, "{G=================================================================={x\n");

	char fstr[25];
	strcpy(fstr, "{G[{W%12s%s%54s{G]{x\n");
	ptb(output, fstr, results_sorted[0].name, " {G-{W", results_sorted[0].site);

	for (i = 1; i < sorted_count; i++)
		ptb(output, fstr,
		    str_cmp(results_sorted[i].name, results_sorted[i - 1].name) ? results_sorted[i].name : "",
		    str_cmp(results_sorted[i].name, results_sorted[i - 1].name) ? " {G-{W" : "  ",
		    results_sorted[i].site);

	add_buf(output, "{G=================================================================={x\n");
	page_to_char(buf_string(output), ch);
	free_buf(output);
}

void do_at(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	ROOM_INDEX_DATA *original;
	OBJ_DATA *on;
	CHAR_DATA *wch;
	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0') {
		stc("At where and what?\n", ch);
		return;
	}

	if ((location = find_location(ch, arg)) == NULL) {
		stc("No such location.\n", ch);
		return;
	}

	if (!is_room_owner(ch, location) && room_is_private(location)
	    &&  !IS_IMP(ch)) {
		stc("That room is private.\n", ch);
		return;
	}

	original = ch->in_room;
	on = ch->on;
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
			ch->on = on;
			break;
		}
	}

	return;
}

/* Check Command borrowed from a web site */
void do_check(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	bool SHOWIMM = FALSE;
	BUFFER *buffer;
	CHAR_DATA *victim;
	argument = one_argument(argument, arg);

	if (!str_cmp(arg, "gods") || !str_cmp(argument, "gods"))
		SHOWIMM = TRUE;

	if (arg[0] == '\0' || !str_prefix1(arg, "gods")) {
		buffer = new_buf();

		for (victim = char_list; victim != NULL; victim = victim->next) {
			if (IS_NPC(victim) || !can_see_char(ch, victim))
				continue;

			if (!SHOWIMM && IS_IMMORTAL(victim))
				continue;

			sprintf(buf, "{W[%12s] Level {C%3d{W connected since %d hours {C(%d total hours){x\n",
			        victim->name, victim->level,
			        ((int)(current_time - victim->logon)) / 3600,
			        (get_play_seconds(victim) + (int)(current_time - victim->logon)) / 3600);
			add_buf(buffer, buf);
		}

		page_to_char(buf_string(buffer), ch);
		free_buf(buffer);
		return;
	}

	if (!str_prefix1(arg, "stats")) {
		buffer = new_buf();

		for (victim = char_list; victim != NULL; victim = victim->next) {
			if (IS_NPC(victim) || !can_see_char(ch, victim))
				continue;

			if (!SHOWIMM && IS_IMMORTAL(victim))
				continue;

			sprintf(buf,
			        "{W[%12s] {P%5d{RHP {P%5d{RMP{c/{G%2d %2d %2d %2d %2d %2d{c/{Y%8ld {bWorth{c/{Y%4d {bQpts{c/{Y%4d {b Spts{x\n",
			        victim->name,
			        GET_MAX_HIT(victim), GET_MAX_MANA(victim), ATTR_BASE(victim, APPLY_STR),
			        ATTR_BASE(victim, APPLY_INT), ATTR_BASE(victim, APPLY_WIS),
			        ATTR_BASE(victim, APPLY_DEX), ATTR_BASE(victim, APPLY_CON),
			        ATTR_BASE(victim, APPLY_CHR),
			        victim->gold + victim->silver / 100,
			        victim->questpoints,
			        !IS_NPC(victim) ? victim->pcdata->skillpoints : 0);
			add_buf(buffer, buf);
		}

		page_to_char(buf_string(buffer), ch);
		free_buf(buffer);
		return;
	}

	if (!str_prefix1(arg, "eq")) {
		buffer = new_buf();

		for (victim = char_list; victim != NULL; victim = victim->next) {
			if (IS_NPC(victim)
			    || !can_see_char(ch, victim))
				continue;

			if (!SHOWIMM && IS_IMMORTAL(victim))
				continue;

			sprintf(buf,
			        "{W[%12s] {b%4d Items (W:%5d){c/{PH:%4d D:%4d{c/{GS:%-4d{c/{CAC:%-5d %-5d %-5d %-5d{x\n",
			        victim->name, get_carry_number(victim), get_carry_weight(victim),
			        GET_ATTR_HITROLL(victim), GET_ATTR_DAMROLL(victim), GET_ATTR_SAVES(victim),
			        GET_AC(victim, AC_PIERCE), GET_AC(victim, AC_BASH),
			        GET_AC(victim, AC_SLASH), GET_AC(victim, AC_EXOTIC));
			add_buf(buffer, buf);
		}

		page_to_char(buf_string(buffer), ch);
		free_buf(buffer);
		return;
	}

	if (!str_prefix1(arg, "absorb")) {
		buffer = new_buf();

		for (victim = char_list; victim != NULL; victim = victim->next) {
			if (IS_NPC(victim) || !can_see_char(ch, victim))
				continue;

			if (!SHOWIMM && IS_IMMORTAL(victim))
				continue;

			sprintf(buf,
			        "{W[%12s] {RABS: {P%s{x\n",
			        victim->name, print_defense_modifiers(victim, TO_ABSORB));
			add_buf(buffer, buf);
		}

		page_to_char(buf_string(buffer), ch);
		free_buf(buffer);
		return;
	}

	if (!str_prefix1(arg, "immune")) {
		buffer = new_buf();

		for (victim = char_list; victim != NULL; victim = victim->next) {
			if (IS_NPC(victim) || !can_see_char(ch, victim))
				continue;

			if (!SHOWIMM && IS_IMMORTAL(victim))
				continue;

			sprintf(buf,
			        "{W[%12s] {RIMM: {P%s{x\n",
			        victim->name,
			        print_defense_modifiers(victim, TO_IMMUNE));
			add_buf(buffer, buf);
		}

		page_to_char(buf_string(buffer), ch);
		free_buf(buffer);
		return;
	}

	if (!str_prefix1(arg, "resistance")) {
		buffer = new_buf();

		for (victim = char_list; victim != NULL; victim = victim->next) {
			if (IS_NPC(victim) || !can_see_char(ch, victim))
				continue;

			if (!SHOWIMM && IS_IMMORTAL(victim))
				continue;

			sprintf(buf,
			        "{W[%12s] {HRES: {G%s{x\n",
			        victim->name,
			        print_defense_modifiers(victim, TO_RESIST));
			add_buf(buffer, buf);
		}

		page_to_char(buf_string(buffer), ch);
		free_buf(buffer);
		return;
	}

	if (!str_prefix1(arg, "vulnerable")) {
		buffer = new_buf();

		for (victim = char_list; victim != NULL; victim = victim->next) {
			if (IS_NPC(victim) || !can_see_char(ch, victim))
				continue;

			if (!SHOWIMM && IS_IMMORTAL(victim))
				continue;

			sprintf(buf,
			        "{W[%12s] {TVUL: {C%s{x\n",
			        victim->name,
			        print_defense_modifiers(victim, TO_VULN));
			add_buf(buffer, buf);
		}

		page_to_char(buf_string(buffer), ch);
		free_buf(buffer);
		return;
	}

	if (!str_prefix1(arg, "snoop")) { /* this part by jerome */
		if (!IS_IMP(ch)) {
			stc("You can't use this check option.\n", ch);
			return;
		}

		buffer = new_buf();

		for (victim = char_list; victim != NULL; victim = victim->next) {
			if (IS_NPC(victim)
			    || victim->desc == NULL
			    || !IS_PLAYING(victim->desc))
				continue;

			if (!SHOWIMM && IS_IMMORTAL(victim))
				continue;

			sprintf(buf, "{W[%12s] is being snooped by {G%s\n", victim->name,
			        (victim->desc->snoop_by != NULL) ? victim->desc->snoop_by->character->name : "nobody");
			add_buf(buffer, buf);
		}

		page_to_char(buf_string(buffer), ch);
		free_buf(buffer);
		return;
	}

	stc("That is not a valid option.\n", ch);
	return;
}

void do_chown(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA  *obj;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		stc("Get what from whom?\n", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg2, VIS_CHAR)) == NULL) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (!OUTRANKS(ch, victim)) {
		stc("I don't think they'd like that too much.\n", ch);
		return;
	}

	// don't use get_obj_carry/wear, the visibility check shouldn't fall to the victim
	if ((obj = get_obj_list(ch, arg1, victim->carrying)) == NULL) {
		stc("They do not have that item.\n", ch);
		return;
	}

	obj_from_char(obj);
	obj_to_char(obj, ch);
	act("$n makes a magical gesture and $p flies from $N to $n.", ch, obj, victim, TO_NOTVICT);
	act("$n makes a magical gesture and $p flies from your body to $s.", ch, obj, victim, TO_VICT);
	act("$p flies from $N to you.", ch, obj, victim, TO_CHAR);
} /* end do_chown() */

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
	OBJ_DATA *c_obj, *t_obj;

	for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content) {
		t_obj = create_object(c_obj->pIndexData, 0);

		if (! t_obj) {
			bug("Error creating object in recursive_clone.", 0);
			return;
		}

		clone_object(c_obj, t_obj);
		obj_to_obj(t_obj, clone);
		recursive_clone(ch, c_obj, t_obj);
	}
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char which[MAX_INPUT_LENGTH];
	const char *rest;
	CHAR_DATA *mob;
	OBJ_DATA  *obj;
	int j;
	int number;
	rest = one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Clone what?\n", ch);
		return;
	}

	if (!str_prefix1(arg, "object")) {
		number = mult_argument(rest, which);
		mob = NULL;
		obj = get_obj_here(ch, which);

		if (obj == NULL) {
			stc("You don't see that here.\n", ch);
			return;
		}
	}
	else if (!str_prefix1(arg, "mobile") || !str_prefix1(arg, "character")) {
		number = mult_argument(rest, which);
		obj = NULL;
		mob = get_char_here(ch, which, VIS_CHAR);

		if (mob == NULL) {
			stc("You don't see that here.\n", ch);
			return;
		}
	}
	else { /* find both */
		number = mult_argument(argument, which);
		mob = get_char_here(ch, which, VIS_CHAR);
		obj = get_obj_here(ch, which);

		if (mob == NULL && obj == NULL) {
			stc("You don't see that here.\n", ch);
			return;
		}
	}

	if (number < 1) {
		stc("Now that's a silly number to want to clone.\n", ch);
		return;
	}

	if (number > 99) {
		stc("Sorry, you can't clone that many.\n", ch);
		return;
	}

	/* clone an object */
	if (obj != NULL) {
		OBJ_DATA *clone = NULL;

		for (j = 1; j <= number; j++) {
			clone = create_object(obj->pIndexData, 0);

			if (! clone) {
				bug("Error creating object in do_clone", 0);
				return;
			}

			clone_object(obj, clone);

			if (obj->carried_by != NULL)
				obj_to_char(clone, ch);
			else
				obj_to_room(clone, ch->in_room);

			recursive_clone(ch, obj, clone);
		}

		if (number == 1) {
			act("$n clones $p.", ch, clone, NULL, TO_ROOM);
			act("You clone $p.", ch, clone, NULL, TO_CHAR);
			wiznet("$N has cloned: $p.", ch, clone, WIZ_LOAD, WIZ_SECURE, GET_RANK(ch));
		}
		else {
			sprintf(arg, "$n clones $p[%d].", number);
			act(arg, ch, clone, NULL, TO_ROOM);
			sprintf(arg, "You clone $p[%d].", number);
			act(arg, ch, clone, NULL, TO_CHAR);
			sprintf(arg, "$N has cloned: $p[%d].", number);
			wiznet(arg, ch, clone, WIZ_LOAD, WIZ_SECURE, GET_RANK(ch));
		}

		return;
	}
	else if (mob != NULL) {
		CHAR_DATA *clone;
		OBJ_DATA *new_obj;
		char buf[MAX_STRING_LENGTH];

		if (!IS_NPC(mob)) {
			stc("You can only clone mobiles.\n", ch);
			return;
		}

		clone = create_mobile(mob->pIndexData);

		/* Check for error. -- Outsider */
		if (! clone) {
			bug("Memory error in do_clone().", 0);
			stc("Error while cloning mob.\n", ch);
			return;
		}

		clone_mobile(mob, clone);

		for (obj = mob->carrying; obj != NULL; obj = obj->next_content) {
			new_obj = create_object(obj->pIndexData, 0);

			if (! new_obj) {
				bug("Error creating object in do_clone", 0);
				return;
			}

			clone_object(obj, new_obj);
			recursive_clone(ch, obj, new_obj);
			obj_to_char(new_obj, clone);
			new_obj->wear_loc = obj->wear_loc;
		}

		char_to_room(clone, ch->in_room);
		act("$n clones $N.", ch, NULL, clone, TO_ROOM);
		act("You clone $N.", ch, NULL, clone, TO_CHAR);
		sprintf(buf, "$N has cloned: %s.", clone->short_descr);
		wiznet(buf, ch, NULL, WIZ_LOAD, WIZ_SECURE, GET_RANK(ch));
		return;
	}
} /* end do_clone() */

/* Funky style clone command */
void do_oclone(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA  *obj;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		stc("Syntax: oclone <object> <person who has object>\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg2, VIS_CHAR)) == NULL) {
		stc("They aren't here.\n", ch);
		return;
	}

	// don't use get_obj_carry/wear, the visibility check shouldn't fall to the victim
	if ((obj = get_obj_list(ch, arg1, victim->carrying)) == NULL) {
		sprintf(buf, "%s doesn't seem to have a %s.\n", victim->name, arg1);
		stc(buf, ch);
		return;
	}

	/* clone an object */
	if (obj != NULL) {
		OBJ_DATA *clone;
		clone = create_object(obj->pIndexData, 0);

		if (! clone) {
			bug("Error cloning an object.", 0);
			return;
		}

		clone_object(obj, clone);

		if (obj->carried_by != NULL)
			obj_to_char(clone, ch);
		else
			obj_to_room(clone, ch->in_room);

		recursive_clone(ch, obj, clone);
		act("$n has cloned the $p $N has.", ch, clone, victim, TO_NOTVICT);
		act("$n has created a copy of your $p.", ch, clone, victim, TO_VICT);
		act("You clone a copy of the $p $N has.", ch, clone, victim, TO_CHAR);
		return;
	}
}

void do_departedlist(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0') {
		stc("Syntax:\n"
		    "  departedlist add    <immortal>\n"
		    "  departedlist remove <immortal>\n", ch);
		return;
	}

	if (!str_cmp(arg, "add")) {
		if (has_departed(argument)) {
			ptc(ch, "'%s' already departed Legacy!\n", argument);
			return;
		}

		insert_departed(argument);
		save_departed_list();
		ptc(ch, "'%s' was successfully added as a departed immortal.\n", argument);
		return;
	}

	if (!str_cmp(arg, "remove")) {
		if (!has_departed(argument)) {
			ptc(ch, "But no one named '%s' ever departed!\n", argument);
			return;
		}

		remove_departed(argument);
		save_departed_list();
		ptc(ch, "'%s' was successfully removed as a departed immortal.\n", argument);
		return;
	}
}

void do_leader(CHAR_DATA *ch, const char *argument)
{
	char arg[MIL];
	CHAR_DATA *victim;
	/* Not using these at the moment. -- Outsider
	int count = 0, ldrcount = 0;
	*/
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Whom do you wish to make a clan leader?\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg, VIS_PLR)) == NULL) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (victim->clan == NULL && !IS_IMMORTAL(victim)) {
		stc("They're not in a clan.\n", ch);
		return;
	}

	if (!IS_IMMORTAL(victim) && victim->clan->independent) {
		stc("Their clan cannot have leaders.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		if (!IS_IMP(ch)) {
			stc("Only implementors may appoint other implementors.\n", ch);
			return;
		}

		if (IS_SET(victim->pcdata->cgroup, GROUP_LEADER)) {
			REM_CGROUP(victim, GROUP_LEADER);
			stc("You are no longer an implementor.\n", victim);
			stc("Leader flag removed.\n", ch);
		}
		else {
			if (IS_SET(victim->pcdata->cgroup, GROUP_DEPUTY)) {
				REM_CGROUP(victim, GROUP_DEPUTY);
				stc("You have been promoted to implementor.\n", victim);
				stc("Deputy flag removed.\n", ch);
			}
			else
				stc("You are now an implementor.\n", victim);

			SET_CGROUP(victim, GROUP_LEADER);
			stc("Leader flag added.\n", ch);
		}

		return;
	}

	/* We no longer care how many leaders are in a clan.
	   -- Outsider

	count = count_clan_members(victim->clan, 0);
	ldrcount = count_clan_members(victim->clan, GROUP_LEADER);

	if (ldrcount >= 3)
	{
	     stc("That clan cannot have any more leaders.\n", ch);
	     return;
	}
	*/

	if (IS_SET(victim->pcdata->cgroup, GROUP_LEADER)) {
		REM_CGROUP(victim, GROUP_LEADER);
		stc("You are no longer a clan leader.\n", victim);
		stc("Leader flag removed.\n", ch);
	}
	else {
		if (IS_SET(victim->pcdata->cgroup, GROUP_DEPUTY)) {
			REM_CGROUP(victim, GROUP_DEPUTY);
			stc("You have been promoted to clan leader.\n", victim);
			stc("Deputy flag removed.\n", ch);
		}
		else
			stc("You are now a clan leader.\n", victim);

		SET_CGROUP(victim, GROUP_LEADER);
		stc("Leader flag added.\n", ch);
	}
}

void do_deputize(CHAR_DATA *ch, const char *argument)
{
	char arg[MIL];
	CHAR_DATA *victim;
	int count = 0, depcount = 0;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Whom do you wish to make a clan deputy?\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg, VIS_PLR)) == NULL) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (victim->clan == NULL && !IS_IMMORTAL(victim)) {
		stc("They're not in a clan.\n", ch);
		return;
	}

	if (!IS_IMMORTAL(victim) && victim->clan->independent) {
		stc("Their clan cannot have deputies.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		if (!IS_IMP(ch)) {
			stc("Only implementors may appoint heads of departments.\n", ch);
			return;
		}

		if (IS_SET(victim->pcdata->cgroup, GROUP_DEPUTY)) {
			REM_CGROUP(victim, GROUP_DEPUTY);
			stc("You are no longer a head of your department.\n", victim);
			stc("Deputy flag removed.\n", ch);
		}
		else {
			if (IS_SET(victim->pcdata->cgroup, GROUP_LEADER)) {
				REM_CGROUP(victim, GROUP_LEADER);
				stc("You have been demoted to department head.\n", victim);
				stc("Leader flag removed.\n", ch);
			}
			else
				stc("You are now a head of your department.\n", victim);

			SET_CGROUP(victim, GROUP_DEPUTY);
			stc("Deputy flag added.\n", ch);
		}

		return;
	}

	count = count_clan_members(victim->clan, 0);
	depcount = count_clan_members(victim->clan, GROUP_DEPUTY);

	if (depcount >= 5 || depcount > count / 5) {
		stc("That clan cannot have any more deputies.\n", ch);
		return;
	}

	if (IS_SET(victim->pcdata->cgroup, GROUP_DEPUTY)) {
		REM_CGROUP(victim, GROUP_DEPUTY);
		stc("You are no longer a clan deputy.\n", victim);
		stc("Deputy flag removed.\n", ch);
	}
	else {
		if (IS_SET(victim->pcdata->cgroup, GROUP_LEADER)) {
			REM_CGROUP(victim, GROUP_LEADER);
			stc("You have been demoted to clan deputy.\n", victim);
			stc("Leader flag removed.\n", ch);
		}
		else
			stc("You are now a clan deputy.\n", victim);

		SET_CGROUP(victim, GROUP_DEPUTY);
		stc("Deputy flag added.\n", ch);
	}
}

void do_despell(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Syntax: \n", ch);
		stc("despell <object name>\n", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		stc("No such item.\n", ch);
		return;
	}

	stc("Item cleared of all spells.\n", ch);
	return;
}

void do_disconnect(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	int desc;
	char buf[MAX_INPUT_LENGTH];

	if (argument == NULL || *argument == '\0') {
		stc("Disconnect which socket? (type SOCKETS for list)\n",
		    ch);
		return;
	}

	one_argument(argument, arg);

	if (!is_number(arg)) {
		stc("Socket must be numeric!\n", ch);
		return;
	}

	if (arg[0] == '\0') {
		stc("Disconnect whom?\n", ch);
		return;
	}

	desc = atoi(arg);

	for (d = descriptor_list; d != NULL; d = d->next) {
		if (d->descriptor == desc) {
			if (d->connected == 0) {
				sprintf(buf,
				        "But '%s' is playing! A simple QUIT would suffice.\n",
				        d->original ? d->original->name : d->character->name);
				stc(buf, ch);
				return;
			}

			close_socket(d);
			stc("The character has been disconnected.\n", ch);
			return;
		}
	}

	sprintf(buf,
	        "No socket number '%d' found. Check SOCKETS and try again!\n",
	        desc);
	stc(buf, ch);
}

/* idea by Erwin Andreasen */
/* Switch into another (perhaps live player) and execute a command, then switch back */
void do_doas(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *orig;
	char arg[MAX_STRING_LENGTH];
	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0') {
		stc("Syntax:  doas <victim> <command>\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) == NULL) {
		stc("You couldn't find them.\n", ch);
		return;
	}

	if (victim == ch) {
		stc("You're too lazy to make yourself do something.\n", ch);
		return;
	}

	if (!str_prefix1(argument, "quit") || !str_prefix1(argument, "delete") || !str_prefix1(argument, "switch")) {
		stc("Don't do that.\n", ch);
		return;
	}

	orig            = victim->desc;
	victim->desc    = ch->desc;
	interpret(victim, argument);
	ch->desc        = victim->desc;
	victim->desc    = orig;
}

void do_echo(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;

	if (argument[0] == '\0') {
		stc("Global echo what?\n", ch);
		return;
	}

	if (swearcheck(argument)) {
		ptc(ch, "Thou shalt not swear on gecho, %s.\n", ch->name);
		return;
	}

	for (d = descriptor_list; d; d = d->next) {
		if (IS_PLAYING(d)) {
			if (IS_IMMORTAL(d->character))
				stc("global> ", d->character);

			stc(argument, d->character);
			stc("\n",   d->character);
		}
	}
}

void do_recho(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;

	if (argument[0] == '\0') {
		stc("Local echo what?\n", ch);
		return;
	}

	for (d = descriptor_list; d; d = d->next) {
		if (IS_PLAYING(d)
		    &&   d->character->in_room == ch->in_room) {
			if (IS_IMMORTAL(d->character))
				stc("local> ", d->character);

			stc(argument, d->character);
			stc("\n",   d->character);
		}
	}

	return;
}

void do_zecho(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;

	if (argument[0] == '\0') {
		stc("Zone echo what?\n", ch);
		return;
	}

	for (d = descriptor_list; d; d = d->next) {
		if (IS_PLAYING(d)
		    &&  d->character->in_room != NULL && ch->in_room != NULL
		    &&  d->character->in_room->area == ch->in_room->area) {
			if (IS_IMMORTAL(d->character))
				stc("zone> ", d->character);

			stc(argument, d->character);
			stc("\n", d->character);
		}
	}
}

void do_pecho(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	argument = one_argument(argument, arg);

	if (argument[0] == '\0' || arg[0] == '\0') {
		stc("Personal echo what?\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) == NULL) {
		stc("Sorry, the target was not found.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim) && !IS_IMP(ch))
		stc("personal> ", victim);

	stc(argument, victim);
	stc("\n", victim);
	stc("personal> ", ch);
	stc(argument, ch);
	stc("\n", ch);
}

/* File command by Lotus */
void do_file(CHAR_DATA *ch, const char *argument)
{
	FILE *req_file;
	int num_lines = 0, req_lines = 0, cur_line = 0, i;
	char field[MIL], value[MIL], buf[MSL], strsave[50];
	BUFFER *buffer;
	struct file_struct {
		char   *cmd;
		char   *file;
	}
	fields[] = {
		{       "bugs",         "bugs.txt"      },
		{       "typos",        "typos.txt"     },
		{       "ridea",        "ridea.txt"     },      /* Montrey */
		{       "email",        "email.txt"     },
		{       "work",         "work.txt"      },      /* Elrac */
		{       "checkout",     "checkout.txt"  },      /* Elrac */
		{       "wbi",          "wbi.txt"       },      /* Elrac */
		{       "wbb",          "wbb.txt"       },      /* Chilalin */
		{       "immapp",       "immapp.txt"    },      /* Montrey */
		{       "punishment",   "punishment.txt"},
		{       "hbi",          "hbi.txt"       },
		{       "hbb",          "hbb.txt"       },      /* Chilalin */
		{       NULL,           NULL }
	};
	argument = one_argument(argument, field);
	one_argument(argument, value);

	if (field[0] == '\0') {
		stc("Accessible files:\n\n"
		    "Name:          Path:\n", ch);

		for (i = 0; fields[i].cmd; i++)
			ptc(ch, "%-14s[%s%-14s]\n", fields[i].cmd, MISC_DIR, fields[i].file);

		stc("\nSyntax:\n"
		    "  {Rfile{x <name> <lines>\n", ch);
		return;
	}

	for (i = 0; fields[i].cmd; i++)
		if (!str_prefix1(field, fields[i].cmd))
			break;

	if (!fields[i].cmd) {
		stc("Invalid file name.\n", ch);
		return;
	}

	if (value[0] == '\0')
		req_lines = 150;
	else
		req_lines = URANGE(1, atoi(value), 150);

	sprintf(strsave, "%s%s", MISC_DIR, fields[i].file);

	if ((req_file = fopen(strsave, "r")) == NULL) {
		stc("That file does not exist.\n", ch);
		return;
	}

	/* count lines in requested file */
	while (fgets(buf, MIL, req_file) != NULL)
		num_lines++;

	fclose(req_file);

	/* Limit # of lines printed to # requested or # of lines in file */
	if (req_lines > num_lines)
		req_lines = num_lines;

	if (num_lines <= 0) {
		stc("That file is empty.\n", ch);
		return;
	}

	if ((req_file = fopen(strsave, "r")) == NULL) {
		bug("do_file: file does not exist on second attempt", 0);
		stc("That file does not exist.\n", ch);
		return;
	}

	buffer = new_buf();

	/* and print the requested lines */
	while (fgets(buf, MIL, req_file) != NULL)
		if (++cur_line > (num_lines - req_lines))
			add_buf(buffer, buf);

	fclose(req_file);
	page_to_char(buf_string(buffer), ch);
	free_buf(buffer);
}

void do_followerlist(CHAR_DATA *ch, const char *argument)
{
	char query[MSL], deity[MSL];

	if (argument[0] == '\0')
		strcpy(deity, ch->name);
	else
		one_argument(argument, deity);

	strcpy(query, "SELECT name, deity FROM pc_index WHERE deity_nocol LIKE '");
	strcat(query, "%");
	strcat(query, deity);
	strcat(query, "%'");

	if (db_query("do_followerlist", query) != SQL_OK)
		return;

	BUFFER *buffer = new_buf();
	ptb(buffer, "{GFollowers of %s{G:{x\n", deity);
	add_buf(buffer, "{G=================================================================={x\n");

	int count = 0;
	while (db_next_row() == SQL_OK) {
		count++;
		char deityblock[MSL];
		int space = 50 - color_strlen(db_get_column_str(1));
		strcpy(deityblock, "{W");

		while (space-- >= 0)
			strcat(deityblock, " ");

		strcat(deityblock, db_get_column_str(1));
		ptb(buffer, "{G[{W%12s{G][%s{G]{x\n", db_get_column_str(0), deityblock);
	}

	if (count == 0)
		ptc(ch, "No one follows %s.\n", deity);
	else {
		add_buf(buffer, "{G=================================================================={x\n");
		ptb(buffer, "{WThere %s %d follower%s of %s{x.\n",
		    count == 1 ? "is" : "are", count, count == 1 ? "" : "s", deity);
		page_to_char(buf_string(buffer), ch);
	}

	free_buf(buffer);
}

/* Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard Right
   now its pretty much useless, but it might be useful in the future
   - Lotus */
const char *name_expand(CHAR_DATA *ch)
{
	int count = 1;
	CHAR_DATA *rch;
	char name[MAX_INPUT_LENGTH];
	static char outbuf[MAX_INPUT_LENGTH];

	if (!IS_NPC(ch))
		return ch->name.c_str();

	one_argument(ch->name, name);  /* copy the first word into name */

	if (!name[0]) { /* weird mob .. no keywords */
		strcpy(outbuf, "");  /* Return an empty buffer */
		return outbuf;
	}

	for (rch = ch->in_room->people; rch && (rch != ch); rch = rch->next_in_room)
		if (is_name(name, rch->name))
			count++;

	sprintf(outbuf, "%d.%s", count, name);
	return outbuf;
}

void do_for(CHAR_DATA *ch, const char *argument)
{
	char range[MIL], buf[MSL];
	ROOM_INDEX_DATA *room, *old_room = NULL;
	CHAR_DATA *p, *p_next;
	bool fGods = FALSE, fMortals = FALSE, fRoom = FALSE, found;
	int i;
	argument = one_argument(argument, range);

	if (!range[0] || !argument[0]) { /* invalid usage? */
		stc("Syntax:\n"
		    "  for all     <action>\n"
		    "  for gods    <action>\n"
		    "  for mortals <action>\n"
		    "  for room    <action>\n", ch);
		return;
	}

	if (!str_prefix1("slay", argument)
	    || !str_prefix1("purge", argument)
	    || !str_prefix1("quit", argument)
	    || !str_prefix1("for ", argument)
	    || !str_prefix1("delete", argument)
	    || !str_prefix1("who", argument)) {
		stc("I don't think so Tim!\n", ch);
		return;
	}

	if (!str_prefix1(range, "all")) {
		fMortals = TRUE;
		fGods = TRUE;
	}
	else if (!str_prefix1(range, "gods"))
		fGods = TRUE;
	else if (!str_prefix1(range, "mortals"))
		fMortals = TRUE;
	else if (!str_prefix1(range, "room"))
		fRoom = TRUE;
	else {
		stc("Syntax:\n"
		    "  for all     <action>\n"
		    "  for gods    <action>\n"
		    "  for mortals <action>\n"
		    "  for room    <action>\n", ch);
		return;
	}

	if (strchr(argument, '#')) { /* replace # ? */
		for (p = char_list; p; p = p_next) {
			p_next = p->next;
			found = FALSE;

			if (!(p->in_room) || (p == ch) || (room_is_private(p->in_room) && IS_IMMORTAL(p)))
				continue;
			else if (IS_NPC(p) && !fRoom)
				continue;
			else if (!IS_IMMORTAL(p) && fMortals)
				found = TRUE;
			else if (IS_IMMORTAL(p) && fGods)
				found = TRUE;
			else if (p->in_room == ch->in_room && fRoom)
				found = TRUE;

			if (found) { /* p is 'appropriate' */
				const char *pSource = argument; /* head of buffer to be parsed */
				char *pDest = buf; /* parse into this */

				while (*pSource) {
					if (*pSource == '#') { /* Replace # with name of target */
						const char *namebuf = name_expand(p);

						if (namebuf) /* in case there is no mob name ?? */
							while (*namebuf) /* copy name over */
								*(pDest++) = *(namebuf++);

						pSource++;
					}
					else
						*(pDest++) = *(pSource++);
				} /* while */

				*pDest = '\0'; /* Terminate */

				/* Execute */
				if (!fRoom) {
					old_room = ch->in_room;
					char_from_room(ch);
					char_to_room(ch, p->in_room);
				}

				interpret(ch, buf);

				if (ch && !fRoom) {     /* make sure ch still exists! :P -- Montrey */
					char_from_room(ch);
					char_to_room(ch, old_room);
				}
			} /* if found */
		} /* for every char */
	}
	else { /* just for every room with the appropriate people in it */
		if (fRoom) {
			interpret(ch, argument);
			return;
		}

		for (i = 0; i < MAX_KEY_HASH; i++) { /* run through all the buckets */
			for (room = room_index_hash[i] ; room ; room = room->next) {
				found = FALSE;

				/* Anyone in here at all? */
				if (!room->people) /* Skip it if room is empty */
					continue;

				/* Check if there is anyone here of the requried type */
				for (p = room->people; p; p = p->next_in_room) {
					if (!(p->in_room) || (p == ch) || (room_is_private(p->in_room) && IS_IMMORTAL(p)))
						continue;
					else if (IS_NPC(p) && !fRoom)
						continue;
					else if (IS_IMMORTAL(p) && fGods)
						found = TRUE;
					else if (!IS_IMMORTAL(p) && fMortals)
						found = TRUE;
				}

				if (found) {
					old_room = ch->in_room;
					char_from_room(ch);
					char_to_room(ch, room);
					interpret(ch, argument);

					if (ch) {
						char_from_room(ch);
						char_to_room(ch, old_room);
					}
				} /* if found */
			} /* Index Hash */
		} /* for every room in a bucket */
	} /* if strchr */
} /* do_for */

void do_goto(CHAR_DATA *ch, const char *argument)
{
	char arg[MIL];
	ROOM_INDEX_DATA *location = NULL;
	CHAR_DATA *rch;
	OBJ_DATA *obj;
	int count = 0;
	bool goto_pet = FALSE;

	if (argument[0] == '\0') {
		stc("Syntax:\n"
		    "  goto <room vnum>\n"
		    "  goto <character name or object name>\n"
		    "  goto obj <object name>\n", ch);
		return;
	}

	one_argument(argument, arg);

	if (is_number(arg))
		location = get_room_index(atoi(arg));
	else {
		if ((rch = get_char_world(ch, argument, VIS_CHAR)) != NULL)
			location = rch->in_room;

		if (!str_prefix1(arg, "object") || location == NULL) {
			location = NULL;

			if ((obj = get_obj_world(ch, argument)) != NULL)
				if (obj->in_room)
					location = obj->in_room;
		}
	}

	if (location == NULL) {
		stc("No such location.\n", ch);
		return;
	}

	for (rch = location->people; rch != NULL; rch = rch->next_in_room)
		count++;

	if (!is_room_owner(ch, location) && room_is_private(location)
	    && (count > 1 || !IS_IMP(ch))) {
		stc("That room is private.\n", ch);
		return;
	}

	if (ch->fighting != NULL)
		stop_fighting(ch, TRUE);

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
		if (can_see_char(rch, ch)) {
			if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
				act("$t", ch, ch->pcdata->bamfout, rch, TO_VICT);
			else
				act("$n warps off to distant lands in a burst of electric energy.", ch, NULL, rch, TO_VICT);
		}
	}

	if (ch->pet != NULL && ch->in_room == ch->pet->in_room && !IS_SET(ch->pet->act, ACT_STAY))
		goto_pet = TRUE;

	char_from_room(ch);
	char_to_room(ch, location);

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
		if (can_see_char(rch, ch)) {
			if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
				act("$t", ch, ch->pcdata->bamfin, rch, TO_VICT);
			else
				act("$n materializes in a burst of electric energy.", ch, NULL, rch, TO_VICT);
		}
	}

	do_look(ch, "auto");

	if (goto_pet) {
		char_from_room(ch->pet);
		char_to_room(ch->pet, location);
		do_look(ch->pet, "auto");
	}
}

/*
** Grouplist command by Demonfire on 02-25-1998
** Grouplist with no arguments will show all the current
** groups on the MUD. Only players will be shown.
*/
void do_grouplist(CHAR_DATA *ch, const char *argument)
{
	struct group_data {
		CHAR_DATA *leader;
		struct group_data *next;
	};
	typedef struct group_data GROUP_DATA;
	DESCRIPTOR_DATA *d;
	GROUP_DATA *leaders = NULL;
	GROUP_DATA *newnode, *curnode, *nextnode;
	CHAR_DATA *victim;
	bool dupe;
	int counter = 0;
	char buf[MAX_STRING_LENGTH];

	/* loop over all players, looking for leaders.
	   we don't find the leaders directly, we get pointers to them
	   from their followers. */
	for (d = descriptor_list; d != NULL; d = d->next) {
		victim = (d->original != NULL) ? d->original : d->character;

		if (victim == NULL)
			continue;

		if (victim->leader == NULL)
			continue;

		/* check for duplicate, add only new leaders */
		dupe = FALSE;

		for (curnode = leaders; curnode != NULL; curnode = curnode->next) {
			if (curnode->leader == victim->leader) {
				dupe = TRUE;
				break;
			}
		}

		if (!dupe) {
			newnode = (GROUP_DATA *)alloc_mem(sizeof(GROUP_DATA));
			newnode->leader = victim->leader;
			newnode->next = leaders;
			leaders = newnode;
		}
	}

	/* loop over all leaders, print the group */
	for (curnode = leaders; curnode != NULL; curnode = curnode->next) {
		sprintf(buf, "{G<G%d> {Y%s{x", ++counter, curnode->leader->name);

		/* find all followers */
		for (d = descriptor_list; d != NULL; d = d->next) {
			victim = (d->original != NULL) ? d->original : d->character;

			if (victim != NULL
			    && victim->leader != victim
			    && victim->leader == curnode->leader) {
				strcat(buf, " ");
				strcat(buf, victim->name);
			}
		}

		strcat(buf, "\n");
		stc(buf, ch);
	}

	/* check for 'no groups' */
	if (leaders == NULL) {
		stc("No groups found.\n", ch);
		return;
	}

	/* release leader chain */
	while (leaders != NULL) {
		nextnode = leaders->next;
		free_mem(leaders, sizeof(GROUP_DATA));
		leaders = nextnode;
	}
} /* end do_grouplist() */

void do_guild(CHAR_DATA *ch, const char *argument)
{
	char arg1[MIL], arg2[MIL];
	CHAR_DATA *victim;
	CLAN_DATA *clan;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		stc("Syntax: guild <char> <clan name>\n", ch);
		return;
	}

	/* players first! */
	victim = get_player_world(ch, arg1, VIS_PLR);

	if (victim == NULL)
		victim = get_char_world(ch, arg1, VIS_PLR);

	if (victim == NULL) {
		stc("The character is not logged in the realm.\n", ch);
		return;
	}

	if (!str_prefix1(arg2, "none")) {
		stc("The character is now clanless.\n", ch);
		stc("You are now clanless.\n", victim);

		if (!IS_IMMORTAL(victim)) {
			/* Remove leaderflag if it's set */
			REM_CGROUP(victim, GROUP_LEADER);
			REM_CGROUP(victim, GROUP_DEPUTY);
			REM_CGROUP(victim, GROUP_CLAN);
		}

		victim->clan = NULL;
		victim->questpoints_donated = 0;
		victim->gold_donated = 0;
		save_char_obj(victim);
		return;
	}

	clan = clan_lookup(arg2);

	if (clan == NULL) {
		stc("No such clan exists.\n", ch);
		return;
	}

	if (ch != victim)
		ptc(ch, "The character is now a member of %s.\n", clan->clanname);

	ptc(victim, "You have now become a member of %s.\n", clan->clanname);

	if (!IS_IMMORTAL(victim)) {
		/* Remove leaderflag if it's set */
		REM_CGROUP(victim, GROUP_LEADER);
		REM_CGROUP(victim, GROUP_DEPUTY);
	}

	SET_CGROUP(victim, GROUP_CLAN);
	victim->clan = clan;
	victim->questpoints_donated = 0;
	victim->gold_donated = 0;
	save_char_obj(victim);
}

/* answer to PRAY. Goes to one mortal like TELL does but does not reveal the
   imm who is sending the message. Also broadcasts to all other imms in game. */
void do_heed(CHAR_DATA *ch, const char *argument)
{
	char arg1[MIL], buf[100 + MIL]; /* enough for pompous intro + text */
	CHAR_DATA *victim, *truevictim;
	PC_DATA *tpc;
	DESCRIPTOR_DATA *d;
	argument = one_argument(argument, arg1);

	if (!arg1[0]) {
		if (IS_SET(ch->pcdata->plr, PLR_HEEDNAME)) {
			stc("Your name will not be shown in heeds.\n", ch);
			REMOVE_BIT(ch->pcdata->plr, PLR_HEEDNAME);
		}
		else {
			stc("Your name will now be shown in heeds.\n", ch);
			SET_BIT(ch->pcdata->plr, PLR_HEEDNAME);
		}

		return;
	}

	/* find a player to talk to. Only REAL players are eligible. */
	for (tpc = pc_list; tpc; tpc = tpc->next)
		if (is_name(arg1, tpc->ch->name))
			break;

	if (!tpc || (truevictim = tpc->ch) == NULL) {
		ptc(ch, "No player called \"%s\" is in the game!\n", arg1);
		return;
	}

	/* is he linkdead? */
	if (IS_SET(truevictim->pcdata->plr, PLR_LINK_DEAD)) {
		ptc(ch, "%s is linkdead at this time.\n", truevictim->name);
		return;
	}

	/* if he's SWITCHed or MORPHed, track down his current char */
	victim = truevictim;

	if (victim->desc == NULL) {
		victim = NULL;

		for (d = descriptor_list; d; d = d->next) {
			if (d->original == truevictim) {
				victim = d->character;
				break;
			}
		}

		if (!victim) {
			ptc(ch, "%s is unavailable at the moment.\n", truevictim->name);
			return;
		}
	}

	/* make sure there's a message */
	if (!argument[0]) {
		ptc(ch, "HEED %s with what message?\n", truevictim->name);
		return;
	}

	/* send the message to the player */
	new_color(victim, CSLOT_CHAN_PRAY);
	ptc(victim, "%s enlightens you with:\n   \"%s\"\n",
	    IS_SET(ch->pcdata->plr, PLR_HEEDNAME) ? ch->name : "An Immortal", argument);
	set_color(victim, WHITE, NOBOLD);
	/* build a message for the other imms */

	const String& wizname = ch->desc && ch->desc->original ? ch->desc->original->name : ch->name;

	if (victim != truevictim)
		sprintf(buf, "%s HEEDs %s (%s): %s\n", wizname, truevictim->name, victim->name, argument);
	else
		sprintf(buf, "%s HEEDs %s: %s\n", wizname, truevictim->name, argument);

	/* send it to all other imms who are connected and listening */
	for (d = descriptor_list; d; d = d->next) {
		if (IS_PLAYING(d)) {
			victim = d->character;
			truevictim = d->original ? d->original : victim;

			if (IS_IMMORTAL(truevictim)
			    && !IS_SET(truevictim->comm, COMM_NOPRAY)
			    && !IS_SET(truevictim->comm, COMM_QUIET)) {
				new_color(victim, CSLOT_CHAN_PRAY);
				stc(buf, victim);
				set_color(victim, WHITE, NOBOLD);
			}
		}
	}
} /* end do_heed() */

void do_linkload(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *dnew;
	DESCRIPTOR_DATA *d;
	int desc = 5;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Whom do you want to linkload?\n", ch);
		return;
	}

	for (d = descriptor_list; d != NULL; d = d->next)
		desc++;

	desc++;

	if (has_slash(arg)) {
		stc("That is not a valid player name.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) != NULL) {
		if (!IS_NPC(victim)) {
			stc("They are already playing or loaded.\n", ch);
			return;
		}
	}

	dnew = new_descriptor();
	dnew->descriptor    = desc;
	dnew->connected     = CON_PLAYING;

	char cname[MIL];
	strcpy(cname, argument);
	cname[0] = UPPER(cname[0]);

	if (load_char_obj(dnew, cname) == TRUE) {
		victim = dnew->character;
		victim->next = char_list;
		char_list    = victim;
		victim->pcdata->next = pc_list;
		pc_list = victim->pcdata;
		victim->desc = NULL;

		if (OUTRANKS(victim, ch)) {
			act("You're not high enough level to linkload $N.", ch, NULL, victim, TO_CHAR);
			extract_char(victim, TRUE);
		}
		else {
			sprintf(buf, "You reach into the pfile and link-load %s from room %d.\n",
			        victim->name, victim->in_room->vnum);
			stc(buf, ch);
			act("$n reaches into the pfiles and link-loads $N.", ch, NULL, victim, TO_NOTVICT);
			sprintf(buf, "$N has link-loaded %s from room %d.", victim->name, victim->in_room->vnum);
			wiznet(buf, ch, NULL, WIZ_LOAD, WIZ_SECURE, 0);
			char_to_room(victim, ch->in_room);

			if (victim->pet != NULL)
				char_to_room(victim->pet, ch->in_room);

			SET_BIT(victim->pcdata->plr, PLR_LINK_DEAD);
		}
	}
	else {
		stc("No such player exists in the pfiles.\n", ch);
		free_char(dnew->character);
	}

	free_descriptor(dnew);
} /* end do_linkload() */

void do_mload(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA *pMobIndex;
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	one_argument(argument, arg);

	if (arg[0] == '\0' || !is_number(arg)) {
		stc("Syntax: load mob <vnum>.\n", ch);
		return;
	}

	if ((pMobIndex = get_mob_index(atoi(arg))) == NULL) {
		stc("No mob has that vnum.\n", ch);
		return;
	}

	victim = create_mobile(pMobIndex);

	if (! victim) {
		bug("Memory error while creating mob.", 0);
		stc("Could not create mob.\n", ch);
		return;
	}

	char_to_room(victim, ch->in_room);
	act("$n creates $N!", ch, NULL, victim, TO_ROOM);
	sprintf(buf, "$N loads %s.", victim->short_descr);
	wiznet(buf, ch, NULL, WIZ_LOAD, WIZ_SECURE, GET_RANK(ch));
	stc("Success.\n", ch);
	return;
}

void do_oload(CHAR_DATA *ch, const char *argument)
{
	char arg1[MIL];
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *obj;
	argument = one_argument(argument, arg1);

	if (arg1[0] == '\0' || !is_number(arg1)) {
		stc("Syntax: load obj <vnum>.\n", ch);
		return;
	}

	if ((pObjIndex = get_obj_index(atoi(arg1))) == NULL) {
		stc("No object has that vnum.\n", ch);
		return;
	}

	obj = create_object(pObjIndex, 0);

	if (! obj) {   /* avoid memory issues */
		stc("You were unable to create that item.\n", ch);
		bug("Error creating object in do_oload.\n", 0);
		return;
	}

	if (CAN_WEAR(obj, ITEM_TAKE))
		obj_to_char(obj, ch);
	else
		obj_to_room(obj, ch->in_room);

	act("$n has created $p!", ch, obj, NULL, TO_ROOM);
	wiznet("$N loads $p.", ch, obj, WIZ_LOAD, WIZ_SECURE, GET_RANK(ch));
	stc("Success.\n", ch);
	return;
}

void do_load(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Syntax:\n", ch);
		stc("  load mob <vnum>\n", ch);
		stc("  load obj <vnum>\n", ch);
		return;
	}

	if (!str_cmp(arg, "mob") || !str_cmp(arg, "char")) {
		do_mload(ch, argument);
		return;
	}

	if (!str_cmp(arg, "obj")) {
		do_oload(ch, argument);
		return;
	}

	/* echo syntax */
	do_load(ch, "");
}

void do_lower(CHAR_DATA *ch, const char *argument)
{
	char what[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	CHAR_DATA *victim;
	int x;
	int qp = 0;
	struct quest_struct {
		int  level;
		int  cost;
	} quest[] = {
		{ LEVEL_IMMORTAL+2, 175 },
		{ LEVEL_IMMORTAL+1, 150 },
		{ LEVEL_IMMORTAL, 125 },
		{ 0, 0 }
	};
	extern void do_quest(CHAR_DATA * ch, const char *argument);
	argument = one_argument(argument, what);

	if (what[0] == '\0') {
		stc("Which item do you wish to lower?\n", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, what)) == NULL) {
		act_new("You're not carrying a(n) $t.", ch, what, NULL, TO_CHAR,
		        POS_DEAD, FALSE);
		return;
	}

	if (argument[0] == '\0') {
		act_new("Whom do you want to lower $t for?", ch,
		        obj->short_descr, NULL, TO_CHAR, POS_DEAD, FALSE);
		return;
	}

	if ((victim = get_player_world(ch, argument, VIS_PLR)) == NULL) {
		act_new("Sorry, no player called '$t' is in the game!", ch,
		        argument, NULL, TO_CHAR, POS_DEAD, FALSE);
		return;
	}

	if (victim->level < LEVEL_HERO) {
		act_new("Sorry, $t must be level 91 to have an item lowered.", ch,
		        victim->name.c_str(), NULL, TO_CHAR, POS_DEAD, FALSE);
		return;
	}

	if (obj->extra_descr != NULL) {
		EXTRA_DESCR_DATA *ed_next;
		EXTRA_DESCR_DATA *ed;

		for (ed = obj->extra_descr; ed != NULL; ed = ed_next) {
			ed_next = ed->next;

			if (!str_cmp(ed->keyword, KEYWD_OWNER)) {
				char strip[MAX_STRING_LENGTH];
				strcpy(strip, ed->description);
				strip[strlen(strip) - 2] = '\0';
				sprintf(buf, "This item is already owned by %s.\n", strip);
				stc(buf, ch);
				return;
			}
		}
	}

	for (x = 0; quest[x].level != 0; x++) {
		if (obj->level >= quest[x].level) {
			qp = quest[x].cost;
			break;
		}
	}

	if (qp == 0) {
		act_new("But $t is already lower than level 92.", ch,
		        obj->short_descr, NULL, TO_CHAR, POS_DEAD, FALSE);
		return;
	}

	if (qp > victim->questpoints && !IS_IMMORTAL(victim)) {
		stc("They do not have enough quest points for that.\n"
		    , ch);
		return;
	}

	if (!IS_IMMORTAL(victim)) {
		sprintf(buf, "%s %s", what, victim->name);
		do_owner(ch, buf);
		sprintf(buf, "deduct %s %d", victim->name, qp);
		do_quest(ch, buf);
	}

	obj->level = LEVEL_HERO;
	act_new("$t has been successfully lowered to level 91.", ch,
	        obj->short_descr, NULL, TO_CHAR, POS_DEAD, FALSE);
}

void do_lurk(CHAR_DATA *ch, const char *argument)
{
	if (ch->lurk_level) {
		ch->lurk_level = 0;
		act("$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM);
		stc("You are no longer cloaked.\n", ch);
	}
	else {
		ch->lurk_level = LEVEL_IMMORTAL;
		act("$n cloaks $s presence to those outside the room.", ch, NULL, NULL, TO_ROOM);
		stc("You cloak your presence to those outside the room.\n", ch);
	}
}

/* Master command by Lotus */
void do_master(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *pet;
	argument = one_argument(argument, arg1);

	if (arg1[0] == '\0' || argument[0] == '\0') {
		stc("Master what mobile to whom?\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == NULL) {
		stc("That player is not here.\n", ch);
		return;
	}

	if ((pet = get_char_here(victim, argument, VIS_CHAR)) == NULL) {
		stc("That mobile is not near the player.\n", ch);
		return;
	}

	/* Beware, if you do master->pet->master, it will loop the mud - Lotus */
	if (IS_NPC(victim) || !IS_NPC(pet)) {
		stc("That is not allowed!\n", ch);
		return;
	}

	if (victim->level < pet->level && !IS_IMMORTAL(victim)) {
		stc("That pet is too powerful for that player.\n", ch);
		return;
	}

	if (victim->pet != NULL) {
		stc("They have a pet already!\n", ch);
		return;
	}

	if (pet->master != NULL) {
		stc("That mobile already has a master.\n", ch);
		return;
	}

	make_pet(ch, pet);

	stc("You have set the player with a pet.\n", ch);
	stc("You have been set with a new pet.\n", victim);
	stc("You have been turned into a pet!\n", pet);
}

/* Olevel and Mlevel from ROM Newsgroup */
void do_olevel(CHAR_DATA *ch, const char *argument)
{
	extern int top_obj_index;
	char buf[MAX_STRING_LENGTH];
	char tmpbuf[80];        // Extra buffer, needed to fix mis-alignment. by Clerve
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	BUFFER *buffer;
	OBJ_INDEX_DATA *pObjIndex;
	int vnum, blevel, elevel;
	int nMatch, matches;
	unsigned long wear_loc;
	bool found, with_wear;
	/* Check 1st argument - required begin level */
	argument = one_argument(argument, arg1);

	if (!is_number(arg1)) {
		stc("Syntax: olevel [beg level] [end level] [wear type]\n", ch);
		return;
	}

	blevel = atoi(arg1);
	elevel = blevel;
	wear_loc = -1;          /* standard: everything */
	with_wear = FALSE;
	/* Check for 2nd argument - optional ending level */
	argument = one_argument(argument, arg2);

	if (arg2[0] != '\0') {
		if (!is_number(arg2)) {
			//stc("Syntax: olevel [beg level] [end level]\n",ch);
			with_wear = TRUE;
		}
		else {
			elevel = atoi(arg2);
			with_wear = FALSE;
		}
	}

	if ((blevel < 0) || (blevel > 120) || (elevel < blevel) || (elevel > 120)) {
		stc("Values must be between 0 and 120.\n", ch);
		stc("Second value must be greater than first.\n", ch);
		return;
	}

	/* Check for 3rd Argument - optional wear location*/
	if (with_wear)
		strcpy(arg3, arg2);
	else
		argument = one_argument(argument, arg3);

	bool specified_wear_loc = FALSE;

	if (arg3[0] != '\0') {
		if (!str_prefix1(arg3, "take"))
			wear_loc = ITEM_TAKE;
		else if (!str_prefix1(arg3, "finger"))
			wear_loc = ITEM_WEAR_FINGER;
		else if (!str_prefix1(arg3, "neck"))
			wear_loc = ITEM_WEAR_NECK;
		else if (!str_prefix1(arg3, "body"))
			wear_loc = ITEM_WEAR_BODY;
		else if (!str_prefix1(arg3, "head"))
			wear_loc = ITEM_WEAR_HEAD;
		else if (!str_prefix1(arg3, "legs"))
			wear_loc = ITEM_WEAR_LEGS;
		else if (!str_prefix1(arg3, "feet"))
			wear_loc = ITEM_WEAR_FEET;
		else if (!str_prefix1(arg3, "hands"))
			wear_loc = ITEM_WEAR_HANDS;
		else if (!str_prefix1(arg3, "arms"))
			wear_loc = ITEM_WEAR_ARMS;
		else if (!str_prefix1(arg3, "shield"))
			wear_loc = ITEM_WEAR_SHIELD;
		else if (!str_prefix1(arg3, "about"))
			wear_loc = ITEM_WEAR_ABOUT;
		else if (!str_prefix1(arg3, "waist"))
			wear_loc = ITEM_WEAR_WAIST;
		else if (!str_prefix1(arg3, "wrist"))
			wear_loc = ITEM_WEAR_WRIST;
		else if (!str_prefix1(arg3, "wield"))
			wear_loc = ITEM_WIELD;
		else if (!str_prefix1(arg3, "hold"))
			wear_loc = ITEM_HOLD;
		else if (!str_prefix1(arg3, "float"))
			wear_loc = ITEM_WEAR_FLOAT;
		else if (!str_prefix1(arg3, "none"))
			wear_loc = 0;
		else {
			stc("That is not a suitable wear location.\n", ch);
			return;
		}

		specified_wear_loc = TRUE;
	}

	buffer = new_buf();
	found = FALSE;
	nMatch = 0;
	matches = 0;

	for (vnum = 0; nMatch < top_obj_index; vnum++) {
		if ((pObjIndex = get_obj_index(vnum)) != NULL) {
			nMatch++;
			found = FALSE;

			if ((blevel <= pObjIndex->level) && (elevel >= pObjIndex->level)) {
				if (!specified_wear_loc)
					found = TRUE;
				else {
					if (IS_SET(pObjIndex->wear_flags, wear_loc) > 0)
						found = TRUE;

					if (pObjIndex->wear_flags == wear_loc)
						found = TRUE;
				}
			}
		}

		if (found) {
			sprintf(tmpbuf, "[%%4d][%%5d] %%-%zus{x [%%s]\n",
			        45 + (strlen(pObjIndex->short_descr) - color_strlen(pObjIndex->short_descr)));
			sprintf(buf, tmpbuf, pObjIndex->level, pObjIndex->vnum,
			        pObjIndex->short_descr, wear_bit_name(pObjIndex->wear_flags));
			add_buf(buffer, buf);
			matches++;
			found = FALSE;
		}
	}

	if (matches == 0)
		stc("No objects by that level.\n", ch);
	else {
		if (blevel < elevel)
			sprintf(buf, "Objects level range: %d to %d.\n", blevel, elevel);
		else
			sprintf(buf, "Objects level: %d\n", blevel);

		stc(buf, ch);
		stc("Level Vnum    Name                                          Wear Loc.\n", ch);
		page_to_char(buf_string(buffer), ch);
		sprintf(buf, "%d match%s found.\n", matches, (matches > 0) ? "es" : "");
		stc(buf, ch);
	}

	free_buf(buffer);
	return;
}

void do_mlevel(CHAR_DATA *ch, const char *argument)
{
	extern int top_mob_index;
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char tmpbuf[80];        // needed to mix misalignment due to colorcodes.. Clerve
	BUFFER *buffer;
	MOB_INDEX_DATA *pMobIndex;
	int vnum, blevel, elevel;
	int nMatch;
	bool found;
	argument = one_argument(argument, arg);

	if (!is_number(arg)) {
		stc("Syntax: mlevel [beg level] [end level]\n", ch);
		return;
	}

	blevel = atoi(arg);

	if (argument[0] != '\0') {
		if (!is_number(argument)) {
			stc("Syntax: olevel [beg level] [end level]\n", ch);
			return;
		}

		elevel = atoi(argument);
	}
	else
		elevel = blevel;

	if ((blevel < 0) || (blevel > 120) || (elevel < blevel) || (elevel > 120)) {
		stc("Values must be between 0 and 120.\n", ch);
		stc("Second value must be greater than first.\n", ch);
		return;
	}

	buffer = new_buf();
	found = FALSE;
	nMatch = 0;

	for (vnum = 0; nMatch < top_mob_index; vnum++) {
		if ((pMobIndex = get_mob_index(vnum)) != NULL) {
			nMatch++;

			if ((blevel <= pMobIndex->level) && (elevel >= pMobIndex->level)) {
				found = TRUE;
				sprintf(tmpbuf, "[%%3d][%%5d] %%-%zus (Align: %%d)\n",
				        40 + (strlen(pMobIndex->short_descr) - color_strlen(pMobIndex->short_descr)));
				sprintf(buf, tmpbuf,
				        pMobIndex->level, pMobIndex->vnum,
				        pMobIndex->short_descr, pMobIndex->alignment);
				add_buf(buffer, buf);
			}
		}
	}

	if (!found)
		stc("No mobiles by that level.\n", ch);
	else
		page_to_char(buf_string(buffer), ch);

	free_buf(buffer);
	return;
}

void do_motd(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];

	if (argument[0] != '\0') {
		buf[0] = '\0';

		if (!str_cmp(argument, "clear")) {
			free_string(time_info.motd);
			time_info.motd = str_dup(buf);
			stc("MOTD cleared.\n", ch);
			return;
		}

		if (argument[0] == '-') {
			int len;
			bool found = FALSE;

			if (time_info.motd == NULL || time_info.motd[0] == '\0') {
				stc("No lines left to remove.\n", ch);
				return;
			}

			strcpy(buf, time_info.motd);

			for (len = strlen(buf); len > 0; len--) {
				if (buf[len] == '\n') {
					if (!found) { /* back it up */
						if (len > 0)
							len--;

						found = TRUE;
					}
					else { /* found the second one */
						buf[len + 1] = '\0';
						free_string(time_info.motd);
						time_info.motd = str_dup(buf);
						set_color(ch, GREEN, NOBOLD);
						stc("The MOTD is:\n", ch);
						stc(time_info.motd ? time_info.motd :
						    "(None).\n", ch);
						set_color(ch, WHITE, NOBOLD);
						return;
					}
				}
			}

			buf[0] = '\0';
			free_string(time_info.motd);
			time_info.motd = str_dup(buf);
			stc("MOTD cleared.\n", ch);
			return;
		}

		if (argument[0] == '+') {
			if (time_info.motd != NULL)
				strcat(buf, time_info.motd);

			argument++;

			while (isspace(*argument))
				argument++;
		}

		if (strlen(buf) + strlen(argument) >= MAX_STRING_LENGTH - 2) {
			stc("MOTD too long.\n", ch);
			return;
		}

		strcat(buf, argument);
		strcat(buf, "\n");
		free_string(time_info.motd);
		time_info.motd = str_dup(buf);
	}

	set_color(ch, WHITE, BOLD);
	stc("The MOTD is:\n", ch);
	stc(time_info.motd ? time_info.motd : "(None).\n", ch);
	set_color(ch, WHITE, NOBOLD);
	return;
}

void do_owhere(CHAR_DATA *ch, const char *argument)
{
	char buf[MSL], arg[MIL], arg2[MIL];
	BUFFER *output;
	OBJ_DATA *obj, *in_obj;
	int count = 1, vnum = 0;
	bool fGround = FALSE;
	int place_last_found = 0;   /* the vnum of the place where we last found an item */
	int item_last_found = 0;    /* the vnum of the last item displayed */
	output = new_buf();
	argument = one_argument(argument, arg);
	one_argument(argument, arg2);

	if (arg[0] == '\0') {
		stc("Syntax:\n"
		    "  owhere <object or vnum> <second keyword, level, or 'ground'>\n", ch);
		return;
	}

	if (is_number(arg))
		vnum = atoi(arg);

	if (!str_prefix1(arg2, "ground"))
		fGround = TRUE;

	add_buf(output, "{VCount {YRoom  {GObject{x\n");

	/* cut off list at 400 objects, to prevent spamming out your link */
	for (obj = object_list; obj != NULL; obj = obj->next) {
		if (fGround && !obj->in_room)
			continue;

		if (vnum > 0) {
			if (obj->pIndexData->vnum != vnum)
				continue;
		}
		else if (!is_name(arg, obj->name))
			continue;

		if (argument[0] != '\0' && !fGround) {
			if (is_number(arg2)) {
				if (atoi(arg2) != obj->level)
					continue;
			}
			else if (vnum > 0 || !is_name(argument, obj->name))
				continue;
		}

		for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj)
			;

		if (in_obj->carried_by) {
			if (in_obj->carried_by->in_room == NULL
			    || !can_see_room(ch, in_obj->carried_by->in_room)
			    || !can_see_char(ch, in_obj->carried_by))
				continue;

			/* keep multiple hits on the same person from being displayed
			   -- Outsider
			*/
			if ((place_last_found == in_obj->carried_by->in_room->vnum) &&
			    (item_last_found == obj->pIndexData->vnum))
				continue;

			sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{H[{G%5d{H]{x %s{x is carried by %s.\n",
			        count,
			        in_obj->carried_by->in_room->vnum,
			        obj->pIndexData->vnum,
			        obj->short_descr,
			        PERS(in_obj->carried_by, ch, VIS_PLR));
			/* mark down this item as the last one found -- Outsider */
			place_last_found = in_obj->carried_by->in_room->vnum;
			item_last_found = obj->pIndexData->vnum;
		}
		else if (in_obj->in_locker) {
			if (in_obj->in_locker->in_room == NULL
			    || !can_see_room(ch, in_obj->in_locker->in_room)
			    || !can_see_char(ch, in_obj->in_locker))
				continue;

			/* keep multiple hits on the same locker from being displayed
			   -- Outsider
			*/
			if ((place_last_found == in_obj->in_locker->in_room->vnum) &&
			    (item_last_found == obj->pIndexData->vnum))
				continue;

			sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{H[{G%5d{H]{x %s{x is in %s's locker.\n",
			        count,
			        in_obj->in_locker->in_room->vnum,
			        obj->pIndexData->vnum,
			        obj->short_descr,
			        PERS(in_obj->in_locker, ch, VIS_PLR));
			/* mark this down as the last one found */
			place_last_found = in_obj->in_locker->in_room->vnum;
			item_last_found = obj->pIndexData->vnum;
		}
		else if (in_obj->in_strongbox) {
			if (in_obj->in_strongbox->in_room == NULL
			    || !can_see_room(ch, in_obj->in_strongbox->in_room)
			    || !can_see_char(ch, in_obj->in_strongbox))
				continue;

			/* try not to get same items in the same place multiple times
			   -- Outsider
			*/
			if ((place_last_found == in_obj->in_strongbox->in_room->vnum) &&
			    (item_last_found == obj->pIndexData->vnum))
				continue;

			sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{H[{G%5d{H]{x %s{x is in %s's strongbox.\n",
			        count,
			        in_obj->in_strongbox->in_room->vnum,
			        obj->pIndexData->vnum,
			        obj->short_descr,
			        PERS(in_obj->in_strongbox, ch, VIS_PLR));
			/* mark the last item and place we found it  -- Outsider */
			place_last_found = in_obj->in_strongbox->in_room->vnum;
			item_last_found = obj->pIndexData->vnum;
		}
		else if (in_obj->in_room) {
			if (!can_see_room(ch, in_obj->in_room))
				continue;

			/* avoid printing duplicate items in the same place -- Outsider */
			if ((place_last_found == in_obj->in_room->vnum) &&
			    (item_last_found == obj->pIndexData->vnum))
				continue;

			sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{H[{G%5d{H]{x %s{x in %s.\n",
			        count,
			        in_obj->in_room->vnum,
			        obj->pIndexData->vnum,
			        obj->short_descr,
			        in_obj->in_room->name);
			/* mark the last place and item we found -- Outsider */
			place_last_found = in_obj->in_room->vnum;
			item_last_found = obj->pIndexData->vnum;
		}
		else    /* what's left? */
			continue;

		/* this should be 400 or less, not 500 -- Outsider */
		if (++count <= 400)     /* count stays one ahead of actual number found */
			add_buf(output, buf);
	}

	if (--count == 0)
		stc("You found no item like that.\n", ch);
	else {
		sprintf(buf, "You found %d matching item%s%s%s.\n",
		        count,
		        count > 1 ? "s" : "",
		        fGround ? " lying around" : "",
		        count > 400 ? ", of which 400 are shown" : "");
		add_buf(output, buf);
		page_to_char(buf_string(output), ch);
	}

	free_buf(output);
}

void do_mwhere(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	BUFFER *output;
	CHAR_DATA *victim;
	bool found;
	output = new_buf();
	argument = one_argument(argument, arg);
	one_argument(argument, arg2);

	if (arg[0] == '\0') {
		stc("Syntax: mwhere <mob name or vnum> <second keyword or level>\n", ch);
		return;
	}

	found = FALSE;

	for (victim = char_list; victim != NULL; victim = victim->next) {
		if (!IS_NPC(victim) || victim->in_room == NULL)
			continue;

		if (is_number(arg)) {
			if (atoi(arg) != victim->pIndexData->vnum)
				continue;
		}
		else if (!is_name(arg, victim->name))
			continue;

		if (arg2[0] != '\0') {
			if (is_number(arg2)) {
				if (atoi(arg2) != victim->level)
					continue;
			}
			else if (!is_name(arg2, victim->name))
				continue;
		}

		found = TRUE;
		sprintf(buf, "[%5d] %s%*s[%5d] %s\n",
		        victim->pIndexData->vnum,
		        victim->short_descr,
		        28 - color_strlen(victim->short_descr),
		        " ",
		        victim->in_room->vnum,
		        victim->in_room->name);
		add_buf(output, buf);
	}

	page_to_char(buf_string(output), ch);
	free_buf(output);

	if (!found) {
		if (is_number(arg))
			ptc(ch, "You did not find a mobile of vnum %d.", atoi(arg));
		else
			act("You did not find: $T.", ch, NULL, arg, TO_CHAR);

		return;
	}
}

/* find a room, given its name */
void do_rwhere(CHAR_DATA *ch, const char *argument)
{
	AREA_DATA *area;
	ROOM_INDEX_DATA *room;
	BUFFER *dbuf = NULL;
	char buf[MAX_INPUT_LENGTH], fname[MAX_INPUT_LENGTH], rbuf[MAX_INPUT_LENGTH];
	char *cp;
	bool found = FALSE;
	int vnum;

	if (*argument == '\0') {
		stc("rwhere which room?\n", ch);
		return;
	}

	dbuf = new_buf();

	for (area = area_first; area; area = area->next) {
		for (vnum = area->min_vnum; vnum <= area->max_vnum; vnum++) {
			room = get_room_index(vnum);

			if (room != NULL) {
				strcpy(rbuf, room->name);

				if (is_name(argument, smash_bracket(rbuf))) {
					found = TRUE;
					strcpy(fname, room->area->file_name);
					cp = strchr(fname, '.');

					if (cp != NULL)
						*cp = '\0';

					sprintf(buf, "[%5d] <%-8.8s> %s{x\n", vnum, fname, room->name);
					add_buf(dbuf, buf);
				}
			}
		}
	}

	if (!found)
		add_buf(dbuf, "No matching rooms found.\n");

	page_to_char(buf_string(dbuf), ch);
	free_buf(dbuf);
} /* end do_rwhere() */

void do_mfind(CHAR_DATA *ch, const char *argument)
{
	extern int top_mob_index;
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	BUFFER *output;
	MOB_INDEX_DATA *pMobIndex;
	int vnum;
	int nMatch;
	bool fAll;
	bool found;
	output = new_buf();
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Find whom?\n", ch);
		return;
	}

	fAll        = FALSE; /* !str_cmp( arg, "all" ); */
	found       = FALSE;
	nMatch      = 0;

	/*
	 * Yeah, so iterating over all vnum's takes 10,000 loops.
	 * Get_mob_index is fast, and I don't feel like threading another link.
	 * Do you?
	 * -- Furey
	 */
	for (vnum = 0; nMatch < top_mob_index; vnum++) {
		if ((pMobIndex = get_mob_index(vnum)) != NULL) {
			nMatch++;

			if (fAll || is_name(argument, pMobIndex->player_name)) {
				found = TRUE;
				sprintf(buf, "M (%3d) [%5d] %s\n",
				        pMobIndex->level, pMobIndex->vnum, pMobIndex->short_descr);
				add_buf(output, buf);
			}
		}
	}

	page_to_char(buf_string(output), ch);
	free_buf(output);

	if (!found)
		stc("No mobiles by that name.\n", ch);

	return;
}

void do_ofind(CHAR_DATA *ch, const char *argument)
{
	extern int top_obj_index;
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA *pObjIndex;
	BUFFER *output;
	int vnum;
	int nMatch;
	bool fAll;
	bool found;
	output = new_buf();
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Find what object?\n", ch);
		return;
	}

	fAll        = FALSE; /* !str_cmp( arg, "all" ); */
	found       = FALSE;
	nMatch      = 0;

	/*
	 * Yeah, so iterating over all vnum's takes 10,000 loops.
	 * Get_obj_index is fast, and I don't feel like threading another link.
	 * Do you?
	 * -- Furey
	 */
	for (vnum = 0; nMatch < top_obj_index; vnum++) {
		if ((pObjIndex = get_obj_index(vnum)) != NULL) {
			nMatch++;

			if (fAll || is_name(argument, pObjIndex->name)) {
				found = TRUE;
				sprintf(buf, "O (%3d) [%5d] %s\n",
				        pObjIndex->level, pObjIndex->vnum, pObjIndex->short_descr);
				add_buf(output, buf);
			}
		}
	}

	page_to_char(buf_string(output), ch);
	free_buf(output);

	if (!found)
		stc("No objects by that name.\n", ch);

	return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */
void do_vnum(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	const char *string;
	string = one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Syntax:\n", ch);
		stc("  vnum obj <name>\n", ch);
		stc("  vnum mob <name>\n", ch);
		stc("  vnum skill <skill or spell>\n", ch);
		return;
	}

	if (!str_cmp(arg, "obj")) {
		do_ofind(ch, string);
		return;
	}

	if (!str_cmp(arg, "mob") || !str_cmp(arg, "char")) {
		do_mfind(ch, string);
		return;
	}

	if (!str_cmp(arg, "skill") || !str_cmp(arg, "spell")) {
		do_slookup(ch, string);
		return;
	}

	/* do both */
	do_mfind(ch, argument);
	do_ofind(ch, argument);
}

void do_canmakebag(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		stc("Change newbie helper flag on who?", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) == NULL) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (IS_SET(victim->act, PLR_MAKEBAG)) {
		stc("You are no longer a Newbie Helper.\n", victim);
		stc("They are no longer a newbie helper.\n", ch);
		REMOVE_BIT(victim->act, PLR_MAKEBAG);
		return;
	}
	else {
		stc("You are now a Newbie Helper!! WooWoo!!.\n", victim);
		stc("They are now a newbie helper.\n", ch);
		SET_BIT(victim->act, PLR_MAKEBAG);
		return;
	}
}

/* Noreply by Lotus */
void do_noreply(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *wch;

	if (IS_NPC(ch))
		return;

	for (wch = char_list; wch != NULL; wch = wch->next) {
		if (! strcasecmp(wch->reply, ch->name))
			wch->reply[0] = '\0';
	}

	stc("Nobody can reply to you now.\n", ch);
	return;
}

/*
 * OWNER <item> <name(s)> -- Elrac
 *
 * This puts an extended description on an object with
 * keyword 'owned-by' and content <name(s)>.
 *
 * Equipment personalized in this way cannot be equipped
 * other than by the owner.
 *
 */
void do_owner(CHAR_DATA *ch, const char *argument)
{
	char what[MIL], whom[MIL];
	OBJ_DATA *item;
	CHAR_DATA *player = NULL;
	EXTRA_DESCR_DATA *ed;
	argument = one_argument(argument, what);
	argument = one_argument(argument, whom);

	if (what[0] == '\0') {
		stc("Which item do you want to personalize?\n", ch);
		return;
	}

	if ((item = get_obj_carry(ch, what)) == NULL) {
		stc("You are not carrying that.\n", ch);
		return;
	}

	if (whom[0] == '\0') {
		ptc(ch, "Whom do you want to personalize the %s to?\n", item->pIndexData->short_descr);
		ptc(ch, "Type 'owner %s none' if you want to make it public.\n", what);
		return;
	}

	if (str_cmp(whom, "none")) {
		if ((player = get_player_world(ch, whom, VIS_PLR)) == NULL) {
			stc("There is no one by that name in the realm.\n", ch);
			return;
		}
	}

	if (!str_cmp(whom, "none")) {
		if (item->extra_descr != NULL) {
			EXTRA_DESCR_DATA *ed_next, *ed_prev = NULL;

			for (ed = item->extra_descr; ed != NULL; ed = ed_next) {
				ed_next = ed->next;

				if (!str_cmp(ed->keyword, KEYWD_OWNER)) {
					if (ed == item->extra_descr)
						item->extra_descr = ed_next;
					else
						ed_prev->next = ed_next;

					free_extra_descr(ed);
				}
				else
					ed_prev = ed;
			}
		}

		act_new("OK, $t now belongs to no one in particular.", ch, item->short_descr, NULL, TO_CHAR, POS_DEAD, FALSE);
		return;
	}

	/* Made it to where you have to "onwer item none" before assigning a new onwer - Lotus  */
	if (item->extra_descr != NULL) {
		EXTRA_DESCR_DATA *ed_next;

		for (ed = item->extra_descr; ed != NULL; ed = ed_next) {
			ed_next = ed->next;

			if (!str_cmp(ed->keyword, KEYWD_OWNER)) {
				ptc(ch, "This item is already owned by %s.\n", ed->description);
				return;
			}
		}
	}

	act_new("OK, $t now belongs to $N.", ch, item->short_descr, player, TO_CHAR, POS_DEAD, FALSE);
	ed                      = new_extra_descr();
	ed->keyword             = str_dup(KEYWD_OWNER);
	ed->description         = str_dup(player->name);
	ed->next                = item->extra_descr;
	item->extra_descr       = ed;
}

void do_peace(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *rch;

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
		if (rch->fighting != NULL)
			stop_fighting(rch, TRUE);

		if (IS_NPC(rch) && IS_SET(rch->act, ACT_AGGRESSIVE))
			REMOVE_BIT(rch->act, ACT_AGGRESSIVE);
	}

	stc("The room is suddenly tranquil.\n", ch);
	return;
}

void do_purge(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	DESCRIPTOR_DATA *d;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		CHAR_DATA *vnext;
		OBJ_DATA  *obj_next;

		for (victim = ch->in_room->people; victim != NULL; victim = vnext) {
			vnext = victim->next_in_room;

			if (IS_NPC(victim)
			    && !IS_SET(victim->act, ACT_NOPURGE)
			    &&  victim != ch)
				extract_char(victim, TRUE);
		}

		for (obj = ch->in_room->contents; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;

			if (!IS_OBJ_STAT(obj, ITEM_NOPURGE))
				extract_obj(obj);
		}

		sprintf(buf, "$N has purged room: %d.", ch->in_room->vnum);
		wiznet(buf, ch, NULL, WIZ_PURGE, WIZ_SECURE, GET_RANK(ch));
		act("$n purges the room of all objects!", ch, NULL, NULL, TO_ROOM);
		stc("The room has been purged.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) == NULL) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		if (ch == victim) {
			stc("'Quit' usually works better.\n", ch);
			return;
		}

		if (!OUTRANKS(ch, victim) && !IS_SET(victim->pcdata->plr, PLR_LINK_DEAD)) {
			sprintf(buf, "$N has tried to purge the immortal: %s", victim->name);
			wiznet(buf, ch, NULL, WIZ_PURGE, WIZ_SECURE, GET_RANK(ch));
			stc("Maybe that wasn't a good idea...\n", ch);
			ptc(victim, "%s just tried to purge you!\n", ch->name);
			return;
		}

		sprintf(buf, "$N has purged the player: %s", victim->name);
		wiznet(buf, ch, NULL, WIZ_PURGE, WIZ_SECURE, GET_RANK(ch));
		act("$n disintegrates $N.", ch, 0, victim, TO_NOTVICT);
		act("You disintegrate $N.", ch, 0, victim, TO_CHAR);

		if (victim->level > 1)
			save_char_obj(victim);

		d = victim->desc;
		extract_char(victim, TRUE);

		if (d != NULL)
			close_socket(d);

		return;
	}

	if (get_mob_here(ch, victim->name, VIS_CHAR) == NULL) {
		stc("There are no mobiles in this room by that name.\n", ch);
		return;
	}

	sprintf(buf, "$N has purged the mobile: %s", victim->short_descr);
	wiznet(buf, ch, NULL, WIZ_PURGE, WIZ_SECURE, GET_RANK(ch));
	act("$n disintegrates $N.", ch, NULL, victim, TO_NOTVICT);
	act("You disintegrate $N.", ch, NULL, victim, TO_CHAR);
	extract_char(victim, TRUE);
}

int has_enough_qps(CHAR_DATA *ch, int number_of)
{
	if (ch->questpoints >= number_of || IS_IMMORTAL(ch))
		return 1;

	return 0;
}

void do_qpconv(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	int number_of = 0, qpcost = 0, what = 0, l = 0;
	char buffer[1024];
	CHAR_DATA *victim;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	strcpy(arg3, argument);

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		stc("Syntax: qpconv <character> <train|prac> <number of>.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == NULL) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		stc("Not on NPC's.\n", ch);
		return;
	}

	number_of = atoi(arg3);

	if (number_of <= 0) {
		stc("Try a number greater then 0.\n", ch);
		return;
	}

	if (number_of > 10) {
		stc("Get real!\n", ch);
		return;
	}

	l = strlen(arg2);

	if (!strncasecmp(arg2, "train", l)) {
		qpcost = number_of * QPS_PER_TRAIN;
		what = 1;
	}

	if (!strncasecmp(arg2, "practice", l)) {
		qpcost = number_of * QPS_PER_PRAC;
		what = 2;
	}

	if (what == 0) {
		stc("{x I don't understand what you are trying to convert.. use 'train' or 'practice'.\n", ch);
		return;
	}

	if (has_enough_qps(victim, qpcost) == 0) {
		sprintf(buffer, "{x%s doesn't seem to have enough the required %d questpoints.\n", victim->name, qpcost);
		stc(buffer, ch);
		return;
	}

	if (!IS_IMMORTAL(ch))
		victim->questpoints -= qpcost;

	switch (what) {
	case 1:
		victim->train += number_of;
		break;

	case 2:
		victim->practice += number_of;
		break;
	}

	sprintf(buffer, "{x%d questpoints deducted, %d %s%sadded.\n", qpcost, number_of, (what == 1) ? "train" : "practice",
	        (number_of) > 1 ? "s " : " ");
	stc(buffer, victim);
	stc("{xConversion was successful.\n", ch);
	return;
}

void restore_char(CHAR_DATA *ch, CHAR_DATA *victim)
{
	affect_remove_sn_from_char(victim, gsn_plague);
	affect_remove_sn_from_char(victim, gsn_poison);
	affect_remove_sn_from_char(victim, gsn_blindness);
	affect_remove_sn_from_char(victim, gsn_dirt_kicking);
	affect_remove_sn_from_char(victim, gsn_fire_breath);
	affect_remove_sn_from_char(victim, gsn_sleep);
	affect_remove_sn_from_char(victim, gsn_curse);
	affect_remove_sn_from_char(victim, gsn_fear);
	victim->hit     = GET_MAX_HIT(victim);
	victim->mana    = GET_MAX_MANA(victim);
	victim->stam    = GET_MAX_STAM(victim);
	update_pos(victim);
	act_new("$n has restored you.", ch, NULL, victim, TO_VICT, POS_SLEEPING, FALSE);
}

void do_restore(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;
	one_argument(argument, arg);

	if (arg[0] == '\0' || !str_cmp(arg, "room")) {
		/* cure room */
		for (victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room)
			restore_char(ch, victim);

		sprintf(buf, "$N has restored room: %d.", ch->in_room->vnum);
		wiznet(buf, ch, NULL, WIZ_RESTORE, WIZ_SECURE, GET_RANK(ch));
		stc("Room restored.\n", ch);
		return;
	}

	if (!str_cmp(arg, "all")) {
		/* cure all */
		for (d = descriptor_list; d != NULL; d = d->next)
			if (d->character != NULL && !IS_NPC(d->character))
				restore_char(ch, d->character);

		stc("All players restored.\n", ch);
		wiznet("$N has restored all.", ch, NULL, WIZ_RESTORE, WIZ_SECURE, GET_RANK(ch));
		return;
	}

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) == NULL) {
		stc("They aren't here.\n", ch);
		return;
	}

	restore_char(ch, victim);
	sprintf(buf, "$N has restored: %s", IS_NPC(victim) ? victim->short_descr : victim->name);
	wiznet(buf, ch, NULL, WIZ_RESTORE, WIZ_SECURE, GET_RANK(ch));
	stc("The player has been restored.\n", ch);
}

/* Secure levels by Lotus */
void do_secure(CHAR_DATA *ch, const char *argument)
{
	char arg[MIL];
	one_argument(argument, arg);

	if (arg[0] == '\0' || !str_prefix1(arg, "immortal")) {
		ch->secure_level = RANK_IMM;
		stc("All immortals can see your immtalks now.\n", ch);
	}
	else if (!str_prefix1(arg, "head") || !str_prefix1(arg, "deputy")) {
		ch->secure_level = RANK_HEAD;
		stc("Only heads of departments and implementors can see your immtalks now.\n", ch);
	}
	else if (!str_prefix1(arg, "implementor") && IS_IMP(ch)) {
		ch->secure_level = RANK_IMP;
		stc("Only implementors can see your immtalks now.\n", ch);
	}
	else {
		stc("Syntax:\n"
		    "  secure imm\n"
		    "  secure head\n", ch);

		if (IS_IMP(ch))
			stc("  secure imp\n", ch);
	}
}

/* The workhorse coding of do_setgamein() and do_setgameout() */
void setgameinout(CHAR_DATA *ch, const char *argument, char *entryexit, char flag)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	char **msgptr;
	argument = one_argument(argument, arg);

	if (! arg[0]) {
		act("Set game $t message for whom?", ch, entryexit, NULL, TO_CHAR);
		return;
	}

	victim = get_player_world(ch, arg, VIS_PLR);

	if (victim == NULL) {
		act("No player named '$t' found in the game!", ch, arg, NULL, TO_CHAR);
		return;
	}

	if (flag == 'I')
		msgptr = &victim->pcdata->gamein;
	else
		msgptr = &victim->pcdata->gameout;

	if (!argument || !argument[0]) {    /* clear flag */
		free_string(*msgptr);
		*msgptr = NULL;
		act("OK, $N no longer has a game $t string",
		    ch, entryexit, victim, TO_CHAR);
	}
	else if (color_strlen(argument) > 70) {
		act("This message is too long. Please try to think of something shorter!",
		    ch, NULL, NULL, TO_CHAR);
		stc("THIS is how long the message is allowed to be:\n", ch);
		act("setgamein $N -----------------------------------"
		    "-----------------------------------", ch, NULL, victim, TO_CHAR);
		return;
	}
	else {
		*msgptr = str_dup(argument);
		act("OK, game $t message set", ch, entryexit, NULL, TO_CHAR);
	}
} /* end setgameinout() */

/* Set the game entry message for a player -- Elrac */
void do_setgamein(CHAR_DATA *ch, const char *argument)
{
	setgameinout(ch, argument, "entry", 'I');
}

/* Set the game exit message for a player -- Elrac */
void do_setgameout(CHAR_DATA *ch, const char *argument)
{
	setgameinout(ch, argument, "exit", 'O');
}

void do_sockets(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d, *dmult;
	CHAR_DATA *vch;
	PC_DATA *vpc, *vpc_next;
	BUFFER *buffer = new_buf();
	char arg[MAX_INPUT_LENGTH];
	char status[MAX_STRING_LENGTH];
	char s[100];
	bool multiplay = FALSE;
	int count = 0, ldcount = 0;
	one_argument(argument, arg);
	add_buf(buffer, "\n{PNum{x|{YConnected_State{x| {BLogin{x |{CIdl{x|{GPlayer  Name{x|{WHost{x\n");
	add_buf(buffer, "---|---------------|-------|---|------------|-------------------------\n");

	for (d = descriptor_list; d != NULL; d = d->next) {
		switch (d->connected) {
		case CON_PLAYING:               strcpy(status, "    Playing    "); break;

		case CON_GET_NAME:              strcpy(status, "   Ask Name    "); break;

		case CON_GET_OLD_PASSWORD:      strcpy(status, " Ask Password  "); break;

		case CON_CONFIRM_NEW_NAME:      strcpy(status, " Confirm Name  "); break;

		case CON_GET_MUD_EXP:           strcpy(status, "    Ask Exp    "); break;

		case CON_GET_NEW_PASSWORD:      strcpy(status, " Ask New Pass  "); break;

		case CON_CONFIRM_NEW_PASSWORD:  strcpy(status, " Confirm Pass  "); break;

		case CON_GET_NEW_RACE:          strcpy(status, "   Ask Race    "); break;

		case CON_ROLL_STATS:            strcpy(status, "  Roll Stats   "); break;

		case CON_GET_NEW_SEX:           strcpy(status, "    Ask Sex    "); break;

		case CON_GET_NEW_CLASS:         strcpy(status, "   Ask Class   "); break;

		case CON_GET_ALIGNMENT:         strcpy(status, "   Ask Align   "); break;

		case CON_DEITY:                 strcpy(status, "   Ask Deity   "); break;

		case CON_DEFAULT_CHOICE:        strcpy(status, " Ask Customize "); break;

		case CON_GEN_GROUPS:            strcpy(status, "  Customizing  "); break;

		case CON_PICK_WEAPON:           strcpy(status, "  Pick Weapon  "); break;

		case CON_READ_IMOTD:            strcpy(status, "     Imotd     "); break;

		case CON_READ_MOTD:             strcpy(status, "     Motd      "); break;

		case CON_READ_NEWMOTD:          strcpy(status, "   New Motd    "); break;

		case CON_BREAK_CONNECT:         strcpy(status, " Break Connect "); break;

		case CON_COPYOVER_RECOVER:      strcpy(status, "   Copyover    "); break;

		case CON_GREETING:              strcpy(status, "   Greeting    "); break;

		default:                        strcpy(status, "    Unknown    "); break;
		}

		if (d->character == NULL) {
			/* no character known -- show it in SOCKETS anyway */
			ptb(buffer, "{P%3d{x|{Y%s{x|       |{C%2d{x |{G%-12s{x|{W%s{x\n",
			    d->descriptor,
			    status,
			    UMAX(0, d->timer),
			    d->original  ? d->original->name  :
			    d->character ? d->character->name : "(none)",
			    d->host);
		}
		else if (can_see_char(ch, d->character)
		         && (arg[0] == '\0'
		             || is_name(arg, d->character->name)
		             || (d->original && is_name(arg, d->original->name)))) {
			count++;
			/* check for multiplayers -- Montrey */
			multiplay = FALSE;

			if (d->connected == 0)
				for (dmult = descriptor_list; dmult != NULL; dmult = dmult->next) {
					if (dmult == d || dmult->connected != 0) /* if not playing */
						continue;

					if (!str_cmp(dmult->host, d->host))
						multiplay = TRUE;
				}

			/* Format "login" value... */
			vch = d->original ? d->original : d->character;
			strftime(s, 100, "%I:%M%p", localtime(&vch->logon));
			ptb(buffer, "{P%3d{x|{Y%s{x|{B%7s{x|{C%2d{x |{G%-12s{x%s%s{x\n",
			    d->descriptor,
			    status,
			    s,
			    UMAX(0, d->timer),
			    d->original ? d->original->name  :
			    d->character ? d->character->name : "(none)",
			    multiplay ? "|{P" : "|{W",
			    d->host);
		}
	}

	add_buf(buffer, "---|---------------|-------|---|------------|-------------------------\n");

	/* now list linkdead ppl */
	for (vpc = pc_list; vpc != NULL; vpc = vpc_next) {
		vpc_next = vpc->next;

		if (vpc->ch != ch
		    && IS_SET(vpc->plr, PLR_LINK_DEAD)
		    && can_see_char(ch, vpc->ch)
		    && (arg[0] == '\0'
		        || is_name(arg, vpc->ch->name))) {
			strftime(s, 100, "%I:%M%p", localtime(&vpc->ch->logon));
			ptb(buffer, "{P---{x|{Y   Linkdead    {x|{B%7s{x|{C%-2d{x |{G%-12s{x|{W%s{x\n",
			    s,
			    UMAX(0, vpc->ch->desc == NULL ? vpc->ch->timer : vpc->ch->desc->timer),
			    vpc->ch->name,
			    vpc->last_lsite);
			ldcount++;
		}
	}

	if (ldcount)
		add_buf(buffer, "---|---------------|-------|---|------------|-------------------------\n");

	if (!count && !ldcount) {
		if (arg[0] == '\0')
			stc("No one is connected...\n", ch);
		else
			stc("No one by that name is connected.\n", ch);

		free_buf(buffer);
		return;
	}

	ptb(buffer, "%d user%s connected", count, count == 1 ? "" : "s");

	if (ldcount)
		ptb(buffer, ", %d user%s linkdead.\n", ldcount, ldcount == 1 ? "" : "s");
	else
		add_buf(buffer, ".\n");

	page_to_char(buf_string(buffer), ch);
	free_buf(buffer);
}

void do_storage(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	STORAGE_DATA *i;

	if (argument[0] == '\0') {
		stc("Syntax:\n"
		    "  storage list                 (lists all characters in storage)\n"
		    "  storage store <player>       (puts a character in storage)\n"
		    "  storage retrieve <player>    (retrieves a character from storage)\n", ch);
		return;
	}

	argument = one_argument(argument, arg1); /* storage command */

	if (!str_cmp(arg1, "list")) {
		stc("Currently in storage are:\n"
		    "\n"
		    "name                |by who              |date\n"
		    "--------------------+--------------------+------------------------\n", ch);
		i = storage_list_head->next;

		while (i != storage_list_tail) {
			ptc(ch, "%20s|%20s|%s\n", i->name,
			    i->by_who,
			    i->date);
			i = i->next;
		}

		return;
	}

	if (!str_cmp(arg1, "store")) {
		char pfile_buf[128];
		char storage_buf[128];
		char command_buf[255];
		FILE *fp;
		STORAGE_DATA *newdata;

		if (argument[0] == '\0') {
			stc("Store who?\n", ch);
			return;
		}

		if (lookup_storage_data(argument) != NULL) {
			stc("That character is already in storage!\n", ch);
			return;
		}

		sprintf(pfile_buf, "%s%s", PLAYER_DIR, capitalize(argument));
		sprintf(storage_buf, "%s%s", STORAGE_DIR, capitalize(argument));

		if ((fp = fopen(pfile_buf, "r")) == NULL) {
			stc("No such character!\n", ch);
			return;
		}

		fclose(fp);
		sprintf(command_buf, "mv %s %s", pfile_buf, storage_buf);

		if (system(command_buf) != 0) {
			ptc(ch, "Error trying to move %s into storage!.\n", argument);
			return;
		}

		newdata = (STORAGE_DATA *)alloc_mem(sizeof(STORAGE_DATA));
		newdata->name = str_dup(capitalize(argument));
		newdata->by_who = str_dup(ch->name);
		newdata->date = str_dup(ctime(&current_time));
		newdata->date[strlen(newdata->date) - 1] = '\0';
		insert_storagedata(newdata);
		save_storage_list();
		ptc(ch, "Character %s successfully put into storage.\n", capitalize(argument));
		return;
	}

	if (!str_cmp(arg1, "retrieve")) {
		char pfile_buf[128];
		char storage_buf[128];
		char command_buf[255];
		STORAGE_DATA *olddata;

		if (argument[0] == '\0') {
			stc("Store who?\n", ch);
			return;
		}

		olddata = lookup_storage_data(argument);

		if (olddata == NULL) {
			stc("No such character is currently in storage!\n", ch);
			return;
		}

		sprintf(pfile_buf, "%s%s", PLAYER_DIR, capitalize(argument));
		sprintf(storage_buf, "%s%s", STORAGE_DIR, capitalize(argument));
		sprintf(command_buf, "mv %s %s", storage_buf, pfile_buf);

		if (system(command_buf) == 0) {
			ptc(ch, "%s has successfully been removed from storage.\n", capitalize(argument));
			remove_storagedata(olddata);
			save_storage_list();
		}
		else
			ptc(ch, "Error while trying to retrieve '%s'.\n", capitalize(argument));

		return;
	}

	do_storage(ch, "");
}

void do_invis(CHAR_DATA *ch, const char *argument)
{
	/* take the default path */
	if (ch->invis_level) {
		ch->invis_level = 0;
		act("$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM);
		stc("You slowly fade back into existence.\n", ch);
	}
	else {
		ch->invis_level = LEVEL_IMMORTAL;
		act("$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM);
		stc("You slowly vanish into thin air.\n", ch);
	}
}

void do_superwiz(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->act, PLR_SUPERWIZ)) {
		stc("You return to reality.\n", ch);
		REMOVE_BIT(ch->act, PLR_SUPERWIZ);
	}
	else {
		stc("You vanish from existence.\n", ch);
		SET_BIT(ch->act, PLR_SUPERWIZ);
	}
}

ROOM_INDEX_DATA *find_location(CHAR_DATA *ch, const char *arg)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	if (is_number(arg))
		return get_room_index(atoi(arg));

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) != NULL)
		return victim->in_room;

	if ((obj = get_obj_world(ch, arg)) != NULL)
		return obj->in_room;

	return NULL;
}

void do_transfer(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0') {
		stc("Transfer whom (and where)?\n", ch);
		return;
	}

	if (!str_cmp(arg1, "all")) {
		for (d = descriptor_list; d != NULL; d = d->next) {
			if (IS_PLAYING(d)
			    &&   d->character != ch
			    &&   d->character->in_room != NULL
			    &&   can_see_char(ch, d->character)) {
				char buf[MAX_STRING_LENGTH];
				sprintf(buf, "%s %s", d->character->name, arg2);
				do_transfer(ch, buf);
			}
		}

		return;
	}

	/*
	 * Thanks to Grodyn for the optional location parameter.
	 */
	if (arg2[0] == '\0')
		location = ch->in_room;
	else {
		if ((location = find_location(ch, arg2)) == NULL) {
			stc("No such location.\n", ch);
			return;
		}

		if (!is_room_owner(ch, location) && room_is_private(location)
		    &&  !IS_IMP(ch)) {
			stc("That room is private.\n", ch);
			return;
		}
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == NULL) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (!IS_NPC(victim) && IS_IMMORTAL(victim) && !IS_IMP(ch)) {
		stc("They wouldn't like that.\n", ch);
		return;
	}

	if (victim->in_room == NULL) {
		stc("They are in limbo.\n", ch);
		return;
	}

	if (victim->fighting != NULL)
		stop_fighting(victim, TRUE);

	act("$n screams as $e is sucked off into the clouds.", victim, NULL, NULL, TO_ROOM);
	char_from_room(victim);
	char_to_room(victim, location);
	act("$n breaks through the clouds and crash lands at your feet.", victim, NULL, NULL, TO_ROOM);

	if (ch != victim)
		ptc(victim, "%s has transported you.\n", PERS(ch, victim, VIS_CHAR).capitalize());

	do_look(victim, "auto");
	stc("Transfer Successful.\n", ch);
}

void do_violate(CHAR_DATA *ch, const char *argument)
{
	char arg[MIL];
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *location = NULL;
	CHAR_DATA *rch;

	if (argument[0] == '\0') {
		stc("Which private room do you wish to violate?\n", ch);
		return;
	}

	one_argument(argument, arg);

	if (is_number(arg))
		location = get_room_index(atoi(arg));
	else {
		if ((rch = get_char_world(ch, argument, VIS_CHAR)) != NULL)
			location = rch->in_room;

		if (!str_prefix1(arg, "object") || location == NULL) {
			location = NULL;

			if ((obj = get_obj_world(ch, argument)) != NULL)
				if (obj->in_room)
					location = obj->in_room;
		}
	}

	if (location == NULL) {
		stc("No such location.\n", ch);
		return;
	}

	if (!room_is_private(location)) {
		stc("That room isn't private, use Goto.\n", ch);
		return;
	}

	if (ch->fighting != NULL)
		stop_fighting(ch, TRUE);

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
		if (can_see_char(rch, ch)) {
			if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
				act("$t", ch, ch->pcdata->bamfout, rch, TO_VICT);
			else
				act("$n warps off to distant lands in a burst of electric energy.", ch, NULL, rch, TO_VICT);
		}
	}

	char_from_room(ch);
	char_to_room(ch, location);

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
		if (can_see_char(rch, ch)) {
			if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
				act("$t", ch, ch->pcdata->bamfin, rch, TO_VICT);
			else
				act("$n materializes in a burst of electric energy.", ch, NULL, rch, TO_VICT);
		}
	}

	do_look(ch, "auto");
	return;
}

/* Command groups - Command to give/take */
void do_wizgroup(CHAR_DATA *ch, const char *argument)
{
	char arg1[MIL], arg2[MIL], arg3[MIL];
	CHAR_DATA *victim;
	int count = 0;
	bool add = FALSE, all = FALSE, found = FALSE;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		stc("wizgroup <add/remove> <player> <group>\n", ch);
		stc("Valid groups are:\n", ch);
		stc("  general quest build code security\n"
		    "  player clan avatar hero leader\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg2, VIS_PLR)) == NULL) {
		stc("Player not found.\n", ch);
		return;
	}

	if (!str_prefix1(arg1, "add"))
		add = TRUE;
	else if (!str_prefix1(arg1, "remove"))
		add = FALSE;
	else {
		do_wizgroup(ch, "");
		return;
	}

	if (!str_cmp(arg3, "all")) {
		stc("The `all' option is no longer enabled.\n", ch);
		return;
	}

	/* loops == good!  else ifs == bad! :) */
	while (cgroup_flags[count].name != NULL) {
		if (!str_prefix1(arg3, cgroup_flags[count].name) || all) {
			found = TRUE;

			if (add)
				SET_BIT(victim->pcdata->cgroup, cgroup_flags[count].bit);
			else
				REMOVE_BIT(victim->pcdata->cgroup, cgroup_flags[count].bit);

			ptc(ch, "%s group %sed for %s.\n",
			    cgroup_flags[count].name, add ? "add" : "remov", victim->name);

			if (!all)
				break;
		}

		count++;
	}

	if (!found)
		do_wizgroup(ch, "");
}

/* Wizify by Pwrdemon */
void do_wizify(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char strsave[MAX_INPUT_LENGTH];
	FILE *fp;
	CHAR_DATA *victim;
	int sn;
	one_argument(argument, arg1);

	if (arg1[0] == '\0') {
		stc("You must provide a name.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == NULL) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		stc("This command can only be used on PC's.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		stc("They are already an immortal.\n", ch);
		return;
	}

	/* Backup their pfile to BACKUP_DIR/pfileGOD.gz */
	sprintf(strsave, "%s%s%s", BACKUP_DIR, victim->name.capitalize(), "GOD.gz");

	if ((fp = fopen(strsave, "r")) != NULL)
		fclose(fp);
	else {
		backup_char_obj(ch);
		stc("Your pfile has been backed up.\n", victim);
		String capname = victim->name.capitalize();
		sprintf(strsave, "mv %s%s.gz %s%sGOD.gz",
		        BACKUP_DIR, capname, BACKUP_DIR, capname);
		system(strsave);
	}

	victim->level                   = MAX_LEVEL;
	victim->hit  = ATTR_BASE(victim, APPLY_HIT)        = 30000;
	victim->mana = ATTR_BASE(victim, APPLY_MANA)       = 30000;
	victim->stam = ATTR_BASE(victim, APPLY_STAM)       = 30000;

	for (int stat = 0; stat < MAX_STATS; stat++)
		ATTR_BASE(victim, stat_to_attr(stat)) = 25;

	for (sn = 0; sn < MAX_SKILL; sn++)
		if (skill_table[sn].name != NULL)
			victim->pcdata->learned[sn] = 100;

	REM_CGROUP(victim, GROUP_LEADER);
	REM_CGROUP(victim, GROUP_DEPUTY);
	victim->clan = NULL;
	update_pos(victim);
	stc("You have created a god.\n", ch);
	stc("You suddenly feel like a GOD!\n", victim);
	save_char_obj(victim);
}

/* Aura command stolen from rank - Lotus */
void do_aura(CHAR_DATA *ch, const char *argument)
{
	char arg1[MIL];
	CHAR_DATA *victim;
	argument = one_argument(argument, arg1);

	if (IS_NPC(ch)) {
		do_huh(ch);
		return;
	}

	if (arg1[0] == '\0') {
		if (ch->pcdata->aura[0] == '\0')
			stc("You have no aura set.\n", ch);
		else
			ptc(ch, "Your aura is currently {W(%s{W){x.\n", ch->pcdata->aura);

		return;
	}

	if (!IS_IMMORTAL(ch)) {
		stc("Only Imms can set your aura.\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg1, VIS_PLR)) == NULL) {
		stc("That player is not logged on.\n", ch);
		return;
	}

	if (argument[0] == '\0') {
		ptc(ch, "Their aura is currently {W(%s{W){x.\n", victim->pcdata->aura);
		return;
	}

	if (!str_cmp(argument, "none")) {
		stc("Aura removed.\n", ch);
		stc("Your aura has been removed.\n", victim);
		free_string(victim->pcdata->aura);
		victim->pcdata->aura = str_dup("");
		return;
	}

	if (color_strlen(argument) > 30) {
		stc("An aura cannot be longer than 30 printed characters.\n", ch);
		return;
	}

	if (strlen(argument) > 100) {
		stc("An aura cannot be longer than 100 total characters.\n", ch);
		return;
	}

	free_string(victim->pcdata->aura);
	victim->pcdata->aura = str_dup(argument);
	ptc(victim, "Your aura is now {W(%s{W){x.\n", victim->pcdata->aura);
	ptc(ch, "Their aura is now {W(%s{W){x.\n", victim->pcdata->aura);
}

void do_bamfin(CHAR_DATA *ch, const char *argument)
{
	if (argument[0] == '\0') {
		ptc(ch, "Your poofin is currently: %s\n", ch->pcdata->bamfin);
		return;
	}

	if (!is_exact_name_color(ch->name, argument)) {
		stc("You must include your name in a poofin.\n", ch);
		return;
	}

	if (strlen(argument) > 100) {
		stc("Poofins cannot be longer than 100 printed characters.\n", ch);
		return;
	}

	if (color_strlen(argument) > 300) {
		stc("Poofins cannot be longer than 300 characters, counting color codes.\n", ch);
		return;
	}

	free_string(ch->pcdata->bamfin);
	ch->pcdata->bamfin = str_dup(argument);
	ptc(ch, "Your poofin is now: %s\n", ch->pcdata->bamfin);
}

void do_bamfout(CHAR_DATA *ch, const char *argument)
{
	if (argument[0] == '\0') {
		ptc(ch, "Your poofout is currently: %s\n", ch->pcdata->bamfout);
		return;
	}

	if (!is_exact_name_color(ch->name, argument)) {
		stc("You must include your name in a poofout.\n", ch);
		return;
	}

	if (strlen(argument) > 100) {
		stc("Poofouts cannot be longer than 100 printed characters.\n", ch);
		return;
	}

	if (color_strlen(argument) > 300) {
		stc("Poofouts cannot be longer than 300 characters, counting color codes.\n", ch);
		return;
	}

	free_string(ch->pcdata->bamfout);
	ch->pcdata->bamfout = str_dup(argument);
	ptc(ch, "Your poofout is now: %s\n", ch->pcdata->bamfout);
}

void do_clanqp(CHAR_DATA *ch, const char *argument)
{
	char arg1[MIL], arg2[MIL], arg3[MIL], buf[MSL];
	CLAN_DATA *target;
	int qp_amount = 0;

	if (argument[0] == '\0') {
		do_claninfo(ch, argument);
		return;
	}

	if (!IS_IMMORTAL(ch)) {
		if (!ch->clan) {
			stc("You're not even in a clan!\n", ch);
			return;
		}

		if (ch->clan->independent) {
			stc("Your clan doesn't support this feature!\n", ch);
			return;
		}
	}

	if (argument[0] == '\0') {
		stc("See 'help clanqp' on how this command works!\n", ch);
		return;
	}

	argument = one_argument(argument, arg1); /* donate/withdraw     */
	argument = one_argument(argument, arg2); /* qp_amount           */
	argument = one_argument(argument, arg3); /* clanname            */

	if (!IS_IMMORTAL(ch))
		target = ch->clan;
	else
		target = (arg3[0] == '\0' ? ch->clan : clan_lookup(arg3));

	if (!target) {
		stc("There is no such clan.\n", ch);
		return;
	}

	if (target->independent) {
		stc("You can't donate questpoints to that clan!\n", ch);
		return;
	}

	if (!is_number(arg2)) {
		stc("You must specify an amount of questpoints.\n", ch);
		return;
	}

	if ((qp_amount = atoi(arg2)) < 1 || qp_amount > 30000) {
		stc("The number of questpoints must be between 1 and 30000.\n", ch);
		return;
	}

	if (!str_prefix1(arg1, "donate")) {
		if (!IS_IMMORTAL(ch)) {
			if (has_enough_qps(ch, qp_amount) == 0) {
				stc("Please try again when you have enough questpoints!\n", ch);
				return;
			}

			ch->questpoints -= qp_amount;
			ch->questpoints_donated += qp_amount;
		}

		target->clanqp += qp_amount;
		save_clan_table();
		save_char_obj(ch);

		if (ch->clan != target)
			ptc(ch, "The %s{x is now %d questpoint%s richer!\n",
			    target->clanname, qp_amount, qp_amount > 1 ? "s" : "");
		else {
			sprintf(buf, "{W[ %s just donated %d questpoint%s to the clan! ]{x\n",
			        ch->name, qp_amount, qp_amount > 1 ? "s" : "");
			send_to_clan(ch, target, buf);
			ptc(ch, "You donate %d questpoint%s to the clan.\n",
			    qp_amount, qp_amount > 1 ? "s" : "");
		}

		return;
	}

	if (!str_prefix1(arg1, "withdraw") || !str_prefix1(arg1, "deduct")) {
		if (!IS_IMMORTAL(ch)) {
			stc("Only immortals are allowed to do that!\n", ch);
			return;
		}

		if (argument[0] == '\0') {
			stc("You have to provide a reason!\n", ch);
			return;
		}

		if (target->clanqp < qp_amount) {
			stc("The clan is not rich enough!\n", ch);
			return;
		}

		target->clanqp -= qp_amount;
		sprintf(buf, "{W[ %s has deducted %d questpoint%s for %s ]\n",
		        ch->name, qp_amount, qp_amount > 1 ? "s" : "", argument);
		send_to_clan(ch, target, buf);
		ptc(ch, "You deduct %d questpoints from the %s{x.\n", qp_amount, target->clanname);
		save_clan_table();
		return;
	}

	stc("See 'help clanqp' on how this command works!\n", ch);
}

/*
This command is used by IMMs to grant or revoke a single
command. The command is added or removed from the granted_commands
list in pc_data.
-- Outsider
*/
void do_grant(CHAR_DATA *ch, const char *argument)
{
	char grant_remove[MSL];
	char player_name[MSL];
	CHAR_DATA *victim;
	int grant_index;
	bool successful = FALSE;
	argument = one_argument(argument, player_name);
	/* try to find targeted player */
	victim = get_char_world(ch, player_name, VIS_CHAR);

	if (! victim) {
		stc("Cannot find that player.\n", ch);
		return;
	}

	argument = one_argument(argument, grant_remove);
	/* argument now holds the command to grant/remove */

	/* make sure we have a command to add/remove */
	if (! argument[0]) {
		stc("usage: grant <player> <add/remove> <command>\n", ch);
		return;
	}

	/* take away granted command */
	if (! strcmp(grant_remove, "remove")) {
		/* go through the granted commands */
		grant_index = 0;

		while ((grant_index < MAX_GRANT) && (! successful)) {
			if (! strcmp(argument, victim->pcdata->granted_commands[grant_index]))
				successful = TRUE;
			else
				grant_index++;
		}

		if (successful) {
			victim->pcdata->granted_commands[grant_index][0] = '\0';
			stc("Command removed.\n", ch);
		}
		else
			stc("Command not found in target's list.\n", ch);
	}   /* end of remove */
	/* add a new command to the player's granted list */
	else if (! strcmp(grant_remove, "add")) {
		/* look for an empty command slot */
		grant_index = 0;

		while ((grant_index < MAX_GRANT) && (! successful)) {
			if (victim->pcdata->granted_commands[grant_index][0])
				grant_index++;
			else
				successful = TRUE;
		}

		if (successful) {
			strcpy(victim->pcdata->granted_commands[grant_index], argument);
			stc("Command granted.\n", ch);
		}
		else
			stc("Player already all the granted commands possible.\n", ch);
	}     /* end of add */
	else
		stc("usage: grant <player> <add/remove> <command>\n", ch);
}

/*
This function displays the specially granted commands issued
to the player. If used by an IMM, the function can also
display granted commands issued to other players.
-- Outsider
*/
void do_grantlist(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	int count;

	if (IS_NPC(ch))
		return;

	if (IS_IMMORTAL(ch)) {
		if (! argument[0]) {  /* do on self */
			stc("Your granted commands are:\n", ch);

			for (count = 0; count < MAX_GRANT; count++) {
				if (ch->pcdata->granted_commands[count][0]) {
					stc("  ", ch);
					stc(ch->pcdata->granted_commands[count], ch);
					stc("\n", ch);
				}
			}    /* end of for loop */

			return;
		}       /* end of perform command on self */

		/* Try to find granted commands on another player. */
		victim = get_char_world(ch, argument, VIS_CHAR);

		if (! victim) {
			stc("Cannot find that character.\n", ch);
			return;
		}

		if (IS_NPC(victim)) {
			stc("NPCs cannot be granted commands.\n", ch);
			return;
		}

		stc("Their granted commands are:\n", ch);

		for (count = 0; count < MAX_GRANT; count++) {
			if (victim->pcdata->granted_commands[count][0]) {
				stc("  ", ch);
				stc(victim->pcdata->granted_commands[count], ch);
				stc("\n", ch);
			}
		}  /* end of for loop */

		return;
	}          /* end of is immortal */

	/* now into the mortal relm */
	/* only display granted commands for self */
	stc("Your granted commands are:\n", ch);

	for (count = 0; count < MAX_GRANT; count++) {
		if (ch->pcdata->granted_commands[count][0]) {
			stc("  ", ch);
			stc(ch->pcdata->granted_commands[count], ch);
			stc("\n", ch);
		}
	}   /* end of for loop */
}             /* end of do_grantlist function */

