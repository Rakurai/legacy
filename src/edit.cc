/********************************************************************/
/*                                                                  */
/* edit.c -- line-oriented editor for notes and other strings       */
/*                                                                  */
/* Elrac, April 1998                                                */
/*                                                                  */
/* Original ROM extension code written specifically for Dizzy MUD   */
/* by Elrac. You may not use this code without Elrac's consent.     */
/* Contact him at: carls@ipf.de .                                   */
/*                                                                  */
/********************************************************************/

/********************************************************************/
/*                                                                  */
/* Editing functions:                                               */
/*                                                                  */
/*    EDIT                                                          */
/*       Tells the user what kind of string he is editing and gives */
/*       a brief list of EDIT options.                              */
/*                                                                  */
/*    EDIT NOTE                                                     */
/*       Starts editing the player's current NOTE.                  */
/*                                                                  */
/*    EDIT <number>                                                 */
/*       Sets current line to <number>                              */
/*                                                                  */
/*    EDIT [<number1>] LIST [<number2>]                             */
/*       Lists <number2> lines starting at <number1>                */
/*    EDIT [<number1>] [<number2>] LIST                             */
/*       Lists lines from <number1> to <number2>                    */
/*                                                                  */
/*    EDIT [<number1>] DELETE [<number2>]                           */
/*       Deletes <number2> lines starting at <number1>              */
/*    EDIT [<number1>] [<number2>] DELETE                           */
/*       Deletes lines from <number1> to <number2>                  */
/*                                                                  */
/*    EDIT [<number>] INSERT <text>                                 */
/*       Inserts <text> after line <number>                         */
/*                                                                  */
/*    EDIT [<number>] CHANGE '<text>' '<text>'                      */
/*                                                                  */
/*    EDIT [<number1>] WRAP [<number2>]                             */
/*       Wraps <number2> lines starting at <number1>                */
/*    EDIT [<number1>] [<number2>] WRAP                             */
/*       Wraps lines from <number1> to <number2>                    */
/*                                                                  */
/*    EDIT [<number>] SPLIT <word>                                  */
/*                                                                  */
/*    EDIT CANCEL                                                   */
/*       Abandon current editing session, original stays as is      */
/*                                                                  */
/*    EDIT DONE                                                     */
/*       Finish editing, save changes                               */
/*                                                                  */
/********************************************************************/

#include "argument.hh"
#include "Character.hh"
#include "declare.hh"
#include "Edit.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Logging.hh"
#include "macros.hh"
#include "memory.hh"
#include "Note.hh"
#include "Player.hh"
#include "RoomPrototype.hh"
#include "sql.hh"
#include "String.hh"

extern struct board_index_struct board_index[];

#define MAX_EDIT_LENGTH 4000
#define WRAP_WIDTH      70
#define ARG_1 Flags::A
#define ARG_2 Flags::B

/* Module global variables */

static int num1, num2;
static Flags numeric_args;
static Edit *ed;


static char *next_line(char *current_line)
{
	char *limit = &ed->edit_string[MAX_STRING_LENGTH - 1];

	for (char *pl = current_line; pl < limit && *pl != '\0'; pl++)
		if (*pl == '\n')
			return ++pl;

	return current_line;
} /* end next_line() */

int count_lines()
{
	int nlines = 0;
	char *line, *next;

	if (ed == nullptr) {
		Logging::bug("count_lines(): not editing anything", 0);
		return 0;
	}

	for (line = ed->edit_string; (next = next_line(line)) != line; line = next)
		nlines++;

	return nlines;
} /* end count_lines() */

static char *find_line(int lineno)
{
	int line = 1;
	char *pl = ed->edit_string;

	if (lineno < 1) {
		Logging::bug("find_line(0)", 0);
		return pl;
	}

	if (lineno > ed->edit_nlines + 1) {
		Logging::bug("find_line(): Line out of range", lineno);
		return pl;
	}

	while (line++ < lineno)
		pl = next_line(pl);

	return pl;
} /* end find_line() */

