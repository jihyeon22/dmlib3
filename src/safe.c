#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include <dm/safe.h>

#define RETRY_COUNT	10
#define SAFE_TIME	100000	// 100msec

void safefree(void *ptr)
{
	if (ptr) {
		free(ptr);
		ptr = NULL;
	}
}

/* 파일시스템상 안정화 시간 필요 */
int safeaccess(const char* file, int flag)
{
	int res = -1;
	int retry = RETRY_COUNT;
	if (file) {
		do {
			res = access(file, flag);
			if (!res)
				break;
			usleep(SAFE_TIME);
		} while (retry--);
	}

	return res;
}

DIR* safeopendir(const char* name)
{
	static DIR* dir = NULL;
	int retry = RETRY_COUNT;
	if (name) {
		do {
			dir = opendir(name);
			if (dir)
				break;
			usleep(SAFE_TIME);
		} while (retry--);
	}

	return dir;
}

int safemkdir(const char* pathname, mode_t mode)
{
	int res = -1;
	int retry = RETRY_COUNT;
	if (pathname) {
		do {
			res = mkdir(pathname, mode);
			if (!res)
				break;
			usleep(SAFE_TIME);
		} while (retry--);
	}

	return res;
}

int saferename(const char* oldpath, const char* newpath)
{
	int res = -1;
	int retry = RETRY_COUNT;

	if (oldpath && newpath) {
		do {
			res = rename(oldpath, newpath);
			if (!res)
				break;
			usleep(SAFE_TIME);
		} while (retry--);
	}

	return res;
}

FILE* safefopen(const char* path, const char* mode)
{
	static FILE* f = NULL;
	int retry = RETRY_COUNT;

	do {
		f = fopen(path, mode);
		if (f)
			break;
		usleep(SAFE_TIME);
	} while(retry--);

	return f;
}

int safechmod(const char* path, mode_t mode)
{
	int res = -1;
	int retry = RETRY_COUNT;

	do {
		res = chmod(path, mode);
		if (!res)
			break;
		usleep(SAFE_TIME);

	} while(retry--);

	return res;
}


