#pragma once

#include <set>
#include "progs/Prog.hh"

#include "String.hh"
#include "Flags.hh"
#include "ObjectValue.hh"
#include "Vnum.hh"
#include "Guild.hh"

class Area;
class ExtraDescr;
namespace affect { class Affect; }

class ObjectPrototype
{
public:
    ObjectPrototype(Area&, const Vnum&, FILE *);
    virtual ~ObjectPrototype() {}

    Area&                   area;
    const Vnum              vnum;
    ObjectPrototype *    next = nullptr;
    ExtraDescr *        extra_descr = nullptr;
    affect::Affect *       affected = nullptr;
    unsigned long       affect_checksum = 0; // for comparing to instances on saving
    String              name;
    String              short_descr;
    String              description;
    int              reset_num = 0;
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
	Guild			guild = Guild::none;
	//int					suffix[3];

    std::vector<progs::Prog *> progs;
    std::set<progs::Type> progtypes;
	
private:
    ObjectPrototype(const ObjectPrototype&);
    ObjectPrototype& operator=(const ObjectPrototype&);
};
