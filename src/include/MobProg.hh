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

/**
 * @defgroup MobProgTypes Constants defining MobProg types
 * @{
 */
constexpr Flags::Bit ERROR_PROG         = Flags::none;
constexpr Flags::Bit IN_FILE_PROG       = Flags::A;
constexpr Flags::Bit ACT_PROG           = Flags::B;
constexpr Flags::Bit SPEECH_PROG        = Flags::C;
constexpr Flags::Bit RAND_PROG          = Flags::D;
constexpr Flags::Bit FIGHT_PROG         = Flags::E;
constexpr Flags::Bit DEATH_PROG         = Flags::F;
constexpr Flags::Bit HITPRCNT_PROG      = Flags::G;
constexpr Flags::Bit ENTRY_PROG         = Flags::H;
constexpr Flags::Bit GREET_PROG         = Flags::I;
constexpr Flags::Bit ALL_GREET_PROG     = Flags::J;
constexpr Flags::Bit GIVE_PROG          = Flags::K;
constexpr Flags::Bit BRIBE_PROG         = Flags::L;
constexpr Flags::Bit BUY_PROG           = Flags::M;
constexpr Flags::Bit TICK_PROG          = Flags::N;
constexpr Flags::Bit BOOT_PROG	       = Flags::O;
constexpr Flags::Bit RAND_AREA_PROG     = Flags::P;
/** @} */

/* mob_prog.c */
void    mprog_wordlist_check   ( const String& arg, Character *mob,
										Character* actor, Object* object,
										void* vo, Flags::Bit type );
void    mprog_percent_check    ( Character *mob, Character* actor,
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
void    mprog_greet_trigger    ( Character* ch );
void    mprog_fight_trigger    ( Character* mob, Character* ch );
void    mprog_buy_trigger      ( Character* mob, Character* ch );
void    mprog_hitprcnt_trigger ( Character* mob, Character* ch );
void    mprog_death_trigger    ( Character* mob );
void    mprog_random_trigger   ( Character* mob );
void    mprog_random_area_trigger   ( Character* mob );
void    mprog_tick_trigger     ( Character* mob );
void 	mprog_boot_trigger( Character* mob	);
void    mprog_speech_trigger   ( const String& txt, Character* mob );
