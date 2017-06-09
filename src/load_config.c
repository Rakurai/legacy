#include "merc.h"
#include "sql.h"
#include "cJSON.h"
#include "Format.hpp"

int load_config(const char *filename) {
	int items = 0;
	cJSON *json = JSON::read_file(filename);

	if (json == nullptr) {
		bug("Error reading config file.", 0);
		return -1;
	}
/*
	// got a valid json object, pull values from it
	if ((obj = cJSON_GetObjectItem(json, "db_host")) != nullptr) {
		DB_HOST = obj->valuestring;
		items++;
	}
	if ((obj = cJSON_GetObjectItem(json, "db_name")) != nullptr) {
		DB_NAME = obj->valuestring;
		items++;
	}
	if ((obj = cJSON_GetObjectItem(json, "db_user")) != nullptr) {
		DB_USER = obj->valuestring;
		items++;
	}
	if ((obj = cJSON_GetObjectItem(json, "db_pass")) != nullptr) {
		DB_PASS = obj->valuestring;
		items++;
	}
*/
	// free the structure
	cJSON_Delete(json);
	Format::printf("loaded %d items from config file\n", items);
	return 0;
}
