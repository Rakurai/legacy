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
 **************************************************************************/
/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

#include <unistd.h>
#include <vector>

#include "act.hh"
#include "argument.hh"
#include "affect/Affect.hh"
#include "Auction.hh"
#include "channels.hh"
#include "Character.hh"
#include "Descriptor.hh"
#include "file.hh"
#include "find.hh"
#include "Game.hh"
#include "interp.hh"
#include "lookup.hh"
#include "Logging.hh"
#include "merc.hh"
#include "Object.hh"
#include "random.hh"
#include "Room.hh"
#include "String.hh"
#include "vt100.hh"
#include "comm.hh"
#include "World.hh"

/*
 * All the posing stuff.
 */

struct new_pose_struct {
	String self_msg;
	String room_msg;
};

std::vector<new_pose_struct> mage_poses = {
	{
		"You call up a {Ps{Yh{Ci{Pm{Cm{Ye{Pr{Ci{Yn{Pg{x sphere of power to repel attacks.",
		"A {Ps{Yh{Ci{Pm{Cm{Ye{Pr{Ci{Yn{Pg{x globe of force surrounds $n, repelling attacks."
	}, {
		"An unearthly calm settles about you as the {Cether{x gathers in preparation for your spell.",
		"The {Cair{x seems to {fwaver{x and a sense of {Gdread{x fills the area as $n nears completion of the spell."
	}, {
		"Your staff starts mouthing off to you.",
		"Strange, $n's staff sounds annoyed off. Waitaminute. IT TALKS?!?"
	}, {
		"Little {Pred lights{x dance in your eyes.",
		"Little {Pred lights{x dance in $n's eyes."
	}, {
		"*{PB{RO{PO{RM{x* OW!  Too much {csulfurous {gash{x leaves you patting out the {Pf{Yi{Pr{Ye{x on your robes.",
		"*{PB{RO{PO{RM{x* $n's spell backfired, leaving $m hastily patting out the {Pf{Yi{Pr{Ye{x."
	}, {
		"A {Hslimy {Ggreen monster{x appears before you and bows.",
		"A {Hslimy {Ggreen monster{x appears before $n and bows."
	}, {
		"*CRASH*BOOM* Your {W{fchain lightning{x wreaks havoc in the area.",
		"{fRUN!{x $n is casting {W{fchain lightning{x again."
	}, {
		"*{RGROWL{x*  The trapped {PDemon{x snarls, \"What now puny mageling?\"",
		"Don't panic.  It *looks* like $n has the {PDemon{x under control."
	}, {
		"A small {Yball of light{x dances on your fingertips.",
		"A small {Yball of light{x dances on $n's fingertips."
	}, {
		"{cS{Wm{co{Wk{ce{x and fumes leak from your nostrils.",
		"{cS{Wm{co{Wk{ce{x and fumes leak from $n's nostrils."
	}, {
		"You drain the {Ye{Pn{Ye{Pr{Yg{Py{x from the room and the waning {W{flight flickers{x.",
		"$n siphons the {Ye{Pn{Ye{Pr{Yg{Py{x  from the room and the waning {W{flight flickers{x."
	}, {
		"A {Pf{Yi{Pr{Ye {Re{Yl{Re{Ym{Re{Yn{Rt{Ya{Rl{x singes your hair.",
		"A {Pf{Yi{Pr{Ye {Re{Yl{Re{Ym{Re{Yn{Rt{Ya{Rl{x singes $n's hair."
	}, {
		"Stones float and careen off the walls at your beckoning.",
		"A {Tw{gh{Ti{gr{Tl{gw{Ti{gn{Td{x of stones fly around $n, lethal missiles awaiting a target."
	}, {
		"Everyone's clothes are {ftr{fan{fsp{far{fen{ft{x, and you are laughing.",
		"Your clothes are {ftransparent{x, and $n is laughing."
	},
};

