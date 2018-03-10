#pragma once

#include "String.hh"
#include "Flags.hh"

class Character;

class Tail
{
public:
    Tail() {}
    virtual ~Tail() {}

    Character * tailed_by = nullptr;
    String      tailer_name;
    Flags       flags;
    Tail *      next = nullptr;

private:
	Tail(const Tail&);
	Tail& operator=(const Tail&);
};

/*
 * Tail (Snoop) flags
 */
#define TAIL_NONE                        Flags::none
#define TAIL_ACT                         Flags::A
#define TAIL_ALL                         TAIL_ACT