static void listline(String& dbuf, int lineno, char *line)
{
	char buf[MAX_STRING_LENGTH];
	char *bp;

	if (lineno == ed->edit_line)
		Format::sprintf(buf, "{W<%3d> {x", lineno);
	else if (lineno <= ed->edit_nlines)
		Format::sprintf(buf, "{x %3d: {x", lineno);
	else
		Format::sprintf(buf, "{x    : {x");

	dbuf += buf;
	bp = buf;

	if (lineno == 0)
		dbuf += "~~~TOP~~~";
	else if (lineno == ed->edit_nlines + 1)
		dbuf += "~~~END~~~";
	else {
		while (*line != '\n' && *line != '\0') {
			*bp++ = *line++;
		}
	}

	strcpy(bp, "{x\n");
	dbuf += buf;
} /* end listline() */

static void edit_list1(Character *ch, int fromline, int toline)
{
	int jline;
	char *cp;
	String dbuf;
	fromline = UMAX(fromline, 0);
	fromline = UMIN(fromline, ed->edit_nlines);
	toline   = UMAX(toline, fromline);
	toline   = UMIN(toline, ed->edit_nlines + 1);

	if (fromline == 0) {
		listline(dbuf, 0, "");
		fromline = 1;
	}

	cp = find_line(fromline);

	for (jline = fromline; jline <= toline; jline++) {
		listline(dbuf, jline, cp);
		cp = next_line(cp);
	}

	page_to_char(dbuf, ch);
} /* end edit_list1() */

static void list_window(Character *ch)
{
	int fromline, toline;

	if (ed == nullptr) {
		stc("{P~~~ You are not editing anything at the moment. ~~~{x\n", ch);
		return;
	}

	if (ed->edit_nlines <= 0) {
		stc("{P~~~ Edit buffer is empty ~~~{x\n", ch);
		return;
	}

	fromline = UMAX(ed->edit_line - 2, 0);
	toline   = UMIN(ed->edit_line + 2, ed->edit_nlines + 1);
	edit_list1(ch, fromline, toline);
} /* end list_window() */

static void edit_status(Character *ch, const String& argument)
{
	char buf[MAX_STRING_LENGTH];

	if (ch->edit == nullptr) {
		stc("{PYou aren't editing anything yet.{x\n", ch);

		if (ch->pnote == nullptr) {
			stc(
			        "You could use {RNOTE{x, {RIDEA{x or {RROLEPLAY{x to start writing a note\n"
			        "and then type {REDIT NOTE{x to edit it.\n", ch);
		}
		else {
			Format::sprintf(buf, "You have started writing a(n) {Y%s{x.\n",
			        board_index[ch->pnote->type].board_long);
			stc(buf, ch);
			stc("You could use {REDIT NOTE{x to edit it.\n", ch);
		}

		stc("Or you could use {REDIT DESC{x to edit {Yyour description{x.\n", ch);
		return;
	}

	ed = ch->edit;

	switch (ed->edit_type) {
	case EDIT_TYPE_NONE:
		stc("{PStrange, you are editing NOTHING!{x\n", ch);
		Logging::bug("editing nothing!", 0);
		return;

	case EDIT_TYPE_NOTE:
		if (ch->pnote == nullptr) {
			stc(
			        "{P You *were* editing a note of some sort.{x\n", ch);
			stc(
			        "{P Now your text is homeless. Please do {REDIT CANCEL{P.{x\n",
			        ch);
			ed->edit_type = EDIT_TYPE_NONE;
			return;
		}

		Format::sprintf(buf, "You are currently editing a(n) {Y%s{x.\n",
		        board_index[ch->pnote->type].board_long);
		stc(buf, ch);
		break;

	case EDIT_TYPE_DESC:
		stc("You are currently editing {Yyour description{x.\n", ch);
		break;

	case EDIT_TYPE_ROOM:
		stc("You are currently editing a {Yroom description{x.\n", ch);
		break;

	case EDIT_TYPE_HELP:
		ptc(ch, "You are currently editing the {Yhelp text{x for ID %d.\n", ed->edit_id);
		break;

	default:
		stc("Strange, I don't know {PWHAT{x you're editing!\n", ch);
		Logging::bug("Unknown edit type", 0);
		return;
	}

	Format::sprintf(buf, "You are on line {C%d{x of {C%d{x.\n",
	        ed->edit_line, ed->edit_nlines);
	stc(buf, ch);
	stc(
	        "{PCommands:{x {RED{xIT {RL{xIST / {RI{xNSERT / {RD{xELETE / "
	        "{RC{xHANGE / {RW{xRAP / {RS{xPLIT\n"
	        "{x         {x {RED{xIT {RDO{xNE / {RCANCEL{x / {RUNDO{x\n", ch);
	stc(
	        "See {RHELP EDIT{x for details.\n", ch);
	list_window(ch);
} /* end edit_status() */

