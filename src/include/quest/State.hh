#pragma once

namespace quest {

class Quest;

class State {
public:
	State(const Quest& q) : quest(&q), step(0) {}
	virtual ~State() {}

	const Quest* quest;
	unsigned char step;

	// no data owned here, copy and assignment are implicit
};

} // namespace quest
