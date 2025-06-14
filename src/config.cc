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

#include <vector>

#include "argument.hh"
#include "Character.hh"
#include "declare.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Logging.hh"
#include "merc.hh"
#include "Player.hh"
#include "String.hh"
#include "tables.hh"
#include "typename.hh"

/* TEMPLATE
void config_wiznet(Character *ch, const char *argument)
{
        char arg1[MIL];
        int argnum = 0;

        argument = one_argument(argument, arg1);

        if (arg1.empty())
        {
                stc("Use 'help' or '?' as an argument after any option for details.\n", ch);
                stc("Censor options:\n\n", ch);
                ptc(ch, "  1.  Channels                                   %s\n",
                        ch->censor_flags.has(CENSOR_CHAN) ?  "{CON{x" : "{POFF{x");
                return;
        }

             if (arg1.is_number())                       argnum = atoi(arg1);
        else if (arg1.is_prefix_of("channels"))        argnum = 1;
        else if (arg1.is_prefix_of("spam"))            argnum = 3;

        switch(argnum)
        {
                default:        break;

                case 1:
                        return;
        }

        stc("That is not a valid censor option.\n", ch);
}*/

void config_color_func(Character *ch, String argument, int type)
{
	String typestr;
	int low, high, mod, slot;

	switch (type) {
	case 3: Format::sprintf(typestr, "channel");            low = 0; high = 29; mod = 1;    break;
	case 4: Format::sprintf(typestr, "score");              low = 50; high = 79; mod = -49;	break;
	case 5: Format::sprintf(typestr, "miscellaneous");      low = 80; high = 99; mod = -79; break;
	default:
		Logging::bug("config_color_func: bad type %d", type);
		return;
	}

	if (argument.empty()) {
		stc("Use the color name or 'reset' after an option to modify.\n", ch);
		ptc(ch, "%s color settings:\n\n", typestr.capitalize());

		for (slot = low; slot < high; slot++) {
			if (csetting_table[slot].name.empty())
				continue;

			if (type == 3
			    && ((slot == CSLOT_CHAN_CLAN   && !ch->clan && !IS_IMMORTAL(ch))
			        || (slot == CSLOT_CHAN_SPOUSE && !ch->pcdata->plr_flags.has(PLR_MARRIED))
			        || (slot == CSLOT_CHAN_IMM    && !IS_IMMORTAL(ch))))
				continue;

			new_color(ch, slot);
			ptc(ch, " {g%2d.{x  %-43s%-s\n",
			    slot + mod, csetting_table[slot].name, get_custom_color_name(ch, slot));
		}

		set_color(ch, WHITE, NOBOLD);
		return;
	}

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	one_argument(argument, arg2);

	if (arg1.is_number())
		slot = atoi(arg1) - mod;
	else
		for (slot = low; slot < high; slot++) {
			if (csetting_table[slot].name.empty())
				continue;

			if (type == 3
			    && ((slot == CSLOT_CHAN_CLAN   && !ch->clan && !IS_IMMORTAL(ch))
			        || (slot == CSLOT_CHAN_SPOUSE && !ch->pcdata->plr_flags.has(PLR_MARRIED))
			        || (slot == CSLOT_CHAN_IMM    && !IS_IMMORTAL(ch))))
				continue;

			if (arg1.is_prefix_of(csetting_table[slot].name))
				break;
		}

	if (csetting_table[slot].name.empty() || slot > high  || slot < low) {
		ptc(ch, "That is not a valid %s.  Type 'config color %s' for a list.\n",
		    type == 3 ? "channel" : "item", typestr);
		return;
	}

	if (arg2 == "reset") {
		ch->pcdata->color[slot] = 0;
		ch->pcdata->bold [slot] = 0;
		stc("Your setting has been restored to the default color.\n", ch);
		return;
	}

	unsigned int i;
	for (i = 0; i < color_table.size(); i++)
		if (arg2.is_prefix_of(color_table[i].name))
			break;

	if (i >= color_table.size()) {
		stc("That is not a color name.  Type 'ctest' for a list.\n", ch);
		return;
	}

	ch->pcdata->color[slot] = color_table[i].color;
	ch->pcdata->bold [slot] = color_table[i].bold;
	set_color(ch, color_table[i].color, color_table[i].bold);
	stc("Color set to your specification! =)\n", ch);
	set_color(ch, WHITE, NOBOLD);
}

