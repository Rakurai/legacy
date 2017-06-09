#pragma once

#include <vector>
#include "declare.h"
#include "String.hpp"

class Area
{
public:
    Area(FILE *fp);
    virtual ~Area();

    void update();
    void reset();

    // descriptive vars
    String              file_name;
    String              name;
    String              credits;
    String              author;     /* -- Elrac */
    String              title;      /* -- Elrac */
    String              keywords;   /* -- Elrac */
    sh_int              version = 1;    /* Area file versioning -- Montrey */
    char                area_type = 0;  /* -- Elrac */
    sh_int              low_range = 0;
    sh_int              high_range = 0;
    sh_int              min_vnum = 0;
    sh_int              max_vnum = 0;

    // state
    sh_int              age = 15;
    sh_int              nplayer = 0;
    bool                empty = FALSE;

    // components
    std::vector<Reset *> resets;

private:
    Area();
    Area(const Area&);
    Area& operator=(const Area&);

    int scan_credits();
    /* pick a random room to reset into -- Montrey */
    RoomPrototype *get_random_reset_room() const;

};
