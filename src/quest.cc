#include "Quest.hh"

bool Quest::
is_target(const QuestTargetable *t) const {
	for (const QuestTarget& target: targets)
		if (target.is_target(t))
			return true;

	return false;
}

bool Quest::
is_complete() const {
	for (const QuestTarget& target: targets)
		if (!target.is_complete)
			return false;

	return true;
}

void Quest::
check_complete(Character *ch, const QuestTargetable *t) {
	bool found = false;

	for (QuestTarget& target: targets)
		if (target.is_target(t) && !target.is_complete) {
			target.is_complete = true;
			found = true;
		}

	if (found && is_complete())
		complete_notify(ch, t);
}
