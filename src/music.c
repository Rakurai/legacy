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
#include "music.h"
#include "recycle.h"
#include "Format.hpp"

int channel_songs[MAX_GLOBAL + 1];
std::vector<struct song_data> song_table;

void song_update(void)
{
	Object *obj;
	Character *victim;
	RoomPrototype *room;
	Descriptor *d;
	char buf[MAX_STRING_LENGTH];
	String line;
	int i;

	/* do the global song, if any */
	if (channel_songs[1] >= song_table.size())
		channel_songs[1] = -1;

	if (channel_songs[1] > -1) {
		if (channel_songs[0] >= MAX_LINES
		    ||  channel_songs[0] >= song_table[channel_songs[1]].lines) {
			channel_songs[0] = -1;

			/* advance songs */
			for (i = 1; i < MAX_GLOBAL; i++)
				channel_songs[i] = channel_songs[i + 1];

			channel_songs[MAX_GLOBAL] = -1;
		}
		else {
			if (channel_songs[0] < 0) {
				Format::sprintf(buf, "Music: %s, %s",
				        song_table[channel_songs[1]].group,
				        song_table[channel_songs[1]].name);
				channel_songs[0] = 0;
			}
			else {
				Format::sprintf(buf, "Music: '%s'",
				        song_table[channel_songs[1]].lyrics[channel_songs[0]]);
				channel_songs[0]++;
			}

			for (d = descriptor_list; d != nullptr; d = d->next) {
				victim = d->original ? d->original : d->character;

				if (IS_PLAYING(d) &&
				    !IS_SET(victim->comm, COMM_NOMUSIC) &&
				    !IS_SET(victim->comm, COMM_QUIET)) {
					new_color(victim, CSLOT_CHAN_MUSIC);
					act("$t", d->character, buf, nullptr, TO_CHAR, POS_SLEEPING, FALSE);
					set_color(victim, WHITE, NOBOLD);
				}
			}
		}
	}

	for (obj = object_list; obj != nullptr; obj = obj->next) {
		if (obj->item_type != ITEM_JUKEBOX || obj->value[1] < 0)
			continue;

		if (obj->value[1] >= song_table.size()) {
			obj->value[1] = -1;
			continue;
		}

		/* find which room to play in */

		if ((room = obj->in_room) == nullptr) {
			if (obj->carried_by == nullptr)
				continue;
			else if ((room = obj->carried_by->in_room) == nullptr)
				continue;
		}

		if (obj->value[0] < 0) {
			Format::sprintf(buf, "$p starts playing %s, %s.",
			        song_table[obj->value[1]].group, song_table[obj->value[1]].name);

			if (room->people != nullptr)
				act(buf, room->people, obj, nullptr, TO_ALL);

			obj->value[0] = 0;
			continue;
		}
		else {
			if (obj->value[0] >= MAX_LINES
			    || obj->value[0] >= song_table[obj->value[1]].lines) {
				obj->value[0] = -1;
				/* scroll songs forward */
				obj->value[1] = obj->value[2];
				obj->value[2] = obj->value[3];
				obj->value[3] = obj->value[4];
				obj->value[4] = -1;
				continue;
			}

			line = song_table[obj->value[1]].lyrics[obj->value[0]];
			obj->value[0]++;
		}

		Format::sprintf(buf, "$p bops: '%s'", line);

		if (room->people != nullptr)
			act(buf, room->people, obj, nullptr, TO_ALL);
	}
}

void load_songs(void)
{
	FILE *fp;
	int i;
	char letter;

	/* reset global */
	for (i = 0; i <= MAX_GLOBAL; i++)
		channel_songs[i] = -1;

	if ((fp = fopen(MUSIC_FILE, "r")) == nullptr) {
		bug("Couldn't open music file, no songs available.", 0);
		return;
	}

	while (TRUE) {
		letter = fread_letter(fp);

		if (letter == '#') {
			fclose(fp);
			return;
		}
		else
			ungetc(letter, fp);

		struct song_data song;
		song.group = fread_string(fp);
		song.name  = fread_string(fp);
		song.lines = 0;

		/* read lyrics */
		for (; ;) {
			letter = fread_letter(fp);

			if (letter == '~') {
				break;
			}
			else
				ungetc(letter, fp);

			if (song.lines >= MAX_LINES) {
				bug("Too many lines in a song -- limit is  %d.", MAX_LINES);

				break;
			}

			song.lyrics[song.lines] = fread_string_eol(fp);
			song.lines++;
		}

		if (song.lines > 0)
			song_table.push_back(song);
	}

	fclose(fp); /* One more close than open -  Lotus */
}

