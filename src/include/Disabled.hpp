#pragma once

#include "String.hpp"

struct cmd_type;

class Disabled
{
public:
    Disabled() {}
    virtual ~Disabled() {}

    Disabled *next = NULL;             /* pointer to next node */
    struct cmd_type const *command = NULL;  /* pointer to the command struct*/
    String disabled_by;               /* name of disabler */
    String reason;

private:
	Disabled(const Disabled&);
	Disabled& operator=(const Disabled&);
};

void load_disabled();
bool check_disabled(const struct cmd_type *command);
