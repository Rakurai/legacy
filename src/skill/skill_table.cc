#include <map>

#include "magic.hh"
#include "skill/skill.hh"
#include "Logging.hh"

namespace skill {

type lookup(const String& name) {
	for (const auto& pair : ::skill_table)
		if (name.is_prefix_of(pair.second.name))
			return pair.first;

	return skill::type::unknown;
}

const skill_table_t& lookup(type t) {
	const auto& pair = ::skill_table.find(t);

	if (pair == ::skill_table.cend()) {
		Logging::bugf("skill lookup: type %d not found", t);
		return ::skill_table.find(type::unknown)->second;
	}

	return pair->second;
}

skill::type from_int(int sn) {
	for (const auto& pair : ::skill_table)
		if ((int)pair.first == sn)
			return pair.first;

	return skill::type::unknown;
}

int num_skills() {
	return skill_table.size()-1;
}

} // namespace skill
