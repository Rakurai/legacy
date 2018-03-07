#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"

namespace conn {

State * CopyoverRecoverState::
handleInput(Descriptor *d, const String& argument) {
	return this;
}

} // namespace conn
