#include "Disabled.hpp"
#include "interp.h"
#include "sql.h"
#include "merc.h"

Disabled *disabled_first;

/* Check if that command is disabled
   Note that we check for equivalence of the do_fun pointers; this means
   that disabling 'chat' will also disable the '.' command
*/
bool check_disabled(const struct cmd_type *command)
{
	Disabled *p;

	for (p = disabled_first; p ; p = p->next)
		if (p->command->do_fun == command->do_fun)
			return TRUE;

	return FALSE;
}

void load_disabled()
{
	Disabled *p;
	int i;
	disabled_first = NULL;

	if (db_query("load_disabled", "SELECT command, immortal, reason FROM disabled") != SQL_OK)
		return;

	while (db_next_row() == SQL_OK) {
		for (i = 0; cmd_table[i].name[0]; i++)
			if (cmd_table[i].name == db_get_column_str(0))
				break;

		if (!cmd_table[i].name[0]) {
			bug("load_disabled: skipping uknown command", 0);
			continue;
		}

		p = new Disabled;
		p->command = &cmd_table[i];
		p->disabled_by = db_get_column_str(1);
		p->reason = db_get_column_str(2);
		p->next = disabled_first;
		disabled_first = p;
	}
}

void do_disable(Character *ch, String argument)
{
	Disabled *p;
	int i;

	String cmd;
	argument = one_argument(argument, cmd);

	if (cmd[0] == '\0') {
		if (!disabled_first) { /* Any disabled at all ? */
			stc("There are no commands disabled.\n", ch);
			return;
		}

		stc("Command             {T|{xDisabled by    {T|{xReason\n"
		    "{T--------------------|---------------|-----------------------------------------------{x\n", ch);

		for (p = disabled_first; p; p = p->next)
			ptc(ch, "%-20s{T|{x%-15s{T|{x%s\n", p->command->name, p->disabled_by, p->reason);

		return;
	}

	/* First check if it is one of the disabled commands */
	for (p = disabled_first; p ; p = p->next)
		if (cmd == p->command->name)
			break;

	if (p) { /* found the command, enable it */
		if (disabled_first == p)
			disabled_first = p->next;
		else {
			Disabled *q;

			for (q = disabled_first; q && q->next; q = q->next)
				if (q->next == p)
					q->next = p->next;
		}

		/* remove it from the database */
		db_commandf("do_disable", "DELETE FROM disabled WHERE command LIKE '%s'", db_esc(p->command->name));
		delete p;
		stc("Command enabled.\n", ch);
		return;
	}

	/* IQ test */
	if (cmd == "disable") {
		stc("You cannot disable the disable command.\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("You must provide a reason.\n", ch);
		return;
	}

	if (strlen(argument) > 45) {
		stc("Please limit the reason to 45 characters or less.\n", ch);
		return;
	}

	for (i = 0; cmd_table[i].name[0] != '\0'; i++)
		if (cmd == cmd_table[i].name)
			break;

	if (cmd_table[i].name[0] == '\0') {
		stc("No such command.\n", ch);
		return;
	}

	/* maybe a command group check here? thinking about it */
	p = new Disabled;
	p->command      = &cmd_table[i];
	p->disabled_by  = ch->name;
	p->reason       = argument;
	p->next = disabled_first;
	disabled_first = p;
	/* add it to the database */
	db_commandf("do_disable", "INSERT INTO disabled VALUES('%s','%s','%s')",
	            db_esc(p->command->name), db_esc(p->disabled_by), db_esc(p->reason));
	stc("Command disabled.\n", ch);
}
