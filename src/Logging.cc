#include "Logging.hh"

#include "channels.hh"
#include "Flags.hh"
#include "Game.hh"
#include "merc.hh" // Game::current_time
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
	strtime                    = ctime(&Game::current_time);
	strtime[strlen(strtime) - 1] = '\0';
	Format::fprintf(stderr, "%s :: %s\n", strtime, str);
	return;
}

void Logging::
file_bug(FILE *fp, const String& str, int param)
{
	if (fp != nullptr) {
		int iLine = 0;
		int iChar = 0;

		if (fp != stdin) {
			iChar = ftell(fp);
			fseek(fp, 0, 0);

			for (iLine = 0; ftell(fp) < iChar; iLine++) {
				while (getc(fp) != '\n')
					;
			}

			fseek(fp, iChar, 0);
		}

		Logging::bugf("[*****] LINE: %d", iLine);
	}

	Logging::bugf(str, param);
}

void Logging::
file_bug(FILE *fp, const String& str, const Vnum& vnum) {
	file_bug(fp, str, vnum.value());
}