std::vector<new_pose_struct> cleric_poses           = {
	{
		"The presence of your deity invigorates you and your heart soars.",
		"$n has a beatific smile on $s face. Conversations with higher powers do that to you."
	}, {
		"A {Ysunbeam{x pierces the clouds, bathing you in light.",
		"The sun pierces the clouds bathing $n in {Ysunlight{x."
	}, {
		"As you kneel in prayer, a voice thunders, \"What Now?\"",
		"As $n kneels in prayer, a voice thunders, \"What Now?\""
	}, {
		"Deep in prayer, you levitate.",
		"Deep in prayer, $n levitates."
	}, {
		"A {Bdeep blue aura{x surrounds you, bringing peace to all.",
		"A peaceful calm radiates from the {Bdeep azure aura{x surrounding $n."
	}, {
		"Your body glows with an {Yu{Wn{Ye{Wa{Yr{Wt{Yh{Wl{Yy {Wl{Yi{Wg{Yh{Wt{x.",
		"$n's body glows with an {Yu{Wn{Ye{Wa{Yr{Wt{Yh{Wl{Yy {Wl{Yi{Wg{Yh{Wt{x."
	}, {
		"Hands bathed in {Wwhite light{x, you grimace in pain as you heal the injured man's wounds.",
		"The mortally wounded man miraculously draws breath, while $n slumps from the healing spell's drain."
	}, {
		"So that everyone takes their watch tonight, you change all the {Mwine{x to {Cwater{x.",
		"*{fSPIT{x* THIS {MWINE'S{x BEEN TURNED INTO {BWATER{x! $n has been screwing around again!"
	}, {
		"A summoned {Cbreeze{x refreshes your party.",
		"$n summons a {Ccool breeze{x to refresh the party."
	}, {
		"The {Ysun{x pierces through the {gc{Cl{go{Cu{gd{Cs{x to illuminate you.",
		"The {Ysun{x pierces through the {gc{Cl{go{Cu{gd{Cs{x to illuminate $n."
	}, {
		"You summon a {gt{ch{gu{cn{gd{ce{gr {cc{gl{co{gu{cd{x to speed your travels.",
		"$n is whisked away by a summoned {cc{gl{co{gu{cd {gc{ch{ga{cr{gi{co{gt{x."
	}, {
		"The {PB{Ru{Yr{Pn{Ri{Yn{Pg {RM{Ya{Pn{x speaks to you.",
		"The {PB{Ru{Yr{Pn{Ri{Yn{Pg {RM{Ya{Pn{x speaks to $n."
	}, {
		"An {feye{x in a {Yp{by{Yr{ba{Ym{bi{Yd{x winks at you.",
		"An {feye{x in a {Yp{by{Yr{ba{Ym{bi{Yd{x winks at $n."
	}, {
		"A {Pcolumn{x of {Yf{Pi{Yr{Pe{x crashes down, bathing your shrieking enemies in {Ppurifying {Yf{Pl{Ya{Pm{Ye{Ps{x.",
		"A {Pcolumn{x of {Yf{Pi{Yr{Pe{x reduces $n's enemies to piles of {cash{x."
	}
};
std::vector<new_pose_struct> thief_poses           = {
	{
		"You chortle in the darkness as your target searches for you in the shadows.",
		"You hear {Cchilling laughter{x from the shadows."
	}, {
		"You catch a pickpocket trying to lighten your purse.*SLAP*",
		"You hear a resounding crack and see a young thief run from your party in tears."
	}, {
		"You disguise yourself as a beggar to earn some cash and plan your next heist.",
		"A poor beggar calls to you \"Alms? Alms for the poor, kind one?\""
	}, {
		"Noticing someone about to stab a friend, you whip your dagger into the foe's jugular.",
		"*SHTICK* You spin and find a would-be attacker clutching at a dagger in his throat."
	}, {
		"ALARM!THIEF!THIEF!",
		"ALARM!THIEF!THIEF!"
	}, {
		"You patiently tail the party, waiting for an opening.",
		"You hear {bleaves{x rustle."
	}, {
		"Click.",
		"Click."
	}, {
		"You grip your dagger in an iron fist and glare at the exposed neck before you.",
		"A nagging itch develops between your shoulderblades. Tread lightly."
	}, {
		"With practiced precision, you pierce the victim's lung, silencing any cry of alarm.",
		"You hear a {Rgurgle{x and some rustling. It appears someone is at work."
	}, {
		"A couple of street thugs recognize you and scurry away.",
		"You see a couple of thugs running away, looking back at your party."
	}, {
		"A fly lands on your nose. *WHACK* Ow!",
		"Bzzzz. *WHACK* Ow!"
	}, {
		"The Grey Mouser buys you a beer.",
		"The Grey Mouser buys $n a beer."
	}, {
		"You throw your voice and your target runs off to the rescue. Fresh Meat! *grin*rub*",
		"You hear a cry for help and rush to the rescue, finding nothing. Unfortunately, you feel a malevolent presence at your back...."
	}
};
std::vector<new_pose_struct> warrior_poses           = {
	{
		"You clobber everyone in a game of knuckle thumping.",
		"Your fingers are bruised and swollen after $n soundly trounces you in a game of knuckle thumping."
	}, {
		"Your weapon glows a {Rruddy red{x and melts armor like a warm knife through {Ybutter{x.",
		"$n's weapon is shedding {Rheat{x like a {Pfurnace{x and melting all metal it touches."
	}, {
		"Armor straps strain as you flex your muscles. *SNAP* *{fPTWING{x* ",
		"$n flexes $s muscles. You hear one of the straps creeeeaaak. *{fPTWING{x* "
	}, {
		"Your weapon starts humming a {Ymerry{x, {Pblood-thirsty{x tune as it hacks and slashes through the melee.",
		"You swear that $n's weapon is humming a bawdy tune punctuated by yelps and screeches of opponents."
	}, {
		"Wounds {Cfreeze{x and weapons shatter when your {Cice brand{x touches them.",
		"A {Cc{Bh{Ci{Bl{Cl{Bi{Cn{Bg{x aura flows from $n's weapon, {Cfreezing{x the room"
	}, {
		"*CRASH* You knock your opponents through walls with rolling thunder exploding every time you strike. *CRASH*{fBOOM{x*",
		"*CRASH* Every time $n hits anything, clouds roll and thunder deafens you. *CRASH*{fBOOM{x*"
	}, {
		"Whispering the command word, your weapon leaps from your hand to continue the battle.",
		"Brushing $s nails on $s shirt, $n's weapon dances around by itself, showering the enemy with numbing blows."
	}, {
		"Sweating with the exertion, you prepare to feel the meaty crunch of your opponents skull....and fall flat on your face.",
		"With blinding speed, $n does a spinning backhand and ... trips. *SPLUT*"
	}, {
		"A sickly {Ggreen aura{x flickers around your weapon, draining everyone of strength and will.",
		" As $n preps $s weapon, a sickly {Ggreen aura{x fills the room and drains your willpower."
	}, {
		"You bellow and snarl, banging your shield for attention. All eyes turn to you. Now what, Tarzan?",
		"$n bangs $s shield and howls, getting everyone's attention and is promptly jumped by the mobs."
	}, {
		"A dreadful keening wail pierces the air as your sword starts wielding YOU!",
		"A keening wail erupts from $n's weapon and it starts wielding $m! A look of horror grows on $s face and a second wail joins the first."
	}, {
		"Piercing metal like so much paper, your opponent is left with armor and dignity in tatters.",
		"Armor crumples and rips before $n's mighty blows, leaving opponents gibbering in terror."
	}, {
		"Your weapon starts whining, \"But I don't LIKE fighting! Why can't we just talk about it?\"",
		"$n's weapon starts whining, \"But I don't LIKE fighting! Why can't we just be friends?!?\""
	}, {
		"An {Teerie{x moan from your weapon chills your bones and {Yecstasy{x fills your mind as you feel stolen energy flow into your body.",
		"A {Pblood-curdling{x moan and $n's gasp of ecstasy are bad, but the feeling of insatiable hunger from $s weapon is absolutely terrifying."
	}
};
std::vector<new_pose_struct> necromancer_poses           = {
	{
		"After battle, using a {bdeep groaning chant{x, you raise up a party of {Hzombies{x.",
		"You feel a {Tchill{x as $n starts a {bgroaning chant{x that ends with a few corpses clambering to their feet."
	}, {
		"You ponder the materials needed for the {PRitual of Blood{x. Funny, your companions seem a little nervous.",
		"*{TShiver{x* $n's piercing gaze evokes memories of your darkest {cnightmares{x and most feared {Rdemons{x."
	}, {
		"You lovingly stroke the {gs{Wk{gu{Wl{gl{x atop your staff, savoring the {bbittersweet{x victory over your former master.",
		"An {Pevil{x grimace crosses $n's face while stroking the {Ws{gk{Wu{gl{Wl{x atop $s staff. "
	}, {
		"*Squish*Thud* One of your {bminions{x falls to pieces.",
		"*Squish*Thud* One of $n's {bminions{x falls to pieces."
	}, {
		"Looking around in disgust, you intone,\"ARISE!\" Your {gs{Wk{ge{Wl{ge{Wt{go{Wn{gs{x awaken and start putting themselves back together.",
		"\"ARISE,\" intones $n. $s party of {gs{Wk{ge{Wl{ge{Wt{go{Wn{gs{x start hooking there bones back together."
	}, {
		"Your sacrifice stares at you in terror as your hand passes through skin and bone to squeeze the life from its {Pheart{x.",
		"*shiver* The sacrifice gasps its last breath when $n thrusts $s hand into its chest and squeezes the life from its {Pheart{x."
	}, {
		"Thrusting your hand into your enemy's chest, you tear out his {Ps{Wo{Pu{Wl{x. Then, you wad it into a ball and hurl it into the ground. *WAAAaaiil.",
		"Before your eyes, $n rips a {Ps{Wo{Pu{Wl{x from its body and hurls it into the ground. *WAAAaaiil*"
	}, {
		"You raise the {s{Rs{Rh{Pa{Rd{Pe{x of your fallen foe and torture it beyond the grave. *SHRIEK*",
		"*SHRIEK* Tortured by $n, a {sshade's{x {gb{Wo{gn{We{x-chilling{x cries send shivers up your spine."
	}, {
		"Your spell strips the {Rs{Pk{Ri{Pn{x, tears at {Pm{Ru{Ps{Rc{Pl{Re{x, and breaks and rebreaks the {Wb{go{Wn{ge{Ws{x of the trespassers at the {PAltar{x of {RBlood{x.",
		"Your stomach churns and roils as you watch the thief flayed, broken, and ripped apart by $n's spell. *vomit* It last for *hours*."
	}, {
		"An evil gleam in your eye and the flash of light from your {Ps{Ca{Vc{Pr{Ci{Vf{Pi{Cc{Vi{Pa{Cl {Vd{Pa{Cg{Vg{Pe{Cr{x prompts the captive to loosen his tongue.",
		"$n whispers icily,\"{RB{Me{Rl{Mi{Ra{Ml{x and {bS{ce{bt{x will be pleased to receive your {Ps{Wo{Pu{Wl{x in trade.\" That wicked look and dagger encourage the captive to speak freely."
	}, {
		"You kiss the young girl, draining her life and leaving nothing but a {bdessicated shell{x.",
		"$n's kiss drains the girl's life from her body, leaving only a {bdried husk{x on the ground."
	}, {
		"After a deep groaning chant,  you are limned with a {Vvibrant violet aura{x, ready for the {PRitual{x of {RBlood{x.",
		"$n performs an eerie groaning chant that leaves him limned with a {Vviolet aura{x, allowing him to approach the {PAltar{x of {RBlood{x."
	}, {
		"A rider vaults from his {gpale horse{x and slaps you. \"We shall meet soon enough,\" he threatens.",
		"A rider vaults from his {gpale horse{x and slaps $n. He utters a threat and disappears."
	}, {
		"*DING* An elevator to the {RU{gn{Rd{ge{Rr{gw{Ro{gr{Rl{gd{x appears. An {Himp{x opens the door, \"Going down?\"",
		"$n presses a stud on $s staff and *DING* an elevator appears. \"PThe River Styx{x, {RHades{x, {YA{Rc{Yh{Re{Yr{Ro{Yn{x, and the {R666{x levels of the {VA{Pb{Vy{Ps{Vs{x. Going down.\""
	}
};
std::vector<new_pose_struct> paladin_poses           = {
	{
		"Shuutz. You feel the {Ylight{x gather around you in a {Yprotective shield{x",
		"Shuutz. A {Ygolden nimbus{x surrounds $n, protecting $m from harm."
	}, {
		"A {Tg{Wh{To{Ws{Tt{Wl{Ty{x hand stops a deadly swing at your head.",
		"A {Wg{Yl{Wo{Yw{Wi{Yn{Wg{x hand saves $n from a painful death."
	}, {
		"Beeeil. Your wounds mend while you meditate.",
		"Beeeil. $n's wounds begin to knit at an accelerated rate."
	}, {
		"The {YGrand {CMaster{x arrives to test your progress and thrashes you like a small child",
		"A small, {gw{Wi{gz{We{gn{We{gd{x man thrashes $n like a {Tn{bo{Tv{bi{Tc{be{x and then shakes his head, \"Pathetic.\""
	}, {
		"Auuug. A {Wbolt{x of {Ypure energy{x blinds your opponent.",
		"Auuug. A {Wblinding white bolt{x of {Yenergy{x blinds $n's opponent."
	}, {
		"You ponder a moment and sprint into the library vaults.",
		"$n scratches his head, then sprints into the endless shelves of books."
	}, {
		"Kaarta. A {Cs{Yh{Pi{Cm{Ym{Pe{Cr{Yi{Pn{Cg{x disc opens before you, displaying a map of the region.",
		"Kaarta. A {Cs{Yh{Pi{Cm{Ym{Pe{Cr{Yi{Pn{Cg{x disc opens before $n displaying a map of the region."
	}, {
		"Haaut. Deep in concentration, sword strikes and spell {P{fblasts{x slide from your {gr{co{gc{ck{g-{ch{ga{cr{gd{x skin like water.",
		"Haaut. $n's skin takes on a {ggreyish{x, {cs{gt{co{gn{ce{y{x hue, reflecting spell {P{fblasts{x and defying the sharpest sword."
	}, {
		"The {PP{Ro{Pw{Re{Pr{x of {gD{We{ca{Gt{Wh{x fills you and seeks release through your hands!",
		"$n gets a crazed look and prepares the {WF{Yi{WN{Yg{WE{Yr{x of {PD{cE{gA{PT{cH{x."
	}, {
		"Maaakt. A {Pred aura{x encompasses your fists, adding power to your blows.",
		"Maaakt. *{PCRASH{x* A bugbear flies across the room, thrown by $n, whose hands are glowing a {Pr{Ru{Pd{Rd{Py {Rr{Pe{Rd{x."
	}, {
		"The {YGrand {CMaster{x is here again. He pounds your {PE{YG{GO{x into a more manageable size.",
		"That {gw{Wi{gz{We{gn{We{gd{x old man is back. He stomps $n in a fight and mutters, \"Ego + Monk = Beating.\""
	}, {
		"Moooo. You feel an overwhelming desire to give {Wmilk{x.",
		"Moooo. $n says, \"{WMilk{x. It does the body good.\""
	}, {
		"*{WSLAP{x* Your opponent drops to the ground, his nerves shattered and body twitching.",
		"*{WSLAP{x* $n's opponent drops, another victim of the {f{PD{cE{gA{PT{cH {gT{PO{cU{gC{PH{x."
	}, {
		"*{YWhap{x*{CSMACK{x*{CSMACK{x*{gPunch{x*{PC{YR{RU{GS{PH{x*{CSMACK{x*{Y{fSLAP{x*{RKick{x*{G{fSLAM{x* --<<{PF{RA{PT{RA{PL{RI{PT{RY{x>>--",
		"*{YWhap{x*{CSMACK{x*{CSMACK{x*{gPunch{x*{PC{YR{RU{GS{PH{x*{CSMACK{x*{Y{fSLAP{x*{RKick{x*{G{fSLAM{x* --<<{PF{RA{PT{RA{PL{RI{PT{RY{x>>-- $n grins and bows."
	}
};
std::vector<new_pose_struct> bard_poses           = {
	{
		"The {Pheart rending{x ballad of the Goddess' lost love almost cripples you with {Tsorrow{x.",
		"$n's {Pheart rending{x ballad of lost love leaves you feeling the Goddess' {Tdevastation{x."
	}, {
		"Stones float and careen off the walls at your beckoning.",
		"A whirlwind of stones fly around $n, protecting $m from harm."
	}, {
		"You see the helplessness and hopelessness of the {CC{Yh{Ca{Ym{Cp{Yi{Co{Yn {CE{Yt{Ce{Yr{Cn{Ya{Cl{x in the eyes of your audience.",
		"$n's tragic legend of the {YE{Ct{Ye{Cr{Yn{Ca{Yl {CC{Yh{Ca{Ym{Cp{Yi{Co{Yn{x leaves you drained and hopeless."
	}, {
		"You juggle small {Yballs of light{x, entertaining the crowd..",
		"$n juggles 10 small {Yballs of light{x without breaking a sweat."
	}, {
		"You patiently tail the party, waiting for an opening.",
		"You hear {bbranches{x rustle."
	}, {
		"The tale of Lotus' epic adventure through {PC{Yh{Pa{Yo{Ps{x and {BC{Gr{Be{Ga{Bt{Gi{Bo{Gn{x leaves the audience speechless.",
		"$n's legend of Lotus' journey through {PC{Yh{Pa{Yo{Ps{x and {BC{Gr{Be{Ga{Bt{Gi{Bo{Gn{x leaves you humbled and awed by his works. "
	}, {
		"Your weapon starts singing a {Ymerry{x, {Pblood-thirsty{x tune as it hacks and slashes through the melee.",
		"You swear $n's weapon is singing a bawdy tune punctuated by raucous yelps and screeches."
	}, {
		"A summoned {Cbreeze{x refreshes your party.",
		"$n summons a {Bcool zephyr{x to revitalize the party."
	}, {
		"Clouds roll in and rain falls as your {Vmelancholy melody{x forces even {GMother Earth{x to weep.",
		"Clouds roll in and rain falls as $n's {Vmelancholy melody{x forces even {GMother Earth{x to weep."
	}, {
		"Noticing someone about to stab a friend, you whip your dagger into the foe's jugular.",
		"*SHTICK* You spin and find an attacker clutching a dart in his throat."
	}, {
		"Your thunderous chant praising the {PGod of War{x rallies your forces from defeat.",
		"A chant thunders across the battlefield, renewing your strength and filling you with the {PWRATH of War{x."
	}, {
		"Sweating with the exertion, you attempt to trip your enemy .... and fall flat on your back.",
		"Faster than the eye, $n ducks down to trip his enemy and ... trips. *SPLUT*"
	}, {
		"You summon a {gt{ch{gu{cn{gd{ce{gr {cc{gl{co{gu{cd{x to talk about the sad weather.",
		"$n speaks with a summoned {gt{ch{gu{cn{gd{ce{gr {cc{gl{co{gu{cd{x to improve the weather."
	}, {
		"Your {Ywistful tune{x catches the ear of the {WPatron of Music{x who stops for a visit.",
		"On the notes of a {Ywistful tune{x, the {WPatron of Music{x drops in to visit $n."
	},
};
std::vector<new_pose_struct> ranger_poses           = {
	{
		"You trail behind the party acting as rearguard.",
		"You hear {btwigs{x snap."
	}, {
		"Some of the local creatures chatter at you in greeting.",
		"$n smiles and speaks with some of the local fauna"
	}, {
		"\"AMBUSH!!\" You dive into the {Hunderbrush{x, weapons flying.",
		"\"AMBUSH!!\" $n dives into the {Hunderbrush{x, weapons flying."
	}, {
		"Sadly, a giant wombat lands on your head.",
		"$n is set upon by furry forest critters. Its best not to ask."
	}, {
		"Roaming ahead of the party, you catch some {Gorcs{x napping and quickly dispatch them.",
		"You hear a scuffle ahead and find a group of {Gorcs{x in various stages of death. $n has been hogging all the fun again."
	}, {
		"Using your {Hherbs{x, you make a remedy for that nasty hangover.",
		"After some {Hherbal tea{x, $n's hangover seems to have lessened."
	}, {
		"Poking your head through the underbrush, you come nose to 'nose' with the business end of a {Ws{ck{Wu{cn{Wk{x.",
		"*sniff*WHEW* $n breaks into the clearing spitting and wiping $s face in an attempt to remove {Ws{ck{Wu{cn{Wk{x spew."
	}, {
		"You notice something odd and tap ahead with a stout branch. *{fSNAP{x* Hmm....bear trap.",
		"$n stops you and taps ahead with a stout branch. *{fSNAP{x* Hmm...bear trap."
	}, {
		"*ROAR* You smile and scratch the {bbrown bear's{x ears as your party scrambles to fend off the ambush.",
		"*ROAR* WHATTHE?!? You scramble to your feet and see $n smugly scratching a {bgiant brown bear's{x ears."
	}, {
		"You forge ahead, acting a vanguard for the party.",
		"\"I'll scout\", says $n who vanishes into the {Gtrees{x."
	}, {
		"A {Wsilver wolf{x pads toward you, \"Greetings of the {Hforest{x to you, Woodfriend.\"",
		"A {Wsilver wolf{x sits on its haunches and converses with $n for a time."
	}, {
		"You step on a twig. *{fSNAP{x *mutter*",
		"*{fSNAP{x * You hear muttered curses."
	}, {
		"Your arrows rapidly turn the {HOgre's rump{x into a pincushion.",
		"An {HOgre{x flees in humiliation with $n's arrows sticking in its butt."
	}, {
		"With extreme caution, you approach the {Wu{Yn{Wi{Yc{Wo{Yr{Wn{x and stroke her mane.",
		"$n cautiously approaches the {Wu{Yn{Wi{Yc{Wo{Yr{Wn{x and strokes her mane."
	}
};

