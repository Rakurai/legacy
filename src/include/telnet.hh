#pragma once
/*	@(#)telnet.h 1.7 88/08/19 SMI; from UCB 5.1 5/30/85	*/
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Definitions for the TELNET protocol.
 */

/**
 * @defgroup TelnetCommands Definitions for the TELNET protocol commands
 * @{
 */
constexpr int IAC   = 255;  /**< interpret as command: */
constexpr int DONT  = 254;  /**< you are not to use option */
constexpr int DO    = 253;  /**< please, you use option */
constexpr int WONT  = 252;  /**< I won't use option */
constexpr int WILL  = 251;  /**< I will use option */
constexpr int SB    = 250;  /**< interpret as subnegotiation */
constexpr int GA    = 249;  /**< you may reverse the line */
constexpr int EL    = 248;  /**< erase the current line */
constexpr int EC    = 247;  /**< erase the current character */
constexpr int AYT   = 246;  /**< are you there */
constexpr int AO    = 245;  /**< abort output--but let prog finish */
constexpr int IP    = 244;  /**< interrupt process--permanently */
constexpr int BREAK = 243;  /**< break */
constexpr int DM    = 242;  /**< data mark--for connect. cleaning */
constexpr int NOP   = 241;  /**< nop */
constexpr int SE    = 240;  /**< end sub negotiation */
constexpr int EOR   = 239;  /**< end of record (transparent mode) */
constexpr int SYNCH = 242;  /**< for telfunc calls */
/** @} */

#ifdef TELCMDS
char *telcmds[] = {
	"SE", "NOP", "DMARK", "BRK", "IP", "AO", "AYT", "EC",
	"EL", "GA", "SB", "WILL", "WONT", "DO", "DONT", "IAC",
};
#endif

/**
 * @defgroup TelnetOptions Definitions for the TELNET options
 * @{
 */
constexpr int TELOPT_BINARY	= 0;	/**< 8-bit data path */
constexpr int TELOPT_ECHO	= 1;	/**< echo */
constexpr int TELOPT_RCP	= 2;	/**< prepare to reconnect */
constexpr int TELOPT_SGA	= 3;	/**< suppress go ahead */
constexpr int TELOPT_NAMS	= 4;	/**< approximate message size */
constexpr int TELOPT_STATUS	= 5;	/**< give status */
constexpr int TELOPT_TM		= 6;	/**< timing mark */
constexpr int TELOPT_RCTE	= 7;	/**< remote controlled transmission and echo */
constexpr int TELOPT_NAOL 	= 8;	/**< negotiate about output line width */
constexpr int TELOPT_NAOP 	= 9;	/**< negotiate about output page size */
constexpr int TELOPT_NAOCRD	= 10;	/**< negotiate about CR disposition */
constexpr int TELOPT_NAOHTS	= 11;	/**< negotiate about horizontal tabstops */
constexpr int TELOPT_NAOHTD	= 12;	/**< negotiate about horizontal tab disposition */
constexpr int TELOPT_NAOFFD	= 13;	/**< negotiate about formfeed disposition */
constexpr int TELOPT_NAOVTS	= 14;	/**< negotiate about vertical tab stops */
constexpr int TELOPT_NAOVTD	= 15;	/**< negotiate about vertical tab disposition */
constexpr int TELOPT_NAOLFD	= 16;	/**< negotiate about output LF disposition */
constexpr int TELOPT_XASCII	= 17;	/**< extended ASCII character set */
constexpr int TELOPT_LOGOUT	= 18;	/**< force logout */
constexpr int TELOPT_BM		= 19;	/**< byte macro */
constexpr int TELOPT_DET	= 20;	/**< data entry terminal */
constexpr int TELOPT_SUPDUP	= 21;	/**< supdup protocol */
constexpr int TELOPT_SUPDUPOUTPUT = 22;	/**< supdup output */
constexpr int TELOPT_SNDLOC	= 23;	/**< send location */
constexpr int TELOPT_TTYPE	= 24;	/**< terminal type */
constexpr int TELOPT_EOR	= 25;	/**< end of record */
constexpr int TELOPT_EXOPL	= 255;	/**< extended-options-list */
/** @} */

#ifdef TELOPTS
constexpr int NTELOPTS = 1 + TELOPT_EOR;
char *telopts[NTELOPTS] = {
	"BINARY", "ECHO", "RCP", "SUPPRESS GO AHEAD", "NAME",
	"STATUS", "TIMING MARK", "RCTE", "NAOL", "NAOP",
	"NAOCRD", "NAOHTS", "NAOHTD", "NAOFFD", "NAOVTS",
	"NAOVTD", "NAOLFD", "EXTEND ASCII", "LOGOUT", "BYTE MACRO",
	"DATA ENTRY TERMINAL", "SUPDUP", "SUPDUP OUTPUT",
	"SEND LOCATION", "TERMINAL TYPE", "END OF RECORD",
};
#endif

// @defgroup TelnetSubOptionQualifiers Definitions for TELNET sub-option qualifiers
// @{
// constexpr int TELQUAL_IS   = 0;  /**< option is... */
// constexpr int TELQUAL_SEND = 1;  /**< send option */
// @}
