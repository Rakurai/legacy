/*************************************************
*                                                *
*               The Age of Legacy                *
*                                                *
* Based originally on ROM 2.4, tested, enhanced, *
* and maintained by the Legacy Team.  If that    *
* doesn't mean you, and you're stealing our      *
* code, at least tell us and boost our egos ;)   *
*************************************************/

#include "merc.h"
#include "memory.h"

/* globals */
int     nAllocString;
int     sAllocString;
int     nAllocPerm;
int     sAllocPerm;
char    *string_space;
char    *top_string;
SEMIPERM *semiperm_list;
char    str_empty[1];
char    *string_hash[MAX_KEY_HASH];
int     top_affect;
int     top_area;
int     top_ed;
int     top_exit;
int     top_mob_index;
int     top_obj_index;
int     top_reset;
int     top_room;
int     top_shop;
int     mobile_count = 0;

/* locals */
void            *rgFreeList[MAX_MEM_LIST];
const   int     rgSizeList[MAX_MEM_LIST] = {
	16,
	32,
	64,
	128,
	256,
	1024,
	2048,
	4096,
	8192,
	16384,
	32768,
	65536,
	131072 - 64
};

/* Allocate some ordinary memory, with the expectation of freeing it someday. */
void *alloc_mem(long sMem)
{
	char *pMem;
	long *magic;
	int iList;
	sMem += sizeof(long);

	for (iList = 0; iList < MAX_MEM_LIST; iList++)
		if (sMem <= rgSizeList[iList])
			break;

	if (iList == MAX_MEM_LIST) {
		bug("Alloc_mem: size %ld too large.", sMem);
		exit(1);
	}

	if (rgFreeList[iList] == NULL)
		pMem = (char *)alloc_perm(rgSizeList[iList]);
	else {
		pMem = (char *)rgFreeList[iList];
		rgFreeList[iList] = * ((void **) rgFreeList[iList]);
	}

	magic = (long *) pMem;
	*magic = MAGIC_NUM;
	pMem += sizeof(*magic);
	return pMem;
}

/*
 * Free some memory.
 * Recycle it back onto the free list for blocks of that size.
 */
void free_mem(void *pMemV, long sMem)
{
	int iList;
	long *magic;
	char *pMem = pMemV;
	pMem -= sizeof(*magic);
	magic = (long *) pMem;

	if (*magic != MAGIC_NUM) {
		bug("Attempt to recyle invalid memory of size %ld.", sMem);
		bug((char *) pMem + sizeof(*magic), 0);
		return;
	}

	*magic = 0;
	sMem += sizeof(*magic);

	for (iList = 0; iList < MAX_MEM_LIST; iList++)
		if (sMem <= rgSizeList[iList])
			break;

	if (iList == MAX_MEM_LIST) {
		bug("Free_mem: size %ld too large.", sMem);
		exit(1);
	}

	*((void **) pMem) = rgFreeList[iList];
	rgFreeList[iList]  = pMem;
}

/*
 * Allocate some permanent memory.
 * Permanent memory is never freed,
 * pointers into it may be copied safely.
 */
void *alloc_perm(long sMem)
{
	static char *pMemPerm;
	static long iMemPerm;
	void *pMem;

	while (sMem % sizeof(long) != 0)
		sMem++;

	if (sMem > MAX_PERM_BLOCK) {
		bug("Alloc_perm: %ld too large.", sMem);
		exit(1);
	}

	if (pMemPerm == NULL || iMemPerm + sMem > MAX_PERM_BLOCK) {
		iMemPerm = 0;

		if ((pMemPerm = calloc(1, MAX_PERM_BLOCK)) == NULL) {
			perror("Alloc_perm");
			exit(1);
		}
	}

	pMem        = pMemPerm + iMemPerm;
	iMemPerm   += sMem;
	nAllocPerm += 1;
	sAllocPerm += sMem;
	return pMem;
}

/* an alloc_perm() with a message to malloc */
void *alloc_perm2(long sMem, const char *message)
{
	static char *pMemPerm;
	static long iMemPerm;
	char buf[MAX_STRING_LENGTH];
	void *pMem;

	while (sMem % sizeof(long) != 0)
		sMem++;

	if (sMem > MAX_PERM_BLOCK) {
		bug("Alloc_perm: %ld too large.", sMem);
		exit(1);
	}

	if (pMemPerm == NULL || iMemPerm + sMem > MAX_PERM_BLOCK) {
		iMemPerm = 0;

		if ((pMemPerm = calloc(1, MAX_PERM_BLOCK)) == NULL) {
			perror("Alloc_perm");
			exit(1);
		}
	}

	sprintf(buf, "{PMALLOC{x [%s] Size: %ld", message, sMem);
	wiznet(buf, NULL, NULL, WIZ_MALLOC, 0, 0);
	pMem        = pMemPerm + iMemPerm;
	iMemPerm   += sMem;
	nAllocPerm += 1;
	sAllocPerm += sMem;
	return pMem;
}
