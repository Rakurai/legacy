#pragma once

#include "event.hh"

namespace event {

class Handler {
public:
	virtual ~Handler() {}
	virtual void notify(Type, Args&) = 0;

protected:
	Handler() {}

private:
	Handler(const Handler&);
	Handler& operator=(const Handler&);
};

} // namespace event