std::vector<std::vector<new_pose_struct> > new_pose_table      = {
	mage_poses,       
	cleric_poses,     
	thief_poses,      
	warrior_poses,    
	necromancer_poses,
	paladin_poses,    
	bard_poses,       
	ranger_poses,     
};

/* New POSE command which is more flexible with regard to
   the number of poses available per class -- Elrac */
int select_pose(Character *ch)
{
	int level;
	int maxpose;
	int pose;

	if (ch->is_npc()) {
		stc("Sorry, mobiles can't pose!\n", ch);
		return -1;
	}

	if (ch->guild == Guild::none) {
		stc("You don't feel confident enough in your skills to pose.\n", ch);
		return -1;
	}

	maxpose = new_pose_table[ch->guild].size();

	if (maxpose <= 0) {
		stc("No poses implemented for your class, sorry!\n", ch);
		return -1;
	}

	level = ch->level;
	maxpose = (level * maxpose + MAX_LEVEL - 1) / MAX_LEVEL;
	pose  = number_range(0, maxpose - 1);
	return pose;
}

void do_pose(Character *ch, String argument)
{
	int pose;
	pose = select_pose(ch);

	if (pose == -1)
		return;

	act(new_pose_table[ch->guild][pose].self_msg, ch, nullptr, nullptr, TO_CHAR);
	act(new_pose_table[ch->guild][pose].room_msg, ch, nullptr, nullptr, TO_ROOM);
	return;
}

