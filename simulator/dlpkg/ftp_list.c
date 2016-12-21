#include <stdio.h>

#include "ftp_list.h"

ftp_server_info_t ftp_table[] =
{
	{
		.addr = "virtual.mdstec.com",
		.port = 21,
		.id = "openm2m",
		.pass = "openm2m.open"
	},
	{
		.addr = "iot.mdstec.com",
		.port = 40021,
		.id = "openm2m",
		.pass = "openm2m.open"	
	},
	{
		.addr = NULL,
		.port = 0,
		.id = NULL,
		.pass = NULL		
	},
};
