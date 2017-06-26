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

#include <arpa/inet.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>

#include "act.hh"
#include "argument.hh"
#include "Area.hh"
#include "channels.hh"
#include "Character.hh"
#include "Clan.hh"
#include "declare.hh"
#include "Descriptor.hh"
#include "Exit.hh"
#include "file.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "interp.hh"
#include "lookup.hh"
#include "Logging.hh"
#include "macros.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "Player.hh"
#include "random.hh"
#include "RoomPrototype.hh"
#include "sql.hh"
#include "String.hh"
#include "vt100.hh" /* VT100 Stuff */

#include "control/PlayerController.hh"

struct ka_struct;

/* EPKA structure */
struct ka_struct *ka;

extern void     goto_line    args((Character *ch, int row, int column));
extern void     set_window   args((Character *ch, int top, int bottom));
extern void     roll_raffects   args((Character *ch, Character *victim));

/*
 * Signal handling.
 */

String exe_file; // the executable this process booted from

/*
 * Global variables.
 */
Descriptor    *descriptor_list;    /* All open descriptors         */
Descriptor    *d_next;             /* Next descriptor in loop      */
bool                god;                /* All new chars are gods!      */
bool            merc_down;              /* shutdown */
char                str_boot_time[MAX_INPUT_LENGTH];
time_t              reboot_time = 0;
time_t              current_time;       /* time of this pulse */
int                 port = 0;           /* telnet port for this MUD */
int                 control_socket;
char                command[MAX_STRING_LENGTH];
int                                     last_signal = -1;

/*
 * OS-dependent local functions.
 */
void    game_loop_unix          args((int control_socket));
int     init_socket             args((int port));
void    init_descriptor         args((int control_socket));

/*
 * Other local functions (OS-independent).
 */
void show_string(Descriptor *d, bool clear_remainder);
extern bool    check_parse_name        args((const String& name));
bool    check_playing           args((Descriptor *d, const String& name));
int     main                    args((int argc, char **argv));
void    nanny                   args((Descriptor *d, String argument));
bool    process_output          args((Descriptor *d, bool fPrompt));
void    read_from_buffer        args((Descriptor *d));
void    stop_idling             args((Character *ch));
void    bust_a_prompt           args((Character *ch));
int     roll_stat               args((Character *ch, int stat));
char    *get_multi_command     args((Descriptor &d, const String& argument));
String expand_color_codes(Character *ch, const String& str);

/* Desparate debugging measure: A function to print a reason for exiting. */
void exit_reason(const char *module, int line, const char *reason)
{
	Format::printf("exiting %s at line %d because: %s\n", module, line, reason);
	fflush(stdout);
}

#define EXIT_REASON(l,r) exit_reason("comm.c", l, r)

