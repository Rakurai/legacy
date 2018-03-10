#include "constants.hh"
#include "declare.hh"
#include "file.hh"
#include "Format.hh"
#include "Logging.hh"
#include "StoredPlayer.hh"
#include "String.hh"

void read_line(FILE *, StoredPlayer *);
void save_line(FILE *, StoredPlayer *);

/**********************************************************************/

StoredPlayer *storage_list_head;
StoredPlayer *storage_list_tail;

/**********************************************************************/

void read_line(FILE *fp, StoredPlayer *sd)
{
	sd->name = fread_string(fp);
	sd->by_who = fread_string(fp);
	sd->date = fread_string(fp);
}

void save_line(FILE *fp, StoredPlayer *sd)
{
	Format::fprintf(fp, "%s~\n", sd->name);
	Format::fprintf(fp, "%s~\n", sd->by_who);
	Format::fprintf(fp, "%s~\n\n", sd->date);
}

void insert_storagedata(StoredPlayer *newdata)
{
	StoredPlayer *i;
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

void remove_storagedata(StoredPlayer *olddata)
{
	StoredPlayer *i;
	i = storage_list_head->next;

	while (i != storage_list_tail) {
		if (!strcasecmp(olddata->name, i->name)) {
			StoredPlayer *p = i->previous;
			StoredPlayer *n = i->next;
			p->next = n;
			n->previous = p;
			delete i;
			return;
		}

		i = i->next;
	}
}

StoredPlayer *lookup_storage_data(const String& name)
{
	StoredPlayer *i;
	i = storage_list_head->next;

	while (i != storage_list_tail) {
		if (!strcasecmp(i->name, name))
			return i;

		i = i->next;
	}

	return nullptr;
}

int count_stored_characters()
{
	StoredPlayer *i;
	int count = 0;
	i = storage_list_head->next;

	while (i != storage_list_tail) {
		count++;
		i = i->next;
	}

	return count;
}

void load_storage_list()
{
	FILE *fp;
	int count, i;
	storage_list_head = new StoredPlayer;
	storage_list_tail = new StoredPlayer;
	storage_list_head->next = storage_list_tail;
	storage_list_tail->previous = storage_list_head;

	if ((fp = fopen(STORAGE_FILE, "r")) == nullptr) {
		Logging::bug("load_storage_list: Cannot open STORAGE_FILE!", 0);
		return;
	}

	fscanf(fp, "%d\n", &count);

	for (i = 0; i < count; i++) {
		StoredPlayer *newData = new StoredPlayer;

		if (newData == nullptr) {
			Logging::bug("Failed to allocate memory for StoredPlayer!", 0);
			return;
		}

		read_line(fp, newData);
		insert_storagedata(newData);
	}

	fclose(fp);
	Format::printf("%d characters in storage found.\n", count);
}

void save_storage_list()
{
	FILE *fp;
	StoredPlayer *i;

	if ((fp = fopen(STORAGE_FILE, "w")) == nullptr) {
		Logging::bug("save_storage_list: Cannot open STORAGE_FILE!", 0);
		return;
	}

	Format::fprintf(fp, "%d\n", count_stored_characters());
	i = storage_list_head->next;

	while (i != storage_list_tail) {
		save_line(fp, i);
		i = i->next;
	}

	fclose(fp);
}
