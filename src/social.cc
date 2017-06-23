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

/* Online Social Editting Module,
 * (c) 1996,97 Erwin S. Andreasen <erwin@pip.dknet.dk>
 * See the file "License" for important licensing information
 */

/* This version contains minor modifications to support ROM 2.4b4. */

/* Changes:
 * Socials are now a double linked list. Every social knows the next social
 * and the previous in the list.
 * Socials when added, get added in alphabetical order.
 * Changed sedit command to work with list, not array.
 * Xsocials added, losely based on Dystopia, but redone.
 * Social searching for unfinished socials.
 * Social renaming.
 */

#include "argument.hh"
#include "Character.hh"
#include "declare.hh"
#include "file.hh"
#include "Format.hh"
#include "interp.hh"
#include "Logging.hh"
#include "memory.hh"
#include "Social.hh"
#include "String.hh"

#define SOCIAL_FILE "../misc/social.txt"

void insert_social(Social *);
void remove_social(const String&);
int count_socials();

int maxSocial;

Social *social_table_head;     /* and social table */
Social *social_table_tail;     /* and social table */

void load_social(FILE *fp, Social *social)
{
	social->name =          fread_string(fp);
	social->char_no_arg =   fread_string(fp);
	social->others_no_arg = fread_string(fp);
	social->char_found =    fread_string(fp);
	social->others_found =  fread_string(fp);
	social->vict_found =    fread_string(fp);
	social->char_auto =     fread_string(fp);
	social->others_auto =   fread_string(fp);
}

void load_social_table()
{
	FILE *fp;
	int i;
	Social *new_social;
	fp = fopen(SOCIAL_FILE, "r");

	if (!fp) {
		Logging::bug("Could not open " SOCIAL_FILE " for reading.", 0);
		return;
	}

	fscanf(fp, "%d\n", &maxSocial);
	social_table_head = new Social;
	social_table_tail = new Social;
	social_table_head->next = social_table_tail;
	social_table_tail->previous = social_table_head;

	for (i = 0; i < maxSocial; i++) {
		new_social = new Social;
		load_social(fp, new_social);
		insert_social(new_social);
	}

	fclose(fp);
}

/*
 * May 9th by Clerve.
 * Inserting a social alphabeticly
 */
void insert_social(Social *s)
{
	Social *iterator;
	/* First element
	if (strlen(social_table->name)<1)
	{       social_table=s;
	        return;
	}
	*/
	/* find the right place */
	iterator = social_table_head->next;

	while (iterator != social_table_tail) {
		if (strcasecmp(s->name, iterator->name) < 1) {
			s->previous = iterator->previous;
			s->previous->next = s;
			s->next = iterator;
			iterator->previous = s;
			return;
		}

		iterator = iterator->next;
	}

	/* append it */
	iterator = social_table_tail->previous;
	iterator->next = s;
	s->previous = iterator;
	s->next = social_table_tail;
	social_table_tail->previous = s;
}

/*
 * May 9th by Clerve.
 * Removing a social
 */
void remove_social(const String& name)
{
	Social *iterator;

	for (iterator = social_table_head->next; iterator != social_table_tail; iterator = iterator->next) {
		if (!strcasecmp(name, iterator->name)) {
			Social *p = iterator->previous;
			Social *n = iterator->next;
			p->next = n;
			n->previous = p;
			delete iterator;
			return;
		}
	}
}

/*
 * May 9th by Clerve.
 * Counting all socials
 */
int count_socials()
{
	Social *iterator;
	int socials = 0;
	/* set to first social */
	iterator = social_table_head->next;

	while (iterator != social_table_tail) {
		socials++;
		iterator = iterator->next;
	}

	return socials;
}

