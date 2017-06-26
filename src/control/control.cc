#include "control.hh"

namespace control {

PlayerController *getPlayerController(Character *ch) {
	for (auto pc : Game::players)
		if (pc->character == ch)
			return pc;

	return nullptr;
}

} // namespace control
