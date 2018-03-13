#pragma once

#include "progs/contexts/declare.hh"
#include "progs/declare.hh"
#include "progs/Type.hh"

class Character;
class String;
class Object;
class Vnum;

namespace progs {

void wordlist_check(const String& arg, std::vector<Prog *> proglist, contexts::Context&, Type);

void    act_trigger      ( const String& buf, Character* mob,
										Character* ch, Object* obj,
										void* vo );
void    bribe_trigger    ( Character* mob, Character* ch,
										int amount );
void    entry_trigger    ( Character* mob );
void    give_trigger     ( Character* mob, Character* ch,
										Object* obj );
void    greet_trigger    ( Character* mob );
void    fight_trigger    ( Character* mob, Character* ch );
void    buy_trigger      ( Character* mob, Character* ch );
void    hitprcnt_trigger ( Character* mob, Character* ch );
void    death_trigger    ( Character* mob );
bool    random_trigger   ( Character* mob );
bool    random_area_trigger   ( Character* mob );
void    tick_trigger     ( Character* mob );
void 	boot_trigger( Character* mob	);
void    speech_trigger   ( const String& txt, Character* mob );
void    call_trigger   (Character *mob, const String& key, const String& argument);
void    quest_request_trigger  (Character *ch);
bool    questor_here_trigger  (Character *ch);

void drop_trigger(Object *obj, Character *ch);

} // namespace progs