void save_social(const Social *s, FILE *fp)
{
	/* get rid of (null) */
	Format::fprintf(fp, "%s~\n", s->name.replace("~", "-"));
	Format::fprintf(fp, "%s~\n", s->char_no_arg.replace("~", "-"));
	Format::fprintf(fp, "%s~\n", s->others_no_arg.replace("~", "-"));
	Format::fprintf(fp, "%s~\n", s->char_found.replace("~", "-"));
	Format::fprintf(fp, "%s~\n", s->others_found.replace("~", "-"));
	Format::fprintf(fp, "%s~\n", s->vict_found.replace("~", "-"));
	Format::fprintf(fp, "%s~\n", s->char_auto.replace("~", "-"));
	Format::fprintf(fp, "%s~\n\n", s->others_auto.replace("~", "-"));
}

void save_social_table()
{
	FILE *fp;
	Social *iterator;
	fp = fopen(SOCIAL_FILE, "w");

	if (!fp) {
		Logging::bug("Could not open " SOCIAL_FILE " for writing.", 0);
		return;
	}

	Format::fprintf(fp, "%d\n", count_socials());
	/* set to first social */
	iterator = social_table_head->next;

	while (iterator != social_table_tail) {
		save_social(iterator, fp);
		iterator = iterator->next;
	}

	fclose(fp);
}

/* Find a social based on name */
Social *social_lookup(const String& name)
{
	Social *iterator;
	/* set to first social */
	iterator = social_table_head->next;

	while (iterator != social_table_tail) {
		if (name == iterator->name)
			return iterator;

		iterator = iterator->next;
	}

	return nullptr;
}

/*
 * Social editting command
 */

