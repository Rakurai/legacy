/*************************************************
*                                                *
*               The Age of Legacy                *
*                                                *
* Based originally on ROM 2.4, tested, enhanced, *
* and maintained by the Legacy Team.  If that    *
* doesn't mean you, and you're stealing our      *
* code, at least tell us and boost our egos ;)   *
*************************************************/

#include "merc.h"
#include "recycle.h"
#include "memory.h"
#include "db.h"

void unique_item(OBJ_DATA *item)
{
	AFFECT_DATA *paf, *af_new;
	bool added = FALSE;

	if (number_range(1, 50) != 1)
		return;

	/* types we won't even mess with, eliminate them now */
	if (item->item_type == ITEM_ANVIL
//	    || item->item_type == ITEM_COACH
	    || item->item_type == ITEM_FOOD
	    || item->item_type == ITEM_BOAT
	    || item->item_type == ITEM_CORPSE_NPC
	    || item->item_type == ITEM_CORPSE_PC
	    || item->item_type == ITEM_FOUNTAIN
	    || item->item_type == ITEM_MAP
	    || item->item_type == ITEM_PBTUBE
	    || item->item_type == ITEM_PBGUN
	    || item->item_type == ITEM_PORTAL
	    || item->item_type == ITEM_TRASH
	    || item->item_type == ITEM_KEY
	    || item->item_type == ITEM_ROOM_KEY
	    || item->item_type == ITEM_WEDDINGRING
	    || item->item_type == ITEM_TOKEN
	    || item->item_type == ITEM_JUKEBOX)
		return;

	/* don't care about most stuff unless it's at least takeable */
	if (item->item_type != ITEM_FURNITURE
	    && !CAN_WEAR(item, ITEM_TAKE))
		return;

	/* don't care about most stuff unless it's wearable */
	if (item->item_type != ITEM_FURNITURE
	    && item->item_type != ITEM_CONTAINER
	    && item->item_type != ITEM_MATERIAL
	    && item->item_type != ITEM_TREASURE
	    && item->item_type != ITEM_GEM
	    && item->item_type != ITEM_JEWELRY
	    && item->item_type != ITEM_SCROLL
	    && item->item_type != ITEM_POTION
	    && item->item_type != ITEM_PILL
	    && item->item_type != ITEM_MONEY
	    && item->wear_flags < ITEM_WEAR_FINGER)
		return;

	/* random apply */
	if (chance(40)) { /* 40% */
		int loc = 0, mod = 0;

		switch (number_range(1, 18)) {
		case 1: /* STR */
			loc = APPLY_STR;
			mod = number_range(-1 * ((item->level / 50) + 1), ((item->level / 50) + 1));
			break;

		case 2: /* DEX */
			loc = APPLY_DEX;
			mod = number_range(-1 * ((item->level / 50) + 1), ((item->level / 50) + 1));
			break;

		case 3: /* INT */
			loc = APPLY_INT;
			mod = number_range(-1 * ((item->level / 50) + 1), ((item->level / 50) + 1));
			break;

		case 4: /* WIS */
			loc = APPLY_WIS;
			mod = number_range(-1 * ((item->level / 50) + 1), ((item->level / 50) + 1));
			break;

		case 5: /* CON */
			loc = APPLY_CON;
			mod = number_range(-1 * ((item->level / 50) + 1), ((item->level / 50) + 1));
			break;

		case 6: /* CHR */
			loc = APPLY_CHR;
			mod = number_range(-1 * ((item->level / 50) + 1), ((item->level / 50) + 1));
			break;

		case 7: /* Age */
			loc = APPLY_AGE;
			mod = number_range(-1 * ((item->level / 20) + 1), ((item->level / 20) + 1));
			break;

		case 8: /* HP */
			loc = APPLY_HIT;
			mod = number_range(-1 * ((item->level / 2) + 1), ((item->level / 2) + 1));
			break;

		case 9: /* Mana */
			loc = APPLY_MANA;
			mod = number_range(-1 * ((item->level / 2) + 1), ((item->level / 2) + 1));
			break;

		case 10: /* Stamina */
			loc = APPLY_STAM;
			mod = number_range(-1 * ((item->level / 2) + 1), ((item->level / 2) + 1));
			break;

		case 11: /* AC */
			loc = APPLY_AC;
			mod = number_range(-1 * ((item->level / 30) + 1), ((item->level / 30) + 1));
			break;

		case 12: /* Hitroll */
			loc = APPLY_HITROLL;
			mod = number_range(-1 * ((item->level / 30) + 1), ((item->level / 30) + 1));
			break;

		case 13: /* Damroll */
			loc = APPLY_DAMROLL;
			mod = number_range(-1 * ((item->level / 30) + 1), ((item->level / 30) + 1));
			break;

		case 14: /* Saves vs paralysis */
			loc = APPLY_SAVING_PARA;
			mod = number_range(-1 * ((item->level / 30) + 1), ((item->level / 30) + 1));
			break;

		case 15: /* Saves vs rod */
			loc = APPLY_SAVING_ROD;
			mod = number_range(-1 * ((item->level / 30) + 1), ((item->level / 30) + 1));
			break;

		case 16: /* Saves vs petrification */
			loc = APPLY_SAVING_PETRI;
			mod = number_range(-1 * ((item->level / 30) + 1), ((item->level / 30) + 1));
			break;

		case 17: /* Saves vs breath */
			loc = APPLY_SAVING_BREATH;
			mod = number_range(-1 * ((item->level / 30) + 1), ((item->level / 30) + 1));
			break;

		case 18: /* Saves vs spell */
			loc = APPLY_SAVING_SPELL;
			mod = number_range(-1 * ((item->level / 30) + 1), ((item->level / 30) + 1));
			break;
		}

		if (mod == 0) {
			if (chance(50))
				mod = 1;
			else
				mod = -1;
		}

		item->enchanted = TRUE;

		/* move affects into new vectors, add to existing affect if it exists */
		for (paf = item->pIndexData->affected; paf != NULL; paf = paf->next) {
			af_new = new_affect();
			af_new->next = item->affected;
			item->affected = af_new;
			af_new->where           = paf->where;
			af_new->type            = UMAX(0, paf->type);
			af_new->level           = paf->level;
			af_new->duration        = paf->duration;
			af_new->location        = paf->location;
			af_new->modifier        = paf->modifier;
			af_new->bitvector       = paf->bitvector;
			af_new->evolution       = paf->evolution;
		}

		paf = new_affect();
		paf->where      = TO_OBJECT;
		paf->type       = 0;
		paf->level      = item->level;
		paf->duration   = -1;
		paf->location   = loc;
		paf->modifier   = mod;
		paf->bitvector  = 0;
		paf->evolution  = 1;
		paf->next       = item->affected;
		item->affected  = paf;
		added = TRUE;
	}
	/* value */
	else if (chance(50)) { /* 30% */
		int x;
		added = TRUE;

		switch (item->item_type) {
		/* don't mess with values on these types */
		case ITEM_LIGHT:
		case ITEM_CLOTHING:
		case ITEM_TREASURE:
		case ITEM_WARP_STONE:
		case ITEM_GEM:
		case ITEM_JEWELRY:
			added = FALSE;
			break;

		case ITEM_MONEY:
			if (chance(50)) /* silver value */
				item->value[0] = number_range(item->value[0] * 1 / 2, item->value[0] * 3 / 2);
			else /* gold value */
				item->value[1] = number_range(item->value[1] * 1 / 2, item->value[1] * 3 / 2);

			break;

		case ITEM_FURNITURE:
			if (chance(50)) /* hp regen */
				item->value[3] = number_range(item->value[3] * 3 / 4, item->value[3] * 5 / 4);
			else /* mana regen */
				item->value[4] = number_range(item->value[4] * 3 / 4, item->value[4] * 5 / 4);

			break;

		case ITEM_CONTAINER:
			if (chance(33)) /* max total weight */
				item->value[0] = number_range(item->value[0] * 3 / 4, item->value[0] * 5 / 4);
			else if (chance(50)) /* max single weight */
				item->value[3] = number_range(item->value[3] * 3 / 4, item->value[3] * 5 / 4);
			else /* weight multiplier */
				item->value[4] = number_range(item->value[4] * 3 / 4, item->value[4] * 5 / 4);

			break;

		case ITEM_DRINK_CON:
			item->value[3] = number_range(0, 1); /* random poisoning */
			break;

		case ITEM_MATERIAL:
			if (chance(33)) /* skill modifier */
				item->value[0] = number_range(item->value[0] * 3 / 4, item->value[0] * 5 / 4);
			else if (chance(50)) { /* dice bonus */
				if (chance(50))
					item->value[1]++;
				else
					item->value[1]--;
			}
			else { /* sides bonus */
				if (chance(50))
					item->value[2]++;
				else
					item->value[2]--;
			}

			break;

		case ITEM_SCROLL:
		case ITEM_POTION:
		case ITEM_PILL:         /* spell level */
			item->value[0] = number_range(item->value[0] * 5 / 6, item->value[0] * 7 / 6);
			break;

		case ITEM_WAND:
		case ITEM_STAFF:
			if (chance(33)) /* spell level */
				item->value[0] = number_range(item->value[0] * 9 / 10, item->value[0] * 11 / 10);
			else if (chance(50)) /* max charges */
				item->value[1] = number_range(item->value[1] * 3 / 4, item->value[1] * 3 / 4);
			else /* current charges */
				item->value[2] = number_range(item->value[2] * 3 / 4, item->value[2] * 3 / 4);

			if (item->value[2] > item->value[1])
				item->value[1] = item->value[2]; /* increase max charges to hold current */

			break;

		case ITEM_ARMOR:
			x = number_range(0, 3); /* AC values */
			item->value[x] = number_range(item->value[x] * 4 / 5, item->value[x] * 6 / 5);
			break;

		case ITEM_WEAPON:
			if (chance(40)) { /* dice */
				if (chance(50))
					item->value[1]++;
				else
					item->value[1]--;
			}
			else if (chance(66)) { /* sides */
				if (chance(50))
					item->value[2]++;
				else
					item->value[2]--;
			}
			else { /* flags, 20% chance */
				switch (number_range(1, 8)) {
				case 1: (IS_WEAPON_STAT(item, WEAPON_FLAMING) ?
					         REMOVE_BIT(item->value[4], WEAPON_FLAMING) :
					         SET_BIT(item->value[4], WEAPON_FLAMING));
					break;

				case 2: (IS_WEAPON_STAT(item, WEAPON_FROST) ?
					         REMOVE_BIT(item->value[4], WEAPON_FROST) :
					         SET_BIT(item->value[4], WEAPON_FROST));
					break;

				case 3: (IS_WEAPON_STAT(item, WEAPON_VAMPIRIC) ?
					         REMOVE_BIT(item->value[4], WEAPON_VAMPIRIC) :
					         SET_BIT(item->value[4], WEAPON_VAMPIRIC));
					break;

				case 4: (IS_WEAPON_STAT(item, WEAPON_SHARP) ?
					         REMOVE_BIT(item->value[4], WEAPON_SHARP) :
					         SET_BIT(item->value[4], WEAPON_SHARP));
					break;

				case 5: (IS_WEAPON_STAT(item, WEAPON_VORPAL) ?
					         REMOVE_BIT(item->value[4], WEAPON_VORPAL) :
					         SET_BIT(item->value[4], WEAPON_VORPAL));
					break;

				case 6: (IS_WEAPON_STAT(item, WEAPON_TWO_HANDS) ?
					         REMOVE_BIT(item->value[4], WEAPON_TWO_HANDS) :
					         SET_BIT(item->value[4], WEAPON_TWO_HANDS));
					break;

				case 7: (IS_WEAPON_STAT(item, WEAPON_SHOCKING) ?
					         REMOVE_BIT(item->value[4], WEAPON_SHOCKING) :
					         SET_BIT(item->value[4], WEAPON_SHOCKING));
					break;

				case 8: (IS_WEAPON_STAT(item, WEAPON_POISON) ?
					         REMOVE_BIT(item->value[4], WEAPON_POISON) :
					         SET_BIT(item->value[4], WEAPON_POISON));
					break;
				}
			}

			break;
		}
	}
	/* condition, weight, cost */
	else if (chance(66)) { /* 20% */
		switch (number_range(1, 3)) {
		case 1: /* cost */
			if (chance(50)) { /* increase the value */
				item->cost = number_range(item->cost, (item->cost * 2) + 1);
				added = TRUE;
			}
			else if (item->cost > 0) { /* lower the value */
				item->cost = number_range(0, item->cost);
				added = TRUE;
			}
			else
				added = FALSE;

			break;

		case 2: /* weight */
			if (chance(50)) { /* increase the weight */
				item->weight = number_range(item->weight, (item->weight * 2) + 1);
				item->weight = UMIN(item->weight, 30000); /* don't let it go over 30k */
				added = TRUE;
			}
			else if (item->weight > 0) { /* lower the weight */
				item->weight = number_range(0, item->weight);
				added = TRUE;
			}
			else
				added = FALSE;

			break;

		case 3: /* condition */
			if (chance(1)) /* make it indestructible */
				item->condition = -1;
			else
				item->condition = number_range(1, 100); /* random condition */

			added = TRUE;
			break;
		}
	}
	/* lower level item */
	else { /* 10% */
		/* not going to put in a higher level detriment, to simplify eq loading on mobs */
		if (item->level > 92) { /* lower it by one, maybe 2 levels, but not below 92 */
			if (chance(75))
				item->level--;
			else
				item->level -= 2;

			if (item->level < 92)
				item->level = 92;

			added = TRUE;
		}
		else if (item->level <= 91 && item->level > 0) {
			if (chance(50))
				item->level --;
			else if (chance(60))
				item->level -= 2;
			else
				item->level -= 3;

			if (item->level < 0)
				item->level = 0;

			added = TRUE;
		}
		else
			added = FALSE;
	}

	if (added) {
		/*              char buf[MAX_STRING_LENGTH]; */

		/*              FILE *fp;

		                for testing, write all modified items in a list to a file

		                if ((fp = fopen(UNIQUE_FILE, "w")) = NULL)
		                {
		                        perror(UNIQUE_FILE);
		                        bug("Could not open unique.txt",0);
		                }
		                else
		                {
		                        fprintf(fp, "%s\n", item->short_descr);
		                        fclose(fp);
		                }

		*/
		if (!item->enchanted) {
			item->enchanted = TRUE;

			for (paf = item->pIndexData->affected; paf != NULL; paf = paf->next) {
				af_new = new_affect();
				af_new->next = item->affected;
				item->affected = af_new;
				af_new->where           = paf->where;
				af_new->type            = UMAX(0, paf->type);
				af_new->level           = paf->level;
				af_new->duration        = paf->duration;
				af_new->location        = paf->location;
				af_new->modifier        = paf->modifier;
				af_new->bitvector       = paf->bitvector;
				af_new->evolution       = paf->evolution;
			}
		}

		/* enable below 2 lines for testing */
		/*              sprintf(buf,"%s, in room %d",item->short_descr, item->in_room == NULL ? 0 : item->in_room->vnum);
		                bug(buf,0); */
	}
}

