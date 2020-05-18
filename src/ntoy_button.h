#ifndef __NTOY_BUTTON_H__
#define __NTOY_BUTTON_H__

#include <glib.h>
#include <gio/gio.h>

#define BUTTON_LONGPRESS_TIMEOUT_MS 1000
#define BUTTON_DBL_CLICK_TIMEOUT_MS 300

int ntoy_button_dbus_init(GDBusConnection *conn);

#endif