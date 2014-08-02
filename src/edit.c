/********************************************************************/
/*                                                                  */
/* edit.c -- line-oriented editor for notes and other strings       */
/*                                                                  */
/* Elrac, April 1998                                                */
/*                                                                  */
/* Original ROM extension code written specifically for Dizzy MUD   */
/* by Elrac. You may not use this code without Elrac's consent.     */
/* Contact him at: carls@ipf.de .                                   */
/*                                                                  */
/********************************************************************/

/********************************************************************/
/*                                                                  */
/* Editing functions:                                               */
/*                                                                  */
/*    EDIT                                                          */
/*       Tells the user what kind of string he is editing and gives */
/*       a brief list of EDIT options.                              */
/*                                                                  */
/*    EDIT NOTE                                                     */
/*       Starts editing the player's current NOTE.                  */
/*                                                                  */
/*    EDIT <number>                                                 */
/*       Sets current line to <number>                              */
/*                                                                  */
/*    EDIT [<number1>] LIST [<number2>]                             */
/*       Lists <number2> lines starting at <number1>                */
/*    EDIT [<number1>] [<number2>] LIST                             */
/*       Lists lines from <number1> to <number2>                    */
/*                                                                  */
/*    EDIT [<number1>] DELETE [<number2>]                           */
/*       Deletes <number2> lines starting at <number1>              */
/*    EDIT [<number1>] [<number2>] DELETE                           */
/*       Deletes lines from <number1> to <number2>                  */
/*                                                                  */
/*    EDIT [<number>] INSERT <text>                                 */
/*       Inserts <text> after line <number>                         */
/*                                                                  */
/*    EDIT [<number>] CHANGE '<text>' '<text>'                      */
/*                                                                  */
/*    EDIT [<number1>] WRAP [<number2>]                             */
/*       Wraps <number2> lines starting at <number1>                */
/*    EDIT [<number1>] [<number2>] WRAP                             */
/*       Wraps lines from <number1> to <number2>                    */
/*                                                                  */
/*    EDIT [<number>] SPLIT <word>                                  */
/*                                                                  */
/*    EDIT CANCEL                                                   */
/*       Abandon current editing session, original stays as is      */
/*                                                                  */
/*    EDIT DONE                                                     */
/*       Finish editing, save changes                               */
/*                                                                  */
/********************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"
#include "mysql.h"

extern struct board_index_struct board_index[];


#define MAX_EDIT_LENGTH 4000
#define WRAP_WIDTH      70


/* Module global variables */

static int num1, num2, argmask;
static EDIT_DATA *ed;


/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 * Shamelessly copied from 'one_argument()' but does NOT lowercase the args.
 */
static char *one_arg( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
        argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *arg_first = *argument;
        arg_first++;
        argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
        argument++;

    return argument;

} /* end one_arg() */


static char *next_line( char *current_line )
{
    char *limit = &ed->edit_string[MAX_STRING_LENGTH-1];
    char *pl;

    if ( *current_line == '\0' )
        return current_line;

    for ( pl = current_line; pl < limit; pl++ )
    {
        if ( *pl == '\r' )
            return ++pl;
    }
    return current_line;

} /* end next_line() */


int count_lines()
{
    int nlines = 0;
    char *line, *next;

    if ( ed == NULL )
    {
        bug( "count_lines(): not editing anything", 0 );
        return 0;
    }
    for ( line = ed->edit_string; (next = next_line( line )) != line; line = next )
        nlines++;
    return nlines;

} /* end count_lines() */


static char *find_line( int lineno )
{
    int line = 1;
    char *pl = ed->edit_string;

    if ( lineno < 1 )
    {
        bug( "find_line(0)", 0 );
        return pl;
    }
    if ( lineno > ed->edit_nlines + 1 )
    {
        bug( "find_line(): Line out of range", lineno );
        return pl;
    }
    while ( line++ < lineno )
        pl = next_line( pl );
    return pl;

} /* end find_line() */


