#include "event/event.hh"
#include "event/EventDispatcher.hh"
#include "event/Subscriber.hh"

namespace event {

// static member
std::multimap<Type, Subscriber *> EventDispatcher::subscribers;

void EventDispatcher::
dispatch() {
	auto range = subscribers.equal_range(type);

	for (auto i = range.first; i != range.second; ++i)
		i->second->notify(this->type, this->args);
}

}
