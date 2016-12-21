/**
 * @file checker.c
 * @brief 
 * @author Jinwook Hong
 * @version 0.1
 * @date 2013-05-30
 */

/* Copyright (C) 
 * 2013 - Jinwook Hong
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "md5.h"
#include "update_state.h"

static FILE *log_fd;

static int del_dir(char* path);

int check_link_file(char *file)
{
	int count = 10;

	while(count--) {
		if(access(file, F_OK) == 0)
			return 0;

		sleep(1);
	}
	fprintf(log_fd, "\n=============================== \n");
	fprintf(log_fd, "%s can not open \n", file);
	fprintf(log_fd, "=============================== \n\n");
	return -1;
}


static int system_tree_check(char *old_dir_s, char *new_dir_s)
{
  	char *ptr;
	double old_dir_d = 0;
	double new_dir_d = 0;
	unsigned int dir_check = 0;
	DIR *dir_info;
	struct dirent *dir_entry;

	dir_info = opendir("/system");
	if (dir_info == NULL) {
		fprintf(log_fd, "can not open /system\n");
		return WRONG_SYSTEM_TREE;
	}

	while ((dir_entry = readdir(dir_info)) != NULL) {
		if (old_dir_d == 0) {
			old_dir_d = strtod(dir_entry->d_name, &ptr);
			if (old_dir_d != 0) {
				if (strcmp(ptr, "")) {
					old_dir_d = 0;
				} else {
					strcpy(old_dir_s, dir_entry->d_name);
					dir_check |= (0x1 << 0);
				}
			}
		} else {
			new_dir_d = strtod(dir_entry->d_name, &ptr);
			if (new_dir_d != 0) {
				if (strcmp(ptr, "")) {
					new_dir_d = 0;
				} else {
					strcpy(new_dir_s, dir_entry->d_name);
					dir_check |= (0x1 << 1);
					if (new_dir_d < old_dir_d) {
						fprintf(log_fd, "new_dir <-> old_dir\n");
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
		if (!strcmp(dir_entry->d_name, "bin"))
			dir_check |= (0x1 << 2);
		if (!strcmp(dir_entry->d_name, "simulinit"))
			dir_check |= (0x1 << 3);
		if (!strcmp(dir_entry->d_name, "sbin"))
			dir_check |= (0x1 << 4);
		if (!strcmp(dir_entry->d_name, "UPDATE"))
			dir_check |= (0x1 << 5);
	}
	closedir(dir_info);

	if( !(dir_check & (0x1 << 2)) || !(dir_check & (0x1 << 3)) || !(dir_check & (0x1 << 4))) {
		fprintf(log_fd, "wrong '/system' file tree 0x%02x\n", dir_check);
		return WRONG_SYSTEM_TREE;
	}

	if(check_link_file("/system/simulinit") < 0) {
		fprintf(log_fd, "/system/simuinit link file broken\n");
		return WRONG_SYSTEM_TREE;
	}
	if(check_link_file("/system/bin") < 0) {
		fprintf(log_fd, "/system/bin link file broken\n");
		return WRONG_SYSTEM_TREE;
	}
	
	if (dir_check == 0x1d) {
		fprintf(log_fd, "start normal routine\n");
		return 0;
	}

	if (dir_check == 0x3f) {
		fprintf(log_fd, "start update routine\n");
		return 1;
	} else if (dir_check == 0x3d) {
		fprintf(log_fd, "abnormal update routine 0x3d\n");
		system("rm -rf /system/UPDATE");
		return 0;
	}

	fprintf(log_fd, "exceptional routine 0x%02x\n", dir_check);
	return 0;
}

struct img_md5_pair{
	char on_img[128];
	char on_md5[128];
};

static int del_dir(char* path)
{
	DIR *dir_info;
	struct dirent *dir_entry;
	char file_path[256];
	struct stat file_stat;


	dir_info = opendir(path);
	if (dir_info == NULL) {
		fprintf(log_fd, "can not open %s\n", path);
		return -1;
	}
	while ((dir_entry = readdir(dir_info)) != NULL) {
		if (strcmp(".",dir_entry->d_name) && strcmp("..",dir_entry->d_name)) {
			memset(file_path, 0, 256);
			strcpy(file_path, path);
			strcat(file_path, "/");
			strcat(file_path, dir_entry->d_name);

			if (lstat(file_path, &file_stat)) {
				perror("can not get file stat");
				goto failed;
			}

			if (S_ISDIR(file_stat.st_mode)) {
				if (del_dir(file_path) < 0)
					goto failed;
			} else { 
				if (unlink(file_path) < 0)
					goto failed;
			}
		}
	}
	if(dir_info != NULL)
		closedir(dir_info);
	return rmdir(path);

failed:
	if(dir_info != NULL)
		closedir(dir_info);
	return -1;
}

static int update_tree_check(char *update_path, struct img_md5_pair *imp_list)
{
	DIR *dir_info;
	struct dirent *dir_entry;
	char tmp0[128] = {0,};
	int i;
	int img_num = 0;
	int md5_num = 0;
	int l_index = 0;

	dir_info = opendir(update_path);
	if (dir_info == NULL) {
		fprintf(log_fd, "can not open %s\n", update_path);
		return WRONG_UPDATE_TREE;
	}

	while ((dir_entry = readdir(dir_info)) != NULL) {
		if (strcmp(".",dir_entry->d_name) && strcmp("..",dir_entry->d_name)) {
			memset(tmp0, 0, 128);
			if (strstr(dir_entry->d_name, ".md5")) {
				md5_num++;
				strncpy(tmp0, dir_entry->d_name, strlen(dir_entry->d_name) - 4);
				for (i = 0; i < l_index; i++) {
					if ((imp_list[i].on_md5[0] == 0) && 
						(!strcmp(tmp0, imp_list[i].on_img))) {
						strcpy(imp_list[i].on_md5, dir_entry->d_name);
						break;
					}
				}
				if (i == l_index) {
					strcpy(imp_list[i].on_md5, dir_entry->d_name);
					l_index++;
				}
			} else {
				img_num++;
				strcpy(tmp0, dir_entry->d_name);
				strcat(tmp0, ".md5");
				for (i = 0; i < l_index; i++) {
					if ((imp_list[i].on_img[0] == 0) && 
						(!strcmp(tmp0, imp_list[i].on_md5))) {
						strcpy(imp_list[i].on_img, dir_entry->d_name);
						break;
					}
				}
				if (i == l_index) {
					strcpy(imp_list[i].on_img, dir_entry->d_name);
					l_index++;
				}
			}
		}
	}

	if(dir_info != NULL)
		closedir(dir_info);

	for (i = 0; i < l_index; i++)
		fprintf(log_fd,"index %d [%s : %s]\n", i, imp_list[i].on_img, imp_list[i].on_md5);
	if ((md5_num != img_num) || (md5_num != l_index)) {
		fprintf(log_fd, "not matched image file number and md5check file number %d : %d : %d\n", img_num, md5_num, l_index);
		return WRONG_UPDATE_TREE;
	}
	
	//add jwrho ++
	if(l_index < 2) {	
		//it is error if file count is under 2.
		del_dir(update_path);
		return WRONG_UPDATE_TREE;
	}
	//add jwrho 22

	return l_index;
}

int checker_tree(char *old_dir_s, char *new_dir_s) 
{
	int ret = 0;
	ret =  system_tree_check(old_dir_s, new_dir_s);
	return ret;
}

int checker_update(char *old_dir_s, char *new_dir_s) 
{
	int ret = UPDATE_SUCCESS;
	char old_path[32] = "/system/";
	char update_path[32] = "/system/";
	struct img_md5_pair imp_list[128];
	char img_file[128] = {0,};
	char md5_file[128] = {0,};
  	char sig_buffer[4096];
  	char stored_md5[33];
	FILE *stream;
	int i;

	strcat(old_path, old_dir_s);
	strcat(update_path, new_dir_s);

	printf("=========================================\n");
	printf("old_path = [%s]\n", old_path);
	printf("update_path = [%s]\n", update_path);
	printf("=========================================\n");

	memset(imp_list, 0, sizeof(struct img_md5_pair) * 128);
	ret = update_tree_check(update_path, imp_list);
	if (ret < 0) {
		del_dir("/system/UPDATE");
		return ret;
	}

	for (i = 0; i < ret; i++) {
		memset(stored_md5, 0, 33);
		strcpy(img_file, update_path);
		strcat(img_file, "/");
		strcat(img_file, imp_list[i].on_img);

		strcpy(md5_file, update_path);
		strcat(md5_file, "/");
		strcat(md5_file, imp_list[i].on_md5);

		stream = fopen(md5_file, "r");
		if (stream == NULL) {
			perror(md5_file);
			fprintf(log_fd, "%s> file open error\n", md5_file);
			ret = ERROR_FILE_HANDLING;
			goto UPDATE_FAIL;
		}

		if (fread(stored_md5, sizeof(char), 32, stream) < 0) {
			perror(md5_file);
			fprintf(log_fd, "%s> file read error\n", md5_file);
			ret = ERROR_FILE_HANDLING;
			goto UPDATE_FAIL;
		}
		fclose(stream);

		if (md5_read_file(img_file, sig_buffer, sizeof(sig_buffer)) < 0) {
			fprintf(log_fd, "can not get md5 checksum of image file\n");
			ret = ERROR_MD5_FROM_IMG;
			goto UPDATE_FAIL;
		}

		if (strcmp(sig_buffer, stored_md5)){
			fprintf(log_fd, "not matched with md5 checksum [%s : %s]\n", 
				sig_buffer, stored_md5);
			ret = MISMATCH_MD5;
			goto UPDATE_FAIL;
		}
	}
	if (unlink("/system/bin")) {
		fprintf(log_fd, "can not remove /system/bin link\n");
		ret = ERROR_FILE_HANDLING;
	}
	if (symlink(update_path, "/system/bin")) {
		fprintf(log_fd, "can not make link\n");
		symlink(old_path, "/system/bin");
		ret = ERROR_FILE_HANDLING;
		goto UPDATE_FAIL;
	}

	DIR *dir_info;
	struct dirent *dir_entry;
	dir_info = opendir(update_path);
	if (dir_info == NULL) {
		fprintf(log_fd, "can not open %s\n", update_path);
		ret = ERROR_FILE_HANDLING;
		goto UPDATE_FAIL;
	}
	while ((dir_entry = readdir(dir_info)) != NULL) {
		if (strstr(dir_entry->d_name, ".md5")) {
			memset(md5_file, 0, 128);
			strcpy(md5_file, "/system/bin/");
			strcat(md5_file, dir_entry->d_name);
			if (unlink(md5_file)) {
				fprintf(log_fd, "can not remove md5_file\n");
				break;
			}
		}
	}

	for(i = 0; i < 3; i++) {
		del_dir(old_path);
		del_dir("/system/UPDATE");
	}
	
	return UPDATE_SUCCESS;
	

UPDATE_FAIL:
	if(dir_info != NULL)
		closedir(dir_info);

	for(i = 0; i < 3; i++) {
		del_dir("/system/UPDATE");
		//del_dir(update_path);
	}

	return ret;
}

#define DTG_INIT_VERSION	"V1.00.03"
int main(int args, char *argv[])
{
	int ret;
	int retry = 0;
	char dtg_env[64];

#if defined(BOARD_NEO_W100)
	int fd, gpio_val;
#endif

char status;
	char old_dir_s[256] = {0,};
	char new_dir_s[256] = {0,};


#if defined(BOARD_NEO_W100)
	system("echo 9 > /sys/class/gpio/export");
	system("echo 10 > /sys/class/gpio/export");
	system("echo 11 > /sys/class/gpio/export");
	system("echo 12 > /sys/class/gpio/export");
	system("echo 13 > /sys/class/gpio/export");
	system("echo 14 > /sys/class/gpio/export");
	system("echo 15 > /sys/class/gpio/export");
	system("echo 16 > /sys/class/gpio/export");
	system("echo 17 > /sys/class/gpio/export");
	system("echo 18 > /sys/class/gpio/export");
#endif

    log_fd = fopen("/var/log/init.log", "w");
	if(log_fd != NULL)
	{
		int val;
		val = fcntl(fileno(log_fd), F_GETFD, 0);
		val |= FD_CLOEXEC;
		fcntl(fileno(log_fd), F_SETFD, val);		
	}
	fprintf(log_fd, "Start logging\n");

	fprintf(log_fd, "\n-------------------------------------------\n");
	fprintf(log_fd, "init %s %s %s\n", "DM SIMULRATOR", __DATE__, __TIME__);
	fprintf(log_fd, "-------------------------------------------\n\n");

#if defined(BOARD_NEO_W100)
	while (retry < 3)
	{
		/* open gpio sysfs */

		fd = open("/sys/class/gpio/gpio10/value", O_RDWR);
		if (fd < 0) {
			fprintf(stderr, "Could not open gpio sysfs\n");
			retry++;
		} else {
			break;
		}
		sleep(1);
	}
	if (retry >= 3){
		system("poweroff &");
		system("poweroff &");
		system("poweroff &");
		system("poweroff &");
	} else {
		retry = 0;
		while (retry < 3) {
			/* read gpio value */
			read(fd, &status, 1);
			gpio_val = atoi(&status);
			if (gpio_val == 1) //w100
				break;
			retry++;
			sleep(1);
		}
		if (gpio_val == 0) { //w100
			system("poweroff &");
			system("poweroff &");
			system("poweroff &");
			system("poweroff &");
		}
	}
	close(fd);
