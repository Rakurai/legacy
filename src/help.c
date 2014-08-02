/*************************************************
*                                                *
*               The Age of Legacy                *
*                                                *
* Based originally on ROM 2.4, tested, enhanced, *
* and maintained by the Legacy Team.  If that    *
* doesn't mean you, and you're stealing our      *
* code, at least tell us and boost our egos ;)   *
*************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <mysql.h>
#include "merc.h"
#include "mysql.h"
#include "tables.h"
#include "recycle.h"
#include "ports.h"

DECLARE_DO_FUN( do_departed	);

#ifdef SEASON_CHRISTMAS
char stupidassline[1000] = "{f{G*{x{H-=-=-{f{C*{x{H-=-=-{f{B*{x{H-=-=-{f{V*{x{H-=-=-{f{P*{x{H-=-=-{f{Y*{x{H-=-=-{f{W*{x{H-=-=-{f{Y*{x{H-=-=-{f{P*{x{H-=-=-{f{V*{x{H-=-=-{f{B*{x{H-=-=-{f{C*{x{H-=-=-{f{G*{x\n\r\0";
#else
char stupidassline[1000] = "{G*{T-=-=-{C*{T-=-=-{B*{T-=-=-{V*{T-=-=-{P*{T-=-=-{Y*{T-=-=-{W*{T-=-=-{Y*{T-=-=-{P*{T-=-=-{V*{T-=-=-{B*{T-=-=-{C*{T-=-=-{G*{x\n\r\0";
#endif

#define HTABLE "helps"
#define HCOL_ID "id"
#define HCOL_ORDER "onum"
#define HCOL_LEVEL "level"
#define HCOL_GROUP "hgroup"
#define HCOL_KEYS "keywords"
#define HCOL_TEXT "text"

extern char *help_greeting;


/*** UTILITY FUNCTIONS ***/

