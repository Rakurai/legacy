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

#include "Game.hh"
#include "Area.hh"
#include "find.hh"
#include "typename.hh"
#include "channels.hh"
#include "merc.hh"
#include "interp.hh"
#include "recycle.hh"
#include "lookup.hh"
#include "tables.hh"
#include "sql.hh"
#include "Affect.hh"
#include "memory.hh"
#include "Format.hh"
#include "GameTime.hh"
#include "StoredPlayer.hh"


extern bool    swearcheck              args((const String& argument));
extern bool check_parse_name(const String& name);
extern  RoomPrototype *room_index_hash[MAX_KEY_HASH];

void do_adjust(Character *ch, String argument)
{
	Character *victim;
	char       buf  [ MAX_STRING_LENGTH ];
	int      value;

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1.empty() || arg2.empty() || !arg2.is_number()) {
		stc("Syntax: Adjust <char> <Exp>.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == nullptr) {
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
	Format::sprintf(buf, "You have adjusted the exp of %s by %d experience points.\n",
	        victim->name, value);
	stc(buf, ch);

	if (value > 0) {
		Format::sprintf(buf, "You have been bonused %d experience points.\n", value);
		stc(buf, victim);
	}
	else {
		Format::sprintf(buf, "You have been penalized %d experience points.\n", value);
		stc(buf, victim);
	}

	return;
}

/* save all players, without lag -- Elrac */
void do_allsave(Character *ch, String argument)
{
	Descriptor *d;
	Character *wch;
	stc("Saving all players... ", ch);

	for (d = descriptor_list; d; d = d->next) {
		wch = (d->original ? d->original : d->character);
		save_char_obj(wch);
	}

	stc("done.\n", ch);
} /* end allsave */

void do_alternate(Character *ch, String argument)
{
	String query;
	char colorsite[MSL], *p, *q;
	String output;
	int sorted_count = 0, i;
	struct alts {
		char name[20];
		char site[100];
		bool printed;
	};
	struct alts results_sorted[500];

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	one_argument(argument, arg2);

	if (arg1.empty() || (arg1 == "site" && arg2.empty())) {
		stc("Syntax:\n"
		    "  alternate <{Yplayer name{x>\n"
		    "  alternate site <{Ysite{x>\n"
		    "    {c(ex. *.tstc.edu, or 208.238.145.242){x\n", ch);
		return;
	}

	if (arg1 == "site") {
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
		query = "SELECT name, site, (DATE() - lastlog) FROM sites "
		       "WHERE site LIKE '";

		if (*p == '*') {
			prefix = TRUE;
			p++;
		}

		for (q = p; * (q + 1); q++);

		if (*q == '*') {
			suffix = TRUE;
			*q = '\0';
		}

		Format::sprintf(colorsite, "{Y%s{W", p);

		if (prefix)
			query += "\%";

		query += p;

		if (suffix)
			query += "\%";

		query += "' ORDER BY name, (DATE() - lastlog)";
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
				String(db_get_column_str(1)).replace(p, colorsite));
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
			Format::sprintf(query, "SELECT name, site FROM sites WHERE ");

			if (!old_char)
				query += "lastlog >= DATE('now', '-1 year') AND ";

			query += "ssite='%s'";

			if (db_queryf("do_alternate", query, db_esc(sitelist[i].ssite)) != SQL_OK)
				return;

			Format::sprintf(colorsite, "{Y%s{W", sitelist[i].ssite);

			while (db_next_row() == SQL_OK) {
				String name = db_get_column_str(0);

				if (name == arg1) {
					if (sorted_count >= 500) {
						bug("do_alternate: WARNING: maximum sorted structure size reached", 0);
						break;
					}

					strcpy(results_sorted[sorted_count].name, name);
					strcpy(results_sorted[sorted_count].site,
						String(db_get_column_str(1)).replace(sitelist[i].ssite, colorsite));
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
						String(db_get_column_str(1)).replace(sitelist[i].ssite, colorsite));
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
					else if (results_to_sort[curname].name != results_to_sort[i].name)
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

	output += "{GMatching characters/sites:{x\n";
	output += "{G=================================================================={x\n";

	char fstr[25];
	strcpy(fstr, "{G[{W%12s%s%54s{G]{x\n");
	output += Format::format(fstr, results_sorted[0].name, " {G-{W", results_sorted[0].site);

	for (i = 1; i < sorted_count; i++)
		output += Format::format(fstr,
		    results_sorted[i].name != results_sorted[i - 1].name ? results_sorted[i].name : "",
		    results_sorted[i].name != results_sorted[i - 1].name ? " {G-{W" : "  ",
		    results_sorted[i].site);

	output += "{G=================================================================={x\n";
	page_to_char(output, ch);
}

void do_at(Character *ch, String argument)
{
	RoomPrototype *location;
	RoomPrototype *original;
	Object *on;
	Character *wch;

	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty() || argument.empty()) {
		stc("At where and what?\n", ch);
		return;
	}

	if ((location = find_location(ch, arg)) == nullptr) {
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
	for (wch = char_list; wch != nullptr; wch = wch->next) {
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
void do_check(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	bool SHOWIMM = FALSE;
	String buffer;
	Character *victim;

	String arg;
	argument = one_argument(argument, arg);

	if (arg == "gods" || argument == "gods")
		SHOWIMM = TRUE;

	if (arg.empty() || arg.is_prefix_of("gods")) {

		for (victim = char_list; victim != nullptr; victim = victim->next) {
			if (IS_NPC(victim) || !can_see_char(ch, victim))
				continue;

			if (!SHOWIMM && IS_IMMORTAL(victim))
				continue;

			Format::sprintf(buf, "{W[%12s] Level {C%3d{W connected since %d hours {C(%d total hours){x\n",
			        victim->name, victim->level,
			        ((int)(current_time - victim->logon)) / 3600,
			        (get_play_seconds(victim) + (int)(current_time - victim->logon)) / 3600);
			buffer += buf;
		}

		page_to_char(buffer, ch);
		return;
	}

	if (arg.is_prefix_of("stats")) {

		for (victim = char_list; victim != nullptr; victim = victim->next) {
			if (IS_NPC(victim) || !can_see_char(ch, victim))
				continue;

			if (!SHOWIMM && IS_IMMORTAL(victim))
				continue;

			Format::sprintf(buf,
			        "{W[%12s] {P%5d{RHP {P%5d{RMP{c/{G%2d %2d %2d %2d %2d %2d{c/{Y%8ld {bWorth{c/{Y%4d {bQpts{c/{Y%4d {b Spts{x\n",
			        victim->name,
			        GET_MAX_HIT(victim), GET_MAX_MANA(victim), ATTR_BASE(victim, APPLY_STR),
			        ATTR_BASE(victim, APPLY_INT), ATTR_BASE(victim, APPLY_WIS),
			        ATTR_BASE(victim, APPLY_DEX), ATTR_BASE(victim, APPLY_CON),
			        ATTR_BASE(victim, APPLY_CHR),
			        victim->gold + victim->silver / 100,
			        victim->questpoints,
			        !IS_NPC(victim) ? victim->pcdata->skillpoints : 0);
			buffer += buf;
		}

		page_to_char(buffer, ch);
		return;
	}

	if (arg.is_prefix_of("eq")) {

		for (victim = char_list; victim != nullptr; victim = victim->next) {
			if (IS_NPC(victim)
			    || !can_see_char(ch, victim))
				continue;

			if (!SHOWIMM && IS_IMMORTAL(victim))
				continue;

			Format::sprintf(buf,
			        "{W[%12s] {b%4d Items (W:%5d){c/{PH:%4d D:%4d{c/{GS:%-4d{c/{CAC:%-5d %-5d %-5d %-5d{x\n",
			        victim->name, get_carry_number(victim), get_carry_weight(victim),
			        GET_ATTR_HITROLL(victim), GET_ATTR_DAMROLL(victim), GET_ATTR_SAVES(victim),
			        GET_AC(victim, AC_PIERCE), GET_AC(victim, AC_BASH),
			        GET_AC(victim, AC_SLASH), GET_AC(victim, AC_EXOTIC));
			buffer += buf;
		}

		page_to_char(buffer, ch);
		return;
	}

	if (arg.is_prefix_of("absorb")) {

		for (victim = char_list; victim != nullptr; victim = victim->next) {
			if (IS_NPC(victim) || !can_see_char(ch, victim))
				continue;

			if (!SHOWIMM && IS_IMMORTAL(victim))
				continue;

			Format::sprintf(buf,
			        "{W[%12s] {RABS: {P%s{x\n",
			        victim->name, print_defense_modifiers(victim, TO_ABSORB));
			buffer += buf;
		}

		page_to_char(buffer, ch);
		return;
	}

	if (arg.is_prefix_of("immune")) {

		for (victim = char_list; victim != nullptr; victim = victim->next) {
			if (IS_NPC(victim) || !can_see_char(ch, victim))
				continue;

			if (!SHOWIMM && IS_IMMORTAL(victim))
				continue;

			Format::sprintf(buf,
			        "{W[%12s] {RIMM: {P%s{x\n",
			        victim->name,
			        print_defense_modifiers(victim, TO_IMMUNE));
			buffer += buf;
		}

		page_to_char(buffer, ch);
		return;
	}

	if (arg.is_prefix_of("resistance")) {

		for (victim = char_list; victim != nullptr; victim = victim->next) {
			if (IS_NPC(victim) || !can_see_char(ch, victim))
				continue;

			if (!SHOWIMM && IS_IMMORTAL(victim))
				continue;

			Format::sprintf(buf,
			        "{W[%12s] {HRES: {G%s{x\n",
			        victim->name,
			        print_defense_modifiers(victim, TO_RESIST));
			buffer += buf;
		}

		page_to_char(buffer, ch);
		return;
	}

	if (arg.is_prefix_of("vulnerable")) {

		for (victim = char_list; victim != nullptr; victim = victim->next) {
			if (IS_NPC(victim) || !can_see_char(ch, victim))
				continue;

			if (!SHOWIMM && IS_IMMORTAL(victim))
				continue;

			Format::sprintf(buf,
			        "{W[%12s] {TVUL: {C%s{x\n",
			        victim->name,
			        print_defense_modifiers(victim, TO_VULN));
			buffer += buf;
		}

		page_to_char(buffer, ch);
		return;
	}

	if (arg.is_prefix_of("snoop")) { /* this part by jerome */
		if (!IS_IMP(ch)) {
			stc("You can't use this check option.\n", ch);
			return;
		}


		for (victim = char_list; victim != nullptr; victim = victim->next) {
			if (IS_NPC(victim)
			    || victim->desc == nullptr
			    || !IS_PLAYING(victim->desc))
				continue;

			if (!SHOWIMM && IS_IMMORTAL(victim))
				continue;

			Format::sprintf(buf, "{W[%12s] is being snooped by {G%s\n", victim->name,
			        (victim->desc->snoop_by != nullptr) ? victim->desc->snoop_by->character->name : "nobody");
			buffer += buf;
		}

		page_to_char(buffer, ch);
		return;
	}

	stc("That is not a valid option.\n", ch);
	return;
}

void do_chown(Character *ch, String argument)
{
	Character *victim;
	Object  *obj;

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1.empty() || arg2.empty()) {
		stc("Get what from whom?\n", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg2, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (!OUTRANKS(ch, victim)) {
		stc("I don't think they'd like that too much.\n", ch);
		return;
	}

	// don't use get_obj_carry/wear, the visibility check shouldn't fall to the victim
	if ((obj = get_obj_list(ch, arg1, victim->carrying)) == nullptr) {
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
void recursive_clone(Character *ch, Object *obj, Object *clone)
{
	Object *c_obj, *t_obj;

	for (c_obj = obj->contains; c_obj != nullptr; c_obj = c_obj->next_content) {
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
void do_clone(Character *ch, String argument)
{
	const char *rest;
	Character *mob;
	Object  *obj;
	int j;
	int number;

	String arg, which;
	rest = one_argument(argument, arg);

	if (arg.empty()) {
		stc("Clone what?\n", ch);
		return;
	}

	if (arg.is_prefix_of("object")) {
		number = mult_argument(rest, which);
		mob = nullptr;
		obj = get_obj_here(ch, which);

		if (obj == nullptr) {
			stc("You don't see that here.\n", ch);
			return;
		}
	}
	else if (arg.is_prefix_of("mobile") || arg.is_prefix_of("character")) {
		number = mult_argument(rest, which);
		obj = nullptr;
		mob = get_char_here(ch, which, VIS_CHAR);

		if (mob == nullptr) {
			stc("You don't see that here.\n", ch);
			return;
		}
	}
	else { /* find both */
		number = mult_argument(argument, which);
		mob = get_char_here(ch, which, VIS_CHAR);
		obj = get_obj_here(ch, which);

		if (mob == nullptr && obj == nullptr) {
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
	if (obj != nullptr) {
		Object *clone = nullptr;

		for (j = 1; j <= number; j++) {
			clone = create_object(obj->pIndexData, 0);

			if (! clone) {
				bug("Error creating object in do_clone", 0);
				return;
			}

			clone_object(obj, clone);

			if (obj->carried_by != nullptr)
				obj_to_char(clone, ch);
			else
				obj_to_room(clone, ch->in_room);

			recursive_clone(ch, obj, clone);
		}

		if (number == 1) {
			act("$n clones $p.", ch, clone, nullptr, TO_ROOM);
			act("You clone $p.", ch, clone, nullptr, TO_CHAR);
			wiznet("$N has cloned: $p.", ch, clone, WIZ_LOAD, WIZ_SECURE, GET_RANK(ch));
		}
		else {
			Format::sprintf(arg, "$n clones $p[%d].", number);
			act(arg, ch, clone, nullptr, TO_ROOM);
			Format::sprintf(arg, "You clone $p[%d].", number);
			act(arg, ch, clone, nullptr, TO_CHAR);
			Format::sprintf(arg, "$N has cloned: $p[%d].", number);
			wiznet(arg, ch, clone, WIZ_LOAD, WIZ_SECURE, GET_RANK(ch));
		}

		return;
	}
	else if (mob != nullptr) {
		Character *clone;
		Object *new_obj;
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

		for (obj = mob->carrying; obj != nullptr; obj = obj->next_content) {
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
		act("$n clones $N.", ch, nullptr, clone, TO_ROOM);
		act("You clone $N.", ch, nullptr, clone, TO_CHAR);
		Format::sprintf(buf, "$N has cloned: %s.", clone->short_descr);
		wiznet(buf, ch, nullptr, WIZ_LOAD, WIZ_SECURE, GET_RANK(ch));
		return;
	}
} /* end do_clone() */

/* Funky style clone command */
void do_oclone(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Character *victim;
	Object  *obj;

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1.empty() || arg2.empty()) {
		stc("Syntax: oclone <object> <person who has object>\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg2, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	// don't use get_obj_carry/wear, the visibility check shouldn't fall to the victim
	if ((obj = get_obj_list(ch, arg1, victim->carrying)) == nullptr) {
		Format::sprintf(buf, "%s doesn't seem to have a %s.\n", victim->name, arg1);
		stc(buf, ch);
		return;
	}

	/* clone an object */
	if (obj != nullptr) {
		Object *clone;
		clone = create_object(obj->pIndexData, 0);

		if (! clone) {
			bug("Error cloning an object.", 0);
			return;
		}

		clone_object(obj, clone);

		if (obj->carried_by != nullptr)
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

void do_departedlist(Character *ch, String argument)
{
	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty() || argument.empty()) {
		stc("Syntax:\n"
		    "  departedlist add    <immortal>\n"
		    "  departedlist remove <immortal>\n", ch);
		return;
	}

	if (arg == "add") {
		if (has_departed(argument)) {
			ptc(ch, "'%s' already departed Legacy!\n", argument);
			return;
		}

		insert_departed(argument);
		save_departed_list();
		ptc(ch, "'%s' was successfully added as a departed immortal.\n", argument);
		return;
	}

	if (arg == "remove") {
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

void do_leader(Character *ch, String argument)
{
	Character *victim;
	/* Not using these at the moment. -- Outsider
	int count = 0, ldrcount = 0;
	*/

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		stc("Whom do you wish to make a clan leader?\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg, VIS_PLR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (victim->clan == nullptr && !IS_IMMORTAL(victim)) {
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

		if (victim->has_cgroup(GROUP_LEADER)) {
			victim->remove_cgroup(GROUP_LEADER);
			stc("You are no longer an implementor.\n", victim);
			stc("Leader flag removed.\n", ch);
		}
		else {
			if (victim->has_cgroup(GROUP_DEPUTY)) {
				victim->remove_cgroup(GROUP_DEPUTY);
				stc("You have been promoted to implementor.\n", victim);
				stc("Deputy flag removed.\n", ch);
			}
			else
				stc("You are now an implementor.\n", victim);

			victim->add_cgroup(GROUP_LEADER);
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

	if (victim->has_cgroup(GROUP_LEADER)) {
		victim->remove_cgroup(GROUP_LEADER);
		stc("You are no longer a clan leader.\n", victim);
		stc("Leader flag removed.\n", ch);
	}
	else {
		if (victim->has_cgroup(GROUP_DEPUTY)) {
			victim->remove_cgroup(GROUP_DEPUTY);
			stc("You have been promoted to clan leader.\n", victim);
			stc("Deputy flag removed.\n", ch);
		}
		else
			stc("You are now a clan leader.\n", victim);

		victim->add_cgroup(GROUP_LEADER);
		stc("Leader flag added.\n", ch);
	}
}

void do_deputize(Character *ch, String argument)
{
	Character *victim;
	int count = 0, depcount = 0;

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		stc("Whom do you wish to make a clan deputy?\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg, VIS_PLR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (victim->clan == nullptr && !IS_IMMORTAL(victim)) {
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

		if (victim->has_cgroup(GROUP_DEPUTY)) {
			victim->remove_cgroup(GROUP_DEPUTY);
			stc("You are no longer a head of your department.\n", victim);
			stc("Deputy flag removed.\n", ch);
		}
		else {
			if (victim->has_cgroup(GROUP_LEADER)) {
				victim->remove_cgroup(GROUP_LEADER);
				stc("You have been demoted to department head.\n", victim);
				stc("Leader flag removed.\n", ch);
			}
			else
				stc("You are now a head of your department.\n", victim);

			victim->add_cgroup(GROUP_DEPUTY);
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

	if (victim->has_cgroup(GROUP_DEPUTY)) {
		victim->remove_cgroup(GROUP_DEPUTY);
		stc("You are no longer a clan deputy.\n", victim);
		stc("Deputy flag removed.\n", ch);
	}
	else {
		if (victim->has_cgroup(GROUP_LEADER)) {
			victim->remove_cgroup(GROUP_LEADER);
			stc("You have been demoted to clan deputy.\n", victim);
			stc("Leader flag removed.\n", ch);
		}
		else
			stc("You are now a clan deputy.\n", victim);

		victim->add_cgroup(GROUP_DEPUTY);
		stc("Deputy flag added.\n", ch);
	}
}

void do_despell(Character *ch, String argument)
{
	Object *obj;

	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty()) {
		stc("Syntax: \n", ch);
		stc("despell <object name>\n", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, arg)) == nullptr) {
		stc("No such item.\n", ch);
		return;
	}

	stc("Item cleared of all spells.\n", ch);
	return;
}

void do_disconnect(Character *ch, String argument)
{
	Descriptor *d;
	int desc;
	char buf[MAX_INPUT_LENGTH];

	if (argument.empty()) {
		stc("Disconnect which socket? (type SOCKETS for list)\n",
		    ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (!arg.is_number()) {
		stc("Socket must be numeric!\n", ch);
		return;
	}

	if (arg.empty()) {
		stc("Disconnect whom?\n", ch);
		return;
	}

	desc = atoi(arg);

	for (d = descriptor_list; d != nullptr; d = d->next) {
		if (d->descriptor == desc) {
			if (d->connected == 0) {
				Format::sprintf(buf,
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

	Format::sprintf(buf,
	        "No socket number '%d' found. Check SOCKETS and try again!\n",
	        desc);
	stc(buf, ch);
}

/* idea by Erwin Andreasen */
/* Switch into another (perhaps live player) and execute a command, then switch back */
void do_doas(Character *ch, String argument)
{
	Character *victim;
	Descriptor *orig;

	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty() || argument.empty()) {
		stc("Syntax:  doas <victim> <command>\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) == nullptr) {
		stc("You couldn't find them.\n", ch);
		return;
	}

	if (victim == ch) {
		stc("You're too lazy to make yourself do something.\n", ch);
		return;
	}

	if (argument.is_prefix_of("quit") || argument.is_prefix_of("delete") || argument.is_prefix_of("switch")) {
		stc("Don't do that.\n", ch);
		return;
	}

	orig            = victim->desc;
	victim->desc    = ch->desc;
	interpret(victim, argument);
	ch->desc        = victim->desc;
	victim->desc    = orig;
}

void do_echo(Character *ch, String argument)
{
	Descriptor *d;

	if (argument.empty()) {
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

void do_recho(Character *ch, String argument)
{
	Descriptor *d;

	if (argument.empty()) {
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

void do_zecho(Character *ch, String argument)
{
	Descriptor *d;

	if (argument.empty()) {
		stc("Zone echo what?\n", ch);
		return;
	}

	for (d = descriptor_list; d; d = d->next) {
		if (IS_PLAYING(d)
		    &&  d->character->in_room != nullptr && ch->in_room != nullptr
		    &&  d->character->in_room->area == ch->in_room->area) {
			if (IS_IMMORTAL(d->character))
				stc("zone> ", d->character);

			stc(argument, d->character);
			stc("\n", d->character);
		}
	}
}

void do_pecho(Character *ch, String argument)
{
	Character *victim;

	String arg;
	argument = one_argument(argument, arg);

	if (argument.empty() || arg.empty()) {
		stc("Personal echo what?\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) == nullptr) {
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
void do_file(Character *ch, String argument)
{
	FILE *req_file;
	int num_lines = 0, req_lines = 0, cur_line = 0, i;
	char buf[MSL], strsave[50];
	String buffer;
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
		{       nullptr,           nullptr }
	};

	String field, value;
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
		if (field.is_prefix_of(fields[i].cmd))
			break;

	if (!fields[i].cmd) {
		stc("Invalid file name.\n", ch);
		return;
	}

	if (value[0] == '\0')
		req_lines = 150;
	else
		req_lines = URANGE(1, atoi(value), 150);

	Format::sprintf(strsave, "%s%s", MISC_DIR, fields[i].file);

	if ((req_file = fopen(strsave, "r")) == nullptr) {
		stc("That file does not exist.\n", ch);
		return;
	}

	/* count lines in requested file */
	while (fgets(buf, MIL, req_file) != nullptr)
		num_lines++;

	fclose(req_file);

	/* Limit # of lines printed to # requested or # of lines in file */
	if (req_lines > num_lines)
		req_lines = num_lines;

	if (num_lines <= 0) {
		stc("That file is empty.\n", ch);
		return;
	}

	if ((req_file = fopen(strsave, "r")) == nullptr) {
		bug("do_file: file does not exist on second attempt", 0);
		stc("That file does not exist.\n", ch);
		return;
	}


	/* and print the requested lines */
	while (fgets(buf, MIL, req_file) != nullptr)
		if (++cur_line > (num_lines - req_lines))
			buffer += buf;

	fclose(req_file);
	page_to_char(buffer, ch);
}

void do_followerlist(Character *ch, String argument)
{
	String query, deity;

	if (argument.empty())
		deity = ch->name;
	else
		one_argument(argument, deity);

	query = "SELECT name, deity FROM pc_index WHERE deity_nocol LIKE '";
	query += "%";
	query += deity;
	query += "%'";

	if (db_query("do_followerlist", query) != SQL_OK)
		return;

	String buffer;
	buffer += Format::format("{GFollowers of %s{G:{x\n", deity);
	buffer += "{G=================================================================={x\n";

	int count = 0;
	while (db_next_row() == SQL_OK) {
		count++;
		int space = 50 - String(db_get_column_str(1)).uncolor().size();
		String deityblock = "{W";

		while (space-- >= 0)
			deityblock += " ";

		deityblock += db_get_column_str(1);
		buffer += Format::format("{G[{W%12s{G][%s{G]{x\n", db_get_column_str(0), deityblock);
	}

	if (count == 0)
		ptc(ch, "No one follows %s.\n", deity);
	else {
		buffer += "{G=================================================================={x\n";
		buffer += Format::format("{WThere %s %d follower%s of %s{x.\n",
		    count == 1 ? "is" : "are", count, count == 1 ? "" : "s", deity);
		page_to_char(buffer, ch);
	}

}

/* Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard Right
   now its pretty much useless, but it might be useful in the future
   - Lotus */
const char *name_expand(Character *ch)
{
	int count = 1;
	Character *rch;
	static char outbuf[MAX_INPUT_LENGTH];

	if (!IS_NPC(ch))
		return ch->name.c_str();

	String name;
	one_argument(ch->name, name);  /* copy the first word into name */

	if (!name[0]) { /* weird mob .. no keywords */
		strcpy(outbuf, "");  /* Return an empty buffer */
		return outbuf;
	}

	for (rch = ch->in_room->people; rch && (rch != ch); rch = rch->next_in_room)
		if (rch->name.has_words(name))
			count++;

	Format::sprintf(outbuf, "%d.%s", count, name);
	return outbuf;
}

void do_for(Character *ch, String argument)
{
	char buf[MSL];
	RoomPrototype *room, *old_room = nullptr;
	Character *p, *p_next;
	bool fGods = FALSE, fMortals = FALSE, fRoom = FALSE, found;
	int i;

	String range;
	argument = one_argument(argument, range);

	if (!range[0] || !argument[0]) { /* invalid usage? */
		stc("Syntax:\n"
		    "  for all     <action>\n"
		    "  for gods    <action>\n"
		    "  for mortals <action>\n"
		    "  for room    <action>\n", ch);
		return;
	}

	if (argument.has_prefix("slay")
	    || argument.has_prefix("purge")
	    || argument.has_prefix("quit")
	    || argument.has_prefix("for ")
	    || argument.has_prefix("delete")
	    || argument.has_prefix("who")) {
		stc("I don't think so Tim!\n", ch);
		return;
	}

	if (range.is_prefix_of("all")) {
		fMortals = TRUE;
		fGods = TRUE;
	}
	else if (range.is_prefix_of("gods"))
		fGods = TRUE;
	else if (range.is_prefix_of("mortals"))
		fMortals = TRUE;
	else if (range.is_prefix_of("room"))
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
				const char *pSource = argument.c_str(); /* head of buffer to be parsed */
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

void do_goto(Character *ch, String argument)
{
	RoomPrototype *location = nullptr;
	Character *rch;
	Object *obj;
	int count = 0;
	bool goto_pet = FALSE;

	if (argument.empty()) {
		stc("Syntax:\n"
		    "  goto <room vnum>\n"
		    "  goto <character name or object name>\n"
		    "  goto obj <object name>\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg.is_number())
		location = get_room_index(atoi(arg));
	else {
		if ((rch = get_char_world(ch, argument, VIS_CHAR)) != nullptr)
			location = rch->in_room;

		if (arg.is_prefix_of("object") || location == nullptr) {
			location = nullptr;

			if ((obj = get_obj_world(ch, argument)) != nullptr)
				if (obj->in_room)
					location = obj->in_room;
		}
	}

	if (location == nullptr) {
		stc("No such location.\n", ch);
		return;
	}

	for (rch = location->people; rch != nullptr; rch = rch->next_in_room)
		count++;

	if (!is_room_owner(ch, location) && room_is_private(location)
	    && (count > 1 || !IS_IMP(ch))) {
		stc("That room is private.\n", ch);
		return;
	}

	if (ch->fighting != nullptr)
		stop_fighting(ch, TRUE);

	for (rch = ch->in_room->people; rch != nullptr; rch = rch->next_in_room) {
		if (can_see_char(rch, ch)) {
			if (ch->pcdata != nullptr && ch->pcdata->bamfout[0] != '\0')
				act("$t", ch, ch->pcdata->bamfout, rch, TO_VICT);
			else
				act("$n warps off to distant lands in a burst of electric energy.", ch, nullptr, rch, TO_VICT);
		}
	}

	if (ch->pet != nullptr && ch->in_room == ch->pet->in_room && !ch->pet->act_flags.has(ACT_STAY))
		goto_pet = TRUE;

	char_from_room(ch);
	char_to_room(ch, location);

	for (rch = ch->in_room->people; rch != nullptr; rch = rch->next_in_room) {
		if (can_see_char(rch, ch)) {
			if (ch->pcdata != nullptr && ch->pcdata->bamfin[0] != '\0')
				act("$t", ch, ch->pcdata->bamfin, rch, TO_VICT);
			else
				act("$n materializes in a burst of electric energy.", ch, nullptr, rch, TO_VICT);
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
void do_grouplist(Character *ch, String argument)
{
	struct group_data {
		Character *leader;
		struct group_data *next;
	};
	typedef struct group_data GROUP_DATA;
	Descriptor *d;
	GROUP_DATA *leaders = nullptr;
	GROUP_DATA *newnode, *curnode, *nextnode;
	Character *victim;
	bool dupe;
	int counter = 0;
	String buf;

	/* loop over all players, looking for leaders.
	   we don't find the leaders directly, we get pointers to them
	   from their followers. */
	for (d = descriptor_list; d != nullptr; d = d->next) {
		victim = (d->original != nullptr) ? d->original : d->character;

		if (victim == nullptr)
			continue;

		if (victim->leader == nullptr)
			continue;

		/* check for duplicate, add only new leaders */
		dupe = FALSE;

		for (curnode = leaders; curnode != nullptr; curnode = curnode->next) {
			if (curnode->leader == victim->leader) {
				dupe = TRUE;
				break;
			}
		}

		if (!dupe) {
			newnode = new GROUP_DATA;
			newnode->leader = victim->leader;
			newnode->next = leaders;
			leaders = newnode;
		}
	}

	/* loop over all leaders, print the group */
	for (curnode = leaders; curnode != nullptr; curnode = curnode->next) {
		Format::sprintf(buf, "{G<G%d> {Y%s{x", ++counter, curnode->leader->name);

		/* find all followers */
		for (d = descriptor_list; d != nullptr; d = d->next) {
			victim = (d->original != nullptr) ? d->original : d->character;

			if (victim != nullptr
			    && victim->leader != victim
			    && victim->leader == curnode->leader) {
				buf += " ";
				buf += victim->name;
			}
		}

		buf += "\n";
		stc(buf, ch);
	}

	/* check for 'no groups' */
	if (leaders == nullptr) {
		stc("No groups found.\n", ch);
		return;
	}

	/* release leader chain */
	while (leaders != nullptr) {
		nextnode = leaders->next;
		delete leaders;
		leaders = nextnode;
	}
} /* end do_grouplist() */

void do_guild(Character *ch, String argument)
{
	Character *victim;
	Clan *clan;

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1.empty() || arg2.empty()) {
		stc("Syntax: guild <char> <clan name>\n", ch);
		return;
	}

	/* players first! */
	victim = get_player_world(ch, arg1, VIS_PLR);

	if (victim == nullptr)
		victim = get_char_world(ch, arg1, VIS_PLR);

	if (victim == nullptr) {
		stc("The character is not logged in the realm.\n", ch);
		return;
	}

	if (arg2.is_prefix_of("none")) {
		stc("The character is now clanless.\n", ch);
		stc("You are now clanless.\n", victim);

		if (!IS_IMMORTAL(victim)) {
			/* Remove leaderflag if it's set */
			victim->remove_cgroup(GROUP_LEADER);
			victim->remove_cgroup(GROUP_DEPUTY);
			victim->remove_cgroup(GROUP_CLAN);
		}

		victim->clan = nullptr;
		victim->questpoints_donated = 0;
		victim->gold_donated = 0;
		save_char_obj(victim);
		return;
	}

	clan = clan_lookup(arg2);

	if (clan == nullptr) {
		stc("No such clan exists.\n", ch);
		return;
	}

	if (ch != victim)
		ptc(ch, "The character is now a member of %s.\n", clan->clanname);

	ptc(victim, "You have now become a member of %s.\n", clan->clanname);

	if (!IS_IMMORTAL(victim)) {
		/* Remove leaderflag if it's set */
		victim->remove_cgroup(GROUP_LEADER);
		victim->remove_cgroup(GROUP_DEPUTY);
	}

	victim->add_cgroup(GROUP_CLAN);
	victim->clan = clan;
	victim->questpoints_donated = 0;
	victim->gold_donated = 0;
	save_char_obj(victim);
}

/* answer to PRAY. Goes to one mortal like TELL does but does not reveal the
   imm who is sending the message. Also broadcasts to all other imms in game. */
void do_heed(Character *ch, String argument)
{
	char buf[100 + MIL]; /* enough for pompous intro + text */
	Character *victim, *truevictim;
	Player *tpc;
	Descriptor *d;

	String arg1;
	argument = one_argument(argument, arg1);

	if (!arg1[0]) {
		if (ch->pcdata->plr_flags.has(PLR_HEEDNAME)) {
			stc("Your name will not be shown in heeds.\n", ch);
			ch->pcdata->plr_flags -= PLR_HEEDNAME;
		}
		else {
			stc("Your name will now be shown in heeds.\n", ch);
			ch->pcdata->plr_flags += PLR_HEEDNAME;
		}

		return;
	}

	/* find a player to talk to. Only REAL players are eligible. */
	for (tpc = pc_list; tpc; tpc = tpc->next)
		if (tpc->ch->name.has_words(arg1))
			break;

	if (!tpc || (truevictim = tpc->ch) == nullptr) {
		ptc(ch, "No player called \"%s\" is in the game!\n", arg1);
		return;
	}

	/* is he linkdead? */
	if (truevictim->pcdata->plr_flags.has(PLR_LINK_DEAD)) {
		ptc(ch, "%s is linkdead at this time.\n", truevictim->name);
		return;
	}

	/* if he's SWITCHed or MORPHed, track down his current char */
	victim = truevictim;

	if (victim->desc == nullptr) {
		victim = nullptr;

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
	    ch->pcdata->plr_flags.has(PLR_HEEDNAME) ? ch->name : "An Immortal", argument);
	set_color(victim, WHITE, NOBOLD);
	/* build a message for the other imms */

	const String& wizname = ch->desc && ch->desc->original ? ch->desc->original->name : ch->name;

	if (victim != truevictim)
		Format::sprintf(buf, "%s HEEDs %s (%s): %s\n", wizname, truevictim->name, victim->name, argument);
	else
		Format::sprintf(buf, "%s HEEDs %s: %s\n", wizname, truevictim->name, argument);

	/* send it to all other imms who are connected and listening */
	for (d = descriptor_list; d; d = d->next) {
		if (IS_PLAYING(d)) {
			victim = d->character;
			truevictim = d->original ? d->original : victim;

			if (IS_IMMORTAL(truevictim)
			    && !truevictim->comm_flags.has(COMM_NOPRAY)
			    && !truevictim->comm_flags.has(COMM_QUIET)) {
				new_color(victim, CSLOT_CHAN_PRAY);
				stc(buf, victim);
				set_color(victim, WHITE, NOBOLD);
			}
		}
	}
} /* end do_heed() */

void do_linkload(Character *ch, String argument)
{
	Character *victim;
	char buf[MAX_STRING_LENGTH];
	Descriptor *dnew;
	Descriptor *d;
	int desc = 5;

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		stc("Whom do you want to linkload?\n", ch);
		return;
	}

	for (d = descriptor_list; d != nullptr; d = d->next)
		desc++;

	desc++;

	if (!check_parse_name(arg)) {
		stc("That is not a valid player name.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) != nullptr) {
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
		victim->desc = nullptr;

		if (OUTRANKS(victim, ch)) {
			act("You're not high enough level to linkload $N.", ch, nullptr, victim, TO_CHAR);
			extract_char(victim, TRUE);
		}
		else {
			Format::sprintf(buf, "You reach into the pfile and link-load %s from room %d.\n",
			        victim->name, victim->in_room->vnum);
			stc(buf, ch);
			act("$n reaches into the pfiles and link-loads $N.", ch, nullptr, victim, TO_NOTVICT);
			Format::sprintf(buf, "$N has link-loaded %s from room %d.", victim->name, victim->in_room->vnum);
			wiznet(buf, ch, nullptr, WIZ_LOAD, WIZ_SECURE, 0);
			char_to_room(victim, ch->in_room);

			if (victim->pet != nullptr)
				char_to_room(victim->pet, ch->in_room);

			victim->pcdata->plr_flags += PLR_LINK_DEAD;
		}
	}
	else {
		stc("No such player exists in the pfiles.\n", ch);
		free_char(dnew->character);
	}

	free_descriptor(dnew);
} /* end do_linkload() */

void do_mload(Character *ch, String argument)
{
	MobilePrototype *pMobIndex;
	Character *victim;
	char buf[MAX_STRING_LENGTH];

	String arg;
	one_argument(argument, arg);

	if (arg.empty() || !arg.is_number()) {
		stc("Syntax: load mob <vnum>.\n", ch);
		return;
	}

	if ((pMobIndex = get_mob_index(atoi(arg))) == nullptr) {
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
	act("$n creates $N!", ch, nullptr, victim, TO_ROOM);
	Format::sprintf(buf, "$N loads %s.", victim->short_descr);
	wiznet(buf, ch, nullptr, WIZ_LOAD, WIZ_SECURE, GET_RANK(ch));
	stc("Success.\n", ch);
	return;
}

void do_oload(Character *ch, String argument)
{
	ObjectPrototype *pObjIndex;
	Object *obj;

	String arg1;
	argument = one_argument(argument, arg1);

	if (arg1.empty() || !arg1.is_number()) {
		stc("Syntax: load obj <vnum>.\n", ch);
		return;
	}

	if ((pObjIndex = get_obj_index(atoi(arg1))) == nullptr) {
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

	act("$n has created $p!", ch, obj, nullptr, TO_ROOM);
	wiznet("$N loads $p.", ch, obj, WIZ_LOAD, WIZ_SECURE, GET_RANK(ch));
	stc("Success.\n", ch);
	return;
}

void do_load(Character *ch, String argument)
{
	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty()) {
		stc("Syntax:\n", ch);
		stc("  load mob <vnum>\n", ch);
		stc("  load obj <vnum>\n", ch);
		return;
	}

	if (arg == "mob" || arg == "char") {
		do_mload(ch, argument);
		return;
	}

	if (arg == "obj") {
		do_oload(ch, argument);
		return;
	}

	/* echo syntax */
	do_load(ch, "");
}

void do_lower(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Object *obj;
	Character *victim;
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

	String what;
	argument = one_argument(argument, what);

	if (what[0] == '\0') {
		stc("Which item do you wish to lower?\n", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, what)) == nullptr) {
		act("You're not carrying a(n) $t.", ch, what, nullptr, TO_CHAR,
		        POS_DEAD, FALSE);
		return;
	}

	if (argument.empty()) {
		act("Whom do you want to lower $t for?", ch,
		        obj->short_descr, nullptr, TO_CHAR, POS_DEAD, FALSE);
		return;
	}

	if ((victim = get_player_world(ch, argument, VIS_PLR)) == nullptr) {
		act("Sorry, no player called '$t' is in the game!", ch,
		        argument, nullptr, TO_CHAR, POS_DEAD, FALSE);
		return;
	}

	if (victim->level < LEVEL_HERO) {
		act("Sorry, $t must be level 91 to have an item lowered.", ch,
		        victim->name.c_str(), nullptr, TO_CHAR, POS_DEAD, FALSE);
		return;
	}

	if (obj->extra_descr != nullptr) {
		ExtraDescr *ed_next;
		ExtraDescr *ed;

		for (ed = obj->extra_descr; ed != nullptr; ed = ed_next) {
			ed_next = ed->next;

			if (ed->keyword == KEYWD_OWNER) {
				char strip[MAX_STRING_LENGTH];
				strcpy(strip, ed->description);
				strip[strlen(strip) - 2] = '\0';
				Format::sprintf(buf, "This item is already owned by %s.\n", strip);
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
		act("But $t is already lower than level 92.", ch,
		        obj->short_descr, nullptr, TO_CHAR, POS_DEAD, FALSE);
		return;
	}

	if (qp > victim->questpoints && !IS_IMMORTAL(victim)) {
		stc("They do not have enough quest points for that.\n"
		    , ch);
		return;
	}

	if (!IS_IMMORTAL(victim)) {
		Format::sprintf(buf, "%s %s", what, victim->name);
		do_owner(ch, buf);
		Format::sprintf(buf, "deduct %s %d", victim->name, qp);
		do_quest(ch, buf);
	}

	obj->level = LEVEL_HERO;
	act("$t has been successfully lowered to level 91.", ch,
	        obj->short_descr, nullptr, TO_CHAR, POS_DEAD, FALSE);
}

void do_lurk(Character *ch, String argument)
{
	if (ch->lurk_level) {
		ch->lurk_level = 0;
		act("$n is no longer cloaked.", ch, nullptr, nullptr, TO_ROOM);
		stc("You are no longer cloaked.\n", ch);
	}
	else {
		ch->lurk_level = LEVEL_IMMORTAL;
		act("$n cloaks $s presence to those outside the room.", ch, nullptr, nullptr, TO_ROOM);
		stc("You cloak your presence to those outside the room.\n", ch);
	}
}

/* Master command by Lotus */
void do_master(Character *ch, String argument)
{
	Character *victim;
	Character *pet;

	String arg1;
	argument = one_argument(argument, arg1);

	if (arg1.empty() || argument.empty()) {
		stc("Master what mobile to whom?\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == nullptr) {
		stc("That player is not here.\n", ch);
		return;
	}

	if ((pet = get_char_here(victim, argument, VIS_CHAR)) == nullptr) {
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

	if (victim->pet != nullptr) {
		stc("They have a pet already!\n", ch);
		return;
	}

	if (pet->master != nullptr) {
		stc("That mobile already has a master.\n", ch);
		return;
	}

	make_pet(ch, pet);

	stc("You have set the player with a pet.\n", ch);
	stc("You have been set with a new pet.\n", victim);
	stc("You have been turned into a pet!\n", pet);
}

/* Olevel and Mlevel from ROM Newsgroup */
void do_olevel(Character *ch, String argument)
{
	extern int top_obj_index;
	char buf[MAX_STRING_LENGTH];
	char tmpbuf[80];        // Extra buffer, needed to fix mis-alignment. by Clerve
	String buffer;
	ObjectPrototype *pObjIndex;
	int vnum, blevel, elevel;
	int nMatch, matches;
	bool found, with_wear;

	/* Check 1st argument - required begin level */
	String arg1, arg2, arg3;
	argument = one_argument(argument, arg1);

	if (!arg1.is_number()) {
		stc("Syntax: olevel [beg level] [end level] [wear type]\n", ch);
		return;
	}

	blevel = atoi(arg1);
	elevel = blevel;
	Flags::Bit wear_loc = Flags::all;          /* standard: everything */
	with_wear = FALSE;

	/* Check for 2nd argument - optional ending level */
	argument = one_argument(argument, arg2);

	if (!arg2.empty()) {
		if (!arg2.is_number()) {
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
		arg3 = arg2;
	else
		argument = one_argument(argument, arg3);

	bool specified_wear_loc = FALSE;

	if (!arg3.empty()) {
		if (arg3.is_prefix_of("take"))
			wear_loc = ITEM_TAKE;
		else if (arg3.is_prefix_of("finger"))
			wear_loc = ITEM_WEAR_FINGER;
		else if (arg3.is_prefix_of("neck"))
			wear_loc = ITEM_WEAR_NECK;
		else if (arg3.is_prefix_of("body"))
			wear_loc = ITEM_WEAR_BODY;
		else if (arg3.is_prefix_of("head"))
			wear_loc = ITEM_WEAR_HEAD;
		else if (arg3.is_prefix_of("legs"))
			wear_loc = ITEM_WEAR_LEGS;
		else if (arg3.is_prefix_of("feet"))
			wear_loc = ITEM_WEAR_FEET;
		else if (arg3.is_prefix_of("hands"))
			wear_loc = ITEM_WEAR_HANDS;
		else if (arg3.is_prefix_of("arms"))
			wear_loc = ITEM_WEAR_ARMS;
		else if (arg3.is_prefix_of("shield"))
			wear_loc = ITEM_WEAR_SHIELD;
		else if (arg3.is_prefix_of("about"))
			wear_loc = ITEM_WEAR_ABOUT;
		else if (arg3.is_prefix_of("waist"))
			wear_loc = ITEM_WEAR_WAIST;
		else if (arg3.is_prefix_of("wrist"))
			wear_loc = ITEM_WEAR_WRIST;
		else if (arg3.is_prefix_of("wield"))
			wear_loc = ITEM_WIELD;
		else if (arg3.is_prefix_of("hold"))
			wear_loc = ITEM_HOLD;
		else if (arg3.is_prefix_of("float"))
			wear_loc = ITEM_WEAR_FLOAT;
		else if (arg3.is_prefix_of("none"))
			wear_loc = Flags::none;
		else {
			stc("That is not a suitable wear location.\n", ch);
			return;
		}

		specified_wear_loc = TRUE;
	}

	found = FALSE;
	nMatch = 0;
	matches = 0;

	for (vnum = 0; nMatch < top_obj_index; vnum++) {
		if ((pObjIndex = get_obj_index(vnum)) != nullptr) {
			nMatch++;
			found = FALSE;

			if ((blevel <= pObjIndex->level) && (elevel >= pObjIndex->level)) {
				if (!specified_wear_loc)
					found = TRUE;
				else {
					if (pObjIndex->wear_flags.has(wear_loc))
						found = TRUE;
				}
			}
		}

		if (found) {
			Format::sprintf(tmpbuf, "[%%4d][%%5d] %%-%zus{x [%%s]\n",
			        45 + (strlen(pObjIndex->short_descr) - pObjIndex->short_descr.uncolor().size()));
			Format::sprintf(buf, tmpbuf, pObjIndex->level, pObjIndex->vnum,
			        pObjIndex->short_descr, wear_bit_name(pObjIndex->wear_flags));
			buffer += buf;
			matches++;
			found = FALSE;
		}
	}

	if (matches == 0)
		stc("No objects by that level.\n", ch);
	else {
		if (blevel < elevel)
			Format::sprintf(buf, "Objects level range: %d to %d.\n", blevel, elevel);
		else
			Format::sprintf(buf, "Objects level: %d\n", blevel);

		stc(buf, ch);
		stc("Level Vnum    Name                                          Wear Loc.\n", ch);
		page_to_char(buffer, ch);
		Format::sprintf(buf, "%d match%s found.\n", matches, (matches > 0) ? "es" : "");
		stc(buf, ch);
	}

	return;
}

void do_mlevel(Character *ch, String argument)
{
	extern int top_mob_index;
	char buf[MAX_STRING_LENGTH];
	char tmpbuf[80];        // needed to mix misalignment due to colorcodes.. Clerve
	String buffer;
	MobilePrototype *pMobIndex;
	int vnum, blevel, elevel;
	int nMatch;
	bool found;

	String arg;
	argument = one_argument(argument, arg);

	if (!arg.is_number()) {
		stc("Syntax: mlevel [beg level] [end level]\n", ch);
		return;
	}

	blevel = atoi(arg);

	if (!argument.empty()) {
		if (!argument.is_number()) {
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

	found = FALSE;
	nMatch = 0;

	for (vnum = 0; nMatch < top_mob_index; vnum++) {
		if ((pMobIndex = get_mob_index(vnum)) != nullptr) {
			nMatch++;

			if ((blevel <= pMobIndex->level) && (elevel >= pMobIndex->level)) {
				found = TRUE;
				Format::sprintf(tmpbuf, "[%%3d][%%5d] %%-%zus (Align: %%d)\n",
				        40 + (strlen(pMobIndex->short_descr) - pMobIndex->short_descr.uncolor().size()));
				Format::sprintf(buf, tmpbuf,
				        pMobIndex->level, pMobIndex->vnum,
				        pMobIndex->short_descr, pMobIndex->alignment);
				buffer += buf;
			}
		}
	}

	if (!found)
		stc("No mobiles by that level.\n", ch);
	else
		page_to_char(buffer, ch);

	return;
}

void do_motd(Character *ch, String argument)
{
	String buf;

	if (!argument.empty()) {
		if (argument == "clear") {
			Game::motd.erase();
			stc("MOTD cleared.\n", ch);
			return;
		}

		if (argument[0] == '-') {
			int len;
			bool found = FALSE;

			if (Game::motd.empty()) {
				stc("No lines left to remove.\n", ch);
				return;
			}

			buf = Game::motd;

			for (len = strlen(buf); len > 0; len--) {
				if (buf[len] == '\n') {
					if (!found) { /* back it up */
						if (len > 0)
							len--;

						found = TRUE;
					}
					else { /* found the second one */
						buf[len + 1] = '\0';
						Game::motd = buf;
						set_color(ch, GREEN, NOBOLD);
						stc("The MOTD is:\n", ch);
						stc(!Game::motd.empty() ? Game::motd :
						    "(None).\n", ch);
						set_color(ch, WHITE, NOBOLD);
						return;
					}
				}
			}

			buf[0] = '\0';
			Game::motd.erase();
			stc("MOTD cleared.\n", ch);
			return;
		}

		if (argument[0] == '+') {
			buf += Game::motd;
			argument = argument.substr(1).lstrip();
		}

		if (strlen(buf) + strlen(argument) >= MAX_STRING_LENGTH - 2) {
			stc("MOTD too long.\n", ch);
			return;
		}

		buf += argument;
		buf += "\n";
		Game::motd = buf;
	}

	set_color(ch, WHITE, BOLD);
	stc("The MOTD is:\n", ch);
	stc(!Game::motd.empty() ? Game::motd : "(None).\n", ch);
	set_color(ch, WHITE, NOBOLD);
	return;
}

void do_owhere(Character *ch, String argument)
{
	char buf[MSL];
	String output;
	Object *obj, *in_obj;
	int count = 1, vnum = 0;
	bool fGround = FALSE;
	int place_last_found = 0;   /* the vnum of the place where we last found an item */
	int item_last_found = 0;    /* the vnum of the last item displayed */

	String arg, arg2;
	argument = one_argument(argument, arg);
	one_argument(argument, arg2);

	if (arg.empty()) {
		stc("Syntax:\n"
		    "  owhere <object or vnum> <second keyword, level, or 'ground'>\n", ch);
		return;
	}

	if (arg.is_number())
		vnum = atoi(arg);

	if (arg2.is_prefix_of("ground"))
		fGround = TRUE;

	output += "{VCount {YRoom  {GObject{x\n";

	/* cut off list at 400 objects, to prevent spamming out your link */
	for (obj = object_list; obj != nullptr; obj = obj->next) {
		if (fGround && !obj->in_room)
			continue;

		if (vnum > 0) {
			if (obj->pIndexData->vnum != vnum)
				continue;
		}
		else if (!obj->name.has_words(arg))
			continue;

		if (!argument.empty() && !fGround) {
			if (arg2.is_number()) {
				if (atoi(arg2) != obj->level)
					continue;
			}
			else if (vnum > 0 || !obj->name.has_words(argument))
				continue;
		}

		for (in_obj = obj; in_obj->in_obj != nullptr; in_obj = in_obj->in_obj)
			;

		if (in_obj->carried_by) {
			if (in_obj->carried_by->in_room == nullptr
			    || !can_see_room(ch, in_obj->carried_by->in_room)
			    || !can_see_char(ch, in_obj->carried_by))
				continue;

			/* keep multiple hits on the same person from being displayed
			   -- Outsider
			*/
			if ((place_last_found == in_obj->carried_by->in_room->vnum) &&
			    (item_last_found == obj->pIndexData->vnum))
				continue;

			Format::sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{H[{G%5d{H]{x %s{x is carried by %s.\n",
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
			if (in_obj->in_locker->in_room == nullptr
			    || !can_see_room(ch, in_obj->in_locker->in_room)
			    || !can_see_char(ch, in_obj->in_locker))
				continue;

			/* keep multiple hits on the same locker from being displayed
			   -- Outsider
			*/
			if ((place_last_found == in_obj->in_locker->in_room->vnum) &&
			    (item_last_found == obj->pIndexData->vnum))
				continue;

			Format::sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{H[{G%5d{H]{x %s{x is in %s's locker.\n",
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
			if (in_obj->in_strongbox->in_room == nullptr
			    || !can_see_room(ch, in_obj->in_strongbox->in_room)
			    || !can_see_char(ch, in_obj->in_strongbox))
				continue;

			/* try not to get same items in the same place multiple times
			   -- Outsider
			*/
			if ((place_last_found == in_obj->in_strongbox->in_room->vnum) &&
			    (item_last_found == obj->pIndexData->vnum))
				continue;

			Format::sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{H[{G%5d{H]{x %s{x is in %s's strongbox.\n",
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

			Format::sprintf(buf, "{M[{V%3d{M]{b[{Y%5d{b]{H[{G%5d{H]{x %s{x in %s.\n",
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
			output += buf;
	}

	if (--count == 0)
		stc("You found no item like that.\n", ch);
	else {
		Format::sprintf(buf, "You found %d matching item%s%s%s.\n",
		        count,
		        count > 1 ? "s" : "",
		        fGround ? " lying around" : "",
		        count > 400 ? ", of which 400 are shown" : "");
		output += buf;
		page_to_char(output, ch);
	}

}

void do_mwhere(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	String output;
	Character *victim;
	bool found;

	String arg, arg2;
	argument = one_argument(argument, arg);
	one_argument(argument, arg2);

	if (arg.empty()) {
		stc("Syntax: mwhere <mob name or vnum> <second keyword or level>\n", ch);
		return;
	}

	found = FALSE;

	for (victim = char_list; victim != nullptr; victim = victim->next) {
		if (!IS_NPC(victim) || victim->in_room == nullptr)
			continue;

		if (arg.is_number()) {
			if (atoi(arg) != victim->pIndexData->vnum)
				continue;
		}
		else if (!victim->name.has_words(arg))
			continue;

		if (!arg2.empty()) {
			if (arg2.is_number()) {
				if (atoi(arg2) != victim->level)
					continue;
			}
			else if (!victim->name.has_words(arg2))
				continue;
		}

		found = TRUE;
		Format::sprintf(buf, "[%5d] %s%*s[%5d] %s\n",
		        victim->pIndexData->vnum,
		        victim->short_descr,
		        28 - victim->short_descr.uncolor().size(),
		        " ",
		        victim->in_room->vnum,
		        victim->in_room->name);
		output += buf;
	}

	page_to_char(output, ch);

	if (!found) {
		if (arg.is_number())
			ptc(ch, "You did not find a mobile of vnum %d.", atoi(arg));
		else
			act("You did not find: $T.", ch, nullptr, arg, TO_CHAR);

		return;
	}
}

/* find a room, given its name */
void do_rwhere(Character *ch, String argument)
{
	RoomPrototype *room;
	String dbuf, rbuf;
	char buf[MAX_INPUT_LENGTH], fname[MAX_INPUT_LENGTH];
	char *cp;
	bool found = FALSE;
	int vnum;

	if (argument.empty()) {
		stc("rwhere which room?\n", ch);
		return;
	}


	for (const auto area: Game::world().areas) {
		for (vnum = area->min_vnum; vnum <= area->max_vnum; vnum++) {
			room = get_room_index(vnum);

			if (room != nullptr) {
				rbuf = room->name;

				if (rbuf.uncolor().has_words(argument)) {
					found = TRUE;
					strcpy(fname, room->area->file_name);
					cp = strchr(fname, '.');

					if (cp != nullptr)
						*cp = '\0';

					Format::sprintf(buf, "[%5d] <%-8.8s> %s{x\n", vnum, fname, room->name);
					dbuf += buf;
				}
			}
		}
	}

	if (!found)
		dbuf += "No matching rooms found.\n";

	page_to_char(dbuf, ch);
} /* end do_rwhere() */

void do_mfind(Character *ch, String argument)
{
	extern int top_mob_index;
	char buf[MAX_STRING_LENGTH];
	String output;
	MobilePrototype *pMobIndex;
	int vnum;
	int nMatch;
	bool fAll;
	bool found;

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		stc("Find whom?\n", ch);
		return;
	}

	fAll        = FALSE; /*  arg == "all" ; */
	found       = FALSE;
	nMatch      = 0;

	/*
	 * Yeah, so iterating over all vnum's takes 10,000 loops.
	 * Get_mob_index is fast, and I don't feel like threading another link.
	 * Do you?
	 * -- Furey
	 */
	for (vnum = 0; nMatch < top_mob_index; vnum++) {
		if ((pMobIndex = get_mob_index(vnum)) != nullptr) {
			nMatch++;

			if (fAll || pMobIndex->player_name.has_words(argument)) {
				found = TRUE;
				Format::sprintf(buf, "M (%3d) [%5d] %s\n",
				        pMobIndex->level, pMobIndex->vnum, pMobIndex->short_descr);
				output += buf;
			}
		}
	}

	page_to_char(output, ch);

	if (!found)
		stc("No mobiles by that name.\n", ch);

	return;
}

void do_ofind(Character *ch, String argument)
{
	extern int top_obj_index;
	char buf[MAX_STRING_LENGTH];
	ObjectPrototype *pObjIndex;
	String output;
	int vnum;
	int nMatch;
	bool fAll;
	bool found;

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		stc("Find what object?\n", ch);
		return;
	}


	fAll        = FALSE; /*  arg == "all" ; */
	found       = FALSE;
	nMatch      = 0;

	/*
	 * Yeah, so iterating over all vnum's takes 10,000 loops.
	 * Get_obj_index is fast, and I don't feel like threading another link.
	 * Do you?
	 * -- Furey
	 */
	for (vnum = 0; nMatch < top_obj_index; vnum++) {
		if ((pObjIndex = get_obj_index(vnum)) != nullptr) {
			nMatch++;

			if (fAll || pObjIndex->name.has_words(argument)) {
				found = TRUE;
				Format::sprintf(buf, "O (%3d) [%5d] %s\n",
				        pObjIndex->level, pObjIndex->vnum, pObjIndex->short_descr);
				output += buf;
			}
		}
	}

	page_to_char(output, ch);

	if (!found)
		stc("No objects by that name.\n", ch);

	return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */
void do_vnum(Character *ch, String argument)
{
	String arg;
	const char *string = one_argument(argument, arg);

	if (arg.empty()) {
		stc("Syntax:\n", ch);
		stc("  vnum obj <name>\n", ch);
		stc("  vnum mob <name>\n", ch);
		stc("  vnum skill <skill or spell>\n", ch);
		return;
	}

	if (arg == "obj") {
		do_ofind(ch, string);
		return;
	}

	if (arg == "mob" || arg == "char") {
		do_mfind(ch, string);
		return;
	}

	if (arg == "skill" || arg == "spell") {
		do_slookup(ch, string);
		return;
	}

	/* do both */
	do_mfind(ch, argument);
	do_ofind(ch, argument);
}

void do_canmakebag(Character *ch, String argument)
{
	Character *victim;

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		stc("Change newbie helper flag on who?", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (victim->act_flags.has(PLR_MAKEBAG)) {
		stc("You are no longer a Newbie Helper.\n", victim);
		stc("They are no longer a newbie helper.\n", ch);
		victim->act_flags -= PLR_MAKEBAG;
		return;
	}
	else {
		stc("You are now a Newbie Helper!! WooWoo!!.\n", victim);
		stc("They are now a newbie helper.\n", ch);
		victim->act_flags += PLR_MAKEBAG;
		return;
	}
}

/* Noreply by Lotus */
void do_noreply(Character *ch, String argument)
{
	Character *wch;

	if (IS_NPC(ch))
		return;

	for (wch = char_list; wch != nullptr; wch = wch->next) {
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
void do_owner(Character *ch, String argument)
{
	Object *item;
	Character *player = nullptr;
	ExtraDescr *ed;

	String what, whom;
	argument = one_argument(argument, what);
	argument = one_argument(argument, whom);

	if (what[0] == '\0') {
		stc("Which item do you want to personalize?\n", ch);
		return;
	}

	if ((item = get_obj_carry(ch, what)) == nullptr) {
		stc("You are not carrying that.\n", ch);
		return;
	}

	if (whom[0] == '\0') {
		ptc(ch, "Whom do you want to personalize the %s to?\n", item->pIndexData->short_descr);
		ptc(ch, "Type 'owner %s none' if you want to make it public.\n", what);
		return;
	}

	if (whom != "none") {
		if ((player = get_player_world(ch, whom, VIS_PLR)) == nullptr) {
			stc("There is no one by that name in the realm.\n", ch);
			return;
		}
	}

	if (whom == "none") {
		if (item->extra_descr != nullptr) {
			ExtraDescr *ed_next, *ed_prev = nullptr;

			for (ed = item->extra_descr; ed != nullptr; ed = ed_next) {
				ed_next = ed->next;

				if (ed->keyword == KEYWD_OWNER) {
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

		act("OK, $t now belongs to no one in particular.", ch, item->short_descr, nullptr, TO_CHAR, POS_DEAD, FALSE);
		return;
	}

	/* Made it to where you have to "onwer item none" before assigning a new onwer - Lotus  */
	if (item->extra_descr != nullptr) {
		ExtraDescr *ed_next;

		for (ed = item->extra_descr; ed != nullptr; ed = ed_next) {
			ed_next = ed->next;

			if (ed->keyword == KEYWD_OWNER) {
				ptc(ch, "This item is already owned by %s.\n", ed->description);
				return;
			}
		}
	}

	act("OK, $t now belongs to $N.", ch, item->short_descr, player, TO_CHAR, POS_DEAD, FALSE);
	ed                      = new_extra_descr();
	ed->keyword             = KEYWD_OWNER;
	ed->description         = player->name;
	ed->next                = item->extra_descr;
	item->extra_descr       = ed;
}

void do_peace(Character *ch, String argument)
{
	Character *rch;

	for (rch = ch->in_room->people; rch != nullptr; rch = rch->next_in_room) {
		if (rch->fighting != nullptr)
			stop_fighting(rch, TRUE);

		if (IS_NPC(rch) && rch->act_flags.has(ACT_AGGRESSIVE))
			rch->act_flags -= ACT_AGGRESSIVE;
	}

	stc("The room is suddenly tranquil.\n", ch);
	return;
}

void do_purge(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Character *victim;
	Object *obj;
	Descriptor *d;

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		Character *vnext;
		Object  *obj_next;

		for (victim = ch->in_room->people; victim != nullptr; victim = vnext) {
			vnext = victim->next_in_room;

			if (IS_NPC(victim)
			    && !victim->act_flags.has(ACT_NOPURGE)
			    &&  victim != ch)
				extract_char(victim, TRUE);
		}

		for (obj = ch->in_room->contents; obj != nullptr; obj = obj_next) {
			obj_next = obj->next_content;

			if (!IS_OBJ_STAT(obj, ITEM_NOPURGE))
				extract_obj(obj);
		}

		Format::sprintf(buf, "$N has purged room: %d.", ch->in_room->vnum);
		wiznet(buf, ch, nullptr, WIZ_PURGE, WIZ_SECURE, GET_RANK(ch));
		act("$n purges the room of all objects!", ch, nullptr, nullptr, TO_ROOM);
		stc("The room has been purged.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		if (ch == victim) {
			stc("'Quit' usually works better.\n", ch);
			return;
		}

		if (!OUTRANKS(ch, victim) && !victim->pcdata->plr_flags.has(PLR_LINK_DEAD)) {
			Format::sprintf(buf, "$N has tried to purge the immortal: %s", victim->name);
			wiznet(buf, ch, nullptr, WIZ_PURGE, WIZ_SECURE, GET_RANK(ch));
			stc("Maybe that wasn't a good idea...\n", ch);
			ptc(victim, "%s just tried to purge you!\n", ch->name);
			return;
		}

		Format::sprintf(buf, "$N has purged the player: %s", victim->name);
		wiznet(buf, ch, nullptr, WIZ_PURGE, WIZ_SECURE, GET_RANK(ch));
		act("$n disintegrates $N.", ch, nullptr, victim, TO_NOTVICT);
		act("You disintegrate $N.", ch, nullptr, victim, TO_CHAR);

		if (victim->level > 1)
			save_char_obj(victim);

		d = victim->desc;
		extract_char(victim, TRUE);

		if (d != nullptr)
			close_socket(d);

		return;
	}

	if (get_mob_here(ch, victim->name, VIS_CHAR) == nullptr) {
		stc("There are no mobiles in this room by that name.\n", ch);
		return;
	}

	Format::sprintf(buf, "$N has purged the mobile: %s", victim->short_descr);
	wiznet(buf, ch, nullptr, WIZ_PURGE, WIZ_SECURE, GET_RANK(ch));
	act("$n disintegrates $N.", ch, nullptr, victim, TO_NOTVICT);
	act("You disintegrate $N.", ch, nullptr, victim, TO_CHAR);
	extract_char(victim, TRUE);
}

int has_enough_qps(Character *ch, int number_of)
{
	if (ch->questpoints >= number_of || IS_IMMORTAL(ch))
		return 1;

	return 0;
}

void do_qpconv(Character *ch, String argument)
{
	int number_of = 0, qpcost = 0, what = 0;
	char buffer[1024];
	Character *victim;

	String arg1, arg2, arg3;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	arg3 = argument;

	if (arg1.empty() || arg2.empty() || arg3.empty()) {
		stc("Syntax: qpconv <character> <train|prac> <number of>.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == nullptr) {
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

	if (arg2 == "train") {
		qpcost = number_of * QPS_PER_TRAIN;
		what = 1;
	}

	if (arg2 == "practice") {
		qpcost = number_of * QPS_PER_PRAC;
		what = 2;
	}

	if (what == 0) {
		stc("{x I don't understand what you are trying to convert.. use 'train' or 'practice'.\n", ch);
		return;
	}

	if (has_enough_qps(victim, qpcost) == 0) {
		Format::sprintf(buffer, "{x%s doesn't seem to have enough the required %d questpoints.\n", victim->name, qpcost);
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

	Format::sprintf(buffer, "{x%d questpoints deducted, %d %s%sadded.\n", qpcost, number_of, (what == 1) ? "train" : "practice",
	        (number_of) > 1 ? "s " : " ");
	stc(buffer, victim);
	stc("{xConversion was successful.\n", ch);
	return;
}

void restore_char(Character *ch, Character *victim)
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
	act("$n has restored you.", ch, nullptr, victim, TO_VICT, POS_SLEEPING, FALSE);
}

void do_restore(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Character *victim;
	Descriptor *d;

	String arg;
	one_argument(argument, arg);

	if (arg.empty() || arg == "room") {
		/* cure room */
		for (victim = ch->in_room->people; victim != nullptr; victim = victim->next_in_room)
			restore_char(ch, victim);

		Format::sprintf(buf, "$N has restored room: %d.", ch->in_room->vnum);
		wiznet(buf, ch, nullptr, WIZ_RESTORE, WIZ_SECURE, GET_RANK(ch));
		stc("Room restored.\n", ch);
		return;
	}

	if (arg == "all") {
		/* cure all */
		for (d = descriptor_list; d != nullptr; d = d->next)
			if (d->character != nullptr && !IS_NPC(d->character))
				restore_char(ch, d->character);

		stc("All players restored.\n", ch);
		wiznet("$N has restored all.", ch, nullptr, WIZ_RESTORE, WIZ_SECURE, GET_RANK(ch));
		return;
	}

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	restore_char(ch, victim);
	Format::sprintf(buf, "$N has restored: %s", IS_NPC(victim) ? victim->short_descr : victim->name);
	wiznet(buf, ch, nullptr, WIZ_RESTORE, WIZ_SECURE, GET_RANK(ch));
	stc("The player has been restored.\n", ch);
}

/* Secure levels by Lotus */
void do_secure(Character *ch, String argument)
{
	String arg;
	one_argument(argument, arg);

	if (arg.empty() || arg.is_prefix_of("immortal")) {
		ch->secure_level = RANK_IMM;
		stc("All immortals can see your immtalks now.\n", ch);
	}
	else if (arg.is_prefix_of("head") || arg.is_prefix_of("deputy")) {
		ch->secure_level = RANK_HEAD;
		stc("Only heads of departments and implementors can see your immtalks now.\n", ch);
	}
	else if (arg.is_prefix_of("implementor") && IS_IMP(ch)) {
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
void setgameinout(Character *ch, String argument, const String& entryexit, char flag)
{
	Character *victim;
	String *msgptr;

	String arg;
	argument = one_argument(argument, arg);

	if (! arg[0]) {
		act("Set game $t message for whom?", ch, entryexit, nullptr, TO_CHAR);
		return;
	}

	victim = get_player_world(ch, arg, VIS_PLR);

	if (victim == nullptr) {
		act("No player named '$t' found in the game!", ch, arg, nullptr, TO_CHAR);
		return;
	}

	if (flag == 'I')
		msgptr = &victim->pcdata->gamein;
	else
		msgptr = &victim->pcdata->gameout;

	if (argument.empty()) {    /* clear flag */
		msgptr->erase();
		act("OK, $N no longer has a game $t string",
		    ch, entryexit, victim, TO_CHAR);
	}
	else if (argument.uncolor().size() > 70) {
		act("This message is too long. Please try to think of something shorter!",
		    ch, nullptr, nullptr, TO_CHAR);
		stc("THIS is how long the message is allowed to be:\n", ch);
		act("setgamein $N -----------------------------------"
		    "-----------------------------------", ch, nullptr, victim, TO_CHAR);
		return;
	}
	else {
		*msgptr = argument;
		act("OK, game $t message set", ch, entryexit, nullptr, TO_CHAR);
	}
} /* end setgameinout() */

/* Set the game entry message for a player -- Elrac */
void do_setgamein(Character *ch, String argument)
{
	setgameinout(ch, argument, "entry", 'I');
}

/* Set the game exit message for a player -- Elrac */
void do_setgameout(Character *ch, String argument)
{
	setgameinout(ch, argument, "exit", 'O');
}

void do_sockets(Character *ch, String argument)
{
	Descriptor *d, *dmult;
	Character *vch;
	Player *vpc, *vpc_next;
	String buffer;
	char status[MAX_STRING_LENGTH];
	char s[100];
	bool multiplay = FALSE;
	int count = 0, ldcount = 0;

	String arg;
	one_argument(argument, arg);
	buffer += "\n{PNum{x|{YConnected_State{x| {BLogin{x |{CIdl{x|{GPlayer  Name{x|{WHost{x\n";
	buffer += "---|---------------|-------|---|------------|-------------------------\n";

	for (d = descriptor_list; d != nullptr; d = d->next) {
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

		if (d->character == nullptr) {
			/* no character known -- show it in SOCKETS anyway */
			buffer += Format::format("{P%3d{x|{Y%s{x|       |{C%2d{x |{G%-12s{x|{W%s{x\n",
			    d->descriptor,
			    status,
			    UMAX(0, d->timer),
			    d->original  ? d->original->name  :
			    d->character ? d->character->name : "(none)",
			    d->host);
		}
		else if (can_see_char(ch, d->character)
		         && (arg.empty()
		             || d->character->name.has_words(arg)
		             || (d->original && d->original->name.has_words(arg)))) {
			count++;
			/* check for multiplayers -- Montrey */
			multiplay = FALSE;

			if (d->connected == 0)
				for (dmult = descriptor_list; dmult != nullptr; dmult = dmult->next) {
					if (dmult == d || dmult->connected != 0) /* if not playing */
						continue;

					if (dmult->host == d->host)
						multiplay = TRUE;
				}

			/* Format "login" value... */
			vch = d->original ? d->original : d->character;
			strftime(s, 100, "%I:%M%p", localtime(&vch->logon));
			buffer += Format::format("{P%3d{x|{Y%s{x|{B%7s{x|{C%2d{x |{G%-12s{x%s%s{x\n",
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

	buffer += "---|---------------|-------|---|------------|-------------------------\n";

	/* now list linkdead ppl */
	for (vpc = pc_list; vpc != nullptr; vpc = vpc_next) {
		vpc_next = vpc->next;

		if (vpc->ch != ch
		    && vpc->plr_flags.has(PLR_LINK_DEAD)
		    && can_see_char(ch, vpc->ch)
		    && (arg.empty()
		        || vpc->ch->name.has_words(arg))) {
			strftime(s, 100, "%I:%M%p", localtime(&vpc->ch->logon));
			buffer += Format::format("{P---{x|{Y   Linkdead    {x|{B%7s{x|{C%-2d{x |{G%-12s{x|{W%s{x\n",
			    s,
			    UMAX(0, vpc->ch->desc == nullptr ? vpc->ch->timer : vpc->ch->desc->timer),
			    vpc->ch->name,
			    vpc->last_lsite);
			ldcount++;
		}
	}

	if (ldcount)
		buffer += "---|---------------|-------|---|------------|-------------------------\n";

	if (!count && !ldcount) {
		if (arg.empty())
			stc("No one is connected...\n", ch);
		else
			stc("No one by that name is connected.\n", ch);

		return;
	}

	buffer += Format::format("%d user%s connected", count, count == 1 ? "" : "s");

	if (ldcount)
		buffer += Format::format(", %d user%s linkdead.\n", ldcount, ldcount == 1 ? "" : "s");
	else
		buffer += ".\n";

	page_to_char(buffer, ch);
}

void do_storage(Character *ch, String argument)
{
	StoredPlayer *i;

	if (argument.empty()) {
		stc("Syntax:\n"
		    "  storage list                 (lists all characters in storage)\n"
		    "  storage store <player>       (puts a character in storage)\n"
		    "  storage retrieve <player>    (retrieves a character from storage)\n", ch);
		return;
	}

	String arg1;
	argument = one_argument(argument, arg1); /* storage command */

	if (arg1 == "list") {
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

	if (arg1 == "store") {
		char pfile_buf[128];
		char storage_buf[128];
		char command_buf[255];
		FILE *fp;
		StoredPlayer *newdata;

		if (argument.empty()) {
			stc("Store who?\n", ch);
			return;
		}

		if (lookup_storage_data(argument) != nullptr) {
			stc("That character is already in storage!\n", ch);
			return;
		}

		Format::sprintf(pfile_buf, "%s%s", PLAYER_DIR, argument.capitalize());
		Format::sprintf(storage_buf, "%s%s", STORAGE_DIR, argument.capitalize());

		if ((fp = fopen(pfile_buf, "r")) == nullptr) {
			stc("No such character!\n", ch);
			return;
		}

		fclose(fp);
		Format::sprintf(command_buf, "mv %s %s", pfile_buf, storage_buf);

		if (system(command_buf) != 0) {
			ptc(ch, "Error trying to move %s into storage!.\n", argument);
			return;
		}

		newdata = new StoredPlayer;
		newdata->name = argument.capitalize();
		newdata->by_who = ch->name;
		newdata->date = ctime(&current_time);
		newdata->date[strlen(newdata->date) - 1] = '\0';
		insert_storagedata(newdata);
		save_storage_list();
		ptc(ch, "Character %s successfully put into storage.\n", argument.capitalize());
		return;
	}

	if (arg1 == "retrieve") {
		char pfile_buf[128];
		char storage_buf[128];
		char command_buf[255];
		StoredPlayer *olddata;

		if (argument.empty()) {
			stc("Store who?\n", ch);
			return;
		}

		olddata = lookup_storage_data(argument);

		if (olddata == nullptr) {
			stc("No such character is currently in storage!\n", ch);
			return;
		}

		Format::sprintf(pfile_buf, "%s%s", PLAYER_DIR, argument.capitalize());
		Format::sprintf(storage_buf, "%s%s", STORAGE_DIR, argument.capitalize());
		Format::sprintf(command_buf, "mv %s %s", storage_buf, pfile_buf);

		if (system(command_buf) == 0) {
			ptc(ch, "%s has successfully been removed from storage.\n", argument.capitalize());
			remove_storagedata(olddata);
			save_storage_list();
		}
		else
			ptc(ch, "Error while trying to retrieve '%s'.\n", argument.capitalize());

		return;
	}

	do_storage(ch, "");
}

void do_invis(Character *ch, String argument)
{
	/* take the default path */
	if (ch->invis_level) {
		ch->invis_level = 0;
		act("$n slowly fades into existence.", ch, nullptr, nullptr, TO_ROOM);
		stc("You slowly fade back into existence.\n", ch);
	}
	else {
		ch->invis_level = LEVEL_IMMORTAL;
		act("$n slowly fades into thin air.", ch, nullptr, nullptr, TO_ROOM);
		stc("You slowly vanish into thin air.\n", ch);
	}
}

void do_superwiz(Character *ch, String argument)
{
	if (ch->act_flags.has(PLR_SUPERWIZ)) {
		stc("You return to reality.\n", ch);
		ch->act_flags -= PLR_SUPERWIZ;
	}
	else {
		stc("You vanish from existence.\n", ch);
		ch->act_flags += PLR_SUPERWIZ;
	}
}

RoomPrototype *find_location(Character *ch, const String& arg)
{
	Character *victim;
	Object *obj;

	if (arg.is_number())
		return get_room_index(atoi(arg));

	if ((victim = get_char_world(ch, arg, VIS_CHAR)) != nullptr)
		return victim->in_room;

	if ((obj = get_obj_world(ch, arg)) != nullptr)
		return obj->in_room;

	return nullptr;
}

void do_transfer(Character *ch, String argument)
{
	RoomPrototype *location;
	Descriptor *d;
	Character *victim;

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1.empty()) {
		stc("Transfer whom (and where)?\n", ch);
		return;
	}

	if (arg1 == "all") {
		for (d = descriptor_list; d != nullptr; d = d->next) {
			if (IS_PLAYING(d)
			    &&   d->character != ch
			    &&   d->character->in_room != nullptr
			    &&   can_see_char(ch, d->character)) {
				char buf[MAX_STRING_LENGTH];
				Format::sprintf(buf, "%s %s", d->character->name, arg2);
				do_transfer(ch, buf);
			}
		}

		return;
	}

	/*
	 * Thanks to Grodyn for the optional location parameter.
	 */
	if (arg2.empty())
		location = ch->in_room;
	else {
		if ((location = find_location(ch, arg2)) == nullptr) {
			stc("No such location.\n", ch);
			return;
		}

		if (!is_room_owner(ch, location) && room_is_private(location)
		    &&  !IS_IMP(ch)) {
			stc("That room is private.\n", ch);
			return;
		}
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (!IS_NPC(victim) && IS_IMMORTAL(victim) && !IS_IMP(ch)) {
		stc("They wouldn't like that.\n", ch);
		return;
	}

	if (victim->in_room == nullptr) {
		stc("They are in limbo.\n", ch);
		return;
	}

	if (victim->fighting != nullptr)
		stop_fighting(victim, TRUE);

	act("$n screams as $e is sucked off into the clouds.", victim, nullptr, nullptr, TO_ROOM);
	char_from_room(victim);
	char_to_room(victim, location);
	act("$n breaks through the clouds and crash lands at your feet.", victim, nullptr, nullptr, TO_ROOM);

	if (ch != victim)
		ptc(victim, "%s has transported you.\n", PERS(ch, victim, VIS_CHAR).capitalize());

	do_look(victim, "auto");
	stc("Transfer Successful.\n", ch);
}

void do_violate(Character *ch, String argument)
{
	Object *obj;
	RoomPrototype *location = nullptr;
	Character *rch;

	if (argument.empty()) {
		stc("Which private room do you wish to violate?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg.is_number())
		location = get_room_index(atoi(arg));
	else {
		if ((rch = get_char_world(ch, argument, VIS_CHAR)) != nullptr)
			location = rch->in_room;

		if (arg.is_prefix_of("object") || location == nullptr) {
			location = nullptr;

			if ((obj = get_obj_world(ch, argument)) != nullptr)
				if (obj->in_room)
					location = obj->in_room;
		}
	}

	if (location == nullptr) {
		stc("No such location.\n", ch);
		return;
	}

	if (!room_is_private(location)) {
		stc("That room isn't private, use Goto.\n", ch);
		return;
	}

	if (ch->fighting != nullptr)
		stop_fighting(ch, TRUE);

	for (rch = ch->in_room->people; rch != nullptr; rch = rch->next_in_room) {
		if (can_see_char(rch, ch)) {
			if (ch->pcdata != nullptr && ch->pcdata->bamfout[0] != '\0')
				act("$t", ch, ch->pcdata->bamfout, rch, TO_VICT);
			else
				act("$n warps off to distant lands in a burst of electric energy.", ch, nullptr, rch, TO_VICT);
		}
	}

	char_from_room(ch);
	char_to_room(ch, location);

	for (rch = ch->in_room->people; rch != nullptr; rch = rch->next_in_room) {
		if (can_see_char(rch, ch)) {
			if (ch->pcdata != nullptr && ch->pcdata->bamfin[0] != '\0')
				act("$t", ch, ch->pcdata->bamfin, rch, TO_VICT);
			else
				act("$n materializes in a burst of electric energy.", ch, nullptr, rch, TO_VICT);
		}
	}

	do_look(ch, "auto");
	return;
}

/* Command groups - Command to give/take */
void do_wizgroup(Character *ch, String argument)
{
	Character *victim;
	int count = 0;
	bool add = FALSE, all = FALSE, found = FALSE;

	String arg1, arg2, arg3;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);

	if (arg1.empty() || arg2.empty() || arg3.empty()) {
		stc("wizgroup <add/remove> <player> <group>\n", ch);
		stc("Valid groups are:\n", ch);
		stc("  general quest build code security\n"
		    "  player clan avatar hero leader\n", ch);
		return;
	}

	if ((victim = get_player_world(ch, arg2, VIS_PLR)) == nullptr) {
		stc("Player not found.\n", ch);
		return;
	}

	if (arg1.is_prefix_of("add"))
		add = TRUE;
	else if (arg1.is_prefix_of("remove"))
		add = FALSE;
	else {
		do_wizgroup(ch, "");
		return;
	}

	if (arg3 == "all") {
		stc("The `all' option is no longer enabled.\n", ch);
		return;
	}

	/* loops == good!  else ifs == bad! :) */
	while (count < cgroup_flags.size()) {
		if (arg3.is_prefix_of(cgroup_flags[count].name) || all) {
			found = TRUE;

			if (add)
				victim->add_cgroup(cgroup_flags[count].bit);
			else
				victim->remove_cgroup(cgroup_flags[count].bit);

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
void do_wizify(Character *ch, String argument)
{
	char strsave[MAX_INPUT_LENGTH];
	FILE *fp;
	Character *victim;
	int sn;

	String arg1;
	one_argument(argument, arg1);

	if (arg1.empty()) {
		stc("You must provide a name.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1, VIS_CHAR)) == nullptr) {
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
	Format::sprintf(strsave, "%s%s%s", BACKUP_DIR, victim->name.capitalize(), "GOD.gz");

	if ((fp = fopen(strsave, "r")) != nullptr)
		fclose(fp);
	else {
		backup_char_obj(ch);
		stc("Your pfile has been backed up.\n", victim);
		String capname = victim->name.capitalize();
		Format::sprintf(strsave, "mv %s%s.gz %s%sGOD.gz",
		        BACKUP_DIR, capname, BACKUP_DIR, capname);
		system(strsave);
	}

	victim->level                   = MAX_LEVEL;
	victim->hit  = ATTR_BASE(victim, APPLY_HIT)        = 30000;
	victim->mana = ATTR_BASE(victim, APPLY_MANA)       = 30000;
	victim->stam = ATTR_BASE(victim, APPLY_STAM)       = 30000;

	for (int stat = 0; stat < MAX_STATS; stat++)
		ATTR_BASE(victim, stat_to_attr(stat)) = 25;

	for (sn = 0; sn < skill_table.size(); sn++)
		victim->pcdata->learned[sn] = 100;

	victim->remove_cgroup(GROUP_LEADER);
	victim->remove_cgroup(GROUP_DEPUTY);
	victim->clan = nullptr;
	update_pos(victim);
	stc("You have created a god.\n", ch);
	stc("You suddenly feel like a GOD!\n", victim);
	save_char_obj(victim);
}

/* Aura command stolen from rank - Lotus */
void do_aura(Character *ch, String argument)
{
	Character *victim;

	String arg1;
	argument = one_argument(argument, arg1);

	if (IS_NPC(ch)) {
		do_huh(ch);
		return;
	}

	if (arg1.empty()) {
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

	if ((victim = get_player_world(ch, arg1, VIS_PLR)) == nullptr) {
		stc("That player is not logged on.\n", ch);
		return;
	}

	if (argument.empty()) {
		ptc(ch, "Their aura is currently {W(%s{W){x.\n", victim->pcdata->aura);
		return;
	}

	if (argument == "none") {
		stc("Aura removed.\n", ch);
		stc("Your aura has been removed.\n", victim);
		victim->pcdata->aura.erase();
		return;
	}

	if (argument.uncolor().size() > 30) {
		stc("An aura cannot be longer than 30 printed characters.\n", ch);
		return;
	}

	if (strlen(argument) > 100) {
		stc("An aura cannot be longer than 100 total characters.\n", ch);
		return;
	}

	victim->pcdata->aura = argument;
	ptc(victim, "Your aura is now {W(%s{W){x.\n", victim->pcdata->aura);
	ptc(ch, "Their aura is now {W(%s{W){x.\n", victim->pcdata->aura);
}

void do_bamfin(Character *ch, String argument)
{
	if (argument.empty()) {
		ptc(ch, "Your poofin is currently: %s\n", ch->pcdata->bamfin);
		return;
	}

	if (!argument.uncolor().has_exact_words(ch->name)) {
		stc("You must include your name in a poofin.\n", ch);
		return;
	}

	if (strlen(argument) > 100) {
		stc("Poofins cannot be longer than 100 printed characters.\n", ch);
		return;
	}

	if (argument.uncolor().size() > 300) {
		stc("Poofins cannot be longer than 300 characters, counting color codes.\n", ch);
		return;
	}

	ch->pcdata->bamfin = argument;
	ptc(ch, "Your poofin is now: %s\n", ch->pcdata->bamfin);
}

void do_bamfout(Character *ch, String argument)
{
	if (argument.empty()) {
		ptc(ch, "Your poofout is currently: %s\n", ch->pcdata->bamfout);
		return;
	}

	if (!argument.uncolor().has_exact_words(ch->name)) {
		stc("You must include your name in a poofout.\n", ch);
		return;
	}

	if (strlen(argument) > 100) {
		stc("Poofouts cannot be longer than 100 printed characters.\n", ch);
		return;
	}

	if (argument.uncolor().size() > 300) {
		stc("Poofouts cannot be longer than 300 characters, counting color codes.\n", ch);
		return;
	}

	ch->pcdata->bamfout = argument;
	ptc(ch, "Your poofout is now: %s\n", ch->pcdata->bamfout);
}

void do_clanqp(Character *ch, String argument)
{
	char buf[MSL];
	Clan *target;
	int qp_amount = 0;

	if (argument.empty()) {
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

	if (argument.empty()) {
		stc("See 'help clanqp' on how this command works!\n", ch);
		return;
	}

	String arg1, arg2, arg3;
	argument = one_argument(argument, arg1); /* donate/withdraw     */
	argument = one_argument(argument, arg2); /* qp_amount           */
	argument = one_argument(argument, arg3); /* clanname            */

	if (!IS_IMMORTAL(ch))
		target = ch->clan;
	else
		target = (arg3.empty() ? ch->clan : clan_lookup(arg3));

	if (!target) {
		stc("There is no such clan.\n", ch);
		return;
	}

	if (target->independent) {
		stc("You can't donate questpoints to that clan!\n", ch);
		return;
	}

	if (!arg2.is_number()) {
		stc("You must specify an amount of questpoints.\n", ch);
		return;
	}

	if ((qp_amount = atoi(arg2)) < 1 || qp_amount > 30000) {
		stc("The number of questpoints must be between 1 and 30000.\n", ch);
		return;
	}

	if (arg1.is_prefix_of("donate")) {
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
			Format::sprintf(buf, "{W[ %s just donated %d questpoint%s to the clan! ]{x\n",
			        ch->name, qp_amount, qp_amount > 1 ? "s" : "");
			send_to_clan(ch, target, buf);
			ptc(ch, "You donate %d questpoint%s to the clan.\n",
			    qp_amount, qp_amount > 1 ? "s" : "");
		}

		return;
	}

	if (arg1.is_prefix_of("withdraw") || arg1.is_prefix_of("deduct")) {
		if (!IS_IMMORTAL(ch)) {
			stc("Only immortals are allowed to do that!\n", ch);
			return;
		}

		if (argument.empty()) {
			stc("You have to provide a reason!\n", ch);
			return;
		}

		if (target->clanqp < qp_amount) {
			stc("The clan is not rich enough!\n", ch);
			return;
		}

		target->clanqp -= qp_amount;
		Format::sprintf(buf, "{W[ %s has deducted %d questpoint%s for %s ]\n",
		        ch->name, qp_amount, qp_amount > 1 ? "s" : "", argument);
		send_to_clan(ch, target, buf);
		ptc(ch, "You deduct %d questpoints from the %s{x.\n", qp_amount, target->clanname);
		save_clan_table();
		return;
	}

	stc("See 'help clanqp' on how this command works!\n", ch);
}

