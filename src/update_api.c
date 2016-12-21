#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <assert.h>

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h> //for mkdir
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>

#include <dictionary.h>
#include <iniparser.h>

#include <dm/board_system.h>
#include <dm/update_api.h>
#include <dm/safe.h>

// ----------------------------------------
//  LOGD(LOG_TARGET,  Target
// ----------------------------------------
#define LOG_TARGET NULL

#define LOGD(svc,msg...)  { printf(msg); printf("\n"); }
#define LOGI(svc,msg...)  { printf(msg); printf("\n"); }
#define LOGE(svc,msg...)  { printf(msg); printf("\n"); }

/* =====================================================================
		Update Call Function

char*  ftpsvr_ini_get(UPDATE_VERS vers)
int  ftpsvr_download(FTP_SERVER *svr_info)
int  version_update(char *file_name)
===================================================================== */
char*  ftpsvr_ini_get(UPDATE_VERS vers)
{
     int  status ;
     static char retval_c[64];

    LOGI(LOG_TARGET, "##############################");  	
    LOGI(LOG_TARGET, "#######   version check   ######");  
    LOGI(LOG_TARGET, "##############################");  
	
     memset(retval_c, 0, sizeof(retval_c));

     status = safeaccess(SVRFTP,F_OK); 
     if (status) {
         LOGI(LOG_TARGET, "\n %s> svrftp.ini don't exist, so it creates new.", __func__);
         create_svrftp_ini_file();
         status = parse_ini_file(SVRFTP);
         //return retval_c;
     } else {
         LOGI(LOG_TARGET, "find %s vers[%d]", SVRFTP, vers.version);
     }

	 if( vers.version == UPDATE_VCMD ) {
		 strcpy(retval_c, ini_read(vers));
		 LOGI(LOG_TARGET, "ftpsvr_ini_get update_ver = %s ",retval_c );
	 } else if ( vers.version == RELEASE_VCMD ) {
		 strcpy(retval_c, ini_read(vers));
		 LOGI(LOG_TARGET, "ftpsvr_ini_get release_ver = %s ",retval_c );
	 } else {
		 strcpy(retval_c,"Can't looking for version infomatin");
	 }

     return retval_c;
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

    LOGI(LOG_TARGET, "ftp server connect checking... ");
	
    connect(sock, (struct sockaddr *)&address, sizeof(address));

    FD_ZERO(&fdset);
    FD_SET(sock, &fdset);
    if(timeout <= 0){	
	    tv.tv_sec = 60;             /* 60 second timeout */
    }else{
	    tv.tv_sec = timeout;
    }
    tv.tv_usec = 0;

    LOGI(LOG_TARGET, "timeout %d second setting ",(int)tv.tv_sec);
	
    if (select(sock + 1, NULL, &fdset, NULL, &tv) == 1)
    {
        int so_error;
        socklen_t len = sizeof so_error;

        getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);

        if (so_error == 0) {
            LOGI(LOG_TARGET, "%s:%d is open ", addr, port);
            ret = 1;
        }
    }else{
            LOGE(LOG_TARGET, "%s:%d isn't open\n", addr, port);
            ret = 2;
    }

    LOGI(LOG_TARGET, "sokect close  [result = %d] ( 0 = fail, 1= success) \n",ret);

    close(sock);
    return ret;
}


