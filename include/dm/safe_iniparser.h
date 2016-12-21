#ifndef _SAFE_INIPARSER_H_
#define _SAFE_INIPARSER_H_

#include <iniparser.h>

dictionary* safe_iniparser_load(const char* file);
char* safe_iniparser_getstring(dictionary* d, const char* key, char* def);
int safe_iniparser_getint(dictionary* d, const char* key, int def);
int safe_iniparser_getboolean(dictionary* d, const char* key, int def);
int safe_iniparser_store(dictionary* d, const char* file);

#endif
