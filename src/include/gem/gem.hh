#pragma once

#include "String.hh"

class Object;

namespace gem {

// constants
constexpr int MAX_GEM_SETTINGS = 4;

/**
 * @defgroup GemValueFields Constants defining gem value fields
 * @{
 */
constexpr int GEM_VALUE_TYPE = 0;
constexpr int GEM_VALUE_QUALITY = 1;
/** @} */

/**
 * @defgroup GemObjectPrimitives Constants defining gem object primitives
 * @{
 */
constexpr int OBJ_VNUM_GEM_RUBY   = 121;
constexpr int OBJ_VNUM_GEM_EMERALD = 122;
/** @} */

// gem qualities
enum Quality : int {
	Rough = 0,
	Cracked,
	Flawed,
	Flawless,
	Perfect,
	Brilliant,
	Dazzling,
	COUNT
};

struct type_st
{
    char *      keyword;
    char		color_code;
    int         vnum;
    int			apply_loc;
    int			modifier[Quality::COUNT];
};

struct quality_st
{
	char *		keyword;
	int			quality;
	int			level;
};

void inset(Object *gem, Object *obj);
const String get_short_string(Object *obj);

} // namespace gem
