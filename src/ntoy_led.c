#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "ntoy_spi.h"
#include "ntoy_led.h"

/**
 * APA102 RGB LEDs Control
 * - https://cdn-shop.adafruit.com/datasheets/APA102.pdf
 *
 * +-------+-------+-------+-------+-------+
 * | 32bit | 32bit | 32bit | 32bit | 32bit |
 * +-------+-------+-------+-------+-------+
 * | Start | LED1  | LED2  | LED3  | End   |
 * | frame | frame | frame | frame | frame |
 * +-------+-------+-------+-------+-------+
 *
 * Start frame (32 bits)
 * +----------+----------+----------+----------+
 * | 00000000 | 00000000 | 00000000 | 00000000 |
 * +----------+----------+----------+----------+
 *
 * End frame (32 bits)
 * +----------+----------+----------+----------+
 * | 11111111 | 11111111 | 11111111 | 11111111 |
 * +----------+----------+----------+----------+
 *
 * LED frame (32 bits)
 * +----------+----------+----------+----------+
 * | 111xxxxx | bbbbbbbb | gggggggg | rrrrrrrr |
 * +----------+----------+----------+----------+
 *  xxxxx (5bits): Brightness 0 ~ 31
 *  bbbbbbbb (8bits): Blue 0 ~ 255
 *  gggggggg (8bits): Green 0 ~ 255
 *  rrrrrrrr (8bits): Red 0 ~ 255
 */

#define LED_START 0xE0

/* Start frame 4 bytes + (Number of LEDs * 4 bytes) + End frame 4 bytes */
static unsigned char led_buf[NTOY_NUM_LEDS * 4 + 8];
static unsigned char *first_led = led_buf + 4;

static void dump_led_buf(void)
{
	size_t i;
	char hex;

	putchar('0');
	putchar('x');

	for (i = 0; i < sizeof(led_buf); i++) {
		hex = (led_buf[i] & 0xF0) >> 4;
		if (hex < 10)
			putchar(hex + '0');
		else
			putchar(hex - 10 + 'A');

		hex = led_buf[i] & 0x0F;
		if (hex < 10)
			putchar(hex + '0');
		else
			putchar(hex - 10 + 'A');

		if (((i + 1) % 4) == 0 && (i + 1) != sizeof(led_buf)) {
			putchar(' ');
			putchar('|');
			putchar(' ');
			putchar('0');
			putchar('x');
		}
	}

	putchar('\n');
}

int ntoy_led_set_brightness(int num, int value)
{
	if (num < 0 || num >= NTOY_NUM_LEDS)
		return -1;

	*(first_led + num * 4) = LED_START | value;

	return 0;
}

int ntoy_led_set_r_g_b(int num, unsigned char r, unsigned char g,
		       unsigned char b)
{
	unsigned char *ptr = first_led + num * 4 + 1;

	if (num < 0 || num >= NTOY_NUM_LEDS)
		return -1;

	/* Blue / Green / Red */
	*(ptr++) = b;
	*(ptr++) = g;
	*(ptr++) = r;

	return 0;
}

int ntoy_led_set_rgb(int num, unsigned int hex)
{
	return ntoy_led_set_r_g_b(num, (0xFF0000 & hex) >> 16,
				  (0x00FF00 & hex) >> 8, 0xFF & hex);
}

void ntoy_led_reset(void)
{
	int i;
	unsigned char *ptr;

	memset(led_buf, 0, sizeof(led_buf));

	for (i = 0; i < NTOY_NUM_LEDS; i++)
		*(first_led + i * 4) = LED_START;

	ptr = first_led + NTOY_NUM_LEDS * 4;
	*(ptr++) = 0xFF;
	*(ptr++) = 0xFF;
	*(ptr++) = 0xFF;
	*(ptr++) = 0xFF;
}

void ntoy_led_clear(void)
{
	int i;
	unsigned char *ptr = led_buf;

	/* Clear R,G,B (without brightness) */
	for (i = 0; i < NTOY_NUM_LEDS; i++) {
		ptr = first_led + i * 4 + 1;
		*(ptr++) = 0x00;
		*(ptr++) = 0x00;
		*(ptr++) = 0x00;
	}
}

int ntoy_led_update(int fd)
{
	if (fd < 0)
		return -1;

	dump_led_buf();

	return ntoy_spi_write(fd, led_buf, sizeof(led_buf));
}

int ntoy_led_open(void)
{
	ntoy_led_reset();

	/**
	 * Bus: 0
	 * Device: 1
	 * Speed: 8000000 Hz
	 */
	return ntoy_spi_open(0, 1, 0x00, 0, 8000000);
}

int ntoy_led_close(int fd)
{
	if (fd < 0)
		return -1;

	close(fd);

	return 0;
}

