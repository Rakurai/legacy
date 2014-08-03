#include <stdlib.h>
#include "merc.h"
#include "include/mysql.h"
#include "deps/cJSON/cJSON.h"

int load_config(char *filename) {
	FILE *cf;
	long fsize;
	char *str;
	cJSON *json, *obj;

	if ((cf = fopen(filename, "rt")) == NULL) {
		bug("Could not open config file for reading.", 0);
		return -1;
	}

	// get size of file
	if (fseek(cf, 0, SEEK_END) != 0 || (fsize = ftell(cf)) < 0) {
		bug("Error in computing size of config file.", 0);
		fclose(cf);
		return -1;
	}

	rewind(cf);

	// alloc mem
	if ((str = malloc(fsize + 1)) == NULL) {
		bug("Could not allocate memory for configuration parsing.", 0);
		fclose(cf);
		return -1;
	}

	// read the file
	if (fread(str, fsize, 1, cf) != fsize) {
		bug("Error in reading config file.", 0);
		fclose(cf);
		return -1;
	}

	str[fsize] = 0; // terminate
	fclose(cf);

	json = cJSON_Parse(str);
	free(str); // done with it

	// parse json string
	if (json == NULL) {
		bugf("Config file error before: [%s]", cJSON_GetErrorPtr());
		return -1;
	}

	// got a valid json object, pull values from it
	if ((obj = cJSON_GetObjectItem(json, "db_host")) != NULL) {
		DB_HOST = str_dup(obj->valuestring);
	}
	if ((obj = cJSON_GetObjectItem(json, "db_name")) != NULL) {
		DB_NAME = str_dup(obj->valuestring);
	}
	if ((obj = cJSON_GetObjectItem(json, "db_user")) != NULL) {
		DB_USER = str_dup(obj->valuestring);
	}
	if ((obj = cJSON_GetObjectItem(json, "db_pass")) != NULL) {
		DB_PASS = str_dup(obj->valuestring);
	}

	// free the structure
	cJSON_Delete(json);
	return 0;
}
