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

    War *  previous = nullptr;
    War *  next = nullptr;

private:
    War(const War&);
    War& operator=(const War&);
};
