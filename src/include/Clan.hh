#pragma once

#include "String.hh"
#include "Vnum.hh"
#include "Location.hh"

class Clan
{
public:
    Clan() {}
    virtual ~Clan() {}

    Location      recall{Vnum(3001)};
    String  name;
    String  who_name;
    Vnum      area_minvnum = 0;
    Vnum      area_maxvnum = 0;
    bool        independent = false; /* true for loners and imm clans */
    long        clanqp = 0;
    long        gold_balance = 0;
    String  clanname;
    String  creator;
    int     score = 0;      /* points remaining to defeat in current war */
    int     warcpmod = 0;   /* how much their power is modified by wars */

    Clan * previous = nullptr;
    Clan * next = nullptr;

private:
    Clan(const Clan&);
    Clan& operator=(const Clan&);
};

/* new clan system by Clerve */
extern      Clan    *clan_table_head;
extern      Clan    *clan_table_tail;

/* clan-edit.c */
int count_clan_members  args((Clan *clan, int bit));
void load_clan_table();
void save_clan_table();
int count_clans();
int calc_cp(Clan *clan, bool curve);
