#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"
#include "argument.hh"
#include "lookup.hh"
#include "channels.hh"
#include "Logging.hh"

namespace conn {

State * GetGuildState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	if (argument.has_prefix("help")) {
		String arg1, arg2;
		arg2 = one_argument(argument, arg1);

		if (arg2.empty())
			help(ch, "class help");
		else
			help(ch, arg2);

		ptc(ch, "What is your class? ");

		return this;
	}

	ch->guild = guild_lookup(argument);

	if (ch->guild == Guild::none) {
		ptc(ch, "That is not a class.\nWhat is your class? ");

		return this;
	}

	String log_buf = Format::format("%s@%s new player.", ch->name, d->host);
	wiznet(log_buf, nullptr, nullptr, WIZ_LOGINS, 0, GET_RANK(ch));
	Logging::log(log_buf);
	log_buf = Format::format("Newbie alert!  %s sighted.", ch->name);
	wiznet(log_buf, ch, nullptr, WIZ_NEWBIE, 0, 0);

	/* paladins can't be neutral */
	ptc(ch, "\nYou may be good%s or evil.\nWhich alignment (G%s/E)? ",
	        ch->guild == Guild::paladin ? "" : ", neutral,",
	        ch->guild == Guild::paladin ? "" : "/N");

	return &State::getAlignment;
}

} // namespace conn
