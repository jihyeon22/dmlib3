#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

#include <dm/rest_json.h>
#include <dm/event.h>
#include <dm/evlist.h>
#include <dm/update.h>
#include <dm/safe.h>

json_t* dm_create_evo(json_t *obj, dm_info *dmi, const char *event, unsigned long long evo, const char *log, int evo_errno);


const char* dm_get_event_result_string(int result)
{
	switch (result) {
		case DM_EV_RESP_ERR_SERVER:	return "server no result"; 
		case DM_EV_RESP_ERR_DATA:	return "invalid date"; 
		case DM_EV_RESP_NO_REG:		return "no registration"; 
		case DM_EV_RESP_OK:			return "ok";
		case DM_EV_RESP_UPDATE:		return "needs update";
		case DM_EV_RESP_SETTING:	return "setting"; 
		default: return NULL;
	}
}

static dm_evres _dm_event_resp(dm_info *dmi, json_t *body)
{
	dm_evres evres;
	json_t *jres;
	json_t *addr, *port, *id, *pw, *file;

	jres = json_object_get(body, DM_LABEL_RESULT);
	if (!jres) {
		return DM_EV_RESP_ERR_SERVER;
	}

	evres = json_integer_value(jres);
	/* Update Routine */
	if (evres == DM_EV_RESP_UPDATE) {
		addr = json_object_get(body, "ip");
		port = json_object_get(body, "port");
		file = json_object_get(body, "path");
		id = json_object_get(body, "id");
		pw = json_object_get(body, "password");
		if (!addr || !port || !file || !id || !pw)
			return DM_ERR_UPDATE_INFO;
		
		if (!dmi->update_info) {
			// TODO: needs free
			dmi->update_info = malloc(sizeof(struct update_info));
			memset(dmi->update_info, 0, sizeof(struct update_info));
		} /* TODO: Check Multi Thread */
	
		safefree(dmi->update_info->addr);
		safefree(dmi->update_info->id);
		safefree(dmi->update_info->pw);
		safefree(dmi->update_info->file);

		dmi->update_info->addr = strdup(json_string_value(addr));
		dmi->update_info->port = atoi(json_string_value(port));
		dmi->update_info->id = strdup(json_string_value(id));
		dmi->update_info->pw = strdup(json_string_value(pw));
		dmi->update_info->file = strdup(json_string_value(file));
	}

	return evres;
}

dm_res dm_evo_insert_usrdata(unsigned long long evo, char *data)
{
	dm_info *dmi = dm_get_info();
	if (!dmi) {
		printf("[dmlib] %s : dm infomation is NULL\n", __func__);
		return DM_FAIL;
	}

	switch (evo) {
		case EVO_USR0: 	dmi->data.usr0 = data;	break;
		case EVO_USR1:	dmi->data.usr1 = data;	break;
		case EVO_USR2:	dmi->data.usr2 = data;	break;
		case EVO_USR3:	dmi->data.usr3 = data;	break;
		case EVO_USR4:	dmi->data.usr4 = data;	break;
		case EVO_USR5:	dmi->data.usr5 = data;	break;
		default: return DM_FAIL;
	}

	return DM_OK;
}

dm_evres dm_event(const char *event, unsigned long long evo, const char *log, int evo_errno)
{
	if (!event)
		return DM_ERR_INVALID_PARAMETER;

    dm_res res;
	dm_info* dmi = dm_get_info();
	if (!dmi) {
		printf("[dmlib] %s : dm infomation error\n", __func__);
		return DM_FAIL;
	}

	/* event json */
	json_t* obj = json_object();
    json_t* jres = NULL;

	/* create event */
	obj = dm_create_evo(obj, dmi, event, evo, log, evo_errno);

	/* session */
	dm_session* dms = dm_create_session();
	if (!dms) return DM_ERR_INTERNAL_ERROR;

	if (dm_set_base_url(dms, dmi->session.url) == DM_FAIL)
		return DM_ERR_INTERNAL_ERROR;

    // log event is not setting timeout.
    if ( strcmp(DM_EVENT_LOG, event) == 0 )
    {
        dmi->session.timeout = 0;
        printf("[dmlib] %s : dm log timeout val set to zero\n", __func__);
    }

    if (dm_set_timeout(dms, dmi->session.timeout) == DM_FAIL)
        return DM_ERR_INTERNAL_ERROR;

	/* json request */
	res = dm_enroll_event_json(dms, event, obj, &jres);
	json_decref(obj);
	
	/* json response */
	if (res == DM_OK && jres) {
		res = _dm_event_resp(dmi, jres);
		printf(">>>>>EVENT [%s] RESULT[%d]-> %s \n", event, res, dm_get_event_result_string(res));
		json_decref(jres);
	} else {
		res = dm_get_last_err_code(dms);
		printf(">>>>>EVENT [%s] ERROR[%d] -> %s \n", event, res, dm_get_last_err_string(dms));
	}

	dm_destroy_session(dms);

	return res;
}

