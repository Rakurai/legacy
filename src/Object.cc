#include "Object.hh"

#include "affect/affect_list.hh"
#include "affect/Affect.hh"
#include "ExtraDescr.hh"
#include "macros.hh"
#include "random.hh"

Object::~Object() {
	// data wholly owned by this obj
	affect::clear_list(&this->affected);
	affect::clear_list(&this->gem_affected);

	ExtraDescr *ed, *ed_next;
	for (ed = this->extra_descr; ed != nullptr; ed = ed_next) {
		ed_next = ed->next;
		delete ed;
	}

	// deal with contents and gems at some point, for now extract_obj handles them
}

void unique_item(Object *item)
{
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
	    || item->item_type == ITEM_WEDDINGRING
	    || item->item_type == ITEM_TOKEN
	    || item->item_type == ITEM_JUKEBOX
	    || item->item_type == ITEM_WARP_CRYSTAL)
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
	    && item->wear_flags.to_ulong() <ITEM_WEAR_FINGER)
		return;

	/* random apply */
	if (chance(40)) { /* 40% */
		int loc = 0, mod = 0;

		switch (number_range(1, 14)) {
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
			loc = APPLY_SAVES;
			mod = number_range(-1 * ((item->level / 30) + 1), ((item->level / 30) + 1));
			break;
		}

		if (mod == 0) {
			if (chance(50))
				mod = 1;
			else
				mod = -1;
		}

		affect::Affect af;
		af.where      = TO_OBJECT;
		af.type       = affect::none;
		af.level      = item->level;
		af.duration   = -1;
		af.location   = loc;
		af.modifier   = mod;
		af.bitvector(0);
		af.evolution  = 1;
		affect::join_to_obj(item, &af);

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
					++item->value[1];
				else
					--item->value[1];
			}
			else { /* sides bonus */
				if (chance(50))
					++item->value[2];
				else
					--item->value[2];
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
					++item->value[1];
				else
					--item->value[1];
			}
			else if (chance(66)) { /* sides */
				if (chance(50))
					++item->value[2];
				else
					--item->value[2];
			}
			else { /* flags, 20% chance */
				switch (number_range(1, 8)) {
				case 1: item->value[4] ^= WEAPON_FLAMING;	break;
				case 2: item->value[4] ^= WEAPON_FROST;		break;
				case 3: item->value[4] ^= WEAPON_VAMPIRIC;	break;
				case 4: item->value[4] ^= WEAPON_SHARP;		break;
				case 5: item->value[4] ^= WEAPON_VORPAL;	break;
				case 6: item->value[4] ^= WEAPON_TWO_HANDS;	break;
				case 7: item->value[4] ^= WEAPON_SHOCKING;	break;
				case 8: item->value[4] ^= WEAPON_POISON;	break;
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
		if (item->level > LEVEL_IMMORTAL) { /* lower it by one, maybe 2 levels, but not below 92 */
			if (chance(75))
				item->level--;
			else
				item->level -= 2;

			if (item->level < LEVEL_IMMORTAL)
				item->level = LEVEL_IMMORTAL;

			added = TRUE;
		}
		else if (item->level < LEVEL_IMMORTAL && item->level > 0) {
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

		                if ((fp = fopen(UNIQUE_FILE, "w")) = nullptr)
		                {
		                        perror(UNIQUE_FILE);
		                        Logging::bug("Could not open unique.txt",0);
		                }
		                else
		                {
		                        Format::fprintf(fp, "%s\n", item->short_descr);
		                        fclose(fp);
		                }

		*/

		/* enable below 2 lines for testing */
		/*              Format::sprintf(buf,"%s, in room %d",item->short_descr, item->in_room == nullptr ? 0 : item->in_room->vnum);
		                Logging::bug(buf,0); */
	}
}
