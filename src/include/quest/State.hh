#pragma once

namespace quest {

class Data;

class State {
public:
	State(const Data& q) : quest(&q), step(0) {}
	virtual ~State() {}

	const Data* quest;
	unsigned char step;

	// no data owned here, copy and assignment are implicit
};

} // namespace quest
