#include "conn/State.hh"
#include "Descriptor.hh"
#include "String.hh"
#include "Character.hh"
#include "Game.hh"
#include "interp.hh"
#include "Logging.hh"
#include "comm.hh"
#include "channels.hh"
#include "act.hh"
#include "sql.hh"
#include "file.hh"
#include "World.hh"
#include "RoomPrototype.hh"
#include "merc.hh"

namespace conn {

unsigned long update_records()
{
	Descriptor *d;
	int count = 0;

	if (Game::port != DIZZYPORT)
		return 0;

	db_command("update_records", "UPDATE records SET logins=logins+1");

	for (d = descriptor_list; d != nullptr; d = d->next)
		if (d->is_playing())
			count++;

	Game::record_players_since_boot = std::max(count, Game::record_players_since_boot);

	if (Game::record_players_since_boot > Game::record_players) {
		Game::record_players = Game::record_players_since_boot;
		db_commandf("update_records", "UPDATE records SET players=%d", Game::record_players_since_boot);
	}

	return ++Game::record_logins;
}

State * ReadNewMOTDState::
handleInput(Descriptor *d, const String& argument) {
	Character *ch = d->character;

	set_color(ch, WHITE, NOBOLD);

	if (ch->pcdata == nullptr || ch->pcdata->pwd.empty()) {
		ptc(ch, "Warning! Null password!\n");
		ptc(ch, "Please report old password with 'bug'.\n");
		ptc(ch, "Type 'password null <new password>' to fix.\n");
	}

	Game::world().add_char(ch);
	Game::world().add_player(ch->pcdata);

	if (ch->level == 0) {
		Object *obj;   /* a generic object variable */
		ATTR_BASE(ch, stat_to_attr(guild_table[ch->guild].stat_prime)) += 3;
		ch->level       = 1;
		ch->exp         = exp_per_level(ch, ch->pcdata->points);
		ch->hit         = GET_MAX_HIT(ch);
		ch->mana        = GET_MAX_MANA(ch);
		ch->stam        = GET_MAX_STAM(ch);
		ch->train       += 3;
		ch->practice    += 5;
		set_title(ch, "({VNewbie Aura{x)");
		do_outfit(ch, "");
		/* This is ugly and doesn't error check. -- Outsider
		   I'm re-writing it underneath.
		       obj_to_char(create_object(Game::world().get_obj_prototype(OBJ_VNUM_MAP), 0), ch);
		       obj_to_char(create_object(Game::world().get_obj_prototype(OBJ_VNUM_TOKEN), 0), ch);
		*/
		obj = create_object(Game::world().get_obj_prototype(OBJ_VNUM_MAP), 0);
		obj_to_char(obj, ch);

		obj = create_object(Game::world().get_obj_prototype(OBJ_VNUM_TOKEN), 0);
		obj_to_char(obj, ch);

		char_to_room(ch, Game::world().get_room(Location(Vnum(ROOM_VNUM_SCHOOL))));
		stc("\n", ch);
		set_color(ch, PURPLE, BOLD);
		help(ch, "NEWBIE INFO");
		set_color(ch, WHITE, NOBOLD);
		stc("\n", ch);
	}
	else if (ch->in_room != nullptr)
		char_to_room(ch, ch->in_room);
	else if (IS_IMMORTAL(ch))
		char_to_room(ch, Game::world().get_room(Location(Vnum(ROOM_VNUM_CHAT))));
	else
		char_to_room(ch, Game::world().get_room(Location(Vnum(ROOM_VNUM_TEMPLE))));

	if (ch->pcdata->email.empty()) {
		set_color(ch, RED, BOLD);
		stc("Your e-mail has not been set, please update your email address\n", ch);
		stc("with the email command!\n\n", ch);
		set_color(ch, WHITE, NOBOLD);
	}

	for (Descriptor *sd = descriptor_list; sd != nullptr; sd = sd->next) {
		Character *victim = sd->original ? sd->original : sd->character;

		if (sd->is_playing()
		    && sd->character != ch
		    && can_see_who(victim, ch)
		    && !victim->comm_flags.has(COMM_NOANNOUNCE)
		    && !victim->comm_flags.has(COMM_QUIET)) {
			if (ch->pcdata && !ch->pcdata->gamein.empty()) {
				set_color(victim, GREEN, BOLD);
				ptc(victim, "[%s] %s\n", ch->name, ch->pcdata->gamein);
			}
			else {
				new_color(victim, CSLOT_CHAN_ANNOUNCE);
				ptc(victim, "[FYI] %s has entered the game.\n", ch->name);
			}

			set_color(victim, WHITE, NOBOLD);
		}
	}

	do_look(ch, "auto");
	wiznet("$N has left real life behind.", ch, nullptr, WIZ_LOGINS, WIZ_SITES, GET_RANK(ch));

	if (ch->pet != nullptr) {
		char_to_room(ch->pet, ch->in_room);
		act("$n has entered the game.", ch->pet, nullptr, nullptr, TO_ROOM);
	}

	do_unread(ch, "");
	stc("\n", ch);

	if (!ch->censor_flags.has(CENSOR_CHAN))
		stc("{BL{Ce{gg{Wa{Cc{By{x is currently rated {PR{x.\n", ch);
	else
		stc("{BL{Ce{gg{Wa{Cc{By{x is currently rated {GPG{x.\n", ch);

	if (get_play_seconds(ch) - ch->pcdata->backup >= 3600) {
		int last = (get_play_seconds(ch) - ch->pcdata->backup) / 3600;

		if (ch->pcdata->backup == 0)
			stc("{W{fThere is currently no backup for your character.{x\n", ch);
		else
			ptc(ch, "%sYou have not backed up for %s%d%s hour%s of gameplay.{x\n",
			    last > 24 ? "{P" : "{W",
			    last > 24 ? "{V" : "{G",
			    last,
			    last > 24 ? "{P" : "{W",
			    last > 1  ? "s"  : "");
	}

	ptc(ch, "\nYour last login was from [{W%s{x] %s",
	    !ch->pcdata->last_lsite.empty() ? ch->pcdata->last_lsite : "Not Available",
	    ch->pcdata->last_ltime != (time_t) 0 ? dizzy_ctime(&ch->pcdata->last_ltime) : "00:00:00");
	ch->pcdata->last_ltime = Game::current_time;
	ch->pcdata->last_lsite = d->host;
	ptc(ch, "\n{VYou are traveler [%lu] of Legacy!!!{x\n", update_records());
	update_pc_index(ch, false);

	/* VT100 Stuff */
	if (ch->pcdata && ch->pcdata->video_flags.has(PLR_VT100)) {
		goto_line(ch, 1, 1);
		clear_window(ch);
		set_window(ch, 1, ch->lines - 2);
		goto_line(ch, ch->lines, 1);
	}

	return &State::playing;
}

} // namespace conn