static void listline( BUFFER *dbuf, int lineno, char *line )
{

    char buf[MAX_STRING_LENGTH];
    char *bp;

    if ( lineno == ed->edit_line )
        sprintf( buf, "{W<%3d> {x", lineno );
    else if ( lineno <= ed->edit_nlines )
        sprintf( buf, "{x %3d: {x", lineno );
    else
        sprintf( buf, "{x    : {x" );

    add_buf( dbuf, buf );
    bp = buf;
    if ( lineno == 0 )
        add_buf( dbuf, "~~~TOP~~~" );
    else if ( lineno == ed->edit_nlines + 1 )
        add_buf( dbuf, "~~~END~~~" );
    else
    {
        while ( *line != '\r' && *line != '\0' )
        {
            if ( *line != '\n' )
                *bp++ = *line;
            line++;
        }
    }
    strcpy( bp, "{x\r\n" );
    add_buf( dbuf, buf );

} /* end listline() */


static void edit_list1( CHAR_DATA *ch, int fromline, int toline )
{
    int jline;
    char *cp;
    BUFFER *dbuf;

    fromline = UMAX(fromline,0);
    fromline = UMIN(fromline,ed->edit_nlines);
    toline   = UMAX(toline,fromline);
    toline   = UMIN(toline,ed->edit_nlines+1);

    dbuf = new_buf();
    if ( fromline == 0 )
    {
        listline( dbuf, 0, "" );
        fromline = 1;
    }
    cp = find_line( fromline );
    for ( jline = fromline; jline <= toline; jline++ )
    {
        listline( dbuf, jline, cp );
        cp = next_line( cp );
    }
    page_to_char( buf_string(dbuf), ch );
    free_buf( dbuf );

} /* end edit_list1() */


static void list_window( CHAR_DATA *ch )
{
    int fromline, toline;

    if ( ed == NULL )
    {
        stc( "{P~~~ You are not editing anything at the moment. ~~~{x\n\r", ch );
        return;
    }
    if ( ed->edit_nlines <= 0 )
    {
        stc( "{P~~~ Edit buffer is empty ~~~{x\n\r", ch );
        return;
    }

    fromline = UMAX(ed->edit_line-2,0);
    toline   = UMIN(ed->edit_line+2,ed->edit_nlines+1);
    edit_list1( ch, fromline, toline );

} /* end list_window() */


static void edit_status( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch->edit == NULL )
    {
        stc( "{PYou aren't editing anything yet.{x\n\r", ch );
        if ( ch->pnote == NULL )
        {
            stc(
                "You could use {RNOTE{x, {RIDEA{x or {RROLEPLAY{x to start writing a note\n\r"
                "and then type {REDIT NOTE{x to edit it.\n\r", ch );
        }
        else
        {
            sprintf( buf, "You have started writing a(n) {Y%s{x.\n\r",
                board_index[ch->pnote->type].board_long );
            stc( buf, ch );
            stc( "You could use {REDIT NOTE{x to edit it.\n\r", ch );
        }
        stc( "Or you could use {REDIT DESC{x to edit {Yyour description{x.\n\r", ch );
        return;
    }

    ed = ch->edit;
    switch ( ed->edit_type )
    {
        case EDIT_TYPE_NONE:
            stc( "{PStrange, you are editing NOTHING!{x\n\r", ch );
            bug( "editing nothing!", 0 );
            return;
        case EDIT_TYPE_NOTE:
            if ( ch->pnote == NULL )
            {
                stc(
                    "{P You *were* editing a note of some sort.{x\n\r", ch );
                stc(
                    "{P Now your text is homeless. Please do {REDIT CANCEL{P.{x\n\r",
                    ch );
                ed->edit_type = EDIT_TYPE_NONE;
                return;
            }
            sprintf( buf, "You are currently editing a(n) {Y%s{x.\n\r",
                board_index[ch->pnote->type].board_long );
            stc( buf, ch );
            break;
        case EDIT_TYPE_DESC:
            stc( "You are currently editing {Yyour description{x.\n\r", ch );
            break;
        case EDIT_TYPE_ROOM:
            stc( "You are currently editing a {Yroom description{x.\n\r", ch );
            break;
        case EDIT_TYPE_HELP:
            ptc(ch, "You are currently editing the {Yhelp text{x for ID %d.\n\r", ed->edit_id );
            break;
        default:
            stc( "Strange, I don't know {PWHAT{x you're editing!\n\r", ch );
            bug( "Unknown edit type", 0 );
            return;
    }
    sprintf( buf, "You are on line {C%d{x of {C%d{x.\n\r",
        ed->edit_line, ed->edit_nlines );
    stc( buf, ch );
    stc(
        "{PCommands:{x {RED{xIT {RL{xIST / {RI{xNSERT / {RD{xELETE / "
                      "{RC{xHANGE / {RW{xRAP / {RS{xPLIT\n\r"
        "{x         {x {RED{xIT {RDO{xNE / {RCANCEL{x / {RUNDO{x\n\r", ch );
    stc(
        "See {RHELP EDIT{x for details.\n\r", ch );
    list_window( ch );

} /* end edit_status() */


