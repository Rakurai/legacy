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

#include "Note.hh"

#include <cstring>
#include <vector>

#include "argument.hh"
#include "channels.hh"
#include "Character.hh"
#include "Clan.hh"
#include "file.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "Logging.hh"
#include "memory.hh"
#include "merc.hh"
#include "Player.hh"
#include "World.hh"


/* local procedures */
void load_thread(char *name, Note **list, int type, time_t free_time);
void parse_note(Character *ch, String argument, int type);
bool hide_note(Character *ch, Note *pnote);

Note *note_list;
Note *idea_list;
Note *roleplay_list;
Note *immquest_list;
Note *changes_list;
Note *personal_list;
Note *trade_list;

/* Names for note types. Keep these in line with the
   #define names NOTE_NOTE, NOTE_IDEA etc. in merc.h !!! */
const std::vector<board_index_struct> board_index = {
	{ "{PN", &note_list,    "note",                 "notes",                "note"                  },
	{ "{YI", &idea_list,    "idea",                 "ideas",                "idea"                  },
	{ "{VR", &roleplay_list, "roleplay",             "roleplays",            "roleplaying note"      },
	{ "{BQ", &immquest_list, "immquest",             "immquests",            "quest note"            },
	{ "{GC", &changes_list, "change",               "changes",              "change"                },
	{ "{CP", &personal_list, "personal message",     "personal messages",    "personal message"      },
	{ "{bT", &trade_list,   "trade note",           "trade notes",          "trade note"            },
};

/* count the number of messages, visible to 'ch', in a given message list */
int count_spool(Character *ch, Note *spool)
{
	int count = 0;
	Note *pnote;

	for (pnote = spool; pnote != nullptr; pnote = pnote->next)
		if (!hide_note(ch, pnote))
			count++;

	return count;
}

/* display the numbers of unread messages of each type, visible to 'ch' */
void do_unread(Character *ch, String argument)
{
	int count;
	bool found = false;

	if (ch->is_npc()) {
		stc("Sorry, only players can read notes!\n", ch);
		return;
	}

	if ((count = count_spool(ch, immquest_list)) > 0) {
		found = true;
		ptc(ch, "There %s %d new {Bimmquest{x note%s waiting.\n",
		    count > 1 ? "are" : "is",
		    count,
		    count > 1 ? "s" : "");
	}

	if ((count = count_spool(ch, changes_list)) > 0) {
		found = true;
		ptc(ch, "There %s %d {Gchange%s{x waiting to be read.\n",
		    count > 1 ? "are" : "is",
		    count,
		    count > 1 ? "s" : "");
	}

	if ((count = count_spool(ch, note_list)) > 0) {
		found = true;
		ptc(ch, "You have %d new {Pnote%s{x waiting.\n",
		    count,
		    count > 1 ? "s" : "");
	}

	if ((count = count_spool(ch, idea_list)) > 0) {
		found = true;
		ptc(ch, "You have %d unread {Yidea%s{x to pursue.\n",
		    count,
		    count > 1 ? "s" : "");
	}

	if ((count = count_spool(ch, roleplay_list)) > 0) {
		found = true;
		ptc(ch, "%d {Vroleplay%s been added.\n",
		    count,
		    count > 1 ? "ing{x notes have" : "{x note has");
	}

	if ((count = count_spool(ch, personal_list)) > 0) {
		found = true;
		ptc(ch, "You have %d {Cpersonal{x message%s to read.\n",
		    count,
		    count > 1 ? "s" : "");
	}

	if ((count = count_spool(ch, trade_list)) > 0) {
		found = true;
		ptc(ch, "You have %d {btrade{x note%s to read.\n",
		    count,
		    count > 1 ? "s" : "");
	}

	if (!found)
		stc("You have no unread notes.\n", ch);
	else {
		stc("Type: {Y<next>{x to scroll through your messages.\n", ch);
		stc("Type: {Y<note wipe>{x to empty all message boards.\n", ch);
	}
}

void do_note(Character *ch, String argument)
{
	parse_note(ch, argument, NOTE_NOTE);
}

void do_idea(Character *ch, String argument)
{
	parse_note(ch, argument, NOTE_IDEA);
}

void do_personal(Character *ch, String argument)
{
	parse_note(ch, argument, NOTE_PERSONAL);
}

void do_roleplay(Character *ch, String argument)
{
	parse_note(ch, argument, NOTE_ROLEPLAY);
}

void do_immquest(Character *ch, String argument)
{
	parse_note(ch, argument, NOTE_IMMQUEST);
}

void do_changes(Character *ch, String argument)
{
	parse_note(ch, argument, NOTE_CHANGES);
}

void do_trade(Character *ch, String argument)
{
	parse_note(ch, argument, NOTE_TRADE);
}

void save_notes(int type)
{
	FILE *fp;
	char *name;
	Note *pnote;

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

	if ((fp = fopen(name, "w")) == nullptr)
		perror(name);
	else {
		for (; pnote != nullptr; pnote = pnote->next) {
			Format::fprintf(fp, "Sender  %s~\n", pnote->sender);
			Format::fprintf(fp, "Date    %s~\n", pnote->date);
			Format::fprintf(fp, "Stamp   %ld\n", pnote->date_stamp);
			Format::fprintf(fp, "To      %s~\n", pnote->to_list);
			Format::fprintf(fp, "Subject %s~\n", pnote->subject);
			Format::fprintf(fp, "Text\n%s~\n",   pnote->text);
		}

		fclose(fp);
		return;
	}
}

