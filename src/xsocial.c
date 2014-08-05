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

// Xsocials by Montrey, mostly copied from:

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

#include "merc.h"
#include "db.h"
#include "interp.h"

#define XSOCIAL_FILE "../misc/xsocial.txt"

void insert_xsocial(struct xsocial_type *);
void remove_xsocial(char *);
void clear_xsocial(struct xsocial_type *);
int count_xsocials();

struct xsocial_type *xsocial_table_head;   /* and social table */
struct xsocial_type *xsocial_table_tail;   /* and social table */

void load_xsocial(FILE *fp, struct xsocial_type *xsocial)
{
	strcpy(xsocial->name, fread_string(fp));
	xsocial->csex           = fread_number(fp);
	xsocial->vsex           = fread_number(fp);
	xsocial->char_no_arg    = fread_string(fp);
	xsocial->others_no_arg  = fread_string(fp);
	xsocial->char_found     = fread_string(fp);
	xsocial->others_found   = fread_string(fp);
	xsocial->vict_found     = fread_string(fp);
	xsocial->char_auto      = fread_string(fp);
	xsocial->others_auto    = fread_string(fp);
}

void load_xsocial_table()
{
	FILE *fp;
	int i;
	struct xsocial_type *new_xsocial;
	fp = fopen(XSOCIAL_FILE, "r");

	if (!fp) {
		bug("Could not open " XSOCIAL_FILE " for reading.", 0);
		return;
	}

	fscanf(fp, "%d\n", &maxXSocial);
	xsocial_table_head = alloc_mem(sizeof(struct xsocial_type));
	xsocial_table_tail = alloc_mem(sizeof(struct xsocial_type));
	xsocial_table_head->next = xsocial_table_tail;
	xsocial_table_tail->previous = xsocial_table_head;

	for (i = 0; i < maxXSocial; i++) {
		new_xsocial = alloc_mem(sizeof(struct xsocial_type));
		load_xsocial(fp, new_xsocial);
		insert_xsocial(new_xsocial);
	}

	fclose(fp);
}

