#include "event/Event.hh"
#include "event/Subscriber.hh"
#include "String.hh"
#include <iostream>

class TestSubscriber : public event::Subscriber {
public:
	TestSubscriber() {
		event::subscribe(event::test, this);
	}

	virtual void notify(event::Event& e) {
		const char *str = (const char *)e.args["str"];
		std::cout << str << std::endl;
	}

};

class Foo {
public:
	char str[20] = "blahblah";
};

int main() {
	Foo *f = new Foo();

	event::Event(event::test, {{"str", f}}).fire();

	TestSubscriber a;
	event::Event(event::test, {{"str", f}}).fire();

}
