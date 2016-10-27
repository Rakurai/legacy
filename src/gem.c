#include "merc.h"
#include "recycle.h"
#include "gem.h"
#include "affect.h"

// Gem Table
const struct gem_type_table_t gem_type_table [MAX_GEM_TYPES] = {
//      keyword         color_code	vnum                    apply_loc           modifier[quality]
	{	"ruby",			'P',		OBJ_VNUM_GEM_RUBY,		APPLY_DAMROLL,		{1, 2, 3, 5, 8, 12, 20}  },
	{	"emerald",		'G',		OBJ_VNUM_GEM_EMERALD,	APPLY_HITROLL,		{1, 2, 3, 5, 8, 12, 20}  }
};

// the strings here might not sensibly apply to every type of gem... amber?  other things?
// but this will work for now.  what if the base obj from the area file had an extra desc
// with a space-delimited list of the words?  and just figure it out at runtime.  Later.
const struct gem_quality_table_t gem_quality_table [MAX_GEM_QUALITIES] = {
//      keyword         quality         level
	{	"rough",		GEM_QUALITY_A,	1 },
	{	"cracked",		GEM_QUALITY_B,	15 },
	{	"flawed",		GEM_QUALITY_C,	30 },
	{	"flawless",		GEM_QUALITY_D,	45 },
	{	"perfect",		GEM_QUALITY_E,	60 },
	{	"brilliant",	GEM_QUALITY_F,	75 },
	{	"dazzling",		GEM_QUALITY_G,	90 }
};

// populate a short string for display, takes a buffer of size GEM_SHORT_STRING_LEN
char *get_gem_short_string(OBJ_DATA *eq) {
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
	for (OBJ_DATA *gem = eq->gems; gem; gem = gem->next_content) {
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

void compile_gem_effects(OBJ_DATA *eq) {
	OBJ_DATA *gem;

	if (eq->wear_loc != WEAR_NONE) {
		bug("compile_gem_effects: eq is worn", 0);
		return;
	}

	// TODO: this needs to be redone
	// blow away affects and rebuild
	while (eq->gem_affected) {
		AFFECT_DATA *t = eq->gem_affected;
		affect_remove_from_list(&eq->gem_affected, t);
		free_affect(t);
	}

	for (gem = eq->gems; gem != NULL; gem = gem->next_content) {
		AFFECT_DATA af;
		af.where              = TO_OBJECT;
		af.type               = -1;
		af.level              = gem->level;
		af.duration           = -1;
		af.location           = gem_type_table[gem->value[GEM_VALUE_TYPE]].apply_loc;
		af.modifier           = gem_type_table[gem->value[GEM_VALUE_TYPE]].modifier[gem->value[GEM_VALUE_QUALITY]];
		af.bitvector          = 0;
		af.evolution          = 1;
		affect_copy_to_list(&eq->gem_affected, &af);
	}
}

void do_inset(CHAR_DATA *ch, const char *argument)
{
	char arg1[MIL], arg2[MIL];
	OBJ_DATA *eq, *gem;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (!str_cmp(arg2, "in"))
		argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		stc("Set what gem in what equipment?\n", ch);
		return;
	}

	if ((gem = get_obj_carry(ch, arg1)) == NULL) {
		act("You don't have a $T.", ch, NULL, arg1, TO_CHAR);
		return;
	}

	if (gem->pIndexData->item_type != ITEM_GEM) {
		act("That's not a gem you can inset.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (!can_drop_obj(ch, gem)) {
		act("You can't let go of $p.\n", ch, gem, NULL, TO_CHAR);
		return;
	}

	if ((eq = get_obj_list(ch, arg2, ch->carrying)) == NULL) {
		act("You don't have a $T.", ch, NULL, arg2, TO_CHAR);
		return;
	}

	if (eq->wear_loc != WEAR_NONE) {
		act("You need to remove it first.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	int count = 0;
	OBJ_DATA *obj;

	for (obj = eq->gems; obj != NULL; obj = obj->next_content)
		count++;

	if (count >= eq->num_settings) {
		act("That item doesn't have an empty setting.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (gem->level > eq->level) {
		act("That item is not powerful enough to hold $p.", ch, gem, NULL, TO_CHAR);
		return;
	}

	act("$n places $p in an empty setting in $P.", ch, gem, eq, TO_ROOM);
	act("You place $p in an empty setting in $P.", ch, gem, eq, TO_CHAR);

	obj_from_char(gem);

	// this stuff mirrors obj_to_obj
	gem->next_content = eq->gems;
	eq->gems = gem;
	gem->in_obj = eq;
	gem->in_room = NULL;
	gem->carried_by = NULL;

	compile_gem_effects(eq);
}
