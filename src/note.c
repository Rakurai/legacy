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

/* globals from db.c for load_notes */
extern  int     _filbuf         args((FILE *));
extern FILE                   *fpArea;
extern char                    strArea[MAX_INPUT_LENGTH];


/* local procedures */
void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time);
void parse_note(CHAR_DATA *ch, char *argument, int type);
bool hide_note(CHAR_DATA *ch, NOTE_DATA *pnote);

NOTE_DATA *note_list;
NOTE_DATA *idea_list;
NOTE_DATA *roleplay_list;
NOTE_DATA *immquest_list;
NOTE_DATA *changes_list;
NOTE_DATA *personal_list;
NOTE_DATA *trade_list;

/* Names for note types. Keep these in line with the
   #define names NOTE_NOTE, NOTE_IDEA etc. in merc.h !!! */
struct board_index_struct board_index [] = {
	{ "{PN", &note_list,    "note",                 "notes",                "note"                  },
	{ "{YI", &idea_list,    "idea",                 "ideas",                "idea"                  },
	{ "{VR", &roleplay_list, "roleplay",             "roleplays",            "roleplaying note"      },
	{ "{BQ", &immquest_list, "immquest",             "immquests",            "quest note"            },
	{ "{GC", &changes_list, "change",               "changes",              "change"                },
	{ "{CP", &personal_list, "personal message",     "personal messages",    "personal message"      },
	{ "{bT", &trade_list,   "trade note",           "trade notes",          "trade note"            },
	{ "",    NULL,          "",                     "",                     ""                      }
};


/* count the number of messages, visible to 'ch', in a given message list */
int count_spool(CHAR_DATA *ch, NOTE_DATA *spool)
{
	int count = 0;
	NOTE_DATA *pnote;

	for (pnote = spool; pnote != NULL; pnote = pnote->next)
		if (!hide_note(ch, pnote))
			count++;

	return count;
}


/* display the numbers of unread messages of each type, visible to 'ch' */
void do_unread(CHAR_DATA *ch)
{
	int count;
	bool found = FALSE;

	if (IS_NPC(ch)) {
		stc("Sorry, only players can read notes!\n\r", ch);
		return;
	}

	if ((count = count_spool(ch, immquest_list)) > 0) {
		found = TRUE;
		ptc(ch, "There %s %d new {Bimmquest{x note%s waiting.\n\r",
		    count > 1 ? "are" : "is",
		    count,
		    count > 1 ? "s" : "");
	}

	if ((count = count_spool(ch, changes_list)) > 0) {
		found = TRUE;
		ptc(ch, "There %s %d {Gchange%s{x waiting to be read.\n\r",
		    count > 1 ? "are" : "is",
		    count,
		    count > 1 ? "s" : "");
	}

	if ((count = count_spool(ch, note_list)) > 0) {
		found = TRUE;
		ptc(ch, "You have %d new {Pnote%s{x waiting.\n\r",
		    count,
		    count > 1 ? "s" : "");
	}

	if ((count = count_spool(ch, idea_list)) > 0) {
		found = TRUE;
		ptc(ch, "You have %d unread {Yidea%s{x to pursue.\n\r",
		    count,
		    count > 1 ? "s" : "");
	}

	if ((count = count_spool(ch, roleplay_list)) > 0) {
		found = TRUE;
		ptc(ch, "%d {Vroleplay%s been added.\n\r",
		    count,
		    count > 1 ? "ing{x notes have" : "{x note has");
	}

	if ((count = count_spool(ch, personal_list)) > 0) {
		found = TRUE;
		ptc(ch, "You have %d {Cpersonal{x message%s to read.\n\r",
		    count,
		    count > 1 ? "s" : "");
	}

	if ((count = count_spool(ch, trade_list)) > 0) {
		found = TRUE;
		ptc(ch, "You have %d {btrade{x note%s to read.\n\r",
		    count,
		    count > 1 ? "s" : "");
	}

	if (!found)
		stc("You have no unread notes.\n\r", ch);
	else {
		stc("Type: {Y<next>{x to scroll through your messages.\n\r", ch);
		stc("Type: {Y<note wipe>{x to empty all message boards.\n\r", ch);
	}
}


void do_note(CHAR_DATA *ch, char *argument)
{
	parse_note(ch, argument, NOTE_NOTE);
}

void do_idea(CHAR_DATA *ch, char *argument)
{
	parse_note(ch, argument, NOTE_IDEA);
}

void do_personal(CHAR_DATA *ch, char *argument)
{
	parse_note(ch, argument, NOTE_PERSONAL);
}

void do_roleplay(CHAR_DATA *ch, char *argument)
{
	parse_note(ch, argument, NOTE_ROLEPLAY);
}

void do_immquest(CHAR_DATA *ch, char *argument)
{
	parse_note(ch, argument, NOTE_IMMQUEST);
}

void do_changes(CHAR_DATA *ch, char *argument)
{
	parse_note(ch, argument, NOTE_CHANGES);
}

void do_trade(CHAR_DATA *ch, char *argument)
{
	parse_note(ch, argument, NOTE_TRADE);
}

void save_notes(int type)
{
	FILE *fp;
	char *name;
	NOTE_DATA *pnote;

	switch (type) {
	default:
		return;

	case NOTE_NOTE:
		name = NOTE_FILE;
		pnote = note_list;
		break;

	case NOTE_IDEA:
		name = IDEA_FILE;
		pnote = idea_list;
		break;

	case NOTE_ROLEPLAY:
		name = ROLEPLAY_FILE;
		pnote = roleplay_list;
		break;

	case NOTE_IMMQUEST:
		name = IMMQUEST_FILE;
		pnote = immquest_list;
		break;

	case NOTE_CHANGES:
		name = CHANGES_FILE;
		pnote = changes_list;
		break;

	case NOTE_PERSONAL:
		name = PERSONAL_FILE;
		pnote = personal_list;
		break;

	case NOTE_TRADE:
		name = TRADE_FILE;
		pnote = trade_list;
		break;
	}

	if ((fp = fopen(name, "w")) == NULL)
		perror(name);
	else {
		for (; pnote != NULL; pnote = pnote->next) {
			fprintf(fp, "Sender  %s~\n", pnote->sender);
			fprintf(fp, "Date    %s~\n", pnote->date);
			fprintf(fp, "Stamp   %ld\n", pnote->date_stamp);
			fprintf(fp, "To      %s~\n", pnote->to_list);
			fprintf(fp, "Subject %s~\n", pnote->subject);
			fprintf(fp, "Text\n%s~\n",   pnote->text);
		}

		fclose(fp);
		return;
	}
}


