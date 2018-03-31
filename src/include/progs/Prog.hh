#pragma once

#include <vector>
#include "String.hh"
#include "Line.hh"
#include "Type.hh"

class Vnum;

namespace progs {

class Prog
{
public:
	Prog(FILE *fp, Vnum);
	virtual ~Prog() {}

    Type               type;
    String             arglist;
    std::vector<Line>  lines;

	void execute(contexts::Context& context) const;

private:
	Prog(const Prog&);
	Prog& operator=(const Prog&);
};

} // namespace progs

