#include <stdio.h>
#include <unistd.h>

#include "ntoy_led.h"

static void br_loop(int fd, int min_br, int max_br, int step)
{
	int i;

	for (i = min_br; i != max_br; i += step) {
		printf("brightness = %d\n", i);
		ntoy_led_set_brightness(0, i);
		ntoy_led_set_brightness(1, i);
		ntoy_led_set_brightness(2, i);
		ntoy_led_update(fd);

		usleep(50 * 1000);
	}
}

static void rotate_loop(int fd, int count, unsigned int r, unsigned int g,
		 unsigned int b, unsigned int br)
{
	int i;
	int num = 0;

	for (i = 0; i < NTOY_NUM_LEDS; i++)
		ntoy_led_set_brightness(i, br);

	for (i = 0; i <= count; i++) {
		ntoy_led_clear();

		printf("num = %d\n", num);
		ntoy_led_set_r_g_b(num, r, g, b);
		ntoy_led_update(fd);

		num++;
		if (num >= NTOY_NUM_LEDS)
			num = 0;

		usleep(50 * 1000);
	}
}

int main(int argc, char *argv[])
{
	int fd;

	printf("Start\n");

	fd = ntoy_led_open();
	if (fd < 0) {
		printf("fail\n");
		return -1;
	}

#if 0
	printf("Clear\n");
	ntoy_led_set_brightness(0, NTOY_LED_MAX_BRIGHTNESS);
	ntoy_led_set_brightness(1, NTOY_LED_MAX_BRIGHTNESS);
	ntoy_led_set_brightness(2, NTOY_LED_MAX_BRIGHTNESS);
	ntoy_led_clear();
	ntoy_led_update(fd);
	sleep(1);

	printf("\nLED-1 Red\n");
	ntoy_led_clear();
	ntoy_led_set_rgb(0, 0xFF0000);
	ntoy_led_update(fd);
	sleep(1);

	printf("\nLED-2 Green\n");
	ntoy_led_clear();
	ntoy_led_set_rgb(1, 0x00FF00);
	ntoy_led_update(fd);
	sleep(1);

	printf("\nLED-3 Blue\n");
	ntoy_led_clear();
	ntoy_led_set_rgb(2, 0x0000FF);
	ntoy_led_update(fd);
	sleep(1);

#else
	printf("\nLED sequence\n");
	rotate_loop(fd, 20, 0, 0, 255, NTOY_LED_MAX_BRIGHTNESS);

	printf("\nWHITE brightness loop\n");
	ntoy_led_set_rgb(0, 0xFFFFFF);
	ntoy_led_set_rgb(1, 0xFFFFFF);
	ntoy_led_set_rgb(2, 0xFFFFFF);

	br_loop(fd, 0, NTOY_LED_MAX_BRIGHTNESS, 1);
	br_loop(fd, NTOY_LED_MAX_BRIGHTNESS, NTOY_LED_MIN_BRIGHTNESS - 1, -1);

	printf("\nRED brightness loop\n");
	ntoy_led_set_rgb(0, 0xFF0000);
	ntoy_led_set_rgb(1, 0xFF0000);
	ntoy_led_set_rgb(2, 0xFF0000);

	br_loop(fd, 0, NTOY_LED_MAX_BRIGHTNESS, 1);
	br_loop(fd, NTOY_LED_MAX_BRIGHTNESS, NTOY_LED_MIN_BRIGHTNESS - 1, -1);

	printf("\nGREEN brightness loop\n");
	ntoy_led_set_rgb(0, 0x00FF00);
	ntoy_led_set_rgb(1, 0x00FF00);
	ntoy_led_set_rgb(2, 0x00FF00);

	br_loop(fd, 0, NTOY_LED_MAX_BRIGHTNESS, 1);
	br_loop(fd, NTOY_LED_MAX_BRIGHTNESS, NTOY_LED_MIN_BRIGHTNESS - 1, -1);

	printf("\nBLUE brightness loop\n");
	ntoy_led_set_rgb(0, 0x0000FF);
	ntoy_led_set_rgb(1, 0x0000FF);
	ntoy_led_set_rgb(2, 0x0000FF);

	br_loop(fd, 0, NTOY_LED_MAX_BRIGHTNESS, 1);
	br_loop(fd, NTOY_LED_MAX_BRIGHTNESS, NTOY_LED_MIN_BRIGHTNESS - 1, -1);
#endif

	printf("\nClose with clear\n");
	ntoy_led_reset();
	ntoy_led_update(fd);
	ntoy_led_close(fd);

	return 0;
}
