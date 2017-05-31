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

/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH             1024
#define MAX_STRING_LENGTH        4608
#define MAX_INPUT_LENGTH          256
#define PAGELEN                    22

extern	int	nAllocString;
extern	int	sAllocString;
extern	int	nAllocPerm;
extern	int	sAllocPerm;
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

void *	alloc_mem    (long sMem);
void *	alloc_perm   (long sMem);
void *	alloc_perm2  (long sMem, const char *message);
void	free_mem     (void *pMem, long sMem);
