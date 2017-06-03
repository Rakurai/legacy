#pragma once

#define NOTE_NOTE       0
#define NOTE_IDEA       1
#define NOTE_ROLEPLAY   2
#define NOTE_IMMQUEST   3
#define NOTE_CHANGES    4
#define NOTE_PERSONAL   5
#define NOTE_TRADE      6

class Note
{
public:
    Note() : next(nullptr) {}
    virtual ~Note() {}

    Note      * next;
    bool        valid;
    int         type;
    String      sender;
    String      date;
    String      to_list;
    String      subject;
    String      text;
    time_t      date_stamp;

private:
    Note(const Note&);
    Note& operator=(const Note&);
};

struct board_index_struct
{
    char *board_hdr;
    Note **board_list;
    char *board_short;
    char *board_plural;
    char *board_long;
};

/* note.c */
/* note recycling */
Note *new_note();
void    free_note(Note *note);
String  format_string( const String& oldstring );
