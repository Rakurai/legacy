#pragma once

#include "Prog.hh"

class Character;
class String;
class Object;
class Vnum;

namespace progs {

extern bool    MOBtrigger;

void wordlist_check(const String& arg, std::vector<Prog *> proglist, Context& context, Prog::Type type);

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
void    control_trigger   (Character *mob, const String& key, Character *target);

} // namespace progs