void load_notes(void)
{
/*
	load_thread(NOTE_FILE, &note_list, NOTE_NOTE, 10 * 24 * 60 * 60);
	load_thread(IDEA_FILE, &idea_list, NOTE_IDEA, 30 * 24 * 60 * 60);
	load_thread(ROLEPLAY_FILE, &roleplay_list, NOTE_ROLEPLAY, 10 * 24 * 60 * 60);
	load_thread(IMMQUEST_FILE, &immquest_list, NOTE_IMMQUEST, 15 * 24 * 60 * 60);
	load_thread(CHANGES_FILE, &changes_list, NOTE_CHANGES, 90 * 24 * 60 * 60);
	load_thread(PERSONAL_FILE, &personal_list, NOTE_PERSONAL, 10 * 24 * 60 * 60);
	load_thread(TRADE_FILE, &trade_list, NOTE_TRADE, 10 * 24 * 60 * 60);
*/
	load_thread(NOTE_FILE, &note_list, NOTE_NOTE, 0);
	load_thread(IDEA_FILE, &idea_list, NOTE_IDEA, 0);
	load_thread(ROLEPLAY_FILE, &roleplay_list, NOTE_ROLEPLAY, 0);
	load_thread(IMMQUEST_FILE, &immquest_list, NOTE_IMMQUEST, 0);
	load_thread(CHANGES_FILE, &changes_list, NOTE_CHANGES, 0);
	load_thread(PERSONAL_FILE, &personal_list, NOTE_PERSONAL, 0);
	load_thread(TRADE_FILE, &trade_list, NOTE_TRADE, 0);
}

void load_thread(char *name, Note **list, int type, time_t free_time)
{
	FILE *fp;
	Note *pnote, *pnotelast;

	if ((fp = fopen(name, "r")) == nullptr)
		return;

	pnotelast = nullptr;

	for (; ;) {
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
		pnote           = new Note();

		if (fread_word(fp) != "sender")
			break;

		pnote->sender   = fread_string(fp);

		if (fread_word(fp) != "date")
			break;

		pnote->date     = fread_string(fp);

		if (fread_word(fp) != "stamp")
			break;

		pnote->date_stamp = fread_number(fp);

		if (fread_word(fp) != "to")
			break;

		pnote->to_list  = fread_string(fp);

		if (fread_word(fp) != "subject")
			break;

		pnote->subject  = fread_string(fp);

		if (fread_word(fp) != "text")
			break;

		pnote->text     = fread_string(fp);

		if (free_time && (pnote->date_stamp < Game::current_time - free_time)) {
			delete pnote;
			continue;
		}

		pnote->type = type;

		if (*list == nullptr)
			*list           = pnote;
		else
			pnotelast->next     = pnote;

		pnotelast       = pnote;
	}

	// getting here means last note was unreadable
	delete pnote;

	Logging::file_bug(fp, "Load_notes: bad key word.", 0);
	exit( 1 );
	return;
}

void append_note(Note *pnote)
{
	FILE *fp;
	char *name;
	Note **list;
	Note *last;

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

	if (*list == nullptr)
		*list = pnote;
	else {
		for (last = *list; last->next != nullptr; last = last->next);

		last->next = pnote;
	}

	if ((fp = fopen(name, "a")) == nullptr)
		perror(name);
	else {
		Format::fprintf(fp, "Sender  %s~\n", pnote->sender.replace("~", "-"));
		Format::fprintf(fp, "Date    %s~\n", pnote->date);
		Format::fprintf(fp, "Stamp   %ld\n", pnote->date_stamp);
		Format::fprintf(fp, "To      %s~\n", pnote->to_list.replace("~", "-"));
		Format::fprintf(fp, "Subject %s~\n", pnote->subject.replace("~", "-"));
		Format::fprintf(fp, "Text\n%s~\n", pnote->text.replace("~", "-"));
		fclose(fp);
		/* Mud has crashed on above line before */
	}
}

bool is_note_to(Character *ch, Note *pnote)
{
	char buf[MSL];
	/* don't show notes to the forwarding person *mutter*  -- Montrey */
	Format::sprintf(buf, "FORWARD(%s)", ch->name);

	if (std::strstr(buf, pnote->subject.uncolor().c_str()))
		return false;

	/* note to followers */
	if (pnote->to_list.has_words("followers")) {
		int i = parse_deity(ch->pcdata->deity);

		if (i >= 0 && pnote->sender == deity_table[i].name)
			return true;
	}

	if (ch->name == pnote->sender)
		return true;

	if (pnote->to_list.has_words("spam")) {
		if (ch->censor_flags.has(CENSOR_SPAM))
			return false;

		return true;
	}

	if (pnote->to_list.has_exact_words("all"))
		return true;

	if (IS_IMP(ch) && pnote->to_list.has_words("imp"))
		return true;

	if (IS_HEAD(ch) && pnote->to_list.has_words("head"))
		return true;

	if (IS_IMMORTAL(ch)
	    && (pnote->to_list.has_words("immortal")
	        || pnote->to_list.has_words("imm")))
		return true;

	if (pnote->to_list.has_exact_words(ch->name))
		return true;

	/* note to clans -- Montrey */
	if (ch->clan
	    && (pnote->to_list.has_exact_words(ch->clan->name)      /* note to one clan */
	        || pnote->to_list.has_words("clan"))) {                /* note to all clans */
		if (pnote->to_list.has_words("leader"))          /* note to leaders */
			if (!ch->has_cgroup(GROUP_LEADER))
				return false;

		if (pnote->to_list.has_words("deputy")           /* note to deputies */
		    || pnote->to_list.has_words("deputies"))
			if (!ch->has_cgroup(GROUP_LEADER)
			    && !ch->has_cgroup(GROUP_DEPUTY))
				return false;

		return true;
	}

	return false;
}

void note_attach(Character *ch, int type)
{
	Note *pnote;

	if (ch->pnote != nullptr)
		return;

	pnote = new Note();
	pnote->sender       = ch->is_npc() ? ch->short_descr : ch->name;
	pnote->type         = type;
	ch->pnote           = pnote;
	return;
}

