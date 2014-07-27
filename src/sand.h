/* sand.h -- header file for simple asynchronous name daemon -- Elrac */

#if !defined(PORTS_H)
    #include "ports.h"
#endif

#define SOCKTYPE    SOCK_DGRAM
#define PROTOCOL    0

typedef unsigned long t_iadr;
#define IADR_SIZE (sizeof(t_iadr))
#define SA_SIZE   (sizeof(struct sockaddr_in))

struct anp {
    t_iadr addr;
    char name[64];
};

extern void sand_close(void);
extern void sand_init(int clientport, int serverport);
extern char *sand_query(t_iadr addr);

#define SAND
#define SAND_H

/* end of sand.h */
