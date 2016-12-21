#ifndef _DM_EVENT_H_
#define _DM_EVENT_H_

typedef int dm_evres;

typedef struct dm_gpsData dm_gpsData_t;
struct dm_gpsData {
	unsigned short year;
	unsigned char mon;
	unsigned char day;
	unsigned char hour;
	unsigned char min;
	unsigned char sec;
	unsigned char satellite;
	float	lat;			// latitude
	float	lon;			// longitude
	int speed;			// speed(km/s)
	unsigned char active;
	float angle;
	time_t utc_sec;
	float hdop;
};

enum _DM_EVENT_RESULT {
	DM_EV_RESP_OK = 0,
	DM_EV_RESP_UPDATE,
	DM_EV_RESP_ERR_SERVER,
	DM_EV_RESP_ERR_DATA,
	DM_EV_RESP_NO_REG,
	DM_EV_RESP_SETTING
};
typedef enum _DM_EVENT_RESULT DM_EVENT_RESULT;	/**<EVENT results returned by the server */

// LABEL
#define DM_LABEL_EVENT			"event"
#define DM_LABEL_HW_VER			"hw_ver"
#define DM_LABEL_SW_AMSS_VER	"sw_amss_ver"
#define DM_LABEL_SW_LINUX_VER	"sw_linux_ver"
#define DM_LABEL_TELECOM		"telecom"
#define DM_LABEL_UP_VER			"up_ver"
#define DM_LABEL_IMEI			"imei"
#define DM_LABEL_PHONE			"phone"
#define	DM_LABEL_GP0			"gp0"
#define	DM_LABEL_GP1			"gp1"
#define	DM_LABEL_GP2			"gp2"
#define	DM_LABEL_GP3			"gp3"
#define	DM_LABEL_GP4			"gp4"
#define	DM_LABEL_GP5			"gp5"
#define	DM_LABEL_GP6			"gp6"
#define	DM_LABEL_GP7			"gp7"
#define DM_LABEL_GP8			"gp8"
#define	DM_LABEL_GP9			"gp9"
#define	DM_LABEL_USR0			"usr0"	
#define	DM_LABEL_USR1			"usr1"
#define	DM_LABEL_USR2			"usr2"
#define	DM_LABEL_USR3			"usr3"
#define	DM_LABEL_USR4			"usr4"
#define	DM_LABEL_USR5			"usr5"
#define	DM_LABEL_MAIN_VOLT		"main_volt"
#define	DM_LABEL_BATT_VOLT		"batt_volt"
#define DM_LABEL_KEY_STAT		"key_stat"
#define DM_LABEL_PWR_STAT		"pwr_stat"
#define	DM_LABEL_GPS_LAT		"gps_lat"
#define DM_LABEL_GPS_LON		"gps_lon"
#define	DM_LABEL_GPS_SPEED		"gps_speed"
#define DM_LABEL_GPS_HDOP		"gps_hdop"
#define DM_LABEL_LOG			"log"
#define DM_LABEL_ERROR			"errno"
#define DM_LABEL_RSSI			"rssi"

#define DM_LABEL_RESULT			"result"

// EVENT
#define DM_EVENT_PWR_ON		"power on"
#define DM_EVENT_PWR_OFF	"power off"
#define DM_EVENT_KEY_ON		"key on"
#define DM_EVENT_KEY_OFF	"key off"
#define DM_EVENT_REPORT		"report"
#define DM_EVENT_STATUS		"status"
#define DM_EVENT_LOG		"log"
#define DM_EVENT_WARNING	"warning"
#define DM_EVENT_BREAKDOWN	"breakdown"
#define DM_EVENT_UPDATE		"check update"

#define EVO_BASE			(unsigned long long) 0x1
#define EVO_HW_VER			EVO_BASE << 1
#define EVO_AMSS_VER		EVO_BASE << 2
#define EVO_LINUX_VER		EVO_BASE << 3
#define EVO_UP_VER			EVO_BASE << 4
#define EVO_TELECOM			EVO_BASE << 5
#define EVO_IMEI			EVO_BASE << 6
#define EVO_PHONE			EVO_BASE << 7
#define EVO_RSSI			EVO_BASE << 8
#define EVO_GP0				EVO_BASE << 9
#define EVO_GP1				EVO_BASE << 10
#define EVO_GP2				EVO_BASE << 11
#define EVO_GP3				EVO_BASE << 12
#define EVO_GP4				EVO_BASE << 13
#define EVO_GP5				EVO_BASE << 14
#define EVO_GP6				EVO_BASE << 15
#define EVO_GP7				EVO_BASE << 16
#define EVO_GP8				EVO_BASE << 17
#define EVO_GP9				EVO_BASE << 18
#define EVO_USR0			EVO_BASE << 19
#define EVO_USR1			EVO_BASE << 20
#define EVO_USR2			EVO_BASE << 21
#define EVO_USR3			EVO_BASE << 22
#define EVO_USR4			EVO_BASE << 23
#define EVO_USR5			EVO_BASE << 24
#define EVO_MAIN_VOLT		EVO_BASE << 25
#define EVO_BATT_VOLT		EVO_BASE << 26
#define EVO_KEY_STAT		EVO_BASE << 27
#define EVO_PWR_STAT		EVO_BASE << 28
#define EVO_GPS_LAT			EVO_BASE << 29
#define EVO_GPS_LON			EVO_BASE << 30
#define EVO_GPS_SPEED		EVO_BASE << 31
#define EVO_GPS_HDOP		EVO_BASE << 32
#define EVO_LOG				EVO_BASE << 33
#define EVO_ERRNO			EVO_BASE << 34
#define EVO_RESERVED		EVO_BASE << 35
#define DM_EVENT_


dm_res dm_evo_insert_usrdata(unsigned long long evo, char *data);
dm_evres dm_event(const char* event, unsigned long long evo, const char *log, int evo_errno);

#endif
