#ifndef DM_INTERNALS_H
#define DM_INTERNALS_H

#include <stdarg.h>
#include <curl/curl.h>

#include <dm/rest.h>

/**
 * Used  HTTP methods
 */

enum _DM_HTTP_METHOD { DM_HTTP_GET, DM_HTTP_POST, DM_HTTP_PUT, DM_HTTP_DELETE, DM_HTTP_PATCH };
typedef enum _DM_HTTP_METHOD DM_HTTP_METHOD;

#define SET_ERROR_AND_RETURN(dms, x) { if(dms) dms->err = x; return DM_FAIL; }

/**
 * Data structure to handle a request session
 */

struct dm_session {
   CURL* curl;
   char* base_url;
   char* path_event;
   int timeout;
//   char* id;				// TODO: support next version
//   char* auth;			// TODO: support next version
//   dm_bool is_master;		// TODO: support next version
   DM_HTTP_RESULT http_res;
   DM_ERR_CODE err;
//   dm_bool verify_ssl;	// TODO: support next version
//   char* ca;				// TODO: support next version
};


/**
 * Utility function for string concatenation
 */
char* dm_concat(dm_session* dms, int num, ... );

/**
 Execute a generic http request.

 @param dms The session reference.
 @param auth Check if auth is present.
 @param http_method Enum that indicate the desired action to be performed on the identified resource.
 @param path Relative path of the requested resource.
 @param headers If not NULL, is an array of string, last element must be NULL.
 @param query If not NULL, the string is append to the path after a '?' character.
 @param result If not NULL, then *result will contain the dynamically allocated json string of the retrieved response body. The caller is responsible to free memory in *result.
 @param result_length The length of the string stored in *result.
  @return DM_SUCCESS if the request succeeds, DM_FAILED otherwise.
*/
dm_res dm_request_exec(dm_session* dms, dm_bool auth, DM_HTTP_METHOD http_method, const char* path, char* headers[], const char* query, const char* body, char** result, size_t* result_length);

#endif // DM_INTERNALS_H
