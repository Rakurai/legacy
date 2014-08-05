/* I'm bloody sick of copyovers and crashes ruining our imm run quests, which
   usually have to do with items on the ground.  Here's the solution:  force
   the mud to save a list of all objects lying on the ground, in what vnum.
   It's clumsy, processor intensive, but I think it's warranted.  Later, when
   we have more confidence in our database, I'll do it there.  -- Montrey */

#include "merc.h"

/* see if an object has contents that don't appear in it's 'put' resets, return
   TRUE if so.  we don't save normal objects that lie around */
bool has_modified_contents(OBJ_DATA *obj)
{
	OBJ_DATA *cobj;

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
bool is_worth_saving(OBJ_DATA *obj)
{
	if (!obj->in_room       /* only items laying around */
	    || obj->carried_by     /* shouldn't be... */
	    || obj->in_obj)        /* shouldn't be... */
		return FALSE;

	if (obj == donation_pit)
		return FALSE;

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
void fwrite_objstate(OBJ_DATA *obj, FILE *fp)
{
	OBJ_DATA *cobj;
	AFFECT_DATA *paf;
	EXTRA_DESCR_DATA *ed;
	int i = 0;
	fprintf(fp, "OBJ\n%d %d %d %d ",
	        obj->pIndexData->vnum,
	        obj->in_room ? obj->in_room->vnum : 0,
	        obj->enchanted ? 1 : 0,
	        obj->cost);

	/* write how many objects are contained inside */
	for (cobj = obj->contains; cobj; cobj = cobj->next_content)
		i++;

	fprintf(fp, "%d\n", i);

	/* these data are only used if they do not match the defaults */
	if (obj->name != obj->pIndexData->name)
		fprintf(fp, "N %s~\n", obj->name);

	if (obj->short_descr != obj->pIndexData->short_descr)
		fprintf(fp, "S %s~\n", obj->short_descr);

	if (obj->description != obj->pIndexData->description)
		fprintf(fp, "D %s~\n", obj->description);

	if (obj->material != obj->pIndexData->material)
		fprintf(fp, "M %s~\n", obj->material);

	if (obj->extra_flags != obj->pIndexData->extra_flags)
		fprintf(fp, "E %ld\n", obj->extra_flags);

	if (obj->wear_flags != obj->pIndexData->wear_flags)
		fprintf(fp, "W %ld\n", obj->wear_flags);

	if (obj->item_type != obj->pIndexData->item_type)
		fprintf(fp, "T %d\n",  obj->item_type);

	if (obj->weight != obj->pIndexData->weight)
		fprintf(fp, "G %d\n",  obj->weight);

	if (obj->condition != obj->pIndexData->condition)
		fprintf(fp, "C %d\n",  obj->condition);

	if (obj->level != obj->pIndexData->level)
		fprintf(fp, "L %d\n",  obj->level);

	if (obj->value[0] != obj->pIndexData->value[0]
	    || obj->value[1] != obj->pIndexData->value[1]
	    || obj->value[2] != obj->pIndexData->value[2]
	    || obj->value[3] != obj->pIndexData->value[3]
	    || obj->value[4] != obj->pIndexData->value[4])
		fprintf(fp, "V %d %d %d %d %d\n",
		        obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4]);

	for (paf = obj->affected; paf; paf = paf->next) {
		if (paf->type < 0 || paf->type >= MAX_SKILL)
			continue;

		fprintf(fp, "A '%s' %3d %3d %3d %3d %3d %10d %d\n",
		        skill_table[paf->type].name,
		        paf->where,
		        paf->level,
		        paf->duration,
		        paf->modifier,
		        paf->location,
		        paf->bitvector,
		        paf->evolution ? paf->evolution : 1);
	}

	for (ed = obj->extra_descr; ed; ed = ed->next)
		fprintf(fp, "X %s~ %s~\n", ed->keyword, ed->description);

	fprintf(fp, "Z\n\n");

	/* recursively write it's contents */
	for (cobj = obj->contains; cobj; cobj = cobj->next_content)
		fwrite_objstate(cobj, fp);
}


/* loop through the object list, save all items that are lying on the ground
   to disk.  This is called occasionally from update_handler, and selectively
   from some parts of get_obj and drop_obj */
void save_items()
{
	FILE *fp;
	OBJ_DATA *obj;

	if ((fp = fopen(COPYOVER_ITEMS, "w")) == NULL) {
		bugf("Could not write to copyover file: %s", COPYOVER_ITEMS);
		return;
	}

	for (obj = object_list; obj != NULL; obj = obj->next)
		if (is_worth_saving(obj))
			fwrite_objstate(obj, fp);

	fprintf(fp, "END\n");
	fclose(fp);
}


OBJ_DATA *fload_objstate(FILE *fp)
{
	ROOM_INDEX_DATA *room;
	OBJ_DATA *obj, *cobj;
	bool extract = FALSE, done = FALSE;
	int rvnum, nests, i, tmp;

	if (feof(fp))
		return NULL;

	if (str_cmp(fread_word(fp), "OBJ"))
		return NULL;

	tmp = fread_number(fp);

	if (get_obj_index(tmp) == NULL) {
		obj = create_object(get_obj_index(GEN_OBJ_TREASURE), 0);

		if (obj)
			extract = TRUE;
		else {
			bug("Memory error creating TREASURE object.", 0);
			return NULL;
		}
	}
	else if ((obj = create_object(get_obj_index(tmp), 0)) == NULL) {
		/* make a temp object, we'll extract it later, so we can read the rest of the list */
		extract = TRUE;
		obj = create_object(get_obj_index(GEN_OBJ_TREASURE), 0);

		if (! obj) {
			bug("Error creating TREASURE object.", 0);
			return NULL;
		}
	}

	rvnum           = fread_number(fp);
	obj->enchanted  = fread_number(fp);
	obj->cost       = fread_number(fp);
	nests           = fread_number(fp);

	while (!done) { /* loop over all lines of obj desc */
		switch (fread_letter(fp)) {
		case 'A': {
				AFFECT_DATA *paf;
				int sn;
				paf = new_affect();
				sn = skill_lookup(fread_word(fp));

				if (sn < 0) {
					free_affect(paf);
					fread_to_eol(fp);
					continue;
				}
				else
					paf->type = sn;

				paf->where      = fread_number(fp);
				paf->level      = fread_number(fp);
				paf->duration   = fread_number(fp);
				paf->modifier   = fread_number(fp);
				paf->location   = fread_number(fp);
				paf->bitvector  = fread_number(fp);
				paf->evolution  = fread_number(fp);
				paf->next       = obj->affected;
				obj->affected   = paf;
				break;
			}

		case 'C':
			obj->condition = fread_number(fp);
			break;

		case 'D':
			free_string(obj->description);
			obj->description = str_dup(fread_string(fp));
			break;

		case 'E':
			obj->extra_flags = fread_number(fp);
			break;

		case 'G':
			obj->weight = fread_number(fp);
			break;

		case 'L':
			obj->level = fread_number(fp);
			break;

		case 'M':
			free_string(obj->material);
			obj->material = str_dup(fread_string(fp));
			break;

		case 'N':
			free_string(obj->name);
			obj->name = str_dup(fread_string(fp));
			break;

		case 'S':
			free_string(obj->short_descr);
			obj->short_descr = str_dup(fread_string(fp));
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
			obj->wear_flags = fread_number(fp);
			break;

		case 'X': {
				EXTRA_DESCR_DATA *ed;
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
	for (i = 0; i < nests; i++) {
		if ((cobj = fload_objstate(fp)) != NULL)
			obj_to_obj(cobj, obj);
	}

	/* if it wasn't found, extract it */
	if (extract)
		extract_obj(obj);
	else if (rvnum > 0) {
		if ((room = get_room_index(rvnum)) == NULL)
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


void load_items()
{
	FILE *fp;

	if ((fp = fopen(COPYOVER_ITEMS, "r")) == NULL) {
		bugf("Could not open copyover file: %s", COPYOVER_ITEMS);
		return;
	}

	for (; ;)
		if (!fload_objstate(fp))        /* load, check for end just in case */
			break;

	fclose(fp);
}



