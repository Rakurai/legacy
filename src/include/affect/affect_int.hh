#pragma once

#include "Type.hh"

namespace affect {

class Affect;

// internal functions for affect handling, prefer not to expose these to regular game code
// and use type-specific accessors

// affect.c
void          affect_add_to_list          ( Affect **list_head, Affect *paf );
void          affect_remove_from_list     ( Affect **list_head, Affect *paf );
void          affect_copy_to_list         ( Affect **list_head, const Affect *paf );
void          affect_dedup_in_list        ( Affect **list_head, Affect *paf );
void          affect_clear_list           ( Affect **list_head );
void          affect_swap                 ( Affect *a, Affect *b );
void          affect_update               ( Affect *paf, const Affect *aff_template );
Affect * affect_find_in_list         ( Affect *list_head, ::affect::type type );

} // namespace affect
