#pragma once

#include "String.hh"

class GameTime;

class Weather
{
public:
	Weather(const GameTime& t);
	virtual ~Weather() {}

	void update();
	const String describe() const;

    enum Sky {
    	Cloudless,
    	Cloudy,
    	Raining,
    	Lightning,
    };

    int         mmhg = 960;
    int         change = 0;
    Sky         sky = Cloudless;

private:
	Weather(const Weather&);
	Weather& operator=(const Weather&);

	const GameTime& time;
};
