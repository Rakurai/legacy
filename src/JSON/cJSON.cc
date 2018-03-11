
#include "../deps/cJSON/cJSON.h"
#include "JSON/cJSON.hh"

#include "Logging.hh"

cJSON * JSON::read_file(const String& filename) {
	FILE *cf;
	long fsize;
	char *str;

	if ((cf = fopen(filename.c_str(), "rb")) == nullptr) {
		Logging::bugf("JSON::read_file: Could not open file '%s' for reading.", filename);
		return nullptr;
	}

	// get size of file
	if (fseek(cf, 0, SEEK_END) != 0 || (fsize = ftell(cf)) < 0) {
		Logging::bugf("JSON::read_file: Error in computing size of file '%s'.", filename);
		fclose(cf);
		return nullptr;
	}

	rewind(cf);

	// alloc mem
	if ((str = (char *)malloc(fsize + 1)) == nullptr) {
		Logging::bugf("JSON::read_file: Could not allocate memory to parse file '%s'.", filename);
		fclose(cf);
		return nullptr;
	}

	// read the file
	if (fread(str, 1, fsize, cf) != (unsigned long)fsize) {
		Logging::bugf("JSON::read_file: Error in reading file '%s'.", filename);
		fclose(cf);
		free(str);
		return nullptr;
	}

	str[fsize] = 0; // terminate
	fclose(cf);

	cJSON *json = cJSON_Parse(str);
	free(str); // done with it

	// parse json string
	if (json == nullptr) {
		Logging::bugf("JSON::read_file: File '%s', error before: [%s]", filename, cJSON_GetErrorPtr());
		return nullptr;
	}

	return json;
}
