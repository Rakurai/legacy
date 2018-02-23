#pragma once

// numbers match those expected in area files
enum class Sector {
	none = -1,
	inside = 0,
	city = 1,
	field = 2,
	forest = 3,
	hills = 4,
	mountain = 5,
	water_swim = 6,
	water_noswim = 7,
//  unused,
	air = 9,
	desert = 10,
	arena = 20,
	clanarena = 21,
};
