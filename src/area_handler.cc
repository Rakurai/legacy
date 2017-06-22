/*************************************************
*                                                *
*               The Age of Legacy                *
*                                                *
* Based originally on ROM 2.4, tested, enhanced, *
* and maintained by the Legacy Team.  If that    *
* doesn't mean you, and you're stealing our      *
* code, at least tell us and boost our egos ;)   *
*************************************************/

#include "argument.hh"
#include "Affect.hh"
#include "Area.hh"
#include "Character.hh"
#include "db.hh"
#include "declare.hh"
#include "ExtraDescr.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Game.hh"
#include "Logging.hh"
#include "macros.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"
#include "ObjectValue.hh"
#include "Player.hh"
#include "random.hh"
#include "recycle.hh"
#include "RoomPrototype.hh"
#include "String.hh"


/*
 * Create an instance of a mobile.
 */
Character *create_mobile(MobilePrototype *pMobIndex)
{
	Character *mob;
	int i, stambase;
	long wealth;
	mobile_count++;

	if (pMobIndex == nullptr) {
		Logging::bug("Create_mobile: nullptr pMobIndex.", 0);
		/* Just return an error, don't exit game. -- Outsider
		exit( 1 );
		*/
		return nullptr;
	}

	mob = new Character();

	/* Check for memory error. -- Outsider */
	if (!mob)
		return nullptr;

	mob->pIndexData     = pMobIndex;
	mob->name           = pMobIndex->player_name;
	mob->id             = get_mob_id();
	mob->short_descr    = pMobIndex->short_descr;
	mob->long_descr     = pMobIndex->long_descr;
	mob->description    = pMobIndex->description;
	mob->spec_fun       = pMobIndex->spec_fun;
	mob->reset      = nullptr;

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
	mob->group_flags              = pMobIndex->group_flags;
	mob->act_flags                = pMobIndex->act_flags;
	mob->comm_flags               = COMM_NOCHANNELS;
	mob->alignment          = pMobIndex->alignment;
	mob->level              = pMobIndex->level;
	ATTR_BASE(mob, APPLY_HITROLL) = pMobIndex->hitroll;
	ATTR_BASE(mob, APPLY_DAMROLL) = pMobIndex->damage[DICE_BONUS];
	ATTR_BASE(mob, APPLY_HIT)     = dice(pMobIndex->hit[DICE_NUMBER],
	                               pMobIndex->hit[DICE_TYPE])
	                          + pMobIndex->hit[DICE_BONUS];
	ATTR_BASE(mob, APPLY_MANA)    = dice(pMobIndex->mana[DICE_NUMBER],
	                               pMobIndex->mana[DICE_TYPE])
	                          + pMobIndex->mana[DICE_BONUS];
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
		mob->armor_base[i]       = pMobIndex->ac[i];

	mob->off_flags          = pMobIndex->off_flags;
	mob->start_pos          = pMobIndex->start_pos;
	mob->default_pos        = pMobIndex->default_pos;

	if (pMobIndex->sex == 3)
		ATTR_BASE(mob, APPLY_SEX) = number_range(1, 2);
	else
		ATTR_BASE(mob, APPLY_SEX) = pMobIndex->sex;

	mob->race               = pMobIndex->race;
	mob->form_flags               = pMobIndex->form_flags;
	mob->parts_flags              = pMobIndex->parts_flags;
	mob->size               = pMobIndex->size;
	mob->material           = pMobIndex->material;

	/* computed on the spot */
	for (int stat = 0; stat < MAX_STATS; stat++)
		ATTR_BASE(mob, stat_to_attr(stat)) = UMIN(25, number_fuzzy(8 + mob->level / 12));

	if (mob->act_flags.has(ACT_WARRIOR)) {
		ATTR_BASE(mob, APPLY_STR) += 3;
		ATTR_BASE(mob, APPLY_INT) -= 2;
		ATTR_BASE(mob, APPLY_CON) += 2;
		ATTR_BASE(mob, APPLY_CHR) -= 1;
		ATTR_BASE(mob, APPLY_WIS) -= 2;
	}
	else if (mob->act_flags.has(ACT_THIEF)) {
		ATTR_BASE(mob, APPLY_DEX) += 3;
		ATTR_BASE(mob, APPLY_WIS) -= 2;
		ATTR_BASE(mob, APPLY_CHR) += 2;
		ATTR_BASE(mob, APPLY_CON) -= 2;
	}
	else if (mob->act_flags.has(ACT_CLERIC)) {
		ATTR_BASE(mob, APPLY_WIS) += 3;
		ATTR_BASE(mob, APPLY_INT) += 1;
		ATTR_BASE(mob, APPLY_DEX) -= 2;
		ATTR_BASE(mob, APPLY_CHR) += 1;
	}
	else if (mob->act_flags.has(ACT_MAGE)) {
		ATTR_BASE(mob, APPLY_INT) += 3;
		ATTR_BASE(mob, APPLY_STR) -= 3;
		ATTR_BASE(mob, APPLY_DEX) += 1;
		ATTR_BASE(mob, APPLY_WIS) += 1;
		ATTR_BASE(mob, APPLY_CON) -= 2;
	}

	if (race_table[mob->race].pc_race == TRUE)
		for (int stat = 0; stat < MAX_STATS; stat++)
			ATTR_BASE(mob, stat_to_attr(stat)) += (pc_race_table[mob->race].stats[stat] - 13);

	/*Speed and size mods*/
	if (mob->off_flags.has(OFF_FAST))
		ATTR_BASE(mob, APPLY_DEX) += 2;

	ATTR_BASE(mob, APPLY_STR) += mob->size - SIZE_MEDIUM;
	ATTR_BASE(mob, APPLY_CON) += (mob->size - SIZE_MEDIUM) / 2;

	affect_add_racial_to_char(mob);

	affect_copy_flags_to_char(mob, 'A', pMobIndex->affect_flags, FALSE); // dispellable
	affect_copy_flags_to_char(mob, 'I', pMobIndex->imm_flags, TRUE);
	affect_copy_flags_to_char(mob, 'R', pMobIndex->res_flags, TRUE);
	affect_copy_flags_to_char(mob, 'V', pMobIndex->vuln_flags, TRUE);

	/* give em some stamina -- Montrey */
	ATTR_BASE(mob, APPLY_STAM) = 100;

	if (mob->act_flags.has(ACT_MAGE))
		stambase = 3;
	else if (mob->act_flags.has(ACT_CLERIC))
		stambase = 4;
	else if (mob->act_flags.has(ACT_THIEF))
		stambase = 7;
	else if (mob->act_flags.has(ACT_WARRIOR))
		stambase = 9;
	else
		stambase = 5;

	for (i = 0; i < mob->level; i++)
		ATTR_BASE(mob, APPLY_STAM) += number_fuzzy(stambase);

	mob->hit = GET_MAX_HIT(mob);
	mob->mana = GET_MAX_MANA(mob);
	mob->stam = GET_MAX_STAM(mob);

	mob->position = mob->start_pos;
	/* link the mob to the world list */
	mob->next           = char_list;
	char_list           = mob;
	pMobIndex->count++;
	return mob;
}

