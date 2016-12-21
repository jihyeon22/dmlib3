#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "at.h"

int at_get_phonenum(char *pnumver, int buf_len)
{
	int size;
	char *phone = getenv("DM_PHONE");

	if (phone) {
		size = strlen(phone);
		if (size < buf_len) {
			strncpy(pnumver, phone, size);
			pnumver[size] = 0x0;
		} else {
			strncpy(pnumver, phone, buf_len);
		}
	}
	return 0;
}