char *one_keyword(char *keywords, char *word)
{
	while (isspace(*keywords))
		keywords++;

	while (*keywords != '\0')
	{
		if (*keywords == '\'' || *keywords == '"')
		{
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


void help_char_search(CHAR_DATA *ch, char *arg)
{
	char buf[MSL] = "\0", query[MSL], *text;
	MYSQL_RES *result;
	MYSQL_ROW row;
	BUFFER *output;
	int i = 0;

	sprintf(query, "SELECT " HCOL_KEYS " FROM " HTABLE " WHERE " HCOL_LEVEL " <= %d "
		"AND " HCOL_KEYS " LIKE '%% %s%%' "
		"OR " HCOL_KEYS " LIKE '%s%%' "
		"OR " HCOL_KEYS " LIKE '%%\\'%s%%' "
		"ORDER BY " HCOL_KEYS,
		ch->level, arg, arg, arg
	);

	if ((result = db_query("help_char_search", query)) == NULL)
	{
		stc("There was a problem with your help query, please notify the imms\n"
		    "using the 'bug' command.  Make sure to say what you typed.\n", ch);
		return;
	}

	if (!mysql_num_rows(result))
	{
		stc("No helps were found with keywords beginning with that character.\n", ch);
		mysql_free_result(result);
		return;
	}

	while ((row = mysql_fetch_row(result)))
	{
		if (!is_name(arg, row[0]))
			continue;

		strcat(buf, ++i % 2 ? " {W" : " {c");
		strcat(buf, row[0]);
	}

	mysql_free_result(result);
	output = new_buf();

	add_buf(output, stupidassline);
	ptb(output, "\n{WHelps beginning with the letter '{c%s{W':{x\n\n", arg);
	text = str_dup(buf);
	format_string(text);
	add_buf(output, text);
	free_string(text);
	ptb(output, "\n{W[%d] total help entries.{x\n\n", i);
	add_buf(output, stupidassline);

	page_to_char(buf_string(output), ch);
	free_buf(output);
}

/* the mud's internal help command, no multiple results, no suggestions.  command groups are not checked */
void help(CHAR_DATA *ch, char *argument)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	char query[MSL], *p;
	BUFFER *output;

	sprintf(query, "SELECT " HCOL_TEXT " FROM " HTABLE " WHERE ");
	p = argument;

	while (*p != '\0')
	{
		char word[MIL];

		p = one_keyword(p, word);

		strcat(query, HCOL_KEYS " LIKE '%");
		strcat(query, db_esc(word));
		strcat(query, "%'");

		if (*p != '\0')
			strcat(query, " AND ");
	}

	strcat(query, " LIMIT 1");

	if ((result = db_query("help", query)) == NULL)
		return;

	if ((row = mysql_fetch_row(result)) == NULL)
	{
		bugf("help():  no helps with keywords '%s'", argument);
		return;
	}

	output = new_buf();
	add_buf(output, row[0] + (row[0][0] == '.' ? 1 : 0));
	mysql_free_result(result);
	page_to_char(buf_string(output), ch);
	free_buf(output);
}

void add_help(int group, int order, int level, char *keywords, char *text) {
	char query[MSL];

	if (!str_cmp(keywords, "GREETING")) {
		free_string(help_greeting);
		help_greeting = str_dup(text);
	}

	sprintf(query, "INSERT INTO " HTABLE " ("HCOL_GROUP "," HCOL_ORDER "," HCOL_LEVEL "," HCOL_KEYS "," HCOL_TEXT ") "
		"VALUES(%d,%d,%d,'", group, order, level
	);

	strcat(query, db_esc(keywords));
	strcat(query, "','");

	strcat(query, db_esc(text));
	strcat(query, "')");

	db_command("add_help", query);
}

/*** USER COMMANDS ***/

/* load the specified help file into the database */
void do_loadhelps(CHAR_DATA *ch, char *argument)
{
	char arg[MIL], buf[MSL], *q, *p;
	FILE *fp;
	int tablenum, count = 0;

	struct help_struct
	{
		int level;
		char *keywords;
		char *text;
	};

	struct help_struct temp_help[500];

	if (argument[0] == '\0')
	{
		stc("Syntax:\n\r"
		    "  loadhelps <filename>\n\r"
		    "  loadhelps all\n\r\n\r"
		    "File names are:\n\r", ch);

		for (tablenum = 0; helpfile_table[tablenum].name != NULL; tablenum++)
			ptc(ch, "%s\n\r", helpfile_table[tablenum].name);

		return;
	}

	if (port != DIZZYPORT)
	{
		ptc(ch, "Please perform loading and printing commands on the port %d copy.\n\r", DIZZYPORT);
		return;
	}

	one_argument(argument, arg);

	if (!str_cmp(arg, "all"))
	{
		for (tablenum = 0; helpfile_table[tablenum].name != NULL; tablenum++)
			do_loadhelps(ch, helpfile_table[tablenum].name);

		stc("All help files in the /area/" HELP_DIR "/ directory loaded.\n\r", ch);
		stc("Please remember to clean up, remove the help files with rm *.help!\n\r", ch);
		return;
	}

	for (tablenum = 0; helpfile_table[tablenum].name != NULL; tablenum++)
		if (!str_prefix1(helpfile_table[tablenum].name, arg))
			break;

	if (helpfile_table[tablenum].name == NULL)
	{
		stc("That is not a valid help file name.  Help file names are:\n\r\n\r", ch);

		for (tablenum = 0; helpfile_table[tablenum].name != NULL; tablenum++)
			ptc(ch, "%s\n\r", helpfile_table[tablenum].name);

		return;
	}

	sprintf(buf, HELP_DIR "/%s.help", helpfile_table[tablenum].name);

	if ((fp = fopen(buf, "r")) == NULL)
	{
		stc("File not found - make sure it is uploaded into the /area/help/ directory.\n\r", ch);
		return;
	}

	while (1)
	{
		if ((temp_help[count].level = fread_number(fp)) < -1)
			break;

		temp_help[count].keywords = fread_string(fp);
		temp_help[count].text = fread_string(fp);

		if (temp_help[count].keywords	== NULL
		 || temp_help[count].text	== NULL
		 || temp_help[count].keywords[0]== '\0'
		 || temp_help[count].text[0]	== '\0')
			stc("Error:  missing text.\n\r", ch);
		else if (strlen(temp_help[count].keywords) > 100)
			stc("Error:  keywords longer than 100 characters.\n\r", ch);
		else if (strlen(temp_help[count].text) > 8000)
			stc("Error:  text longer than 8000 characters.\n\r", ch);
		else
		{
			count++;
			continue;
		}

		ptc(ch, "There was an error loading the help file '%s.help',",
			helpfile_table[tablenum].name);

		if (count > 0)
			ptc(ch, " after\n\rthe help '%s'.\n\r", temp_help[count-1].keywords);
		else
			stc(" on the first help.\n\r", ch);

		fclose(fp);
		return;
	}

	db_commandf("do_loadhelps", "DELETE FROM " HTABLE " WHERE " HCOL_GROUP "=%d", helpfile_table[tablenum].group);

	for (count = 0; temp_help[count].level >= -1; count++)
	{
		bool foundspace = FALSE;

		/* unfuck any weird spacing */
		buf[0] = '\0';
		q = buf;
		p = temp_help[count].keywords;

		while (*p != '\0')
		{
			if (*p == ' ')
			{
				if (foundspace)
				{
					p++;
					continue;
				}

				foundspace = TRUE;
			}
			else
				foundspace = FALSE;

			*q = *p;
			q++;
			p++;
		}

		*q = '\0';
		q = buf;

		add_help(
			helpfile_table[tablenum].group,
			count+1,
			temp_help[count].level,
			q,
			temp_help[count].text
		);
	}

	ptc(ch, "File /area/" HELP_DIR "/%s.help: %d helps loaded.\n\r", helpfile_table[tablenum].name, count);
}

/* print all helps matching a group to file */
void do_printhelps(CHAR_DATA *ch, char *argument)
{
	char arg[MIL], buf[MSL*3];

	MYSQL_RES *result;
	MYSQL_ROW row;
	FILE *fp;
	int tablenum, count = 0;

	if (argument[0] == '\0')
	{
		stc("Syntax:\n\r"
		    "  printhelps <filename>\n\r"
		    "  printhelps all\n\r\n\r"
		    "Filenames are:\n\r", ch);

		for (tablenum = 0; helpfile_table[tablenum].name != NULL; tablenum++)
			ptc(ch, "%s\n\r", helpfile_table[tablenum].name);

		return;
	}

	if (port != DIZZYPORT)
	{
		stc("Please perform loading and printing commands on the port 3000 copy.\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if (!str_cmp(arg, "all"))
	{
		for (tablenum = 0; helpfile_table[tablenum].name != NULL; tablenum++)
			do_printhelps(ch, helpfile_table[tablenum].name);

		stc("All helps have been printed to file in the /area/" HELP_DIR "/ directory.\n\r", ch);
		return;
	}

	for (tablenum = 0; helpfile_table[tablenum].name != NULL; tablenum++)
		if (!str_prefix1(helpfile_table[tablenum].name, arg))
			break;

	if (helpfile_table[tablenum].name == NULL)
	{
		stc("That is not a valid help file name.  Help file names are:\n\r\n\r", ch);

		for (tablenum = 0; helpfile_table[tablenum].name != NULL; tablenum++)
			ptc(ch, "%s\n\r", helpfile_table[tablenum].name);

		return;
	}

	if ((result = db_queryf("do_printhelps",
			"SELECT " HCOL_LEVEL "," HCOL_KEYS "," HCOL_TEXT " FROM " HTABLE " WHERE " HCOL_GROUP "=%d ORDER BY " HCOL_ORDER,
			helpfile_table[tablenum].group)) == NULL)
		return;

	if (!mysql_num_rows(result))
	{
		ptc(ch, "No help files fall into the '%s' group.\n\r", helpfile_table[tablenum].name);
		mysql_free_result(result);
		return;
	}

	if ((fp = fopen(TEMP_FILE, "w")) == NULL)
	{
		bug("do_printhelps: unable to open temp file", 0);
		mysql_free_result(result);
		return;
	}

	while ((row = mysql_fetch_row(result)))
	{
		strcpy(buf, row[0]);
		strcat(buf, " ");
		strcat(buf, row[1]);
		strcat(buf, "~\n");
		strcat(buf, row[2]);
		strcat(buf, "~\n\n");
		fputs(buf, fp);
		count++;
	}

	fprintf(fp, "-2\n");
	fclose(fp);
	sprintf(buf, HELP_DIR "/%s.help", helpfile_table[tablenum].name);
	rename(TEMP_FILE, buf);
	ptc(ch, "File /area/" HELP_DIR "/%s.help: %d helps printed.\n\r", helpfile_table[tablenum].name, count);
	mysql_free_result(result);
}

void do_help(CHAR_DATA *ch, char *argument)
{
	char arg[MIL], query[MSL], *p;
	MYSQL_RES *result;
	MYSQL_ROW row;
	BUFFER *output;
	int result_count = 0, partial_count = 0, result_num = 0, i;

	struct help_struct
	{
		int type;
		int hgroup;
		char keywords[256];
		char text[32767];
		int id;
	};

	struct help_struct temp_help[100];

	one_argument(argument, arg);

	if (arg[0] == '\0')
		strcpy(argument, "SUMMARY");
	else if (!str_cmp(arg, "departed"))
	{
		do_departed(ch, "");
		return;
	}

	/* on a one char search, we print a list of all helps starting with that char */
	if ((argument[1] == '\0' || argument[1] == ' ')
	 && ((argument[0] >= 'a' && argument[0] <= 'z')
	  || (argument[0] >= 'A' && argument[0] <= 'Z')))
	{
		one_argument(argument, arg);
		help_char_search(ch, arg);
		return;
	}

	/* poll the database for all helps containing the arguments */
	sprintf(query, "SELECT " HCOL_GROUP "," HCOL_KEYS "," HCOL_TEXT "," HCOL_ID
		" FROM " HTABLE " WHERE " HCOL_LEVEL " <= %d AND ",
		ch->level
	);
	p = argument;

	while (*p != '\0')
	{
		char word[MIL];

		p = one_keyword(p, word);

		strcat(query, HCOL_KEYS " LIKE '%");
		strcat(query, db_esc(word));
		strcat(query, "%'");

		if (*p != '\0')
			strcat(query, " AND ");
	}

	/* display the normal helps, followed by immortal helps */
	strcat(query, " ORDER BY " HCOL_ORDER);

	if ((result = db_query("do_help", query)) == NULL)
	{
		stc("There was a problem with your help query, please notify the imms\n"
		    "using the 'bug' command.  Make sure to say what you typed.\n", ch);
		return;
	}

	while ((row = mysql_fetch_row(result)))
	{
		/* 0 if not keyword, 1 if exact match, 2 if semi-match */
		if (is_exact_name(argument, row[1]))
			temp_help[result_count].type = 1;
		else if (is_name(argument, row[1]))
		{
			temp_help[result_count].type = 2;
			partial_count++;
		}
		else
			continue;

		for (i = 0; helpfile_table[i].name != NULL; i++)
			if (helpfile_table[i].group == atoi(row[0]))
			{
				temp_help[result_count].hgroup = i;
				break;
			}

		strcpy(temp_help[result_count].keywords, row[1]);

		/* don't bother if we won't display it */
		if (temp_help[result_count].type != 2 || partial_count <= 1)
			strcpy(temp_help[result_count].text, row[2]);

		temp_help[result_count].id = atoi(row[3]);
		result_count++;
	}

	mysql_free_result(result);

	if (result_count == 0)
	{
		stc("No helps were found with those keywords.\n", ch);
		return;
	}

	output = new_buf();

	/* if we have an exact result, display them, then a list of partial match keywords */
	if (result_count > partial_count)	/* exact results? */
	{
		for (result_num = 0; result_num < result_count; result_num++)
		{
			if (temp_help[result_num].type != 1)
				continue;

			char immbuf[MSL] = "";

			if IS_IMMORTAL(ch)
				sprintf(immbuf, "(id %d, file %s) ", temp_help[result_num].id, helpfile_table[temp_help[result_num].hgroup].name);

			ptb(output, "%s\n{W%s%s{x\n\n",
				stupidassline,
				immbuf,
				temp_help[result_num].keywords);

			/* Strip leading '.' to allow initial blanks. */
			add_buf(output, temp_help[result_num].text + (temp_help[result_num].text[0] == '.' ? 1 : 0));

			ptb(output, "\n%s", stupidassline);
		}

		if (partial_count)
			add_buf(output, "\nYou could also try the following partial matches:\n\n");
	}
	else if (partial_count == 1)
	{
		/* no exact matches, if there's only one partial match, let's show it and be done */
			char immbuf[MSL] = "";

			if IS_IMMORTAL(ch)
				sprintf(immbuf, "(id %d, file %s) ", temp_help[result_num].id, helpfile_table[temp_help[result_num].hgroup].name);

			ptb(output, "%s\n{W%s%s{x\n\n",
				stupidassline,
				immbuf,
				temp_help[0].keywords);
		add_buf(output, temp_help[0].text + (temp_help[0].text[0] == '.' ? 1 : 0));
		ptb(output, "\n%s", stupidassline);
		/* done, we'll drop through the next if statement to the printing */
	}
	else
		add_buf(output, "No helps were found matching all of your keywords, but the\n"
			"following partial matches may direct you to the proper help:\n\n");

	if (partial_count && (result_count > partial_count || partial_count > 1))
	{
		int newres;

		for (result_num = 0; result_num < result_count; result_num++)
		{
			if (temp_help[result_num].type != 2)
				continue;

			/* no duplicates! */
			for (newres = 0; newres < result_num; newres++)
			{
				if (temp_help[newres].type != 2)
					continue;

				if (!str_cmp(temp_help[result_num].keywords, temp_help[newres].keywords))
					break;
			}

			if (newres != result_num)
				continue;

			ptb(output, "%s\n", temp_help[result_num].keywords);
		}
	}

	page_to_char(buf_string(output), ch);
	free_buf(output);
}

void do_hedit (CHAR_DATA *ch, char *argument)
{
	char cmd[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];
	MYSQL_RES *result;
	MYSQL_ROW row;

	smash_tilde (argument);
	
	argument = one_argument (argument,cmd);
	
	if (!cmd[0])
	{
		ptc(ch, "Syntax:  hedit new <keywords>\n\r"
			"               delete <id>\n\r"
			"               show <id>\n\r"
			"               %s|%s|%s|%s|%s <id> <value>\n\r",
			HCOL_GROUP, HCOL_ORDER, HCOL_LEVEL, HCOL_KEYS, HCOL_TEXT
		);
		return;
	}

	if (!str_cmp(cmd, "new")) {
		if (!argument[0]) {
			stc("You need to specify some keywords.\n\r", ch);
			return;
		}

		if (!db_commandf("do_hedit", "insert into " HTABLE " (" HCOL_KEYS ") values('%s')", db_esc(argument))) {
			stc("Could not create a help with those keywords.\n\r", ch);
			return;
		}

		if ((result = db_query("do_help", "select last_insert_id()")) == NULL) {
			stc("Couldn't retrieve the ID of the new help.\n\r", ch);
			return;
		}

		row = mysql_fetch_row(result);

		ptc(ch, "Success, the new help has an ID of %s.\n\r", row[0]);
		mysql_free_result(result);
		return;
	}

	argument = one_argument (argument,arg);
		
	if (!arg[0])
	{
		stc("What help do you want to operate on?\n\r",ch);
		return;
	}

	if (db_countf("do_hedit", "select count(*) from " HTABLE " where " HCOL_ID "=%s", arg) < 1) {
		stc("No help found with that ID.\n\r", ch);
		return;
	}

	if (!str_cmp(cmd, "delete")) {
		db_commandf("do_hedit", "delete from " HTABLE " where " HCOL_ID "=%s", arg);
		stc ("That help is history now.\n\r", ch);
		return;
	}

	if (!str_cmp(cmd, "show")) {
		if ((result = db_queryf("do_help",
			"select " HCOL_GROUP "," HCOL_ORDER "," HCOL_LEVEL "," HCOL_KEYS "," HCOL_TEXT
			" from " HTABLE " where " HCOL_ID "=%s", arg)) == NULL) {
			stc("Couldn't retrieve a help with that ID.\n\r", ch);
			return;
		}

		row = mysql_fetch_row(result);

		ptc(ch, "ID: %4s  File: %s  Order: %s  Level: %s\n\rKeywords: %s\n\r%s\n\r",
			arg, row[0], row[1], row[2], row[3], row[4]
		);

		mysql_free_result(result);
		return;
	}

	if (!argument[0]) {
		stc("What value do you want to set it to?\n\r", ch);
		return;
	}

	if (!str_cmp(cmd, HCOL_GROUP) || !str_cmp(cmd, HCOL_ORDER) || !str_cmp(cmd, HCOL_LEVEL)) {
		if (!is_number(argument)) {
			stc("New value has to be a number.\n\r", ch);
			return;
		}

		db_commandf("do_hedit", "update " HTABLE " set %s=%d where " HCOL_ID "=%s", cmd, atoi(argument), arg);
		stc("Done.\n\r", ch);
		return;
	}

	if (!str_cmp(cmd, HCOL_KEYS) || !str_cmp(cmd, HCOL_TEXT)) {
		db_commandf("do_hedit", "update " HTABLE " set %s=%d where " HCOL_ID "=%s", cmd, db_esc(argument), arg);
		stc("Done.\n\r", ch);
		return;
	}

	ptc(ch, "Unknown command '%s'.\n\r", cmd);
}
