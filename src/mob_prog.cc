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
 *  The MOBprograms have been contributed by N'Atas-ha.  Any support for   *
 *  these routines should not be expected from Merc Industries.  However,  *
 *  under no circumstances should the blame for bugs, etc be placed on     *
 *  Merc Industries.  They are not guaranteed to work on all systems due   *
 *  to their frequent use of strxxx functions.  They are also not the most *
 *  efficient way to perform their tasks, but hopefully should be in the   *
 *  easiest possible way to install and begin using. Documentation for     *
 *  such installation can be found in INSTALL.  Enjoy...         N'Atas-Ha *
 ***************************************************************************/

#include <cstring>

#include "argument.hh"
#include "affect/Affect.hh"
#include "Area.hh"
#include "Character.hh"
#include "declare.hh"
#include "Descriptor.hh"
#include "Flags.hh"
#include "GameTime.hh"
#include "Logging.hh"
#include "memory.hh"
#include "merc.hh"
#include "MobilePrototype.hh"
#include "MobProg.hh"
#include "MobProgActList.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"
#include "ObjectValue.hh"
#include "Player.hh"
#include "random.hh"
#include "Room.hh"
#include "String.hh"
#include "World.hh"

bool MOBtrigger;

/*
 * Local function prototypes
 */

char *mprog_next_command  args((char *clist));
bool    mprog_seval             args((const String& lhs, const String& opr, const String& rhs));
bool    mprog_veval             args((int lhs, const String& opr, int rhs));
bool    mprog_do_ifchck         args((const String& ifchck, Character *mob,
                                      Character *actor, Object *obj,
                                      void *vo, Character *rndm));
char *mprog_process_if  args((const String& ifchck, char *com_list,
                                      Character *mob, Character *actor,
                                      Object *obj, void *vo,
                                      Character *rndm));
String  mprog_translate         args((char ch, Character *mob,
                                      Character *actor, Object *obj,
                                      void *vo, Character *rndm));
void    mprog_process_cmnd      args((const String& cmnd, Character *mob,
                                      Character *actor, Object *obj,
                                      void *vo, Character *rndm));
void    mprog_driver            args((const String& com_list, Character *mob,
                                      Character *actor, Object *obj,
                                      void *vo));

/***************************************************************************
 * Local function code and brief comments.
 */

/* Used to get sequential lines of a multi line string (separated by "\n")
 * Thus its like one_argument(), but a trifle different. It is destructive
 * to the multi line string argument, and thus clist must not be shared.
 */
char *mprog_next_command(char *clist)
{
	char *pointer = clist;

	while (*pointer != '\n' && *pointer != '\0')
		pointer++;

	if (*pointer == '\n')
		*pointer++ = '\0';

	return (pointer);
}

/* These two functions do the basic evaluation of ifcheck operators.
 *  It is important to note that the string operations are not what
 *  you probably expect.  Equality is exact and division is substring.
 *  remember that lhs has been stripped of leading space, but can
 *  still have trailing spaces so be careful when editing since:
 *  "guard" and "guard " are not equal.
 */
bool mprog_seval(const String& lhs, const char *opr, const char *rhs)
{
	if (!strcmp(opr, "=="))
		return (bool)(!strcmp(lhs, rhs));

	if (!strcmp(opr, "!="))
		return (bool)(strcmp(lhs, rhs));

	if (!strcmp(opr, "/"))
		return lhs.has_infix(rhs);

	if (!strcmp(opr, "!/"))
		return !lhs.has_infix(rhs);

	Logging::bug("Improper MOBprog operator\n", 0);
	return 0;
}

bool mprog_veval(int lhs, const char *opr, int rhs)
{
	if (!strcmp(opr, "=="))
		return (lhs == rhs);

	if (!strcmp(opr, "!="))
		return (lhs != rhs);

	if (!strcmp(opr, ">"))
		return (lhs > rhs);

	if (!strcmp(opr, "<"))
		return (lhs < rhs);

	if (!strcmp(opr, "<="))
		return (lhs <= rhs);

	if (!strcmp(opr, ">="))
		return (lhs >= rhs);

	if (!strcmp(opr, "&"))
		return (lhs & rhs);

	if (!strcmp(opr, "|"))
		return (lhs | rhs);

	Logging::bug("Improper MOBprog operator\n", 0);
	return 0;
}

/* This function performs the evaluation of the if checks.  It is
 * here that you can add any ifchecks which you so desire. Hopefully
 * it is clear from what follows how one would go about adding your
 * own. The syntax for an if check is: ifchck ( arg ) [opr val]
 * where the parenthesis are required and the opr and val fields are
 * optional but if one is there then both must be. The spaces are all
 * optional. The evaluation of the opr expressions is farmed out
 * to reduce the redundancy of the mammoth if statement list.
 * If there are errors, then return -1 otherwise return boolean 1,0
 */
