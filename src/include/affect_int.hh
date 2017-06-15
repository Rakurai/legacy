#ifndef _AFFECT_INT_H
#define _AFFECT_INT_H

// internal functions for affect handling, prefer not to expose these to regular game code
// and use type-specific accessors

// affect.c
void          affect_add_to_list          args(( Affect **list_head, Affect *paf ));
void          affect_remove_from_list     args(( Affect **list_head, Affect *paf ));
void          affect_copy_to_list         args(( Affect **list_head, const Affect *paf ));
void          affect_dedup_in_list        args(( Affect **list_head, Affect *paf ));
void          affect_clear_list           args(( Affect **list_head ));
void          affect_swap                 args(( Affect *a, Affect *b ));
void          affect_update               args(( Affect *paf, const Affect *template ));
Affect * affect_find_in_list         args(( Affect *list_head, int sn ));

void          affect_modify_obj           args(( Object *obj, const Affect *paf, bool fAdd ));
void          affect_modify_char          args(( Character *ch, const Affect *paf, bool fAdd ));
void          affect_modify_room          args(( RoomPrototype *obj, const Affect *paf, bool fAdd ));

#endif // _AFFECT_INT_H
