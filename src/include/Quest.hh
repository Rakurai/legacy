#pragma once

#include <vector>
#include "declare.hh"
#include "QuestTarget.hh"

class Quest {
public:
	Quest(sh_int giver, sh_int time) :
		quest_giver(giver), countdown(time) {}
	virtual ~Quest() {}

	virtual void complete_notify(Character *ch, const QuestTargetable *t) const = 0;

	bool is_target(const QuestTargetable *t) const;
	bool is_complete() const;
	void check_complete(Character *ch, const QuestTargetable *t);

	const sh_int quest_giver;
	sh_int countdown;

	std::vector<QuestTarget> targets;

private:
	Quest(const Quest&);
	Quest& operator=(const Quest&);
};
