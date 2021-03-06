#ifndef __DL_GPIO_H__
#define __DL_GPIO_H__

#define GPIO_NO_MIN		9
#define GPIO_NO_MAX	66
#define GPIO_SRC_NUM_POWER	66
#define GPIO_SRC_NUM_IGNITION	10

typedef enum gpioDirection gpioDirection_t;
enum gpioDirection
{
	eGpioInput = 0,
	eGpioOutput
};

int gpio_get_value(const int gpio);
int gpio_set_value(const int gpio, const int value);
int gpio_set_direction(const int gpio, gpioDirection_t direction);

#endif
