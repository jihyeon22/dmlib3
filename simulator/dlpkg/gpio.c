#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "gpio.h"

static int _valid_gpio(const int gpio);

int gpio_get_value(const int gpio)
{
	int fd = 0, value = 0;
	char status[2] = {0};
	char buf[32] = {0};

	/* check valied gpio number */
	if(!_valid_gpio(gpio)) {
		fprintf(stderr, "Gpio[%d] is not valied\n", gpio);
		goto err;
	}

	sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);

	/* open gpio sysfs */
	fd = open(buf, O_RDWR);
	if(fd < 0) {
		fprintf(stderr, "Could not open gpio sysfs\n");
		goto err;
	}

	/* read gpio value */
	read(fd, status, 1);
	value = atoi(status);

	/* close gpio sysfs */
	close(fd);
	return value;

err:
	return -1;
}

int gpio_set_value(const int gpio, const int value)
{
	int fd = 0;
	char buf[32] = {0};

	/* check valied gpio number */
	if(!_valid_gpio(gpio)) {
		printf("gpio[%d] is not valied\n", gpio);
		return -1;
	}

	sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);

	fd = open(buf, O_RDWR);
	if(fd < 0) {
		printf("gpio sysfs open failed\n");
		return -1;
	}

	if(value) {
		write(fd, "1", 1);
	}
	else {
		write(fd, "0", 1);
	}

	close(fd);
	return 0;
}

int gpio_set_direction(const int gpio, gpioDirection_t direction)
{
	int fd = 0;
	char buf[32] = {0};

	/* check valied gpio number */
	if(!_valid_gpio(gpio)) {
		printf("gpio[%d] is not valied\n", gpio);
		return -1;
	}

	sprintf(buf, "/sys/class/gpio/gpio%d/direction", gpio);

	fd = open(buf, O_RDWR);
	if(fd < 0) {
		printf("gpio sysfs open failed\n");
		return -1;
	}

	if(direction) {
		write(fd, "out", sizeof("out"));
	}
	else {
		write(fd, "in", sizeof("in"));
	}

	close(fd);
	return 0;
}

static int _valid_gpio(const int gpio)
{
	return (gpio < GPIO_NO_MIN || gpio > GPIO_NO_MAX) ? 0 : 1;
}