void insert_xsocial(struct xsocial_type *s)
{
	struct xsocial_type *iterator;
	/* find the right place */
	iterator = xsocial_table_head->next;

	while (iterator != xsocial_table_tail) {
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
	iterator = xsocial_table_tail->previous;
	iterator->next = s;
	s->previous = iterator;
	s->next = xsocial_table_tail;
	xsocial_table_tail->previous = s;
}

void remove_xsocial(char *name)
{
	struct xsocial_type *iterator;

	for (iterator = xsocial_table_head->next; iterator != xsocial_table_tail; iterator = iterator->next) {
		if (!strcasecmp(name, iterator->name)) {
			struct xsocial_type *p = iterator->previous;
			struct xsocial_type *n = iterator->next;
			p->next = n;
			n->previous = p;
			clear_xsocial(iterator);
			free_mem(iterator, sizeof(iterator));
			return;
		}
	}
}

void clear_xsocial(struct xsocial_type *xsocial)
{
	xsocial->csex = 0;
	xsocial->vsex = 0;
	free_string(xsocial->char_no_arg);
	free_string(xsocial->others_no_arg);
	free_string(xsocial->char_found);
	free_string(xsocial->others_found);
	free_string(xsocial->vict_found);
	free_string(xsocial->char_auto);
	free_string(xsocial->others_auto);
}

int count_xsocials()
{
	struct xsocial_type *iterator;
	int xsocials = 0;
	/* set to first social */
	iterator = xsocial_table_head->next;

	while (iterator != xsocial_table_tail) {
		xsocials++;
		iterator = iterator->next;
	}

	return xsocials;
}

void save_xsocial(const struct xsocial_type *s, FILE *fp)
{
	/* get rid of (null) */
	fprintf(fp, "%s~\n%d %d\n%s~\n%s~\n%s~\n%s~\n%s~\n%s~\n%s~\n\n",
	        s->name                 ? s->name               : "",
	        s->csex,
	        s->vsex,
	        s->char_no_arg          ? s->char_no_arg        : "",
	        s->others_no_arg        ? s->others_no_arg      : "",
	        s->char_found           ? s->char_found         : "",
	        s->others_found         ? s->others_found       : "",
	        s->vict_found           ? s->vict_found         : "",
	        s->char_auto            ? s->char_auto          : "",
	        s->others_auto          ? s->others_auto        : "");
}

void save_xsocial_table()
{
	FILE *fp;
	struct xsocial_type *iterator;
	fp = fopen(XSOCIAL_FILE, "w");

	if (!fp) {
		bug("Could not open " XSOCIAL_FILE " for writing.", 0);
		return;
	}

	fprintf(fp, "%d\n", count_xsocials());
	/* set to first xsocial */
	iterator = xsocial_table_head->next;

	while (iterator != xsocial_table_tail) {
		save_xsocial(iterator, fp);
		iterator = iterator->next;
	}

	fclose(fp);
}

struct xsocial_type *xsocial_lookup(const char *name)
{
	struct xsocial_type *iterator;
	/* set to first xsocial */
	iterator = xsocial_table_head->next;

	while (iterator != xsocial_table_tail) {
		if (!str_cmp(name, iterator->name))
			return iterator;

		iterator = iterator->next;
	}

	return NULL;
}

void do_xedit(CHAR_DATA *ch, char *argument)
{
	char cmd[MAX_INPUT_LENGTH], xsocial[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	struct xsocial_type *iXSocial;
	smash_tilde(argument);
	argument = one_argument(argument, cmd);
	argument = one_argument(argument, xsocial);

	if (!cmd[0]) {
		stc("Huh? Type HELP XEDIT to see syntax.\n\r", ch);
		return;
	}

	if (!xsocial[0]) {
		if (!str_cmp(cmd, "find"))
			stc("What do you wish to find?\n\r", ch);
		else
			stc("What xsocial do you want to operate on?\n\r", ch);

		return;
	}

	iXSocial = xsocial_lookup(xsocial);

	if (str_cmp(cmd, "new") && str_cmp(cmd, "find") && (iXSocial == NULL)) {
		stc("No such xsocial exists.\n\r", ch);
		return;
	}

	if (!str_cmp(cmd, "delete")) { /* Remove a social */
		remove_xsocial(iXSocial->name);
		stc("That xsocial is history now.\n\r", ch);
	}
	else if (!str_cmp(cmd, "new")) { /* Create a new social */
		struct xsocial_type *new_xsocial = alloc_mem(sizeof(struct xsocial_type));
		int x;

		if (iXSocial != NULL) {
			stc("An xsocial with that name already exists.\n\r", ch);
			return;
		}

		for (x = 0; cmd_table[x].name[0] != '\0'; x++) {
			if (!str_prefix1(xsocial, cmd_table[x].name)) {
				stc("A command with that name already exists.\n", ch);
				return;
			}
		}

		strcpy(new_xsocial->name, str_dup(xsocial));
		new_xsocial->csex = 0;
		new_xsocial->vsex = 0;
		new_xsocial->char_no_arg = str_dup("");
		new_xsocial->others_no_arg = str_dup("");
		new_xsocial->char_found = str_dup("");
		new_xsocial->others_found = str_dup("");
		new_xsocial->vict_found = str_dup("");
		new_xsocial->char_auto = str_dup("");
		new_xsocial->others_auto = str_dup("");
		insert_xsocial(new_xsocial);
		stc("New xsocial added.\n\r", ch);
	}
	else if (!str_cmp(cmd, "rename")) { /* Rename an xsocial */
		struct xsocial_type *new_xsocial = alloc_mem(sizeof(struct xsocial_type));
		int x;

		if (argument[0] == '\0') {
			stc("Rename it to what?\n\r", ch);
			return;
		}

		if (xsocial_lookup(argument) != NULL) {
			stc("An xsocial with that name already exists.\n\r", ch);
			return;
		}

		for (x = 0; cmd_table[x].name[0] != '\0'; x++) {
			if (!str_prefix1(argument, cmd_table[x].name)) {
				stc("A command with that name already exists.\n", ch);
				return;
			}
		}

		strcpy(new_xsocial->name, str_dup(argument));
		new_xsocial->csex               = iXSocial->csex;
		new_xsocial->vsex               = iXSocial->vsex;
		new_xsocial->char_no_arg        = iXSocial->char_no_arg;
		new_xsocial->others_no_arg      = iXSocial->others_no_arg;
		new_xsocial->char_found         = iXSocial->char_found;
		new_xsocial->others_found       = iXSocial->others_found;
		new_xsocial->vict_found         = iXSocial->vict_found;
		new_xsocial->char_auto          = iXSocial->char_auto;
		new_xsocial->others_auto        = iXSocial->others_auto;
		insert_xsocial(new_xsocial);
		remove_xsocial(iXSocial->name);
		stc("Xsocial renamed.\n\r", ch);
	}
	else if (!str_cmp(cmd, "show")) { /* Show a certain social */
		sprintf(buf,   "{HXSocial: %s{x\n\r"
		        "{G[csex]{c   Required character sex (0=either,1=male,2=female):\n\r"
		        "{Y         %d\n\r"
		        "{G[vsex]{c   Required victim sex (0=either,1=male,2=female):\n\r"
		        "{Y         %d\n\r"
		        "{G[cnoarg]{c No argument given, character sees:\n\r"
		        "{Y         %s\n\r"
		        "{G[onoarg]{c No argument given, others see:\n\r"
		        "{Y         %s\n\r"
		        "{G[cfound]{c Target found, character sees:\n\r"
		        "{Y         %s\n\r"
		        "{G[ofound]{c Target found, others see:\n\r"
		        "{Y         %s\n\r"
		        "{G[vfound]{c Target found, victim sees:\n\r"
		        "{Y         %s\n\r"
		        "{G[cself]{c  Target is self:\n\r"
		        "{Y         %s\n\r"
		        "{G[oself]{c  Target is self, others see:\n\r"
		        "{Y         %s{x\n\r",
		        iXSocial->name,
		        iXSocial->csex,
		        iXSocial->vsex,
		        iXSocial->char_no_arg,
		        iXSocial->others_no_arg,
		        iXSocial->char_found,
		        iXSocial->others_found,
		        iXSocial->vict_found,
		        iXSocial->char_auto,
		        iXSocial->others_auto);
		stc(buf, ch);
		return; /* return right away, do not save the table */
	}
	else if (!str_prefix1(cmd, "find")) { /* Find an xsocial */
		struct xsocial_type *i;
		bool fAll = FALSE;

		if (argument[0] == '\0')
			fAll = TRUE;

		if (!str_cmp(xsocial, "unfinished")) {
			bool unfin;
			int count = 0;

			for (i = xsocial_table_head->next; i != xsocial_table_tail; i = i->next) {
				unfin = FALSE;

				if (!str_cmp(i->char_no_arg, "")   && (fAll || !str_cmp(argument, "cnoarg")))
					unfin = TRUE;

				if (!str_cmp(i->others_no_arg, "") && (fAll || !str_cmp(argument, "onoarg")))
					unfin = TRUE;

				if (!str_cmp(i->char_found, "")    && (fAll || !str_cmp(argument, "cfound")))
					unfin = TRUE;

				if (!str_cmp(i->others_found, "")  && (fAll || !str_cmp(argument, "ofound")))
					unfin = TRUE;

				if (!str_cmp(i->vict_found, "")    && (fAll || !str_cmp(argument, "vfound")))
					unfin = TRUE;

				if (!str_cmp(i->char_auto, "")     && (fAll || !str_cmp(argument, "cself")))
					unfin = TRUE;

				if (!str_cmp(i->others_auto, "")   && (fAll || !str_cmp(argument, "oself")))
					unfin = TRUE;

				if (unfin) {
					ptc(ch, "%s\n\r", i->name);
					count++;
				}
			}

			ptc(ch, "%sYou found %d xsocials matching your criteria.\n\r",
			    count > 0 ? "\n\r" : "", count);
			return;
		}

		stc("Find what?\n\r", ch);
		return;
	}
	else if (!str_cmp(cmd, "csex")) { /* Set that argument */
		if (!is_number(argument)) {
			stc("The value must be numeric, 0=either,1=male,2=female.\n\r", ch);
			return;
		}

		if (atoi(argument) < 0 || atoi(argument) > 2) {
			stc("Valid sexes are: 0=either,1=male,2=female.\n\r", ch);
			return;
		}

		iXSocial->csex = atoi(argument);
		ptc(ch, "The required character sex is now %s.\n\r",
		    iXSocial->csex == 0 ? "irrelevant" : iXSocial->csex == 1 ? "male" : "female");
	}
	else if (!str_cmp(cmd, "vsex")) { /* Set that argument */
		if (!is_number(argument)) {
			stc("The value must be numeric, 0=either,1=male,2=female.\n\r", ch);
			return;
		}

		if (atoi(argument) < 0 || atoi(argument) > 2) {
			stc("Valid sexes are: 0=either,1=male,2=female.\n\r", ch);
			return;
		}

		iXSocial->vsex = atoi(argument);
		ptc(ch, "The required victim sex is now %s.\n\r",
		    iXSocial->vsex == 0 ? "irrelevant" : iXSocial->vsex == 1 ? "male" : "female");
	}
	else if (!str_cmp(cmd, "cnoarg")) { /* Set that argument */
		free_string(iXSocial->char_no_arg);
		iXSocial->char_no_arg = str_dup(argument);

		if (!argument[0])
			stc("Character will now see nothing when this social is used without arguments.\n\r", ch);
		else
			ptc(ch, "New message is now:\n\r%s\n\r", argument);
	}
	else if (!str_cmp(cmd, "onoarg")) {
		free_string(iXSocial->others_no_arg);
		iXSocial->others_no_arg = str_dup(argument);

		if (!argument[0])
			stc("Others will now see nothing when this social is used without arguments.\n\r", ch);
		else
			ptc(ch, "New message is now:\n\r%s\n\r", argument);
	}
	else if (!str_cmp(cmd, "cfound")) {
		free_string(iXSocial->char_found);
		iXSocial->char_found = str_dup(argument);

		if (!argument[0])
			stc("The character will now see nothing when a target is found.\n\r", ch);
		else
			ptc(ch, "New message is now:\n\r%s\n\r", argument);
	}
	else if (!str_cmp(cmd, "ofound")) {
		free_string(iXSocial->others_found);
		iXSocial->others_found = str_dup(argument);

		if (!argument[0])
			stc("Others will now see nothing when a target is found.\n\r", ch);
		else
			ptc(ch, "New message is now:\n\r%s\n\r", argument);
	}
	else if (!str_cmp(cmd, "vfound")) {
		free_string(iXSocial->vict_found);
		iXSocial->vict_found = str_dup(argument);

		if (!argument[0])
			stc("Victim will now see nothing when a target is found.\n\r", ch);
		else
			ptc(ch, "New message is now:\n\r%s\n\r", argument);
	}
	else if (!str_cmp(cmd, "cself")) {
		free_string(iXSocial->char_auto);
		iXSocial->char_auto = str_dup(argument);

		if (!argument[0])
			stc("Character will now see nothing when targetting self.\n\r", ch);
		else
			ptc(ch, "New message is now:\n\r%s\n\r", argument);
	}
	else if (!str_cmp(cmd, "oself")) {
		free_string(iXSocial->others_auto);
		iXSocial->others_auto = str_dup(argument);

		if (!argument[0])
			stc("Others will now see nothing when character targets self.\n\r", ch);
		else
			ptc(ch, "New message is now:\n\r%s\n\r", argument);
	}
	else {
		stc("Huh? Try HELP XEDIT.\n\r", ch);
		return;
	}

	/* We have done something. update social table */
	save_xsocial_table();
}

