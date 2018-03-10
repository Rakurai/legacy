#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"

extern void substitute_alias(Descriptor *d, String input);

namespace conn {

State * PlayingState::
handleInput(Descriptor *d, const String& argument) {
	substitute_alias(d, argument);
	return this;
}

} // namespace conn
