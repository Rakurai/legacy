#include "quest/State.hh"
#include "quest/Quest.hh"

namespace quest {
	
State::
State(const Quest& q) : quest(&q) {
	// reserve 1 map per step
	stepmaps.resize(quest->steps.size());
}

} // namespace quest