void note_remove(Character *ch, Note *pnote, bool del)
{
	Note *prev;
	Note **list;

	if (!del) {
		/* make a new list */
		String to_new, to_list = pnote->to_list;

		while (!to_list.empty()) {
			String to_one;
			to_list = one_argument(to_list, to_one);

			if (!to_one.empty() && ch->name != to_one) {
				to_new += " ";
				to_new += to_one;
			}
		}

		/* Just a simple recipient removal? */
		if (ch->name != pnote->sender && !to_new.empty()) {
			pnote->to_list = to_new.substr(1);
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
		for (prev = *list; prev != nullptr; prev = prev->next) {
			if (prev->next == pnote)
				break;
		}

		if (prev == nullptr) {
			Logging::bug("Note_remove: pnote not found.", 0);
			return;
		}

		prev->next = pnote->next;
	}

	save_notes(pnote->type);
	delete pnote;
	return;
}

bool hide_note(Character *ch, Note *pnote)
{
	time_t last_read;

	/* Mob notes
	    if (ch->is_npc())
	        return true;
	*/
	switch (pnote->type) {
	default:
		return true;

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
		return true;

	if (ch->name == pnote->sender)
		return true;

	if (!is_note_to(ch, pnote))
		return true;

	return false;
}

void update_read(Character *ch, Note *pnote)
{
	time_t stamp;
	/* Mob Notes
	    if (ch->is_npc())
	        return;
	*/
	stamp = pnote->date_stamp;

	switch (pnote->type) {
	default:
		return;

	case NOTE_NOTE:
		ch->pcdata->last_note = std::max(ch->pcdata->last_note, stamp);
		break;

	case NOTE_IDEA:
		ch->pcdata->last_idea = std::max(ch->pcdata->last_idea, stamp);
		break;

	case NOTE_ROLEPLAY:
		ch->pcdata->last_roleplay = std::max(ch->pcdata->last_roleplay, stamp);
		break;

	case NOTE_IMMQUEST:
		ch->pcdata->last_immquest = std::max(ch->pcdata->last_immquest, stamp);
		break;

	case NOTE_CHANGES:
		ch->pcdata->last_changes = std::max(ch->pcdata->last_changes, stamp);
		break;

	case NOTE_PERSONAL:
		ch->pcdata->last_personal = std::max(ch->pcdata->last_personal, stamp);
		break;

	case NOTE_TRADE:
		ch->pcdata->last_trade = std::max(ch->pcdata->last_trade, stamp);
		break;
	}
}

void notify_note_post(Note *pnote, Character *vch, int type)
{
	Character *ch;
	String buf;
	const String& list_name = board_index[type].board_long;

	for (ch = Game::world().char_list; ch != nullptr; ch = ch->next) {
		if (ch->is_npc())
			continue;

		if (is_note_to(ch, pnote)) {
			Format::sprintf(buf, "{W[FYI] New %s from %s. Subject: [%s]. ",
			        list_name, pnote->sender, pnote->subject);

			if (ch->clan && pnote->to_list.has_words(ch->clan->name))
				buf += "Guild note.{x\n";
			else if (pnote->to_list.has_exact_words(ch->name))
				buf += "Personal note.{x\n";
			else if (pnote->to_list.has_words("all"))
				buf += "Global note.{x\n";
			else if (IS_IMMORTAL(ch) && pnote->to_list.has_words("immortal"))
				buf += "Immortal note.{x\n";
			else
				buf += "{x\n";

			if (ch != vch
			    /* don't info author of forward -- Elrac */
			    && strcmp(pnote->sender, ch->name)
			    && !ch->pcdata->plr_flags.has(PLR_NONOTIFY))
				stc(buf, ch);
		}
	}
}

void parse_note(Character *ch, String argument, int type)
{
	String buffer;
	char buf[MSL];
	Note *pnote;
	Note **list;
	
	int vnum = 0, anum = 0;
	/* NOTE: Mobs CAN currently do notes. Don't do anything player-specific! */
	list = board_index[type].board_list;
	const String& list_name = board_index[type].board_plural;

	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty() || arg.is_prefix_of("read")) {
		bool fAll = false;

		if (ch->is_npc()) {
			stc("Sorry, mobiles can't read notes.\n", ch);
			return;
		}

		/* read next unread note */
		if (argument.empty() || argument.is_prefix_of("next")) {
			for (pnote = *list; pnote != nullptr; pnote = pnote->next) {
				if (!hide_note(ch, pnote)) {
					ptc(ch, "{W[%3d] From: {x%s\n"
					    "        {WTo: {x%s\n"
					    "      {WDate: {x%s\n"
					    "   {WSubject: {x%s\n"
					    "{W================================================================================{x\n",
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

			ptc(ch, "You have no unread %s.\n", list_name);
			return;
		}
		else if (argument.is_number())
			anum = atoi(argument);
		else if (argument == "all")
			fAll = true;
		else {
			stc("Read which number?\n", ch);
			return;
		}

		for (pnote = *list; pnote != nullptr; pnote = pnote->next) {
			if (is_note_to(ch, pnote) && (vnum++ == anum || fAll)) {
				ptc(ch, "{W[%3d] From: {x%s\n"
				    "        {WTo: {x%s\n"
				    "      {WDate: {x%s\n"
				    "   {WSubject: {x%s\n"
				    "{W================================================================================{x\n",
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

		ptc(ch, "There aren't that many %s.\n", list_name);
		return;
	}

	if (arg.is_prefix_of("list")) {
		bool search = false, nw = false, found = false, all = false;

		if (ch->is_npc()) {
			stc("Sorry, mobiles can't read notes.\n", ch);
			return;
		}

		if (argument == "new")
			nw = true;
		else if (argument == "all")
			all = true;
		else if (!argument.empty())
			search = true;


		for (pnote = *list, vnum = -1; pnote != nullptr; pnote = pnote->next) {
			if (is_note_to(ch, pnote)) {
				vnum++;

				if ((search) && !argument.is_prefix_of(pnote->sender))
					continue;

				if ((nw) && hide_note(ch, pnote))
					continue;

				Format::sprintf(buf, "{W[{x%3d%s{W][{x%12s{W]{x %s{x\n",
				        vnum,
				        hide_note(ch, pnote) ? " " : "N",
				        pnote->sender,
				        pnote->subject);

				if (all)
					stc(buf, ch);
				else
					buffer += buf;

				found = true;
			}
		}

		if (!found)
			stc("There are no messages that match that criteria.\n", ch);

		if (!all)
			page_to_char(buffer, ch);

		return;
	}

	if (arg.is_prefix_of("remove")) {
		if (!argument.is_number()) {
			stc("Note remove which number?\n", ch);
			return;
		}

		anum = atoi(argument);
		vnum = 0;

		for (pnote = *list; pnote != nullptr; pnote = pnote->next) {
			if (is_note_to(ch, pnote) && vnum++ == anum) {
				note_remove(ch, pnote, false);
				stc("Message removed.\n", ch);
				return;
			}
		}

		Format::sprintf(buf, "There aren't that many %s.", list_name);
		stc(buf, ch);
		return;
	}

	if (arg.is_prefix_of("delete") && IS_IMP(ch)) {
		if (!argument.is_number()) {
			stc("Message delete which number?\n", ch);
			return;
		}

		anum = atoi(argument);
		vnum = 0;

		for (pnote = *list; pnote != nullptr; pnote = pnote->next) {
			if (is_note_to(ch, pnote) && vnum++ == anum) {
				note_remove(ch, pnote, true);
				stc("Message deleted.\n", ch);
				return;
			}
		}

		Format::sprintf(buf, "There aren't that many %s.\n", list_name);
		stc(buf, ch);
		return;
	}

	if (arg.is_prefix_of("forward")) {
		Note *newnote;

		if (ch->is_npc()) {
			stc("Mobs can't forward notes.\n", ch);
			return;
		}

		String forward;
		argument = one_argument(argument, forward);

		if (forward.empty()) {
			stc("Foward note to who?\n", ch);
			return;
		}

		if (!argument.is_number()) {
			stc("Note forward which number?\n", ch);
			return;
		}

		anum = atoi(argument);
		vnum = 0;

		for (pnote = *list; pnote != nullptr; pnote = pnote->next) {
			if (is_note_to(ch, pnote) && vnum++ == anum) {
				newnote = new Note();
				newnote->sender   = pnote->sender;
				newnote->date     = pnote->date;
				newnote->date_stamp           = Game::current_time;
				newnote->to_list  = forward;
				/* is_note_to relies on the text before the forwarding person's name
				   to be 14 characters, including color codes.  change it if you
				   change this!  the smash_bracket is to assure there's no color
				   codes in their name, even though mobs can't forward -- Montrey */
				Format::sprintf(buf, "{VFORWARD{W({V%s{W){x: %s", ch->name.uncolor(), pnote->subject);
				newnote->subject  = buf;
				newnote->text     = pnote->text;
				newnote->type     = pnote->type;
				append_note(newnote);
				stc("Note Forwarded.\n", ch);
				notify_note_post(newnote, ch, type);
				return;
			}
		}

		Format::sprintf(buf, "There aren't that many %s.", list_name);
		stc(buf, ch);
		return;
	}

	if (arg.is_prefix_of("repost") && IS_IMMORTAL(ch)) {
		Note *newnote;

		if (!argument.is_number()) {
			stc("Note repost which number?\n", ch);
			return;
		}

		anum = atoi(argument);
		vnum = 0;

		for (pnote = *list; pnote != nullptr; pnote = pnote->next) {
			if (is_note_to(ch, pnote) && vnum++ == anum) {
				newnote = new Note();
				newnote->sender   = pnote->sender;
				newnote->date     = pnote->date;
				newnote->date_stamp           = Game::current_time;
				newnote->to_list  = "Immortal";
				Format::sprintf(buf, "{PIMM REPOST{W({P%s{W){x: %s", ch->name,
				        pnote->subject);
				newnote->subject  = buf;
				newnote->text     = pnote->text;
				newnote->type     = pnote->type;
				append_note(newnote);
				note_remove(ch, pnote, true);
				stc("Note Reposted.\n", ch);
				notify_note_post(newnote, ch, type);
				return;
			}
		}

		Format::sprintf(buf, "There aren't that many %s.", list_name);
		stc(buf, ch);
		return;
	}

	if (arg.is_prefix_of("wipe")) {
		ch->pcdata->last_note = Game::current_time;
		ch->pcdata->last_idea = Game::current_time;
		ch->pcdata->last_roleplay = Game::current_time;
		ch->pcdata->last_changes = Game::current_time;
		ch->pcdata->last_immquest = Game::current_time;
		ch->pcdata->last_personal = Game::current_time;
		ch->pcdata->last_trade = Game::current_time;
		stc("You are now caught up on all messages.\n", ch);
		return;
	}

	if (arg.is_prefix_of("catchup")) {
		switch (type) {
		case NOTE_NOTE:
			ch->pcdata->last_note = Game::current_time;
			stc("You are now caught up on notes.\n", ch);
			break;

		case NOTE_IDEA:
			ch->pcdata->last_idea = Game::current_time;
			stc("You are now caught up on ideas.\n", ch);
			break;

		case NOTE_ROLEPLAY:
			ch->pcdata->last_roleplay = Game::current_time;
			stc("You are now caught up on roleplay notes.\n", ch);
			break;

		case NOTE_IMMQUEST:
			ch->pcdata->last_immquest = Game::current_time;
			stc("You are now caught up on quest notes.\n", ch);
			break;

		case NOTE_CHANGES:
			ch->pcdata->last_changes = Game::current_time;
			stc("You are now caught up on changes.\n", ch);
			break;

		case NOTE_PERSONAL:
			ch->pcdata->last_personal = Game::current_time;
			stc("You are now caught up on personal messages.\n", ch);
			break;

		case NOTE_TRADE:
			ch->pcdata->last_trade = Game::current_time;
			stc("You are now caught up on trade notes.\n", ch);
			break;
		}

		return;
	}

	/* message move: e.g. NOTE MOVE 10 IDEA -- Elrac */
	if (arg.is_prefix_of("move")) {
		Note *thenote, *newnote, *newlist;
		int newtype = 0;
		/* get message number */
		argument = one_argument(argument, arg);

		if (arg.empty() || !arg.is_number()) {
			ptc(ch, "Move which %s number?\n", board_index[type].board_short);
			return;
		}

		anum = atoi(arg);
		/* find the message in the list */
		vnum = 0;
		thenote = nullptr;

		for (pnote = *list; pnote != nullptr; pnote = pnote->next) {
			if (is_note_to(ch, pnote) && vnum++ == anum) {
				thenote = pnote;
				break;
			}
		}

		if (thenote == nullptr) {
			ptc(ch, "There aren't that many %s.", board_index[type].board_long);
			return;
		}

		if (strcmp(thenote->sender, ch->name) && !IS_IMMORTAL(ch)) {
			ptc(ch, "You are not the author of this %s, so you can't move it!\n",
			    board_index[type].board_short);
			return;
		}

		/* get new list name */
		newlist = nullptr;
		argument = one_argument(argument, arg);

		for (unsigned long j = 0; j < board_index.size(); j++) {
			if (arg.is_prefix_of(board_index[j].board_plural)) {
				newtype = j;
				newlist = *(board_index[j].board_list);
				break;
			}
		}

		if (newlist == nullptr) {
			ptc(ch, "There's no such board name as '%s'\n", arg);
			return;
		}

		if (newtype == NOTE_IMMQUEST && !IS_IMMORTAL(ch)) {
			stc("You aren't allowed to move a message to IMMQUEST, sorry!\n", ch);
			return;
		}

		if (newtype == NOTE_CHANGES && !IS_IMMORTAL(ch)) {
			stc("You aren't allowed to move a message to CHANGES, sorry!\n", ch);
			return;
		}

		/* copy message to new list */
		newnote                 = new Note();
		newnote->sender         = pnote->sender;
		newnote->date           = pnote->date;
		newnote->date_stamp     = Game::current_time;
		newnote->to_list        = pnote->to_list;
		newnote->subject        = pnote->subject;
		newnote->text           = pnote->text;
		newnote->type           = newtype;
		append_note(newnote);
		note_remove(ch, pnote, true);
		ptc(ch, "OK, %s %d moved to %s.\n",
		    board_index[type].board_short,
		    anum,
		    board_index[newtype].board_plural);
		notify_note_post(newnote, ch, newtype);
		return;
	}

	if (ch->revoke_flags.has(REVOKE_NOTE)) {
		stc("Your note writing priviledges have been revoked.\n", ch);
		return;
	}

	/* below this point only certain people can edit notes */
	if ((type == NOTE_IMMQUEST || type == NOTE_CHANGES)
	    && !IS_IMMORTAL(ch)) {
		ptc(ch, "You aren't high enough level to write %s.\n", list_name);
		return;
	}

	if (arg == "+") {
		note_attach(ch, type);

		if (ch->pnote->type != type) {
			stc(
			        "You already have a different note in progress.\n", ch);
			return;
		}


		if (strlen(ch->pnote->text) + strlen(argument) >= 4096) {
			stc("Note too long.\n", ch);

			if (!ch->is_npc()) ch->pcdata->plr_flags += PLR_STOPCRASH;

			return;
		}

		buffer += ch->pnote->text;
		buffer += argument;
		buffer += "\n";
		ch->pnote->text = buffer;
		stc("Line added.\n", ch);
		return;
	}

	if (arg == "replace") {
		String old, nw;
		argument = one_argument(argument, old);
		argument = one_argument(argument, nw);

		if ((old.empty()) || (nw.empty())) {
			stc("Usage: note replace 'old string' 'new string'\n",
			    ch);
			return;
		}

		if (ch->pnote == nullptr) {
			stc("You have no note in progress.\n", ch);
			return;
		}

		if (ch->pnote->text.empty()) {
			stc("You have not entered any text.\n", ch);
			return;
		}

		String temp = ch->pnote->text;
		ch->pnote->text = temp.replace(old, nw);
		Format::sprintf(buf, "'%s' replaced with '%s'.\n", old, nw);
		stc(buf, ch);
		return;
	}

	if (arg == "format") {
		if (ch->pnote == nullptr) {
			stc("You have no note in progress.\n", ch);
			return;
		}

		if (ch->pnote->text.empty()) {
			stc("You have not entered any text.\n", ch);
			return;
		}

		ch->pnote->text = format_string(ch->pnote->text);
		stc("Note formatted.\n", ch);
		return;
	}

	if (arg == "-") {
		note_attach(ch, type);

		if (ch->pnote->type != type) {
			stc("You already have a different note in progress.\n", ch);
			return;
		}

		if (ch->pnote->text.empty()) {
			stc("No lines left to remove.\n", ch);
			return;
		}

		if (!ch->is_npc() && ch->pcdata->plr_flags.has(PLR_STOPCRASH)) {
			stc("You cannot edit this note any further.\n", ch);
			stc("Please either post or clear this note.\n", ch);
			return;
		}

		strcpy(buf, ch->pnote->text);
		bool found = false;

		// find the last and second to last newlines, remove all after second to last
		for (int len = strlen(buf); len > 0; len--) {
			if (buf[len] == '\n') {
				if (found) { /* found the second one */
					buf[len + 1] = '\0';
					ch->pnote->text = buf;
					stc("Line removed.\n", ch);
					return;
				}
				else
					found = true;
			}
		}

		buf[0] = '\0';
		ch->pnote->text = buf;
		stc("Line removed.\n", ch);
		return;
	}

	if (arg.is_prefix_of("subject")) {
		note_attach(ch, type);

		if (ch->pnote->type != type) {
			stc(
			        "You already have a different note in progress.\n", ch);
			return;
		}

		ch->pnote->subject = argument;
		stc("Subject set.\n", ch);
		return;
	}

	if (arg.is_prefix_of("to")) {
		if (ch->comm_flags.has(COMM_NOCHANNELS) &&
		    strcmp(argument, "immortal")) {
			stc("You can currently send notes only to immortal.\n", ch);
			return;
		}

		note_attach(ch, type);

		if (ch->pnote->type != type) {
			stc(
			        "You already have a different note in progress.\n", ch);
			return;
		}

		if (argument.empty()) {
			stc("Whom do you wish to address it to?\n", ch);
			return;
		}

		/* if they're not an imm and in a clan, replace 'clan' with their clan's name.
		   but not 'clans'!     -- Montrey */
		if (!IS_IMMORTAL(ch) && ch->clan && argument.has_exact_words("clan")) {
			char line[MSL], *p;
			/* copy string, it'll get mangled */
			strcpy(line, argument);
			p = std::strstr(line, "clan");

			while (*(p + 4) != ' ' && *(p + 4) != '\0')
				p = std::strstr(p + 4, "clan");

			*p = '\0';
			p += 4;
			Format::sprintf(buf, "%s%s%s", line, ch->clan->name.capitalize(), p);
			ch->pnote->to_list = buf;
		}
		else
			ch->pnote->to_list = argument;

		stc("Recipient list set.\n", ch);
		return;
	}

	if (arg.is_prefix_of("clear")) {
		if (ch->pnote != nullptr) {
			delete ch->pnote;
			ch->pnote = nullptr;
		}

		stc("Note cleared.\n", ch);

		if (!ch->is_npc()) ch->pcdata->plr_flags -= PLR_STOPCRASH;

		return;
	}

	if (arg.is_prefix_of("show")) {
		if (ch->pnote == nullptr) {
			stc("You have no note in progress.\n", ch);
			return;
		}

		if (ch->pnote->type != type) {
			stc("You aren't working on that kind of note.\n", ch);
			return;
		}

		Format::sprintf(buf, "   {WFrom: {x%s\n     {WTo: {x%s\n{WSubject: {x%s\n",
		        ch->pnote->sender,
		        ch->pnote->to_list,
		        ch->pnote->subject
		       );
		stc(buf, ch);
		stc("{W================================================================================{x\n", ch);
		stc(ch->pnote->text, ch);
		return;
	}

	if (arg.is_prefix_of("post") || arg.is_prefix_of("send")) {
		char *strtime;
		char buf2[MAX_STRING_LENGTH];

		if (ch->pnote == nullptr) {
			stc("You have no note in progress.\n", ch);
			return;
		}

		if (ch->pnote->type != type) {
			stc("You aren't working on that kind of note.\n", ch);
			return;
		}

		if (ch->pnote->to_list == "") {
			stc(
			        "You need to provide a recipient (name, all, or immortal).\n",
			        ch);
			return;
		}

		if (ch->pnote->subject == "") {
			stc("You need to provide a subject.\n", ch);
			return;
		}

		/* If a note is personal, route it to the personal board - Lotus */
		if ((ch->pnote->type == NOTE_NOTE) &&
		    ((!ch->pnote->to_list.has_exact_words("all"))
		     || (!ch->pnote->to_list.has_exact_words("spam")))) {
			ch->pnote->type = NOTE_PERSONAL;
			type = NOTE_PERSONAL;
		}

		/* If a note is not personal, rout it to the note board */
		if ((ch->pnote->type == NOTE_PERSONAL) &&
		    (ch->pnote->to_list.has_exact_words("all") ||
		     ch->pnote->to_list.has_exact_words("spam"))) {
			ch->pnote->type = NOTE_NOTE;
			type = NOTE_NOTE;
		}

		ch->pnote->next                 = nullptr;
		strtime                         = ctime(&Game::current_time);
		strtime[strlen(strtime) - 1]      = '\0';
		ch->pnote->date                 = strtime;
		ch->pnote->date_stamp           = Game::current_time;
		Format::sprintf(buf2, "%s has just posted a %s to: %s", ch->name,
		        board_index[type].board_long, ch->pnote->to_list);
		wiznet(buf2, ch, nullptr, WIZ_MAIL, 0, GET_RANK(ch));
		append_note(ch->pnote);
		notify_note_post(ch->pnote, ch, type);
		ch->pnote = nullptr;
		Format::sprintf(buf2, "Your %s has been posted.\n", board_index[type].board_long);
		stc(buf2, ch);

		if (!ch->is_npc()) ch->pcdata->plr_flags -= PLR_STOPCRASH;

		return;
	}

	stc("You can't do that.\n", ch);
	return;
}

void do_old_next(Character *ch)
{
	char buf[MAX_STRING_LENGTH];
	Note *pnote;
	Note **list;
	int vnum;

	if (ch->is_npc()) {
		stc("Sorry, mobiles can't read notes.\n" , ch);
		return;
	}

	list = &note_list;
	pnote = *list;

	if (pnote != nullptr) {
		vnum = 0;

		for (pnote = *list; pnote != nullptr; pnote = pnote->next) {
			if (!hide_note(ch, pnote)) {
				Format::sprintf(buf,
				        "{W[{PN{W: {x%3d{W] From:{x %s\n           {WTo: {x%s\n         {WDate: {x%s\n      {WSubject: {x%s\n",
				        vnum,
				        pnote->sender,
				        pnote->to_list,
				        pnote->date,
				        pnote->subject);
				stc(buf, ch);
				stc("{W================================================================================{x\n", ch);
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

	if (pnote != nullptr) {
		vnum = 0;

		for (pnote = *list; pnote != nullptr; pnote = pnote->next) {
			if (!hide_note(ch, pnote)) {
				Format::sprintf(buf,
				        "{W[{YI{W: {x%3d{W] From:{x %s\n           {WTo: {x%s\n         {WDate: {x%s\n      {WSubject: {x%s\n",
				        vnum,
				        pnote->sender,
				        pnote->to_list,
				        pnote->date,
				        pnote->subject);
				stc(buf, ch);
				stc("{W================================================================================{x\n", ch);
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

	if (pnote != nullptr) {
		vnum = 0;

		for (pnote = *list; pnote != nullptr; pnote = pnote->next) {
			if (!hide_note(ch, pnote)) {
				Format::sprintf(buf,
				        "{W[{GC{W: {x%3d{W] From:{x %s\n           {WTo: {x%s\n         {WDate: {x%s\n      {WSubject: {x%s\n",
				        vnum,
				        pnote->sender,
				        pnote->to_list,
				        pnote->date,
				        pnote->subject);
				stc(buf, ch);
				stc("{W================================================================================{x\n", ch);
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

	if (pnote != nullptr) {
		vnum = 0;

		for (pnote = *list; pnote != nullptr; pnote = pnote->next) {
			if (!hide_note(ch, pnote)) {
				Format::sprintf(buf,
				        "{W[{BQ{W: {x%3d{W] From:{x %s\n           {WTo: {x%s\n         {WDate: {x%s\n      {WSubject: {x%s\n",
				        vnum,
				        pnote->sender,
				        pnote->to_list,
				        pnote->date,
				        pnote->subject);
				stc(buf, ch);
				stc("{W================================================================================{x\n", ch);
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

	if (pnote != nullptr) {
		vnum = 0;

		for (pnote = *list; pnote != nullptr; pnote = pnote->next) {
			if (!hide_note(ch, pnote)) {
				Format::sprintf(buf,
				        "{W[{VR{W: {x%3d{W] From:{x %s\n           {WTo: {x%s\n         {WDate: {x%s\n      {WSubject: {x%s\n",
				        vnum,
				        pnote->sender,
				        pnote->to_list,
				        pnote->date,
				        pnote->subject);
				stc(buf, ch);
				stc("{W================================================================================{x\n", ch);
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

	if (pnote != nullptr) {
		vnum = 0;

		for (pnote = *list; pnote != nullptr; pnote = pnote->next) {
			if (!hide_note(ch, pnote)) {
				Format::sprintf(buf,
				        "{W[{CP{W: {x%3d{W] From:{x %s\n           {WTo: {x%s\n         {WDate: {x%s\n      {WSubject: {x%s\n",
				        vnum,
				        pnote->sender,
				        pnote->to_list,
				        pnote->date,
				        pnote->subject);
				stc(buf, ch);
				stc("{W================================================================================{x\n", ch);
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

	if (pnote != nullptr) {
		vnum = 0;

		for (pnote = *list; pnote != nullptr; pnote = pnote->next) {
			if (!hide_note(ch, pnote)) {
				Format::sprintf(buf,
				        "{W[{bT{W: {x%3d{W] From:{x %s\n           {WTo: {x%s\n         {WDate: {x%s\n      {WSubject: {x%s\n",
				        vnum,
				        pnote->sender,
				        pnote->to_list,
				        pnote->date,
				        pnote->subject);
				stc(buf, ch);
				stc("{W================================================================================{x\n", ch);
				page_to_char(pnote->text, ch);
				update_read(ch, pnote);
				return;
			}
			else if (is_note_to(ch, pnote))
				vnum++;
		}
	}

	stc("That's all folks!\n", ch);
	return;
}

/* Chronological NEXT -- Elrac */
void do_next(Character *ch, String argument)
{
	const board_index_struct *obis = nullptr;
	time_t ostamp = (time_t) 0;
	Note *pnote, *onote = nullptr;
	Note **plist;
	int nnum, onum = 0;

	if (ch->is_npc()) {
		stc("Sorry, mobiles can't read notes.\n", ch);
		return;
	}

	/* with an argument, e.g. 'next unread', they get old format */
	if (!argument.empty()) {
		do_old_next(ch);
		return;
	}

	/* loop thru all boards, find the lowest unread note timestamp */
	for (auto it = board_index.begin(); it != board_index.end(); ++it) {
		plist = it->board_list;
		nnum = 0;

		/* find the oldest non-hidden note on the board */
		for (pnote = *plist; pnote != nullptr; pnote = pnote->next) {
			if (!hide_note(ch, pnote)) {
				if (onote == nullptr || pnote->date_stamp <= ostamp) {
					obis = &*it;
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
	if (onote == nullptr) {
		stc("That's all folks!\n", ch);
		return;
	}

	/* there was a note -- show it to player */
	ptc(ch, "{W[%s{W: {x%3d{W] From:{x %s\n"
	    "           {WTo: {x%s\n"
	    "         {WDate: {x%s\n"
	    "      {WSubject: {x%s\n",
	    obis->board_hdr,
	    onum,
	    onote->sender,
	    onote->to_list,
	    onote->date,
	    onote->subject);
	stc("{W================================================================================{x\n", ch);
	page_to_char(onote->text, ch);
	update_read(ch, onote);
} /* end do_next */

// wrap a line of text, aware of color codes
const String wrap_string(const String& s, unsigned long wrap_len) {
	String str(s), output;
	unsigned long color_pos = 0, uncolor_pos = 0, last_space_pos = 0;

	while (color_pos < str.size()) {
		switch (str[color_pos]) {
			case '{':
				if (color_pos < str.size()-1
				 && str[color_pos+1] == '{')
					uncolor_pos++;

				color_pos += 2;
				break;
			case ' ':
				last_space_pos = color_pos;
				// fall through to increment
			default:
				uncolor_pos++;
				color_pos++;
				break;
		}

		if (uncolor_pos > wrap_len) {
			output += str.substr(0, last_space_pos);
			output += '\n';
			str = str.substr(last_space_pos+1);
			color_pos = uncolor_pos = last_space_pos = 0;
		}
	}

	output += str;
	return output;
}

// transform a block of newline-separated text into a single formatted line
const String format_paragraph(const String& orig_paragraph) {
	// take out internal newlines so we can wrap it
	String paragraph = orig_paragraph.replace("\n", " ").strip();

	if (paragraph.empty())
		return "";

	String buf;
	unsigned long pos;

	// take out double spacing so we can normalize it
	while ((pos = paragraph.find("  ")) != std::string::npos)
		paragraph = paragraph.substr(0, pos) + paragraph.substr(pos+1);

	// remove spaces between a sentence end and a closing parentheses
	paragraph = paragraph.replace(". )", ".)");
	paragraph = paragraph.replace("! )", "!)");
	paragraph = paragraph.replace("? )", "?)");

	// capitalize first letter after sentence end
	for (unsigned long pos = 0; pos < paragraph.size()-2; pos++)
		if (paragraph[pos+1] == ' '
		 && (paragraph[pos] == '.'
		  || paragraph[pos] == '!'
		  || paragraph[pos] == '?'))
			paragraph[pos+2] = toupper(paragraph[pos+2]);

	// double space sentences
	paragraph = paragraph.replace(". ", ".  ");
	paragraph = paragraph.replace("! ", "!  ");
	paragraph = paragraph.replace("? ", "?  ");

	return String("    ") + paragraph;
}

// split a page into a sequence of paragraphs, wrap each, and make output
const String format_page(const String& orig_page, int wrap_len) {
	String page(orig_page), paragraph;
	std::vector<String> paragraphs;

	// preserve initial newlines
	while (!page.empty() && page.front() == '\n') {
		page.erase(0, 1);
		paragraphs.push_back("");
	}

	// break on a double newline
	while (!page.empty()) {
		unsigned long pos = page.find("\n\n");

		if (pos == std::string::npos) {
			paragraph = page;
			page.clear();
		}
		else {
			paragraph = page.substr(0, pos);

			if (pos < page.size()-1)
				page = page.substr(pos+1); // keeps one of the newlines to push
			else
				page.clear();
		}

		paragraphs.push_back(wrap_string(format_paragraph(paragraph), wrap_len));

		// add additional newlines after a double newline
		while (!page.empty() && page.front() == '\n') {
			page.erase(0, 1);
			paragraphs.push_back("");
		}
	}

	String output;

	for (String& paragraph: paragraphs) {
		output += paragraph;
		output += '\n';
	}

	return output;
}

String format_string(const String& s) {
	return format_page(s, 77);
}

/*
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap() written by Surreality.
 */
String format_string_old(const String& oldstring)
{
	String xbuf;
	char xbuf2[MAX_STRING_LENGTH];
	char *rdesc;
	int i = 0, j;
	bool cap = true, blankline = false;
	xbuf2[0] = '\0';

	char oldbuf[MSL];
	strcpy(oldbuf, oldstring);

	for (rdesc = oldbuf; *rdesc; rdesc++) {
		/* change line breaks to spaces, so we can reformat the width.  keep track of
		   the line breaks, 2 in a row means a blank line, we simply leave in a \n for
		   that, it'll be formatted in the next loop -- Montrey */
		if (*rdesc == '\n') {
			if (blankline) {
				xbuf[i] = '\n';
				cap = true;     /* capitalize the first word after a blank line */
				i++;
			}
			else if (xbuf[i - 1] != ' ') {
				xbuf[i] = ' ';
				i++;
			}

			blankline = true;
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
		blankline = false;

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
		else if (*rdesc == '.' || *rdesc == '?' || *rdesc == '!') {
			// do we have ".  ."
			if (xbuf[i - 1] == ' '
			 && xbuf[i - 2] == ' '
			 && (xbuf[i - 3] == '.'
			  || xbuf[i - 3] == '?'
			  || xbuf[i - 3] == '!')) {

				// change to ".."
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

			cap = true;
		}
		/* it's a letter, if it's the start of a new sentence, capitalize it */
		else {
			xbuf[i] = *rdesc;

			if (cap) {
				cap = false;
				xbuf[i] = toupper(xbuf[i]);
			}

			i++;
		}
	}

	/* clean up the end, reset our pointer */
//	xbuf.erase(i);
	strcpy(xbuf2, xbuf);
	rdesc = xbuf2;
	xbuf.clear();
	blankline = false;

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
			xbuf += rdesc;

			if (!blankline)
				xbuf += "\n";

			xbuf += "\n";
			blankline = true;
			rdesc += i + 1;

			while (*rdesc == ' ')
				rdesc++;

			continue;
		}

		blankline = false;

		if (j < 77)
			break;

		/* backtrack until we hit a space */
		for (; i; i--)
			if (*(rdesc + i) == ' ')
				break;

		if (i) {
			*(rdesc + i) = 0;
			xbuf += rdesc;
			xbuf += "\n";
			rdesc += i + 1;

			while (*rdesc == ' ')
				rdesc++;
		}
		else {
			/* means that they have a single word longer than 77 characters, cut it off */
			*(rdesc + 75) = 0;
			xbuf += rdesc;
			xbuf += "-\n";
			rdesc += 76;
		}
	}

	/* eliminate trailing spaces or line breaks */
	while (*(rdesc + i) && (*(rdesc + i) == ' '
	                        || *(rdesc + i) == '\n'
	                        || *(rdesc + i) == '\r'))
		i--;

	*(rdesc + i + 1) = 0;
	xbuf += rdesc;

	if (xbuf[strlen(xbuf) - 2] != '\n')
		xbuf += "\n";

	return xbuf;
}
