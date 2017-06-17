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

typedef std::map<const char *, void *> EventArgs;

class Event {
public:
	virtual ~Event() {}

private:
	Event(Type t, EventArgs& a) :
		type(t), args(a) {}

	Event();
	Event(const Event&);
	Event& operator=(const Event&);

	void dispatch(); // send event to subscribers

	Type type;
	EventArgs& args;

	static std::multimap<Type, Subscriber *> subscribers;

	friend void fire(Type, EventArgs);
	friend void subscribe(Type, Subscriber *);
	friend void unsubscribe(Type, Subscriber *);
};

inline void fire(Type type, EventArgs args) {
	Event(type, args).dispatch();
}

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

