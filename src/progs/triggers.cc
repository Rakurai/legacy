#include "String.hh"
#include "Character.hh"
#include "Object.hh"
#include "Room.hh"
#include "MobilePrototype.hh"
#include "progs/Prog.hh"
#include "progs/MobProgContext.hh"
#include "progs/MobProgActList.hh"
#include "argument.hh"
#include "random.hh"
#include "merc.hh"
#include "Descriptor.hh"
#include "Area.hh"

namespace progs {

/* The next two routines are the basic trigger types. Either trigger
 *  on a certain percent, or trigger on a keyword or word phrase.
 *  To see how this works, look at the various trigger routines..
 */
void wordlist_check(const String& arg, std::vector<Prog *> proglist, Context& context, Prog::Type type)
{
	char        temp1[ MAX_STRING_LENGTH ];
	char        temp2[ MAX_INPUT_LENGTH ];
	const char       *list;
	char       *start;
	char       *dupl;
	char       *end;

	String word;

	for (const auto prog : proglist) {
		if (context.self_is_garbage())
			break;

		if (prog->type != type)
			continue;

		strcpy(temp1, prog->arglist);

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
					prog->execute(context);
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
						prog->execute(context);
						break;
					}
					else
						dupl = start + 1;
		}
	}

	return;
}

bool percent_check(std::vector<Prog *> proglist, Context& context, Prog::Type type)
{
	for (const auto prog : proglist) {
		if (context.self_is_garbage())
			break;

		if (prog->type != type)
			continue;

		if (number_percent() < atoi(prog->arglist)) {
			prog->execute(context);

			if (type != Prog::Type::GREET_PROG && type != Prog::Type::ALL_GREET_PROG)
				return true;
		}
	}

	return false;
}


/* The triggers.. These are really basic, and since most appear only
 * once in the code (hmm. i think they all do) it would be more efficient
 * to substitute the code in and make the mprog_xxx_check routines global.
 * However, they are all here in one nice place at the moment to make it
 * easier to see what they look like. If you do substitute them back in,
 * make sure you remember to modify the variable names to the ones in the
 * trigger calls.
 */
void act_trigger(const String& buf, Character *mob, Character *ch, Object *obj, void *vo)
{
	if (mob->is_npc()
	    && (mob->pIndexData->progtypes.count(Prog::Type::ACT_PROG))) {
		MobProgContext context(mob, ch, obj, vo);
		MobProgActList *tmp_act = new MobProgActList(buf, context);

		tmp_act->next	= mob->mpact;
		mob->mpact      = tmp_act;
	}

	return;
}

void bribe_trigger(Character *mob, Character *ch, int amount)
{
	Object *obj;

	if (!mob->is_npc())
		return;

	// this object is used for messages in the prog, it doesn't actually go to the mob,
	// they already have the cash
	if ((obj = create_money(0, amount)) == nullptr)
		return;

	for (const auto mprg : mob->pIndexData->mobprogs)
		if (mprg->type == Prog::Type::BRIBE_PROG) {
			if (amount >= atoi(mprg->arglist)) {
				MobProgContext context(mob, ch, obj, nullptr);
				mprg->execute(context);
				break;
			}
		}

	extract_obj(obj);
}

void death_trigger(Character *mob)
{
	if (!mob->is_npc())
		return;

	if (mob->pIndexData->progtypes.count(Prog::Type::DEATH_PROG)) {
		MobProgContext context(mob, nullptr, nullptr, nullptr);
		percent_check(mob->pIndexData->mobprogs, context, Prog::Type::DEATH_PROG);
	}

	if (!mob->is_garbage())
		death_cry(mob);

	return;
}

void entry_trigger(Character *mob)
{
	if (mob->is_npc() && (mob->pIndexData->progtypes.count(Prog::Type::ENTRY_PROG))) {
		MobProgContext context(mob, nullptr, nullptr, nullptr);
		percent_check(mob->pIndexData->mobprogs, context, Prog::Type::ENTRY_PROG);
	}

	return;
}

void fight_trigger(Character *mob, Character *ch)
{
	if (mob->is_npc() && (mob->pIndexData->progtypes.count(Prog::Type::FIGHT_PROG))) {
		MobProgContext context(mob, ch, nullptr, nullptr);
		percent_check(mob->pIndexData->mobprogs, context, Prog::Type::FIGHT_PROG);
	}

	return;
}

void buy_trigger(Character *mob, Character *ch)
{
	if (mob->is_npc() && (mob->pIndexData->progtypes.count(Prog::Type::BUY_PROG))) {
		MobProgContext context(mob, ch, nullptr, nullptr);
		percent_check(mob->pIndexData->mobprogs, context, Prog::Type::BUY_PROG);
	}

	return;
}

