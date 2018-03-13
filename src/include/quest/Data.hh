#pragma once

#include <vector>
#include "String.hh"
#include "Vnum.hh"

namespace quest {

class Data {
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

	Data(const String& f, FILE *fp);
	virtual ~Data() {}

	String filename;
	String id;
	String name;

	std::vector<Step> steps;
	std::vector<Reward> rewards;

	inline friend bool operator==(const Data& lhs, const Data& rhs) { return lhs.id == rhs.id; }

private:
//	Data(const Data&);
	Data& operator=(const Data&);
};

} // namespace quest