void config_color(Character *ch, String argument)
{
	int argnum = 0;

	if (argument.empty()) {
		stc("Use 'help' or '?' as an argument after any option for details.\n", ch);
		stc("Color options:\n\n", ch);
		ptc(ch, "  1.  Color                                      %s\n",
		    ch->act_flags.has(PLR_COLOR) ? "{CON{x" : "{POFF{x");
		ptc(ch, "  2.  Crazy Color                                %s\n",
		    ch->act_flags.has(PLR_COLOR2) ? "{CON{x" : "{POFF{x");
		stc("  3.  Channels\n", ch);
		stc("  4.  Score\n", ch);
		stc("  5.  Miscellaneous\n", ch);
		stc("  6.  Reset\n", ch);
		return;
	}

	String arg1;
	argument = one_argument(argument, arg1);

	if (arg1.is_prefix_of("help") || arg1 == "?") {
		stc("Config color allows you to specify the what color options you\n"
		    "prefer.  You can toggle color and crazy color on and off, or\n"
		    "add a personal touch to the colors of channels, score items,\n"
		    "and others.  Use without an argument to see a list of options.\n", ch);
		return;
	}

	if (arg1.is_number())                       argnum = atoi(arg1);
	else if (arg1.is_prefix_of("color"))           argnum = 1;
	else if (arg1.is_prefix_of("crazy"))           argnum = 2;
	else if (arg1.is_prefix_of("channels"))        argnum = 3;
	else if (arg1.is_prefix_of("score"))           argnum = 4;
	else if (arg1.is_prefix_of("miscellaneous"))   argnum = 5;
	else if (arg1.is_prefix_of("reset"))           argnum = 6;

	switch (argnum) {
	default:        break;

	case 1: /* color */
		if (argument.is_prefix_of("help") || argument == "?")
			stc("This toggles color on or off.\n", ch);
		else if (argument.empty()) {
			if (ch->act_flags.has(PLR_COLOR)) {
				stc("You see things in ", ch);
				set_color(ch, WHITE, NOBOLD);
				stc("BLACK and ", ch);
				set_color(ch, WHITE, BOLD);
				stc("WHITE.\n", ch);
				set_color(ch, WHITE, NOBOLD);
				ch->act_flags -= PLR_COLOR;
				ch->act_flags -= PLR_COLOR2;
			}
			else {
				ch->act_flags += PLR_COLOR;
				ch->act_flags += PLR_COLOR2;
				stc("{TYou {Hsee {bthings {Nin {Mmany {YC{GO{PL{BO{VR{CS{Y!{x\n", ch);
			}
		}
		else
			stc("Valid options are {Phelp{x and no argument (toggle).\n", ch);

		return;

	case 2: /* crazy */
		if (argument.is_prefix_of("help") || argument == "?")
			stc("This toggles crazy color on or off.  Crazy color is a feature\n"
			    "that allows people to use color codes (found in 'ctest').\n"
			    "Toggling this option determines if the affected test looks\n"
			    "normal or colored to you.\n", ch);
		else if (argument.empty()) {
			if (ch->act_flags.has(PLR_COLOR2)) {
				ch->act_flags -= PLR_COLOR2;
				stc("Crazy Color Disabled =(\n", ch);
			}
			else {
				ch->act_flags += PLR_COLOR2;
				stc("Crazy Color Enabled!!! Yea!! =)\n", ch);
			}
		}
		else
			stc("Valid options are {Phelp{x and no argument (toggle).\n", ch);

		return;

	case 3: /* channels */
	case 4: /* score */
	case 5: /* misc */
		config_color_func(ch, argument, argnum);
		return;

	case 6: /* reset */
		if (!argument.empty())
			stc("This option, without an argument, will reset your personal color\n"
			    "settings to their default scheme.\n", ch);
		else {
			int i;

			for (i = 0; i < MAX_COLORS; i++) {
				ch->pcdata->color[i] = 0;
				ch->pcdata->bold[i] = 0;
			}

			stc("Default colors established.\n", ch);
		}

		return;
	}

	stc("That is not a valid color option.\n", ch);
}

