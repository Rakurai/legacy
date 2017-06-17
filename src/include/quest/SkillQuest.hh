#pragma once

#include "Quest.hh"

namespace quest {

class SkillQuest : public Quest {
public:
	SkillQuest(sh_int giver, sh_int time) : Quest(giver, time) {}
	virtual ~SkillQuest() {}

	static SkillQuest *generate(Character *ch, Character *questman);
	static void squestobj_to_squestmob(Character *ch, Object *obj, Character *mob);

	void complete_notify(Character *ch, const QuestTargetable *t) const;

	bool mob_found() const;
	bool obj_found() const;

private:
	SkillQuest(const SkillQuest&);
	SkillQuest& operator=(const SkillQuest&);
};

} // namespace quest