/* pick a random room to reset into -- Montrey */
ROOM_INDEX_DATA *get_random_reset_room(AREA_DATA *area)
{
	ROOM_INDEX_DATA *room;
	int i, count, flags, pick = 0, pass = 1;

	while (pass <= 2) {
		for (i = area->min_vnum, count = 0; i <= area->max_vnum; i++) {
			if ((room = get_room_index(i)) == NULL)
				continue;

			flags = (room->original_flags) | (room->room_flags);

			if (IS_SET(flags, ROOM_NO_MOB
			           | ROOM_PRIVATE
			           | ROOM_SAFE
			           | ROOM_SOLITARY
			           | ROOM_PET_SHOP
			           | ROOM_IMP_ONLY
			           | ROOM_GODS_ONLY
			           | ROOM_LEADER_ONLY
			           | ROOM_TELEPORT
			           | ROOM_NORANDOMRESET))
				continue;

			count++;

			if (pass == 2 && count == pick)
				return room;
		}

		if (pass++ == 2 || count == 0)
			break;

		pick = number_range(0, count);
	}

	return NULL;
}

/*
 * Reset one area.
 */
void reset_area(AREA_DATA *pArea)
{
	RESET_DATA *pReset;
	CHAR_DATA *mob;
	bool last;
	int level;
	mob         = NULL;
	last        = TRUE;
	level       = 0;

	for (pReset = pArea->reset_first; pReset != NULL; pReset = pReset->next) {
		ROOM_INDEX_DATA *pRoomIndex;
		MOB_INDEX_DATA *pMobIndex;
		OBJ_INDEX_DATA *pObjIndex;
		OBJ_INDEX_DATA *pObjToIndex;
		EXIT_DATA *pexit;
		OBJ_DATA *obj;
		OBJ_DATA *obj_to;
		int count, limit;

		switch (pReset->command) {
		default:
			bug("Reset_area: bad command %c.", pReset->command);
			break;

		case 'M':
			if ((pMobIndex = get_mob_index(pReset->arg1)) == NULL) {
				bug("Reset_area: 'M': bad vnum %d.", pReset->arg1);
				continue;
			}

			if (pMobIndex->count >= pReset->arg2) {
				last = FALSE;
				break;
			}

			if (pReset->arg3 == 0) { /* random room */
				if (!chance(pReset->arg4))
					continue;

				if ((pRoomIndex = get_random_reset_room(pArea)) == NULL) {
					bug("Reset_area: 'R': no random room found.", 0);
					continue;
				}
			}
			else {
				if ((pRoomIndex = get_room_index(pReset->arg3)) == NULL) {
					bug("Reset_area: 'R': bad vnum %d.", pReset->arg3);
					continue;
				}

				for (mob = pRoomIndex->people, count = 0; mob != NULL; mob = mob->next_in_room)
					if (mob->pIndexData == pMobIndex) {
						count++;

						if (count >= pReset->arg4) {
							last = FALSE;
							break;
						}
					}

				if (count >= pReset->arg4)
					break;
			}

			mob = create_mobile(pMobIndex);

			/* Check for memory error. -- Outsider */
			if (! mob) {
				bug("Memory error creating mob in reset_area().", 0);
				break;
			}

			mob->reset = pReset;    /* keep track of what reset it -- Montrey */
			/* Check for pet shop. */
			{
				ROOM_INDEX_DATA *pRoomIndexPrev;
				pRoomIndexPrev = get_room_index(pRoomIndex->vnum - 1);

				if (pRoomIndexPrev != NULL
				    && IS_SET(pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
					SET_BIT(mob->act, ACT_PET);
			}
			/* set area */
			mob->zone = pRoomIndex->area;
			char_to_room(mob, pRoomIndex);
			level = URANGE(0, mob->level - 2, LEVEL_HERO - 1);
			last  = TRUE;
			break;

		case 'O':
			if (pArea->nplayer > 0 && pReset->arg1 != OBJ_VNUM_PIT) {
				last = FALSE;
				break;
			}

			if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL) {
				bug("Reset_area: 'O': bad vnum %d.", pReset->arg1);
				continue;
			}

			if ((pRoomIndex = get_room_index(pReset->arg3)) == NULL) {
				bug("Reset_area: 'R': bad vnum %d.", pReset->arg3);
				continue;
			}

			if (count_obj_list(pObjIndex, pRoomIndex->contents) > 0) {
				last = FALSE;
				break;
			}

			obj = create_object(pObjIndex, UMIN(number_fuzzy(level),
			                                    LEVEL_HERO - 1));

			if (! obj) {
				bug("Error creating object in area_reset.", 0);
				return;
			}

			obj->reset = pReset;    /* keep track of what reset it -- Montrey */
			obj_to_room(obj, pRoomIndex);

			if (pObjIndex->vnum == OBJ_VNUM_PIT)
				donation_pit = obj;
			else
				unique_item(obj);

			last = TRUE;
			break;

		case 'P':
			if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL) {
				bug("Reset_area: 'P': bad vnum %d.", pReset->arg1);
				continue;
			}

			if ((pObjToIndex = get_obj_index(pReset->arg3)) == NULL) {
				bug("Reset_area: 'P': bad vnum %d.", pReset->arg3);
				continue;
			}

			if (pReset->arg2 > 50) /* old format */
				limit = 6;
			else if (pReset->arg2 == -1) /* no limit */
				limit = 999;
			else
				limit = pReset->arg2;

			if (pArea->nplayer > 0
			    || (obj_to = get_obj_type(pObjToIndex)) == NULL
			    || (obj_to->in_room == NULL && !last)
			    || (pObjIndex->count >= limit && number_range(0, 4) != 0)
			    || (count = count_obj_list(pObjIndex, obj_to->contains))
			    > pReset->arg4) {
				last = FALSE;
				break;
			}

			while (count < pReset->arg4) {
				obj = create_object(pObjIndex, number_fuzzy(obj_to->level));

				if (! obj) {
					bug("Memory error creating object.", 0);
					return;
				}

				obj->reset = pReset;    /* keep track of what reset it -- Montrey */
				unique_item(obj);
				obj_to_obj(obj, obj_to);
				count++;

				if (pObjIndex->count >= limit)
					break;
			}

			/* fix object lock state! */
			obj_to->value[1] = obj_to->pIndexData->value[1];
			last = TRUE;
			break;

		case 'G':
		case 'E':
			if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL) {
				bug("Reset_area: 'E' or 'G': bad vnum %d.", pReset->arg1);
				continue;
			}

			if (!last)
				break;

			if (mob == NULL) {
				bug("Reset_area: 'E' or 'G': null mob for vnum %d.",
				    pReset->arg1);
				last = FALSE;
				break;
			}

			if (mob->pIndexData->pShop != NULL) {
				int olevel = 0;
				obj = create_object(pObjIndex, olevel);

				if (! obj) {
					bug("Error making object for mob inventory.", 0);
					return;
				}

				SET_BIT(obj->extra_flags, ITEM_INVENTORY);
			}
			else {
				if (pReset->arg2 == -1) /* no limit */
					limit = 999;
				else
					limit = pReset->arg2;

				if (pObjIndex->count < limit || number_range(0, 4) == 0) {
					obj = create_object(pObjIndex, UMIN(number_fuzzy(level),
					                                    LEVEL_HERO - 1));

					if (! obj) {
						bug("Memory error creating object to equip mob.", 0);
						return;
					}

					unique_item(obj);

					/* error message if it is too high */
					if (obj->level > mob->level + 3)
						fprintf(stderr,
						        "Err: obj %s (%d) -- %d, mob %s (%d) -- %d\n",
						        obj->short_descr, obj->pIndexData->vnum, obj->level,
						        mob->short_descr, mob->pIndexData->vnum, mob->level);
				}
				else
					break;
			}

			obj->reset = pReset;    /* keep track of what reset it -- Montrey */
			obj_to_char(obj, mob);

			if (pReset->command == 'E')
				equip_char(mob, obj, pReset->arg3);

			last = TRUE;
			break;

		case 'D':
			if ((pRoomIndex = get_room_index(pReset->arg1)) == NULL) {
				bug("Reset_area: 'D': bad vnum %d.", pReset->arg1);
				continue;
			}

			if ((pexit = pRoomIndex->exit[pReset->arg2]) == NULL)
				break;

			switch (pReset->arg3) {
			case 0:
				REMOVE_BIT(pexit->exit_info, EX_CLOSED);
				REMOVE_BIT(pexit->exit_info, EX_LOCKED);
				break;

			case 1:
				SET_BIT(pexit->exit_info, EX_CLOSED);
				REMOVE_BIT(pexit->exit_info, EX_LOCKED);
				break;

			case 2:
				SET_BIT(pexit->exit_info, EX_CLOSED);
				SET_BIT(pexit->exit_info, EX_LOCKED);
				break;
			}

			last = TRUE;
			break;

		case 'R':
			if ((pRoomIndex = get_room_index(pReset->arg1)) == NULL) {
				bug("Reset_area: 'R': bad vnum %d.", pReset->arg1);
				continue;
			}

			{
				int d0;
				int d1;

				for (d0 = 0; d0 < pReset->arg2 - 1; d0++) {
					d1                   = number_range(d0, pReset->arg2 - 1);
					pexit                = pRoomIndex->exit[d0];
					pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
					pRoomIndex->exit[d1] = pexit;
				}
			}

			break;
		}
	}

	return;
}

