#pragma once

#include <vector>
#include "String.hh"
#include "Line.hh"
#include "Operator.hh"

class Vnum;

namespace progs {

class Context;

class Prog
{
public:
	enum class Type {
		ERROR_PROG,
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

	Prog(FILE *fp, Vnum);
	virtual ~Prog() {}

    Type               type;
    String             arglist;
    std::vector<Line>  lines;
    String             original; // just for showing in mpstat

	static Type name_to_type(const String&);
	static const String type_to_name(Type);

	void execute(Context& context) const;

private:
	Prog(const Prog&);
	Prog& operator=(const Prog&);
};

} // namespace progs

