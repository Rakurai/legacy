#include "event/Dispatcher.hh"

#include "event/event.hh"
#include "event/Handler.hh"

namespace event {

void Dispatcher::
fire(Type type, Args args) {
	auto range = subscribers.equal_range(type);
	auto it = range.first;

	// iterate carefully here - dispatching could result in the handler
	// being removed from the subscriber list.  The iterator will update
	// automatically, so have to check its value to see if we still need
	// to increment.
	while (it != range.second) {
		Handler *p = it->second; // keep a copy of the pointer
		p->notify(type, args);

		if (it->second == p) // only increment if not changed
			++it;
	}
}

void Dispatcher::
subscribe(Type type, Handler *s) {
	// in case a dispatch results in new subscribers, place them at the front
	// so we don't have nondeterministic effects (dispatching the same event
	// to the new subscribers)
	auto pos = subscribers.cbegin(); 
	subscribers.emplace_hint(pos, type, s);
}

void Dispatcher::
unsubscribe(Type type, Handler *s) {
	auto range = subscribers.equal_range(type);
	auto it = range.first;

	while (it != range.second) {
		if (it->second == s)
			it = subscribers.erase(it);
		else
			++it;
	}
}

}