/*
 * Repopulate areas periodically.
 */
void area_update(void)
{
	AREA_DATA *pArea;
	char buf[MAX_STRING_LENGTH];

	for (pArea = area_first; pArea != NULL; pArea = pArea->next) {
		if (++pArea->age < 3)
			continue;

		/*
		 * Check age and reset.
		 * Note: Mud School resets every 3 minutes (not 15).
		 */
		if ((!pArea->empty && (pArea->nplayer == 0 || pArea->age >= 15))
		    ||    pArea->age >= 31) {
			ROOM_INDEX_DATA *pRoomIndex;
			reset_area(pArea);
			sprintf(buf, "%s has just been reset.", pArea->name);
			wiznet(buf, NULL, NULL, WIZ_RESETS, 0, 0);
			pArea->age = number_range(0, 3);
			pRoomIndex = get_room_index(ROOM_VNUM_SCHOOL);

			if (pRoomIndex != NULL && pArea == pRoomIndex->area)
				pArea->age = 15 - 2;
			else if (pArea->nplayer == 0)
				pArea->empty = TRUE;
		}
	}

	return;
}

/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile(MOB_INDEX_DATA *pMobIndex)
{
	CHAR_DATA *mob;
	int i, stambase;
	long wealth;
	AFFECT_DATA af;
	mobile_count++;

	if (pMobIndex == NULL) {
		bug("Create_mobile: NULL pMobIndex.", 0);
		/* Just return an error, don't exit game. -- Outsider
		exit( 1 );
		*/
		return NULL;
	}

	mob = new_char();

	/* Check for memory error. -- Outsider */
	if (!mob)
		return NULL;

	mob->pIndexData     = pMobIndex;
	mob->name           = pMobIndex->player_name;
	mob->id             = get_mob_id();
	mob->short_descr    = pMobIndex->short_descr;
	mob->long_descr     = pMobIndex->long_descr;
	mob->description    = pMobIndex->description;
	mob->spec_fun       = pMobIndex->spec_fun;
	mob->prompt         = NULL;
	mob->reset      = NULL;

	if (pMobIndex->wealth <= 0) {
		mob->silver = 0;
		mob->gold   = 0;
	}
	else {
		wealth = mob->pIndexData->wealth;
		wealth = number_range(wealth / 2, 3 * wealth / 2);
		mob->silver = wealth % 100;
		mob->gold   = wealth / 100;
	}

	/* read from prototype */
	mob->group              = pMobIndex->group;
	mob->act                = pMobIndex->act;
	mob->affected_by        = pMobIndex->affected_by;
	mob->comm               = COMM_NOCHANNELS;
	mob->alignment          = pMobIndex->alignment;
	mob->level              = pMobIndex->level;
	mob->hitroll            = pMobIndex->hitroll;
	mob->damroll            = pMobIndex->damage[DICE_BONUS];
	mob->max_hit            = dice(pMobIndex->hit[DICE_NUMBER],
	                               pMobIndex->hit[DICE_TYPE])
	                          + pMobIndex->hit[DICE_BONUS];
	mob->hit                = mob->max_hit;
	mob->max_mana           = dice(pMobIndex->mana[DICE_NUMBER],
	                               pMobIndex->mana[DICE_TYPE])
	                          + pMobIndex->mana[DICE_BONUS];
	mob->mana               = mob->max_mana;
	mob->damage[DICE_NUMBER] = pMobIndex->damage[DICE_NUMBER];
	mob->damage[DICE_TYPE]  = pMobIndex->damage[DICE_TYPE];
	mob->dam_type           = pMobIndex->dam_type;

	if (mob->dam_type == 0)
		switch (number_range(1, 3)) {
		case (1): mob->dam_type = 3;        break;  /* slash */

		case (2): mob->dam_type = 7;        break;  /* pound */

		case (3): mob->dam_type = 11;       break;  /* pierce */
		}

	for (i = 0; i < 4; i++)
		mob->armor_a[i]       = pMobIndex->ac[i];

	mob->off_flags          = pMobIndex->off_flags;
	mob->drain_flags        = pMobIndex->drain_flags;
	mob->imm_flags          = pMobIndex->imm_flags;
	mob->res_flags          = pMobIndex->res_flags;
	mob->vuln_flags         = pMobIndex->vuln_flags;
	mob->start_pos          = pMobIndex->start_pos;
	mob->default_pos        = pMobIndex->default_pos;
	mob->sex                = pMobIndex->sex;

	if (mob->sex == 3) /* random sex */
		mob->sex = number_range(1, 2);

	mob->race               = pMobIndex->race;
	mob->form               = pMobIndex->form;
	mob->parts              = pMobIndex->parts;
	mob->size               = pMobIndex->size;
	mob->material           = str_dup(pMobIndex->material);

	/* computed on the spot */

	for (i = 0; i < MAX_STATS; i ++)
		mob->perm_stat[i] = UMIN(25, number_fuzzy(8 + mob->level / 12));

	if (IS_SET(mob->act, ACT_WARRIOR)) {
		mob->perm_stat[STAT_STR] += 3;
		mob->perm_stat[STAT_INT] -= 2;
		mob->perm_stat[STAT_CON] += 2;
		mob->perm_stat[STAT_CHR] -= 1;
		mob->perm_stat[STAT_WIS] -= 2;
	}
	else if (IS_SET(mob->act, ACT_THIEF)) {
		mob->perm_stat[STAT_DEX] += 3;
		mob->perm_stat[STAT_WIS] -= 2;
		mob->perm_stat[STAT_CHR] += 2;
		mob->perm_stat[STAT_CON] -= 2;
	}
	else if (IS_SET(mob->act, ACT_CLERIC)) {
		mob->perm_stat[STAT_WIS] += 3;
		mob->perm_stat[STAT_INT] += 1;
		mob->perm_stat[STAT_DEX] -= 2;
		mob->perm_stat[STAT_CHR] += 1;
	}
	else if (IS_SET(mob->act, ACT_MAGE)) {
		mob->perm_stat[STAT_INT] += 3;
		mob->perm_stat[STAT_STR] -= 3;
		mob->perm_stat[STAT_DEX] += 1;
		mob->perm_stat[STAT_WIS] += 1;
		mob->perm_stat[STAT_CON] -= 2;
	}

	if (race_table[mob->race].pc_race == TRUE)
		for (i = 0; i < MAX_STATS; i++)
			mob->perm_stat[i] += (pc_race_table[mob->race].stats[i] - 13);

	/*Speed and size mods*/
	if (IS_SET(mob->off_flags, OFF_FAST))
		mob->perm_stat[STAT_DEX] += 2;

	mob->perm_stat[STAT_STR] += mob->size - SIZE_MEDIUM;
	mob->perm_stat[STAT_CON] += (mob->size - SIZE_MEDIUM) / 2;
	/* let's get some spell action */
	{
		struct maff {
			sh_int  sn;
			sh_int  loc;
			sh_int  mod;
			long    bit;
		};
		const struct maff maff_table[] = {
			{       gsn_blindness,          APPLY_HITROLL,  -4,             AFF_BLIND       },
			{       gsn_invis,              APPLY_NONE,     0,              AFF_INVISIBLE   },
			{       gsn_sanctuary,          APPLY_NONE,     0,              AFF_SANCTUARY   },
			{       gsn_faerie_fire,        APPLY_AC,       mob->level * 2,   AFF_FAERIE_FIRE },
			{       gsn_curse,              APPLY_HITROLL,  -mob->level / 8,  AFF_CURSE       },
			{       gsn_curse,              APPLY_SAVES,    mob->level / 8,   AFF_CURSE       },
			{       gsn_fear,               APPLY_HITROLL,  -mob->level / 10, AFF_FEAR        },
			{       gsn_fear,               APPLY_DAMROLL,  -mob->level / 14, AFF_FEAR        },
			{       gsn_fear,               APPLY_SAVES,    -mob->level / 16, AFF_FEAR        },
			{       gsn_poison,             APPLY_STR,      -2,             AFF_POISON      },
			{       gsn_protection_evil,    APPLY_SAVES,    -1,             AFF_PROTECT_EVIL},
			{       gsn_protection_good,    APPLY_SAVES,    -1,             AFF_PROTECT_GOOD},
			{       gsn_charm_person,       APPLY_NONE,     0,              AFF_CHARM       },
			{       gsn_fly,                APPLY_NONE,     0,              AFF_FLYING      },
			{       gsn_pass_door,          APPLY_NONE,     0,              AFF_PASS_DOOR   },
//	{    gsn_haste,              APPLY_DEX,      mob->level/25+2,AFF_HASTE       },
			{       gsn_haste,              APPLY_DEX,      0,              AFF_HASTE       },
			{       gsn_calm,               APPLY_HITROLL,  -2,             AFF_CALM        },
			{       gsn_calm,               APPLY_DAMROLL,  -2,             AFF_CALM        },
			{       gsn_plague,             APPLY_STR,      -mob->level / 20 - 1, AFF_PLAGUE     },
			{       gsn_steel_mist,         APPLY_AC,       -mob->level / 10, AFF_STEEL       },
			{       gsn_divine_regeneration, APPLY_NONE,     0,              AFF_DIVINEREGEN },
			{       gsn_berserk,            APPLY_HITROLL,  mob->level / 8,   AFF_BERSERK     },
			{       gsn_berserk,            APPLY_DAMROLL,  mob->level / 8,   AFF_BERSERK     },
			{       gsn_berserk,            APPLY_AC,       mob->level * 2,   AFF_BERSERK     },
			{       gsn_flameshield,        APPLY_AC,       -20,            AFF_FLAMESHIELD },
			{       gsn_regeneration,       APPLY_NONE,     0,              AFF_REGENERATION},
//	{    gsn_slow,               APPLY_DEX,      -mob->level/25-2,AFF_SLOW       },
			{       gsn_slow,               APPLY_DEX,      0,              AFF_SLOW        },
			{       gsn_talon,              APPLY_NONE,     0,              AFF_TALON       },
			{       -1,                     0,              0,              0               }
		};
		af.where = TO_AFFECTS;
		af.level = mob->level;
		af.duration = -1;
		af.evolution = 1;

		for (i = 0; maff_table[i].sn >= 0; i++)
			if (IS_AFFECTED(mob, maff_table[i].bit)) {
				af.type      = maff_table[i].sn;
				af.location  = maff_table[i].loc;
				af.modifier  = maff_table[i].mod;
				af.bitvector = maff_table[i].bit;
				affect_to_char(mob, &af);
			}
	}
	/* give em some stamina -- Montrey */
	mob->max_stam = 100;

	if (IS_SET(mob->act, ACT_MAGE))
		stambase = 3;
	else if (IS_SET(mob->act, ACT_CLERIC))
		stambase = 4;
	else if (IS_SET(mob->act, ACT_THIEF))
		stambase = 7;
	else if (IS_SET(mob->act, ACT_WARRIOR))
		stambase = 9;
	else
		stambase = 5;

	for (i = 0; i < mob->level; i++)
		mob->max_stam += number_fuzzy(stambase);

	mob->stam = mob->max_stam / 2;
	mob->position = mob->start_pos;
	/* link the mob to the world list */
	mob->next           = char_list;
	char_list           = mob;
	pMobIndex->count++;
	return mob;
}

