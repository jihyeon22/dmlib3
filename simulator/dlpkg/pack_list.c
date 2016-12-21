#include <stdio.h>
#include "file_check.h"

packageInfo_t package_list[] =
{
	{
		.remotePath = "/home/LIBS_x86_64/libjansson.4.7.0_x86_64.tar.gz",
		
		.localFiles[0].localPath = "/usr/lib/libjansson.so",
		.localFiles[0].size = 41800,
		.localFiles[1].localPath = "/usr/lib/libjansson.so.4",
		.localFiles[1].size = 41800,
		.localFiles[2].localPath = "/usr/lib/libjansson.so.4.7.0",
		.localFiles[2].size = 41800,	
		.localFiles[3].localPath = NULL,
		.localFiles[3].size = 0
	},
	{
		.remotePath = "/home/LIBS_x86_64/libcurl.4.3.0_x86_64.tar.gz",
		
		.localFiles[0].localPath = "/usr/lib/libcurl.so",
		.localFiles[0].size = 307720,
		.localFiles[1].localPath = "/usr/lib/libcurl.so.4",
		.localFiles[1].size = 307720,
		.localFiles[2].localPath = "/usr/lib/libcurl.so.4.3.0",
		.localFiles[2].size = 307720,
		.localFiles[3].localPath = NULL,
		.localFiles[3].size = 0
	},
	{
		.remotePath = NULL,
		
		.localFiles[0].localPath = NULL,
		.localFiles[0].size = 0
	}	
};
