#pragma once

namespace control {

class PlayerController;

enum Type {
	Mobile,
	Player,
};

PlayerController *getPlayerController(Character *ch);

} // namespace control
