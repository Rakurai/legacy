#pragma once

#include "quest/Quest.hh"

namespace quest {

class PointQuest :
public Quest,
public event::Subscriber {
public:
	virtual ~PointQuest() {}

	static PointQuest *generate(Character *ch, Character *questman);

	virtual void complete_notify(Character *ch, const Targetable *t) const;

	// event::Subscriber
	void notify(event::Type type, event::Args& args);

private:
	PointQuest(sh_int giver, sh_int time) : Quest(giver, time) {
		event::subscribe(event::character_kill_other, this); // mob quests
		event::subscribe(event::object_to_char, this);
	}

	PointQuest(const PointQuest&);
	PointQuest& operator=(const PointQuest&);
};

} // namespace quest