int  ftpsvr_download(FTP_SERVER *svr_info)
{
    int retval_i = 0;
    DIR* dp = NULL;
    int  status ;

    LOGI(LOG_TARGET, "############################## ");  	
    LOGI(LOG_TARGET, "#######   ftpsvr_download start   ######  ");  
    LOGI(LOG_TARGET, "############################## "); 

    if((dp = opendir(UPDATE_DIR)) == NULL) { 
        LOGD(LOG_TARGET, "mkdir %s", UPDATE_DIR);
		if (mkdir(UPDATE_DIR, 0755) != 0) {
			LOGE(LOG_TARGET, "cannot make directory %s", UPDATE_DIR);
			assert(0);
		}
    }

	status = safeaccess(SVRFTP,F_OK); 
	if (status) {
		LOGE(LOG_TARGET, "%s> svrftp.ini don't exist, so it creates new.", __func__);
		create_svrftp_ini_file();
		status = parse_ini_file(SVRFTP);
		//return retval_c;
	} else {
		LOGI(LOG_TARGET, "finded %s", SVRFTP);
	}

#if DEBUG
    LOGI(LOG_TARGET, "ftp Server addr = %s ", svr_info->addr);
    LOGI(LOG_TARGET, "ftp Server port = %d ", svr_info->port);
    LOGI(LOG_TARGET, "ftp Server id   = %s ", svr_info->id);
    LOGI(LOG_TARGET, "ftp Server pass = %s ", svr_info->pass);
    LOGI(LOG_TARGET, "ftp Server file = %s ", svr_info->file);
#endif

    int chk;
    for(chk = 1; chk < 3; chk++){
		
	    retval_i =  ftp_server_check(svr_info, 20);
		
	    if(retval_i == 1){
			LOGI(LOG_TARGET, "ftp Server connect susses \n");
	       	retval_i = success;	 				
			break;
	    } else {
			if(chk == 1){
				LOGE(LOG_TARGET, "ftp Server connect timeout retry \n");
			}else{
				LOGE(LOG_TARGET, "ftp Server connect failed \n");
				retval_i = failure;						
				return retval_i;			
			}		
	    }    
    }

    download_run_script(svr_info);
	
    retval_i = md5check(svr_info);

    if(retval_i == 1){
        if(ini_version_update(svr_info) < 0)
			retval_i = failure;
		else 
			retval_i = success;
    } else {
		LOGE(LOG_TARGET, "ftp Server download failed");
	retval_i = failure;
    }

    closedir(dp);

    return retval_i;
}



int  version_update(UPDATE_VERS *versions)
{
     int retval_i = 0;
     char vers[64];
     char cmd[64];
     int  status;

	LOGI(LOG_TARGET, "############################## ");  	
	LOGI(LOG_TARGET, "#######   version_update   ######  ");  
	LOGI(LOG_TARGET, "############################## ");  	
   
     status = safeaccess(SVRFTP,F_OK); 
     if (status) {
         LOGE(LOG_TARGET, "\n not find svrftp.ini \n");
         create_svrftp_ini_file();
         status = parse_ini_file(SVRFTP);
         retval_i = failure;
         return retval_i;
     }

	    LOGI(LOG_TARGET, "\nversion [%d] ", versions->version );          
	    LOGI(LOG_TARGET, "\nupdate file [%s] ", versions->update_file );   
	    LOGI(LOG_TARGET, "\nupdate path [%s] ", versions->update_path );   		

		// For new dm
		snprintf(cmd, sizeof(cmd)-1, "rm -r %s", DIR_NEW_PKG);
		system(cmd);
		
   	   if( versions->version == UPDATE_VCMD ||  versions->version == RELEASE_VCMD) {  
			printf("\n update install version[%d] \n", versions->version );
			if( versions->version == UPDATE_VCMD ) {  
				if(ini_field_get("Update_Server:UPDATE_VER", vers) < 0) {
					retval_i = failure;
			        return retval_i;
				}
			} else if( versions->version == RELEASE_VCMD ){
				if(ini_field_get("Release_Server:UPDATE_VER", vers) < 0) {
					retval_i = failure;
			        return retval_i;
				}
			}
			sprintf(cmd, "tar xvf %s -C /", vers);   
			system(cmd);
			LOGI(LOG_TARGET, " VERSION UPDATE [%s]  ",vers);			
          }else{
		    sprintf(versions->update_path, "%s/%s", UPDATE_DIR, versions->update_file);
		    LOGI(LOG_TARGET, "\n update install file [%s] \n", versions->update_path );
		    if (safeaccess(versions->update_path,F_OK)) {
				retval_i = failure;
				LOGE(LOG_TARGET, "UPDATE_FAILURE : No search update file [%s], update result [%d] \n",versions->update_file,retval_i);    		
				return retval_i;
		    }else{
				sprintf(cmd, "tar xvf %s -C /", versions->update_path);   
				system(cmd);
				LOGI(LOG_TARGET, " FILE update [%s] \n",versions->update_path);			
	          }
		
      	   }
		   
		   if(safeaccess(DIR_NEW_PKG, F_OK) >= 0)
		   {
			   char old_dir_s[255] = {0};
			   char new_dir_s[255] = {0};
			   double new_dir_d = 0;
			   double newer_dir_d = 0;
			   char *ptr;
			   
				tree_check(TREE_ROOT, old_dir_s, new_dir_s);
				
				new_dir_d = strtod(new_dir_s ,&ptr);
				newer_dir_d = new_dir_d + 0.01;	
//				snprintf(newer_dir_s, sizeof(newer_dir_s)-1, "%f", newer_dir_d);
				
				snprintf(cmd, sizeof(cmd)-1, "mv -f %s %s/%05.2f", DIR_NEW_PKG, TREE_ROOT, newer_dir_d);
				system(cmd);
				
				retval_i = success;
		   }
		   else
		   {
			   retval_i = failure;
		   }

		   printf("version_update result [%d] \n", retval_i);    		

#if 0
    /* release_server file and info delete 
       download file delete .tar .md5 .md5.chk */
    ini_field_get("Release_Server:FILE", release_vers.file);
    LOGI(LOG_TARGET, "Deleate Release_Server:FILE* rm -r %s \n",release_vers.file );
    sprintf(cmd, "rm -r %s*", release_vers.update_ver);   
    system(cmd);
#endif 

    return retval_i;
}