/* duplicate a mobile exactly -- except inventory */
void clone_mobile(Character *parent, Character *clone)
{
	if (parent == nullptr || clone == nullptr || !IS_NPC(parent))
		return;

	/* start fixing values */
	clone->name         = parent->name;
	clone->short_descr  = parent->short_descr;
	clone->long_descr   = parent->long_descr;
	clone->description  = parent->description;
	clone->material     = parent->material;
	clone->prefix     = parent->prefix;
	clone->prompt     = parent->prompt;

	for (int i = 1; i < MAX_ATTR; i++)
		ATTR_BASE(clone, i) = ATTR_BASE(parent, i);

	clone->group_flags  = parent->group_flags;
//	clone->sex          = parent->sex;
	clone->cls        = parent->cls;
	clone->race         = parent->race;
	clone->level        = parent->level;
	clone->timer        = parent->timer;
	clone->wait         = parent->wait;
	clone->hit          = parent->hit;
//	clone->max_hit      = parent->max_hit;
	clone->mana         = parent->mana;
//	clone->max_mana     = parent->max_mana;
	clone->stam         = parent->stam;
//	clone->max_stam     = parent->max_stam;
	clone->gold         = /*parent->gold;*/ 0;
	clone->silver       = /*parent->silver;*/ 0;
	clone->exp          = parent->exp;
	clone->act_flags          = parent->act_flags;
	clone->comm_flags         = parent->comm_flags;
	clone->invis_level  = parent->invis_level;
	clone->position     = parent->position;
	clone->practice     = parent->practice;
	clone->train        = parent->train;
//	clone->saving_throw = parent->saving_throw;
	clone->alignment    = parent->alignment;
//	clone->hitroll      = parent->hitroll;
//	clone->damroll      = parent->damroll;
	clone->wimpy        = parent->wimpy;
	clone->form_flags         = parent->form_flags;
	clone->parts_flags        = parent->parts_flags;
	clone->size         = parent->size;
	clone->off_flags    = parent->off_flags;
	clone->dam_type     = parent->dam_type;
	clone->start_pos    = parent->start_pos;
	clone->default_pos  = parent->default_pos;
	clone->spec_fun     = parent->spec_fun;

	for (int i = 0; i < 4; i++)
		clone->armor_base[i] = parent->armor_base[i];

	/* don't clone armor_mod, it's magical eq and spell ac */

//	for (int i = 0; i < MAX_STATS; i++) {
//		clone->base_stat[i]     = parent->base_stat[i];
//	}

	for (int i = 0; i < 3; i++)
		clone->damage[i]        = parent->damage[i];

	affect_remove_all_from_char(clone, TRUE);
	affect_remove_all_from_char(clone, FALSE);

	for (const Affect *paf = affect_list_char(parent); paf != nullptr; paf = paf->next)
		affect_copy_to_char(clone, paf);
}

