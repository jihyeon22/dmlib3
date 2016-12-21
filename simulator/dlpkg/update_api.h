#ifndef __DL_UPDATE_API_H__
#define __DL_UPDATE_API_H__

struct FTP_SERVER {
	char addr[256];
	int  port;
	char id[16];
	char pass[16];
	char file[255];
	char update_ver[64];
	char update_md5[64];
};
typedef struct FTP_SERVER FTP_SERVER;

#define UPDATE_SUCCESS					0
#define UPDATE_FAILURE					-1

int  ftpsvr_download(FTP_SERVER *svr_info);
int extract_gz(char *source, char *target);
int del_temp_dir();
int ftp_server_check(FTP_SERVER *svr, int timeout);
int download_size_get(char *download_file);

#endif
