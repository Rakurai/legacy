#pragma once

#include "memory.hh"

class Edit
{
public:
    Edit() {}
    virtual ~Edit() {}

    int edit_type = 0;
    int edit_line = 0;
    int edit_nlines = 0;
    bool edit_undo_ok = false;
    char edit_string[MAX_STRING_LENGTH] = {0};
    char edit_backup[MAX_STRING_LENGTH] = {0};
    int edit_id = 0;

private:
	Edit(const Edit&);
	Edit& operator=(const Edit&);
};

/**
 * @defgroup EditTypes Constants defining edit types
 * @{
 */
constexpr int EDIT_TYPE_NONE = 0;
constexpr int EDIT_TYPE_NOTE = 1;
constexpr int EDIT_TYPE_DESC = 2;
constexpr int EDIT_TYPE_ROOM = 3;
constexpr int EDIT_TYPE_HELP = 4;
/** @} */
