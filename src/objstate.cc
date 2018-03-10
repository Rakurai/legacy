/* I'm bloody sick of copyovers and crashes ruining our imm run quests, which
   usually have to do with items on the ground.  Here's the solution:  force
   the mud to save a list of all objects lying on the ground, in what vnum.
   It's clumsy, processor intensive, but I think it's warranted.  Later, when
   we have more confidence in our database, I'll do it there.  -- Montrey */

#include <vector>

#include "affect/Affect.hh"
#include "JSON/cJSON.hh"
#include "declare.hh"
#include "ExtraDescr.hh"
#include "file.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "Logging.hh"
#include "merc.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"
#include "ObjectValue.hh"
#include "Reset.hh"
#include "Room.hh"
#include "String.hh"
#include "World.hh"

extern int CURRENT_VERSION;
extern cJSON *fwrite_obj(Object *);
extern Object *fread_obj(cJSON *, int);

/* see if an object has contents that don't appear in it's 'put' resets, return
   true if so.  we don't save normal objects that lie around */
bool has_modified_contents(Object *obj)
{
	Object *cobj;

	for (cobj = obj->contains; cobj; cobj = cobj->next_content)
		if (!cobj->reset
		    || cobj->reset->command != 'P'
		    || cobj->reset->arg3 != obj->pIndexData->vnum)
			return true;

	return false;
}

/* determine if it's worth writing this object to the list.  only things that
   are in rooms, no stuff with timers, no potions/scrolls/pills/staves/wands/
   npc corpses.  check if it resets on the ground, if it does, only save it
   if it's contents are modified */
bool is_worth_saving(Object *obj)
{
	if (!obj->in_room       /* only items laying around */
	    || obj->carried_by     /* shouldn't be... */
	    || obj->in_obj)        /* shouldn't be... */
		return false;

	// save the donation pit, can be uncommented in case of problems -- Montrey
//	if (obj == Game::world().donation_pit)
//		return false;

	switch (obj->item_type) {
	case ITEM_POTION:       /* usually have timers, and trying to speed this up */
	case ITEM_SCROLL:
	case ITEM_PILL:
	case ITEM_STAFF:
	case ITEM_WAND:
	case ITEM_CORPSE_NPC:
		return false;
	}

	if ((obj->timer > 0 || obj->clean_timer > 0) && obj->item_type != ITEM_CORPSE_PC)
		return false;

	/* let's see if the item resets in this room */
	if (obj->reset && obj->reset->command == 'O')
		if (Location((int)obj->reset->arg3) == obj->in_room->location)
			if (!obj->contains || !has_modified_contents(obj))
				return false;

	return true;
}

/* loop through the object list, save all items that are lying on the ground
   to disk.  This is called occasionally from update_handler, and selectively
   from some parts of get_obj and drop_obj */
int objstate_save_items()
{
	if (Game::port != DIZZYPORT)
		return 0;

	cJSON *root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "version", CURRENT_VERSION);

	cJSON *objects = cJSON_CreateArray();

	for (Object *obj = Game::world().object_list; obj != nullptr; obj = obj->next) {
		if (is_worth_saving(obj)) {
			cJSON *o = cJSON_CreateObject();
			cJSON_AddNumberToObject(o, "room", obj->in_room->location.to_int());
			cJSON_AddItemToObject(o, "obj", fwrite_obj(obj));
			cJSON_AddItemToArray(objects, o);
		}
	}

	cJSON_AddItemToObject(root, "objects", objects);
	char *JSONstring = cJSON_Print(root);
	cJSON_Delete(root);

	FILE *fp;
	if ((fp = fopen(TEMP_FILE, "w")) != nullptr) {
		fputs(JSONstring, fp);
		fclose(fp);
		rename(TEMP_FILE, COPYOVER_ITEMS ".json");
	}
	else {
		Logging::bugf("Could not write to copyover file: %s", COPYOVER_ITEMS);
	}

	free(JSONstring);
	return 0;
}

int count_items(const Object *obj) {
	int count = 1; // this item

	for (const Object *c = obj->contains; c != nullptr; c = c->next_content)
		count += count_items(c);

	return count;
}

int objstate_load_items() {
	cJSON *root = JSON::read_file(COPYOVER_ITEMS ".json");

	if (root == nullptr)
		return 0;

	int version = CURRENT_VERSION;
	JSON::get_int(root, &version, "version");

	cJSON *objects = cJSON_GetObjectItem(root, "objects");
	int total_count = 0;

	for (cJSON *o = objects->child; o; o = o->next) {
		int room_loc_int;
		JSON::get_int(o, &room_loc_int, "room");
		Room *room = Game::world().get_room(Location(room_loc_int));

		if (room == nullptr)
			continue;

		Object *obj = fread_obj(cJSON_GetObjectItem(o, "obj"), version);

		if (obj == nullptr)
			continue;

		/* see if the original object is already in the room, if it is,
		   extract it to be replaced by the loaded one */
		for (Object *cobj = room->contents; cobj != nullptr; cobj = cobj->next_content) {
			if (cobj->pIndexData->vnum == obj->pIndexData->vnum) {
				obj_from_room(cobj);
				extract_obj(cobj);
				break;
			}
		}

		int count = count_items(obj);

		if (obj->pIndexData->vnum == OBJ_VNUM_PIT && Game::world().donation_pit == nullptr) {
			Game::world().donation_pit = obj;
			Format::printf("Loaded donation pit with %d items.\n", count);
		}

		obj_to_room(obj, room);
		total_count += count;
	}

	cJSON_Delete(root); // finished with it
	return total_count;
}
