#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "ntoy_gpio.h"
#include "ntoy_button.h"

#define BUTTON_GPIO_PIN 17

#define BUTTON_PRESSED 0
#define BUTTON_RELEASED 1

#define DBUS_OBJECT_PATH "/Button"
#define DBUS_INTERFACE_NAME NTOY_DBUS_SERVICE ".Button"
#define DBUS_INTROSPECTION                                                     \
	"<node>"                                                               \
	"  <interface name='" DBUS_INTERFACE_NAME "'>"                         \
	"    <signal name='Pressed'></signal>"                                 \
	"    <signal name='Released'></signal>"                                \
	"    <signal name='Clicked'></signal>"                                 \
	"    <signal name='DoubleClicked'></signal>"                           \
	"    <signal name='LongPress'></signal>"                               \
	"  </interface>"                                                       \
	"</node>"

static struct timespec _prev_ts;
static guint _timer_src;
static gboolean _pressed;
static gboolean _dbl_click;
static GDBusConnection *_conn;

static long diff_in_us(struct timespec *t1, struct timespec *t2)
{
	struct timespec diff;

	if (t2->tv_nsec - t1->tv_nsec < 0) {
		diff.tv_sec = t2->tv_sec - t1->tv_sec - 1;
		diff.tv_nsec = t2->tv_nsec - t1->tv_nsec + 1000000000;
	} else {
		diff.tv_sec = t2->tv_sec - t1->tv_sec;
		diff.tv_nsec = t2->tv_nsec - t1->tv_nsec;
	}

	return (diff.tv_sec * 1000.0 + diff.tv_nsec / 1000000.0);
}

static gboolean onLongpress(gpointer userdata)
{
	GError *e = NULL;

	printf("\nLongpress!\n");
	g_dbus_connection_emit_signal(_conn, NULL, DBUS_OBJECT_PATH,
				      DBUS_INTERFACE_NAME, "LongPress", NULL,
				      &e);
	if (e) {
		fprintf(stderr, "%s\n", e->message);
		g_error_free(e);
	}

	_timer_src = 0;

	return FALSE;
}

static void onButtonPress(struct timespec *ts)
{
	GError *e = NULL;

	printf("\nButton Press\n");
	printf("- diff: %ld ms\n", diff_in_us(&_prev_ts, ts));
	g_dbus_connection_emit_signal(_conn, NULL, DBUS_OBJECT_PATH,
				      DBUS_INTERFACE_NAME, "Pressed", NULL, &e);
	if (e) {
		fprintf(stderr, "%s\n", e->message);
		g_error_free(e);
	}

	if (_timer_src)
		g_source_remove(_timer_src);

	if (diff_in_us(&_prev_ts, ts) < BUTTON_DBL_CLICK_TIMEOUT_MS) {
		printf("- check double click\n");
		_dbl_click = TRUE;
		_pressed = FALSE;
	} else {
		printf("- check normal press\n");
		_dbl_click = FALSE;
		_pressed = TRUE;

		_timer_src = g_timeout_add(BUTTON_LONGPRESS_TIMEOUT_MS,
					   onLongpress, NULL);
	}

	_prev_ts.tv_sec = ts->tv_sec;
	_prev_ts.tv_nsec = ts->tv_nsec;
}

static void onButtonRelease(struct timespec *ts)
{
	GError *e = NULL;

	if (_dbl_click) {
		printf("\nButton Double click\n");
		g_dbus_connection_emit_signal(_conn, NULL, DBUS_OBJECT_PATH,
					      DBUS_INTERFACE_NAME,
					      "DoubleClicked", NULL, &e);
		if (e) {
			fprintf(stderr, "%s\n", e->message);
			g_error_free(e);
		}
	} else {
		printf("\nButton click\n");
		g_dbus_connection_emit_signal(_conn, NULL, DBUS_OBJECT_PATH,
					      DBUS_INTERFACE_NAME, "Clicked",
					      NULL, &e);
		if (e) {
			fprintf(stderr, "%s\n", e->message);
			g_error_free(e);
		}
	}

	printf("\nButton Release\n");
	printf("- diff: %ld ms\n", diff_in_us(&_prev_ts, ts));
	g_dbus_connection_emit_signal(_conn, NULL, DBUS_OBJECT_PATH,
				      DBUS_INTERFACE_NAME, "Released", NULL,
				      &e);
	if (e) {
		fprintf(stderr, "%s\n", e->message);
		g_error_free(e);
	}

	if (_timer_src) {
		g_source_remove(_timer_src);
		_timer_src = 0;
	}
}

static void onButton(unsigned char value, struct timespec *ts)
{
	if (value == BUTTON_PRESSED)
		onButtonPress(ts);
	else if (value == BUTTON_RELEASED)
		onButtonRelease(ts);
}

int ntoy_button_dbus_init(GDBusConnection *conn)
{
	GDBusNodeInfo *intro;
	GError *e = NULL;

	if (ntoy_setup_gpio(BUTTON_GPIO_PIN, "in", "both") < 0)
		return -1;

	intro = g_dbus_node_info_new_for_xml(DBUS_INTROSPECTION, &e);
	if (!intro) {
		fprintf(stderr, "g_dbus_node_info_new_for_xml() failed. %s\n",
			e->message);
		g_error_free(e);
		return -1;
	}

	ntoy_add_gpio_handler(BUTTON_GPIO_PIN, onButton);

	_conn = conn;

	return g_dbus_connection_register_object(conn, DBUS_OBJECT_PATH,
						 intro->interfaces[0], NULL,
						 NULL, NULL, &e);
}
