#pragma once

class Character;
class Object;

// dispel.c
bool    saves_spell(int level, Character *victim, int dam_type);
bool    check_dispel_char(int dis_level, Character *victim, affect::type type, bool save);
bool    check_dispel_obj(int dis_level, Object *obj, affect::type type, bool save);
bool    undo_spell(int dis_level, Character *victim, affect::type type, bool save);
bool    dispel_char(Character *victim, int level, bool cancellation);
bool    level_save(int dis_level, int save_level);
