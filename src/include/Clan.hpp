#pragma once

class Clan
{
public:
    Clan() {}
    virtual ~Clan() {}

    String  name;
    String  who_name;
    sh_int      hall = 0;
    sh_int      area_minvnum = 0;
    sh_int      area_maxvnum = 0;
    bool        independent = FALSE; /* true for loners and imm clans */
    long        clanqp = 0;
    long        gold_balance = 0;
    String  clanname;
    String  creator;
    int     score = 0;      /* points remaining to defeat in current war */
    int     warcpmod = 0;   /* how much their power is modified by wars */

    Clan * previous = NULL;
    Clan * next = NULL;

private:
    Clan(const Clan&);
    Clan& operator=(const Clan&);
};