static bool check_line( CHAR_DATA *ch, int line )
{
    char buf[MAX_INPUT_LENGTH];

    if ( line < 0 || line > ed->edit_nlines )
    {
        sprintf( buf, "{PThere is no line number %d{x.\n\r", line );
        stc( buf, ch );
        return FALSE;
    }
    return TRUE;

} /* end check_line() */


static bool check_range( CHAR_DATA *ch, int *fromline, int *toline )
{
    char buf[MAX_INPUT_LENGTH];

    if ( IS_SET(argmask,2) )
    {
        if ( !check_line( ch, num1 ) || !check_line( ch, num2 ) )
            return FALSE;
        if ( num2 > ed->edit_nlines )
        {
            num2 = ed->edit_nlines;
            sprintf( buf, "{PLast line adjusted to {Y%d{x.\n\r", num2 );
            stc( buf, ch );
        }
        else if ( num2 < num1 )
        {
            stc( "{PLine numbers out of sequence.{x\n\r", ch );
            return FALSE;
        }
        *fromline = num1;
        *toline = num2;
    }
    else if ( IS_SET(argmask,1) )
    {
        if ( !check_line( ch, num1 ) )
            return FALSE;
        *fromline = num1;
        *toline = num1;
    }
    else
    {
        *fromline = ed->edit_line;
        *toline = ed->edit_line;
    }
    return TRUE;

} /* end check_range() */


static void edit_goto1( CHAR_DATA * ch, int lineno )
{
    if ( lineno < 0 || lineno > ed->edit_nlines )
    {
        /* bug( "edit_goto1(%d)", lineno ); */
        lineno = UMAX(lineno,0);
        lineno = UMIN(lineno,ed->edit_nlines);
    }
    ed->edit_line = lineno;

} /* end edit_goto1() */


static bool is_blank_line( char *line )
{
    while ( *line != 0 && *line != '\r' )
    {
        if ( *line > ' ' )
            return FALSE;
        line++;
    }
    return TRUE;

} /* end is_blank_line() */


static void backup( void )
{
    strcpy( ed->edit_backup, ed->edit_string );
    ed->edit_undo_ok = TRUE;

} /* end backup() */


/********** Main functions **********/


static void edit_cancel( CHAR_DATA *ch, char *argument )
{
    free_mem( ed, sizeof(EDIT_DATA) );
    ch->edit = NULL;
    stc( "OK, editing session aborted, {Ynothing changed{x.\n\r", ch );

} /* end edit_cancel() */


static void edit_change( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int this_line = ed->edit_line;
    char *this, *next;
    char *where;
    char *end_pos;
    char end_char;
    BUFFER *dbuf;

    if ( IS_SET(argmask,1) )
    {
        if ( !check_line( ch, num1 ) )
        {
            return;
        }
        this_line = num1;
    }
    edit_goto1( ch, this_line );

    argument = one_arg( argument, arg1 );
    smash_tilde( arg1 );
    if ( arg1[0] == '\0' )
    {
        stc( "{PYou must specify a string to change.{x\n\r", ch );
        return;
    }
    argument = one_arg( argument, arg2 );
    smash_tilde( arg2 );

    if (  strlen( arg2 ) > strlen( arg1 )
       && strlen(ed->edit_string) + strlen( arg2 ) - strlen( arg1 ) > MAX_EDIT_LENGTH )
    {
        stc( "{PEdit limit exceeded - you cannot add any more text.{x\n\r", ch );
        return;
    }

    this = find_line( this_line );
    next = next_line( this );

    /* terminate current line so search will not run over */
    end_pos = next - 1;
    end_char = *end_pos;
    *end_pos = '\0';

    /* look for search string */
    where = strstr( this, arg1 );
    *end_pos = end_char;
    if ( where == NULL )
    {
        sprintf( arg2, "{PSearch string '%s{P' not found in current line.{x\n\r", arg1 );
        stc( arg2, ch );
        return;
    }

    backup();
    dbuf = new_buf();
    add_buf( dbuf, arg2 );
    add_buf( dbuf, where + strlen( arg1 ) );
    strcpy( where, buf_string(dbuf) );
    free_buf( dbuf );

} /* end edit_change() */


