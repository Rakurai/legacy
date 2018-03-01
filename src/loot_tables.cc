#include <map>
#include <vector>

#include "affect/Affect.hh"
#include "declare.hh"
#include "Flags.hh"
#include "lootv2.hh"
#include "String.hh"

/************************************************************************************
 *                              Begin Tables                                        *
 ************************************************************************************/


const std::multimap<int, String> base_name_table = {
    { ARMOR_LIGHT, "Dim Light" },
    { ARMOR_LIGHT, "Bright Light" },
    { ARMOR_LIGHT, "Brilliant Light" },
    { ARMOR_LIGHT, "Shard of Rose Quartz" },
    { ARMOR_LIGHT, "Blessed Orb" },
    { ARMOR_LIGHT, "Divine Orb" },
    { ARMOR_LIGHT, "Alchemic Orb" },
    { ARMOR_LIGHT, "Chromatic Orb" },
    { ARMOR_LIGHT, "Prismatic Orb" },
    
    { ARMOR_FINGER, "Iron Ring" },
    { ARMOR_FINGER, "Steel Ring" },
    { ARMOR_FINGER, "Sapphire Ring" },
    { ARMOR_FINGER, "Ruby Ring" },
    { ARMOR_FINGER, "Gold Ring" },
    { ARMOR_FINGER, "Two-Stone Ring" },
    { ARMOR_FINGER, "Moonstone Ring" },
    { ARMOR_FINGER, "Diamond Ring" },
    { ARMOR_FINGER, "Prismatic Ring" },
    
    { ARMOR_NECK, "Gold Amulet" },
    { ARMOR_NECK, "Amber Amulet" },
    { ARMOR_NECK, "Turquoise Amulet" },
    { ARMOR_NECK, "Agate Amulet" },
    { ARMOR_NECK, "Citrine Amulet" },
    { ARMOR_NECK, "Onyx Amulet" },
    { ARMOR_NECK, "Ruby Amulet" },
    { ARMOR_NECK, "Chrysalis Talisman" },
    { ARMOR_NECK, "Prismatic Talisman" },
    
    { ARMOR_TORSO, "Padded Vest" },
    { ARMOR_TORSO, "Silken Vest" },
    { ARMOR_TORSO, "Steel Plate" },
    { ARMOR_TORSO, "War Plate" },
    { ARMOR_TORSO, "Scholar's Robe" },
    { ARMOR_TORSO, "Holy Chainmail" },
    { ARMOR_TORSO, "Full Chainmail" },
    { ARMOR_TORSO, "Carnal Armor" },
    { ARMOR_TORSO, "Prismatic Vest" },
    
    { ARMOR_HEAD, "Close Helmet" },
    { ARMOR_HEAD, "Raven Mask" },
    { ARMOR_HEAD, "Noble Tricorne" },
    { ARMOR_HEAD, "Deicide Mask" },
    { ARMOR_HEAD, "Fluted Bascinet" },
    { ARMOR_HEAD, "Hubris Circlet" },
    { ARMOR_HEAD, "Praetor Crown" },
    { ARMOR_HEAD, "Bone Circlet" },
    { ARMOR_HEAD, "Prismatic Mask" },
    
    { ARMOR_LEGS, "Leather Leggings" },
    { ARMOR_LEGS, "Padded Leggings" },
    { ARMOR_LEGS, "War Leggings" },
    { ARMOR_LEGS, "Holy Leggings" },
    { ARMOR_LEGS, "Bone Leggings" },
    { ARMOR_LEGS, "Steel Leggings" },
    { ARMOR_LEGS, "Carnal Leggings" },
    { ARMOR_LEGS, "Ornate Leggings" },
    { ARMOR_LEGS, "Prismatic Leggings" },
    
    { ARMOR_FEET, "Rawhide Boots" },
    { ARMOR_FEET, "Goathide Boots" },
    { ARMOR_FEET, "Clasped Boots" },
    { ARMOR_FEET, "Scholar Boots" },
    { ARMOR_FEET, "Reinforced Greaves" },
    { ARMOR_FEET, "Goliath Boots" },
    { ARMOR_FEET, "Wyrmscale Boots" },
    { ARMOR_FEET, "Arcanist Slippers" },
    { ARMOR_FEET, "Prismatic Boots" },
    
    { ARMOR_HANDS, "Wool Gloves" },
    { ARMOR_HANDS, "Deerskin Gloves" },
    { ARMOR_HANDS, "Embroidered Gloves" },
    { ARMOR_HANDS, "Trapper Mitts" },
    { ARMOR_HANDS, "Steel Gauntlets" },
    { ARMOR_HANDS, "Plated Gauntlets" },
    { ARMOR_HANDS, "Ironscale Gauntlets" },
    { ARMOR_HANDS, "Hydrascale Gauntlets" },
    { ARMOR_HANDS, "Prismatic Silk Gloves" },
    
    { ARMOR_ARMS, "Silk Sleeves" },
    { ARMOR_ARMS, "Rawhide Sleeves" },
    { ARMOR_ARMS, "Scholar's Sleeves" },
    { ARMOR_ARMS, "Occultist's Vestment" },
    { ARMOR_ARMS, "Ornate Armguards" },
    { ARMOR_ARMS, "Necromancer Silks" },
    { ARMOR_ARMS, "Goathide Armguards" },
    { ARMOR_ARMS, "Embroidered Sleeves" },
    { ARMOR_ARMS, "Prismatic Sleeves" },
    
    { ARMOR_SHIELD, "Goathide Buckler" },
    { ARMOR_SHIELD, "Tower Shield" },
    { ARMOR_SHIELD, "Rawhide Tower Shield" },
    { ARMOR_SHIELD, "Painted Buckler" },
    { ARMOR_SHIELD, "Linden Kite Shield" },
    { ARMOR_SHIELD, "Hammered Buckler" },
    { ARMOR_SHIELD, "Ornate Spiked Shield" },
    { ARMOR_SHIELD, "Harmonic Spirit Shield" },
    { ARMOR_SHIELD, "Prismatic Round Shield" },
    
    { ARMOR_BODY, "Tattered Robe" },
    { ARMOR_BODY, "Cloth Robe" },
    { ARMOR_BODY, "Cotton Robe" },
    { ARMOR_BODY, "Embroidered Robe" },
    { ARMOR_BODY, "Blue Robe" },
    { ARMOR_BODY, "Necromancer's Robe" },
    { ARMOR_BODY, "Silver Robe" },
    { ARMOR_BODY, "Ornate Robe" },
    { ARMOR_BODY, "Prismatic Robe" },
    
    { ARMOR_WAIST, "Sash" },
    { ARMOR_WAIST, "Chain Belt" },
    { ARMOR_WAIST, "Leather Belt" },
    { ARMOR_WAIST, "Heavy Belt" },
    { ARMOR_WAIST, "Studded Belt" },
    { ARMOR_WAIST, "Cloth Belt" },
    { ARMOR_WAIST, "Vanguard Belt" },
    { ARMOR_WAIST, "Crystal Belt" },
    { ARMOR_WAIST, "Prismatic Belt" },
    
    { ARMOR_WRIST, "Gold Bracelet" },
    { ARMOR_WRIST, "Amber Bracelet" },
    { ARMOR_WRIST, "Turquoise Bracer" },
    { ARMOR_WRIST, "Agate Bracelet" },
    { ARMOR_WRIST, "Citrine Bracelet" },
    { ARMOR_WRIST, "Onyx Bracer" },
    { ARMOR_WRIST, "Ruby Bracelet" },
    { ARMOR_WRIST, "Chrysalis Bracer" },
    { ARMOR_WRIST, "Prismatic Bracer" },
    
    { ARMOR_FLOATING, "Dim Light" },
    { ARMOR_FLOATING, "Wisp" },
    { ARMOR_FLOATING, "Pixie" },
    { ARMOR_FLOATING, "Blessed Orb" },
    { ARMOR_FLOATING, "Heavy Sigh" },
    { ARMOR_FLOATING, "Muttered Words" },
    { ARMOR_FLOATING, "Crystal Orb" },
    { ARMOR_FLOATING, "Chrysalis Orb" },
    { ARMOR_FLOATING, "Prismatic Orb" },
    
    { ARMOR_HOLD, "Horned Talisman" },
    { ARMOR_HOLD, "Jet Talisman" },
    { ARMOR_HOLD, "Clutching Talisman" },
    { ARMOR_HOLD, "Marble Talisman" },
    { ARMOR_HOLD, "Gold Talisman" },
    { ARMOR_HOLD, "Shadow Talisman" },
    { ARMOR_HOLD, "Chromatic Talisman" },
    { ARMOR_HOLD, "Chrysalis Talisman" },
    { ARMOR_HOLD, "Prismatic Talisman" },

    { WIELD_AXE, "Rusted Hatchet" },
    { WIELD_AXE, "Spectral Axe" },
    { WIELD_AXE, "Wraith Axe" },
    { WIELD_AXE, "Siege Axe" },
    { WIELD_AXE, "Infernal Axe" },
    { WIELD_AXE, "Wrist Chopper" },
    { WIELD_AXE, "Chest Splitter" },
    { WIELD_AXE, "Shadow Axe" },
    { WIELD_AXE, "Timber Axe" },

    { WIELD_DAGGER, "Glass Shank" },
    { WIELD_DAGGER, "Boot Knife" },
    { WIELD_DAGGER, "Prong Dagger" },
    { WIELD_DAGGER, "Golden Kris" },
    { WIELD_DAGGER, "Sai" },
    { WIELD_DAGGER, "Demon Dagger" },
    { WIELD_DAGGER, "Flaying Knife" },
    { WIELD_DAGGER, "Ambusher" },
    { WIELD_DAGGER, "Stiletto" },

    { WIELD_EXOTIC, "Sharktooth Claw" },
    { WIELD_EXOTIC, "Spine Bow" },
    { WIELD_EXOTIC, "Imperial Bow" },
    { WIELD_EXOTIC, "Gemini Claw" },
    { WIELD_EXOTIC, "Sage Wand" },
    { WIELD_EXOTIC, "Reflex Bow" },
    { WIELD_EXOTIC, "Sniper Bow" },
    { WIELD_EXOTIC, "Twin Claw" },
    { WIELD_EXOTIC, "Crystal Source" },

    { WIELD_FLAIL, "Light Flail" },
    { WIELD_FLAIL, "Heavy Flail" },
    { WIELD_FLAIL, "Steel Flail" },
    { WIELD_FLAIL, "Imperial Flail" },
    { WIELD_FLAIL, "Shadow Flail" },
    { WIELD_FLAIL, "Gemini Flail" },
    { WIELD_FLAIL, "War Flail" },
    { WIELD_FLAIL, "Auric Flail" },
    { WIELD_FLAIL, "Shadowsteel Flail" },

    { WIELD_MACE, "War Hammer" },
    { WIELD_MACE, "Dream Mace" },
    { WIELD_MACE, "Pernarch" },
    { WIELD_MACE, "Ancestral Club" },
    { WIELD_MACE, "Auric Mace" },
    { WIELD_MACE, "Legion Hammer" },
    { WIELD_MACE, "Nightmare Mace" },
    { WIELD_MACE, "Phantom Hammer" },
    { WIELD_MACE, "Ornate Mace" },

    { WIELD_POLEARM, "Coiled Staff" },
    { WIELD_POLEARM, "Long Staff" },
    { WIELD_POLEARM, "Highborn Staff" },
    { WIELD_POLEARM, "Lathi" },
    { WIELD_POLEARM, "Eclipse Staff" },
    { WIELD_POLEARM, "Maelstrom Staff" },
    { WIELD_POLEARM, "Imperial Staff" },
    { WIELD_POLEARM, "Ezomyte Staff" },
    { WIELD_POLEARM, "Serpentine Staff" },

    { WIELD_SPEAR, "Wooden Spear" },
    { WIELD_SPEAR, "Steel Spear" },
    { WIELD_SPEAR, "Short Spear" },
    { WIELD_SPEAR, "Long Spear" },
    { WIELD_SPEAR, "Legion Spear" },
    { WIELD_SPEAR, "Ezomyte Spear" },
    { WIELD_SPEAR, "Phantom Spear" },
    { WIELD_SPEAR, "Imperial Spear" },
    { WIELD_SPEAR, "Serpentine Spear" },

    { WIELD_SWORD, "Rusted Sword" },
    { WIELD_SWORD, "Sabre" },
    { WIELD_SWORD, "Dusk Blade" },
    { WIELD_SWORD, "Cutlass" },
    { WIELD_SWORD, "Baselard" },
    { WIELD_SWORD, "Hook Sword" },
    { WIELD_SWORD, "Elder Sword" },
    { WIELD_SWORD, "Twilight Blade" },
    { WIELD_SWORD, "Midnight Blade" },

    { WIELD_WHIP, "Rope Whip" },
    { WIELD_WHIP, "Steel Whip" },
    { WIELD_WHIP, "2-Headed Whip" },
    { WIELD_WHIP, "Whisper Whip" },
    { WIELD_WHIP, "Imperial Whip" },
    { WIELD_WHIP, "Shadow Whip" },
    { WIELD_WHIP, "Greysteel Whip" },
    { WIELD_WHIP, "Cat o Nine Tail" },
    { WIELD_WHIP, "Twilight Whip" },
};

