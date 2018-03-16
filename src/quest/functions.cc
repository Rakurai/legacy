#include "quest/Quest.hh"
#include "quest/State.hh"
#include "Player.hh"
#include "Format.hh"
#include "Game.hh"
#include "Logging.hh"
#include "Character.hh"
#include "Object.hh"
#include "quest/functions.hh"
#include "World.hh"
#include "random.hh"

namespace quest {

const String divider_line("{Y======================================================================={x\n");

const Quest *lookup(const String& id) {
	const auto entry = Game::world().quests.find(id);

	if (entry == Game::world().quests.cend())
		return nullptr;

	return &entry->second;
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

	ptc(&player->ch, divider_line);
	ptc(&player->ch, "\n  {YNew QUEST - %s{x\n\n", quest->name);
	ptc(&player->ch, "\n  {Y%s{x\n\n", quest->steps[0].description);
	ptc(&player->ch, divider_line);

	player->quests.push_back(*quest);
}

// only called from quest::progress, error checking done
void complete(Player *player, const Quest *quest) {
	Character *ch = &player->ch;
	int exp_gained = 0;

	ptc(ch, divider_line);
	ptc(ch, "\n  {Y%s - QUEST COMPLETE!\n\n", quest->name);

	// roll some loot!
	for (const auto& params : quest->rewards) {
		if (!roll_chance(params.pct_chance))
			continue;

		int amount = number_range(params.amount_min, params.amount_max);

		if (params.type == "gold") {
			ptc(ch, "  You receive %d gold coin%s.\n",
				amount, amount > 1 ? "s" : "");
			ch->gold += amount;
		}
		else if (params.type == "silver") {
			ptc(ch, "  You receive %d silver coin%s.\n",
				amount, amount > 1 ? "s" : "");
			ch->silver += amount;
		}
		else if (params.type == "exp") {
			if (!ch->revoke_flags.has(REVOKE_EXP)) {
				ptc(ch, "  You receive %d experience point%s.\n",
					amount, amount > 1 ? "s" : "");
				exp_gained += amount;
			}
		}
		else if (params.type == "qp") {
			ptc(ch, "  You receive %d quest point%s.\n",
				amount, amount > 1 ? "s" : "");
			player->questpoints += amount;
		}
		else if (params.type == "sp") {
			ptc(ch, "  You receive %d skill point%s.\n",
				amount, amount > 1 ? "s" : "");
			player->skillpoints += amount;
		}
		else if (params.type == "prac") {
			ptc(ch, "  You receive %d practice%s.\n",
				amount, amount > 1 ? "s" : "");
			ch->practice += amount;
		}
		else if (params.type == "train") {
			ptc(ch, "  You receive %d train%s.\n",
				amount, amount > 1 ? "s" : "");
			ch->train += amount;
		}
		else if (params.type == "obj") {
			Vnum vnum(params.what);
			ObjectPrototype *proto = Game::world().get_obj_prototype(vnum);

			if (proto == nullptr)
				Logging::bugf("quest::complete: unable to find object with vnum %d", vnum);
			else {
				for (int i = 0; i < amount; i++) {
					Object *obj = create_object(proto, 0);

					if (obj == nullptr) {
						Logging::bugf("quest::complete: unable to create object with vnum %d", vnum);
						break;
					}

					if (CAN_WEAR(obj, ITEM_TAKE)) {
						ptc(ch, "  You receive %s{Y!\n", obj->short_descr);
						obj_to_char(obj, ch);
					}
					else {
						ptc(ch, "  %s {Yappears before you!\n", obj->short_descr);
						obj_to_room(obj, ch->in_room);
					}
				}
			}
		}

		// quit after successful roll if stop is set
		if (params.stop)
			break;
	}

	ptc(ch, "\n");
	ptc(ch, divider_line);

	// delayed to here to not mess up the quest text block
	if (exp_gained != 0)
		gain_exp(ch, exp_gained);

	// remove the quest
	remove(player, quest);
	player->completed_quests.insert(quest->id);
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

	if (state->step == quest->steps.size()){
		complete(player, quest);
	}
	else {
		ptc(&player->ch, divider_line);
		ptc(&player->ch, "\n  {Y%s: step complete!\n", quest->name);
		ptc(&player->ch, "\n  Next step: %s\n\n", quest->steps[state->step].description);
		ptc(&player->ch, divider_line);
	}
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

bool can_start(Player *player, const Quest *quest) {
	Character *ch = &player->ch;

	if (get_state(player, quest) != nullptr)
		return false;

	for (const auto& req : quest->prereqs) {
		// things that expect a non-integer value

		if (req.type == "quest_com") {
			if (player->completed_quests.count(req.value) == 0)
				return false;
		}

		if (req.type == "quest_not") {
			if (player->completed_quests.count(req.value) > 0)
				return false;
		}

		// things that expect an integer value
		int value = atoi(req.value);

		if ((req.type == "minlevel" && ch->level < value)
		 || (req.type == "maxlevel" && ch->level > value)
		 || (req.type == "minremort" && player->remort_count < value)
		 || (req.type == "maxremort" && player->remort_count > value))
			return false;

		// pass, on to next req
	}

	return true;
}

} // namespace quest
