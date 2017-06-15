#pragma once

#include "declare.hh"
#include "String.hh"

#define MUD_YEAR		17	/* months */
#define MUD_MONTH		35	/* days */
#define MUD_DAY			24	/* hours */
#define MUD_HOUR		45	/* seconds */

class GameTime
{
public:
	GameTime() {}
	virtual ~GameTime() {}

    int         hour = 0;
    int         day = 0;
    int         month = 0;
    int         year = 0;
    String      motd;

private:
	GameTime(const GameTime&);
	GameTime& operator=(const GameTime&);
};

extern GameTime time_info;
