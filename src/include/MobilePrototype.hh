#pragma once

#include <vector>

#include "declare.hh"
#include "String.hh"
#include "Flags.hh"
#include "Vnum.hh"

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
class MobilePrototype
{
public:
    MobilePrototype(Area&, const Vnum&, FILE *);
    virtual ~MobilePrototype();

    Area&                   area;
    const Vnum                vnum;
    MobilePrototype *   next = nullptr;
    SPEC_FUN *          spec_fun = nullptr;
    Shop *              pShop = nullptr;
    Flags               group_flags;
    int              count = 0;
    int              killed = 0;
    String              player_name;
    String              short_descr;
    String              long_descr;
    String              description;
    Flags               act_flags;
    Flags               affect_flags;
    int              alignment = 0;
    int              level = 0;
    int              hitroll = 0;
    int              hit[3] = {0};
    int              mana[3] = {0};
    int              damage[3] = {0};
    int              ac[4] = {0};
    int              dam_type = 0;
    Flags               off_flags;
    Flags               absorb_flags;
    Flags               imm_flags;
    Flags               res_flags;
    Flags               vuln_flags;
    int              start_pos = 0;
    int              default_pos = 0;
    int              sex = 0;
    int              race = 0;
    long                wealth = 0;
    Flags               form_flags;
    Flags               parts_flags;
    int              size = 0;
    String              material;
    std::vector<MobProg *> mobprogs;
    Flags               progtype_flags;

    void read_mobprogs(FILE *fp);

private:
    MobilePrototype();
    MobilePrototype(const MobilePrototype&);
    MobilePrototype& operator=(const MobilePrototype&);
};
