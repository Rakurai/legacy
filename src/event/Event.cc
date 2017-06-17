#include "event/Event.hh"
#include "event/Subscriber.hh"

namespace event {

void Event::
fire() {
	for (Subscriber *s: subscribers[type])
		s->notify(*this);
}

}
