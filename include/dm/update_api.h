#ifndef __LINUX_UPDATE_H
#define __LINUX_UPDATE_H

struct FTP_SERVER {
	char addr[256];
	int  port;
	char id[16];
	char pass[16];
	char file[256];
	char update_ver[256];
	char release_ver[256];
	char update_md5[256];
};
typedef struct FTP_SERVER FTP_SERVER;


enum update_vers {
    INITIAL_CMD = 0,
	UPDATE_VCMD = 1,
	RELEASE_VCMD = 2,
	UPDATE_CMD = 3,	
	DELETE_CMD = 4
};
typedef enum update_vers update_vers;

struct UPDATE_VERS{
       char update_file[256];
	char update_path[256];
	enum update_vers version;
};
typedef struct UPDATE_VERS UPDATE_VERS;


enum update_status {
	success = 1,
	failure = 2,
};
typedef enum update_status update_status;

int  ftpsvr_download(FTP_SERVER *svr_info);
int  version_update(UPDATE_VERS *versions);
int download_run_script(FTP_SERVER *svr);

int ini_version_update(FTP_SERVER *new_vers);
int ini_field_get(char *title, char *field);
int ini_field_set(char *title, char *field);
char*  ini_read(UPDATE_VERS vers);
void create_svrftp_ini_file(void);
int parse_ini_file(char * ini_name);

int md5check(FTP_SERVER * svr);

int check_file(char *file);
int tree_check(const char *root_dir_s, char *old_dir_s, char *new_dir_s);
#endif
