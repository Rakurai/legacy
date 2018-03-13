#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"
#include "comm.hh"
#include "sql.hh"
#include "Logging.hh"
#include "channels.hh"
#include "Game.hh"
#include "World.hh"
#include "RoomPrototype.hh"
#include "merc.hh"

namespace conn {

bool check_deny(const String& name)
{
	if (db_countf("check_deny", "SELECT COUNT(*) FROM denies WHERE name='%s'", name) <= 0)
		return false;

	return true;
}

bool check_ban(const String& site, int type)
{
	bool ban = false;

	if (db_queryf("check_ban", "SELECT flags, site FROM bans WHERE ((flags>>2)<<2)=%d", type) != SQL_OK)
		return false;

	while (!ban && db_next_row() == SQL_OK) {
		Flags flags = db_get_column_flags(0);
		String str = db_get_column_str(1);
		bool prefix = flags.has(BAN_PREFIX);
		bool suffix = flags.has(BAN_SUFFIX);

		if ((prefix  &&  suffix && str.is_infix_of(site))
		    || (prefix  && !suffix && str.is_suffix_of(site))
		    || (!prefix &&  suffix && str.is_prefix_of(site))
		    || (!prefix && !suffix && site == str))
			ban = true;
	}

	return ban;
}

/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect(Descriptor *d, const String& name)
{
	for (auto ch : Game::world().char_list) {
		if (!ch->is_npc()
		 && d->character != ch
		 && d->character->name == ch->name) {
			d->character->pcdata->pwd = ch->pcdata->pwd;
			return true;
		}
	}

	return false;
}

State * GetNameState::
handleInput(Descriptor *d, const String& args) {
	String argument(args);

	if (argument.empty()) {
		close_socket(d);
		return &State::closed;
	}

	if (args == "new") {
		if (Game::newlock) {
			write_to_buffer(d, 
				"Due to technical difficulties, we are not accepting new players\n"
			    "at this time.  Please try again in a few hours.\n");
			close_socket(d);
			return &State::closed;
		}

		if (check_ban(d->host, BAN_NEWBIES)) {
			String log_buf = Format::format("Disconnecting because NewbieBANned: %s", d->host);
			Logging::log(log_buf);
			wiznet(log_buf, nullptr, nullptr, WIZ_LOGINS, 0, 0);
			write_to_buffer(d, 
				"New players are not allowed from your site.\n"
			    "If you feel that your site has been banned in error, or would\n"
			    "like to request special permission to play, please contact us at:\n"
			    "   admin@ageoflegacy.com\n");
			close_socket(d);
			return &State::closed;
		}

		load_char_obj(d, ""); // makes a blank character with normal player stuff filled in
		State::getRace.transitionIn(d->character);
		return &State::getRace;
	}

	bool logon_lurk = false;

	if (argument[0] == '-') { /* Lurk mode -- Elrac */
		logon_lurk = true;
		argument.erase(0, 1);
	}

	char name[MIL];
	strcpy(name, argument);
	name[0] = toupper(name[0]);

	/* Below this point we have a character, we can use stc */
	/********************************************************/
	bool fOld = load_char_obj(d, name);
	Character *ch = d->character;

	/********************************************************/

	if (!fOld) {
		write_to_buffer(d, "Sorry, no characters by that name were found.\n"
			               "Try another name, or type 'new' to start a new character.\n"
			               "\n"
			               "Name: ");
		delete ch;
		d->character = nullptr;
		return this;
	}

	if (check_deny(ch->name)) {
		Logging::logf("Denying access to %s@%s.", ch->name, d->host);
		write_to_buffer(d, "You are denied access to Legacy.\n");
		close_socket(d);
		return &State::closed;
	}

	if (check_ban(d->host, BAN_ALL) && !ch->act_flags.has(PLR_PERMIT)) {
		String log_buf = Format::format("Disconnecting because BANned: %s", d->host);
		Logging::log(log_buf);
		wiznet(log_buf, nullptr, nullptr, WIZ_LOGINS, 0, 0);
		write_to_buffer(d, "Your site has been banned from this mud.\n"
		                "If you feel that your site has been banned in error, or would\n"
		                "like to request special permission to play, please contact us at:\n"
		                "   admin@ageoflegacy.com\n");
		close_socket(d);
		return &State::closed;
	}

	if (check_reconnect(d, name))
		fOld = true;
	else if (Game::wizlock && !IS_IMMORTAL(ch)) {
		write_to_buffer(d, "Access has been limited to imms only at this time.\n");
		close_socket(d);
		return &State::closed;
	}

	if (logon_lurk && IS_IMMORTAL(ch))
		ch->lurk_level = LEVEL_IMMORTAL;

	write_to_buffer(d, "What is your password? ");
	echo_off(d);
	return &State::getOldPass;
}

} // namespace conn
