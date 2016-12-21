#ifndef DM_REST_H
#define DM_REST_H

#include <stddef.h>

#include <dm/internal.h>

typedef struct dm_session dm_session; /**<Reference to a session */

enum _DM_HTTP_RESULT {   
	DM_HTTP_OK = 200,
	DM_HTTP_CREATED = 201,
	DM_HTTP_MULTI_STATUS  =  207,
	DM_HTTP_BAD_REQUEST = 400,
	DM_HTTP_UNAUTHORIZED = 401,
	DM_HTTP_PAYMENT_REQUIRED = 402,
	DM_HTTP_FORBIDDEN = 403,
	DM_HTTP_NOT_FOUND = 404,
	DM_HTTP_NOT_ALLOWED = 405,
	DM_HTTP_NOT_ACCEPTABLE = 406,
	DM_HTTP_SERVER_ERROR = 500,
	DM_HTTP_NOT_IMPLEMENTED = 501,
	DM_HTTP_BAD_GATEWAY = 502,
	DM_HTTP_SERVICE_UNAVAILABLE = 503 
};

typedef enum _DM_HTTP_RESULT DM_HTTP_RESULT;   /**<HTTP results returned by the server */


/**
 @return CP_OK on success, CP_FAIL otherwise.
*/
dm_res dm_rest_init();

/**
 @return CP_OK on success, CP_FAIL otherwise.
*/
dm_res dm_rest_shutdown();

/**
 Change the default base url.

 @param dms The session reference.
 @param url Http or https url string.
 @return DM_OK if the new base url is set correctly, CP_FAIL otherwise.
*/
dm_res dm_set_base_url(dm_session* dms, const char* url);

/**
 Get the current base url.

 @param dms The session reference.
 @return The base url.
*/
const char* dm_get_base_url(dm_session* dms);

/**
 Change the default timeout.

 @param dms The session reference.
 @param timeout Timeout in seconds.
 @return DM_OK if the new timeout is set correctly, CP_FAIL otherwise.
*/
dm_res dm_set_timeout(dm_session* dms, int timeout);

/**
 Get the current timeout.

 @param dms The session reference.
 @return The current timeout in seconds.
*/
int dm_get_timeout(dm_session* dms);

/**
 Return a human-readable string that describes the last error.

 @param dms The session reference.
 @return A human-readable string that describes the last error.
*/
const char* dm_get_last_err_string(dm_session* dms);

/**
 Return a human-readable string that describes the last error.

 @param dms The session reference.
 @return A human-readable string that describes the last error.
*/
DM_ERR_CODE dm_get_last_err_code(dm_session* dms);

/**
 Return a human-readable string that describes the last error.

 @param dms The session reference.
 @return A human-readable string that describes the last error.
*/
const char* dm_get_last_http_result_string(dm_session* dms);

/**
 Return a human-readable string that describes the last error.

 @param dms The session reference.
 @return A human-readable string that describes the last error.
*/
DM_HTTP_RESULT dm_get_last_http_result(dm_session* dms);


/**
 Initializes a HTTP persistent connection.

 @result The session reference, NULL if occurred an error.
*/
dm_session* dm_create_session(void);

/**
 Closes and cleans a session.

 @param dms The session reference.
 @return DM_OK if the session is correctly closed, CP_FAIL otherwise.
*/
dm_res dm_destroy_session(dm_session* dms);

/**
 enroll DM eventn.

 @param dms The session reference.
 @param body
 @param result
 @param result_lengrh
 @return DM_OK if the session is correctly closed, CP_FAIL otherwise.
*/
dm_res dm_enroll_event(dm_session *dms, const char* body, char** result, size_t* result_length);


#endif //DM_REST_H
