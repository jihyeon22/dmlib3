#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include <dm/rest.h>
#include <dm/http.h>
#include <dm/board_system.h>
#include <dm/constants.h>
#include <dm/safe.h>

dm_res dm_rest_init() {
    return curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK ? DM_OK : DM_FAIL;
}

dm_res dm_rest_shutdown() {
    curl_global_cleanup();
    return DM_OK;
}

dm_res dm_set_base_url(dm_session* dms, const char* url) {
    if (!dms || !url) 
		SET_ERROR_AND_RETURN(dms, DM_ERR_INVALID_PARAMETER);

    int http = !strncmp(url, DM_HTTP_STR, LIT_STR_LEN(DM_HTTP_STR));
    if (http) {
        safefree(dms->base_url);
        dms->base_url = url[strlen(url)-1] == '/' ? dm_concat(dms, 1, url) : dm_concat(dms, 2, url, "/");
        return dms->base_url ? DM_OK : DM_FAIL;
    } else SET_ERROR_AND_RETURN(dms, DM_ERR_INVALID_PARAMETER);
}

const char* dm_get_base_url(dm_session* dms) {
    if(!dms) 
		return NULL;

    return dms->base_url ? dms->base_url : DM_URL;
}

dm_res dm_set_timeout(dm_session* dms, int timeout) {
    if(dms && timeout >= 0) {
        dms->timeout = timeout ? timeout : DM_TIMEOUT;
        return DM_OK;
    }
    
	return DM_FAIL;
}

int dm_get_timeout(dm_session* dms) {
    return dms ? dms->timeout : -1;
}

const char* dm_get_last_err_string(dm_session* dms) {
    switch(dms->err) {
        case DM_ERR_INTERNAL_ERROR:	return "Internal Library Error";
        case DM_ERR_OUT_OF_MEMORY: return "Out of memory";
        case DM_ERR_INVALID_SESSION: return "Invalid session";
        case DM_ERR_QUERY_IS_NOT_AN_OBJECT: return "Query is not an object";
        case DM_ERR_QUERY_INVALID_TYPE: return "Query contain invalid type";
        case DM_ERR_HEADERS_MUST_BE_STRING: return "Header value must be a string";
        case DM_ERR_INVALID_PARAMETER: return "Invalid parameter";
        case DM_ERR_INVALID_LOGIN: return "Invalid login";
        case DM_ERR_JSON_PARSE: return "JSON parse error";
        case DM_ERR_JSON_ENCODE: return "JSON encode error";
        case DM_ERR_HTTP: return "HTTP error";
        default: return NULL;
   }
}

DM_ERR_CODE dm_get_last_err_code(dm_session* dms){
    return dms->err;
}

const char* dm_get_last_http_result_string(dm_session* dms){
    switch(dms->http_res) {
      case DM_HTTP_OK:			return "Ok";
      case DM_HTTP_CREATED:		return "Created";
      case DM_HTTP_MULTI_STATUS:		return "Multi-Status";
      case DM_HTTP_BAD_REQUEST:		return "Bad Request";
      case DM_HTTP_UNAUTHORIZED:		return "Unauthorized";
      case DM_HTTP_PAYMENT_REQUIRED:	return "Payment Required";
      case DM_HTTP_FORBIDDEN:		return "Forbidden";
      case DM_HTTP_NOT_FOUND:		return "Not found";
      case DM_HTTP_NOT_ALLOWED:		return "Method Not Allowed";
      case DM_HTTP_NOT_ACCEPTABLE:	return "Not Acceptable";
      case DM_HTTP_SERVER_ERROR:		return "Internal Server Error";
      case DM_HTTP_NOT_IMPLEMENTED:	return "Not Implemented";
      case DM_HTTP_BAD_GATEWAY:		return "Bad Gateway";
      case DM_HTTP_SERVICE_UNAVAILABLE:	return "Service Unavailable";
      default: return NULL;
    }
}

DM_HTTP_RESULT dm_get_last_http_result(dm_session* dms)
{
    return dms->http_res;
}

dm_session* dm_create_session(void) 
{
	dm_session* dms = malloc(sizeof(struct dm_session));
	if(!dms) return NULL;

	dms->curl = curl_easy_init();
	if(!dms->curl) {
		free(dms);
		return NULL;
	}
	dms->timeout = -1;
	dms->base_url = NULL;
	dms->http_res = 0;
	dms->err = 0;

	return dms;
}

dm_res dm_destroy_session(dm_session* dms) 
{
    if(!dms) return DM_FAIL;

    curl_easy_cleanup(dms->curl);
    free(dms->base_url);
    free(dms);

    return DM_OK;
}

dm_bool dm_extract_string_from_json(char** json, const char* key, char* res, const int n) {
    char*  start = strstr(*json, key);
    if(!start) return DM_FALSE;
    start = strchr(start,':');
    if(!start) return DM_FALSE;
    start = strchr(start,'\"');
    if(!start) return DM_FALSE;
    start++;
    char* end = strchr(start,'\"');
    if(!end) return DM_FALSE;

    int len = (int) (end-start);
    if(len>n) return DM_FALSE;
    strncpy(res, start, (size_t) len);
    return DM_TRUE;
}

dm_res dm_enroll_event(dm_session *dms, const char* body, char** result, size_t* result_length) {
    if(!dms || !body) SET_ERROR_AND_RETURN(dms, DM_ERR_INVALID_PARAMETER);
    dm_res dm_res = dm_request_exec(dms, DM_TRUE, DM_HTTP_POST, PATH_EVENT, NULL, NULL, body, result, result_length);
    return dm_res;
}
