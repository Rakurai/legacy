#pragma once

#include "event.hh"

namespace event {

class EventDispatcher {
public:
	virtual ~EventDispatcher() {}

private:
	EventDispatcher(Type t, Args& a) :
		type(t), args(a) {}

	EventDispatcher();
	EventDispatcher(const EventDispatcher&);
	EventDispatcher& operator=(const EventDispatcher&);

	void dispatch(); // send event to subscribers

	Type type;
	Args& args;

	static std::multimap<Type, Subscriber *> subscribers;

	friend void fire(Type, Args);
	friend void subscribe(Type, Subscriber *);
	friend void unsubscribe(Type, Subscriber *);
};

} // namespace event
