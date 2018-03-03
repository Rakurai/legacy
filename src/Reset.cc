#include "Reset.hh"

#include "Area.hh"
#include "Exit.hh"
#include "file.hh"
#include "Flags.hh"
#include "macros.hh"
#include "merc.hh"
#include "ObjectPrototype.hh"
#include "Room.hh"
#include "Game.hh"
#include "Logging.hh"

Reset::
Reset(FILE *fp) {
	ObjectPrototype *temp_index;
	String tmp_buf;

	command = fread_letter(fp);
	          fread_number(fp); // 'if' flag

/*
	arg1    = fread_number(fp);
	arg2    = fread_number(fp);
	arg3    = (command == 'G' || command == 'R') ? 0 : fread_number(fp);
	arg4    = (command == 'P' || command == 'M') ? fread_number(fp) : 0;
	fread_to_eol(fp);
*/
	/* Validate parameters.  We're calling the index functions for the side effect. */
	switch (command) {
	default:
		Logging::file_bug(fp, "Load_resets: bad command '%c'.", command);
		exit(1);
		break;

	/* Mobile resets, worked over to allow for resetting into random rooms -- Montrey */
	case 'M':
		arg1    = fread_number(fp);
		arg2    = fread_number(fp);
//		arg3    = fread_number(fp);
		tmp_buf = fread_word(fp); // interpret as a location string
		arg3    = atoi(tmp_buf);
		arg4    = fread_number(fp);

		/* make sure the mobile exists */
		Game::world().get_mob_prototype(arg1);

		/* limit the global limit to 20, -1 is max */
		/* This is 2017, I don't think we need to worry about this.  Let the builder decide. -- Montrey
		if (arg2 > 20 || arg2 < 0)
			arg2 = 20;
		*/


		/* make sure the room exists, vnum 0 means it's a random reset */
		// note, these are only for regions, if an area has no regions a 0 room does nothing
		if (arg3 != 0)
			arg3 = Location(tmp_buf).to_int(); // doesn't test for real location

		/* on a random reset, local limit is percentage chance, limit to 0 to 100 */
		if (arg3 == 0)
			arg4 = URANGE(0, arg4, 100);

		break;

	case 'O':
		arg1    = fread_number(fp);
		arg2    = fread_number(fp);
		tmp_buf = fread_word(fp); // interpret as a location string
//		arg3    = fread_number(fp);
		arg4    = 0;

		temp_index = Game::world().get_obj_prototype(arg1);
		temp_index->reset_num++;
		arg3 = Location(tmp_buf).to_int(); // doesn't test for real location
		break;

	case 'P':
		arg1    = fread_number(fp);
		arg2    = fread_number(fp);
		arg3    = fread_number(fp);
		arg4    = fread_number(fp);

		temp_index = Game::world().get_obj_prototype(arg1);
		temp_index->reset_num++;
		Game::world().get_obj_prototype(arg3);

		/*
		if (arg4 > 20 || arg4 < 0)
			arg4 = 20;      Lets add a limit - Lotus
							let's not -- Montrey
		*/

		break;

	case 'G':
		arg1    = fread_number(fp);
		arg2    = fread_number(fp);
		arg3    = 0;
		arg4    = 0;

		temp_index = Game::world().get_obj_prototype(arg1);
		temp_index->reset_num++;
		break;

	case 'E':
		arg1    = fread_number(fp);
		arg2    = fread_number(fp);
		arg3    = fread_number(fp);
		arg4    = 0;

		temp_index = Game::world().get_obj_prototype(arg1);
		temp_index->reset_num++;
		break;

	case 'D': {
		tmp_buf = fread_word(fp); // interpret as a location string
//		arg1    = fread_number(fp);
		arg2    = fread_number(fp);
		arg3    = fread_number(fp);
		arg4    = 0;

		if (arg2 < 0 || arg2 > 5) {
			Logging::file_bug(fp, "Load_resets: 'D': exit %d not door.", arg2);
			exit(1);
		}

		if (arg3 < 0 || arg3 > 2) {
			Logging::file_bug(fp, "Load_resets: 'D': bad 'locks': %d.", arg3);
			exit(1);
		}

		// arg1 could be a vnum or could be map coordinate.  if it's
		// a vnum, we can test here that the prototype has a door.
		// if not, well, let the bug messages roll
		Location loc(tmp_buf);
		arg1 = loc.to_int();

		if (loc.room_id.is_valid()) {
			RoomPrototype *room = Game::world().get_room_prototype(loc.room_id.get_vnum());
			ExitPrototype *pexit;

			if (room == nullptr) {
				Logging::file_bug(fp, "Load_resets: 'D': no such room %d.", arg1);
				exit(1);
			}

			if ((pexit = room->exit[arg2]) == nullptr
			    || !pexit->exit_flags.has(EX_ISDOOR)) {
				Logging::file_bug(fp, "Load_resets: 'D': exit %d not door.", arg2);
				exit(1);
			}
		}

		break;
	}

	case 'R': {
		tmp_buf = fread_word(fp); // interpret as a location string
//		arg1    = fread_number(fp);
		arg2    = fread_number(fp);
		arg3    = 0;
		arg4    = 0;

		arg1 = Location(tmp_buf).to_int();

		if (arg2 < 0 || arg2 > 6) {
			Logging::file_bug(fp, "Load_resets: 'R': bad exit %d.", arg2);
			exit(1);
		}

		break;
	}
	}

	fread_to_eol(fp);
}
