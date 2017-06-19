#pragma once

#include "event.hh"

namespace event {

class Handler;

class Dispatcher {
public:
	Dispatcher();
	virtual ~Dispatcher() {}

	void fire(Type type, Args args);
	void subscribe(Type type, Handler *s);
	void unsubscribe(Type type, Handler *s);

private:
	Dispatcher(const Dispatcher&);
	Dispatcher& operator=(const Dispatcher&);

	std::multimap<Type, Handler *> subscribers;
};

} // namespace event
