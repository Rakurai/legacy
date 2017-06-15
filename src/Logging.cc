
#include "Logging.hh"
#include "channels.hh"
#include "merc.hh" // current_time

/*
 * Reports a bug.
 */
void Logging::
bug(const String& str, int param)
{
	String buf = Format::format(String("[*****] BUG: ") + str, param);
	log_string(buf);
	wiznet(buf, nullptr, nullptr, WIZ_BUGS, 0, 0);
	return;
}

/*
 * Writes a string to the log.
 */
void Logging::
log_string(const String& str)
{
	char *strtime;
	strtime                    = ctime(&current_time);
	strtime[strlen(strtime) - 1] = '\0';
	Format::fprintf(stderr, "%s :: %s\n", strtime, str);
	return;
}
