#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <regex.h>

#include <dm/dm.h>
#include <dm/event.h>
#include <dm/util.h>
#include <dm/package.h>
#include <dm/update.h>

int at_send_sms(const char *dest, const char *msg, const int timeout)
{
	return 1;
}

int at_get_rssi(char *buff, const int len)
{
	buff = "-60";

	return 1;
}

int gpio_get_value(const int gpio)
{
	return gpio % 2 == 0 ? 1 : 0;
}

int battery_get_battlevel_car(void)
{
	int dummy = 0;

	dummy = rand();
	dummy = dummy % 10000;
	
	return dummy;
}

int battery_get_battlevel_internal(void)
{
	int dummy = 0;

	dummy = rand();
	dummy = dummy % 10000;
	
	return dummy;
}

void gps_get_curr_data(dm_gpsData_t *out)
{
	out->lat = 127.1234;
	out->lon = 127.1234;
	out->speed = 180;
	out->hdop = 12.34;
}

void sample_allocate_dm_info(dm_info *dmi, char *imei, char *phone);


#define ALL_EVENT	(EVO_HW_VER | EVO_AMSS_VER | EVO_LINUX_VER | EVO_UP_VER | \
					EVO_TELECOM | EVO_IMEI | EVO_PHONE | EVO_RSSI | \
					EVO_GP0 | EVO_GP1 | EVO_GP2 | EVO_GP3 | EVO_GP4 | \
					EVO_GP5 | EVO_GP6 | EVO_GP7 | EVO_GP8 | EVO_GP9 | \
					EVO_USR0 | EVO_USR1 | EVO_USR2 | EVO_USR3 | EVO_USR4 | EVO_USR5 | \
					EVO_MAIN_VOLT | EVO_BATT_VOLT | \
					EVO_KEY_STAT | EVO_PWR_STAT | \
					EVO_GPS_LAT | EVO_GPS_LON | EVO_GPS_SPEED | EVO_GPS_HDOP | \
					EVO_LOG | EVO_ERRNO)

#define TEST_EVENT	(EVO_HW_VER | EVO_AMSS_VER | EVO_LINUX_VER | EVO_UP_VER | \
					EVO_TELECOM | EVO_IMEI | EVO_PHONE | EVO_RSSI | \
					EVO_KEY_STAT | EVO_PWR_STAT | \
					EVO_GPS_LAT | EVO_GPS_LON | EVO_GPS_SPEED | EVO_GPS_HDOP | \
					EVO_LOG | EVO_ERRNO)

