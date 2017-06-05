#pragma once

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
        Opponent *  next = NULL;

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
        Event *    next = NULL;

    private:
        Event(const Event&);
        Event& operator=(const Event&);
    };

    War();
    virtual ~War();

    War::Opponent *  chal[4] = {NULL};
    War::Opponent *  def[4] = {NULL};
    War::Event *    events = NULL;

    bool        ongoing = FALSE;

    bool        valid = FALSE;
    War *  previous = NULL;
    War *  next = NULL;

private:
    War(const War&);
    War& operator=(const War&);
};
