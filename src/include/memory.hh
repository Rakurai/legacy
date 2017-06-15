#pragma once

#include "declare.hh"
/*************************************************
*                                                *
*               The Age of Legacy                *
*                                                *
* Based originally on ROM 2.4, tested, enhanced, *
* and maintained by the Legacy Team.  If that    *
* doesn't mean you, and you're stealing our      *
* code, at least tell us and boost our egos ;)   *
*************************************************/

/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH             1024
#define MAX_STRING_LENGTH        4608
#define MAX_INPUT_LENGTH          256
#define PAGELEN                    22

extern	int	top_affect;
extern	int	top_area;
extern	int	top_ed;
extern	int	top_exit;
extern	int	top_mob_index;
extern	int	top_obj_index;
extern	int	top_reset;
extern	int	top_room;
extern	int	top_shop;
extern	int	mobile_count;
