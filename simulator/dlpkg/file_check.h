#ifndef __DL_LIB_CHECK_H__
#define __DL_LIB_CHECK_H__

typedef struct fileInfo fileInfo_t;
struct fileInfo
{
	char *localPath;
	int size;
};

typedef struct packageInfo packageInfo_t;
struct packageInfo
{
	char *remotePath;
	fileInfo_t localFiles[30];
};

int check_package(packageInfo_t pack_info);

#endif
