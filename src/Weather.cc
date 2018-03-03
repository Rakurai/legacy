#include "Weather.hh"

#include "Character.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "GameTime.hh"
#include "Logging.hh"
#include "macros.hh"
#include "merc.hh"
#include "random.hh"
#include "Room.hh"

Weather::
Weather(const GameTime& t) :
	time(t)
{
	if (time.month >= 7 && time.month <= 12)
		mmhg += number_range(1, 50);
	else
		mmhg += number_range(1, 80);

	     if (mmhg <=  980)  sky = Lightning;
	else if (mmhg <= 1000)  sky = Raining;
	else if (mmhg <= 1020)  sky = Cloudy;
	else                    sky = Cloudless;
}

const String Weather::
describe() const {
	static const String sky_look[4] = {
		"bright and sunny",
		"cloudy",
#ifdef SEASON_CHRISTMAS
		"white with snow",
#else
		"raining with heavy droplets of water",
#endif
		"on fire with lightning"
	};

	String buf = Format::format("The sky is %s and %s.\n",
		sky_look[sky],
		change >= 0
#ifdef SEASON_CHRISTMAS
		? "a cold southerly breeze blows"
		: "a freezing northern gust blows"
#else
		? "a warm southerly breeze blows"
		: "a cold northern gust blows"
#endif
	);

	return buf;
}

void Weather::
update()
{
	String buf;
	int diff;

	/*
	 * Weather change.
	 */
	if (time.month >= 9 && time.month <= 16)
		diff = mmhg > 985 ? -2 : 2;
	else
		diff = mmhg > 1015 ? -2 : 2;

	change    += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
	change     = UMAX(change, -12);
	change     = UMIN(change, 12);
	mmhg      += change;
	mmhg       = UMAX(mmhg, 960);
	mmhg       = UMIN(mmhg, 1040);

	switch (sky) {
	default:
		Logging::bug("Weather_update: bad sky %d.", sky);
		sky = Cloudless;
		break;

	case Cloudless:
		if (mmhg < 990 || (mmhg < 1010 && number_bits(2) == 0)) {
			buf += "The sky grows dark with rolling grey clouds.\n";
			sky = Cloudy;
		}

		break;

	case Cloudy:
		if (mmhg < 970 || (mmhg < 990 && number_bits(2) == 0)) {
#ifdef SEASON_CHRISTMAS
			buf += "Snow starts to fall from the sky.\n";
#else
			buf += "It starts to rain a heavy downpour.\n";
#endif
			sky = Raining;
		}

		if (mmhg > 1030 && number_bits(2) == 0) {
			buf += "Shafts of light cut through the dense clouds above.\n";
			sky = Cloudless;
		}

		break;

	case Raining:
		if (mmhg < 970 && number_bits(2) == 0) {
			buf += "Lightning flashes in the sky.\n";
			sky = Lightning;
		}

		if (mmhg > 1030 || (mmhg > 1010 && number_bits(2) == 0)) {
#ifdef SEASON_CHRISTMAS
			buf += "The snow slows to a few flakes, and finally stops.\n";
#else
			buf += "The rain stopped.\n";
#endif
			sky = Cloudy;
		}

		break;

	case Lightning:
		if (mmhg > 1010 || (mmhg >  990 && number_bits(2) == 0)) {
			buf += "The lightning has stopped.\n";
			sky = Raining;
			break;
		}

		break;
	}

	if (!buf.empty()) {
		for (Character *ch = Game::world().char_list; ch != nullptr; ch = ch->next)
			/* why send it to mobs? */
			if (!IS_NPC(ch)
			 && IS_OUTSIDE(ch)
			 && IS_AWAKE(ch)
			 && ch->act_flags.has(PLR_TICKS))
				stc(buf, ch);
	}
}
