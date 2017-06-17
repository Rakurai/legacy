#include "event/Event.hh"
#include "event/Subscriber.hh"

namespace event {

// static member
std::multimap<Type, Subscriber *> Event::subscribers;

void Event::
fire() {
	auto range = subscribers.equal_range(type);

	for (auto i = range.first; i != range.second; ++i)
		i->second->notify(*this);
}

}
