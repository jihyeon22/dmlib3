#ifndef _DM_INTERNAL_H_
#define _DM_INTERNAL_H_

#define DM_OK 0
#define DM_FAIL 1
typedef int dm_res; /**<An integer representing the result of a request */

typedef int dm_bool; /**<Library internal booleans */
#define DM_TRUE 1
#define DM_FALSE 0

/**
 Number of milliseconds in GMT since the time of Epoch
*/
typedef double dm_time;

enum _DM_ERR_CODE { 
	DM_ERR_INTERNAL_ERROR = -1,
	DM_ERR_OUT_OF_MEMORY = -2,
	DM_ERR_INVALID_SESSION = -3,
	DM_ERR_QUERY_IS_NOT_AN_OBJECT = -4,
	DM_ERR_QUERY_INVALID_TYPE = -5,
	DM_ERR_HEADERS_MUST_BE_STRING = -6,
	DM_ERR_INVALID_PARAMETER = -7,
	DM_ERR_INVALID_LOGIN = -8,
	DM_ERR_JSON_PARSE = -9,
	DM_ERR_JSON_ENCODE = -10,
	DM_ERR_INVALID_CONTENT_LENGTH = -11,
	DM_ERR_HTTP = -12,
	DM_ERR_INVALID_EVENT = -13,
	DM_ERR_UPDATE = -14,
	DM_ERR_UPDATE_INFO = -15,
};

typedef enum _DM_ERR_CODE DM_ERR_CODE; /**<Library internal error codes */

#endif
