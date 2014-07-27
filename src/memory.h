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
 * Memory management.
 * Increase MAX_STRING if you have too.
 * Tune the others only if you understand what you're doing.
 */
#define	MAX_MEM_LIST	13
#define MAX_STRING	4500000
#define MAX_PERM_BLOCK	131072

/* Magic number for memory allocation */
#define MAGIC_NUM 52571214

extern	int	nAllocString;
extern	int	sAllocString;
extern	int	nAllocPerm;
extern	int	sAllocPerm;
extern	char	*string_space;
extern	char	*top_string;
extern	SEMIPERM *semiperm_list;
extern	char	str_empty[1];
extern	char	*string_hash[MAX_KEY_HASH];
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
