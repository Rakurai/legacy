#include "quest/Quest.hh"
#include "quest/State.hh"
#include "Player.hh"
#include "Format.hh"
#include "Game.hh"
#include "Logging.hh"
#include "quest/functions.hh"
#include "World.hh"

namespace quest {

const Quest *lookup(const String& id) {
	const auto entry = Game::world().quests.find(id);

	if (entry == Game::world().quests.cend())
		return nullptr;

	return &entry->second;
}

const String list(Player *player) {
	String buf;
	int index = 1;

	for (const auto state : player->quests) {
		buf += Format::format("%2d) (%8s) %-30s - %s (step %d)\n",
			index++,
			state.quest->id,
			state.quest->name,
			state.quest->steps[state.step].description,
			state.step
		);
	}

	buf += "\n";
	return buf;
}

State *get_state(Player *player, const Quest *quest) {
	for (auto& state : player->quests)
		if (state.quest == quest)
			return &state;

	return nullptr;
}

State *get_state(Player *player, int index) {
	if (index >= 0 && (unsigned int)index < player->quests.size())
		return &player->quests[index];

	return nullptr;
}

void assign(Player *player, const Quest *quest) {
	if (get_state(player, quest) != nullptr) {
		Logging::bugf("quest::assign: player already has quest assigned");
		return;
	}

	player->quests.push_back(*quest);
}

// only called from quest::progress, error checking done
void complete(Player *player, const Quest *quest) {
	// roll some loot!
	for (const auto& reward : quest->rewards) {
		Logging::bugf("rolling reward");
	}

	// remove the quest
	remove(player, quest);
}

void progress(Player *player, const Quest *quest) {
	State *state = get_state(player, quest);

	if (state == nullptr) {
		Logging::bugf("quest::progress: player does not have quest");
		return;
	}

	if (state->step < 0 || state->step >= quest->steps.size()) {
		Logging::bugf("quest::progress: player has quest at state %d out of %d",
			state->step, quest->steps.size());
		return;
	}

	state->step++;

	if (state->step == quest->steps.size())
		complete(player, quest);
}

void remove(Player *player, const Quest *quest) {
	for (auto it = player->quests.begin(); it != player->quests.end(); /**/)
		if (it->quest == quest)
			it = player->quests.erase(it);
		else
			++it;
}

void remove(Player *player, int index) {
	if (get_state(player, index) != nullptr)
		player->quests.erase(player->quests.begin() + index);
}

} // namespace quest
