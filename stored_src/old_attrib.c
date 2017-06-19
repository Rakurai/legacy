/* perform a full reset on a character, can be called any time, but is always
   used in load_char_obj */
void reset_char(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	AFFECT_DATA *af;
	int loc, mod, stat, i;

	/* initialize */
	if (!IS_NPC(ch)) {
		ch->sex                 = ch->pcdata->true_sex;
		GET_ATTR(ch, APPLY_HIT)             = ch->pcdata->perm_hit;
		GET_ATTR(ch, APPLY_MANA)            = ch->pcdata->perm_mana;
		GET_ATTR(ch, APPLY_STAM)            = ch->pcdata->perm_stam;
		ch->pcdata->pktimer     = 0;
		ch->hitroll             = 0;
		ch->damroll             = 0;

		for (i = 0; i < 4; i++) {
			ch->armor_a[i] = 100;
			ch->armor_m[i] = 0;
		}
	}
	else {  /* reset a mobile, used in fread_pet */
		int stambase = 5;
		ch->sex                 = URANGE(0, ch->pIndexData->sex, 2);    /* skip the randomizing */
		GET_ATTR(ch, APPLY_HIT)             =  dice(ch->pIndexData->hit[DICE_NUMBER],
		                                ch->pIndexData->hit[DICE_TYPE])
		                           + ch->pIndexData->hit[DICE_BONUS];
		GET_ATTR(ch, APPLY_MANA)            =  dice(ch->pIndexData->mana[DICE_NUMBER],
		                                ch->pIndexData->mana[DICE_TYPE])
		                           + ch->pIndexData->mana[DICE_BONUS];
		GET_ATTR(ch, APPLY_STAM) = 100;

		if (IS_SET(ch->act, ACT_MAGE))             stambase = 3;
		else if (IS_SET(ch->act, ACT_CLERIC))           stambase = 4;
		else if (IS_SET(ch->act, ACT_THIEF))            stambase = 7;
		else if (IS_SET(ch->act, ACT_WARRIOR))          stambase = 9;

		for (i = 0; i < ch->level; i++)
			GET_ATTR(ch, APPLY_STAM) += number_fuzzy(stambase);

		ch->hitroll             = ch->pIndexData->hitroll;
		ch->damroll             = ch->pIndexData->damage[DICE_BONUS];

		for (i = 0; i < 4; i++) {
			ch->armor_a[i] = ch->pIndexData->ac[i];
			ch->armor_m[i] = 0;
		}
	}

	ch->saving_throw        = 0;

	/* now start adding back the effects */
	for (loc = 0; loc < MAX_WEAR; loc++) {
		if ((obj = get_eq_char(ch, loc)) == NULL)
			continue;

		for (i = 0; i < 4; i++)
			ch->armor_a[i] -= apply_ac(obj, loc, i);

		for (af = obj->affected; af != NULL; af = af->next) {
			mod = af->modifier;

			switch (af->location) {
			case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;

			case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;

			case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;

			case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;

			case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;

			case APPLY_CHR:         ch->mod_stat[STAT_CHR]  += mod; break;

			case APPLY_SEX:         ch->sex                 += mod; break;

			case APPLY_MANA:        GET_ATTR(ch, APPLY_MANA)            += mod; break;

			case APPLY_HIT:         GET_ATTR(ch, APPLY_HIT)             += mod; break;

			case APPLY_STAM:        GET_ATTR(ch, APPLY_STAM)            += mod; break;

			case APPLY_AC:
				for (i = 0; i < 4; i ++)
					ch->armor_m[i] += mod;

				break;

			case APPLY_HITROLL:     ch->hitroll             += mod; break;

			case APPLY_DAMROLL:     ch->damroll             += mod; break;

			case APPLY_SAVES:       ch->saving_throw        += mod; break;

			case APPLY_SAVING_ROD:  ch->saving_throw        += mod; break;

			case APPLY_SAVING_PETRI: ch->saving_throw        += mod; break;

			case APPLY_SAVING_BREATH: ch->saving_throw       += mod; break;

			case APPLY_SAVING_SPELL: ch->saving_throw        += mod; break;
			}
		}
	}

	/* now add back spell effects */
	for (af = ch->affected; af != NULL; af = af->next) {
		mod = af->modifier;

		switch (af->location) {
		case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;

		case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;

		case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;

		case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;

		case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;

		case APPLY_CHR:         ch->mod_stat[STAT_CHR]  += mod; break;

		case APPLY_SEX:         ch->sex                 += mod; break;

		case APPLY_MANA:        GET_ATTR(ch, APPLY_MANA)            += mod; break;

		case APPLY_HIT:         GET_ATTR(ch, APPLY_HIT)             += mod; break;

		case APPLY_STAM:        GET_ATTR(ch, APPLY_STAM)            += mod; break;

		case APPLY_AC:
			for (i = 0; i < 4; i ++)
				ch->armor_m[i] += mod;

			break;

		case APPLY_HITROLL:     ch->hitroll             += mod; break;

		case APPLY_DAMROLL:     ch->damroll             += mod; break;

		case APPLY_SAVES:       ch->saving_throw        += mod; break;

		case APPLY_SAVING_ROD:  ch->saving_throw        += mod; break;

		case APPLY_SAVING_PETRI: ch->saving_throw        += mod; break;

		case APPLY_SAVING_BREATH: ch->saving_throw       += mod; break;

		case APPLY_SAVING_SPELL: ch->saving_throw        += mod; break;
		}
	}

	REMOVE_BIT(GET_FLAGS(ch, TO_IMMUNE), IMM_SHADOW);
}
