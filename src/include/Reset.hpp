#pragma once

#include "declare.h"
#include <cstdio>

/*
 * Area-reset definition.
 */
class Reset
{
public:
    Reset(FILE *fp);
	virtual ~Reset() {}

    char                command = 0;
    sh_int              arg1 = 0;
    sh_int              arg2 = 0;
    sh_int              arg3 = 0;
    sh_int              arg4 = 0;

private:
    Reset();
	Reset(const Reset&);
	Reset& operator=(const Reset&);
};

/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */
