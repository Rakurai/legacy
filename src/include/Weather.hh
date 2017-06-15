#pragma once

#include "declare.hh"

class Weather
{
public:
	Weather() {}
	virtual ~Weather() {}

    int         mmhg = 0;
    int         change = 0;
    int         sky = 0;
    int         sunlight = 0;

private:
	Weather(const Weather&);
	Weather& operator=(const Weather&);
};

extern Weather weather_info;

/*
 * Time and weather stuff.
 */
#define SUN_DARK                    0
#define SUN_RISE                    1
#define SUN_LIGHT                   2
#define SUN_SET                     3

#define SKY_CLOUDLESS               0
#define SKY_CLOUDY                  1
#define SKY_RAINING                 2
#define SKY_LIGHTNING               3
