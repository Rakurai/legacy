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
constexpr Flags::Bit TAIL_NONE = Flags::none;
constexpr Flags::Bit TAIL_ACT  = Flags::A;
constexpr Flags::Bit TAIL_ALL  = TAIL_ACT;
