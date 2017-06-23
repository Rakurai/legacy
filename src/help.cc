/*************************************************
*                                                *
*               The Age of Legacy                *
*                                                *
* Based originally on ROM 2.4, tested, enhanced, *
* and maintained by the Legacy Team.  If that    *
* doesn't mean you, and you're stealing our      *
* code, at least tell us and boost our egos ;)   *
*************************************************/

#include <vector>

#include "argument.hh"
#include "Character.hh"
#include "declare.hh"
#include "file.hh"
#include "Flags.hh"
#include "Format.hh"
#include "interp.hh"
#include "Logging.hh"
#include "macros.hh"
#include "merc.hh"
#include "Note.hh"
#include "Player.hh"
#include "sql.hh"
#include "String.hh"

#ifdef SEASON_CHRISTMAS
char stupidassline[1000] =
        "{f{G*{x{H-=-=-{f{C*{x{H-=-=-{f{B*{x{H-=-=-{f{V*{x{H-=-=-{f{P*{x{H-=-=-{f{Y*{x{H-=-=-{f{W*{x{H-=-=-{f{Y*{x{H-=-=-{f{P*{x{H-=-=-{f{V*{x{H-=-=-{f{B*{x{H-=-=-{f{C*{x{H-=-=-{f{G*{x\n\0";
#else
char stupidassline[1000] =
        "{G*{T-=-=-{C*{T-=-=-{B*{T-=-=-{V*{T-=-=-{P*{T-=-=-{Y*{T-=-=-{W*{T-=-=-{Y*{T-=-=-{P*{T-=-=-{V*{T-=-=-{B*{T-=-=-{C*{T-=-=-{G*{x\n\0";
#endif

#define HTABLE "helps"
#define HCOL_ID "id"
#define HCOL_ORDER "onum"
#define HCOL_LEVEL "level"
#define HCOL_GROUP "hgroup"
#define HCOL_KEYS "keywords"
#define HCOL_TEXT "text"

/* help file type defines */
/* here, and not in the table, is where order is important.  helps are sorted
   by precedence of their type, so imm helps will follow mortal helps, etc.
   if any changes to the order need to be made, make the change, then wipe the
   table and reload fresh, because loadhelps() will not wipe the table properly
   if numbers don't match up.  same goes for if any categories are removed */
#define	HELP_INFO		Flags::A
#define	HELP_CLAN		Flags::B
#define	HELP_SKILL		Flags::C
#define	HELP_SPELL		Flags::D
#define	HELP_RACE		Flags::E
#define	HELP_CLASS		Flags::F
#define	HELP_REMORT		Flags::G
#define	HELP_MOVE		Flags::H
#define	HELP_OBJECT		Flags::I
#define	HELP_COMM		Flags::J
#define	HELP_COMBAT		Flags::K
#define HELP_MISC		Flags::L
#define	HELP_WIZQUEST		Flags::M
#define	HELP_WIZSECURE		Flags::N
#define	HELP_WIZCODE		Flags::O
#define	HELP_WIZGEN		Flags::P
#define	HELP_WIZBUILD		Flags::Q
#define	HELP_WIZHELP		Flags::R

struct helpfile_table_type
{
	String name;
	int	group;
};
//extern	const	std::vector<helpfile_table_type> helpfile_table;	/* help name group table */

const std::vector<helpfile_table_type> helpfile_table = {
	{       "wizhelp",      HELP_WIZHELP    },
	{       "wizquest",     HELP_WIZQUEST   },
	{       "wizsecure",    HELP_WIZSECURE  },
	{       "wizcode",      HELP_WIZCODE    },
	{       "wizgen",       HELP_WIZGEN     },
	{       "wizbuild",     HELP_WIZBUILD   },
	{       "skill",        HELP_SKILL      },
	{       "spell",        HELP_SPELL      },
	{       "race",         HELP_RACE       },
	{       "class",        HELP_CLASS      },
	{       "remort",       HELP_REMORT     },
	{       "clan",         HELP_CLAN       },
	{       "move",         HELP_MOVE       },
	{       "object",       HELP_OBJECT     },
	{       "comm",         HELP_COMM       },
	{       "info",         HELP_INFO       },
	{       "combat",       HELP_COMBAT     },
	{       "misc",         HELP_MISC       },
};

extern String help_greeting;

/*** UTILITY FUNCTIONS ***/

const char *one_keyword(const char *keywords, char *word)
{
	while (isspace(*keywords))
		keywords++;

	while (*keywords != '\0') {
		if (*keywords == '\'' || *keywords == '"') {
			keywords++;
			continue;
		}

		if (*keywords == ' ')
			break;

		*word = *keywords;
		word++;
		keywords++;
	}

	*word = '\0';

	while (isspace(*keywords))
		keywords++;

	return keywords;
}

