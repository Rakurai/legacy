#include "merc.h"
#include "recycle.h"
#include "memory.h"
#include "Format.hpp"

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
		if (c0 == LOWER(bstr[ichar]) && a.has_prefix(bstr + ichar))
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

	if (sstr1 <= sstr2 && astr == bstr + sstr2 - sstr1)
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
	static String output;
	int length;

	/* if string is longer than the space, just cut it off and return it */
	if ((length = s.uncolor().size()) > space) {
		Format::sprintf(output, "%s", str);
		output[space] = '\0';
	}
	else {
		int extraspace = space - length, lspace, rspace;
		lspace = extraspace / 2;
		rspace = extraspace - lspace;
		Format::sprintf(output, " ");

		while (--lspace > 0)    output += " ";

		output += str;

		while (rspace-- > 0)    output += " ";
	}

	return output.c_str();
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
	spaces = (((length - s.uncolor().size()) / 2));
	buf[0] = '\0';
	spacebuf[0] = '\0';

	for (x = 0; x < spaces; x++)
		spacebuf[x] = ' ';

	spacebuf[x] =  '\0';
	strcat(buf, spacebuf);
	strcat(buf, str);
	strcat(buf, spacebuf);

	if (String(buf).uncolor().size() == (length - 1))
		strcat(buf, " ");

	return buf;
}

/* Count characters in a string, ignoring color codes */
/* color_strlen now counts {{s as a character, fixes a few bugs -- Montrey */
int color_strlen(const String& argument)
{
	const char *str;
	int length;

	if (argument.empty())
		return 0;

	length = 0;
	str = argument.c_str();

	while (*str != '\0') {
		if (*str != '{') {
			str++;
			length++;
			continue;
		}

		if (*(++str) == '{')
			length++;

		str++;
	}

	return length;
}

/* Tell if a given string has a slash in it.
   This is useful for making sure a given name is not a directory name. */
bool has_slash(const char *str)
{
	return (strchr(str, '/') != NULL);
} /* end has_slash() */

/*
 * See if a string is one of the names of an object.
 */
bool is_name(const String& s, const String& nl, bool exact)
{
	String str(s), namelist(nl);

	/* we need ALL parts of string to match part of namelist */
	for (; ;) {  /* start parsing string */

		String part;
		str = one_argument(str, part);

		if (part.empty())
			return TRUE;

		/* check to see if this is part of namelist */
		String list(namelist);

		for (; ;) {  /* start parsing namelist */
			String name;
			list = one_argument(list, name);

			if (name.empty())  /* this name was not found */
				return FALSE;

			if (exact) {
				if (s == name)
					return TRUE;

				if (part == name)
					break;
			}
			else {
				if (s.is_prefix_of(name))
					return TRUE; /* full pattern match */

				if (part.is_prefix_of(name))
					break;
			}
		}
	}
}

/* Is Exact Name by Lotus */
bool is_exact_name(const String& s, const String& nl)
{
	return nl.has_words(s, TRUE);
}

bool is_exact_name_color(const String& s, const String& nl)
{
	return nl.uncolor().has_words(s.uncolor(), TRUE);
}

/* Note name match exact */
bool note_is_name(const String& s, const String& nl)
{
	if (s.empty() || nl.empty())
		return FALSE;

	return nl.has_words(s, FALSE);
}
