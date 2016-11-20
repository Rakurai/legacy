#include "merc.h"
#include "deps/cJSON/cJSON.h"
#include "db.h"
#include <float.h>

#define SCOPE_AREA 0
#define SCOPE_WORLD 1
#define MOBSTAT_FILE "../utils/mobs/mob_stats.json"
void load_mob_attrs() {
	cJSON *root = NULL;
	FILE *fp;

	if ((fp = fopen(MOBSTAT_FILE, "rb")) != NULL) {
		int length;
		char *buffer;

		fseek (fp, 0, SEEK_END);
		length = ftell (fp);
		fseek (fp, 0, SEEK_SET);
		buffer = malloc (length);

		fread (buffer, 1, length, fp);
		fclose (fp);

		root = cJSON_Parse(buffer);
		free(buffer);
	}

	if (root == NULL) {
		bug("load_mob_attrs: unable to read mobstat file", 0);
		return;
	}

	struct mobstat_keyword_t {
		int type;
		char *keyword;
	};

	const struct mobstat_keyword_t mobstat_keywords[] = {
		{ MOB_ATTR_HP, "hp" },
		{ MOB_ATTR_MANA, "mana" },
		{ MOB_ATTR_AC, "ac" },
		{ MOB_ATTR_HITROLL, "hitroll" },
		{ MOB_ATTR_DAMROLL, "damroll" }
	};

	for (AREA_DATA *ap = area_first; ap; ap = ap->next) {
		cJSON *area_obj = cJSON_GetObjectItem(root, ap->file_name);

		if (area_obj == NULL)
			continue;

		cJSON *inout = cJSON_GetObjectItem(area_obj, "inside");

		for (int i = 0; i < 5; i++) {
			cJSON *coeffs = cJSON_GetObjectItem(inout, mobstat_keywords[i].keyword);
			ap->inside_m2[mobstat_keywords[i].type] = coeffs->child->valuedouble;
			ap->inside_m1[mobstat_keywords[i].type] = coeffs->child->next->valuedouble;
			ap->inside_m0[mobstat_keywords[i].type] = coeffs->child->next->next->valuedouble;
		}

		inout = cJSON_GetObjectItem(area_obj, "outside");

		for (int i = 0; i < 5; i++) {
			cJSON *coeffs = cJSON_GetObjectItem(inout, mobstat_keywords[i].keyword);
			ap->outside_m2[mobstat_keywords[i].type] = coeffs->child->valuedouble;
			ap->outside_m1[mobstat_keywords[i].type] = coeffs->child->next->valuedouble;
			ap->outside_m0[mobstat_keywords[i].type] = coeffs->child->next->next->valuedouble;
		}
	}
}

float get_attr_avg(int level, int scope, int attr_type, AREA_DATA *area) {
	float m2 = scope == SCOPE_AREA ? area->inside_m2[attr_type] : area->outside_m2[attr_type];
	float m1 = scope == SCOPE_AREA ? area->inside_m1[attr_type] : area->outside_m1[attr_type];
	float m0 = scope == SCOPE_AREA ? area->inside_m0[attr_type] : area->outside_m0[attr_type];

	return m2*level*level + m1*level + m0;
}

float get_attr_ratio(float attr, int level, int scope, int attr_type, AREA_DATA *area) {
	float avg = get_attr_avg(level, scope, attr_type, area);

	if (avg == 0)
		return FLT_MAX;

	return attr/avg;
}

char *print_mob_multipliers(CHAR_DATA *mob) {
	static char buf[MSL];
	char tbuf[MSL];

	float mob_hp = ATTR_BASE(mob, APPLY_HIT);
	float mob_mana = ATTR_BASE(mob, APPLY_MANA);
	float mob_ac = 0.0;
	for (int i = 0; i < 4; i++)
		mob_ac += mob->armor_base[i];
	mob_ac /= 4;
	float mob_hitroll = ATTR_BASE(mob, APPLY_HITROLL);
	float mob_damroll = ATTR_BASE(mob, APPLY_DAMROLL);

	sprintf(buf, "comp to area : hp: %f  ma: %f  ac: %f  hr: %f  dr: %f\n",
		get_attr_ratio(mob_hp, mob->level, SCOPE_AREA, MOB_ATTR_HP, mob->pIndexData->area),
		get_attr_ratio(mob_mana, mob->level, SCOPE_AREA, MOB_ATTR_MANA, mob->pIndexData->area),
		get_attr_ratio(mob_ac, mob->level, SCOPE_AREA, MOB_ATTR_AC, mob->pIndexData->area),
		get_attr_ratio(mob_hitroll, mob->level, SCOPE_AREA, MOB_ATTR_HITROLL, mob->pIndexData->area),
		get_attr_ratio(mob_damroll, mob->level, SCOPE_AREA, MOB_ATTR_DAMROLL, mob->pIndexData->area)
	);

	sprintf(tbuf, "comp to world: hp: %f  ma: %f  ac: %f  hr: %f  dr: %f\n",
		get_attr_ratio(mob_hp, mob->level, SCOPE_WORLD, MOB_ATTR_HP, mob->pIndexData->area),
		get_attr_ratio(mob_mana, mob->level, SCOPE_WORLD, MOB_ATTR_MANA, mob->pIndexData->area),
		get_attr_ratio(mob_ac, mob->level, SCOPE_WORLD, MOB_ATTR_AC, mob->pIndexData->area),
		get_attr_ratio(mob_hitroll, mob->level, SCOPE_WORLD, MOB_ATTR_HITROLL, mob->pIndexData->area),
		get_attr_ratio(mob_damroll, mob->level, SCOPE_WORLD, MOB_ATTR_DAMROLL, mob->pIndexData->area)
	);

	strcat(buf, tbuf);
	return buf;
}
