#pragma once

#include <set>
#include <vector>
#include <cstdio>

#include "progs/Type.hh"
#include "progs/contexts/declare.hh"
#include "Vnum.hh"

namespace progs {

class Prog;

extern void read_from_file(FILE *fp, Vnum vnum, std::vector<Prog *>& progs, std::set<Type>& types);

void debug(const contexts::Context&, const String& str);

} // namespace progs
