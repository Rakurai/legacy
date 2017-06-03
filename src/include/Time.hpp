#pragma once

#include "String.hpp"

#define MUD_YEAR		17	/* months */
#define MUD_MONTH		35	/* days */
#define MUD_DAY			24	/* hours */
#define MUD_HOUR		45	/* seconds */

class Time
{
public:
	Time() {}
	virtual ~Time() {}

    int         hour;
    int         day;
    int         month;
    int         year;
    String      motd;

private:
	Time(const Time&);
	Time& operator=(const Time&);
};

extern Time time_info;
