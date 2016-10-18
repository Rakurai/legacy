#include "merc.h"
#include "recycle.h"
#include "memory.h"

extern char *string_space;
extern char *top_string;

/* semiperm strings:  a linked list of structures containing a string and a pointer to
   the next one.  at the end of each game_loop, we free the entire list.  these are used
   more as a shortcut than anything, so we can have multiple string function calls that
   would normally return static chars in the same sprintf type call. -- Montrey */
char *str_dup_semiperm(const char *string)
{
	SEMIPERM *semiperm = new_semiperm();
	semiperm->string = str_dup(string);
	semiperm->next = semiperm_list;
	semiperm_list = semiperm;
	return semiperm->string;
}

/*
 * Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared.
 */
char *str_dup(const char *str)
{
	char *str_new;

	if (str == NULL) {
		bug("str_dup: NULL string", 0);
		return &str_empty[0];
	}

	if (str[0] == '\0')
		return &str_empty[0];

	if (str >= string_space && str < top_string)
		return (char *) str;

	str_new = alloc_mem(strlen(str) + 1);
	strcpy(str_new, str);
	return str_new;
}

/*
 * Free a string.
 * Null is legal here to simplify callers.
 * Read-only shared strings are not touched.
 */
void free_string(char *pstr)
{
	if (pstr == NULL
	    ||   pstr == &str_empty[0]
	    || (pstr >= string_space && pstr < top_string))
		return;

	free_mem(pstr, strlen(pstr) + 1);
	return;
}

/* Removes the tildes from a string.
   Used for player-entered strings that go into disk files. */
const char *smash_tilde(const char *str)
{
	static char buf[MSL];
	char *pbuf;

	for (pbuf = buf; *str != '\0'; str++, pbuf++)
		if (*str == '~')
			*pbuf = '-';
		else
			*pbuf = *str;

	*pbuf = '\0';
	return buf;
}

/* Removes the brackets from a string.  Used to convert a color coded
   string to normal. */
const char *smash_bracket(const char *str)
{
	static char retstr[MSL];
	const char *p;
	char *q;

	if (strlen(str) + 1 >= sizeof(retstr))
		strncpy(retstr, str, sizeof(retstr) - 100);
	else
		strcpy(retstr, str);

	p = str;
	q = retstr;

	while (*p) {
		if (*p == '{') {
			if (p[1] == '\0')
				break;
			else if (p[1] == '{')
				*q++ = *p;

			p += 2;
		}
		else
			*q++ = *p++;
	}

	*q = '\0';
	return retstr;
}

/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp(const char *astr, const char *bstr)
{
	if (!astr || !bstr) {
		bugf("str_cmp: null %sstr", astr ? "b" : "a");
		return TRUE;
	}

	for (; *astr || *bstr; astr++, bstr++)
		if (LOWER(*astr) != LOWER(*bstr))
			return TRUE;

	return FALSE;
}

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix(const char *astr, const char *bstr)
{
	if (!astr || !bstr) {
		bugf("str_prefix: null %sstr", astr ? "b" : "a");
		return TRUE;
	}

	for (; *astr; astr++, bstr++)
		if (LOWER(*astr) != LOWER(*bstr))
			return TRUE;

	return FALSE;
}

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 * like str_prefix, but insists on at least 1 matching char.
 */
bool str_prefix1(const char *astr, const char *bstr)
{
	/* I feel stupid even doing this...We have a bug
	   somewhere that calls with function with an
	   invalid "astr" value. (astr == 1)
	   I'm hoping this check will help us out until
	   I can track it down.
	   -- Outsider
	*/
	if (astr == (char *) 1) return TRUE;

	if (!astr || !bstr) {
		bugf("str_prefix1: null %sstr", astr ? "b" : "a");
		return TRUE;
	}

	/* I think this should be "*bstr" not "bstr". -- Outsider */
	if (*astr == '\0' || *bstr == '\0')
		return TRUE;

	for (; *astr; astr++, bstr++)
		if (LOWER(*astr) != LOWER(*bstr))
			return TRUE;

	return FALSE;
}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE if astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix(const char *astr, const char *bstr)
{
	int sstr1, sstr2, ichar;
	char c0;

	if ((c0 = LOWER(astr[0])) == '\0')
		return FALSE;

	sstr1 = strlen(astr);
	sstr2 = strlen(bstr);

	for (ichar = 0; ichar <= sstr2 - sstr1; ichar++)
		if (c0 == LOWER(bstr[ichar]) && !str_prefix(astr, bstr + ichar))
			return FALSE;

	return TRUE;
}

/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix(const char *astr, const char *bstr)
{
	int sstr1, sstr2;
	sstr1 = strlen(astr);
	sstr2 = strlen(bstr);

	if (sstr1 <= sstr2 && !str_cmp(astr, bstr + sstr2 - sstr1))
		return FALSE;

	return TRUE;
}

/*
 * Returns an initial-capped string.
 */
const char *capitalize(const char *str)
{
	static char strcap[MSL];
	int i;

	for (i = 0; str[i] != '\0'; i++)
		strcap[i] = LOWER(str[i]);

	strcap[i] = '\0';
	strcap[0] = UPPER(strcap[0]);
	return strcap;
}

void strcut(char *str, int length)
{
	if (strlen(str) > length)
		str[length] = '\0';
}

const char *strcenter(const char *string, int space)
{
	static char output[MSL];
	int length;

	/* if string is longer than the space, just cut it off and return it */
	if ((length = color_strlen(string)) > space) {
		sprintf(output, "%s", string);
		output[space] = '\0';
	}
	else {
		int extraspace = space - length, lspace, rspace;
		lspace = extraspace / 2;
		rspace = extraspace - lspace;
		sprintf(output, " ");

		while (--lspace > 0)    strcat(output, " ");

		strcat(output, string);

		while (rspace-- > 0)    strcat(output, " ");
	}

	return output;
}

const char *strrpc(const char *replace, const char *with, const char *in)
{
	int replacelen = strlen(replace), i;
	static char out[MSL * 2];
	const char *replaceptr, *withptr = with, *inptr = in;
	char *outptr = out;

	if (replacelen <= 0
	    || strlen(in) < replacelen
	    || (replaceptr = strstr(in, replace)) == NULL)
		return in;

	while (inptr != replaceptr) {
		*outptr = *inptr;
		outptr++;
		inptr++;
	}

	while (*withptr) {
		*outptr = *withptr;
		outptr++;
		withptr++;
	}

	for (i = 0; i < replacelen; i++)
		inptr++;

	while (*inptr) {
		*outptr = *inptr;
		outptr++;
		inptr++;
	}

	*outptr = '\0';
	return out;
}

/* insert a string at a specified point in a string -- Montrey */
const char *strins(const char *string, const char *ins, int place)
{
	static char output[MSL];
	memset(output, 0, MSL);
	strncat(output, string, place);
	strcat(output, ins);
	strcat(output, string + place);
	return output;
}

const char *center_string_in_whitespace(const char *string, int length)
{
	char spacebuf[MAX_STRING_LENGTH];
	static char buf[MAX_STRING_LENGTH];
	int x, spaces;
	spaces = (((length - color_strlen(string)) / 2));
	buf[0] = '\0';
	spacebuf[0] = '\0';

	for (x = 0; x < spaces; x++)
		spacebuf[x] = ' ';

	spacebuf[x] =  '\0';
	strcat(buf, spacebuf);
	strcat(buf, string);
	strcat(buf, spacebuf);

	if (color_strlen(buf) == (length - 1))
		strcat(buf, " ");

	return buf;
}
