/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

#include "merc.h"
#include "affect.h"

void acid_effect(void *vo, int level, int dam, int target, int evolution)
{
	if (target == TARGET_ROOM) { /* nail objects on the floor */
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		for (obj = room->contents; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			acid_effect(obj, level, dam, TARGET_OBJ, evolution);
		}

		return;
	}

	if (target == TARGET_CHAR) { /* do the effect on a victim */
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		/* let's toast some gear */
		if (!IS_IMMORTAL(victim)) {
			for (obj = victim->carrying; obj != NULL; obj = obj_next) {
				obj_next = obj->next_content;
				acid_effect(obj, level, dam, TARGET_OBJ, evolution);
			}
		}

		return;
	}

	if (target == TARGET_OBJ) { /* toast an object */
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		int chance;
		char *msg;
		CHAR_DATA *owner;

		/* sheen protects absolutely */
		if ((owner = obj->carried_by) != NULL
		    && affect_find_in_char(owner, gsn_sheen))
			return;

		if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)
		    ||  IS_OBJ_STAT(obj, ITEM_NOPURGE)
		    ||  number_range(0, 4) == 0)
			return;

		chance = level / 4 + dam / 10;

		if (chance > 25)
			chance = (chance - 25) / 2 + 25;

		if (chance > 50)
			chance = (chance - 50) / 2 + 50;

		if (IS_OBJ_STAT(obj, ITEM_BLESS))
			chance -= 5;

		chance -= obj->level * 2;

		switch (obj->item_type) {
		default:
			return;

		case ITEM_CONTAINER:
		case ITEM_CORPSE_PC:
		case ITEM_CORPSE_NPC:
			msg = "$p fumes and dissolves.";
			break;

		case ITEM_ARMOR:
			msg = "$p is pitted and etched.";
			break;

		case ITEM_CLOTHING:
			msg = "$p is corroded into scrap.";
			break;

		case ITEM_STAFF:
		case ITEM_WAND:
			chance -= 10;
			msg = "$p corrodes and breaks.";
			break;

		case ITEM_SCROLL:
			chance += 10;
			msg = "$p is burned into waste.";
			break;
		}

		chance = URANGE(5, chance, 95);

		if (number_percent() > chance)
			return;

		if (obj->item_type != ITEM_ARMOR) {
			if (obj->carried_by != NULL)
				act(msg, obj->carried_by, obj, NULL, TO_ALL);
			else if (obj->in_room != NULL && obj->in_room->people != NULL)
				act(msg, obj->in_room->people, obj, NULL, TO_ALL);
		}
		else {  /* etch it */
			obj->condition -= number_range(5, 10);

			if (obj->condition > 0) {
				int place = number_range(0, 3);
				int amount = UMIN(obj->value[place], number_range(1, 4));

				if (amount <= 0)
					return;

				if (obj->carried_by != NULL)
					act("$p is pitted and etched.", obj->carried_by, obj, NULL, TO_ALL);
				else if (obj->in_room != NULL && obj->in_room->people != NULL)
					act("$p is pitted and etched.", obj->in_room->people, obj, NULL, TO_ALL);

				obj->value[place] -= amount;

				if (obj->carried_by != NULL && obj->wear_loc != WEAR_NONE)
					obj->carried_by->armor_a[place] += amount;

				return;
			}

			if (obj->carried_by)
				act("$p is dissolved by the acid!", obj->carried_by, obj, NULL, TO_ALL);
			else if (obj->in_room != NULL && obj->in_room->people != NULL)
				act("$p is dissolved by the acid!", obj->in_room->people, obj, NULL, TO_ALL);
		}

		if (obj->contains) { /* dump contents */
			OBJ_DATA *t_obj, *n_obj;

			if (obj->carried_by) {
				if (!char_in_darena_room(obj->carried_by))
					act("$p's contents scatter on the ground.", obj->carried_by, obj, NULL, TO_ALL);
			}
			else if (obj->in_room != NULL && obj->in_room->people != NULL)
				act("$p's contents scatter on the ground.", obj->in_room->people, obj, NULL, TO_ALL);

			for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj) {
				n_obj = t_obj->next_content;
				obj_from_obj(t_obj);

				if (obj->carried_by) {
					if (obj->carried_by->in_room == NULL)
						extract_obj(t_obj);
					else if (char_in_darena_room(obj->carried_by))
						obj_to_char(t_obj, obj->carried_by);
					else
						obj_to_room(t_obj, obj->carried_by->in_room);
				}
				else if (obj->in_room != NULL)
					obj_to_room(t_obj, obj->in_room);
				else {
					extract_obj(t_obj);
					continue;
				}

				acid_effect(t_obj, level / 2, dam / 2, TARGET_OBJ, evolution);
			}
		}

		extract_obj(obj);
	}
}

