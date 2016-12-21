#include <stdio.h>
#include <string.h>
#include "update_api.h"
#include "ftp_list.h"

int 
update_ftp_download(const char *file, char *target)
{
	int ret = 0;
	int i = 0;
	FTP_SERVER ftp_info;
	int is_normal = 1;

	memset(&ftp_info, 0, sizeof(FTP_SERVER));
	
	is_normal = 1;
	for(i=0; ftp_table[i].addr != NULL; i++)
	{
		strncpy(ftp_info.addr, ftp_table[i].addr, sizeof(ftp_info.addr)-1);
		ftp_info.port = ftp_table[i].port;
		strncpy(ftp_info.id, ftp_table[i].id, sizeof(ftp_info.id)-1);
		strncpy(ftp_info.pass, ftp_table[i].pass, sizeof(ftp_info.pass)-1);
		
		strncpy(ftp_info.file, file, sizeof(ftp_info.file)-1);
		
		ret = ftpsvr_download(&ftp_info);
		if(ret == UPDATE_SUCCESS)
		{
			break;
			
		}
		
		is_normal = 0;		
	}
	
	if(!is_normal)
	{
		printf("ERROR:FINAL DOWNLOAD fail!!!\n");
		return UPDATE_FAILURE;
	}
	
	strcpy(target, ftp_info.update_ver);
	
	return UPDATE_SUCCESS;
}