void config_video(Character *ch, String argument)
{
	int argnum = 0;

	if (argument.empty()) {
		stc("Use 'help' or '?' as an argument after any option for details.\n", ch);
		stc("Video options:\n\n", ch);
		ptc(ch, "  1.  Flash                                      %s\n",
		    ch->pcdata->video_flags.has(VIDEO_FLASH_OFF) ?
		    ch->pcdata->video_flags.has(VIDEO_FLASH_LINE) ?
		    "{C{fUNDERLINE{x" : "{POFF{x" : "{C{fON{x");
		ptc(ch, "  2.  Dark                                       %s\n",
		    ch->pcdata->video_flags.has(VIDEO_DARK_MOD) ? "{CON{x" : "{POFF{x");
		ptc(ch, "  3.  Codes                                      %s\n",
		    ch->pcdata->video_flags.has(VIDEO_CODES_SHOW) ? "{CON{x" : "{POFF{x");
		ptc(ch, "  4.  VT100                                      %s\n",
		    ch->pcdata->video_flags.has(VIDEO_VT100) ?  "{CON{x" : "{POFF{x");
		return;
	}

	String arg1;
	argument = one_argument(argument, arg1);

	if (arg1.is_prefix_of("help") || arg1 == "?") {
		stc("With config video, you can set your video options, determining\n"
		    "how flashing text, colors, and other display features are shown\n"
		    "to you.  Use without an argument for a list of options.\n", ch);
		return;
	}

	if (arg1.is_number())                       argnum = atoi(arg1);
	else if (arg1.is_prefix_of("flash"))           argnum = 1;
	else if (arg1.is_prefix_of("dark"))            argnum = 2;
	else if (arg1.is_prefix_of("codes"))           argnum = 3;
	else if (arg1.is_prefix_of("vt100"))           argnum = 4;

	switch (argnum) {
	default:        break;

	case 1: /* FLASH */
		if (argument.is_prefix_of("help") || argument == "?")
			stc("Video flash controls how flashing text will appear on your\n"
			    "screen.  Without an argument, video flash toggles on or off,\n"
			    "but you can use 'video flash underline' to make flashing text\n"
			    "appear as underlined or as reversed video, depending on your\n"
			    "screen emulator.\n", ch);
		else if (argument.is_prefix_of("underline")) {
			if (ch->pcdata->video_flags.has(VIDEO_FLASH_LINE)) {
				ch->pcdata->video_flags -= VIDEO_FLASH_LINE;
				stc("Flashing text will no longer be converted to underlined.\n", ch);
			}
			else {
				ch->pcdata->video_flags += VIDEO_FLASH_OFF;
				ch->pcdata->video_flags += VIDEO_FLASH_LINE;
				stc("Flashing text should now look {funderlined{x to you!\n", ch);
			}
		}
		else if (argument.empty()) {
			if (ch->pcdata->video_flags.has(VIDEO_FLASH_LINE)) {
				ch->pcdata->video_flags -= VIDEO_FLASH_LINE;
				stc("Flashing text will no longer be converted to underlined.\n", ch);
			}
			else if (ch->pcdata->video_flags.has(VIDEO_FLASH_OFF)) {
				ch->pcdata->video_flags -= VIDEO_FLASH_OFF;
				stc("You can now see {fflashing{x text!\n", ch);
			}
			else {
				ch->pcdata->video_flags += VIDEO_FLASH_OFF;
				stc("Flashing text now holds still for you.\n", ch);
			}
		}
		else
			stc("Valid options are {Phelp{x, {Punderline{x, and no argument (toggle).\n", ch);

		return;

	case 2:
		if (argument.is_prefix_of("help") || argument == "?")
			stc("Video dark toggles whether or not charcoal and black text are\n"
			    "converted to grey.", ch);
		/* backwards compatible with old video command */
		else if (argument.is_prefix_of("modify")
		         || (argument.empty() && !ch->pcdata->video_flags.has(VIDEO_DARK_MOD))) {
			ch->pcdata->video_flags += VIDEO_DARK_MOD;
			stc("You can now see {ccharcoal{x and {kblack{x!\n", ch);
		}
		else if (argument.is_prefix_of("normal")
		         || (argument.empty() && ch->pcdata->video_flags.has(VIDEO_DARK_MOD))) {
			ch->pcdata->video_flags -= VIDEO_DARK_MOD;
			stc("Can you still see {ccharcoal{x and {kblack{x?\n", ch);
		}
		else
			stc("Valid options are {Phelp{x and no argument (toggle).\n", ch);

		return;

	case 3:
		if (argument.is_prefix_of("help") || argument == "?")
			stc("Video codes toggles whether color codes ({{) are shown ({CON{x)\n"
			    "or converted into colors ({POFF{x).\n", ch);
		/* backwards compatible with old video command */
		else if (argument.is_prefix_of("show")
		         || (argument.empty() && !ch->pcdata->video_flags.has(VIDEO_CODES_SHOW))) {
			ch->pcdata->video_flags += VIDEO_CODES_SHOW;
			stc("{RColor{x {Ccodes{x will now be shown.\n", ch);
		}
		else if (argument.is_prefix_of("hide")
		         || (argument.empty() && ch->pcdata->video_flags.has(VIDEO_CODES_SHOW))) {
			ch->pcdata->video_flags -= VIDEO_CODES_SHOW;
			stc("You see no more {Rcolor{x {Ccodes{x.\n", ch);
		}
		else
			stc("Valid options are {Phelp{x and no argument (toggle).\n", ch);

		return;

	case 4:
		if (argument.is_prefix_of("help") || argument == "?")
			stc("Video VT100 emulation mode looks terrible if you do not have\n"
			    "a VT100, so only use it if you must.\n", ch);
		else if (argument.empty()) {
			if (ch->pcdata->video_flags.has(VIDEO_VT100)) {
				ch->pcdata->video_flags -= VIDEO_VT100;
				stc("VT100 mode off.\n", ch);
			}
			else {
				stc("VT100 mode on.  Type {Rconfig video vt100{x if your screen goes screwy.\n", ch);
				ch->pcdata->video_flags += VIDEO_VT100;
			}
		}
		else
			stc("Valid options are {Phelp{x and no argument (toggle).\n", ch);

		return;
	}

	stc("That is not a valid video option.\n", ch);
}

