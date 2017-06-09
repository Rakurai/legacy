#pragma once

/*
 * Utility macros.
 */
#define UMIN(a, b)              ((a) < (b) ? (a) : (b))
#define UMAX(a, b)              ((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)         ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)                ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)                ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)       ((flag) & (bit))
#define SET_BIT(var, bit)       ((var) |= (bit))
#define REMOVE_BIT(var, bit)    ((var) &= ~(bit))
#define chance(prc)		(((number_range(1,100)) <= (prc)))
#define rounddown(x, inc)	((inc > x) ? 0 : (x - (x % inc)))
#define roundup(x, inc)		((inc > x) ? inc : (x - ((x % inc) == 0 ? inc : (x % inc)) + inc))

#define CHARTYPE_TEST(chx)     (IS_NPC(chx) ? ENTITY_M : ENTITY_P)
#define CHARTYPE_MATCH(chx,ct) ((CHARTYPE_TEST(chx) & ct) != 0)


#define IS_QUESTSHOPKEEPER(mob) ((mob->pIndexData->pShop != nullptr) && (mob->pIndexData->pShop->buy_type[0] == ITEM_QUESTSHOP))

/*
 * Character macros.
 */
#define IS_NPC(ch)              ((ch)->pcdata == nullptr ? TRUE : FALSE)
#define IS_PLAYING(d)		(d && d->connected == CON_PLAYING && d->character)

// Character attribute accessors (see attribute.c for explanations)

#define ATTR_BASE(ch, where) ((ch)->attr_base[where]) // intentionally settable
#define GET_ATTR_MOD(ch, where)  ((ch)->apply_cache ? (ch)->apply_cache[where] : 0) // intentionally not settable
#define GET_ATTR(ch, where) (ATTR_BASE(ch, where) + GET_ATTR_MOD(ch, where)) // intentionally not settable

#define GET_ATTR_STR(ch) (URANGE(3, GET_ATTR(ch, APPLY_STR), get_max_stat(ch, STAT_STR)))
#define GET_ATTR_INT(ch) (URANGE(3, GET_ATTR(ch, APPLY_INT), get_max_stat(ch, STAT_INT)))
#define GET_ATTR_WIS(ch) (URANGE(3, GET_ATTR(ch, APPLY_WIS), get_max_stat(ch, STAT_WIS)))
#define GET_ATTR_DEX(ch) (URANGE(3, GET_ATTR(ch, APPLY_DEX), get_max_stat(ch, STAT_DEX)))
#define GET_ATTR_CON(ch) (URANGE(3, GET_ATTR(ch, APPLY_CON), get_max_stat(ch, STAT_CON)))
#define GET_ATTR_CHR(ch) (URANGE(3, GET_ATTR(ch, APPLY_CHR), get_max_stat(ch, STAT_CHR)))
#define GET_ATTR_SEX(ch) (GET_ATTR((ch), APPLY_SEX) % 3) // gives range of 0-2
#define GET_ATTR_AGE(ch) (get_age(ch))
#define GET_ATTR_AC(ch)  (GET_ATTR(ch, APPLY_AC)                              \
						+ ( IS_AWAKE(ch)                                      \
						? dex_app[GET_ATTR_DEX(ch)].defensive : 0 )           \
						- (( !IS_NPC(ch) && ch->pcdata->remort_count > 0 )    \
						? (((ch->pcdata->remort_count * ch->level) / 50)) : 0 )) /* should give -1 per 10 levels,
																				   -1 per 5 remorts -- Montrey */
#define GET_ATTR_HITROLL(ch) \
				(GET_ATTR((ch), APPLY_HITROLL) + str_app[GET_ATTR_STR((ch))].tohit)
#define GET_ATTR_DAMROLL(ch) \
				(GET_ATTR((ch), APPLY_DAMROLL) + str_app[GET_ATTR_STR((ch))].todam)
#define GET_ATTR_SAVES(ch) (GET_ATTR((ch), APPLY_SAVES))
#define GET_MAX_HIT(ch)    (URANGE(1, GET_ATTR((ch), APPLY_HIT), 30000))
#define GET_MAX_MANA(ch)   (URANGE(1, GET_ATTR((ch), APPLY_MANA), 30000))
#define GET_MAX_STAM(ch)   (URANGE(1, GET_ATTR((ch), APPLY_STAM), 30000))
#define GET_DEFENSE_MOD(ch, dam_type) (dam_type == DAM_NONE ? 0 :             \
						  (ch)->defense_mod ? (ch)->defense_mod[dam_type] : 0)