/* duplicate a mobile exactly -- except inventory */
void clone_mobile(CHAR_DATA *parent, CHAR_DATA *clone)
{
	int i;
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	if (parent == NULL || clone == NULL || !IS_NPC(parent))
		return;

	/* start fixing values */
	clone->name         = str_dup(parent->name);
	clone->short_descr  = str_dup(parent->short_descr);
	clone->long_descr   = str_dup(parent->long_descr);
	clone->description  = str_dup(parent->description);
	clone->group        = parent->group;
	clone->sex          = parent->sex;
	clone->class        = parent->class;
	clone->race         = parent->race;
	clone->level        = parent->level;
	clone->timer        = parent->timer;
	clone->wait         = parent->wait;
	clone->hit          = parent->hit;
	clone->max_hit      = parent->max_hit;
	clone->mana         = parent->mana;
	clone->max_mana     = parent->max_mana;
	clone->stam         = parent->stam;
	clone->max_stam     = parent->max_stam;
	clone->gold         = /*parent->gold;*/ 0;
	clone->silver       = /*parent->silver;*/ 0;
	clone->exp          = parent->exp;
	clone->act          = parent->act;
	clone->comm         = parent->comm;
	clone->drain_flags  = parent->drain_flags;
	clone->imm_flags    = parent->imm_flags;
	clone->res_flags    = parent->res_flags;
	clone->vuln_flags   = parent->vuln_flags;
	clone->invis_level  = parent->invis_level;
	clone->affected_by  = parent->affected_by;
	clone->position     = parent->position;
	clone->practice     = parent->practice;
	clone->train        = parent->train;
	clone->saving_throw = parent->saving_throw;
	clone->alignment    = parent->alignment;
	clone->hitroll      = parent->hitroll;
	clone->damroll      = parent->damroll;
	clone->wimpy        = parent->wimpy;
	clone->form         = parent->form;
	clone->parts        = parent->parts;
	clone->size         = parent->size;
	clone->material     = str_dup(parent->material);
	clone->off_flags    = parent->off_flags;
	clone->dam_type     = parent->dam_type;
	clone->start_pos    = parent->start_pos;
	clone->default_pos  = parent->default_pos;
	clone->spec_fun     = parent->spec_fun;

	for (i = 0; i < 4; i++)
		clone->armor_a[i] = parent->armor_a[i];

	/* don't clone armor_m, it's magical eq and spell ac */

	for (i = 0; i < MAX_STATS; i++) {
		clone->perm_stat[i]     = parent->perm_stat[i];
		clone->mod_stat[i]      = parent->mod_stat[i];
	}

	for (i = 0; i < 3; i++)
		clone->damage[i]        = parent->damage[i];

	for (paf = clone->affected; paf != NULL; paf = paf_next) {
		paf_next = paf->next;
		affect_remove(clone, paf);
	}

	/* now add the affects */
	for (paf = parent->affected; paf != NULL; paf = paf->next)
		affect_to_char(clone, paf);
}

