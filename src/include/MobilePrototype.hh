#pragma once

#include "declare.hh"
#include "String.hh"

#include "String.hh"
#include "Flags.hh"

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
class MobilePrototype
{
public:
    MobilePrototype(FILE *fp, int vnum);
    virtual ~MobilePrototype() {}

    MobilePrototype *   next = nullptr;
    SPEC_FUN *          spec_fun = nullptr;
    Shop *              pShop = nullptr;
    sh_int              vnum = 0;
    Flags               group_flags;
    sh_int              count = 0;
    sh_int              killed = 0;
    String              player_name;
    String              short_descr;
    String              long_descr;
    String              description;
    Flags               act_flags;
    Flags               affect_flags;
    sh_int              alignment = 0;
    sh_int              level = 0;
    sh_int              hitroll = 0;
    sh_int              hit[3] = {0};
    sh_int              mana[3] = {0};
    sh_int              damage[3] = {0};
    sh_int              ac[4] = {0};
    sh_int              dam_type = 0;
    Flags               off_flags;
    Flags               absorb_flags;
    Flags               imm_flags;
    Flags               res_flags;
    Flags               vuln_flags;
    sh_int              start_pos = 0;
    sh_int              default_pos = 0;
    sh_int              sex = 0;
    sh_int              race = 0;
    long                wealth = 0;
    Flags               form_flags;
    Flags               parts_flags;
    sh_int              size = 0;
    String              material;
    MobProg *           mobprogs = nullptr;
    Flags               progtype_flags;

private:
    MobilePrototype();
    MobilePrototype(const MobilePrototype&);
    MobilePrototype& operator=(const MobilePrototype&);
};
