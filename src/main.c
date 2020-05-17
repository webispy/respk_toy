#include <stdio.h>
#include <unistd.h>

#include <glib.h>
#include <gio/gio.h>

#include "ntoy_led.h"

static void _bus_acquired(GDBusConnection *conn, const gchar *name,
			  gpointer user_data)
{
	printf("dbus bus acquired\n");

	ntoy_led_dbus_init(conn);
}

static void _name_acquired(GDBusConnection *connection, const gchar *name,
			   gpointer user_data)
{
	printf("dbus name('%s') acquired\n", name);
}

static void _name_lost(GDBusConnection *connection, const gchar *name,
		       gpointer user_data)
{
	printf("dbus name('%s') lost\n", name);
}

int main(int argc, char *argv[])
{
	GMainLoop *loop;
	guint id;

	loop = g_main_loop_new(NULL, FALSE);

	id = g_bus_own_name(G_BUS_TYPE_SYSTEM, NTOY_DBUS_SERVICE,
			    G_BUS_NAME_OWNER_FLAGS_REPLACE, _bus_acquired,
			    _name_acquired, _name_lost, NULL, NULL);
	if (id == 0)
		return -1;

	printf("Start\n");
	g_main_loop_run(loop);

	g_bus_unown_name(id);

	g_main_loop_unref(loop);

	return 0;
}
