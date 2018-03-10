#pragma once

#include <map>

#include "Type.hh"

//#include "declare.hh"
//#include "merc.hh"
#include "Pooled.hh"
#include "Flags.hh"
#include "String.hh"

class Character;
class Object;
class Room;

namespace affect {

/*
 * An affect.
 */
class Affect :
public Pooled<Affect>
{
public:
    // note: the grouping of these fields is important for computing checksums.
    // if any of these change (especially these first 4) the checksum function will need updating.
    Affect *       next = nullptr;
    Affect *       prev = nullptr; // now a doubly liked list -- Montrey
    bool                mark = false; // mark for deletion from list, other uses

    // fields included in checksum
    int              where = 0;
    ::affect::type                type = ::affect::type::none;
    int              level = 0;
    int              duration = 0;
    int              location = 0;
    int              modifier = 0;
    int                 _bitvector = 0; // only for weapon flags now
    int              evolution = 0;
    bool                permanent = false;

    const Flags bitvector() const { return Flags(_bitvector); }
    void bitvector(const Flags& f) { _bitvector = f.to_ulong(); }

} __attribute__((packed, aligned(1))); // no alignment padding, for checksums

/* where definitions */
#define TO_AFFECTS      0
#define TO_OBJECT       1 // obj->extra_flags
#define TO_DEFENSE      2
#define TO_WEAPON       5
/* new definitions for room affects -- Montrey */
#define TO_ROOMFLAGS	6
#define TO_HPREGEN	7
#define TO_MPREGEN	8

// these aren't used by affects, just to communicate what kind of defense for printing things
#define TO_ABSORB  20
#define TO_IMMUNE  21
#define TO_RESIST  22
#define TO_VULN    23

struct table_entry
{
    String        name;                   /* Name of skill                */
    String        msg_off;                /* Wear off message             */
    String        msg_room;              // wear off message to room
    String        msg_obj;                /* Wear off message for obects  */
};

extern  const   std::map<::affect::type, const table_entry>     affect_table;
const table_entry& lookup(::affect::type type);
const ::affect::type lookup(const String& name);

// function pointer definitions for callback functions, for safe list iteration

typedef int (*affect_fn)(Affect *node, void *data);
typedef void (*owner_modifier_fn)(void *owner, const Affect *paf, bool fAdd);

typedef struct fn_params {
	void *owner;
//	affect_fn fn;
	owner_modifier_fn modifier;
	void *data;
} affect_fn_params;

// handy little container to pass an affect enum value as a data parameter by pointer
struct fn_data_container_type {
    ::affect::type type;
};

// comparators return negative if lhs < rhs (i.e. return lhs.field - rhs.field)
typedef int (*comparator)(const Affect *lhs, const Affect *rhs);

// comparators
int           comparator_mark      ( const Affect *lhs, const Affect *rhs );
int           comparator_duration  ( const Affect *lhs, const Affect *rhs );
int           comparator_type      ( const Affect *lhs, const Affect *rhs );
int           comparator_permanent ( const Affect *lhs, const Affect *rhs );

// callbacks (reusable ones, it's ok to create one within a compilation unit)
int           fn_fade_spell        ( Affect *node, void *data );
int           fn_set_level         ( Affect *node, void *data );

// single affect operations
void          update               ( Affect *paf, const Affect *aff_template );
void          swap                 ( Affect *a, Affect *b );
unsigned long checksum             ( const Affect *paf );
bool          parse_flags          ( char letter, Affect *paf, Flags& bitvector );
String        print_cache          ( Character *ch );
bool          in_cache             ( const Character *ch, ::affect::type type );
void          update_cache         ( Character *ch, ::affect::type type, bool fAdd );
void          free_cache           ( Character *ch );

// outward facing interface, all calls to obj/char/room affects should be through
// these procedures.  attempt to force some accessor safety on this crap -- Montrey

// public list accessors defined in affect_<entity>.c

// flags

// searching
bool                enchanted_obj             ( Object *obj );
const Affect * list_obj                       ( Object *obj );
const Affect * list_char                      ( Character *ch );
const Affect * list_room                      ( Room *room );
bool                exists_on_obj             ( Object *obj, ::affect::type type );
bool                exists_on_char            ( const Character *ch, ::affect::type type );
bool                exists_on_room            ( Room *room, ::affect::type type );
const Affect * find_on_obj                    ( Object *obj, ::affect::type type );
const Affect * find_on_char                   ( Character *ch, ::affect::type type );
const Affect * find_on_room                   ( Room *room, ::affect::type type );

// adding
void                copy_to_obj               ( Object *obj, const Affect *paf );
void                copy_to_char              ( Character *ch, const Affect *paf );
void                copy_to_room              ( Room *room, const Affect *paf );
void                join_to_obj               ( Object *obj, Affect *paf );
void                join_to_char              ( Character *ch, Affect *paf );
void                join_to_room              ( Room *room, Affect *paf );
void                add_perm_to_char          ( Character *ch, ::affect::type type );
void                copy_flags_to_char        ( Character *ch, char letter, Flags flags, bool permanent );
void                add_type_to_char          ( Character *ch, ::affect::type type, int level, int duration, int evolution, bool permanent );
void                add_racial_to_char        ( Character *ch );

// removing
void                remove_from_obj           ( Object *obj, Affect *paf );
void                remove_from_char          ( Character *ch, Affect *paf );
void                remove_from_room          ( Room *room, Affect *paf );
void                remove_matching_from_obj  ( Object *obj, comparator comp, const Affect *pattern );
void                remove_matching_from_char ( Character *ch, comparator comp, const Affect *pattern );
void                remove_matching_from_room ( Room *room, comparator comp, const Affect *pattern );
void                remove_marked_from_obj    ( Object *obj );
void                remove_marked_from_char   ( Character *ch );
void                remove_marked_from_room   ( Room *room );
void	            remove_type_from_obj      ( Object *obj, ::affect::type type );
void	            remove_type_from_char     ( Character *ch, ::affect::type type );
void	            remove_type_from_room     ( Room *room, ::affect::type type );
void                remove_all_from_obj       ( Object *obj, bool permanent );
void                remove_all_from_char      ( Character *ch, bool permanent );
void                remove_all_from_room      ( Room *room, bool permanent );

// modifying
void                modify_obj                ( Object *obj, const Affect *paf, bool fAdd );
void                modify_char               ( Character *ch, const Affect *paf, bool fAdd );
void                modify_room               ( Room *obj, const Affect *paf, bool fAdd );
void                iterate_over_obj          ( Object *obj, affect_fn fn, void *data );
void                iterate_over_char         ( Character *ch, affect_fn fn, void *data );
void                iterate_over_room         ( Room *room, affect_fn fn, void *data );
void                sort_obj                  ( Object *ch, comparator comp );
void                sort_char                 ( Character *ch, comparator comp );
void                sort_room                 ( Room *ch, comparator comp );

void                remort_affect_modify_char ( Character *ch, int where, Flags bitvector, bool fAdd );

} // namespace affect

// weird things out of namespace, fix up when possible
void show_affect_to_char(const affect::Affect *paf, Character *ch);
void spread_plague(Room *room, const affect::Affect *plague, int chance );
