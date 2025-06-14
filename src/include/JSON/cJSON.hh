#pragma once

#include "String.hh"
#include "Flags.hh"
#include "../deps/cJSON/cJSON.h"

namespace JSON {

inline void addStringToObject(cJSON *obj, const String& key, const String& str) {
	cJSON_AddStringToObject(obj, key.c_str(), str.c_str());
}

inline void get_boolean(cJSON *obj, bool *target, const String& key) {
	cJSON *val = cJSON_GetObjectItem(obj, key.c_str());

	if (val != nullptr)
		*target = (val->valueint != 0);
}

inline void get_short(cJSON *obj, void *target, const String& key) {
	cJSON *val = cJSON_GetObjectItem(obj, key.c_str());

	if (val != nullptr)
		*(int *)target = val->valueint;
}

inline void get_int(cJSON *obj, int *target, const String& key) {
	cJSON *val = cJSON_GetObjectItem(obj, key.c_str());

	if (val != nullptr)
		*target = val->valueint;
}

inline void get_long(cJSON *obj, long *target, const String& key) {
	cJSON *val = cJSON_GetObjectItem(obj, key.c_str());

	if (val != nullptr)
		*target = val->valueint;
}

inline void get_flags(cJSON *obj, Flags *target, const String& key) {
	cJSON *val = cJSON_GetObjectItem(obj, key.c_str());

	if (val != nullptr)
		*target = Flags(val->valuestring);
}

inline void get_string(cJSON *obj, String *target, const String& key) {
	cJSON *val = cJSON_GetObjectItem(obj, key.c_str());

	if (val != nullptr) {
		target->assign(val->valuestring);
	}
}

cJSON *read_file(const String& filename);

} // namespace JSON
