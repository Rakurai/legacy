#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"
#include "Game.hh"
#include "Area.hh"
#include "MobilePrototype.hh"
#include "unistd.h"
#include "merc.hh"
#include "World.hh"

extern bool check_parse_name(const String& name);

namespace conn {

bool check_player_exists(Descriptor *d, const String& name)
{
	Descriptor *dold;
	StoredPlayer *exist = nullptr;    /* is character in storage */

	String strsave = Format::format("%s%s", PLAYER_DIR, name.lowercase().capitalize());
	if (access(strsave.c_str(), F_OK) != -1) {
		ptc(d->character,
			"A character by that name already exists.  Please choose something else.\n");
		return true;
	}

	for (dold = descriptor_list; dold; dold = dold->next) {
		if (dold != d
		    &&   dold->character != nullptr
		    &&   dold->character->level < 1
		    &&   dold->state != &conn::State::playing
		    &&   name == (dold->original
		                  ? dold->original->name : dold->character->name)) {
			ptc(d->character,
                "A character by that name is currently being created.\n"
                "You cannot access that character.\n"
                "Please create a character with a different name, and\n"
                "ask an Immortal for help if you need it.\n");
			return true;
		}
	}

	/* make sure that we do not re-create a character currently
	   in storage -- Outsider <slicer69@hotmail.com>
	*/
	/* first make sure we have the list of stored characters */
	if (! storage_list_head)
		load_storage_list();

	/* search storage for character name */
	exist = lookup_storage_data(name);

	if (exist) {
		ptc(d->character,
            "A character by that name is currently in storage.\n"
            "You cannot create a character by this name.\n"
            "Please create a character with a different name, and\n"
            "ask an Immortal for help if you need it.\n");
		return true;
	}

	return false;
}

bool check_mob_exists(const String& name)
{
	for (const auto& area_pair : Game::world().areas)
		for (const auto& pair : area_pair.second->mob_prototypes)
			if (pair.second->player_name.has_exact_words(name))
				return true;

	return false;
}

void GetNewNameState::
prompt(Character *ch) {
	ptc(ch, "\n{YEnter your name:{x ");
}

void GetNewNameState::
transitionIn(Character *ch) {
	help(ch, "creation_get_new_name_1");
	ptc(ch, race_table[ch->race].name);
	help(ch, "creation_get_new_name_2");
	prompt(ch);
}

State * GetNewNameState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	String name = argument.strip().capitalize();

	if (check_player_exists(d, name)) {
		prompt(ch);
		return this;
	}

	/* check for attempt to newly create with a mob name -- Elrac */
	if (check_mob_exists(name)) {
		ptc(ch, "Sorry, we already have a mobile by that name.\n"
		        "Please choose another name!\n");
		prompt(ch);
		return this;
	}

	/* Check valid name - Lotus */
	if (!check_parse_name(name)) {
		ptc(ch, "Sorry, that name cannot be used.\n"
		        "Please choose another name!\n");
		prompt(ch);
		return this;
	}

	ch->name = name;
	State::confirmNewName.transitionIn(ch);
	return &State::confirmNewName;
}

} // namespace conn
