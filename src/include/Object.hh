#pragma once

#include "declare.hh"
#include "Actable.hh"
#include "String.hh"
#include "Flags.hh"
#include "ObjectValue.hh"

/*
 * One object.
 */
class Object: public Actable
{
public:
    Object() {};
    virtual ~Object();

	ObjectPrototype *	pIndexData = nullptr;
	const Reset *		reset = nullptr;		/* let's make it keep track of what reset it */
	Object *            next = nullptr;
	Object *            next_content = nullptr;
	Object *            in_obj = nullptr;
	Object *            on = nullptr;
	RoomPrototype *	in_room = nullptr;
	Character *		carried_by = nullptr;
	Character *		in_locker = nullptr;
	Character *		in_strongbox = nullptr;
	Object *		contains = nullptr;
    long            donated = 0; // timestamp when an obj was donated, for pit cleanup

	String 	        name;
	String 	        short_descr;
	String 	        description;
	String 	        material;
	String 	        owner;
	ExtraDescr *	extra_descr = nullptr;
	sh_int			item_type = 0;
	Flags           extra_flags;
	Flags           wear_flags;
	ObjectValue     value[5];
	sh_int			weight = 0;
	int			cost = 0;
	sh_int			level = 0;
	sh_int			condition = 0;
	sh_int			wear_loc = 0;
	sh_int			timer = 0;
	sh_int			clean_timer = 0;		/* Montrey */
	bool			valid = FALSE;

    Flags           cached_extra_flags;
    Flags           cached_weapon_flags;

    /* ugly way to do this: rather than everywhere cycling through the affects given by
       the object's index data separately from the affects given by inset gems, we
       compile a list of affects whenever one of those changes (rare event). -- Montrey */
 //   Affect *   perm_affected; // initially identical to the index, can be changed by enchants and addapply
    Affect *   affected = nullptr; // the compiled list, never shown in 'stat' or 'lore', so it can be deduped.
    bool            affects_modified = FALSE; // set TRUE if an affect changes, so they can be recompiled in the update loop

    char            num_settings = 0;
    Object *      gems = nullptr; // gems in settings
    Affect *   gem_affected = nullptr;

    virtual const std::string identifier() const { return this->name; }

private:
	Object(const String&);
	String& operator=(const String&);
};

/* object recycling */
Object	*new_obj args( (void) );
void	free_obj args( (Object *obj) );

void    extract_obj     args(( Object *obj ) );
void unique_item(Object *item);
