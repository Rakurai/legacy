#include "QuestTarget.hh"
#include "Object.hh"
#include "Character.hh"
#include "MobilePrototype.hh"

namespace quest {

bool QuestTarget::
is_target(const QuestTargetable* t) const {
	if (type == Obj) {
		const Object *ct = dynamic_cast<const Object *>(t);
		return ct && ct == dynamic_cast<const Object *>(target);
	}

	if (type == Mob) {
		const Character *ct = dynamic_cast<const Character *>(t);
		return ct && ct == dynamic_cast<const Character *>(target);
	}

	if (type == MobVnum) {
		const Character *ct = dynamic_cast<const Character *>(t);
		return ct && ct->pIndexData == dynamic_cast<const MobilePrototype *>(target);
	}

	return false;
}

} // namespace quest
