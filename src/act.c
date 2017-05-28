#include "merc.h"
#include "recycle.h"

/* the guts of act, taken out to reduce complexity. */
void act_format(const String& format, CHAR_DATA *ch,
                const CHAR_DATA *vch, const CHAR_DATA *vch2,
                const String *arg1, const String *arg2,
                const OBJ_DATA *obj1, const OBJ_DATA *obj2,
                CHAR_DATA *to, bool snooper, int vis)
{
	static char *const he_she  [] = { "it",  "he",  "she" };
	static char *const him_her [] = { "it",  "him", "her" };
	static char *const his_her [] = { "its", "his", "her" };
	char buf[MAX_STRING_LENGTH];
    String fname;
	const char *str;
	const char *i;
	char *point;
	char dollarmsg[3];
	point   = buf;

	if (snooper) {
		strcpy(point, "{n[T] ");
		point += 6;
	}

	str     = format.c_str();

	while (*str != '\0') {
		if (*str != '$') {
			*point++ = *str++;
			continue;
		}

		/* '$' sign after this point */
		++str;
		Format::sprintf(dollarmsg, "$%c", *str);
		i = dollarmsg;

		switch (*str) {
		default:                                                break;

		/* The following codes need 'ch', which should always be OK */

		case 'n': i = PERS(ch, to, vis).c_str();            break;

		case 'e': i = he_she  [GET_ATTR_SEX(ch)];        break;

		case 'm': i = him_her [GET_ATTR_SEX(ch)];        break;

		case 's': i = his_her [GET_ATTR_SEX(ch)];        break;

		/* The following codes need 'vch' */

		case 'N':
			if (vch == NULL) {
				bug("Missing vch for '$$N'", 0);
				/*   bug( format, 0);  This will cause an endless loop */
			}
			else
				i = PERS(vch, to, vis).c_str();

			break;

		case 'E':
			if (vch == NULL)
				bug("Missing vch for '$$E'", 0);
			else
				i = he_she[GET_ATTR_SEX(vch)];

			break;

		case 'M':
			if (vch == NULL)
				bug("Missing vch for '$$M'", 0);
			else
				i = him_her[GET_ATTR_SEX(vch)];

			break;

		case 'S':
			if (vch == NULL)
				bug("Missing vch for '$$S'", 0);
			else
				i = his_her[GET_ATTR_SEX(vch)];

			break;

		/* The following codes need valid objects in obj1/obj2 */

		case 'p':
			if (obj1 == NULL)
				bug("Missing obj1 for '$$p'", 0);
			else if (can_see_obj(to, obj1))
				i = obj1->short_descr;
			else
				i = "something";

			break;

		case 'P':
			if (obj2 == NULL) {
				bug("Missing obj2 for '$$P'", 0);
				bug(format, 0);
			}
			else if (can_see_obj(to, obj2))
				i = obj2->short_descr;
			else
				i = "something";

			break;

		/* The following needs a string describing a door. */

		case 'd':
			if (arg2 == NULL || ((char *) arg2)[0] == '\0')
				i = "door";
			else {
				one_argument((char *) arg2, fname);
				i = fname.c_str();
			}

			break;

		/* The following codes need valid strings in arg1/arg2 */

		case 't':
			if (arg1 == NULL || arg1->empty())
				bug("Missing arg1 for '$$t'", 0);
			else
				i = arg1->c_str();

			break;

		case 'T':
			if (arg2 == NULL || arg1->empty())
				bug("Missing arg2 for '$$T'", 0);
			else
				i = arg2->c_str();

			break;

		/* The following codes need no checking */

		case 'G': i = "\007";                                   break;

		case '$': i = "$";                                      break;
		}

		++str;

		while ((*point = *i) != '\0')
			++point, ++i;
	}

	if (snooper) {
		*point++ = '{';
		*point++ = 'x';
	}

	*point++ = '\n';
//	*point++ = '\r';
	*point   = 0;
	buf[0]   = UPPER(buf[0]);

	if (to->desc)
		stc(buf, to);

	if (MOBtrigger)
        // removing const from things here, because i don't want to follow that rabbit
        // hole right now.  it does need to be fixed by making mprog stuff use const
        // object pointers, but at a later date.
		mprog_act_trigger(buf, to, ch,
            const_cast<OBJ_DATA *>(obj1),
            const_cast<CHAR_DATA *>(vch));
} /* end act_format() */

