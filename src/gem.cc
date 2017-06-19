#include "gem.hh"

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

// Gem Table
const struct gem_type_table_t gem_type_table [MAX_GEM_TYPES] = {
//      keyword         color_code	vnum                    apply_loc           modifier[quality]
	{	"ruby",			'P',		OBJ_VNUM_GEM_RUBY,		APPLY_DAMROLL,		{1, 2, 3, 5, 8, 12, 20}  },
	{	"emerald",		'G',		OBJ_VNUM_GEM_EMERALD,	APPLY_HITROLL,		{1, 2, 3, 5, 8, 12, 20}  }
};

// the strings here might not sensibly apply to every type of gem... amber?  other things?
// but this will work for now.  what if the base obj from the area file had an extra desc
// with a space-delimited list of the words?  and just figure it out at runtime.  Later.
/*const struct gem_quality_table_t gem_quality_table [MAX_GEM_QUALITIES] = {
//      keyword         quality         level
	{	"rough",		GEM_QUALITY_A,	1 },
	{	"cracked",		GEM_QUALITY_B,	15 },
	{	"flawed",		GEM_QUALITY_C,	30 },
	{	"flawless",		GEM_QUALITY_D,	45 },
	{	"perfect",		GEM_QUALITY_E,	60 },
	{	"brilliant",	GEM_QUALITY_F,	75 },
	{	"dazzling",		GEM_QUALITY_G,	90 }
};
*/
// populate a short string for display, takes a buffer of size GEM_SHORT_STRING_LEN
char *get_gem_short_string(Object *eq) {
	static char buf[MAX_GEM_SETTINGS * 3 + 9];

	if (eq->num_settings == 0) {
		buf[0] = '\0';
		return buf;
	}

	char bracket_symbol_open = '[';
	char bracket_symbol_close = ']';
	char bracket_color = 'g';
	char gem_symbol = '*';
	char empty_color = 'c';
	char empty_symbol = '.';

	int pos = 0;
	int count = 0;

	buf[pos++] = '{';
	buf[pos++] = bracket_color;
	buf[pos++] = bracket_symbol_open;

	// gems in the eq
	for (Object *gem = eq->gems; gem; gem = gem->next_content) {
		count++;
		buf[pos++] = '{';
		buf[pos++] = gem_type_table[gem->value[0]].color_code;
		buf[pos++] = gem_symbol;
	}

	// empty settings
	while (count < eq->num_settings) {
		count++;
		buf[pos++] = '{';
		buf[pos++] = empty_color;
		buf[pos++] = empty_symbol;
	}

	buf[pos++] = '{';
	buf[pos++] = bracket_color;
	buf[pos++] = bracket_symbol_close;

	// trailing blank spaces
//	while (count < MAX_GEM_SETTINGS) {
//		count++;
//		buf[pos++] = '{';
//		buf[pos++] = 'x';
//		buf[pos++] = ' ';
//	}

	buf[pos++] = '{';
	buf[pos++] = 'x';
	buf[pos] = '\0';

	return buf;
}

void compile_gem_effects(Object *eq) {
	Object *gem;

	if (eq->wear_loc != WEAR_NONE) {
		Logging::bug("compile_gem_effects: eq is worn", 0);
		return;
	}

	// TODO: this needs to be redone
	// blow away affects and rebuild
	affect_clear_list(&eq->gem_affected);

	for (gem = eq->gems; gem != nullptr; gem = gem->next_content) {
		Affect af;
		af.where              = TO_OBJECT;
		af.type               = 0;
		af.level              = gem->level;
		af.duration           = -1;
		af.location           = gem_type_table[gem->value[GEM_VALUE_TYPE]].apply_loc;
		af.modifier           = gem_type_table[gem->value[GEM_VALUE_TYPE]].modifier[gem->value[GEM_VALUE_QUALITY]];
		af.bitvector(0);
		af.evolution          = 1;
		affect_copy_to_list(&eq->gem_affected, &af);
	}
}

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

	compile_gem_effects(eq);
}