void copyover_recover()
{
	FILE *fp;
	char name[100];
	char host[MSL], msg1[MSL], msg2[MSL];
	int desc;
	Logging::log("Copyover recovery initiated");

	if ((fp = fopen(COPYOVER_LOG, "r")) == nullptr) {
		perror("copyover_recover:fopen");
		Logging::log("Copyover log not found. Exitting.\n");
		exit(1);
	}

#if 0
	unlink(COPYOVER_LOG);
#endif
	String logname = fread_string(fp);
	fclose(fp);

	if ((fp = fopen(COPYOVER_FILE, "r")) == nullptr) {
		perror("copyover_recover:fopen");
		Logging::log("Copyover file not found. Exitting.\n");
		exit(1);
	}

#if 0
	unlink(COPYOVER_FILE); /* In case something crashes - doesn't prevent reading */
#endif

	/* starting recovery message, do NOT use standard color codes here, it's a wtd */
	if (logname == "Liriel")
		Format::sprintf(msg1, "\nYou blink, and all of a sudden, you are back where you were before,\n"
		        "the world refreshed and born anew.\n");
	else if (logname == "Outsider")
		Format::sprintf(msg1, "\nDispite all of the confusion, you feel no danger in the soft hug.\n");
	else if (logname == "Kenneth")
		Format::sprintf(msg1, "\nThe hand is cold, but seems to exert no pressure at all on your face.\n");
	else if (logname == "Montrey")
		Format::sprintf(msg1, "\nThe world begins to take shape before your eyes.\n");
	else if (logname == "Xenith")
		Format::sprintf(msg1, "\nYour vision returns, as your body forms again.\n");
	else
		Format::sprintf(msg1, "\nRestoring from copyover...\n");

	/* finished recovery message, it's a stc so colors are ok */
	if (logname == "Xenith")
		Format::sprintf(msg2, "\nThe world begins anew, better from the destruction.\n");
	else if (logname == "Outsider")
		Format::sprintf(msg2, "\nThe angel steps back from you, revealing a cleaner, newer world. You\n"
		        "rub your eyes and when you open them again, the angel is gone.\n");
	else if (logname == "Montrey")
		Format::sprintf(msg2, "\nYou blink, and see the world complete once again.\n"
		        "Yet, something seems somehow... different.\n");
	else if (logname == "Liriel")
		Format::sprintf(msg2, "\nThe only trace of the faerie is a little blue glimmer that quickly winks\n"
		        "and vanishes.\n");
	else if (logname == "Kenneth")
		Format::sprintf(msg2, "\nHalf a second later your sight is returned, the man is gone, and the\n"
		        "world is changed.\n");
	else
		Format::sprintf(msg2, "\nCopyover recovery complete.\n");

	for (; ;) {
		fscanf(fp, "%d %s %s\n", &desc, name, host);

		if (desc == -1)
			break;

		Descriptor d(desc);

		/* Write something, and check if it goes error-free */
		if (!d.write(msg1)) {
			close(desc);  /* nope */
			continue;
		}

		d.host = host; // should be part of descriptor constructor?
		d.connected = CON_COPYOVER_RECOVER; /* -15, so close_socket frees the char */

		control::PlayerController *pc = new control::PlayerController(d);

		/* Now, find the pfile */
		if (!load_char_obj(pc, name)) {  /* Player file not found?! */
			d.write("\nSomehow, your character was lost in the copyover. Sorry.\n");
			delete pc;
			continue;
		}

		/* Insert in the char_list */
		Game::players.push_back(pc); // now owned by Game

		pc->character->next = char_list;
		char_list = pc->character;
		pc->character->pcdata->next = pc_list;
		pc_list = pc->character->pcdata;

		stc(msg2, pc->character);

		char_to_room(pc->character, pc->character->in_room);
		do_look(pc->character, "auto");
		act("$n materializes!", pc->character, nullptr, nullptr, TO_ROOM);

		d.connected = CON_PLAYING;

		if (pc->character->pet != nullptr) {
			char_to_room(pc->character->pet, pc->character->in_room);
			act("$n materializes!", pc->character->pet, nullptr, nullptr, TO_ROOM);
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
	exe_file = argv[0];

	struct timeval now_time;
	bool fCopyOver = FALSE;
	FILE *fpBoot = nullptr;
	Character *tempchars;
	struct sigaction sig_act;
	/* our signal handler.  more signals can be caught with repeated calls to sigaction,
	   using the same struct and different signals.  -- Montrey */
	sig_act.sa_handler = sig_handle;
	sigemptyset(&sig_act.sa_mask);
	sig_act.sa_flags = 0;
	sigaction(SIGPIPE, &sig_act, 0);

	/* Init time. */
	gettimeofday(&now_time, nullptr);
	current_time = (time_t) now_time.tv_sec;
	strcpy(str_boot_time, ctime(&current_time));

	/* Create boot file for script control -- Elrac
	   This file is created here and deleted after boot_db
	   completes, indicating a successful boot. */
	fpBoot = fopen(BOOT_FILE, "w");

	if (fpBoot)
		fclose(fpBoot);

	/* Get the port number. */
	port = DIZZYPORT;

	if (argc > 1) {
		if (!String(argv[1]).is_number()) {
			Format::fprintf(stderr, "Usage: %s [port #]\n", argv[0]);
			EXIT_REASON(433, "bad Legacy port arg");
			exit(1);
		}
		else if ((port = atoi(argv[1])) <= 1024) {
			Format::fprintf(stderr, "Port number must be above 1024.\n");
			EXIT_REASON(439, "bad Legacy port number");
			exit(1);
		}
	}

	if (port != DIZZYPORT) {
		/* not running on Dizzy port, so no need for boot control */
		unlink(BOOT_FILE);
	}

	/* Check for COPYOVER argument; Get 'control' and 'rmud' descriptors if so. */
	if (argc > 3) {
		/* Are we recovering from a copyover? */
		if (argv[3] && argv[3][0]) {
			if (argc <= 4) {
				Format::fprintf(stderr, "Not enough args for COPYOVER\n");
				EXIT_REASON(480, "not enough args for COPYOVER");
				exit(1);
			}

			if (!String(argv[4]).is_number()) {
				Format::fprintf(stderr, "Bad 'control_socket' value '%s'\n", argv[4]);
				EXIT_REASON(486, "bad control_socket value for COPYOVER");
				exit(1);
			}

			control_socket = atoi(argv[4]);
			fCopyOver = TRUE;
		}
	}

	if (!fCopyOver)
		control_socket = init_socket(port);

#if defined(SAND)
	/* simple asynchronous name daemon support -- Elrac */
	sand_init(SANDCLIENTPORT, SANDSERVERPORT);
#endif
        // load our configuration
        if (load_config(CONFIG_FILE) != 0) {
                Logging::bugf("Failed to load configuration from %s.", CONFIG_FILE);
                exit(1);
        }

	boot_db();
	Format::sprintf(log_buf, "Legacy is ready to rock on port %d.", port);
	Logging::log(log_buf);

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

		if ((pidfile = fopen(PID_FILE, "w")) == nullptr) {
			perror("getpid:fopen");
			Logging::log("pid file not found. Exitting.\n");
			exit(1);
		}

		Format::fprintf(pidfile, "%d", pid);
		fflush(pidfile);
		fclose(pidfile);
	}

	game_loop_unix(control_socket);
	close(control_socket);

	/* close our database */
	db_close();

	/* That's all, folks. */
	Logging::log("Normal termination of game.");
	exit(0);
	return 0;
}

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
#if defined(SO_DONTLINGER)
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

void game_loop_unix(int control_socket)
{
	static struct timeval null_time;
	struct timeval last_time;
	gettimeofday(&last_time, nullptr);
	current_time = (time_t) last_time.tv_sec;

	/* Main loop */
	while (!merc_down) {
		fd_set in_set;
		fd_set out_set;
		fd_set exc_set;
		Descriptor *d;
		int maxdesc;

		/* check and clear our signal buffer */
		if (last_signal != -1) {
			switch (last_signal) {
			case SIGPIPE:           Logging::bug("received signal SIGPIPE", 0);      break;

			default:                        Logging::bug("received signal %d", last_signal);
			}

			last_signal = -1;
		}

		/* Poll all active descriptors. */
		FD_ZERO(&in_set);
		FD_ZERO(&out_set);
		FD_ZERO(&exc_set);
		FD_SET(control_socket, &in_set);
		maxdesc = control_socket;

		for (control::PlayerController *pc: Game::players) {
			int desc = pc->descriptor.desc;
			maxdesc = UMAX(maxdesc, desc);
			FD_SET(desc, &in_set);
			FD_SET(desc, &out_set);
			FD_SET(desc, &exc_set);
		}

		null_time.tv_sec = 0;
		null_time.tv_usec = 0;

		if (select(maxdesc + 1, &in_set, &out_set, &exc_set, &null_time) < 0) {
			perror("Game_loop: select: poll");
			EXIT_REASON(793, "game_loop select() failure");
			exit(1);
		}

		/* New connection? */
		if (FD_ISSET(control_socket, &in_set)) {
			init_descriptor(control_socket);
		}

		/* Kick out the freaky folks. */
		for (auto it = Game::players.begin(); it != Game::players.end(); ) {
			control::PlayerController *pc = *it;
			int desc = pc->descriptor.desc;

			if (FD_ISSET(desc, &exc_set)) {
				FD_CLR(desc, &in_set);
				FD_CLR(desc, &out_set);

				Character *ch = pc->character;
//				Character *ch = d->original ? d->original : d->character;

				if (ch) {
					if (ch->level > 1)
						save_char_obj(ch);

					Format::sprintf(log_buf, "Kicking out char %s", ch->name);
				}
				else
					strcpy(log_buf, "Kicking out unknown char");

				Logging::log(log_buf);
				wiznet(log_buf, nullptr, nullptr, WIZ_LOGINS, 0, 0);

				d->outbuf.clear(); // important
				delete pc;
				it = Game::players.erase(it);
				continue;
			}

			++it;
		}

		/* Process input. */
		for (auto it = Game::players.begin(); it != Game::players.end(); ) {
			control::PlayerController *pc = *it;
			Descriptor &d = pc->descriptor;

			d.fcommand = FALSE;

			// read from socket into descriptor's buffer
			if (FD_ISSET(d.desc, &in_set)) {
				if (pc->character != nullptr) {
					d.timer = 0;
					pc->character->timer = 0;
				}

				if (pc->original != nullptr) {
					pc->original->timer = 0;
					d.timer = 0;
				}

				if (!d.read()) {
					FD_CLR(d.desc, &out_set);

					Character *ch = pc->character;
//					Character *ch = d->original ? d->original : d->character;

					if (ch) {
						if (ch->level > 1)
							save_char_obj(ch);

						Format::sprintf(log_buf, "Char %s disconnected", ch->name);
					}
					else 
						Format::sprintf(log_buf, "bad read_from_descriptor: socket %d disconnected.", d.desc);

					Logging::log(log_buf);
					wiznet(log_buf, nullptr, nullptr, WIZ_MALLOC, 0, 0);

					d.outbuf.clear();
					delete pc;
					it = Game::players.erase(it); // increment by deleting
					continue;
				}
			}

			if (pc->character && pc->character->wait > 0) {
				++it; // normal increment
				continue;
			}

			// no command waiting?  fetch some input
			if (d.incomm[0] == '\0')
				d.read_to_buffer();

			// got some input waiting now?  process it
			if (d.incomm[0] != '\0') {
				char *command2;
				d.fcommand = TRUE;
				stop_idling(pc->character);
				command2 = get_multi_command(d, d.incomm);

				if (!d->showstr_head.empty())
					show_string(d, !String(command2).lstrip().empty()); // ugly, fix someday
				else if (d->connected == CON_PLAYING) {
					if (d->character == nullptr) {
						// how did this happen?
						Logging::bug("playing descriptor with null character, closing phantom socket", 0);

						d->outbuf.clear();
						delete pc;
						it = Game::players.erase(it); // increment by deleting
						continue;
					}

					substitute_alias(d, command2);
				}
				else {
					nanny(d, command2);
					d->incomm[0] = '\0';
				}
			}    /* end of have input */

			++it; // normal increment
		} /* end of input loop */

		update_handler();

		/* Output. */
		for (auto it = Game::players.begin(); it != Game::players.end();  ) {
			Descriptor d = (*it)->descriptor;

			if (d == nullptr
			 || !FD_ISSET(d->desc, &out_set)
			 || (!d->fcommand && d->outbuf.empty())) {
				++it;
				continue;
			}

			Character *ch = (*it)->character;
//			Character *ch = d->original ? d->original : d->character;

			if (!process_output(d, TRUE)) {
				if (ch && ch->level > 1)
					save_char_obj(ch);

				Logging::bugf("bad write_to_descriptor: socket %d disconnected.", d->desc);
				d->outbuf.clear();
				delete *it;
				it = Game::players.erase(it);
			}
			else
				++it;
		}

		/* Synchronize to a clock.
		   Sleep( last_time + 1/PULSE_PER_SECOND - now ).
		   Careful here of signed versus unsigned arithmetic. */
		{
			struct timeval now_time;
			long secDelta;
			long usecDelta;
			gettimeofday(&now_time, nullptr);
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

				if (select(0, nullptr, nullptr, nullptr, &stall_time) < 0) {
					if (port == DIZZYPORT) { // don't count a stall on the debugger -- Montrey
						perror("Game_loop: select: stall");
						EXIT_REASON(979, "game_loop select() stall");
						exit(1);
					}
				}
			}
		}
		gettimeofday(&last_time, nullptr);
		current_time = (time_t) last_time.tv_sec;
	}

	return;
}