void cold_effect(void *vo, int level, int dam, int target, int evolution)
{
	if (target == TARGET_ROOM) { /* nail objects on the floor */
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		for (obj = room->contents; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			cold_effect(obj, level, dam, TARGET_OBJ, evolution);
		}

		return;
	}

	if (target == TARGET_CHAR) { /* whack a character */
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		/* chill touch effect */
		if (!saves_spell(level / 4 + dam / 20, victim, DAM_COLD)) {
			act("$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM);
			act("A chill sinks deep into your bones.", victim, NULL, NULL, TO_CHAR);

			affect_add_sn_to_char(victim,
				gsn_chill_touch,
				level,
				6,
				URANGE(1, evolution - 1, 4),
				FALSE
			);
		}

		/* hunger! (warmth sucked out) */
		if (!IS_NPC(victim))
			gain_condition(victim, COND_HUNGER, dam / 20);

		/* let's toast some gear */
		if (!IS_IMMORTAL(victim)) {
			for (obj = victim->carrying; obj != NULL; obj = obj_next) {
				obj_next = obj->next_content;
				cold_effect(obj, level, dam, TARGET_OBJ, evolution);
			}
		}

		return;
	}

	if (target == TARGET_OBJ) { /* toast an object */
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		int chance;
		char *msg;
		CHAR_DATA *owner;

		/* sheen protects absolutely */
		if ((owner = obj->carried_by) != NULL
		    && affect_find_in_char(owner, gsn_sheen))
			return;

		if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)
		    ||  IS_OBJ_STAT(obj, ITEM_NOPURGE)
		    ||  number_range(0, 4) == 0)
			return;

		chance = level / 4 + dam / 10;

		if (chance > 25)
			chance = (chance - 25) / 2 + 25;

		if (chance > 50)
			chance = (chance - 50) / 2 + 50;

		if (IS_OBJ_STAT(obj, ITEM_BLESS))
			chance -= 5;

		chance -= obj->level * 2;

		switch (obj->item_type) {
		default:
			return;

		case ITEM_POTION:
			msg = "$p freezes and EXPLODES!";
			chance += 25;
			break;

		case ITEM_DRINK_CON:
			msg = "$p freezes and EXPLODES!";
			chance += 5;
			break;
		}

		chance = URANGE(5, chance, 95);

		if (number_percent() > chance)
			return;

		if (obj->carried_by)
			act(msg, obj->carried_by, obj, NULL, TO_ALL);
		else if (obj->in_room != NULL && obj->in_room->people != NULL)
			act(msg, obj->in_room->people, obj, NULL, TO_ALL);

		if (obj->contains) { /* dump contents */
			OBJ_DATA *t_obj, *n_obj;

			if (obj->carried_by) {
				if (!char_in_darena_room(obj->carried_by))
					act("$p's contents scatter on the ground.", obj->carried_by, obj, NULL, TO_ALL);
			}
			else if (obj->in_room != NULL && obj->in_room->people != NULL)
				act("$p's contents scatter on the ground.", obj->in_room->people, obj, NULL, TO_ALL);

			for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj) {
				n_obj = t_obj->next_content;
				obj_from_obj(t_obj);

				if (obj->carried_by) {
					if (obj->carried_by->in_room == NULL)
						extract_obj(t_obj);

					if (char_in_darena_room(obj->carried_by))
						obj_to_char(t_obj, obj->carried_by);
					else
						obj_to_room(t_obj, obj->carried_by->in_room);
				}
				else if (obj->in_room != NULL)
					obj_to_room(t_obj, obj->in_room);
				else {
					extract_obj(t_obj);
					continue;
				}

				cold_effect(t_obj, level / 2, dam / 2, TARGET_OBJ, evolution);
			}
		}

		extract_obj(obj);
	}
}