/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object(OBJ_INDEX_DATA *pObjIndex, int level)
{
	AFFECT_DATA *paf;
	OBJ_DATA *obj;
	int i;

	if (pObjIndex == NULL) {
		bug("Create_object: NULL pObjIndex.", 0);
		/* Let's not exit the game for this. Just report the error.
		   -- Outsider
		exit( 1 );
		*/
		return NULL;
	}

	obj = new_obj();

	/* Check for memory error. -- Outsider */
	if (! obj) {
		bug("create_object: unable to allocate memory", 0);
		return NULL;
	}

	obj->pIndexData     = pObjIndex;
	obj->in_room        = NULL;
	obj->reset      = NULL;
	obj->clean_timer = 0;
	obj->enchanted      = FALSE;
	obj->level          = pObjIndex->level;
	obj->wear_loc       = -1;
	obj->name           = pObjIndex->name;
	obj->short_descr    = pObjIndex->short_descr;
	obj->description    = pObjIndex->description;
	obj->material       = str_dup(pObjIndex->material);
	obj->condition      = pObjIndex->condition;
	obj->item_type      = pObjIndex->item_type;
	obj->extra_flags    = pObjIndex->extra_flags;
	obj->wear_flags     = pObjIndex->wear_flags;
	obj->value[0]       = pObjIndex->value[0];
	obj->value[1]       = pObjIndex->value[1];
	obj->value[2]       = pObjIndex->value[2];
	obj->value[3]       = pObjIndex->value[3];
	obj->value[4]       = pObjIndex->value[4];
	obj->num_settings	= pObjIndex->num_settings;
	obj->weight         = pObjIndex->weight;
	obj->cost           = pObjIndex->cost;

	/*
	 * Mess with object properties.
	 */
	switch (obj->item_type) {
	default:
		bug("Read_object: vnum %d bad type.", pObjIndex->vnum);
		break;

	case ITEM_LIGHT:
		if (obj->value[2] == 999)
			obj->value[2] = -1;

		break;

	case ITEM_JUKEBOX:
		for (i = 0; i < 5; i++)
			obj->value[i] = -1;

		break;

	case ITEM_SCROLL:
	case ITEM_WAND:
	case ITEM_STAFF:
	case ITEM_WEAPON:
	case ITEM_ARMOR:
	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_MONEY:
//	case ITEM_COACH:
	case ITEM_ANVIL:
	case ITEM_FURNITURE:
	case ITEM_TRASH:
	case ITEM_CONTAINER:
	case ITEM_DRINK_CON:
	case ITEM_KEY:
	case ITEM_FOOD:
	case ITEM_BOAT:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	case ITEM_FOUNTAIN:
	case ITEM_MAP:
	case ITEM_PBTUBE:
	case ITEM_PBGUN:
	case ITEM_MATERIAL:
	case ITEM_CLOTHING:
	case ITEM_PORTAL:
	case ITEM_TREASURE:
	case ITEM_WARP_STONE:
	case ITEM_ROOM_KEY:
	case ITEM_GEM:
	case ITEM_JEWELRY:
	case ITEM_WEDDINGRING:
	case ITEM_TOKEN:
		break;
	}

	for (paf = pObjIndex->affected; paf != NULL; paf = paf->next)
		if (paf->location == APPLY_SPELL_AFFECT)
			affect_to_obj(obj, paf);

	obj->next           = object_list;
	object_list         = obj;
	pObjIndex->count++;
	return obj;
}

