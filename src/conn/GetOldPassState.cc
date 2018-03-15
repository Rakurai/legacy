#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"
#include "comm.hh"
#include "Game.hh"
#include "Logging.hh"
#include "channels.hh"
#include "sql.hh"
#include "World.hh"
#include "Room.hh"

namespace conn {

/*
 * Check if already playing.
 */
bool check_playing(Descriptor *d, const String& name)
{
	Descriptor *dold;

	for (dold = descriptor_list; dold; dold = dold->next) {
		if (dold != d
		    &&   dold->character != nullptr
		    &&   dold->state != &conn::State::getName
		    &&   dold->state != &conn::State::getOldPass
		    &&   name == (dold->original
		                  ? dold->original->name : dold->character->name)) {
			write_to_buffer(d, "That character is already playing.\n");
			write_to_buffer(d, "Do you wish to connect anyway (Y/N)?");
			return true;
		}
	}

	return false;
}

/*
 * Look for link-dead player to reconnect.
 */
bool attempt_reconnect(Descriptor *d, const String& name)
{
	for (auto ch : Game::world().char_list) {
		if (!ch->is_npc()
		 && d->character != ch
		 && ch->desc == nullptr
		 && d->character->name == ch->name) {
			Character *rch;
			delete d->character;
			d->character = ch;
			ch->desc         = d;
			ch->desc->timer  = 0;
			stc("Reconnecting...\n", ch);

			if (!ch->is_npc())
				if (!ch->pcdata->buffer.empty())
					stc("You have messages: Type 'replay'\n", ch);

			for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
				if (ch != rch && can_see_char(rch, ch))
					ptc(rch, "%s has reconnected.\n", PERS(ch, rch, VIS_CHAR));

			Logging::logf("%s@%s reconnected.", ch->name, d->host);
			wiznet("$N reclaims the fullness of $S link.",
			       ch, nullptr, WIZ_LINKS, 0, 0);

			Room *room = ch->in_room;

			if (room != nullptr) {
				char_from_room(ch);
				char_to_room(ch, room);
			}

			ch->pcdata->plr_flags -= PLR_LINK_DEAD;
			return true;
		}
	}

	return false;
}

const String site_to_ssite(const String& site)
{
	bool alpha = false;
	int dotcount = 0;

	/* Parse the site, determine type.  For alphanumeric hosts, we
	   match the last three dot sections, for straight numerics we
	   match the first three. */
	for (const char *p = site.c_str(); *p; p++) {
		if (*p == '.')
			dotcount++;
		else if (!isdigit(*p))
			alpha = true;
	}

	if (alpha) {
		if (dotcount < 3)
			return site;

		return site.substr(site.find_nth(dotcount - 2, "."));
	}

	return site.substr(0, site.find_nth(3, "."));
}

void update_site(const Character *ch)
{
	String shortsite = site_to_ssite(ch->desc->host);

	if (db_countf("update_site",
	              "SELECT COUNT(*) FROM sites WHERE name='%s' AND ssite='%s'", ch->name, shortsite) > 0)
		db_commandf("update_site", "UPDATE sites SET lastlog=DATE(), site='%s' WHERE name='%s' AND ssite='%s'",
		            db_esc(ch->desc->host), db_esc(ch->name), db_esc(shortsite));
	else
		db_commandf("update_site", "INSERT INTO sites VALUES('%s','%s','%s',DATE())",
		            db_esc(ch->name), db_esc(ch->desc->host), db_esc(shortsite));
}

State * GetOldPassState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	write_to_buffer(d, "\n");

	if (strcmp(argument, ch->pcdata->pwd)) {
		stc("{bIncorrect password!{x\n", ch);
		close_socket(d);
		return &State::closed;
	}

	echo_on(d);

	if (check_playing(d, ch->name))
		return &State::breakConnect;

	if (attempt_reconnect(d, ch->name))
		return &State::playing;

	ch->pcdata->plr_flags -= PLR_LINK_DEAD;
	quest_cleanup(ch);
	sq_cleanup(ch);

	String log_buf = Format::format("%s@%s has connected.", ch->name, d->host);
	Logging::log(log_buf);
	wiznet(log_buf, nullptr, nullptr, WIZ_SITES, WIZ_LOGINS, GET_RANK(ch));

	log_buf = Format::format("Last Site: %s",
	        !ch->pcdata->last_lsite.empty() ? ch->pcdata->last_lsite : "Not Available");
	wiznet(log_buf, nullptr, nullptr, WIZ_SITES, WIZ_LOGINS, GET_RANK(ch));

	update_site(ch);

	if (IS_IMMORTAL(ch)) {
		set_color(ch, RED, BOLD);
		help(ch, "imotd");
		set_color(ch, WHITE, NOBOLD);
		return &State::readIMOTD;
	}

	set_color(ch, CYAN, NOBOLD);
	help(ch, "automotd");
	set_color(ch, WHITE, NOBOLD);

	return &State::readMOTD;
}

} // namespace conn