void fire_effect(void *vo, int level, int dam, int target, int evolution)
{
	if (target == TARGET_ROOM) { /* nail objects on the floor */
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		for (obj = room->contents; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			fire_effect(obj, level, dam, TARGET_OBJ, evolution);
		}

		return;
	}

	if (target == TARGET_CHAR) { /* do the effect on a victim */
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		/* chance of blindness */
		if (!is_blinded(victim)
		 && !saves_spell(level / 4 + dam / 20, victim, DAM_FIRE)) {
			act("$n is blinded by smoke!", victim, NULL, NULL, TO_ROOM);
			act("Your eyes tear up from smoke...you can't see a thing!",
			    victim, NULL, NULL, TO_CHAR);

			affect_add_sn_to_char(victim,
				gsn_fire_breath,
				level,
				number_range(0, 2),
				URANGE(1, evolution - 1, 4),
				FALSE
			);
		}

		/* getting thirsty */
		if (!IS_NPC(victim))
			gain_condition(victim, COND_THIRST, dam / 20);

		/* let's toast some gear! */
		if (!IS_IMMORTAL(victim)) {
			for (obj = victim->carrying; obj != NULL; obj = obj_next) {
				obj_next = obj->next_content;
				fire_effect(obj, level, dam, TARGET_OBJ, evolution);
			}
		}

		return;
	}

	if (target == TARGET_OBJ) { /* toast an object */
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		int chance;
		char *msg;
		CHAR_DATA *owner;

		/* sheen protects absolutely */
		if ((owner = obj->carried_by) != NULL
		    && affect_find_in_char(owner, gsn_sheen))
			return;

		if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)
		    ||  IS_OBJ_STAT(obj, ITEM_NOPURGE)
		    ||  number_range(0, 4) == 0)
			return;

		chance = level / 4 + dam / 10;

		if (chance > 25)
			chance = (chance - 25) / 2 + 25;

		if (chance > 50)
			chance = (chance - 50) / 2 + 50;

		if (IS_OBJ_STAT(obj, ITEM_BLESS))
			chance -= 5;

		chance -= obj->level * 2;

		switch (obj->item_type) {
		default:
			return;

		case ITEM_CONTAINER:
			msg = "$p ignites and burns!";
			break;

		case ITEM_POTION:
			chance += 25;
			msg = "$p bubbles and boils!";
			break;

		case ITEM_SCROLL:
			chance += 50;
			msg = "$p crackles and burns!";
			break;

		case ITEM_STAFF:
			chance += 10;
			msg = "$p smokes and chars!";
			break;

		case ITEM_WAND:
			msg = "$p sparks and sputters!";
			break;

		case ITEM_FOOD:
			msg = "$p blackens and crisps!";
			break;

		case ITEM_PILL:
			msg = "$p melts and drips!";
			break;
		}

		chance = URANGE(5, chance, 95);

		if (number_percent() > chance)
			return;

		if (obj->carried_by)
			act(msg, obj->carried_by, obj, NULL, TO_ALL);
		else if (obj->in_room != NULL && obj->in_room->people != NULL)
			act(msg, obj->in_room->people, obj, NULL, TO_ALL);

		if (obj->contains) { /* dump contents */
			OBJ_DATA *t_obj, *n_obj;

			if (obj->carried_by) {
				if (!char_in_darena_room(obj->carried_by))
					act("$p's contents scatter on the ground.", obj->carried_by, obj, NULL, TO_ALL);
			}
			else if (obj->in_room != NULL && obj->in_room->people != NULL)
				act("$p's contents scatter on the ground.", obj->in_room->people, obj, NULL, TO_ALL);

			for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj) {
				n_obj = t_obj->next_content;
				obj_from_obj(t_obj);

				if (obj->carried_by) {
					if (obj->carried_by->in_room == NULL)
						extract_obj(t_obj);

					if (char_in_darena_room(obj->carried_by))
						obj_to_char(t_obj, obj->carried_by);
					else
						obj_to_room(t_obj, obj->carried_by->in_room);
				}
				else if (obj->in_room != NULL)
					obj_to_room(t_obj, obj->in_room);
				else {
					extract_obj(t_obj);
					continue;
				}

				fire_effect(t_obj, level / 2, dam / 2, TARGET_OBJ, evolution);
			}
		}

		extract_obj(obj);
	}
}