/* =====================================================================
		Update Basic Function

char*  md5read(char *file_name)
int md5check(FTP_SERVER * svr)
int download_size_get(FTP_SERVER * svr)
int download_run_script(FTP_SERVER * svr)
===================================================================== */

char*  md5read(char *file_name){

    FILE* fp;
    char md5info[256];
    char * md5data;
    char *temp_bp = NULL;

    //printf("\n md5read file: %s\n", file_name);

    if (safeaccess(file_name,F_OK)) {
        LOGE(LOG_TARGET, "\n md5read no file %s\n",file_name);
#if REBOOT
	LOGE(LOG_TARGET, "\n reboot from md5read \n" );
	sleep(5);
	system("reboot");
#endif
        return "md5read no file";
    }
    
	
	fp = safefopen(file_name, "rt");
	if (fp == NULL) {
        fprintf(stderr, "cannot open md5 file: %s\n", file_name);

#if REBOOT
	LOGI(LOG_TARGET, "\n reboot from md5read \n" );
	sleep(5);
	system("reboot");
#endif
	md5data = "cannot open md5 file";
        return md5data;
    }

    if(fgets(md5info, 256, fp) != NULL){      
        md5data = strtok_r(md5info, " ", &temp_bp);   
    } else {
        LOGE(LOG_TARGET, "\n download file check error : \n %s \n", md5info); 

#if REBOOT
	LOGI(LOG_TARGET, "\n reboot from md5read \n" );
	sleep(5);
	system("reboot");
#endif 
	md5data = "download file check error";
        return md5data;
    }

    fclose(fp);    
    return md5data;
}


int md5check(FTP_SERVER * svr)
{
    char md5info[256];
    char md5_make[256];
    char md5_file[256];

    sprintf(md5_file, "%s.md5.chk", svr->update_ver);
    
    LOGI(LOG_TARGET, "md5 make %s running... \n", svr->file);   
    sprintf(md5_make, "md5sum %s > %s", svr->update_ver, md5_file );
    //printf("md5 cmd = %s  \n", md5_make); 
    system(md5_make);    

    strcpy(md5info, md5read(md5_file));
    //printf("md5info run %s \n", md5info); 
    //printf("svr->update_md5 %d %d \n", strlen(svr->update_md5), strlen(md5info)); 
    if(!strncmp(svr->update_md5, md5info, strlen(md5info))){
		LOGI(LOG_TARGET, "[%s] same [%s] \n", svr->update_md5, md5info);
    }else{
		LOGE(LOG_TARGET, "[%s] different \n", svr->update_md5);
 		LOGE(LOG_TARGET, "[%s] different \n", md5info);
#if REBOOT
		LOGE(LOG_TARGET, "\n reboot from exit \n" );
		sleep(5);
		system("reboot");
#endif
		return 2;
    }

    return 1;
}


int download_size_get(char *download_file)
{
    FILE *fp;
    int ret;
    int  status;
    
    status = safeaccess(download_file,F_OK); 
    if (status) {
        LOGE(LOG_TARGET, "\ncmdftp: connection failed \n");
#if REBOOT
        LOGE(LOG_TARGET, "\n reboot from ftpcmd \n" );
        sleep(5);
		system("reboot");
#endif
		exit(1);
    } 

    LOGI(LOG_TARGET, "\ncmdftp: bye \n");

    fp = safefopen(download_file, "rb");

    fseek(fp, 0L, SEEK_END);
    ret = ftell(fp);
    LOGI(LOG_TARGET, "download_size_get %d \n", ret);

    fclose(fp);
    return ret;
}