#define GET_AC(ch, type) ((ch)->armor_base[type] + GET_ATTR_AC((ch)))


/* permission checking stuff */
#define IS_HERO(ch)         (!IS_NPC(ch) && ch->level >= LEVEL_HERO)
#define IS_REMORT(ch)		(!IS_NPC(ch) && ch->pcdata->remort_count > 0)
#define IS_HEROIC(ch)		(IS_HERO(ch) || IS_REMORT(ch))
#define IS_IMM_GROUP(bit)	(IS_SET(bit, GROUP_GEN|GROUP_QUEST|GROUP_BUILD|GROUP_CODE|GROUP_SECURE))
#define HAS_CGROUP(ch, bit)	(!IS_NPC(ch) && (((ch->pcdata->cgroup & bit) == bit)))
#define RANK(flags)			(IS_IMM_GROUP(flags) ?												\
							(IS_SET(flags, GROUP_LEADER) ?	RANK_IMP	:					\
							(IS_SET(flags, GROUP_DEPUTY) ?	RANK_HEAD 	: RANK_IMM))	:	\
							(IS_SET(flags, GROUP_PLAYER) ?	RANK_MORTAL : RANK_MOBILE))
#define	GET_RANK(ch)		(IS_NPC(ch) ? RANK_MOBILE : RANK(ch->pcdata->cgroup))
#define IS_IMMORTAL(ch)		(GET_RANK(ch) >= RANK_IMM)
#define IS_IMP(ch)			(GET_RANK(ch) == RANK_IMP)
#define IS_HEAD(ch)			(GET_RANK(ch) >= RANK_HEAD)
#define OUTRANKS(ch, victim)	(GET_RANK(ch) > GET_RANK(victim))

#define SET_CGROUP(ch, bit)	(SET_BIT(ch->pcdata->cgroup, bit))
#define REM_CGROUP(ch, bit)	(REMOVE_BIT(ch->pcdata->cgroup, bit))

/* other shortcuts */
#define IS_GOOD(ch)             (ch->alignment >= 350)
#define IS_EVIL(ch)             (ch->alignment <= -350)
#define IS_NEUTRAL(ch)          (!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)            (ch->position > POS_SLEEPING)
#define GET_ROOM_FLAGS(room)    ((room)->room_flags | (room)->room_flag_cache)
#define IS_OUTSIDE(ch)          (!IS_SET(GET_ROOM_FLAGS((ch)->in_room), ROOM_INDOORS))

#define WAIT_STATE(ch, npulse)  (!IS_IMMORTAL(ch) ? \
	(ch->wait = UMAX(ch->wait, npulse)) : (ch->wait = 0))
#define DAZE_STATE(ch, npulse)  (!IS_IMMORTAL(ch) ? \
	(ch->daze = UMAX(ch->daze, npulse)) : (ch->daze = 0))
#define gold_weight(amount)  ((amount) * 2 / 5)
#define silver_weight(amount) ((amount)/ 10)
#define IS_QUESTOR(ch)     (IS_SET((ch)->act_flags, PLR_QUESTOR))
#define IS_SQUESTOR(ch)    (!IS_NPC(ch) && IS_SET((ch)->pcdata->plr, PLR_SQUESTOR))
#define IS_KILLER(ch)		(IS_SET((ch)->act_flags, PLR_KILLER))
#define IS_THIEF(ch)		(IS_SET((ch)->act_flags, PLR_THIEF))
#define CAN_FLY(ch)         (affect_exists_on_char((ch), gsn_fly))
#define IS_FLYING(ch)       ((ch)->position >= POS_FLYING)

/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)     (IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)  (IS_SET((obj)->extra_flags | (obj)->extra_flag_cache, (stat)))
#define IS_WEAPON_STAT(obj,stat)(IS_SET((obj)->value[4] | (obj)->weapon_flag_cache,(stat)))
#define WEIGHT_MULT(obj)        ((obj)->item_type == ITEM_CONTAINER ? \
		(obj)->value[4] : 100)


/*
 * Identd stuff
 */

#define CH(d)         ((d)->original ? (d)->original : (d)->character )


/*
 * Description macros.
 */
#define PERS(ch, looker, vis)   (  (vis == VIS_ALL					\
				|| (vis == VIS_CHAR && can_see_char(looker, ch))		\
				|| (vis == VIS_PLR && can_see_who(looker, ch))) ?	\
				IS_NPC(ch) ? ch->short_descr : ch->name : "someone")
