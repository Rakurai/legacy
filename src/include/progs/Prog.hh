#pragma once

#include <vector>
#include "progs/Operator.hh"
#include "String.hh"
#include "Line.hh"

class Vnum;

namespace progs {

struct Line;

class Prog
{
public:
	Prog(FILE *fp, Vnum);
	virtual ~Prog() {}

    Type               type;
    String             arglist;
    std::vector<Line>  lines;
    String             original; // just for showing in mpstat

	void execute(contexts::Context& context) const;

private:
	Prog(const Prog&);
	Prog& operator=(const Prog&);
};

} // namespace progs

