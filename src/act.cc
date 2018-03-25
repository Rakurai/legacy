#include "argument.hh"
#include "Area.hh"
#include "Character.hh"
#include "Duel.hh"
#include "Logging.hh"
#include "progs/triggers.hh"
#include "Object.hh"
#include "Room.hh"
#include "String.hh"
#include "Tail.hh"
#include "RoomPrototype.hh"
#include "Game.hh"

void act_bug(const String& var, char letter, const String fmt) {
	// make sure $ is doubled here, or wiznet will cause a loop
	Logging::bugf("Missing %s for '$$%c' in format string '%s'",
		var, letter, fmt.replace("$", "$$"));
}

/* the guts of act, taken out to reduce complexity. */
void act_format(const String& format, Character *actor,
				const Character *vch1, const Character *vch2,
				const String *str1, const String *str2,
				const Object *obj1, const Object *obj2,
				Character *observer, bool snooper, int vis)
{
	static char *const he_she  [] = { "it",  "he",  "she" };
	static char *const him_her [] = { "it",  "him", "her" };
	static char *const his_her [] = { "its", "his", "her" };

	String buf, i;

	if (snooper) {
		buf += "{n[T] ";
	}

	auto str = format.cbegin();

	while (str != format.cend()) {
		if (*str != '$') {
			buf += *str++;
			continue;
		}

		/* '$' sign after this point */
		++str;
		i = Format::format("$%c", *str); // default output if we don't find it

		switch (*str) {
		default:                                                break;

		/* The following codes need 'actor' */

		case 'n':
			if (actor == nullptr)
				act_bug("actor", *str, format);
			else
				i = PERS(actor, observer, vis);

			break;

		case 'e':
			if (actor == nullptr)
				act_bug("actor", *str, format);
			else
				i = he_she[GET_ATTR_SEX(actor)];

			break;

		case 'm': 
			if (actor == nullptr)
				act_bug("actor", *str, format);
			else
				i = him_her[GET_ATTR_SEX(actor)];

			break;

		case 's':
			if (actor == nullptr)
				act_bug("actor", *str, format);
			else
				i = his_her[GET_ATTR_SEX(actor)];

			break;

		/* The following codes need 'vch1' */

		case 'N':
			if (vch1 == nullptr)
				act_bug("vch1", *str, format);
			else
				i = PERS(vch1, observer, vis);

			break;

		case 'E':
			if (vch1 == nullptr)
				act_bug("vch1", *str, format);
			else
				i = he_she[GET_ATTR_SEX(vch1)];

			break;

		case 'M':
			if (vch1 == nullptr)
				act_bug("vch1", *str, format);
			else
				i = him_her[GET_ATTR_SEX(vch1)];

			break;

		case 'S':
			if (vch1 == nullptr)
				act_bug("vch1", *str, format);
			else
				i = his_her[GET_ATTR_SEX(vch1)];

			break;

		/* The following codes need valid objects in obj1/obj2 */

		case 'p':
			if (obj1 == nullptr)
				act_bug("obj1", *str, format);
			else if (can_see_obj(observer, obj1))
				i = obj1->short_descr;
			else
				i = "something";

			break;

		case 'P':
			if (obj2 == nullptr)
				act_bug("obj2", *str, format);
			else if (can_see_obj(observer, obj2))
				i = obj2->short_descr;
			else
				i = "something";

			break;

		/* The following needs a string describing a door. */

		case 'd':
			if (str2 == nullptr) {
				act_bug("str2", *str, format);
				i = "door";
			}
			else
				i = *str2;

			break;

		/* The following codes need valid strings in str1/str2 */

		case 't':
			if (str1 == nullptr)
				act_bug("str1", *str, format);
			else
				i = *str1;

			break;

		case 'T':
			if (str2 == nullptr)
				act_bug("str2", *str, format);
			else
				i = *str2;

			break;

		/* The following codes need no checking */

		case 'G': i = "\007";                                   break;

		case '$': i = "$";                                      break;
		}

		++str;
		buf += i;
	}

	if (snooper)
		buf += "{x";

	buf += '\n';
	buf[0] = toupper(buf[0]);

	if (observer->desc)
		stc(buf, observer);

	if (Game::MOBtrigger)
        // removing const from things here, because i don't want to follow that rabbit
        // hole right now.  it does need to be fixed by making mprog stuff use const
        // object pointers, but at a later date.
		progs::act_trigger(buf, observer, actor,
            const_cast<Object *>(obj1),
            const_cast<Character *>(vch1));
} /* end act_format() */