static void edit_delete( CHAR_DATA *ch, char *argument )
{
    int linefrom = ed->edit_line;
    int lineto = ed->edit_line;
    char *line1, *line9;

    if ( !check_range( ch, &linefrom, &lineto ) )
        return;

    if ( linefrom == 0 )
    {
        stc( "{PCannot delete line 0{x.\n\r", ch );
        return;
    }

    line1 = find_line( linefrom );

    backup();
    if ( lineto >= ed->edit_nlines )
    {
        *line1 = '\0';
    }
    else
    {
        line9 = find_line( lineto + 1 );
        strcpy( line1, line9 );
    }
    ed->edit_nlines = count_lines();
    ed->edit_line = UMAX(0,linefrom - 1);

} /* end edit_delete() */


static void edit_desc( CHAR_DATA *ch, char *argument )
{
    if ( ch->edit != NULL )
    {
        stc( "{PBut you are already editing something!{x\n\r", ch );
        edit_status( ch, "" );
        return;
    }

    char *str = ch->long_descr;
    if (!str)
        str = "";

    ed = alloc_mem( sizeof(EDIT_DATA) );
    ch->edit = ed;
    ed->edit_type = EDIT_TYPE_DESC;
    strcpy( ed->edit_string, str);
    backup();
    ed->edit_nlines = count_lines();
    edit_goto1( ch, 1 );
    edit_status( ch, "" );

} /* end edit_note() */


static void edit_done( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    switch( ed->edit_type )
    {
        default:
            stc( "{POops, I lost track of myself.{x\n\r", ch );
            bug( "edit_done(): unknown edit type", 0 );
            break;
        case EDIT_TYPE_NONE:
            stc( "{PHmm, looks like you weren't editing anything.{x\n\r", ch );
            bug( "edit_done(): edit type NONE", 0 );
            break;
        case EDIT_TYPE_NOTE:
            if ( ch->pnote == NULL )
            {
                stc( "{PI'm afraid your note is not there any more.{x\n\r"
                              "{PYour edited text is going down the drain now.{x\n\r", ch );
            }
            else
            {
                sprintf( buf, "OK, I'm saving your {Y%s{x.\n\r",
                    board_index[ch->pnote->type].board_long );
                stc( buf, ch );
                stc( "Don't forget to {RPOST{x it!\n\r", ch );
                free_string( ch->pnote->text );
                ch->pnote->text = str_dup( ed->edit_string );
            }
            break;
        case EDIT_TYPE_DESC:
            stc( "OK, I'm saving {Yyour description{x.\n\r", ch );

            if ( ch->long_descr != NULL )
                free_string( ch->long_descr );

            ch->description = str_dup( ed->edit_string );
            break;
        case EDIT_TYPE_ROOM:
            if ( ch->in_room == NULL )
            {
                stc( "{PI'm sorry, you don't seem to be in a room.{x\n\r", ch );
                return;
            }

            stc( "OK, I'm saving your {Yroom description{x.\n\r", ch );

            if ( ch->in_room->description != NULL )
                free_string( ch->in_room->description );

            ch->in_room->description = str_dup( ed->edit_string );
            break;
        case EDIT_TYPE_HELP:
            ptc(ch, "OK, I'm saving the {Yhelp text{x for ID %d.\n\r", ed->edit_id);
            db_commandf("edit_done", "update helps set text='%s' where id=%d", ed->edit_string, ed->edit_id);
            break;
    }
    free_mem( ed, sizeof(EDIT_DATA) );
    ch->edit = NULL;
    ed = NULL;

} /* end edit_done() */


static void edit_goto( CHAR_DATA *ch, char *argument )
{
    if ( check_line( ch, num1 ) )
        edit_goto1( ch, num1 );

} /* end edit_goto() */