int main(int argc, char **argv) {

	dm_info *dmi = NULL;
	dm_res res;
	char packagename[255] = {0};
	char appkey[33]={0};

	printf("**** %s-%s TEST APP****\n", libdm_name, libdm_version);
	res = dm_global_init();
	if (res == DM_FAIL) {
		printf("dm_global_init failure\n");
		return -1;
	}

	dmi = dm_get_info();
	dmi->user_ini = "./dm.ini";
	dmi->user_package = "./PACKAGE";

	res = dm_load_ini(dmi, dmi->user_ini);
	if (res == DM_FAIL) {
		printf("dm_load_ini failure\n");
		return -1;
	}

	sample_allocate_dm_info(dmi, argv[1], argv[2]);
	sprintf(packagename, "%s", dmi->package.name);

	// regex appkey-version (ex. asdf3asdf
	{
	regex_t    preg;   
	char       *pattern = "(.*{1,32})-([[:digit:]]{1,2}\\.[[:digit:]]{1,2})$";
	int        rc;     
	size_t     nmatch = 3;
	regmatch_t pmatch[3];
	
	if (0 != (rc = regcomp(&preg, pattern, REG_EXTENDED))) {
		printf("regcomp() failed, returning nonzero (%d)\n", rc);
		exit(EXIT_FAILURE);
	}

	if (0 != (rc = regexec(&preg, packagename, nmatch, pmatch, 0))) {
		printf("Failed to match '%s' with '%s',returning %d.\n",
				packagename, pattern, rc);
	}
	else {  
		printf("appkey, %.*s\n",
				pmatch[1].rm_eo - pmatch[1].rm_so, &packagename[pmatch[1].rm_so]);
		printf("version, %.*s\n",
				pmatch[2].rm_eo - pmatch[2].rm_so, &packagename[pmatch[2].rm_so]);

		sprintf(appkey, "%.*s", pmatch[1].rm_eo - pmatch[1].rm_so, &packagename[pmatch[1].rm_so]);
	}
	regfree(&preg);
	}

	unsigned int i = 0;
	char logmsg[255] = {0};
	while (1) {
		// Scenario Loop
		dm_evo_insert_usrdata(EVO_USR0, "TEST USR0");
		dm_evo_insert_usrdata(EVO_USR1, "TEST USR1");
		dm_evo_insert_usrdata(EVO_USR2, "TEST USR2");
		dm_evo_insert_usrdata(EVO_USR3, "TEST USR3");
		dm_evo_insert_usrdata(EVO_USR4, "TEST USR4");
		dm_evo_insert_usrdata(EVO_USR5, "TEST USR5");
		
		sleep(5);
		res = dm_event(DM_EVENT_PWR_ON, TEST_EVENT, "simulator - power on", 0);

		sleep(5);
		res = dm_event(DM_EVENT_KEY_ON, TEST_EVENT, "simulator - key on", 0);
		
		sleep(5);
		res = dm_event(DM_EVENT_UPDATE, TEST_EVENT, "simulator - check update", 0);
		if (dmi->update_info) {
			{
				regex_t    preg;   
				char       *pattern = "/.*/(.*)-([[:digit:]]{1,2}\\.[[:digit:]]{1,2})\\.(tar\\.gz|tgz|tar)$";
				int        rc;     
				size_t     nmatch = 3;
				regmatch_t pmatch[3];

				if (0 != (rc = regcomp(&preg, pattern, REG_EXTENDED))) {
					printf("regcomp() failed, returning nonzero (%d)\n", rc);
					exit(EXIT_FAILURE);
				}

				if (0 != (rc = regexec(&preg, dmi->update_info->file, nmatch, pmatch, 0))) {
					printf("Failed to match '%s' with '%s',returning %d.\n",
							dmi->update_info->file, pattern, rc);
				}
				else {  
					printf("appkey, %.*s\n",
							pmatch[1].rm_eo - pmatch[1].rm_so, &(dmi->update_info->file)[pmatch[1].rm_so]);
					printf("version, %.*s\n",
							pmatch[2].rm_eo - pmatch[2].rm_so, &(dmi->update_info->file)[pmatch[2].rm_so]);

					memset(packagename, 0, sizeof(packagename));
					sprintf(packagename, "%.*s-%.*s", 
							pmatch[1].rm_eo - pmatch[1].rm_so,
							&(dmi->update_info->file[pmatch[1].rm_so]),
							pmatch[2].rm_eo - pmatch[2].rm_so, 
							&(dmi->update_info->file[pmatch[2].rm_so]));
				}
				regfree(&preg);
			}
			sleep(5);
			res = dm_event(DM_EVENT_PWR_OFF, TEST_EVENT, "simulator power off", 0);
			dmi->package.name = packagename;
			
			sleep(5);
			continue;
		}
		
		sleep(5);

		for (i=0; i<10; i++) {
			memset(logmsg, 0, sizeof(logmsg));
			sprintf(logmsg, "%s [%d]", "simulator - status", i);
			res = dm_event(DM_EVENT_STATUS, TEST_EVENT, logmsg, i);
			sleep(1);
		}

		sleep(5);
		res = dm_event(DM_EVENT_KEY_OFF, TEST_EVENT, "simulator key off", 0);
		
		sleep(5);
		res = dm_event(DM_EVENT_PWR_OFF, TEST_EVENT, "simulator power off", 0);
	}

	dm_global_shutdown();
	
	return 0;
}

void sample_allocate_dm_info(dm_info *dmi, char *imei, char *phone)
{
	static char imeibuf[16];
	
	sprintf(imeibuf, "00000000000%s", imei);
	dmi->version.hw 	= dm_get_hw_version();
	dmi->version.amss 	= dm_get_amss_version();
	dmi->version.linu 	= dm_get_linux_version();
	
	dmi->modem.telecom	= dm_get_telecom_provider();
	dmi->modem.imei		= imeibuf;
	dmi->modem.phone	= strdup(phone);
	
	dmi->package.name 	= dm_load_package_file(dmi->user_package);
	dmi->package.major	= 0;
	dmi->package.minor	= 1;
	
	dmi->gpio.ign		= 10;
	dmi->gpio.epwr		= 66;
//	dmi->gpio.gp0		= 11;
//	dmi->gpio.gp1		= 13;
//	dmi->gpio.gp2		= 14;
//	dmi->gpio.gp3		= 15;
//	dmi->gpio.gp4		= -1;
//	dmi->gpio.gp5		= 18;
//	dmi->gpio.gp6		= 50;
//	dmi->gpio.gp7		= 51;
//	dmi->gpio.gp8		= 52;
//	dmi->gpio.gp9		= 53;
	
	dmi->func.send_sms	= at_send_sms;
	dmi->func.rssi_get	= at_get_rssi;
	dmi->func.gpio_get	= gpio_get_value;
	dmi->func.gps_get	= gps_get_curr_data;
	dmi->func.epwr_get	= battery_get_battlevel_car;
	dmi->func.batt_get	= battery_get_battlevel_internal;
}
