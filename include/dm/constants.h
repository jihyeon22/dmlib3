#ifndef DM_CONSTANTS_H
#define DM_CONSTANTS_H

#define DM_URL "http://dm.mdstec.com"
#define DM_MAX_URL_LENGTH 2048
#define DM_TIMEOUT 30
//#define DM_INI	"/system/bin/dm.ini"			//define.h로 이동
//#define DM_SERVICE_PACKAGE "/system/bin/PACKAGE"	//define.h로 이동

#define LIT_STR_LEN(x) (sizeof(x) - 1)

#define DM_HTTP_STR "http://"
#define DM_HTTPS_STR "https://"

//ERROR
#define DM_JSON_ERR "err"
#define DM_JSON_BODY "body"

#define DM_JSON_STRING_TRUE "true"
#define DM_JSON_STRING_FALSE "false"

#define PLUG_AUTH_HEADER "X-Plug-Auth: "
#define PLUG_ID_HEADER "X-Plug-Id: "
#define PLUG_EMAIL_HEADER "X-Plug-Email: "
#define PLUG_MASTER_HEADER "X-Plug-Master: "

#define CONTENT_TYPE_JSON "Content-type: application/json"

#define PATH_EVENT "api/event"

#define EVENT		"event"


#endif // CP_CONSTANTS_H
