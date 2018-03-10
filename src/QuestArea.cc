#include "QuestArea.hh"

#include "Logging.hh"
#include "merc.hh" // Game::world().get_room
#include "Room.hh"
#include "Game.hh"
#include "World.hh"

/* The Vnum of the quest start location -- hard coded! */
#define QUEST_STARTROOM 12000

void QuestArea::
init() {
	startroom = Game::world().get_room(Location(Vnum(QUEST_STARTROOM)));

	if (startroom == nullptr)
		Logging::bug("quest_init: Can't find quest start room %d.", QUEST_STARTROOM);
//	else
//		area = startroom->area();
} /* end quest_init */

const Area& QuestArea::
area() const {
	return startroom->area();
}
