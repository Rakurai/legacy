#pragma once

#include "declare.hh"
#include <ctime>

class GameTime
{
public:
	GameTime(std::time_t system_time);
	virtual ~GameTime() {}

	void update();

    const String& day_name() const;
    const String day_string() const;
    const String& month_name() const;

    enum Sun {
    	Night,
    	Sunrise,
    	Sunset,
    	Day,
    };

    int         hour = 0;
    int         day = 0;
    int         month = 0;
    int         year = 0;
    Sun         sunlight = Night;

private:
	GameTime(const GameTime&);
	GameTime& operator=(const GameTime&);
};

#define MUD_YEAR		17	/* months */
#define MUD_MONTH		35	/* days */
#define MUD_DAY			24	/* hours */
#define MUD_HOUR		45	/* seconds */
