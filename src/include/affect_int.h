#ifndef _AFFECT_INT_H
#define _AFFECT_INT_H

typedef int (*affect_fn)(AFFECT_DATA *node, void *data);
typedef void (*affect_owner_modifier_fn)(void *owner, const AFFECT_DATA *paf, bool fAdd);

typedef struct affect_fn_params {
	void *owner;
//	affect_fn fn;
	affect_owner_modifier_fn modifier;
	void *data;
} affect_fn_params;

//typedef int (*affect_fn_wrapper)(AFFECT_DATA *node, affect_fn_params *params);

// comparators return negative if lhs < rhs (i.e. return lhs.field - rhs.field)
typedef int (*affect_comparator)(const AFFECT_DATA *lhs, const AFFECT_DATA *rhs);

// internal functions for affect handling, prefer not to expose these to regular game code
// and use type-specific accessors

// affect.c

// single affect operations
void          affect_update               args(( AFFECT_DATA *paf, const AFFECT_DATA *template ));
void          affect_swap                 args(( AFFECT_DATA *a, AFFECT_DATA *b ));

// list operations
void          affect_add_to_list          args(( AFFECT_DATA **list_head, AFFECT_DATA *paf ));
void          affect_remove_from_list     args(( AFFECT_DATA **list_head, AFFECT_DATA *paf ));
void          affect_copy_to_list         args(( AFFECT_DATA **list_head, const AFFECT_DATA *paf ));
void          affect_dedup_in_list        args(( AFFECT_DATA **list_head, AFFECT_DATA *paf ));
void          affect_clear_list           args(( AFFECT_DATA **list_head ));
AFFECT_DATA * affect_find_in_list         args(( AFFECT_DATA **list_head, int sn ));
void          affect_remove_matching_from_list    args(( AFFECT_DATA **list_head, affect_comparator comp, const AFFECT_DATA *pattern, affect_fn_params *params ));
void          affect_iterate_over_list    args(( AFFECT_DATA **list_head, affect_fn fn, affect_fn_params *params ));
void          affect_sort_list            args(( AFFECT_DATA **list_head, affect_comparator comp ));

void          affect_modify_obj           args(( void *owner, const AFFECT_DATA *paf, bool fAdd ));
void          affect_modify_char          args(( void *owner, const AFFECT_DATA *paf, bool fAdd ));
void          affect_modify_room          args(( void *owner, const AFFECT_DATA *paf, bool fAdd ));

#endif // _AFFECT_INT_H