void load_notes(void)
{
	load_thread(NOTE_FILE, &note_list, NOTE_NOTE, 10 * 24 * 60 * 60);
	load_thread(IDEA_FILE, &idea_list, NOTE_IDEA, 30 * 24 * 60 * 60);
	load_thread(ROLEPLAY_FILE, &roleplay_list, NOTE_ROLEPLAY, 10 * 24 * 60 * 60);
	load_thread(IMMQUEST_FILE, &immquest_list, NOTE_IMMQUEST, 15 * 24 * 60 * 60);
	load_thread(CHANGES_FILE, &changes_list, NOTE_CHANGES, 90 * 24 * 60 * 60);
	load_thread(PERSONAL_FILE, &personal_list, NOTE_PERSONAL, 10 * 24 * 60 * 60);
	load_thread(TRADE_FILE, &trade_list, NOTE_TRADE, 10 * 24 * 60 * 60);
}


void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time)
{
	FILE *fp;
	NOTE_DATA *pnotelast;

	if ((fp = fopen(name, "r")) == NULL)
		return;

	pnotelast = NULL;

	for (; ;) {
		NOTE_DATA *pnote;
		char letter;

		do {
			letter = getc(fp);

			if (feof(fp)) {
				fclose(fp);
				return;
			}
		}
		while (isspace(letter));

		ungetc(letter, fp);
		pnote           = alloc_perm(sizeof(*pnote));

		if (str_cmp(fread_word(fp), "sender"))
			break;

		pnote->sender   = fread_string(fp);

		if (str_cmp(fread_word(fp), "date"))
			break;

		pnote->date     = fread_string(fp);

		if (str_cmp(fread_word(fp), "stamp"))
			break;

		pnote->date_stamp = fread_number(fp);

		if (str_cmp(fread_word(fp), "to"))
			break;

		pnote->to_list  = fread_string(fp);

		if (str_cmp(fread_word(fp), "subject"))
			break;

		pnote->subject  = fread_string(fp);

		if (str_cmp(fread_word(fp), "text"))
			break;

		pnote->text     = fread_string(fp);

		if (free_time && (pnote->date_stamp < current_time - free_time)) {
			free_note(pnote);
			continue;
		}

		pnote->type = type;

		if (*list == NULL)
			*list           = pnote;
		else
			pnotelast->next     = pnote;

		pnotelast       = pnote;
	}

	strcpy(strArea, NOTE_FILE);
	fpArea = fp;
	bug("Load_notes: bad key word.", 0);
	/* We probably should not crash the program over a
	   bad note entry. Simply return to the calling function.
	   Outsider <slicer69@hotmail.com> Feb 7, 2004
	*/
	/* exit( 1 ); */
	return;
}


void append_note(NOTE_DATA *pnote)
{
	FILE *fp;
	char *name;
	NOTE_DATA **list;
	NOTE_DATA *last;

	switch (pnote->type) {
	default:
		return;

	case NOTE_NOTE:
		name = NOTE_FILE;
		list = &note_list;
		break;

	case NOTE_IDEA:
		name = IDEA_FILE;
		list = &idea_list;
		break;

	case NOTE_ROLEPLAY:
		name = ROLEPLAY_FILE;
		list = &roleplay_list;
		break;

	case NOTE_IMMQUEST:
		name = IMMQUEST_FILE;
		list = &immquest_list;
		break;

	case NOTE_CHANGES:
		name = CHANGES_FILE;
		list = &changes_list;
		break;

	case NOTE_PERSONAL:
		name = PERSONAL_FILE;
		list = &personal_list;
		break;

	case NOTE_TRADE:
		name = TRADE_FILE;
		list = &trade_list;
		break;
	}

	if (*list == NULL)
		*list = pnote;
	else {
		for (last = *list; last->next != NULL; last = last->next);

		last->next = pnote;
	}

	if ((fp = fopen(name, "a")) == NULL)
		perror(name);
	else {
		fprintf(fp, "Sender  %s~\n", pnote->sender);
		fprintf(fp, "Date    %s~\n", pnote->date);
		fprintf(fp, "Stamp   %ld\n", pnote->date_stamp);
		fprintf(fp, "To      %s~\n", pnote->to_list);
		fprintf(fp, "Subject %s~\n", pnote->subject);
		fprintf(fp, "Text\n%s~\n", pnote->text);
		fclose(fp);
		/* Mud has crashed on above line before */
	}
}


bool is_note_to(CHAR_DATA *ch, NOTE_DATA *pnote)
{
	char buf[MSL];
	/* don't show notes to the forwarding person *mutter*  -- Montrey */
	sprintf(buf, "FORWARD(%s)", ch->name);

	if (strstr(buf, smash_bracket(pnote->subject)))
		return FALSE;

	/* note to followers */
	if (is_name("followers", pnote->to_list)) {
		int i = parse_deity(ch->pcdata->deity);

		if (i >= 0 && !str_cmp(pnote->sender, deity_table[i].name))
			return TRUE;
	}

	if (!str_cmp(ch->name, pnote->sender))
		return TRUE;

	if (is_name("spam", pnote->to_list)) {
		if (IS_SET(ch->censor, CENSOR_SPAM))
			return FALSE;

		return TRUE;
	}

	if (is_exact_name("all", pnote->to_list))
		return TRUE;

	if (IS_IMP(ch) && is_name("imp", pnote->to_list))
		return TRUE;

	if (IS_HEAD(ch) && is_name("head", pnote->to_list))
		return TRUE;

	if (IS_IMMORTAL(ch)
	    && (is_name("immortal", pnote->to_list)
	        || is_name("imm", pnote->to_list)))
		return TRUE;

	if (is_exact_name(ch->name, pnote->to_list))
		return TRUE;

	/* note to clans -- Montrey */
	if (ch->clan
	    && (is_exact_name(ch->clan->name, pnote->to_list)      /* note to one clan */
	        || is_name("clan", pnote->to_list))) {                /* note to all clans */
		if (is_name("leader", pnote->to_list))          /* note to leaders */
			if (!HAS_CGROUP(ch, GROUP_LEADER))
				return FALSE;

		if (is_name("deputy", pnote->to_list)           /* note to deputies */
		    || is_name("deputies", pnote->to_list))
			if (!HAS_CGROUP(ch, GROUP_LEADER)
			    && !HAS_CGROUP(ch, GROUP_DEPUTY))
				return FALSE;

		return TRUE;
	}

	return FALSE;
}



void note_attach(CHAR_DATA *ch, int type)
{
	NOTE_DATA *pnote;

	if (ch->pnote != NULL)
		return;

	pnote = new_note();
	pnote->next         = NULL;
	pnote->sender       = str_dup(IS_NPC(ch) ? ch->short_descr : ch->name);
	pnote->date         = str_dup("");
	pnote->to_list      = str_dup("");
	pnote->subject      = str_dup("");
	pnote->text         = str_dup("");
	pnote->type         = type;
	ch->pnote           = pnote;
	return;
}



