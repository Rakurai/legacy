/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku vMud improvments copyright (C) 1992, 1993 by Michael         *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#include <unistd.h>
#include <sys/shm.h>
#include <arpa/inet.h>

#include "merc.h"
#include "recycle.h"
#include "vt100.h" /* VT100 Stuff */
#include "ports.h"
#include "lookup.h"
#include "sql.h"
#include "memory.h"


/* command procedures needed */
DECLARE_DO_FUN(do_color);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_skills);
DECLARE_DO_FUN(do_outfit);
DECLARE_DO_FUN(do_unread);


/* EPKA structure */
struct ka_struct *ka;


extern void     goto_line    args((CHAR_DATA *ch, int row, int column));
extern void     set_window   args((CHAR_DATA *ch, int top, int bottom));
extern void     roll_raffects   args((CHAR_DATA *ch, CHAR_DATA *victim));

/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern  int     malloc_debug    args((int));
extern  int     malloc_verify   args((void));
#endif

/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif



#if     defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "telnet.h"
#if defined(SAND)
#include "sand.h"
#endif
#if !defined( STDOUT_FILENO )
#define STDOUT_FILENO 1
#endif

const   char    echo_off_comm    [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const   char    echo_on_comm     [] = { IAC, WONT, TELOPT_ECHO, '\0' };
const   char    go_ahead_comm    [] = { IAC, GA, '\0' };
/*
Overflow messages are due to fact this is not declared as unsigned...however, declaring
as unsigned causes errors in write_to_buff.  Therefore, ignore the warning - Lotus
*/

#endif



/*
 * OS-dependent declarations.
 */
#if     defined(_AIX)
#include <sys/select.h>
int     accept          args((int s, struct sockaddr *addr, int *addrlen));
int     bind            args((int s, struct sockaddr *name, int namelen));
void    bzero           args((char *b, int length));
int     getpeername     args((int s, struct sockaddr *name, int *namelen));
int     getsockname     args((int s, struct sockaddr *name, int *namelen));
int     gettimeofday    args((struct timeval *tp, struct timezone *tzp));
int     listen          args((int s, int backlog));
int     setsockopt      args((int s, int level, int optname, void *optval,
                              int optlen));
int     socket          args((int domain, int type, int protocol));
#endif

/*#if     defined(apollo)
#include <unistd.h>
void    bzero           args( ( char *b, int length ) );
#endif
*/

#if     defined(__hpux)
int     accept          args((int s, void *addr, int *addrlen));
int     bind            args((int s, const void *addr, int addrlen));
void    bzero           args((char *b, int length));
int     getpeername     args((int s, void *addr, int *addrlen));
int     getsockname     args((int s, void *name, int *addrlen));
int     gettimeofday    args((struct timeval *tp, struct timezone *tzp));
int     listen          args((int s, int backlog));
int     setsockopt      args((int s, int level, int optname,
                              const void *optval, int optlen));
int     socket          args((int domain, int type, int protocol));
#endif

#if     defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

#if     defined(linux)
/*
int     accept          args( ( int s, struct sockaddr *addr, int *addrlen ) );
int     bind            args( ( int s, struct sockaddr *name, int namelen ) );
*/
int     close           args((int fd));
//int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) );
//int     getsockname     args( ( int s, struct sockaddr *name, int *namelen ) );
//int     gettimeofday    args((struct timeval *tp, struct timezone *tzp));
//int     listen          args( ( int s, int backlog ) );
int     select          args((int width, fd_set *readfds, fd_set *writefds,
                              fd_set *exceptfds, struct timeval *timeout));
int     socket          args((int domain, int type, int protocol));
//int     read            args( ( int fd, char *buf, int nbyte ) );
//int     write           args( ( int fd, char *buf, int nbyte ) );
pid_t   waitpid         args((pid_t pid, int *status, int options));
pid_t   fork            args((void));
int     kill            args((pid_t pid, int sig));
int     pipe            args((int filedes[2]));
int     dup2            args((int oldfd, int newfd));
int     execl           args((const char *path, const char *arg, ...));

/* Added by demonfire */

u_short htons           args((u_short hostshort));
//u_long  ntohl           args( ( u_long hostlong ) );
u_short ntohs           args((u_short netshort));

/* Added by demonfire */

#endif

#if     defined(MIPS_OS)
extern  int             errno;
#endif

#if     defined(NeXT)
int     close           args((int fd));
int     fcntl           args((int fd, int cmd, int arg));
#if     !defined(htons)
u_short htons           args((u_short hostshort));
#endif
#if     !defined(ntohl)
u_long  ntohl           args((u_long hostlong));
#endif
int     read            args((int fd, char *buf, int nbyte));
int     select          args((int width, fd_set *readfds, fd_set *writefds,
                              fd_set *exceptfds, struct timeval *timeout));
int     write           args((int fd, char *buf, int nbyte));
#endif

#if     defined(sequent)
int     accept          args((int s, struct sockaddr *addr, int *addrlen));
int     bind            args((int s, struct sockaddr *name, int namelen));
int     close           args((int fd));
int     fcntl           args((int fd, int cmd, int arg));
int     getpeername     args((int s, struct sockaddr *name, int *namelen));
int     getsockname     args((int s, struct sockaddr *name, int *namelen));
int     gettimeofday    args((struct timeval *tp, struct timezone *tzp));
#if     !defined(htons)
u_short htons           args((u_short hostshort));
#endif
int     listen          args((int s, int backlog));
#if     !defined(ntohl)
u_long  ntohl           args((u_long hostlong));
#endif
int     read            args((int fd, char *buf, int nbyte));
int     select          args((int width, fd_set *readfds, fd_set *writefds,
                              fd_set *exceptfds, struct timeval *timeout));
int     setsockopt      args((int s, int level, int optname, caddr_t optval,
                              int optlen));
int     socket          args((int domain, int type, int protocol));
int     write           args((int fd, char *buf, int nbyte));
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
int     accept          args((int s, struct sockaddr *addr, int *addrlen));
int     bind            args((int s, struct sockaddr *name, int namelen));
void    bzero           args((char *b, int length));
int     close           args((int fd));
int     getpeername     args((int s, struct sockaddr *name, int *namelen));
int     getsockname     args((int s, struct sockaddr *name, int *namelen));
int     gettimeofday    args((struct timeval *tp, struct timezone *tzp));
int     listen          args((int s, int backlog));
int     read            args((int fd, char *buf, int nbyte));
int     select          args((int width, fd_set *readfds, fd_set *writefds,
                              fd_set *exceptfds, struct timeval *timeout));
#if defined(SYSV)
int setsockopt          args((int s, int level, int optname,
                              const char *optval, int optlen));
#else
int     setsockopt      args((int s, int level, int optname, void *optval,
                              int optlen));
#endif

int     socket          args((int do
	                              main, int type, int protocol));

int     write           args((int fd, char *buf, int nbyte));
#endif

#if defined(ultrix)
int     accept          args((int s, struct sockaddr *addr, int *addrlen));
int     bind            args((int s, struct sockaddr *name, int namelen));
void    bzero           args((char *b, int length));
int     close           args((int fd));
int     getpeername     args((int s, struct sockaddr *name, int *namelen));
int     getsockname     args((int s, struct sockaddr *name, int *namelen));
int     gettimeofday    args((struct timeval *tp, struct timezone *tzp));
int     listen          args((int s, int backlog));
int     read            args((int fd, char *buf, int nbyte));
int     select          args((int width, fd_set *readfds, fd_set *writefds,
                              fd_set *exceptfds, struct timeval *timeout));
int     setsockopt      args((int s, int level, int optname, void *optval,
                              int optlen));
int     socket          args((int domain, int type, int protocol));
int     write           args((int fd, char *buf, int nbyte));
#endif



/*
 * Global variables.
 */
DESCRIPTOR_DATA    *descriptor_list;    /* All open descriptors         */
DESCRIPTOR_DATA    *d_next;             /* Next descriptor in loop      */
bool                god;                /* All new chars are gods!      */
bool            merc_down;              /* shutdown */
bool                wizlock;            /* Game is wizlocked            */
bool                newlock;            /* Game is newlocked            */
char                str_boot_time[MAX_INPUT_LENGTH];
time_t              reboot_time = 0;
time_t              current_time;       /* time of this pulse */
int                 port = 0;           /* telnet port for this MUD */
int                 control;
char                command[MAX_STRING_LENGTH];
int                                     last_signal = -1;

/*
 * Debug vars.
 */
char *dv_where = "";
CHAR_DATA *dv_char = NULL;
DESCRIPTOR_DATA *dv_desc = NULL;
extern char dv_command[];

/*
 * OS-dependent local functions.
 */
#if defined(unix)
void    game_loop_unix          args((int control));
int     init_socket             args((int port));
void    init_descriptor         args((int control));
bool    read_from_descriptor    args((DESCRIPTOR_DATA *d));
bool    write_to_descriptor     args((int desc, char *txt, int length));
#endif



/*
 * Other local functions (OS-independent).
 */
bool    check_parse_name        args((char *name));
bool    check_reconnect         args((DESCRIPTOR_DATA *d, char *name,
                                      bool fConn));
bool    check_playing           args((DESCRIPTOR_DATA *d, char *name));
int     main                    args((int argc, char **argv));
void    nanny                   args((DESCRIPTOR_DATA *d, char *argument));
bool    process_output          args((DESCRIPTOR_DATA *d, bool fPrompt));
void    read_from_buffer        args((DESCRIPTOR_DATA *d));
void    stop_idling             args((CHAR_DATA *ch));
void    bust_a_prompt           args((CHAR_DATA *ch));
bool    check_player_exist      args((DESCRIPTOR_DATA *d, char *name));
int     roll_stat               args((CHAR_DATA *ch, int stat));
char    *get_multi_command     args((DESCRIPTOR_DATA *d, char *argument));

/* Desparate debugging measure: A function to print a reason for exiting. */
void exit_reason(const char *module, int line, const char *reason)
{
	printf("exiting %s at line %d because: %s\n", module, line, reason);
	fflush(stdout);
}

#define EXIT_REASON(l,r) exit_reason("comm.c", l, r)


void copyover_recover()
{
	DESCRIPTOR_DATA *d;
	FILE *fp;
	char name[100], *logname;
	char host[MSL], msg1[MSL], msg2[MSL];
	int desc;
	log_string("Copyover recovery initiated");

	if ((fp = fopen(COPYOVER_LOG, "r")) == NULL) {
		perror("copyover_recover:fopen");
		log_string("Copyover log not found. Exitting.\n\r");
		exit(1);
	}

#if 0
	unlink(COPYOVER_LOG);
#endif
	logname = str_dup(fread_string(fp));
	fclose(fp);

	if ((fp = fopen(COPYOVER_FILE, "r")) == NULL) {
		perror("copyover_recover:fopen");
		log_string("Copyover file not found. Exitting.\n\r");
		exit(1);
	}

#if 0
	unlink(COPYOVER_FILE); /* In case something crashes - doesn't prevent reading */
#endif

	/* starting recovery message, do NOT use standard color codes here, it's a wtd */
	if (!str_cmp(logname, "Liriel"))
		sprintf(msg1, "\n\rYou blink, and all of a sudden, you are back where you were before,\n\r"
		        "the world refreshed and born anew.\n\r");
	else if (!str_cmp(logname, "Outsider"))
		sprintf(msg1, "\n\rDispite all of the confusion, you feel no danger in the soft hug.\n\r");
	else if (!str_cmp(logname, "Kenneth"))
		sprintf(msg1, "\n\rThe hand is cold, but seems to exert no pressure at all on your face.\n\r");
	else if (!str_cmp(logname, "Montrey"))
		sprintf(msg1, "\nThe world begins to take shape before your eyes.\n");
	else if (!str_cmp(logname, "Xenith"))
		sprintf(msg1, "\nYour vision returns, as your body forms again.\n");
	else
		sprintf(msg1, "\n\rRestoring from copyover...\n\r");

	/* finished recovery message, it's a stc so colors are ok */
	if (!str_cmp(logname, "Xenith"))
		sprintf(msg2, "\nThe world begins anew, better from the destruction.\n");
	else if (!str_cmp(logname, "Outsider"))
		sprintf(msg2, "\n\rThe angel steps back from you, revealing a cleaner, newer world. You\n\r"
		        "rub your eyes and when you open them again, the angel is gone.\n\r");
	else if (!str_cmp(logname, "Montrey"))
		sprintf(msg2, "\nYou blink, and see the world complete once again.\n"
		        "Yet, something seems somehow... different.\n");
	else if (!str_cmp(logname, "Liriel"))
		sprintf(msg2, "\n\rThe only trace of the faerie is a little blue glimmer that quickly winks\n\r"
		        "and vanishes.\n\r");
	else if (!str_cmp(logname, "Kenneth"))
		sprintf(msg2, "\n\rHalf a second later your sight is returned, the man is gone, and the\n\r"
		        "world is changed.\n\r");
	else
		sprintf(msg2, "\n\rCopyover recovery complete.\n\r");

	for (; ;) {
		fscanf(fp, "%d %s %s\n", &desc, name, host);

		if (desc == -1)
			break;

		/* Write something, and check if it goes error-free */
		if (!write_to_descriptor(desc, msg1, 0)) {
			close(desc);  /* nope */
			continue;
		}

		d = new_descriptor();
		d->descriptor = desc;
		d->host = str_dup(host);
		d->next = descriptor_list;
		descriptor_list = d;
		d->connected = CON_COPYOVER_RECOVER; /* -15, so close_socket frees the char */

		/* Now, find the pfile */
		if (!load_char_obj(d, name)) {  /* Player file not found?! */
			write_to_descriptor(desc, "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r", 0);
			close_socket(d);
			continue;
		}

		/* Just In Case */
		if (!d->character->in_room)
			d->character->in_room = get_room_index(ROOM_VNUM_TEMPLE);

		/* Insert in the char_list */
		d->character->next = char_list;
		char_list = d->character;
		d->character->pcdata->next = pc_list;
		pc_list = d->character->pcdata;
		write_to_descriptor(desc, msg2, 0);
		char_to_room(d->character, d->character->in_room);
		do_look(d->character, "auto");
		act("$n materializes!", d->character, NULL, NULL, TO_ROOM);
		d->connected = CON_PLAYING;

		if (d->character->pet != NULL) {
			char_to_room(d->character->pet, d->character->in_room);
			act("$n materializes!", d->character->pet, NULL, NULL, TO_ROOM);
		}

		record_players_since_boot++;
	}

	fclose(fp);
}

/* handle sigpipe -- Montrey */
void sig_handle(int sig)
{
	last_signal = sig;
}

int main(int argc, char **argv)
{
	struct timeval now_time;
	bool fCopyOver = FALSE;
	FILE *fpBoot = NULL;
	CHAR_DATA *tempchars;
	struct sigaction sig_act;
	/* our signal handler.  more signals can be caught with repeated calls to sigaction,
	   using the same struct and different signals.  -- Montrey */
	sig_act.sa_handler = sig_handle;
	sigemptyset(&sig_act.sa_mask);
	sig_act.sa_flags = 0;
	sigaction(SIGPIPE, &sig_act, 0);
	/* Memory debugging if needed. */
#if defined(MALLOC_DEBUG)
	malloc_debug(2);
#endif
	/* Init time. */
	gettimeofday(&now_time, NULL);
	current_time = (time_t) now_time.tv_sec;
	strcpy(str_boot_time, ctime(&current_time));
	/* Create boot file for script control -- Elrac
	   This file is created here and deleted after boot_db
	   completes, indicating a successful boot. */
#if defined(unix)
	fpBoot = fopen(BOOT_FILE, "w");

	if (fpBoot) fclose(fpBoot);

#endif
	/* Get the port number. */
	port = DIZZYPORT;

	if (argc > 1) {
		if (!is_number(argv[1])) {
			fprintf(stderr, "Usage: %s [port #]\n", argv[0]);
			EXIT_REASON(433, "bad Legacy port arg");
			exit(1);
		}
		else if ((port = atoi(argv[1])) <= 1024) {
			fprintf(stderr, "Port number must be above 1024.\n");
			EXIT_REASON(439, "bad Legacy port number");
			exit(1);
		}
	}

#if defined(unix)

	if (port != DIZZYPORT) {
		/* not running on Dizzy port, so no need for boot control */
		unlink(BOOT_FILE);
	}

#endif
	/* COPYOVER won't work anywhere but UNIX */
#if defined(unix)

	/* Check for COPYOVER argument; Get 'control' and 'rmud' descriptors if so. */
	if (argc > 3) {
		/* Are we recovering from a copyover? */
		if (argv[3] && argv[3][0]) {
			if (argc <= 4) {
				fprintf(stderr, "Not enough args for COPYOVER\n");
				EXIT_REASON(480, "not enough args for COPYOVER");
				exit(1);
			}

			if (!is_number(argv[4])) {
				fprintf(stderr, "Bad 'control' value '%s'\n", argv[4]);
				EXIT_REASON(486, "bad control value for COPYOVER");
				exit(1);
			}

			control = atoi(argv[4]);
			fCopyOver = TRUE;
		}
	}

#endif

	if (!fCopyOver)
		control = init_socket(port);

#if defined(unix)
#if defined(SAND)
	/* simple asynchronous name daemon support -- Elrac */
	sand_init(SANDCLIENTPORT, SANDSERVERPORT);
#endif
        // load our configuration
        if (load_config(CONFIG_FILE) != 0) {
                bugf("Failed to load configuration from %s.", CONFIG_FILE);
                exit(1);
        }

	/* constant connection to mysql db */
	/* this must come before boot_db() */
	db_open();
	boot_db();
	sprintf(log_buf, "Legacy is ready to rock on port %d.", port);
	log_string(log_buf);

	if (fCopyOver)
		copyover_recover();

	/* At this point, boot was successful. */
	unlink(BOOT_FILE);

	for (tempchars = char_list; tempchars; tempchars = tempchars->next) {
		if (IS_NPC(tempchars))
			mprog_boot_trigger(tempchars);
	}

	if (port == DIZZYPORT) {
		FILE *pidfile;
		int pid = getpid();

		if ((pidfile = fopen(PID_FILE, "w")) == NULL) {
			perror("getpid:fopen");
			log_string("pid file not found. Exitting.\n\r");
			exit(1);
		}

		fprintf(pidfile, "%d", pid);
		fflush(pidfile);
		fclose(pidfile);
	}

	game_loop_unix(control);
	close(control);
	/* close our database */
	db_close();
#endif
	/* That's all, folks. */
	log_string("Normal termination of game.");
	exit(0);
	return 0;
}

#if defined(unix)
int init_socket(int port)
{
	int x = 1;
	int fd;
#ifdef IPV6
	static struct sockaddr_in6 sa_zero;
	struct sockaddr_in6 sa;

	if ((fd = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
#else
	static struct sockaddr_in sa_zero;

	struct sockaddr_in sa;

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
#endif
	{
		perror("Init_socket: socket");
		EXIT_REASON(527, "Init_socket() problem");
		exit(1);
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &x, sizeof(x)) < 0) {
		perror("Init_socket: SO_REUSEADDR");
		close(fd);
		EXIT_REASON(536, "setsockopt(SO_REUSEADDR) failed");
		exit(1);
	}

#if defined(SO_NOSIGPIPE)

	if (setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, (char *) &x, sizeof(x)) < 0) {
		perror("Init_socket: SO_NOSIGPIPE");
		close(fd);
		EXIT_REASON(536, "setsockopt(SO_NOSIGPIPE) failed");
		exit(1);
	}

#endif
#if defined(SO_DONTLINGER) && !defined(SYSV)
	{
		struct  linger  ld;
		ld.l_onoff  = 1;
		ld.l_linger = 1000;

		if (setsockopt(fd, SOL_SOCKET, SO_DONTLINGER,
		               (char *) &ld, sizeof(ld)) < 0) {
			perror("Init_socket: SO_DONTLINGER");
			close(fd);
			EXIT_REASON(552, "SO_DONTLINGER failed");
			exit(1);
		}
	}
#endif
	sa              = sa_zero;
#ifdef IPV6
	sa.sin6_family   = AF_INET6;
	sa.sin6_port     = htons(port);
#else
	sa.sin_family   = AF_INET;
	sa.sin_port     = htons(port);
#endif

	if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		perror("Init socket: bind");
		close(fd);
		EXIT_REASON(566, "bind failure");
		exit(1);
	}

	if (listen(fd, 3) < 0) {
		perror("Init socket: listen");
		close(fd);
		EXIT_REASON(575, "listen() failure");
		exit(1);
	}

	return fd;
}
#endif


