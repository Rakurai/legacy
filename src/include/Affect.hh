#pragma once

#include "declare.hh"
#include "Flags.hh"

/*
 * An affect.
 */
class Affect
{
public:
    // note: the grouping of these fields is important for computing checksums.
    // if any of these change (especially these first 4) the checksum function will need updating.
    Affect *       next = nullptr;
    Affect *       prev = nullptr; // now a doubly liked list -- Montrey
    bool                valid = FALSE;
    bool                mark = FALSE; // mark for deletion from list, other uses

    // fields included in checksum
    sh_int              where = 0;
    sh_int              type = 0;
    sh_int              level = 0;
    sh_int              duration = 0;
    sh_int              location = 0;
    sh_int              modifier = 0;
    int                 _bitvector = 0; // only for weapon flags now
    sh_int              evolution = 0;
    bool                permanent = FALSE;

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

// function pointer definitions for callback functions, for safe list iteration

typedef int (*affect_fn)(Affect *node, void *data);
typedef void (*affect_owner_modifier_fn)(void *owner, const Affect *paf, bool fAdd);

typedef struct affect_fn_params {
	void *owner;
//	affect_fn fn;
	affect_owner_modifier_fn modifier;
	void *data;
} affect_fn_params;

// comparators return negative if lhs < rhs (i.e. return lhs.field - rhs.field)
typedef int (*affect_comparator)(const Affect *lhs, const Affect *rhs);

// comparators
int           affect_comparator_mark      args(( const Affect *lhs, const Affect *rhs ));
int           affect_comparator_duration  args(( const Affect *lhs, const Affect *rhs ));
int           affect_comparator_type      args(( const Affect *lhs, const Affect *rhs ));
int           affect_comparator_permanent args(( const Affect *lhs, const Affect *rhs ));

// callbacks (reusable ones, it's ok to create one within a compilation unit)
int           affect_fn_fade_spell        args(( Affect *node, void *data ));

// single affect operations
void          affect_update               args(( Affect *paf, const Affect 
*aff_template ));
void          affect_swap                 args(( Affect *a, Affect *b ));
unsigned long affect_checksum             args(( const Affect *paf ));
bool          affect_parse_flags      args(( char letter, Affect *paf, Flags& bitvector ));
String        affect_print_cache          args(( Character *ch ));
bool          affect_in_cache             args(( const Character *ch, sh_int sn ));
void          update_affect_cache         args(( Character *ch, sh_int sn, bool fAdd ));
void          free_affect_cache           args(( Character *ch ));

// outward facing interface, all calls to obj/char/room affects should be through
// these procedures.  attempt to force some accessor safety on this crap -- Montrey

// public list accessors defined in affect_<entity>.c

// flags

// searching
bool                affect_enchanted_obj             args(( Object *obj ));
const Affect * affect_list_obj                  args(( Object *obj ));
const Affect * affect_list_char                 args(( Character *ch ));
const Affect * affect_list_room                 args(( RoomPrototype *room ));
bool                affect_exists_on_obj             args(( Object *obj, int sn ));
bool                affect_exists_on_char            args(( const Character *ch, int sn ));
bool                affect_exists_on_room            args(( RoomPrototype *room, int sn ));
const Affect * affect_find_on_obj               args(( Object *obj, int sn ));
const Affect * affect_find_on_char              args(( Character *ch, int sn ));
const Affect * affect_find_on_room              args(( RoomPrototype *room, int sn ));

// adding
void                affect_copy_to_obj               args(( Object *obj, const Affect *paf ));
void                affect_copy_to_char              args(( Character *ch, const Affect *paf ));
void                affect_copy_to_room              args(( RoomPrototype *room, const Affect *paf ));
void                affect_join_to_obj               args(( Object *obj, Affect *paf ));
void                affect_join_to_char              args(( Character *ch, Affect *paf ));
void                affect_join_to_room              args(( RoomPrototype *room, Affect *paf ));
void                affect_add_perm_to_char          args(( Character *ch, int sn ));
void                affect_copy_flags_to_char        args(( Character *ch, char letter, Flags flags, bool permanent ));
void                affect_add_sn_to_char            args(( Character *ch, sh_int sn, sh_int level, sh_int duration, sh_int evolution, bool permanent ));
void                affect_add_racial_to_char        args(( Character *ch ));

// removing
void                affect_remove_from_obj           args(( Object *obj, Affect *paf ));
void                affect_remove_from_char          args(( Character *ch, Affect *paf ));
void                affect_remove_from_room          args(( RoomPrototype *room, Affect *paf ));
void                affect_remove_matching_from_obj  args(( Object *obj, affect_comparator comp, const Affect *pattern ));
void                affect_remove_matching_from_char args(( Character *ch, affect_comparator comp, const Affect *pattern ));
void                affect_remove_matching_from_room args(( RoomPrototype *room, affect_comparator comp, const Affect *pattern ));
void                affect_remove_marked_from_obj    args(( Object *obj ));
void                affect_remove_marked_from_char   args(( Character *ch ));
void                affect_remove_marked_from_room   args(( RoomPrototype *room ));
void	            affect_remove_sn_from_obj        args(( Object *obj, int sn ));
void	            affect_remove_sn_from_char       args(( Character *ch, int sn ));
void	            affect_remove_sn_from_room       args(( RoomPrototype *room, int sn ));
void                affect_remove_all_from_obj       args(( Object *obj, bool permanent ));
void                affect_remove_all_from_char      args(( Character *ch, bool permanent ));
void                affect_remove_all_from_room      args(( RoomPrototype *room, bool permanent ));

// modifying
void                affect_iterate_over_obj          args(( Object *obj, affect_fn fn, void *data ));
void                affect_iterate_over_char         args(( Character *ch, affect_fn fn, void *data ));
void                affect_iterate_over_room         args(( RoomPrototype *room, affect_fn fn, void *data ));
void                affect_sort_obj                  args(( Object *ch, affect_comparator comp ));
void                affect_sort_char                 args(( Character *ch, affect_comparator comp ));
void                affect_sort_room                 args(( RoomPrototype *ch, affect_comparator comp ));

void                remort_affect_modify_char        args(( Character *ch, int where, Flags bitvector, bool fAdd ));

/* affect recycling */
Affect *new_affect args( (void) );
void    free_affect args( (Affect *af) );