void note_remove(CHAR_DATA *ch, NOTE_DATA *pnote, bool delete)
{
	char to_new[MAX_INPUT_LENGTH];
	char to_one[MAX_INPUT_LENGTH];
	NOTE_DATA *prev;
	NOTE_DATA **list;
	char *to_list;

	if (!delete) {
		/* make a new list */
		to_new[0]       = '\0';
		to_list = pnote->to_list;

		while (*to_list != '\0') {
			to_list     = one_argument(to_list, to_one);

			if (to_one[0] != '\0' && str_cmp(ch->name, to_one)) {
				strcat(to_new, " ");
				strcat(to_new, to_one);
			}
		}

		/* Just a simple recipient removal? */
		if (str_cmp(ch->name, pnote->sender) && to_new[0] != '\0') {
			free_string(pnote->to_list);
			pnote->to_list = str_dup(to_new + 1);
			return;
		}
	}

	/* nuke the whole note */

	switch (pnote->type) {
	default:
		return;

	case NOTE_NOTE:
		list = &note_list;
		break;

	case NOTE_IDEA:
		list = &idea_list;
		break;

	case NOTE_ROLEPLAY:
		list = &roleplay_list;
		break;

	case NOTE_IMMQUEST:
		list = &immquest_list;
		break;

	case NOTE_CHANGES:
		list = &changes_list;
		break;

	case NOTE_PERSONAL:
		list = &personal_list;
		break;

	case NOTE_TRADE:
		list = &trade_list;
		break;
	}

	/*
	 * Remove note from linked list.
	 */
	if (pnote == *list)
		*list = pnote->next;
	else {
		for (prev = *list; prev != NULL; prev = prev->next) {
			if (prev->next == pnote)
				break;
		}

		if (prev == NULL) {
			bug("Note_remove: pnote not found.", 0);
			return;
		}

		prev->next = pnote->next;
	}

	save_notes(pnote->type);
	free_note(pnote);
	return;
}


bool hide_note(CHAR_DATA *ch, NOTE_DATA *pnote)
{
	time_t last_read;

	/* Mob notes
	    if (IS_NPC(ch))
	        return TRUE;
	*/
	switch (pnote->type) {
	default:
		return TRUE;

	case NOTE_NOTE:
		last_read = ch->pcdata->last_note;
		break;

	case NOTE_IDEA:
		last_read = ch->pcdata->last_idea;
		break;

	case NOTE_ROLEPLAY:
		last_read = ch->pcdata->last_roleplay;
		break;

	case NOTE_IMMQUEST:
		last_read = ch->pcdata->last_immquest;
		break;

	case NOTE_CHANGES:
		last_read = ch->pcdata->last_changes;
		break;

	case NOTE_PERSONAL:
		last_read = ch->pcdata->last_personal;
		break;

	case NOTE_TRADE:
		last_read = ch->pcdata->last_trade;
		break;
	}

	if (pnote->date_stamp <= last_read)
		return TRUE;

	if (!str_cmp(ch->name, pnote->sender))
		return TRUE;

	if (!is_note_to(ch, pnote))
		return TRUE;

	return FALSE;
}


void update_read(CHAR_DATA *ch, NOTE_DATA *pnote)
{
	time_t stamp;
	/* Mob Notes
	    if (IS_NPC(ch))
	        return;
	*/
	stamp = pnote->date_stamp;

	switch (pnote->type) {
	default:
		return;

	case NOTE_NOTE:
		ch->pcdata->last_note = UMAX(ch->pcdata->last_note, stamp);
		break;

	case NOTE_IDEA:
		ch->pcdata->last_idea = UMAX(ch->pcdata->last_idea, stamp);
		break;

	case NOTE_ROLEPLAY:
		ch->pcdata->last_roleplay = UMAX(ch->pcdata->last_roleplay, stamp);
		break;

	case NOTE_IMMQUEST:
		ch->pcdata->last_immquest = UMAX(ch->pcdata->last_immquest, stamp);
		break;

	case NOTE_CHANGES:
		ch->pcdata->last_changes = UMAX(ch->pcdata->last_changes, stamp);
		break;

	case NOTE_PERSONAL:
		ch->pcdata->last_personal = UMAX(ch->pcdata->last_personal, stamp);
		break;

	case NOTE_TRADE:
		ch->pcdata->last_trade = UMAX(ch->pcdata->last_trade, stamp);
		break;
	}
}


void notify_note_post(NOTE_DATA *pnote, CHAR_DATA *vch, int type)
{
	CHAR_DATA *ch;
	char buf[MAX_STRING_LENGTH];
	char *list_name;
	list_name = board_index[type].board_long;

	for (ch = char_list; ch != NULL; ch = ch->next) {
		if (IS_NPC(ch))
			continue;

		if (is_note_to(ch, pnote)) {
			sprintf(buf, "{W[FYI] New %s from %s. Subject: [%s]. ",
			        list_name, pnote->sender, pnote->subject);

			if (ch->clan && is_name(ch->clan->name, pnote->to_list))
				strcat(buf, "Guild note.{x\n\r");
			else if (note_is_name(ch->name, pnote->to_list))
				strcat(buf, "Personal note.{x\n\r");
			else if (is_name("all", pnote->to_list))
				strcat(buf, "Global note.{x\n\r");
			else if (IS_IMMORTAL(ch) && is_name("immortal", pnote->to_list))
				strcat(buf, "Immortal note.{x\n\r");
			else
				strcat(buf, "{x\n\r");

			if (ch != vch
			    /* don't info author of forward -- Elrac */
			    && strcmp(pnote->sender, ch->name)
			    && !IS_SET(ch->pcdata->plr, PLR_NONOTIFY))
				stc(buf, ch);
		}
	}
}


