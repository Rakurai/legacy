#pragma once

#include "String.hpp"
#include "Format.hpp"

/* stuff for providing a crash-proof buffer */

#define MAX_BUF	      65536
#define MAX_BUF_LIST     12
#define BASE_BUF       1024

/* valid states */
#define BUFFER_SAFE	      0
#define BUFFER_OVERFLOW	  1
#define BUFFER_FREED 	  2

typedef struct buf_type
{
    struct buf_type *    next;
    bool        valid;
    int      state;  /* error state of the buffer */
    long      size;   /* size in k */
    char *      string; /* buffer's string */
} BUFFER;

/* buffer procedures */
BUFFER	*new_buf();
BUFFER  *new_buf_size(long size);
void	free_buf(BUFFER *buffer);
bool	add_buf(BUFFER *buffer, const String& string);
void	clear_buf(BUFFER *buffer);
char	*buf_string(BUFFER *buffer);

extern void bug(const String&, int);

// print stuff, append to buffer. safe.
template<class... Params>
int ptb(BUFFER *buffer, const String& fmt, Params&&... params)
{
	char buf[MAX_BUF];
	int res = Format::snprintf(buf, MAX_BUF, fmt.c_str(), params...);

	if (res >= MAX_BUF - 1) {
		buf[0] = '\0';
		bug("print_to_buffer: overflow to buffer, aborting", 0);
	}
	else
		add_buf(buffer, buf);

	return res;
}
