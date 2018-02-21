#include "GameTime.hh"

#include "Character.hh"
#include "Flags.hh"
#include "Format.hh"
#include "macros.hh"
#include "merc.hh"
#include "Room.hh"
#include "String.hh"

GameTime::
GameTime(std::time_t system_time) {
	unsigned long lhour, lday, lmonth;
	lhour           = (system_time - 650336715) / (PULSE_TICK / PULSE_PER_SECOND);
	hour            = lhour  % MUD_DAY;
	lday            = lhour  / MUD_DAY;
	day             = lday   % MUD_MONTH;
	lmonth          = lday   / MUD_MONTH;
	month           = lmonth % MUD_YEAR;
	year            = lmonth / MUD_YEAR;

	     if (hour <  5) sunlight = Night;
	else if (hour <  6) sunlight = Sunrise;
	else if (hour < 19) sunlight = Day;
	else if (hour < 20) sunlight = Sunset;
	else                sunlight = Night;
}

const String& GameTime::
day_name() const {
	static const String days[] = {
		"Regeneration",
		"Endeavor",
		"The Sun",
		"The Great Gods",
		"The Blue Moon",
		"The Lesser Moon",
		"Omens"
	};

	return days[day % 7];
}

const String& GameTime::
month_name() const {
	static const String months[] = {
		"Abundance", "Perseverance", "Challenge",
		"Sacrifice", "Continuity", "Reverence",
		"Harmony", "Strife", "Peace",
		"Futility", "Courtship", "Awakening",
		"Long Shadows", "Silence", "Ancient Darkness",
		"Endings", "Rapture"
	};

	return months[month];
}

const String GameTime::
day_string() const {
	String suffix;

	if (day > 4 && day < 20) suffix = "th";
	else if (day % 10 == 1)  suffix = "st";
	else if (day % 10 == 2)  suffix = "nd";
	else if (day % 10 == 3)  suffix = "rd";
	else                     suffix = "th";

	return Format::format("%d%s", day + 1, suffix);
}

void GameTime::
update() {
	++hour;

	if (hour >= MUD_DAY) {
		hour = 0;
		day++;
	}

	if (day >= MUD_MONTH) {
		day = 0;
		month++;
	}

	if (month >= MUD_YEAR) {
		month = 0;
		year++;
	}

	String buf;

	switch (hour) {
	case  5:
		sunlight = Day;
		buf = Format::format("{BThera wakes to the Day of %s,\n%s of the Month of %s.{x\n",
			day_name(), day_string(), month_name());
		buf += "The Clocktower Bell rings as another day begins.\n";
		break;

	case  6:
		sunlight = Sunrise;
		buf = "The sun rises in the east.\n";
		break;

	case 12:
		buf = "The Clocktower Bell signals the midday.\n";
		break;

	case 19:
		sunlight = Sunset;
		buf = "The sun slowly disappears in the west.\n";
		break;

	case 20:
		sunlight = Night;
		buf = "The night has begun.\n";
		break;

	case MUD_DAY:
		buf = "The Clocktower Bell tolls, declaring midnight.\n";
		break;
	}

	for (Character *ch = char_list; ch != nullptr; ch = ch->next)
		/* why send it to mobs? */
		if (!IS_NPC(ch)
		 && IS_OUTSIDE(ch)
		 && IS_AWAKE(ch)
		 && ch->act_flags.has(PLR_TICKS))
			stc(buf, ch);
}
