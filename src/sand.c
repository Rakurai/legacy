/* sand.c -- Simple Asynchronous Name Daemon client routines */

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include "sand.h"


/* module-global data */

static int sockfd;
static int server_port;
static t_iadr localhost = 0x7F000001;

static struct sockaddr_in sai_client;
static struct sockaddr *sa_client = (struct sockaddr *) &sai_client;
static struct sockaddr_in sai_server;
static struct sockaddr *sa_server = (struct sockaddr *) &sai_server;
static int server_len = SA_SIZE;

/********************************************************************
* function sand_close:
* close the port for copyover.
*/
void sand_close(void)
{
    int stat;

    printf( "Closing SAND socket %d\n", sockfd );
    stat = close( sockfd );
    if ( stat != 0 )
    {
        perror( "sand_close:" );
    }
    sockfd = 0;

} /* end sand_close() */


/********************************************************************
* function sand_init:
* open the port to prepare for querying.
*/
void sand_init(int clientport, int serverport)
{
    int stat;

    printf("sand client/server ports %d / %d.\n", clientport, serverport);

    /* open socket */
    sockfd = socket(AF_INET, SOCKTYPE, PROTOCOL);
    if (sockfd < 0)
    {
        fprintf(stderr, "sand_init: socket() error %d\n", errno);
        return;
    }

    /* bind socket to our port */
    memset(&sai_client, 0, SA_SIZE);
    sai_client.sin_family = AF_INET;
    sai_client.sin_port = htons(clientport);
    sai_client.sin_addr.s_addr = htonl(localhost);
    stat = bind(sockfd, sa_client, SA_SIZE);
    if (stat < 0)
    {
        fprintf(stderr, "sand_init: bind() error %d\n", errno);
        return;
    }

    /* remember server port */
    server_port = serverport;

} /* end sand_init() */


/********************************************************************
* function sand_query:
* try to reverse lookup a given address using the sand server.
*/
char *sand_query(t_iadr addr)
{

    int stat;
    struct timeval tv;
    fd_set rfds;
    static struct anp anp;

    /* set up server socket address */
    memset(&sai_server, 0, SA_SIZE);
    sai_server.sin_family = AF_INET;
    sai_server.sin_port = htons(server_port);
    sai_server.sin_addr.s_addr = htonl(localhost);

    /* query for the name */
    stat = sendto(sockfd, &addr, IADR_SIZE, 0, sa_server, SA_SIZE);
    if (stat < 0) return NULL;

    /* wait for result of query -- up to 0.25 seconds */
    tv.tv_sec = 0;
    tv.tv_usec = 250000;

    while (1) /* retry until we get answer for correct address */
    {
        FD_ZERO(&rfds);
        FD_SET(sockfd, &rfds);
        stat = select(sockfd+1, &rfds, NULL, NULL, &tv);
        if (stat <= 0) return NULL;

        /* data available -- read and return it */
        stat = recvfrom(sockfd, &anp, sizeof(anp), 0, sa_server, &server_len);
        if (stat == sizeof(anp) && anp.addr == addr)
            return anp.name;

        /* incorrect data -- see if there is more in the queue, but quick */
        tv.tv_usec = 100000;
    }
    return NULL;

} /* end sand_query() */


