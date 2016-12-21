#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>

#include "update_api.h"
#include "util.h"

#define UPDATE_DIR "/system/UPDATE_LIB"
#define FTPSCRIPT "/system/UPDATE_LIB/ftp_script.sh"

static 	char cmd_temp[255] = {0};

int  md5read(char *file_name, char* md5data){

    FILE    *   fp;
    char md5info[256];
    char *temp_bp = NULL;
	char *p_md5 = NULL;

    //printf("\n md5read file: %s\n", file_name);

    if (access(file_name,F_OK)) {
        printf("\n md5read no file %s\n",file_name);

        return UPDATE_FAILURE;
    }

    if ( (fp = fopen(file_name, "rt")) == NULL) {
        fprintf(stderr, "cannot open md5 file: %s\n", file_name);
        return UPDATE_FAILURE;
    }

    if(fgets(md5info, 256, fp) != NULL){      
        p_md5 = strtok_r(md5info, " ", &temp_bp);
		strcpy(md5data, p_md5);
		
    } else {
        printf("\n download file check error : \n %s \n", md5info); 
        return UPDATE_FAILURE;
    }

    fclose(fp);
	
    return UPDATE_SUCCESS;
}


int md5check(FTP_SERVER * svr)
{
    char md5info[256] = {0};
    char md5_make[256] = {0};
    char md5_file[256] = {0};

    sprintf(md5_file, "%s.md5.chk", svr->update_ver);
    
    printf("md5 make %s running... \n", svr->file);   
    sprintf(md5_make, "md5sum %s > %s", svr->update_ver, md5_file );
    system(md5_make);    

    if(md5read(md5_file, md5info) == UPDATE_FAILURE)
	{
		return UPDATE_FAILURE;
	}

    if(!strncmp(svr->update_md5, md5info, strlen(md5info))){
		printf("[%s] same [%s] \n", svr->update_md5, md5info);
    }else{
		printf("[%s] different \n", svr->update_md5);
 		printf("[%s] different \n", md5info);
		return UPDATE_FAILURE;
    }

    return UPDATE_SUCCESS;
}

int download_size_get(char *download_file)
{
    FILE *fp;
    int ret;
    int  status;
    
    status = access(download_file,F_OK); 
    if (status) {
        printf("\nfile not found %s\n", download_file);
		return UPDATE_FAILURE;
    } 

    fp = fopen(download_file, "rb");

    fseek(fp, 0L, SEEK_END);
    ret = ftell(fp);
    printf("download_size_get %d \n", ret);

    fclose(fp);
    return ret;
}

int download_run_script(FTP_SERVER *svr)
{
    FILE    *sh ;
    int rst=0;
    char md5_file[64] = {0};

    char string[255] = {0};
    char file_path[255] = {0};
    char file_name[255] = {0};
    char *pos;

    pos = strrchr(svr->file,'/');
    if(pos == NULL){
       strcpy( file_name, svr->file);
    } else {
       strncpy(file_path, svr->file, strlen(svr->file) - strlen(strrchr(svr->file,'/'))+1);   
       strcpy( string, svr->file);
       pos = strrchr( string, '/' );
       strcpy( file_name, pos+1);
    }

    printf("create %s\n", FTPSCRIPT);

    sh = fopen(FTPSCRIPT, "w");

    fprintf(sh,
    "#!/bin/sh\n"
    "PORT='%d'\n"
    "HOST='%s'\n"
    "USER='%s'\n"
    "PASSWD='%s'\n"
    "FILE='%s*'\n"
    "\n"
    "cmdftp -p $PORT -n $HOST -t 630 <<END_SCRIPT\n"
    "$USER\n"
    "$PASSWD\n"
    "d $FILE %s\n"
    "quit\n"
    "END_SCRIPT\n"
    "exit 0\n"
    "\n"
    "\n",svr->port, svr->addr, svr->id, svr->pass, svr->file, UPDATE_DIR);

    fclose(sh);

	memset(cmd_temp,0,sizeof(cmd_temp));
	snprintf(cmd_temp, sizeof(cmd_temp)-1, "chmod 775 %s", FTPSCRIPT);
    rst = system(cmd_temp);
    printf("change mode %s %d\n", FTPSCRIPT, rst);   

    rst = system(FTPSCRIPT);
    printf("run %s %d ", FTPSCRIPT, rst);   

    sprintf(svr->update_ver, "%s/%s", UPDATE_DIR, file_name);
    if(download_size_get(svr->update_ver) == UPDATE_FAILURE)
	{
		return UPDATE_FAILURE;
	}
	
    md5_file[0] = '\0';
    sprintf(md5_file, "%s.md5", svr->update_ver);
    printf("update_ver md5 read %s \n", md5_file);
    md5read(md5_file, svr->update_md5);
    printf("svr->update_md5 run %s \n", svr->update_md5); 

    return UPDATE_SUCCESS;
}