void help_char_search(Character *ch, const String& arg)
{
	String buf, text;
	char query[MSL];
	String output;
	int i = 0, count = 0;
	Format::sprintf(query, "SELECT " HCOL_KEYS " FROM " HTABLE " WHERE " HCOL_LEVEL " <= %d "
	        "AND " HCOL_KEYS " LIKE '%% %s%%' "
	        "OR " HCOL_KEYS " LIKE '%s%%' "
	        "OR " HCOL_KEYS " LIKE '%%\\'%s%%' "
	        "ORDER BY " HCOL_KEYS,
	        ch->level, db_esc(arg), db_esc(arg), db_esc(arg)
	       );

	if (db_query("help_char_search", query) != SQL_OK) {
		stc("There was a problem with your help query, please notify the imms\n"
		    "using the 'bug' command.  Make sure to say what you typed.\n", ch);
		return;
	}

	while (db_next_row() == SQL_OK) {
		count++;
		const char *keywords = db_get_column_str(0);

		if (!String(keywords).has_words(arg))
			continue;

		buf += ++i % 2 ? " {W" : " {c";
		buf += keywords;
	}

	if (count == 0) {
		stc("No helps were found with keywords beginning with that character.\n", ch);
		return;
	}

	output += stupidassline;
	output += Format::format("\n{WHelps beginning with the letter '{c%s{W':{x\n\n", arg);
	text = format_string(buf);
	output += text;
	output += Format::format("\n{W[%d] total help entries.{x\n\n", i);
	output += stupidassline;
	page_to_char(output, ch);
}

/* the mud's internal help command, no multiple results, no suggestions.  command groups are not checked */
void help(Character *ch, const String& argument)
{
	String query;
	const char *p;
	String output;
	Format::sprintf(query, "SELECT " HCOL_TEXT " FROM " HTABLE " WHERE ");
	p = argument.c_str();

	while (*p != '\0') {
		char word[MIL];
		p = one_keyword(p, word);
		query += HCOL_KEYS " LIKE '%";
		query += db_esc(word);
		query += "%'";

		if (*p != '\0')
			query += " AND ";
	}

	query += " LIMIT 1";

	if (db_query("help", query) != SQL_OK)
		return;

	if (db_next_row() != SQL_OK) {
		Logging::bugf("help():  no helps with keywords '%s'", argument);
		return;
	}

	const char *text = db_get_column_str(0);

	if (text == nullptr) {
		Logging::bugf("help():  help with keywords '%s' has null text", argument);
		return;
	}

	output += text + (text[0] == '.' ? 1 : 0);
	page_to_char(output, ch);
}

void add_help(int group, int order, int level, const String& keywords, const String& text)
{
	String query;

	if (keywords == "GREETING") {
		help_greeting = text;
	}

	Format::sprintf(query, "INSERT INTO " HTABLE " (" HCOL_GROUP "," HCOL_ORDER "," HCOL_LEVEL "," HCOL_KEYS "," HCOL_TEXT ") "
	        "VALUES(%d,%d,%d,'", group, order, level
	       );
	query += db_esc(keywords);
	query += "','";
	query += db_esc(text);
	query += "')";
	db_command("add_help", query);
}

/*** USER COMMANDS ***/

