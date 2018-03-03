#include "MobilePrototype.hh"

#include "file.hh"
#include "Area.hh"
#include "lookup.hh"
#include "Logging.hh"
#include "merc.hh"
#include "MobProg.hh"

MobilePrototype::
MobilePrototype(Area& area, const Vnum& vnum, FILE *fp) :
	area(area),
	vnum(vnum)
{
	player_name          = fread_string(fp);
	short_descr          = fread_string(fp);
	long_descr           = fread_string(fp).capitalize();
	description          = fread_string(fp).capitalize();
	race                 = race_lookup(fread_string(fp));
	act_flags            = fread_flag(fp) + race_table[race].act;

	// affect flags no longer includes flags already on the race affect bitvector
	affect_flags         = fread_flag(fp) - race_table[race].aff;

	alignment            = fread_number(fp);
	group_flags          = fread_flag(fp);
	level                = fread_number(fp);
	hitroll              = fread_number(fp);
	/* read hit dice */
	hit[DICE_NUMBER]     = fread_number(fp);
	/* 'd'          */     fread_letter(fp);
	hit[DICE_TYPE]       = fread_number(fp);
	/* '+'          */     fread_letter(fp);
	hit[DICE_BONUS]      = fread_number(fp);
	/* read mana dice */
	mana[DICE_NUMBER]    = fread_number(fp);
	fread_letter(fp);
	mana[DICE_TYPE]      = fread_number(fp);
	fread_letter(fp);
	mana[DICE_BONUS]     = fread_number(fp);
	/* read damage dice */
	damage[DICE_NUMBER]  = fread_number(fp);
	fread_letter(fp);
	damage[DICE_TYPE]    = fread_number(fp);
	fread_letter(fp);
	damage[DICE_BONUS]   = fread_number(fp);
	dam_type         = attack_lookup(fread_word(fp));
	/* read armor class */
	ac[AC_PIERCE]        = fread_number(fp) * 10;
	ac[AC_BASH]          = fread_number(fp) * 10;
	ac[AC_SLASH]         = fread_number(fp) * 10;
	ac[AC_EXOTIC]        = fread_number(fp) * 10;
	/* read flags and add in data from the race table */
	off_flags            = fread_flag(fp) + race_table[race].off;

	// defense flags no longer includes flags already on the race bitvector
	absorb_flags         = 0; /* fix when we change the area versions */
	imm_flags            = fread_flag(fp) - race_table[race].imm;
	res_flags            = fread_flag(fp) - race_table[race].res;
	vuln_flags           = fread_flag(fp) - race_table[race].vuln;

	// fix old style ACT_IS_NPC (A) flag, changed to ACT_NOSUMMON (A)
	act_flags -= Flags::A;

	// fix old style IMM_SUMMON (A) flag, changed to ACT_NOSUMMON (A)
	if (imm_flags.has(Flags::A))
		act_flags += ACT_NOSUMMON;
	imm_flags -= Flags::A;
	res_flags -= Flags::A;
	vuln_flags -= Flags::A;

	/* vital statistics */
	start_pos        = position_lookup(fread_word(fp));

	if (start_pos == POS_STANDING && affect_flags.has(AFF_FLYING))
		start_pos = POS_FLYING;

	default_pos      = position_lookup(fread_word(fp));

	if (default_pos == POS_STANDING && affect_flags.has(AFF_FLYING))
		default_pos = POS_FLYING;

	sex              = sex_lookup(fread_word(fp));

	if (sex < 0) {
		Logging::bugf("Load_mobiles: bad sex for vnum %d.", vnum);
		exit(1);
	}

	wealth               = fread_number(fp);
	form_flags                 = fread_flag(fp) + race_table[race].form;
	parts_flags                = fread_flag(fp) + race_table[race].parts;
	/* size */
	size                 = size_lookup(fread_word(fp));

	if (size < 0) {
		Logging::bugf("Load_mobiles: bad size for vnum %d.", vnum);
		exit(1);
	}

	material             = fread_word(fp);

	for (; ;) {
		char letter = fread_letter(fp);

		if (letter == 'F') {
			String word  = fread_word(fp);
			Flags vector = fread_flag(fp);

			     if (word.is_prefix_of("act"))  act_flags -= vector;
			else if (word.is_prefix_of("aff"))  affect_flags -= vector;
			else if (word.is_prefix_of("off"))  off_flags -= vector;
			else if (word.is_prefix_of("drn"))  absorb_flags -= vector;
			else if (word.is_prefix_of("imm"))  imm_flags -= vector;
			else if (word.is_prefix_of("res"))  res_flags -= vector;
			else if (word.is_prefix_of("vul"))  vuln_flags -= vector;
			else if (word.is_prefix_of("for"))  form_flags -= vector;
			else if (word.is_prefix_of("par"))  parts_flags -= vector;
			else {
				Logging::file_bug(fp, "Flag remove: flag not found.", 0);
				exit(1);
			}
		}
		else {
			ungetc(letter, fp);
			break;
		}
	}

	read_mobprogs(fp);
}

MobilePrototype::
~MobilePrototype() {
	for (auto mobprog : mobprogs)
		delete mobprog;
}

void MobilePrototype::
read_mobprogs(FILE *fp) {
	while (true) {
		char letter = fread_letter(fp);

		if (letter != '>') {
			if (letter != '|')
				ungetc(letter, fp);

			break;
		}

		MobProg *mprg = new MobProg(fp);

		if (mprg->type == ERROR_PROG) {
			Logging::file_bug(fp, "Load_mobiles: vnum %d invalid MOBPROG type.", vnum);
			exit(1);
		}

		mobprogs.push_back(mprg);
		progtype_flags += mprg->type;
	}
}
