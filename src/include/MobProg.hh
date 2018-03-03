#pragma once

#include "declare.hh"
#include "Flags.hh"
#include "String.hh"

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
