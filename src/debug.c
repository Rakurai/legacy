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
#include "sql.h"
#include "recycle.h"

extern AREA_DATA *area_first;

/* DEBUG command, by Elrac. This can be modified for various subfunctions */
void do_debug(CHAR_DATA *ch, const char *argument)
{
	char subfunc[MIL];
	argument = one_argument(argument, subfunc);

	if (subfunc[0] == '\0') {
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
		    "  objstate - save all objects lying on the ground\n", ch);
		return;
	}

	if (!strcmp(subfunc, "define")) {
		stc("Defined:\n", ch);
#if defined(sun)
		stc("sun\n", ch);
#endif
#if defined(MALLOC_DEBUG)
		stc("MALLOC_DEBUG\n", ch);
#endif
#if defined(apollo)
		stc("apollo\n", ch);
#endif
#if defined(unix)
		stc("unix\n", ch);
#endif
#if defined(STDOUT_FILENO)
		stc("STDOUT_FILENO\n", ch);
#endif
#if defined(_AIX)
		stc("_AIX\n", ch);
#endif
#if defined(__hpux)
		stc("__hpux\n", ch);
#endif
#if defined(interactive)
		stc("interactive\n", ch);
#endif
#if defined(linux)
		stc("linux\n", ch);
#endif
#if defined(MIPS_OS)
		stc("MIPS_OS\n", ch);
#endif
#if defined(NeXT)
		stc("NeXT\n", ch);
#endif
#if defined(htons)
		stc("htons\n", ch);
#endif
#if defined(ntohl)
		stc("ntohl\n", ch);
#endif
#if defined(sequent)
		stc("sequent\n", ch);
#endif
#if defined(SYSV)
		stc("SYSV\n", ch);
#endif
#if defined(ultrix)
		stc("ultrix\n", ch);
#endif
#if defined(SAND)
		stc("SAND\n", ch);
#endif
#if defined(SO_DONTLINGER)
		stc("SO_DONTLINGER\n", ch);
#endif
#if defined(FNDELAY)
		stc("FNDELAY\n", ch);
#endif
#if defined(OLD_RAND)
		stc("OLD_RAND\n", ch);
#endif
#if defined(NOCRYPT)
		stc("NOCRYPT\n", ch);
#endif
		stc("Not defined:\n", ch);
#if !defined(sun)
		stc("sun\n", ch);
#endif
#if !defined(MALLOC_DEBUG)
		stc("MALLOC_DEBUG\n", ch);
#endif
#if !defined(apollo)
		stc("apollo\n", ch);
#endif
#if !defined(unix)
		stc("unix\n", ch);
#endif
#if !defined(STDOUT_FILENO)
		stc("STDOUT_FILENO\n", ch);
#endif
#if !defined(_AIX)
		stc("_AIX\n", ch);
#endif
#if !defined(__hpux)
		stc("__hpux\n", ch);
#endif
#if !defined(interactive)
		stc("interactive\n", ch);
#endif
#if !defined(linux)
		stc("linux\n", ch);
#endif
#if !defined(MIPS_OS)
		stc("MIPS_OS\n", ch);
#endif
#if !defined(NeXT)
		stc("NeXT\n", ch);
#endif
#if !defined(htons)
		stc("htons\n", ch);
#endif
#if !defined(ntohl)
		stc("ntohl\n", ch);
#endif
#if !defined(sequent)
		stc("sequent\n", ch);
#endif
#if !defined(SYSV)
		stc("SYSV\n", ch);
#endif
#if !defined(ultrix)
		stc("ultrix\n", ch);
#endif
#if !defined(SAND)
		stc("SAND\n", ch);
#endif
#if !defined(SO_DONTLINGER)
		stc("SO_DONTLINGER\n", ch);
#endif
#if !defined(FNDELAY)
		stc("FNDELAY\n", ch);
#endif
#if !defined(OLD_RAND)
		stc("OLD_RAND\n", ch);
#endif
#if !defined(NOCRYPT)
		stc("NOCRYPT\n", ch);
#endif
		return;
	}
