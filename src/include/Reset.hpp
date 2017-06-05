#pragma once

/*
 * Area-reset definition.
 */
class Reset
{
public:
	Reset() {}
	virtual ~Reset() {}

    Reset *        next = NULL;
    sh_int		version = 0;	/* Reset versioning -- Montrey */
    char                command = 0;
    sh_int              arg1 = 0;
    sh_int              arg2 = 0;
    sh_int              arg3 = 0;
    sh_int              arg4 = 0;

private:
	Reset(const Reset&);
	Reset& operator=(const Reset&);
};

/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */
