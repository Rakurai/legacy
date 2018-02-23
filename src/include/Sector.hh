#pragma once

// numbers match those expected in area files.
// this needs to be convertable to unsigned char, so only 0-255 allowed
enum class Sector {
	inside = 0,
	city = 1,
	field = 2,
	forest_sparse = 3,
	hills = 4,
	mountain = 5,
	water_swim = 6,
	water_noswim = 7,
    road = 8,
	air = 9,
	desert = 10,
	forest_medium = 11,
	forest_dense = 12,
	arena = 20,
	clanarena = 21,

    area_entrance = 253,
    unknown = 254,
	none = 255 // maximum, don't go over this
};