#if defined(unix)
void game_loop_unix(int control)
{
	static struct timeval null_time;
	struct timeval last_time;
	SEMIPERM *semiperm;
	gettimeofday(&last_time, NULL);
	current_time = (time_t) last_time.tv_sec;

	/* Main loop */
	while (!merc_down) {
		fd_set in_set;
		fd_set out_set;
		fd_set exc_set;
		DESCRIPTOR_DATA *d;
		int maxdesc;
#if defined(MALLOC_DEBUG)

		if (malloc_verify() != 1)
			abort();

#endif

		/* check and clear our signal buffer */
		if (last_signal != -1) {
			switch (last_signal) {
			case SIGPIPE:           bug("received signal SIGPIPE", 0);      break;

			default:                        bug("received signal %d", last_signal);
			}

			last_signal = -1;
		}

		/* Poll all active descriptors. */
		FD_ZERO(&in_set);
		FD_ZERO(&out_set);
		FD_ZERO(&exc_set);
		FD_SET(control, &in_set);
		maxdesc = control;

		for (d = descriptor_list; d; d = d->next) {
			maxdesc = UMAX(maxdesc, d->descriptor);
			FD_SET(d->descriptor, &in_set);
			FD_SET(d->descriptor, &out_set);
			FD_SET(d->descriptor, &exc_set);
		}

		null_time.tv_sec = 0;
		null_time.tv_usec = 0;

		if (select(maxdesc + 1, &in_set, &out_set, &exc_set, &null_time) < 0) {
			perror("Game_loop: select: poll");
			EXIT_REASON(793, "game_loop select() failure");
			exit(1);
		}

		/* New connection? */
		if (FD_ISSET(control, &in_set)) {
			dv_where = "init_descriptor()";
			init_descriptor(control);
		}

		/* Kick out the freaky folks. */
		dv_where = "kickout loop";

		for (d = descriptor_list; d != NULL; d = d_next) {
			d_next = d->next;
			dv_desc = d;

			if (FD_ISSET(d->descriptor, &exc_set)) {
				FD_CLR(d->descriptor, &in_set);
				FD_CLR(d->descriptor, &out_set);
				dv_char = d->character;
//				dv_char = d->original ? d->original : d->character;

				if (dv_char && dv_char->level > 1) {
					save_char_obj(dv_char);
					sprintf(log_buf, "Kicking out char %s", dv_char->name);
				}
				else
					strcpy(log_buf, "Kicking out unknown char");

				log_string(log_buf);
				wiznet(log_buf, NULL, NULL, WIZ_LOGINS, 0, 0);
				d->outtop = 0;
				close_socket(d);
			}
		}

		/* Process input. */
		dv_where = "input loop";

		for (d = descriptor_list; d != NULL; d = d_next) {
			d_next = d->next;
			dv_desc = d;
			dv_char = d->original ? d->original : d->character;
			d->fcommand = FALSE;

			if (FD_ISSET(d->descriptor, &in_set)) {
				if (d->character != NULL) {
					d->timer = 0;
					d->character->timer = 0;
				}

				if (d->original != NULL) {
					d->original->timer = 0;
					d->timer = 0;
				}

				if (!read_from_descriptor(d)) {
					FD_CLR(d->descriptor, &out_set);

					if (d->character != NULL && d->character->level > 1) {
						save_char_obj(d->character);
						sprintf(log_buf, "Char %s disconnected", d->character->name);
						log_string(log_buf);
						wiznet(log_buf, NULL, NULL, WIZ_MALLOC, 0, 0);
					}

					d->outtop = 0;
					close_socket(d);
					continue;
				}
			}

			if (d->character && d->character->wait > 0)
				continue;

			if (d->incomm[0] == '\0')
				read_from_buffer(d);

			if (d->incomm[0] != '\0') {
				char *command2;
				char *tempbuf;
				d->fcommand = TRUE;
				stop_idling(d->character);
				tempbuf = str_dup(d->incomm);
				command2 = get_multi_command(d, d->incomm);
				strcpy(dv_command, command2);

				if (d->showstr_point)
					show_string(d, tempbuf);
				else if (d->connected == CON_PLAYING) {
					if (d->character == NULL) {
						bug("playing descriptor with null character, closing phantom socket", 0);
						close_socket(d);
						continue;
					}

					dv_where = "before substitute_alias()";
					substitute_alias(d, command2);
					dv_where = "after substitute_alias()";
				}
				else {
					dv_where = "before nanny()";
					nanny(d, command2);
					dv_where = "after  nanny()";
					d->incomm[0] = '\0';
				}

				/* Attempt to fix memory bug -- Elrac */
				if (tempbuf)
					free_string(tempbuf);
			}    /* end of have input */
		} /* end of input loop */

		dv_where = "before update_handler()";
		update_handler();
		dv_where = "after  update_handler()";
		/* Output. */
		dv_where = "output loop";

		for (d = descriptor_list; d != NULL; d = d_next) {
			d_next = d->next;
			dv_desc = d;
			dv_char = d->character;

			if ((d->fcommand || d->outtop > 0)
			    && FD_ISSET(d->descriptor, &out_set)) {
				if (!process_output(d, TRUE)) {
					if (d->character != NULL && d->character->level > 1)
						save_char_obj(d->character);

					d->outtop = 0;
					close_socket(d);
				}
			}
		}

		/* Synchronize to a clock.
		   Sleep( last_time + 1/PULSE_PER_SECOND - now ).
		   Careful here of signed versus unsigned arithmetic. */
		{
			struct timeval now_time;
			long secDelta;
			long usecDelta;
			gettimeofday(&now_time, NULL);
			usecDelta = ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			            + 1000000 / PULSE_PER_SECOND;
			secDelta  = ((int) last_time.tv_sec) - ((int) now_time.tv_sec);

			while (usecDelta < 0) {
				usecDelta += 1000000;
				secDelta--;
			}

			while (usecDelta >= 1000000) {
				usecDelta -= 1000000;
				secDelta++;
			}

			if (secDelta > 0 || (secDelta == 0 && usecDelta > 0)) {
				struct timeval stall_time;
				stall_time.tv_usec = usecDelta;
				stall_time.tv_sec  = secDelta;

				if (select(0, NULL, NULL, NULL, &stall_time) < 0) {
					perror("Game_loop: select: stall");
					EXIT_REASON(979, "game_loop select() stall");
					exit(1);
				}
			}
		}
		gettimeofday(&last_time, NULL);
		current_time = (time_t) last_time.tv_sec;

		/* free our semiperm list */
		for (semiperm = semiperm_list; semiperm; semiperm = semiperm->next)
			free_semiperm(semiperm);
	}

	return;
}
#endif



