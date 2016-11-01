#ifndef _AFFECT_INT_H
#define _AFFECT_INT_H

// internal list operations on affects, should not be exposed outside of affect_* compilation units

void                affect_add_to_list                args(( AFFECT_DATA **list_head, AFFECT_DATA *paf ));
void                affect_remove_from_list           args(( AFFECT_DATA **list_head, AFFECT_DATA *paf ));
void                affect_copy_to_list               args(( AFFECT_DATA **list_head, const AFFECT_DATA *paf ));
void                affect_dedup_in_list              args(( AFFECT_DATA **list_head, AFFECT_DATA *paf, affect_fn_params *params ));
void                affect_clear_list                 args(( AFFECT_DATA **list_head ));
const AFFECT_DATA * affect_find_in_list               args(( AFFECT_DATA **list_head, int sn ));
void                affect_remove_matching_from_list  args(( AFFECT_DATA **list_head, affect_comparator comp, const AFFECT_DATA *pattern, affect_fn_params *params ));
void                affect_iterate_over_list          args(( AFFECT_DATA **list_head, affect_fn fn, affect_fn_params *params ));
unsigned long       affect_checksum_list              args(( AFFECT_DATA **list_head ));
void                affect_sort_list                  args(( AFFECT_DATA **list_head, affect_comparator comp ));

//void          affect_modify_obj           args(( void *owner, const AFFECT_DATA *paf, bool fAdd ));
//void          affect_modify_char          args(( void *owner, const AFFECT_DATA *paf, bool fAdd ));
//void          affect_modify_room          args(( void *owner, const AFFECT_DATA *paf, bool fAdd ));

#endif // _AFFECT_INT_H
