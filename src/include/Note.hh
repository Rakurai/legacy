#pragma once

#include "String.hh"
#include "Pooled.hh"

constexpr int NOTE_NOTE       = 0;
constexpr int NOTE_IDEA       = 1;
constexpr int NOTE_ROLEPLAY   = 2;
constexpr int NOTE_IMMQUEST   = 3;
constexpr int NOTE_CHANGES    = 4;
constexpr int NOTE_PERSONAL   = 5;
constexpr int NOTE_TRADE      = 6;

class Note :
public Pooled<Note>
{
public:
    Note() {}
    virtual ~Note() {}

    Note      * next = nullptr;
    int         type = 0;
    String      sender;
    String      date;
    String      to_list;
    String      subject;
    String      text;
    time_t      date_stamp = 0;

private:
    Note(const Note&);
    Note& operator=(const Note&);
};

struct board_index_struct
{
    const String board_hdr;
    Note **board_list;
    const String board_short;
    const String board_plural;
    const String board_long;
};

extern const std::vector<board_index_struct> board_index;

/* note.c */
/* note recycling */
Note *new_note();
void    free_note(Note *note);
String  format_string( const String& oldstring );
void    load_notes(void);
