#pragma once

#include "String.hh"
#include "Vnum.hh"

class Character;
class Room;

class Duel
{
public:
    class Arena
    {
    public:
        Arena() {}
        virtual ~Arena() {}

        String  keyword;
        String  name;
        String  desc;

        Vnum minvnum = 0;    /* actual arena, not prep rooms or view room */
        Vnum maxvnum = 0;
        Room *chalprep = nullptr;
        Room *defprep = nullptr;
        Room *viewroom = nullptr;

        Duel::Arena *    previous = nullptr;
        Duel::Arena *    next = nullptr;
    private:
        Arena(const Arena&);
        Arena& operator=(const Arena&);
    };

    Duel() {}
    virtual ~Duel() {}

    Character * challenger = nullptr;
    Character * defender = nullptr;
    Duel::Arena *    arena = nullptr;
    int     accept_timer = 0;
    int     prep_timer = 0;

    Duel *previous = nullptr;
    Duel *next = nullptr;

private:
    Duel(const Duel&);
    Duel& operator=(const Duel&);
};

extern      Duel::Arena *arena_table_head;
extern      Duel::Arena *arena_table_tail;
extern      Duel    *duel_table_head;
extern      Duel    *duel_table_tail;
