#pragma once

#include "String.hh"
#include "Flags.hh"

class ExitPrototype;
class Room;

class Exit
{
public:
	Exit(const ExitPrototype& p, Room *to_room);
	virtual ~Exit() {}

    const ExitPrototype& prototype;
    Flags exit_flags; // current state, this resets to the prototype's values on area resets
    Room *to_room;

    static const String& dir_name(unsigned int dir, bool reverse = false);
    static unsigned int rev_dir(unsigned int dir);

    int key() const;
    const String& keyword() const;
    const String& description() const;

private:
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