static bool check_line(Character *ch, int line)
{
	char buf[MAX_INPUT_LENGTH];

	if (line < 0 || line > ed->edit_nlines) {
		Format::sprintf(buf, "{PThere is no line number %d{x.\n", line);
		stc(buf, ch);
		return FALSE;
	}

	return TRUE;
} /* end check_line() */

static bool check_range(Character *ch, int *fromline, int *toline)
{
	char buf[MAX_INPUT_LENGTH];

	if (numeric_args.has(ARG_2)) {
		if (!check_line(ch, num1) || !check_line(ch, num2))
			return FALSE;

		if (num2 > ed->edit_nlines) {
			num2 = ed->edit_nlines;
			Format::sprintf(buf, "{PLast line adjusted to {Y%d{x.\n", num2);
			stc(buf, ch);
		}
		else if (num2 < num1) {
			stc("{PLine numbers out of sequence.{x\n", ch);
			return FALSE;
		}

		*fromline = num1;
		*toline = num2;
	}
	else if (numeric_args.has(ARG_1)) {
		if (!check_line(ch, num1))
			return FALSE;

		*fromline = num1;
		*toline = num1;
	}
	else {
		*fromline = ed->edit_line;
		*toline = ed->edit_line;
	}

	return TRUE;
} /* end check_range() */

static void edit_goto1(Character *ch, int lineno)
{
	if (lineno < 0 || lineno > ed->edit_nlines) {
		/* Logging::bug( "edit_goto1(%d)", lineno ); */
		lineno = UMAX(lineno, 0);
		lineno = UMIN(lineno, ed->edit_nlines);
	}

	ed->edit_line = lineno;
} /* end edit_goto1() */

static bool is_blank_line(char *line)
{
	while (*line != 0 && *line != '\n') {
		if (*line > ' ')
			return FALSE;

		line++;
	}

	return TRUE;
} /* end is_blank_line() */

static void backup(void)
{
	strcpy(ed->edit_backup, ed->edit_string);
	ed->edit_undo_ok = TRUE;
} /* end backup() */

/********** Main functions **********/

static void edit_cancel(Character *ch, const String& argument)
{
	delete ed;
	ch->edit = nullptr;
	ed = nullptr;
	stc("OK, editing session aborted, {Ynothing changed{x.\n", ch);
} /* end edit_cancel() */

