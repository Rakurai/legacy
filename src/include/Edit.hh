#pragma once

#include "declare.hh"

class Edit
{
public:
    Edit() {}
    virtual ~Edit() {}

    int edit_type = 0;
    int edit_line = 0;
    int edit_nlines = 0;
    bool edit_undo_ok = FALSE;
    char edit_string[MAX_STRING_LENGTH] = {0};
    char edit_backup[MAX_STRING_LENGTH] = {0};
    int edit_id = 0;

private:
	Edit(const Edit&);
	Edit& operator=(const Edit&);
};

#define EDIT_TYPE_NONE   0
#define EDIT_TYPE_NOTE   1
#define EDIT_TYPE_DESC   2
#define EDIT_TYPE_ROOM   3
#define EDIT_TYPE_HELP   4