void parse_note(CHAR_DATA *ch, char *argument, int type)
{
	BUFFER *buffer;
	char buf[MSL], arg[MIL];
	NOTE_DATA *pnote;
	NOTE_DATA **list;
	char *list_name;
	int vnum = 0, anum = 0;
	/* NOTE: Mobs CAN currently do notes. Don't do anything player-specific! */
	list = board_index[type].board_list;
	list_name = board_index[type].board_plural;
	argument = one_argument(argument, arg);
	smash_tilde(argument);

	if (arg[0] == '\0' || !str_prefix1(arg, "read")) {
		bool fAll = FALSE;

		if (IS_NPC(ch)) {
			stc("Sorry, mobiles can't read notes.\n\r", ch);
			return;
		}

		/* read next unread note */
		if (argument[0] == '\0' || !str_prefix1(argument, "next")) {
			for (pnote = *list; pnote != NULL; pnote = pnote->next) {
				if (!hide_note(ch, pnote)) {
					ptc(ch, "{W[%3d] From: {x%s\n\r"
					    "        {WTo: {x%s\n\r"
					    "      {WDate: {x%s\n\r"
					    "   {WSubject: {x%s\n\r"
					    "{W================================================================================{x\n\r",
					    vnum,
					    pnote->sender,
					    pnote->to_list,
					    pnote->date,
					    pnote->subject);
					page_to_char(pnote->text, ch);
					update_read(ch, pnote);
					return;
				}
				else if (is_note_to(ch, pnote))
					vnum++;
			}

			ptc(ch, "You have no unread %s.\n\r", list_name);
			return;
		}
		else if (is_number(argument))
			anum = atoi(argument);
		else if (!str_cmp(argument, "all"))
			fAll = TRUE;
		else {
			stc("Read which number?\n\r", ch);
			return;
		}

		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (is_note_to(ch, pnote) && (vnum++ == anum || fAll)) {
				ptc(ch, "{W[%3d] From: {x%s\n\r"
				    "        {WTo: {x%s\n\r"
				    "      {WDate: {x%s\n\r"
				    "   {WSubject: {x%s\n\r"
				    "{W================================================================================{x\n\r",
				    vnum - 1,
				    pnote->sender,
				    pnote->to_list,
				    pnote->date,
				    pnote->subject);
				page_to_char(pnote->text, ch);
				update_read(ch, pnote);
				return;
			}
		}

		ptc(ch, "There aren't that many %s.\n\r", list_name);
		return;
	}

	if (!str_prefix1(arg, "list")) {
		bool search = FALSE, new = FALSE, found = FALSE, all = FALSE;

		if (IS_NPC(ch)) {
			stc("Sorry, mobiles can't read notes.\n\r", ch);
			return;
		}

		if (!str_cmp(argument, "new"))
			new = TRUE;
		else if (!str_cmp(argument, "all"))
			all = TRUE;
		else if (argument[0] != '\0')
			search = TRUE;

		buffer = new_buf();

		for (pnote = *list, vnum = -1; pnote != NULL; pnote = pnote->next) {
			if (is_note_to(ch, pnote)) {
				vnum++;

				if ((search) && str_prefix1(argument, pnote->sender))
					continue;

				if ((new) && hide_note(ch, pnote))
					continue;

				sprintf(buf, "{W[{x%3d%s{W][{x%12s{W]{x %s{x\n\r",
				        vnum,
				        hide_note(ch, pnote) ? " " : "N",
				        pnote->sender,
				        pnote->subject);

				if (all)
					stc(buf, ch);
				else
					add_buf(buffer, buf);

				found = TRUE;
			}
		}

		if (!found)
			stc("There are no messages that match that criteria.\n\r", ch);

		if (!all)
			page_to_char(buf_string(buffer), ch);

		free_buf(buffer);
		return;
	}

	if (!str_prefix1(arg, "remove")) {
		if (!is_number(argument)) {
			stc("Note remove which number?\n\r", ch);
			return;
		}

		anum = atoi(argument);
		vnum = 0;

		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (is_note_to(ch, pnote) && vnum++ == anum) {
				note_remove(ch, pnote, FALSE);
				stc("Message removed.\n\r", ch);
				return;
			}
		}

		sprintf(buf, "There aren't that many %s.", list_name);
		stc(buf, ch);
		return;
	}

	if (!str_prefix1(arg, "delete") && IS_IMP(ch)) {
		if (!is_number(argument)) {
			stc("Message delete which number?\n\r", ch);
			return;
		}

		anum = atoi(argument);
		vnum = 0;

		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (is_note_to(ch, pnote) && vnum++ == anum) {
				note_remove(ch, pnote, TRUE);
				stc("Message deleted.\n\r", ch);
				return;
			}
		}

		sprintf(buf, "There aren't that many %s.\n\r", list_name);
		stc(buf, ch);
		return;
	}

	if (!str_prefix1(arg, "forward")) {
		NOTE_DATA *newnote;
		char forward[MAX_STRING_LENGTH];

		if (IS_NPC(ch)) {
			stc("Mobs can't forward notes.\n\r", ch);
			return;
		}

		argument = one_argument(argument, forward);

		if (forward[0] == '\0') {
			stc("Foward note to who?\n\r", ch);
			return;
		}

		if (!is_number(argument)) {
			stc("Note forward which number?\n\r", ch);
			return;
		}

		anum = atoi(argument);
		vnum = 0;

		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (is_note_to(ch, pnote) && vnum++ == anum) {
				newnote = new_note();
				newnote->next     = NULL;
				newnote->sender   = str_dup(pnote->sender);
				newnote->date     = str_dup(pnote->date);
				newnote->date_stamp           = current_time;
				newnote->to_list  = str_dup(forward);
				/* is_note_to relies on the text before the forwarding person's name
				   to be 14 characters, including color codes.  change it if you
				   change this!  the smash_bracket is to assure there's no color
				   codes in their name, even though mobs can't forward -- Montrey */
				sprintf(buf, "{VFORWARD{W({V%s{W){x: %s", smash_bracket(ch->name), pnote->subject);
				newnote->subject  = str_dup(buf);
				newnote->text     = str_dup(pnote->text);
				newnote->type     = pnote->type;
				append_note(newnote);
				stc("Note Forwarded.\n\r", ch);
				notify_note_post(newnote, ch, type);
				return;
			}
		}

		sprintf(buf, "There aren't that many %s.", list_name);
		stc(buf, ch);
		return;
	}

	if (!str_prefix1(arg, "repost") && IS_IMMORTAL(ch)) {
		NOTE_DATA *newnote;

		if (!is_number(argument)) {
			stc("Note repost which number?\n\r", ch);
			return;
		}

		anum = atoi(argument);
		vnum = 0;

		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (is_note_to(ch, pnote) && vnum++ == anum) {
				newnote = new_note();
				newnote->next     = NULL;
				newnote->sender   = str_dup(pnote->sender);
				newnote->date     = str_dup(pnote->date);
				newnote->date_stamp           = current_time;
				newnote->to_list  = str_dup("Immortal");
				sprintf(buf, "{PIMM REPOST{W({P%s{W){x: %s", ch->name,
				        pnote->subject);
				newnote->subject  = str_dup(buf);
				newnote->text     = str_dup(pnote->text);
				newnote->type     = pnote->type;
				append_note(newnote);
				note_remove(ch, pnote, TRUE);
				stc("Note Reposted.\n\r", ch);
				notify_note_post(newnote, ch, type);
				return;
			}
		}

		sprintf(buf, "There aren't that many %s.", list_name);
		stc(buf, ch);
		return;
	}

	if (!str_prefix1(arg, "wipe")) {
		ch->pcdata->last_note = current_time;
		ch->pcdata->last_idea = current_time;
		ch->pcdata->last_roleplay = current_time;
		ch->pcdata->last_changes = current_time;
		ch->pcdata->last_immquest = current_time;
		ch->pcdata->last_personal = current_time;
		ch->pcdata->last_trade = current_time;
		stc("You are now caught up on all messages.\n\r", ch);
		return;
	}

	if (!str_prefix1(arg, "catchup")) {
		switch (type) {
		case NOTE_NOTE:
			ch->pcdata->last_note = current_time;
			stc("You are now caught up on notes.\n\r", ch);
			break;

		case NOTE_IDEA:
			ch->pcdata->last_idea = current_time;
			stc("You are now caught up on ideas.\n\r", ch);
			break;

		case NOTE_ROLEPLAY:
			ch->pcdata->last_roleplay = current_time;
			stc("You are now caught up on roleplay notes.\n\r", ch);
			break;

		case NOTE_IMMQUEST:
			ch->pcdata->last_immquest = current_time;
			stc("You are now caught up on quest notes.\n\r", ch);
			break;

		case NOTE_CHANGES:
			ch->pcdata->last_changes = current_time;
			stc("You are now caught up on changes.\n\r", ch);
			break;

		case NOTE_PERSONAL:
			ch->pcdata->last_personal = current_time;
			stc("You are now caught up on personal messages.\n\r", ch);
			break;

		case NOTE_TRADE:
			ch->pcdata->last_trade = current_time;
			stc("You are now caught up on trade notes.\n\r", ch);
			break;
		}

		return;
	}

	/* message move: e.g. NOTE MOVE 10 IDEA -- Elrac */
	if (!str_prefix1(arg, "move")) {
		NOTE_DATA *thenote, *newnote, *newlist;
		int newtype = 0, j;
		/* get message number */
		argument = one_argument(argument, arg);

		if (arg[0] == '\0' || !is_number(arg)) {
			ptc(ch, "Move which %s number?\n\r", board_index[type].board_short);
			return;
		}

		anum = atoi(arg);
		/* find the message in the list */
		vnum = 0;
		thenote = NULL;

		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (is_note_to(ch, pnote) && vnum++ == anum) {
				thenote = pnote;
				break;
			}
		}

		if (thenote == NULL) {
			ptc(ch, "There aren't that many %s.", board_index[type].board_long);
			return;
		}

		if (strcmp(thenote->sender, ch->name) && !IS_IMMORTAL(ch)) {
			ptc(ch, "You are not the author of this %s, so you can't move it!\n\r",
			    board_index[type].board_short);
			return;
		}

		/* get new list name */
		newlist = NULL;
		argument = one_argument(argument, arg);

		for (j = 0; board_index[j].board_hdr[0] != '\0'; j++) {
			if (!str_prefix1(arg, board_index[j].board_plural)) {
				newtype = j;
				newlist = *(board_index[j].board_list);
				break;
			}
		}

		if (newlist == NULL) {
			ptc(ch, "There's no such board name as '%s'\n\r", arg);
			return;
		}

		if (newtype == NOTE_IMMQUEST && !IS_IMMORTAL(ch)) {
			stc("You aren't allowed to move a message to IMMQUEST, sorry!\n\r", ch);
			return;
		}

		if (newtype == NOTE_CHANGES && !IS_IMMORTAL(ch)) {
			stc("You aren't allowed to move a message to CHANGES, sorry!\n\r", ch);
			return;
		}

		/* copy message to new list */
		newnote                 = new_note();
		newnote->next           = NULL;
		newnote->sender         = str_dup(pnote->sender);
		newnote->date           = str_dup(pnote->date);
		newnote->date_stamp     = current_time;
		newnote->to_list        = str_dup(pnote->to_list);
		newnote->subject        = str_dup(pnote->subject);
		newnote->text           = str_dup(pnote->text);
		newnote->type           = newtype;
		append_note(newnote);
		note_remove(ch, pnote, TRUE);
		ptc(ch, "OK, %s %d moved to %s.\n\r",
		    board_index[type].board_short,
		    anum,
		    board_index[newtype].board_plural);
		notify_note_post(newnote, ch, newtype);
		return;
	}

	if (IS_SET(ch->revoke, REVOKE_NOTE)) {
		stc("Your note writing priviledges have been revoked.\n\r", ch);
		return;
	}

	/* below this point only certain people can edit notes */
	if ((type == NOTE_IMMQUEST || type == NOTE_CHANGES)
	    && !IS_IMMORTAL(ch)) {
		ptc(ch, "You aren't high enough level to write %s.\n\r", list_name);
		return;
	}

	if (!str_cmp(arg, "+")) {
		note_attach(ch, type);

		if (ch->pnote->type != type) {
			stc(
			        "You already have a different note in progress.\n\r", ch);
			return;
		}

		buffer = new_buf();

		if (strlen(ch->pnote->text) + strlen(argument) >= 4096) {
			stc("Note too long.\n\r", ch);

			if (!IS_NPC(ch)) SET_BIT(ch->pcdata->plr, PLR_STOPCRASH);

			return;
		}

		add_buf(buffer, ch->pnote->text);
		add_buf(buffer, argument);
		add_buf(buffer, "\n\r");
		free_string(ch->pnote->text);
		ch->pnote->text = str_dup(buf_string(buffer));
		free_buf(buffer);
		stc("Line added.\n\r", ch);
		return;
	}

	if (!str_cmp(arg, "replace")) {
		char old[MAX_INPUT_LENGTH];
		char new[MAX_INPUT_LENGTH];
		argument = first_arg(argument, old, FALSE);
		argument = first_arg(argument, new, FALSE);

		if ((old[0] == '\0') || (new[0] == '\0')) {
			stc("Usage: note replace 'old string' 'new string'\n\r",
			    ch);
			return;
		}

		if (ch->pnote == NULL || ch->pnote->text == NULL) {
			stc("You have no note in progress.\n\r", ch);
			return;
		}

		ch->pnote->text = string_replace(ch->pnote->text, old, new);
		sprintf(buf, "'%s' replaced with '%s'.\n\r", old, new);
		stc(buf, ch);
		return;
	}

	if (!str_cmp(arg, "format")) {
		if (ch->pnote == NULL || ch->pnote->text == NULL) {
			stc("You have no note in progress.\n\r", ch);
			return;
		}

		ch->pnote->text = format_string(ch->pnote->text);
		stc("Note formatted.\n\r", ch);
		return;
	}

	if (!str_cmp(arg, "-")) {
		int len;
		bool found = FALSE;
		note_attach(ch, type);

		if (ch->pnote->type != type) {
			stc(
			        "You already have a different note in progress.\n\r", ch);
			return;
		}

		if (ch->pnote->text == NULL || ch->pnote->text[0] == '\0') {
			stc("No lines left to remove.\n\r", ch);
			return;
		}

		if (!IS_NPC(ch) && IS_SET(ch->pcdata->plr, PLR_STOPCRASH)) {
			stc("You cannot edit this note any further.\n\r", ch);
			stc("Please either post or clear this note.\n\r", ch);
			return;
		}

		strcpy(buf, ch->pnote->text);

		for (len = strlen(buf); len > 0; len--) {
			if (buf[len] == '\r') {
				if (!found) { /* back it up */
					if (len > 0)
						len--;

					found = TRUE;
				}
				else { /* found the second one */
					buf[len + 1] = '\0';
					free_string(ch->pnote->text);
					ch->pnote->text = str_dup(buf);
					return;
				}
			}
		}

		buf[0] = '\0';
		free_string(ch->pnote->text);
		ch->pnote->text = str_dup(buf);
		stc("Line removed.\n\r", ch);
		return;
	}

	if (!str_prefix1(arg, "subject")) {
		note_attach(ch, type);

		if (ch->pnote->type != type) {
			stc(
			        "You already have a different note in progress.\n\r", ch);
			return;
		}

		free_string(ch->pnote->subject);
		ch->pnote->subject = str_dup(argument);
		stc("Subject set.\n\r", ch);
		return;
	}

	if (!str_prefix1(arg, "to")) {
		if (IS_SET(ch->comm, COMM_NOCHANNELS) &&
		    strcmp(argument, "immortal")) {
			stc("You can currently send notes only to immortal.\n\r", ch);
			return;
		}

		note_attach(ch, type);

		if (ch->pnote->type != type) {
			stc(
			        "You already have a different note in progress.\n\r", ch);
			return;
		}

		if (argument[0] == '\0') {
			stc("Whom do you wish to address it to?\n\r", ch);
			return;
		}

		free_string(ch->pnote->to_list);

		/* if they're not an imm and in a clan, replace 'clan' with their clan's name.
		   but not 'clans'!     -- Montrey */
		if (!IS_IMMORTAL(ch) && ch->clan && is_exact_name("clan", argument)) {
			char line[MSL], *p;
			/* copy string, it'll get mangled */
			strcpy(line, argument);
			p = strstr(line, "clan");

			while (*(p + 4) != ' ' && *(p + 4) != '\0')
				p = strstr(p + 4, "clan");

			*p = '\0';
			p += 4;
			sprintf(buf, "%s%s%s", line, capitalize(ch->clan->name), p);
			ch->pnote->to_list = str_dup(buf);
		}
		else
			ch->pnote->to_list = str_dup(argument);

		stc("Recipient list set.\n\r", ch);
		return;
	}

	if (!str_prefix1(arg, "clear")) {
		if (ch->pnote != NULL) {
			free_note(ch->pnote);
			ch->pnote = NULL;
		}

		stc("Note cleared.\n\r", ch);

		if (!IS_NPC(ch)) REMOVE_BIT(ch->pcdata->plr, PLR_STOPCRASH);

		return;
	}

	if (!str_prefix1(arg, "show")) {
		if (ch->pnote == NULL) {
			stc("You have no note in progress.\n\r", ch);
			return;
		}

		if (ch->pnote->type != type) {
			stc("You aren't working on that kind of note.\n\r", ch);
			return;
		}

		sprintf(buf, "   {WFrom: {x%s\n\r     {WTo: {x%s\n\r{WSubject: {x%s\n\r",
		        ch->pnote->sender,
		        ch->pnote->to_list,
		        ch->pnote->subject
		       );
		stc(buf, ch);
		stc("{W================================================================================{x\n\r", ch);
		stc(ch->pnote->text, ch);
		return;
	}

	if (!str_prefix1(arg, "post") || !str_prefix1(arg, "send")) {
		char *strtime;
		char buf2[MAX_STRING_LENGTH];

		if (ch->pnote == NULL) {
			stc("You have no note in progress.\n\r", ch);
			return;
		}

		if (ch->pnote->type != type) {
			stc("You aren't working on that kind of note.\n\r", ch);
			return;
		}

		if (!str_cmp(ch->pnote->to_list, "")) {
			stc(
			        "You need to provide a recipient (name, all, or immortal).\n\r",
			        ch);
			return;
		}

		if (!str_cmp(ch->pnote->subject, "")) {
			stc("You need to provide a subject.\n\r", ch);
			return;
		}

		/* If a note is personal, route it to the personal board - Lotus */
		if ((ch->pnote->type == NOTE_NOTE) &&
		    ((!is_exact_name("all", ch->pnote->to_list))
		     || (!is_exact_name("spam", ch->pnote->to_list)))) {
			ch->pnote->type = NOTE_PERSONAL;
			type = NOTE_PERSONAL;
		}

		/* If a note is not personal, rout it to the note board */
		if ((ch->pnote->type == NOTE_PERSONAL) &&
		    (is_exact_name("all", ch->pnote->to_list) ||
		     is_exact_name("spam", ch->pnote->to_list))) {
			ch->pnote->type = NOTE_NOTE;
			type = NOTE_NOTE;
		}

		ch->pnote->next                 = NULL;
		strtime                         = ctime(&current_time);
		strtime[strlen(strtime) - 1]      = '\0';
		ch->pnote->date                 = str_dup(strtime);
		ch->pnote->date_stamp           = current_time;
		sprintf(buf2, "%s has just posted a %s to: %s", ch->name,
		        board_index[type].board_long, ch->pnote->to_list);
		wiznet(buf2, ch, NULL, WIZ_MAIL, 0, GET_RANK(ch));
		append_note(ch->pnote);
		notify_note_post(ch->pnote, ch, type);
		ch->pnote = NULL;
		sprintf(buf2, "Your %s has been posted.\n\r", board_index[type].board_long);
		stc(buf2, ch);

		if (!IS_NPC(ch)) REMOVE_BIT(ch->pcdata->plr, PLR_STOPCRASH);

		return;
	}

	stc("You can't do that.\n\r", ch);
	return;
}


