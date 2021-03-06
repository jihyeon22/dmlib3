#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "at.h"

static int _wait_read(int fd, unsigned char *buf, int buf_len, int ftime);

static char gPN[AT_LEN_PHONENUM_BUFF] = {0,};

int at_get_phonenum(char *pnumber, int buf_len)
{
	int fd = 0;
	int ret = -1;
	int retry;
	char *tr;
	int len, i;
	char token_0[ ] = ",";
	char *pAtcmd = NULL;
	char buffer[AT_MAX_BUFF_SIZE];
	char *temp_bp = NULL;

	int no_cmd_write = 0;
	
	if(gPN[0] != 0) {
		strncpy(pnumber, gPN, buf_len);
		return 0;
	}

	if(pnumber == NULL) {
		printf("<atd> %s : %d, buffer NULL error", __func__, __LINE__);
		return -1;
	}

	if(buf_len < AT_LEN_PHONENUM_BUFF) {
		printf("<atd> %s : %d, buffer size error", __func__, __LINE__);
		return -1;
	}

	fd = open(AT_DEV_FILE, O_RDWR | O_NONBLOCK);
	if(fd < 0)
	{
		printf("<atd> %s : %d, AT Channel Open Error errno[%d]", __func__, __LINE__, errno);
		return -1;
	}

	retry = 0;
	no_cmd_write = 0;
	while(retry++ < AT_MAX_RETRY_WRITE) 
	{
		if(retry > 1)
			sleep(1);

		if(!no_cmd_write) {
			ret = write(fd, "at$$msisdn\r\n", sizeof("at$$msisdn\r\n"));
			if(ret != sizeof("at$$msisdn\r\n"))
				continue;
		}

		memset(buffer, 0, sizeof(buffer));
		if(_wait_read(fd, buffer, AT_MAX_BUFF_SIZE, AT_MAX_WAIT_READ_SEC) < 0)
			continue;

		no_cmd_write = 0;
		pAtcmd = strstr(buffer, "$$MSISDN:");
		if(pAtcmd == NULL)
		{
			// input cmd echo skip..
			pAtcmd = strstr(buffer, "at$$msisdn");
			if(pAtcmd != NULL) {
				printf("read result is input cmd echo.... skip routine  \r\n");
				no_cmd_write = 1;
			}
			continue;
		}
		
		
		tr = strtok_r(pAtcmd, token_0, &temp_bp);
		if(tr == NULL) continue;
		tr = strtok_r(NULL, token_0, &temp_bp);
		if(tr == NULL) continue;
		tr = strtok_r(NULL, token_0, &temp_bp);
		if(tr == NULL) continue;
		
		len = strlen(tr);
		if(len < 10)
			continue;

		for(i = 0; i < len-1; i++)
			if(!(tr[i] >= '0' && tr[i] <= '9'))
				continue;

		break; //success
	}
	close(fd); //uard fd close
	
	if(retry >= 3) 
	{
		printf("<atd> %s : %d, AT Channel retry [%d]", __func__, __LINE__, retry);
		return -1;
	}
	
	if( (len-4) >= buf_len) {
		printf("<atd> %s : %d, buffer size error, buffer size[%d], need size[%d]", __func__, __LINE__, buf_len, len+1);
		return -1;
	}

	memset(pnumber, 0, buf_len);
	pnumber[0] = '0';
	memcpy(&pnumber[1], &tr[3], strlen(tr)-4);

	if(gPN[0] == 0) {
		memcpy(gPN, pnumber, buf_len);
	}

	return 0;
}

static int _wait_read(int fd, unsigned char *buf, int buf_len, int ftime)
{
	fd_set reads;
	struct timeval tout;
	int result = 0;

	FD_ZERO(&reads);
	FD_SET(fd, &reads);

	while (1) {
		tout.tv_sec = ftime;
		tout.tv_usec = 0;
		result = select(fd + 1, &reads, 0, 0, &tout);
		if(result <= 0) //time out & select error
			return -1;
		
		if ( read(fd, buf, buf_len) <= 0)
			return -1;

		break; //success
	}

	return 0;
}