static void edit_change(Character *ch, String argument)
{
	int this_line = ed->edit_line;
	char *here, *next;
	char *where;
	char *end_pos;
	char end_char;
	String dbuf;

	if (numeric_args.has(ARG_1)) {
		if (!check_line(ch, num1))
			return;

		this_line = num1;
	}

	edit_goto1(ch, this_line);

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1.empty()) {
		stc("{PYou must specify a string to change.{x\n", ch);
		return;
	}

	if (strlen(arg2) > strlen(arg1)
	    && strlen(ed->edit_string) + strlen(arg2) - strlen(arg1) > MAX_EDIT_LENGTH) {
		stc("{PEdit limit exceeded - you cannot add any more text.{x\n", ch);
		return;
	}

	here = find_line(this_line);
	next = next_line(here);
	/* terminate current line so search will not run over */
	end_pos = next - 1;
	end_char = *end_pos;
	*end_pos = '\0';
	/* look for search string */
	where = strstr(here, arg1);
	*end_pos = end_char;

	if (where == nullptr) {
		Format::sprintf(arg2, "{PSearch string '%s{P' not found in current line.{x\n", arg1);
		stc(arg2, ch);
		return;
	}

	backup();
	dbuf += arg2;
	dbuf += where + strlen(arg1);
	strcpy(where, dbuf);
} /* end edit_change() */

static void edit_delete(Character *ch, const String& argument)
{
	int linefrom = ed->edit_line;
	int lineto = ed->edit_line;
	char *line1, *line9;

	if (!check_range(ch, &linefrom, &lineto))
		return;

	if (linefrom == 0) {
		stc("{PCannot delete line 0{x.\n", ch);
		return;
	}

	line1 = find_line(linefrom);
	backup();

	if (lineto >= ed->edit_nlines)
		*line1 = '\0';
	else {
		line9 = find_line(lineto + 1);
		strcpy(line1, line9);
	}

	ed->edit_nlines = count_lines();
	ed->edit_line = UMAX(0, linefrom - 1);
} /* end edit_delete() */

static void edit_desc(Character *ch, const String& argument)
{
	if (ch->edit != nullptr) {
		stc("{PBut you are already editing something!{x\n", ch);
		edit_status(ch, "");
		return;
	}

	ed = new Edit;
	ch->edit = ed;
	ed->edit_type = EDIT_TYPE_DESC;
	strcpy(ed->edit_string, ch->long_descr);
	backup();
	ed->edit_nlines = count_lines();
	edit_goto1(ch, 1);
	edit_status(ch, "");
} /* end edit_note() */

static void edit_done(Character *ch, const String& argument)
{
	char buf[MAX_INPUT_LENGTH];

	switch (ed->edit_type) {
	default:
		stc("{POops, I lost track of myself.{x\n", ch);
		Logging::bug("edit_done(): unknown edit type", 0);
		break;

	case EDIT_TYPE_NONE:
		stc("{PHmm, looks like you weren't editing anything.{x\n", ch);
		Logging::bug("edit_done(): edit type NONE", 0);
		break;

	case EDIT_TYPE_NOTE:
		if (ch->pnote == nullptr) {
			stc("{PI'm afraid your note is not there any more.{x\n"
			    "{PYour edited text is going down the drain now.{x\n", ch);
		}
		else {
			Format::sprintf(buf, "OK, I'm saving your {Y%s{x.\n",
			        board_index[ch->pnote->type].board_long);
			stc(buf, ch);
			stc("Don't forget to {RPOST{x it!\n", ch);
			ch->pnote->text = ed->edit_string;
		}

		break;

	case EDIT_TYPE_DESC:
		stc("OK, I'm saving {Yyour description{x.\n", ch);
		ch->description = ed->edit_string;
		break;

	case EDIT_TYPE_ROOM:
		if (ch->in_room == nullptr) {
			stc("{PI'm sorry, you don't seem to be in a room.{x\n", ch);
			return;
		}

		stc("OK, I'm saving your {Yroom description{x.\n", ch);
		ch->in_room->description = ed->edit_string;
		break;

	case EDIT_TYPE_HELP:
		if (db_commandf("edit_done", "update helps set text='%s' where id=%d", db_esc(ed->edit_string), ed->edit_id))
			ptc(ch, "OK, I'm saving the {Yhelp text{x for ID %d.\n", ed->edit_id);
		else
			stc("I wasn't able to save the text right now.\n", ch);

		break;
	}

	delete ed;
	ch->edit = nullptr;
	ed = nullptr;
} /* end edit_done() */

static void edit_goto(Character *ch, const String& argument)
{
	if (check_line(ch, num1))
		edit_goto1(ch, num1);
} /* end edit_goto() */

