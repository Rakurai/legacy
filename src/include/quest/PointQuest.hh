#pragma once

#include "Quest.hh"

namespace quest {

class PointQuest : public Quest {
public:
	PointQuest(sh_int giver, sh_int time) : Quest(giver, time) {}
	virtual ~PointQuest() {}

	static PointQuest *generate(Character *ch, Character *questman);
	virtual void complete_notify(Character *ch, const QuestTargetable *t) const;

private:
	PointQuest(const PointQuest&);
	PointQuest& operator=(const PointQuest&);
};

} // namespace quest
