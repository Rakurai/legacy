#pragma once

#include "affect/Affect.hh"
#include "declare.hh"
#include "String.hh"
#include "Flags.hh"
#include "ObjectValue.hh"

class ObjectPrototype
{
public:
    ObjectPrototype() {}
    virtual ~ObjectPrototype() {}

    ObjectPrototype *    next = nullptr;
    ExtraDescr *        extra_descr = nullptr;
    affect::Affect *       affected = nullptr;
    unsigned long       affect_checksum = 0; // for comparing to instances on saving
    String              name;
    String              short_descr;
    String              description;
    int              vnum = 0;
    int              reset_num = 0;
    int	            version = 0;
    String              material;
    int              item_type = 0;
    Flags               extra_flags; /* Formerly INT */
    Flags               wear_flags;  /* Formerly INT */
    int              level = 0;
    int              condition = 0;
    int              count = 0;
    int              weight = 0;
    int                 cost = 0;
    ObjectValue         value[5];

    int                 num_settings = 0; // for socketed gems
	//int					suffix[3];
	
private:
    ObjectPrototype(const ObjectPrototype&);
    ObjectPrototype& operator=(const ObjectPrototype&);
};
