#pragma once

#include "Format.hh"

namespace Logging {

void bug(const String& str, int param);
void log_string(const String& str);

template<class... Params>
void bugf(const String& fmt, Params... params)
{
	Logging::bug(Format::format(fmt, params...), 0);
}
	
}
