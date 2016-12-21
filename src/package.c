#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include <dm/internal.h>
#include <dm/package.h>
#include <dm/safe.h>

#define LINE_SIZE	256
static char *_package = NULL;
char* dm_load_package_file(const char *file)
{
	char line[LINE_SIZE];
	char *str = line;
	char *ptr;
	FILE *fp;
	char *temp_bp = NULL;

	if (!file) {
		printf("[dmlib] %s : invalid parameter [file is null]\n", __func__);
		return NULL;
	}

	if (_package != NULL) return _package;

	_package = calloc(1, LINE_SIZE);

	if (safeaccess(file, F_OK) < 0) {
		printf("[dmlib] %s : %s access failture\n", __func__, file);
		return NULL;
	}

	fp = safefopen(file, "r");
	if (!fp) {
		printf("[dmlib] %s : %s open error\n", __func__, file);
		return NULL;
	}

	/* 1 LINE : PACKAGE TYPE (format PACKAGE: xxx.xxx) */
	fgets(line, LINE_SIZE, fp);		
	fprintf(stderr, "LINE 1: %s\n", line);
	ptr = strtok_r(str, ":", &temp_bp);		// PACKAGE
	fprintf(stderr, "%s\n", ptr);
	if (strncmp(ptr, "UP_VER", strlen("UP_VER")) != 0) return NULL;
	ptr = strtok_r(NULL, "\n", &temp_bp);		// device
	fprintf(stderr, "UP_VER: %s\n", ptr);
	
	fclose(fp);
	
	snprintf(_package, LINE_SIZE, "%s", ptr + 1);

	return _package;
}
