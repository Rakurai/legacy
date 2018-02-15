#include <map>

#include "magic.hh"
#include "skill/skill.hh"
#include "Logging.hh"

namespace skill {

Type lookup(const String& name) {
	for (auto pair : ::skill_table)
		if (name.is_prefix_of(pair.second.name))
			return pair.first;

	return skill::unknown;
}

const skill_type& lookup(Type type) {
	auto pair = ::skill_table.find(type);

	if (pair == ::skill_table.cend()) {
		Logging::bugf("skill lookup: type %d not found", type);
		return ::skill_table.find(Type::unknown)->second;
	}

	return pair->second;
}

} // namespace skill
