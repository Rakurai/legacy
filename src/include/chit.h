/*

This file contains defaults for the chit
moduel.

*/

#ifndef CHIT_HEADER_FILE__
#define CHIT_HEADER_FILE__

#ifndef _REENTRANT
#define _REENTRANT
#endif
                                                                                
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>


/* server port to connect to */
#define CHIT_DEFAULT_PORT 24016

/* size of data to send to server */
#define BUFFER_SIZE 256

/* max size of username and password */
#define CHIT_USERNAME_LENGTH 64

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


typedef struct
{
   int socket;    /* socket to server */
   int connected;  /* Whether we have a connection or not */
   char username[CHIT_USERNAME_LENGTH];  
   char data_from_server[BUFFER_SIZE];   /* incomming data */
} Client_Info;

/* Make a global (ack) varible to Chat server connections. */
Client_Info *chit_info;

/* Connect to Chat server.
The server's IP address should be passed in as
"server_address". Server port is the server's port to
connect to. If you wish, you can use "DEFAULT_PORT" as
the server_port.

If NULL is passed in as the "server_address" then this
function reads the server address from the third line of
the chit.txt file.

Returns NULL on error or a Client_Info pointer
on success.
Clean up should be performed with Chit_Clean_Up()
*/
Client_Info *Chit_Connect(char *server_address, int server_port);


/* Send data to the server. Returns TRUE on success and
FALSE on failure.
The var "to_user" is the username the message is sent to.
Use the username "ALL" to send the message to everyone else.
The string sent to the user looks like:
<TO-CLIENT> <FROM-CLIENT> <MESSAGE> ... for example
"ALL Legacy Bob gossip hi there.\n\r"
*/
int Write_To_Server(Client_Info *client_info, char *to_user, char *message);


/*
This function checks for data coming from the server.
If there is data from the server, the function returns
a positive integer. Any data is stored in "buffer".
If there is no data, then 0 is returned and "buffer"
is made a NULL string. If an error occures, then -1 is
returned and buffer is made a NULL string.
*/
int Read_From_Server(Client_Info *client_info, char *buffer);


/*
This function cleanly closes the connection to the Chat server.
It sends the message "QUIT" and then shutdown the socket.
The Client_Info struc passed in is freed.
*/
void Chit_Clean_Up(Client_Info *client_info);


#endif
