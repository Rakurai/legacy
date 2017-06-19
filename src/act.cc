#include "argument.hh"
#include "Area.hh"
#include "Character.hh"
#include "declare.hh"
#include "Duel.hh"
#include "Flags.hh"
#include "Format.hh"
#include "Logging.hh"
#include "macros.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobProg.hh"
#include "Object.hh"
#include "Player.hh"
#include "recycle.hh"
#include "RoomPrototype.hh"
#include "String.hh"
#include "Tail.hh"

/* the guts of act, taken out to reduce complexity. */
void act_format(const String& format, Character *ch,
                const Character *vch, const Character *vch2,
                const String *str1, const String *str2,
                const Object *obj1, const Object *obj2,
                Character *to, bool snooper, int vis)
{
	static char *const he_she  [] = { "it",  "he",  "she" };
	static char *const him_her [] = { "it",  "him", "her" };
	static char *const his_her [] = { "its", "his", "her" };

    String buf, i;

	if (snooper) {
		buf += "{n[T] ";
	}

	const char *str = format.c_str();

	while (*str != '\0') {
		if (*str != '$') {
            buf += *str++;
			continue;
		}

		/* '$' sign after this point */
		++str;
		i = Format::format("$%c", *str); // default output if we don't find it

		switch (*str) {
		default:                                                break;

		/* The following codes need 'ch', which should always be OK */

		case 'n': i = PERS(ch, to, vis).c_str();            break;

		case 'e': i = he_she  [GET_ATTR_SEX(ch)];        break;

		case 'm': i = him_her [GET_ATTR_SEX(ch)];        break;

		case 's': i = his_her [GET_ATTR_SEX(ch)];        break;

		/* The following codes need 'vch' */

		case 'N':
			if (vch == nullptr) {
				Logging::bug("Missing vch for '$$N'", 0);
				/*   Logging::bug( format, 0);  This will cause an endless loop */
			}
			else
				i = PERS(vch, to, vis).c_str();

			break;

		case 'E':
			if (vch == nullptr)
				Logging::bug("Missing vch for '$$E'", 0);
			else
				i = he_she[GET_ATTR_SEX(vch)];

			break;

		case 'M':
			if (vch == nullptr)
				Logging::bug("Missing vch for '$$M'", 0);
			else
				i = him_her[GET_ATTR_SEX(vch)];

			break;

		case 'S':
			if (vch == nullptr)
				Logging::bug("Missing vch for '$$S'", 0);
			else
				i = his_her[GET_ATTR_SEX(vch)];

			break;

		/* The following codes need valid objects in obj1/obj2 */

		case 'p':
			if (obj1 == nullptr)
				Logging::bug("Missing obj1 for '$$p'", 0);
			else if (can_see_obj(to, obj1))
				i = obj1->short_descr;
			else
				i = "something";

			break;

		case 'P':
			if (obj2 == nullptr) {
				Logging::bug("Missing obj2 for '$$P'", 0);
				Logging::bug(format, 0);
			}
			else if (can_see_obj(to, obj2))
				i = obj2->short_descr;
			else
				i = "something";

			break;

		/* The following needs a string describing a door. */

		case 'd':
			if (str2 == nullptr || str2->empty())
				i = "door";
			else
				one_argument(*str2, i);

			break;

		/* The following codes need valid strings in str1/str2 */

		case 't':
			if (str1 == nullptr || str1->empty())
				Logging::bug("Missing str1 for '$$t'", 0);
			else
				i = *str1;

			break;

		case 'T':
			if (str2 == nullptr || str2->empty())
				Logging::bug("Missing str2 for '$$T'", 0);
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
	buf[0] = UPPER(buf[0]);

	if (to->desc)
		stc(buf, to);

	if (MOBtrigger)
        // removing const from things here, because i don't want to follow that rabbit
        // hole right now.  it does need to be fixed by making mprog stuff use const
        // object pointers, but at a later date.
		mprog_act_trigger(buf.c_str(), to, ch,
            const_cast<Object *>(obj1),
            const_cast<Character *>(vch));
} /* end act_format() */

void act_parse(
    const String& fmt,
    Character *ch,
    const Character *vch, const Character *vch2,
    const String *str1, const String *str2,
    const Object *obj1, const Object *obj2,
    int type,
    int min_pos,
    bool censor
) {
    String format(fmt);
    Duel::Arena *arena = arena_table_head->next;
    Character *to;
    bool SNEAKING = FALSE;
    Tail *td;
    char fake_message[MAX_INPUT_LENGTH];
    int vis = VIS_CHAR;

    /*
     * Discard null and zero-length messages.
     */
    if (format.empty())
        return;

    /* discard null rooms and chars */
    if (ch == nullptr || ch->in_room == nullptr)
        return;

    to = ch->in_room->people;

    if (min_pos == POS_SNEAK) {
        min_pos = POS_RESTING;
        SNEAKING = TRUE;
    }

    /* blah, special hack for channel visibility.  rewrite this crap someday, i don't
       have time right now to give act the attention it needs.  -- Montrey */
    if (type == TO_VICT_CHANNEL) {
        type = TO_VICT;
        vis = VIS_PLR;
    }

    if (type == TO_VICT) {
        if (vch == nullptr) {
            Logging::bug("Act: null vch with TO_VICT.", 0);
            return;
        }

        if (vch->in_room == nullptr)
            return;

        to = vch->in_room->people;
    }

    if (type == TO_WORLD) {
        if (vch2 == nullptr) {
            Logging::bug("Act: null vch2 with TO_WORLD.", 0);
            return;
        }

        if (vch2->in_room == nullptr && ch->tail == nullptr)
            return;

        to = vch2->in_room->people;
    }

    /*** first loop, for normal recipients of ACT */
    for (; to != nullptr; to = to->next_in_room) {
        if (censor && IS_NPC(to))
            continue;

        if (get_position(to) < min_pos)
            continue;

        if (SNEAKING) {
            if (!IS_IMMORTAL(to)) /* eliminates mobs too */
                continue;

            if (ch->act_flags.has(PLR_SUPERWIZ) && !IS_IMP(to))
                continue;
        }

        if ((type == TO_CHAR) && to != ch)
            continue;

        if (type == TO_VICT && (to != vch || to == ch))
            continue;

        if (type == TO_ROOM && to == ch)
            continue;

        if (type == TO_NOTVICT && (to == ch || to == vch))
            continue;

        if (type == TO_WORLD && (to == ch || to == vch || to != vch2))
            continue;

        if (type == TO_NOTVIEW && to == ch) /* same as TO_ROOM */
            continue;

        if (type == TO_VIEW)
            continue;

        /**********************************************************************/
        act_format(format, ch, vch, vch2, str1, str2, obj1, obj2, to, FALSE, vis);
        /**********************************************************************/
    }

    /* viewing room stuff */
    if (!censor && (type == TO_ROOM || type == TO_NOTVICT || type == TO_VIEW)) {
        while (arena != arena_table_tail) {
            if (ch->in_room->vnum >= arena->minvnum
                && ch->in_room->vnum <= arena->maxvnum)
                break;

            arena = arena->next;
        }

        if (arena != arena_table_tail && arena->viewroom->people != nullptr) {
            Format::sprintf(fake_message, "{Y[V]{x %s", format);
            format = fake_message;

            for (to = arena->viewroom->people; to != nullptr; to = to->next_in_room) {
                if (get_position(to) < min_pos)
                    continue;

                /**********************************************************************/
                act_format(fake_message, ch, vch, vch2, str1, str2, obj1, obj2, to, FALSE, vis);
                /**********************************************************************/
            }
        }
    }

    /* TAIL stuff -- Elrac */
    if (ch->tail == nullptr)
        return;

    if (type != TO_ROOM && type != TO_NOTVICT && type != TO_WORLD && type != TO_NOTVIEW)
        return;

    if (format.has_prefix("$n says '")
        || format.has_prefix("$n leaves "))
        return;

    if (format.has_prefix("$n has arrived.")) {
        Format::sprintf(fake_message, "$n has arrived at %s (%s).",
                ch->in_room->name, ch->in_room->area->file_name);
        format = fake_message;
    }

    /* check integrity of tailer. untail if bad. */
    for (td = ch->tail; td;) {
        if (!IS_VALID(td->tailed_by)
            || td->tailer_name != td->tailed_by->name) {
            set_tail(td->tailed_by, ch, TAIL_NONE);
            td = ch->tail;
            continue;
        }

        td = td->next;
    }

    /*** second loop, for tailers ***/
    for (td = ch->tail; td; td = td->next) {
        /* check if tailer in room with actor */
        for (to = ch->in_room->people; to; to = to->next_in_room) {
            if (to == td->tailed_by)
                break;
        }

        if (to)
            continue;

        /**********************************************************************/
        act_format(format, ch, vch, vch2, str1, str2, obj1, obj2,
                   td->tailed_by, TRUE, vis);
        /**********************************************************************/
    }

    /* Add this to turn Mob Programs Off
                    MOBtrigger = FALSE;
    Add before the call to act */
    MOBtrigger = TRUE;
    return;

}

void act(const String& format, Character *ch, const Actable* arg1, const Actable* arg2, int type, int min_pos, bool censor) {
    act_parse(
        format, ch,
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
void act(const String& format, Character *ch, const Actable* arg1, const Actable& arg2, int type, int min_pos, bool censor) {
    act(format, ch, arg1, &arg2, type, min_pos, censor);
}

// reference and pointer
void act(const String& format, Character *ch, const Actable& arg1, const Actable* arg2, int type, int min_pos, bool censor) {
    act(format, ch, &arg1, arg2, type, min_pos, censor);
}

// reference and reference
void act(const String& format, Character *ch, const Actable& arg1, const Actable& arg2, int type, int min_pos, bool censor) {
    act(format, ch, &arg1, &arg2, type, min_pos, censor);
}

/*
 * implementations with 1 Actable*
 */

// arg and char*
void act(const String& format, Character *ch, const Actable* arg1, const char *arg2, int type, int min_pos, bool censor) {
    act(format, ch, arg1, String(arg2), type, min_pos, censor);
}

// arg and null
void act(const String& format, Character *ch, const Actable* arg1, std::nullptr_t arg2, int type, int min_pos, bool censor) {
    act(format, ch, arg1, String(), type, min_pos, censor);
}

// char* and arg
void act(const String& format, Character *ch, const char *arg1, const Actable* arg2, int type, int min_pos, bool censor) {
    act(format, ch, String(arg1), arg2, type, min_pos, censor);
}

// null and arg
void act(const String& format, Character *ch, std::nullptr_t arg1, const Actable* arg2, int type, int min_pos, bool censor) {
    act(format, ch, String(), arg2, type, min_pos, censor);
}

/*
 * implementations with 1 Actable&
 */

// arg and char*
void act(const String& format, Character *ch, const Actable& arg1, const char *arg2, int type, int min_pos, bool censor) {
    act(format, ch, &arg1, String(arg2), type, min_pos, censor);
}

// arg and null
void act(const String& format, Character *ch, const Actable& arg1, std::nullptr_t arg2, int type, int min_pos, bool censor) {
    act(format, ch, &arg1, String(), type, min_pos, censor);
}

// char* and arg
void act(const String& format, Character *ch, const char *arg1, const Actable& arg2, int type, int min_pos, bool censor) {
    act(format, ch, String(arg1), &arg2, type, min_pos, censor);
}

// null and arg
void act(const String& format, Character *ch, std::nullptr_t arg1, const Actable& arg2, int type, int min_pos, bool censor) {
    act(format, ch, String(), &arg2, type, min_pos, censor);
}

/*
 * implementations with no Actable
 */

// char* and char*
void act(const String& format, Character *ch, const char *arg1, const char *arg2, int type, int min_pos, bool censor) {
    act(format, ch, String(arg1), String(arg2), type, min_pos, censor);
}

// char* and null
void act(const String& format, Character *ch, const char *arg1, std::nullptr_t arg2, int type, int min_pos, bool censor) {
    act(format, ch, String(arg1), String(), type, min_pos, censor);
}

// null and char*
void act(const String& format, Character *ch, std::nullptr_t arg1, const char *arg2, int type, int min_pos, bool censor) {
    act(format, ch, String(), String(arg2), type, min_pos, censor);
}

// null and null
void act(const String& format, Character *ch, std::nullptr_t arg1, std::nullptr_t arg2, int type, int min_pos, bool censor) {
    act(format, ch, String(), String(), type, min_pos, censor);
}
