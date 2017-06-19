#include "Logging.hh"

#include "channels.hh"
#include "Flags.hh"
#include "merc.hh" // current_time
#include "String.hh"

/*
 * Reports a bug.
 */
void Logging::
bug(const String& str, int param)
{
	String buf = Format::format(String("[*****] BUG: ") + str, param);
	Logging::log(buf);
	wiznet(buf, nullptr, nullptr, WIZ_BUGS, 0, 0);
	return;
}

/*
 * Writes a string to the log.
 */
void Logging::
log(const String& str)
{
	char *strtime;
	strtime                    = ctime(&current_time);
	strtime[strlen(strtime) - 1] = '\0';
	Format::fprintf(stderr, "%s :: %s\n", strtime, str);
	return;
}
