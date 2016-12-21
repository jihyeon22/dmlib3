#ifndef _DM_H_
#define _DM_H_

#include <stddef.h>

#include <dm/internal.h>
#include <dm/rest_json.h>
#include <dm/event.h>
#include <dm/util.h>
#include <dm/package.h>
#include <dm/sms.h>
#include <dm/version.h>

struct dm_info {
	/* Session Infomation */
	struct { char *url; char *path_event; int timeout; } session;
	/* H/W and S/W Infomation */
	struct { char *hw; char *amss; char *linu; } version;
	/* Modem Infomation */
	struct { char *telecom; char *imei; char *phone; } modem;
	/* Service Infomation */
	struct { char *name; int major;	int minor; } package;
	/* Gpio Infomation */
	struct { 
		int ign; int epwr;
		int gp0; int gp1; int gp2; int gp3; int gp4; 
		int gp5; int gp6; int gp7; int gp8; int gp9;
	} gpio;
	/* Function */
	struct {
		int (*send_sms)(const char *dest, const char *msg, const int timeout);
		int (*rssi_get)(int *rssi);
		int (*gpio_get)(const int gpio);
		void (*gps_get)(dm_gpsData_t *out);
		int (*epwr_get)(void);
		int (*batt_get)(void);
	} func;
	struct { 
		char *usr0; char *usr1; char *usr2; 
		char *usr3; char *usr4; char *usr5;
	} data;
	/* DM Option */
	char *user_ini;
	char *user_package;
	int debug;
	/* DM Update Infomation */
	struct update_info *update_info;
};
typedef struct dm_info dm_info;

dm_res dm_global_init(void);
dm_res dm_global_shutdown(void);
dm_info* dm_get_info(void);
dm_res dm_set_info(dm_info *dmi);
#endif