void do_sedit(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Social *iSocial;

	String cmd, social;
	argument = one_argument(argument, cmd);
	argument = one_argument(argument, social);

	if (cmd.empty()) {
		stc("Huh? Type HELP SEDIT to see syntax.\n", ch);
		return;
	}

	if (social.empty()) {
		if (cmd == "find")
			stc("What do you wish to find?\n", ch);
		else
			stc("What social do you want to operate on?\n", ch);

		return;
	}

	iSocial = social_lookup(social);

	if (cmd != "new" && cmd != "find" && (iSocial == nullptr)) {
		stc("No such social exists.\n", ch);
		return;
	}

	if (cmd == "delete") { /* Remove a social */
		remove_social(iSocial->name);
		stc("That social is history now.\n", ch);
	}
	else if (cmd == "new") { /* Create a new social */
		if (iSocial != nullptr) {
			stc("A social with that name already exists.\n", ch);
			return;
		}

		for (int x = 0; x < cmd_table.size(); x++) {
			if (social.is_prefix_of(cmd_table[x].name)) {
				stc("A command with that name already exists.\n", ch);
				return;
			}
		}

		Social *new_social = new Social;

		new_social->name = social;
		insert_social(new_social);
		stc("New social added.\n", ch);
	}
	else if (cmd == "rename") { /* Rename a social */
		if (argument.empty()) {
			stc("Rename it to what?\n", ch);
			return;
		}

		if (social_lookup(argument) != nullptr) {
			stc("A social with that name already exists.\n", ch);
			return;
		}

		for (int x = 0; x < cmd_table.size(); x++) {
			if (argument.is_prefix_of(cmd_table[x].name)) {
				stc("A command with that name already exists.\n", ch);
				return;
			}
		}

		Social *new_social = new Social;

		new_social->name                = argument;
		new_social->char_no_arg         = iSocial->char_no_arg;
		new_social->others_no_arg       = iSocial->others_no_arg;
		new_social->char_found          = iSocial->char_found;
		new_social->others_found        = iSocial->others_found;
		new_social->vict_found          = iSocial->vict_found;
		new_social->char_auto           = iSocial->char_auto;
		new_social->others_auto         = iSocial->others_auto;
		insert_social(new_social);
		remove_social(iSocial->name);
		stc("Social renamed.\n", ch);
	}
	else if (cmd == "show") { /* Show a certain social */
		Format::sprintf(buf, "{HSocial: %s{x\n"
		        "{G[cnoarg]{c No argument given, character sees:\n"
		        "{Y         %s\n"
		        "{G[onoarg]{c No argument given, others see:\n"
		        "{Y         %s\n"
		        "{G[cfound]{c Target found, character sees:\n"
		        "{Y         %s\n"
		        "{G[ofound]{c Target found, others see:\n"
		        "{Y         %s\n"
		        "{G[vfound]{c Target found, victim sees:\n"
		        "{Y         %s\n"
		        "{G[cself]{c  Target is self:\n"
		        "{Y         %s\n"
		        "{G[oself]{c  Target is self, others see:\n"
		        "{Y         %s{x\n",
		        iSocial->name,
		        iSocial->char_no_arg,
		        iSocial->others_no_arg,
		        iSocial->char_found,
		        iSocial->others_found,
		        iSocial->vict_found,
		        iSocial->char_auto,
		        iSocial->others_auto);
		stc(buf, ch);
		return; /* return right away, do not save the table */
	}
	else if (cmd.is_prefix_of("find")) { /* Find a social */
		Social *i;
		bool fAll = FALSE;

		if (argument.empty())
			fAll = TRUE;

		if (social == "unfinished") {
			bool unfin;
			int count = 0;

			for (i = social_table_head->next; i != social_table_tail; i = i->next) {
				unfin = FALSE;

				if (i->char_no_arg == ""   && (fAll || argument == "cnoarg"))
					unfin = TRUE;

				if (i->others_no_arg == "" && (fAll || argument == "onoarg"))
					unfin = TRUE;

				if (i->char_found == ""    && (fAll || argument == "cfound"))
					unfin = TRUE;

				if (i->others_found == ""  && (fAll || argument == "ofound"))
					unfin = TRUE;

				if (i->vict_found == ""    && (fAll || argument == "vfound"))
					unfin = TRUE;

				if (i->char_auto == ""     && (fAll || argument == "cself"))
					unfin = TRUE;

				if (i->others_auto == ""   && (fAll || argument == "oself"))
					unfin = TRUE;

				if (unfin) {
					ptc(ch, "%s\n", i->name);
					count++;
				}
			}

			ptc(ch, "%sYou found %d socials matching your criteria.\n",
			    count > 0 ? "\n" : "", count);
			return;
		}

		stc("Find what?\n", ch);
		return;
	}
	else if (cmd == "cnoarg") { /* Set that argument */
		iSocial->char_no_arg = argument;

		if (argument.empty())
			stc("Character will now see nothing when this social is used without arguments.\n", ch);
		else
			ptc(ch, "New message is now:\n%s\n", argument);
	}
	else if (cmd == "onoarg") {
		iSocial->others_no_arg = argument;

		if (argument.empty())
			stc("Others will now see nothing when this social is used without arguments.\n", ch);
		else
			ptc(ch, "New message is now:\n%s\n", argument);
	}
	else if (cmd == "cfound") {
		iSocial->char_found = argument;

		if (argument.empty())
			stc("The character will now see nothing when a target is found.\n", ch);
		else
			ptc(ch, "New message is now:\n%s\n", argument);
	}
	else if (cmd == "ofound") {
		iSocial->others_found = argument;

		if (argument.empty())
			stc("Others will now see nothing when a target is found.\n", ch);
		else
			ptc(ch, "New message is now:\n%s\n", argument);
	}
	else if (cmd == "vfound") {
		iSocial->vict_found = argument;

		if (argument.empty())
			stc("Victim will now see nothing when a target is found.\n", ch);
		else
			ptc(ch, "New message is now:\n%s\n", argument);
	}
	else if (cmd == "cself") {
		iSocial->char_auto = argument;

		if (argument.empty())
			stc("Character will now see nothing when targetting self.\n", ch);
		else
			ptc(ch, "New message is now:\n%s\n", argument);
	}
	else if (cmd == "oself") {
		iSocial->others_auto = argument;

		if (argument.empty())
			stc("Others will now see nothing when character targets self.\n", ch);
		else
			ptc(ch, "New message is now:\n%s\n", argument);
	}
	else {
		stc("Huh? Try HELP SEDIT.\n", ch);
		return;
	}

	/* We have done something. update social table */
	save_social_table();
}