bool mprog_do_ifchck(const char *ifchck, Character *mob, Character *actor,
                     Object *obj, void *vo, Character *rndm)
{
	char buf[ MAX_INPUT_LENGTH ];
	char arg[ MAX_INPUT_LENGTH ];
	char opr[ MAX_INPUT_LENGTH ];
	char val[ MAX_INPUT_LENGTH ];
	Character *vict = (Character *) vo;
	Object *v_obj = (Object *) vo;
	char     *bufpt = buf;
	char     *argpt = arg;
	char     *oprpt = opr;
	char     *valpt = val;
	const char     *point = ifchck;
	int       lhsvl;
	int       rhsvl;

	if (*point == '\0') {
		Logging::bugf("Mob: %d null ifchck", mob->pIndexData->vnum);
		return -1;
	}

	/* skip leading spaces */
	while (*point == ' ')
		point++;

	/* get whatever comes before the left paren.. ignore spaces */
	while (*point != '(')
		if (*point == '\0') {
			Logging::bugf("Mob: %d ifchck syntax error", mob->pIndexData->vnum);
			return -1;
		}
		else if (*point == ' ')
			point++;
		else
			*bufpt++ = *point++;

	*bufpt = '\0';
	point++;

	/* get whatever is in between the parens.. ignore spaces */
	while (*point != ')')
		if (*point == '\0') {
			Logging::bugf("Mob: %d ifchck syntax error", mob->pIndexData->vnum);
			return -1;
		}
		else if (*point == ' ')
			point++;
		else
			*argpt++ = *point++;

	*argpt = '\0';
	point++;

	/* check to see if there is an operator */
	while (*point == ' ')
		point++;

	if (*point == '\0') {
		*opr = '\0';
		*val = '\0';
	}
	else { /* there should be an operator and value, so get them */
		while ((*point != ' ') && (!isalnum(*point)))
			if (*point == '\0') {
				Logging::bugf("Mob: %d ifchck operator without value",
				    mob->pIndexData->vnum);
				return -1;
			}
			else
				*oprpt++ = *point++;

		*oprpt = '\0';

		/* finished with operator, skip spaces and then get the value */
		while (*point == ' ')
			point++;

		for (; ;) {
			if ((*point != ' ') && (*point == '\0'))
				break;
			else
				*valpt++ = *point++;
		}

		*valpt = '\0';
	}

	bufpt = buf;
	argpt = arg;
	oprpt = opr;
	valpt = val;

	/* Ok... now buf contains the ifchck, arg contains the inside of the
	 *  parentheses, opr contains an operator if one is present, and val
	 *  has the value if an operator was present.
	 *  So.. basically use if statements and run over all known ifchecks
	 *  Once inside, use the argument and expand the lhs. Then if need be
	 *  send the lhs,opr,rhs off to be evaluated.
	 */

	if (!strcmp(buf, "rand"))
		return (number_percent() <= atoi(arg));

	if (!strcmp(buf, "mudtime")) {
		lhsvl = mob->in_room->area().world.time.hour;
		rhsvl = atoi(val);
		return mprog_veval(lhsvl, opr, rhsvl);
	}

	if (!strcmp(buf, "ispc")) {
		switch (arg[1]) { /* arg should be "$*" so just get the letter */
		case 'i': return 0;

		case 'n': if (actor)                            return (!actor->is_npc());
			else                                  return -1;

		case 't': if (vict)                             return (!vict->is_npc());
			else                                  return -1;

		case 'r': if (rndm)                             return (!rndm->is_npc());
			else                                  return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'ispc'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "isnpc")) {
		switch (arg[1]) { /* arg should be "$*" so just get the letter */
		case 'i':                                       return 1;

		case 'n': if (actor)                            return (actor->is_npc());
			else                                  return -1;

		case 't': if (vict)                             return (vict->is_npc());
			else                                  return -1;

		case 'r': if (rndm)                             return (rndm->is_npc());
			else                                  return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'isnpc'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "isgood")) {
		switch (arg[1]) { /* arg should be "$*" so just get the letter */
		case 'i':                                       return IS_GOOD(mob);

		case 'n': if (actor)                            return IS_GOOD(actor);
			else                                  return -1;

		case 't': if (vict)                             return IS_GOOD(vict);
			else                                  return -1;

		case 'r': if (rndm)                             return IS_GOOD(rndm);
			else                                  return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'isgood'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "isevil")) {
		switch (arg[1]) { /* arg should be "$*" so just get the letter */
		case 'i':                                       return IS_EVIL(mob);

		case 'n': if (actor)                            return IS_EVIL(actor);
			else                                  return -1;

		case 't': if (vict)                             return IS_EVIL(vict);
			else                                  return -1;

		case 'r': if (rndm)                             return IS_EVIL(rndm);
			else                                  return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'isevil'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "isneutral")) {
		switch (arg[1]) { /* arg should be "$*" so just get the letter */
		case 'i':                                       return IS_NEUTRAL(mob);

		case 'n': if (actor)                            return IS_NEUTRAL(actor);
			else                                  return -1;

		case 't': if (vict)                             return IS_NEUTRAL(vict);
			else                                  return -1;

		case 'r': if (rndm)                             return IS_NEUTRAL(rndm);
			else                                  return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'isneutral'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "isfight")) {
		switch (arg[1]) { /* arg should be "$*" so just get the letter */
		case 'i':                                       return (mob->fighting)   ? 1 : 0;

		case 'n': if (actor)                            return (actor->fighting) ? 1 : 0;
			else                                  return -1;

		case 't': if (vict)                             return (vict->fighting)  ? 1 : 0;
			else                                  return -1;

		case 'r': if (rndm)                             return (rndm->fighting)  ? 1 : 0;
			else                                  return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'isfight'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "isimmort")) {
		switch (arg[1]) { /* arg should be "$*" so just get the letter */
//		case 'i':                       return IS_IMMORTAL(mob);  impossible

		case 'n': if (actor)            return IS_IMMORTAL(actor);
			else                  return -1;

		case 't': if (vict)             return IS_IMMORTAL(vict);
			else                  return -1;

		case 'r': if (rndm)             return IS_IMMORTAL(rndm);
			else                  return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'isimmort'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "iskiller")) {
		switch (arg[1]) {  /* arg should be "$*" so just get the letter */
		case 'i': return 0;
			break;

		case 'n': if (actor)
				if (IS_KILLER(actor))
					return 1;
				else
					return 0;
			else return -1;

			break;

		case 't': if (vict)
				if (IS_KILLER(vict))
					return 1;
				else
					return 0;
			else return -1;

			break;

		case 'r': if (rndm)
				if (IS_KILLER(rndm))
					return 1;
				else
					return 0;
			else return -1;

			break;

		default:
			Logging::bugf("Mob: %d bad argument to 'iskiller'", mob->pIndexData->vnum);
			return -1;
			break;
		}
	}

	if (!strcmp(buf, "isthief")) {
		switch (arg[1]) {  /* arg should be "$*" so just get the letter */
		case 'i': return 0;
			break;

		case 'n': if (actor)
				if (IS_THIEF(actor))
					return 1;
				else
					return 0;
			else return -1;

			break;

		case 't': if (vict)
				if (IS_THIEF(vict))
					return 1;
				else
					return 0;
			else return -1;

			break;

		case 'r': if (rndm)
				if (IS_THIEF(rndm))
					return 1;
				else
					return 0;
			else return -1;

			break;

		default:
			Logging::bugf("Mob: %d bad argument to 'isthief'", mob->pIndexData->vnum);
			return -1;
			break;
		}
	}

	if (!strcmp(buf, "ischarmed")) {
		switch (arg[1]) {  /* arg should be "$*" so just get the letter */
		case 'i': return affect::exists_on_char(mob, affect::type::charm_person) ? 1 : 0;

		case 'n': if (actor)
				return affect::exists_on_char(actor, affect::type::charm_person) ? 1 : 0;
			else return -1;

		case 't': if (vict)
				return affect::exists_on_char(vict, affect::type::charm_person) ? 1 : 0;
			else return -1;

		case 'r': if (rndm)
				return affect::exists_on_char(rndm, affect::type::charm_person) ? 1 : 0;
			else return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'ischarmed'",
			    mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "isfollow")) {
		switch (arg[1]) { /* arg should be "$*" so just get the letter */
		case 'i':       return (mob->master != nullptr && mob->master->in_room == mob->in_room);

		case 'n':       if (actor)
				return (actor->master != nullptr
				        && actor->master->in_room == actor->in_room);
			else
				return -1;

		case 't':       if (vict)
				return (vict->master != nullptr
				        && vict->master->in_room == vict->in_room);
			else
				return -1;

		case 'r':       if (rndm)
				return (rndm->master != nullptr
				        && rndm->master->in_room == rndm->in_room);
			else
				return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'isfollow'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "ismaster")) { /* is $ their master? */
		switch (arg[1]) { /* arg should be "$*" so just get the letter */
		case 'i':       return -1;

		case 'n':       if (actor)
				return (mob->master == actor);
			else
				return -1;

		case 't':       if (vict)
				return (mob->master == vict);
			else
				return -1;

		case 'r':       if (rndm)
				return (mob->master == rndm);
			else
				return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'ismaster'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "isleader")) { /* is $ their leader? */
		switch (arg[1]) { /* arg should be "$*" so just get the letter */
		case 'i':       return -1;

		case 'n':       if (actor)
				return (mob->leader == actor);
			else
				return -1;

		case 't':       if (vict)
				return (mob->leader == vict);
			else
				return -1;

		case 'r':       if (rndm)
				return (mob->leader == rndm);
			else
				return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'isleader'", mob->pIndexData->vnum);
			return -1;
		}
	}
#if 0 // TODO: removed affect bits, replace this with looking up sn, but have to do word parsing
	if (!strcmp(buf, "isaffected")) {
		skill::type sn = skill::lookup(arg);

		if (sn <= 0) {
			Logging::bugf("Mob: %d bad skill type '%s' to 'isaffected'", mob->pIndexData->vnum, arg);
		}
		switch (arg[1]) {  /* arg should be "$*" so just get the letter */
		case 'i': return affect_flag_on_char(mob, atoi(arg));

		case 'n': if (actor)
				return affect_flag_on_char(actor, atoi(arg));
			else return -1;

		case 't': if (vict)
				return affect_flag_on_char(vict, atoi(arg));
			else return -1;

		case 'r': if (rndm)
				return affect_flag_on_char(rndm, atoi(arg));
			else return -1;

		default:
			Logging::bug("Mob: %d bad argument '%s' to 'isaffected'",
			    mob->pIndexData->vnum, );
			return -1;
		}
	}
#endif // ifdef 0
	if (!strcmp(buf, "hitprcnt")) {
		switch (arg[1]) {  /* arg should be "$*" so just get the letter */
		case 'i': lhsvl = mob->hit / GET_MAX_HIT(mob);
			rhsvl = atoi(val);
			return mprog_veval(lhsvl, opr, rhsvl);

		case 'n': if (actor) {
				lhsvl = actor->hit / GET_MAX_HIT(actor);
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 't': if (vict) {
				lhsvl = vict->hit / GET_MAX_HIT(vict);
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 'r': if (rndm) {
				lhsvl = rndm->hit / GET_MAX_HIT(rndm);
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'hitprcnt'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "inroom")) {
		switch (arg[1]) {  /* arg should be "$*" so just get the letter */
		case 'i': lhsvl = mob->in_room->location.to_int();
			rhsvl = Location(val).to_int();
			return mprog_veval(lhsvl, opr, rhsvl);

		case 'n': if (actor) {
				lhsvl = actor->in_room->location.to_int();
				rhsvl = Location(val).to_int();
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 't': if (vict) {
				lhsvl = vict->in_room->location.to_int();
				rhsvl = Location(val).to_int();
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 'r': if (rndm) {
				lhsvl = rndm->in_room->location.to_int();
				rhsvl = Location(val).to_int();
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'inroom'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "sex")) {
		switch (arg[1]) {  /* arg should be "$*" so just get the letter */
		case 'i': lhsvl = GET_ATTR_SEX(mob);
			rhsvl = atoi(val);
			return mprog_veval(lhsvl, opr, rhsvl);

		case 'n': if (actor) {
				lhsvl = GET_ATTR_SEX(actor);
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 't': if (vict) {
				lhsvl = GET_ATTR_SEX(vict);
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 'r': if (rndm) {
				lhsvl = GET_ATTR_SEX(rndm);
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'sex'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "position")) {
		switch (arg[1]) {  /* arg should be "$*" so just get the letter */
		case 'i': lhsvl = mob->position;
			rhsvl = atoi(val);
			return mprog_veval(lhsvl, opr, rhsvl);

		case 'n': if (actor) {
				lhsvl = actor->position;
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 't': if (vict) {
				lhsvl = vict->position;
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 'r': if (rndm) {
				lhsvl = rndm->position;
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'position'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "level")) {
		switch (arg[1]) {  /* arg should be "$*" so just get the letter */
		case 'i': lhsvl = mob->level;
			rhsvl = atoi(val);
			return mprog_veval(lhsvl, opr, rhsvl);

		case 'n': if (actor) {
				lhsvl = actor->level;
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 't': if (vict) {
				lhsvl = vict->level;
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 'r': if (rndm) {
				lhsvl = rndm->level;
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'level'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "class")) {
		switch (arg[1]) {  /* arg should be "$*" so just get the letter */
		case 'i': lhsvl = mob->guild;
			rhsvl = atoi(val);
			return mprog_veval(lhsvl, opr, rhsvl);

		case 'n': if (actor) {
				lhsvl = actor->guild;
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 't': if (vict) {
				lhsvl = vict->guild;
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 'r': if (rndm) {
				lhsvl = rndm->guild;
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'class'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "goldamt")) {
		switch (arg[1]) {  /* arg should be "$*" so just get the letter */
		case 'i': lhsvl = mob->gold;
			rhsvl = atoi(val);
			return mprog_veval(lhsvl, opr, rhsvl);

		case 'n': if (actor) {
				lhsvl = actor->gold;
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 't': if (vict) {
				lhsvl = vict->gold;
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 'r': if (rndm) {
				lhsvl = rndm->gold;
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'goldamt'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "objtype")) {
		switch (arg[1]) {  /* arg should be "$*" so just get the letter */
		case 'o': if (obj) {
				lhsvl = obj->item_type;
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 'p': if (v_obj) {
				lhsvl = v_obj->item_type;
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'objtype'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "objval0")) {
		switch (arg[1]) {  /* arg should be "$*" so just get the letter */
		case 'o': if (obj) {
				lhsvl = obj->value[0];
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 'p': if (v_obj) {
				lhsvl = v_obj->value[0];
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'objval0'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "objval1")) {
		switch (arg[1]) {  /* arg should be "$*" so just get the letter */
		case 'o': if (obj) {
				lhsvl = obj->value[1];
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 'p': if (v_obj) {
				lhsvl = v_obj->value[1];
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'objval1'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "objval2")) {
		switch (arg[1]) {  /* arg should be "$*" so just get the letter */
		case 'o': if (obj) {
				lhsvl = obj->value[2];
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 'p': if (v_obj) {
				lhsvl = v_obj->value[2];
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'objval2'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "objval3")) {
		switch (arg[1]) {  /* arg should be "$*" so just get the letter */
		case 'o': if (obj) {
				lhsvl = obj->value[3];
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 'p': if (v_obj) {
				lhsvl = v_obj->value[3];
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'objval3'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "number")) {
		switch (arg[1]) {  /* arg should be "$*" so just get the letter */
		case 'i': lhsvl = mob->gold;
			rhsvl = atoi(val);
			return mprog_veval(lhsvl, opr, rhsvl);

		case 'n': if (actor) {
				if (actor->is_npc()) {
					lhsvl = actor->pIndexData->vnum.value();
					rhsvl = atoi(val);
					return mprog_veval(lhsvl, opr, rhsvl);
				}
			}
			else
				return -1;

		case 't': if (vict) {
				if (actor->is_npc()) {
					lhsvl = vict->pIndexData->vnum.value();
					rhsvl = atoi(val);
					return mprog_veval(lhsvl, opr, rhsvl);
				}
			}
			else
				return -1;

		case 'r': if (rndm) {
				if (actor->is_npc()) {
					lhsvl = rndm->pIndexData->vnum.value();
					rhsvl = atoi(val);
					return mprog_veval(lhsvl, opr, rhsvl);
				}
			}
			else return -1;

		case 'o': if (obj) {
				lhsvl = obj->pIndexData->vnum.value();
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		case 'p': if (v_obj) {
				lhsvl = v_obj->pIndexData->vnum.value();
				rhsvl = atoi(val);
				return mprog_veval(lhsvl, opr, rhsvl);
			}
			else
				return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'number'", mob->pIndexData->vnum);
			return -1;
		}
	}

	if (!strcmp(buf, "name")) {
		switch (arg[1]) {  /* arg should be "$*" so just get the letter */
		case 'i': return mprog_seval(mob->name, opr, val);

		case 'n': if (actor)
				return mprog_seval(actor->name, opr, val);
			else
				return -1;

		case 't': if (vict)
				return mprog_seval(vict->name, opr, val);
			else
				return -1;

		case 'r': if (rndm)
				return mprog_seval(rndm->name, opr, val);
			else
				return -1;

		case 'o': if (obj)
				return mprog_seval(obj->name, opr, val);
			else
				return -1;

		case 'p': if (v_obj)
				return mprog_seval(v_obj->name, opr, val);
			else
				return -1;

		default:
			Logging::bugf("Mob: %d bad argument to 'name'", mob->pIndexData->vnum);
			return -1;
		}
	}

	/* Ok... all the ifchcks are done, so if we didnt find ours then something
	 * odd happened.  So report the bug and abort the MOBprogram (return error)
	 */
	Logging::bugf("Mob: %d unknown ifchck", mob->pIndexData->vnum);
	return -1;
}
/* Quite a long and arduous function, this guy handles the control
 * flow part of MOBprograms.  Basicially once the driver sees an
 * 'if' attention shifts to here.  While many syntax errors are
 * caught, some will still get through due to the handling of break
 * and errors in the same fashion.  The desire to break out of the
 * recursion without catastrophe in the event of a mis-parse was
 * believed to be high. Thus, if an error is found, it is bugged and
 * the parser acts as though a break were issued and just bails out
 * at that point. I havent tested all the possibilites, so I'm speaking
 * in theory, but it is 'guaranteed' to work on syntactically correct
 * MOBprograms, so if the mud crashes here, check the mob carefully!
 */
/* Yesterday, this routine *did* crash on a syntactically correct
 * mobprog. com_list was nullptr, and yet its value was being checked.
 * While the code is terriffic in general, I think the original author
 * was a bit fuzzy on the difference between nullptr and an empty string.
 * Possibly all the return '\0's should be changed to return "".
 * -- Elrac
 */
char *mprog_process_if(const char *ifchck, char *com_list, Character *mob,
                       Character *actor, Object *obj, void *vo,
                       Character *rndm)
{
	const char *morebuf = nullptr;
	char    *cmnd = nullptr;
	bool loopdone = false;
	bool     flag = false;
	int  legal;

	/* check for trueness of the ifcheck */
	if ((legal = mprog_do_ifchck(ifchck, mob, actor, obj, vo, rndm))) {
		if (legal == 1)
			flag = true;
		else
			return nullptr;
	}

	String buf;

	while (loopdone == false) { /*scan over any existing or statements */
		cmnd     = com_list;
		com_list = mprog_next_command(com_list);

		while (*cmnd == ' ')
			cmnd++;

		if (*cmnd == '\0') {
			Logging::bugf("Mob: %d no commands after IF/OR", mob->pIndexData->vnum);
			return nullptr;
		}

		morebuf = one_argument(cmnd, buf);

		if (!strcmp(buf, "or")) {
			if ((legal = mprog_do_ifchck(morebuf, mob, actor, obj, vo, rndm))) {
				if (legal == 1)
					flag = true;
				else
					return nullptr;
			}
		}
		else
			loopdone = true;
	}

	if (flag)
		for (; ;) { /*ifcheck was true, do commands but ignore else to endif*/
			if (!strcmp(buf, "if")) {
				com_list = mprog_process_if(morebuf, com_list, mob, actor, obj, vo, rndm);

				while (*cmnd == ' ')
					cmnd++;

				if (com_list == nullptr || *com_list == '\0')
					return nullptr;

				cmnd     = com_list;
				com_list = mprog_next_command(com_list);
				morebuf  = one_argument(cmnd, buf);
				continue;
			}

			if (!strcmp(buf, "break"))
				return nullptr;

			if (!strcmp(buf, "endif"))
				return com_list;

			if (!strcmp(buf, "else")) {
				while (strcmp(buf, "endif")) {
					cmnd     = com_list;
					com_list = mprog_next_command(com_list);

					while (*cmnd == ' ')
						cmnd++;

					if (*cmnd == '\0') {
						Logging::bugf("Mob: %d missing endif after else",
						    mob->pIndexData->vnum);
						return nullptr;
					}

					morebuf = one_argument(cmnd, buf);
				}

				return com_list;
			}

			mprog_process_cmnd(cmnd, mob, actor, obj, vo, rndm);

			if (mob->is_garbage())
				return nullptr;

			cmnd     = com_list;
			com_list = mprog_next_command(com_list);

			while (*cmnd == ' ')
				cmnd++;

			if (*cmnd == '\0') {
				Logging::bugf("Mob: %d missing else or endif", mob->pIndexData->vnum);
				return nullptr;
			}

			morebuf = one_argument(cmnd, buf);
		}
	else { /*false ifcheck, find else and do existing commands or quit at endif*/
		while ((strcmp(buf, "else")) && (strcmp(buf, "endif"))) {
			cmnd     = com_list;
			com_list = mprog_next_command(com_list);

			while (*cmnd == ' ')
				cmnd++;

			if (*cmnd == '\0') {
				Logging::bugf("Mob: %d missing an else or endif",
				    mob->pIndexData->vnum);
				return nullptr;
			}

			morebuf = one_argument(cmnd, buf);
		}

		/* found either an else or an endif.. act accordingly */
		if (!strcmp(buf, "endif"))
			return com_list;

		cmnd     = com_list;
		com_list = mprog_next_command(com_list);

		while (*cmnd == ' ')
			cmnd++;

		if (*cmnd == '\0') {
			Logging::bugf("Mob: %d missing endif", mob->pIndexData->vnum);
			return nullptr;
		}

		morebuf = one_argument(cmnd, buf);

		for (; ;) { /*process the post-else commands until an endif is found.*/
			if (!strcmp(buf, "if")) {
				com_list = mprog_process_if(morebuf, com_list, mob, actor,
				                            obj, vo, rndm);

				while (*cmnd == ' ')
					cmnd++;

				if (com_list == nullptr || *com_list == '\0')
					return nullptr;

				cmnd     = com_list;
				com_list = mprog_next_command(com_list);
				morebuf  = one_argument(cmnd, buf);
				continue;
			}

			if (!strcmp(buf, "else")) {
				Logging::bugf("Mob: %d found else in an else section",
				    mob->pIndexData->vnum);
				return nullptr;
			}

			if (!strcmp(buf, "break"))
				return nullptr;

			if (!strcmp(buf, "endif"))
				return com_list;

			mprog_process_cmnd(cmnd, mob, actor, obj, vo, rndm);

			if (mob->is_garbage())
				return nullptr;

			cmnd     = com_list;
			com_list = mprog_next_command(com_list);

			while (*cmnd == ' ')
				cmnd++;

			if (*cmnd == '\0') {
				Logging::bugf("Mob:%d missing endif in else section",
				    mob->pIndexData->vnum);
				return nullptr;
			}

			morebuf = one_argument(cmnd, buf);
		}
	}
}

/* This routine handles the variables for command expansion.
 * If you want to add any go right ahead, it should be fairly
 * clear how it is done and they are quite easy to do, so you
 * can be as creative as you want. The only catch is to check
 * that your variables exist before you use them. At the moment,
 * using $t when the secondary target refers to an object
 * i.e. >prog_act drops~<nl>if ispc($t)<nl>sigh<nl>endif<nl>~<nl>
 * probably makes the mud crash (vice versa as well) The cure
 * would be to change act() so that vo becomes vict & v_obj.
 * but this would require a lot of small changes all over the code.
 */
String mprog_translate(char ch, Character *mob, Character *actor,
                     Object *obj, void *vo, Character *rndm)
{
	static char *he_she        [] = { "it",  "he",  "she" };
	static char *him_her       [] = { "it",  "him", "her" };
	static char *his_her       [] = { "its", "his", "her" };
	Character   *vict             = (Character *) vo;
	Object    *v_obj            = (Object *) vo;
	String t;

	switch (ch) {
	case 'i':
		t = mob->name.lsplit();
		break;

	case 'I':
		t = mob->short_descr;
		break;

	case 'n':
		if (actor) {
			if (can_see_char(mob, actor))
				t = actor->name;

			if (!actor->is_npc())
				t[0] = toupper(t[0]);
		}

		break;

	case 'N':
		if (actor) {
			if (can_see_char(mob, actor)) {
				if (actor->is_npc())
					t = actor->short_descr;
				else {
					t = actor->name;
					t += " ";
					t += actor->pcdata->title;
				}
			}
			else
				t = "someone";
		}

		break;

	case 't':
		if (vict) {
			if (can_see_char(mob, vict))
				t = vict->name;

			if (!vict->is_npc())
				t[0] = toupper(t[0]);
		}

		break;

	case 'T':
		if (vict) {
			if (can_see_char(mob, vict)) {
				if (vict->is_npc())
					t = vict->short_descr;
				else {
					t = vict->name;
					t += " ";
					t += vict->pcdata->title;
				}
			}
			else
				t = "someone";
		}

		break;

	case 'r':
		if (rndm) {
			if (can_see_char(mob, rndm))
				t = rndm->name.lsplit();
			else
				t = "someone";

			if (!rndm->is_npc())
				t[0] = toupper(t[0]);
		}

		break;

	case 'R':
		if (rndm) {
			if (can_see_char(mob, rndm)) {
				if (rndm->is_npc())
					t = rndm->short_descr;
				else {
					t = rndm->name;
					t += " ";
					t += rndm->pcdata->title;
				}
			}
			else
				t = "someone";
		}

		break;

	case 'e':
		if (actor)
			t = can_see_char(mob, actor) ? he_she[GET_ATTR_SEX(actor)] : "someone";

		break;

	case 'm':
		if (actor)
			t = can_see_char(mob, actor) ? him_her[GET_ATTR_SEX(actor)] : "someone";

		break;

	case 's':
		if (actor)
			t = can_see_char(mob, actor) ? his_her[GET_ATTR_SEX(actor)] : "someone's";

		break;

	case 'E':
		if (vict)
			t = can_see_char(mob, vict) ? he_she[GET_ATTR_SEX(vict)] : "someone";

		break;

	case 'M':
		if (vict)
			t = can_see_char(mob, vict) ? him_her[GET_ATTR_SEX(vict)] : "someone";

		break;

	case 'S':
		if (vict)
			t = can_see_char(mob, vict) ? his_her[GET_ATTR_SEX(vict)] : "someone's";

		break;

	case 'j':
		t = he_she[GET_ATTR_SEX(mob)];
		break;

	case 'k':
		t = him_her[GET_ATTR_SEX(mob)];
		break;

	case 'l':
		t = his_her[GET_ATTR_SEX(mob)];
		break;

	case 'J':
		if (rndm)
			t = can_see_char(mob, rndm) ? he_she[GET_ATTR_SEX(rndm)] : "someone";

		break;

	case 'K':
		if (rndm)
			t = can_see_char(mob, rndm) ? him_her[GET_ATTR_SEX(rndm)] : "someone";

		break;

	case 'L':
		if (rndm)
			t = can_see_char(mob, rndm) ? his_her[GET_ATTR_SEX(rndm)] : "someone's";

		break;

	case 'o':
		if (obj)
			t = can_see_obj(mob, obj) ? String(obj->name).lsplit() : "something";

		break;

	case 'O':
		if (obj)
			t = can_see_obj(mob, obj) ? obj->short_descr : "something";

		break;

	case 'p':
		if (v_obj)
			t = can_see_obj(mob, v_obj) ? String(v_obj->name).lsplit() : "something";

		break;

	case 'P':
		if (v_obj)
			t = can_see_obj(mob, v_obj) ? v_obj->short_descr : "something";

		break;

	case 'a':
		if (obj)
			switch (tolower(obj->name[0])) {
			case 'a': case 'e': case 'i':
			case 'o': case 'u': t = "an";
				break;

			default: t = "a";
			}

		break;

	case 'A':
		if (v_obj)
			switch (tolower(v_obj->name[0])) {
			case 'a': case 'e': case 'i':
			case 'o': case 'u': t = "an";
				break;

			default: t = "a";
			}

		break;

	case '$':
		t = "$";
		break;

	default:
		Logging::bugf("Mob: %d bad $var", mob->pIndexData->vnum);
		break;
	}

	return t;
}

/* This procedure simply copies the cmnd to a buffer while expanding
 * any variables by calling the translate procedure.  The observant
 * code scrutinizer will notice that this is taken from act()
 */
void mprog_process_cmnd(const String& cmnd, Character *mob, Character *actor,
                        Object *obj, void *vo, Character *rndm)
{
	String buf;
	const char *str = cmnd.c_str();

	while (*str != '\0') {
		if (*str != '$') {
			buf += *str++;
			continue;
		}

		str++;
		buf += mprog_translate(*str, mob, actor, obj, vo, rndm);
		str++;
	}

	interpret(mob, buf);
	return;
}

/* The main focus of the MOBprograms.  This routine is called
 *  whenever a trigger is successful.  It is responsible for parsing
 *  the command list and figuring out what to do. However, like all
 *  complex procedures, everything is farmed out to the other guys.
 */
void mprog_driver(const String& com_list, Character *mob, Character *actor,
                  Object *obj, void *vo)
{
	char tmpcmndlst[ MAX_STRING_LENGTH ];
	const char *morebuf;
	char *command_list;
	char *cmnd;
	Character *rndm  = nullptr;
	Character *vch   = nullptr;
	int        count = 0;

	/*    if affect::exists_on_char( mob, affect::type::charm_person )
	        return;                                 why? :P  -- Montrey */

	/* get a random visable mortal player who is in the room with the mob */
	for (vch = mob->in_room->people; vch; vch = vch->next_in_room) {
		if (!vch->is_npc()
		    &&  !IS_IMMORTAL(vch)
		    &&  can_see_char(mob, vch)) {
			if (number_range(0, count) == 0)
				rndm = vch;

			count++;
		}
	}

	strcpy(tmpcmndlst, com_list);
	command_list = tmpcmndlst;
	cmnd         = command_list;
	command_list = mprog_next_command(command_list);

	char buf[MSL];

	while (*cmnd != '\0') {
		morebuf = one_argument(cmnd, buf);

		if (!strcmp(buf, "if")) {
			command_list = mprog_process_if(morebuf, command_list, mob,
			                                actor, obj, vo, rndm);

			if (command_list == nullptr)
				break;
		}
		else {
			mprog_process_cmnd(cmnd, mob, actor, obj, vo, rndm);

			if (mob->is_garbage())
				return;
		}

		cmnd = command_list;
		command_list = mprog_next_command(command_list);
	}
} /* end mprog_driver() */

/***************************************************************************
 * Global function code and brief comments.
 */

/* The next two routines are the basic trigger types. Either trigger
 *  on a certain percent, or trigger on a keyword or word phrase.
 *  To see how this works, look at the various trigger routines..
 */
void mprog_wordlist_check(const String& arg, Character *mob, Character *actor,
                          Object *obj, void *vo, Flags::Bit type)
{
	char        temp1[ MAX_STRING_LENGTH ];
	char        temp2[ MAX_INPUT_LENGTH ];
	const char       *list;
	char       *start;
	char       *dupl;
	char       *end;

	String word;

	for (const auto mprg : mob->pIndexData->mobprogs) {
		if (mob->is_garbage())
			break;

		if (mprg->type == type) {
			strcpy(temp1, mprg->arglist);

			for (unsigned int i = 0; i < strlen(temp1); i++)
				temp1[i] = tolower(temp1[i]);

			list = temp1;
			strcpy(temp2, arg);
			dupl = temp2;

			for (unsigned int i = 0; i < strlen(dupl); i++)
				dupl[i] = tolower(dupl[i]);

			if ((list[0] == 'p') && (list[1] == ' ')) {
				list += 2;

				while ((start = std::strstr(dupl, list)))
					if ((start == dupl || *(start - 1) == ' ')
					    && (*(end = start + strlen(list)) == ' '
					        || *end == '\n'
					        || *end == '\r'
					        || *end == '\0')) {
						mprog_driver(mprg->comlist, mob, actor, obj, vo);
						break;
					}
					else
						dupl = start + 1;
			}
			else {
				list = one_argument(list, word);

				for (; !word.empty(); list = one_argument(list, word))
					while ((start = strstr(dupl, word)))
						if ((start == dupl || *(start - 1) == ' ')
						    && (*(end = start + strlen(word)) == ' '
						        || *end == '\n'
						        || *end == '\r'
						        || *end == '\0')) {
							mprog_driver(mprg->comlist, mob, actor, obj, vo);
							break;
						}
						else
							dupl = start + 1;
			}
		}
	}

	return;
}

void mprog_percent_check(Character *mob, Character *actor, Object *obj,
                         void *vo, Flags::Bit type)
{
	for (const auto mprg : mob->pIndexData->mobprogs) {
		if (mob->is_garbage())
			break;

		if ((mprg->type == type)
		    && (number_percent() < atoi(mprg->arglist))) {
			mprog_driver(mprg->comlist, mob, actor, obj, vo);

			if (type != GREET_PROG && type != ALL_GREET_PROG)
				break;
		}
	}

	return;
}

/* The triggers.. These are really basic, and since most appear only
 * once in the code (hmm. i think they all do) it would be more efficient
 * to substitute the code in and make the mprog_xxx_check routines global.
 * However, they are all here in one nice place at the moment to make it
 * easier to see what they look like. If you do substitute them back in,
 * make sure you remember to modify the variable names to the ones in the
 * trigger calls.
 */
void mprog_act_trigger(const char *buf, Character *mob, Character *ch,
                       Object *obj, void *vo)
{
	if (mob->is_npc()
	    && (mob->pIndexData->progtype_flags.has(ACT_PROG))) {
		MobProgActList *tmp_act = new MobProgActList;

		tmp_act->next	= mob->mpact;
		mob->mpact      = tmp_act;
		mob->mpact->buf = buf;
		mob->mpact->ch  = ch;
		mob->mpact->obj = obj;
		mob->mpact->vo  = vo;
	}

	return;
}

void mprog_bribe_trigger(Character *mob, Character *ch, int amount)
{
	Object *obj;

	if (!mob->is_npc())
		return;

	for (const auto mprg : mob->pIndexData->mobprogs)
		if (mprg->type == BRIBE_PROG) {
			if ((obj = create_money(0, amount)) == nullptr)
				return;

			deduct_cost(mob, amount);

			if (amount >= atoi(mprg->arglist))
				mprog_driver(mprg->comlist, mob, ch, obj, nullptr);

			break;
		}
}

void mprog_death_trigger(Character *mob)
{
	if (mob->is_npc()
	    && (mob->pIndexData->progtype_flags.has(DEATH_PROG)))
		mprog_percent_check(mob, nullptr, nullptr, nullptr, DEATH_PROG);

	death_cry(mob);
	return;
}

void mprog_entry_trigger(Character *mob)
{
	if (mob->is_npc()
	    && (mob->pIndexData->progtype_flags.has(ENTRY_PROG)))
		mprog_percent_check(mob, nullptr, nullptr, nullptr, ENTRY_PROG);

	return;
}

void mprog_fight_trigger(Character *mob, Character *ch)
{
	if (mob->is_npc()
	    && (mob->pIndexData->progtype_flags.has(FIGHT_PROG)))
		mprog_percent_check(mob, ch, nullptr, nullptr, FIGHT_PROG);

	return;
}

void mprog_buy_trigger(Character *mob, Character *ch)
{
	if (mob->is_npc()
	    && (mob->pIndexData->progtype_flags.has(BUY_PROG)))
		mprog_percent_check(mob, ch, nullptr, nullptr, BUY_PROG);

	return;
}

void mprog_give_trigger(Character *mob, Character *ch, Object *obj)
{
	String buf;

	if (mob->is_npc()
	    && (mob->pIndexData->progtype_flags.has(GIVE_PROG)))
		for (const auto mprg : mob->pIndexData->mobprogs) {
			one_argument(mprg->arglist, buf);

			if ((mprg->type == GIVE_PROG)
			    && ((!strcmp(obj->name, mprg->arglist))
			        || (!strcmp("all", buf)))) {
				mprog_driver(mprg->comlist, mob, ch, obj, nullptr);
				break;
			}
		}

	return;
}

void mprog_greet_trigger(Character *ch)
{
	Character *vmob;

	for (vmob = ch->in_room->people; vmob != nullptr; vmob = vmob->next_in_room) {
		if (ch->is_garbage())
			break;

		if (vmob->is_npc()
		    && ch != vmob
		    && can_see_char(vmob, ch)
		    && (vmob->fighting == nullptr)
		    && IS_AWAKE(vmob)
		    && (vmob->pIndexData->progtype_flags.has(GREET_PROG)))
			mprog_percent_check(vmob, ch, nullptr, nullptr, GREET_PROG);
		else if (vmob->is_npc()
		         && ch != vmob
		         && can_see_char(vmob, ch)
		         && (vmob->fighting == nullptr)
		         && IS_AWAKE(vmob)
		         && (vmob->pIndexData->progtype_flags.has(ALL_GREET_PROG)))
			mprog_percent_check(vmob, ch, nullptr, nullptr, ALL_GREET_PROG);
	}

	return;
}

void mprog_hitprcnt_trigger(Character *mob, Character *ch)
{
	if (mob->is_npc()
	    && (mob->pIndexData->progtype_flags.has(HITPRCNT_PROG)))
		for (const auto mprg : mob->pIndexData->mobprogs)
			if ((mprg->type == HITPRCNT_PROG)
			    && ((100 * mob->hit / GET_MAX_HIT(mob)) < atoi(mprg->arglist))) {
				mprog_driver(mprg->comlist, mob, ch, nullptr, nullptr);
				break;
			}

	return;
}

void mprog_boot_trigger(Character *mob)
{
	if (mob->pIndexData->progtype_flags.has(BOOT_PROG))
		mprog_percent_check(mob, nullptr, nullptr, nullptr, BOOT_PROG);

	return;
}

void mprog_random_trigger(Character *mob)
{
	if (mob->pIndexData->progtype_flags.has(RAND_PROG))
		mprog_percent_check(mob, nullptr, nullptr, nullptr, RAND_PROG);

	return;
}

void mprog_random_area_trigger(Character *mob)
{
	if (!mob->pIndexData->progtype_flags.has(RAND_AREA_PROG))
		return;

	// this is static to avoid creating the object every time, make sure to clear it below
	static std::set<Room *> rooms;

	// build a set of all rooms in the area that have players
	for (Descriptor *d = descriptor_list; d; d = d->next) {
		if (d->is_playing()
		 && d->character->in_room
		 && d->character->in_room->area() == mob->in_room->area())
			rooms.emplace(d->character->in_room);
	}

	if (rooms.empty())
		return;

	Room *orig_room = mob->in_room;
	char_from_room(mob);

	for (Room *room : rooms) {
		char_to_room(mob, room);
		mprog_percent_check(mob, nullptr, nullptr, nullptr, RAND_AREA_PROG);
		char_from_room(mob);

		if (mob->is_garbage()) // got killed by something?
			break;
	}

	if (!mob->is_garbage())
		char_to_room(mob, orig_room);

	rooms.clear();
}

void mprog_tick_trigger(Character *mob)    /* Montrey */
{
	if (mob->pIndexData->progtype_flags.has(TICK_PROG))
		mprog_percent_check(mob, nullptr, nullptr, nullptr, TICK_PROG);

	return;
}

void mprog_speech_trigger(const String& txt, Character *mob)
{
	Character *vmob;

	for (vmob = mob->in_room->people; vmob != nullptr; vmob = vmob->next_in_room) {
		if (mob->is_garbage())
			break;

		if (vmob->is_npc() && (vmob->pIndexData->progtype_flags.has(SPEECH_PROG)))
			mprog_wordlist_check(txt.c_str(), vmob, mob, nullptr, nullptr, SPEECH_PROG);
	}

	return;
}

