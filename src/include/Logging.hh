#pragma once

#include "Format.hh"

class String;
class Vnum;

namespace Logging {

void bug(const String& str, int param);
void log(const String& str);
void file_bug(FILE *fp, const String& str, int param);
void file_bug(FILE *fp, const String& str, const Vnum& vnum);

template<class... Params>
void bugf(const String& fmt, Params... params)
{
	Logging::bug(Format::format(fmt, params...), 0);
}

template<class... Params>
void logf(const String& fmt, Params... params)
{
	Logging::log(Format::format(fmt, params...));
}

}
