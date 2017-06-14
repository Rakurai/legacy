#pragma once

#include "declare.h"
#include "String.hpp"
#include "Flags.hpp"
#include "ObjectValue.hpp"

class ObjectPrototype
{
public:
    ObjectPrototype() {}
    virtual ~ObjectPrototype() {}

    ObjectPrototype *    next = nullptr;
    ExtraDescr *        extra_descr = nullptr;
    Affect *       affected = nullptr;
    unsigned long       affect_checksum = 0; // for comparing to instances on saving
    String              name;
    String              short_descr;
    String              description;
    sh_int              vnum = 0;
    sh_int              reset_num = 0;
    sh_int	            version = 0;
    String              material;
    sh_int              item_type = 0;
    Flags               extra_flags; /* Formerly INT */
    Flags               wear_flags;  /* Formerly INT */
    sh_int              level = 0;
    sh_int              condition = 0;
    sh_int              count = 0;
    sh_int              weight = 0;
    int                 cost = 0;
    ObjectValue         value[5];

    int                 num_settings = 0; // for socketed gems

private:
    ObjectPrototype(const ObjectPrototype&);
    ObjectPrototype& operator=(const ObjectPrototype&);
};
