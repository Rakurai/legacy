#pragma once

#include <map>
#include "String.hh"

namespace event {

// forward declarations
class Subscriber;

enum Type {
	test,
	character_to_room,
	character_give_obj,
	character_kill_other,
};

class Event {
public:
	Event(Type t, std::map<const char *, void *> a) :
		type(t), args(a) {}
	virtual ~Event() {}

	void fire();

	Type type;
	std::map<const char *, void *> args;

private:
	Event();
	Event(const Event&);
	Event& operator=(const Event&);

	static std::multimap<Type, Subscriber *> subscribers;

	friend void subscribe(Type, Subscriber *);
	friend void unsubscribe(Type, Subscriber *);
};

inline void subscribe(Type type, Subscriber *s) {
	Event::subscribers.emplace(type, s);
}

inline void unsubscribe(Type type, Subscriber *s) {
	auto range = Event::subscribers.equal_range(type);

	for (auto it = range.first; it != range.second; ++it)
		if (it->second == s)
			Event::subscribers.erase(it);
}

} // namespace event

