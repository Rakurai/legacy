#include <map>
#include "String.hh"

// external forward declarations
class Character;
class RoomPrototype;

namespace event {

// forward declarations
class Subscriber;

enum Type {
	test,
	character_enter_room,
	character_give_obj,
};

class Event {
public:
	Event(Type type, std::map<const char *, void *> args) :
		type(type), args(args) {}
	virtual ~Event() {}

	void fire();

	Type type;
	std::map<const char *, void *> args;

private:
	Event();
	Event(const Event&);
	Event& operator=(const Event&);

	static std::multimap<Type, Subscriber *> subscribers;

	friend void subscribe(Type, Subscriber *);
	friend void unsubscribe(Type, Subscriber *);
};

inline void subscribe(Type type, Subscriber *s) {
	Event::subscribers.emplace(type, s);
}

inline void unsubscribe(Type type, Subscriber *s) {
	auto range = Event::subscribers.equal_range(type);

	for (auto it = range.first; it != range.second; ++it)
		if (it->second == s)
			Event::subscribers.erase(it);
}

} // namespace event