void do_old_next(CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH];
	NOTE_DATA *pnote;
	NOTE_DATA **list;
	int vnum;

	if (IS_NPC(ch)) {
		stc("Sorry, mobiles can't read notes.\n\r" , ch);
		return;
	}

	list = &note_list;
	pnote = *list;

	if (pnote != NULL) {
		vnum = 0;

		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (!hide_note(ch, pnote)) {
				sprintf(buf,
				        "{W[{PN{W: {x%3d{W] From:{x %s\n\r           {WTo: {x%s\n\r         {WDate: {x%s\n\r      {WSubject: {x%s\n\r",
				        vnum,
				        pnote->sender,
				        pnote->to_list,
				        pnote->date,
				        pnote->subject);
				stc(buf, ch);
				stc("{W================================================================================{x\n\r", ch);
				page_to_char(pnote->text, ch);
				update_read(ch, pnote);
				return;
			}
			else if (is_note_to(ch, pnote))
				vnum++;
		}
	}

	list = &idea_list;
	pnote = *list;

	if (pnote != NULL) {
		vnum = 0;

		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (!hide_note(ch, pnote)) {
				sprintf(buf,
				        "{W[{YI{W: {x%3d{W] From:{x %s\n\r           {WTo: {x%s\n\r         {WDate: {x%s\n\r      {WSubject: {x%s\n\r",
				        vnum,
				        pnote->sender,
				        pnote->to_list,
				        pnote->date,
				        pnote->subject);
				stc(buf, ch);
				stc("{W================================================================================{x\n\r", ch);
				page_to_char(pnote->text, ch);
				update_read(ch, pnote);
				return;
			}
			else if (is_note_to(ch, pnote))
				vnum++;
		}
	}

	list = &changes_list;
	pnote = *list;

	if (pnote != NULL) {
		vnum = 0;

		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (!hide_note(ch, pnote)) {
				sprintf(buf,
				        "{W[{GC{W: {x%3d{W] From:{x %s\n\r           {WTo: {x%s\n\r         {WDate: {x%s\n\r      {WSubject: {x%s\n\r",
				        vnum,
				        pnote->sender,
				        pnote->to_list,
				        pnote->date,
				        pnote->subject);
				stc(buf, ch);
				stc("{W================================================================================{x\n\r", ch);
				page_to_char(pnote->text, ch);
				update_read(ch, pnote);
				return;
			}
			else if (is_note_to(ch, pnote))
				vnum++;
		}
	}

	list = &immquest_list;
	pnote = *list;

	if (pnote != NULL) {
		vnum = 0;

		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (!hide_note(ch, pnote)) {
				sprintf(buf,
				        "{W[{BQ{W: {x%3d{W] From:{x %s\n\r           {WTo: {x%s\n\r         {WDate: {x%s\n\r      {WSubject: {x%s\n\r",
				        vnum,
				        pnote->sender,
				        pnote->to_list,
				        pnote->date,
				        pnote->subject);
				stc(buf, ch);
				stc("{W================================================================================{x\n\r", ch);
				page_to_char(pnote->text, ch);
				update_read(ch, pnote);
				return;
			}
			else if (is_note_to(ch, pnote))
				vnum++;
		}
	}

	list = &roleplay_list;
	pnote = *list;

	if (pnote != NULL) {
		vnum = 0;

		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (!hide_note(ch, pnote)) {
				sprintf(buf,
				        "{W[{VR{W: {x%3d{W] From:{x %s\n\r           {WTo: {x%s\n\r         {WDate: {x%s\n\r      {WSubject: {x%s\n\r",
				        vnum,
				        pnote->sender,
				        pnote->to_list,
				        pnote->date,
				        pnote->subject);
				stc(buf, ch);
				stc("{W================================================================================{x\n\r", ch);
				page_to_char(pnote->text, ch);
				update_read(ch, pnote);
				return;
			}
			else if (is_note_to(ch, pnote))
				vnum++;
		}
	}

	list = &personal_list;
	pnote = *list;

	if (pnote != NULL) {
		vnum = 0;

		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (!hide_note(ch, pnote)) {
				sprintf(buf,
				        "{W[{CP{W: {x%3d{W] From:{x %s\n\r           {WTo: {x%s\n\r         {WDate: {x%s\n\r      {WSubject: {x%s\n\r",
				        vnum,
				        pnote->sender,
				        pnote->to_list,
				        pnote->date,
				        pnote->subject);
				stc(buf, ch);
				stc("{W================================================================================{x\n\r", ch);
				page_to_char(pnote->text, ch);
				update_read(ch, pnote);
				return;
			}
			else if (is_note_to(ch, pnote))
				vnum++;
		}
	}

	list = &trade_list;
	pnote = *list;

	if (pnote != NULL) {
		vnum = 0;

		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (!hide_note(ch, pnote)) {
				sprintf(buf,
				        "{W[{bT{W: {x%3d{W] From:{x %s\n\r           {WTo: {x%s\n\r         {WDate: {x%s\n\r      {WSubject: {x%s\n\r",
				        vnum,
				        pnote->sender,
				        pnote->to_list,
				        pnote->date,
				        pnote->subject);
				stc(buf, ch);
				stc("{W================================================================================{x\n\r", ch);
				page_to_char(pnote->text, ch);
				update_read(ch, pnote);
				return;
			}
			else if (is_note_to(ch, pnote))
				vnum++;
		}
	}

	stc("That's all folks!\n\r", ch);
	return;
}