void act_parse(
	String format, // make a copy
	Character *actor,
	Room *room,
	const Character *vch1, const Character *vch2,
	const String *str1, const String *str2,
	const Object *obj1, const Object *obj2,
	int type,
	int min_pos,
	bool censor
) {
	// Discard zero-length messages.
	if (format.empty())
		return;

	// figure out the start of the char list that we'll act to
	if (room == nullptr) {
		// defaults, room could be specialized below
		if (actor != nullptr)
			room = actor->in_room;
		else if (obj1 != nullptr)
			room = obj1->in_room; // special case for objects as actors, obj1 will be valid but actor will not
	}

	if (type == TO_VICT) {
		if (vch1 == nullptr) {
			Logging::bugf("Act: null vch1 with TO_VICT in format string '%s'", format.replace("$", "$$"));
			return;
		}

		if (vch1->in_room == nullptr)
			return;

		room = vch1->in_room;
	}

	if (type == TO_WORLD) {
		if (vch2 == nullptr) {
			Logging::bugf("Act: null vch2 with TO_WORLD in format string '%s'", format.replace("$", "$$"));
			return;
		}

		if (vch2->in_room == nullptr && actor && actor->tail == nullptr)
			return;

		room = vch2->in_room;
	}

	// if no room to act in, just move on
	if (room == nullptr)
		return;

	bool SNEAKING = false;
	int vis = VIS_CHAR;

	/* blah, special hack for channel visibility.  rewrite this crap someday, i don't
	   have time right now to give act the attention it needs.  -- Montrey */
	if (type == TO_VICT_CHANNEL) {
		type = TO_VICT;
		vis = VIS_PLR;
	}

	if (min_pos == POS_SNEAK) {
		min_pos = POS_RESTING;
		SNEAKING = true;
	}

	/*** first loop, for normal recipients of ACT */
	for (Character *observer = room->people; observer != nullptr; observer = observer->next_in_room) {
		if (censor && observer->is_npc())
			continue;

		if (get_position(observer) < min_pos)
			continue;

		if (SNEAKING) {
			if (!IS_IMMORTAL(observer)) /* eliminates mobs too */
				continue;

			if (actor && actor->act_flags.has(PLR_SUPERWIZ) && !IS_IMP(observer))
				continue;
		}

		// if actor == nullptr, these checks should be safe since observer != nullptr
		if (type == TO_CHAR && observer != actor)
			continue;

		if (type == TO_VICT && (observer != vch1 || observer == actor))
			continue;

		if (type == TO_ROOM && observer == actor)
			continue;

		if (type == TO_NOTVICT && (observer == actor || observer == vch1))
			continue;

		if (type == TO_WORLD && (observer == actor || observer == vch1 || observer != vch2))
			continue;

		if (type == TO_NOTVIEW && observer == actor) /* same as TO_ROOM */
			continue;

		if (type == TO_VIEW)
			continue;

		/**********************************************************************/
		act_format(format, actor, vch1, vch2, str1, str2, obj1, obj2, observer, false, vis);
		/**********************************************************************/
	}

	/* viewing room stuff */
	if (!censor && (type == TO_ROOM || type == TO_NOTVICT || type == TO_VIEW)) {
		Duel::Arena *arena = arena_table_head->next;

		while (arena != arena_table_tail) {
			if (room->prototype.vnum >= arena->minvnum
			 && room->prototype.vnum <= arena->maxvnum)
				break;

			arena = arena->next;
		}

		if (arena != arena_table_tail && arena->viewroom->people != nullptr) {
			for (Character *observer = arena->viewroom->people; observer != nullptr; observer = observer->next_in_room) {
				if (get_position(observer) < min_pos)
					continue;

				/**********************************************************************/
				act_format("{Y[V]{x " + format, actor, vch1, vch2, str1, str2, obj1, obj2, observer, false, vis);
				/**********************************************************************/
			}
		}
	}

	/* TAIL stuff -- Elrac */
	if (actor == nullptr || actor->tail == nullptr)
		return;

	if (type != TO_ROOM && type != TO_NOTVICT && type != TO_WORLD && type != TO_NOTVIEW)
		return;

	if (format.has_prefix("$n says '")
	 || format.has_prefix("$n leaves "))
		return;

	if (format.has_prefix("$n has arrived."))
		format = Format::format("$n has arrived at %s (%s).",
				actor->in_room->name(), actor->in_room->area().file_name);

	/* check integrity of tailer. untail if bad. */
	for (Tail *td = actor->tail; td; /**/) {
		if (td->tailer_name != td->tailed_by->name) {
			set_tail(td->tailed_by, actor, TAIL_NONE);
			td = actor->tail;
			continue;
		}

		td = td->next;
	}

	/*** second loop, for tailers ***/
	for (Tail *td = actor->tail; td; td = td->next) {
		Character *observer = nullptr;
		/* check if tailer in room with actor */
		for (observer = actor->in_room->people; observer; observer = observer->next_in_room) {
			if (observer == td->tailed_by)
				break;
		}

		if (observer)
			continue;

		/**********************************************************************/
		act_format(format, actor, vch1, vch2, str1, str2, obj1, obj2,
				   td->tailed_by, true, vis);
		/**********************************************************************/
	}
    Game::MOBtrigger = true;
    return;
}

