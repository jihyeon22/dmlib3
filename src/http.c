#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dm/http.h>
#include <dm/board_system.h>
#include <dm/constants.h>

char* dm_concat(dm_session *dms, int num, ... ) {
    if(!dms) return NULL;
    va_list arguments;
    int sum = 0;
    va_start ( arguments, num );
    int x;
    for(x = 0; x < num; x++) {
        char* k = va_arg(arguments, char*);
        if(!k) {
            va_end(arguments);
            return NULL;
        }
        sum += strlen(k);
    }
    va_end(arguments);

    char* s = (char*) malloc((sum+1)* sizeof(char));
    if(!s) {
        if(dms) dms->err = DM_ERR_OUT_OF_MEMORY;
        return s;
    }
    va_start(arguments, num);
    strcpy(s, va_arg(arguments, char*));

    for(x = 1; x < num; x++) {
        strcat(s, va_arg(arguments, char*));
    }
    va_end(arguments);

    return s;
}

//static const char* DM_HTTP_METHODS[] = { "GET", "POST", "PUT", "DELETE", "PATCH" };

struct _dm_req_buffer {
  char* body;
  size_t len;
  size_t offset;
  dm_session *dms;
};

typedef struct _dm_req_buffer dm_req_buffer;

static size_t writefunc(void* ptr, size_t size, size_t nmemb, dm_req_buffer* b) {
    if(!b->body) {
        double sz;
        b->len = 0;
        CURLcode res = curl_easy_getinfo(b->dms->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &sz);
        if(res == CURLE_OK && sz != -1) {
            b->offset = 0;
            b->len = (size_t) sz;
            if(sz > 0){
                b->body = (char*) malloc((b->len+1)*sizeof(char));
                b->body[b->len] = '\0';
            }
        } else {
            b->dms->err = DM_ERR_INVALID_CONTENT_LENGTH;
            return 0;//If that amount differs from the amount passed to your function, it'll signal an error to the library
        }
    }

    size_t tot = size * nmemb;
    size_t off = b->offset + tot;
    if(off > b->len) {
        b->dms->err = DM_ERR_INVALID_CONTENT_LENGTH;
        b->len = 0;
    } else {
        memcpy(b->body+b->offset, ptr, tot);
        b->offset = off;
    }
    return tot;//Return the number of bytes actually taken care of.
}



dm_res dm_request_exec(dm_session* dms, dm_bool auth, DM_HTTP_METHOD http_method, const char* path, char* headers[], const char* query, const char* body, char** result, size_t* result_length)
{
	/* Yoonki: using remove warning 20150707 */
	(void) auth;
	(void) http_method;

	if (!dms) return DM_FAIL;
	if (!path) return DM_FAIL;

    CURLcode curl_res;
	struct curl_slist* chunk = NULL;
    CURL* curl = NULL;
    dms->http_res = 0;
    dms->err = 0;
    curl = dms->curl;

	/* Timeout */
    curl_easy_setopt(dms->curl, CURLOPT_CONNECTTIMEOUT, dms->timeout);
    curl_easy_setopt(dms->curl, CURLOPT_TIMEOUT, dms->timeout * 2);
	
	/* Ignore Signal */
    curl_easy_setopt(dms->curl, CURLOPT_NOSIGNAL, 1);
	
	char* full_url = query ? dm_concat(dms, 3, path, "?", query) : dm_concat(dms, 2, dms->base_url, path);
    if(!full_url) {
        curl_easy_reset(curl);
        if(chunk) curl_slist_free_all(chunk);
        return DM_FAIL;
    }

    curl_easy_setopt(curl, CURLOPT_URL, full_url);
    free(full_url);

    /* is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    dm_req_buffer b;
    if(result) {
        b.body = NULL;
        b.dms = dms;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &b);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    }

    if(headers) {
        int i = 0 ;
        while(headers[i] != 0) {
            chunk = curl_slist_append(chunk, headers[i]);
            i++;
        }
    }
    chunk = curl_slist_append(chunk, CONTENT_TYPE_JSON);

    if(chunk) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

    if(body) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

    /* Perform the request, res will get the return code */
    curl_res = curl_easy_perform(curl);
    /* Check for errors */
    if(curl_res != CURLE_OK) {
        const char *error = curl_easy_strerror(curl_res);
        int resl = strlen(error);
        if(result_length) *result_length = resl;
        if(result) {
            *result = (char*) malloc(resl * sizeof(char));
            if(*result) strcpy(*result, error);
        }
    } else {
        if(result) *result = b.body;
        if(result_length) {
            double sz;
            CURLcode res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &sz);
            if(res == CURLE_OK) *result_length = (size_t) sz;
        }
    }
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &dms->http_res);

	/* always cleanup */
	
    curl_easy_reset(curl);
    if(chunk) curl_slist_free_all(chunk);
	if (dms->http_res == DM_HTTP_OK || dms->http_res == DM_HTTP_CREATED) {
		return DM_OK;
	} else if (dms->http_res == 0) {
		dms->err = DM_ERR_INVALID_SESSION;
		return DM_FAIL;
	} else {
		dms->err = DM_ERR_HTTP;
		return DM_FAIL;
	}
}

