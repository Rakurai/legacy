#include "String.hh"
#include "Room.hh"
#include "Character.hh"
#include "Object.hh"
#include "Game.hh"
#include "World.hh"
#include "Exit.hh"
#include "act.hh"
#include "argument.hh"
#include "Logging.hh"
#include "progs/contexts/Context.hh"
#include "quest/functions.hh"

namespace progs {
namespace symbols {

void fn_helper_echo(const String& format, Character *actor, Actable *v1, Actable *v2, Room *room) {
	act(format, actor, v1, v2, TO_ROOM, POS_RESTING, false, room);
}

void fn_helper_echo_to(const String& format, Character *actor, Actable *v1, Actable *v2, Room *room) {
	act(format, actor, v1, v2, TO_VICT, POS_RESTING, false, room);
}

void fn_helper_echo_other(const String& format, Character *actor, Actable *v1, Actable *v2, Room *room) {
	act(format, actor, v1, v2, TO_NOTVICT, POS_RESTING, false, room);
}

void fn_helper_echo_near(const String& format, Character *actor, Actable *v1, Actable *v2, Room *room) {
	if (room == nullptr)
		return;

	for (int door = 0; door <= 5; door++) {
		Exit *pexit = room->exit[door];

		if (pexit != nullptr
		 && pexit->to_room != nullptr
		 && pexit->to_room != room)
			fn_helper_echo(format, actor, v1, v2, pexit->to_room);
	}
}

Character * fn_helper_get_char(const String& str, contexts::Context& context, Room *room) {
	String arg;
	int count = 0, number = number_argument(str, arg);

	if (arg.empty())
		return nullptr;

	for (auto ch = room->people; ch; ch = ch->next_in_room)
		if (context.can_see(ch)
		 && (ch->name.has_words(arg))
		 && ++count == number)
			return ch;

	return nullptr;
}

void fn_helper_junk(Character *ch, Object *obj) {
	if (obj == nullptr)
		return;

	if (obj->wear_loc != WEAR_NONE)
		unequip_char(ch, obj);

	extract_obj(obj);
}

void fn_helper_purge_room(Room *room, Character *safe_ch, Object *safe_obj) {
	if (room == nullptr)
		return;

	// no arg, purge whole room except mob
	Character *vnext;
	for (Character *victim = room->people; victim != nullptr; victim = vnext) {
		vnext = victim->next_in_room;

		if (victim->is_npc() && victim != safe_ch)
			extract_char(victim, true);
	}

	Object *obj_next;
	for (Object *obj = room->contents; obj != nullptr; obj = obj_next) {
		obj_next = obj->next_content;

		if (obj != safe_obj)
			extract_obj(obj);
	}
}

void fn_helper_purge_char(Character *victim) {
	if (victim != nullptr) {
		if (!victim->is_npc())
			throw Format::format("attempting to purge PC '%s'", victim->name);

		extract_char(victim, true);
	}
}

void fn_helper_purge_obj(Object *obj) {
	if (obj != nullptr)
		extract_obj(obj);
}

void fn_helper_transfer(Character *victim, Room *location) {
	if (victim == nullptr)
		throw String("null victim");

	if (location == nullptr)
		throw String("null location");

	if (location->is_private()) {
		throw String("private room");
		return;
	}

	if (victim->fighting != nullptr)
		stop_fighting(victim, true);

	char_from_room(victim);
	char_to_room(victim, location);
}

void fn_helper_quest_assign(Character *victim, const String& quest_id) {
	if (victim == nullptr)
		throw String("null victim");

	if (victim->is_npc())
		throw String("victim is NPC");

	if (quest_id.empty())
		throw String("empty quest ID");

	const quest::Quest* quest = quest::lookup(quest_id);

	if (quest == nullptr)
		throw Format::format("no such quest ID '%s'", quest_id);

	if (get_state(victim->pcdata, quest) != nullptr)
		throw Format::format("player '%s' already has quest '%s'", victim->name, quest_id);

	quest::assign(victim->pcdata, quest);
}

void fn_helper_quest_progress(Character *victim, const String& quest_id) {
	if (victim == nullptr)
		throw String("null victim");

	if (victim->is_npc())
		throw String("victim is NPC");

	if (quest_id.empty())
		throw String("empty quest ID");

	const quest::Quest* quest = quest::lookup(quest_id);

	if (quest == nullptr)
		throw Format::format("no such quest ID '%s'", quest_id);

	if (quest::get_state(victim->pcdata, quest) == nullptr)
		throw Format::format("player '%s' isn't on quest '%s'", victim->name, quest_id);

    quest::progress(victim->pcdata, quest);
}

void fn_helper_quest_state(Character *victim, const String& quest_id, int step, const String& key, const String& value) {
	if (victim == nullptr)
		throw String("null victim");

	if (victim->is_npc())
		throw String("victim is NPC");

	if (quest_id.empty())
		throw String("empty quest ID");

	const quest::Quest* quest = quest::lookup(quest_id);

	if (quest == nullptr)
		throw Format::format("no such quest ID '%s'", quest_id);

	quest::set_state_mapping(victim->pcdata, quest::lookup(quest_id), step, key, value);
}

} // namespace symbols
} // namespace progs
