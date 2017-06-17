#include "event/event.hh"
#include "event/EventDispatcher.hh"
#include "event/Subscriber.hh"

namespace event {

void fire(Type type, Args args) {
	EventDispatcher(type, args).dispatch();
}

void subscribe(Type type, Subscriber *s) {
	EventDispatcher::subscribers.emplace(type, s);
}

void unsubscribe(Type type, Subscriber *s) {
	auto range = EventDispatcher::subscribers.equal_range(type);

	for (auto it = range.first; it != range.second; ++it)
		if (it->second == s)
			EventDispatcher::subscribers.erase(it);
}

} // namespace event