static void edit_insert(Character *ch, const String& argument)
{
	char *lp;
	String dbuf;
	int after_line = ed->edit_line + 1;

	if (numeric_args.has(ARG_1)) {
		if (!check_line(ch, num1))
			return;

		after_line = num1 + 1;
	}

	if (strlen(argument) + strlen(ed->edit_string) + 1 > MAX_EDIT_LENGTH) {
		stc("{PEdit limit exceeded - you cannot add any more text.{x\n", ch);
		return;
	}

	backup();
	dbuf += argument;
	dbuf += "\n";
	lp = find_line(after_line);
	dbuf += lp;
	strcpy(lp, dbuf);
	ed->edit_nlines = count_lines();
	edit_goto1(ch, after_line);
} /* end edit_insert() */

static void edit_list(Character *ch, const String& argument)
{
	int fromline, toline;

	/* check args */
	if (numeric_args.empty()) {
		edit_list1(ch, 0, ed->edit_nlines + 1);
		return;
	}

	if (!check_range(ch, &fromline, &toline))
		return;

	edit_list1(ch, fromline, toline);
} /* end edit_list() */

static void edit_note(Character *ch, const String& argument)
{
	if (ch->edit != nullptr) {
		stc("{PBut you are already editing something!{x\n", ch);
		edit_status(ch, "");
		return;
	}

	if (ch->pnote == nullptr) {
		stc("{PBut you haven't started writing any note yet!{x\n", ch);
		return;
	}

	ed = new Edit;
	ch->edit = ed;
	ed->edit_type = EDIT_TYPE_NOTE;
	strcpy(ed->edit_string, ch->pnote->text);
	backup();
	ed->edit_nlines = count_lines();
	edit_goto1(ch, 1);
	edit_status(ch, "");
} /* end edit_note() */

static void edit_room(Character *ch, const String& argument)
{
	if (ch->edit != nullptr) {
		stc("{PBut you are already editing something!{x\n", ch);
		edit_status(ch, "");
		return;
	}

	if (ch->in_room == nullptr) {
		stc("{PYou don't seem to be in a room at the moment.{x\n", ch);
		return;
	}

	ed = new Edit;
	ch->edit = ed;
	ed->edit_type = EDIT_TYPE_ROOM;
	strcpy(ed->edit_string, ch->in_room->description);
	backup();
	ed->edit_nlines = count_lines();
	edit_goto1(ch, 1);
	edit_status(ch, "");
} /* end edit_room() */

static void edit_help(Character *ch, const String& argument)
{
	if (ch->edit != nullptr) {
		stc("{PBut you are already editing something!{x\n", ch);
		edit_status(ch, "");
		return;
	}

	if (argument.empty() || !argument.is_number()) {
		stc("You must specify a help ID to edit it.", ch);
		return;
	}

	int id = atoi(argument);

	if (db_queryf("edit_help", "select text from helps where id=%d", id) == SQL_ERROR) {
		stc("Query error, couldn't access helps.\n", ch);
		return;
	}

	if (db_next_row() == SQL_OK) {
		ed = new Edit;
		ch->edit = ed;
		ed->edit_type = EDIT_TYPE_HELP;
		strcpy(ed->edit_string, db_get_column_str(0));
		backup();
		ed->edit_nlines = count_lines();
		ed->edit_id = id;
		edit_goto1(ch, 1);
		edit_status(ch, "");
	}
	else {
		stc("Couldn't retrieve a help with that ID.\n", ch);
	}

} /* end edit_room() */