/* load the specified help file into the database */
void do_loadhelps(Character *ch, String argument)
{
	FILE *fp;
	int tablenum, count = 0;
	struct help_struct {
		int level;
		String keywords;
		String text;
	};
	struct help_struct temp_help[500];

	if (argument.empty()) {
		stc("Syntax:\n"
		    "  loadhelps <filename>\n"
		    "  loadhelps all\n\n"
		    "File names are:\n", ch);

		for (tablenum = 0; tablenum < helpfile_table.size(); tablenum++)
			ptc(ch, "%s\n", helpfile_table[tablenum].name);

		return;
	}

	if (port != DIZZYPORT) {
		ptc(ch, "Please perform loading and printing commands on the port %d copy.\n", DIZZYPORT);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg == "all") {
		for (tablenum = 0; tablenum < helpfile_table.size(); tablenum++)
			do_loadhelps(ch, helpfile_table[tablenum].name);

		stc("All help files in the " HELP_DIR " directory loaded.\n", ch);
		stc("Please remember to clean up, remove the help files with rm *.help!\n", ch);
		return;
	}

	for (tablenum = 0; tablenum < helpfile_table.size(); tablenum++)
		if (helpfile_table[tablenum].name.is_prefix_of(arg))
			break;

	if (tablenum >= helpfile_table.size()) {
		stc("That is not a valid help file name.  Help file names are:\n\n", ch);

		for (tablenum = 0; tablenum < helpfile_table.size(); tablenum++)
			ptc(ch, "%s\n", helpfile_table[tablenum].name);

		return;
	}

	String buf = Format::format(HELP_DIR "%s.help", helpfile_table[tablenum].name);

	if ((fp = fopen(buf.c_str(), "r")) == nullptr) {
		stc("File not found - make sure it is uploaded into the /area/help/ directory.\n", ch);
		return;
	}

	while (1) {
		if ((temp_help[count].level = fread_number(fp)) < -1)
			break;

		temp_help[count].keywords = fread_string(fp);
		temp_help[count].text = fread_string(fp);

		if (temp_help[count].keywords.empty()
		    || temp_help[count].text.empty())
			stc("Error:  missing text.\n", ch);
		else if (strlen(temp_help[count].keywords) > 100)
			stc("Error:  keywords longer than 100 characters.\n", ch);
		else if (strlen(temp_help[count].text) > 8000)
			stc("Error:  text longer than 8000 characters.\n", ch);
		else {
			count++;
			continue;
		}

		ptc(ch, "There was an error loading the help file '%s.help',",
		    helpfile_table[tablenum].name);

		if (count > 0)
			ptc(ch, " after\nthe help '%s'.\n", temp_help[count - 1].keywords);
		else
			stc(" on the first help.\n", ch);

		fclose(fp);
		return;
	}

	db_commandf("do_loadhelps", "DELETE FROM " HTABLE " WHERE " HCOL_GROUP "=%d", helpfile_table[tablenum].group);

	for (count = 0; temp_help[count].level >= -1; count++) {
		bool foundspace = FALSE;
		/* unfuck any weird spacing */
		String buf;
		auto p = temp_help[count].keywords.cbegin();

		while (p != temp_help[count].keywords.cend()) {
			if (*p == ' ') {
				if (foundspace) {
					p++;
					continue;
				}

				foundspace = TRUE;
			}
			else
				foundspace = FALSE;

			buf += *p++;
		}

		add_help(
		        helpfile_table[tablenum].group,
		        count + 1,
		        temp_help[count].level,
		        buf,
		        temp_help[count].text
		);
	}

	ptc(ch, "File " HELP_DIR "%s.help: %d helps loaded.\n", helpfile_table[tablenum].name, count);
}