int download_run_script(FTP_SERVER *svr)
{
    FILE    *   sh ;
    int rst=0;
    char md5_file[256];

    char string[100];
    char file_path[256] = {0};
    char file_name[256] = {0};
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

    LOGI(LOG_TARGET, "create ftp_script.sh ");

    sh = safefopen(FTPSCRIPT, "w");

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

    rst = safechmod(FTPSCRIPT, 755);
    LOGI(LOG_TARGET, "ftp_script.sh change mode 755 [%d] ", rst);   

    rst = system(FTPSCRIPT);
    LOGI(LOG_TARGET, "ftp_script.sh run %d ", rst);   

    sprintf(svr->update_ver, "%s/%s", UPDATE_DIR, file_name);
    download_size_get(svr->update_ver);

    md5_file[0] = '\0';
    sprintf(md5_file, "%s.md5", svr->update_ver);
    LOGI(LOG_TARGET, "update_ver md5 read %s \n", md5_file);   
    strcpy(svr->update_md5, md5read(md5_file));  
    LOGI(LOG_TARGET, "svr->update_md5 run %s \n", svr->update_md5); 

    return 0;
}

/* =====================================================================
		iniparser Call Function

void create_svrftp_ini_file(void);
int  parse_ini_file(char * ini_name);
int  parse_ini_file_get(char * ini_name);
int  parse_ini_file_set(char * ini_name);
===================================================================== */

int ini_version_update(FTP_SERVER *new_vers) 
{

    FTP_SERVER update_vers;
    FTP_SERVER release_vers;

	char temp_port[10] = {0};

    memset(&update_vers, 0, sizeof(FTP_SERVER));
    memset(&release_vers, 0, sizeof(FTP_SERVER));

#if 0
    char cmd[64];

    /* release_server file and info delete 
       download file delete .tar .md5 .md5.chk */
    ini_field_get("Release_Server:FILE", release_vers.file);
    LOGI(LOG_TARGET, "Deleate Release_Server:FILE* rm -r %s \n",release_vers.file );
    sprintf(cmd, "rm -r %s*", release_vers.update_ver);   
    system(cmd);
#endif 
	system(STEP_DEBUG_MDOE_1);
	sprintf(temp_port, "%d", update_vers.port);
	system(STEP_DEBUG_MDOE_2);
    /* update_server read */
    if(ini_field_get("Update_Server:ADDR", update_vers.addr) < 0)
		return -1;
    if(ini_field_get("Update_Server:PORT", temp_port) < 0)
		return -2;
    if(ini_field_get("Update_Server:ID",   update_vers.id) < 0)
		return -3;
    if(ini_field_get("Update_Server:PASS", update_vers.pass) < 0)
		return -4;
    if(ini_field_get("Update_Server:FILE", update_vers.file) < 0)
		return -5;
    if(ini_field_get("Update_Server:UPDATE_VER", update_vers.update_ver) < 0)
		return -6;
    if(ini_field_get("Update_Server:RELEASE_VER", update_vers.release_ver) < 0)
		return -7;
    if(ini_field_get("Update_Server:UPDATE_MD5", update_vers.update_md5) < 0)
		return -8;
system(STEP_DEBUG_MDOE_3);
#if DEBUG
    LOGI(LOG_TARGET, "Update_Server: = %s ", update_vers.addr);
    LOGI(LOG_TARGET, "Update_Server: = %d ", update_vers.port);
    LOGI(LOG_TARGET, "Update_Server: = %s ", update_vers.id);
    LOGI(LOG_TARGET, "Update_Server: = %s ", update_vers.pass);
    LOGI(LOG_TARGET, "Update_Server: = %s ", update_vers.file);
#endif

    /* update_server -> release_server Set Server attributes */
    release_vers = update_vers;
    if(ini_field_set("Release_Server:ADDR", release_vers.addr) < 0)
		return -9;
    if(ini_field_set("Release_Server:PORT", temp_port) <  0)
		return -10;
    if(ini_field_set("Release_Server:ID",   release_vers.id) < 0)
		return -11;
    if(ini_field_set("Release_Server:PASS", release_vers.pass) < 0)
		return -12;
    if(ini_field_set("Release_Server:FILE", release_vers.file) < 0)
		return -13;
    if(ini_field_set("Release_Server:UPDATE_VER", release_vers.update_ver) < 0 )
		return -14;
    if(ini_field_set("Release_Server:RELEASE_VER", release_vers.release_ver) < 0)
		return -15;
    if(ini_field_set("Release_Server:UPDATE_MD5", release_vers.update_md5) < 0)
		return -16;
system(STEP_DEBUG_MDOE_4);
    /* New update_server -> update_server Set Server attributes */
    update_vers = *new_vers;

    /* update_server write */
    if(ini_field_set("Update_Server:ADDR", update_vers.addr) < 0)
		return -17;
    if(ini_field_set("Update_Server:PORT", temp_port) < 0)
		return -18;
    if(ini_field_set("Update_Server:ID",   update_vers.id) < 0)
		return -19;
    if(ini_field_set("Update_Server:PASS", update_vers.pass) <  0)
		return -20;
    if(ini_field_set("Update_Server:FILE", update_vers.file) < 0)
		return -21;
    if(ini_field_set("Update_Server:UPDATE_VER", update_vers.update_ver) < 0)
		return -22;
    if(ini_field_set("Update_Server:RELEASE_VER", update_vers.release_ver) < 0)
		return -23;
    if(ini_field_set("Update_Server:UPDATE_MD5", update_vers.update_md5) < 0 )
		return -24;
system(STEP_DEBUG_MDOE_5);
#if DEBUG
    LOGI(LOG_TARGET, "nUpdate_Server: = %s ", update_vers.addr);
    LOGI(LOG_TARGET, "nUpdate_Server: = %d ", update_vers.port);
    LOGI(LOG_TARGET, "nUpdate_Server: = %s ", update_vers.id);
    LOGI(LOG_TARGET, "nUpdate_Server: = %s ", update_vers.pass);
    LOGI(LOG_TARGET, "nUpdate_Server: = %s ", update_vers.file);
#endif
	return 0;
}

