#include "Reset.hh"

#include "db.hh" // boot_bug
#include "Exit.hh"
#include "file.hh"
#include "Flags.hh"
#include "macros.hh"
#include "merc.hh"
#include "ObjectPrototype.hh"
#include "Room.hh"

Reset::
Reset(FILE *fp) {
	ObjectPrototype *temp_index;

	command = fread_letter(fp);
	          fread_number(fp); // 'if' flag
	arg1    = fread_number(fp);
	arg2    = fread_number(fp);
	arg3    = (command == 'G' || command == 'R') ? 0 : fread_number(fp);
	arg4    = (command == 'P' || command == 'M') ? fread_number(fp) : 0;
	fread_to_eol(fp);

	/* Validate parameters.  We're calling the index functions for the side effect. */
	switch (command) {
	default:
		boot_bug("Load_resets: bad command '%c'.", command);
		exit(1);
		break;

	/* Mobile resets, worked over to allow for resetting into random rooms -- Montrey */
	case 'M':
		/* make sure the mobile exists */
		get_mob_index(arg1);

		/* limit the global limit to 20, -1 is max */
		/* This is 2017, I don't think we need to worry about this.  Let the builder decide. -- Montrey
		if (arg2 > 20 || arg2 < 0)
			arg2 = 20;
		*/

		/* make sure the room exists, vnum 0 means it's a random reset */
		if (arg3 != 0)
			get_room_prototype(arg3);

		/* on a random reset, local limit is percentage chance, limit to 0 to 100 */
		if (arg3 == 0)
			arg4 = URANGE(0, arg4, 100);

		break;

	case 'O':
		temp_index = get_obj_index(arg1);
		temp_index->reset_num++;
		get_room_prototype(arg3);
		break;

	case 'P':
		temp_index = get_obj_index(arg1);
		temp_index->reset_num++;
		get_obj_index(arg3);

		/*
		if (arg4 > 20 || arg4 < 0)
			arg4 = 20;      Lets add a limit - Lotus
							let's not -- Montrey
		*/

		break;

	case 'G':
	case 'E':
		temp_index = get_obj_index(arg1);
		temp_index->reset_num++;
		break;

	case 'D': {
		RoomPrototype *room = get_room_prototype(arg1);
		ExitPrototype *pexit;

		if (arg2 < 0 || arg2 > 5
		    || (pexit = room->exit[arg2]) == nullptr
		    || !pexit->exit_flags.has(EX_ISDOOR)) {
			boot_bug("Load_resets: 'D': exit %d not door.", arg2);
			exit(1);
		}

		if (arg3 < 0 || arg3 > 2) {
			boot_bug("Load_resets: 'D': bad 'locks': %d.", arg3);
			exit(1);
		}

		break;
	}

	case 'R':
		get_room_prototype(arg1);

		if (arg2 < 0 || arg2 > 6) {
			boot_bug("Load_resets: 'R': bad exit %d.", arg2);
			exit(1);
		}

		break;
	}
}