int  ftpsvr_download(FTP_SERVER *svr_info)
{
    int retval_i = UPDATE_FAILURE;
    DIR* dp = NULL;

    printf("##############################\n");  	
    printf("#######   ftpsvr_download start   ######\n");  
    printf("##############################\n"); 

    if((dp = opendir(UPDATE_DIR)) == NULL) { 
        printf("mkdir %s\n", UPDATE_DIR);
		if (mkdir(UPDATE_DIR, 0755) != 0) {
			printf("cannot make directory %s\n", UPDATE_DIR);
			closedir(dp);
			return UPDATE_FAILURE;
		}
    }
	closedir(dp);
	
#if DEBUG
    printf("ftp Server addr = %s ", svr_info->addr);
    printf("ftp Server port = %d ", svr_info->port);
    printf("ftp Server id   = %s ", svr_info->id);
    printf("ftp Server pass = %s ", svr_info->pass);
    printf("ftp Server file = %s ", svr_info->file);
#endif

    int chk;
    for(chk = 1; chk < 3; chk++){
		
	    retval_i =  ftp_server_check(svr_info, 20);
		
	    if(retval_i == UPDATE_SUCCESS){
			printf("ftp Server connect success \n");
	       	retval_i = 0;
			break;
	    } else {
			if(chk == 1){
				printf("ftp Server connect timeout retry \n");
			}else{
				printf("ftp Server connect failed \n");
				return UPDATE_FAILURE;
			}		
	    }    
    }

    if(download_run_script(svr_info) == UPDATE_FAILURE)
	{
		return UPDATE_FAILURE;
	}

    retval_i = md5check(svr_info);

	if(retval_i == UPDATE_FAILURE)
	{
		printf("error> download fail!\n");
		return UPDATE_FAILURE;
	}

	printf("download success!\n");
    return UPDATE_SUCCESS;
}

int extract_gz(char *source, char *target)
{
	char cmd[255] = {0};
	
	sprintf(cmd, "tar xvf %s -C %s", source, target);
	system(cmd);
	
	return 0;
}

int del_temp_dir()
{
	del_dir(UPDATE_DIR);
	
	return 0;
}

int ftp_server_check(FTP_SERVER *svr, int timeout) {
    u_short port;                /* user specified port number */
    char *addr;                  /* will be a pointer to the address */
    struct sockaddr_in address;  /* the libc network address data structure */
    short int sock = -1;         /* file descriptor for the network socket */
    fd_set fdset;
    struct timeval tv;
    int ret = 0;

    port = svr->port; 
    addr = svr->addr;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(addr); /* assign the address */
    address.sin_port = htons(port);            /* translate int2port num */

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    printf("ftp server connect checking... ");
	
    connect(sock, (struct sockaddr *)&address, sizeof(address));

    FD_ZERO(&fdset);
    FD_SET(sock, &fdset);
    if(timeout <= 0){	
	    tv.tv_sec = 60;             /* 60 second timeout */
    }else{
	    tv.tv_sec = timeout;
    }
    tv.tv_usec = 0;

    printf("timeout %d second setting \n",(int)tv.tv_sec);
	
    if (select(sock + 1, NULL, &fdset, NULL, &tv) == 1)
    {
        int so_error;
        socklen_t len = sizeof so_error;

        getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);

        if (so_error == 0) {
            printf("%s:%d is open \n", addr, port);
            ret = UPDATE_SUCCESS;
        }
    }else{
            printf("%s:%d isn't open\n", addr, port);
            ret = UPDATE_FAILURE;
    }

    printf("sokect close  [result = %d] ( 0 = fail, 1= success) \n",ret);

    close(sock);
    return ret;
}
