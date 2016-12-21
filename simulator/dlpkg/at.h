#define AT_DEV_FILE "/dev/atcmd"
#define AT_MAX_BUFF_SIZE    512

#define AT_LEN_PHONENUM   11
#define AT_LEN_PHONENUM_BUFF	(AT_LEN_PHONENUM + 1)

#define AT_MAX_RETRY_WRITE	3
#define AT_MAX_WAIT_READ_SEC	1

int at_get_phonenum(char *pnumber, int buf_len);