void config_censor(Character *ch, String argument)
{
	int argnum = 0;

	if (argument.empty()) {
		stc("Use 'help' or '?' as an argument after any option for details.\n", ch);
		stc("Censor options:\n\n", ch);
		ptc(ch, "  1.  Channels                                   %s\n",
		    ch->censor_flags.has(CENSOR_CHAN) ?  "{CON{x" : "{POFF{x");
		ptc(ch, "  2.  Spam                                       %s\n",
		    ch->censor_flags.has(CENSOR_SPAM) ?  "{CON{x" : "{POFF{x");
		return;
	}

	String arg1;
	argument = one_argument(argument, arg1);

	if (arg1.is_prefix_of("help") || arg1 == "?") {
		stc("Censor allows players to specify their censorship settings, to\n"
		    "determine what text the mud will not show them.  Use without an\n"
		    "argument to see a lit of options.\n", ch);
		return;
	}

	if (arg1.is_number())                       argnum = atoi(arg1);
	else if (arg1.is_prefix_of("channels"))        argnum = 1;
	else if (arg1.is_prefix_of("spam"))            argnum = 2;

	switch (argnum) {
	default:        break;

	case 1: /* CHANNELS */
		if (argument.is_prefix_of("help") || argument == "?")
			stc("Censor channels filters channel communications that have a word\n"
			    "in them that appears on the censor list.  Note that gossip,\n"
			    "socials, emotes, tells, says, heeds, and notes are NOT censored\n"
			    "and therefore are not suitable for any vulgarities.  Please type\n"
			    "HELP CENSOR to for more information, or use LIST as an option\n"
			    "rather than help to see what words are censored.  Turning on this\n"
			    "option allows you to see and use the FLAME channel.\n", ch);
		else if (argument.is_prefix_of("list")) {
			char *const swear [] = {
				"shit",         "fuck",         "bitch",        "bastard",
				"bullshit",     "pussy",        "dick",         "cock",
				"motherfuck",   "clit",         "damn",         "dammit",
				"dumbass",      "slut",         "whore",        "ass",
				nullptr
			};
			int x;

			for (x = 0; swear[x] != nullptr; x++) {
				ptc(ch, "%15s", swear[x]);

				if (x % 4 == 3)
					stc("\n", ch);
			}

			if (x % 4 != 3)
				stc("\n", ch);
		}
		else if (argument.empty()) {
			if (ch->censor_flags.has(CENSOR_CHAN)) {
				ch->censor_flags -= CENSOR_CHAN;
				stc("{BL{Ce{gg{Wa{Cc{By{x is now rated {PR{x.\n", ch);
			}
			else {
				ch->censor_flags += CENSOR_CHAN;
				stc("{BL{Ce{gg{Wa{Cc{By{x is now rated {GPG{x.\n", ch);
			}
		}
		else
			stc("Valid options are {Phelp{x, {Plist{x, and no argument (toggle).\n", ch);

		return;

	case 2: /* SPAM */
		if (argument.is_prefix_of("help") || argument == "?")
			stc("The spam censor toggles whether or not you will see notes\n"
			    "addressed to 'spam'.  Swearing censors still apply to spam\n"
			    "notes, no vulgarities, racial, sexual, or ethnic slurs are\n"
			    "allowed under any circumstance.\n", ch);
		else if (argument.empty()) {
			if (ch->censor_flags.has(CENSOR_SPAM)) {
				ch->censor_flags -= CENSOR_SPAM;
				stc("{BL{Ce{gg{Wa{Cc{By{x is now {GSPAMMY{x!\n", ch);
			}
			else {
				ch->censor_flags += CENSOR_SPAM;
				stc("{BL{Ce{gg{Wa{Cc{By{x is set to {YLow Fat SPAM{x.\n", ch);
			}
		}
		else
			stc("Valid options are {Phelp{x and no argument (toggle).\n", ch);

		return;
	}

	stc("That is not a valid censor option.\n", ch);
}

