//#include <sys/types.h>
//#include <stdio.h>
//#include <string.h>
#include "merc.h"


void read_line(FILE *, STORAGE_DATA *);
void save_line(FILE *, STORAGE_DATA *);


/**********************************************************************/

STORAGE_DATA *storage_list_head;
STORAGE_DATA *storage_list_tail;

/**********************************************************************/

void read_line(FILE *fp, STORAGE_DATA *sd)
{
	sd->name = str_dup(fread_string(fp));
	sd->by_who = str_dup(fread_string(fp));
	sd->date = str_dup(fread_string(fp));
}

void save_line(FILE *fp, STORAGE_DATA *sd)
{
	fprintf(fp, "%s~\n\r", sd->name);
	fprintf(fp, "%s~\n\r", sd->by_who);
	fprintf(fp, "%s~\n\r\n\r", sd->date);
}

void load_storage_list()
{
	FILE *fp;
	int count, i;
	storage_list_head = alloc_mem(sizeof(STORAGE_DATA));
	storage_list_tail = alloc_mem(sizeof(STORAGE_DATA));
	storage_list_head->next = storage_list_tail;
	storage_list_tail->previous = storage_list_head;

	if ((fp = fopen(STORAGE_FILE, "r")) == NULL) {
		bug("load_storage_list: Cannot open STORAGE_FILE!", 0);
		return;
	}

	fscanf(fp, "%d\n", &count);

	for (i = 0; i < count; i++) {
		STORAGE_DATA *newData = alloc_mem(sizeof(STORAGE_DATA));

		if (newData == NULL) {
			bug("Failed to allocate memory for STORAGE_DATA!", 0);
			return;
		}

		read_line(fp, newData);
		insert_storagedata(newData);
	}

	fclose(fp);
	printf("%d characters in storage found.\n\r", count);
}

void save_storage_list()
{
	FILE *fp;
	STORAGE_DATA *i;

	if ((fp = fopen(STORAGE_FILE, "w")) == NULL) {
		bug("save_storage_list: Cannot open STORAGE_FILE!", 0);
		return;
	}

	fprintf(fp, "%d\n", count_stored_characters());
	i = storage_list_head->next;

	while (i != storage_list_tail) {
		save_line(fp, i);
		i = i->next;
	}

	fclose(fp);
}


void insert_storagedata(STORAGE_DATA *newdata)
{
	STORAGE_DATA *i;
	i = storage_list_head->next;

	while (i != storage_list_tail) {
		if (strcasecmp(newdata->name, i->name) < 1) {
			newdata->previous = i->previous;
			newdata->previous->next = newdata;
			newdata->next = i;
			i->previous = newdata;
			return;
		}

		i = i->next;
	}

	i = storage_list_tail->previous;
	i->next = newdata;
	newdata->previous = i;
	newdata->next = storage_list_tail;
	storage_list_tail->previous = newdata;
	return;
}

void remove_storagedata(STORAGE_DATA *olddata)
{
	STORAGE_DATA *i;
	i = storage_list_head->next;

	while (i != storage_list_tail) {
		if (!strcasecmp(olddata->name, i->name)) {
			STORAGE_DATA *p = i->previous;
			STORAGE_DATA *n = i->next;
			p->next = n;
			n->previous = p;
			free_mem(i, sizeof(STORAGE_DATA));
			return;
		}

		i = i->next;
	}
}

STORAGE_DATA *lookup_storage_data(char *name)
{
	STORAGE_DATA *i;
	i = storage_list_head->next;

	while (i != storage_list_tail) {
		if (!strcasecmp(i->name, name))
			return i;

		i = i->next;
	}

	return NULL;
}

int count_stored_characters()
{
	STORAGE_DATA *i;
	int count = 0;
	i = storage_list_head->next;

	while (i != storage_list_tail) {
		count++;
		i = i->next;
	}

	return count;
}
