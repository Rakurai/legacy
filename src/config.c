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
#include "tables.h"
#include "Format.hpp"

/* TEMPLATE
void config_wiznet(CHAR_DATA *ch, const char *argument)
{
        char arg1[MIL];
        int argnum = 0;

        argument = one_argument(argument, arg1);

        if (arg1.empty())
        {
                stc("Use 'help' or '?' as an argument after any option for details.\n", ch);
                stc("Censor options:\n\n", ch);
                ptc(ch, "  1.  Channels                                   %s\n",
                        IS_SET(ch->censor, CENSOR_CHAN) ?  "{CON{x" : "{POFF{x");
                return;
        }

             if (is_number(arg1))                       argnum = atoi(arg1);
        else if (!str_prefix1(arg1, "channels"))        argnum = 1;
        else if (!str_prefix1(arg1, "spam"))            argnum = 3;

        switch(argnum)
        {
                default:        break;

                case 1:
                        return;
        }

        stc("That is not a valid censor option.\n", ch);
}*/

void config_color_func(CHAR_DATA *ch, String argument, int type)
{
	char typestr[20];
	int low, high, mod, slot, i;

	switch (type) {
	case 3: Format::sprintf(typestr, "channel");            low = 0; high = 29; mod = 1;    break;
	case 4: Format::sprintf(typestr, "score");              low = 50; high = 79; mod = -49;	break;
	case 5: Format::sprintf(typestr, "miscellaneous");      low = 80; high = 99; mod = -79; break;
	default:
		bug("config_color_func: bad type %d", type);
		return;
	}

	if (argument.empty()) {
		stc("Use the color name or 'reset' after an option to modify.\n", ch);
		ptc(ch, "%s color settings:\n\n", capitalize(typestr));

		for (slot = low; slot < high; slot++) {
			if (csetting_table[slot].name == NULL)
				continue;

			if (type == 3
			    && ((slot == CSLOT_CHAN_CLAN   && !ch->clan && !IS_IMMORTAL(ch))
			        || (slot == CSLOT_CHAN_SPOUSE && !IS_SET(ch->pcdata->plr, PLR_MARRIED))
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

	if (is_number(arg1))
		slot = atoi(arg1) - mod;
	else
		for (slot = low; slot < high; slot++) {
			if (csetting_table[slot].name == NULL)
				continue;

			if (type == 3
			    && ((slot == CSLOT_CHAN_CLAN   && !ch->clan && !IS_IMMORTAL(ch))
			        || (slot == CSLOT_CHAN_SPOUSE && !IS_SET(ch->pcdata->plr, PLR_MARRIED))
			        || (slot == CSLOT_CHAN_IMM    && !IS_IMMORTAL(ch))))
				continue;

			if (!str_prefix1(arg1, csetting_table[slot].name))
				break;
		}

	if (csetting_table[slot].name == NULL || slot > high  || slot < low) {
		ptc(ch, "That is not a valid %s.  Type 'config color %s' for a list.\n",
		    type == 3 ? "channel" : "item", typestr);
		return;
	}

	if (!str_cmp(arg2, "reset")) {
		ch->pcdata->color[slot] = 0;
		ch->pcdata->bold [slot] = 0;
		stc("Your setting has been restored to the default color.\n", ch);
		return;
	}

	for (i = 0; color_table[i].name != NULL; i++)
		if (!str_prefix(arg2, color_table[i].name))
			break;

	if (color_table[i].name == NULL) {
		stc("That is not a color name.  Type 'ctest' for a list.\n", ch);
		return;
	}

	ch->pcdata->color[slot] = color_table[i].color;
	ch->pcdata->bold [slot] = color_table[i].bold;
	set_color(ch, color_table[i].color, color_table[i].bold);
	stc("Color set to your specification! =)\n", ch);
	set_color(ch, WHITE, NOBOLD);
}

void config_color(CHAR_DATA *ch, String argument)
{
	int argnum = 0;

	if (argument.empty()) {
		stc("Use 'help' or '?' as an argument after any option for details.\n", ch);
		stc("Color options:\n\n", ch);
		ptc(ch, "  1.  Color                                      %s\n",
		    IS_SET(ch->act, PLR_COLOR) ? "{CON{x" : "{POFF{x");
		ptc(ch, "  2.  Crazy Color                                %s\n",
		    IS_SET(ch->act, PLR_COLOR2) ? "{CON{x" : "{POFF{x");
		stc("  3.  Channels\n", ch);
		stc("  4.  Score\n", ch);
		stc("  5.  Miscellaneous\n", ch);
		stc("  6.  Reset\n", ch);
		return;
	}

	String arg1;
	argument = one_argument(argument, arg1);

	if (!str_prefix1(arg1, "help") || !str_cmp(arg1, "?")) {
		stc("Config color allows you to specify the what color options you\n"
		    "prefer.  You can toggle color and crazy color on and off, or\n"
		    "add a personal touch to the colors of channels, score items,\n"
		    "and others.  Use without an argument to see a list of options.\n", ch);
		return;
	}

	if (is_number(arg1))                       argnum = atoi(arg1);
	else if (!str_prefix1(arg1, "color"))           argnum = 1;
	else if (!str_prefix1(arg1, "crazy"))           argnum = 2;
	else if (!str_prefix1(arg1, "channels"))        argnum = 3;
	else if (!str_prefix1(arg1, "score"))           argnum = 4;
	else if (!str_prefix1(arg1, "miscellaneous"))   argnum = 5;
	else if (!str_prefix1(arg1, "reset"))           argnum = 6;

	switch (argnum) {
	default:        break;

	case 1: /* color */
		if (!str_prefix1(argument, "help") || !str_cmp(argument, "?"))
			stc("This toggles color on or off.\n", ch);
		else if (argument.empty()) {
			if (IS_SET(ch->act, PLR_COLOR)) {
				stc("You see things in ", ch);
				set_color(ch, WHITE, NOBOLD);
				stc("BLACK and ", ch);
				set_color(ch, WHITE, BOLD);
				stc("WHITE.\n", ch);
				set_color(ch, WHITE, NOBOLD);
				REMOVE_BIT(ch->act, PLR_COLOR);
				REMOVE_BIT(ch->act, PLR_COLOR2);
			}
			else {
				SET_BIT(ch->act, PLR_COLOR);
				SET_BIT(ch->act, PLR_COLOR2);
				stc("{TYou {Hsee {bthings {Nin {Mmany {YC{GO{PL{BO{VR{CS{Y!{x\n", ch);
			}
		}
		else
			stc("Valid options are {Phelp{x and no argument (toggle).\n", ch);

		return;

	case 2: /* crazy */
		if (!str_prefix1(argument, "help") || !str_cmp(argument, "?"))
			stc("This toggles crazy color on or off.  Crazy color is a feature\n"
			    "that allows people to use color codes (found in 'ctest').\n"
			    "Toggling this option determines if the affected test looks\n"
			    "normal or colored to you.\n", ch);
		else if (argument.empty()) {
			if (IS_SET(ch->act, PLR_COLOR2)) {
				REMOVE_BIT(ch->act, PLR_COLOR2);
				stc("Crazy Color Disabled =(\n", ch);
			}
			else {
				SET_BIT(ch->act, PLR_COLOR2);
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

void config_video(CHAR_DATA *ch, String argument)
{
	int argnum = 0;

	if (argument.empty()) {
		stc("Use 'help' or '?' as an argument after any option for details.\n", ch);
		stc("Video options:\n\n", ch);
		ptc(ch, "  1.  Flash                                      %s\n",
		    IS_SET(ch->pcdata->video, VIDEO_FLASH_OFF) ?
		    IS_SET(ch->pcdata->video, VIDEO_FLASH_LINE) ?
		    "{C{fUNDERLINE{x" : "{POFF{x" : "{C{fON{x");
		ptc(ch, "  2.  Dark                                       %s\n",
		    IS_SET(ch->pcdata->video, VIDEO_DARK_MOD) ? "{CON{x" : "{POFF{x");
		ptc(ch, "  3.  Codes                                      %s\n",
		    IS_SET(ch->pcdata->video, VIDEO_CODES_SHOW) ? "{CON{x" : "{POFF{x");
		ptc(ch, "  4.  VT100                                      %s\n",
		    IS_SET(ch->pcdata->video, VIDEO_VT100) ?  "{CON{x" : "{POFF{x");
		return;
	}

	String arg1;
	argument = one_argument(argument, arg1);

	if (!str_prefix1(arg1, "help") || !str_cmp(arg1, "?")) {
		stc("With config video, you can set your video options, determining\n"
		    "how flashing text, colors, and other display features are shown\n"
		    "to you.  Use without an argument for a list of options.\n", ch);
		return;
	}

	if (is_number(arg1))                       argnum = atoi(arg1);
	else if (!str_prefix1(arg1, "flash"))           argnum = 1;
	else if (!str_prefix1(arg1, "dark"))            argnum = 2;
	else if (!str_prefix1(arg1, "codes"))           argnum = 3;
	else if (!str_prefix1(arg1, "vt100"))           argnum = 4;

	switch (argnum) {
	default:        break;

	case 1: /* FLASH */
		if (!str_prefix1(argument, "help") || !str_cmp(argument, "?"))
			stc("Video flash controls how flashing text will appear on your\n"
			    "screen.  Without an argument, video flash toggles on or off,\n"
			    "but you can use 'video flash underline' to make flashing text\n"
			    "appear as underlined or as reversed video, depending on your\n"
			    "screen emulator.\n", ch);
		else if (!str_prefix1(argument, "underline")) {
			if (IS_SET(ch->pcdata->video, VIDEO_FLASH_LINE)) {
				REMOVE_BIT(ch->pcdata->video, VIDEO_FLASH_LINE);
				stc("Flashing text will no longer be converted to underlined.\n", ch);
			}
			else {
				SET_BIT(ch->pcdata->video, VIDEO_FLASH_OFF);
				SET_BIT(ch->pcdata->video, VIDEO_FLASH_LINE);
				stc("Flashing text should now look {funderlined{x to you!\n", ch);
			}
		}
		else if (argument.empty()) {
			if (IS_SET(ch->pcdata->video, VIDEO_FLASH_LINE)) {
				REMOVE_BIT(ch->pcdata->video, VIDEO_FLASH_LINE);
				stc("Flashing text will no longer be converted to underlined.\n", ch);
			}
			else if (IS_SET(ch->pcdata->video, VIDEO_FLASH_OFF)) {
				REMOVE_BIT(ch->pcdata->video, VIDEO_FLASH_OFF);
				stc("You can now see {fflashing{x text!\n", ch);
			}
			else {
				SET_BIT(ch->pcdata->video, VIDEO_FLASH_OFF);
				stc("Flashing text now holds still for you.\n", ch);
			}
		}
		else
			stc("Valid options are {Phelp{x, {Punderline{x, and no argument (toggle).\n", ch);

		return;

	case 2:
		if (!str_prefix1(argument, "help") || !str_cmp(argument, "?"))
			stc("Video dark toggles whether or not charcoal and black text are\n"
			    "converted to grey.", ch);
		/* backwards compatible with old video command */
		else if (!str_prefix1(argument, "modify")
		         || (argument.empty() && !IS_SET(ch->pcdata->video, VIDEO_DARK_MOD))) {
			SET_BIT(ch->pcdata->video, VIDEO_DARK_MOD);
			stc("You can now see {ccharcoal{x and {kblack{x!\n", ch);
		}
		else if (!str_prefix1(argument, "normal")
		         || (argument.empty() && IS_SET(ch->pcdata->video, VIDEO_DARK_MOD))) {
			REMOVE_BIT(ch->pcdata->video, VIDEO_DARK_MOD);
			stc("Can you still see {ccharcoal{x and {kblack{x?\n", ch);
		}
		else
			stc("Valid options are {Phelp{x and no argument (toggle).\n", ch);

		return;

	case 3:
		if (!str_prefix1(argument, "help") || !str_cmp(argument, "?"))
			stc("Video codes toggles whether color codes ({{) are shown ({CON{x)\n"
			    "or converted into colors ({POFF{x).\n", ch);
		/* backwards compatible with old video command */
		else if (!str_prefix1(argument, "show")
		         || (argument.empty() && !IS_SET(ch->pcdata->video, VIDEO_CODES_SHOW))) {
			SET_BIT(ch->pcdata->video, VIDEO_CODES_SHOW);
			stc("{RColor{x {Ccodes{x will now be shown.\n", ch);
		}
		else if (!str_prefix1(argument, "hide")
		         || (argument.empty() && IS_SET(ch->pcdata->video, VIDEO_CODES_SHOW))) {
			REMOVE_BIT(ch->pcdata->video, VIDEO_CODES_SHOW);
			stc("You see no more {Rcolor{x {Ccodes{x.\n", ch);
		}
		else
			stc("Valid options are {Phelp{x and no argument (toggle).\n", ch);

		return;

	case 4:
		if (!str_prefix1(argument, "help") || !str_cmp(argument, "?"))
			stc("Video VT100 emulation mode looks terrible if you do not have\n"
			    "a VT100, so only use it if you must.\n", ch);
		else if (argument.empty()) {
			if (IS_SET(ch->pcdata->video, VIDEO_VT100)) {
				REMOVE_BIT(ch->pcdata->video, VIDEO_VT100);
				stc("VT100 mode off.\n", ch);
			}
			else {
				stc("VT100 mode on.  Type {Rconfig video vt100{x if your screen goes screwy.\n", ch);
				SET_BIT(ch->pcdata->video, VIDEO_VT100);
			}
		}
		else
			stc("Valid options are {Phelp{x and no argument (toggle).\n", ch);

		return;
	}

	stc("That is not a valid video option.\n", ch);
}

void config_censor(CHAR_DATA *ch, String argument)
{
	int argnum = 0;

	if (argument.empty()) {
		stc("Use 'help' or '?' as an argument after any option for details.\n", ch);
		stc("Censor options:\n\n", ch);
		ptc(ch, "  1.  Channels                                   %s\n",
		    IS_SET(ch->censor, CENSOR_CHAN) ?  "{CON{x" : "{POFF{x");
		ptc(ch, "  2.  Spam                                       %s\n",
		    IS_SET(ch->censor, CENSOR_SPAM) ?  "{CON{x" : "{POFF{x");
		return;
	}

	String arg1;
	argument = one_argument(argument, arg1);

	if (!str_prefix1(arg1, "help") || !str_cmp(arg1, "?")) {
		stc("Censor allows players to specify their censorship settings, to\n"
		    "determine what text the mud will not show them.  Use without an\n"
		    "argument to see a lit of options.\n", ch);
		return;
	}

	if (is_number(arg1))                       argnum = atoi(arg1);
	else if (!str_prefix1(arg1, "channels"))        argnum = 1;
	else if (!str_prefix1(arg1, "spam"))            argnum = 2;

	switch (argnum) {
	default:        break;

	case 1: /* CHANNELS */
		if (!str_prefix1(argument, "help") || !str_cmp(argument, "?"))
			stc("Censor channels filters channel communications that have a word\n"
			    "in them that appears on the censor list.  Note that gossip,\n"
			    "socials, emotes, tells, says, heeds, and notes are NOT censored\n"
			    "and therefore are not suitable for any vulgarities.  Please type\n"
			    "HELP CENSOR to for more information, or use LIST as an option\n"
			    "rather than help to see what words are censored.  Turning on this\n"
			    "option allows you to see and use the FLAME channel.\n", ch);
		else if (!str_prefix1(argument, "list")) {
			char *const swear [] = {
				"shit",         "fuck",         "bitch",        "bastard",
				"bullshit",     "pussy",        "dick",         "cock",
				"motherfuck",   "clit",         "damn",         "dammit",
				"dumbass",      "slut",         "whore",        "ass",
				NULL
			};
			int x;

			for (x = 0; swear[x] != NULL; x++) {
				ptc(ch, "%15s", swear[x]);

				if (x % 4 == 3)
					stc("\n", ch);
			}

			if (x % 4 != 3)
				stc("\n", ch);
		}
		else if (argument.empty()) {
			if (IS_SET(ch->censor, CENSOR_CHAN)) {
				REMOVE_BIT(ch->censor, CENSOR_CHAN);
				stc("{BL{Ce{gg{Wa{Cc{By{x is now rated {PR{x.\n", ch);
			}
			else {
				SET_BIT(ch->censor, CENSOR_CHAN);
				stc("{BL{Ce{gg{Wa{Cc{By{x is now rated {GPG{x.\n", ch);
			}
		}
		else
			stc("Valid options are {Phelp{x, {Plist{x, and no argument (toggle).\n", ch);

		return;

	case 2: /* SPAM */
		if (!str_prefix1(argument, "help") || !str_cmp(argument, "?"))
			stc("The spam censor toggles whether or not you will see notes\n"
			    "addressed to 'spam'.  Swearing censors still apply to spam\n"
			    "notes, no vulgarities, racial, sexual, or ethnic slurs are\n"
			    "allowed under any circumstance.\n", ch);
		else if (argument.empty()) {
			if (IS_SET(ch->censor, CENSOR_SPAM)) {
				REMOVE_BIT(ch->censor, CENSOR_SPAM);
				stc("{BL{Ce{gg{Wa{Cc{By{x is now {GSPAMMY{x!\n", ch);
			}
			else {
				SET_BIT(ch->censor, CENSOR_SPAM);
				stc("{BL{Ce{gg{Wa{Cc{By{x is set to {YLow Fat SPAM{x.\n", ch);
			}
		}
		else
			stc("Valid options are {Phelp{x and no argument (toggle).\n", ch);

		return;
	}

	stc("That is not a valid censor option.\n", ch);
}

void config_immortal(CHAR_DATA *ch, String argument)
{
	int argnum = 0;

	if (argument.empty()) {
		stc("Use 'help' or '?' as an argument after any option for details.\n", ch);
		stc("Immortal options:\n\n", ch);
		ptc(ch, "  1.  Immprefix                                  %s\n",
		    ch->pcdata->immprefix[0] != '\0' ? ch->pcdata->immprefix : "(none)");
		ptc(ch, "  2.  Immname                                    %s\n",
		    ch->pcdata->immname[0] != '\0' ? ch->pcdata->immname : "(none)");
		return;
	}

	String arg1;
	argument = one_argument(argument, arg1);

	if (!str_prefix1(arg1, "help") || !str_cmp(arg1, "?")) {
		stc("Various configuration options are available only to the Legacy\n"
		    "staff.  Use without an argument to see a lit of options.\n", ch);
		return;
	}

	if (is_number(arg1))                       argnum = atoi(arg1);
	else if (!str_prefix1(arg1, "immprefix"))        argnum = 1;
	else if (!str_prefix1(arg1, "immname"))            argnum = 2;

	switch (argnum) {
	default:        break;

	case 1: /* Immprefix */
		if (!str_cmp(argument, "help") || !str_cmp(argument, "?"))
			stc("Your immprefix is what appears at the beginning of each immtalk\n"
				"line.  It must include your name.  Use 'none' for the default.\n", ch);
		else if (argument.empty())
			ptc(ch, "Your immtalk prefix is currently: %s{x\n",
				ch->pcdata->immprefix[0] != '\0' ? ch->pcdata->immprefix : "(none)");
		else if (!str_cmp(argument, "none")) {
			free_string(ch->pcdata->immprefix);
			ch->pcdata->immprefix = str_dup("");
			stc("Your immtalk prefix has been removed.\n", ch);
		}
		else {
			char buf[MIL];
			strcpy(buf, smash_bracket(argument));

			if (strlen(buf) > 30)
				stc("Your immtalk prefix can be no longer than 30 printed characters.\n", ch);
			else if (!strstr(buf, ch->name))
				stc("Your immtalk prefix must include your name.\n", ch);
			else {
				free_string(ch->pcdata->immprefix);
				ch->pcdata->immprefix = str_dup(argument);
				ptc(ch, "Your immtalk prefix is now: %s{x\n", ch->pcdata->immprefix);
			}
		}

		return;

	case 2: /* Immname */
		if (!str_cmp(argument, "help") || !str_cmp(argument, "?"))
			stc("Your immname is what appears in the WHO list in place of a player's\n"
				"race, class, and level.  Use 'none' for the default.\n", ch);
		else if (argument.empty())
			ptc(ch, "Your immname is currently: %s{x\n",
				ch->pcdata->immname[0] != '\0' ? ch->pcdata->immname : "(none)");
		else if (!str_cmp(argument, "none")) {
			free_string(ch->pcdata->immprefix);
			ch->pcdata->immname = str_dup("");
			stc("Your immname has been removed.\n", ch);
		}
		else {
			String buf = smash_bracket(argument);

			if (strlen(buf) > TITLEBLOCK)
				stc("Your immname can be no longer than 13 printed characters.\n", ch);
			else {
				free_string(ch->pcdata->immname);
				buf[0]  = '\0';
				buf += "{W[{x";
				buf += center_string_in_whitespace(argument, TITLEBLOCK);
				buf += "{W]{x";
				ch->pcdata->immname = str_dup(buf);
				ptc(ch, "Your immname is now: %s{x\n", ch->pcdata->immname);
			}
		}

		return;
	}

	stc("That is not a valid immortal configuration option.\n", ch);
}

void config_wiznet(CHAR_DATA *ch, String argument)
{
	int argnum = -1;
	int flag;

	String arg1;
	argument = one_argument(argument, arg1);

	if (!str_prefix1(arg1, "help") || !str_cmp(arg1, "?")) {
		stc("This allows you to specify your wiznet options.  Turning options\n"
		    "on means that wiznet will report the specified field to you.\n"
		    "Use without an argument to see a list of wiznet flags.\n", ch);
		return;
	}

	/* hack so you can type config wiznet on or off */
	if (!str_cmp(arg1, "on")) {
		stc("Welcome to Wiznet!\n", ch);
		SET_BIT(ch->wiznet, WIZ_ON);
		return;
	}
	else if (!str_cmp(arg1, "off")) {
		stc("You have now signed off Wiznet.\n", ch);
		REMOVE_BIT(ch->wiznet, WIZ_ON);
		return;
	}

	if (arg1.empty() || !str_prefix1(arg1, "status")) {
		stc("Use 'help' or '?' as an argument after any option for details.\n", ch);
		stc("Special for Wiznet: {Con{x, {Coff{x, {Cstatus{x, {Cshow{x\n", ch);
		stc("Wiznet options:\n\n", ch);

		for (flag = 0; wiznet_table[flag].name != NULL; flag++)
			if (GET_RANK(ch) >= wiznet_table[flag].level)
				ptc(ch, "  %2d.  %-15s({Y%4s{x)                        %s\n",
				    flag, capitalize(wiznet_table[flag].name),
				    wiznet_table[flag].level == IMM ? "Imm" :
				    wiznet_table[flag].level == HED ? "Head" : "Imp",
				    IS_SET(ch->wiznet, wiznet_table[flag].flag) ?
				    "{CON{x" : "{POFF{x");

		return;
	}

	/* old school 'show' option */
	if (!str_prefix1(arg1, "show")) {
		stc("Use 'help' or '?' as an argument after any option for details.\n", ch);
		stc("Special for Wiznet: {Con{x, {Coff{x, {Cstatus{x, {Cshow{x\n", ch);
		stc("Wiznet options:\n\n", ch);

		for (flag = 0; wiznet_table[flag].name != NULL; flag++)
			if (GET_RANK(ch) >= wiznet_table[flag].level)
				ptc(ch, "%-10s - %s\n", wiznet_table[flag].name, wiznet_table[flag].desc);

		return;
	}

	if (is_number(arg1)) {
		if (wiznet_table[atoi(arg1)].name != NULL)
			argnum = atoi(arg1);
	}
	else
		for (flag = 0; wiznet_table[flag].name != NULL; flag++)
			if (!str_prefix1(arg1, wiznet_table[flag].name))
				argnum = flag;

	if (argnum >= 0)
		if (GET_RANK(ch) < wiznet_table[argnum].level)
			argnum = -1;

	if (argnum == -1) {
		stc("That is not a valid wiznet option.\n", ch);
		return;
	}

	if (!str_prefix1(argument, "help") || !str_cmp(argument, "?"))
		ptc(ch, "%s\n", wiznet_table[argnum].desc);
	else if (argument.empty()) {
		if (argnum == 0) {      /* Wiznet ON */
			if (IS_SET(ch->wiznet, wiznet_table[argnum].flag)) {
				stc("You have now signed off Wiznet.\n", ch);
				REMOVE_BIT(ch->wiznet, wiznet_table[argnum].flag);
			}
			else {
				stc("Welcome to Wiznet!\n", ch);
				SET_BIT(ch->wiznet, wiznet_table[argnum].flag);
			}
		}
		else {
			if (IS_SET(ch->wiznet, wiznet_table[argnum].flag)) {
				REMOVE_BIT(ch->wiznet, wiznet_table[argnum].flag);
				ptc(ch, "Wiznet will no longer provide you with: %s\n", wiznet_table[argnum].name);
			}
			else {
				SET_BIT(ch->wiznet, wiznet_table[argnum].flag);
				ptc(ch, "Wiznet will now provide you with: %s\n", wiznet_table[argnum].name);
			}
		}
	}
	else
		stc("Valid options are {Phelp{x and no argument (toggle).\n", ch);
}

void do_config(CHAR_DATA *ch, String argument)
{
	int argnum = 0;

	if (argument.empty()) {
		stc("Configuration groups:\n\n", ch);
//		stc("  1.  Channels\n", ch);

		if (!IS_NPC(ch)) {
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

	if (is_number(arg1))                       argnum = atoi(arg1);
	else if (!str_prefix1(arg1, "channels"))        argnum = 1;
	else if (!IS_NPC(ch) &&          !str_prefix1(arg1, "color"))           argnum = 5;
	else if (!IS_NPC(ch) &&          !str_prefix1(arg1, "video"))           argnum = 6;
	else if (!str_prefix1(arg1, "censor"))          argnum = 7;
	else if (IS_IMMORTAL(ch) &&      !str_prefix1(arg1, "immortal"))        argnum = 8;
	else if (IS_IMMORTAL(ch) &&      !str_prefix1(arg1, "wiznet"))          argnum = 9;

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

void do_color(CHAR_DATA *ch, String argument)
{
	char buf[MIL];

	if (IS_NPC(ch)) {
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

void do_video(CHAR_DATA *ch, String argument)
{
	char buf[MIL];

	if (IS_NPC(ch)) {
		stc("Mobiles can't see colors anyway. :)\n", ch);
		return;
	}

	Format::sprintf(buf, "video %s", argument);
	do_config(ch, buf);
}

void do_censor(CHAR_DATA *ch, String argument)
{
	char buf[MIL];
	Format::sprintf(buf, "censor %s", argument);
	do_config(ch, buf);
}

void do_wiznet(CHAR_DATA *ch, String argument)
{
	char buf[MIL];

	if (IS_NPC(ch)) {
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
