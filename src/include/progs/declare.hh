#pragma once

#include <set>
#include <vector>
#include <cstdio>

#include "progs/Type.hh"
#include "Vnum.hh"

namespace progs {

class Prog;

extern void read_from_file(FILE *fp, Vnum vnum, std::vector<Prog *>& progs, std::set<Type>& types);

} // namespace progs
