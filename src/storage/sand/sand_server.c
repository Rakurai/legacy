/* sand_server */

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "../sand.h"

#define ERR_SOCKET  1
#define ERR_BIND    2

#define TMPFILE "sand_tmp.txt"
#define HSTFILE "sand_hst.txt"

/* global data */

int sockfd, stat;
t_iadr addr, localhost = 0x7F000001;
char *progname;

struct sockaddr_in sai_client;
struct sockaddr *sa_client = (struct sockaddr *) &sai_client;
unsigned int len_client;
struct sockaddr_in sai_server;
struct sockaddr *sa_server = (struct sockaddr *) &sai_server;
#define SA_SIZE (sizeof(struct sockaddr_in))

/* list */
#define MAX_ITEMS 250

typedef struct s_item {
    t_iadr addr;
    char *name;
    struct s_item *next;
} t_item;

t_item *head = NULL, *tail = NULL;
int num_items = 0;


/********************************************************************
* function dequeue:
* remove an item from the head of the list.
*/
void dequeue(void) {
    
    t_item *victim;

    if (num_items <= 0) return;
    victim = head;
    head = victim->next;
    if (victim->name != NULL) free(victim->name);
    free(victim);
    num_items--;

} /* end dequeue */
    

/********************************************************************
* function dns_lookup:
* request a reverse dns lookup from the system on a given address.
*/
char *dns_lookup(t_iadr addr) {

    struct hostent *he;
    t_iadr naddr;
    static char namecopy[64];
    char *blank;

    naddr = htonl(addr);
    he = gethostbyaddr((char *) &naddr, IADR_SIZE, AF_INET);
    if (he == NULL) return NULL;
    strncpy(namecopy, he->h_name, 63);
    blank = strchr(namecopy, ' ');
    if (blank != NULL) *blank = '\0';
    if (blank == namecopy) return NULL;

/*
    if ( !strcmp( namecopy, "dhcp14-10.superstream.net" ) )
        sprintf( namecopy, "%s", "www.microsoft.com" );
*/
   
    return namecopy;

} /* end dns_lookup() */


/********************************************************************
* function dump:
* write current list of address/name pairs to file.
*/
void dump(void) {

    t_item *ptr;
    FILE *ofp;
    t_iadr addr;
    int a1, a2, a3, a4;

    if (head == NULL) {
        fprintf(stderr, "%s: Attempt to dump empty list\n", progname);
        return;
    }
    ofp = fopen(TMPFILE, "w");
    if (ofp == NULL) {
        fprintf(stderr, "%s: Unable to write to file %s\n", 
            progname, TMPFILE);
        return;
    }
    ptr = head;
    while (ptr != NULL) {
        addr = ptr->addr; a4 = addr & 0xFF; 
        addr = addr >> 8; a3 = addr & 0xFF;
        addr = addr >> 8; a2 = addr & 0xFF;
        addr = addr >> 8; a1 = addr & 0xFF;
        fprintf(ofp, "%d.%d.%d.%d %s\n", a1, a2, a3, a4, ptr->name);
        ptr = ptr->next;
    }
    fclose(ofp);
    if (rename(TMPFILE, HSTFILE)) {
        fprintf(stderr, "%s: Unable to rename %s to %s\n",
            progname, TMPFILE, HSTFILE);
        perror("error code");
    }

} /* end dump() */


/********************************************************************
* function enqueue:
* add a new address/name combination to the list.
*/
void enqueue(t_iadr addr, char *name) {

    char *new_name;
    t_item *new_item;

    if (num_items >= MAX_ITEMS) dequeue();
    new_name = calloc(1, 1+strlen(name));
    if (new_name == NULL) return;
    strcpy(new_name, name);
    new_item = calloc(1, sizeof(t_item));
    if (new_item == NULL) {
        free(new_name);
        return;
    }
    new_item->addr = addr;
    new_item->name = new_name;
    new_item->next = NULL;
    if (tail != NULL) tail->next = new_item;
    tail = new_item;
    if (head == NULL) head = new_item;
    num_items++;

} /* end enqueue */
    

/********************************************************************
* function list_lookup:
* look up the given address in the list.
* return the corresponding name if found, else NULL.
*/
char *list_lookup(t_iadr addr) {

    t_item *ptr;

    ptr = head;
    while (ptr != NULL) {
        if (ptr->addr == addr) return ptr->name;
        ptr = ptr->next;
    }
    return NULL;

} /* end list_lookup() */

 
/********************************************************************
* function report:
* send a packet with address/name information back to client.
*/
void report(t_iadr addr, char *name) {

    int stat;
    struct anp anp;

    anp.addr = addr;
    sprintf(anp.name, "%-1.63s", name);
    stat = sendto(sockfd, &anp, sizeof(anp), 0, sa_client, len_client);
    if (stat < 0) {
        fprintf(stderr, "%s: sendto() error code %d\n",
            progname, errno);
    }
} /* end report() */


/********************************************************************
* function main:
*/
int main(int argc, char *argv[]) {

    char *name;

    progname = argv[0];
    printf("%s initializing on port %d\n", progname, SANDSERVERPORT);

    /* open socket */
    sockfd = socket(AF_INET, SOCKTYPE, PROTOCOL);
    if (sockfd < 0) {
        fprintf(stderr, "socket() error %d\n", errno);
        exit(ERR_SOCKET);
    }

    /* bind socket to our port */
    memset(&sai_server, 0, SA_SIZE);
    sai_server.sin_family = AF_INET;
    sai_server.sin_port = htons(SANDSERVERPORT);
    sai_server.sin_addr.s_addr = htonl(INADDR_ANY);
    stat = bind(sockfd, sa_server, SA_SIZE);
    if (stat < 0) {
        fprintf(stderr, "bind() error %d\n", errno);
        exit(ERR_BIND);
    }

    /* initialize client info */
    memset(&sai_client, 0, SA_SIZE);
    sai_client.sin_family = AF_INET;
    sai_client.sin_port = htons(0);
    sai_client.sin_addr.s_addr = htonl(INADDR_ANY);

    /* handle address resolution requests */
    while (1) {
        len_client = SA_SIZE;
        stat = recvfrom(sockfd, &addr, IADR_SIZE, 0, sa_client, &len_client);
        if (stat == -1) {
            fprintf(stderr, "%s: recvfrom() error %d\n", argv[0], errno);
        } else {
            printf("%s: Received addr %lx, stat = %d\n", argv[0], addr, stat);
        }
	name = list_lookup(addr);
	if (name != NULL) {
            report(addr, name);
        } else {
            name = dns_lookup(addr);
            if (name != NULL) {
                report(addr, name);
		enqueue(addr, name);
                dump();
            }
        }
    }

    return 0;

} /* end main() */

