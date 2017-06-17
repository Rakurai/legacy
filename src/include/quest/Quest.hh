#pragma once

#include <vector>
#include "event/Subscriber.hh"
#include "declare.hh"
#include "QuestTarget.hh"

namespace quest {

class Quest :
public event::Subscriber {
public:
	Quest(sh_int giver, sh_int time) :
		quest_giver(giver), countdown(time) {}
	virtual ~Quest() {}

	virtual void complete_notify(Character *ch, const QuestTargetable *t) const = 0;

	bool is_target(const QuestTargetable *t) const;
	bool is_complete() const;
	void check_complete(Character *ch, const QuestTargetable *t);

	// event::Subscriber
	virtual void notify(event::Event&);

	const sh_int quest_giver;
	sh_int countdown;

	std::vector<QuestTarget> targets;

private:
	Quest(const Quest&);
	Quest& operator=(const Quest&);
};

} // namespace quest
