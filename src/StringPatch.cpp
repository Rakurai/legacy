#include <string>
#include "merc.h"
#include "String.hpp"

char * 
strcat(char *dest, const String& src) {
	return std::strcat(dest, src.c_str());
}

char * 
strncat(char *dest, const String& src, size_t n) {
	return std::strncat(dest, src.c_str(), n);
}

char * 
strcpy(char *dest, const String& src) {
	return std::strcpy(dest, src.c_str());
}

char * 
strncpy(char *dest, const String& src, size_t n) {
	return std::strncpy(dest, src.c_str(), n);
}

size_t
strlen(const String& str) {
	return str.size();
}

const char *
strchr(const String& str, int ch) {
	return std::strchr(str.c_str(), ch);
}

const char *
strstr(const String& astr, const String& bstr) {
	return std::strstr(astr.c_str(), bstr.c_str());
}

int
strcmp(const String& astr, const String& bstr) {
	return std::strcmp(astr.c_str(), bstr.c_str());
}

int
strncmp(const String& astr, const String& bstr, size_t n) {
	return std::strncmp(astr.c_str(), bstr.c_str(), n);
}

int strcasecmp(const String& astr, const String& bstr) {
	return strcasecmp(astr.c_str(), bstr.c_str());
}

bool is_number(const String& str) {
	extern bool is_number(const char *str);
	return is_number(str.c_str());
}

/*
 * Given a string like 14.foo, return 14 and 'foo'
 *
 * It is most certainly NOT ok for argument to be identical to arg!
 * The strcpy at the end sometimes throws a SIGABRT on overlap! -- Montrey
 */
int number_argument(const char *argument, char *arg)
{
	/* Check for leading digit. Saves time */
	if (*argument >= '0' && *argument <= '9') {
		/* Check for dot */
		const char *pdot = strstr(argument, ".");

		if (pdot != NULL) {
			char tmp_buf[MAX_INPUT_LENGTH];
			strcpy(tmp_buf, argument);
			char *tdot = &tmp_buf[pdot - argument];
			*tdot = '\0';

			/* Check for good numeric */
			if (is_number(tmp_buf)) {
				/* strip out number, return numeric value */
				strcpy(arg, tdot + 1);
				return atoi(tmp_buf);
			}
		}
	}

	strcpy(arg, argument);
	return 1;
}

/*
 * scan a 'p.' or 'm.' or 'o.' or 'r.' out of an argument
 * before or after a <number>'.' .
 *
 * 'p.'{<number>'.'}<name> -> ENTITY_P and {<number>'.'}<name>
 * 'm.'{<number>'.'}<name> -> ENTITY_M and {<number>'.'}<name>
 * <number>'.p.'<name> -> ENTITY_P and <number>'.'<name>
 * <number>'.m.'<name> -> ENTITY_M and <number>'.'<name>
 *
 * - It's OK for argument to be identical to arg, but arg MUST be a
 * character array rather than a str_dup()'d buffer, as its length
 * may change!
 *
 * It is most certainly NOT ok for argument to be identical to arg!
 * The strcpy at the end sometimes throws a SIGABRT on overlap! -- Montrey
 */
int entity_argument(const char *argument, char *arg)
{
	char tmp_buf[MAX_STRING_LENGTH];
	char *ap;   /* pointer to argument */
	int  number;
	int  etype = 0;
	/* check for and isolate leading numeric */
	number = number_argument(argument, tmp_buf);
	ap = tmp_buf;

	if (ap[0] != '\0' && ap[1] == '.') {
		switch (LOWER(ap[0])) {
		case 'p':
			etype = ENTITY_P;  ap += 2;
			break;

		case 'm':
			etype = ENTITY_M;  ap += 2;
			break;

		case 'o':
			etype = ENTITY_O;  ap += 2;
			break;

		case 'r':
			etype = ENTITY_R;  ap += 2;
			break;
		}
	}
	else if (ap[0] != '\0' && ap[1] == '#') {
		switch (LOWER(ap[0])) {
		case 'm':
			etype = ENTITY_VM;  ap += 2;
			break;

		case 'o':
			etype = ENTITY_VO;  ap += 2;
			break;

		case 'r':
			etype = ENTITY_VR;  ap += 2;
			break;
		}
	}

	/* recombine number (if any) and remaining arg. */
	if (number == 1)
		strcpy(arg, ap);
	else
		Format::sprintf(arg, "%d.%s", number, ap);

	return etype;
}

/*
 * Given a string like 14*foo, return 14 and 'foo'
*/
int mult_argument(const char *argument, char *arg)
{
	int number;
	char buf[MSL];

	strcpy(buf, argument);
	char *pstar = strchr(buf, '*');

	if (pstar == NULL) {
		/* no star: return 1 */
		strcpy(arg, argument);
		return 1;
	}

	*pstar = '\0';
	number = atoi(argument);
	*pstar = '*';
	strcpy(arg, pstar + 1);
	return number;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
const char *one_argument(const char *argument, char *arg_first)
{
	char cEnd;

	while (isspace(*argument))
		argument++;

	cEnd = ' ';

	if (*argument == '\'' || *argument == '"')
		cEnd = *argument++;

	while (*argument != '\0') {
		if (*argument == cEnd) {
			argument++;
			break;
		}

		*arg_first = /*LOWER(*/*argument/*)*/;
		arg_first++;
		argument++;
	}

	*arg_first = '\0';

	while (isspace(*argument))
		argument++;

	return argument;
}
