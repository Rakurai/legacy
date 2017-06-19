#include "QuestArea.hh"

#include "Logging.hh"
#include "merc.hh" // get_room_index
#include "RoomPrototype.hh"

/* The Vnum of the quest start location -- hard coded! */
#define QUEST_STARTROOM 12000

void QuestArea::
init() {
	startroom = get_room_index(QUEST_STARTROOM);

	if (startroom == nullptr)
		Logging::bug("quest_init: Can't find quest start room %d.", QUEST_STARTROOM);
	else
		area = startroom->area;
} /* end quest_init */
