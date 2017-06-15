/* I'm bloody sick of copyovers and crashes ruining our imm run quests, which
   usually have to do with items on the ground.  Here's the solution:  force
   the mud to save a list of all objects lying on the ground, in what vnum.
   It's clumsy, processor intensive, but I think it's warranted.  Later, when
   we have more confidence in our database, I'll do it there.  -- Montrey */

#include "file.hh"
#include "merc.hh"
#include "recycle.hh"
#include "Affect.hh"
#include "Format.hh"
#include "Reset.hh"

/* see if an object has contents that don't appear in it's 'put' resets, return
   TRUE if so.  we don't save normal objects that lie around */
bool has_modified_contents(Object *obj)
{
	Object *cobj;

	for (cobj = obj->contains; cobj; cobj = cobj->next_content)
		if (!cobj->reset
		    || cobj->reset->command != 'P'
		    || cobj->reset->arg3 != obj->pIndexData->vnum)
			return TRUE;

	return FALSE;
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
		return FALSE;

	// save the donation pit, can be uncommented in case of problems -- Montrey
//	if (obj == donation_pit)
//		return FALSE;

	switch (obj->item_type) {
	case ITEM_POTION:       /* usually have timers, and trying to speed this up */
	case ITEM_SCROLL:
	case ITEM_PILL:
	case ITEM_STAFF:
	case ITEM_WAND:
	case ITEM_CORPSE_NPC:
		return FALSE;
	}

	if ((obj->timer > 0 || obj->clean_timer > 0) && obj->item_type != ITEM_CORPSE_PC)
		return FALSE;

	/* let's see if the item resets in this room */
	if (obj->reset && obj->reset->command == 'O')
		if (obj->reset->arg3 == obj->in_room->vnum)
			if (!obj->contains || !has_modified_contents(obj))
				return FALSE;

	return TRUE;
}

/* write one object recursively.  this is the guts of fwrite_obj, simplified a bit. */
void fwrite_objstate(Object *obj, FILE *fp, int *count)
{
	Object *cobj;
	ExtraDescr *ed;
	int i = 0;
	bool enchanted = affect_enchanted_obj(obj); // whether to write affects or not

	(*count)++;

	Format::fprintf(fp, "OBJ\n%d %d %d %d ",
	        obj->pIndexData->vnum,
	        obj->in_room ? obj->in_room->vnum : 0,
	        enchanted ? 1 : 0,
	        obj->cost);

	/* write how many objects are contained inside */
	for (cobj = obj->contains; cobj; cobj = cobj->next_content)
		i++;

	Format::fprintf(fp, "%d\n", i);

	/* these data are only used if they do not match the defaults */
	if (obj->name != obj->pIndexData->name)
		Format::fprintf(fp, "N %s~\n", obj->name);

	if (obj->short_descr != obj->pIndexData->short_descr)
		Format::fprintf(fp, "S %s~\n", obj->short_descr);

	if (obj->description != obj->pIndexData->description)
		Format::fprintf(fp, "D %s~\n", obj->description);

	if (obj->material != obj->pIndexData->material)
		Format::fprintf(fp, "M %s~\n", obj->material);

	if (obj->extra_flags != obj->pIndexData->extra_flags)
		Format::fprintf(fp, "E %s\n", obj->extra_flags);

	if (obj->wear_flags != obj->pIndexData->wear_flags)
		Format::fprintf(fp, "W %s\n", obj->wear_flags);

	if (obj->item_type != obj->pIndexData->item_type)
		Format::fprintf(fp, "T %d\n",  obj->item_type);

	if (obj->weight != obj->pIndexData->weight)
		Format::fprintf(fp, "G %d\n",  obj->weight);

	if (obj->condition != obj->pIndexData->condition)
		Format::fprintf(fp, "C %d\n",  obj->condition);

	if (obj->level != obj->pIndexData->level)
		Format::fprintf(fp, "L %d\n",  obj->level);

	if (obj->value[0] != obj->pIndexData->value[0]
	    || obj->value[1] != obj->pIndexData->value[1]
	    || obj->value[2] != obj->pIndexData->value[2]
	    || obj->value[3] != obj->pIndexData->value[3]
	    || obj->value[4] != obj->pIndexData->value[4])
		Format::fprintf(fp, "V %d %d %d %d %d\n",
		        obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4]);

	if (enchanted) {
		for (const Affect *paf = affect_list_obj(obj); paf; paf = paf->next) {
			if (paf->type < 0 || paf->type >= skill_table.size())
				continue;

			Format::fprintf(fp, "A '%s' %3d %3d %3d %3d %3d %10d %d\n",
			        skill_table[paf->type].name,
			        paf->where,
			        paf->level,
			        paf->duration,
			        paf->modifier,
			        paf->location,
			        paf->bitvector(),
			        paf->evolution ? paf->evolution : 1);
		}
	}

	for (ed = obj->extra_descr; ed; ed = ed->next)
		Format::fprintf(fp, "X %s~ %s~\n", ed->keyword, ed->description);

	Format::fprintf(fp, "Z\n\n");

	/* recursively write it's contents */
	for (cobj = obj->contains; cobj; cobj = cobj->next_content)
		fwrite_objstate(cobj, fp, count);
}

/* loop through the object list, save all items that are lying on the ground
   to disk.  This is called occasionally from update_handler, and selectively
   from some parts of get_obj and drop_obj */
