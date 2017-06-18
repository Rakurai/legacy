#include "quest/QuestManager.hh"
#include "Player.hh"
#include "Character.hh"
#include "channels.hh"

namespace quest {

int QuestManager::doubleqp = 0;

void QuestManager::
stop_quest() {
	delete quest;
	quest = nullptr;
}

void QuestManager::
update() {
	if (quest) {
		quest->countdown--;

		if (quest->countdown <= 0) {
			stc("You have run out of time for your quest!\nYou may now quest again.\n", player.ch);
			stop_quest();
			nextquest = 0;
		}
		else if (quest->countdown < 6)
			stc("Better hurry, you're almost out of time for your quest!\n", player.ch);
	}
	else {
		if (nextquest > 0) {
			nextquest--;

			if (nextquest == 0)
				stc("You may now quest again.\n", player.ch);
		}
	}
}

} // namespace quest