/* code to test poses -- Elrac */
void do_testpose(Character *ch, String argument)
{
	if (argument.empty()) {
		stc("Syntax: testpose [class [number]]\n", ch);
		return;
	}

	String arg;
	argument = one_argument(argument, arg);
	Guild guild = guild_lookup(arg);

	if (guild == Guild::none) {
		stc("That's not a valid class\n", ch);
		return;
	}

	if (argument.empty()) {
		Format::sprintf(arg, "%d poses defined for class '%s'\n",
		        new_pose_table[guild].size(), guild_table[guild].name);
		stc(arg, ch);
		return;
	}

	argument = one_argument(argument, arg);

	if (!arg.is_number()) {
		stc("Pose number must be numeric!\n", ch);
		return;
	}

	unsigned int pose = atoi(arg); // 1-indexed

	if (pose < 1 || pose > new_pose_table[guild].size()) {
		stc("Pose number out of range!\n", ch);
		return;
	}

	pose--;
	stc("{Yto {Cyourself{x:\n", ch);
	act(new_pose_table[guild][pose].self_msg, ch, nullptr, nullptr, TO_CHAR);
	stc("{Yto {Cothers{x:\n", ch);
	act(new_pose_table[guild][pose].room_msg, ch, nullptr, nullptr, TO_CHAR);
} /* end do_testpose() */

/* RT code to delete yourself */
void do_delet(Character *ch, String argument)
{
	stc("You must type the full command to delete yourself.\n", ch);
}

void do_delete(Character *ch, String argument)
{
	Descriptor *d, *d_next;
	int id;
	char strsave[MAX_INPUT_LENGTH];
	bool important = true;

	if (ch->is_npc()) {
		stc("You're not getting out of here that easily!\n", ch);
		return;
	}

	if (get_duel(ch)) {
		stc("You are involved in a duel, deal with it first.\n", ch);
		return;
	}

	switch (ch->pcdata->confirm_delete) {
	case 0:     /* first DELETE */
		if (!argument.empty()) {
			stc("Just type DELETE. No argument.\n", ch);
			return;
		}

		wiznet("$N is contemplating deletion.", ch, nullptr, 0, 0, GET_RANK(ch));
		ptc(ch, "{RType {PDELETE{R again if you really want to delete '%s{R';\n{x", ch->name);
		stc("{Rtype {PDELETE NO{R to cancel this command.{x\n", ch);
		ch->pcdata->confirm_delete = 1;
		break;

	case 1:     /* second DELETE */
		if (!argument.empty()) {
			stc("Delete status removed.\n", ch);
			ch->pcdata->confirm_delete = 0;
			return;
		}

		if (ch->pcdata->remort_count > 0)
			ptc(ch, "This is a %dx remort character.\n", ch->pcdata->remort_count);
		else if (get_play_hours(ch) >= 100)
			ptc(ch, "You have played this character for approximately %d hours.\n",
			    get_play_hours(ch));
		else if (ch->level >= 30)
			ptc(ch, "This is a level %d character.\n", ch->level);
		else
			important = false;

		if (important) {
			stc("You could still delay your decision, {RQUIT{x and think it over.\n", ch);
			stc("Be aware that this decision is {RFINAL{x;\nWe will {RNOT{x resurrect you.\n", ch);
		}

		ptc(ch, "{RType {PDELETE{R one last time if you really, really want to delete '%s{R';{x",
		    ch->name);
		stc("{Rtype {PDELETE NO{R to cancel this command.{x\n", ch);
		ch->pcdata->confirm_delete = 2;
		break;

	case 2:     /* third DELETE */
		if (!argument.empty()) {
			stc("Delete status removed.\n", ch);
			ch->pcdata->confirm_delete = 0;
			return;
		}

		Format::sprintf(strsave, "%s%s", PLAYER_DIR, String(ch->name).lowercase().capitalize());
		wiznet("$N has wiped $Mself from these realms.", ch, nullptr, 0, 0, 0);
		update_pc_index(ch, true);
		id = ch->pcdata->id;
		d = ch->desc;

		if (ch->pcdata && ch->pcdata->video_flags.has(VIDEO_VT100)) {
			clear_window(ch);
			reset_terminal(ch);
		}

		extract_char(ch, true);

		if (d != nullptr)
			close_socket(d);

		/* toast evil cheating bastards */
		for (d = descriptor_list; d != nullptr; d = d_next) {
			Character *tch;
			d_next = d->next;
			tch = d->original ? d->original : d->character;

			if (tch && tch->pcdata && tch->pcdata->id == id) {
				extract_char(tch, true);
				close_socket(d);
			}
		}

		unlink(strsave);
		break;

	default:
		stc("For technical reasons, you cannot delete yourself just now.\n"
		    "Please tell an Imm about this.\n", ch);
		Logging::bug("do_delete: bad confirm_delete: %d ", ch->pcdata->confirm_delete);
		break;
	} /* end switch */
} /* end do_delete() */

/* Lotus - Clear Screen Quickie */
void do_clear(Character *ch, String argument)
{
	stc("\033[2J", ch);
}

/* Newbie Bags by Lotus */
void do_newbiekit(Character *ch, String argument)
{
	Object *obj;
	Object *kit;
	int i, kitvnum = 72;

	if (!IS_IMMORTAL(ch) && !ch->act_flags.has(PLR_MAKEBAG)) {
		do_huh(ch);
		return;
	}

	kit = create_object(Game::world().get_obj_prototype(kitvnum), 0);

	if (! kit) {
		Logging::bug("Error creating kit in do_newbiekit.", 0);
		stc("You were unable to create a newbiekit.\n", 0);
		return;
	}

	/* Newbie kit items are coded in consecutive OBJ VNUMs 73 to 83 in LIMBO.ARE */
	for (i = 73; i <= 83 ; i++) {
		obj = create_object(Game::world().get_obj_prototype(i), 0);

		if (! obj) {
			Logging::bug("Error creating object in do_newbiekit.", 0);
			return;
		}

		obj_to_obj(obj, kit);
	}

	obj_to_char(kit, ch);
	act("$n has created a $p!!!", ch, kit, nullptr, TO_ROOM);
	act("You have created $p!! WooWoo!!!", ch, kit, nullptr, TO_CHAR);
}

/* OOC by Lotus */
void do_ooc(Character *ch, String argument)
{
	if (ch->is_npc()) {
		stc("Mobiles don't care about RP =).\n", ch);
		return;
	}

	if (ch->pcdata->plr_flags.has(PLR_OOC)) {
		stc("You are no longer role playing.\n", ch);
		ch->pcdata->plr_flags -= PLR_OOC;
		wiznet("$N is now in *OOC* mode.", ch, nullptr, WIZ_MISC, 0, 0);
	}
	else {
		stc("You slip into character.\n", ch);
		ch->pcdata->plr_flags += PLR_OOC;
		wiznet("$N is now in *RP* mode.", ch, nullptr, WIZ_MISC, 0, 0);
	}
}

void do_pk(Character *ch, String argument)
{
	Character *wch;

	if (ch->is_npc()) {
		stc("Mobiles don't care about PK =).\n", ch);
		return;
	}

	if (ch->act_flags.has(PLR_NOPK)) {
		stc("You are stuck in PK mode!  I hope no one is trying to kill you.\n", ch);
		return;
	}

	if (ch->pcdata->plr_flags.has(PLR_PK) && (argument.empty() || !IS_IMP(ch)))
		stc("Your PK flag is already up!\n", ch);
	else if (argument == "on") {
		stc("Everyone is going to be keeping an eye on you now =).\n", ch);
		ch->pcdata->plr_flags += PLR_PK;
		wiznet("$N is now in *PK* mode.", ch, nullptr, WIZ_MISC, 0, 0);
	}
	else if (!argument.empty() && IS_IMP(ch)
	         && (wch = get_player_world(ch, argument, VIS_PLR)) != nullptr) {
		if (!wch->pcdata->plr_flags.has(PLR_PK)) {
			stc("That player's PK flag is already down.\n", ch);
			return;
		}

		wch->pcdata->plr_flags -= PLR_PK;
		wiznet("$N's PK flag has been lowered.", wch, nullptr, WIZ_MISC, 0, 0);
	}
	else {
		stc("Once you raise your PK flag it stays up until someone kills you!\n"
		    "If you are sure you want this, type PK ON .\n", ch);
	}
} /* end do_pk() */