void poison_effect(void *vo, int level, int dam, int target, int evolution)
{
	if (target == TARGET_ROOM) { /* nail objects on the floor */
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		for (obj = room->contents; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			poison_effect(obj, level, dam, TARGET_OBJ, evolution);
		}

		return;
	}

	if (target == TARGET_CHAR) { /* do the effect on a victim */
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		/* chance of poisoning */
		if (!saves_spell(level / 4 + dam / 20, victim, DAM_POISON)) {
			stc("You feel poison coursing through your veins.\n",
			    victim);
			act("$n looks very ill.", victim, NULL, NULL, TO_ROOM);

			affect_add_sn_to_char(victim,
				gsn_poison,
				level,
				level / 2,
				URANGE(1, evolution - 1, 4),
				FALSE
			);
		}

		/* equipment */
		if (!IS_IMMORTAL(victim)) {
			for (obj = victim->carrying; obj != NULL; obj = obj_next) {
				obj_next = obj->next_content;
				poison_effect(obj, level, dam, TARGET_OBJ, evolution);
			}
		}

		return;
	}

	if (target == TARGET_OBJ) { /* do some poisoning */
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		int chance;

		if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)
		    ||  IS_OBJ_STAT(obj, ITEM_BLESS)
		    ||  number_range(0, 4) == 0)
			return;

		chance = level / 4 + dam / 10;

		if (chance > 25)
			chance = (chance - 25) / 2 + 25;

		if (chance > 50)
			chance = (chance - 50) / 2 + 50;

		chance -= obj->level * 2;

		switch (obj->item_type) {
		default:
			return;

		case ITEM_FOOD:
			break;

		case ITEM_DRINK_CON:
			if (obj->value[0] == obj->value[1])
				return;

			break;
		}

		chance = URANGE(5, chance, 95);

		if (number_percent() > chance)
			return;

		obj->value[3] = 1;
		return;
	}
}

void shock_effect(void *vo, int level, int dam, int target, int evolution)
{
	if (target == TARGET_ROOM) {
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		for (obj = room->contents; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			shock_effect(obj, level, dam, TARGET_OBJ, evolution);
		}

		return;
	}

	if (target == TARGET_CHAR) {
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		/* daze and confused? */
		if (!saves_spell(level / 4 + dam / 20, victim, DAM_ELECTRICITY)) {
			stc("Your muscles stop responding.\n", victim);
			DAZE_STATE(victim, UMAX(12, level / 4 + dam / 20));
		}

		/* toast some gear */
		if (!IS_IMMORTAL(victim)) {
			for (obj = victim->carrying; obj != NULL; obj = obj_next) {
				obj_next = obj->next_content;
				shock_effect(obj, level, dam, TARGET_OBJ, evolution);
			}
		}

		return;
	}

	if (target == TARGET_OBJ) {
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		int chance;
		char *msg;
		CHAR_DATA *owner;

		/* sheen protects absolutely */
		if ((owner = obj->carried_by) != NULL
		    && affect_find_in_char(owner, gsn_sheen))
			return;

		if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)
		    ||  IS_OBJ_STAT(obj, ITEM_NOPURGE)
		    ||  number_range(0, 4) == 0)
			return;

		chance = level / 4 + dam / 10;

		if (chance > 25)
			chance = (chance - 25) / 2 + 25;

		if (chance > 50)
			chance = (chance - 50) / 2 + 50;

		if (IS_OBJ_STAT(obj, ITEM_BLESS))
			chance -= 5;

		chance -= obj->level * 2;

		switch (obj->item_type) {
		default:
			return;

		case ITEM_WAND:
		case ITEM_STAFF:
			chance += 10;
			msg = "$p overloads and explodes!";
			break;

		case ITEM_JEWELRY:
			chance -= 10;
			msg = "$p is fused into a worthless lump.";
		}

		chance = URANGE(5, chance, 95);

		if (number_percent() > chance)
			return;

		if (obj->carried_by)
			act(msg, obj->carried_by, obj, NULL, TO_ALL);
		else if (obj->in_room != NULL && obj->in_room->people != NULL)
			act(msg, obj->in_room->people, obj, NULL, TO_ALL);

		if (obj->contains) { /* dump contents */
			OBJ_DATA *t_obj, *n_obj;

			if (obj->carried_by) {
				if (!char_in_darena_room(obj->carried_by))
					act("$p's contents scatter on the ground.", obj->carried_by, obj, NULL, TO_ALL);
			}
			else if (obj->in_room != NULL && obj->in_room->people != NULL)
				act("$p's contents scatter on the ground.", obj->in_room->people, obj, NULL, TO_ALL);

			for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj) {
				n_obj = t_obj->next_content;
				obj_from_obj(t_obj);

				if (obj->carried_by) {
					if (obj->carried_by->in_room == NULL)
						extract_obj(t_obj);

					if (char_in_darena_room(obj->carried_by))
						obj_to_char(t_obj, obj->carried_by);
					else
						obj_to_room(t_obj, obj->carried_by->in_room);
				}
				else if (obj->in_room != NULL)
					obj_to_room(t_obj, obj->in_room);
				else {
					extract_obj(t_obj);
					continue;
				}

				shock_effect(t_obj, level / 2, dam / 2, TARGET_OBJ, evolution);
			}
		}

		extract_obj(obj);
	}
}

