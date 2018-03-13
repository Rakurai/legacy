#pragma once

#include "Flags.hh"
#include "String.hh"

class Character;
class Object;

class MobProg
{
public:
	MobProg(FILE *fp);
	virtual ~MobProg() {}

    Flags::Bit         type;
    String      arglist;
    String      comlist;

	static Flags::Bit name_to_type(const String&);
	static const String type_to_name(Flags::Bit);
private:

	MobProg(const MobProg&);
	MobProg& operator=(const MobProg&);
};

extern bool    MOBtrigger;

#define ERROR_PROG         Flags::none
#define IN_FILE_PROG       Flags::A
#define ACT_PROG           Flags::B
#define SPEECH_PROG        Flags::C
#define RAND_PROG          Flags::D
#define FIGHT_PROG         Flags::E
#define DEATH_PROG         Flags::F
#define HITPRCNT_PROG      Flags::G
#define ENTRY_PROG         Flags::H
#define GREET_PROG         Flags::I
#define ALL_GREET_PROG     Flags::J
#define GIVE_PROG          Flags::K
#define BRIBE_PROG         Flags::L
#define BUY_PROG           Flags::M
#define TICK_PROG          Flags::N
#define BOOT_PROG	       Flags::O
#define RAND_AREA_PROG     Flags::P
#define CONTROL_PROG          Flags::S

/* mob_prog.c */
void    mprog_wordlist_check   ( const String& arg, Character *mob,
										Character* actor, Object* object,
										void* vo, Flags::Bit type );
bool    mprog_percent_check    ( Character *mob, Character* actor,
										Object* object, void* vo,
										Flags::Bit type );
void    mprog_act_trigger      ( const char* buf, Character* mob,
										Character* ch, Object* obj,
										void* vo );
void    mprog_bribe_trigger    ( Character* mob, Character* ch,
										int amount );
void    mprog_entry_trigger    ( Character* mob );
void    mprog_give_trigger     ( Character* mob, Character* ch,
										Object* obj );
void    mprog_greet_trigger    ( Character* mob );
void    mprog_fight_trigger    ( Character* mob, Character* ch );
void    mprog_buy_trigger      ( Character* mob, Character* ch );
void    mprog_hitprcnt_trigger ( Character* mob, Character* ch );
void    mprog_death_trigger    ( Character* mob );
bool    mprog_random_trigger   ( Character* mob );
bool    mprog_random_area_trigger   ( Character* mob );
void    mprog_tick_trigger     ( Character* mob );
void 	mprog_boot_trigger( Character* mob	);
void    mprog_speech_trigger   ( const String& txt, Character* mob );
void    mprog_control_trigger   (Character *mob, const String& key, Character *target);