/* Chatmode by Lotus */
void do_chatmode(Character *ch, String argument)
{
	if (ch->is_npc())
		return;

	if (ch->pcdata->plr_flags.has(PLR_CHATMODE)) {
		stc("You have exited chatmode.\n", ch);
		ch->pcdata->plr_flags -= PLR_CHATMODE;
	}
	else {
		stc("You will no longer see hunger/thirst messages.\n", ch);
		ch->pcdata->plr_flags += PLR_CHATMODE;
	}
}

/* Private for swho by Lotus */
void do_private(Character *ch, String argument)
{
	if (ch->is_npc()) {
		stc("Mobiles don't care about privacy =).\n", ch);
		return;
	}

	if (ch->pcdata->plr_flags.has(PLR_PRIVATE)) {
		stc("Your room will now display in shortwho.\n", ch);
		ch->pcdata->plr_flags -= PLR_PRIVATE;
	}
	else {
		stc("Your room will no longer display in shortwho.\n", ch);
		ch->pcdata->plr_flags += PLR_PRIVATE;
	}
}

/* showlast for finger by Lotus */
void do_showlast(Character *ch, String argument)
{
	if (ch->is_npc()) {
		stc("Mobiles don't care about their showlast =).\n", ch);
		return;
	}

	if (ch->pcdata->plr_flags.has(PLR_NOSHOWLAST)) {
		ch->pcdata->plr_flags -= PLR_NOSHOWLAST;
		stc("Your last time on will now display in finger.\n", ch);
	}
	else {
		ch->pcdata->plr_flags += PLR_NOSHOWLAST;
		stc("Your last time on will no longer display in finger.\n", ch);
	}

	do_save(ch, "");
} /* end do_showlast() */

/* Autorecall by Lotus */
void do_autorecall(Character *ch, String argument)
{
	if (ch->act_flags.has(PLR_WIMPY)) {
		stc("You will no longer recall in link-dead combat.\n", ch);
		ch->act_flags -= PLR_WIMPY;
	}
	else {
		stc("You will now recall in link-dead combat.\n", ch);
		ch->act_flags += PLR_WIMPY;
	}
}
/* PlayerTicks by Lotus */
void do_autotick(Character *ch, String argument)
{
	if (ch->act_flags.has(PLR_TICKS)) {
		stc("You will no longer see ticks.\n", ch);
		ch->act_flags -= PLR_TICKS;
	}
	else {
		stc("You will now see ticks.\n", ch);
		ch->act_flags += PLR_TICKS;
	}
}

/* AutoPeek -- Elrac */
void do_autopeek(Character *ch, String argument)
{
	if (ch->is_npc()) return;

	if (!get_skill_level(ch, skill::type::peek)) return;

	if (ch->pcdata->plr_flags.has(PLR_AUTOPEEK)) {
		stc("You will no longer PEEK automatically.\n", ch);
		ch->pcdata->plr_flags -= PLR_AUTOPEEK;
	}
	else {
		stc("You will now PEEK automatically when you LOOK.\n", ch);
		ch->pcdata->plr_flags += PLR_AUTOPEEK;
	}
}

/* ShowRaff - shows raffects in 'aff' */
void do_showraff(Character *ch, String argument)
{
	if (ch->is_npc()) {
		stc("Huh?\n", ch);
		return;
	}

	if (ch->pcdata->remort_count < 1) {
		stc("You have not been reborn yet.\n", ch);
		return;
	}

	if (ch->pcdata->plr_flags.has(PLR_SHOWRAFF)) {
		stc("Your remort affects will no longer show in 'affects'.\n", ch);
		ch->pcdata->plr_flags -= PLR_SHOWRAFF;
	}
	else {
		stc("Your remort affects will now show in 'affects'.\n", ch);
		ch->pcdata->plr_flags += PLR_SHOWRAFF;
	}
}

/* RT deaf blocks out all shouts */
void do_deaf(Character *ch, String argument)
{
	if (ch->comm_flags.has(COMM_DEAF)) {
		new_color(ch, CSLOT_CHAN_TELL);
		stc("You can now hear tells again.\n", ch);
		ch->comm_flags -= COMM_DEAF;
		set_color(ch, WHITE, NOBOLD);
	}
	else {
		new_color(ch, CSLOT_CHAN_TELL);
		stc("From now on, you won't hear tells.\n", ch);
		ch->comm_flags += COMM_DEAF;
		set_color(ch, WHITE, NOBOLD);
	}
}

/* RT quiet blocks out all communication */

void do_quiet(Character *ch, String argument)
{
	if (ch->comm_flags.has(COMM_QUIET)) {
		stc("Quiet mode removed.\n", ch);
		ch->comm_flags -= COMM_QUIET;
	}
	else {
		stc("From now on, you will only hear says and emotes.\n", ch);
		ch->comm_flags += COMM_QUIET;
	}
}

/* afk command */

void do_afk(Character *ch, String argument)
{
	char *strtime;

	if (ch->comm_flags.has(COMM_AFK)) {
		set_color(ch, YELLOW, NOBOLD);
		stc("AFK mode removed.\n", ch);

		if (!ch->is_npc())
			if (!ch->pcdata->buffer.empty())
				stc("You have messages: Type 'replay'\n", ch);

		ch->comm_flags -= COMM_AFK;
		set_color(ch, WHITE, NOBOLD);
		/*
		      act( "$N has returned to $S keyboard.", ch, nullptr, nullptr, TO_ROOM );
		*/
		wiznet("$N has returned to $S keyboard.", ch, nullptr, WIZ_MISC, 0, 0);
	}
	else {
		strtime                         = ctime(&Game::current_time);
		strtime[strlen(strtime) - 1]      = '\0';
		set_color(ch, YELLOW, NOBOLD);
		stc("You are now in AFK mode.\n", ch);
		ch->comm_flags += COMM_AFK;
		set_color(ch, WHITE, NOBOLD);
		wiznet("$N has gone AFK.", ch, nullptr, WIZ_MISC, 0, 0);

		/*
		     act( "$N has gone AFK.", ch, nullptr, nullptr, TO_ROOM );
		*/
		if (!ch->is_npc()) {
			char buf[MAX_STRING_LENGTH];

			if (!argument.empty())
				Format::sprintf(buf, "{B[{C%s{B] {C%s\n", strtime,
				        argument);
			else
				Format::sprintf(buf, "{B[{C%s{B] {CUnknown Departure.{x\n",
				        strtime);

			ch->pcdata->afk = buf;
			stc(buf, ch);
		}
	}
}

/* Note notify by PwrDemon */
void do_notify(Character *ch, String argument)
{
	if (ch->is_npc()) return;

	if (ch->pcdata->plr_flags.has(PLR_NONOTIFY)) {
		stc("From now on, you will be notified of new notes.\n", ch);
		ch->pcdata->plr_flags -= PLR_NONOTIFY;
	}
	else {
		stc("Note notify mode removed.\n", ch);
		ch->pcdata->plr_flags += PLR_NONOTIFY;
	}
}

/* Append a string to a file, used for our in game text files */
void update_text_file(Character *ch, const String& file, const String& str)
{
	if (ch->is_npc() || str.empty())
		return;

	const char *time_format = "%m/%d/%y";
	char timebuf[100];
	strftime(timebuf, sizeof(timebuf), time_format, localtime(&Game::current_time));

	String buf = Format::format("{Y[{x%8s{Y]{x {C[{x%s{C]{x %s: %s\n",
	        timebuf,
	        ch->in_room ? ch->in_room->location.to_string(false) : "    0",
	        ch->name,
	        str);

	fappend(file, buf);
}

void do_wbi(Character *ch, String argument)
{
	if (argument.empty()) {
		do_file(ch, "wbi 150");
		return;
	}

	update_text_file(ch, WBI_FILE, argument);
	stc("And may it be imped soon! :)\n", ch);
}

void do_hbi(Character *ch, String argument)
{
	if (argument.empty()) {
		do_file(ch, "hbi 150");
		return;
	}

	update_text_file(ch, HBI_FILE, argument);
	stc("It will be posted in a change note soon! :)\n", ch);
}

void do_wbb(Character *ch, String argument)
{
	if (argument.empty()) {
		do_file(ch, "wbb 150");
		return;
	}

	update_text_file(ch, WBB_FILE, argument);
	stc("And may it be built soon! :)\n", ch);
}

void do_hbb(Character *ch, String argument)
{
	if (argument.empty()) {
		do_file(ch, "hbb 150");
		return;
	}

	update_text_file(ch, HBB_FILE, argument);
	stc("It will be posted in a change note soon! :)\n", ch);
}

void do_work(Character *ch, String argument)
{
	if (argument.empty()) {
		do_file(ch, "work 150");
		return;
	}

	update_text_file(ch, WORK_FILE, argument);
	stc("May your ideas be developed soon! :)\n", ch);
}

