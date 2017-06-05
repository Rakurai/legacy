#pragma once

class Shop;
class MobProg;

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
class MobilePrototype
{
public:
    MobilePrototype() {}
    virtual ~MobilePrototype() {}

    MobilePrototype *   next = NULL;
    SPEC_FUN *          spec_fun = NULL;
    Shop *              pShop = NULL;
    sh_int              vnum = 0;
    sh_int              group = 0;
    sh_int		        version = 0;
    sh_int              count = 0;
    sh_int              killed = 0;
    String              player_name;
    String              short_descr;
    String              long_descr;
    String              description;
    long                act = 0;
    long                affect_flags = 0;
    sh_int              alignment = 0;
    sh_int              level = 0;
    sh_int              hitroll = 0;
    sh_int              hit[3] = {0};
    sh_int              mana[3] = {0};
    sh_int              damage[3] = {0};
    sh_int              ac[4] = {0};
    sh_int              dam_type = 0;
    long                off_flags = 0;
    long		        absorb_flags = 0;
    long                imm_flags = 0;
    long                res_flags = 0;
    long                vuln_flags = 0;
    sh_int              start_pos = 0;
    sh_int              default_pos = 0;
    sh_int              sex = 0;
    sh_int              race = 0;
    long                wealth = 0;
    long                form = 0;
    long                parts = 0;
    sh_int              size = 0;
    String              material;
    MobProg *           mobprogs = NULL;
    int                 progtypes = 0;

private:
    MobilePrototype(const MobilePrototype&);
    MobilePrototype& operator=(const MobilePrototype&);
};
