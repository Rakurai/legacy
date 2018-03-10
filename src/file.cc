/*************************************************
*                                                *
*               The Age of Legacy                *
*                                                *
* Based originally on ROM 2.4, tested, enhanced, *
* and maintained by the Legacy Team.  If that    *
* doesn't mean you, and you're stealing our      *
* code, at least tell us and boost our egos ;)   *
*************************************************/

#include "constants.hh"
#include "file.hh"

#include "Flags.hh"
#include "Logging.hh"
#include "String.hh"
#include "Game.hh"

/* Read a letter from a file. */
char fread_letter(FILE *fp)
{
	char c;

	do {
		c = getc(fp);
	}
	while (isspace(c));

	return c;
}

/* Read a number from a file. */
int fread_number(FILE *fp)
{
	int number = 0;
	bool sign = false;
	char c;

	do {
		c = getc(fp);
	}
	while (isspace(c));

	if (c == '+')
		c = getc(fp);
	else if (c == '-') {
		sign = true;
		c = getc(fp);
	}

	if (!isdigit(c)) {
		Logging::bug("Fread_number: bad format.", 0);
		number = 0;
		return number;
	}

	while (isdigit(c)) {
		number = number * 10 + c - '0';
		c = getc(fp);
	}

	if (sign)
		number = 0 - number;

	if (c == '|')
		number += fread_number(fp);
	else if (c != ' ')
		ungetc(c, fp);

	return number;
}

const Flags fread_flag(FILE *fp)
{
	return Flags(fread_word(fp));
}

String fread_string(FILE *fp, char to_char)
{
	char c;

	/* Skip blanks. Read first char. */
	do {
		c = getc(fp);
	}
	while (isspace(c));

	if (c == to_char)
		return "";

	String buf;
	buf = c;

	while (true) {
		c = getc(fp);

		switch (c) {
			case EOF:
				Logging::bug("Fread_string: EOF", 0);
				return buf;
			case '\r':
				break; // skip it
			default:
				if (c == to_char)
					return buf;

				buf += c;
				break;
		}
	}
}

String fread_string_eol(FILE *fp)
{
	return fread_string(fp, '\n');
}

/*
 * Read to end of line (for comments).
 */
void fread_to_eol(FILE *fp)
{
	char c;

	do {
		c = getc(fp);
	}
	while (c != '\n' && c != '\r');

	do {
		c = getc(fp);
	}
	while (c == '\n' || c == '\r');

	ungetc(c, fp);
}

/*
 * Read one word (into static buffer).
 */
String fread_word(FILE *fp)
{
	char word[MIL];
	char *pword;
	char cEnd;

	do {
		cEnd = getc(fp);
	}
	while (isspace(cEnd));

	if (cEnd == '\'' || cEnd == '"')
		pword = word;
	else {
		word[0] = cEnd;
		pword   = word + 1;
		cEnd    = ' ';
	}

	for (; pword < word + MIL; pword++) {
		*pword = getc(fp);

		if (cEnd == ' ' ? isspace(*pword) : *pword == cEnd) {
			if (cEnd == ' ')
				ungetc(*pword, fp);

			*pword = '\0';
			return word;
		}
	}

	Logging::bug("Fread_word: word too long.", 0);
//	exit(1);
	return "";
}

/* Append a string to a file */
void fappend(const String& file, const String& str)
{
	FILE *fp;

	if (str.empty())
		return;

	if ((fp = fopen(file.c_str(), "a")) != nullptr) {
		fputs(str.c_str(), fp);
		fclose(fp);
	}
	else
		Logging::bugf("fappend(): could not open %s", file);
}


/*
 * This function works just like ctime() does on current Linux systems.
 * I am only implementing it to make sure that dizzy_scantime(), which
 * decodes the output from ctime() and dizzy_ctime(), will always work
 * even if the system on which this code is run implements ctime()
 * differently.
 *
 * The output format for dizzy_ctime() is like this:
 *      Wed Jun 30 21:49:08 1993\n
 *
 * Like ctime(), dizzy_ctime() writes to a static string which will change
 * with the next invocation of dizzy_ctime().
 */

static const String day_names[] =
{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const String month_names[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

const char *dizzy_ctime(time_t *timep)
{
	static char ctime_buf[40];
	struct tm loc_tm;
	loc_tm = *localtime(timep);
	Format::sprintf(ctime_buf, "%s %s %02d %02d:%02d:%02d %04d\n",
	        day_names[loc_tm.tm_wday],
	        month_names[loc_tm.tm_mon],
	        loc_tm.tm_mday,
	        loc_tm.tm_hour, loc_tm.tm_min, loc_tm.tm_sec,
	        1900 + loc_tm.tm_year);
	return ctime_buf;
} /* end dizzy_ctime() */

/*
 * decode a time string as produced by dizzy_ctime()
 * Day of week is scanned in spite of not being needed so that the
 * return value from Format::sprintf() will be significant.
 */
time_t dizzy_scantime(const String& ctime)
{
	char cdow[4], cmon[4];
	int year, month, day, hour, minute, second;
	char msg[MAX_INPUT_LENGTH];
	struct tm loc_tm;
	/* this helps initialize local-dependent stuff like TZ, etc. */
	loc_tm = *localtime(&Game::current_time);

	if (sscanf(ctime.c_str(), " %3s %3s %d %d:%d:%d %d",
	           cdow, cmon, &day, &hour, &minute, &second, &year) < 7) {
		Format::sprintf(msg, "dizzy_scantime(): Error scanning date/time: '%s'", ctime);
		Logging::bug(msg, 0);
		goto endoftime;
	}

	for (month = 0; month < 12; month++) {
		if (month_names[month].is_prefix_of(ctime.substr(4)))
			break;
	}

	if (month >= 12) {
		Format::sprintf(msg, "dizzy_scantime(): Bad month in %s", ctime);
		Logging::bug(msg, 0);
		goto endoftime;
	}

	loc_tm.tm_mon  = month;
	loc_tm.tm_mday = day;
	loc_tm.tm_hour = hour;
	loc_tm.tm_min  = minute;
	loc_tm.tm_sec  = second;
	loc_tm.tm_year = year - 1900;
endoftime:
	return mktime(&loc_tm);
} /* end dizzy_scantime() */
