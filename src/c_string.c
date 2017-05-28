#include "merc.h"
#include "recycle.h"
#include "memory.h"
#include "Format.hpp"

extern char *string_space;
extern char *top_string;

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

	str_new = (char *)alloc_mem(strlen(str) + 1);
	strcpy(str_new, str);
	return str_new;
}

char *str_dup(const String& str) {
	return str_dup(str.data());
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
const char *smash_tilde(const String& s)
{
	const char *str = s.c_str();
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
const char *smash_bracket(const String& s)
{
	const char *str = s.c_str();
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
bool str_cmp(const String& a, const String& b)
{
	const char *astr = a.c_str(), *bstr = b.c_str();
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
 * Return TRUE if astr NOT a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix(const String& astr, const String& bstr)
{
	if (astr.empty() || bstr.empty())
		return FALSE;

	for (const char *a = astr.c_str(), *b = bstr.c_str(); *a && *b; a++, b++)
		if (LOWER(*a) != LOWER(*b))
			return TRUE;

	return FALSE;
}

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 * like str_prefix, but insists on at least 1 matching char.
 */
bool str_prefix1(const String& astr, const String& bstr)
{
	if (astr.empty() || bstr.empty())
		return TRUE;

	return str_prefix(astr, bstr);
}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE if astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix(const String& a, const String& b)
{
	const char *astr = a.c_str(), *bstr = b.c_str();
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
bool str_suffix(const String& a, const String& b)
{
	const char *astr = a.c_str(), *bstr = b.c_str();
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

void strcut(char *str, unsigned int length)
{
	if (strlen(str) > length)
		str[length] = '\0';
}

const char *strcenter(const String& s, int space)
{
	const char* str = s.c_str();
	static char output[MSL];
	int length;

	/* if string is longer than the space, just cut it off and return it */
	if ((length = color_strlen(str)) > space) {
		Format::sprintf(output, "%s", str);
		output[space] = '\0';
	}
	else {
		int extraspace = space - length, lspace, rspace;
		lspace = extraspace / 2;
		rspace = extraspace - lspace;
		Format::sprintf(output, " ");

		while (--lspace > 0)    strcat(output, " ");

		strcat(output, str);

		while (rspace-- > 0)    strcat(output, " ");
	}

	return output;
}

/* insert a string at a specified point in a string -- Montrey */
const char *strins(const String& string, const String& ins, int place)
{
	static char output[MSL];
	memset(output, 0, MSL);
	strncat(output, string, place);
	strcat(output, ins);
	strcat(output, string.c_str() + place);
	return output;
}

const char *center_string_in_whitespace(const String& s, int length)
{
	const char* str = s.c_str();
	char spacebuf[MAX_STRING_LENGTH];
	static char buf[MAX_STRING_LENGTH];
	int x, spaces;
	spaces = (((length - color_strlen(str)) / 2));
	buf[0] = '\0';
	spacebuf[0] = '\0';

	for (x = 0; x < spaces; x++)
		spacebuf[x] = ' ';

	spacebuf[x] =  '\0';
	strcat(buf, spacebuf);
	strcat(buf, str);
	strcat(buf, spacebuf);

	if (color_strlen(buf) == (length - 1))
		strcat(buf, " ");

	return buf;
}

/* Note name match exact */
bool note_is_name(const String& s, const String& nl)
{
	const char *str = s.c_str(), *namelist = nl.c_str();
	const char *list, *string;
	String part, name;

	/* fix crash on NULL namelist */
	if (namelist == NULL || namelist[0] == '\0')
		return FALSE;

	/* fixed to prevent is_name on "" returning TRUE */
	if (str[0] == '\0')
		return FALSE;

	string = str;

	/* we need ALL parts of string to match part of namelist */
	for (; ;) {  /* start parsing string */
		str = one_argument(str, part);

		if (part[0] == '\0')
			return TRUE;

		/* check to see if this is part of namelist */
		list = namelist;

		for (; ;) {  /* start parsing namelist */
			list = one_argument(list, name);

			if (name[0] == '\0')  /* this name was not found */
				return FALSE;

			if (!str_prefix1(string, name))
				return TRUE; /* full pattern match */

			if (!str_prefix1(part, name))
				break;
		}
	}
}

/* Is Exact Name by Lotus */
bool is_exact_name(const String& s, const String& nl)
{
	const char *str = s.c_str(), *namelist = nl.c_str();
	const char *list, *string;
	String part, name;
	string = str;

	/* we need ALL parts of string to match part of namelist */
	for (; ;) {  /* start parsing string */
		str = one_argument(str, part);

		if (part[0] == '\0')
			return TRUE;

		/* check to see if this is part of namelist */
		list = namelist;

		for (; ;) {  /* start parsing namelist */
			list = one_argument(list, name);

			if (name[0] == '\0')  /* this name was not found */
				return FALSE;

			if (!str_cmp(string, name))
				return TRUE; /* full pattern match */

			if (!str_cmp(part, name))
				break;
		}
	}
}

/*
 * See if a string is one of the names of an object.
 */
bool is_name(const String& s, const String& nl)
{
	const char *str = s.c_str(), *namelist = nl.c_str();
	const char *list, *string;
	String part, name;
	string = str;

	/* we need ALL parts of string to match part of namelist */
	for (; ;) {  /* start parsing string */
		str = one_argument(str, part);

		if (part[0] == '\0')
			return TRUE;

		/* check to see if this is part of namelist */
		list = namelist;

		for (; ;) {  /* start parsing namelist */
			list = one_argument(list, name);

			if (name[0] == '\0')  /* this name was not found */
				return FALSE;

			if (!str_prefix1(string, name))
				return TRUE; /* full pattern match */

			if (!str_prefix1(part, name))
				break;
		}
	}
}

bool is_exact_name_color(const String& s, const String& nl)
{
	const char *str = s.c_str(), *namelist = nl.c_str();
	const char *list, *string;
	String part, name;
	/* strip the color codes */
	str = smash_bracket(str);
	namelist = smash_bracket(namelist);
	string = str;

	/* we need ALL parts of string to match part of namelist */
	for (; ;) {  /* start parsing string */
		str = one_argument(str, part);

		if (part[0] == '\0')
			return TRUE;

		/* check to see if this is part of namelist */
		list = namelist;

		for (; ;) {  /* start parsing namelist */
			list = one_argument(list, name);

			if (name[0] == '\0')  /* this name was not found */
				return FALSE;

			if (!str_cmp(string, name))
				return TRUE; /* full pattern match */

			if (!str_cmp(part, name))
				break;
		}
	}
}
