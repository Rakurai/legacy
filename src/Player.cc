#include "Player.hh"

#include "Game.hh"
#include "merc.hh"

Player::Player() :
	learned(skill::num_skills(), 0),
	evolution(skill::num_skills(), 0),
	group_known(group_table.size(), false)
{
	static long last_id;
	last_id = std::max(Game::current_time, last_id+1);
	id = last_id;
}