#if defined(unix)
void init_descriptor(int control)
{
	DESCRIPTOR_DATA *dnew;
#ifdef IPV6
	struct sockaddr_in6 sock;
#else
	struct sockaddr_in sock;
#endif
	struct hostent *from;
	int desc;
	unsigned int size; /* Added unsigned Lotus359 */
#if defined(SAND)
	char *tmp_name;
#endif
	size = sizeof(sock);
	getsockname(control, (struct sockaddr *) &sock, &size);

	if ((desc = accept(control, (struct sockaddr *) &sock, &size)) < 0) {
		wiznet("init_descriptor: error accepting new connection",
		       NULL, NULL, WIZ_MALLOC, 0, 0);
		perror("New_descriptor: accept");
		return;
	}

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

	if (fcntl(desc, F_SETFL, FNDELAY) == -1) {
		wiznet("init_descriptor: error setting FNDELAY",
		       NULL, NULL, WIZ_MALLOC, 0, 0);
		perror("New_descriptor: fcntl: FNDELAY");
		return;
	}

	/*
	 * Cons a new descriptor.
	 */
	dnew = new_descriptor();
	dnew->descriptor = desc;
	/* printf( "new descriptor at socket %d\n", desc ); */
	size = sizeof(sock);

	if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0) {
		wiznet("init_descriptor: error getting peername",
		       NULL, NULL, WIZ_MALLOC, 0, 0);
		perror("New_descriptor: getpeername");
		dnew->host = str_dup("(unknown)");
	}
	else {
		/*
		 * Would be nice to use inet_ntoa here but it takes a struct arg,
		 * which ain't very compatible between gcc and system libraries.
		 */
#ifdef IPV6
		char buf[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &sock.sin6_addr, buf, sizeof(buf));
#else
		char buf[INET_ADDRSTRLEN];
		int addr = ntohl(sock.sin_addr.s_addr);
		sprintf(buf, "%d.%d.%d.%d",
		        (addr >> 24) & 0xFF, (addr >> 16) & 0xFF,
		        (addr >>  8) & 0xFF, (addr) & 0xFF
		       );
#endif
//        if ( addr != 0x7F000001L ) /* don't log localhost -- Elrac */
		{
			sprintf(log_buf, "init_descriptor: sock.sinaddr  = %s", buf);
			log_string(log_buf);
			wiznet(log_buf, NULL, NULL, WIZ_MALLOC, 0, 0);
		}
		from = NULL;
		/* New coding to access the sand server. -- Elrac
		   This prevents crashes and lag from overly long lookups. */
#if defined(SAND)
		tmp_name = sand_query(addr);

		if (tmp_name == NULL) {
			sprintf(log_buf, "name not available");
			log_string(log_buf);
			dnew->host = str_dup(buf);
		}
		else {
			dnew->host = str_dup(tmp_name);
//                if ( addr != 0x7F000001L ) /* don't log localhost -- Elrac */
			{
				if (strcmp("kyndig.com", dnew->host)) {
					sprintf(log_buf, "init_descriptor: host name = %s", dnew->host);
					log_string(log_buf);
					wiznet(log_buf, NULL, NULL, WIZ_SITES, 0, 0);
				}
			}
		}

#else
#ifdef IPV6
		from = gethostbyaddr((char *) &sock.sin6_addr, sizeof(sock.sin6_addr), AF_INET6);
#else
		from = gethostbyaddr((char *) &sock.sin_addr, sizeof(sock.sin_addr), AF_INET);
#endif

		if (from == NULL || from->h_name == NULL) {
			sprintf(log_buf, "name not available");
			log_string(log_buf);
			dnew->host = str_dup(buf);
		}
		else {
			dnew->host = str_dup(from->h_name);
//                if ( addr != 0x7F000001L ) /* don't log localhost -- Elrac */
			{
				if (strcmp("kyndig.com", dnew->host)) {
					sprintf(log_buf, "init_descriptor: host name = %s", dnew->host);
					log_string(log_buf);
					wiznet(log_buf, NULL, NULL, WIZ_SITES, 0, 0);
				}
			}
		}

#endif
	}

	/*
	 * Init descriptor data.
	 */
	dnew->next                  = descriptor_list;
	descriptor_list             = dnew;
	{
		extern char *help_greeting;
		cwtb(dnew, help_greeting[0] == '.' ? help_greeting + 1 : help_greeting);
	}
	return;
}
#endif


void close_socket(DESCRIPTOR_DATA *dclose)
{
	CHAR_DATA *ch;
	DESCRIPTOR_DATA *d;

	if (dclose->outtop > 0)
		process_output(dclose, FALSE);

	if (dclose->snoop_by != NULL)
		write_to_buffer(dclose->snoop_by, "Your victim has left the game.\n\r", 0);

	for (d = descriptor_list; d != NULL; d = d->next)
		if (d->snoop_by == dclose)
			d->snoop_by = NULL;

	if ((ch = dclose->character) == NULL) {
		sprintf(log_buf, "Closing link to phantom at socket %d.", dclose->descriptor);
		/* log_string( log_buf ); */
	}
	else {
		sprintf(log_buf, "Closing link to %s.", ch->name);
		log_string(log_buf);

		if (dclose->connected == CON_PLAYING) {
			CHAR_DATA *rch;

			for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
				if (!IS_IMMORTAL(ch) || can_see(rch, ch))
					ptc(rch, "%s has lost %s link.\n\r",
					    PERS(ch, rch, VIS_CHAR),
					    GET_SEX(ch) == SEX_FEMALE ? "her" :
					    GET_SEX(ch) == SEX_MALE   ? "his" : "its");

			wiznet("$N has lost link.", ch, NULL, WIZ_LINKS, 0, 0);

			if (!IS_NPC(ch))
				SET_BIT(ch->pcdata->plr, PLR_LINK_DEAD);
			else {
				/* been having problems with this -- Montrey */
				if (ch->desc == NULL)
					bug("close_socket: NPC without descriptor!", 0);
				else if (ch->desc->original != NULL
				         && ch->desc->original->pcdata != NULL)
					SET_BIT(ch->desc->original->pcdata->plr, PLR_LINK_DEAD);

				if (IS_SET(ch->act, ACT_MORPH)) {
					if (ch->desc->original != NULL) {
						ROOM_INDEX_DATA *location;

						if (ch->in_room == NULL)
							location = get_room_index(ROOM_VNUM_MORGUE);
						else
							location = ch->in_room;

						char_from_room(ch->desc->original);
						char_to_room(ch->desc->original, location);

						if (ch->in_room != NULL)
							char_from_room(ch);
					}
				}
			}

			ch->desc = NULL;
		}
		else
			free_char(dclose->character);
	}

	if (d_next == dclose)
		d_next = d_next->next;

	if (dclose == descriptor_list)
		descriptor_list = descriptor_list->next;
	else {
		DESCRIPTOR_DATA *d;

		for (d = descriptor_list; d && d->next != dclose; d = d->next)
			;

		if (d != NULL)
			d->next = dclose->next;
		else
			bug("Close_socket: dclose not found.", 0);
	}

	printf("Closing socket %d\n", dclose->descriptor);
	close(dclose->descriptor);
	free_descriptor(dclose);
	return;
}



