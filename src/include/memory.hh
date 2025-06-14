#pragma once

#include "constants.hh"
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

// for character stats
#define DEFENSE_MOD_MEM_SIZE (sizeof(int) * 32)
#define APPLY_CACHE_MEM_SIZE (sizeof(int) * MAX_ATTR)

extern	int	top_exit;
extern	int	top_mob_index;
extern	int	top_obj_index;
extern	int	top_reset;
extern	int	top_room;
extern	int	top_shop;
