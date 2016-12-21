#ifndef DM_REST_JSON_H
#define DM_REST_JSON_H

#include <jansson.h>

#include <dm/rest.h>

dm_res dm_enroll_event_json(dm_session *dms, const char* event, json_t* objs, json_t** result);

#endif // CP_REST_JSON_H
