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

#include "argument.hh"
#include "affect/Affect.hh"
#include "Area.hh"
#include "Character.hh"
#include "declare.hh"
#include "find.hh"
#include "Flags.hh"
#include "Game.hh"
#include "GameTime.hh"
#include "lootv2.hh"
#include "macros.hh"
#include "merc.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"
#include "Player.hh"
#include "random.hh"
#include "Room.hh"
#include "String.hh"
#include "Weather.hh"

/* DEBUG command, by Elrac. This can be modified for various subfunctions */
void do_debug(Character *ch, String argument)
{
	String subfunc;
	argument = one_argument(argument, subfunc);

	if (subfunc.empty()) {
		stc("Currently defined DEBUG subfunctions are:\n"
		    "  lcheck   - shows a player's locker number and weight, and strongbox number\n"
		    "  rcheck   - check for rooms without exits and NO_RECALL\n"
		    "  rcheck2  - check for rooms flagged NOLIGHT\n"
		    "  qtz      - set next quest time to 0\n"
		    "  tick     - advances the mud by one tick, weather_update, char_update, descrip_update\n"
		    "  compart  - finds all objects inside objects that aren't containers\n"
		    "  newflag  - finds all objects flagged compartment (formerly dark) or lock (nonexistant)\n"
		    "  aversion - lists all areas and their versions\n"
		    "  define   - lists all defines that the preprocessor checks for\n"
		    "  objstate - save all objects lying on the ground\n"
		    "  geneq    - generate a piece of eq\n"
		    "  affcall  - iterate through affects\n", ch);
		return;
	}

	if (subfunc == "objstate") {
		objstate_save_items();
		return;
	}

	if (subfunc == "strtest") {
		for (int row = 0; row < 16; row++) {
			for (int col = 0; col < 8; col++) {
				unsigned char v = row * 8 + col + 128;
				ptc(ch, "%d: %c  ", v, v);
			}
			stc("\n", ch);
		}
		return;
	}

	if (subfunc == "crash") {
		Character *v = nullptr;
		v->name = "test";
		return;
	}
/*
	if (!strcmp(subfunc, "fullupdate")) {
		MYSQL_RES *result;
		MYSQL_ROW row;
		Descriptor *d;
		int desc = 6;

		for (d = descriptor_list; d != nullptr; d = d->next)
			desc++;

		if ((result = db_query("do_debug:fullupdate", "SELECT name FROM pc_index")) == nullptr)
			return;

		while ((row = mysql_fetch_row(result))) {
			Character *victim;

			if (get_player_world(ch, row[0], VIS_CHAR))
				continue;

			db_commandf("do_debug:fullupdate", "DELETE FROM pc_index WHERE name='%s'", db_esc(row[0]));
			d = new_descriptor();
			d->descriptor    = desc;
			d->connected     = CON_PLAYING;

			if (!load_char_obj(d, row[0])) {
				delete d->character;
				free_descriptor(d);
				continue;
			}

			victim = d->character;
			victim->next = char_list;
			char_list    = victim;
			victim->validate();
			victim->pcdata->next = pc_list;
			pc_list = victim->pcdata;
			victim->desc = nullptr;
			free_descriptor(d);
			char_to_room(victim, get_room(ROOM_VNUM_ALTAR));
			db_commandf("do_debug:fullupdate",
			            "INSERT INTO pc_index VALUES('%s','%s','%s','%s',%ld,%d,%d,'%s','%s')",
			            db_esc(victim->name),
			            db_esc(victim->pcdata->title),
			            db_esc(victim->pcdata->deity),
			            db_esc(victim->pcdata->deity.uncolor()),
			            victim->pcdata->cgroup_flags,
			            victim->level,
			            victim->pcdata->remort_count,
			            victim->clan ? victim->clan->name : "",
			            victim->clan && victim->pcdata->rank ? victim->pcdata->rank : "");
			extract_char(victim, TRUE);
		}

		stc("Done.\n", ch);
		mysql_free_result(result);
		return;
	}
*/
	if (!strcmp(subfunc, "geneq")) {
		String arg;
		argument = one_argument(argument, arg);

		if (arg.empty() || !arg.is_number()) {
			stc("Usage:  debug geneq <level>\n", ch);
			return;
		}

		int level = atoi(arg);

		if (level < 1 || level > 100) {
			stc("Level is between 1 and 100.\n", ch);
			return;
		}

		Object *obj = generate_eq(level);

		if (obj)
			obj_to_char(obj, ch);

		stc("Loot Rolled\n", ch);
		return;
	}

	if (!strcmp(subfunc, "rng")) {
		long iterations = atol(argument.c_str());

		long nums[100];

		for (int i = 0; i < 100; i++)
			nums[i] = 0;

		for (long i = 0; i < iterations; i++)
			nums[number_range(1,100)-1]++;

		long sum = 0;

		for (int i = 0; i < 100; i++) {
			sum += nums[i];
			ptc(ch, "[%3d] %.4f %.4f (%ld)\n",
				i + 1,
				100 * nums[i] / (float)iterations,
				100 * sum / (float)iterations,
				nums[i]
			);
		}

		return;
	}

	if (!strcmp(subfunc, "rng2")) {
		long iterations = atol(argument.c_str());

		long nums[100];

		for (int i = 0; i < 100; i++)
			nums[i] = 0;

		for (long i = 0; i < iterations; i++)
			nums[number_percent()-1]++;

		long sum = 0;

		for (int i = 0; i < 100; i++) {
			sum += nums[i];
			ptc(ch, "[%3d] %.4f %.4f (%ld)\n",
				i + 1,
				100 * nums[i] / (float)iterations,
				100 * sum / (float)iterations,
				nums[i]
			);
		}

		return;
	}

	if (!strcmp(subfunc, "aversion")) {
		for (Area *area: Game::world().areas)
			ptc(ch, "%-20s%d\n", area->file_name, area->version);

		return;
	}

	if (!strcmp(subfunc, "compart")) {
		Object *container, *obj;

		for (obj = object_list; obj != nullptr; obj = obj->next) {
			if ((container = obj->in_obj) == nullptr)
				continue;

			if (obj == container) {
				ptc(ch, "obj %d is in itself.\n", obj->pIndexData->vnum);
				continue;
			}

			if (container->item_type != ITEM_CONTAINER)
				ptc(ch, "%s (%d) is in non-container %s (%d).%s\n",
				    obj->short_descr, obj->pIndexData->vnum,
				    container->short_descr, container->pIndexData->vnum,
				    IS_OBJ_STAT(container, ITEM_COMPARTMENT) ? "  ({PCOMPARTMENT{x)" : "");
		}

		return;
	}

	if (!strcmp(subfunc, "rcheck")) {
		Room *room = nullptr;
		int x, vnum;
		bool found;

		for (vnum = 1; vnum < 32600; vnum++) {
			found = FALSE;

			if ((room = get_room(vnum)) == nullptr)
				continue;

			if (!room->flags().has(ROOM_NO_RECALL))
				continue;

			for (x = 0; x <= 5; x++)
				if (room->exit[x] != nullptr)
					found = TRUE;

			if (!found)
				ptc(ch, "{W[{P%5d{W]{x %s\n", vnum, room->name());
		}

		return;
	}

	if (!strcmp(subfunc, "rcheck2")) {
		Room *room = nullptr;
		int vnum;

		for (vnum = 1; vnum < 32600; vnum++) {
			if ((room = get_room(vnum)) == nullptr)
				continue;

			if (room->flags().has(ROOM_NOLIGHT))
				ptc(ch, "{W[{P%5d{W]{x %s\n", vnum, room->name());
		}

		return;
	}

	if (!strcmp(subfunc, "qtz")) {
		/* Quest Time Zero */
		Character *questor;

		String arg;
		argument = one_argument(argument, arg);

		if (arg == "all") {
			for (questor = char_list; questor; questor = questor->next)
				if (!IS_NPC(questor) && !IS_IMMORTAL(questor)) {
					questor->nextquest = 0;
					questor->pcdata->nextsquest = 0;
					stc("You may now quest again.\n", questor);
				}

			stc("OK, everyone may quest again immediately.\n", ch);
			return;
		}

		if ((questor = get_player_world(ch, arg, VIS_PLR)) == nullptr) {
			ptc(ch, "No player named '%s' found in game, sorry!\n", arg);
			return;
		}

		questor->nextquest = 0;
		questor->pcdata->nextsquest = 0;
		ptc(ch, "OK, %s may quest again immediately.\n", questor->name);
		return;
	}

	if (!strcmp(subfunc, "tick")) {
		int count, number = 1;
		extern void     char_update     args((void));
		extern void     obj_update      args((void));
		extern void     room_update     args((void));

		if (!argument.empty() && argument.is_number())
			number = URANGE(1, atoi(argument), 100);

		for (count = 0; count < number; count++) {
			Game::world().time.update();
			Game::world().weather.update();
			char_update();
			obj_update();
			room_update();
		}

		ptc(ch, "You advance time by %d hour%s.\n", number, number > 1 ? "s" : "");
		return;
	}

	stc("Unknown DEBUG subfunction.\n", ch);
} /* end do_debug() */

