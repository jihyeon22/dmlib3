#ifndef __DL_SYSLOG_API_H__
#define __DL_SYSLOG_API_H__

#if 0
#define LOG_EMERG   0   /* system is unusable */
#define LOG_ALERT   1   /* action must be taken immediately */
#define LOG_CRIT    2   /* critical conditions */
#define LOG_ERR     3   /* error conditions */
#define LOG_WARNING 4   /* warning conditions */
#define LOG_NOTICE  5   /* normal but significant condition */
#define LOG_INFO    6   /* informational */
#define LOG_DEBUG   7   /* debug-level messages */
#endif

#define SYSLOG_ADDR	"iot.mdstec.com"
//#define SYSLOG_ADDR	"m2m.mdstec.com"
#define SYSLOG_PORT	40514
//#define SYSLOG_PORT	30005

int init_syslog(void);
int deinit_syslog(void);

#endif