void config_immortal(Character *ch, String argument)
{
	int argnum = 0;

	if (argument.empty()) {
		stc("Use 'help' or '?' as an argument after any option for details.\n", ch);
		stc("Immortal options:\n\n", ch);
		ptc(ch, "  1.  Immprefix                                  %s\n",
		    !ch->pcdata->immprefix.empty() ? ch->pcdata->immprefix : "(none)");
		ptc(ch, "  2.  Immname                                    %s\n",
		    !ch->pcdata->immname.empty() ? ch->pcdata->immname : "(none)");
		return;
	}

	String arg1;
	argument = one_argument(argument, arg1);

	if (arg1.is_prefix_of("help") || arg1 == "?") {
		stc("Various configuration options are available only to the Legacy\n"
		    "staff.  Use without an argument to see a lit of options.\n", ch);
		return;
	}

	if (arg1.is_number())                       argnum = atoi(arg1);
	else if (arg1.is_prefix_of("immprefix"))        argnum = 1;
	else if (arg1.is_prefix_of("immname"))            argnum = 2;

	switch (argnum) {
	default:        break;

	case 1: /* Immprefix */
		if (argument == "help" || argument == "?")
			stc("Your immprefix is what appears at the beginning of each immtalk\n"
				"line.  It must include your name.  Use 'none' for the default.\n", ch);
		else if (argument.empty())
			ptc(ch, "Your immtalk prefix is currently: %s{x\n",
				!ch->pcdata->immprefix.empty() ? ch->pcdata->immprefix : "(none)");
		else if (argument == "none") {
			ch->pcdata->immprefix.erase();
			stc("Your immtalk prefix has been removed.\n", ch);
		}
		else {
			char buf[MIL];
			strcpy(buf, argument.uncolor());

			if (strlen(buf) > 30)
				stc("Your immtalk prefix can be no longer than 30 printed characters.\n", ch);
			else if (!strstr(buf, ch->name))
				stc("Your immtalk prefix must include your name.\n", ch);
			else {
				ch->pcdata->immprefix = argument;
				ptc(ch, "Your immtalk prefix is now: %s{x\n", ch->pcdata->immprefix);
			}
		}

		return;

	case 2: /* Immname */
		if (argument == "help" || argument == "?")
			stc("Your immname is what appears in the WHO list in place of a player's\n"
				"race, class, and level.  Use 'none' for the default.\n", ch);
		else if (argument.empty())
			ptc(ch, "Your immname is currently: %s{x\n",
				!ch->pcdata->immname.empty() ? ch->pcdata->immname : "(none)");
		else if (argument == "none") {
			ch->pcdata->immname.erase();
			stc("Your immname has been removed.\n", ch);
		}
		else {
			if (strlen(argument.uncolor()) > 15)
				ptc(ch, "Your immname can be no longer than %d printed characters.\n", 15);
			else {
				ch->pcdata->immname = argument;
				ptc(ch, "Your immname is now: %s{x\n", ch->pcdata->immname);
			}
		}

		return;
	}

	stc("That is not a valid immortal configuration option.\n", ch);
}

