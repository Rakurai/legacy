#pragma once

namespace quest {

class Quest;

class State {
public:
	State(const Quest& q) : quest(&q), step(0) {}
	State(const Quest& q, int s) : quest(&q), step(s) {}
	virtual ~State() {}

	const Quest* quest;
	unsigned char step;

	// no data owned here, copy and assignment are implicit
};

} // namespace quest
