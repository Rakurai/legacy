/*************************************************
*                                                *
*               The Age of Legacy                *
*                                                *
* Based originally on ROM 2.4, tested, enhanced, *
* and maintained by the Legacy Team.  If that    *
* doesn't mean you, and you're stealing our      *
* code, at least tell us and boost our egos ;)   *
*************************************************/

#include "merc.h"
#include "memory.h"
#include "db.h"
#include "Format.hpp"

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
	bool sign = FALSE;
	char c;

	do {
		c = getc(fp);
	}
	while (isspace(c));

	if (c == '+')
		c = getc(fp);
	else if (c == '-') {
		sign = TRUE;
		c = getc(fp);
	}

	if (!isdigit(c)) {
		bug("Fread_number: bad format.", 0);
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

long fread_flag(FILE *fp)
{
	int number;
	char c;
	bool sign = FALSE;

	do {
		c = getc(fp);
	}
	while (isspace(c));

	if (c == '-') {
		sign = TRUE;
		c = getc(fp);
	}

	number = 0;

	if (!isdigit(c)) {
		while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
			number += flag_convert(c);
			c = getc(fp);
		}
	}

	while (isdigit(c)) {
		number = number * 10 + c - '0';
		c = getc(fp);
	}

	if (sign)
		number = 0 - number;

	if (c == '|')
		number += fread_flag(fp);
	else if (c != ' ')
		ungetc(c, fp);

	return number;
}

/*
 * Read and allocate space for a string from a file.
 * These strings are read-only and shared.
 * Strings are hashed:
 *   each string prepended with hash pointer to prev string,
 *   hash code is simply the string length.
 *   this function takes 40% to 50% of boot-up time.
 */
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

	while (TRUE) {
		c = getc(fp);

		switch (c) {
			case EOF:
				bug("Fread_string: EOF", 0);
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

	bug("Fread_word: word too long.", 0);
//	exit(1);
	return NULL;
}

/* Append a string to a file */
void fappend(const char *file, const char *str)
{
	FILE *fp;

	if (str[0] == '\0')
		return;

	if ((fp = fopen(file, "a")) != NULL) {
		fputs(str, fp);
		fclose(fp);
	}
	else
		bugf("fappend(): could not open %s", file);
}
