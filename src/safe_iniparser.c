#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dm/safe.h>
#include <dm/safe_iniparser.h>

dictionary* safe_iniparser_load(const char* file)
{
	if (safeaccess(file, F_OK)) {
		return NULL;
	}

	dictionary* dict = NULL;
	dict = iniparser_load(file);

	return dict;
}

char* safe_iniparser_getstring(dictionary* d, const char* key, char* def)
{
	char* value = NULL;
	
	value = iniparser_getstring(d, key, def);
	
	if (value && strlen(value) == 0) value = def;

	return value;
}

int safe_iniparser_getint(dictionary* d, const char* key, int def)
{
	int value = 0;
	char *string = NULL;

	string = iniparser_getstring(d, key, NULL);
	if (string && strlen(string) == 0) string = NULL;

	if (!string)
		value = def;
	else
		value = iniparser_getint(d, key, def);

	return value;
}

int safe_iniparser_getboolean(dictionary* d, const char* key, int def)
{
	int value = 0;
	char *string = NULL;

	string = iniparser_getstring(d, key, NULL);
	if (string && strlen(string) == 0) string = NULL;

	if (!string)
		value = def;
	else
		value = iniparser_getboolean(d, key, def);

	return value;
}

int safe_iniparser_store(dictionary* d, const char* file)
{
	int res = 0;
	FILE *f;
	char renamefile[256] = {0};

	if (!d || !file) return -1;

	if (!safeaccess(file, F_OK)) {
		snprintf(renamefile, sizeof(renamefile) - 4, "%s.org", file);
		res = saferename(file, renamefile);
		if (res)
			return -1;
	}

	f = safefopen(file, "w");
	if (f == NULL)
		return -1;

	iniparser_dump_ini(d, f);
	fclose(f);

	return 0;
}
