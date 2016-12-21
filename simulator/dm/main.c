#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <dm/dm.h>
#include <dm/event.h>
#include <dm/util.h>
#include <dm/package.h>

char* unit_get_hw_ver()
{
	return dm_get_info()->hw_version;
}

char* unit_get_imei(void)
{
	return dm_get_info()->imei;
}

char* unit_get_phone(void)
{
	return dm_get_info()->phone;
}

char* unit_get_up_ver()
{
	return dm_get_info()->service;
}

char* unit_get_telecom()
{
	return dm_get_info()->telecom;
}

dm_event_object objs_key_on[] = 
{
	{DM_LABEL_UP_VER,		unit_get_up_ver},
	{DM_LABEL_IMEI,			unit_get_imei},
	{NULL,					NULL}
};

dm_event_object objs_power_off[] =
{
	{DM_LABEL_IMEI,			unit_get_imei},
	{NULL,					NULL}
};

dm_event_object objs_power_on[] = 
{
	{DM_LABEL_HW_VER, 		unit_get_hw_ver},
	{DM_LABEL_TELECOM,		unit_get_telecom},
	{DM_LABEL_IMEI,			unit_get_imei},
	{DM_LABEL_PHONE,		unit_get_phone},
	{NULL,					NULL}
};

dm_event_object objs_key_off[] =
{
	{DM_LABEL_IMEI,			unit_get_imei},
	{NULL,					NULL}
};

dm_event_object objs_log[] = {
	{DM_LABEL_IMEI,			unit_get_imei},
	{NULL,					NULL}
};

dm_event_object objs_report[] = {
	{DM_LABEL_IMEI,			unit_get_imei},
	{NULL,					NULL}
};

char *rootdir = NULL;

int main(int arc, char **argv) {

	dm_info *dminfo;
	dm_res res;

	char inifile[256] = {0,};
	char packagefile[256] = {0,};

	/* DIR */
	rootdir = getenv("DM_ROOTDIR");
	if (rootdir) {
		sprintf(inifile, "%s/dm.ini", rootdir);
		sprintf(packagefile, "%s/PACKAGE", rootdir);
	} else {
		sprintf(inifile, "./dm.ini");
		sprintf(packagefile, "./PACKAGE");
	}

	while (1) {
		printf("**** DM CURL START ****\n");
		res = dm_global_init();
		if (res == DM_FAIL) {
			printf("dm_global_init failure\n");
			return -1;
		}
		printf("========== dm_global_init RESULT : %d ==========\n", res);

		res = dm_load_ini(inifile);
		if (res == DM_FAIL) {
			printf("%s file is not exist\n", inifile);
			return 0;
		}
		printf("%s file loaded\n", inifile);

		dminfo = dm_get_info();

		/* TELECOM */
		dminfo->telecom = "SKT";

		/* IMEI */
		char imei[16]= {0,};
		dminfo->imei = getenv("DM_IMEI");
		if (!dminfo->imei)
			sprintf(imei, "%s", "000000000001000");
		else
			sprintf(imei, "000000000001%03d", atoi(dminfo->imei));
		dminfo->imei = imei;

		/* PHONE */
		dminfo->phone = getenv("DM_PHONE");
		if (!dminfo->phone)
			dminfo->phone = "01012345678";

		/* HW VERISON */
		dminfo->hw_version = "W200";
		/* AMSS_VERSION */
		dminfo->amss_version = dm_get_amss_version();
		/* LINUX VERSION */
		dminfo->linux_version = dm_get_linux_version();

		/* SERVICE */
		char service[128] = {0,};
		res = dm_load_package_file(packagefile, service, 128);
		if (res == DM_FAIL) {
			printf("%s file is not exist\n", packagefile);
			return 0;
		}
		printf("%s file is loaded\n", packagefile);
		dminfo->service = service;
		
		printf("<SESSION>\n"
				" ADDR : %s, EVENT : %s, TIMEOUT : %d\n"
				"<VERSION>\n"
				" HW : %s\n"
				" AMSS : %s\n"
				" LINUX : %s\n"
				"<DEVICE>\n"
				" TELECOM : %s, IMEI : %s, PHONE : %s\n"
				"<SERVICE>\n SERVICE : %s\n"
				"<OPTION>\n AUTOUPDATE : %d\n",
				dminfo->url, dminfo->path_event, dminfo->timeout,
				dminfo->hw_version, dminfo->amss_version, dminfo->linux_version,
				dminfo->telecom, dminfo->imei, dminfo->phone,
				dminfo->service,
				dminfo->autoupdate);

		sleep(2);

		dm_allocate_event(DM_EVENT_PWR_ON, objs_power_on);
		dm_allocate_event(DM_EVENT_KEY_ON, objs_key_on);
		dm_allocate_event(DM_EVENT_REPORT, objs_report);
		dm_allocate_event(DM_EVENT_KEY_OFF, objs_key_off);
		dm_allocate_event(DM_EVENT_PWR_OFF, objs_power_off);
		dm_allocate_event(DM_EVENT_LOG, objs_log);
		dm_allocate_event("TEST", objs_log); // TEST: WRONG EVENT

		res = dm_event_append_obj(DM_EVENT_PWR_ON, 4,
				"sw_amss_ver", dminfo->amss_version ? dminfo->amss_version : "None",
				"sw_linux_ver", dminfo->linux_version);
		printf(">>>> %s RES:%d\n", DM_EVENT_PWR_ON, res);

		/* wrong Event */
		printf("SEND WRONG EVENT:TEST\n");
		res = dm_event_append_obj("TEST", 4,
				"sw_amss_ver", dminfo->amss_version ? dminfo->amss_version : "None",
				"sw_linux_ver", dminfo->linux_version);
		printf(">>>> RES:%d\n", res);
		sleep(1);

		res = dm_event(DM_EVENT_KEY_ON);
		if (res == DM_EV_RESP_UPDATE) {
			printf(">>>>>> NEEDS UPDATE <<<<<<\n");
		}
		printf(">>>> RES:%d\n", res);
		sleep(1);

		res = dm_event(DM_EVENT_REPORT);
		printf(">>>> RES:%d\n", res);
		sleep(1);
		
		res = dm_event(DM_EVENT_REPORT);
		printf(">>>> RES:%d\n", res);
		sleep(1);
		
		res = dm_event(DM_EVENT_REPORT);
		printf(">>>> RES:%d\n", res);
		sleep(1);
		
		res = dm_event(DM_EVENT_REPORT);
		printf(">>>> RES:%d\n", res);
		sleep(1);
		
		res = dm_event(DM_EVENT_REPORT);
		printf(">>>> RES:%d\n", res);
		sleep(1);

		res = dm_event_append_obj(DM_EVENT_LOG, 2, "log", "yoonki");
		printf(">>>> RES:%d\n", res);
		sleep(1);

		res = dm_event(DM_EVENT_KEY_OFF);
		printf(">>>> RES:%d\n", res);
		sleep(1);

		res = dm_event(DM_EVENT_PWR_OFF);
		printf(">>>> RES:%d\n", res);
		sleep(1);

		dm_release_all_event();

		dm_global_shutdown();
	}
    return 0;
}