/* duplicate an object exactly -- except contents */
void clone_object(OBJ_DATA *parent, OBJ_DATA *clone)
{
	int i;
	AFFECT_DATA *paf;
	EXTRA_DESCR_DATA *ed, *ed_new;

	if (parent == NULL || clone == NULL)
		return;

	/* start fixing the object */
	clone->name         = str_dup(parent->name);
	clone->short_descr  = str_dup(parent->short_descr);
	clone->description  = str_dup(parent->description);
	clone->item_type    = parent->item_type;
	clone->extra_flags  = parent->extra_flags;
	clone->wear_flags   = parent->wear_flags;
	clone->weight       = parent->weight;
	clone->cost         = parent->cost;
	clone->level        = parent->level;
	clone->condition    = parent->condition;
	clone->material     = str_dup(parent->material);
	clone->timer        = parent->timer;

	for (i = 0;  i < 5; i ++)
		clone->value[i] = parent->value[i];

	/* affects */
	clone->enchanted    = parent->enchanted;

	for (paf = parent->affected; paf != NULL; paf = paf->next)
		affect_to_obj(clone, paf);

	/* extended desc */
	for (ed = parent->extra_descr; ed != NULL; ed = ed->next) {
		ed_new                  = new_extra_descr();
		ed_new->keyword         = str_dup(ed->keyword);
		ed_new->description     = str_dup(ed->description);
		ed_new->next            = clone->extra_descr;
		clone->extra_descr      = ed_new;
	}
}