void act_parse(
    const String& fmt,
    CHAR_DATA *ch,
    const CHAR_DATA *vch, const CHAR_DATA *vch2,
    const String *str1, const String *str2,
    const OBJ_DATA *obj1, const OBJ_DATA *obj2,
    int type,
    int min_pos,
    bool censor
) {
    String format(fmt);
    ARENA_DATA *arena = arena_table_head->next;
    CHAR_DATA *to;
    bool SNEAKING = FALSE;
    TAIL_DATA *td;
    char fake_message[MAX_INPUT_LENGTH];
    int vis = VIS_CHAR;

    /*
     * Discard null and zero-length messages.
     */
    if (format == NULL || format[0] == '\0')
        return;

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL)
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
        if (vch == NULL) {
            bug("Act: null vch with TO_VICT.", 0);
            return;
        }

        if (vch->in_room == NULL)
            return;

        to = vch->in_room->people;
    }

    if (type == TO_WORLD) {
        if (vch2 == NULL) {
            bug("Act: null vch2 with TO_WORLD.", 0);
            return;
        }

        if (vch2->in_room == NULL && ch->tail == NULL)
            return;

        to = vch2->in_room->people;
    }

    /*** first loop, for normal recipients of ACT */
    for (; to != NULL; to = to->next_in_room) {
        if (censor && IS_NPC(to))
            continue;

        if (get_position(to) < min_pos)
            continue;

        if (SNEAKING) {
            if (!IS_IMMORTAL(to)) /* eliminates mobs too */
                continue;

            if (IS_SET(ch->act, PLR_SUPERWIZ) && !IS_IMP(to))
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

        if (arena != arena_table_tail && arena->viewroom->people != NULL) {
            Format::sprintf(fake_message, "{Y[V]{x %s", format);
            format = fake_message;

            for (to = arena->viewroom->people; to != NULL; to = to->next_in_room) {
                if (get_position(to) < min_pos)
                    continue;

                /**********************************************************************/
                act_format(fake_message, ch, vch, vch2, str1, str2, obj1, obj2, to, FALSE, vis);
                /**********************************************************************/
            }
        }
    }

    /* TAIL stuff -- Elrac */
    if (ch->tail == NULL)
        return;

    if (type != TO_ROOM && type != TO_NOTVICT && type != TO_WORLD && type != TO_NOTVIEW)
        return;

    if (!str_prefix1("$n says '", format)
        || !str_prefix1("$n leaves ", format))
        return;

    if (!str_prefix1("$n has arrived.", format)) {
        Format::sprintf(fake_message, "$n has arrived at %s (%s).",
                ch->in_room->name, ch->in_room->area->file_name);
        format = fake_message;
    }

    /* check integrity of tailer. untail if bad. */
    for (td = ch->tail; td;) {
        if (!IS_VALID(td->tailed_by)
            || str_cmp(td->tailer_name, td->tailed_by->name)) {
            set_tail(td->tailed_by, ch, 0);
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

void act(const String& format, CHAR_DATA *ch, const Actable* arg1, const Actable* arg2, int type, int min_pos, bool censor) {
    act_parse(
        format, ch,
        dynamic_cast<const CHAR_DATA *>(arg2),
        dynamic_cast<const CHAR_DATA *>(arg1),
        dynamic_cast<const String *>(arg1),
        dynamic_cast<const String *>(arg2),
        dynamic_cast<const OBJ_DATA *>(arg1),
        dynamic_cast<const OBJ_DATA *>(arg2),
        type, min_pos, censor
    );
}

// pointer and reference
void act(const String& format, CHAR_DATA *ch, const Actable* arg1, const Actable& arg2, int type, int min_pos, bool censor) {
    act(format, ch, arg1, &arg2, type, min_pos, censor);
}

// reference and pointer
void act(const String& format, CHAR_DATA *ch, const Actable& arg1, const Actable* arg2, int type, int min_pos, bool censor) {
    act(format, ch, &arg1, arg2, type, min_pos, censor);
}

// reference and reference
void act(const String& format, CHAR_DATA *ch, const Actable& arg1, const Actable& arg2, int type, int min_pos, bool censor) {
    act(format, ch, &arg1, &arg2, type, min_pos, censor);
}

/*
 * implementations with 1 Actable*
 */

// arg and char*
void act(const String& format, CHAR_DATA *ch, const Actable* arg1, const char *arg2, int type, int min_pos, bool censor) {
    act(format, ch, arg1, String(arg2), type, min_pos, censor);
}

// arg and null
void act(const String& format, CHAR_DATA *ch, const Actable* arg1, std::nullptr_t arg2, int type, int min_pos, bool censor) {
    act(format, ch, arg1, String(), type, min_pos, censor);
}

// char* and arg
void act(const String& format, CHAR_DATA *ch, const char *arg1, const Actable* arg2, int type, int min_pos, bool censor) {
    act(format, ch, String(arg1), arg2, type, min_pos, censor);
}

// null and arg
void act(const String& format, CHAR_DATA *ch, std::nullptr_t arg1, const Actable* arg2, int type, int min_pos, bool censor) {
    act(format, ch, String(), arg2, type, min_pos, censor);
}

/*
 * implementations with 1 Actable&
 */

// arg and char*
void act(const String& format, CHAR_DATA *ch, const Actable& arg1, const char *arg2, int type, int min_pos, bool censor) {
    act(format, ch, &arg1, String(arg2), type, min_pos, censor);
}

// arg and null
void act(const String& format, CHAR_DATA *ch, const Actable& arg1, std::nullptr_t arg2, int type, int min_pos, bool censor) {
    act(format, ch, &arg1, String(), type, min_pos, censor);
}

// char* and arg
void act(const String& format, CHAR_DATA *ch, const char *arg1, const Actable& arg2, int type, int min_pos, bool censor) {
    act(format, ch, String(arg1), &arg2, type, min_pos, censor);
}

// null and arg
void act(const String& format, CHAR_DATA *ch, std::nullptr_t arg1, const Actable& arg2, int type, int min_pos, bool censor) {
    act(format, ch, String(), &arg2, type, min_pos, censor);
}

/*
 * implementations with no Actable
 */

// char* and char*
void act(const String& format, CHAR_DATA *ch, const char *arg1, const char *arg2, int type, int min_pos, bool censor) {
    act(format, ch, String(arg1), String(arg2), type, min_pos, censor);
}

// char* and null
void act(const String& format, CHAR_DATA *ch, const char *arg1, std::nullptr_t arg2, int type, int min_pos, bool censor) {
    act(format, ch, String(arg1), String(), type, min_pos, censor);
}

// null and char*
void act(const String& format, CHAR_DATA *ch, std::nullptr_t arg1, const char *arg2, int type, int min_pos, bool censor) {
    act(format, ch, String(), String(arg2), type, min_pos, censor);
}

// null and null
void act(const String& format, CHAR_DATA *ch, std::nullptr_t arg1, std::nullptr_t arg2, int type, int min_pos, bool censor) {
    act(format, ch, String(), String(), type, min_pos, censor);
}
