#pragma once

#include "declare.hh"
#include "String.hh"
#include "Flags.hh"
#include "ExitPrototype.hh"

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

    int key() const { return prototype.key; }
    const String& keyword() const { return prototype.keyword; }
    const String& description() const { return prototype.description; }

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