/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index(int vnum)
{
	MOB_INDEX_DATA *pMobIndex;

	for (pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH];
	     pMobIndex != NULL;
	     pMobIndex  = pMobIndex->next) {
		if (pMobIndex->vnum == vnum)
			return pMobIndex;
	}

	if (fBootDb) {
		bug("Get_mob_index: bad vnum %d.", vnum);
		/* Don't do this, we already return NULL on error. -- Outsider
		exit( 1 );
		*/
	}

	return NULL;
}

/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index(int vnum)
{
	OBJ_INDEX_DATA *pObjIndex;

	for (pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
	     pObjIndex != NULL;
	     pObjIndex  = pObjIndex->next) {
		if (pObjIndex->vnum == vnum)
			return pObjIndex;
	}

	if (fBootDb) {
		bug("Get_obj_index: bad vnum %d.", vnum);
		/* Don't exit, we already return NULL on error. -- Outsider
		exit( 1 );
		*/
	}

	return NULL;
}

/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index(int vnum)
{
	ROOM_INDEX_DATA *pRoomIndex;

	for (pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH];
	     pRoomIndex != NULL;
	     pRoomIndex  = pRoomIndex->next) {
		if (pRoomIndex->vnum == vnum)
			return pRoomIndex;
	}

	if (fBootDb) {
		bug("Get_room_index: bad vnum %d.", vnum);
		/* Don't exit here, we already return NULL on error. -- Outsider
		exit( 1 );
		*/
	}

	return NULL;
}

