#ifndef __DL_FTP_LIST_H__
#define __DL_FTP_LIST_H__

typedef struct ftp_server_info ftp_server_info_t;
struct ftp_server_info
{
	char *addr;
	int  port;
	char *id;
	char *pass;
};

extern ftp_server_info_t ftp_table[];

#endif
