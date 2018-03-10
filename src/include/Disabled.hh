#pragma once

#include "String.hh"

struct cmd_type;

class Disabled
{
public:
    Disabled() {}
    virtual ~Disabled() {}

    Disabled *next = nullptr;             /* pointer to next node */
    struct cmd_type const *command = nullptr;  /* pointer to the command struct*/
    String disabled_by;               /* name of disabler */
    String reason;

private:
	Disabled(const Disabled&);
	Disabled& operator=(const Disabled&);
};

void load_disabled();
bool check_disabled(const struct cmd_type *command);