/* print all helps matching a group to file */
void do_printhelps(Character *ch, String argument)
{
	String buf;
	FILE *fp;
	int tablenum, count = 0;

	if (argument.empty()) {
		stc("Syntax:\n"
		    "  printhelps <filename>\n"
		    "  printhelps all\n\n"
		    "Filenames are:\n", ch);

		for (tablenum = 0; tablenum < helpfile_table.size(); tablenum++)
			ptc(ch, "%s\n", helpfile_table[tablenum].name);

		return;
	}

	if (port != DIZZYPORT) {
		stc("Please perform loading and printing commands on the port 3000 copy.\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg == "all") {
		for (tablenum = 0; tablenum < helpfile_table.size(); tablenum++)
			do_printhelps(ch, helpfile_table[tablenum].name);

		stc("All helps have been printed to file in the " HELP_DIR " directory.\n", ch);
		return;
	}

	for (tablenum = 0; tablenum < helpfile_table.size(); tablenum++)
		if (helpfile_table[tablenum].name.is_prefix_of(arg))
			break;

	if (tablenum >= helpfile_table.size()) {
		stc("That is not a valid help file name.  Help file names are:\n\n", ch);

		for (tablenum = 0; tablenum < helpfile_table.size(); tablenum++)
			ptc(ch, "%s\n", helpfile_table[tablenum].name);

		return;
	}

	if (db_queryf("do_printhelps",
	                        "SELECT " HCOL_LEVEL "," HCOL_KEYS "," HCOL_TEXT " FROM " HTABLE " WHERE " HCOL_GROUP "=%d ORDER BY " HCOL_ORDER,
	                        helpfile_table[tablenum].group) != SQL_OK)
		return;

	if ((fp = fopen(TEMP_FILE, "w")) == nullptr) {
		Logging::bug("do_printhelps: unable to open temp file", 0);
		return;
	}

	while (db_next_row() == SQL_OK) {
		buf = db_get_column_str(0);
		buf += " ";
		buf += String(db_get_column_str(1)).replace("~", "-");
		buf += "~\n";
		buf += String(db_get_column_str(2)).replace("~", "-");
		buf += "~\n\n";
		fputs(buf.c_str(), fp);
		count++;
	}

	if (count == 0) {
		ptc(ch, "No help files fall into the '%s' group.\n", helpfile_table[tablenum].name);
		fclose(fp);
		return;
	}

	Format::fprintf(fp, "-2\n");
	fclose(fp);
	Format::sprintf(buf, HELP_DIR "%s.help", helpfile_table[tablenum].name);
	rename(TEMP_FILE, buf.c_str());
	ptc(ch, "File " HELP_DIR "%s.help: %d helps printed.\n", helpfile_table[tablenum].name, count);
}

void do_help(Character *ch, String argument)
{
	String query;
	const char *p;
	String output;
	int result_count = 0, partial_count = 0, result_num = 0, i;
	struct help_struct {
		int type;
		int hgroup;
		char keywords[256];
		char text[32767];
		int id;
	};
	struct help_struct temp_help[100];

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		do_help(ch, "SUMMARY");
		return;
	}

	if (arg == "departed") {
		do_departed(ch, "");
		return;
	}

	/* on a one char search, we print a list of all helps starting with that char */
	if ((argument[1] == '\0' || argument[1] == ' ')
	    && ((argument[0] >= 'a' && argument[0] <= 'z')
	        || (argument[0] >= 'A' && argument[0] <= 'Z'))) {
		one_argument(argument, arg);
		help_char_search(ch, arg);
		return;
	}

	/* poll the database for all helps containing the arguments */
	Format::sprintf(query, "SELECT " HCOL_GROUP "," HCOL_KEYS "," HCOL_TEXT "," HCOL_ID
	        " FROM " HTABLE " WHERE " HCOL_LEVEL " <= %d AND ",
	        ch->level
	       );
	p = argument.c_str();

	while (*p != '\0') {
		char word[MIL];
		p = one_keyword(p, word);
		query += HCOL_KEYS " LIKE '%";
		query += db_esc(word);
		query += "%'";

		if (*p != '\0')
			query += " AND ";
	}

	/* display the normal helps, followed by immortal helps */
	query += " ORDER BY " HCOL_ORDER;

	if (db_query("do_help", query) != SQL_OK) {
		stc("There was a problem with your help query, please notify the imms\n"
		    "using the 'bug' command.  Make sure to say what you typed.\n", ch);
		return;
	}

	while (db_next_row() == SQL_OK) {
		int group = db_get_column_int(0);
		const char *keywords = db_get_column_str(1);
		const char *text = db_get_column_str(2);
		int id = db_get_column_int(3);

		if (keywords == nullptr || text == nullptr)
			continue;

		/* 0 if not keyword, 1 if exact match, 2 if semi-match */
		if (String(keywords).has_exact_words(argument))
			temp_help[result_count].type = 1;
		else if (String(keywords).has_words(argument)) {
			temp_help[result_count].type = 2;
			partial_count++;
		}
		else
			continue;

		for (i = 0; i < helpfile_table.size(); i++)
			if (helpfile_table[i].group == group) {
				temp_help[result_count].hgroup = i;
				break;
			}

		strcpy(temp_help[result_count].keywords, keywords);

		/* don't bother if we won't display it */
		if (temp_help[result_count].type != 2 || partial_count <= 1)
			strcpy(temp_help[result_count].text, text);

		temp_help[result_count].id = id;
		result_count++;
	}

	if (result_count == 0) {
		stc("No helps were found with those keywords.\n", ch);
		return;
	}


	/* if we have an exact result, display them, then a list of partial match keywords */
	if (result_count > partial_count) {     /* exact results? */
		for (result_num = 0; result_num < result_count; result_num++) {
			if (temp_help[result_num].type != 1)
				continue;

			char immbuf[MSL] = "";

			if (IS_IMMORTAL(ch))
				Format::sprintf(immbuf, "(id %d, file %s) ", temp_help[result_num].id, helpfile_table[temp_help[result_num].hgroup].name);

			output += Format::format("%s\n{W%s%s{x\n\n",
			    stupidassline,
			    immbuf,
			    temp_help[result_num].keywords);
			/* Strip leading '.' to allow initial blanks. */
			output += temp_help[result_num].text + (temp_help[result_num].text[0] == '.' ? 1 : 0);
			output += Format::format("\n%s", stupidassline);
		}

		if (partial_count)
			output += "\nYou could also try the following partial matches:\n\n";
	}
	else if (partial_count == 1) {
		/* no exact matches, if there's only one partial match, let's show it and be done */
		char immbuf[MSL] = "";

		if (IS_IMMORTAL(ch))
			Format::sprintf(immbuf, "(id %d, file %s) ", temp_help[result_num].id, helpfile_table[temp_help[result_num].hgroup].name);

		output += Format::format("%s\n{W%s%s{x\n\n",
		    stupidassline,
		    immbuf,
		    temp_help[0].keywords);
		output += temp_help[0].text + (temp_help[0].text[0] == '.' ? 1 : 0);
		output += Format::format("\n%s", stupidassline);
		/* done, we'll drop through the next if statement to the printing */
	}
	else {
		output += "No helps were found matching all of your keywords, but the\n";
		output += "following partial matches may direct you to the proper help:\n\n";
	}

	if (partial_count && (result_count > partial_count || partial_count > 1)) {
		int newres;

		for (result_num = 0; result_num < result_count; result_num++) {
			if (temp_help[result_num].type != 2)
				continue;

			/* no duplicates! */
			for (newres = 0; newres < result_num; newres++) {
				if (temp_help[newres].type != 2)
					continue;

				if (temp_help[result_num].keywords == temp_help[newres].keywords)
					break;
			}

			if (newres != result_num)
				continue;

			output += Format::format("%s\n", temp_help[result_num].keywords);
		}
	}

	page_to_char(output, ch);
}

