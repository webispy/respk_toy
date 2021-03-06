#ifndef __NTOY_GPIO_H__
#define __NTOY_GPIO_H__

#include <time.h>

typedef void (*GpioEventCallback)(unsigned char value, struct timespec *ts);

int ntoy_setup_gpio(unsigned int pin, const char *dir, const char *edge);
int ntoy_open_gpio(unsigned int pin);

int ntoy_add_gpio_handler(unsigned int pin, GpioEventCallback func);

#endif