int objstate_save_items()
{
	FILE *fp;
	Object *obj;
	int count = 0;

	if (port != DIZZYPORT)
		return 0;

	if ((fp = fopen(COPYOVER_ITEMS, "w")) == nullptr) {
		bugf("Could not write to copyover file: %s", COPYOVER_ITEMS);
		return 0;
	}

	for (obj = object_list; obj != nullptr; obj = obj->next)
		if (is_worth_saving(obj))
			fwrite_objstate(obj, fp, &count);

	Format::fprintf(fp, "END\n");
	fclose(fp);
	return count;
}

Object *fload_objstate(FILE *fp, int *count)
{
	RoomPrototype *room;
	Object *obj, *cobj;
	bool extract = FALSE, done = FALSE;
	int rvnum, nests, ovnum, enchanted;

	if (feof(fp))
		return nullptr;

	if (fread_word(fp) != "OBJ")
		return nullptr;

	ovnum = fread_number(fp);

	if (get_obj_index(ovnum) == nullptr) {
		obj = create_object(get_obj_index(GEN_OBJ_TREASURE), 0);

		if (obj)
			extract = TRUE;
		else {
			bug("Memory error creating TREASURE object.", 0);
			return nullptr;
		}
	}
	else if ((obj = create_object(get_obj_index(ovnum), 0)) == nullptr) {
		/* make a temp object, we'll extract it later, so we can read the rest of the list */
		extract = TRUE;
		obj = create_object(get_obj_index(GEN_OBJ_TREASURE), 0);

		if (! obj) {
			bug("Error creating TREASURE object.", 0);
			return nullptr;
		}
	}

	(*count)++;
	rvnum           = fread_number(fp);
	enchanted       = fread_number(fp);
	obj->cost       = fread_number(fp);
	nests           = fread_number(fp);

	if (enchanted)
		affect_remove_all_from_obj(obj, TRUE); // read them from the file

	if (ovnum == OBJ_VNUM_PIT && donation_pit == nullptr) {
		donation_pit = obj;
		Format::printf("Loading donation pit with %d items.\n", nests);
	}

	while (!done) { /* loop over all lines of obj desc */
		switch (fread_letter(fp)) {
		case 'A': {
				Affect af;

				af.type = skill_lookup(fread_word(fp));

				if (af.type < 0) {
					fread_to_eol(fp);
					continue;
				}

				af.where      = fread_number(fp);
				af.level      = fread_number(fp);
				af.duration   = fread_number(fp);
				af.modifier   = fread_number(fp);
				af.location   = fread_number(fp);
				af.bitvector(fread_flag(fp));
				af.evolution  = fread_number(fp);
				affect_copy_to_obj(obj, &af);
				break;
			}

		case 'C':
			obj->condition = fread_number(fp);
			break;

		case 'D':
			obj->description = fread_string(fp);
			break;

		case 'E':
			obj->extra_flags = fread_flag(fp);
			break;

		case 'G':
			obj->weight = fread_number(fp);
			break;

		case 'L':
			obj->level = fread_number(fp);
			break;

		case 'M':
			obj->material = fread_string(fp);
			break;

		case 'N':
			obj->name = fread_string(fp);
			break;

		case 'S':
			obj->short_descr = fread_string(fp);
			break;

		case 'T':
			obj->item_type = fread_number(fp);
			break;

		case 'V':
			obj->value[0] = fread_number(fp);
			obj->value[1] = fread_number(fp);
			obj->value[2] = fread_number(fp);
			obj->value[3] = fread_number(fp);
			obj->value[4] = fread_number(fp);
			break;

		case 'W':
			obj->wear_flags = fread_flag(fp);
			break;

		case 'X': {
				ExtraDescr *ed;
				ed = new_extra_descr();
				ed->keyword     = fread_string(fp);
				ed->description = fread_string(fp);
				ed->next        = obj->extra_descr;
				obj->extra_descr = ed;
				break;
			}

		case 'Z':
			done = TRUE;
			break;

		default:
			bug("fload_objstate: no match", 0);
			fread_to_eol(fp);
			break;
		}
	}

	/* load it's contents */
	for (int i = 0; i < nests; i++) {
		if ((cobj = fload_objstate(fp, count)) != nullptr)
			obj_to_obj(cobj, obj);
	}

	/* if it wasn't found, extract it */
	if (extract)
		extract_obj(obj);
	else if (rvnum > 0) {
		if ((room = get_room_index(rvnum)) == nullptr)
			/* room not found, extract the object */
			extract_obj(obj);
		else {
			/* see if the original object is already in the room, if it is,
			   extract it to be replaced by the loaded one */
			for (cobj = room->contents; cobj; cobj = cobj->next_content)
				if (cobj->pIndexData->vnum == obj->pIndexData->vnum) {
					obj_from_room(cobj);
					extract_obj(cobj);
					break;
				}

			obj_to_room(obj, room);
		}
	}

	return obj;
}

int objstate_load_items()
{
	FILE *fp;
	int count = 0;

	if ((fp = fopen(COPYOVER_ITEMS, "r")) == nullptr) {
		bugf("Could not open copyover file: %s", COPYOVER_ITEMS);
		return 0;
	}

	for (; ;)
		if (!fload_objstate(fp, &count))        /* load, check for end just in case */
			break;

	fclose(fp);
	return count;
}

