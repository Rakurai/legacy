#ifndef _AFFECT_H
#define _AFFECT_H

// function pointer definitions for callback functions, for safe list iteration

typedef int (*affect_fn)(AFFECT_DATA *node, void *data);
typedef void (*affect_owner_modifier_fn)(void *owner, const AFFECT_DATA *paf, bool fAdd);

typedef struct affect_fn_params {
	void *owner;
//	affect_fn fn;
	affect_owner_modifier_fn modifier;
	void *data;
} affect_fn_params;

// comparators return negative if lhs < rhs (i.e. return lhs.field - rhs.field)
typedef int (*affect_comparator)(const AFFECT_DATA *lhs, const AFFECT_DATA *rhs);

// comparators
int           affect_comparator_mark      args(( const AFFECT_DATA *lhs, const AFFECT_DATA *rhs ));
int           affect_comparator_duration  args(( const AFFECT_DATA *lhs, const AFFECT_DATA *rhs ));
int           affect_comparator_type      args(( const AFFECT_DATA *lhs, const AFFECT_DATA *rhs ));

// callbacks (reusable ones, it's ok to create one within a compilation unit)
int           affect_fn_fade_spell        args(( AFFECT_DATA *node, void *data ));

// single affect operations
void          affect_update               args(( AFFECT_DATA *paf, const AFFECT_DATA *template ));
void          affect_swap                 args(( AFFECT_DATA *a, AFFECT_DATA *b ));
unsigned long affect_checksum             args(( const AFFECT_DATA *paf ));
bool          affect_parse_prototype      args(( char letter, AFFECT_DATA *paf, unsigned int *bitvector ));
char *        affect_print_cache          args(( CHAR_DATA *ch ));
void          update_affect_cache         args(( CHAR_DATA *ch, sh_int sn, bool fAdd ));

// outward facing interface, all calls to obj/char/room affects should be through
// these procedures.  attempt to force some accessor safety on this crap -- Montrey

// public list accessors defined in affect_<entity>.c

// flags

bool                affect_flag_on_char              args(( CHAR_DATA *ch, unsigned int flag ));
void                affect_flag_add_to_char          args(( CHAR_DATA *ch, unsigned int flag ));
void                affect_flag_remove_from_char     args(( CHAR_DATA *ch, unsigned int flag ));
unsigned int        affect_flag_get_char             args(( CHAR_DATA *ch ));
void                affect_flag_clear_char           args(( CHAR_DATA *ch ));

// searching
bool                affect_enchanted_obj             args(( OBJ_DATA *obj ));
const AFFECT_DATA * affect_list_obj                  args(( OBJ_DATA *obj ));
const AFFECT_DATA * affect_list_char                 args(( CHAR_DATA *ch ));
const AFFECT_DATA * affect_list_room                 args(( ROOM_INDEX_DATA *room ));
const AFFECT_DATA * affect_find_in_obj               args(( OBJ_DATA *obj, int sn ));
const AFFECT_DATA * affect_find_in_char              args(( CHAR_DATA *ch, int sn ));
const AFFECT_DATA * affect_find_in_room              args(( ROOM_INDEX_DATA *room, int sn ));

// adding
void                affect_copy_to_obj               args(( OBJ_DATA *obj, const AFFECT_DATA *paf ));
void                affect_copy_to_char              args(( CHAR_DATA *ch, const AFFECT_DATA *paf ));
void                affect_copy_to_room              args(( ROOM_INDEX_DATA *room, const AFFECT_DATA *paf ));
void                affect_join_to_obj               args(( OBJ_DATA *obj, AFFECT_DATA *paf ));
void                affect_join_to_char              args(( CHAR_DATA *ch, AFFECT_DATA *paf ));
void                affect_join_to_room              args(( ROOM_INDEX_DATA *room, AFFECT_DATA *paf ));
void                affect_add_perm_to_char          args(( CHAR_DATA *ch, int sn ));
void                affect_copy_flags_to_char        args(( CHAR_DATA *ch, char letter, unsigned int bitvector ));

// removing
void                affect_remove_from_obj           args(( OBJ_DATA *obj, AFFECT_DATA *paf ));
void                affect_remove_from_char          args(( CHAR_DATA *ch, AFFECT_DATA *paf ));
void                affect_remove_from_room          args(( ROOM_INDEX_DATA *room, AFFECT_DATA *paf ));
void                affect_remove_matching_from_obj  args(( OBJ_DATA *obj, affect_comparator comp, const AFFECT_DATA *pattern ));
void                affect_remove_matching_from_char args(( CHAR_DATA *ch, affect_comparator comp, const AFFECT_DATA *pattern ));
void                affect_remove_matching_from_room args(( ROOM_INDEX_DATA *room, affect_comparator comp, const AFFECT_DATA *pattern ));
void                affect_remove_marked_from_obj    args(( OBJ_DATA *obj ));
void                affect_remove_marked_from_char   args(( CHAR_DATA *ch ));
void                affect_remove_marked_from_room   args(( ROOM_INDEX_DATA *room ));
void	            affect_remove_sn_from_obj        args(( OBJ_DATA *obj, int sn ));
void	            affect_remove_sn_from_char       args(( CHAR_DATA *ch, int sn ));
void	            affect_remove_sn_from_room       args(( ROOM_INDEX_DATA *room, int sn ));
void                affect_remove_all_from_obj       args(( OBJ_DATA *obj ));
void                affect_remove_all_from_char      args(( CHAR_DATA *ch ));
void                affect_remove_all_from_room      args(( ROOM_INDEX_DATA *room ));

// modifying
void                affect_iterate_over_obj          args(( OBJ_DATA *obj, affect_fn fn, void *data ));
void                affect_iterate_over_char         args(( CHAR_DATA *ch, affect_fn fn, void *data ));
void                affect_iterate_over_room         args(( ROOM_INDEX_DATA *room, affect_fn fn, void *data ));
void                affect_sort_obj                  args(( OBJ_DATA *ch, affect_comparator comp ));
void                affect_sort_char                 args(( CHAR_DATA *ch, affect_comparator comp ));
void                affect_sort_room                 args(( ROOM_INDEX_DATA *ch, affect_comparator comp ));

void                remort_affect_modify_char        args(( CHAR_DATA *ch, int where, unsigned int bitvector, bool fAdd ));

#endif // _AFFECT_H
