#include "Player.hpp"
#include "merc.h"

Player::Player() :
	learned(skill_table.size(), 0),
	evolution(skill_table.size(), 0),
	group_known(group_table.size(), FALSE)
{}
