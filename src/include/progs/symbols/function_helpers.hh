#pragma once

namespace progs {
namespace symbols {

void fn_helper_echo(const String& format, Character *actor, Actable *v1, Actable *v2, Room *room);
void fn_helper_echo_to(const String& format, Character *actor, Actable *v1, Actable *v2, Room *room);
void fn_helper_echo_other(const String& format, Character *actor, Actable *v1, Actable *v2, Room *room);
void fn_helper_echo_near(const String& format, Character *actor, Actable *v1, Actable *v2, Room *room);
Character * fn_helper_get_char(const String& str, contexts::Context& context, Room *room);
void fn_helper_junk(Character *ch, Object *obj);
void fn_helper_purge_room(Room *, Character *safe_ch, Object *safe_obj);
void fn_helper_purge_char(Character *);
void fn_helper_purge_obj(Object *);
void fn_helper_transfer(Character *victim, Room *location);
void fn_helper_quest_assign(Character *victim, const String& quest_id);
void fn_helper_quest_progress(Character *victim, const String& quest_id);

} // namespace symbols
} // namespace progs
