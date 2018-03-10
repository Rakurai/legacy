#include "declare.hh"
#include "argument.hh"
#include "Flags.hh"
#include "String.hh"

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

		if (pdot != nullptr) {
			char tmp_buf[strlen(argument)+1];
			strcpy(tmp_buf, argument);
			char *tdot = &tmp_buf[pdot - argument];
			*tdot = '\0';

			/* Check for good numeric */
			if (String(tmp_buf).is_number()) {
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
Flags::Bit entity_argument(const char *argument, char *arg)
{
	char tmp_buf[strlen(argument)+1];
	char *ap;   /* pointer to argument */
	int  number;
	Flags::Bit etype = Flags::none;
	/* check for and isolate leading numeric */
	number = number_argument(argument, tmp_buf);
	ap = tmp_buf;

	if (ap[0] != '\0' && ap[1] == '.') {
		switch (tolower(ap[0])) {
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
		switch (tolower(ap[0])) {
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
		sprintf(arg, "%d.%s", number, ap);

	return etype;
}

/*
 * Given a string like 14*foo, return 14 and 'foo'
*/
int mult_argument(const char *argument, char *arg)
{
	int number;
	char buf[strlen(argument)+1];

	strcpy(buf, argument);
	char *pstar = strchr(buf, '*');

	if (pstar == nullptr) {
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

		*arg_first = /*tolower(*/*argument/*)*/;
		arg_first++;
		argument++;
	}

	*arg_first = '\0';

	while (isspace(*argument))
		argument++;

	return argument;
}

int number_argument(const String& argument, String& arg) {
	char buf[argument.size()+1];
	int ret = number_argument(argument.c_str(), buf);
	arg.assign(buf);
	return ret;
}

Flags::Bit entity_argument(const String& argument, String& arg) {
	char buf[argument.size()+1];
	Flags::Bit ret = entity_argument(argument.c_str(), buf);
	arg.assign(buf);
	return ret;
}

int mult_argument(const String& argument, String& arg) {
	char buf[argument.size()+1];
	int ret = mult_argument(argument.c_str(), buf);
	arg.assign(buf);
	return ret;
}

const char *one_argument(const String& argument, String& arg) {
	char buf[argument.size()+1];
	const char *ret = one_argument(argument.c_str(), buf);
	arg.assign(buf);
	return ret;
}