const std::vector<String> legendary_base_pool_table = {
    /* Description: Pool of Legendary quality prefixes for use with random name
    generator, used at equipment generation (if its of legendary quality) */
    "Agony", "Apocalypse", "Armageddon", "Beast", "Behemoth", "Blight", "Blood",
    "Bramble", "Brimstone", "Brood", "Carrion", "Cataclysm", "Chimeric", "Corpse",
    "Corruption", "Damnation", "Death", "Demon", "Dire", "Dragon", "Dread", "Doom",
    "Dusk", "Eagle", "Empyrean", "Fate", "Foe", "Gale", "Ghoul", "Gloom", "Glyph",
    "Golem", "Grim", "Hate", "Havoc", "Honour", "Horror", "Hypnotic", "Kraken",
    "Loath", "Maelstrom", "Mind", "Miracle", "Morbid", "Oblivion", "Onslaught",
    "Pain", "Pandemonium", "Phoenix", "Plague", "Rage", "Rapture", "Rune", "Skull",
    "Sol", "Soul", "Sorrow", "Spirit", "Storm", "Tempest", "Torment", "Vengeance",
    "Victory", "Viper", "Vortex", "Woe", "Wrath"
};

const std::map<int, std::vector<String> > legendary_name_table = {
    { ARMOR_LIGHT, {
        "Wisp", "Lantern", "Glowing Orb", "Torch", "Wisp", "Faerie", "Kindle", "Star",
        "Dawn", "Flare", "Sparkle", "Banner", "Sunrise", "Luminosity", "Dawning", "Spark"
    }},
    { ARMOR_FINGER, {
        "Band", "Circle", "Coil", "Eye", "Finger", "Grasp", "Grip", "Gyre", "Hold",
        "Knot", "Knuckle", "Loop", "Nail", "Spiral", "Turn", "Twirl"
    }},
    { ARMOR_NECK, {
        "Beads", "Braid", "Charm", "Choker", "Clasp", "Collar", "Idol", "Gorget",
        "Heart", "Locket", "Medallion", "Pendant", "Noose", "Scarab", "Torc", "Talisman"
    }},
    { ARMOR_TORSO, {
        "Carapace", "Cloak", "Coat", "Curtain", "Guardian", "Hide", "Jack", "Keep",
        "Mantle", "Salvation", "Sanctuary", "Shell", "Shelter", "Shroud", "Suit", "Veil"
    }},
    { ARMOR_HEAD, {
        "Brow", "Corona", "Cowl", "Crest", "Crown", "Dome", "Glance", "Guardian",
        "Halo", "Horn", "Keep", "Peak", "Salvation", "Shelter", "Star", "Veil"
    }},
    { ARMOR_LEGS, {
        "Gaiters", "Spats", "Brogues", "Chaps", "Trousers", "Puttee", "Sheath",
        "Breeches", "Jodhpurs", "Buskins", "Putts", "Half-Boots", "Leggings", "Chinos", "Baggies", "Cover"
    }},
    { ARMOR_FEET, {
        "Dash", "Goad", "Hoof", "League", "March", "Pace", "Road", "Slippers",
        "Sole", "Span", "Spark", "Spur", "Stride", "Track", "Trail", "Tread"
    }},
    { ARMOR_HANDS, {    
        "Caress", "Claw", "Clutches", "Fingers", "Fist", "Grasp", "Grip", "Hand",
        "Hold", "Knuckle", "Mitts", "Nails", "Palm", "Paw", "Talons", "Touch"
    }},
    { ARMOR_ARMS, {
        "Cuff", "Arm", "Shell", "Sheath", "Fretwork", "Grating", "Skin", "Mantel",
        "Matting", "Twill", "Casing", "Liner", "Skein", "Bushing", "Filigree", "Beslaver"
    }},
    { ARMOR_SHIELD, {
        "Aegis", "Badge", "Barrier", "Bastion", "Bulwark", "Duty", "Emblem", "Fend",
        "Guard", "Mark", "Refuge", "Rock", "Rook", "Sanctuary", "Span", "Tower"
    }},
    { ARMOR_BODY, {
        "Garb", "Garment", "Habit", "Attire", "Frock", "Mantle", "Vestment", "Tog",
        "Drape", "Coat", "Robe", "Covering", "Cape", "Cover", "Tunic", "Peignoir"
    }},
    { ARMOR_WAIST, {
        "Bind", "Bond", "Buckle", "Clasp", "Cord", "Girdle", "Harness", "Lash",
        "Leash", "Lock", "Locket", "Shackle", "Snare", "Strap", "Twine", "Tether"
    }},
    { ARMOR_WRIST, {
        "Bracer", "Bracelet", "Brace", "Armguard", "Stanchion", "Girder", "Bracket",
        "Manacle", "Ornament", "Wristlet", "Band", "Circlet", "Trinket", "Charm", "Cuff", "Fillet"
    }},
    { ARMOR_FLOATING, {
        "Song", "Curse", "Thoughts", "Orb", "Scarab", "Spell", "Star", "Ward",
        "Weaver", "Wish", "Haven", "Rock", "Rook", "Sanctuary", "Span", "Tower"
    }},
    { ARMOR_HOLD, {
        "Trinket", "Locket", "Span", "Orb", "Scarab", "Caress", "Star", "Ward",
        "Weaver", "Wish", "Haven", "Rock", "Rook", "Sanctuary", "Span", "Tower"
    }},
    { WIELD_AXE, {
        "Bane", "Beak", "Bite", "Butcher", "Edge", "Etcher", "Gnash", "Hunger",
        "Mangler", "Rend", "Roar", "Sever", "Slayer", "Spawn", "Splitter", "Star"
    }},
    { WIELD_DAGGER, {
        "Bane", "Barb", "Bite", "Edge", "Etcher", "Fang", "Gutter", "Hunger",
        "Impaler", "Needle", "Razer", "Scapel", "Scratch", "Sever", "Stinger", "Spike"
    }},
    { WIELD_EXOTIC, {
        "Bane", "Beam", "Breeze", "Chant", "Gnarl", "Spell", "Hunger", "Mast",
        "Nock", "Guide", "Shatter", "Siege", "Song", "Weaver", "Volley", "Thunder"
    }},
    { WIELD_FLAIL, {
        "Bane", "Blow", "Cry", "Chant", "Crack", "Crusher", "Star", "Guide", "Gnash",
        "Mangler", "Pile", "Ram", "Siege", "Slayer", "Strike", "Thresher"
    }},
    { WIELD_MACE, {
        "Bane", "Batter", "Blast", "Blow", "Blunt", "Breaker", "Brand", "Burst",
        "Crack", "Crusher", "Grinder", "Knell", "Mangler", "Ram", "Roar", "Shatter"
    }},
    { WIELD_POLEARM, {
        "Bane", "Beam", "Branch", "Call", "Chant", "Cry", "Gnarl", "Goad", "Grinder",
        "Mast", "Pile", "Pillar", "Pole", "Post", "Roar", "Spire"
    }},
    { WIELD_SPEAR, {
        "Bane", "Beam", "Branch", "Call", "Chant", "Cry", "Gnarl", "Goad", "Grinder",
        "Mast", "Pile", "Pillar", "Pole", "Post", "Roar", "Spire"
    }},
    { WIELD_SWORD, {
        "Bane", "Barb", "Beak", "Bite", "Edge", "Fang", "Gutter", "Hunger", "Impaler",
        "Needle", "Razor", "Scratch", "Skewer", "Spike", "Stinger", "Thirst"
    }},
    { WIELD_WHIP, {
        "Razor", "Edge", "Whip", "Scratch", "Stinger", "Barb", "Arch", "Beak", "Blast",
        "Etcher", "Guide", "Rend", "Star", "Thresher", "Twine", "Wreck"
    }}
};