int ini_field_get(char *title, char *field)
{
    dictionary  *ini ;
	char *s;

    ini = iniparser_load(SVRFTP);
    if (ini == NULL) {
        fprintf(stderr, "ini_field_get: %s\n", SVRFTP);
		LOGE(LOG_TARGET, "ini_field_get: %s ", SVRFTP);
        return -1;
    }

	s = iniparser_getstring(ini, title, NULL);
	if(s == NULL) {
		fprintf(stderr, "ini_field_get: %s field is null\n", title);
		LOGE(LOG_TARGET, "ini_field_get: %s field is null", title);
		return -1;
	}
    strcpy( field, s);

	LOGI(LOG_TARGET, "ini_field_get field =  field [ %s ]", field);

    iniparser_freedict(ini);
    return 0;
}

int ini_field_set(char *title, char *field)
{
    dictionary  *   ini ;

    ini = iniparser_load(SVRFTP);
    if (ini == NULL) {
        LOGE(LOG_TARGET, "ini_field_set: %s", SVRFTP);
        return -1;
    }

    LOGI(LOG_TARGET, "ini_field_set [ %s ] + [ %s ]", title, field);

    iniparser_set(ini, title, field);
    iniparser_store(ini, SVRFTP);

    iniparser_freedict(ini);
    return 0;
}


char*  ini_read(UPDATE_VERS vers)
{
    dictionary  *   ini ;


    /* Some temporary variables to hold query results */
    char        *   s = "NULL";
    static char version_info[40];

    ini = iniparser_load(SVRFTP);
    if (ini == NULL) {
        LOGE(LOG_TARGET, "cannot parse file: %s", SVRFTP);
        return s;
    }

    //printf("\nini_read update_ver[%d], release_ver[%d]",update_ver, release_ver);
    if ( vers.version == UPDATE_VCMD ) {
        s = iniparser_getstring(ini, "Update_Server:UPDATE_VER", NULL);
		if(s != NULL) {
	    	LOGI(LOG_TARGET, "\nini_read Update_Server:UPDATE_VER [ %s ]\n", s);
		}
		else {
			s = "NULL";
		}
    } else if ( vers.version == RELEASE_VCMD ) {
    	s = iniparser_getstring(ini, "Update_Server:RELEASE_VER", NULL);
		if(s != NULL) {
    		LOGI(LOG_TARGET, "\nini_read Update_Server:RELEASE_VER [ %s ]\n", s);
		}
		else {
			s = "NULL";
		}
    } else {
    	s = "NULL";
    }

    strcpy(version_info,s);

    iniparser_freedict(ini);

    return version_info;
}


