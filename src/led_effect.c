#include <stdio.h>
#include <unistd.h>

#include "ntoy_led.h"

static void br_loop(int min_br, int max_br, int step)
{
	int i;

	for (i = min_br; i != max_br; i += step) {
		printf("brightness = %d\n", i);
		ntoy_led_set_brightness(0, i);
		ntoy_led_set_brightness(1, i);
		ntoy_led_set_brightness(2, i);
		ntoy_led_update();

		usleep(50 * 1000);
	}
}

static void rotate_loop(int count, unsigned int r, unsigned int g,
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
		ntoy_led_update();

		num++;
		if (num >= NTOY_NUM_LEDS)
			num = 0;

		usleep(50 * 1000);
	}
}

static void gradient_loop(void)
{
	int r, g, b;
	double value;
	double i;

	printf("\nGradient loop\n");

	for (i = 0.0; i <= 1.0; i += 0.01) {
		if (i > 0.5) {
			value = i - 0.5;
			r = 0;
			g = (int)((1 - 2 * value) * 255);
			b = (int)(2 * value * 255);
		} else if (i <= 0.5) {
			value = i;
			r = (int)((1 - 2 * value) * 255);
			g = (int)(2 * value * 255);
			b = 0;
		}

		printf("seq=%f, r=%d, g=%d, b=%d\n", i, r, g, b);

		ntoy_led_set_r_g_b(0, r, g, b);
		ntoy_led_set_r_g_b(1, r, g, b);
		ntoy_led_set_r_g_b(2, r, g, b);

		ntoy_led_update();

		usleep(50 * 1000);
	}
}

static void simple_test(void)
{
	printf("\nSimple test\n");

	printf("\nLED-1 Red\n");
	ntoy_led_clear();
	ntoy_led_set_rgb(0, 0xFF0000);
	ntoy_led_update();
	sleep(1);

	printf("\nLED-2 Green\n");
	ntoy_led_clear();
	ntoy_led_set_rgb(1, 0x00FF00);
	ntoy_led_update();
	sleep(1);

	printf("\nLED-3 Blue\n");
	ntoy_led_clear();
	ntoy_led_set_rgb(2, 0x0000FF);
	ntoy_led_update();
	sleep(1);
}

static void simple_sequence(void)
{
	printf("\nSimple LED sequence\n");
	rotate_loop(20, 0, 0, 255, NTOY_LED_MAX_BRIGHTNESS);

	printf("\nWHITE brightness loop\n");
	ntoy_led_set_rgb(0, 0xFFFFFF);
	ntoy_led_set_rgb(1, 0xFFFFFF);
	ntoy_led_set_rgb(2, 0xFFFFFF);

	br_loop(0, NTOY_LED_MAX_BRIGHTNESS, 1);
	br_loop(NTOY_LED_MAX_BRIGHTNESS, NTOY_LED_MIN_BRIGHTNESS - 1, -1);

	printf("\nRED brightness loop\n");
	ntoy_led_set_rgb(0, 0xFF0000);
	ntoy_led_set_rgb(1, 0xFF0000);
	ntoy_led_set_rgb(2, 0xFF0000);

	br_loop(0, NTOY_LED_MAX_BRIGHTNESS, 1);
	br_loop(NTOY_LED_MAX_BRIGHTNESS, NTOY_LED_MIN_BRIGHTNESS - 1, -1);

	printf("\nGREEN brightness loop\n");
	ntoy_led_set_rgb(0, 0x00FF00);
	ntoy_led_set_rgb(1, 0x00FF00);
	ntoy_led_set_rgb(2, 0x00FF00);

	br_loop(0, NTOY_LED_MAX_BRIGHTNESS, 1);
	br_loop(NTOY_LED_MAX_BRIGHTNESS, NTOY_LED_MIN_BRIGHTNESS - 1, -1);

	printf("\nBLUE brightness loop\n");
	ntoy_led_set_rgb(0, 0x0000FF);
	ntoy_led_set_rgb(1, 0x0000FF);
	ntoy_led_set_rgb(2, 0x0000FF);

	br_loop(0, NTOY_LED_MAX_BRIGHTNESS, 1);
	br_loop(NTOY_LED_MAX_BRIGHTNESS, NTOY_LED_MIN_BRIGHTNESS - 1, -1);
}

int main(int argc, char *argv[])
{
	printf("Start\n");

	if (ntoy_led_open(1) < 0) {
		printf("fail\n");
		return -1;
	}

	if (argc == 2) {
		printf("Clear\n");
		ntoy_led_set_brightness(0, NTOY_LED_MAX_BRIGHTNESS);
		ntoy_led_set_brightness(1, NTOY_LED_MAX_BRIGHTNESS);
		ntoy_led_set_brightness(2, NTOY_LED_MAX_BRIGHTNESS);
		ntoy_led_clear();
		ntoy_led_update();

		if (argv[1][0] == '0')
			simple_test();
		else if (argv[1][0] == '1')
			simple_sequence();
		else if (argv[1][0] == '2')
			gradient_loop();
	}

	printf("\nClose with clear\n");
	ntoy_led_reset();
	ntoy_led_update();
	ntoy_led_close();

	return 0;
}