void config_wiznet(Character *ch, String argument)
{
	int argnum = -1;

	String arg1;
	argument = one_argument(argument, arg1);

	if (arg1.is_prefix_of("help") || arg1 == "?") {
		stc("This allows you to specify your wiznet options.  Turning options\n"
		    "on means that wiznet will report the specified field to you.\n"
		    "Use without an argument to see a list of wiznet flags.\n", ch);
		return;
	}

	/* hack so you can type config wiznet on or off */
	if (arg1 == "on") {
		stc("Welcome to Wiznet!\n", ch);
		ch->wiznet_flags += WIZ_ON;
		return;
	}
	else if (arg1 == "off") {
		stc("You have now signed off Wiznet.\n", ch);
		ch->wiznet_flags -= WIZ_ON;
		return;
	}

	if (arg1.empty() || arg1.is_prefix_of("status")) {
		stc("Use 'help' or '?' as an argument after any option for details.\n", ch);
		stc("Special for Wiznet: {Con{x, {Coff{x, {Cstatus{x, {Cshow{x\n", ch);
		stc("Wiznet options:\n\n", ch);

		for (unsigned int flag = 0; flag < wiznet_table.size(); flag++)
			if (GET_RANK(ch) >= wiznet_table[flag].level)
				ptc(ch, "  %2d.  %-15s({Y%4s{x)                        %s\n",
				    flag, wiznet_table[flag].name.capitalize(),
				    wiznet_table[flag].level == IMM ? "Imm" :
				    wiznet_table[flag].level == HED ? "Head" : "Imp",
				    ch->wiznet_flags.has(wiznet_table[flag].flag) ?
				    "{CON{x" : "{POFF{x");

		return;
	}

	/* old school 'show' option */
	if (arg1.is_prefix_of("show")) {
		stc("Use 'help' or '?' as an argument after any option for details.\n", ch);
		stc("Special for Wiznet: {Con{x, {Coff{x, {Cstatus{x, {Cshow{x\n", ch);
		stc("Wiznet options:\n\n", ch);

		for (const auto& entry : wiznet_table)
			if (GET_RANK(ch) >= entry.level)
				ptc(ch, "%-10s - %s\n", entry.name, entry.desc);

		return;
	}

	if (arg1.is_number()) {
		if ((unsigned int)atoi(arg1) < wiznet_table.size())
			argnum = atoi(arg1);
	}
	else
		for (unsigned int flag = 0; flag < wiznet_table.size(); flag++)
			if (arg1.is_prefix_of(wiznet_table[flag].name))
				argnum = flag;

	if (argnum >= 0)
		if (GET_RANK(ch) < wiznet_table[argnum].level)
			argnum = -1;

	if (argnum < 0) {
		stc("That is not a valid wiznet option.\n", ch);
		return;
	}

	if (argument.is_prefix_of("help") || argument == "?")
		ptc(ch, "%s\n", wiznet_table[argnum].desc);
	else if (argument.empty()) {
		if (argnum == 0) {      /* Wiznet ON */
			if (ch->wiznet_flags.has(wiznet_table[argnum].flag)) {
				stc("You have now signed off Wiznet.\n", ch);
				ch->wiznet_flags -= wiznet_table[argnum].flag;
			}
			else {
				stc("Welcome to Wiznet!\n", ch);
				ch->wiznet_flags += wiznet_table[argnum].flag;
			}
		}
		else {
			if (ch->wiznet_flags.has(wiznet_table[argnum].flag)) {
				ch->wiznet_flags -= wiznet_table[argnum].flag;
				ptc(ch, "Wiznet will no longer provide you with: %s\n", wiznet_table[argnum].name);
			}
			else {
				ch->wiznet_flags += wiznet_table[argnum].flag;
				ptc(ch, "Wiznet will now provide you with: %s\n", wiznet_table[argnum].name);
			}
		}
	}
	else
		stc("Valid options are {Phelp{x and no argument (toggle).\n", ch);
}

