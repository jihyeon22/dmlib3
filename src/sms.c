#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <regex.h>

#include <dm/dm.h>
#include <dm/board_system.h>
#include <dm/constants.h>
#include <dm/internal.h>
#include <dm/safe_iniparser.h>
#include <dm/config.h>
#include <dm/sys.h>
#include <dm/sms.h>

char *dm_sms_cmd[] = {
	"reset",
	"set url ",
	"set timeout ",
	"get url",
	"get timeout"
};

enum { 
	DM_SMS_RESET = 0,
	DM_SMS_SET_URL = 1,
	DM_SMS_SET_TIMEOUT = 2,
	DM_SMS_GET_URL = 3,
	DM_SMS_GET_TIMEOUT = 4
};

static bool _is_valid_ip(const char *ip);
static bool _is_digit(const char *str);
static char *_trimwhitespace(char *str);
static void _reset(const char *sender, bool reply);
static dm_res _set_url(const char *sender, const char *arg, bool reply);
static dm_res _set_timeout(const char *sender, const char *arg, bool reply);
static dm_res _get_url(const char *sender, bool reply);
static dm_res _get_timeout(const char *sender, bool reply);
static dm_res _invalid(const char *sender, const char *arg, bool reply);

static bool _is_valid_ip(const char *ip)
{
	struct sockaddr_in sa;
	if (inet_pton(AF_INET, ip, &(sa.sin_addr)) != 0)
		return true;
	
	return false;
}

static bool _is_digit(const char *str)
{
	int i, len;
	len = strlen(str);
	for (i = 0; i < len; i++) {
		if (!isdigit(str[i])) {
			return false;
		}
	}

	return true;

}

static char *_trimwhitespace(char *str)
{
	char *end;

	// Trim leading space
	while(isspace(*str)) str++;

	if(*str == 0)  // All spaces?
		return str;

	// Trim trailing space
	end = str + strlen(str) - 1;
	while(end > str && isspace(*end)) end--;

	// Write new null terminator
	*(end+1) = 0;

	return str;
}

static void _reset(const char *sender, bool reply)
{
	printf("[dmlib] %s : sender[%s], reply[%d]\n",__func__, sender, reply);
	dm_info *dmi = dm_get_info();
	
	if (reply && dmi->func.send_sms)
		dmi->func.send_sms(sender, "#dm> sms reset, bye~~", 10);

	//NORETURN
	dm_sys_poweroff(0);
}

//FIXME: needs ip, dns, port check routine
static dm_res _set_url(const char *sender, const char *arg, bool reply)
{
	dm_res res = DM_FAIL;
	bool validate = true;
	char buffer[128] = {0,};
	char *str = buffer;
	char *ptr;

	printf("[dmlib] %s : sender[%s] arg[%s], reply[%d]\n", __func__, sender, arg, reply);

	if (arg[strlen(arg) - 1] == '/' ||
			isalnum(arg[strlen(arg) - 1]) != 0) {
		memcpy(buffer, arg, strlen(arg) + 1);
		ptr = strtok(str, ":");	// ip or dns
		ptr = strtok(NULL, ":");	// port validation check
		if (ptr) {
			if (_is_digit(ptr) == true) {
				if (atoi(ptr) < 0 || atoi(ptr) > 65535) {
					printf("[dmlib] %s : %s is invalid range (0~65535)\n", __func__, ptr);
					validate = false;
				}
			} else {
				printf("[dmlib] %s : %s is invalid port\n", __func__, ptr);
				validate = false;
			}
		} else if (isdigit(arg[strlen(arg) - 1]) != 0) {	// ip validation check
			if (_is_valid_ip(arg) == false) {
				printf("[dmlib] %s : %s is invalid ip\n", __func__, arg);
				validate = false;
			}
		}

	} else {
		printf("[dmlib] %s : %s is invalid url\n", __func__, arg);
		validate = false;
	}


	//ip or dns check ????
	dm_info *dmi = dm_get_info();
	if (validate == true) {
		if (strncmp("http://", arg, strlen("http://")) != 0) {
			memset(buffer, 0, sizeof(buffer));
			sprintf(buffer, "http://%s", arg);
		}
		res = dm_save_ini_dict("session:url", buffer, dmi->user_ini ? dmi->user_ini : DM_INI);
	}

	if (reply && dmi->func.send_sms) {
		memset(buffer, 0, sizeof(buffer));
		if (res == DM_OK)
			snprintf(buffer, sizeof(buffer), "#dm> set url %s", validate == true ? "success" : "failture");
		else
			snprintf(buffer, sizeof(buffer), "#dm> set url failture [ini error]");

		dmi->func.send_sms(sender, buffer, 10);
	}

	return res;
}

static dm_res _set_timeout(const char *sender, const char *arg, bool reply)
{
	dm_res res = DM_FAIL;
	char buffer[128] = {0,};
	bool validate = false;
	int timeout = 0;

	printf("[dmlib] %s : sender[%s] arg[%s], reply[%d]\n", __func__, sender, arg, reply);
	
	dm_info *dmi = dm_get_info();
	if (_is_digit(arg) == true) {
		timeout = atoi(arg);
		if (timeout > 0 && timeout <= 60)
			res = dm_save_ini_dict("session:timeout", arg, dmi->user_ini ? dmi->user_ini : DM_INI);
		else {
			printf("[dmlib] %s : %s is invalid range (1~60)\n", __func__, arg);
			validate = false;
		}
	} else {
		printf("[dmlib] %s : %s is invalid digit\n", __func__, arg);
		validate = false;
	}

	if (reply && dmi->func.send_sms) {
		memset(buffer, 0, sizeof(buffer));
		if (res == DM_OK)
			snprintf(buffer, sizeof(buffer), "#dm> set timeout %s", validate == true ? "success" : "failture");
		else 
			snprintf(buffer, sizeof(buffer), "#dm> set timeout failture [ini error]");

		dmi->func.send_sms(sender, buffer, 10);
	}

	return res;
}