void do_play(Character *ch, String argument)
{
	Object *juke;
	const char *str;
	int song, i;
	bool global = FALSE;

	String arg;
	str = one_argument(argument, arg);

	for (juke = ch->carrying; juke != nullptr; juke = juke->next_content) {
		if (juke->item_type == ITEM_JUKEBOX && can_see_obj(ch, juke))
			break;
	}

	if (juke == nullptr) {
		for (juke = ch->in_room->contents; juke != nullptr; juke = juke->next_content) {
			if (juke->item_type == ITEM_JUKEBOX && can_see_obj(ch, juke))
				break;
		}
	}

	if (juke == nullptr) {
		stc("You see nothing to play.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("Play what?\n", ch);
		return;
	}

	if (arg == "list") {
		String buffer;
		char buf[MAX_STRING_LENGTH];
		int col = 0;
		bool artist = FALSE, match = FALSE;
		argument = str;
		argument = one_argument(argument, arg);

		if (arg == "artist")
			artist = TRUE;

		if (!argument.empty())
			match = TRUE;

		buffer += Format::format("%s has the following songs available:\n", juke->short_descr.capitalize());

		for (i = 0; i < song_table.size(); i++) {
			if (artist && (!match
			               ||             argument.is_prefix_of(song_table[i].group)))
				Format::sprintf(buf, "%-39s %-39s\n",
				        song_table[i].group, song_table[i].name);
			else if (!artist && (!match
			                     ||                   argument.is_prefix_of(song_table[i].name)))
				Format::sprintf(buf, "%-35s ", song_table[i].name);
			else
				continue;

			buffer += buf;

			if (!artist && ++col % 2 == 0)
				buffer += "\n";
		}

		if (!artist && col % 2 != 0)
			buffer += "\n";

		page_to_char(buffer, ch);
		return;
	}

	if (arg == "loud" && IS_IMMORTAL(ch)) {
		argument = str;
		global = TRUE;
	}

	if (arg == "stop") {
		if (juke) { /* stop the jukebox in their inventory first */
			if (!juke->carried_by) {
				if (!IS_IMMORTAL(ch)) {
					stc("You can only stop a jukebox in your inventory.\n", ch);
					return;
				}
			}

			ptc(ch, "You stop %s.\n", juke->short_descr);

			for (i = 0; i < 5; i++)
				juke->value[i] = -1;

			return;
		}

		if (!IS_IMMORTAL(ch)) {
			stc("You don't see a jukebox around.\n", ch);
			return;
		}

		for (i = 0; i <= MAX_GLOBAL; i++)
			channel_songs[i] = -1;

		return;
	}

	if (argument.empty()) {
		stc("Play what?\n", ch);
		return;
	}

	if ((global && channel_songs[MAX_GLOBAL] > -1)
	    || (!global && juke->value[4] > -1)) {
		stc("The jukebox is full up right now.\n", ch);
		return;
	}

	for (song = 0; song < song_table.size(); song++) {
		if (argument.is_prefix_of(song_table[song].name))
			break;
	}

	if (song >= song_table.size()) {
		stc("That song isn't available.\n", ch);
		return;
	}

	stc("Coming right up.\n", ch);

	if (global) {
		for (i = 1; i <= MAX_GLOBAL; i++)
			if (channel_songs[i] < 0) {
				if (i == 1)
					channel_songs[0] = -1;

				channel_songs[i] = song;
				return;
			}
	}
	else {
		for (i = 1; i < 5; i++)
			if (juke->value[i] < 0) {
				if (i == 1)
					juke->value[0] = -1;

				juke->value[i] = song;
				return;
			}
	}
}
