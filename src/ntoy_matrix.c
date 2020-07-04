#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "ntoy_spi.h"
#include "ntoy_matrix.h"

/**
 * MAX7219
 *
 * Serial-Data Format (16 Bits)
 *
 * |      8 Bits       |      8 Bits       |
 * +---------+---------+---------+---------+
 * | X X X X |   addr  |        DATA       |
 * +---------+---------+---------+---------+
 *                      MSB             LSB
 */

#define DBUS_OBJECT_PATH "/Matrix"
#define DBUS_INTERFACE_NAME NTOY_DBUS_SERVICE ".Matrix"
#define DBUS_INTROSPECTION                                                     \
	"<node>"                                                               \
	"  <interface name='" DBUS_INTERFACE_NAME "'>"                         \
	"    <method name='Clear'>"                                            \
	"    </method>"                                                        \
	"    <method name='SetPixel'>"                                         \
	"      <arg type='y' name='row'/>"                                     \
	"      <arg type='y' name='col'/>"                                     \
	"      <arg type='b' name='fill'/>"                                    \
	"    </method>"                                                        \
	"    <method name='Brightness'>"                                       \
	"      <arg type='y' name='value'/>"                                   \
	"    </method>"                                                        \
	"  </interface>"                                                       \
	"</node>"

/**
 * Register Address Map
 */
enum command {
	/**
	 * No Op
	 */
	COMMAND_NOOP = 0x0,

	/**
	 * Digit 0 ~ 7
	 */
	COMMAND_DIGIT_0 = 0x1,
	COMMAND_DIGIT_1 = 0x2,
	COMMAND_DIGIT_2 = 0x3,
	COMMAND_DIGIT_3 = 0x4,
	COMMAND_DIGIT_4 = 0x5,
	COMMAND_DIGIT_5 = 0x6,
	COMMAND_DIGIT_6 = 0x7,
	COMMAND_DIGIT_7 = 0x8,

	/**
	 * 0x00: No decode for digits 7-0
	 * 0x01, 0x0F, 0xFF: Segment display
	 */
	COMMAND_DECODE_MODE = 0x09,

	/**
	 * Brightness
	 * 0x00(Min) ~ 0x0F(Max)
	 */
	COMMAND_INTENSITY = 0x0A,

	/**
	 * 0x00: Display digit 0 only
	 * 0x01: digit 0 & 1
	 * 0x02: 0 1 2
	 * ...
	 * 0x07: 0 1 2 3 4 5 6 7
	 */
	COMMAND_SCAN_LIMIT = 0x0B,

	/**
	 * 0: Shutdown
	 * 1: Normal
	 */
	COMMAND_SHUTDOWN = 0x0C,

	/**
	 * 0: Normal operation
	 * 1: Display Test Mode
	 */
	COMMAND_TEST_MODE = 0x0F
};

static unsigned int bitmap[NTOY_NUM_ROWS];
static unsigned int dirty[NTOY_NUM_ROWS];

static int _write_max7219(unsigned char *buf, size_t length)
{
	if (ntoy_spi_write(0, buf, length) < 0) {
		printf("failed (channel=%d)\n", 0);
		return -1;
	}

	return 0;
}

static int _setup_command(enum command cmd, unsigned char value)
{
	unsigned char buf[NTOY_NUM_MATRIX * 2];
	int i;

	printf("Set %d command value to 0x%X\n", cmd, value);
	memset(buf, 0, sizeof(buf));
	for (i = 0; i < NTOY_NUM_MATRIX * 2; i += 2) {
		buf[i] = cmd;
		buf[i + 1] = value;
	}
	_write_max7219(buf, sizeof(buf));

	return 0;
}

static int m_write(int row, unsigned int cols_value)
{
	unsigned char buf[NTOY_NUM_MATRIX * 2];
	int i;
	int j;
	unsigned char value[NTOY_NUM_MATRIX];

	for (i = NTOY_NUM_MATRIX - 1; i >= 0; i--) {
		value[i] = cols_value & 0xFF;
		cols_value = cols_value >> 8;
	}

	j = NTOY_NUM_MATRIX - 1;
	memset(buf, 0, sizeof(buf));
	for (i = 0; i < NTOY_NUM_MATRIX * 2; i += 2, j--) {
		buf[i] = COMMAND_DIGIT_0 + row;
		buf[i + 1] = value[j];
	}

	_write_max7219(buf, sizeof(buf));

	return 0;
}

int ntoy_matrix_set_pixel(int row, int col, int value)
{
	int mid;
	int mask;

	if (row < 0 || col < 0 || row > NTOY_NUM_ROWS - 1 ||
	    col > NTOY_NUM_COLS - 1)
		return -1;

	mid = col / 8;

	/**
	 * Matrix 0 ~ 3 units to 32 bits integer bitmap
	 *
	 *  0 ( 0~ 7) => 0xFF000000 (0xFF << 24)
	 *  1 ( 8~15) => 0x00FF0000 (0xFF << 16)
	 *  2 (16~23) => 0x0000FF00 (0xFF << 8)
	 *  3 (24~31) => 0x000000FF (0xFF << 0)
	*/

	/* Map 0~7 to 1,2,4,8,16,32,64,128 */
	mask = 1 << (7 - (col % 8));

	/* Move to matched matrix */
	mask = mask << (24 - (mid * 8));

	if (value == 1)
		bitmap[row] = bitmap[row] | mask;
	else if (value == 0)
		bitmap[row] = bitmap[row] & ~mask;

	return 0;
}

