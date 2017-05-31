/*
 * Departed.c
 *
 * Routines to handle departed immortals
 *
 */

#include "merc.h"
#include "memory.h"

DEPARTED_DATA *departed_list_head;
DEPARTED_DATA *departed_list_tail;

char *departed_list_line = "            {c|  {g\\{WV{g/  {c|      {k.{W";

void load_departed_list()
{
	char *buf;
	FILE *fp;
	departed_list_head = (DEPARTED_DATA *)alloc_mem(sizeof(DEPARTED_DATA));
	departed_list_tail = (DEPARTED_DATA *)alloc_mem(sizeof(DEPARTED_DATA));
	departed_list_head->next = departed_list_tail;
	departed_list_tail->previous = departed_list_head;

	if ((fp = fopen(DEPARTED_FILE, "r")) == NULL) {
		bug("Departed: failed loading departed_file.", 0);
		goto bailout;
	}

	for (;;) {
		buf = fread_word(fp);

		if (buf[0] == '#')
			break;

		insert_departed(buf);
	}

	fclose(fp);
bailout:
	return;
}

void save_departed_list()
{
	DEPARTED_DATA *iterator;
	FILE *fp;

	if ((fp = fopen(DEPARTED_FILE, "w")) == NULL) {
		bug("Departed: failed saving departed_file.", 0);
		goto bailout;
	}

	iterator = departed_list_head->next;

	while (iterator != departed_list_tail) {
		Format::fprintf(fp, "%s\n", iterator->name);
		iterator = iterator->next;
	}

	Format::fprintf(fp, "#\n");
	fclose(fp);
bailout:
	return;
}

void insert_departed(const String& name)
{
	DEPARTED_DATA *iterator, *newDeparted;
	iterator = departed_list_head->next;
	newDeparted = (DEPARTED_DATA *)alloc_mem(sizeof(DEPARTED_DATA));
	newDeparted->name = name;

	while (iterator != departed_list_tail) {
		if (strcasecmp(newDeparted->name, iterator->name) < 1) {
			newDeparted->previous = iterator->previous;
			newDeparted->previous->next = newDeparted;
			newDeparted->next = iterator;
			iterator->previous = newDeparted;
			return;
		}

		iterator = iterator->next;
	}

	iterator = iterator->previous;
	iterator->next = newDeparted;
	newDeparted->previous = iterator;
	newDeparted->next = departed_list_tail;
	departed_list_tail->previous = newDeparted;
}

void remove_departed(const String& name)
{
	DEPARTED_DATA *iterator;
	iterator = departed_list_head->next;

	while (iterator != departed_list_tail) {
		if (!strcasecmp(name, iterator->name)) {
			DEPARTED_DATA *n = iterator->next;
			DEPARTED_DATA *p = iterator->previous;
			p->next = n;
			n->previous = p;
			free_mem(iterator, sizeof(DEPARTED_DATA));
			return;
		}

		iterator = iterator->next;
	}
}

bool has_departed(const String& name)
{
	DEPARTED_DATA *iterator;
	iterator = departed_list_head->next;

	while (iterator != departed_list_tail) {
		if (!strcasecmp(name, iterator->name))
			return TRUE;

		iterator = iterator->next;
	}

	return FALSE;
}
