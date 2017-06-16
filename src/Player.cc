#include "Player.hh"
#include "Quest.hh"
#include "merc.hh"

Player::Player() :
	learned(skill_table.size(), 0),
	evolution(skill_table.size(), 0),
	group_known(group_table.size(), FALSE),
	quests(*this)
{}
