#pragma once

#include <vector>
#include "StateMap.hh"

namespace quest {

class Quest;

class State {
public:
	State(const Quest& q);
	State(const Quest& q, int s) : State(q) { current_step = s; }
	virtual ~State() {}

	const Quest* quest;
	unsigned char current_step = 0;

	StateMap map;
	std::vector<StateMap> stepmaps;

	// no data owned here, copy and assignment are implicit
};

} // namespace quest
