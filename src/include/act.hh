#pragma once

#include "constants.hh"
#include <cstddef> // nullptr_t

class String;
class Actable;
class Character;
class Object;
class Room;

// guts pushed to implementation so we only need forward class declarations here

void act_parse(const String& format, Character *ch,
                const Character *vch, const Character *vch2,
                const String *str1, const String *str2,
                const Object *obj1, const Object *obj2,
                int type, int min_pos, bool censor);

// 2 Actables
void act(const String& format, Character *ch, const Actable* arg1, const Actable* arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr);
void act(const String& format, Character *ch, const Actable* arg1, const Actable& arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr);
void act(const String& format, Character *ch, const Actable& arg1, const Actable* arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr);
void act(const String& format, Character *ch, const Actable& arg1, const Actable& arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr);
// implementations with 1 Actable*
void act(const String& format, Character *ch, const Actable* arg1, const char *arg2,    int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr);
void act(const String& format, Character *ch, const Actable* arg1, std::nullptr_t arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr);
void act(const String& format, Character *ch, const char *arg1,    const Actable* arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr);
void act(const String& format, Character *ch, std::nullptr_t arg1, const Actable* arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr);
// implementations with 1 Actable&
void act(const String& format, Character *ch, const Actable& arg1, const char *arg2,    int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr);
void act(const String& format, Character *ch, const Actable& arg1, std::nullptr_t arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr);
void act(const String& format, Character *ch, const char *arg1,    const Actable& arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr);
void act(const String& format, Character *ch, std::nullptr_t arg1, const Actable& arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr);
// implementations with no Actable
void act(const String& format, Character *ch, const char *arg1,    const char *arg2,    int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr);
void act(const String& format, Character *ch, const char *arg1,    std::nullptr_t arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr);
void act(const String& format, Character *ch, std::nullptr_t arg1, const char *arg2,    int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr);
void act(const String& format, Character *ch, std::nullptr_t arg1, std::nullptr_t arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr);
