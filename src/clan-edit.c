/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Starfeldt, Tom Madsen, and Katja Nyboe.   *
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

/* Clan Editing Module,
 * (c) 1999, Johan Jonkers(aka Clerve) (scoobers@hotmail.com)
 */

#include "merc.h"
#include "lookup.h"
#include "sql.h"

#define CLAN_DIR        "../misc/"
#define CLAN_FILE       "clans.txt"

/* Forward declaration of functions in this module */
void append_clan(CLAN_DATA *);
void remove_clan(char *);
int count_clans();

/* Needed variables for using the list */
struct clan_type *clan_table_head;
struct clan_type *clan_table_tail;

int count_clan_members(CLAN_DATA *clan, int bit)
{
	char query[MSL];
	int count;

	if (bit != 0 && bit != GROUP_LEADER && bit != GROUP_DEPUTY)
		return 0;

	sprintf(query, "SELECT COUNT(*) FROM pc_index WHERE clan LIKE '%s'", db_esc(clan->name));

	if (bit != 0) {
		char buf[MSL];
		sprintf(buf, " AND cgroup&%d", bit);
		strcat(query, buf);
	}

	if ((count = db_count("count_clan_members", query)) == -1)
		return 0;

	return count;
}

void load_clan_table()
{
	CLAN_DATA *clan;
	int count = 0;
	clan_table_head                 = alloc_mem(sizeof(CLAN_DATA));
	clan_table_tail                 = alloc_mem(sizeof(CLAN_DATA));

	/* Watch out for memory errors -- Outsider */
	if ((!clan_table_head) || (!clan_table_tail)) {
		printf("load_clan_table: Error on memory alloc.\n");
		return;
	}

	clan_table_head->next           = clan_table_tail;
	clan_table_tail->previous       = clan_table_head;

	if (db_query("load_clan_table",
	                       "SELECT current, name, who_name, clanname, creator, hall, minvnum, maxvnum, "
	                       "independent, clanqp, gold, score, warcpmod FROM clans") != SQL_OK)
		return;

	while (db_next_row() == SQL_OK) {
		/* the first field is 'current', old clans aren't wiped out */
		/*
		I don't care if they are current. the database is always corrupted.
		-- Outsider
		  if (atoi(row[0]) == 0)
		          continue;
		*/
		if ((clan = alloc_mem(sizeof(CLAN_DATA))) == NULL) {
			bug("load_clan_table: unable to allocate memory for new clan", 0);
			return;
		}

		clan->name              = str_dup(db_get_column_str(1));
		clan->who_name          = str_dup(db_get_column_str(2));
		clan->clanname          = str_dup(db_get_column_str(3));
		clan->creator           = str_dup(db_get_column_str(4));
		clan->hall              = db_get_column_int(5);
		clan->area_minvnum      = db_get_column_int(6);
		clan->area_maxvnum      = db_get_column_int(7);
		clan->independent       = db_get_column_int(8);
		clan->clanqp            = db_get_column_int(9);
		clan->gold_balance      = db_get_column_int(10);
		clan->score             = db_get_column_int(11);
		clan->warcpmod          = db_get_column_int(12);
		append_clan(clan);
		printf("Loaded clan '%s'\n", clan->name);
		count++;
	}

	printf("Total of %d clans loaded.\n", count);
}