/*
 * Create an instance of an object.
 */
Object *create_object(ObjectPrototype *pObjIndex, int level)
{
	Object *obj;
	int i;

	if (pObjIndex == nullptr) {
		Logging::bug("Create_object: nullptr pObjIndex.", 0);
		/* Let's not exit the game for this. Just report the error.
		   -- Outsider
		exit( 1 );
		*/
		return nullptr;
	}

	obj = new_obj();

	/* Check for memory error. -- Outsider */
	if (! obj) {
		Logging::bug("create_object: unable to allocate memory", 0);
		return nullptr;
	}

	obj->pIndexData     = pObjIndex;
	obj->level          = pObjIndex->level;
	obj->wear_loc       = -1;
	obj->name           = pObjIndex->name;
	obj->short_descr    = pObjIndex->short_descr;
	obj->description    = pObjIndex->description;
	obj->material       = pObjIndex->material;
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
		Logging::bug("Read_object: vnum %d bad type.", pObjIndex->vnum);
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

	for (const Affect *paf = pObjIndex->affected; paf != nullptr; paf = paf->next)
		affect_copy_to_obj(obj, paf);

	obj->next           = object_list;
	object_list         = obj;
	pObjIndex->count++;
	return obj;
}

/* duplicate an object exactly -- except contents */
void clone_object(Object *parent, Object *clone)
{
	int i;
	ExtraDescr *ed, *ed_new;

	if (parent == nullptr || clone == nullptr)
		return;

	/* start fixing the object */
	clone->name         = parent->name;
	clone->short_descr  = parent->short_descr;
	clone->description  = parent->description;
	clone->item_type    = parent->item_type;
	clone->extra_flags  = parent->extra_flags;
	clone->wear_flags   = parent->wear_flags;
	clone->weight       = parent->weight;
	clone->cost         = parent->cost;
	clone->level        = parent->level;
	clone->condition    = parent->condition;
	clone->material     = parent->material;
	clone->timer        = parent->timer;

	for (i = 0;  i < 5; i ++)
		clone->value[i] = parent->value[i];

	/* affects */
	affect_remove_all_from_obj(clone, TRUE);

	for (const Affect *paf = affect_list_obj(parent); paf; paf = paf->next)
		affect_copy_to_obj(clone, paf);

	/* extended desc */
	for (ed = parent->extra_descr; ed != nullptr; ed = ed->next) {
		ed_new                  = new ExtraDescr(*ed);
		ed_new->next            = clone->extra_descr;
		clone->extra_descr      = ed_new;
	}
}

