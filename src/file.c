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
char *fread_string(FILE *fp)
{
	char *plast, c;
	plast = top_string + sizeof(char *);

	if (plast > &string_space[MAX_STRING - MAX_STRING_LENGTH]) {
		bug("Fread_string: MAX_STRING %d exceeded.", MAX_STRING);
		exit(1);
	}

	/* Skip blanks. Read first char. */
	do {
		c = getc(fp);
	}
	while (isspace(c));

	if ((*plast++ = c) == '~')
		return &str_empty[0];

	for (; ;) {
		switch (*plast = getc(fp)) {
		default:
			plast++;
			break;

		case EOF:
			/* temp fix */
			bug("Fread_string: EOF", 0);
			return NULL;

		/* exit( 1 ); */

		case '\n':
			plast++;
			break;

		case '\r':
			break;

		case '~':
			plast++;
			{
				union {
					char *pc;
					char rgc[sizeof(char *)];
				} u1;
				int ic, iHash;
				char *pHash, *pHashPrev, *pString;
				plast[-1] = '\0';
				iHash = UMIN(MAX_KEY_HASH - 1, plast - 1 - top_string);

				for (pHash = string_hash[iHash]; pHash; pHash = pHashPrev) {
					for (ic = 0; ic < sizeof(char *); ic++)
						u1.rgc[ic] = pHash[ic];

					pHashPrev = u1.pc;
					pHash += sizeof(char *);

					if (top_string[sizeof(char *)] == pHash[0]
					    && !strcmp(top_string + sizeof(char *) + 1, pHash + 1))
						return pHash;
				}

				if (fBootDb) {
					pString         = top_string;
					top_string      = plast;
					u1.pc           = string_hash[iHash];

					for (ic = 0; ic < sizeof(char *); ic++)
						pString[ic] = u1.rgc[ic];

					string_hash[iHash] = pString;
					nAllocString++;
					sAllocString += top_string - pString;
					/* Has crashed on above line before - Lotus */
					return pString + sizeof(char *);
				}
				else
					return str_dup(top_string + sizeof(char *));
			}
		}
	}
}

char *fread_string_eol(FILE *fp)
{
	static bool char_special[256 - EOF];
	char *plast;
	char c;

	if (char_special[EOF - EOF] != TRUE) {
		char_special[EOF -  EOF] = TRUE;
		char_special['\n' - EOF] = TRUE;
		char_special['\r' - EOF] = TRUE;
	}

	plast = top_string + sizeof(char *);

	if (plast > &string_space[MAX_STRING - MAX_STRING_LENGTH]) {
		bug("Fread_string: MAX_STRING %d exceeded.", MAX_STRING);
		exit(1);
	}

	/*
	 * Skip blanks.
	 * Read first char.
	 */
	do {
		c = getc(fp);
	}
	while (isspace(c));

	if ((*plast++ = c) == '\n')
		return &str_empty[0];

	for (;;) {
		if (!char_special[(*plast++ = getc(fp)) - EOF ])
			continue;

		switch (plast[-1]) {
		default:
			break;

		case EOF:
			bug("Fread_string_eol  EOF", 0);
			exit(1);
			break;

		case '\n':  case '\r': {
				union {
					char       *pc;
					char        rgc[sizeof(char *)];
				} u1;
				int ic;
				int iHash;
				char *pHash;
				char *pHashPrev;
				char *pString;
				plast[-1] = '\0';
				iHash     = UMIN(MAX_KEY_HASH - 1, plast - 1 - top_string);

				for (pHash = string_hash[iHash]; pHash; pHash = pHashPrev) {
					for (ic = 0; ic < sizeof(char *); ic++)
						u1.rgc[ic] = pHash[ic];

					pHashPrev = u1.pc;
					pHash    += sizeof(char *);

					if (top_string[sizeof(char *)] == pHash[0]
					    &&   !strcmp(top_string + sizeof(char *) + 1, pHash + 1))
						return pHash;
				}

				if (fBootDb) {
					pString             = top_string;
					top_string          = plast;
					u1.pc               = string_hash[iHash];

					for (ic = 0; ic < sizeof(char *); ic++)
						pString[ic] = u1.rgc[ic];

					string_hash[iHash]  = pString;
					nAllocString += 1;
					sAllocString += top_string - pString;
					return pString + sizeof(char *);
				}
				else
					return str_dup(top_string + sizeof(char *));
			}
		}
	}
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
char *fread_word(FILE *fp)
{
	static char word[MIL];
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
