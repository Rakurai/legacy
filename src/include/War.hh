#pragma once

#include "declare.hh"
#include "String.hh"

class War
{
public:
    class Opponent
    {
    public:
        Opponent() {}
        virtual ~Opponent() {}

        String  name;
        String  clanname;
        bool        inwar = FALSE;
        sh_int      start_score = 0;
        sh_int      final_score = 0;

        bool        valid = FALSE;
        Opponent *  next = nullptr;

    private:
        Opponent(const Opponent&);
        Opponent& operator=(const Opponent&);
    };

    class Event
    {
    public:
        Event() {}
        virtual ~Event() {}

        sh_int      type = 0;
        String  astr;
        String  bstr;
        sh_int      number = 0;
        time_t      time = 0;

        bool        valid = FALSE;
        Event *    next = nullptr;

    private:
        Event(const Event&);
        Event& operator=(const Event&);
    };

    War();
    virtual ~War();

    War::Opponent *  chal[4] = {nullptr};
    War::Opponent *  def[4] = {nullptr};
    War::Event *    events = nullptr;

    bool        ongoing = FALSE;

    bool        valid = FALSE;
    War *  previous = nullptr;
    War *  next = nullptr;

private:
    War(const War&);
    War& operator=(const War&);
};

/* war recycling */
War *new_war args((void));
void        free_war args((War *war));

/* war opponent recycling */
War::Opponent   *new_opp args((void));
void        free_opp args((War::Opponent *opp));

/* war event recycling */
War::Event  *new_event args((void));
void        free_event args((War::Event *event));
