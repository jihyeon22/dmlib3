#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/utsname.h>
#include <regex.h>

#include <dm/board_system.h>

static char *_hw_version = NULL;
static char *_amss_version = NULL;
static char *_linux_version = NULL;
static char *_telecom = NULL;

char* dm_get_hw_version(void)
{
	struct utsname buffer;

	if (_hw_version)
		return _hw_version;

	// step1. get utsname
	if (uname(&buffer) != 0) {
		switch (errno) {
			case EFAULT:
				printf("[dmlib] %s : call to uname() with an invalid pointer\n", __func__);
				break;
			default:
				printf("[dmlib] %s : error calling uname()\n", __func__);
		}
		return NULL;
	}

	// read from /etc/btime
	//   # cat /etc/btime
	//   TX501S:
	//   Fri Oct 21 14:09:29 KST 2016
	_hw_version = calloc(20, sizeof(char));
	{
		FILE *fp_sour;
		char	buff[1024];

		char 	*tr;
		char 	token_0[ ] = ",:\r\n";
		char 	*temp_bp = NULL;
		char 	*p_tmp = NULL;

		int i = 0;
		int upper_len = 0;

		fp_sour  = fopen( "/etc/btime"  , "r");
		if ( fp_sour == NULL )
		{
			strcpy(_hw_version, "UNKNOWN - case 1");
			return _hw_version;
		}

		if ( fgets( buff, 1024, fp_sour) <= 0 )
		{
			strcpy(_hw_version, "UNKNOWN - case 2");
		}
		fclose(fp_sour);

		p_tmp = buff;
		tr = strtok_r(p_tmp, token_0, &temp_bp);
	    if ( tr != NULL )	
		{
			upper_len = strlen(tr);
			for ( i = 0 ; i < upper_len ; i ++)
				_hw_version[i] = toupper(tr[i]);

			// switch to TX500X --> TX501X
			if (strncmp(_hw_version, "TX500S", strlen(_hw_version)) == 0)
			{
				memset(_hw_version, 0x00, 20);
				strcpy(_hw_version, "TX501S")
			}
		   	else if (strncmp(_hw_version, "TX500K", strlen(_hw_version)) == 0)
			{
				memset(_hw_version, 0x00, 20);
				strcpy(_hw_version, "TX501K")
			} 
			else if (strncmp(_hw_version, "TX500L", strlen(_hw_version)) == 0)
			{
				memset(_hw_version, 0x00, 20);
				strcpy(_hw_version, "TX501L")
			}
		}
		else
		{
			upper_len = strlen(buff);
			for ( i = 0 ; i < upper_len ; i ++)
				_hw_version[i] = toupper(buff[i]);
		}

	}
	
	printf("%s()-[%d] --> [%s]\r\n", __func__, __LINE__, _hw_version);
	return _hw_version;
}

char* dm_get_amss_version(void)
{
	struct utsname buffer;

	if (_amss_version)
		return _amss_version;

	// step1. get utsname
	if (uname(&buffer) != 0) {
		switch (errno) {
			case EFAULT:
				printf("[dmlib] %s : call to uname() with an invalid pointer\n", __func__);
				break;
			default:
				printf("[dmlib] %s : error calling uname()\n", __func__);
		}
		return NULL;
	}

	_amss_version = calloc(30, sizeof(char));

	{
		FILE *fp_sour;
		char	buff[1024];

		char 	*tr;
		char 	token_0[ ] = ",:\r\n";
		char 	*temp_bp = NULL;
		char 	*p_tmp = NULL;

		int i = 0;
		int upper_len = 0;


		fp_sour  = fopen( "/etc/version"  , "r");
		if ( fp_sour == NULL )
		{
			strcpy(_amss_version, "UNKNOWN - case 1");
			return _amss_version;
		}
		
		if ( fgets( buff, 1024, fp_sour) <= 0 )
		{
			strcpy(_amss_version, "UNKNOWN - case 2");
		}

		fclose(fp_sour);

		p_tmp = buff;
		tr = strtok_r(p_tmp, token_0, &temp_bp);
		if (tr != NULL)
		{
			upper_len = strlen(tr);
			for ( i = 0 ; i < upper_len ; i ++)
				_amss_version[i] = toupper(tr[i]);
		}
		else
		{
			upper_len = strlen(buff);
			for ( i = 0 ; i < upper_len ; i ++)
				_amss_version[i] = toupper(buff[i]);
		}

	}
	printf("%s()-[%d] --> [%s]\r\n", __func__, __LINE__, _amss_version);
	return _amss_version;
}

char* dm_get_linux_version(void)
{
	char buffer[128] = {0,};
	char linux_ver_str[128] = {0,};
	char* p_tmp = NULL;

	struct utsname utsname;

	if (_linux_version)
		return _linux_version;

	if (uname(&utsname) != 0) {
		switch (errno) {
			case EFAULT:
				printf("[dmlib] %s : call to uname() with an invalid pointer\n", __func__);
				break;
			default:
				printf("[dmlib] %s : error calling uname()\n", __func__);
		}
		return NULL;
	}
	
	_linux_version = calloc(50, sizeof(char));

	if ((p_tmp = strstr(utsname.version, "#1 PREEMPT ")) != NULL)
		strcpy(linux_ver_str, utsname.version + strlen("#1 PREEMPT "));
	else
		strcpy(linux_ver_str, utsname.version);

	
	if (_linux_version) {
		sprintf(buffer, "%s %s", utsname.release, linux_ver_str);
		strncpy(_linux_version, buffer, 50);
	}

	printf("%s()-[%d] --> [%s]\r\n", __func__, __LINE__, _linux_version);
	return _linux_version;
}

// FIXME: 개발버전은 모델명에 m2m이 들어가서 통신사는 항상 SKT로 인식
char* dm_get_telecom_provider(void)
{
	struct utsname buffer;

	if (_telecom)
		return _telecom;

	if (uname(&buffer) != 0) {
		switch (errno) {
			case EFAULT:
				printf("[dmlib] %s : call to uname() with an invalid pointer\n", __func__);
				break;
			default:
				printf("[dmlib] %s : error calling uname()\n", __func__);
		}
		return NULL;
	}
	
	char *str = strstr(buffer.machine, DEVICE_MODEL);

	if (str == NULL)
	{
		// SKT 향 (W100/200)
		_telecom = strdup(SKT_CARRIOR_MODEL);
	}
	else
	{
		// KT 향(W100/200K)
		_telecom = strdup(KT_CARRIOR_MODEL);
	}
	
	return _telecom;
}