const std::multimap<int, affect::type> prefixes_allowed = {
    { ARMOR_LIGHT,      affect::type::obj_prefix_solar            },
    { ARMOR_LIGHT,      affect::type::obj_prefix_astral           },
    { ARMOR_LIGHT,      affect::type::obj_prefix_lunar            },
    { ARMOR_LIGHT,      affect::type::obj_prefix_windy            },
    { ARMOR_LIGHT,      affect::type::obj_prefix_healthy          },
    { ARMOR_LIGHT,      affect::type::obj_prefix_hearty           },
    { ARMOR_LIGHT,      affect::type::obj_prefix_stalwart         },
    { ARMOR_LIGHT,      affect::type::obj_prefix_mystical         },
    { ARMOR_LIGHT,      affect::type::obj_prefix_magical          },
    { ARMOR_LIGHT,      affect::type::obj_prefix_lucky            },
    { ARMOR_LIGHT,      affect::type::obj_prefix_fortuitous       },
    { ARMOR_LIGHT,      affect::type::obj_prefix_sturdy           },
    { ARMOR_LIGHT,      affect::type::obj_prefix_reinforced       },
    { ARMOR_LIGHT,      affect::type::obj_prefix_precise          },
    { ARMOR_LIGHT,      affect::type::obj_prefix_skilled          },
    { ARMOR_LIGHT,      affect::type::obj_prefix_jagged           },
    { ARMOR_LIGHT,      affect::type::obj_prefix_brutal           },

    { ARMOR_FINGER,     affect::type::obj_prefix_solar            },
    { ARMOR_FINGER,     affect::type::obj_prefix_astral           },
    { ARMOR_FINGER,     affect::type::obj_prefix_lunar            },
    { ARMOR_FINGER,     affect::type::obj_prefix_windy            },
    { ARMOR_FINGER,     affect::type::obj_prefix_healthy          },
    { ARMOR_FINGER,     affect::type::obj_prefix_hearty           },
    { ARMOR_FINGER,     affect::type::obj_prefix_stalwart         },
    { ARMOR_FINGER,     affect::type::obj_prefix_mystical         },
    { ARMOR_FINGER,     affect::type::obj_prefix_magical          },
    { ARMOR_FINGER,     affect::type::obj_prefix_lucky            },
    { ARMOR_FINGER,     affect::type::obj_prefix_fortuitous       },
    { ARMOR_FINGER,     affect::type::obj_prefix_sturdy           },
    { ARMOR_FINGER,     affect::type::obj_prefix_reinforced       },
    { ARMOR_FINGER,     affect::type::obj_prefix_precise          },
    { ARMOR_FINGER,     affect::type::obj_prefix_skilled          },
    { ARMOR_FINGER,     affect::type::obj_prefix_jagged           },
    { ARMOR_FINGER,     affect::type::obj_prefix_brutal           },

    { ARMOR_NECK,       affect::type::obj_prefix_solar            },
    { ARMOR_NECK,       affect::type::obj_prefix_astral           },
    { ARMOR_NECK,       affect::type::obj_prefix_lunar            },
    { ARMOR_NECK,       affect::type::obj_prefix_windy            },
    { ARMOR_NECK,       affect::type::obj_prefix_healthy          },
    { ARMOR_NECK,       affect::type::obj_prefix_hearty           },
    { ARMOR_NECK,       affect::type::obj_prefix_stalwart         },
    { ARMOR_NECK,       affect::type::obj_prefix_mystical         },
    { ARMOR_NECK,       affect::type::obj_prefix_magical          },
    { ARMOR_NECK,       affect::type::obj_prefix_lucky            },
    { ARMOR_NECK,       affect::type::obj_prefix_fortuitous       },
    { ARMOR_NECK,       affect::type::obj_prefix_sturdy           },
    { ARMOR_NECK,       affect::type::obj_prefix_reinforced       },
    { ARMOR_NECK,       affect::type::obj_prefix_precise          },
    { ARMOR_NECK,       affect::type::obj_prefix_skilled          },
    { ARMOR_NECK,       affect::type::obj_prefix_jagged           },
    { ARMOR_NECK,       affect::type::obj_prefix_brutal           },

    { ARMOR_TORSO,      affect::type::obj_prefix_solar            },
    { ARMOR_TORSO,      affect::type::obj_prefix_astral           },
    { ARMOR_TORSO,      affect::type::obj_prefix_lunar            },
    { ARMOR_TORSO,      affect::type::obj_prefix_windy            },
    { ARMOR_TORSO,      affect::type::obj_prefix_healthy          },
    { ARMOR_TORSO,      affect::type::obj_prefix_hearty           },
    { ARMOR_TORSO,      affect::type::obj_prefix_stalwart         },
    { ARMOR_TORSO,      affect::type::obj_prefix_mystical         },
    { ARMOR_TORSO,      affect::type::obj_prefix_magical          },
    { ARMOR_TORSO,      affect::type::obj_prefix_lucky            },
    { ARMOR_TORSO,      affect::type::obj_prefix_fortuitous       },
    { ARMOR_TORSO,      affect::type::obj_prefix_sturdy           },
    { ARMOR_TORSO,      affect::type::obj_prefix_reinforced       },
    { ARMOR_TORSO,      affect::type::obj_prefix_precise          },
    { ARMOR_TORSO,      affect::type::obj_prefix_skilled          },
    { ARMOR_TORSO,      affect::type::obj_prefix_jagged           },
    { ARMOR_TORSO,      affect::type::obj_prefix_brutal           },

    { ARMOR_HEAD,       affect::type::obj_prefix_solar            },
    { ARMOR_HEAD,       affect::type::obj_prefix_astral           },
    { ARMOR_HEAD,       affect::type::obj_prefix_lunar            },
    { ARMOR_HEAD,       affect::type::obj_prefix_windy            },
    { ARMOR_HEAD,       affect::type::obj_prefix_healthy          },
    { ARMOR_HEAD,       affect::type::obj_prefix_hearty           },
    { ARMOR_HEAD,       affect::type::obj_prefix_stalwart         },
    { ARMOR_HEAD,       affect::type::obj_prefix_mystical         },
    { ARMOR_HEAD,       affect::type::obj_prefix_magical          },
    { ARMOR_HEAD,       affect::type::obj_prefix_lucky            },
    { ARMOR_HEAD,       affect::type::obj_prefix_fortuitous       },
    { ARMOR_HEAD,       affect::type::obj_prefix_sturdy           },
    { ARMOR_HEAD,       affect::type::obj_prefix_reinforced       },
    { ARMOR_HEAD,       affect::type::obj_prefix_precise          },
    { ARMOR_HEAD,       affect::type::obj_prefix_skilled          },
    { ARMOR_HEAD,       affect::type::obj_prefix_jagged           },
    { ARMOR_HEAD,       affect::type::obj_prefix_brutal           },

    { ARMOR_LEGS,       affect::type::obj_prefix_solar            },
    { ARMOR_LEGS,       affect::type::obj_prefix_astral           },
    { ARMOR_LEGS,       affect::type::obj_prefix_lunar            },
    { ARMOR_LEGS,       affect::type::obj_prefix_windy            },
    { ARMOR_LEGS,       affect::type::obj_prefix_healthy          },
    { ARMOR_LEGS,       affect::type::obj_prefix_hearty           },
    { ARMOR_LEGS,       affect::type::obj_prefix_stalwart         },
    { ARMOR_LEGS,       affect::type::obj_prefix_mystical         },
    { ARMOR_LEGS,       affect::type::obj_prefix_magical          },
    { ARMOR_LEGS,       affect::type::obj_prefix_lucky            },
    { ARMOR_LEGS,       affect::type::obj_prefix_fortuitous       },
    { ARMOR_LEGS,       affect::type::obj_prefix_sturdy           },
    { ARMOR_LEGS,       affect::type::obj_prefix_reinforced       },
    { ARMOR_LEGS,       affect::type::obj_prefix_precise          },
    { ARMOR_LEGS,       affect::type::obj_prefix_skilled          },
    { ARMOR_LEGS,       affect::type::obj_prefix_jagged           },
    { ARMOR_LEGS,       affect::type::obj_prefix_brutal           },

    { ARMOR_FEET,       affect::type::obj_prefix_solar            },
    { ARMOR_FEET,       affect::type::obj_prefix_astral           },
    { ARMOR_FEET,       affect::type::obj_prefix_lunar            },
    { ARMOR_FEET,       affect::type::obj_prefix_windy            },
    { ARMOR_FEET,       affect::type::obj_prefix_healthy          },
    { ARMOR_FEET,       affect::type::obj_prefix_hearty           },
    { ARMOR_FEET,       affect::type::obj_prefix_stalwart         },
    { ARMOR_FEET,       affect::type::obj_prefix_mystical         },
    { ARMOR_FEET,       affect::type::obj_prefix_magical          },
    { ARMOR_FEET,       affect::type::obj_prefix_lucky            },
    { ARMOR_FEET,       affect::type::obj_prefix_fortuitous       },
    { ARMOR_FEET,       affect::type::obj_prefix_sturdy           },
    { ARMOR_FEET,       affect::type::obj_prefix_reinforced       },
    { ARMOR_FEET,       affect::type::obj_prefix_precise          },
    { ARMOR_FEET,       affect::type::obj_prefix_skilled          },
    { ARMOR_FEET,       affect::type::obj_prefix_jagged           },
    { ARMOR_FEET,       affect::type::obj_prefix_brutal           },

    { ARMOR_HANDS,      affect::type::obj_prefix_solar            },
    { ARMOR_HANDS,      affect::type::obj_prefix_astral           },
    { ARMOR_HANDS,      affect::type::obj_prefix_lunar            },
    { ARMOR_HANDS,      affect::type::obj_prefix_windy            },
    { ARMOR_HANDS,      affect::type::obj_prefix_healthy          },
    { ARMOR_HANDS,      affect::type::obj_prefix_hearty           },
    { ARMOR_HANDS,      affect::type::obj_prefix_stalwart         },
    { ARMOR_HANDS,      affect::type::obj_prefix_mystical         },
    { ARMOR_HANDS,      affect::type::obj_prefix_magical          },
    { ARMOR_HANDS,      affect::type::obj_prefix_lucky            },
    { ARMOR_HANDS,      affect::type::obj_prefix_fortuitous       },
    { ARMOR_HANDS,      affect::type::obj_prefix_sturdy           },
    { ARMOR_HANDS,      affect::type::obj_prefix_reinforced       },
    { ARMOR_HANDS,      affect::type::obj_prefix_precise          },
    { ARMOR_HANDS,      affect::type::obj_prefix_skilled          },
    { ARMOR_HANDS,      affect::type::obj_prefix_jagged           },
    { ARMOR_HANDS,      affect::type::obj_prefix_brutal           },

    { ARMOR_ARMS,       affect::type::obj_prefix_solar            },
    { ARMOR_ARMS,       affect::type::obj_prefix_astral           },
    { ARMOR_ARMS,       affect::type::obj_prefix_lunar            },
    { ARMOR_ARMS,       affect::type::obj_prefix_windy            },
    { ARMOR_ARMS,       affect::type::obj_prefix_healthy          },
    { ARMOR_ARMS,       affect::type::obj_prefix_hearty           },
    { ARMOR_ARMS,       affect::type::obj_prefix_stalwart         },
    { ARMOR_ARMS,       affect::type::obj_prefix_mystical         },
    { ARMOR_ARMS,       affect::type::obj_prefix_magical          },
    { ARMOR_ARMS,       affect::type::obj_prefix_lucky            },
    { ARMOR_ARMS,       affect::type::obj_prefix_fortuitous       },
    { ARMOR_ARMS,       affect::type::obj_prefix_sturdy           },
    { ARMOR_ARMS,       affect::type::obj_prefix_reinforced       },
    { ARMOR_ARMS,       affect::type::obj_prefix_precise          },
    { ARMOR_ARMS,       affect::type::obj_prefix_skilled          },
    { ARMOR_ARMS,       affect::type::obj_prefix_jagged           },
    { ARMOR_ARMS,       affect::type::obj_prefix_brutal           },

    { ARMOR_SHIELD,     affect::type::obj_prefix_solar            },
    { ARMOR_SHIELD,     affect::type::obj_prefix_astral           },
    { ARMOR_SHIELD,     affect::type::obj_prefix_lunar            },
    { ARMOR_SHIELD,     affect::type::obj_prefix_windy            },
    { ARMOR_SHIELD,     affect::type::obj_prefix_healthy          },
    { ARMOR_SHIELD,     affect::type::obj_prefix_hearty           },
    { ARMOR_SHIELD,     affect::type::obj_prefix_stalwart         },
    { ARMOR_SHIELD,     affect::type::obj_prefix_mystical         },
    { ARMOR_SHIELD,     affect::type::obj_prefix_magical          },
    { ARMOR_SHIELD,     affect::type::obj_prefix_lucky            },
    { ARMOR_SHIELD,     affect::type::obj_prefix_fortuitous       },
    { ARMOR_SHIELD,     affect::type::obj_prefix_sturdy           },
    { ARMOR_SHIELD,     affect::type::obj_prefix_reinforced       },
    { ARMOR_SHIELD,     affect::type::obj_prefix_precise          },
    { ARMOR_SHIELD,     affect::type::obj_prefix_skilled          },
    { ARMOR_SHIELD,     affect::type::obj_prefix_jagged           },
    { ARMOR_SHIELD,     affect::type::obj_prefix_brutal           },

    { ARMOR_BODY,       affect::type::obj_prefix_solar            },
    { ARMOR_BODY,       affect::type::obj_prefix_astral           },
    { ARMOR_BODY,       affect::type::obj_prefix_lunar            },
    { ARMOR_BODY,       affect::type::obj_prefix_windy            },
    { ARMOR_BODY,       affect::type::obj_prefix_healthy          },
    { ARMOR_BODY,       affect::type::obj_prefix_hearty           },
    { ARMOR_BODY,       affect::type::obj_prefix_stalwart         },
    { ARMOR_BODY,       affect::type::obj_prefix_mystical         },
    { ARMOR_BODY,       affect::type::obj_prefix_magical          },
    { ARMOR_BODY,       affect::type::obj_prefix_lucky            },
    { ARMOR_BODY,       affect::type::obj_prefix_fortuitous       },
    { ARMOR_BODY,       affect::type::obj_prefix_sturdy           },
    { ARMOR_BODY,       affect::type::obj_prefix_reinforced       },
    { ARMOR_BODY,       affect::type::obj_prefix_precise          },
    { ARMOR_BODY,       affect::type::obj_prefix_skilled          },
    { ARMOR_BODY,       affect::type::obj_prefix_jagged           },
    { ARMOR_BODY,       affect::type::obj_prefix_brutal           },

    { ARMOR_WAIST,      affect::type::obj_prefix_solar            },
    { ARMOR_WAIST,      affect::type::obj_prefix_astral           },
    { ARMOR_WAIST,      affect::type::obj_prefix_lunar            },
    { ARMOR_WAIST,      affect::type::obj_prefix_windy            },
    { ARMOR_WAIST,      affect::type::obj_prefix_healthy          },
    { ARMOR_WAIST,      affect::type::obj_prefix_hearty           },
    { ARMOR_WAIST,      affect::type::obj_prefix_stalwart         },
    { ARMOR_WAIST,      affect::type::obj_prefix_mystical         },
    { ARMOR_WAIST,      affect::type::obj_prefix_magical          },
    { ARMOR_WAIST,      affect::type::obj_prefix_lucky            },
    { ARMOR_WAIST,      affect::type::obj_prefix_fortuitous       },
    { ARMOR_WAIST,      affect::type::obj_prefix_sturdy           },
    { ARMOR_WAIST,      affect::type::obj_prefix_reinforced       },
    { ARMOR_WAIST,      affect::type::obj_prefix_precise          },
    { ARMOR_WAIST,      affect::type::obj_prefix_skilled          },
    { ARMOR_WAIST,      affect::type::obj_prefix_jagged           },
    { ARMOR_WAIST,      affect::type::obj_prefix_brutal           },

    { ARMOR_WRIST,      affect::type::obj_prefix_solar            },
    { ARMOR_WRIST,      affect::type::obj_prefix_astral           },
    { ARMOR_WRIST,      affect::type::obj_prefix_lunar            },
    { ARMOR_WRIST,      affect::type::obj_prefix_windy            },
    { ARMOR_WRIST,      affect::type::obj_prefix_healthy          },
    { ARMOR_WRIST,      affect::type::obj_prefix_hearty           },
    { ARMOR_WRIST,      affect::type::obj_prefix_stalwart         },
    { ARMOR_WRIST,      affect::type::obj_prefix_mystical         },
    { ARMOR_WRIST,      affect::type::obj_prefix_magical          },
    { ARMOR_WRIST,      affect::type::obj_prefix_lucky            },
    { ARMOR_WRIST,      affect::type::obj_prefix_fortuitous       },
    { ARMOR_WRIST,      affect::type::obj_prefix_sturdy           },
    { ARMOR_WRIST,      affect::type::obj_prefix_reinforced       },
    { ARMOR_WRIST,      affect::type::obj_prefix_precise          },
    { ARMOR_WRIST,      affect::type::obj_prefix_skilled          },
    { ARMOR_WRIST,      affect::type::obj_prefix_jagged           },
    { ARMOR_WRIST,      affect::type::obj_prefix_brutal           },

    { ARMOR_FLOATING,   affect::type::obj_prefix_solar            },
    { ARMOR_FLOATING,   affect::type::obj_prefix_astral           },
    { ARMOR_FLOATING,   affect::type::obj_prefix_lunar            },
    { ARMOR_FLOATING,   affect::type::obj_prefix_windy            },
    { ARMOR_FLOATING,   affect::type::obj_prefix_healthy          },
    { ARMOR_FLOATING,   affect::type::obj_prefix_hearty           },
    { ARMOR_FLOATING,   affect::type::obj_prefix_stalwart         },
    { ARMOR_FLOATING,   affect::type::obj_prefix_mystical         },
    { ARMOR_FLOATING,   affect::type::obj_prefix_magical          },
    { ARMOR_FLOATING,   affect::type::obj_prefix_lucky            },
    { ARMOR_FLOATING,   affect::type::obj_prefix_fortuitous       },
    { ARMOR_FLOATING,   affect::type::obj_prefix_sturdy           },
    { ARMOR_FLOATING,   affect::type::obj_prefix_reinforced       },
    { ARMOR_FLOATING,   affect::type::obj_prefix_precise          },
    { ARMOR_FLOATING,   affect::type::obj_prefix_skilled          },
    { ARMOR_FLOATING,   affect::type::obj_prefix_jagged           },
    { ARMOR_FLOATING,   affect::type::obj_prefix_brutal           },

    { ARMOR_HOLD,       affect::type::obj_prefix_solar            },
    { ARMOR_HOLD,       affect::type::obj_prefix_astral           },
    { ARMOR_HOLD,       affect::type::obj_prefix_lunar            },
    { ARMOR_HOLD,       affect::type::obj_prefix_windy            },
    { ARMOR_HOLD,       affect::type::obj_prefix_healthy          },
    { ARMOR_HOLD,       affect::type::obj_prefix_hearty           },
    { ARMOR_HOLD,       affect::type::obj_prefix_stalwart         },
    { ARMOR_HOLD,       affect::type::obj_prefix_mystical         },
    { ARMOR_HOLD,       affect::type::obj_prefix_magical          },
    { ARMOR_HOLD,       affect::type::obj_prefix_lucky            },
    { ARMOR_HOLD,       affect::type::obj_prefix_fortuitous       },
    { ARMOR_HOLD,       affect::type::obj_prefix_sturdy           },
    { ARMOR_HOLD,       affect::type::obj_prefix_reinforced       },
    { ARMOR_HOLD,       affect::type::obj_prefix_precise          },
    { ARMOR_HOLD,       affect::type::obj_prefix_skilled          },
    { ARMOR_HOLD,       affect::type::obj_prefix_jagged           },
    { ARMOR_HOLD,       affect::type::obj_prefix_brutal           },

    { WIELD_AXE,        affect::type::obj_prefix_solar            },
    { WIELD_AXE,        affect::type::obj_prefix_astral           },
    { WIELD_AXE,        affect::type::obj_prefix_lunar            },
    { WIELD_AXE,        affect::type::obj_prefix_windy            },
    { WIELD_AXE,        affect::type::obj_prefix_healthy          },
    { WIELD_AXE,        affect::type::obj_prefix_hearty           },
    { WIELD_AXE,        affect::type::obj_prefix_stalwart         },
    { WIELD_AXE,        affect::type::obj_prefix_mystical         },
    { WIELD_AXE,        affect::type::obj_prefix_magical          },
    { WIELD_AXE,        affect::type::obj_prefix_lucky            },
    { WIELD_AXE,        affect::type::obj_prefix_fortuitous       },
    { WIELD_AXE,        affect::type::obj_prefix_sturdy           },
    { WIELD_AXE,        affect::type::obj_prefix_reinforced       },
    { WIELD_AXE,        affect::type::obj_prefix_precise          },
    { WIELD_AXE,        affect::type::obj_prefix_skilled          },
    { WIELD_AXE,        affect::type::obj_prefix_jagged           },
    { WIELD_AXE,        affect::type::obj_prefix_brutal           },
    { WIELD_AXE,        affect::type::weapon_acidic               },
    { WIELD_AXE,        affect::type::weapon_flaming              },
    { WIELD_AXE,        affect::type::weapon_frost                },
    { WIELD_AXE,        affect::type::weapon_vampiric             },
    { WIELD_AXE,        affect::type::weapon_shocking             },
    { WIELD_AXE,        affect::type::weapon_vorpal               },
    { WIELD_AXE,        affect::type::weapon_sharp                },
    { WIELD_AXE,        affect::type::poison                      },

    { WIELD_DAGGER,     affect::type::obj_prefix_solar            },
    { WIELD_DAGGER,     affect::type::obj_prefix_astral           },
    { WIELD_DAGGER,     affect::type::obj_prefix_lunar            },
    { WIELD_DAGGER,     affect::type::obj_prefix_windy            },
    { WIELD_DAGGER,     affect::type::obj_prefix_healthy          },
    { WIELD_DAGGER,     affect::type::obj_prefix_hearty           },
    { WIELD_DAGGER,     affect::type::obj_prefix_stalwart         },
    { WIELD_DAGGER,     affect::type::obj_prefix_mystical         },
    { WIELD_DAGGER,     affect::type::obj_prefix_magical          },
    { WIELD_DAGGER,     affect::type::obj_prefix_lucky            },
    { WIELD_DAGGER,     affect::type::obj_prefix_fortuitous       },
    { WIELD_DAGGER,     affect::type::obj_prefix_sturdy           },
    { WIELD_DAGGER,     affect::type::obj_prefix_reinforced       },
    { WIELD_DAGGER,     affect::type::obj_prefix_precise          },
    { WIELD_DAGGER,     affect::type::obj_prefix_skilled          },
    { WIELD_DAGGER,     affect::type::obj_prefix_jagged           },
    { WIELD_DAGGER,     affect::type::obj_prefix_brutal           },
    { WIELD_DAGGER,     affect::type::weapon_acidic               },
    { WIELD_DAGGER,     affect::type::weapon_flaming              },
    { WIELD_DAGGER,     affect::type::weapon_frost                },
    { WIELD_DAGGER,     affect::type::weapon_vampiric             },
    { WIELD_DAGGER,     affect::type::weapon_shocking             },
    { WIELD_DAGGER,     affect::type::weapon_vorpal               },
    { WIELD_DAGGER,     affect::type::weapon_sharp                },
    { WIELD_DAGGER,     affect::type::poison                      },

    { WIELD_EXOTIC,     affect::type::obj_prefix_solar            },
    { WIELD_EXOTIC,     affect::type::obj_prefix_astral           },
    { WIELD_EXOTIC,     affect::type::obj_prefix_lunar            },
    { WIELD_EXOTIC,     affect::type::obj_prefix_windy            },
    { WIELD_EXOTIC,     affect::type::obj_prefix_healthy          },
    { WIELD_EXOTIC,     affect::type::obj_prefix_hearty           },
    { WIELD_EXOTIC,     affect::type::obj_prefix_stalwart         },
    { WIELD_EXOTIC,     affect::type::obj_prefix_mystical         },
    { WIELD_EXOTIC,     affect::type::obj_prefix_magical          },
    { WIELD_EXOTIC,     affect::type::obj_prefix_lucky            },
    { WIELD_EXOTIC,     affect::type::obj_prefix_fortuitous       },
    { WIELD_EXOTIC,     affect::type::obj_prefix_sturdy           },
    { WIELD_EXOTIC,     affect::type::obj_prefix_reinforced       },
    { WIELD_EXOTIC,     affect::type::obj_prefix_precise          },
    { WIELD_EXOTIC,     affect::type::obj_prefix_skilled          },
    { WIELD_EXOTIC,     affect::type::obj_prefix_jagged           },
    { WIELD_EXOTIC,     affect::type::obj_prefix_brutal           },
    { WIELD_EXOTIC,     affect::type::weapon_acidic               },
    { WIELD_EXOTIC,     affect::type::weapon_flaming              },
    { WIELD_EXOTIC,     affect::type::weapon_frost                },
    { WIELD_EXOTIC,     affect::type::weapon_vampiric             },
    { WIELD_EXOTIC,     affect::type::weapon_shocking             },
    { WIELD_EXOTIC,     affect::type::weapon_vorpal               },
    { WIELD_EXOTIC,     affect::type::weapon_sharp                },
    { WIELD_EXOTIC,     affect::type::poison                      },

    { WIELD_FLAIL,      affect::type::obj_prefix_solar            },
    { WIELD_FLAIL,      affect::type::obj_prefix_astral           },
    { WIELD_FLAIL,      affect::type::obj_prefix_lunar            },
    { WIELD_FLAIL,      affect::type::obj_prefix_windy            },
    { WIELD_FLAIL,      affect::type::obj_prefix_healthy          },
    { WIELD_FLAIL,      affect::type::obj_prefix_hearty           },
    { WIELD_FLAIL,      affect::type::obj_prefix_stalwart         },
    { WIELD_FLAIL,      affect::type::obj_prefix_mystical         },
    { WIELD_FLAIL,      affect::type::obj_prefix_magical          },
    { WIELD_FLAIL,      affect::type::obj_prefix_lucky            },
    { WIELD_FLAIL,      affect::type::obj_prefix_fortuitous       },
    { WIELD_FLAIL,      affect::type::obj_prefix_sturdy           },
    { WIELD_FLAIL,      affect::type::obj_prefix_reinforced       },
    { WIELD_FLAIL,      affect::type::obj_prefix_precise          },
    { WIELD_FLAIL,      affect::type::obj_prefix_skilled          },
    { WIELD_FLAIL,      affect::type::obj_prefix_jagged           },
    { WIELD_FLAIL,      affect::type::obj_prefix_brutal           },
    { WIELD_FLAIL,      affect::type::weapon_acidic               },
    { WIELD_FLAIL,      affect::type::weapon_flaming              },
    { WIELD_FLAIL,      affect::type::weapon_frost                },
    { WIELD_FLAIL,      affect::type::weapon_vampiric             },
    { WIELD_FLAIL,      affect::type::weapon_shocking             },
    { WIELD_FLAIL,      affect::type::weapon_vorpal               },
    { WIELD_FLAIL,      affect::type::weapon_sharp                },
    { WIELD_FLAIL,      affect::type::poison                      },

    { WIELD_MACE,       affect::type::obj_prefix_solar            },
    { WIELD_MACE,       affect::type::obj_prefix_astral           },
    { WIELD_MACE,       affect::type::obj_prefix_lunar            },
    { WIELD_MACE,       affect::type::obj_prefix_windy            },
    { WIELD_MACE,       affect::type::obj_prefix_healthy          },
    { WIELD_MACE,       affect::type::obj_prefix_hearty           },
    { WIELD_MACE,       affect::type::obj_prefix_stalwart         },
    { WIELD_MACE,       affect::type::obj_prefix_mystical         },
    { WIELD_MACE,       affect::type::obj_prefix_magical          },
    { WIELD_MACE,       affect::type::obj_prefix_lucky            },
    { WIELD_MACE,       affect::type::obj_prefix_fortuitous       },
    { WIELD_MACE,       affect::type::obj_prefix_sturdy           },
    { WIELD_MACE,       affect::type::obj_prefix_reinforced       },
    { WIELD_MACE,       affect::type::obj_prefix_precise          },
    { WIELD_MACE,       affect::type::obj_prefix_skilled          },
    { WIELD_MACE,       affect::type::obj_prefix_jagged           },
    { WIELD_MACE,       affect::type::obj_prefix_brutal           },
    { WIELD_MACE,       affect::type::weapon_acidic               },
    { WIELD_MACE,       affect::type::weapon_flaming              },
    { WIELD_MACE,       affect::type::weapon_frost                },
    { WIELD_MACE,       affect::type::weapon_vampiric             },
    { WIELD_MACE,       affect::type::weapon_shocking             },
    { WIELD_MACE,       affect::type::weapon_vorpal               },
    { WIELD_MACE,       affect::type::weapon_sharp                },
    { WIELD_MACE,       affect::type::poison                      },

    { WIELD_POLEARM,    affect::type::obj_prefix_solar            },
    { WIELD_POLEARM,    affect::type::obj_prefix_astral           },
    { WIELD_POLEARM,    affect::type::obj_prefix_lunar            },
    { WIELD_POLEARM,    affect::type::obj_prefix_windy            },
    { WIELD_POLEARM,    affect::type::obj_prefix_healthy          },
    { WIELD_POLEARM,    affect::type::obj_prefix_hearty           },
    { WIELD_POLEARM,    affect::type::obj_prefix_stalwart         },
    { WIELD_POLEARM,    affect::type::obj_prefix_mystical         },
    { WIELD_POLEARM,    affect::type::obj_prefix_magical          },
    { WIELD_POLEARM,    affect::type::obj_prefix_lucky            },
    { WIELD_POLEARM,    affect::type::obj_prefix_fortuitous       },
    { WIELD_POLEARM,    affect::type::obj_prefix_sturdy           },
    { WIELD_POLEARM,    affect::type::obj_prefix_reinforced       },
    { WIELD_POLEARM,    affect::type::obj_prefix_precise          },
    { WIELD_POLEARM,    affect::type::obj_prefix_skilled          },
    { WIELD_POLEARM,    affect::type::obj_prefix_jagged           },
    { WIELD_POLEARM,    affect::type::obj_prefix_brutal           },
    { WIELD_POLEARM,    affect::type::weapon_acidic               },
    { WIELD_POLEARM,    affect::type::weapon_flaming              },
    { WIELD_POLEARM,    affect::type::weapon_frost                },
    { WIELD_POLEARM,    affect::type::weapon_vampiric             },
    { WIELD_POLEARM,    affect::type::weapon_shocking             },
    { WIELD_POLEARM,    affect::type::weapon_vorpal               },
    { WIELD_POLEARM,    affect::type::weapon_sharp                },
    { WIELD_POLEARM,    affect::type::poison                      },

    { WIELD_SPEAR,      affect::type::obj_prefix_solar            },
    { WIELD_SPEAR,      affect::type::obj_prefix_astral           },
    { WIELD_SPEAR,      affect::type::obj_prefix_lunar            },
    { WIELD_SPEAR,      affect::type::obj_prefix_windy            },
    { WIELD_SPEAR,      affect::type::obj_prefix_healthy          },
    { WIELD_SPEAR,      affect::type::obj_prefix_hearty           },
    { WIELD_SPEAR,      affect::type::obj_prefix_stalwart         },
    { WIELD_SPEAR,      affect::type::obj_prefix_mystical         },
    { WIELD_SPEAR,      affect::type::obj_prefix_magical          },
    { WIELD_SPEAR,      affect::type::obj_prefix_lucky            },
    { WIELD_SPEAR,      affect::type::obj_prefix_fortuitous       },
    { WIELD_SPEAR,      affect::type::obj_prefix_sturdy           },
    { WIELD_SPEAR,      affect::type::obj_prefix_reinforced       },
    { WIELD_SPEAR,      affect::type::obj_prefix_precise          },
    { WIELD_SPEAR,      affect::type::obj_prefix_skilled          },
    { WIELD_SPEAR,      affect::type::obj_prefix_jagged           },
    { WIELD_SPEAR,      affect::type::obj_prefix_brutal           },
    { WIELD_SPEAR,      affect::type::weapon_acidic               },
    { WIELD_SPEAR,      affect::type::weapon_flaming              },
    { WIELD_SPEAR,      affect::type::weapon_frost                },
    { WIELD_SPEAR,      affect::type::weapon_vampiric             },
    { WIELD_SPEAR,      affect::type::weapon_shocking             },
    { WIELD_SPEAR,      affect::type::weapon_vorpal               },
    { WIELD_SPEAR,      affect::type::weapon_sharp                },
    { WIELD_SPEAR,      affect::type::poison                      },

    { WIELD_SWORD,      affect::type::obj_prefix_solar            },
    { WIELD_SWORD,      affect::type::obj_prefix_astral           },
    { WIELD_SWORD,      affect::type::obj_prefix_lunar            },
    { WIELD_SWORD,      affect::type::obj_prefix_windy            },
    { WIELD_SWORD,      affect::type::obj_prefix_healthy          },
    { WIELD_SWORD,      affect::type::obj_prefix_hearty           },
    { WIELD_SWORD,      affect::type::obj_prefix_stalwart         },
    { WIELD_SWORD,      affect::type::obj_prefix_mystical         },
    { WIELD_SWORD,      affect::type::obj_prefix_magical          },
    { WIELD_SWORD,      affect::type::obj_prefix_lucky            },
    { WIELD_SWORD,      affect::type::obj_prefix_fortuitous       },
    { WIELD_SWORD,      affect::type::obj_prefix_sturdy           },
    { WIELD_SWORD,      affect::type::obj_prefix_reinforced       },
    { WIELD_SWORD,      affect::type::obj_prefix_precise          },
    { WIELD_SWORD,      affect::type::obj_prefix_skilled          },
    { WIELD_SWORD,      affect::type::obj_prefix_jagged           },
    { WIELD_SWORD,      affect::type::obj_prefix_brutal           },
    { WIELD_SWORD,      affect::type::weapon_acidic               },
    { WIELD_SWORD,      affect::type::weapon_flaming              },
    { WIELD_SWORD,      affect::type::weapon_frost                },
    { WIELD_SWORD,      affect::type::weapon_vampiric             },
    { WIELD_SWORD,      affect::type::weapon_shocking             },
    { WIELD_SWORD,      affect::type::weapon_vorpal               },
    { WIELD_SWORD,      affect::type::weapon_sharp                },
    { WIELD_SWORD,      affect::type::poison                      },

    { WIELD_WHIP,       affect::type::obj_prefix_solar            },
    { WIELD_WHIP,       affect::type::obj_prefix_astral           },
    { WIELD_WHIP,       affect::type::obj_prefix_lunar            },
    { WIELD_WHIP,       affect::type::obj_prefix_windy            },
    { WIELD_WHIP,       affect::type::obj_prefix_healthy          },
    { WIELD_WHIP,       affect::type::obj_prefix_hearty           },
    { WIELD_WHIP,       affect::type::obj_prefix_stalwart         },
    { WIELD_WHIP,       affect::type::obj_prefix_mystical         },
    { WIELD_WHIP,       affect::type::obj_prefix_magical          },
    { WIELD_WHIP,       affect::type::obj_prefix_lucky            },
    { WIELD_WHIP,       affect::type::obj_prefix_fortuitous       },
    { WIELD_WHIP,       affect::type::obj_prefix_sturdy           },
    { WIELD_WHIP,       affect::type::obj_prefix_reinforced       },
    { WIELD_WHIP,       affect::type::obj_prefix_precise          },
    { WIELD_WHIP,       affect::type::obj_prefix_skilled          },
    { WIELD_WHIP,       affect::type::obj_prefix_jagged           },
    { WIELD_WHIP,       affect::type::obj_prefix_brutal           },
    { WIELD_WHIP,       affect::type::weapon_acidic               },
    { WIELD_WHIP,       affect::type::weapon_flaming              },
    { WIELD_WHIP,       affect::type::weapon_frost                },
    { WIELD_WHIP,       affect::type::weapon_vampiric             },
    { WIELD_WHIP,       affect::type::weapon_shocking             },
    { WIELD_WHIP,       affect::type::weapon_vorpal               },
    { WIELD_WHIP,       affect::type::weapon_sharp                },
    { WIELD_WHIP,       affect::type::poison                      },
};