dm_evres dm_event_append_obj(const char *event, unsigned long long evo, const char *log, int evo_errno, int num, ...)
{
	if (!event || num % 2 != 0)
		return DM_ERR_INVALID_PARAMETER;

    dm_res res;
	dm_info* dmi = dm_get_info();
	if (!dmi) {
		printf("[dmlib] %s : dm infomation error\n", __func__);
		return DM_FAIL;
	}

	/* event json */
	json_t* obj = json_object();
    json_t* jres = NULL;

	/* create event */
	obj = dm_create_evo(obj, dmi, event, evo, log, evo_errno);

	/* append event objs */
	va_list arguments;
	int i;
	va_start(arguments, num);
	for(i = 0; i < num/2; i++) {
		char *key = va_arg(arguments, char*);
		char *value = va_arg(arguments, char*);
		json_object_set_new(obj, key, json_string(value));
	}
	va_end(arguments);

	/* session */
	dm_session* dms = dm_create_session();
	if (!dms) return DM_ERR_INTERNAL_ERROR;

	if (dm_set_base_url(dms, dmi->session.url) != DM_OK)
		return DM_ERR_INTERNAL_ERROR;

	if (dm_set_timeout(dms, dmi->session.timeout) != DM_OK)
		return DM_ERR_INTERNAL_ERROR;

	/* json request */
	res = dm_enroll_event_json(dms, event, obj, &jres);
	json_decref(obj);
	
	/* json response */
	if (res == DM_OK && jres) {
		res = _dm_event_resp(dmi, jres);
		printf(">>>>>EVENT [%s] RESULT[%d]-> %s \n", event, res, dm_get_event_result_string(res));
		json_decref(jres);
	} else {
		res = dm_get_last_err_code(dms);
		printf(">>>>>EVENT [%s] ERROR[%d] -> %s \n", event, res, dm_get_last_err_string(dms));
	}

	dm_destroy_session(dms);

	return res;
}

