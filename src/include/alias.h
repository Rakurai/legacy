#ifndef ALIAS_H
#define ALIAS_H

typedef struct {
	char *text;
	char *sub;

	alias_t *next;
} alias_t;

#endif // ALIAS_H