#define DBUS_OBJECT_PATH "/LED"
#define DBUS_INTERFACE_NAME NTOY_DBUS_SERVICE ".LED"
#define DBUS_INTROSPECTION                                                     \
	"<node>"                                                               \
	"  <interface name='" DBUS_INTERFACE_NAME "'>"                         \
	"    <method name='RGB'>"                                              \
	"      <arg type='y' name='red'/>"                                     \
	"      <arg type='y' name='green'/>"                                   \
	"      <arg type='y' name='blue'/>"                                    \
	"    </method>"                                                        \
	"    <method name='Brightness'>"                                       \
	"      <arg type='y' name='value'/>"                                   \
	"    </method>"                                                        \
	"    <method name='Off'>"                                              \
	"    </method>"                                                        \
	"    <method name='On'>"                                               \
	"    </method>"                                                        \
	"  </interface>"                                                       \
	"</node>"

static int _led_fd;

static void _dbus_method_call(GDBusConnection *connection, const gchar *sender,
			      const gchar *object_path,
			      const gchar *interface_name,
			      const gchar *method_name, GVariant *parameters,
			      GDBusMethodInvocation *invocation,
			      gpointer user_data)
{
	int led_id = GPOINTER_TO_INT(user_data);
	printf("method: '%s' from '%s' (LED %d)\n", method_name, sender,
	       led_id);

	if (!g_strcmp0(method_name, "RGB")) {
		unsigned char r, g, b;

		g_variant_get(parameters, "(yyy)", &r, &g, &b);
		printf("LED-%d: R(%d), G(%d), B(%d)\n", led_id - 1, r, g, b);

		if (led_id == 0) {
			ntoy_led_set_r_g_b(0, r, g, b);
			ntoy_led_set_r_g_b(1, r, g, b);
			ntoy_led_set_r_g_b(2, r, g, b);
		} else {
			ntoy_led_set_r_g_b(led_id - 1, r, g, b);
		}

		ntoy_led_update(_led_fd);
	} else if (!g_strcmp0(method_name, "Brightness")) {
		unsigned char value;

		g_variant_get(parameters, "(y)", &value);
		printf("LED-%d: Brightness(%d)\n", led_id - 1, value);

		if (led_id == 0) {
			ntoy_led_set_brightness(0, value);
			ntoy_led_set_brightness(1, value);
			ntoy_led_set_brightness(2, value);
		} else {
			ntoy_led_set_brightness(led_id - 1, value);
		}

		ntoy_led_update(_led_fd);
	} else if (!g_strcmp0(method_name, "Off")) {
		if (led_id == 0) {
			ntoy_led_set_brightness(0, 0);
			ntoy_led_set_brightness(1, 0);
			ntoy_led_set_brightness(2, 0);
		} else {
			ntoy_led_set_brightness(led_id - 1, 0);
		}

		ntoy_led_update(_led_fd);
	} else if (!g_strcmp0(method_name, "On")) {
		if (led_id == 0) {
			ntoy_led_set_brightness(0, NTOY_LED_MAX_BRIGHTNESS);
			ntoy_led_set_brightness(1, NTOY_LED_MAX_BRIGHTNESS);
			ntoy_led_set_brightness(2, NTOY_LED_MAX_BRIGHTNESS);
		} else {
			ntoy_led_set_brightness(led_id - 1,
						NTOY_LED_MAX_BRIGHTNESS);
		}

		ntoy_led_update(_led_fd);
	}

	g_dbus_method_invocation_return_value(invocation, g_variant_new("()"));
}

static GDBusInterfaceVTable ops = {
	/* Method call only */
	.method_call = _dbus_method_call,
	.get_property = NULL,
	.set_property = NULL
};

static int _register_dbus_path(int led_id, GDBusConnection *conn)
{
	GDBusNodeInfo *intro;
	GError *e = NULL;
	char buf[255];

	intro = g_dbus_node_info_new_for_xml(DBUS_INTROSPECTION, &e);
	if (!intro) {
		fprintf(stderr, "g_dbus_node_info_new_for_xml() failed. %s\n",
			e->message);
		g_error_free(e);
		return -1;
	}

	if (led_id == 0)
		snprintf(buf, sizeof(buf), DBUS_OBJECT_PATH);
	else
		snprintf(buf, sizeof(buf), DBUS_OBJECT_PATH "/%d", led_id - 1);

	return g_dbus_connection_register_object(conn, buf,
						 intro->interfaces[0], &ops,
						 GINT_TO_POINTER(led_id), NULL,
						 &e);
}

int ntoy_led_dbus_init(GDBusConnection *conn)
{
	_led_fd = ntoy_led_open();
	if (_led_fd < 0)
		return -1;

	/* All LEDs */
	_register_dbus_path(0, conn);

	/* LED-0 */
	_register_dbus_path(1, conn);

	/* LED-1 */
	_register_dbus_path(2, conn);

	/* LED-2 */
	_register_dbus_path(3, conn);

	return 0;
}