void do_hedit(Character *ch, String argument)
{
	if (argument.empty()) {
		ptc(ch, "Syntax:  hedit new <keywords>\n"
		    "               delete <id>\n"
		    "               show <id>\n"
		    "               %s|%s|%s|%s|%s <id> <value>\n",
		    HCOL_GROUP, HCOL_ORDER, HCOL_LEVEL, HCOL_KEYS, HCOL_TEXT
		   );
		return;
	}

	String cmd;
	argument = one_argument(argument, cmd);

	if (cmd == "new") {
		if (argument.empty()) {
			stc("You need to specify some keywords.\n", ch);
			return;
		}

		if (!db_commandf("do_hedit", "insert into " HTABLE " (" HCOL_KEYS "," HCOL_TEXT ") values('%s','')",
		                 db_esc(argument))) {
			stc("Could not create a help with those keywords.\n", ch);
			return;
		}

		if (db_query("do_help", "select last_insert_rowid()") != SQL_OK
		 || db_next_row() != SQL_OK) {
			stc("Couldn't retrieve the ID of the new help.\n", ch);
			return;
		}

		ptc(ch, "Success, the new help has an ID of %s.\n", db_get_column_str(0));
		return;
	}

	String arg;
	argument = one_argument(argument, arg);

	if (arg.empty()) {
		stc("What help do you want to operate on?\n", ch);
		return;
	}

	if (db_countf("do_hedit", "select count(*) from " HTABLE " where " HCOL_ID "=%s", arg) < 1) {
		stc("No help found with that ID.\n", ch);
		return;
	}

	if (cmd == "delete") {
		db_commandf("do_hedit", "delete from " HTABLE " where " HCOL_ID "=%s", arg);
		stc("That help is history now.\n", ch);
		return;
	}

	if (cmd == "show") {
		if (db_queryf("do_help",
		                        "select " HCOL_GROUP "," HCOL_ORDER "," HCOL_LEVEL "," HCOL_KEYS "," HCOL_TEXT
		                        " from " HTABLE " where " HCOL_ID "=%s", arg) != SQL_OK
		 || db_next_row() != SQL_OK) {
			stc("Couldn't retrieve a help with that ID.\n", ch);
			return;
		}

		ptc(ch, "ID: %4s  File: %s  Order: %s  Level: %s\nKeywords: %s\n%s\n",
		    arg,
		    db_get_column_str(0),
		    db_get_column_str(1),
		    db_get_column_str(2),
		    db_get_column_str(3),
		    db_get_column_str(4)
		);
		return;
	}

	if (argument.empty()) {
		stc("What value do you want to set it to?\n", ch);
		return;
	}

	if (cmd == HCOL_GROUP || cmd == HCOL_ORDER || cmd == HCOL_LEVEL) {
		if (!argument.is_number()) {
			stc("New value has to be a number.\n", ch);
			return;
		}

		db_commandf("do_hedit", "update " HTABLE " set %s=%d where " HCOL_ID "=%s", cmd, atoi(argument), arg);
		stc("Done.\n", ch);
		return;
	}

	if (cmd == HCOL_KEYS || cmd == HCOL_TEXT) {
		db_commandf("do_hedit", "update " HTABLE " set %s='%s' where " HCOL_ID "=%s", cmd, db_esc(argument), arg);
		stc("Done.\n", ch);
		return;
	}

	ptc(ch, "Unknown command '%s'.\n", cmd);
}
