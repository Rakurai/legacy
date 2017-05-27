#pragma once

#include "merc.h"
#include "String.hpp"
#include "../deps/cJSON/cJSON.h"

namespace JSON {

inline void addStringToObject(cJSON *obj, const String& key, const String& str) {
	cJSON_AddStringToObject(obj, key.c_str(), str.c_str());
}

inline void get_boolean(cJSON *obj, bool *target, const String& key) {
	cJSON *val = cJSON_GetObjectItem(obj, key.c_str());

	if (val != NULL)
		*target = (val->valueint != 0);
}

inline void get_short(cJSON *obj, sh_int *target, const String& key) {
	cJSON *val = cJSON_GetObjectItem(obj, key.c_str());

	if (val != NULL)
		*target = val->valueint;
}

inline void get_int(cJSON *obj, int *target, const String& key) {
	cJSON *val = cJSON_GetObjectItem(obj, key.c_str());

	if (val != NULL)
		*target = val->valueint;
}

inline void get_long(cJSON *obj, long *target, const String& key) {
	cJSON *val = cJSON_GetObjectItem(obj, key.c_str());

	if (val != NULL)
		*target = val->valueint;
}

inline void get_flags(cJSON *obj, long *target, const String& key) {
	cJSON *val = cJSON_GetObjectItem(obj, key.c_str());

	if (val != NULL)
		*target = string_to_flags(val->valuestring);
}

inline void get_string(cJSON *obj, char **target, const String& key) {
	cJSON *val = cJSON_GetObjectItem(obj, key.c_str());

	if (val != NULL) {
		if (*target != NULL) {
			free_string(*target);
		}
		*target = str_dup(val->valuestring);
	}
}

} // namespace JSON