void do_immapp(Character *ch, String argument)
{
	if (argument.empty()) {
		do_file(ch, "immapp 150");
		return;
	}

	update_text_file(ch, IMMAPP_FILE, argument);
	stc("May they be considered soon! :)\n", ch);
}

void do_ridea(Character *ch, String argument)
{
	if (argument.empty()) {
		do_file(ch, "ridea 150");
		return;
	}

	update_text_file(ch, RIDEA_FILE, argument);
	stc("And may it be debated soon! :)\n", ch);
}

void do_punish(Character *ch, String argument)
{
	if (argument.empty()) {
		do_file(ch, "punishment 150");
		return;
	}

	update_text_file(ch, PUNISHMENT_FILE, argument);
	stc("Punishment logged.\n", ch);
}

void do_bug(Character *ch, String argument)
{
	if (argument.empty()) {
		if (IS_IMMORTAL(ch))
			do_file(ch, "bugs 150");

		stc("Which bug would you like to report?\n"
		    "Syntax: {Rbug{x your description of the bug\n", ch);
		return;
	}

	update_text_file(ch, BUG_FILE, argument);
	stc("Bug logged.\n", ch);
}

void do_typo(Character *ch, String argument)
{
	if (argument.empty()) {
		if (IS_IMMORTAL(ch))
			do_file(ch, "typos 150");

		stc("Which typo would you like to report?\n"
		    "Syntax: {Rtypo{x your description of the typo\n", ch);
		return;
	}

	update_text_file(ch, TYPO_FILE, argument);
	stc("Typo logged.\n", ch);
}

void do_qui(Character *ch, String argument)
{
	stc("If you want to QUIT, you have to spell it out.\n", ch);
	return;
}

/* recursive showlost -- Montrey */
// this code is inactive, leaving it in case we decide otherwise
bool showlost(Character *ch, Object *obj, bool found, bool locker)
{
	for (; obj != nullptr; obj = obj->next_content) {
		if ((obj->level > get_holdable_level(ch))
		    || (obj->item_type == ITEM_KEY && (obj->value[0] == 0))
		    || (obj->item_type == ITEM_MAP && !obj->value[0])) {
			if (!found)
				ptc(ch, "{CThe following items in your %s will be lost if you quit:{x\n",
				    locker ? "locker" : "inventory");

			found = true;
			ptc(ch, "%s\n", obj->short_descr);
		}

		if (obj->contains)
			found = showlost(ch, obj->contains, found, locker);
	}

	return found;
}

void do_quit(Character *ch, String argument)
{
	Descriptor *d, *d_next, *sd;
	Character *victim;
	int id, lnum;
	char *const message [] = {
		"Your world shatters into a billion numbers circling around you.\n  They all flash, '{G0{x', and fade into blackness.\n",
		"Stop, come back, you haven't broken Ramaru's time online record yet!\n",
		"This concludes our broadcast of the emergency mudding system.\n",
		"Thank you, please drive through.\n",
		"What???? You're not addicted yet?????\n",
		"{PYou have been KILLED!!!{x\n",
		"Today, Tomorrow, Yesterday...on the mud, they are all the same.\n",
		"Whoops, Alexia accidentally hits your power switch with her big toe!\n",
		"It's not a bug, it's a feature!!!\n",
		"Kefta boots you in the head on your way out.  OuCH!!!\n"
	};

	if (ch->is_npc()) {
		stc("Ha!  You are forever doomed to stay on Legacy, deal with it.\n", ch);
		return;
	}

	if (ch->pnote) {
		stc("You still have a note in progress, finish it first please.\n", ch);
		return;
	}

	if (ch->fighting) {
		stc("No way! You are fighting.\n", ch);
		return;
	}

	if (ch->in_room->sector_type() == Sector::arena) {
		stc("You're not getting out of this that easy.\n", ch);
		return;
	}

	if (ch->pcdata->pktimer > 0) {
		stc("Your blood is racing too fast to leave now!!\n", ch);
		return;
	}

	if (get_duel(ch)) {
		stc("You are involved in a duel, deal with it first.\n", ch);
		return;
	}

	if (get_position(ch) <= POS_STUNNED) {
		stc("You're not DEAD yet.\n", ch);
		return;
	}

	if (auction.is_participant(ch)) {
		stc("Wait till you have sold/bought the item on auction.\n", ch);
		return;
	}
/*
	if (ch->pcdata->plr_flags.has(PLR_SHOWLOST) && argument.empty() && !IS_IMMORTAL(ch)) {
		bool found_inv, found_loc;
		found_inv = showlost(ch, ch->carrying, false, false);
		found_loc = showlost(ch, ch->pcdata->locker,   false, true);

		if (found_inv || found_loc)
			return;
	}
*/
	lnum = get_locker_number(ch);

	if (lnum > 0 && !IS_IMMORTAL(ch)) {
		if (deduct_cost(ch, lnum * 10)) {
			ptc(ch, "%d silver has been deducted for your locker.\n",
			    lnum * 10);
			ch->act_flags -= PLR_CLOSED;
		}
		else {
			stc("You cannot afford to pay your locker fee.\n", ch);
			stc("Your locker has been closed.\n", ch);
			ch->act_flags += PLR_CLOSED;
		}
	}

	stc(message[number_range(0, 9)], ch);

	for (sd = descriptor_list; sd != nullptr; sd = sd->next) {
		victim = sd->original ? sd->original : sd->character;

		if (sd->is_playing()
		    && sd->character != ch
		    && can_see_who(victim, ch)
		    && !victim->comm_flags.has(COMM_NOANNOUNCE)
		    && !victim->comm_flags.has(COMM_QUIET)) {
			if (ch->pcdata && !ch->pcdata->gameout.empty()) {
				set_color(victim, GREEN, BOLD);
				ptc(victim, "[%s] %s\n", ch->name, ch->pcdata->gameout);
			}
			else {
				new_color(victim, CSLOT_CHAN_ANNOUNCE);
				ptc(victim, "[FYI] %s has quit.\n", ch->name);
			}

			set_color(victim, WHITE, NOBOLD);
		}
	}

	wiznet("$N rejoins the real world.", ch, nullptr, WIZ_LOGINS, 0, GET_RANK(ch));
	save_char_obj(ch);
	id = ch->pcdata->id;
	d = ch->desc;

	if (ch->pcdata && ch->pcdata->video_flags.has(VIDEO_VT100)) {
		stc(VT_SETWIN_CLEAR, ch);
		stc(VT_RESET_TERMINAL, ch);
	}

	extract_char(ch, true);

	/* After extract_char the ch is no longer valid! */
	if (d != nullptr)
		close_socket(d);

	/* toast evil cheating bastards */
	for (d = descriptor_list; d != nullptr; d = d_next) {
		Character *tch;
		d_next = d->next;
		tch = d->original ? d->original : d->character;

		if (tch && tch->pcdata && tch->pcdata->id == id) {
			extract_char(tch, true);
			close_socket(d);
		}
	}
}

void do_fuckoff(Character *ch, String argument)
{
	Descriptor *d, *d_next;
	int id;

	if (ch->is_npc())
		return;

	String log_buf = Format::format("%s has been fried.", ch->name);
	do_send_announce(ch, log_buf);
	Logging::log(log_buf);
	wiznet("$N has been terminated.", ch, nullptr, WIZ_LOGINS, 0, GET_RANK(ch));
	/*
	 * After extract_char the ch is no longer valid!
	 */
	save_char_obj(ch);
	id = ch->pcdata->id;
	d = ch->desc;
	extract_char(ch, true);

	if (d != nullptr)
		close_socket(d);

	/* toast evil cheating bastards */
	for (d = descriptor_list; d != nullptr; d = d_next) {
		Character *tch;
		d_next = d->next;
		tch = d->original ? d->original : d->character;

		if (tch && tch->pcdata && tch->pcdata->id == id) {
			extract_char(tch, true);
			close_socket(d);
		}
	}
}

void do_backup(Character *ch, String argument)
{
	if (ch->is_npc())
		return;

	stc("Backing up pfile.\n", ch);
	backup_char_obj(ch);
	WAIT_STATE(ch, 4 * PULSE_VIOLENCE);
	stc("Done.\n", ch);
	ch->pcdata->backup = get_play_seconds(ch);
	return;
}

void do_save(Character *ch, String argument)
{
	if (ch->is_npc())
		return;

	save_char_obj(ch);
	stc("Saving. Remember, Legacy has automatic saving.\n", ch);
	WAIT_STATE(ch, 4 * PULSE_VIOLENCE);
	return;
}

