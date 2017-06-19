#pragma once

#include <map>

namespace event {

enum Type {
	test,
	character_to_room,
	character_give_obj,
	character_kill_other,
};

typedef std::map<const char *, void *> Args;

} // namespace event

