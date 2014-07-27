/***********************************************************************
*                                                                      *
*   VT100 Code by Lotus of DizzyMUD                           12/4/96  *
*   lostwaves.op.net 9000                           headn@okstate.edu  *
*                                                                      *
***********************************************************************/

#define VT_SAVECURSOR      "\033[s"  /* Save cursor and attrib */
#define VT_RESTORECURSOR   "\033[u"  /* Restore cursor pos and attribs */
#define VT_SETWIN_CLEAR    "\033[r"  /* Clear scrollable window size */
#define VT_CLEAR_SCREEN    "\033[2J" /* Clear screen and home cursor */
#define VT_CLEAR_LINE      "\033[K"  /* Clear to end of line */
#define VT_RESET_TERMINAL  "\033"
#define VT_BAR "-----------------------------------------------------------------------------"

