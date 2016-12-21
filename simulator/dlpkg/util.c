#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

int del_dir(char* path)
{
	DIR *dir_info;
	struct dirent *dir_entry;
	char file_path[256];
	struct stat file_stat;


	dir_info = opendir(path);
	if (dir_info == NULL) {
		fprintf(stderr, "can not open %s\n", path);
		return -1;
	}
	while ((dir_entry = readdir(dir_info))!= NULL) {
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
	closedir(dir_info);
	return rmdir(path);

failed:
	closedir(dir_info);
	return -1;
}
