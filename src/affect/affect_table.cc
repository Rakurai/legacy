#include "affect/Affect.hh"
#include "Logging.hh"

namespace affect {

const table_entry& lookup(::affect::type type) {
	const auto& pair = affect_table.find(type);

	if (pair == affect_table.cend()) {
		Logging::bugf("affect lookup: type %d not found", type);
		return affect_table.find(::affect::type::unknown)->second;
	}

	return pair->second;
}

const ::affect::type lookup(const String& name) {
	for (const auto& pair : affect_table)
		if (name.is_prefix_of(pair.second.name))
			return pair.first;

	return type::unknown;
}

const std::map<::affect::type, const table_entry> affect_table = {
	// reserved entries
	{ type::unknown,           { "unknown",              "",                     "" }},
	{ type::none,              { "reserved",             "",                     "" }},

	// real entries
	{ type::acid_breath,       { "acid breath",          "!Acid Breath!",        "" }},
	{ type::age,               { "age",                  "You feel younger.",    "" }},
	{ type::armor,             { "armor",                "You feel less armored.",       "" }},
	{ type::bless,             { "bless",                "You feel less righteous.",     "$p's holy aura fades." }},
	{ type::blindness,         { "blindness",            "You can see again.",   "" }},
	{ type::blood_moon,        { "blood moon",           "You feel less bloodthirsty.",  "$p's evil aura fades." }},
	{ type::calm,              { "calm",                 "You have lost your peace of mind.",    "" }},
	{ type::change_sex,        { "change sex",           "Your body feels familiar again.",      "" }},
	{ type::channel,           { "channel",              "You feel revived.",    "" }},
	{ type::charm_person,      { "charm person",         "You feel more self-confident.",        "" }},
	{ type::chill_touch,       { "chill touch",          "You feel less cold.",  "" }},
	{ type::curse,             { "curse",                "The curse wears off.",         "$p is no longer impure." }},
	{ type::darkness,          { "darkness",             "",     "The room is no longer so dark." }},
	{ type::dazzling_light,    { "dazzling light",       "!Dazzling Light!",     "$p's light seems less penetrating." }},
	{ type::detect_evil,       { "detect evil",          "The red in your vision disappears.",   "" }},
	{ type::detect_good,       { "detect good",          "The gold in your vision disappears.",  "" }},
	{ type::detect_hidden,     { "detect hidden",        "You feel less aware of your surroundings.",    "" }},
	{ type::detect_invis,      { "detect invis",         "You no longer see the invisible.",     "" }},
	{ type::detect_magic,      { "detect magic",         "The detect magic wears off.",  "" }},
	{ type::divine_regeneration, { "divine regeneration",  "You no longer feel so vibrant.",       "" }},
	{ type::enchant_armor,     { "enchant armor",        "!Enchant Armor!",      "" }},
	{ type::enchant_weapon,    { "enchant weapon",       "!Enchant Weapon!",     "" }},
	{ type::faerie_fire,       { "faerie fire",          "The pink aura around you fades away.", "" }},
	{ type::fear,              { "fear",                 "You are no longer afraid.",    "$p is no longer afraid." }},
	{ type::fire_breath,       { "fire breath",          "The smoke leaves your eyes.",  "" }},
	{ type::fireproof,         { "fireproof",            "",     "$p's protective aura fades." }},
	{ type::flameshield,       { "flameshield",          "The circle of flames around your body dissipates.", "$p's flameshield dissipates." }},
	{ type::fly,               { "fly",                  "You slowly float to the ground.",      "" }},
	{ type::frenzy,            { "frenzy",               "Your rage ebbs.",      "" }},
	{ type::giant_strength,    { "giant strength",       "You feel weaker.",     "" }},
	{ type::haste,             { "haste",                "You feel yourself slow down.", "" }},
	{ type::invis,             { "invisibility",         "You are no longer invisible.", "$p fades into view." }},
	{ type::mass_invis,        { "mass invis",           "You are no longer invisible.", "" }},
	{ type::night_vision,      { "infravision",          "You no longer see in the dark.",       "" }},
	{ type::pass_door,         { "pass door",            "You feel solid again.",        "" }},
	{ type::plague,            { "plague",               "Your sores vanish.",   "" }},
	{ type::poison,            { "poison",               "You feel less sick.",  "The poison on $p dries up." }},
	{ type::protection_evil,   { "protection evil",      "You feel less protected from evil.",   "" }},
	{ type::protection_good,   { "protection good",      "You feel less protected from good.",   "" }},
	{ type::rayban,            { "rayban",               "You blink as your eye protection fades.",   "" }},
	{ type::regeneration,      { "regeneration",         "You no longer feel so vibrant.",       "" }},
	{ type::sanctuary,         { "sanctuary",            "The white aura around your body fades.",       "" }},
	{ type::shield,            { "shield",               "Your force shield shimmers then fades away.",  "" }},
	{ type::sleep,             { "sleep",                "You feel less tired.", "" }},
	{ type::slow,              { "slow",                 "You feel yourself speed up.",  "" }},
	{ type::smokescreen,       { "smokescreen",          "Smoke no longer clouds your vision.",  "" }},
	{ type::steel_mist,        { "steel mist",           "The steel mist fades from your armor.",        "" }},
	{ type::stone_skin,        { "stone skin",           "Your skin feels soft again.",  "" }},
	{ type::talon,             { "talon",                "You no longer hold your weapon so tightly.",   "" }},
	{ type::weaken,            { "weaken",               "You feel stronger.",   "" }},
	{ type::berserk,           { "berserk",              "You feel your pulse slow down.",       "" }},
	{ type::dirt_kicking,      { "dirt kicking",         "You rub the dirt out of your eyes.",   "" }},
	{ type::hide,              { "hide",                 "You are no longer hidden.",    "" }},
	{ type::meditation,        { "meditation",           "Meditation",           "" }},
	{ type::sneak,             { "sneak",                "You no longer feel stealthy.", "" }},
	{ type::sheen,             { "sheen",                "Your armor loses its sheen.",          "" }},
	{ type::focus,             { "focus",                "Your spells lose their focus.",        "" }},
	{ type::paralyze,          { "paralyze",             "You can feel your limbs again!",       "" }},
	{ type::ironskin,          { "ironskin", "Your skin softens considerably.", "" }},
	{ type::barrier,           { "barrier",              "The barrier around you crumbles.",     "" }},
	{ type::dazzle,            { "dazzle",               "The spots in your vision fade.",       "" }},
	{ type::midnight,          { "midnight",             "You step out of the shadows.",         "" }},
	{ type::shadow_form,       { "shadow form",          "!Shadow Form!",        "" }},
	{ type::hex,               { "hex",                  "You feel again the warmth of light.",  "" }},
	{ type::bone_wall,         { "bone wall",            "The swirling bones fall to the ground.", "" }},
	{ type::hammerstrike,      { "hammerstrike",         "The power of the gods has left you.",  "" }},
	{ type::force_shield,      { "force shield",         "Your mystical aura fades.",            "" }},

	// weapon effects
	{ type::weapon_acidic,     { "acidic",            "",        "The sizzling sound from $p quiets." }},
	{ type::weapon_vampiric,   { "vampiric",          "",        "$p is no longer a bloodthirsty weapon." }},
	{ type::weapon_flaming,    { "flaming",           "",        "$p's flame diminishes." }},
	{ type::weapon_frost,      { "frost",             "",        "$p is no longer so cold to the touch." }},
	{ type::weapon_shocking,   { "shocking",          "",        "$p's energy fizzles out." }},
	{ type::weapon_two_hands,  { "two-handed",        "",        "$p suddenly feels lighter." }},
	{ type::weapon_sharp,      { "sharp",             "",        "$p no longer looks so sharp." }},
	{ type::weapon_vorpal,     { "vorpal",            "",        "$p feels less drawn to severing necks." }},

	// object prefixes
    { type::obj_prefix_solar,		{ "solar",		"",	"" }},
    { type::obj_prefix_astral,	{ "astral",		"",	"" }},
    { type::obj_prefix_lunar,		{ "lunar",		"",	"" }},
    { type::obj_prefix_windy,		{ "windy",		"",	"" }},
    { type::obj_prefix_healthy,	{ "healthy",	"",	"" }},
    { type::obj_prefix_hearty,	{ "hearty",		"",	"" }},
    { type::obj_prefix_stalwart,	{ "stalwart",	"",	"" }},
    { type::obj_prefix_mystical,	{ "mystical",	"",	"" }},
    { type::obj_prefix_magical,	{ "magical",	"",	"" }},
    { type::obj_prefix_lucky,		{ "lucky",		"",	"" }},
    { type::obj_prefix_fortuitous,{ "fortuitous",	"",	"" }},
    { type::obj_prefix_sturdy,	{ "sturdy",		"",	"" }},
    { type::obj_prefix_reinforced,{ "reinforced",	"",	"" }},
    { type::obj_prefix_precise,	{ "precise",	"",	"" }},
    { type::obj_prefix_skilled,	{ "skilled",	"",	"" }},
    { type::obj_prefix_jagged,	{ "jagged",		"",	"" }},
    { type::obj_prefix_brutal,	{ "brutal",		"",	"" }},

    { type::obj_suffix_initiate_questor, { "initiate questor", "", "" }},
    { type::obj_suffix_advanced_questor, { "advanced questor", "", "" }},
    { type::obj_suffix_master_questor, { "master questor", "", "" }},
    { type::obj_suffix_skilled, { "skilled", "", "" }},
    { type::obj_suffix_skillful, { "skillful", "", "" }},
    { type::obj_suffix_experienced, { "experienced", "", "" }},
    { type::obj_suffix_initiate_mage, { "initiate mage", "", "" }},
    { type::obj_suffix_advanced_mage, { "advanced mage", "", "" }},
    { type::obj_suffix_master_mage, { "master mage", "", "" }},
    { type::obj_suffix_swallow, { "swallow", "", "" }},
    { type::obj_suffix_hawk, { "hawk", "", "" }},
    { type::obj_suffix_falcon, { "falcon", "", "" }},
    { type::obj_suffix_devastation, { "devastation", "", "" }},
    { type::obj_suffix_annihilation, { "annihilation", "", "" }},
    { type::obj_suffix_arcane_power, { "arcane power", "", "" }},
    { type::obj_suffix_mystical_power, { "mystical power", "", "" }},
    { type::obj_suffix_hexxing, { "hexxing", "", "" }},

};

} // namespace affect
