#pragma once

#include <map>

namespace event {

// forward declarations
class Subscriber;

enum Type {
	test,
	character_to_room,
	character_give_obj,
	character_kill_other,

	object_to_char,

	skill_quest_obj_to_mob,
	skill_quest_found_obj,
	skill_quest_found_mob,
};

typedef std::map<const char *, void *> Args;

void fire(Type type, Args args);
void subscribe(Type type, Subscriber *s);
void unsubscribe(Type type, Subscriber *s);

} // namespace event

