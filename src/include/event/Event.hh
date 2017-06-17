#include <list>
#include <map>

// external forward declarations
class Character;
class RoomPrototype;

namespace event {

// forward declarations
class Subscriber;

class Event {
public:
	virtual ~Event() {}

	void fire();

	void subscribe(Subscriber *s) { subscribers[type].push_back(s); }
	void unsubscribe(Subscriber *s) { subscribers[type].remove(s); }

protected:
	enum Type {
		character_enter_room,
		character_give_obj,
	};

	Event(Type t) : type(t) {}

private:
	Event();
	Event(const Event&);
	Event& operator=(const Event&);

	Type type;
	static std::map<Type, std::list<Subscriber *> > subscribers;
};

class EnterRoomEvent : public Event {
public:
	EnterRoomEvent(Character *ch, RoomPrototype *room) :
		Event(character_enter_room),
		ch(ch), room(room) {}

	Character *ch;
	RoomPrototype *room;
};

} // namespace event