static dm_res _get_url(const char *sender, bool reply)
{
	dm_res res = DM_FAIL;
	dictionary *dict = NULL;
	char *value = NULL;
	char sms[80] = {0,};
	
	printf("[dmlib] %s : sender[%s], reply[%d]\n", __func__, sender, reply);
	
	dm_info *dmi = dm_get_info();
	if (reply && dmi->func.send_sms) {
		dict = safe_iniparser_load("/system/bin/dm.ini");
		if (!dict) {
			snprintf(sms, sizeof(sms), "#dm> dm.ini load failture");
		} else {
			value = safe_iniparser_getstring(dict, "session:url", NULL);
			if (!value) {
				snprintf(sms, sizeof(sms), "#dm> url load failture");
			} else {
				snprintf(sms, sizeof(sms), "#dm>\ndmi:%s\nini:%s", dmi->session.url, value);
				res = DM_OK;
			}

			dmi->func.send_sms(sender, sms, 10);
			
			iniparser_freedict(dict);
		}
			
	}

	return res;
}

static dm_res _get_timeout(const char *sender, bool reply)
{
	dm_res res = DM_FAIL;
	dictionary *dict = NULL;
	int timeout = -1;
	char sms[80] = {0,};
	
	printf("[dmlib] %s : sender[%s], reply[%d]\n", __func__, sender, reply);
	
	dm_info *dmi = dm_get_info();
	if (reply && dmi->func.send_sms) {
		dict = safe_iniparser_load("/system/bin/dm.ini");
		if (!dict) {
			snprintf(sms, sizeof(sms), "#dm> dm.ini load failture");
		} else {
			timeout = safe_iniparser_getint(dict, "session:timeout", -1);
			if (timeout == -1) {
				snprintf(sms, sizeof(sms), "#dm> timeout load failture");
			} else {
				snprintf(sms, sizeof(sms), "#dm>\ndmi:%d\nini:%d", dmi->session.timeout, timeout);
				res = DM_OK;
			}

			dmi->func.send_sms(sender, sms, 10);
			
			iniparser_freedict(dict);
		}
	}

	return res;
}

static dm_res _invalid(const char *sender, const char *arg, bool reply)
{
	dm_res res = DM_OK;

	printf("[dmlib] %s : arg[%s], reply[%d]\n", __func__, arg, reply);
	
	dm_info *dmi = dm_get_info();
	if (reply && dmi->func.send_sms) {
		dmi->func.send_sms(sender, "#dm> invalid sms", 10);
	}

	return res;
}

dm_res dm_sms_process(const char *time, const char *sender, const char *sms)
{
	dm_res res = DM_FAIL;
	if (!time && !sender && !sms)
		return DM_FAIL;

	bool reply;
	regex_t state;
	const char *pattern = "^s*([@#])dm>(.*)";
	size_t     nmatch = 3;
	regmatch_t pmatch[3];
	int status;
	char text[80] = {0,};
	char *trimtext = NULL;
	char *arg = NULL;
	int cmd = -1;
	int i;

	if (regcomp(&state, pattern, REG_EXTENDED))
		return DM_FAIL;

	status = regexec(&state, sms, nmatch, pmatch, 0);
	if (status != 0) {
		printf("[dmlib] %s : sms is not dm format\n", __func__);
		return DM_OK; // check

	} else {
		if (sms[pmatch[1].rm_eo - 1] == '#') {
			printf("[dmlib] %s : sms replay\n", __func__);
			reply = true;
		} else if (sms[pmatch[1].rm_eo - 1] == '@') {
			printf("[dmlib] %s : sms non-reply\n", __func__);
			reply = false;
		}

		sprintf(text, "%.*s", pmatch[2].rm_eo - pmatch[2].rm_so, &sms[pmatch[2].rm_so]);
		trimtext = _trimwhitespace(text);
	}
	regfree(&state);

	for (i = 0; i < (int)(sizeof(dm_sms_cmd)/sizeof(dm_sms_cmd[0])); i++) {
		if (strncmp(dm_sms_cmd[i], trimtext, strlen(dm_sms_cmd[i])) == 0) {
			cmd = i;
			break;
		}
	}

	if (cmd == DM_SMS_RESET || cmd == DM_SMS_GET_URL || cmd == DM_SMS_GET_TIMEOUT) {
		if (strcmp(dm_sms_cmd[cmd], trimtext) != 0)
			cmd = -1;
	} else {
		arg = &trimtext[strlen(dm_sms_cmd[cmd])];
		if (arg[0] == '\0') {
			cmd = -1;
		}
	}

	switch(cmd)
	{
		case DM_SMS_RESET:
			// NORETURN
			_reset(sender, reply);
			break;
		case DM_SMS_SET_URL:
			res = _set_url(sender, arg, reply);
			break;
		case DM_SMS_SET_TIMEOUT: 
			res = _set_timeout(sender, arg, reply);
			break;
		case DM_SMS_GET_URL:
			res = _get_url(sender, reply);
			break;
		case DM_SMS_GET_TIMEOUT:
			res = _get_timeout(sender, reply);
			break;
		default:
			res = _invalid(sender, trimtext, reply);
			break;
	}

	printf("[dmlib] %s : result %s\n", __func__, res == DM_OK ? "success" : "failure" );

	return res;
}
