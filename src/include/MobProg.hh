#pragma once

#include <vector>
#include "Flags.hh"
#include "String.hh"

class Character;
class Object;

class MobProg
{
public:
	enum class Type {
		ERROR_PROG,
		IN_FILE_PROG,
		ACT_PROG,
		SPEECH_PROG,
		RAND_PROG,
		FIGHT_PROG,
		DEATH_PROG,
		HITPRCNT_PROG,
		ENTRY_PROG,
		GREET_PROG,
		ALL_GREET_PROG,
		GIVE_PROG,
		BRIBE_PROG,
		BUY_PROG,
		TICK_PROG,
		BOOT_PROG,
		RAND_AREA_PROG,
		CONTROL_PROG,
	};

	struct Line {
		enum class Type {
			IF,
			AND,
			OR,
			ELSE,
			ENDIF,
			BREAK,
			COMMAND,
		};

		Type type;
		String text;

		static Type get_type(const String& word) {
			     if (word == "if")    return Type::IF;
			else if (word == "and")   return Type::AND;
			else if (word == "or")    return Type::OR;
			else if (word == "else")  return Type::ELSE;
			else if (word == "endif") return Type::ENDIF;
			else if (word == "break") return Type::BREAK;

			return Type::COMMAND;
		}

		static const String get_type(Type type) {
			switch (type) {
				case Type::IF:      return "if";
				case Type::AND:     return "and";
				case Type::OR:      return "or";
				case Type::ELSE:    return "else";
				case Type::ENDIF:   return "endif";
				case Type::BREAK:   return "break";
				case Type::COMMAND: return "command";
			}

			return "unknown";
		}
	};

	MobProg(FILE *fp);
	virtual ~MobProg() {}

    Type               type;
    String             arglist;
    std::vector<Line>  lines;
    String             original; // just for showing in mpstat

	static Type name_to_type(const String&);
	static const String type_to_name(Type);
private:

	MobProg(const MobProg&);
	MobProg& operator=(const MobProg&);
};

extern bool    MOBtrigger;

/* mob_prog.c */
void    mprog_wordlist_check   ( const String& arg, Character *mob,
										Character* actor, Object* object,
										void* vo, MobProg::Type type );
bool    mprog_percent_check    ( Character *mob, Character* actor,
										Object* object, void* vo,
										MobProg::Type type );
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
