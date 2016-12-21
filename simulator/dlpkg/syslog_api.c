#include <syslog.h>
#include <stdlib.h>

#include "syslog_api.h"

int init_syslog(void)
{
	char buf_cmd[255] = {0};
	
	setlogmask (LOG_UPTO (LOG_CRIT));
	openlog ("MDS", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
	
	snprintf(buf_cmd, sizeof(buf_cmd)-1, "syslogd -R %s:%d", SYSLOG_ADDR, SYSLOG_PORT);
	
	system("killall syslogd");
	sleep(1);
	system(buf_cmd);
	
	return 0;
}

int deinit_syslog(void)
{
	closelog();
	system("killall syslogd");
}

