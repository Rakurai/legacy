#include "Descriptor.hh"
#include "conn/State.hh"

Descriptor::
Descriptor() : state(&conn::State::getName)
{}

bool Descriptor::
is_playing() const {
    return state == &conn::State::playing
        && character != nullptr;
}
