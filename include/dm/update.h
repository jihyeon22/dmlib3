#ifndef _UPDATE_API_H
#define _UPDATE_API_H

#include <dm/dm.h>
struct update_info {
	char *addr;
	int port;
	char *id;
	char *pw;
	char *file;
};

dm_res dm_update_ftp_download(const char *addr, int port, const char *id, const char *pw, const char *file);
dm_res dm_update_cmd(int vers, const char *file, const char *path);
dm_res dm_update(void);

#endif
