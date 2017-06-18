#pragma once

#include <vector>
#include "declare.hh"
#include "quest/Objective.hh"

namespace quest {

class Quest {
public:
	Quest(sh_int giver, sh_int time) :
		quest_giver(giver), countdown(time) {}
	virtual ~Quest() {}

	unsigned int next_objective_pos();
	virtual void complete_notify(Character *ch, const Targetable *t) const = 0;

	bool is_target(const Targetable *t) const;
	bool is_complete() const;
	void check_complete(Character *ch, const Targetable *t);

	const sh_int quest_giver;
	sh_int countdown;

	std::vector<Objective> objectives;

private:
	Quest(const Quest&);
	Quest& operator=(const Quest&);
};

} // namespace quest
