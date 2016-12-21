#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dm/safe_iniparser.h>

#include <dm/dm.h>
#include <dm/config.h>
#include <dm/constants.h>
#include <dm/safe.h>

dm_res dm_load_ini(dm_info *dmi, const char *file)
{
	if (!dmi || !file)
		return DM_FAIL;

	dictionary *dict = NULL;
	char *value = NULL;
	char buffer[128] = {0,};

	dict = safe_iniparser_load(file);
	if (!dict)
		return DM_FAIL;

	safefree(dmi->session.url);
	safefree(dmi->session.path_event);

	value = safe_iniparser_getstring(dict, "session:url", NULL);
	if (!value) {
		printf("[dmlib] %s : url is empty\n", __func__);
		dmi->session.url = NULL;
	} else {
		if (strncmp("http://", value, strlen("http://")) != 0) {
			memset(buffer, 0, sizeof(buffer));
			sprintf(buffer, "http://%s", value);
			dmi->session.url = strdup(buffer);
		} else {
			dmi->session.url = strdup(value);
		}
	}

	
	value = safe_iniparser_getstring(dict, "session:service", NULL);
	if (!value)
	printf("[dmlib] %s : service path is empty\n", __func__);
	dmi->session.path_event = value ? strdup(value) : NULL;
	
	dmi->session.timeout = safe_iniparser_getint(dict, "session:timeout", -1);
	if (dmi->session.timeout <= 0)
		printf("[dmlib] %s : timeout is invalid.\n", __func__);

	dmi->gpio.gp0 = safe_iniparser_getint(dict, "monitor:gp0", -1);
	dmi->gpio.gp1 = safe_iniparser_getint(dict, "monitor:gp1", -1);
	dmi->gpio.gp2 = safe_iniparser_getint(dict, "monitor:gp2", -1);
	dmi->gpio.gp3 = safe_iniparser_getint(dict, "monitor:gp3", -1);
	dmi->gpio.gp4 = safe_iniparser_getint(dict, "monitor:gp4", -1);
	dmi->gpio.gp5 = safe_iniparser_getint(dict, "monitor:gp5", -1);
	dmi->gpio.gp6 = safe_iniparser_getint(dict, "monitor:gp6", -1);
	dmi->gpio.gp7 = safe_iniparser_getint(dict, "monitor:gp7", -1);
	dmi->gpio.gp8 = safe_iniparser_getint(dict, "monitor:gp8", -1);
	dmi->gpio.gp9 = safe_iniparser_getint(dict, "monitor:gp9", -1);

	iniparser_freedict(dict);

	return DM_OK;
}

dm_res dm_save_ini(dm_info *dmi, const char *file)
{
	if (!dmi || !file)
		return DM_FAIL;

	char buffer[256] = {0};
	
	snprintf(buffer, sizeof(buffer), "echo [session] > %s", file);
	system(buffer);
	
	memset(buffer, 0, sizeof(buffer));
	dm_save_ini_dict("session:url", dmi->session.url, file);
	dm_save_ini_dict("session:path_event", dmi->session.path_event, file);
	snprintf(buffer, 5, "%d", dmi->session.timeout);
	dm_save_ini_dict("session:timeout", buffer, file);
	
	return DM_OK;
}

dm_res dm_save_ini_dict(const char *key, const char *value, const char *file)
{
	if (!key || !value || !file)
		return DM_FAIL;

	dictionary *dict = NULL;
	dict = safe_iniparser_load(file);
	if (!dict)
		return DM_FAIL;

	if (iniparser_set(dict, key, value) < 0) {
		iniparser_freedict(dict);
		return DM_FAIL;
	}

	safe_iniparser_store(dict, file);

	iniparser_freedict(dict);

	return DM_OK;
}
