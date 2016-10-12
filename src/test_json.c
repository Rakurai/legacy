#include <stdio.h>
#include <stdlib.h>
#include "deps/cJSON/cJSON.h"

int main(int argc, char **argv) {
	FILE * f = fopen(argv[1], "rb");
	int length;
	char *buffer;

	if (f) {
		fseek (f, 0, SEEK_END);
		length = ftell (f);
		fseek (f, 0, SEEK_SET);
		buffer = malloc (length);

		fread (buffer, 1, length, f);
		fclose (f);
	}

	cJSON * root = cJSON_Parse(buffer);

	cJSON * o = cJSON_GetObjectItem(root, "Alias");
	printf("%s\n", o->string);

	if (o == NULL)
		printf("o is null\n");

	cJSON *item;

	for (item = o->child; item != NULL; item = item->next)
		printf("%s %s\n", item->child->valuestring, 
item->child->next->valuestring);
//	printf("%s\n", root->child->next->next->next->next->next->child->next->string);
}