/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MobilePrototype *get_mob_index(int vnum)
{
	MobilePrototype *pMobIndex;

	for (pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH];
	     pMobIndex != nullptr;
	     pMobIndex  = pMobIndex->next) {
		if (pMobIndex->vnum == vnum)
			return pMobIndex;
	}

	if (fBootDb) {
		Logging::bug("Get_mob_index: bad vnum %d.", vnum);
		/* Don't do this, we already return nullptr on error. -- Outsider
		exit( 1 );
		*/
	}

	return nullptr;
}

/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
ObjectPrototype *get_obj_index(int vnum)
{
	ObjectPrototype *pObjIndex;

	for (pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
	     pObjIndex != nullptr;
	     pObjIndex  = pObjIndex->next) {
		if (pObjIndex->vnum == vnum)
			return pObjIndex;
	}

	if (fBootDb) {
		Logging::bug("Get_obj_index: bad vnum %d.", vnum);
		/* Don't exit, we already return nullptr on error. -- Outsider
		exit( 1 );
		*/
	}

	return nullptr;
}

/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
RoomPrototype *get_room_index(int vnum)
{
	RoomPrototype *pRoomIndex;

	for (pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH];
	     pRoomIndex != nullptr;
	     pRoomIndex  = pRoomIndex->next) {
		if (pRoomIndex->vnum == vnum)
			return pRoomIndex;
	}

	if (fBootDb) {
		Logging::bug("Get_room_index: bad vnum %d.", vnum);
		/* Don't exit here, we already return nullptr on error. -- Outsider
		exit( 1 );
		*/
	}

	return nullptr;
}

/* this command is here just to share some local variables, and to prevent crowding act_info.c */
/* new, improved AREAS command -- Elrac */
void do_areas(Character *ch, String argument)
{
	/* user parameters */
	bool showall = TRUE;
	bool sortv = FALSE;
	int level = 0;
	long vnum = 0;
	String keywords;
	bool star = FALSE;
	/* output data management */
	size_t ptrs_size;
	int    count = 0;
	Area *ap;
	char filename[9];    /* 12345678    + '\0' */
	char range[12];      /* {xnnn-nnn{x + '\0' */
	char buf[MAX_INPUT_LENGTH];
	String dbuf;
	/* misc */
	char *p;
	int j, k;
	/* scan syntax: [lvl] {[keywd|'*'|'#'] ... } */
	String arg;
	argument = one_argument(argument, arg);

	if (arg[0]) {
		showall = FALSE;

		if (arg.is_number()) {
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
					keywords += " ";

				keywords += arg;
			}

			argument = one_argument(argument, arg);
		}
	}

	/* Allocate space for pointers to all areas. */
	ptrs_size = (top_area + 1) * sizeof(ap);
	Area *ptrs[ptrs_size];

	/* Gather pointers to all areas of interest */
	for (Area *ap: Game::world().areas) {
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
			if (!ap->keywords.has_words(keywords))
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
			Format::sprintf(range, "{x%3d-%3d{x", ap->low_range, ap->high_range);

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
			char buf2[MAX_STRING_LENGTH];
			strcpy(buf2, ap->file_name);

			p = strchr(buf2, '.');

			if (p)
				*p = '\0';

			buf2[sizeof(filename)] = '\0';
			strcpy(filename, buf2);

			if (ap->nplayer == 0) {
				Format::sprintf(buf, "%-8.8s [%5d-%5d]%3s ",
				        filename, ap->min_vnum, ap->max_vnum, " ");
			}
			else {
				Format::sprintf(buf, "%-8.8s [%5d-%5d]{Y%3d{x ",
				        filename, ap->min_vnum, ap->max_vnum, ap->nplayer);
			}
		}

		Format::sprintf(buf + strlen(buf),
		        "<%s> %-s{a{x%*s%-s{a{x\n", range,
		        ap->title, 25 - ap->title.uncolor().size(), " ",
		        ap->author);
		dbuf += buf;
	}

	if (showall) {
		Format::sprintf(buf,
		        "%d areas listed. Type {Rhelp areas{x to see selection options.\n",
		        count);
		dbuf += buf;
	}
	else if (count <= 0) {
		Format::sprintf(buf, "No areas found matching your search criteria.\n");
		dbuf += buf;
	}

	page_to_char(dbuf, ch);
} /* end do_areas() */
