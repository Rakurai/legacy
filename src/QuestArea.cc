#include "QuestArea.hh"

#include "Logging.hh"
#include "merc.hh" // get_room
#include "Room.hh"

/* The Vnum of the quest start location -- hard coded! */
#define QUEST_STARTROOM 12000

void QuestArea::
init() {
	startroom = get_room(QUEST_STARTROOM);

	if (startroom == nullptr)
		Logging::bug("quest_init: Can't find quest start room %d.", QUEST_STARTROOM);
//	else
//		area = startroom->area();
} /* end quest_init */

const Area& QuestArea::
area() const {
	return startroom->area();
}
