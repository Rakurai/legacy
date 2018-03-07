#include "conn/State.hh"
#include "Logging.hh"

namespace conn {

State * ClosedState::
handleInput(Descriptor *d, const String& argument) {
	// shouldn't happen
	Logging::bugf("conn::ClosedState: handleInput called");
	return this;
}

} // namespace conn