void save_clan_table()
{
	CLAN_DATA *clan;
	int count;
	/* first, set all entries to current 0 */
	db_command("save_clan_table", "UPDATE clans SET current=0");

	for (clan = clan_table_head->next; clan != clan_table_tail; clan = clan->next) {
		if ((count = db_countf("save_clan_table",
		                       "SELECT COUNT(*) FROM clans WHERE name='%s'", clan->name)) == -1)
			return;

		if (!count) {
			db_commandf("save_clan_table", "INSERT INTO clans VALUES("
			            "1,'%s','%s','%s','%s',%d,%d,%d,%d,%ld,%ld,%d,%d)",
			            db_esc(clan->name),
			            db_esc(clan->who_name),
			            db_esc(clan->clanname),
			            db_esc(clan->creator),
			            clan->hall,
			            clan->area_minvnum,
			            clan->area_maxvnum,
			            clan->independent,
			            clan->clanqp,
			            clan->gold_balance,
			            clan->score,
			            clan->warcpmod);
			continue;
		}

		db_commandf("save_clan_table", "UPDATE clans SET current=1, name='%s', who_name='%s', "
		            "clanname='%s', creator='%s', hall=%d, minvnum=%d, maxvnum=%d, independent=%d, "
		            "clanqp=%ld, gold=%ld, score=%d, warcpmod=%d WHERE name='%s'",
		            db_esc(clan->name),
		            db_esc(clan->who_name),
		            db_esc(clan->clanname),
		            db_esc(clan->creator),
		            clan->hall,
		            clan->area_minvnum,
		            clan->area_maxvnum,
		            clan->independent,
		            clan->clanqp,
		            clan->gold_balance,
		            clan->score,
		            clan->warcpmod,
		            clan->name);
	}
}

/* Append clan_info to the end of the list */
void append_clan(CLAN_DATA *c)
{
	c->previous = clan_table_tail->previous;
	c->previous->next = c;
	c->next = clan_table_tail;
	clan_table_tail->previous = c;
}

/* Count the number of clans */
int count_clans()
{
	CLAN_DATA *iterator;
	int clans = 0;
	iterator = clan_table_head->next;

	while (iterator != clan_table_tail) {
		clans++;
		iterator = iterator->next;
	}

	return clans;
}

/* calculate a clan's power */
int calc_cp(CLAN_DATA *clan, bool curve)
{
	int members = count_clan_members(clan, 0), clanpower;
	clanpower       = ((members / 2) + (clan->gold_balance / 10000) + (clan->clanqp / 100) + clan->warcpmod);

	/* curve it */
	if (curve && clanpower != 30 && clanpower > 0) {
		int modtotal = clanpower, curved;
		long mod;

		if (clanpower < 30)
			modtotal = (30 - clanpower) + 30;

		mod = ((modtotal * 1000) + 10000) / (modtotal - 10);
		curved = (((modtotal - 30) * mod) / 1000) - (modtotal - 30);

		if (clanpower < 30)
			clanpower = 30 - curved;
		else
			clanpower = 30 + curved;
	}

	return clanpower;
}

/* Remove a clan */
void remove_clan(char *name)
{
	CLAN_DATA *iterator;
	iterator = clan_table_head->next;

	while (iterator != clan_table_tail) {
		if (!strcasecmp(iterator->name, name)) {
			CLAN_DATA *p = iterator->previous;
			CLAN_DATA *n = iterator->next;
			p->next = n;
			n->previous = p;
			free_mem(iterator, sizeof(CLAN_DATA));
			return;
		}

		iterator = iterator->next;
	}
}

