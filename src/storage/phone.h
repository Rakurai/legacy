/*
phone.h

This file contains headers and definitions to be used
to connect characters between MUDs. This code (and
the code found in phone.c) are primarily responsible for
connecting a character to another MUD. This may also be
expanded to allow characters to be shared across MUDs.

Any MUD specific code should be stored here, under
a line containing the comment CHANGE.

Code written by Jesse (Outsider) <slicer69@hotmail.com>
2004. For Age of Legacy.
This code is open source to any MUD wishing to connect to
Legacy.
*/

#ifndef PHONE_HEADER_FILE__
#define PHONE_HEADER_FILE__

#include "merc.h"
#include "chit.h"

/* The list of MUDs this MUD can connect to. */
#define OTHER_MUDS "soe"

/* Name and port to connect to THIS MUD. */
#define MUD_NAME_AND_PORT "pilot.mudmagic.com 3000"


/* The command "phone" calls this function. */
void do_phone(CHAR_DATA *ch, char *argument);

#endif