/* Chronological NEXT -- Elrac */
void do_next(CHAR_DATA *ch, char *argument)
{
	struct board_index_struct *pbis, *obis = NULL;
	time_t ostamp = (time_t) 0;
	NOTE_DATA *pnote, *onote = NULL;
	NOTE_DATA **plist;
	int nnum, onum = 0;

	if (IS_NPC(ch)) {
		stc("Sorry, mobiles can't read notes.\n\r", ch);
		return;
	}

	/* with an argument, e.g. 'next unread', they get old format */
	if (argument[0] != '\0') {
		do_old_next(ch);
		return;
	}

	/* loop thru all boards, find the lowest unread note timestamp */
	for (pbis = board_index; pbis->board_hdr[0]; pbis++) {
		plist = pbis->board_list;
		nnum = 0;

		/* find the oldest non-hidden note on the board */
		for (pnote = *plist; pnote != NULL; pnote = pnote->next) {
			if (!hide_note(ch, pnote)) {
				if (onote == NULL || pnote->date_stamp <= ostamp) {
					obis = pbis;
					onum = nnum;
					onote = pnote;
					ostamp = pnote->date_stamp;
				}

				break;
			}
			else if (is_note_to(ch, pnote))
				nnum++;
		}
	}

	/* was there an unread note? */
	if (onote == NULL) {
		stc("That's all folks!\n\r", ch);
		return;
	}

	/* there was a note -- show it to player */
	ptc(ch, "{W[%s{W: {x%3d{W] From:{x %s\n\r"
	    "           {WTo: {x%s\n\r"
	    "         {WDate: {x%s\n\r"
	    "      {WSubject: {x%s\n\r",
	    obis->board_hdr,
	    onum,
	    onote->sender,
	    onote->to_list,
	    onote->date,
	    onote->subject);
	stc("{W================================================================================{x\n\r", ch);
	page_to_char(onote->text, ch);
	update_read(ch, onote);
} /* end do_next */


