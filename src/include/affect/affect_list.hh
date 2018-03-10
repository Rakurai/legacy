#pragma once

#include "Type.hh"
#include "Affect.hh"

namespace affect {

// internal list operations on affects, should not be exposed outside of affect_* compilation units

void                add_to_list                ( Affect **list_head, Affect *paf );
void                remove_from_list           ( Affect **list_head, Affect *paf );
void                copy_to_list               ( Affect **list_head, const Affect *paf );
void                dedup_in_list              ( Affect **list_head, Affect *paf, fn_params *params );
void                clear_list                 ( Affect **list_head );
const Affect *      find_in_list               ( Affect **list_head, ::affect::type type );
void                remove_matching_from_list  ( Affect **list_head, comparator comp, const Affect *pattern, affect_fn_params *params );
void                iterate_over_list          ( Affect **list_head, affect_fn fn, fn_params *params );
unsigned long       checksum_list              ( Affect **list_head );
void                sort_list                  ( Affect **list_head, comparator comp );

//void          affect_modify_obj           ( void *owner, const Affect *paf, bool fAdd );
//void          affect_modify_char          ( void *owner, const Affect *paf, bool fAdd );
//void          affect_modify_room          ( void *owner, const Affect *paf, bool fAdd );

} // namespace affect