static void edit_split(Character *ch, String argument)
{
	char buf[MAX_INPUT_LENGTH];
	int this_line = ed->edit_line;
	char *here, *next;
	char *where;
	char *end_pos;
	char end_char;
	String dbuf;

	if (numeric_args.has(ARG_1)) {
		if (!check_line(ch, num1))
			return;

		this_line = num1;
	}

	edit_goto1(ch, this_line);

	if (argument.empty()) {
		stc("{PYou must specify a string to split before.{x\n", ch);
		return;
	}

	String token;
	argument = one_argument(argument, token);

	here = find_line(ed->edit_line);
	next = next_line(here);
	/* terminate current line so search will not overrun */
	end_pos = next - 1;
	end_char = *end_pos;
	*end_pos = '\0';
	where = strstr(here, token);
	*end_pos = end_char;

	if (where == nullptr) {
		Format::sprintf(buf, "{PCharacters '%s{P' not found in current line.{x\n", token);
		stc(buf, ch);
		return;
	}

	backup();
	dbuf += where;
	strcpy(where, "\n");
	strcat(where, dbuf);
	ed->edit_nlines = count_lines();
} /* end edit_split() */

static void edit_undo(Character *ch, const String& junk)
{
	if (!ed->edit_undo_ok) {
		stc("{PSorry, you have already undone your most recent change!{x\n",
		    ch);
		return;
	}

	strcpy(ed->edit_string, ed->edit_backup);
	ed->edit_nlines = count_lines();

	if (ed->edit_line > ed->edit_nlines)
		edit_goto1(ch, ed->edit_line);

	stc("{POK{x, your most recent change (if any) has been {Pundone{x.\n",
	    ch);
	ed->edit_undo_ok = FALSE;
} /* end edit_undo() */

static void edit_wrap(Character *ch, const String& argument)
{
	int prev_blank_line = 0;
	int linefrom, lineto;
	int foll_blank_line = ed->edit_nlines + 1;
	int lineno, linelen, wordlen;
	char *start, *after;
	char *cp, *wp, *lp;
	String dbuf;
	char word[MAX_STRING_LENGTH];
	char line[MAX_INPUT_LENGTH];
	bool in_word;

	if (numeric_args.empty()) {
		/* wrap current paragraph */
		lineno = 1;
		lp = ed->edit_string;

		while (lineno <= ed->edit_nlines) {
			if (is_blank_line(lp)) {
				if (lineno < ed->edit_line)
					prev_blank_line = lineno;
				else if (lineno > ed->edit_line) {
					foll_blank_line = lineno;
					break;
				}
				else {
					/* wrapping on a empty line */
					return;
				}
			}

			lp = next_line(lp);
			lineno ++;
		}
	}
	else {
		if (!check_range(ch, &linefrom, &lineto))
			return;
		else {
			prev_blank_line = UMAX(linefrom - 1, 0);
			foll_blank_line = UMIN(lineto + 1, ed->edit_nlines + 1);
		}
	}

	/* we plan to wrap the text from after prev_blank_line to before foll_blank_line. */
	lineno = prev_blank_line + 1;
	start = find_line(lineno);

	if (foll_blank_line <= ed->edit_nlines)
		after = find_line(foll_blank_line);
	else
		after = nullptr;

	backup();
	wp = word;
	lp = line;
	linelen = 0;
	in_word = FALSE;

	for (cp = start; lineno < foll_blank_line; cp++) {
		if (*cp > ' ') {
			/* Found a printable character. Start or continue word. */
			*wp++ = *cp;
			in_word = TRUE;
		}
		else {
			if (*cp == '\n')
				lineno++;

			if (in_word) {
				/* Found a nonprintable. Terminate word. */
				*wp = '\0';
				in_word = FALSE;

				if (strlen(word) == 0)
					continue;

				wordlen = String(word).uncolor().size();

				/* If word is too long to fit in line, dump line and/or chop word. */
				while (linelen + 1 + wordlen > WRAP_WIDTH) {
					if (linelen == 0) {
						for (wp = word; String(line).uncolor().size() < WRAP_WIDTH;)
							*lp++ = *wp++;

						strcpy(word, --wp);
						wordlen = String(word).uncolor().size();
						lp--;
					}

					strcpy(lp, "\n");
					dbuf += line;
					lp = line;
					linelen = 0;
				}

				if (strlen(word) == 0)
					continue;

				if (linelen > 0) {
					*lp++ = ' ';
					linelen++;
				}

				strcpy(lp, word);
				lp += strlen(word);
				linelen += wordlen;
				wp = word;
			}
		}
	}

	/* finish current line, if started */
	if (lp > line) {
		strcpy(lp, "\n");
		dbuf += line;
	}

	if (after != nullptr)
		dbuf += after;

	strcpy(start, dbuf);
	ed->edit_nlines = count_lines();
	edit_goto1(ch, prev_blank_line + 1);
} /* end edit_wrap() */