void do_follow(Character *ch, String argument)
{
	/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
	Character *victim;

	if (argument.empty()) {
		stc("Follow whom?\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (affect::exists_on_char(ch, affect::type::charm_person) && ch->master != nullptr) {
		act("But you'd rather follow $N!", ch, nullptr, ch->master, TO_CHAR);
		return;
	}

	if (victim == ch) {
		if (ch->master == nullptr) {
			stc("You now stare at your own butt.\n", ch);
			return;
		}

		stop_follower(ch);
		return;
	}

	if (!victim->is_npc() && victim->act_flags.has(PLR_NOFOLLOW) && !IS_IMMORTAL(ch)) {
		act("$N doesn't seem to want any followers.\n",
		    ch, nullptr, victim, TO_CHAR);
		return;
	}

	ch->act_flags -= PLR_NOFOLLOW;

	if (ch->master != nullptr)
		stop_follower(ch);

	add_follower(ch, victim);
	return;
}

void add_follower(Character *ch, Character *master)
{
	if (ch->master != nullptr) {
		Logging::bug("Add_follower: non-null master.", 0);
		return;
	}

	ch->master        = master;
	ch->leader        = nullptr;

	if (can_see_char(master, ch))
		act("$n now follows you.", ch, nullptr, master, TO_VICT);

	act("You now follow $N.",  ch, nullptr, master, TO_CHAR);
	return;
}

void stop_follower(Character *ch)
{
	if (ch->master == nullptr) {
		Logging::bug("Stop_follower: null master.", 0);
		return;
	}

	if (affect::exists_on_char(ch, affect::type::charm_person)) {
		affect::remove_type_from_char(ch, affect::type::charm_person);
	}

	if (can_see_char(ch->master, ch) && ch->in_room != nullptr) {
		act("$n stops following you.",     ch, nullptr, ch->master, TO_VICT);
		act("You stop following $N.",      ch, nullptr, ch->master, TO_CHAR);
	}

	if (ch->master->pet == ch)
		ch->master->pet = nullptr;

	ch->master = nullptr;
	ch->leader = nullptr;
	return;
}

/* nukes charmed monsters and pets */
void nuke_pets(Character *ch)
{
	Character *pet;

	if ((pet = ch->pet) != nullptr) {
		stop_follower(pet);

		if (pet->in_room != nullptr)
			act("$N slowly fades away.", ch, nullptr, pet, TO_NOTVICT);

		extract_char(pet, true);
	}

	ch->pet = nullptr;
	return;
}

void die_follower(Character *ch)
{
	if (ch->master != nullptr) {
		if (ch->master->pet == ch)
			ch->master->pet = nullptr;

		if (ch->master->pcdata != nullptr) {
			if (ch->master->pcdata->skeleton == ch)
				ch->master->pcdata->skeleton = nullptr;

			if (ch->master->pcdata->zombie == ch)
				ch->master->pcdata->zombie = nullptr;

			if (ch->master->pcdata->wraith == ch)
				ch->master->pcdata->wraith = nullptr;

			if (ch->master->pcdata->gargoyle == ch)
				ch->master->pcdata->gargoyle = nullptr;
		}

		stop_follower(ch);
	}

	ch->leader = nullptr;

	for (auto fch : Game::world().char_list) {
		if (fch->master == ch)
			stop_follower(fch);

		if (fch->leader == ch)
			fch->leader = fch;
	}

	return;
}

void do_order(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Character *victim;
	Character *och;
	Character *och_next;
	bool found;
	bool fAll;
	bool remote_familiar = false;

	String arg, arg2;
	argument = one_argument(argument, arg);
	one_argument(argument, arg2);

	if (arg2 == "delete"
	    || arg2.has_prefix("mp")) {
		stc("That will NOT be done.\n", ch);
		return;
	}

	if (arg.empty() || argument.empty()) {
		stc("Order whom to do what?\n", ch);
		return;
	}

	if (affect::exists_on_char(ch, affect::type::charm_person)) {
		stc("You feel like taking, not giving, orders.\n", ch);
		return;
	}

	if (arg == "all") {
		fAll   = true;
		victim = nullptr;
	}
	else {
		fAll   = false;

		/* check for target in this room */
		if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr) {
			/* Check for familiar for remote orders
			   -- Outsider
			*/
			victim = get_char_world(ch, arg, VIS_CHAR);

			/* the victim must be your pet and your familiar */
			if ((!victim) || (victim != ch->pet) || (!ch->pcdata->familiar)) {
				stc("They aren't here.\n", ch);
				return;
			}
			else {
				stc("You remotely order your familiar...\n", ch);
				remote_familiar = true;
			}
		}

		/* Stay/Follow code by Lotus */
		if (arg2 == "stay" && victim->is_npc()) {
			stc("You order your pet to stay.\n", ch);
			victim->act_flags += ACT_STAY;
			return;
		}

		if (arg2 == "follow" && victim->is_npc()) {
			stc("You order your pet to follow you.\n", ch);
			victim->act_flags -= ACT_STAY;
			return;
		}

		if (arg2 == "wimpy" && victim->is_npc()) {
			stc("You order your pet to flee if injured.\n", ch);
			victim->act_flags += ACT_WIMPY;
			return;
		}

		if (arg2 == "courage" && victim->is_npc()) {
			stc("You order your pet fight to the finish.\n", ch);
			victim->act_flags -= ACT_WIMPY;
			return;
		}

		if (victim == ch) {
			stc("Aye aye, right away!\n", ch);
			return;
		}

		if (!affect::exists_on_char(victim, affect::type::charm_person) || victim->master != ch ||  IS_IMMORTAL(victim)) {
			stc("Do it yourself!\n", ch);
			return;
		}
	}

	found = false;

	for (och = ch->in_room->people; och != nullptr; och = och_next) {
		och_next = och->next_in_room;

		if (affect::exists_on_char(och, affect::type::charm_person) &&
		    och->master == ch &&
		    (fAll || och == victim)) {
			if (! found) {
				stc("You give the order.\n", ch);
				found = true;
			}

			Format::sprintf(buf, "$n orders you to '%s{x'.", argument);
			act(buf, ch, nullptr, och, TO_VICT);
			interpret(och, argument);
		}
	}

	if ((! found) && (remote_familiar)) {
		Format::sprintf(buf, "$n orders you to '%s{x'.", argument);
		act(buf, ch, nullptr, ch->pet, TO_VICT);
		interpret(ch->pet, argument);
		found = true;
	}

	if (found)
		WAIT_STATE(ch, PULSE_VIOLENCE);
	else
		stc("You have no followers here.\n", ch);
}

/*
This function basically acts as an alias to do_order(),
but is used just for pets. If you have a pet, you can
use a command like this:
pet <your order here>
rather than
order pet <your order here>

Okay, so I just need something to do...
-- Outsider
*/
void do_pet(Character *ch, String argument)
{
	String buffer;
	int index;
	char letter;

	if (! ch->pet) {
		stc("But you do not have a pet.\n", ch);
		return;
	}

	if (strlen(argument) > (MIL - 16)) {
		stc("Please use smaller words.\n", ch);
		return;
	}

	/* Copy first part of pet's name into buffer. */
	index = 0;
	letter = ch->pet->name[index];

	while ((letter != ' ') && (letter) && (index < 100)) {
		buffer[index] = letter;
		index++;
		letter = ch->pet->name[index];
	}

	/* Place order after name. */
	buffer += " ";
	buffer += argument;
	do_order(ch, buffer);
	return;
}

void do_group(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Character *victim;

	String arg;
	one_argument(argument, arg);

	if (arg.empty()) {
		Character *leader;
		leader = (ch->leader != nullptr) ? ch->leader : ch;
		Format::sprintf(buf, "%s's group:\n", PERS(leader, ch, VIS_PLR));
		set_color(ch, PURPLE, BOLD);
		stc(buf, ch);
		set_color(ch, WHITE, NOBOLD);

		for (auto gch : Game::world().char_list) {
			if (is_same_group(gch, ch)) {
				String guild =
					gch->is_npc() ? "Mob" :
					gch->guild == Guild::none ? "---" :
					guild_table[gch->guild].who_name;

				Format::sprintf(buf,
				        "[%2d %s] %-16s %4d/%4d hp %4d/%4d mana %4d/%4d st %6ld tnl\n",
				        gch->level,
				        guild,
				        PERS(gch, ch, VIS_PLR),
				        gch->hit,   GET_MAX_HIT(gch),
				        gch->mana,  GET_MAX_MANA(gch),
				        gch->stam,  GET_MAX_STAM(gch),
				        ((gch->level >= LEVEL_HERO) || gch->is_npc()) ? 0 : (gch->level + 1) *
				        exp_per_level(gch, gch->pcdata->points) - gch->exp);
				set_color(ch, PURPLE, BOLD);
				stc(buf, ch);
				set_color(ch, WHITE, NOBOLD);
			}
		}

		return;
	}

	if ((victim = get_char_here(ch, arg, VIS_CHAR)) == nullptr) {
		stc("They aren't here.\n", ch);
		return;
	}

	if (victim == ch) {
		stc("Feeling lonely?\n", ch);
		return;
	}

	if (ch->master != nullptr || (ch->leader != nullptr && ch->leader != ch)) {
		stc("But you are following someone else!\n", ch);
		return;
	}

	if (victim->master != ch && ch != victim) {
		act("$N isn't following you.", ch, nullptr, victim, TO_CHAR);
		return;
	}

	if (affect::exists_on_char(victim, affect::type::charm_person)) {
		stc("You can't remove charmed mobs from your group.\n", ch);
		return;
	}

	if (affect::exists_on_char(ch, affect::type::charm_person)) {
		act("You like your master too much to leave $m!", ch, nullptr, victim, TO_VICT);
		return;
	}

	if (is_same_group(victim, ch) && ch != victim) {
		victim->leader = nullptr;
		act("$n removes $N from $s group.",   ch, nullptr, victim, TO_NOTVICT);
		set_color(victim, PURPLE, BOLD);
		act("$n removes you from $s group.",  ch, nullptr, victim, TO_VICT);
		set_color(victim, WHITE, NOBOLD);
		set_color(ch, PURPLE, BOLD);
		act("You remove $N from your group.", ch, nullptr, victim, TO_CHAR);
		set_color(ch, WHITE, NOBOLD);
		return;
	}

	victim->leader = ch;
	act("$N joins $n's group.", ch, nullptr, victim, TO_NOTVICT);
	act("You join $n's group.", ch, nullptr, victim, TO_VICT);
	act("$N joins your group.", ch, nullptr, victim, TO_CHAR);
	return;
}

