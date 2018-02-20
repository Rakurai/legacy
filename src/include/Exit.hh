#pragma once

#include "declare.hh"
#include "String.hh"
#include "Flags.hh"

class Exit
{
public:
	Exit() {}
	virtual ~Exit() {}

    union
    {
        RoomPrototype *       to_room;
        int                  vnum;
    } u1 = {0};
    Flags               exit_flags;
    int              key = 0;
    String              keyword;
    String              description;

    static const String& dir_name(unsigned int dir, bool reverse = false);
    static unsigned int rev_dir(unsigned int dir);

private:
	Exit(const Exit&);
	Exit& operator=(const Exit&);
};

inline unsigned int Exit::
rev_dir(unsigned int dir) {
    static const int rdir[] = {
        2, 3, 0, 1, 5, 4
    };

    return rdir[dir % 6];
}

inline const String& Exit::
dir_name(unsigned int dir, bool reverse) {
    static const String dir_name[] = {
        "north", "east", "south", "west", "up", "down"
    };

    dir = dir % 6;

    if (reverse)
        dir = rev_dir(dir);

    return dir_name[dir % 6];
}
