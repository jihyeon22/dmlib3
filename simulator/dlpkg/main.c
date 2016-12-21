#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>

#include "update_api.h"
#include "file_check.h"
#include "common.h"
#include "gpio.h"
#include "pack_list.h"
#include "update.h"
#include "at.h"

void norifier_end()
{
	system("/system/sbin/alive.notifier end");
}

void poweroff()
{
	while(1)
	{
		system("poweroff &");
		sleep(10);
	}
}

int main(int argc, char** argv)
{
	char target_file[255] = {0};
	int i = 0;

	int is_downloaded = 0;
	int is_normal = 1;
	int n_retry = 0;
	
	char phone[AT_LEN_PHONENUM_BUFF];

	// get argument 
	if (argc >= 2) 
	{
    	for (i = 0; i<argc ; i++)
    	{
    	    if( strcmp(argv[i],"-v") == 0 )
	    	{
	    		printf("dlpkg x86 simulator\n");
				
				return 0;
	    	}
			else if( strcmp(argv[i],"-h") == 0 )
	    	{
	    		printf("command\n");
				printf("-h : help\n");
				printf("-v : version\n");
				
				return 0;
	    	}
			else if( strcmp(argv[i],"-t") == 0 )
	    	{
				init_syslog();
				
				sleep(1);
				at_get_phonenum(phone, sizeof(phone));
				syslog(LOG_CRIT, "DLPKG : pkg test<%s>", phone);
				
				sleep(10);
				deinit_syslog();
				
				return 0;
	    	}
    	}
    }
	
	while(1)
	{
		if(n_retry++ >= RETRY_COUNT)
		{
			printf("retry count is max : pooweroff\n");
			
			init_syslog();
			
			sleep(1);
			at_get_phonenum(phone, sizeof(phone));
			syslog(LOG_CRIT, "DLPKG : pkg download error <%s>", phone);
			
			sleep(10);
			deinit_syslog();
			
			poweroff();
		}

		// Simulator code
		printf("DLPKG SIMULATOR> POWER CHECK PASS\n");
/* Orignal Code
		if(gpio_get_value(GPIO_SRC_NUM_POWER) != 0)
		{
			printf("power pin off : poweroff\n");
			poweroff();
		}
*/		
		is_downloaded = 0;
		for(i = 0; package_list[i].remotePath != NULL  ; i++)
		{
			if(check_package(package_list[i]) >= 0)
			{
				printf("package ok! [0]%s\n", package_list[i].localFiles[0].localPath);
				continue;
			}
			
			is_normal = 0;
			
			norifier_end();
			if(update_ftp_download(package_list[i].remotePath, target_file) == UPDATE_FAILURE)
			{
				continue;
			}
			
			extract_gz(target_file,"/");
			
			del_temp_dir();
			
			is_downloaded = 1;
		}
		
		if(is_normal == 1)
		{
			return 0;
		}

		is_normal = 1;
		for(i = 0; package_list[i].remotePath != NULL ; i++)
		{
			if(check_package(package_list[i]) < 0)
			{
				is_normal = 0;
				break;
			}

			printf("package ok! [0]%s\n", package_list[i].localFiles[0].localPath);
		}
		
		if(is_normal)
		{
			if(is_downloaded)
			{
				poweroff();
			}
			return 0;
		}
		
		sleep(RETRY_SECS);
	}
}
