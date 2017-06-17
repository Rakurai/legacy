#include "QuestManager.hh"
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

bool QuestManager::
is_target(const QuestTargetable *t) const {
	return (quest && quest->is_target(t))
	 || (squest && squest->is_target(t));
}

void QuestManager::
check_complete(const QuestTargetable *t) {
	if (quest)
		quest->check_complete(player.ch, t);

	if (squest)
		squest->check_complete(player.ch, t);
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