#endif //BOARD_NEO_W100

	retry = 0;
	while (retry < 5) {
		ret = checker_tree(old_dir_s, new_dir_s);
		if (ret < 0) {
			fprintf(log_fd, "wrong file system & dtg tree\n");
			sleep(2);
			retry++;
		} else {
			fprintf(log_fd, "state normal or success update\n");
			break;
		}
	}
	if (ret < 0){
		fprintf(log_fd, "Run recovery service.\n");
		system("ls -la /system/sbin/");
		system("touch /var/recov_5.dat");
		system("/system/sbin/recov_scv.sh");
		system("/system/sbin/alive.notifier end");
		system("/system/sbin/pwrd 10 10 &");
		fclose(log_fd);
		return 0;

	}
	printf("checker_tree---> [%d]\n", ret);

	if(ret == 1) //update need (when exist update folder) 
	{
		retry = 0;
		while (retry < 5) {
			ret = checker_update(old_dir_s, new_dir_s);
			if (ret < 0) {
				fprintf(log_fd, "update fail\n");
				sleep(2);
				retry++;
			} else {
				fprintf(log_fd, "update success\n");
				break;
			}
		}
		printf("checker_update---> [%d]\n", ret);
	}

	memset(dtg_env, 0, 64);
	sprintf(dtg_env, "%s=%02d", "DTG_UPD_STAT", ret);
	putenv(dtg_env);
	fprintf(log_fd, "%s	= %s\n", "DTG_UPD_STAT", getenv("DTG_UPD_STAT"));

	// simulator code
	system("/system/sbin/dlpkg3");
	printf("Waiting 5sec... before run dm\n");
	sleep(5);
	system("/system/bin/dm");
/* ORIGNAL_CODE
	if(check_link_file("/system/bin/mond") < 0) {
		fprintf(log_fd, "/system/bin/mond file don't exist\n");
		if(check_link_file("/system/bin/mon") < 0) {
			fprintf(log_fd, "/system/bin/mon file don't exist\n");
			fprintf(log_fd, "Run recovery service.\n");
			system("ls -la /system/sbin/");
			system("touch /var/recov_6.dat");
			system("/system/sbin/recov_scv.sh");
			system("/system/sbin/alive.notifier end");
			system("/system/sbin/pwrd 10 10 &");
		}
		else{
			system("/system/bin/mon");
		}
		
	}else {
		system("/system/bin/mond");
	}
*/
	
	fclose(log_fd);
	return 0;
}

