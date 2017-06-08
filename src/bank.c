/*
 * Bank Code 1.00b  Wow..  am I cool.... (ya, but I'm cooler)
 *        - Cosmo..  The Cossmic Coder  :)
 *
 *    Modified by PwrDemon for Rom 2.4
 */

#include "merc.h"
#include "lookup.h"
#include "Format.hpp"

void do_deposit(Character *ch, String argument)
{
	char buf[MAX_INPUT_LENGTH];
	long max_gold;  /* to get rid of a warning on make :P */
	long max_silver;

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (IS_NPC(ch)) {
		stc("Mobiles can't deposit money!\n", ch);
		return;
	}

	if (!IS_SET(GET_ROOM_FLAGS(ch->in_room), ROOM_BANK)) {
		stc("Funny, this doesn't look like a bank...\n", ch);
		return;
	}

	if (arg1.empty() || !arg1.is_number() || atoi(arg1) <= 0 ||
	    (strcasecmp(arg2, "gold") != 0 && strcasecmp(arg2, "silver") != 0)) {
		stc("Syntax:\n"
		    "  deposit <amount> <gold or silver>\n", ch);
		return;
	}

	find_money(ch);

	if (strcasecmp(arg2, "gold") == 0) {
		if (ch->gold < atoi(arg1)) {
			stc("You don't HAVE that much gold..\n", ch);
			return;
		}

		max_gold = ((ch->pcdata->remort_count * 1000) + 50000);

		if ((ch->gold_in_bank + atoi(arg1)) > max_gold) {
			ptc(ch, "You can't put more than %ld gold in the bank.\n", max_gold);
			return;
		}

		ch->gold -= atoi(arg1);
		ch->gold_in_bank += atoi(arg1);
		Format::sprintf(buf, "Your balance is %ld gold and %ld silver.\n",
		        ch->gold_in_bank, ch->silver_in_bank);
		stc(buf, ch);
	}

	if (strcasecmp(arg2, "silver") == 0) {
		if (ch->silver < atoi(arg1)) {
			stc("You don't HAVE that much silver.\n", ch);
			return;
		}

		max_silver = ((ch->pcdata->remort_count * 100000) + 1000000);

		if ((ch->silver_in_bank + atoi(arg1)) > max_silver) {
			ptc(ch, "You can't put more than %ld silver in the bank.\n", max_silver);
			return;
		}

		ch->silver -= atoi(arg1);
		ch->silver_in_bank += atoi(arg1);
		Format::sprintf(buf, "Your balance is %ld gold and %ld silver.\n",
		        ch->gold_in_bank, ch->silver_in_bank);
		stc(buf, ch);
	}

	return;
}