/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split(Character *ch, String argument)
{
	char buf[MAX_STRING_LENGTH];
	Character *gch;
	int members;
	int amount_gold = 0, amount_silver = 0;
	int share_gold, share_silver;
	int extra_gold, extra_silver;

	if (argument.empty()) {
		stc("Split how much?\n", ch);
		return;
	}

	String arg1, arg2;
	argument = one_argument(argument, arg1);
	one_argument(argument, arg2);

	amount_silver = atoi(arg1);

	if (!arg2.empty())
		amount_gold = atoi(arg2);

	if (amount_gold < 0 || amount_silver < 0) {
		stc("Your group wouldn't like that.\n", ch);
		return;
	}

	if (amount_gold == 0 && amount_silver == 0) {
		stc("You hand out zero coins, but no one notices.\n", ch);
		return;
	}

	if (ch->gold <  amount_gold || ch->silver < amount_silver) {
		stc("You don't have that much to split.\n", ch);
		return;
	}

	members = 0;

	for (gch = ch->in_room->people; gch != nullptr; gch = gch->next_in_room) {
		if (is_same_group(gch, ch) && !affect::exists_on_char(gch, affect::type::charm_person))
			members++;
	}

	if (members < 2) {
		stc("Just keep it all.\n", ch);
		return;
	}

	share_silver = amount_silver / members;
	extra_silver = amount_silver % members;
	share_gold   = amount_gold / members;
	extra_gold   = amount_gold % members;

	if (share_gold == 0 && share_silver == 0) {
		stc("Don't even bother, cheapskate.\n", ch);
		return;
	}

	ch->silver  -= amount_silver;
	ch->silver  += share_silver + extra_silver;
	ch->gold    -= amount_gold;
	ch->gold    += share_gold + extra_gold;

	if (share_silver > 0)
		ptc(ch, "You split %d silver coin%s. Your share is %d silver.\n",
		    amount_silver, amount_silver > 1 ? "s" : "", share_silver + extra_silver);

	if (share_gold > 0)
		ptc(ch, "You split %d gold coin%s. Your share is %d gold.\n",
		    amount_gold, amount_gold > 1 ? "s" : "", share_gold + extra_gold);

	if (share_gold == 0) {
		Format::sprintf(buf, "$n splits %d silver coin%s. Your share is %d silver.",
		        amount_silver, amount_silver > 1 ? "s" : "", share_silver);
	}
	else if (share_silver == 0) {
		Format::sprintf(buf, "$n splits %d gold coin%s. Your share is %d gold.",
		        amount_gold, amount_gold > 1 ? "s" : "", share_gold);
	}
	else {
		Format::sprintf(buf,
		        "$n splits %d silver and %d gold coin%s, giving you %d silver and %d gold.\n",
		        amount_silver, amount_gold, amount_gold > 1 ? "s" : "", share_silver, share_gold);
	}

	for (gch = ch->in_room->people; gch != nullptr; gch = gch->next_in_room) {
		if (gch != ch && is_same_group(gch, ch) && !gch->is_npc()) {
			act(buf, ch, nullptr, gch, TO_VICT);
			gch->gold += share_gold;
			gch->silver += share_silver;
		}
	}

	return;
}

/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group(Character *ach, Character *bch)
{
	if (ach == nullptr || bch == nullptr)
		return false;

	if (ach->leader != nullptr) ach = ach->leader;

	if (bch->leader != nullptr) bch = bch->leader;

	return ach == bch;
}

void align(Character *ch, int new_align, char *align_str)
{
	char buf[MAX_INPUT_LENGTH];

	if (!deduct_stamina(ch, skill::type::align))
		return;

	if (get_skill_level(ch, skill::type::align) < number_percent()) {
		stc("You fail to change your alignment.\n", ch);
		check_improve(ch, skill::type::align, false, 20);
	}
	else {
		ch->alignment = new_align;
		Format::sprintf(buf, "You are now %s.\n", align_str);
		stc(buf, ch);
		check_improve(ch, skill::type::align, true, 20);
	}

	WAIT_STATE(ch, 4 * PULSE_PER_SECOND);
} /* end align() */

void do_align(Character *ch, String argument)
{
	if (ch->is_npc()) {
		stc("Silly, mobiles can't change their alignment!\n", ch);
		return;
	}

	if (!CAN_USE_RSKILL(ch, skill::type::align)) {
		stc("Huh?\n", ch);
		return;
	}

	if (argument.empty()) {
		stc("Syntax: {RALIGN GOOD{x\n"
		    "or      {RALIGN NEUTRAL{x\n"
		    "or      {RALIGN EVIL{x\n", ch);
		return;
	}

	String arg;
	one_argument(argument, arg);

	if (arg.is_prefix_of("good"))
		align(ch, 750, "good");
	else if (arg.is_prefix_of("neutral"))
		align(ch, 0, "neutral");
	else if (arg.is_prefix_of("evil"))
		align(ch, -750, "evil");
	else
		do_align(ch, "");
} /* end do_align() */

/* equips a character */
void do_outfit(Character *ch, String argument)
{
	Object *obj;
	int vnum;

	if (ch->level > 5 || ch->is_npc()) {
		stc("Find it yourself.\n", ch);
		return;
	}

	if ((obj = get_eq_char(ch, WEAR_LIGHT)) == nullptr) {
		obj = create_object(Game::world().get_obj_prototype(OBJ_VNUM_SCHOOL_BANNER), 0);

		if (! obj) {
			Logging::bug("Error making light in do_outfit.", 0);
			return;
		}

		obj->cost = 0;
		obj_to_char(obj, ch);
		equip_char(ch, obj, WEAR_LIGHT);
	}

	if ((obj = get_eq_char(ch, WEAR_BODY)) == nullptr) {
		obj = create_object(Game::world().get_obj_prototype(OBJ_VNUM_SCHOOL_VEST), 0);

		if (! obj) {
			Logging::bug("Error making vest in do_outfit.", 0);
			return;
		}

		obj->cost = 0;
		obj_to_char(obj, ch);
		equip_char(ch, obj, WEAR_BODY);
	}

	/* do the weapon thing */
	if ((obj = get_eq_char(ch, WEAR_WIELD)) == nullptr) {
		skill::type sn = skill::type::dagger; // default
		vnum = OBJ_VNUM_SCHOOL_SWORD; /* just in case! */

		for (const auto& entry : weapon_table) {
			if (get_skill_level(ch, sn) <
			    get_skill_level(ch, entry.skill)) {
				sn = entry.skill;
				vnum = entry.vnum;
			}
		}

		obj = create_object(Game::world().get_obj_prototype(vnum), 0);

		if (! obj) {
			Logging::bug("Error creating weapon object in do_outfit.", 0);
			return;
		}

		obj_to_char(obj, ch);
		equip_char(ch, obj, WEAR_WIELD);
	}

	if (((obj = get_eq_char(ch, WEAR_WIELD)) == nullptr
	     ||   !affect::exists_on_obj(obj, affect::type::weapon_two_hands))
	    && (obj = get_eq_char(ch, WEAR_SHIELD)) == nullptr) {
		obj = create_object(Game::world().get_obj_prototype(OBJ_VNUM_SCHOOL_SHIELD), 0);

		if (! obj) {
			Logging::bug("Error creating shield in do_outfit.", 0);
			return;
		}

		obj->cost = 0;
		obj_to_char(obj, ch);
		equip_char(ch, obj, WEAR_SHIELD);
	}

	ptc(ch, "You have been equipped by %s.\n",
	    !ch->pcdata->deity.empty() ? ch->pcdata->deity : "Alisa");
}

