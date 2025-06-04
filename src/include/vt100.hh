#pragma once
/***********************************************************************
*                                                                      *
*   VT100 Code by Lotus of DizzyMUD                           12/4/96  *
*   lostwaves.op.net 9000                           headn@okstate.edu  *
*                                                                      *
***********************************************************************/

constexpr const char* VT_SAVECURSOR      = "\033[s";  /* Save cursor and attrib */
constexpr const char* VT_RESTORECURSOR   = "\033[u";  /* Restore cursor pos and attribs */
constexpr const char* VT_SETWIN_CLEAR    = "\033[r";  /* Clear scrollable window size */
constexpr const char* VT_CLEAR_SCREEN    = "\033[2J"; /* Clear screen and home cursor */
constexpr const char* VT_CLEAR_LINE      = "\033[K";  /* Clear to end of line */
constexpr const char* VT_RESET_TERMINAL  = "\033";
constexpr const char* VT_BAR = "-----------------------------------------------------------------------------";

