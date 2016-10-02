#ifndef GEM_H
#define GEM_H

// constants
#define MAX_GEM_TYPES               2
#define MAX_GEM_QUALITIES           7
#define MAX_GEM_SETTINGS            4

// obj value fields
#define GEM_VALUE_TYPE              0
#define GEM_VALUE_QUALITY           1

// gem qualities
#define GEM_QUALITY_A               0
#define GEM_QUALITY_B               1
#define GEM_QUALITY_C               2
#define GEM_QUALITY_D               3
#define GEM_QUALITY_E               4
#define GEM_QUALITY_F               5
#define GEM_QUALITY_G               6

// gem object primitives
#define OBJ_VNUM_GEM_RUBY           121
#define OBJ_VNUM_GEM_EMERALD        122

extern char *get_gem_short_string(OBJ_DATA *obj);

struct gem_type_table_t  /* additional data for pc races */
{
    char *      keyword;
    char		color_code;
    int         vnum;
    int			apply_loc;
    int			modifier[MAX_GEM_QUALITIES];
};

struct gem_quality_table_t
{
	char *		keyword;
	int			quality;
	int			level;
};

#endif // GEM_H