void init_descriptor(int control_socket)
{
	Descriptor *dnew;
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
	getsockname(control_socket, (struct sockaddr *) &sock, &size);

	if ((desc = accept(control_socket, (struct sockaddr *) &sock, &size)) < 0) {
		wiznet("init_descriptor: error accepting new connection",
		       nullptr, nullptr, WIZ_MALLOC, 0, 0);
		perror("new Descriptor: accept");
		return;
	}

#if !defined(FNDELAY)
 #define FNDELAY O_NDELAY
#endif

	if (fcntl(desc, F_SETFL, FNDELAY) == -1) {
		wiznet("init_descriptor: error setting FNDELAY",
		       nullptr, nullptr, WIZ_MALLOC, 0, 0);
		perror("new Descriptor: fcntl: FNDELAY");
		return;
	}

	/*
	 * Cons a new descriptor.
	 */
	dnew = new Descriptor(desc);
	/* Format::printf( "new descriptor at socket %d\n", desc ); */
	size = sizeof(sock);

	if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0) {
		wiznet("init_descriptor: error getting peername",
		       nullptr, nullptr, WIZ_MALLOC, 0, 0);
		perror("new Descriptor: getpeername");
		dnew->host = "(unknown)";
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
		Format::sprintf(buf, "%d.%d.%d.%d",
		        (addr >> 24) & 0xFF, (addr >> 16) & 0xFF,
		        (addr >>  8) & 0xFF, (addr) & 0xFF
		       );
#endif
//        if ( addr != 0x7F000001L ) /* don't log localhost -- Elrac */
		{
			Format::sprintf(log_buf, "init_descriptor: sock.sinaddr  = %s", buf);
			Logging::log(log_buf);
			wiznet(log_buf, nullptr, nullptr, WIZ_MALLOC, 0, 0);
		}
		from = nullptr;
		/* New coding to access the sand server. -- Elrac
		   This prevents crashes and lag from overly long lookups. */
#if defined(SAND)
		tmp_name = sand_query(addr);

		if (tmp_name == nullptr) {
			Format::sprintf(log_buf, "name not available");
			Logging::log(log_buf);
			dnew->host = buf;
		}
		else {
			dnew->host = tmp_name;
//                if ( addr != 0x7F000001L ) /* don't log localhost -- Elrac */
			{
				if (strcmp("kyndig.com", dnew->host)) {
					Format::sprintf(log_buf, "init_descriptor: host name = %s", dnew->host);
					Logging::log(log_buf);
					wiznet(log_buf, nullptr, nullptr, WIZ_SITES, 0, 0);
				}
			}
		}

#else
#ifdef IPV6
		from = gethostbyaddr((char *) &sock.sin6_addr, sizeof(sock.sin6_addr), AF_INET6);
#else
		from = gethostbyaddr((char *) &sock.sin_addr, sizeof(sock.sin_addr), AF_INET);
#endif

		if (from == nullptr || from->h_name == nullptr) {
			Format::sprintf(log_buf, "name not available");
			Logging::log(log_buf);
			dnew->host = buf;
		}
		else {
			dnew->host = from->h_name;
//                if ( addr != 0x7F000001L ) /* don't log localhost -- Elrac */
			{
				if (strcmp("kyndig.com", dnew->host)) {
					Format::sprintf(log_buf, "init_descriptor: host name = %s", dnew->host);
					Logging::log(log_buf);
					wiznet(log_buf, nullptr, nullptr, WIZ_SITES, 0, 0);
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
		extern String help_greeting;
		cwtb(dnew, help_greeting[0] == '.' ? help_greeting.substr(1) : help_greeting);
	}
	return;
}

void close_socket(Descriptor *dclose)
{
	Character *ch;
	Descriptor *d;

	if (!dclose->outbuf.empty())
		process_output(dclose, FALSE);

	if (dclose->snoop_by != nullptr)
		write_to_buffer(dclose->snoop_by, "Your victim has left the game.\n");

	for (control::PlayerController *pc: Game::players)
		if (pc->descriptor && pc->descriptor->snoop_by == dclose)
			pc->descriptor->snoop_by = nullptr;

	if ((ch = dclose->character) == nullptr) {
		Format::sprintf(log_buf, "Closing link to phantom at socket %d.", dclose->descriptor);
		/* log_string( log_buf ); */
	}
	else {
		Format::sprintf(log_buf, "Closing link to %s.", ch->name);
		Logging::log(log_buf);

		if (dclose->connected == CON_PLAYING) {
			Character *rch;

			for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
				if (!IS_IMMORTAL(ch) || can_see_char(rch, ch))
					ptc(rch, "%s has lost %s link.\n",
					    PERS(ch, rch, VIS_CHAR),
					    GET_ATTR_SEX(ch) == SEX_FEMALE ? "her" :
					    GET_ATTR_SEX(ch) == SEX_MALE   ? "his" : "its");

			wiznet("$N has lost link.", ch, nullptr, WIZ_LINKS, 0, 0);

			if (!IS_NPC(ch))
				ch->pcdata->plr_flags += PLR_LINK_DEAD;
			else {
				/* been having problems with this -- Montrey */
				if (ch->desc == nullptr)
					Logging::bug("close_socket: NPC without descriptor!", 0);
				else if (ch->desc->original != nullptr
				         && ch->desc->original->pcdata != nullptr)
					ch->desc->original->pcdata->plr_flags += PLR_LINK_DEAD;

				if (ch->act_flags.has(ACT_MORPH)) {
					if (ch->desc->original != nullptr) {
						RoomPrototype *location;

						if (ch->in_room == nullptr)
							location = get_room_index(ROOM_VNUM_MORGUE);
						else
							location = ch->in_room;

						char_from_room(ch->desc->original);
						char_to_room(ch->desc->original, location);

						if (ch->in_room != nullptr)
							char_from_room(ch);
					}
				}
			}

			ch->desc = nullptr;
		}
		else
			delete dclose->character;
	}

	if (d_next == dclose)
		d_next = d_next->next;

	if (dclose == descriptor_list)
		descriptor_list = descriptor_list->next;
	else {
		Descriptor *d;

		for (d = descriptor_list; d && d->next != dclose; d = d->next)
			;

		if (d != nullptr)
			d->next = dclose->next;
		else
			Logging::bug("Close_socket: dclose not found.", 0);
	}

	Format::printf("Closing socket %d\n", dclose->descriptor);
	close(dclose->descriptor);
	delete dclose;
	return;
}

/*
 * Low level output function.
 */
bool process_output(Descriptor *d, bool fPrompt)
{
	extern bool merc_down;

	/* VT100 Stuff */
	if (IS_PLAYING(d)
	    && d->character->pcdata
	    && d->character->pcdata->video_flags.has(VIDEO_VT100)) {
		write_to_buffer(d, VT_SAVECURSOR);
		goto_line(d->character, d->character->lines - 2, 1);
	}

	/*
	 * Bust a prompt.
	 */
	if (!merc_down && !d->showstr_head.empty())
		write_to_buffer(d, "[Hit Enter to continue]\n");
	else if (fPrompt && !merc_down && IS_PLAYING(d)) {
		Character *ch;
		Character *victim;
		ch = d->character;

		/* battle prompt */
		if ((victim = ch->fighting) != nullptr) {
			String atb;

			if (ch->comm_flags.has(COMM_ATBPROMPT)) {
				if (ch->wait > 40)      Format::sprintf(atb, "{B[{C*{T*********{B]{x ");
				else if (ch->wait > 36) Format::sprintf(atb, "{B[{Y*{C*{T********{B]{x ");
				else if (ch->wait > 32) Format::sprintf(atb, "{B[{C*{Y*{C*{T*******{B]{x ");
				else if (ch->wait > 28) Format::sprintf(atb, "{B[{T*{C*{Y*{C*{T******{B]{x ");
				else if (ch->wait > 24) Format::sprintf(atb, "{B[{T**{C*{Y*{C*{T*****{B]{x ");
				else if (ch->wait > 20) Format::sprintf(atb, "{B[{T***{C*{Y*{C*{T****{B]{x ");
				else if (ch->wait > 16) Format::sprintf(atb, "{B[{T****{C*{Y*{C*{T***{B]{x ");
				else if (ch->wait > 12) Format::sprintf(atb, "{B[{T*****{C*{Y*{C*{T**{B]{x ");
				else if (ch->wait > 8)  Format::sprintf(atb, "{B[{T******{C*{Y*{C*{T*{B]{x ");
				else if (ch->wait > 4)  Format::sprintf(atb, "{B[{T*******{C*{Y*{C*{B]{x ");
				else if (ch->wait > 0)  Format::sprintf(atb, "{B[{T********{C*{Y*{B]{x ");
				else                    Format::sprintf(atb, "{B[{C**{YREADY!{C**{B]{x ");
			}/*

                                if (ch->wait > 40)      Format::sprintf(atb, "{P[{P*{R*********{P]{x ");
                                else if (ch->wait > 36) Format::sprintf(atb, "{P[{Y*{P*{R********{P]{x ");
                                else if (ch->wait > 32) Format::sprintf(atb, "{P[{P*{Y*{P*{R*******{P]{x ");
                                else if (ch->wait > 28) Format::sprintf(atb, "{P[{R*{P*{Y*{P*{R******{P]{x ");
                                else if (ch->wait > 24) Format::sprintf(atb, "{P[{R**{P*{Y*{P*{R*****{P]{x ");
                                else if (ch->wait > 20) Format::sprintf(atb, "{P[{R***{P*{Y*{P*{R****{P]{x ");
                                else if (ch->wait > 16) Format::sprintf(atb, "{P[{R****{P*{Y*{P*{R***{P]{x ");
                                else if (ch->wait > 12) Format::sprintf(atb, "{P[{R*****{P*{Y*{P*{R**{P]{x ");
                                else if (ch->wait > 8)  Format::sprintf(atb, "{P[{R******{P*{Y*{P*{R*{P]{x ");
                                else if (ch->wait > 4)  Format::sprintf(atb, "{P[{R*******{P*{Y*{P*{P]{x ");
                                else if (ch->wait > 0)  Format::sprintf(atb, "{P[{R********{P*{Y*{P]{x ");
                                else                    Format::sprintf(atb, "{P[{R**{YREADY!{R**{P]{x ");

                                Format::sprintf(buf, "({G%d{x) ", ch->fightpulse);
                                atb += buf;
                        } */
			else
				Format::sprintf(atb, "{x");

			if (can_see_char(ch, victim)) {
				int percent;

				if (IS_NPC(victim))
					atb += victim->short_descr;
				else
					atb += victim->name;

				if (GET_MAX_HIT(victim) > 0)
					percent = victim->hit * 100 / GET_MAX_HIT(victim);
				else
					percent = -1;

				if (percent >= 100)     atb += " is in excellent condition.";
				else if (percent >= 90) atb += " has a few scratches.";
				else if (percent >= 75) atb += " has some small wounds and bruises.";
				else if (percent >= 50) atb += " has quite a few wounds.";
				else if (percent >= 30) atb += " has some big nasty wounds and scratches.";
				else if (percent >= 15) atb += " looks pretty hurt.";
				else if (percent >= 1)  atb += " is in awful condition.";
				else if (percent >= 0)  atb += " will soon be toast!!!";
				else                    atb += " is in need of ***SERIOUS*** medical attention!";
			}

			ptc(ch, "%s\n", atb);
		}

		ch = d->original ? d->original : d->character;

		if (!ch->comm_flags.has(COMM_COMPACT))
			write_to_buffer(d, "\n");

		if (ch->comm_flags.has(COMM_PROMPT)) {
			set_color(ch, CYAN, NOBOLD);
			bust_a_prompt(d->character);
			set_color(ch, WHITE, NOBOLD);
		}
	}

	/*
	 * Short-circuit if nothing to write.
	 */
	if (d->outbuf.empty())
		return TRUE;

	/*
	 * Snoop-o-rama.
	 */
	if (d->snoop_by != nullptr) {
		if (d->character != nullptr)
			write_to_buffer(d->snoop_by, (d->character)->name);

		write_to_buffer(d->snoop_by, "> ");
		write_to_buffer(d->snoop_by, d->outbuf);
	}

	/* VT100 Stuff */
	if (IS_PLAYING(d)
	    && d->character->pcdata
	    && d->character->pcdata->video_flags.has(PLR_VT100)) {
		goto_line(d->character, d->character->lines - 1, 1);
		write_to_buffer(d, VT_CLEAR_LINE);
		write_to_buffer(d, VT_BAR);
		write_to_buffer(d, VT_RESTORECURSOR);
	}

	/*
	 * OS-dependent output.
	 */
	if (!d->write(d->outbuf)) {
		d->outbuf.clear();
		return FALSE;
	}
	else {
		d->outbuf.clear();
		return TRUE;
	}
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt(Character *ch)
{
	String buf;

	if (ch->prompt.empty()) {
		ptc(ch, "{W<{C%d{Thp {G%d{Hma {B%d{Nst{W>{x %s", ch->hit, ch->mana, ch->stam, ch->prefix);
		return;
	}

	if (ch->comm_flags.has(COMM_AFK)) {
		stc("{b<AFK> {x", ch);
		return;
	}

	auto str = ch->prompt.cbegin();

	while (str != ch->prompt.cend()) {
		if (*str == '{') {
			// copy the code straight in, let expand_codes handle it
			++str;

			if (*str == '\0')
				continue;

			buf += '{';
			buf += *str;
			++str;
			continue;
		}

		if (*str != '%') {
			buf += *str++;
			continue;
		}

		++str;

		switch (*str) {
		default:
			buf += " ";
			break;

		case 'e': {
			bool found = FALSE;

			for (int door = 0; door < 6; door++) {
				Exit *pexit;

				if ((pexit = ch->in_room->exit[door]) != nullptr
				    && pexit ->u1.to_room != nullptr
				    && can_see_room(ch, pexit->u1.to_room)
				    && can_see_in_room(ch, pexit->u1.to_room)) {
					found = TRUE;

					if (!pexit->exit_flags.has(EX_CLOSED))
						buf += UPPER(Exit::dir_name(door)[0]);
					else
						buf += LOWER(Exit::dir_name(door)[0]);
				}
			}

			if (!found)
				buf += "none";

			break;
		}
		case 'c':  buf += '\n'; break;
		case 'h':  buf += Format::format("%d", ch->hit); break;
		case 'H':  buf += Format::format("%d", GET_MAX_HIT(ch)); break;
		case 'm':  buf += Format::format("%d", ch->mana); break;
		case 'M':  buf += Format::format("%d", GET_MAX_MANA(ch)); break;
		case 'v':  buf += Format::format("%d", ch->stam); break;
		case 'V':  buf += Format::format("%d", GET_MAX_STAM(ch)); break;
		case 'x':  buf += Format::format("%d", ch->exp); break;
		case 'X':
			if (!IS_NPC(ch))
				buf += Format::format("%ld",
				        (ch->level + 1) * exp_per_level(ch, ch->pcdata->points) - ch->exp);

			break;
		case 'g':  buf += Format::format("%ld", ch->gold); break;
		case 's':  buf += Format::format("%ld", ch->silver); break;
		case 'a':
			if (ch->level > 9)
				buf += Format::format("%d", ch->alignment);
			else
				buf += IS_GOOD(ch) ? "good" : IS_EVIL(ch) ? "evil" : "neutral";

			break;
		case 'r':
			if (ch->in_room != nullptr)
				buf += can_see_in_room(ch, ch->in_room) ? ch->in_room->name.uncolor() : "darkness";
			else
				buf += ' ';

			break;
		case 'R':
			if (IS_IMMORTAL(ch) && ch->in_room != nullptr)
				buf += Format::format("%d", ch->in_room->vnum);
			else
				buf += ' ';

			break;
		case 'z':
			if (ch->in_room != nullptr)
				buf += ch->in_room->area->name;
			else
				buf += ' ';

			break;
		case 't':
			if (ch->in_room != nullptr)
				buf += sector_lookup(ch->in_room->sector_type);
			else
				buf += ' ';

			break;
		case 'q':
			if (!IS_QUESTOR(ch))
				buf += Format::format("%d", ch->nextquest);
			else
				buf += Format::format("%d", ch->countdown);

			break;
		case 'Q':
			if (IS_QUESTOR(ch)) {
//				ObjectPrototype *questinfoobj;
				MobilePrototype *questinfo;

				if (ch->questmob == -1 || ch->questobf == -1)
					buf += "*report!*";
				else if (ch->questobj > 0) {
//					if ((questinfoobj = get_obj_index(ch->questobj)) != nullptr)
//						Format::sprintf(buf2, "%s", questinfoobj->name);
					if (ch->questloc)
						buf += get_room_index(ch->questloc)->name.uncolor();
					else
						buf += "Unknown";
				}
				else if (ch->questmob > 0) {
					if ((questinfo = get_mob_index(ch->questmob)) != nullptr)
						buf += questinfo->short_descr.uncolor();
					else
						buf += "Unknown";
				}
				else
					buf += "Unknown";
			}

			break;
		case 'p':
			if (!IS_NPC(ch))
				buf += Format::format("%d", ch->questpoints);

			break;
		case 'j':
			if (!IS_NPC(ch)) {
				if (!IS_SQUESTOR(ch))
					buf += Format::format("%d", ch->pcdata->nextsquest);
				else
					buf += Format::format("%d", ch->pcdata->sqcountdown);
			}
			else
				buf += '0';

			break;
		case 'J':
			if (IS_SQUESTOR(ch)) {
				if (ch->pcdata->squestobj != nullptr && ch->pcdata->squestmob == nullptr) {
					if (!ch->pcdata->squestobjf)
//						buf += ch->pcdata->squestobj->short_descr;
						buf += get_room_index(ch->pcdata->squestloc1)->name.uncolor();
					else
						buf += "*report!*";
				}
				else if (ch->pcdata->squestmob != nullptr && ch->pcdata->squestobj == nullptr) {
					if (!ch->pcdata->squestmobf)
						buf += ch->pcdata->squestmob->short_descr.uncolor();
					else
						buf += "*report!*";
				}
				else if (ch->pcdata->squestobj != nullptr && ch->pcdata->squestmob != nullptr) {
					if (ch->pcdata->squestobjf) {
						if (!ch->pcdata->squestmobf)
							buf += ch->pcdata->squestmob->short_descr.uncolor();
						else
							buf += "*report!*";
					}
					else
//						buf += ch->pcdata->squestobj->short_descr;
						buf += get_room_index(ch->pcdata->squestloc1)->name.uncolor();
				}
				else
					buf += "Unknown";
			}

			break;
		case 'k':
			if (!IS_NPC(ch))
				buf += Format::format("%d", ch->pcdata->skillpoints);

			break;
		case 'K':
			if (!IS_NPC(ch))
				buf += Format::format("%d", ch->pcdata->rolepoints);

			break;
		case 'w':
			if (IS_IMMORTAL(ch)) {
				if (!ch->invis_level && !ch->lurk_level)
					buf += "VIS";
				else
					buf += Format::format("%d/%d", ch->invis_level, ch->lurk_level);

				if (ch->act_flags.has(PLR_SUPERWIZ))
					buf += "WIZ";
			}
			else
				buf += ' ';

			break;
		case 'C':
			++str;

			if (*str == '\0')
				continue;

			// put in a color code, let expand_codes handle it
			buf += '{';
			buf += *str;
			break;
		case 'l':  buf += Format::format("%d", ch->level); break;
		case '%':  buf += '%'; break;
		}

		++str;
	}

	stc(buf, ch);

	if (!ch->prefix.empty())
		stc(ch->prefix, ch);
} /* end bust_a_prompt() */

/* write_to_buffer with color codes -- Montrey */
void cwtb(Descriptor *d, const String& txt)
{
	if (txt.empty())
		return;

	write_to_buffer(d, expand_color_codes(d->character, txt));
}

/*
 * Append onto an output buffer.
 */
void write_to_buffer(Descriptor *d, const String& txt)
{
	if (d == nullptr)
		return;

	/*
	 * Initial \n if needed.
	 */
	if (d->outbuf.empty() && !d->fcommand) {
		d->outbuf += '\n';
		d->outbuf += '\r';
	}

	/*
	 * Copy.
	 */
	d->outbuf += txt;
	return;
}

/*
 * Check if already playing.
 */
bool check_playing(Descriptor *d, const String& name)
{
	for (control::PlayerController *pc: Game::players) {
		Descriptor *dold = pc->descriptor;

		if (dold != d
		    &&   dold->character != nullptr
		    &&   dold->connected != CON_GET_NAME
		    &&   dold->connected != CON_GET_OLD_PASSWORD
		    &&   name == (dold->original
		                  ? dold->original->name : dold->character->name)) {
			write_to_buffer(d, "That character is already playing.\n");
			write_to_buffer(d, "Do you wish to connect anyway (Y/N)?");
			d->connected = CON_BREAK_CONNECT;
			return TRUE;
		}
	}

	return FALSE;
}

void stop_idling(Character *ch)
{
	if (ch == nullptr
	    ||   !IS_PLAYING(ch->desc)
	    ||   ch->was_in_room == nullptr
	    ||   ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
		return;

	ch->desc->timer = 0;
	char_from_room(ch);
	char_to_room(ch, ch->was_in_room);
	ch->was_in_room     = nullptr;
	act("$n has returned from the void.", ch, nullptr, nullptr, TO_ROOM);
	return;
}

String interpret_color_code(Character *ch, char a)
{
	// ch could be nullptr here, and might not be PC, so lots of checks
	String code;

	switch (a) {
	case 'x': 
		if (ch && ch->pcdata)
			code = Format::format("%s%s\033[%d;%dm",
				CLEAR, B_BLACK,
				ch->pcdata->lastcolor[1], ch->pcdata->lastcolor[0]);
		else
			code = Format::format("%s%s", CLEAR, B_BLACK);

		break;

	case 'N': code = C_BLUE; break;
	case 'T': code = C_CYAN; break;
	case 'H': code = C_GREEN; break;
	case 'k':
		if (ch && ch->pcdata && ch->pcdata->video_flags.has(VIDEO_DARK_MOD))
			code = C_WHITE;
		else
			code = C_BLACK;

		break;

	case 'M': code = C_MAGENTA; break;
	case 'R': code = C_RED; break;
	case 'g': code = C_WHITE; break;
	case 'b': code = C_YELLOW; break;
	case 'B': code = C_B_BLUE; break;
	case 'C': code = C_B_CYAN; break;
	case 'G': code = C_B_GREEN; break;
	case 'V': code = C_B_MAGENTA; break;
	case 'P': code = C_B_RED; break;
	case 'W': code = C_B_WHITE; break;
	case 'Y': code = C_B_YELLOW; break;
	case 'c':
		if (ch && ch->pcdata && ch->pcdata->video_flags.has(VIDEO_DARK_MOD))
			code = C_WHITE;
		else
			code = C_B_GREY;

		break;

	case 'f':
		code = C_FLASH;

		if (ch && ch->pcdata) {
			if (ch->pcdata->video_flags.has(VIDEO_FLASH_OFF))
				code.erase();

			if (ch->pcdata->video_flags.has(VIDEO_FLASH_LINE))
				code += C_UNDERLINE;
		}

		break;

	case 's': code = C_REVERSE; break;
	case 'e': code = B_GREY; break;
	case 'r': code = B_RED; break;
	case 'y': code = B_YELLOW; break;
	case 'h': code = B_GREEN; break;
	case 't': code = B_CYAN; break;
	case 'n': code = B_BLUE; break;
	case 'm': code = B_MAGENTA; break;
	case 'a': code = B_BLACK; break;
	case '{': code = "{"; break;
	default: break;
	}

	return code;
}

String expand_color_codes(Character *ch, const String& str) {
	String out;

	for (auto it = str.cbegin(); it != str.cend(); it++) {
		if (*it != '{') {
			out += *it;
			continue;
		}

		if (++it == str.cend())
			break;

		out += interpret_color_code(ch, *it);
	}

	return out;
}

/*
 * Write to one char.
 */
void stc(const String& txt, Character *ch)
{
	if (txt.empty() || ch->desc == nullptr)
		return;

	if (!IS_NPC(ch) && ch->pcdata->video_flags.has(VIDEO_CODES_SHOW))
		write_to_buffer(ch->desc, txt);
	else if (ch->act_flags.has(PLR_COLOR2))
		write_to_buffer(ch->desc, expand_color_codes(ch, txt));
	else
		write_to_buffer(ch->desc, txt.uncolor());
} /* end stc() */

/* string pager */
void show_string(Descriptor *d, bool clear_remainder)
{
	// if not nothing, done paging output, clear the remainders
	if (clear_remainder) {
		d->showstr_head.erase();
		return;
	}

	std::size_t page_len;

	// how many lines per page?
	if (d->character) {
		std::size_t break_pos = d->showstr_head.find_nth(d->character->lines, "\n");

		if (break_pos == std::string::npos)
			page_len = d->showstr_head.size();
		else
			page_len = break_pos + 1;
	}
	else
		page_len = d->showstr_head.size();

	String page = d->showstr_head.substr(0, page_len);
	d->showstr_head.erase(0, page_len);

	if (d->character)
		stc(page, d->character);
	else
		cwtb(d, page);
} /* end show_string() */

/*
 * Send a page to one char.
 */
void page_to_char(const String& txt, Character *ch)
{
	if (txt.empty() || ch->desc == nullptr)
		return;

	ch->desc->showstr_head += txt;
	show_string(ch->desc, false);
}

char *get_multi_command(Descriptor &d, const String& argument)
{
	char *pcom;
	pcom = command;
	const char *argptr = argument.c_str();

	while (*argptr != '\0') {
		if (argptr[0] == '|') {
			if (*++argptr != '|') {
				strcpy(d.incomm, argptr);
				*pcom = '\0';
				return command;
			}
		}

		*pcom++ = *argptr++;
	}

	*pcom = '\0';
	d.incomm[0] = '\0';
	return command;
} /* end get_multi_command() */

int roll_stat(Character *ch, int stat)
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

void do_copyove(Character *ch)
{
	ptc(ch, "You have to spell out COPYOVER if you want to copyover. Silly %s.\n", ch->name);
	return;
}

void do_copyover(Character *ch, String argument)
{
	FILE *fp;
	Descriptor *d, *d_next;
	String buf;
	do_allsave(ch, "");
	save_clan_table();

	if ((fp = fopen(COPYOVER_FILE, "w")) == nullptr) {
		stc("Copyover file not writeable, aborted.\n", ch);
		Format::sprintf(buf, "Could not write to copyover file: %s", COPYOVER_FILE);
		Logging::log(buf);
		perror("do_copyover:fopen");
		return;
	}

	/* save the socket state of all active players, only */
	for (control::PlayerController *pc : Game::players) {
		Descriptor *d = pc->descriptor;
		Format::printf("found socket %d, host %s, conn %d\n", d->descriptor, d->host, d->connected);

		if (IS_PLAYING(d)) {
			Character *och = CH(d);
			one_argument(och->name, buf);
			Format::fprintf(fp, "%d %s %s\n", d->descriptor, buf, d->host);
		}
	}

	Format::fprintf(fp, "-1\n");
	fclose(fp);

	if ((fp = fopen(COPYOVER_LOG, "w")) == nullptr) {
		stc("Copyover file not writeable, aborted.\n", ch);
		Format::sprintf(buf, "Could not write to copyover file: %s", COPYOVER_LOG);
		Logging::log(buf);
		perror("do_copyover:fopen");
		return;
	}

	Format::fprintf(fp, "%s~\n", ch->name);
	fclose(fp);

	/*****/
	objstate_save_items();

	/*****/

	/* yes, this is ugly, but i don't think it warrants a place in pcdata */
	if (ch->name == "Montrey") {
		Format::sprintf(buf, "\nA beam of pure white light arcs down from the heavens, striking\n"
		        "the earth just beyond the horizon.  The ground starts to shake,\n"
		        "and a curtain of fiery destruction sweeps over the land, clearing\n"
		        "away the old in preparation for the new.\n");
	}
	else if (ch->name == "Outsider") {
		Format::sprintf(buf, "\nAn angel, clothed in white light, lands before you. He stands\n"
		        "before you, blocking your view of the rest of the world. His wings\n"
		        "wrap about you in a soft, feathery embrace. All around you hear screams\n"
		        "and loud cracking sounds, as if the world is turning inside out.\n");
	}
	else if (ch->name == "Xenith") {
		Format::sprintf(buf, "\nThe winds suddenly spring up, as the storm destroys the world.\n"
		        "Your body becomes dust before the maelstrom.\n"
		        "Your vision dissapates as the universe vanishes in a flash.\n");
	}
	else if (ch->name == "Liriel") {
		Format::sprintf(buf, "\nA tiny faerie shimmers into existence in front of you, it's magics quickly\n"
		        "wrapping around you to form a protective barrier ... You watch in awe as the\n"
		        "world around you blurs and distorts, reality as you know it washing away.\n");
	}
	else if (ch->name == "Kenneth") {
		Format::sprintf(buf, "\nA casual tap on the shoulder causes you to spin around.  Directly in front\n"
		        "of you is a plain, unassuming man who gently covers your eyes.\n");
	}
	else
		Format::sprintf(buf, "\n*** COPYOVER by %s - please remain seated!\n", ch->name);

	/* For each playing descriptor, save its state */
	for (auto it = Game::players.begin(); it != Game::players.end();  ) {
		control::PlayerController *pc = *it;
		Descriptor *d = pc->descriptor;

		if (d->connected != CON_PLAYING) {
			/* drop those logging on */
			Format::printf("closing socket %d from host %s\n",
			       d->desc, d->host);
			d->write("\nSorry, we are rebooting. Come back in a minute.\n");
			close_socket(d);  /* throw'em out */
		}
		else {
			/* regular character -- save and notify */
			Character *och = pc->character;
			Format::printf("closing socket %d from char %s\n",
			       d->desc, och->name);
			save_char_obj(och);
			d->write(buf);
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
	char portbuf[MSL], controlbuf[MSL];
	Format::sprintf(portbuf,  "%d", port);
	Format::sprintf(controlbuf, "%d", control_socket);
	execl(exe_file.c_str(), "legacy", portbuf, "null", "copyover", controlbuf, "null", (char*)0);
	/* Failed - sucessful exec will not return */
	perror("do_copyover: execl");
	stc("Copyover FAILED!\n", ch);
}
