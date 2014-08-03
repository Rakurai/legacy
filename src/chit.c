/*
chit.c

This file handles connecting to a Chat server.
Defaults from headers and config files.
See chit.h and chit.txt

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

#include "chit.h"
#include "newline.h"


Client_Info *Chit_Connect(char *server_name, int port_number)
{
	FILE *config_file;
	Client_Info *client_info;
	char password[CHIT_USERNAME_LENGTH];
	char ip_address[CHIT_USERNAME_LENGTH];   /* backup for server_name */
	struct sockaddr_in server_address;
	struct hostent *host_server;
	char buffer[BUFFER_SIZE];
	int status;
	client_info = (Client_Info *) calloc(1, sizeof(Client_Info));

	if (!client_info)
		return NULL;

	/* load defaults */
	config_file = fopen(CONFIG_FILE, "r");

	if (!config_file) {
		free(client_info);
		return NULL;
	}

	/* read username and password */
	fgets(client_info->username, CHIT_USERNAME_LENGTH, config_file);
	Strip_Newline(client_info->username);
	fgets(password, CHIT_USERNAME_LENGTH, config_file);
	Strip_Newline(password);
	fgets(ip_address, CHIT_USERNAME_LENGTH, config_file);
	Strip_Newline(ip_address);

	/* if we have a server name passed in, use that instead */
	if (server_name)
		strcpy(ip_address, server_name);

	fclose(config_file);
	/* connect to server */
	host_server = gethostbyname(ip_address);

	if (!host_server) {
		free(client_info);
		return NULL;
	}

	memset(&server_address, 0, sizeof(ip_address));
	server_address.sin_family = AF_INET;
	memcpy(&server_address.sin_addr, host_server->h_addr, host_server->h_length);
	server_address.sin_port = htons(port_number);
	// free(host_server);
	// printf("Freed host data.\n");
	client_info->socket = socket(AF_INET, SOCK_STREAM, 0);

	if (client_info->socket < 0) {
		free(client_info);
		return NULL;
	}

	if (connect(client_info->socket, (struct sockaddr *) &server_address,
	            sizeof(server_address)) < 0) {
		close(client_info->socket);
		free(client_info);
		return NULL;
	}

	/* log-in */
	/* get first prompt. */
	status = read(client_info->socket, buffer, BUFFER_SIZE);
	/* send the username */
	write(client_info->socket, client_info->username,
	      strlen(client_info->username));
	/* get response */
	status = read(client_info->socket, buffer, BUFFER_SIZE);

	/* server should return "Password: " */
	if ((status < 1) || (buffer[0] != 'P')) {
		close(client_info->socket);
		free(client_info);
		return NULL;
	}

	/* send password */
	write(client_info->socket, password, strlen(password));
	status = read(client_info->socket, buffer, BUFFER_SIZE);

	/* server should return "Ok.\n" */
	if ((status < 1) || (buffer[0] != 'O')) {
		close(client_info->socket);
		free(client_info);
		return NULL;
	}

	client_info->connected = TRUE;
	return client_info;
}



int Write_To_Server(Client_Info *client_info, char *to_user, char *message)
{
	// char *full_message;
	char full_message[BUFFER_SIZE];
	int status;
	int full_message_length;

	/* make sure we are connected to a server */
	if (! client_info->connected)
		return FALSE;

	/* get size of message */
	full_message_length = strlen(client_info->username) +
	                      strlen(to_user) +
	                      strlen(message) + 16;

	/* make sure message isn't too big */
	if (full_message_length >= BUFFER_SIZE)
		return FALSE;

	/* For some reason, these calloc calls seem to be crashing the
	   MUD. I'm going to take this calloc out and make one large
	   buffer for "full_message".
	   -- Outsider
	full_message = (char *) calloc(full_message_length, sizeof(char) );
	if (!full_message)
	   return FALSE;
	*/
	/* put message all together */
	sprintf(full_message, "%s %s %s", to_user, client_info->username,
	        message);
	status = write(client_info->socket, full_message, strlen(full_message));
	// free(full_message);

	if (status < 1) {
		client_info->connected = FALSE;
		return FALSE;
	}
	else
		return TRUE;
}



int Read_From_Server(Client_Info *client_info, char *buffer)
{
	int bytes_read;
	fd_set read_flag;
	int status;
	struct timeval wait_time;
	memset(buffer, '\0', BUFFER_SIZE);    /* make buffer a NULL string */

	if (! client_info->connected)
		return -1;

	/* clear flag */
	FD_ZERO(&read_flag);
	FD_SET(client_info->socket, &read_flag);
	/* wait for upto a second */
	wait_time.tv_sec = 0;
	wait_time.tv_usec = 1;
	/* check for avail input */
	status = select(client_info->socket + 1, &read_flag, NULL, NULL, &wait_time);

	if (status) {
		bytes_read = read(client_info->socket, buffer, BUFFER_SIZE);
		return bytes_read;
	}
	else
		return 0;
}



void Chit_Clean_Up(Client_Info *client_info)
{
	char msg[8];
	strcpy(msg, "QUIT");

	if (client_info->connected)
		write(client_info->socket, msg, strlen(msg));

	sleep(1);
	close(client_info->socket);
	free(client_info);
}