void do_cedit(CHAR_DATA *ch, const char *argument)
{
	char cmd[MAX_INPUT_LENGTH], clanname[MAX_INPUT_LENGTH];
	CLAN_DATA *cdata;
	argument = one_argument(argument, cmd);
	argument = one_argument(argument, clanname);

	if (!cmd[0]) {
		stc("Huh? type HELP CEDIT to see syntax.\n", ch);
		return;
	}

	if (!clanname[0]) {
		stc("What clan do you want to operate on?\n", ch);
		return;
	}

	cdata = clan_lookup(clanname);

	if (str_cmp(cmd, "new") && (cdata == NULL)) {
		stc("No such clan exists.\n", ch);
		return;
	}

	if (!str_cmp(cmd, "delete")) {
		if (cdata == NULL)
			stc("You need to specify a clanname.\n", ch);
		else {
			remove_clan(cdata->name);
			stc("Clan deleted.\n", ch);
			save_clan_table();
		}

		return;
	}

	if (!str_cmp(cmd, "new")) {
		CLAN_DATA *new_clan = alloc_mem(sizeof(CLAN_DATA));

		if (new_clan == NULL) {
			bug("Unable to allocate memory for new clan!", 0);
			stc("Allocation of memory for the new clan failed!\n", ch);
			return;
		}

		if (cdata != NULL) {
			stc("A clan with that name already exists.\n", ch);
			return;
		}

		new_clan->name = str_dup(clanname);
		new_clan->who_name = str_dup("         ");
		new_clan->hall = 3001;
		new_clan->area_minvnum = 0;
		new_clan->area_maxvnum = 0;
		new_clan->independent = FALSE;
		new_clan->clanqp = 0;
		new_clan->gold_balance = 1000;
		new_clan->clanname = str_dup("");
		new_clan->creator = str_dup("");
		new_clan->score = 0;
		new_clan->warcpmod = 0;
		append_clan(new_clan);
		save_clan_table();
		stc("New clan added.\n", ch);
		return;
	}

	if (!str_cmp(cmd, "show")) {
		ptc(ch,     "{HClan: %s\n"
		    "{G[whoname]{c    The name that shows in the who-list:\n"
		    "{Y             %s\n"
		    "{G[hall]{c       The vnum of clanrecall:\n"
		    "{Y             %d\n"
		    "{G[areamin]{c    The lower vnum of the clanhall:\n"
		    "{Y             %d\n"
		    "{G[areamax]{c    The upper vnum of the clanhall:\n"
		    "{Y             %d\n"
		    "{G[indepedent]{c Loner or IMM clan:\n"
		    "{Y             %s\n"
		    "{G[clanqp]{c     Number of questpoints the clan has:\n"
		    "{Y             %ld\n"
		    "{G[clangold]{c   Number of gold coins the clan has:\n"
		    "{Y             %ld\n"
		    "{G[clanname]{c   Full name of the clan:\n"
		    "{Y             %s\n"
		    "{G[creator]{c    Name of the creator(s) of the clan:\n"
		    "{Y             %s\n"
		    "{G[score]{c      Points remaining until defeat in current war:\n"
		    "{Y             %d\n"
		    "{G[warcpmod]{c   Modification to power based on wars:\n"
		    "{Y             %d\n",
		    cdata->name,
		    cdata->who_name,
		    cdata->hall,
		    cdata->area_minvnum,
		    cdata->area_maxvnum,
		    cdata->independent ? "Yes" : "No",
		    cdata->clanqp,
		    cdata->gold_balance,
		    cdata->clanname,
		    cdata->creator,
		    cdata->score,
		    cdata->warcpmod);
		return;
	}

	if (!str_cmp(cmd, "whoname")) {
		free_string(cdata->who_name);
		cdata->who_name = str_dup(argument);

		if (!argument[0])
			stc("The who-name is set to none.\n", ch);
		else
			ptc(ch, "New whoname: '%s{x'.\n", cdata->who_name);

		save_clan_table();
		return;
	}

	if (!str_cmp(cmd, "hall")) {
		if (!argument[0] || !is_number(argument)) {
			stc("Clanrecall set to normal recall.\n", ch);
			cdata->hall = 3001;
		}
		else {
			if (!find_location(ch, argument))
				stc("No such location.\n", ch);
			else {
				cdata->hall = atoi(argument);
				ptc(ch, "Clanrecall set to %d.\n", cdata->hall);
			}
		}

		save_clan_table();
		return;
	}

	if (!str_cmp(cmd, "areamin")) {
		int vnum = -1;

		if (!argument[0] || !is_number(argument)) {
			stc("Areamin set to 0.\n", ch);
			cdata->area_minvnum = 0;
		}
		else {
			vnum = atoi(argument);

			if (vnum < 0) {
				stc("Areamin must be greater or equal to 0.\n", ch);
				return;
			}

			cdata->area_minvnum = vnum;
			ptc(ch, "Areamin set to %d.\n", cdata->area_minvnum);
		}

		save_clan_table();
		return;
	}

	if (!str_cmp(cmd, "areamax")) {
		int vnum = -1;

		if (!argument[0] || !is_number(argument)) {
			stc("Areamax set to 0.\n", ch);
			cdata->area_maxvnum = 0;
		}
		else {
			vnum = atoi(argument);

			if (vnum < 0) {
				stc("Areamax must be greater or equal to 0.\n", ch);
				return;
			}

			cdata->area_maxvnum = vnum;
			ptc(ch, "Areamax set to %d.\n", cdata->area_maxvnum);
		}

		save_clan_table();
		return;
	}

	if (!str_cmp(cmd, "independent")) {
		int independent = 0;

		if (!IS_IMP(ch)) {
			stc("You cannot make this an immortal clan.\n", ch);
			return;
		}

		if (!argument[0] || !is_number(argument)) {
			stc("Dependent set to 0 (not dependent)\n", ch);
			cdata->independent = 0;
		}
		else {
			independent = atoi(argument);

			if (independent < 0 || independent > 1) {
				stc("Use the values: 0=NO, 1=YES\n", ch);
				return;
			}

			cdata->independent = independent;

			if (independent)
				stc("The clan is now independent.\n", ch);
			else
				stc("The clan is now dependent.\n", ch);
		}

		save_clan_table();
		return;
	}

	if (!str_cmp(cmd, "clanqp")) {
		int clanqp = -1;

		if (!argument[0] || !is_number(argument)) {
			stc("You need to provide a numerical argument.\n", ch);
			return;
		}

		clanqp = atoi(argument);

		if (clanqp < 0) {
			stc("You need to provide a positive numerical argument.\n", ch);
			return;
		}

		cdata->clanqp = clanqp;
		ptc(ch, "Clanqp set to %ld.\n", cdata->clanqp);
		save_clan_table();
		return;
	}

	if (!str_cmp(cmd, "clangold")) {
		int gold_balance = -1;

		if (!argument[0] || !is_number(argument)) {
			stc("You need to provide a numerical argument.\n", ch);
			return;
		}

		gold_balance = atoi(argument);

		if (gold_balance < 0) {
			stc("You need to provide a positive numerical argument.\n", ch);
			return;
		}

		cdata->gold_balance = gold_balance;
		ptc(ch, "Clangold set to %ld.\n", cdata->gold_balance);
		save_clan_table();
		return;
	}

	if (!str_cmp(cmd, "clanname")) {
		free_string(cdata->clanname);
		cdata->clanname = str_dup(argument);

		if (!argument[0])
			stc("Clanname set to none.\n", ch);
		else
			ptc(ch, "Clanname set to '%s{x'.\n", cdata->clanname);

		save_clan_table();
		return;
	}

	if (!str_cmp(cmd, "creator")) {
		free_string(cdata->creator);
		cdata->creator = str_dup(argument);

		if (!argument[0])
			stc("Creator set to none.\n", ch);
		else
			ptc(ch, "Creator set to '%s{x'.\n", cdata->creator);

		save_clan_table();
		return;
	}

	if (!str_cmp(cmd, "score")) {
		int score = 0;

		if (!argument[0] || !is_number(argument)) {
			stc("You need to provide a numerical argument.\n", ch);
			return;
		}

		score = atoi(argument);

		if (score < 0) {
			stc("You need to provide a positive numerical argument.\n", ch);
			return;
		}

		cdata->score = score;
		ptc(ch, "Score set to %d.\n", cdata->score);
		save_clan_table();
		return;
	}

	if (!str_cmp(cmd, "warcpmod")) {
		int mod = 0;

		if (!argument[0] || !is_number(argument)) {
			stc("You need to provide a numerical argument.\n", ch);
			return;
		}

		mod = atoi(argument);
		cdata->warcpmod = mod;
		ptc(ch, "The clan now has their power modified by %d.\n", cdata->warcpmod);
		save_clan_table();
		return;
	}

	stc("Huh? type HELP CEDIT to see syntax.\n", ch);
}
