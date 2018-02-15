#pragma once

// dispel.c
bool    saves_spell       args(( int level, Character *victim, int dam_type ) );
bool    check_dispel_char args(( int dis_level, Character *victim, affect::Type type, bool save ));
bool    check_dispel_obj  args(( int dis_level, Object *obj, affect::Type type, bool save ));
bool    undo_spell        args(( int dis_level, Character *victim, affect::Type type, bool save ));
bool    dispel_char       args(( Character *victim, int level, bool cancellation ));
bool    level_save        args(( int dis_level, int save_level));