/******************************************************************
                    Loot Code Tables Below
 ******************************************************************/
const std::multimap<int, affect::type> suffixes_allowed = {
    { ARMOR_LIGHT,      affect::type::obj_suffix_skilled },
    { ARMOR_LIGHT,      affect::type::obj_suffix_skillful },

//  { ARMOR_FINGER,     affect::type::obj_suffix_initiate_questor },
	{ ARMOR_FINGER,      	affect::type::obj_suffix_devastation },
    { ARMOR_FINGER,      	affect::type::obj_suffix_annihilation },
//  { ARMOR_NECK,       affect::type::obj_suffix_initiate_questor },

    { ARMOR_TORSO,      affect::type::obj_suffix_initiate_questor },
    { ARMOR_TORSO,      affect::type::obj_suffix_advanced_questor },
    { ARMOR_TORSO,      affect::type::obj_suffix_master_questor },

    { ARMOR_HEAD,       affect::type::obj_suffix_initiate_mage },
    { ARMOR_HEAD,       affect::type::obj_suffix_advanced_mage },
    { ARMOR_HEAD,       affect::type::obj_suffix_master_mage },
    { ARMOR_HEAD,       affect::type::obj_suffix_experienced },
    { ARMOR_HEAD,      	affect::type::obj_suffix_arcane_power },
    { ARMOR_HEAD,      	affect::type::obj_suffix_mystical_power },
	

    { ARMOR_LEGS,       affect::type::obj_suffix_initiate_mage },
    { ARMOR_LEGS,       affect::type::obj_suffix_advanced_mage },
    { ARMOR_LEGS,       affect::type::obj_suffix_master_mage },

    { ARMOR_FEET,       affect::type::obj_suffix_initiate_questor },
    { ARMOR_FEET,       affect::type::obj_suffix_advanced_questor },
    { ARMOR_FEET,       affect::type::obj_suffix_master_questor },
    { ARMOR_FEET,       affect::type::obj_suffix_swallow },
    { ARMOR_FEET,       affect::type::obj_suffix_hawk },
    { ARMOR_FEET,       affect::type::obj_suffix_falcon },
    { ARMOR_FEET,       affect::type::obj_suffix_skilled },
    { ARMOR_FEET,       affect::type::obj_suffix_skillful },

    { ARMOR_HANDS,      affect::type::obj_suffix_initiate_questor },
    { ARMOR_HANDS,      affect::type::obj_suffix_advanced_questor },
    { ARMOR_HANDS,      affect::type::obj_suffix_master_questor },
    { ARMOR_HANDS,      affect::type::obj_suffix_initiate_mage },
    { ARMOR_HANDS,      affect::type::obj_suffix_advanced_mage },
    { ARMOR_HANDS,      affect::type::obj_suffix_master_mage },
    { ARMOR_HANDS,      affect::type::obj_suffix_swallow },
    { ARMOR_HANDS,      affect::type::obj_suffix_hawk },
    { ARMOR_HANDS,      affect::type::obj_suffix_falcon },
    { ARMOR_HANDS,      affect::type::obj_suffix_experienced },
	{ ARMOR_HANDS,      affect::type::obj_suffix_devastation },
    { ARMOR_HANDS,      affect::type::obj_suffix_annihilation },
    { ARMOR_HANDS,      affect::type::obj_suffix_arcane_power },
    { ARMOR_HANDS,      affect::type::obj_suffix_mystical_power },

    { ARMOR_ARMS,       affect::type::obj_suffix_initiate_questor },
    { ARMOR_ARMS,       affect::type::obj_suffix_advanced_questor },
    { ARMOR_ARMS,       affect::type::obj_suffix_master_questor },

//  { ARMOR_SHIELD,     affect::type::obj_suffix_initiate_questor },
	{ ARMOR_SHIELD,     affect::type::obj_suffix_restorative }, //shield only
	{ ARMOR_SHIELD,     affect::type::obj_suffix_mystic },		//shield only
	
    { ARMOR_BODY,       affect::type::obj_suffix_initiate_questor },
    { ARMOR_BODY,       affect::type::obj_suffix_advanced_questor },
    { ARMOR_BODY,       affect::type::obj_suffix_master_questor },

//  { ARMOR_WAIST,      affect::type::obj_suffix_initiate_questor },

//  { ARMOR_WRIST,      affect::type::obj_suffix_initiate_questor },
	{ ARMOR_WRIST,      affect::type::obj_suffix_arcane_power },
    { ARMOR_WRIST,      affect::type::obj_suffix_mystical_power },

    { ARMOR_FLOATING,   affect::type::obj_suffix_skilled },
    { ARMOR_FLOATING,   affect::type::obj_suffix_skillful },
    { ARMOR_FLOATING,   affect::type::obj_suffix_experienced },

//  { ARMOR_HOLD,       affect::type::obj_suffix_initiate_questor },

    { WIELD_AXE,        affect::type::obj_suffix_devastation },
    { WIELD_AXE,        affect::type::obj_suffix_annihilation },
    { WIELD_AXE,        affect::type::obj_suffix_arcane_power },
    { WIELD_AXE,        affect::type::obj_suffix_mystical_power },
    { WIELD_AXE,        affect::type::obj_suffix_hexxing },

    { WIELD_DAGGER,     affect::type::obj_suffix_devastation },
    { WIELD_DAGGER,     affect::type::obj_suffix_annihilation },
    { WIELD_DAGGER,     affect::type::obj_suffix_arcane_power },
    { WIELD_DAGGER,     affect::type::obj_suffix_mystical_power },
    { WIELD_DAGGER,     affect::type::obj_suffix_hexxing },

    { WIELD_EXOTIC,     affect::type::obj_suffix_devastation },
    { WIELD_EXOTIC,     affect::type::obj_suffix_annihilation },
    { WIELD_EXOTIC,     affect::type::obj_suffix_arcane_power },
    { WIELD_EXOTIC,     affect::type::obj_suffix_mystical_power },
    { WIELD_EXOTIC,     affect::type::obj_suffix_hexxing },

    { WIELD_FLAIL,      affect::type::obj_suffix_devastation },
    { WIELD_FLAIL,      affect::type::obj_suffix_annihilation },
    { WIELD_FLAIL,      affect::type::obj_suffix_arcane_power },
    { WIELD_FLAIL,      affect::type::obj_suffix_mystical_power },
    { WIELD_FLAIL,      affect::type::obj_suffix_hexxing },

    { WIELD_MACE,       affect::type::obj_suffix_devastation },
    { WIELD_MACE,       affect::type::obj_suffix_annihilation },
    { WIELD_MACE,       affect::type::obj_suffix_arcane_power },
    { WIELD_MACE,       affect::type::obj_suffix_mystical_power },
    { WIELD_MACE,       affect::type::obj_suffix_hexxing },

    { WIELD_POLEARM,    affect::type::obj_suffix_devastation },
    { WIELD_POLEARM,    affect::type::obj_suffix_annihilation },
    { WIELD_POLEARM,    affect::type::obj_suffix_arcane_power },
    { WIELD_POLEARM,    affect::type::obj_suffix_mystical_power },
    { WIELD_POLEARM,    affect::type::obj_suffix_hexxing },

    { WIELD_SPEAR,      affect::type::obj_suffix_devastation },
    { WIELD_SPEAR,      affect::type::obj_suffix_annihilation },
    { WIELD_SPEAR,      affect::type::obj_suffix_arcane_power },
    { WIELD_SPEAR,      affect::type::obj_suffix_mystical_power },
    { WIELD_SPEAR,      affect::type::obj_suffix_hexxing },

    { WIELD_SWORD,      affect::type::obj_suffix_devastation },
    { WIELD_SWORD,      affect::type::obj_suffix_annihilation },
    { WIELD_SWORD,      affect::type::obj_suffix_arcane_power },
    { WIELD_SWORD,      affect::type::obj_suffix_mystical_power },
    { WIELD_SWORD,      affect::type::obj_suffix_hexxing },

    { WIELD_WHIP,       affect::type::obj_suffix_devastation },
    { WIELD_WHIP,       affect::type::obj_suffix_annihilation },
    { WIELD_WHIP,       affect::type::obj_suffix_arcane_power },
    { WIELD_WHIP,       affect::type::obj_suffix_mystical_power },
    { WIELD_WHIP,       affect::type::obj_suffix_hexxing },

};



