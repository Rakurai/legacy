#pragma once

namespace event {

class Subscriber {
public:
	virtual ~Subscriber() {}
	virtual void notify(Type, EventArgs&) = 0;

protected:
	Subscriber() {}

private:
	Subscriber(const Subscriber&);
	Subscriber& operator=(const Subscriber&);
};

} // namespace event