static void edit_insert( CHAR_DATA *ch, char *argument )
{
    char *lp;
    BUFFER *dbuf;
    int after_line = ed->edit_line + 1;

    if ( IS_SET(argmask,1) )
    {
        if ( !check_line( ch, num1 ) )
            return;
        after_line = num1 + 1;
    }

    smash_tilde( argument );
    if ( strlen(argument) + strlen(ed->edit_string) + 1 > MAX_EDIT_LENGTH )
    {
        stc( "{PEdit limit exceeded - you cannot add any more text.{x\n\r", ch );
        return;
    }

    backup();
    dbuf = new_buf();
    add_buf( dbuf, argument );
    add_buf( dbuf, "\n\r" );
    lp = find_line( after_line );
    add_buf( dbuf, lp );
    strcpy( lp, buf_string(dbuf) );
    free_buf( dbuf );
    ed->edit_nlines = count_lines();
    edit_goto1( ch, after_line );

} /* end edit_insert() */


static void edit_list( CHAR_DATA *ch, char *argument )
{
    int fromline, toline;

    /* check args */
    if ( argmask == 0 )
    {
        edit_list1( ch, 0, ed->edit_nlines + 1 );
        return;
    }
    if ( !check_range( ch, &fromline, &toline ) )
        return;
    edit_list1( ch, fromline, toline );

} /* end edit_list() */


static void edit_note( CHAR_DATA *ch, char *argument )
{
    if ( ch->edit != NULL )
    {
        stc( "{PBut you are already editing something!{x\n\r", ch );
        edit_status( ch, "" );
        return;
    }

    if ( ch->pnote == NULL )
    {
        stc( "{PBut you haven't started writing any note yet!{x\n\r", ch );
        return;
    }

    ed = alloc_mem( sizeof(EDIT_DATA) );
    ch->edit = ed;
    ed->edit_type = EDIT_TYPE_NOTE;
    strcpy( ed->edit_string, ch->pnote->text );
    backup();
    ed->edit_nlines = count_lines();
    edit_goto1( ch, 1 );
    edit_status( ch, "" );

} /* end edit_note() */


static void edit_room( CHAR_DATA *ch, char *argument )
{
    if ( ch->edit != NULL )
    {
        stc( "{PBut you are already editing something!{x\n\r", ch );
        edit_status( ch, "" );
        return;
    }

    if ( ch->in_room == NULL )
    {
        stc( "{PYou don't seem to be in a room at the moment.{x\n\r", ch );
        return;
    }

    char *str = ch->in_room->description;
    if (!str)
        str = "";

    ed = alloc_mem( sizeof(EDIT_DATA) );
    ch->edit = ed;
    ed->edit_type = EDIT_TYPE_ROOM;
    strcpy( ed->edit_string, str);
    backup();
    ed->edit_nlines = count_lines();
    edit_goto1( ch, 1 );
    edit_status( ch, "" );

} /* end edit_room() */

static void edit_help( CHAR_DATA *ch, char *argument )
{
    if ( ch->edit != NULL )
    {
        stc( "{PBut you are already editing something!{x\n\r", ch );
        edit_status( ch, "" );
        return;
    }

    if (!argument[0] || !is_number(argument)) {
        stc("You must specify a help ID to edit it.", ch);
        return;
    }

    int id = atoi(argument);
    MYSQL_RES *result;
    MYSQL_ROW row;

    if ((result = db_queryf("edit_help", "select text from helps where id=%d", id)) == NULL) {
        stc("Couldn't retrieve a help with that ID.\n\r", ch);
        return;
    }

    row = mysql_fetch_row(result);
    char *str = row[0];
    if (!str)
        str = "";

    ed = alloc_mem( sizeof(EDIT_DATA) );
    ch->edit = ed;
    ed->edit_type = EDIT_TYPE_HELP;
    strcpy( ed->edit_string, str);
    backup();
    ed->edit_nlines = count_lines();
    ed->edit_id = id;
    edit_goto1( ch, 1 );
    edit_status( ch, "" );

    mysql_free_result(result);
} /* end edit_room() */


