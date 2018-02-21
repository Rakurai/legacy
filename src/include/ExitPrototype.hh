#pragma once

#include "declare.hh"
#include "String.hh"
#include "Flags.hh"
#include "Vnum.hh"

class ExitPrototype
{
public:
	ExitPrototype(FILE *fp);
	virtual ~ExitPrototype() {}

    String              description;
    String              keyword;
    Vnum                to_vnum = 0;
    Flags               exit_flags; // default values the exit will reset to
    int                 key = 0; // actually a vnum, but we use -1 as a placeholder for some reason

private:
    ExitPrototype(const ExitPrototype&);
	Exit& operator=(const Exit&);
};
