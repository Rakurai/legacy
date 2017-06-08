#pragma once

#include "declare.h"
#include "Actable.hpp"
#include "String.hpp"

class ObjectPrototype;
class RoomPrototype;
class Reset;
class ExtraDescr;
class Affect;

/*
 * One object.
 */
class Object: public Actable
{
public:
    Object() {};
    virtual ~Object();

	ObjectPrototype *	pIndexData = NULL;
	Reset *		reset = NULL;		/* let's make it keep track of what reset it */
	Object *            next = NULL;
	Object *            next_content = NULL;
	Object *            in_obj = NULL;
	Object *            on = NULL;
	RoomPrototype *	in_room = NULL;
	Character *		carried_by = NULL;
	Character *		in_locker = NULL;
	Character *		in_strongbox = NULL;
	Object *		contains = NULL;
    long            donated = 0; // timestamp when an obj was donated, for pit cleanup

	String 	        name;
	String 	        short_descr;
	String 	        description;
	String 	        material;
	String 	        owner;
	ExtraDescr *	extra_descr = NULL;
	sh_int			item_type = 0;
	unsigned long	extra_flags = 0;
	unsigned long   wear_flags = 0;
	int			value[5] = {0};
	sh_int			weight = 0;
	int			cost = 0;
	sh_int			level = 0;
	sh_int			condition = 0;
	sh_int			wear_loc = 0;
	sh_int			timer = 0;
	sh_int			clean_timer = 0;		/* Montrey */
	bool			valid = FALSE;

    unsigned long   extra_flag_cache = 0;
    unsigned long   weapon_flag_cache = 0;

    /* ugly way to do this: rather than everywhere cycling through the affects given by
       the object's index data separately from the affects given by inset gems, we
       compile a list of affects whenever one of those changes (rare event). -- Montrey */
 //   Affect *   perm_affected; // initially identical to the index, can be changed by enchants and addapply
    Affect *   affected = NULL; // the compiled list, never shown in 'stat' or 'lore', so it can be deduped.
    bool            affects_modified = FALSE; // set TRUE if an affect changes, so they can be recompiled in the update loop

    char            num_settings = 0;
    Object *      gems = NULL; // gems in settings
    Affect *   gem_affected = NULL;

    virtual std::string identifier() const { return this->name; }

private:
	Object(const String&);
	String& operator=(const String&);
};

/* object recycling */
Object	*new_obj args( (void) );
void	free_obj args( (Object *obj) );

void    extract_obj     args(( Object *obj ) );
