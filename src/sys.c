#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <dm/board_system.h>
#include <dm/internal.h>

// NORETURN
void dm_sys_poweroff(int delay)
{
	if (delay > 0)
		sleep(delay);
	
	printf("[dmlib] %s : poweroff. bye~~~\n", __func__);

	do {
		system(CMD_POWER_OFF);
		sleep(5);
	} while (DM_TRUE);
}
