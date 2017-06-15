#pragma once

#include "declare.hh"
#include "Affect.hh"

// internal list operations on affects, should not be exposed outside of affect_* compilation units

void                affect_add_to_list                args(( Affect **list_head, Affect *paf ));
void                affect_remove_from_list           args(( Affect **list_head, Affect *paf ));
void                affect_copy_to_list               args(( Affect **list_head, const Affect *paf ));
void                affect_dedup_in_list              args(( Affect **list_head, Affect *paf, affect_fn_params *params ));
void                affect_clear_list                 args(( Affect **list_head ));
const Affect * affect_find_in_list               args(( Affect **list_head, int sn ));
void                affect_remove_matching_from_list  args(( Affect **list_head, affect_comparator comp, const Affect *pattern, affect_fn_params *params ));
void                affect_iterate_over_list          args(( Affect **list_head, affect_fn fn, affect_fn_params *params ));
unsigned long       affect_checksum_list              args(( Affect **list_head ));
void                affect_sort_list                  args(( Affect **list_head, affect_comparator comp ));

//void          affect_modify_obj           args(( void *owner, const Affect *paf, bool fAdd ));
//void          affect_modify_char          args(( void *owner, const Affect *paf, bool fAdd ));
//void          affect_modify_room          args(( void *owner, const Affect *paf, bool fAdd ));
