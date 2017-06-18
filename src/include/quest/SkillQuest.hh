#pragma once

#include "Quest.hh"
#include "event/Subscriber.hh"

namespace quest {

class SkillQuest : 
public Quest,
public event::Subscriber {
public:
	SkillQuest(sh_int giver, sh_int time) : Quest(giver, time) {
		event::subscribe(event::character_give_obj, this); // obj+mob quests
		event::subscribe(event::object_to_char, this); // obj, obj+mob quests
		event::subscribe(event::character_to_room, this); // mob quests
	}
	virtual ~SkillQuest() {}

	static SkillQuest *generate(Character *ch, Character *questman);
	static void squestobj_to_squestmob(Character *ch, Object *obj, Character *mob);

	void complete_notify(Character *ch, const Targetable *t) const;

	bool mob_found() const;
	bool obj_found() const;

	// event::Subscriber
	void notify(event::Type type, event::Args& args);

private:
	SkillQuest(const SkillQuest&);
	SkillQuest& operator=(const SkillQuest&);
};

} // namespace quest
