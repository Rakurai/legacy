#pragma once

class Area
{
public:
    Area() {}
    virtual ~Area() {}

    Area *         next = NULL;
    Reset *             reset_first = NULL;
    Reset *             reset_last = NULL;
    sh_int		        version = 0;	/* Area file versioning -- Montrey */
    String              file_name;
    String              name;
    String              credits;
    sh_int              age = 0;
    sh_int              nplayer = 0;
    sh_int              low_range = 0;
    sh_int              high_range = 0;
    sh_int              min_vnum = 0;
    sh_int              max_vnum = 0;
    bool                empty = FALSE;
    char                area_type = 0;  /* -- Elrac */
    String              author;     /* -- Elrac */
    String              title;      /* -- Elrac */
    String              keywords;   /* -- Elrac */

private:
    Area(const Area&);
    Area& operator=(const Area&);
};
