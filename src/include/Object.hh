#pragma once

#include "Actable.hh"
#include "String.hh"
#include "Flags.hh"
#include "ObjectValue.hh"
#include "Pooled.hh"

class ObjectPrototype;
class Reset;
class Room;
class Character;
class ExtraDescr;
namespace affect { class Affect; }

/*
 * One object.
 */
class Object :
public Pooled<Object>,
public Actable
{
public:
    Object() {};
	Object(ObjectPrototype *p) : pIndexData(p) {}
    virtual ~Object();

	ObjectPrototype *	pIndexData = nullptr;
	const Reset *		reset = nullptr;		/* let's make it keep track of what reset it */
	Object *            next = nullptr;
	Object *            next_content = nullptr;
	Object *            in_obj = nullptr;
	Object *            on = nullptr;
	Room *	in_room = nullptr;
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
	int			item_type = 0;
	Flags           extra_flags;
	Flags           wear_flags;
	ObjectValue     value[5];
	int			weight = 0;
	int			cost = 0;
	int			level = 0;
	int			condition = 0;
	int			wear_loc = 0;
	int			timer = 0;
	int			clean_timer = 0;		/* Montrey */

    Flags           cached_extra_flags;

    /* ugly way to do this: rather than everywhere cycling through the affects given by
       the object's index data separately from the affects given by inset gems, we
       compile a list of affects whenever one of those changes (rare event). -- Montrey */
 //   Affect *   perm_affected; // initially identical to the index, can be changed by enchants and addapply
    affect::Affect *   affected = nullptr; // the compiled list, never shown in 'stat' or 'lore', so it can be deduped.
    bool            affects_modified = false; // set true if an affect changes, so they can be recompiled in the update loop

    char            num_settings = 0;
    Object *      gems = nullptr; // gems in settings
    affect::Affect *   gem_affected = nullptr;

    virtual const std::string identifier() const { return this->name; }

private:
	Object(const Object&);
	Object& operator=(const Object&);
};

/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)     ((obj)->wear_flags.has((part)))
#define IS_OBJ_STAT(obj, stat)  (((obj)->extra_flags + (obj)->cached_extra_flags).has((stat)))
#define WEIGHT_MULT(obj)        ((obj)->item_type == ITEM_CONTAINER ? \
		(obj)->value[4] : 100)

void    destroy_obj( Object *obj);
void    extract_obj( Object *obj );
void unique_item(Object *item);
