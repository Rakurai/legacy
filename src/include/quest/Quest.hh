#pragma once

#include <vector>
#include "String.hh"
#include "Vnum.hh"

namespace quest {

class Quest {
public:
	struct Step {
		Step(FILE *fp);

		const String description;
	};

	struct Reward {
		Reward(FILE *fp);

		const String type;
		const Vnum what;
		const int amount_min;
		const int amount_max;
		const bool stop;
		const int pct_chance;
	};

	Quest(const String& f, FILE *fp);
	virtual ~Quest() {}

	String filename;
	String id;
	String name;

	std::vector<Step> steps;
	std::vector<Reward> rewards;

	inline friend bool operator==(const Quest& lhs, const Quest& rhs) { return lhs.id == rhs.id; }

private:
//	Quest(const Quest&);
	Quest& operator=(const Quest&);
};

} // namespace quest
