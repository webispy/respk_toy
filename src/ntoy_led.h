#ifndef __NTOY_LED_H__
#define __NTOY_LED_H__

#include <glib.h>
#include <gio/gio.h>

#define NTOY_NUM_LEDS 3
#define NTOY_LED_MIN_BRIGHTNESS 0
#define NTOY_LED_MAX_BRIGHTNESS 31

int ntoy_led_set_brightness(int num, int value);
int ntoy_led_set_r_g_b(int num, unsigned char r, unsigned char g, unsigned char b);
int ntoy_led_set_rgb(int num, unsigned int hex);

void ntoy_led_reset(void);
void ntoy_led_clear(void);

int ntoy_led_update(void);
int ntoy_led_open(int channel);
int ntoy_led_close(void);

int ntoy_led_dbus_init(GDBusConnection *conn);

#endif
