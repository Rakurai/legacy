#pragma once

namespace progs {
namespace symbols {

void fn_helper_echo(const String& format, Character *actor, Actable *v1, Actable *v2, Room *room);
void fn_helper_echo_at(const String& format, Character *actor, Actable *v1, Actable *v2, Room *room);
void fn_helper_echo_other(const String& format, Character *actor, Actable *v1, Actable *v2, Room *room);
void fn_helper_echo_near(const String& format, Character *actor, Actable *v1, Actable *v2, Room *room);
Character * fn_helper_get_char(const String& str, contexts::Context& context, Room *room);
void fn_helper_junk(Character *ch, Object *obj);

} // namespace symbols
} // namespace progs
