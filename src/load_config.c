#include "merc.h"
#include "sql.h"
#include "cJSON.h"
#include "Format.hpp"

int load_config(const char *filename) {
	int items = 0;
	cJSON *json = JSON::read_file(filename);

	if (json == NULL) {
		bug("Error reading config file.", 0);
		return -1;
	}
/*
	// got a valid json object, pull values from it
	if ((obj = cJSON_GetObjectItem(json, "db_host")) != NULL) {
		DB_HOST = obj->valuestring;
		items++;
	}
	if ((obj = cJSON_GetObjectItem(json, "db_name")) != NULL) {
		DB_NAME = obj->valuestring;
		items++;
	}
	if ((obj = cJSON_GetObjectItem(json, "db_user")) != NULL) {
		DB_USER = obj->valuestring;
		items++;
	}
	if ((obj = cJSON_GetObjectItem(json, "db_pass")) != NULL) {
		DB_PASS = obj->valuestring;
		items++;
	}
*/
	// free the structure
	cJSON_Delete(json);
	Format::printf("loaded %d items from config file\n", items);
	return 0;
}
