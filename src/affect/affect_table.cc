#include "affect/Affect.hh"
#include "Logging.hh"

namespace affect {

const table_entry& lookup(Type type) {
	auto pair = affect_table.find(type);

	if (pair == affect_table.cend()) {
		Logging::bugf("affect lookup: type %d not found", type);
		return affect_table.find(Type::unknown)->second;
	}

	return pair->second;
}

const Type lookup(const String& name) {
	for (auto pair : affect_table)
		if (name.is_prefix_of(pair.second.name))
			return pair.first;

	return unknown;
}

const std::map<Type, const table_entry> affect_table = {
	// reserved entries
	{ unknown,           { "unknown",              "",                     "" }},
	{ none,              { "reserved",             "",                     "" }},

	// real entries
	{ acid_breath,       { "acid breath",          "!Acid Breath!",        "" }},
	{ age,               { "age",                  "You feel younger.",    "" }},
	{ armor,             { "armor",                "You feel less armored.",       "" }},
	{ bless,             { "bless",                "You feel less righteous.",     "$p's holy aura fades." }},
	{ blindness,         { "blindness",            "You can see again.",   "" }},
	{ blood_blade,       { "blood blade",          "!Blood Blade!",        "$p is no longer a bloodthirsty weapon." }},
	{ blood_moon,        { "blood moon",           "You feel less bloodthirsty.",  "$p's evil aura fades." }},
	{ calm,              { "calm",                 "You have lost your peace of mind.",    "" }},
	{ change_sex,        { "change sex",           "Your body feels familiar again.",      "" }},
	{ channel,           { "channel",              "You feel revived.",    "" }},
	{ charm_person,      { "charm person",         "You feel more self-confident.",        "" }},
	{ chill_touch,       { "chill touch",          "You feel less cold.",  "" }},
	{ curse,             { "curse",                "The curse wears off.",         "$p is no longer impure." }},
	{ darkness,          { "darkness",             "",     "The room is no longer so dark." }},
	{ dazzling_light,    { "dazzling light",       "!Dazzling Light!",     "$p's light seems less penetrating." }},
	{ detect_evil,       { "detect evil",          "The red in your vision disappears.",   "" }},
	{ detect_good,       { "detect good",          "The gold in your vision disappears.",  "" }},
	{ detect_hidden,     { "detect hidden",        "You feel less aware of your surroundings.",    "" }},
	{ detect_invis,      { "detect invis",         "You no longer see the invisible.",     "" }},
	{ detect_magic,      { "detect magic",         "The detect magic wears off.",  "" }},
	{ divine_regeneration, { "divine regeneration",  "You no longer feel so vibrant.",       "" }},
	{ enchant_armor,     { "enchant armor",        "!Enchant Armor!",      "" }},
	{ enchant_weapon,    { "enchant weapon",       "!Enchant Weapon!",     "" }},
	{ faerie_fire,       { "faerie fire",          "The pink aura around you fades away.", "" }},
	{ fear,              { "fear",                 "You are no longer afraid.",    "$p is no longer afraid." }},
	{ fire_breath,       { "fire breath",          "The smoke leaves your eyes.",  "" }},
	{ fireproof,         { "fireproof",            "",     "$p's protective aura fades." }},
	{ flame_blade,       { "flame blade",          "!Flame Blade!",        "$p's flame diminishes." }},
	{ flameshield,       { "flameshield",          "The circle of flames around your body dissipates.", "$p's flameshield dissipates." }},
	{ fly,               { "fly",                  "You slowly float to the ground.",      "" }},
	{ frenzy,            { "frenzy",               "Your rage ebbs.",      "" }},
	{ frost_blade,       { "frost blade",          "!Frost Blade!",        "$p is no longer so cold to the touch." }},
	{ giant_strength,    { "giant strength",       "You feel weaker.",     "" }},
	{ haste,             { "haste",                "You feel yourself slow down.", "" }},
	{ invis,             { "invisibility",         "You are no longer invisible.", "$p fades into view." }},
	{ mass_invis,        { "mass invis",           "You are no longer invisible.", "" }},
	{ night_vision,      { "infravision",          "You no longer see in the dark.",       "" }},
	{ pass_door,         { "pass door",            "You feel solid again.",        "" }},
	{ plague,            { "plague",               "Your sores vanish.",   "" }},
	{ poison,            { "poison",               "You feel less sick.",  "The poison on $p dries up." }},
	{ protection_evil,   { "protection evil",      "You feel less protected from evil.",   "" }},
	{ protection_good,   { "protection good",      "You feel less protected from good.",   "" }},
	{ rayban,            { "rayban",               "You blink as your eye protection fades.",   "" }},
	{ regeneration,      { "regeneration",         "You no longer feel so vibrant.",       "" }},
	{ sanctuary,         { "sanctuary",            "The white aura around your body fades.",       "" }},
	{ shield,            { "shield",               "Your force shield shimmers then fades away.",  "" }},
	{ shock_blade,       { "shock blade",          "!Flame Blade!",        "$p's energy fizzles out." }},
	{ sleep,             { "sleep",                "You feel less tired.", "" }},
	{ slow,              { "slow",                 "You feel yourself speed up.",  "" }},
	{ smokescreen,       { "smokescreen",          "Smoke no longer clouds your vision.",  "" }},
	{ steel_mist,        { "steel mist",           "The steel mist fades from your armor.",        "" }},
	{ stone_skin,        { "stone skin",           "Your skin feels soft again.",  "" }},
	{ talon,             { "talon",                "You no longer hold your weapon so tightly.",   "" }},
	{ weaken,            { "weaken",               "You feel stronger.",   "" }},
	{ berserk,           { "berserk",              "You feel your pulse slow down.",       "" }},
	{ dirt_kicking,      { "dirt kicking",         "You rub the dirt out of your eyes.",   "" }},
	{ hide,              { "hide",                 "You are no longer hidden.",    "" }},
	{ meditation,        { "meditation",           "Meditation",           "" }},
	{ sneak,             { "sneak",                "You no longer feel stealthy.", "" }},
	{ sheen,             { "sheen",                "Your armor loses its sheen.",          "" }},
	{ focus,             { "focus",                "Your spells lose their focus.",        "" }},
	{ paralyze,          { "paralyze",             "You can feel your limbs again!",       "" }},
	{ ironskin,          { "ironskin", "Your skin softens considerably.", "" }},
	{ barrier,           { "barrier",              "The barrier around you crumbles.",     "" }},
	{ dazzle,            { "dazzle",               "The spots in your vision fade.",       "" }},
	{ midnight,          { "midnight",             "You step out of the shadows.",         "" }},
	{ shadow_form,       { "shadow form",          "!Shadow Form!",        "" }},
	{ hex,               { "hex",                  "You feel again the warmth of light.",  "" }},
	{ bone_wall,         { "bone wall",            "The swirling bones fall to the ground.", "" }},
	{ hammerstrike,      { "hammerstrike",         "The power of the gods has left you.",  "" }},
	{ force_shield,      { "force shield",         "Your mystical aura fades.",            "" }},
};

} // namespace affect