/*
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap() written by Surreality.
 */
char *format_string(char *oldstring)
{
	char xbuf[MAX_STRING_LENGTH], xbuf2[MAX_STRING_LENGTH];
	char *rdesc;
	int i = 0, j;
	bool cap = TRUE, blankline = FALSE;
	xbuf[0] = xbuf2[0] = 0;

	for (rdesc = oldstring; *rdesc; rdesc++) {
		/* change line breaks to spaces, so we can reformat the width.  keep track of
		   the line breaks, 2 in a row means a blank line, we simply leave in a \n for
		   that, it'll be formatted in the next loop -- Montrey */
		if (*rdesc == '\n') {
			if (blankline) {
				xbuf[i] = '\n';
				cap = TRUE;     /* capitalize the first word after a blank line */
				i++;
			}
			else if (xbuf[i - 1] != ' ') {
				xbuf[i] = ' ';
				i++;
			}

			blankline = TRUE;
			continue;
		}

		/* ignore \r */
		if (*rdesc == '\r')
			continue;

		/* eliminate double spaces, we'll double space sentences below */
		if (*rdesc == ' ') {
			if (xbuf[i - 1] != ' ') {
				xbuf[i] = ' ';
				i++;
			}

			continue;
		}

		/* safe to say this line isn't empty */
		blankline = FALSE;

		/* after the end of a sentence, eliminate spaces before a close parentheses */
		if (*rdesc == ')') {
			if (xbuf[i - 1] == ' '
			    && xbuf[i - 2] == ' '
			    && (xbuf[i - 3] == '.'
			        || xbuf[i - 3] == '?'
			        || xbuf[i - 3] == '!')) {
				xbuf[i - 2] = *rdesc;
				xbuf[i - 1] = ' ';
				xbuf[i] = ' ';
				i++;
			}
			else {
				xbuf[i] = *rdesc;
				i++;
			}
		}
		/* run together punctuation marks, fiddle with quotations, double space sentences.
		   capitalize the next letter after the punctuation mark is found */
		else if (*rdesc == '.'
		         || *rdesc == '?'
		         || *rdesc == '!') {
			if (xbuf[i - 1] == ' '
			    && xbuf[i - 2] == ' '
			    && (xbuf[i - 3] == '.'
			        || xbuf[i - 3] == '?'
			        || xbuf[i - 3] == '!')) {
				xbuf[i - 2] = *rdesc;

				if (*(rdesc + 1) != '\"') {
					xbuf[i - 1] = ' ';
					xbuf[i] = ' ';
					i++;
				}
				else {
					xbuf[i - 1] = '\"';
					xbuf[i] = ' ';
					xbuf[i + 1] = ' ';
					i += 2;
					rdesc++;
				}
			}
			else {
				xbuf[i] = *rdesc;

				if (*(rdesc + 1) != '\"') {
					xbuf[i + 1] = ' ';
					xbuf[i + 2] = ' ';
					i += 3;
				}
				else {
					xbuf[i + 1] = '\"';
					xbuf[i + 2] = ' ';
					xbuf[i + 3] = ' ';
					i += 4;
					rdesc++;
				}
			}

			cap = TRUE;
		}
		/* it's a letter, if it's the start of a new sentence, capitalize it */
		else {
			xbuf[i] = *rdesc;

			if (cap) {
				cap = FALSE;
				xbuf[i] = UPPER(xbuf[i]);
			}

			i++;
		}
	}

	/* clean up the end, reset our pointer */
	xbuf[i] = 0;
	strcpy(xbuf2, xbuf);
	rdesc = xbuf2;
	xbuf[0] = 0;
	blankline = FALSE;

	for (; ;) {
		/* we use 2 counters instead of the old way using one, because we want to
		   ignore color codes.  i is the actual iterator, j is the counter of how
		   many displayed characters we have in our line  -- Montrey */
		for (i = 0, j = 0; * (rdesc + i) && j < 77; i++) {
			/* the above loop will eliminate line breaks, EXCEPT when they wanted
			   a blank line.  a line break in here means we need to reinsert a blank */
			if (*(rdesc + i) == '\n')
				break;
			else if (*(rdesc + i) == '{') {
				if (*(rdesc + i + 1) == '{')
					j++;

				i++;
			}
			else
				j++;
		}

		/* if current character is a line break, insert a blank line and continue */
		if (*(rdesc + i) == '\n') {
			*(rdesc + i) = 0;
			strcat(xbuf, rdesc);

			if (!blankline)
				strcat(xbuf, "\n\r");

			strcat(xbuf, "\n\r");
			blankline = TRUE;
			rdesc += i + 1;

			while (*rdesc == ' ')
				rdesc++;

			continue;
		}

		blankline = FALSE;

		if (j < 77)
			break;

		/* backtrack until we hit a space */
		for (; i; i--)
			if (*(rdesc + i) == ' ')
				break;

		if (i) {
			*(rdesc + i) = 0;
			strcat(xbuf, rdesc);
			strcat(xbuf, "\n\r");
			rdesc += i + 1;

			while (*rdesc == ' ')
				rdesc++;
		}
		else {
			/* means that they have a single word longer than 77 characters, cut it off */
			*(rdesc + 75) = 0;
			strcat(xbuf, rdesc);
			strcat(xbuf, "-\n\r");
			rdesc += 76;
		}
	}

	/* eliminate trailing spaces or line breaks */
	while (*(rdesc + i) && (*(rdesc + i) == ' '
	                        || *(rdesc + i) == '\n'
	                        || *(rdesc + i) == '\r'))
		i--;

	*(rdesc + i + 1) = 0;
	strcat(xbuf, rdesc);

	if (xbuf[strlen(xbuf) - 2] != '\n')
		strcat(xbuf, "\n\r");

	free_string(oldstring);
	return (str_dup(xbuf));
}


char *string_replace(char *orig, char *old, char *new)
{
	char xbuf[MAX_STRING_LENGTH];
	int i;
	xbuf[0] = '\0';
	strcpy(xbuf, orig);

	if (strstr(orig, old) != NULL) {
		i = strlen(orig) - strlen(strstr(orig, old));
		xbuf[i] = '\0';
		strcat(xbuf, new);
		strcat(xbuf, &orig[i + strlen(old)]);
		free_string(orig);
	}

	return str_dup(xbuf);
}


/* in support of war coding -- Elrac */
NOTE_DATA *get_declaration(CHAR_DATA *ch, int number)
{
	NOTE_DATA **list;
	NOTE_DATA *pnote;
	int cur_num;
	list = &roleplay_list;
	cur_num = 0;

	for (pnote = *list; pnote != NULL; pnote = pnote->next) {
		if (is_note_to(ch, pnote) && (cur_num++ == number))
			return pnote;
	}

	return NULL;
} /* end get_declaration() */

