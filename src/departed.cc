/*
 * Departed.c
 *
 * Routines to handle departed immortals
 *
 */

#include "declare.hh"
#include "DepartedPlayer.hh"
#include "file.hh"
#include "Format.hh"
#include "Logging.hh"
#include "merc.hh"
#include "String.hh"

DepartedPlayer *departed_list_head;
DepartedPlayer *departed_list_tail;

char *departed_list_line = "            {c|  {g\\{WV{g/  {c|      {k.{W";

void load_departed_list()
{
	FILE *fp;
	departed_list_head = new DepartedPlayer;
	departed_list_tail = new DepartedPlayer;
	departed_list_head->next = departed_list_tail;
	departed_list_tail->previous = departed_list_head;

	if ((fp = fopen(DEPARTED_FILE, "r")) == nullptr) {
		Logging::bug("Departed: failed loading departed_file.", 0);
		goto bailout;
	}

	for (;;) {
		String buf = fread_word(fp);

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
	DepartedPlayer *iterator;
	FILE *fp;

	if ((fp = fopen(DEPARTED_FILE, "w")) == nullptr) {
		Logging::bug("Departed: failed saving departed_file.", 0);
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
	DepartedPlayer *iterator, *newDeparted;
	iterator = departed_list_head->next;
	newDeparted = new DepartedPlayer;
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
	DepartedPlayer *iterator;
	iterator = departed_list_head->next;

	while (iterator != departed_list_tail) {
		if (!strcasecmp(name, iterator->name)) {
			DepartedPlayer *n = iterator->next;
			DepartedPlayer *p = iterator->previous;
			p->next = n;
			n->previous = p;
			delete iterator;
			return;
		}

		iterator = iterator->next;
	}
}

bool has_departed(const String& name)
{
	DepartedPlayer *iterator;
	iterator = departed_list_head->next;

	while (iterator != departed_list_tail) {
		if (!strcasecmp(name, iterator->name))
			return true;

		iterator = iterator->next;
	}

	return false;
}
