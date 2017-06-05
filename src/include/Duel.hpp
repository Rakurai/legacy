#pragma once

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

        int     minvnum = 0;    /* actual arena, not prep rooms or view room */
        int     maxvnum = 0;
        RoomPrototype *chalprep = NULL;
        RoomPrototype *defprep = NULL;
        RoomPrototype *viewroom = NULL;

        Duel::Arena *    previous = NULL;
        Duel::Arena *    next = NULL;
    private:
        Arena(const Arena&);
        Arena& operator=(const Arena&);
    };

    Duel() {}
    virtual ~Duel() {}

    Character * challenger = NULL;
    Character * defender = NULL;
    Duel::Arena *    arena = NULL;
    int     accept_timer = 0;
    int     prep_timer = 0;

    bool        valid = FALSE;
    Duel *previous = NULL;
    Duel *next = NULL;

private:
    Duel(const Duel&);
    Duel& operator=(const Duel&);
};
