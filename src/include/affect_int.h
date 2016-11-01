#ifndef _AFFECT_INT_H
#define _AFFECT_INT_H

// internal functions for affect handling, prefer not to expose these to regular game code
// and use type-specific accessors

// affect.c
void          affect_add_to_list          args(( AFFECT_DATA **list_head, AFFECT_DATA *paf ));
void          affect_remove_from_list     args(( AFFECT_DATA **list_head, AFFECT_DATA *paf ));
void          affect_copy_to_list         args(( AFFECT_DATA **list_head, const AFFECT_DATA *paf ));
void          affect_dedup_in_list        args(( AFFECT_DATA **list_head, AFFECT_DATA *paf ));
void          affect_clear_list           args(( AFFECT_DATA **list_head ));
void          affect_swap                 args(( AFFECT_DATA *a, AFFECT_DATA *b ));
void          affect_update               args(( AFFECT_DATA *paf, const AFFECT_DATA *template ));
AFFECT_DATA * affect_find_in_list         args(( AFFECT_DATA *list_head, int sn ));

void          affect_modify_obj           args(( OBJ_DATA *obj, const AFFECT_DATA *paf, bool fAdd ));
void          affect_modify_char          args(( CHAR_DATA *ch, const AFFECT_DATA *paf, bool fAdd ));
void          affect_modify_room          args(( ROOM_INDEX_DATA *obj, const AFFECT_DATA *paf, bool fAdd ));

#endif // _AFFECT_INT_H
