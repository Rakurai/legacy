#pragma once

#include "declare.hh"

namespace gem {

// constants
#define MAX_GEM_SETTINGS            4

// obj value fields
#define GEM_VALUE_TYPE              0
#define GEM_VALUE_QUALITY           1

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

// gem object primitives
#define OBJ_VNUM_GEM_RUBY           121
#define OBJ_VNUM_GEM_EMERALD        122

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
