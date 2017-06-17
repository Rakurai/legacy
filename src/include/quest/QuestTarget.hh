#pragma once

#include "declare.hh"

namespace quest {

class QuestTarget {
public:
	enum Type {
		Mob,
		Obj,
		MobVnum,
	};

	QuestTarget(Type type, sh_int location, QuestTargetable *target) :
		type(type), location(location), target(target) {}
	QuestTarget(const QuestTarget& t) :
		type(t.type), location(t.location), target(t.target) {}
	virtual ~QuestTarget() {}

	bool is_target(const QuestTargetable *) const;

	const Type type;
	const sh_int location; // this could be a RoomPrototype pointer?
	QuestTargetable *target;
	bool is_complete = false;

private:
	QuestTarget& operator=(const QuestTarget&);
};

} // namespace quest
