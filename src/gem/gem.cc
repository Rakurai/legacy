#include "gem/gem.hh"

#include "act.hh"
#include "affect_list.hh"
#include "argument.hh"
#include "Affect.hh"
#include "Character.hh"
#include "find.hh"
#include "Flags.hh"
#include "Logging.hh"
#include "Object.hh"
#include "ObjectPrototype.hh"
#include "String.hh"

namespace gem {

// Gem Table
const std::vector<type_st> type_table = {
//const struct gem_type_table_t gem_type_table [MAX_GEM_TYPES] = {
//      keyword         color_code	vnum                    apply_loc           modifier[quality]
	{	"ruby",			'P',		OBJ_VNUM_GEM_RUBY,		APPLY_DAMROLL,		{1, 2, 3, 5, 8, 12, 20}  },
	{	"emerald",		'G',		OBJ_VNUM_GEM_EMERALD,	APPLY_HITROLL,		{1, 2, 3, 5, 8, 12, 20}  }
};

// the strings here might not sensibly apply to every type of gem... amber?  other things?
// but this will work for now.  what if the base obj from the area file had an extra desc
// with a space-delimited list of the words?  and just figure it out at runtime.  Later.
const quality_st quality_table [Quality::COUNT] = {
//      keyword         quality         	level
	{	"rough",		Quality::Rough,		1 },
	{	"cracked",		Quality::Cracked,	15 },
	{	"flawed",		Quality::Flawed,	30 },
	{	"flawless",		Quality::Flawless,	45 },
	{	"perfect",		Quality::Perfect,	60 },
	{	"brilliant",	Quality::Brilliant,	75 },
	{	"dazzling",		Quality::Dazzling,	90 }
};

/*
 * Move a gem into an object.
 */
void inset(Object *gem, Object *obj) {
	gem->next_content = obj->gems;
	obj->gems = gem;
	gem->in_obj = obj;
	gem->in_room = nullptr;
	gem->carried_by = nullptr;
}

// populate a short string for display, takes a buffer of size GEM_SHORT_STRING_LEN
const String get_short_string(Object *eq) {
	String buf;

	if (eq->num_settings == 0) {
		return buf;
	}

	char bracket_symbol_open = '[';
	char bracket_symbol_close = ']';
	char bracket_color = 'g';
	char gem_symbol = '*';
	char empty_color = 'c';
	char empty_symbol = '.';

	int count = 0;

	buf += '{';
	buf += bracket_color;
	buf += bracket_symbol_open;

	// gems in the eq
	for (Object *gem = eq->gems; gem; gem = gem->next_content) {
		count++;
		buf += '{';
		buf += type_table[gem->value[0]].color_code;
		buf += gem_symbol;
	}

	// empty settings
	while (count < eq->num_settings) {
		count++;
		buf += '{';
		buf += empty_color;
		buf += empty_symbol;
	}

	buf += '{';
	buf += bracket_color;
	buf += bracket_symbol_close;

	// trailing blank spaces
//	while (count < MAX_GEM_SETTINGS) {
//		count++;
//		buf += '{';
//		buf += 'x';
//		buf += ' ';
//	}

	buf += '{';
	buf += 'x';

	return buf;
}

void compile_effects(Object *eq) {
	if (eq->wear_loc != WEAR_NONE) {
		Logging::bug("gem::compile_effects: eq is worn", 0);
		return;
	}

	// TODO: this needs to be redone
	// blow away affects and rebuild
	affect_clear_list(&eq->gem_affected);

	for (Object *gem = eq->gems; gem != nullptr; gem = gem->next_content) {
		Affect af;
		af.where              = TO_OBJECT;
		af.type               = 0;
		af.level              = gem->level;
		af.duration           = -1;
		af.location           = type_table[gem->value[GEM_VALUE_TYPE]].apply_loc;
		af.modifier           = type_table[gem->value[GEM_VALUE_TYPE]].modifier[gem->value[GEM_VALUE_QUALITY]];
		af.bitvector(0);
		af.evolution          = 1;
		affect_copy_to_list(&eq->gem_affected, &af);
	}
}

} // namespace gem

void do_inset(Character *ch, String argument)
{
	Object *eq, *gem;

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg2 == "in")
		argument = one_argument(argument, arg2);

	if (arg1.empty() || arg2.empty()) {
		stc("Set what gem in what equipment?\n", ch);
		return;
	}

	if ((gem = get_obj_carry(ch, arg1)) == nullptr) {
		act("You don't have a $T.", ch, nullptr, arg1, TO_CHAR);
		return;
	}

	if (gem->pIndexData->item_type != ITEM_GEM) {
		act("That's not a gem you can inset.", ch, nullptr, nullptr, TO_CHAR);
		return;
	}

	if (!can_drop_obj(ch, gem)) {
		act("You can't let go of $p.\n", ch, gem, nullptr, TO_CHAR);
		return;
	}

	if ((eq = get_obj_list(ch, arg2, ch->carrying)) == nullptr) {
		act("You don't have a $T.", ch, nullptr, arg2, TO_CHAR);
		return;
	}

	if (eq->wear_loc != WEAR_NONE) {
		act("You need to remove it first.", ch, nullptr, nullptr, TO_CHAR);
		return;
	}

	int count = 0;
	Object *obj;

	for (obj = eq->gems; obj != nullptr; obj = obj->next_content)
		count++;

	if (count >= eq->num_settings) {
		act("That item doesn't have an empty setting.", ch, nullptr, nullptr, TO_CHAR);
		return;
	}

	if (gem->level > eq->level) {
		act("That item is not powerful enough to hold $p.", ch, gem, nullptr, TO_CHAR);
		return;
	}

	act("$n places $p in an empty setting in $P.", ch, gem, eq, TO_ROOM);
	act("You place $p in an empty setting in $P.", ch, gem, eq, TO_CHAR);

	obj_from_char(gem);

	// this stuff mirrors obj_to_obj
	gem->next_content = eq->gems;
	eq->gems = gem;
	gem->in_obj = eq;
	gem->in_room = nullptr;
	gem->carried_by = nullptr;

	gem::compile_effects(eq);
}
