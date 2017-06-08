#pragma once

#include "declare.h"
#include "String.hpp"

class ObjectPrototype
{
public:
    ObjectPrototype() {}
    virtual ~ObjectPrototype() {}

    ObjectPrototype *    next = NULL;
    ExtraDescr *        extra_descr = NULL;
    Affect *       affected = NULL;
    unsigned long       affect_checksum = 0; // for comparing to instances on saving
    String              name;
    String              short_descr;
    String              description;
    sh_int              vnum = 0;
    sh_int              reset_num = 0;
    sh_int	            version = 0;
    String              material;
    sh_int              item_type = 0;
    unsigned long       extra_flags = 0; /* Formerly INT */
    unsigned long       wear_flags = 0;  /* Formerly INT */
    sh_int              level = 0;
    sh_int              condition = 0;
    sh_int              count = 0;
    sh_int              weight = 0;
    int                 cost = 0;
    int                 value[5] = {0};

    int                 num_settings = 0; // for socketed gems

private:
    ObjectPrototype(const ObjectPrototype&);
    ObjectPrototype& operator=(const ObjectPrototype&);
};
