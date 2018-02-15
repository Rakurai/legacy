#include "Player.hh"

#include "merc.hh"

Player::Player() :
	learned(skill::size, 0),
	evolution(skill::size, 0),
	group_known(group_table.size(), FALSE)
{
	static long last_id;
	last_id = std::max(current_time, last_id+1);
	id = last_id;
}