int ntoy_matrix_set_row(int row, unsigned int value)
{
	if (row < 0 || row > NTOY_NUM_ROWS - 1)
		return -1;

	bitmap[row] = value;

	return 0;
}

static void _matrix_draw(int force_draw)
{
	int i;

	for (i = 0; i < NTOY_NUM_ROWS; i++) {
		if (force_draw == 0) {
			if (bitmap[i] == dirty[i])
				continue;
		}

		m_write(i, bitmap[i]);
		dirty[i] = bitmap[i];
	}
}

void ntoy_matrix_draw(void)
{
	_matrix_draw(0);
}

int ntoy_matrix_set_brightness(int value)
{
	if (value > NTOY_MATRIX_MAX_BRIGHTNESS ||
	    value < NTOY_MATRIX_MIN_BRIGHTNESS)
		return -1;

	_setup_command(COMMAND_INTENSITY, value);

	return 0;
}

void ntoy_matrix_clear(void)
{
	int i;

	for (i = 0; i < NTOY_NUM_ROWS; i++)
		bitmap[i] = 0;

	_matrix_draw(1);
}

static void _matrix_setup(void)
{
	_setup_command(COMMAND_SHUTDOWN, 0x1);
	_setup_command(COMMAND_SCAN_LIMIT, 0x7);
	_setup_command(COMMAND_INTENSITY, 0x1);
	_setup_command(COMMAND_DECODE_MODE, 0x0);
	_setup_command(COMMAND_TEST_MODE, 0x0);
}

int ntoy_matrix_open(void)
{
	/**
	 * Channel: 0
	 * Device: 1
	 * Speed: 1000000 Hz (= 1000 KHz = 1 MHz)
	 */
	if (ntoy_spi_open(0, 0x00, 8, 1000000) < 0)
		return -1;

	_matrix_setup();

	return 0;
}

int ntoy_matrix_close(void)
{
	ntoy_spi_close(0);

	return 0;
}

static void _dbus_method_call(GDBusConnection *connection, const gchar *sender,
			      const gchar *object_path,
			      const gchar *interface_name,
			      const gchar *method_name, GVariant *parameters,
			      GDBusMethodInvocation *invocation,
			      gpointer user_data)
{
	int matrix_id = GPOINTER_TO_INT(user_data);
	printf("method: '%s' from '%s' (Matrix %d)\n", method_name, sender,
	       matrix_id);

	if (!g_strcmp0(method_name, "Clear")) {
		ntoy_matrix_clear();
	} else if (!g_strcmp0(method_name, "SetPixel")) {
		unsigned char row, col, fill;

		g_variant_get(parameters, "(yyb)", &row, &col, &fill);
		printf("SetPixel: row=%d, col=%d, fill=%d\n", row, col, fill);

		ntoy_matrix_set_pixel(row, col, fill);
		ntoy_matrix_draw();
	} else if (!g_strcmp0(method_name, "Brightness")) {
		unsigned char value;

		g_variant_get(parameters, "(y)", &value);
		printf("Brightness(%d)\n", value);

		ntoy_matrix_set_brightness(value);
	}

	g_dbus_method_invocation_return_value(invocation, g_variant_new("()"));
}

static GDBusInterfaceVTable ops = {
	/* Method call only */
	.method_call = _dbus_method_call,
	.get_property = NULL,
	.set_property = NULL
};

static int _register_dbus_path(int matrix_id, GDBusConnection *conn)
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

	if (matrix_id == 0)
		snprintf(buf, sizeof(buf), DBUS_OBJECT_PATH);
	else
		snprintf(buf, sizeof(buf), DBUS_OBJECT_PATH "/%d",
			 matrix_id - 1);

	return g_dbus_connection_register_object(conn, buf,
						 intro->interfaces[0], &ops,
						 GINT_TO_POINTER(matrix_id),
						 NULL, &e);
}

int ntoy_matrix_dbus_init(GDBusConnection *conn)
{
	if (ntoy_matrix_open() < 0) {
		printf("matrix open failed\n");
		return -1;
	}

	ntoy_matrix_clear();

	/* All Matrix */
	_register_dbus_path(0, conn);

#if 0
	/* Matrix-0 */
	_register_dbus_path(1, conn);

	/* Matrix-1 */
	_register_dbus_path(2, conn);

	/* Matrix-2 */
	_register_dbus_path(3, conn);

	/* Matrix-3 */
	_register_dbus_path(4, conn);
#endif

	return 0;
}