void do_config(Character *ch, String argument)
{
	int argnum = 0;

	if (argument.empty()) {
		stc("Configuration groups:\n\n", ch);
//		stc("  1.  Channels\n", ch);

		if (!ch->is_npc()) {
			stc("  5.  Color\n", ch);
			stc("  6.  Video\n", ch);
		}

		stc("  7.  Censor\n", ch);

		if (IS_IMMORTAL(ch)) {
			stc("  8.  Immortal\n", ch);
			stc("  9.  Wiznet\n", ch);
		}

		return;
	}

	String arg1;
	argument = one_argument(argument, arg1);

	if (arg1.is_number())                       argnum = atoi(arg1);
	else if (arg1.is_prefix_of("channels"))        argnum = 1;
	else if (!ch->is_npc() &&          arg1.is_prefix_of("color"))           argnum = 5;
	else if (!ch->is_npc() &&          arg1.is_prefix_of("video"))           argnum = 6;
	else if (arg1.is_prefix_of("censor"))          argnum = 7;
	else if (IS_IMMORTAL(ch) &&      arg1.is_prefix_of("immortal"))        argnum = 8;
	else if (IS_IMMORTAL(ch) &&      arg1.is_prefix_of("wiznet"))          argnum = 9;

	switch (argnum) {
	default:                                        break;

//		case 1:     config_channels(ch, argument);  return;
	case 5: config_color(ch, argument);  return;

	case 6: config_video(ch, argument);  return;

	case 7: config_censor(ch, argument);  return;

	case 8: config_immortal(ch, argument);  return;
	case 9: config_wiznet(ch, argument);  return;
	}

	stc("That is not a valid configuration option.\n", ch);
}

/*****
 Replaced configuration commands, left for oldbies
 *****/

void do_color(Character *ch, String argument)
{
	char buf[MIL];

	if (ch->is_npc()) {
		stc("Mobiles can't see colors. :)\n", ch);
		return;
	}

	if (argument.empty())
		do_config(ch, "color color");
	else {
		Format::sprintf(buf, "color %s", argument);
		do_config(ch, buf);
	}
}

void do_video(Character *ch, String argument)
{
	char buf[MIL];

	if (ch->is_npc()) {
		stc("Mobiles can't see colors anyway. :)\n", ch);
		return;
	}

	Format::sprintf(buf, "video %s", argument);
	do_config(ch, buf);
}

void do_censor(Character *ch, String argument)
{
	char buf[MIL];
	Format::sprintf(buf, "censor %s", argument);
	do_config(ch, buf);
}

void do_wiznet(Character *ch, String argument)
{
	char buf[MIL];

	if (ch->is_npc()) {
		stc("Tiny mobile brains would overload with the power of Wiznet.\n", ch);
		return;
	}

	if (argument.empty())
		do_config(ch, "wiznet wiznet");
	else {
		Format::sprintf(buf, "wiznet %s", argument);
		do_config(ch, buf);
	}
}
