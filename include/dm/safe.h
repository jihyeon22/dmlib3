#ifndef _DM_SAFE_H_
#define _DM_SAFE_H_

#include <unistd.h>
#include <dirent.h>

/* Memory */
void safefree(void *ptr);

/* File */
int safeaccess(const char* file, int flag);
DIR* safeopendir(const char* name);
int safemkdir(const char* pathname, mode_t mode);
int saferename(const char* oldpath, const char* newpath);
FILE* safefopen(const char* path, const char* mode);
int safechmod(const char* path, mode_t mode);

#endif

