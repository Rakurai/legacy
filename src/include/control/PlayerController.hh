#pragma once

#include "declare.hh"
#include "Controller.hh"
#include "Descriptor.hh"

namespace control {

class PlayerController : public Controller {
public:
	PlayerController(const Descriptor& d) : descriptor(d) {}
	virtual ~PlayerController() {}

	void update();
	void read_from_descriptor();

	Descriptor descriptor;
	Character *original = nullptr;

private:
	PlayerController();
	PlayerController(const PlayerController&);
	PlayerController& operator=(const PlayerController&);
};

} // namespace control