json_t* dm_create_evo(json_t *obj, dm_info *dmi, const char *event, unsigned long long evo, const char *log, int evo_errno)
{
	int i;
	dm_gpsData_t out;
	int gps_flag = 0;
	memset(&out, sizeof(out), 0);
	
	json_object_set_new(obj, "event", json_string(event));
	
	printf("[dmlib] evo=%llu\n", evo);
	for ( i = 0; i < 64; i++) {
		//printf("TEST i=%d evo=%llu\n", i, evo >> i );
		switch (evo & ((unsigned long long)0x1 << i)) 
		{
			case EVO_HW_VER:
				if (dmi->version.hw)
					json_object_set_new(obj, "hw_ver", json_string(dmi->version.hw));
				break;
			case EVO_AMSS_VER:
				if (dmi->version.amss)
					json_object_set_new(obj, "sw_amss_ver", json_string(dmi->version.amss));
				break;
			case EVO_LINUX_VER:
				if (dmi->version.linu)
					json_object_set_new(obj, "sw_linux_ver", json_string(dmi->version.linu));
				break;
			case EVO_UP_VER:
				if (dmi->package.name)
					json_object_set_new(obj, "up_ver", json_string(dmi->package.name));
				break;

			case EVO_TELECOM:
				if (dmi->modem.telecom)
					json_object_set_new(obj, "telecom", json_string(dmi->modem.telecom));
				break;
			case EVO_IMEI:
				if (dmi->modem.imei)
					json_object_set_new(obj, "imei", json_string(dmi->modem.imei));
				break;
			case EVO_PHONE:
				if (dmi->modem.phone)
					json_object_set_new(obj, "phone", json_string(dmi->modem.phone));
				break;
			case EVO_RSSI:
				{
					if (dmi->func.rssi_get) {
						char str_rssi[10] = {0};
						int rssi;
						int res = (dmi->func.rssi_get)(&rssi);
						if (res == 0) {
							sprintf(str_rssi, "%d", rssi);
							json_object_set_new(obj, "rssi", json_string(str_rssi));
						}
					}
				}
				break;

			case EVO_GP0:
				if (dmi->gpio.gp0 && dmi->func.gpio_get) {
					json_t *gpio = json_integer((dmi->func.gpio_get)(dmi->gpio.gp0));
					json_object_set_new(obj, "gp0", gpio);
				}
				break;
			case EVO_GP1:
				if (dmi->gpio.gp1 > -1 && dmi->func.gpio_get) {
					json_t *gpio = json_integer((dmi->func.gpio_get)(dmi->gpio.gp1));
					json_object_set_new(obj, "gp1", gpio);
				}
				break;
			case EVO_GP2:
				if (dmi->gpio.gp2 > -1 && dmi->func.gpio_get) {
					json_t *gpio = json_integer((dmi->func.gpio_get)(dmi->gpio.gp2));
					json_object_set_new(obj, "gp2", gpio);
				}
				break;
			case EVO_GP3:
				if (dmi->gpio.gp3 > -1 && dmi->func.gpio_get) {
					json_t *gpio = json_integer((dmi->func.gpio_get)(dmi->gpio.gp3));
					json_object_set_new(obj, "gp3", gpio);
				}
				break;
			case EVO_GP4:
				if (dmi->gpio.gp4 > -1 && dmi->func.gpio_get) {
					json_t *gpio = json_integer((dmi->func.gpio_get)(dmi->gpio.gp4));
					json_object_set_new(obj, "gp4", gpio);
				}
				break;
			case EVO_GP5:
				if (dmi->gpio.gp5 > -1 && dmi->func.gpio_get) {
					json_t *gpio = json_integer((dmi->func.gpio_get)(dmi->gpio.gp5));
					json_object_set_new(obj, "gp5", gpio);
				}
				break;
			case EVO_GP6:
				if (dmi->gpio.gp6 > -1 && dmi->func.gpio_get) {
					json_t *gpio = json_integer((dmi->func.gpio_get)(dmi->gpio.gp6));
					json_object_set_new(obj, "gp6", gpio);
				}
				break;
			case EVO_GP7:
				if (dmi->gpio.gp7 > -1 && dmi->func.gpio_get) {
					json_t *gpio = json_integer((dmi->func.gpio_get)(dmi->gpio.gp7));
					json_object_set_new(obj, "gp7", gpio);
				}
				break;
			case EVO_GP8:
				if (dmi->gpio.gp8 > -1 && dmi->func.gpio_get) {
					json_t *gpio = json_integer((dmi->func.gpio_get)(dmi->gpio.gp8));
					json_object_set_new(obj, "gp8", gpio);
				}
				break;
			case EVO_GP9:
				if (dmi->gpio.gp9 > -1 && dmi->func.gpio_get) {
					json_t *gpio = json_integer((dmi->func.gpio_get)(dmi->gpio.gp9));
					json_object_set_new(obj, "gp9", gpio);
				}
				break;

			case EVO_USR0:
				if (dmi->data.usr0) {
					json_object_set_new(obj, "usr0", json_string(dmi->data.usr0));
					dmi->data.usr0 = NULL;
				}
				break;
			case EVO_USR1:
				if (dmi->data.usr1) {
					json_object_set_new(obj, "usr1", json_string(dmi->data.usr1));
					dmi->data.usr1 = NULL;
				}
				break;
			case EVO_USR2:
				if (dmi->data.usr2) {
					json_object_set_new(obj, "usr2", json_string(dmi->data.usr2));
					dmi->data.usr2 = NULL;
				}
				break;
			case EVO_USR3:
				if (dmi->data.usr3) {
					json_object_set_new(obj, "usr3", json_string(dmi->data.usr3));
					dmi->data.usr3 = NULL;
				}
				break;
			case EVO_USR4:
				if (dmi->data.usr4) {
					json_object_set_new(obj, "usr4", json_string(dmi->data.usr4));
					dmi->data.usr4 = NULL;
				}
				break;
			case EVO_USR5:
				if (dmi->data.usr5) {
					json_object_set_new(obj, "usr5", json_string(dmi->data.usr5));
					dmi->data.usr5 = NULL;
				}
				break;

			case EVO_MAIN_VOLT:
				if (dmi->func.epwr_get) {
					char buff[6] = {0};
					int volt = (dmi->func.epwr_get)();
					snprintf(buff, sizeof(buff), "%6.3f", (float)volt / 1000);
					json_object_set_new(obj, "main_volt", json_string(buff));
				}
				break;
			case EVO_BATT_VOLT:
				if (dmi->func.batt_get) {
					char buff[6] = {0};
					int volt = (dmi->func.batt_get)();
					snprintf(buff, sizeof(buff), "%6.3f", (float)volt / 1000);
					json_object_set_new(obj, "batt_volt", json_string(buff));
				}
				break;

			case EVO_KEY_STAT:
				if (dmi->gpio.ign > -1 && dmi->func.gpio_get) {
					int stat = (dmi->func.gpio_get)(dmi->gpio.ign);
					json_object_set_new(obj, "key_stat", json_integer(stat));
				}
				break;
			case EVO_PWR_STAT:
				if (dmi->gpio.epwr > -1 && dmi->func.gpio_get) {
					int stat = (dmi->func.gpio_get)(dmi->gpio.epwr);
					json_object_set_new(obj, "pwr_stat", json_integer(stat));
				}
				break;

			case EVO_GPS_LAT:
				if (dmi->func.gps_get) {
					char lat[20] = {0};
					(dmi->func.gps_get)(&out);
					snprintf(lat, sizeof(lat), "%10.6f", out.lat);
					json_object_set_new(obj, "gps_lat", json_string(lat));
					gps_flag = 1;
				}
				break;
			case EVO_GPS_LON:
				if (dmi->func.gps_get) {
					char lon[20] = {0};
					if (!gps_flag) {
						(dmi->func.gps_get)(&out);
						gps_flag = 1;
					}
					snprintf(lon, sizeof(lon), "%10.6f", out.lon);
					json_object_set_new(obj, "gps_lon", json_string(lon));
				}
				break;
			case EVO_GPS_SPEED:
				if (dmi->func.gps_get) {
					int speed = 0;
					if (!gps_flag) {
						(dmi->func.gps_get)(&out);
						gps_flag = 1;
					}
					speed = out.speed;
					json_object_set_new(obj, "gps_speed", json_integer(speed));
				}
				break;
			case EVO_GPS_HDOP:
				if (dmi->func.gps_get) {
					char hdop[6] = {0};
					if (!gps_flag) {
						(dmi->func.gps_get)(&out);
						gps_flag = 1;
					}
					snprintf(hdop, sizeof(hdop), "%5.1f", out.hdop);
					json_object_set_new(obj, "gps_hdop", json_string(hdop));
					gps_flag = 0;
				}
				break;
			
			case EVO_LOG:
				{
					char logmsg[512] = {0};
					if (log) {
						snprintf(logmsg, sizeof(logmsg)-1, "%s", log);
						json_object_set_new(obj, "log", json_string(logmsg));
					}
				}
				break;

			case EVO_ERRNO:
				if (evo_errno) 
					json_object_set_new(obj, "errno", json_integer(evo_errno));
				break;

			default:
				break;
		}
	}
	
	return obj;
}