/* this command is here just to share some local variables, and to prevent crowding act_info.c */
/* new, improved AREAS command -- Elrac */
void do_areas(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	/* user parameters */
	bool showall = TRUE;
	bool sortv = FALSE;
	int level = 0;
	long vnum = 0;
	char keywords[MAX_INPUT_LENGTH] = "";
	bool star = FALSE;
	/* output data management */
	size_t ptrs_size;
	AREA_DATA **ptrs;
	int    count = 0;
	AREA_DATA *ap;
	char filename[9];    /* 12345678    + '\0' */
	char range[12];      /* {xnnn-nnn{x + '\0' */
	char buf[MAX_INPUT_LENGTH];
	BUFFER *dbuf = NULL;
	/* misc */
	char *p;
	int j, k;
	/* scan syntax: [lvl] {[keywd|'*'|'#'] ... } */
	argument = one_argument(argument, arg);

	if (arg[0]) {
		showall = FALSE;

		if (is_number(arg)) {
			level = atoi(arg);

			if (IS_IMMORTAL(ch) && level > 150) {
				vnum = level;
				level = 0;
			}
			else if (level < 0 || level > 100) {
				stc("Level must be between 1 and 100!\n", ch);
				return;
			}

			argument = one_argument(argument, arg);
		}

		while (arg[0]) {
			if (IS_IMMORTAL(ch) && !strcmp("*", arg))
				star = TRUE;
			else if (IS_IMMORTAL(ch) && !strcmp("#", arg))
				sortv = TRUE;
			else {
				if (keywords[0])
					strcat(keywords, " ");

				strcat(keywords, arg);
			}

			argument = one_argument(argument, arg);
		}
	}

	/* Allocate space for pointers to all areas. */
	ptrs_size = (top_area + 1) * sizeof(ap);
	ptrs = alloc_mem(ptrs_size);

	/* Gather pointers to all areas of interest */
	for (ap = area_first; ap; ap = ap->next) {

		if (!IS_IMMORTAL(ch) && ap->area_type == AREA_TYPE_XXX)
			continue;

		if (star) {
			if (ap->nplayer <= 0)
				continue;
		}

		if (vnum) {
			if (ap->min_vnum > vnum || vnum > ap->max_vnum)
				continue;
		}

		if (level)
			if (level < ap->low_range || level > ap->high_range)
				continue;

		if (keywords[0]) {
			if (!is_name(keywords, ap->keywords))
				continue;
		}

		if (level)
			if (ap->area_type == AREA_TYPE_CLAN ||
			    ap->area_type == AREA_TYPE_ARENA)
				continue;

		ptrs[count++] = ap;
	}

	/* sort pointers by alphabet */
	/* my favorite sort, exchange sort. Clumsy but simple. */
	for (j = 0; j < count - 1; j++)
		for (k = j + 1; k < count; k++)
			if ((!sortv && strcmp(ptrs[j]->title, ptrs[k]->title) > 0)
			    || (sortv && ptrs[j]->min_vnum > ptrs[k]->min_vnum)) {
				ap = ptrs[j];
				ptrs[j] = ptrs[k];
				ptrs[k] = ap;
			}

	/* Output area data to buffer */
	dbuf = new_buf();

	for (j = 0; j < count; j++) {
		ap = ptrs[j];

		switch (ap->area_type) {
		case AREA_TYPE_ALL:
			strcpy(range, "{W  ALL  {x");
			break;

		case AREA_TYPE_IMMS:
			strcpy(range, "{g  IMM  {x");
			break;

		case AREA_TYPE_CLAN:
			strcpy(range, "{g CLANS {x");
			break;

		case AREA_TYPE_HERO:
			strcpy(range, "{g  HRO  {x");
			break;

		case AREA_TYPE_ARENA:
			strcpy(range, "{g ARENA {x");
			break;

		case AREA_TYPE_XXX:
			strcpy(range, "{g  XXX  {x");
			break;

		default:
			sprintf(range, "{x%3d-%3d{x", ap->low_range, ap->high_range);

			if (ap->high_range <= 15)
				range[1] = 'C';
			else if (ap->high_range <= 50)
				range[1] = 'G';
			else if (ap->low_range >= 80)
				range[1] = 'M';
			else if (ap->low_range >= 50)
				range[1] = 'R';
			else if (ap->low_range >= 30)
				range[1] = 'Y';
			else
				range[1] = 'W';

			break;
		} /* end switch */

		buf[0] = '\0';

		if (IS_IMMORTAL(ch)) {
			strcpy(arg, ap->file_name);
			p = strchr(arg, '.');

			if (p)
				*p = '\0';

			arg[sizeof(filename)] = '\0';
			strcpy(filename, arg);

			if (ap->nplayer == 0) {
				sprintf(buf, "%-8.8s [%5d-%5d]%3s ",
				        filename, ap->min_vnum, ap->max_vnum, " ");
			}
			else {
				sprintf(buf, "%-8.8s [%5d-%5d]{Y%3d{x ",
				        filename, ap->min_vnum, ap->max_vnum, ap->nplayer);
			}
		}

		sprintf(buf + strlen(buf),
		        "<%s> %-s{a{x%*s%-s{a{x\n", range,
		        ap->title, 25 - color_strlen(ap->title), " ",
		        ap->author);
		add_buf(dbuf, buf);
	}

	if (showall) {
		sprintf(buf,
		        "%d areas listed. Type {Rhelp areas{x to see selection options.\n",
		        count);
		add_buf(dbuf, buf);
	}
	else if (count <= 0) {
		sprintf(buf, "No areas found matching your search criteria.\n");
		add_buf(dbuf, buf);
	}

	page_to_char(buf_string(dbuf), ch);
	/* clean up */
	free_mem(ptrs, ptrs_size);
	free_buf(dbuf);
} /* end do_areas() */
