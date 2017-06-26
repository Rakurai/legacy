#pragma once

#include "declare.hh"
#include "Format.hh"

namespace Logging {

void bug(const String& str, int param);
void log(const String& str);

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
