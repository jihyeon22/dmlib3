#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include <dm/dm.h>
#include <dm/rest_json.h>
#include <dm/http.h>
#include <dm/board_system.h>
#include <dm/constants.h>

static char* get_http_query(dm_session* dms, json_t* data) {
    CURL* curl = dms->curl;
    char* result = (char*) malloc(DM_MAX_URL_LENGTH* sizeof(char));
    if(!result) {
        dms->err = DM_ERR_OUT_OF_MEMORY;
        return NULL;
    }
    strcpy(result, "");
    if(!json_is_object(data)) {
        dms->err = DM_ERR_QUERY_IS_NOT_AN_OBJECT;
        return NULL;
    }
    const char* key;
    json_t* value;

    json_object_foreach(data, key, value) {
        char* ekey = curl_easy_escape(curl, key, strlen(key));
        strcat(result, ekey);
        free(ekey);
        strcat(result, "=");
        if(json_is_string(value)) {
            const char* s = json_string_value(value);
            char* es = curl_easy_escape(curl, s, strlen(s));
            strcat(result, es);
            free(es);
        } else if(json_is_number(value)) sprintf(result+strlen(result), "%g", json_number_value(value));
        else if(json_is_true(value)) strcat(result, DM_JSON_STRING_TRUE);
        else if(json_is_false(value)) strcat(result, DM_JSON_STRING_FALSE);
        //TODO else if(json_is_array(value)) {}
        else {
            dms->err = DM_ERR_QUERY_INVALID_TYPE;
            return NULL;
        }
        strcat(result, "&");
    }
    result[strlen(result)-1] = '\0';
    return result;
}

static void free_http_headers(char** h_array){
    if(h_array) {
        int i = 0;
        while(h_array[i]) {
            free(h_array[i]);
            i++;
        }
        free(h_array);
    }
}

static char** get_http_headers(dm_session* dms, json_t* headers){
    char**h_array = NULL;
    const char* key;
    json_t* value;
    int i = 0;
    h_array = malloc(sizeof (char* )*  (json_object_size(headers) + 1));
    if(!h_array){
        dms->err = DM_ERR_OUT_OF_MEMORY;
        return NULL;
    }
    json_object_foreach(headers, key, value) {
        if(!json_is_string(value)) {
            dms->err = DM_ERR_HEADERS_MUST_BE_STRING;
            free_http_headers(h_array);
            return NULL;
        }
        const char* v = json_string_value(value);
        h_array[i] = dm_concat(dms, 3, key, ": ", v);
        if(!h_array[i]) {
            free_http_headers(h_array);
            return NULL;
        }
        i++;
    }
    return h_array;
}

/**
  This function execute a generic http request

  @param dms The connection reference used for the request
  @param http_method Enum that indicate the desired action to be performed on the identified resource
  @param path Relative path of the requested resource
  @param headers JSON object converted to "<key>: <value>"
  @param query JSON object converted to "?<key1>=<value1>&<key2>=<value2>"
  @param body JSON object sent as body
  @param result A pointer such that *result will contain the dynamically allocated json object of the retrieved response body. The caller is responsible to free memory in *result.
  @return DM_SUCCESS if the request succeeds, DM_FAILED otherwise.
*/
static dm_res dm_request_json(dm_session* dms, dm_bool auth, DM_HTTP_METHOD http_method, const char* path, json_t* headers, json_t* query, json_t* body, json_t** result)
{
    dm_res dm_res = DM_FAIL;
    if(!dms) return dm_res;

    char**h_array = headers ? get_http_headers(dms, headers) : NULL;
    if(headers && !h_array) return dm_res;

    char* squery = query ? get_http_query(dms, query) : NULL;
    if(query && !squery) {
        free_http_headers(h_array);
        return dm_res;
    }

    char* sbody = body ? json_dumps(body, JSON_ENCODE_ANY) : NULL;
    if(body && !sbody) {
        free_http_headers(h_array);
        if(squery) free(squery);
        dms->err = DM_ERR_JSON_ENCODE;
        return dm_res;
    }

    size_t len = 0;
    char* sres = NULL;
    dm_res = dm_request_exec(dms, auth, http_method, path, h_array, squery, sbody, result ? &sres : NULL, result ? &len : NULL);

    if(dm_res == DM_OK && len && result) {
        json_error_t error;
        *result = json_loads(sres, JSON_DECODE_ANY, &error);
        if(!*result) {
            *result = json_object();
            json_object_set_new(*result, DM_JSON_ERR, json_string(error.text));
            json_object_set_new(*result, DM_JSON_BODY, json_string(sres));
            dms->err = DM_ERR_JSON_PARSE;
        }
        free(sres);
    } else if(result) {
        *result = NULL;
    }

    free_http_headers(h_array);
    if(squery) free(squery);
    if(sbody) free(sbody);
    return dm_res;
}

dm_res dm_enroll_event_json(dm_session* dms, const char* event, json_t* objs, json_t** result) {
    if(!dms) return DM_FAIL;
    if(!event || !objs) SET_ERROR_AND_RETURN(dms, DM_ERR_INVALID_PARAMETER);

	json_t* body = json_array();
    if(!body) SET_ERROR_AND_RETURN(dms, DM_ERR_OUT_OF_MEMORY);
	if (objs)
		json_object_set_new(objs, EVENT, json_string(event));

	json_array_append_new(body, objs);

    char* dumps = NULL;
    if(body) dumps = json_dumps(body, JSON_INDENT(3) | JSON_ENCODE_ANY );
        fprintf(stderr, "Event Request: %s\n", dumps);
    if(dumps) free(dumps);

	dm_info* dmi = dm_get_info();

    dm_res res = dm_request_json(dms, DM_FALSE, DM_HTTP_POST, dmi->session.path_event, NULL, NULL, body, result);
	
	if (result) dumps = json_dumps(*result, JSON_INDENT(3) | JSON_ENCODE_ANY );
	if (res == DM_FAIL) {
		fprintf(stderr, "Event Response: [ERROR-> %s] [HTTP_RES %d] %s\n", dm_get_last_err_string(dms), dm_get_last_http_result(dms), dumps);
	} else {
		fprintf(stderr, "Event Response: %s\n", dumps);
	}
	if(dumps) free(dumps);

	json_decref(body);
    return res;
}