void ini_write(UPDATE_VERS vers, char *info)
{
    dictionary  *   ini ;

    ini = iniparser_load(SVRFTP);
    if (ini == NULL) {
        LOGE(LOG_TARGET, "cannot parse file: %s\n", SVRFTP);
        return;
    }

    if ( vers.version == UPDATE_VCMD ) {
        iniparser_set(ini, "Update_Server:UPDATE_VER", info);
    } else if ( vers.version == RELEASE_VCMD ) {
        iniparser_set(ini, "Update_Server:RELEASE_VER", info);
    } else {
        LOGE(LOG_TARGET, "cannot file write [ %d ] [ %s ] \n", vers.version, info);  
    }

    iniparser_store(ini, SVRFTP);

    iniparser_freedict(ini);
}

void create_svrftp_ini_file(void)
{
    FILE    *   ini ;

    ini = safefopen(SVRFTP, "w");

    fprintf(ini,
    "#\n"
    "# This is an upgrade ftp server ini file\n"
    "#\n"
    "\n"
    "[Update_Server]\n"
    "\n"
    "ADDR        = NONE ;\n"
    "PORT        = NONE ;\n"
    "ID          = NONE ;\n"
    "PASS        = NONE ;\n"
    "FILE        = NONE ;\n"
    "UPDATE_VER  = NONE ;\n"
    "RELEASE_VER = NONE ;\n"
    "UPDATE_MD5  = NONE ;\n"
    "\n"
    "\n"
    "[Release_Server]\n"
    "\n"
    "ADDR        = NONE ;\n"
    "PORT        = NONE ;\n"
    "ID          = NONE ;\n"
    "PASS        = NONE ;\n"
    "FILE        = NONE ;\n"
    "UPDATE_VER  = NONE ;\n"
    "RELEASE_VER = NONE ;\n"
    "UPDATE_MD5  = NONE ;\n"
    "\n");

    fclose(ini);
}


int parse_ini_file(char * ini_name)
{
    dictionary  *   ini ;

    /* Some temporary variables to hold query results */

    ini = iniparser_load(ini_name);
    if (ini==NULL) {
        LOGE(LOG_TARGET, "cannot parse file: %s\n", ini_name);
        return -1 ;
    }

    //iniparser_dump(ini, stderr);

    iniparser_freedict(ini);
    return 0 ;
}

int tree_check(const char *root_dir_s, char *old_dir_s, char *new_dir_s)
{
  	char *ptr;
	double old_dir_d = 0;
	double new_dir_d = 0;
	DIR *dir_info;
	struct dirent *dir_entry;
	int is_found_old = 0;
	int is_found_new = 0;

	dir_info = safeopendir(root_dir_s);
	if (dir_info == NULL) {
		LOGE(LOG_TARGET, "can not open %s\n", root_dir_s);
		return -1;
	}

	while ((dir_entry = readdir(dir_info)) != NULL) {
		if (old_dir_d == 0) {
			old_dir_d = strtod(dir_entry->d_name, &ptr);
			if (old_dir_d != 0) {
				if (strcmp(ptr, "")) {
					old_dir_d = 0;
				} else {
					strcpy(old_dir_s, dir_entry->d_name);
					is_found_old = 1;
				}
			}
		} else {
			new_dir_d = strtod(dir_entry->d_name, &ptr);
			if (new_dir_d != 0) {
				if (strcmp(ptr, "")) {
					new_dir_d = 0;
				} else {
					strcpy(new_dir_s, dir_entry->d_name);
					is_found_new = 1;
					if (new_dir_d < old_dir_d) {
						LOGI(LOG_TARGET, "new_dir <-> old_dir\n");
						char tmp_dir_s[10] = {0,};
						double tmp_dir_d = 0;
						strcpy(tmp_dir_s, old_dir_s);
						strcpy(old_dir_s, new_dir_s);
						strcpy(new_dir_s, tmp_dir_s);
						tmp_dir_d = old_dir_d;
						old_dir_d = new_dir_d;
						new_dir_d = tmp_dir_d;
					}
				}
			}
		}
	}
	closedir(dir_info);
	
	
	if(is_found_new == 0 && is_found_old == 0)
	{
		old_dir_s[0] = 0;
		new_dir_s[0] = 0;
	}
	else if(is_found_new == 0 && is_found_old == 1)
	{
		strcpy(new_dir_s ,old_dir_s);
	}	
	
	return 0;
}
