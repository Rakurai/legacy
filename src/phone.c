/*
phone.c

Please check phone.h for information conserning this
file and its terms of use.

*/

#include <stdio.h>
#include <sys/select.h>
#include "chit.h"
#include "phone.h"


/* This function is called when the user wishes to
list other MUDs (by name) or connect to another MUD.
-- Outsider
*/
void do_phone(CHAR_DATA *ch, char *argument)
{
	char first_arg[BUFFER_SIZE];
	char output[BUFFER_SIZE];

	if (IS_NPC(ch)) {
		stc("Transfering is a players only option.\n\r", ch);
		return;
	}

	if (! chit_info) {
		stc("We are not currently connected to another MUD.\n\r", ch);
		return;
	}

	/* Get first argument. */
	argument = one_argument(argument, first_arg);

	/* List games we know how to connect with. */
	if (! strcmp(first_arg, "list")) {
		sprintf(output, "Possible MUDs are:\n\r{Y%s{x\n\r", OTHER_MUDS);
		stc(output, ch);
		return;
	}
	/* Try to form a connection with target MUD. */
	else if (! strcmp(first_arg, "call")) {
		/* check position */
		if (ch->position != POS_SLEEPING) {
			stc("You may only cross to another land while sleeping.\n\r", ch);
			return;
		}

		/* check for a mud name */
		if (! argument[0]) {
			stc("phone usage: phone call <mud_name>\n\r", ch);
			return;
		}

		/* target MUD will be "argument" */
		stc("Calling...\n\r", ch);
		sprintf(output, "%s phonecall", ch->name);
		/* send to mud "argument" <username> phonecall */
		Write_To_Server(chit_info, argument, output);
		return;
	}
	/* No argument, print help. */
	else {
		stc("Phone syntax:\n\r", ch);
		stc("  phone list -- prints list of other MUDs\n\r", ch);
		stc("  phone call <mud> -- attempts to connect you to another MUD.\n\r", ch);
	}
}



/* This function handles incoming requests for player connections
from remote MUDs.
-- Outsider */
void do_remote_phonecall(char *from_mud, char *from_user, char *data)
{
	char filename[128];
	FILE *player_file;
	char accept_string[BUFFER_SIZE];

	if (! chit_info) return;

	/* check to make sure that player exists */
	sprintf(filename, "%s/%s", PLAYER_DIR, from_user);
	player_file = fopen(filename, "r");

	if (player_file) { /* player exists, connect them */
		fclose(player_file);
		sprintf(accept_string, "%s phoneaccept %s", from_user, MUD_NAME_AND_PORT);
		Write_To_Server(chit_info, from_mud, accept_string);
		return;
	}

	/* There player file does not exist. Return an error. */
	sprintf(accept_string, "SYSTEM print %s Character not setup on %s.\n\r", from_user, chit_info->username);
	Write_To_Server(chit_info, from_mud, accept_string);
}




/* This function gets called when another MUD agrees to accept a
connection. This function must match the player with the name given and
attempt to re-direct the player's input to the target MUD.
The variable "data" contains the target MUD's host name and port.

Note: In this case the "from_user" variable is also the target player's
name. Since the name is the same on both MUDs.

-- Outsider
*/
void do_remote_phoneaccept(char *from_mud, char *from_user, char *data)
{
	char mud_name[BUFFER_SIZE];
	int mud_port;
	DESCRIPTOR_DATA *d;
	CHAR_DATA *ch = NULL;
	bool found = FALSE;
	struct sockaddr_in server_address;
	struct hostent *host_server;
	char buffer[BUFFER_SIZE];          /* data to be sent upon login */
	/* Find the player who is to be connected. */
	d = descriptor_list;

	while ((! found) && (d)) {
		if (IS_PLAYING(d)) {
			ch = d->original ? d->original : d->character;

			if (! strcasecmp(from_user, ch->name))
				found = TRUE;
		}   /* end of if playing */

		d = d->next;
	}      /* end of for loop */

	if (! found)
		return;

	/* Check if character is already connected to another MUD */
	if (ch->pcdata->phone_socket) {
		/* send disconnect command */
		stc("Leaving other world.\n\r", ch);
		write(ch->pcdata->phone_socket, "quit", strlen("quit"));
		close(ch->pcdata->phone_socket);
		ch->pcdata->phone_socket = 0;
	}

	/* Get MUD name and port number */
	sscanf(data, "%s %d", mud_name, &mud_port);
	/* Try to create new socket. */
	host_server = gethostbyname(mud_name);

	if (! host_server) {
		stc("Unable to find target game.\n\r", ch);
		return;
	}

	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	memcpy(&server_address.sin_addr, host_server->h_addr, host_server->h_length);
	server_address.sin_port = htons(mud_port);
	ch->pcdata->phone_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (ch->pcdata->phone_socket < 0) {
		stc("Error opening a connection to target MUD.\n\r", ch);
		return;
	}

	/* Try to connect to target MUD */
	if (connect(ch->pcdata->phone_socket, (struct sockaddr *) &server_address,
	            sizeof(server_address)) < 0) {
		close(ch->pcdata->phone_socket);
		ch->pcdata->phone_socket = 0;
		return;
	}

	stc("You are transported to another land...\n\r", ch);
	/* I'm going to put this in and see what happens.
	   Let's try to login the player with their username/password.
	   -- Outsider
	*/
	sprintf(buffer, "%s\n%s\n", ch->name, ch->pcdata->pwd);
	write(ch->pcdata->phone_socket, buffer, strlen(buffer));
	/* Block remote messages...which will now be local messages. -- Outsider */
	ch->pcdata->block_remote = TRUE;
}