static void edit_split( CHAR_DATA *ch, char *argument )
{
    char token[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    int this_line = ed->edit_line;
    char *this, *next;
    char *where;
    char *end_pos;
    char end_char;
    BUFFER *dbuf;

    if ( IS_SET(argmask,1) )
    {
        if ( !check_line( ch, num1 ) )
        {
            return;
        }
        this_line = num1;
    }
    edit_goto1( ch, this_line );

    argument = one_arg( argument, token );
    if ( token[0] == '\0' )
    {
        stc( "{PYou must specify a string to split before.{x\n\r", ch );
        return;
    }

    this = find_line( ed->edit_line );
    next = next_line( this );

    /* terminate current line so search will not overrun */
    end_pos = next - 1;
    end_char = *end_pos;
    *end_pos = '\0';

    where = strstr( this, token );
    *end_pos = end_char;

    if ( where == NULL )
    {
        sprintf( buf, "{PCharacters '%s{P' not found in current line.{x\n\r", token );
        stc( buf, ch );
        return;
    }

    backup();
    dbuf = new_buf();
    add_buf( dbuf, where );
    strcpy( where, "\n\r" );
    strcat( where, buf_string(dbuf) );
    free_buf( dbuf );
    ed->edit_nlines = count_lines();

} /* end edit_split() */


static void edit_undo( CHAR_DATA *ch, char *junk )
{
    if ( !ed->edit_undo_ok )
    {
        stc( "{PSorry, you have already undone your most recent change!{x\n\r",
            ch );
        return;
    }
    strcpy( ed->edit_string, ed->edit_backup );
    ed->edit_nlines = count_lines();
    if ( ed->edit_line > ed->edit_nlines )
        edit_goto1( ch, ed->edit_line );
    stc( "{POK{x, your most recent change (if any) has been {Pundone{x.\n\r",
            ch );
    ed->edit_undo_ok = FALSE;

} /* end edit_undo() */


static void edit_wrap( CHAR_DATA *ch, char *argument )
{
    int prev_blank_line = 0;
    int linefrom, lineto;
    int foll_blank_line = ed->edit_nlines + 1;
    int lineno, linelen, wordlen;
    char *start, *after;
    char *cp, *wp, *lp;
    BUFFER *dbuf;
    char word[MAX_STRING_LENGTH];
    char line[MAX_INPUT_LENGTH];
    bool in_word;

    if ( argmask == 0 )
    {
        /* wrap current paragraph */
        lineno = 1;
        lp = ed->edit_string;
        while ( lineno <= ed->edit_nlines )
        {
            if ( is_blank_line( lp ) )
            {
                if ( lineno < ed->edit_line )
                {
                    prev_blank_line = lineno;
                }
                else if ( lineno > ed->edit_line )
                {
                    foll_blank_line = lineno;
                    break;
                }
                else
                {
                    /* wrapping on a empty line */
                    return;
                }
            }
            lp = next_line( lp );
            lineno ++;
        }
    }
    else
    {
        if ( !check_range( ch, &linefrom, &lineto ) )
        {
            return;
        }
        else
        {
            prev_blank_line = UMAX( linefrom - 1, 0 );
            foll_blank_line = UMIN( lineto + 1, ed->edit_nlines + 1 );
        }
    }

    /* we plan to wrap the text from after prev_blank_line to before foll_blank_line. */
    lineno = prev_blank_line + 1;
    start = find_line( lineno );
    if ( foll_blank_line <= ed->edit_nlines )
        after = find_line( foll_blank_line );
    else
        after = NULL;

    backup();
    dbuf = new_buf();

    wp = word;
    lp = line;
    linelen = 0;
    in_word = FALSE;
    for ( cp = start; lineno < foll_blank_line; cp++ )
    {
        if ( *cp > ' ' )
        {
            /* Found a printable character. Start or continue word. */
            *wp++ = *cp;
            in_word = TRUE;
        }
        else
        {
            if ( *cp == '\r' )
                lineno++;
            if ( in_word )
            {
                /* Found a nonprintable. Terminate word. */
                *wp = '\0';
                in_word = FALSE;
                if ( strlen( word ) == 0 )
                    continue;
                wordlen = color_strlen( word );
                /* If word is too long to fit in line, dump line and/or chop word. */
                while ( linelen + 1 + wordlen > WRAP_WIDTH )
                {
                    if ( linelen == 0 )
                    {
                        for ( wp = word; color_strlen( line ) < WRAP_WIDTH; )
                            *lp++ = *wp++;
                        strcpy( word, --wp );
                        wordlen = color_strlen( word );
                        lp--;
                    }
                    strcpy( lp, "\n\r" );
                    add_buf( dbuf, line );
                    lp = line;
                    linelen = 0;
                }
                if ( strlen( word ) == 0 )
                    continue;
                if ( linelen > 0 )
                {
                    *lp++ = ' ';
                    linelen++;
                }
                strcpy( lp, word );
                lp += strlen( word );
                linelen += wordlen;
                wp = word;
            }
        }
    }
    /* finish current line, if started */
    if ( lp > line )
    {
        strcpy( lp, "\n\r" );
        add_buf( dbuf, line );
    }
    if ( after != NULL )
        add_buf( dbuf, after );
    strcpy( start, buf_string(dbuf) );
    free_buf( dbuf );
    ed->edit_nlines = count_lines();
    edit_goto1( ch, prev_blank_line + 1 );

} /* end edit_wrap() */


/* Main edit function. Some pre-scanning, then branch to appropriate subfunction. */
void do_edit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    char *new_arg;

    argmask = 0;

    /* scan numeric args, if any. */
    if ( *argument != '\0' )
    {
        new_arg = one_argument( argument, arg );
        if ( is_number( arg ) )
        {
            num1 = atoi( arg );
            SET_BIT(argmask,1);
            argument = new_arg;
        }
    }
    if ( IS_SET(argmask,1) && *argument != '\0' )
    {
        new_arg = one_argument( argument, arg );
        if ( is_number( arg ) )
        {
            num2 = atoi( arg );
            SET_BIT(argmask,2);
            argument = new_arg;
        }
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' && argmask == 0 )
    {
        edit_status( ch, argument );
        return;
    }

    if ( strlen( arg ) >= 3 && !str_prefix1( arg, "description" ) )
    {
        edit_desc( ch, argument );
        return;
    }
    if ( !str_prefix1( arg, "note" ) )
    {
        edit_note( ch, argument );
        return;
    }
    if ( IS_IMMORTAL(ch) && !str_prefix1( arg, "room" ) )
    {
        edit_room( ch, argument );
        return;
    }

    if ( IS_IMMORTAL(ch) && !str_prefix1( arg, "help" ) )
    {
        edit_help( ch, argument );
        return;
    }

    if ( ch->edit == NULL )
    {
        ed = NULL;
        list_window( ch );
        return;
    }
    ed = ch->edit;

    /* below this point, character is sure to be editing something */

    if ( arg[0] == '\0' )
    {
        edit_goto( ch, argument );
        list_window( ch );
        return;
    }

    /* below this point, there is at least a keyword */

    if ( !str_prefix1( arg, "change" ) )
    {
        edit_change( ch, argument );
        list_window( ch );
        return;
    }
    if ( !str_prefix1( arg, "insert" ) )
    {
        edit_insert( ch, argument );
        list_window( ch );
        return;
    }

    if ( !str_prefix1( arg, "split" ) )
    {
        edit_split( ch, argument );
        list_window( ch );
        return;
    }

    /* below this point, functions want no args */

    if ( !str_cmp( arg, "cancel" ) )
    {
        edit_cancel( ch, argument );
        return;
    }
    if ( !str_prefix1( arg, "cance" ) )
    {
        stc(
            "{PIf you want to CANCEL your editing, write {RCANCEL{P out in full!{x\n\r",
            ch );
        return;
    }
    if ( !str_prefix1( arg, "delete" ) )
    {
        edit_delete( ch, argument );
        list_window( ch );
        return;
    }
    if ( !str_prefix1( arg, "done" ) )
    {
        edit_done( ch, argument );
        return;
    }

    if ( !str_prefix1( arg, "list" ) )
    {
        edit_list( ch, argument );
        return;
    }
    if ( !str_cmp( arg, "undo" ) )
    {
        edit_undo( ch, argument );
        list_window( ch );
        return;
    }
    if ( !str_prefix1( arg, "und" ) )
    {
        stc(
            "{PIf you want to UNDO your last change, write {RUNDO{P out in full!\n\r",
            ch );
        return;
    }
    if ( !str_prefix1( arg, "wrap" ) )
    {
        edit_wrap( ch, argument );
        list_window( ch );
        return;
    }

    sprintf( buf, "{PUnknown {CEDIT{x function '{R%s{P'.{x\n\r", arg );
    stc( buf, ch );
    edit_status( ch, "" );

} /* end do_edit() */


