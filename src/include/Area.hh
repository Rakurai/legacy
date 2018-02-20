#pragma once

#include <vector>
#include "declare.hh"
#include "String.hh"

class Area
{
public:
    Area(World& w, FILE *fp);
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
    int              version = 1;    /* Area file versioning -- Montrey */
    char                area_type = 0;  /* -- Elrac */
    int              low_range = 0;
    int              high_range = 0;
    int              min_vnum = 0;
    int              max_vnum = 0;

    // state
    int              age = 15;
    int              nplayer = 0;
    bool                empty = FALSE;

    // components
    World& world;
    std::vector<Reset *> resets;

private:
    Area();
    Area(const Area&);
    Area& operator=(const Area&);

    int scan_credits();
    /* pick a random room to reset into -- Montrey */
    RoomPrototype *get_random_reset_room() const;
};
