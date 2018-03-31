#include "progs/contexts/MobProgContext.hh"
#include "progs/contexts/ObjProgContext.hh"
#include "Character.hh"
#include "Game.hh"

namespace progs {
namespace contexts {

template <>
void get_alias(Context& context, const String& key, Character** datap) {
	if (key == "self") {
		MobProgContext *ctx = dynamic_cast<MobProgContext *>(&context);

		if (ctx == nullptr)
			throw Format::format("progs::contexts::get_alias: wrong context for Character * '%s'", key);

		*datap = ctx->mob;
		return;
	}

	if (key == "master") {
		MobProgContext *ctx = dynamic_cast<MobProgContext *>(&context);

		if (ctx == nullptr)
			throw Format::format("progs::contexts::get_alias: wrong context for Character * '%s'", key);

		*datap = ctx->mob->master;
		return;
	}

	throw Format::format("progs::contexts::get_alias: unhandled key '%s' for type Character *", key);
}

template <>
void get_alias(Context& context, const String& key, Object** datap) {
	if (key == "self") {
		ObjProgContext *ctx = dynamic_cast<ObjProgContext *>(&context);

		if (ctx == nullptr)
			throw Format::format("progs::contexts::get_alias: wrong context for Object * '%s'", key);

		*datap = ctx->obj;
		return;
	}

	throw Format::format("progs::contexts::get_alias: unhandled key '%s' for type Object *", key);
}

template <>
void get_alias(Context& context, const String& key, Room** datap) {
	if (key == "room") {
		*datap = context.in_room();
		return;
	}

	throw Format::format("progs::contexts::get_alias: unhandled key '%s' for type Room *", key);
}

template <>
void get_alias(Context& context, const String& key, World** datap) {
	if (key == "world") {
		*datap = &Game::world();
		return;
	}

	throw Format::format("progs::contexts::get_alias: unhandled key '%s' for type Room *", key);
}

} // namespace contexts
} // namespace progs