void give_trigger(Character *mob, Character *ch, Object *obj)
{
	String buf;

	if (mob->is_npc() && (mob->pIndexData->progtypes.count(Prog::Type::GIVE_PROG)))
		for (const auto mprg : mob->pIndexData->mobprogs) {
			one_argument(mprg->arglist, buf);

			if ((mprg->type == Prog::Type::GIVE_PROG)
			    && ((!strcmp(obj->name, mprg->arglist))
			        || (!strcmp("all", buf)))) {
				MobProgContext context(mob, ch, obj, nullptr);
				mprg->execute(context);
				break;
			}
		}

	return;
}

void greet_trigger(Character *ch)
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
		 && (vmob->pIndexData->progtypes.count(Prog::Type::GREET_PROG))) {
			MobProgContext context(vmob, ch, nullptr, nullptr);
			percent_check(vmob->pIndexData->mobprogs, context, Prog::Type::GREET_PROG);
		}
		else if (vmob->is_npc()
		 && ch != vmob
		 && (vmob->fighting == nullptr)
		 && IS_AWAKE(vmob)
		 && (vmob->pIndexData->progtypes.count(Prog::Type::ALL_GREET_PROG))) {
			MobProgContext context(vmob, ch, nullptr, nullptr);
			percent_check(vmob->pIndexData->mobprogs, context, Prog::Type::ALL_GREET_PROG);
		}
	}

	return;
}

void hitprcnt_trigger(Character *mob, Character *ch)
{
	if (mob->is_npc()
	    && (mob->pIndexData->progtypes.count(Prog::Type::HITPRCNT_PROG)))
		for (const auto mprg : mob->pIndexData->mobprogs)
			if ((mprg->type == Prog::Type::HITPRCNT_PROG)
			    && ((100 * mob->hit / GET_MAX_HIT(mob)) < atoi(mprg->arglist))) {
				MobProgContext context(mob, ch, nullptr, nullptr);
				mprg->execute(context);
				break;
			}

	return;
}

void boot_trigger(Character *mob)
{
	if (mob->pIndexData->progtypes.count(Prog::Type::BOOT_PROG)) {
		MobProgContext context(mob, nullptr, nullptr, nullptr);
		percent_check(mob->pIndexData->mobprogs, context, Prog::Type::BOOT_PROG);
	}

	return;
}

bool random_trigger(Character *mob)
{
	if (mob->pIndexData->progtypes.count(Prog::Type::RAND_PROG)) {
		MobProgContext context(mob, nullptr, nullptr, nullptr);
		return percent_check(mob->pIndexData->mobprogs, context, Prog::Type::RAND_PROG);
	}

	return false;
}

bool random_area_trigger(Character *mob)
{
	if (!mob->pIndexData->progtypes.count(Prog::Type::RAND_AREA_PROG))
		return false;

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
		return false;

	Room *orig_room = mob->in_room;
	char_from_room(mob);
	bool triggered = false;

	MobProgContext context(mob, nullptr, nullptr, nullptr);

	for (Room *room : rooms) {
		char_to_room(mob, room);
		if (percent_check(mob->pIndexData->mobprogs, context, Prog::Type::RAND_AREA_PROG))
			triggered = true;

		char_from_room(mob);

		if (mob->is_garbage()) // got killed by something?
			break;
	}

	if (!mob->is_garbage())
		char_to_room(mob, orig_room);

	rooms.clear();
	return triggered;
}

void tick_trigger(Character *mob)    /* Montrey */
{
	if (mob->pIndexData->progtypes.count(Prog::Type::TICK_PROG)) {
		MobProgContext context(mob, nullptr, nullptr, nullptr);
		percent_check(mob->pIndexData->mobprogs, context, Prog::Type::TICK_PROG);
	}

	return;
}

void speech_trigger(const String& txt, Character *mob)
{
	Character *vmob;

	for (vmob = mob->in_room->people; vmob != nullptr; vmob = vmob->next_in_room) {
		if (mob->is_garbage())
			break;

		if (vmob->is_npc() && (vmob->pIndexData->progtypes.count(Prog::Type::SPEECH_PROG))) {
			MobProgContext context(vmob, mob, nullptr, nullptr);
			wordlist_check(
				txt,
				vmob->pIndexData->mobprogs,
				context,
				Prog::Type::SPEECH_PROG
			);
		}
	}

	return;
}

void control_trigger(Character *mob, const String& key, Character *target) {
	if (mob->is_npc() && mob->pIndexData->progtypes.count(Prog::Type::CONTROL_PROG))
		for (const auto mprg : mob->pIndexData->mobprogs) {
			if (mob->is_garbage())
				break;

			if (mprg->type == Prog::Type::CONTROL_PROG && key == mprg->arglist) {
				MobProgContext context(mob, target, nullptr, nullptr);
				mprg->execute(context);
			}
		}
}

} // namespace progs
