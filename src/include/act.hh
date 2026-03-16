#pragma once

#include "constants.hh"
#include <cstddef> // nullptr_t
#include "String.hh"

class String;
class Actable;
class Character;
class Object;
class Room;

// guts pushed to implementation so we only need forward class declarations here

void act_parse(
	String format,
	Character *actor,
	Room *room,
	const Character *vch1, const Character *vch2,
	const String *str1, const String *str2,
	const Object *obj1, const Object *obj2,
	int type,
	int min_pos,
	bool censor
);

// 2 Actables
// \fn void act(const String& format, Character *actor, const Actable* arg1, const Actable* arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr)
inline void act(const String& format, Character *actor, const Actable* arg1, const Actable* arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr) {
	act_parse(
		format, actor, room,
		(const Character *)arg2,
		(const Character *)arg1,
		(const String *)arg1,
		(const String *)arg2,
		(const Object *)arg1,
		(const Object *)arg2,
		type, min_pos, censor
	);
}

// pointer and reference
// \overload void act(const String& format, Character *actor, const Actable* arg1, const Actable& arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr)
inline void act(const String& format, Character *actor, const Actable* arg1, const Actable& arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr) {
	act(format, actor, arg1, &arg2, type, min_pos, censor, room);
}

// reference and pointer
// \overload void act(const String& format, Character *actor, const Actable& arg1, const Actable* arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr)
inline void act(const String& format, Character *actor, const Actable& arg1, const Actable* arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr) {
	act(format, actor, &arg1, arg2, type, min_pos, censor, room);
}

// reference and reference
// \overload void act(const String& format, Character *actor, const Actable& arg1, const Actable& arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr)
inline void act(const String& format, Character *actor, const Actable& arg1, const Actable& arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr) {
	act(format, actor, &arg1, &arg2, type, min_pos, censor, room);
}

/*
 * implementations with 1 Actable*
 */

// arg and char*
// \overload void act(const String& format, Character *actor, const Actable* arg1, const char *arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr)
inline void act(const String& format, Character *actor, const Actable* arg1, const char *arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr) {
	act(format, actor, arg1, String(arg2), type, min_pos, censor, room);
}

// arg and null
// \overload void act(const String& format, Character *actor, const Actable* arg1, std::nullptr_t arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr)
inline void act(const String& format, Character *actor, const Actable* arg1, std::nullptr_t arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr) {
	act(format, actor, arg1, String(), type, min_pos, censor, room);
}

// char* and arg
// \overload void act(const String& format, Character *actor, const char *arg1, const Actable* arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr)
inline void act(const String& format, Character *actor, const char *arg1, const Actable* arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr) {
	act(format, actor, String(arg1), arg2, type, min_pos, censor, room);
}

// null and arg
// \overload void act(const String& format, Character *actor, std::nullptr_t arg1, const Actable* arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr)
inline void act(const String& format, Character *actor, std::nullptr_t arg1, const Actable* arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr) {
	act(format, actor, String(), arg2, type, min_pos, censor, room);
}

/*
 * implementations with 1 Actable&
 */

// arg and char*
// \overload void act(const String& format, Character *actor, const Actable& arg1, const char *arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr)
inline void act(const String& format, Character *actor, const Actable& arg1, const char *arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr) {
	act(format, actor, &arg1, String(arg2), type, min_pos, censor, room);
}

// arg and null
// \overload void act(const String& format, Character *actor, const Actable& arg1, std::nullptr_t arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr)
inline void act(const String& format, Character *actor, const Actable& arg1, std::nullptr_t arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr) {
	act(format, actor, &arg1, String(), type, min_pos, censor, room);
}

// char* and arg
// \overload void act(const String& format, Character *actor, const char *arg1, const Actable& arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr)
inline void act(const String& format, Character *actor, const char *arg1, const Actable& arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr) {
	act(format, actor, String(arg1), &arg2, type, min_pos, censor, room);
}

// null and arg
// \overload void act(const String& format, Character *actor, std::nullptr_t arg1, const Actable& arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr)
inline void act(const String& format, Character *actor, std::nullptr_t arg1, const Actable& arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr) {
	act(format, actor, String(), &arg2, type, min_pos, censor, room);
}

/*
 * implementations with no Actable
 */

// char* and char*
// \overload void act(const String& format, Character *actor, const char *arg1, const char *arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr)
inline void act(const String& format, Character *actor, const char *arg1, const char *arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr) {
	act(format, actor, String(arg1), String(arg2), type, min_pos, censor, room);
}

// char* and null
// \overload void act(const String& format, Character *actor, const char *arg1, std::nullptr_t arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr)
inline void act(const String& format, Character *actor, const char *arg1, std::nullptr_t arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr) {
	act(format, actor, String(arg1), String(), type, min_pos, censor, room);
}

// null and char*
// \overload void act(const String& format, Character *actor, std::nullptr_t arg1, const char *arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr)
inline void act(const String& format, Character *actor, std::nullptr_t arg1, const char *arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr) {
	act(format, actor, String(), String(arg2), type, min_pos, censor, room);
}

// null and null
// \overload void act(const String& format, Character *actor, std::nullptr_t arg1, std::nullptr_t arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr)
inline void act(const String& format, Character *actor, std::nullptr_t arg1, std::nullptr_t arg2, int type, int min_pos = POS_RESTING, bool censor = false, Room *room = nullptr) {
	act(format, actor, String(), String(), type, min_pos, censor, room);
}
