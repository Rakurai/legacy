#pragma once

#include "event.hh"

namespace event {

class Subscriber {
public:
	virtual ~Subscriber() {}
	virtual void notify(Type, Args&) = 0;

protected:
	Subscriber() {}

private:
	Subscriber(const Subscriber&);
	Subscriber& operator=(const Subscriber&);
};

} // namespace event