const std::map<affect::type, mod_t> mod_table = {
    /*
    {   type,                           {
        text,                           group, rarity,
        af_where,   af_loc,     af_mod_min, af_mod_max, scaling
    }}
    */
    {   affect::type::obj_prefix_solar,       {
        "{YS{bo{Rl{ba{Yr",              1,  100,
        TO_OBJECT,  APPLY_STR,  1,  5, true   
    }},
    {   affect::type::obj_prefix_astral,      {
        "{gA{Ws{Ttr{Wa{gl",             1,  100,
        TO_OBJECT,  APPLY_INT,  1,  5, true   
    }},
    {   affect::type::obj_prefix_lunar,       {
        "{WL{gu{gn{ca{cr",              1,  100,
        TO_OBJECT,  APPLY_WIS,  1,  5, true   
    }},
    {   affect::type::obj_prefix_windy,       {
        "{cWi{gnd{Wy",                  1,  100,
        TO_OBJECT,  APPLY_DEX,  1,  5, true   
    }},
    {   affect::type::obj_prefix_healthy,     {
        "{HH{Pe{Halt{Ph{Hy",            1,  100,
        TO_OBJECT,  APPLY_CON,  1,  5, true   
    }},
    /*  Defense Group (Group 2)
        Contains HP, Mana, Saves, and AC prefixes
    */
    {   affect::type::obj_prefix_hearty,      {
        "Hearty",                           2,  100,
        TO_OBJECT,  APPLY_HIT,  10, 40, true  
    }},
    {   affect::type::obj_prefix_stalwart,    {
        "{HSt{ca{Hlw{ca{Hrt",               2,  100,
        TO_OBJECT,  APPLY_HIT,  41, 75, true  
    }},
    {   affect::type::obj_prefix_mystical,    {
        "{YM{by{Ps{ct{Ni{Bc{Ta{gl",         2,  100,
        TO_OBJECT,  APPLY_MANA, 10, 40, true  
    }},
    {   affect::type::obj_prefix_magical,     {
        "{CM{Ba{Ng{Mi{Nc{Ba{Cl",            2,  100,
        TO_OBJECT,  APPLY_MANA, 41, 75, true  
    }},
    {   affect::type::obj_prefix_lucky,       {
        "{HL{Gu{gc{Gk{Hy",                  2,  100,
        TO_OBJECT,  APPLY_SAVES, -1,  -2, false   
    }},
    {   affect::type::obj_prefix_fortuitous,  {
        "{GFo{Hr{ct{Gui{Ht{co{Gus",         2,  100,
        TO_OBJECT,  APPLY_SAVES, -3,  -4, false   
    }},
    {   affect::type::obj_prefix_sturdy,      {
        "{bSt{Pu{brd{Py",                   2,  100,
        TO_OBJECT,  APPLY_AC,   -1,  -5, true   
    }},
    {   affect::type::obj_prefix_reinforced,  {
        "{WR{ge{Ti{Wn{gf{To{Wr{gc{Te{gd",   2,  100,
        TO_OBJECT,  APPLY_AC,   -6,  -10, true  
    }},
    /*  Offense Group (Group 3)
        Contains hit and dam roll prefixes
    */
    {   affect::type::obj_prefix_precise,     {
        "{TPr{Pe{Yc{Pi{Tse",              3,  100,
        TO_OBJECT,  APPLY_HITROLL,  1,  3, true   
    }},
    {   affect::type::obj_prefix_skilled,     {
        "{MS{Vk{Bi{Ml{Vl{Be{Md",          3,  100,
        TO_OBJECT,  APPLY_HITROLL,  4,  6, true   
    }},
    {   affect::type::obj_prefix_jagged,      {
        "{WJ{ga{cg{Wg{ge{cd",             3,  100,
        TO_OBJECT,  APPLY_DAMROLL,  1,  3, true   
    }},
    {   affect::type::obj_prefix_brutal,      {
        "{RB{br{Hu{Rt{ba{Hl",             3,  100,
        TO_OBJECT,  APPLY_DAMROLL,  4,  6, true   
    }},
    /*  Misc. Group (Group 4)
        Contains misc prefixes (spells on armor ect....)
    */
    {   affect::type::sanctuary,              {
        "{NA{Bn{gg{We{gl{Bi{Nc",          4,  100,
        TO_AFFECTS, 0,  0,  0, false   
    }},
    {   affect::type::sneak,                  {
        "{WSt{gea{Wlt{ghy",               4,  100,
        TO_AFFECTS, 0,  0,  0, false   
    }},
    {   affect::type::protection_evil,        {
        "{TPure",                         4,  100,
        TO_AFFECTS, 0,  0,  0, false   
    }},
    {   affect::type::protection_good,        {
        "{RT{ba{Mi{Bn{Ht{be{Rd",          4,  100,
        TO_AFFECTS, 0,  0,  0, false   
    }},
    {   affect::type::flameshield,            {
        "Flaming",                        4,  100,
        TO_AFFECTS, 0,  0,  0, false   
    }},
    {   affect::type::pass_door,              {
        "{CM{gi{Cst{gy",                  4,  100,
        TO_AFFECTS, 0,  0,  0, false   
    }},
    {   affect::type::regeneration,           {
        "Regenerative",                   4,  100,
        TO_AFFECTS, 0,  0,  0, false   
    }},
    {   affect::type::haste,                  {
        "{CS{Tw{Bi{Cf{Tt",                4,  100,
        TO_AFFECTS, 0,  0,  0, false   
    }},

    //weapon flags
    {   affect::type::weapon_acidic,          {
        "{cC{Gor{Hr{Go{Hs{Giv{ce",        3,  100,
        TO_WEAPON,  0,  0,  0, false   
    }},
    {   affect::type::weapon_flaming,         {
        "{cF{Pl{Yami{Pn{cg",              3,  100,
        TO_WEAPON,  0,  0,  0, false   
    }},
    {   affect::type::weapon_frost,           {
        "{BI{Cc{By",                      3,  100,
        TO_WEAPON,  0,  0,  0, false   
    }},
    {   affect::type::weapon_vampiric,        {
        "{TT{Bh{Ci{gr{Ts{Bt{Cy",          3,  100,
        TO_WEAPON,  0,  0,  0, false   
    }},
    {   affect::type::weapon_shocking,        {
        "{NA{Cr{Wc{Ni{Cn{Wg",             3,  100,
        TO_WEAPON,  0,  0,  0, false   
    }},
    {   affect::type::weapon_vorpal,          {
        "{gVo{Rrp{gal",                   3,  100,
        TO_WEAPON,  0,  0,  0, false   
    }},
    {   affect::type::weapon_sharp,           {
        "{gH{bo{gn{be{gd",                3,  100,
        TO_WEAPON,  0,  0,  0, false   
    }},
    {   affect::type::poison,                 {
        "{bTo{Hx{bic",                    3,  100,
        TO_WEAPON,  0,  0,  0, false 
    }},


    // SUFFIXES
    {   affect::type::obj_suffix_initiate_questor, {
        "of the Initiate Questor",      1,  100,
        TO_OBJECT,  APPLY_QUESTPOINTS,      1,  1, false
    }},
    {   affect::type::obj_suffix_advanced_questor, {
        "of the Advanced Questor",  1,  100,
        TO_OBJECT,  APPLY_QUESTPOINTS,  2,  2, false
    }},
    {   affect::type::obj_suffix_master_questor, {
        "of the Master Questor",    1,  100,
        TO_OBJECT,  APPLY_QUESTPOINTS,  3,  3, false
    }},
    {   affect::type::obj_suffix_skilled, {
        "of the Skilled",   1,  100,
        TO_OBJECT,  APPLY_SKILLPOINTS,  1,  1, false
    }},
    {   affect::type::obj_suffix_skillful, {
        "of the Skillful",  1,  100,
        TO_OBJECT,  APPLY_SKILLPOINTS,  2,  2, false
    }},
    {   affect::type::obj_suffix_experienced, {
        "of the Experienced",   1,  100,
        TO_OBJECT,  APPLY_EXP_PCT,  1,  3, false
    }},
    {   affect::type::obj_suffix_initiate_mage, {
        "of the Initiate Mage", 1,  100,
        TO_OBJECT,  APPLY_MANA_COST_PCT,    1,  2, false
    }},
    {   affect::type::obj_suffix_advanced_mage, {
        "of the Advanced Mage", 1,  100,
        TO_OBJECT,  APPLY_MANA_COST_PCT,    3,  4, false
    }},
    {   affect::type::obj_suffix_master_mage, {
        "of the Master Mage",   1,  100,
        TO_OBJECT,  APPLY_MANA_COST_PCT,    5,  6, false
    }},
    {   affect::type::obj_suffix_swallow, {
        "of the Swallow",   1,  100,
        TO_OBJECT,  APPLY_STAM_COST_PCT,    1,  3, false
    }},
    {   affect::type::obj_suffix_hawk, {
        "of the Hawk",  1,  100,
        TO_OBJECT,  APPLY_STAM_COST_PCT,    3,  6, false
    }},
    {   affect::type::obj_suffix_falcon, {
        "of the Falcon",    1,  100,
        TO_OBJECT,  APPLY_STAM_COST_PCT,    6,  10, false
    }},
    {   affect::type::obj_suffix_devastation, {
        "of Devastation",   1,  100,
        TO_OBJECT,  APPLY_WPN_DAMAGE_PCT,   1,  5, false
    }},
    {   affect::type::obj_suffix_annihilation, {
        "of Annihilation",  1,  100,
        TO_OBJECT,  APPLY_WPN_DAMAGE_PCT,   6,10, false
    }},
    {   affect::type::obj_suffix_arcane_power, {
        "of Arcane Power",  1,  100,
        TO_OBJECT,  APPLY_SPELL_DAMAGE_PCT, 1,  5, false
    }},
    {   affect::type::obj_suffix_mystical_power, {
        "of Mystical Power",    1,  100,
        TO_OBJECT,  APPLY_SPELL_DAMAGE_PCT, 6,10, false
    }},
    {   affect::type::obj_suffix_hexxing, {
        "of Hexxing",   1,  100,
        TO_OBJECT,  APPLY_VAMP_BONUS_PCT,   1,10, false
    }},
	{   affect::type::obj_suffix_restorative, {
        "of Restoration",   1,  100,
        TO_OBJECT,  APPLY_HP_BLOCK_PCT,   1,10, false
    }},
	{   affect::type::obj_suffix_mystic, {
        "of Mystic Energy",   1,  100,
        TO_OBJECT,  APPLY_MANA_BLOCK_PCT,   1,10, false
    }}

};