void do_clandeposit(Character *ch, String argument)
{
	char buf[MAX_INPUT_LENGTH];
	Clan *target;

	if (IS_NPC(ch)) {
		stc("Mobiles can't deposit money!\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("{PHelp clandeposit{x for help on how to use this.\n", ch);
		return;
	}

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	/* Take care of mortals first */
	if (!IS_IMMORTAL(ch)) {
		if (!IS_SET(GET_ROOM_FLAGS(ch->in_room), ROOM_BANK)) {
			stc("Funny, this doesn't look like a bank...\n", ch);
			return;
		}

		if (ch->clan == NULL) {
			stc("You are not in a clan.\n", ch);
			return;
		}

		if (ch->clan->independent) {
			stc("Your clan does not have a bank account.\n", ch);
			return;
		}

		if (!arg1.is_number() || atoi(arg1) <= 0) {
			stc("Syntax:\n"
			    "  clandeposit <amount>.\n", ch);
			return;
		}

		if (ch->gold < atoi(arg1)) {
			stc("You don't HAVE that much gold...\n", ch);
			return;
		}

		if ((ch->clan->gold_balance + atoi(arg1)) > 200000) {
			stc("The clanbank limit is 200000 gold.\n", ch);
			return;
		}

		ch->gold -= atoi(arg1);
		ch->gold_donated += atoi(arg1);
		ch->clan->gold_balance += atoi(arg1);
		save_clan_table();
		save_char_obj(ch);
		Format::sprintf(buf, "{W[ %s just deposited %d gold coin%s in the clan's bank account! ]{x\n",
		        ch->name, atoi(arg1), (atoi(arg1) > 1) ? "s" : "");
		send_to_clan(ch, ch->clan, buf);
		ptc(ch, "Your clan's balance is now %ld gold.\n", ch->clan->gold_balance);
		return;
	}

	/* now for imms */

	if (!HAS_CGROUP(ch, GROUP_BUILD) && !IS_IMP(ch)) {
		stc("You're not high enough level to give gold to clans.\n", ch);
		return;
	}

	if (arg1.empty()) {
		stc("Syntax:\n"
		    "  clandeposit <clanname> <amount>.\n", ch);
		return;
	}

	target = clan_lookup(arg1);

	if (target == NULL) {
		stc("There is no such clan.\n", ch);
		return;
	}

	if (target->independent) {
		stc("That clan does not have a bank account.\n", ch);
		return;
	}

	if (atoi(arg2) <= 0) {
		stc("The amount must be greater than 0.\n", ch);
		return;
	}

	if ((target->gold_balance + atoi(arg2)) > 200000) {
		stc("The clanbank limit is 200000 gold.\n", ch);
		return;
	}

	target->gold_balance += atoi(arg2);
	Format::sprintf(buf, "{W[ %s just deposited %d gold coin%s in the clan's bank account! ]{x\n",
	        ch->name, atoi(arg2), (atoi(arg2) > 1) ? "s" : "");
	send_to_clan(ch, target, buf);
	ptc(ch, "That clan's balance is now %ld gold.\n", target->gold_balance);
	save_clan_table();
	return;
}

void find_money(Character *ch)
{
	sh_int j;
	long max_gold, max_silver;

	if (IS_NPC(ch))
		return;

	if ((ch->last_bank - current_time) > 86400) {
		for (j = (ch->last_bank - current_time) / 86400; j != 0 ; j--) {
			ch->gold_in_bank += ch->gold_in_bank * .02;
			ch->silver_in_bank += ch->silver_in_bank * .02;
		}

		ch->last_bank = time(NULL);
	}

	max_gold = ((ch->pcdata->remort_count * 1000) + 50000);
	max_silver = ((ch->pcdata->remort_count * 100000) + 1000000);

	if ((ch->gold_in_bank > max_gold) || ch->gold_in_bank < 0)
		ch->gold_in_bank = max_gold;

	if ((ch->silver_in_bank > max_silver) || ch->gold_in_bank < 0)
		ch->silver_in_bank = max_silver;
}

void do_balance(Character *ch, void *vo)
{
	char buf[MAX_INPUT_LENGTH];

	if (IS_NPC(ch)) {
		stc("Mobiles do not have bank accounts.\n", ch);
		return;
	}

	if (!IS_SET(GET_ROOM_FLAGS(ch->in_room), ROOM_BANK)) {
		stc("There is no bank here.\n", ch);
		return;
	}

	find_money(ch);
	Format::sprintf(buf, "You have %ld gold and %ld silver pieces in the bank.\n",
	        ch->gold_in_bank, ch->silver_in_bank);
	stc(buf, ch);
}

void do_withdraw(Character *ch, String argument)
{
	char buf[MAX_INPUT_LENGTH];

	if (IS_NPC(ch)) {
		stc("NPC's can't get money..  dumbass!!!!\n", ch);
		return;
	}

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (!IS_SET(GET_ROOM_FLAGS(ch->in_room), ROOM_BANK)) {
		stc("Funny, this doesn't look like a bank...\n", ch);
		return;
	}

	find_money(ch);

	if (arg1.empty() || !arg1.is_number() || atoi(arg1) <= 0 ||
	    (strcasecmp(arg2, "gold") != 0 && strcasecmp(arg2, "silver") != 0)) {
		stc("Syntax:\n"
		    "  withdraw <amount> <gold or silver>\n", ch);
		return;
	}

	if (strcasecmp(arg2, "gold") == 0) {
		if (ch->gold_in_bank < atoi(arg1)) {
			stc("You don't have that much gold in the bank!\n", ch);
			return;
		}

		if (get_carry_weight(ch) + gold_weight(atoi(arg1)) > can_carry_w(ch)) {
			act("You can't carry that much weight.",
			    ch, NULL, NULL, TO_CHAR);
			return;
		}

		ch->gold_in_bank -= atoi(arg1);
		ch->gold += atoi(arg1);
		find_money(ch);
		Format::sprintf(buf, "You now have %ld gold and %ld silver in the bank.\n",
		        ch->gold_in_bank, ch->silver_in_bank);
		stc(buf, ch);
	}

	if (strcasecmp(arg2, "silver") == 0) {
		if (ch->silver_in_bank < atoi(arg1)) {
			stc("You don't have that much silver in the bank!\n", ch);
			return;
		}

		if (get_carry_weight(ch) + silver_weight(atoi(arg1)) > can_carry_w(ch)) {
			act("You can't carry that much weight.",
			    ch, NULL, NULL, TO_CHAR);
			return;
		}

		ch->silver_in_bank -= atoi(arg1);
		ch->silver += atoi(arg1);
		find_money(ch);
		Format::sprintf(buf, "You now have %ld gold and %ld silver in the bank.\n",
		        ch->gold_in_bank, ch->silver_in_bank);
		stc(buf, ch);
	}

	return;
}

void do_clanwithdraw(Character *ch, String argument)
{
	char buf[MAX_INPUT_LENGTH];
	Clan *target;

	if (IS_NPC(ch)) {
		stc("Mobiles don't need money.\n", ch);
		return;
	}

	String arg1;
	argument = one_argument(argument, arg1);

	/* do the mortal case first */
	if (!IS_IMMORTAL(ch)) {
		if (!IS_SET(GET_ROOM_FLAGS(ch->in_room), ROOM_BANK)) {
			stc("Funny, this doesn't look like a bank...\n", ch);
			return;
		}

		if (ch->clan == NULL) {
			stc("You are not in a clan.\n", ch);
			return;
		}

		if (ch->clan->independent) {
			stc("Your clan does not have a bank account.\n", ch);
			return;
		}

		if (!HAS_CGROUP(ch, GROUP_LEADER)) {
			stc("Only leaders may withdraw gold from the clan bank account.\n", ch);
			return;
		}

		if (arg1.empty() || !arg1.is_number() || atoi(arg1) <= 0) {
			stc("Syntax:\n"
			    "  clanwithdraw <amount>.\n", ch);
			return;
		}

		if (argument.empty()) {
			stc("You must provide a reason.\n", ch);
			return;
		}

		if (ch->clan->gold_balance < atoi(arg1)) {
			stc("Your clan does not have that much gold in the bank.\n", ch);
			return;
		}

		if (get_carry_weight(ch) + gold_weight(atoi(arg1)) > can_carry_w(ch)) {
			stc("You can't carry that much weight.\n", ch);
			return;
		}

		ch->clan->gold_balance -= atoi(arg1);
		ch->gold += atoi(arg1);
		Format::sprintf(buf, "{W[ %s has withdrawn %d gold coin%s for %s. ]{x\n",
		        ch->name, atoi(arg1), (atoi(arg1) > 1) ? "s" : "", argument);
		send_to_clan(ch, ch->clan, buf);
		Format::sprintf(buf, "{WYour clan's balance is now %ld gold.{x\n", ch->clan->gold_balance);
		send_to_clan(ch, ch->clan, buf);
		save_clan_table();
		save_char_obj(ch);
		return;
	}

	/* now for imms */
	if (!HAS_CGROUP(ch, GROUP_BUILD) && !IS_IMP(ch)) {
		stc("You're not high enough level to withdraw gold from clans.\n", ch);
		return;
	}

	if (arg1.empty()) {
		stc("Syntax:\n"
		    "  clanwithdraw <clanname> <amount> <reason>\n", ch);
		return;
	}

	target = clan_lookup(arg1);

	if (target == NULL) {
		stc("There is no such clan.\n", ch);
		return;
	}

	if (target->independent) {
		stc("That clan does not have a bank account.\n", ch);
		return;
	}

	String arg2;
	argument = one_argument(argument, arg2);

	if (atoi(arg2) <= 0) {
		stc("The amount must be greater than 0.\n", ch);
		return;
	}

	if (target->gold_balance < atoi(arg2)) {
		stc("That clan does not have that much gold in the bank.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("You must provide a reason.\n", ch);
		return;
	}

	target->gold_balance -= atoi(arg2);
	Format::sprintf(buf, "{W[ %s has deducted %d gold coin%s for %s. ]\n",
	        ch->name, atoi(arg2), (atoi(arg2) > 1) ? "s" : "", argument);
	send_to_clan(ch, target, buf);
	Format::sprintf(buf, "{WYour clan's balance is now %ld gold.{x\n", target->gold_balance);
	send_to_clan(ch, target, buf);
	ptc(ch, "Their clan's balance is now %ld gold.\n", target->gold_balance);
	save_clan_table();
	return;
}
