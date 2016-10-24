#ifndef _AFFECT_H
#define _AFFECT_H

// outward facing interface, all calls to obj/char/room affects should be through
// these procedures.  attempt to force some accessor safety on this crap -- Montrey

bool          is_affected                 args(( CHAR_DATA *ch, int sn ));
char *        affect_print_cache          args(( CHAR_DATA *ch ));
void          affect_add_perm_to_char     args(( CHAR_DATA *ch, int sn ));
bool          affect_parse_prototype      args(( char letter, AFFECT_DATA *af, unsigned int *bitvector ));

// affect_<entity>.c
AFFECT_DATA * affect_find_in_obj          args(( OBJ_DATA *obj, int sn ));
AFFECT_DATA * affect_find_in_char         args(( CHAR_DATA *ch, int sn ));
AFFECT_DATA * affect_find_in_room         args(( ROOM_INDEX_DATA *room, int sn ));

void          affect_copy_to_obj          args(( OBJ_DATA *obj, const AFFECT_DATA *paf ));
void          affect_copy_to_char         args(( CHAR_DATA *ch, const AFFECT_DATA *paf ));
void          affect_copy_to_room         args(( ROOM_INDEX_DATA *room, const AFFECT_DATA *paf ));

void          affect_join_to_obj          args(( OBJ_DATA *obj, AFFECT_DATA *paf ));
void          affect_join_to_char         args(( CHAR_DATA *ch, AFFECT_DATA *paf ));
void          affect_join_to_room         args(( ROOM_INDEX_DATA *room, AFFECT_DATA *paf ));

void          affect_remove_from_obj      args(( OBJ_DATA *obj, AFFECT_DATA *paf ));
void          affect_remove_from_char     args(( CHAR_DATA *ch, AFFECT_DATA *paf ));
void          affect_remove_from_room     args(( ROOM_INDEX_DATA *room, AFFECT_DATA *paf ));

void	      affect_remove_sn_from_obj   args(( OBJ_DATA *obj, int sn ));
void	      affect_remove_sn_from_char  args(( CHAR_DATA *ch, int sn ));
void	      affect_remove_sn_from_room  args(( ROOM_INDEX_DATA *room, int sn ));

void          affect_remove_all_from_obj       args(( OBJ_DATA *obj ));
void          affect_remove_all_from_char      args(( CHAR_DATA *ch ));
void          affect_remove_all_from_room      args(( ROOM_INDEX_DATA *room ));

#endif // _AFFECT_H