bool read_from_descriptor(DESCRIPTOR_DATA *d)
{
	int iStart;

	/* Hold horses if pending command already. */
	if (d->incomm[0] != '\0')
		return TRUE;

	/* Check for overflow. */
	iStart = strlen(d->inbuf);

	if (iStart >= sizeof(d->inbuf) - 10) {
		sprintf(log_buf, "%s input overflow!", d->host);
		log_string(log_buf);
		write_to_descriptor(d->descriptor,
		                    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0);
		return FALSE;
	}

	/* Snarf input. */
#if defined(unix)

	for (; ;) {
		int nRead;
		nRead = read(d->descriptor, d->inbuf + iStart,
		             sizeof(d->inbuf) - 10 - iStart);

		if (nRead > 0) {
			iStart += nRead;

			if (d->inbuf[iStart - 1] == '\n' || d->inbuf[iStart - 1] == '\r')
				break;
		}
		else if (nRead == 0) {
			if (d->character && d->character->level > 0)
				sprintf(log_buf, "EOF on read from char %s", d->character->name);
			else if (!strcmp(d->host, "localhost"))
				return FALSE;
			else
				sprintf(log_buf, "EOF on read from host %s", d->host);

			log_string(log_buf);
			return FALSE;
		}
		else if (errno == EWOULDBLOCK)
			break;
		else {
			perror("Read_from_descriptor");
			return FALSE;
		}
	}

#endif
	d->inbuf[iStart] = '\0';
	return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer(DESCRIPTOR_DATA *d)
{
	int i, j, k;

	/*
	 * Hold horses if pending command already.
	 */
	if (d->incomm[0] != '\0')
		return;

	/*
	 * Look for at least one new line.
	 */
	for (i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++) {
		if (d->inbuf[i] == '\0')
			return;
	}

	/*
	 * Canonical input processing.
	 */
	for (i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++) {
		if (k >= MAX_INPUT_LENGTH - 2) {
			write_to_descriptor(d->descriptor, "Line too long.\n\r", 0);

			/* skip the rest of the line */
			for (; d->inbuf[i] != '\0'; i++) {
				if (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
					break;
			}

			d->inbuf[i]   = '\n';
			d->inbuf[i + 1] = '\0';
			break;
		}

		if (d->inbuf[i] == '\b' && k > 0)
			--k;
		else if (isascii(d->inbuf[i]) && isprint(d->inbuf[i]))
			d->incomm[k++] = d->inbuf[i];
	}

	/*
	 * Finish off the line.
	 */
	if (k == 0)
		d->incomm[k++] = ' ';

	d->incomm[k] = '\0';

	/*
	 * Deal with bozos with #repeat 1000 ...
	 */

	if (k > 1 || d->incomm[0] == '!') {
		if (d->incomm[0] != '!' && strcmp(d->incomm, d->inlast))
			d->repeat = 0;
		else {
			if (++d->repeat >= 25) {
				sprintf(log_buf, "%s: input spamming!", d->host);
				log_string(log_buf);
				wiznet("And the spammer of the year is:  $N!!!",
				       d->character, NULL, WIZ_SPAM, 0, GET_RANK(d->character));

				if (d->incomm[0] == '!')
					wiznet(d->inlast, d->character, NULL, WIZ_SPAM, 0,
					       GET_RANK(d->character));
				else
					wiznet(d->incomm, d->character, NULL, WIZ_SPAM, 0,
					       GET_RANK(d->character));

				d->repeat = 0;
				/*
				                write_to_descriptor( d->descriptor,
				                    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
				                strcpy( d->incomm, "quit" );
				*/
			}
		}
	}

	/*
	 * Do '!' substitution.
	 */
	if (d->incomm[0] == '!') {
		/* Allow commands to be appended to "!" commands.
		   This is done, only if we don't overflow the buffer.
		   -- Outsider
		*/
		if ((strlen(d->incomm) + strlen(d->inlast) + 16) < MAX_INPUT_LENGTH) {
			char temp_buffer[MAX_INPUT_LENGTH];
			/* new command will be old command + everything after the "!" */
			sprintf(temp_buffer, "%s%s", d->inlast, & (d->incomm[1]));
			strcpy(d->incomm, temp_buffer);
		}
		else   /* message was too long, use last command */
			strcpy(d->incomm, d->inlast);
	}
	else
		strcpy(d->inlast, d->incomm);

	/*
	 * Shift the input buffer.
	 */
	while (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
		i++;

	for (j = 0; (d->inbuf[j] = d->inbuf[i + j]) != '\0'; j++)
		;

	return;
}



/*
 * Low level output function.
 */
bool process_output(DESCRIPTOR_DATA *d, bool fPrompt)
{
	extern bool merc_down;

	/* VT100 Stuff */
	if (IS_PLAYING(d)
	    && d->character->pcdata
	    && IS_SET(d->character->pcdata->video, VIDEO_VT100)) {
		write_to_buffer(d, VT_SAVECURSOR, 0);
		goto_line(d->character, d->character->lines - 2, 1);
	}

	/*
	 * Bust a prompt.
	 */
	if (!merc_down && d->showstr_point)
		write_to_buffer(d, "[Hit Enter to continue]\n\r", 0);
	else if (fPrompt && !merc_down && IS_PLAYING(d)) {
		CHAR_DATA *ch;
		CHAR_DATA *victim;
		ch = d->character;

		/* battle prompt */
		if ((victim = ch->fighting) != NULL) {
			char atb[MSL];

			if (IS_SET(ch->comm, COMM_ATBPROMPT)) {
				if (ch->wait > 40)      sprintf(atb, "{B[{C*{T*********{B]{x ");
				else if (ch->wait > 36) sprintf(atb, "{B[{Y*{C*{T********{B]{x ");
				else if (ch->wait > 32) sprintf(atb, "{B[{C*{Y*{C*{T*******{B]{x ");
				else if (ch->wait > 28) sprintf(atb, "{B[{T*{C*{Y*{C*{T******{B]{x ");
				else if (ch->wait > 24) sprintf(atb, "{B[{T**{C*{Y*{C*{T*****{B]{x ");
				else if (ch->wait > 20) sprintf(atb, "{B[{T***{C*{Y*{C*{T****{B]{x ");
				else if (ch->wait > 16) sprintf(atb, "{B[{T****{C*{Y*{C*{T***{B]{x ");
				else if (ch->wait > 12) sprintf(atb, "{B[{T*****{C*{Y*{C*{T**{B]{x ");
				else if (ch->wait > 8)  sprintf(atb, "{B[{T******{C*{Y*{C*{T*{B]{x ");
				else if (ch->wait > 4)  sprintf(atb, "{B[{T*******{C*{Y*{C*{B]{x ");
				else if (ch->wait > 0)  sprintf(atb, "{B[{T********{C*{Y*{B]{x ");
				else                    sprintf(atb, "{B[{C**{YREADY!{C**{B]{x ");
			}/*

                                if (ch->wait > 40)      sprintf(atb, "{P[{P*{R*********{P]{x ");
                                else if (ch->wait > 36) sprintf(atb, "{P[{Y*{P*{R********{P]{x ");
                                else if (ch->wait > 32) sprintf(atb, "{P[{P*{Y*{P*{R*******{P]{x ");
                                else if (ch->wait > 28) sprintf(atb, "{P[{R*{P*{Y*{P*{R******{P]{x ");
                                else if (ch->wait > 24) sprintf(atb, "{P[{R**{P*{Y*{P*{R*****{P]{x ");
                                else if (ch->wait > 20) sprintf(atb, "{P[{R***{P*{Y*{P*{R****{P]{x ");
                                else if (ch->wait > 16) sprintf(atb, "{P[{R****{P*{Y*{P*{R***{P]{x ");
                                else if (ch->wait > 12) sprintf(atb, "{P[{R*****{P*{Y*{P*{R**{P]{x ");
                                else if (ch->wait > 8)  sprintf(atb, "{P[{R******{P*{Y*{P*{R*{P]{x ");
                                else if (ch->wait > 4)  sprintf(atb, "{P[{R*******{P*{Y*{P*{P]{x ");
                                else if (ch->wait > 0)  sprintf(atb, "{P[{R********{P*{Y*{P]{x ");
                                else                    sprintf(atb, "{P[{R**{YREADY!{R**{P]{x ");

                                sprintf(buf, "({G%d{x) ", ch->fightpulse);
                                strcat(atb, buf);
                        } */
			else
				sprintf(atb, "{x");

			if (can_see(ch, victim)) {
				int percent;

				if (IS_NPC(victim))
					strcat(atb, victim->short_descr);
				else
					strcat(atb, victim->name);

				if (victim->max_hit > 0)
					percent = victim->hit * 100 / victim->max_hit;
				else
					percent = -1;

				if (percent >= 100)     strcat(atb, " is in excellent condition.");
				else if (percent >= 90) strcat(atb, " has a few scratches.");
				else if (percent >= 75) strcat(atb, " has some small wounds and bruises.");
				else if (percent >= 50) strcat(atb, " has quite a few wounds.");
				else if (percent >= 30) strcat(atb, " has some big nasty wounds and scratches.");
				else if (percent >= 15) strcat(atb, " looks pretty hurt.");
				else if (percent >= 1)  strcat(atb, " is in awful condition.");
				else if (percent >= 0)  strcat(atb, " will soon be toast!!!");
				else                    strcat(atb, " is in need of ***SERIOUS*** medical attention!");
			}

			ptc(ch, "%s\n\r", atb);
		}

		ch = d->original ? d->original : d->character;

		if (!IS_SET(ch->comm, COMM_COMPACT))
			write_to_buffer(d, "\n\r", 2);

		if (IS_SET(ch->comm, COMM_PROMPT)) {
			set_color(ch, CYAN, NOBOLD);
			bust_a_prompt(d->character);
			set_color(ch, WHITE, NOBOLD);
		}
	}

	/*
	 * Short-circuit if nothing to write.
	 */
	if (d->outtop == 0)
		return TRUE;

	/*
	 * Snoop-o-rama.
	 */
	if (d->snoop_by != NULL) {
		if (d->character != NULL)
			write_to_buffer(d->snoop_by, (d->character)->name, 0);

		write_to_buffer(d->snoop_by, "> ", 2);
		write_to_buffer(d->snoop_by, d->outbuf, d->outtop);
	}

	/* VT100 Stuff */
	if (IS_PLAYING(d)
	    && d->character->pcdata
	    && IS_SET(d->character->pcdata->video, PLR_VT100)) {
		goto_line(d->character, d->character->lines - 1, 1);
		write_to_buffer(d, VT_CLEAR_LINE, 0);
		write_to_buffer(d, VT_BAR, 0);
		write_to_buffer(d, VT_RESTORECURSOR, 0);
	}

	/*
	 * OS-dependent output.
	 */
	if (!write_to_descriptor(d->descriptor, d->outbuf, d->outtop)) {
		d->outtop = 0;
		return FALSE;
	}
	else {
		d->outtop = 0;
		return TRUE;
	}
}




/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt(CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
	char doors[MAX_INPUT_LENGTH];
	const char *str, *i;
	char *point;
	EXIT_DATA *pexit;
	bool found;
	const char *dir_name[] = {"N", "E", "S", "W", "U", "D"};
	const char *dirl_name[] = {"n", "e", "s", "w", "u", "d"};
	int door, color, bold;
	point = buf;
	str = ch->prompt;

	if (str == NULL || str[0] == '\0') {
		ptc(ch, "{W<{C%d{Thp {G%d{Hma {B%d{Nst{W>{x %s", ch->hit, ch->mana, ch->stam, ch->prefix);
		return;
	}

	if (IS_SET(ch->comm, COMM_AFK)) {
		stc("{b<AFK> {x", ch);
		return;
	}

	while (*str != '\0') {
		if (*str == '{') {      /* hack to make it backwards compatible with %C(color) */
			++str;

			if (*str == '{') {
				sprintf(buf2, "{");
				i = buf2;
				++str;

				while ((*point = *i) != '\0')
					++point, ++i;

				continue;
			}

			switch (*str) {
			default:        color = WHITE;  bold = NOBOLD;  break;

			case 'W':       color = WHITE;  bold = BOLD;    break;

			case 'g':       color = WHITE;  bold = NOBOLD;  break;

			case 'P':       color = RED;    bold = BOLD;    break;

			case 'R':       color = RED;    bold = NOBOLD;  break;

			case 'Y':       color = YELLOW; bold = BOLD;    break;

			case 'b':       color = YELLOW; bold = NOBOLD;  break;

			case 'G':       color = GREEN;  bold = BOLD;    break;

			case 'H':       color = GREEN;  bold = NOBOLD;  break;

			case 'C':       color = CYAN;   bold = BOLD;    break;

			case 'T':       color = CYAN;   bold = NOBOLD;  break;

			case 'B':       color = BLUE;   bold = BOLD;    break;

			case 'N':       color = BLUE;   bold = NOBOLD;  break;

			case 'V':       color = PURPLE; bold = BOLD;    break;

			case 'M':       color = PURPLE; bold = NOBOLD;  break;

			case 'c':       color = GREY;   bold = BOLD;    break;

			case 'k':       color = GREY;   bold = NOBOLD;  break;
			}

			if (IS_SET(ch->act, PLR_COLOR))
				sprintf(buf2, "\033[%d;%dm", bold, color);
			else
				sprintf(buf2, " ");

			i = buf2;
			++str;

			while ((*point = *i) != '\0')
				++point, ++i;

			continue;
		}

		if (*str != '%') {
			*point++ = *str++;
			continue;
		}

		++str;

		switch (*str) {
		default:
			i = " ";
			break;

		case 'e':
			found = FALSE;
			doors[0] = '\0';

			for (door = 0; door < 6; door++) {
				if ((pexit = ch->in_room->exit[door]) != NULL
				    &&  pexit ->u1.to_room != NULL
				    && (can_see_room(ch, pexit->u1.to_room)
				        || (IS_AFFECTED(ch, AFF_INFRARED)))
				    &&   !IS_AFFECTED(ch, AFF_BLIND)) {
					found = TRUE;

					if (!IS_SET(pexit->exit_info, EX_CLOSED))
						strcat(doors, dir_name[door]);
					else
						strcat(doors, dirl_name[door]);
				}
			}

			if (!found)
				strcat(doors, "none");

			sprintf(buf2, "%s", doors);
			i = buf2;
			break;

		case 'c':
			sprintf(buf2, "%s", "\n\r");
			i = buf2;
			break;

		case 'h':
			sprintf(buf2, "%d", ch->hit);
			i = buf2;
			break;

		case 'H':
			sprintf(buf2, "%d", ch->max_hit);
			i = buf2;
			break;

		case 'm':
			sprintf(buf2, "%d", ch->mana);
			i = buf2;
			break;

		case 'M':
			sprintf(buf2, "%d", ch->max_mana);
			i = buf2;
			break;

		case 'v':
			sprintf(buf2, "%d", ch->stam);
			i = buf2;
			break;

		case 'V':
			sprintf(buf2, "%d", ch->max_stam);
			i = buf2;
			break;

		case 'x':
			sprintf(buf2, "%d", ch->exp);
			i = buf2;
			break;

		case 'X':
			if (!IS_NPC(ch))
				sprintf(buf2, "%ld",
				        (ch->level + 1) * exp_per_level(ch, ch->pcdata->points) - ch->exp);

			i = buf2;
			break;

		case 'g':
			sprintf(buf2, "%ld", ch->gold);
			i = buf2;
			break;

		case 's':
			sprintf(buf2, "%ld", ch->silver);
			i = buf2;
			break;

		case 'a':
			if (ch->level > 9)
				sprintf(buf2, "%d", ch->alignment);
			else
				sprintf(buf2, "%s", IS_GOOD(ch) ? "good" : IS_EVIL(ch) ? "evil" : "neutral");

			i = buf2;
			break;

		case 'r':
			if (ch->in_room != NULL)
				sprintf(buf2, "%s",
				        (IS_IMMORTAL(ch) ||
				         (!IS_AFFECTED(ch, AFF_BLIND) &&
				          !room_is_dark(ch->in_room)))
				        ? ch->in_room->name : "darkness");
			else
				sprintf(buf2, " ");

			i = smash_bracket(buf2);
			break;

		case 'R':
			if (IS_IMMORTAL(ch) && ch->in_room != NULL)
				sprintf(buf2, "%d", ch->in_room->vnum);
			else
				sprintf(buf2, " ");

			i = buf2;
			break;

		case 'z':
			if (ch->in_room != NULL)
				sprintf(buf2, "%s", ch->in_room->area->name);
			else
				sprintf(buf2, " ");

			i = buf2;
			break;

		case 't':
			if (ch->in_room != NULL)
				sprintf(buf2, "%s", sector_lookup(ch->in_room->sector_type));
			else
				sprintf(buf2, " ");

			i = buf2;
			break;

		case 'q':
			if (!IS_QUESTOR(ch))
				sprintf(buf2, "%d", ch->nextquest);
			else
				sprintf(buf2, "%d", ch->countdown);

			i = buf2;
			break;

		case 'Q':
			if (IS_QUESTOR(ch)) {
				OBJ_INDEX_DATA *questinfoobj;
				MOB_INDEX_DATA *questinfo;

				if (ch->questmob == -1 || ch->questobf == -1)
					sprintf(buf2, "*report!*");
				else if (ch->questobj > 0) {
					if ((questinfoobj = get_obj_index(ch->questobj)) != NULL)
						sprintf(buf2, "%s", questinfoobj->name);
					else
						sprintf(buf2, "Unknown");
				}
				else if (ch->questmob > 0) {
					if ((questinfo = get_mob_index(ch->questmob)) != NULL)
						sprintf(buf2, "%s", questinfo->short_descr);
					else
						sprintf(buf2, "Unknown");
				}
				else
					sprintf(buf2, "Unknown");
			}
			else
				sprintf(buf2, "None");

			i = smash_bracket(buf2);
			break;

		case 'p':
			if (!IS_NPC(ch))
				sprintf(buf2, "%d", ch->questpoints);

			i = buf2;
			break;

		case 'j':
			if (!IS_NPC(ch)) {
				if (!IS_SQUESTOR(ch))
					sprintf(buf2, "%d", ch->pcdata->nextsquest);
				else
					sprintf(buf2, "%d", ch->pcdata->sqcountdown);
			}
			else
				sprintf(buf2, "0");

			i = buf2;
			break;

		case 'J':
			if (IS_SQUESTOR(ch)) {
				if (ch->pcdata->squestobj != NULL && ch->pcdata->squestmob == NULL) {
					if (!ch->pcdata->squestobjf)
						sprintf(buf2, "%s", ch->pcdata->squestobj->short_descr);
					else
						sprintf(buf2, "*report!*");
				}
				else if (ch->pcdata->squestmob != NULL && ch->pcdata->squestobj == NULL) {
					if (!ch->pcdata->squestmobf)
						sprintf(buf2, "%s", ch->pcdata->squestmob->short_descr);
					else
						sprintf(buf2, "*report!*");
				}
				else if (ch->pcdata->squestobj != NULL && ch->pcdata->squestmob != NULL) {
					if (ch->pcdata->squestobjf) {
						if (!ch->pcdata->squestmobf)
							sprintf(buf2, "%s", ch->pcdata->squestmob->short_descr);
						else
							sprintf(buf2, "*report!*");
					}
					else
						sprintf(buf2, "%s", ch->pcdata->squestobj->short_descr);
				}
				else
					sprintf(buf2, "Unknown");
			}
			else
				sprintf(buf2, "None");

			i = smash_bracket(buf2);
			break;

		case 'k':
			if (!IS_NPC(ch))
				sprintf(buf2, "%d", ch->pcdata->skillpoints);

			i = buf2;
			break;

		case 'K':
			if (!IS_NPC(ch))
				sprintf(buf2, "%d", ch->pcdata->rolepoints);

			i = buf2;
			break;

		case 'w':
			if (IS_IMMORTAL(ch)) {
				if (!ch->invis_level && !ch->lurk_level)
					sprintf(buf2, "VIS");
				else
					sprintf(buf2, "%d/%d", ch->invis_level, ch->lurk_level);

				if (IS_SET(ch->act, PLR_SUPERWIZ))
					sprintf(buf2, "WIZ");
			}
			else
				sprintf(buf2, " ");

			i = buf2;
			break;

		case 'C':
			++str;

			switch (*str) {
			default:        color = WHITE;  bold = NOBOLD;  break;

			case 'W':       color = WHITE;  bold = BOLD;    break;

			case 'g':       color = WHITE;  bold = NOBOLD;  break;

			case 'P':       color = RED;    bold = BOLD;    break;

			case 'R':       color = RED;    bold = NOBOLD;  break;

			case 'Y':       color = YELLOW; bold = BOLD;    break;

			case 'b':       color = YELLOW; bold = NOBOLD;  break;

			case 'G':       color = GREEN;  bold = BOLD;    break;

			case 'H':       color = GREEN;  bold = NOBOLD;  break;

			case 'C':       color = CYAN;   bold = BOLD;    break;

			case 'T':       color = CYAN;   bold = NOBOLD;  break;

			case 'B':       color = BLUE;   bold = BOLD;    break;

			case 'N':       color = BLUE;   bold = NOBOLD;  break;

			case 'V':       color = PURPLE; bold = BOLD;    break;

			case 'M':       color = PURPLE; bold = NOBOLD;  break;

			case 'c':       color = GREY;   bold = BOLD;    break;

			case 'k':       color = GREY;   bold = NOBOLD;  break;
			}

			if (IS_SET(ch->act, PLR_COLOR))
				sprintf(buf2, "\033[%d;%dm", bold, color);
			else
				sprintf(buf2, " ");

			i = buf2;
			break;

		/* added by Outsider to allow a character to see
		   his/her level in the prompt */
		case 'l':
			sprintf(buf2, "%d", ch->level);
			i = buf2;
			break;

		case '%':
			sprintf(buf2, "%%");
			i = buf2;
			break;
		}

		++str;

		while ((*point = *i) != '\0')
			++point, ++i;
	}

	write_to_buffer(ch->desc, buf, point - buf);

	if (ch->prefix[0] != '\0')
		write_to_buffer(ch->desc, ch->prefix, 0);
} /* end bust_a_prompt() */


/* write_to_buffer with color codes -- Montrey */
void cwtb(DESCRIPTOR_DATA *d, char *txt)
{
	char *a, *b;
	int length, l, curlen = 0;
	a = txt;
	length = strlen(a);

	if (a != NULL) {
		while (curlen < length) {
			for (b = a, l = 0; curlen < length && *a != '{'; l++, curlen++, a++)
				;

			if (l > 0)
				write_to_buffer(d, b, l);

			if (*a) {
				a++;
				curlen++;

				if (curlen < length) {
					char code[35];
					bool found = TRUE;

					switch (*a++) {
					case 'x': sprintf(code, "%s%s",
						                  CLEAR, B_BLACK);        break;

					case 'N': sprintf(code, C_BLUE);        break;

					case 'T': sprintf(code, C_CYAN);        break;

					case 'H': sprintf(code, C_GREEN);       break;

					case 'k': sprintf(code, C_BLACK);       break;

					case 'M': sprintf(code, C_MAGENTA);     break;

					case 'R': sprintf(code, C_RED);         break;

					case 'g': sprintf(code, C_WHITE);       break;

					case 'b': sprintf(code, C_YELLOW);      break;

					case 'B': sprintf(code, C_B_BLUE);      break;

					case 'C': sprintf(code, C_B_CYAN);      break;

					case 'G': sprintf(code, C_B_GREEN);     break;

					case 'V': sprintf(code, C_B_MAGENTA);   break;

					case 'P': sprintf(code, C_B_RED);       break;

					case 'W': sprintf(code, C_B_WHITE);     break;

					case 'Y': sprintf(code, C_B_YELLOW);    break;

					case 'c': sprintf(code, C_B_GREY);      break;

					case 's': sprintf(code, C_REVERSE);     break;

					case 'e': sprintf(code, B_GREY);        break;

					case 'r': sprintf(code, B_RED);         break;

					case 'y': sprintf(code, B_YELLOW);      break;

					case 'h': sprintf(code, B_GREEN);       break;

					case 't': sprintf(code, B_CYAN);        break;

					case 'n': sprintf(code, B_BLUE);        break;

					case 'm': sprintf(code, B_MAGENTA);     break;

					case 'a': sprintf(code, B_BLACK);       break;

					case '{': strcpy(code, "{");            break;

					default:  found = FALSE;                break;
					}

					if (found)
						write_to_buffer(d, code, strlen(code));

					curlen++;
				}
				else {
					a++;
					curlen++;
				}
			}
		}
	}
}

/*
 * Append onto an output buffer.
 */
void write_to_buffer(DESCRIPTOR_DATA *d, const char *txt, int length)
{
	if (d == NULL)
		return;

	/*
	 * Find length in case caller didn't.
	 */
	if (length <= 0)
		length = strlen(txt);

	/*
	 * Initial \n\r if needed.
	 */
	if (d->outtop == 0 && !d->fcommand) {
		d->outbuf[0]    = '\n';
		d->outbuf[1]    = '\r';
		d->outtop       = 2;
	}

	/*
	 * Expand the buffer as needed.
	 */
	while (d->outtop + length >= d->outsize) {
		char *outbuf;

		if (d->outsize >= 64000) {
			char *culprit;
			char buf[MAX_INPUT_LENGTH];
			culprit = d->original ? d->original->name : d->character->name;
			sprintf(buf, "Buffer overflow. Closing connection to %s.\n\r",
			        culprit ? culprit : "culprit");
			close_socket(d);
			bug(buf, 0);
			return;
		}

		outbuf      = alloc_mem(2 * d->outsize);
		strncpy(outbuf, d->outbuf, d->outtop);
		free_mem(d->outbuf, d->outsize);
		d->outbuf   = outbuf;
		d->outsize *= 2;
	}

	/*
	 * Copy.
	 */
	strncpy(d->outbuf + d->outtop, txt, length);
	d->outtop += length;
	d->outbuf[d->outtop] = '\0';
	return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor(int desc, char *txt, int length)
{
	int iStart;
	int nWrite;
	int nBlock;

	if (length <= 0)
		length = strlen(txt);

	for (iStart = 0; iStart < length; iStart += nWrite) {
		nBlock = UMIN(length - iStart, 4096);

		if ((nWrite = write(desc, txt + iStart, nBlock)) < 0) {
			perror("Write_to_descriptor");

			if (errno == EBADF) {
				char *nullptr = NULL;

				if (*nullptr != '\0') abort();
			}

			return FALSE;
		}
	}

	return TRUE;
}



/*
 * Parse a name for acceptability.
 */
bool check_parse_name(char *name)
{
	CLAN_DATA *clan;

	/*
	 * Reserved words.
	 */
	if (is_name(name,
	            "all auto immortal self remort imms private someone something the you"))
		return FALSE;

	if ((clan = clan_lookup(name)) != NULL)
		return FALSE;

	/*
	 * Length restrictions.
	 */

	if (strlen(name) <  2)
		return FALSE;

#if defined(unix)

	if (strlen(name) > 12)
		return FALSE;

#endif
	/*
	 * Alphanumerics only.
	 * Lock out IllIll twits.
	 */
	{
		char *pc;
		bool fIll, adjcaps = FALSE, cleancaps = FALSE;
		int total_caps = 0;
		fIll = TRUE;

		for (pc = name; *pc != '\0'; pc++) {
			if (!isalpha(*pc))
				return FALSE;

			if (isupper(*pc)) { /* ugly anti-caps hack */
				if (adjcaps)
					cleancaps = TRUE;

				total_caps++;
				adjcaps = TRUE;
			}
			else
				adjcaps = FALSE;

			if (LOWER(*pc) != 'i' && LOWER(*pc) != 'l')
				fIll = FALSE;
		}

		if (fIll)
			return FALSE;

		if (cleancaps || (total_caps > (strlen(name)) / 2 && strlen(name) < 3))
			return FALSE;
	}
	/*
	 * Prevent players from naming themselves after mobs.
	 */
	/* Yeah, but do it somewhere else -- Elrac
	{
	    extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	    MOB_INDEX_DATA *pMobIndex;
	    int iHash;

	    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	    {
	        for ( pMobIndex  = mob_index_hash[iHash];
	              pMobIndex != NULL;
	              pMobIndex  = pMobIndex->next )
	        {
	            if ( is_name( name, pMobIndex->player_name ) )
	                return FALSE;
	        }
	    }
	}
	*/
	return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect(DESCRIPTOR_DATA *d, char *name, bool fConn)
{
	CHAR_DATA *ch;
	ROOM_INDEX_DATA *room;

	for (ch = char_list; ch != NULL; ch = ch->next) {
		if (!IS_NPC(ch)
		    && d->character != ch
		    && (!fConn || ch->desc == NULL)
		    &&   !str_cmp(d->character->name, ch->name)) {
			if (fConn == FALSE) {
				free_string(d->character->pcdata->pwd);
				d->character->pcdata->pwd = str_dup(ch->pcdata->pwd);
			}
			else {
				CHAR_DATA *rch;
				free_char(d->character);
				d->character = ch;
				ch->desc         = d;
				ch->desc->timer  = 0;
				stc("Reconnecting...\n\r", ch);

				if (!IS_NPC(ch))
					if (ch->pcdata->buffer->string[0] != '\0')
						stc("You have messages: Type 'replay'\n\r", ch);

				for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
					if (ch != rch && can_see(rch, ch))
						ptc(rch, "%s has reconnected.\n\r", PERS(ch, rch, VIS_CHAR));

				sprintf(log_buf, "%s@%s reconnected.", ch->name, d->host);
				log_string(log_buf);
				wiznet("$N reclaims the fullness of $S link.",
				       ch, NULL, WIZ_LINKS, 0, 0);

				if ((room = ch->in_room) != NULL) {
					char_from_room(ch);
					char_to_room(ch, room);
				}

				REMOVE_BIT(ch->pcdata->plr, PLR_LINK_DEAD);
				d->connected = CON_PLAYING;
			}

			return TRUE;
		}
	}

	return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing(DESCRIPTOR_DATA *d, char *name)
{
	DESCRIPTOR_DATA *dold;

	for (dold = descriptor_list; dold; dold = dold->next) {
		if (dold != d
		    &&   dold->character != NULL
		    &&   dold->connected != CON_GET_NAME
		    &&   dold->connected != CON_GET_OLD_PASSWORD
		    &&   !str_cmp(name, dold->original
		                  ? dold->original->name : dold->character->name)) {
			write_to_buffer(d, "That character is already playing.\n\r", 0);
			write_to_buffer(d, "Do you wish to connect anyway (Y/N)?", 0);
			d->connected = CON_BREAK_CONNECT;
			return TRUE;
		}
	}

	return FALSE;
}
bool check_player_exist(DESCRIPTOR_DATA *d, char *name)
{
	DESCRIPTOR_DATA *dold;
	STORAGE_DATA *exist = NULL;    /* is character in storage */

	for (dold = descriptor_list; dold; dold = dold->next) {
		if (dold != d
		    &&   dold->character != NULL
		    &&   dold->character->level < 1
		    &&   dold->connected != CON_PLAYING
		    &&   !str_cmp(name, dold->original
		                  ? dold->original->name : dold->character->name)) {
			write_to_buffer(d,
			                "A character by that name is currently being created.\n\r"
			                "You cannot access that character.\n\r"
			                "Please create a character with a different name, and\n\r"
			                "ask an Immortal for help if you need it.\n\r"
			                "\n\r"
			                "Name: ", 0);
			d->connected = CON_GET_NAME;
			return TRUE;
		}
	}

	/* make sure that we do not re-create a character currently
	   in storage -- Outsider <slicer69@hotmail.com>
	*/
	/* first make sure we have the list of stored characters */
	if (! storage_list_head)
		load_storage_list();

	/* search storage for character name */
	exist = lookup_storage_data(name);

	if (exist) {
		write_to_buffer(d,
		                "A character by that name is currently in storage.\n\r"
		                "You cannot create a character by this name.\n\r"
		                "Please create a character with a different name, and\n\r"
		                "ask an Immortal for help if you need it.\n\r"
		                "\n\r"
		                "Name: ", 0);
		d->connected = CON_GET_NAME;
		return TRUE;
	}

	return FALSE;
}


void stop_idling(CHAR_DATA *ch)
{
	if (ch == NULL
	    ||   !IS_PLAYING(ch->desc)
	    ||   ch->was_in_room == NULL
	    ||   ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
		return;

	ch->desc->timer = 0;
	char_from_room(ch);
	char_to_room(ch, ch->was_in_room);
	ch->was_in_room     = NULL;
	act("$n has returned from the void.", ch, NULL, NULL, TO_ROOM);
	return;
}


void process_color(CHAR_DATA *ch, char a)
{
	char code[35];

	if (IS_NPC(ch))
		return;

	switch (a) {
	case 'x':
		sprintf(code, "%s%s\033[%d;%dm", CLEAR, B_BLACK,
		        ch->pcdata->lastcolor[1],
		        ch->pcdata->lastcolor[0]);
		break;

	case 'N':
		sprintf(code, C_BLUE);
		break;

	case 'T':
		sprintf(code, C_CYAN);
		break;

	case 'H':
		sprintf(code, C_GREEN);
		break;

	case 'k':
		strcpy(code, C_BLACK);

		if (ch->pcdata && IS_SET(ch->pcdata->video, VIDEO_DARK_MOD))
			strcpy(code, C_WHITE);

		break;

	case 'M':
		sprintf(code, C_MAGENTA);
		break;

	case 'R':
		sprintf(code, C_RED);
		break;

	case 'g':
		sprintf(code, C_WHITE);
		break;

	case 'b':
		sprintf(code, C_YELLOW);
		break;

	case 'B':
		sprintf(code, C_B_BLUE);
		break;

	case 'C':
		sprintf(code, C_B_CYAN);
		break;

	case 'G':
		sprintf(code, C_B_GREEN);
		break;

	case 'V':
		sprintf(code, C_B_MAGENTA);
		break;

	case 'P':
		sprintf(code, C_B_RED);
		break;

	case 'W':
		sprintf(code, C_B_WHITE);
		break;

	case 'Y':
		sprintf(code, C_B_YELLOW);
		break;

	case 'c':
		strcpy(code, C_B_GREY);

		if (ch->pcdata && IS_SET(ch->pcdata->video, VIDEO_DARK_MOD))
			strcpy(code, C_WHITE);

		break;

	case 'f':
		strcpy(code, "");
		sprintf(code, C_FLASH);

		if (ch->pcdata) {
			if (IS_SET(ch->pcdata->video, VIDEO_FLASH_OFF))
				strcpy(code, "");

			if (IS_SET(ch->pcdata->video, VIDEO_FLASH_LINE))
				strcat(code, C_UNDERLINE);
		}

		break;

	case 's':
		sprintf(code, C_REVERSE);
		break;

	case 'e':
		sprintf(code, B_GREY);
		break;

	case 'r':
		sprintf(code, B_RED);
		break;

	case 'y':
		sprintf(code, B_YELLOW);
		break;

	case 'h':
		sprintf(code, B_GREEN);
		break;

	case 't':
		sprintf(code, B_CYAN);
		break;

	case 'n':
		sprintf(code, B_BLUE);
		break;

	case 'm':
		sprintf(code, B_MAGENTA);
		break;

	case 'a':
		sprintf(code, B_BLACK);
		break;

	case '{':
		strcpy(code, "{");
		break;

	default:
		return;
	}

	write_to_buffer(ch->desc, code, strlen(code));
}

/*
 * Write to one char.
 */
void stc(char *txt, CHAR_DATA *ch)
{
	char *a, *b;
	int length, l, curlen = 0;
	a = txt;
	length = strlen(txt);

	if (txt != NULL && ch->desc != NULL) {
		while (curlen < length) {
			b = a;
			l = 0;

			while (curlen < length && *a != '{') {
				l++;
				curlen++;
				a++;
			}

			if (l > 0)
				write_to_buffer(ch->desc, b, l);

			if (*a) {
				if (!IS_NPC(ch) && IS_SET(ch->pcdata->video, VIDEO_CODES_SHOW)) {
					process_color(ch, 'x');
					write_to_buffer(ch->desc, a, 2);
				}

				a++;
				curlen++;

				if (curlen < length && IS_SET(ch->act, PLR_COLOR2)) {
					process_color(ch, *a++);
					curlen++;
				}
				else {
					a++;
					curlen++;
				}
			}
		}
	}
} /* end stc() */


/*
 * Send a page to one char.
 */
void page_to_char(char *txt, CHAR_DATA *ch)
{
	if (txt == NULL /*|| ch->desc == NULL (this is screwing up doas, we'll trust the caller*/)
		return;

	/* The old method - Lotus
	    ch->desc->showstr_head = alloc_mem(strlen(txt) + 1);
	    strcpy(ch->desc->showstr_head,txt);
	    ch->desc->showstr_point = ch->desc->showstr_head;
	    show_string(ch->desc,"");
	*/
	if (ch->desc->showstr_head &&
	    (strlen(txt) + strlen(ch->desc->showstr_head) + 1) < 32000) {
		char *temp = alloc_mem(strlen(txt) + strlen(ch->desc->showstr_head) + 1);
		strcpy(temp, ch->desc->showstr_head);
		strcat(temp, txt);
		ch->desc->showstr_point = temp +
		                          (ch->desc->showstr_point - ch->desc->showstr_head);
		free_mem(ch->desc->showstr_head, strlen(ch->desc->showstr_head) + 1);
		ch->desc->showstr_head = temp;
	}
	else {
		if (ch->desc->showstr_head)
			free_mem(ch->desc->showstr_head, strlen(ch->desc->showstr_head) + 1);

		ch->desc->showstr_head = alloc_mem(strlen(txt) + 1);
		strcpy(ch->desc->showstr_head, txt);
		ch->desc->showstr_point = ch->desc->showstr_head;
		show_string(ch->desc, "");
	}
}


/* string pager */
void show_string(struct descriptor_data *d, char *input)
{
	char buffer[4 * MAX_STRING_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	register char *scan;
	int lines = 0, toggle = 1;
	int show_lines;

	// get the first word from the input
	one_argument(input, buf);

	// if nothing, done paging output, clear the remainders
	if (buf[0] != '\0') {
		if (d->showstr_head) {
			free_string(d->showstr_head);
			d->showstr_head = 0;
		}

		d->showstr_point  = 0;
		return;
	}

	// how many lines per page?
	if (d->character)
		show_lines = d->character->lines;
	else
		show_lines = 0;


	for (scan = buffer; ; scan++, d->showstr_point++) {
		// copy each line to the buffer
		// toggle apparantly expecting all lines to end with \n and \r?
		if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')
		    && (toggle = -toggle) < 0)
			lines++;
		else if (!*scan || (show_lines > 0 && lines >= show_lines)) {
			// cut off and send the output part
			*scan = '\0';

			if (d-> character)
				stc(buffer, d-> character);
			else
				write_to_buffer(d, buffer, strlen(buffer));

			// remove leading spaces from remainder
			// why strip spaces? - Montrey
/*
			// and the semicolon following this looks like a bug,
			// but it broke when i 'fixed' it - Montrey
			char *chk;
	
			for (chk = d->showstr_point; isspace(*chk); chk++);

			{
				if (!*chk) {
					if (d->showstr_head) {
						free_string(d->showstr_head);
						d->showstr_head = 0;
					}

					d->showstr_point  = 0;
				}
			}
*/
			return;
		}
	}
} /* end show_string() */



void act(const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2,
         int type)
{
	/* to be compatible with older code */
	act_new(format, ch, arg1, arg2, type, POS_RESTING, FALSE);
}

/* special act for xsocials, to work with censor */
void xact(const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type)
{
	act_new(format, ch, arg1, arg2, type, POS_RESTING, TRUE);
}

/* the guts of act_new, taken out to reduce complexity. */
void act_format(const char *format, CHAR_DATA *ch,
                CHAR_DATA *vch, CHAR_DATA *vch2,
                const void *arg1, const void *arg2,
                OBJ_DATA *obj1, OBJ_DATA *obj2,
                CHAR_DATA *to, bool snooper, int vis)
{
	static char *const he_she  [] = { "it",  "he",  "she" };
	static char *const him_her [] = { "it",  "him", "her" };
	static char *const his_her [] = { "its", "his", "her" };
	char buf[MAX_STRING_LENGTH];
	char fname[MAX_INPUT_LENGTH];
	const char *str;
	const char *i;
	char *point;
	char dollarmsg[3];
	point   = buf;

	if (snooper) {
		strcpy(point, "{n[T] ");
		point += 6;
	}

	str     = format;

	while (*str != '\0') {
		if (*str != '$') {
			*point++ = *str++;
			continue;
		}

		/* '$' sign after this point */
		++str;
		sprintf(dollarmsg, "$%c", *str);
		i = dollarmsg;

		switch (*str) {
		default:                                                break;

		/* The following codes need 'ch', which should always be OK */

		case 'n': i = PERS(ch, to, vis);            break;

		case 'e': i = he_she  [GET_SEX(ch)];        break;

		case 'm': i = him_her [GET_SEX(ch)];        break;

		case 's': i = his_her [GET_SEX(ch)];        break;

		/* The following codes need 'vch' */

		case 'N':
			if (vch == NULL || !valid_character(vch)) {
				bug("Missing vch for '$$N'", 0);
				/*   bug( format, 0);  This will cause an endless loop */
			}
			else
				i = PERS(vch, to, vis);

			break;

		case 'E':
			if (vch == NULL || !valid_character(vch))
				bug("Missing vch for '$$E'", 0);
			else
				i = he_she[GET_SEX(vch)];

			break;

		case 'M':
			if (vch == NULL || !valid_character(vch))
				bug("Missing vch for '$$M'", 0);
			else
				i = him_her[GET_SEX(vch)];

			break;

		case 'S':
			if (vch == NULL || !valid_character(vch))
				bug("Missing vch for '$$S'", 0);
			else
				i = his_her[GET_SEX(vch)];

			break;

		/* The following codes need valid objects in obj1/obj2 */

		case 'p':
			if (obj1 == NULL || !valid_object(obj1))
				bug("Missing obj1 for '$$p'", 0);
			else if (can_see_obj(to, obj1))
				i = obj1->short_descr;
			else
				i = "something";

			break;

		case 'P':
			if (obj2 == NULL || !valid_object(obj2)) {
				bug("Missing obj2 for '$$P'", 0);
				bug(format, 0);
			}
			else if (can_see_obj(to, obj2))
				i = obj2->short_descr;
			else
				i = "something";

			break;

		/* The following needs a string describing a door. */

		case 'd':
			if (arg2 == NULL || ((char *) arg2)[0] == '\0')
				i = "door";
			else {
				one_argument((char *) arg2, fname);
				i = fname;
			}

			break;

		/* The following codes need valid strings in arg1/arg2 */

		case 't':
			if (arg1 == NULL)
				bug("Missing arg1 for '$$t'", 0);
			else
				i = (char *) arg1;

			break;

		case 'T':
			if (arg2 == NULL)
				bug("Missing arg2 for '$$T'", 0);
			else
				i = (char *) arg2;

			break;

		/* The following codes need no checking */

		case 'G': i = "\007";                                   break;

		case '$': i = "$";                                      break;
		}

		++str;

		while ((*point = *i) != '\0')
			++point, ++i;
	}

	if (snooper) {
		*point++ = '{';
		*point++ = 'x';
	}

	*point++ = '\n';
	*point++ = '\r';
	*point   = 0;
	buf[0]   = UPPER(buf[0]);

	if (to->desc)
		stc(buf, to);

	if (MOBtrigger)
		mprog_act_trigger(buf, to, ch, obj1, vch);
} /* end act_format() */


void act_new(const char *format, CHAR_DATA *ch, const void *arg1,
             const void *arg2, int type, int min_pos, bool censor)
{
	ARENA_DATA *arena = arena_table_head->next;
	CHAR_DATA *to;
	CHAR_DATA *vch = (CHAR_DATA *) arg2;
	CHAR_DATA *vch2 = (CHAR_DATA *) arg1;
	OBJ_DATA *obj1 = (OBJ_DATA *) arg1;
	OBJ_DATA *obj2 = (OBJ_DATA *) arg2;
	bool SNEAKING = FALSE;
	TAIL_DATA *td;
	char fake_message[MAX_INPUT_LENGTH];
	int vis = VIS_CHAR;

	/*
	 * Discard null and zero-length messages.
	 */
	if (format == NULL || format[0] == '\0')
		return;

	/* discard null rooms and chars */
	if (ch == NULL || ch->in_room == NULL)
		return;

	to = ch->in_room->people;

	if (min_pos == POS_SNEAK) {
		min_pos = POS_RESTING;
		SNEAKING = TRUE;
	}

	/* blah, special hack for channel visibility.  rewrite this crap someday, i don't
	   have time right now to give act the attention it needs.  -- Montrey */
	if (type == TO_VICT_CHANNEL) {
		type = TO_VICT;
		vis = VIS_PLR;
	}

	if (type == TO_VICT) {
		if (vch == NULL) {
			bug("Act: null vch with TO_VICT.", 0);
			return;
		}

		if (vch->in_room == NULL)
			return;

		to = vch->in_room->people;
	}

	if (type == TO_WORLD) {
		if (vch2 == NULL) {
			bug("Act: null vch2 with TO_WORLD.", 0);
			return;
		}

		if (vch2->in_room == NULL && ch->tail == NULL)
			return;

		to = vch2->in_room->people;
	}

	/*** first loop, for normal recipients of ACT */
	for (; to != NULL; to = to->next_in_room) {
		if (censor && (IS_NPC(to) || IS_SET(to->censor, CENSOR_XSOC)))
			continue;

		if (get_position(to) < min_pos)
			continue;

		if (SNEAKING) {
			if (!IS_IMMORTAL(to)) /* eliminates mobs too */
				continue;

			if (IS_SET(ch->act, PLR_SUPERWIZ) && !IS_IMP(to))
				continue;
		}

		if ((type == TO_CHAR) && to != ch)
			continue;

		if (type == TO_VICT && (to != vch || to == ch))
			continue;

		if (type == TO_ROOM && to == ch)
			continue;

		if (type == TO_NOTVICT && (to == ch || to == vch))
			continue;

		if (type == TO_WORLD && (to == ch || to == vch || to != vch2))
			continue;

		if (type == TO_NOTVIEW && to == ch) /* same as TO_ROOM */
			continue;

		if (type == TO_VIEW)
			continue;

		/**********************************************************************/
		act_format(format, ch, vch, vch2, arg1, arg2, obj1, obj2, to, FALSE, vis);
		/**********************************************************************/
	}

	/* viewing room stuff */
	if (!censor && (type == TO_ROOM || type == TO_NOTVICT || type == TO_VIEW)) {
		while (arena != arena_table_tail) {
			if (ch->in_room->vnum >= arena->minvnum
			    && ch->in_room->vnum <= arena->maxvnum)
				break;

			arena = arena->next;
		}

		if (arena != arena_table_tail && arena->viewroom->people != NULL) {
			sprintf(fake_message, "{Y[V]{x %s", format);
			format = fake_message;

			for (to = arena->viewroom->people; to != NULL; to = to->next_in_room) {
				if (get_position(to) < min_pos)
					continue;

				/**********************************************************************/
				act_format(format, ch, vch, vch2, arg1, arg2, obj1, obj2, to, FALSE, vis);
				/**********************************************************************/
			}
		}
	}

	/* TAIL stuff -- Elrac */
	if (ch->tail == NULL)
		return;

	if (type != TO_ROOM && type != TO_NOTVICT && type != TO_WORLD && type != TO_NOTVIEW)
		return;

	if (!str_prefix1("$n says '", format)
	    || !str_prefix1("$n leaves ", format))
		return;

	if (!str_prefix1("$n has arrived.", format)) {
		sprintf(fake_message, "$n has arrived at %s (%s).",
		        ch->in_room->name, ch->in_room->area->file_name);
		format = fake_message;
	}

	/* check integrity of tailer. untail if bad. */
	for (td = ch->tail; td;) {
		if (!IS_VALID(td->tailed_by)
		    || str_cmp(td->tailer_name, td->tailed_by->name)) {
			set_tail(td->tailed_by, ch, 0);
			td = ch->tail;
			continue;
		}

		td = td->next;
	}

	/*** second loop, for tailers ***/
	for (td = ch->tail; td; td = td->next) {
		/* check if tailer in room with actor */
		for (to = ch->in_room->people; to; to = to->next_in_room) {
			if (to == td->tailed_by)
				break;
		}

		if (to)
			continue;

		/**********************************************************************/
		act_format(format, ch, vch, vch2, arg1, arg2, obj1, obj2,
		           td->tailed_by, TRUE, vis);
		/**********************************************************************/
	}

	/* Add this to turn Mob Programs Off
	                MOBtrigger = FALSE;
	Add before the call to act_new */
	MOBtrigger = TRUE;
	return;
}


char *get_multi_command(DESCRIPTOR_DATA *d, char *argument)
{
	char *pcom;
	pcom = command;

	while (*argument != '\0') {
		if (*argument == '|') {
			if (*++argument != '|') {
				strcpy(d->incomm, argument);
				*pcom = '\0';
				return command;
			}
		}

		*pcom++ = *argument++;
	}

	*pcom = '\0';
	d->incomm[0] = '\0';
	return command;
} /* end get_multi_command() */


int roll_stat(CHAR_DATA *ch, int stat)
{
	int percent, bonus, temp, low, high;
	percent = number_percent();

	if (percent > 99)
		bonus = 2;
	else if (percent > 95)
		bonus = 1;
	else if (percent < 5)
		bonus = -1;
	else
		bonus = 0;

	high = pc_race_table[ch->race].max_stats[stat] - (3 - bonus);
	low = pc_race_table[ch->race].stats[stat] - (3 - bonus);
	temp = (number_range(low, high));
	return temp;
}



/* COPYOVER stuff is so very system dependent I decided
   to move it here from act_wiz.c, which is very full anyway.
   -- Elrac
*/

void do_copyove(CHAR_DATA *ch)
{
	ptc(ch, "You have to spell out COPYOVER if you want to copyover. Silly %s.\n", ch->name);
	return;
}

void do_copyover(CHAR_DATA *ch, char *argument)
{
	FILE *fp;
	DESCRIPTOR_DATA *d, *d_next;
	char buf [100], buf3[100];
	do_allsave(ch, "");
	save_clan_table();

	if ((fp = fopen(COPYOVER_FILE, "w")) == NULL) {
		stc("Copyover file not writeable, aborted.\n\r", ch);
		sprintf(buf3, "Could not write to copyover file: %s", COPYOVER_FILE);
		log_string(buf3);
		perror("do_copyover:fopen");
		return;
	}

	/* save the socket state of all active players, only */
	for (d = descriptor_list; d; d = d->next) {
		printf("found socket %d, host %s, conn %d\n", d->descriptor, d->host, d->connected);

		if (IS_PLAYING(d)) {
			CHAR_DATA *och = CH(d);
			one_argument(och->name, buf);
			fprintf(fp, "%d %s %s\n", d->descriptor, buf, d->host);
		}
	}

	fprintf(fp, "-1\n");
	fclose(fp);

	if ((fp = fopen(COPYOVER_LOG, "w")) == NULL) {
		stc("Copyover file not writeable, aborted.\n\r", ch);
		sprintf(buf3, "Could not write to copyover file: %s", COPYOVER_LOG);
		log_string(buf3);
		perror("do_copyover:fopen");
		return;
	}

	fprintf(fp, "%s~\n", ch->name);
	fclose(fp);

	/*****/
	if (port == DIZZYPORT)
		save_items();

	/*****/

	/* yes, this is ugly, but i don't think it warrants a place in pcdata */
	if (!str_cmp(ch->name, "Montrey")) {
		sprintf(buf, "\n\rA beam of pure white light arcs down from the heavens, striking\n\r"
		        "the earth just beyond the horizon.  The ground starts to shake,\n\r"
		        "and a curtain of fiery destruction sweeps over the land, clearing\n\r"
		        "away the old in preparation for the new.\n\r");
	}
	else if (!str_cmp(ch->name, "Outsider")) {
		sprintf(buf, "\n\rAn angel, clothed in white light, lands before you. He stands\n\r"
		        "before you, blocking your view of the rest of the world. His wings\n\r"
		        "wrap about you in a soft, feathery embrace. All around you hear screams\n\r"
		        "and loud cracking sounds, as if the world is turning inside out.\n\r");
	}
	else if (!str_cmp(ch->name, "Xenith")) {
		sprintf(buf, "\n\rThe winds suddenly spring up, as the storm destroys the world.\n"
		        "Your body becomes dust before the maelstrom.\n"
		        "Your vision dissapates as the universe vanishes in a flash.\n");
	}
	else if (!str_cmp(ch->name, "Liriel")) {
		sprintf(buf, "\n\rA tiny faerie shimmers into existence in front of you, it's magics quickly\n\r"
		        "wrapping around you to form a protective barrier ... You watch in awe as the\n\r"
		        "world around you blurs and distorts, reality as you know it washing away.\n\r");
	}
	else if (!str_cmp(ch->name, "Kenneth")) {
		sprintf(buf, "\n\rA casual tap on the shoulder causes you to spin around.  Directly in front\n\r"
		        "of you is a plain, unassuming man who gently covers your eyes.\n\r");
	}
	else
		sprintf(buf, "\n\r*** COPYOVER by %s - please remain seated!\n\r", ch->name);

	/* For each playing descriptor, save its state */
	for (d = descriptor_list; d ; d = d_next) {
		CHAR_DATA *och;
		d_next = d->next;

		if (d->connected < 0) {
			/* we don't know what this descriptor is, just close it */
			printf("closing descriptor %d, conn stat %d\n",
			       d->descriptor, d->connected);
			close(d->descriptor);
		}
		else if (d->connected != CON_PLAYING) {
			/* drop those logging on */
			printf("closing socket %d from host %s\n",
			       d->descriptor, d->host);
			write_to_descriptor(d->descriptor,
			                    "\n\rSorry, we are rebooting. Come back in a minute.\n\r",
			                    0);
			close_socket(d);  /* throw'em out */
		}
		else {
			/* regular character -- save and notify */
			och =  CH(d);
			printf("closing socket %d from char %s\n",
			       d->descriptor, och->name);
			save_char_obj(och);
			write_to_descriptor(d->descriptor, buf, 0);
		}
	}

	/* Close reserve and other always-open files
	                and release other resources */
#if defined(SAND)
	sand_close();
#endif
	/* close the database */
	db_close();

	/* exec - descriptors are inherited */
	sprintf(buf,  "%d", port);
	sprintf(buf3, "%d", control);
	execl(EXE_FILE, "legacy", buf, "null", "copyover", buf3, "null", (char*)0);
	/* Failed - sucessful exec will not return */
	perror("do_copyover: execl");
	stc("Copyover FAILED!\n\r", ch);
}