/*
	if (!strcmp(subfunc, "fullupdate")) {
		MYSQL_RES *result;
		MYSQL_ROW row;
		DESCRIPTOR_DATA *d;
		int desc = 6;

		for (d = descriptor_list; d != NULL; d = d->next)
			desc++;

		if ((result = db_query("do_debug:fullupdate", "SELECT name FROM pc_index")) == NULL)
			return;

		while ((row = mysql_fetch_row(result))) {
			CHAR_DATA *victim;

			if (get_player_world(ch, row[0], VIS_CHAR))
				continue;

			db_commandf("do_debug:fullupdate", "DELETE FROM pc_index WHERE name='%s'", db_esc(row[0]));
			d = new_descriptor();
			d->descriptor    = desc;
			d->connected     = CON_PLAYING;

			if (!load_char_obj(d, row[0])) {
				free_char(d->character);
				free_descriptor(d);
				continue;
			}

			victim = d->character;
			victim->next = char_list;
			char_list    = victim;
			victim->pcdata->next = pc_list;
			pc_list = victim->pcdata;
			victim->desc = NULL;
			free_descriptor(d);
			char_to_room(victim, get_room_index(ROOM_VNUM_ALTAR));
			db_commandf("do_debug:fullupdate",
			            "INSERT INTO pc_index VALUES('%s','%s','%s','%s',%ld,%d,%d,'%s','%s')",
			            db_esc(victim->name),
			            db_esc(victim->pcdata->title),
			            db_esc(victim->pcdata->deity),
			            db_esc(smash_bracket(victim->pcdata->deity)),
			            victim->pcdata->cgroup,
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
	if (!strcmp(subfunc, "rng")) {
		long iterations = atol(argument);

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
		long iterations = atol(argument);

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
		AREA_DATA *area;

		for (area = area_first; area != NULL; area = area->next)
			ptc(ch, "%-20s%d\n", area->file_name, area->version);

		return;
	}

	if (!strcmp(subfunc, "compart")) {
		OBJ_DATA *container, *obj;

		for (obj = object_list; obj != NULL; obj = obj->next) {
			if ((container = obj->in_obj) == NULL)
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
		ROOM_INDEX_DATA *room = NULL;
		int x, vnum;
		bool found;

		for (vnum = 1; vnum < 32600; vnum++) {
			found = FALSE;

			if ((room = get_room_index(vnum)) == NULL)
				continue;

			if (!IS_SET(room->room_flags, ROOM_NO_RECALL))
				continue;

			for (x = 0; x <= 5; x++)
				if (room->exit[x] != NULL)
					found = TRUE;

			if (!found)
				ptc(ch, "{W[{P%5d{W]{x %s\n", vnum, room->name);
		}

		return;
	}

	if (!strcmp(subfunc, "rcheck2")) {
		ROOM_INDEX_DATA *room = NULL;
		int vnum;

		for (vnum = 1; vnum < 32600; vnum++) {
			if ((room = get_room_index(vnum)) == NULL)
				continue;

			if (IS_SET(room->room_flags, ROOM_NOLIGHT))
				ptc(ch, "{W[{P%5d{W]{x %s\n", vnum, room->name);
		}

		return;
	}

	if (!strcmp(subfunc, "qtz")) {
		/* Quest Time Zero */
		char arg[MIL];
		CHAR_DATA *questor;
		argument = one_argument(argument, arg);

		if (!str_cmp(arg, "all")) {
			for (questor = char_list; questor; questor = questor->next)
				if (!IS_NPC(questor) && !IS_IMMORTAL(questor)) {
					questor->nextquest = 0;
					questor->pcdata->nextsquest = 0;
					stc("You may now quest again.\n", questor);
				}

			stc("OK, everyone may quest again immediately.\n", ch);
			return;
		}

		if ((questor = get_player_world(ch, arg, VIS_PLR)) == NULL) {
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
		extern void     weather_update  args((void));
		extern void     char_update     args((void));
		extern void     obj_update      args((void));
		extern void     room_update     args((void));

		if (argument[0] != '\0' && is_number(argument))
			number = URANGE(1, atoi(argument), 100);

		for (count = 0; count < number; count++) {
			weather_update();
			char_update();
			obj_update();
			room_update();
		}

		ptc(ch, "You advance time by %d hour%s.\n", number, number > 1 ? "s" : "");
		return;
	}

	stc("Unknown DEBUG subfunction.\n", ch);
} /* end do_debug() */