void act(const String& format, Character *actor, const Actable* arg1, const Actable* arg2, int type, int min_pos, bool censor, Room *room) {
	act_parse(
		format, actor, room,
		(const Character *)arg2,
		(const Character *)arg1,
		(const String *)arg1,
		(const String *)arg2,
		(const Object *)arg1,
		(const Object *)arg2,
		type, min_pos, censor
	);
}

// pointer and reference
void act(const String& format, Character *actor, const Actable* arg1, const Actable& arg2, int type, int min_pos, bool censor, Room *room) {
	act(format, actor, arg1, &arg2, type, min_pos, censor, room);
}

// reference and pointer
void act(const String& format, Character *actor, const Actable& arg1, const Actable* arg2, int type, int min_pos, bool censor, Room *room) {
	act(format, actor, &arg1, arg2, type, min_pos, censor, room);
}

// reference and reference
void act(const String& format, Character *actor, const Actable& arg1, const Actable& arg2, int type, int min_pos, bool censor, Room *room) {
	act(format, actor, &arg1, &arg2, type, min_pos, censor, room);
}

/*
 * implementations with 1 Actable*
 */

// arg and char*
void act(const String& format, Character *actor, const Actable* arg1, const char *arg2, int type, int min_pos, bool censor, Room *room) {
	act(format, actor, arg1, String(arg2), type, min_pos, censor, room);
}

// arg and null
void act(const String& format, Character *actor, const Actable* arg1, std::nullptr_t arg2, int type, int min_pos, bool censor, Room *room) {
	act(format, actor, arg1, String(), type, min_pos, censor, room);
}

// char* and arg
void act(const String& format, Character *actor, const char *arg1, const Actable* arg2, int type, int min_pos, bool censor, Room *room) {
	act(format, actor, String(arg1), arg2, type, min_pos, censor, room);
}

// null and arg
void act(const String& format, Character *actor, std::nullptr_t arg1, const Actable* arg2, int type, int min_pos, bool censor, Room *room) {
	act(format, actor, String(), arg2, type, min_pos, censor, room);
}

/*
 * implementations with 1 Actable&
 */

// arg and char*
void act(const String& format, Character *actor, const Actable& arg1, const char *arg2, int type, int min_pos, bool censor, Room *room) {
	act(format, actor, &arg1, String(arg2), type, min_pos, censor, room);
}

// arg and null
void act(const String& format, Character *actor, const Actable& arg1, std::nullptr_t arg2, int type, int min_pos, bool censor, Room *room) {
	act(format, actor, &arg1, String(), type, min_pos, censor, room);
}

// char* and arg
void act(const String& format, Character *actor, const char *arg1, const Actable& arg2, int type, int min_pos, bool censor, Room *room) {
	act(format, actor, String(arg1), &arg2, type, min_pos, censor, room);
}

// null and arg
void act(const String& format, Character *actor, std::nullptr_t arg1, const Actable& arg2, int type, int min_pos, bool censor, Room *room) {
	act(format, actor, String(), &arg2, type, min_pos, censor, room);
}

/*
 * implementations with no Actable
 */

// char* and char*
void act(const String& format, Character *actor, const char *arg1, const char *arg2, int type, int min_pos, bool censor, Room *room) {
	act(format, actor, String(arg1), String(arg2), type, min_pos, censor, room);
}

// char* and null
void act(const String& format, Character *actor, const char *arg1, std::nullptr_t arg2, int type, int min_pos, bool censor, Room *room) {
	act(format, actor, String(arg1), String(), type, min_pos, censor, room);
}

// null and char*
void act(const String& format, Character *actor, std::nullptr_t arg1, const char *arg2, int type, int min_pos, bool censor, Room *room) {
	act(format, actor, String(), String(arg2), type, min_pos, censor, room);
}

// null and null
void act(const String& format, Character *actor, std::nullptr_t arg1, std::nullptr_t arg2, int type, int min_pos, bool censor, Room *room) {
	act(format, actor, String(), String(), type, min_pos, censor, room);
}