/* Main edit function. Some pre-scanning, then branch to appropriate subfunction. */
void do_edit(Character *ch, String argument)
{
	char buf[MAX_INPUT_LENGTH];
	const char *new_arg;
	String arg;

	numeric_args.clear();

	/* scan numeric args, if any. */
	if (!argument.empty()) {
		new_arg = one_argument(argument, arg);

		if (arg.is_number()) {
			num1 = atoi(arg);
			numeric_args += ARG_1;
			argument = new_arg;
		}
	}

	if (numeric_args.has(ARG_1) && !argument.empty()) {
		new_arg = one_argument(argument, arg);

		if (arg.is_number()) {
			num2 = atoi(arg);
			numeric_args += ARG_2;
			argument = new_arg;
		}
	}

	argument = one_argument(argument, arg);

	if (arg.empty() && numeric_args.empty()) {
		edit_status(ch, argument);
		return;
	}

	if (strlen(arg) >= 3 && arg.is_prefix_of("description")) {
		edit_desc(ch, argument);
		return;
	}

	if (arg.is_prefix_of("note")) {
		edit_note(ch, argument);
		return;
	}

	if (IS_IMMORTAL(ch) && arg.is_prefix_of("room")) {
		edit_room(ch, argument);
		return;
	}

	if (IS_IMMORTAL(ch) && arg.is_prefix_of("help")) {
		edit_help(ch, argument);
		return;
	}

	if (ch->edit == nullptr) {
		ed = nullptr;
		list_window(ch);
		return;
	}

	ed = ch->edit;

	/* below this point, character is sure to be editing something */

	if (arg.empty()) {
		edit_goto(ch, argument);
		list_window(ch);
		return;
	}

	/* below this point, there is at least a keyword */

	if (arg.is_prefix_of("change")) {
		edit_change(ch, argument);
		list_window(ch);
		return;
	}

	if (arg.is_prefix_of("insert")) {
		edit_insert(ch, argument);
		list_window(ch);
		return;
	}

	if (arg.is_prefix_of("split")) {
		edit_split(ch, argument);
		list_window(ch);
		return;
	}

	/* below this point, functions want no args */

	if (arg == "cancel") {
		edit_cancel(ch, argument);
		return;
	}

	if (arg.is_prefix_of("cance")) {
		stc(
		        "{PIf you want to CANCEL your editing, write {RCANCEL{P out in full!{x\n",
		        ch);
		return;
	}

	if (arg.is_prefix_of("delete")) {
		edit_delete(ch, argument);
		list_window(ch);
		return;
	}

	if (arg.is_prefix_of("done")) {
		edit_done(ch, argument);
		return;
	}

	if (arg.is_prefix_of("list")) {
		edit_list(ch, argument);
		return;
	}

	if (arg == "undo") {
		edit_undo(ch, argument);
		list_window(ch);
		return;
	}

	if (arg.is_prefix_of("und")) {
		stc(
		        "{PIf you want to UNDO your last change, write {RUNDO{P out in full!\n",
		        ch);
		return;
	}

	if (arg.is_prefix_of("wrap")) {
		edit_wrap(ch, argument);
		list_window(ch);
		return;
	}

	Format::sprintf(buf, "{PUnknown {CEDIT{x function '{R%s{P'.{x\n", arg);
	stc(buf, ch);
	edit_status(ch, "");
} /* end do_edit() */

