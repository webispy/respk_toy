#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <glib.h>
#include <gio/gio.h>

#include "ntoy_gpio.h"

static unsigned char prev_value = 255;

static gboolean gpio_event_cb(GIOChannel *channel, GIOCondition condition,
			      gpointer user_data)
{
	char buf[1];
	GError *error = 0;
	gsize bytes_read = 0;
	GpioEventCallback func = user_data;
	unsigned char value;
	struct timespec spec;

	g_io_channel_seek_position(channel, 0, G_SEEK_SET, 0);
	g_io_channel_read_chars(channel, buf, 1, &bytes_read, &error);

	if (error) {
		fprintf(stderr, "error: '%s'\n", error->message);
		g_error_free(error);
	}

	value = buf[0] - '0';
	if (value == prev_value)
		return 1;

	/* un-expect release event */
	if (prev_value == 255 && value == 1) {
		prev_value = value;
		return 1;
	}

	clock_gettime(CLOCK_REALTIME, &spec);

	if (func)
		func(value, &spec);

	prev_value = value;

	return 1;
}

static int gpio_export(unsigned int gpio)
{
	int fd;
	size_t len;
	char buf[255];
	struct stat statbuf;

	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d", gpio);
	if (stat(buf, &statbuf) == 0) {
		if ((statbuf.st_mode & S_IFMT) == S_IFDIR)
			return 0;
	}

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "open failed: %s\n", strerror(errno));
		return -1;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	if (write(fd, buf, len) <= 0) {
		fprintf(stderr, "export: write failed: %s\n", strerror(errno));
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

static int gpio_set_dir(unsigned int gpio, const char *dir)
{
	int fd;
	char buf[255];

	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "open %s failed: %s\n", buf, strerror(errno));
		return -1;
	}

	if (write(fd, dir, strlen(dir) + 1) <= 0) {
		fprintf(stderr, "dir: write failed: %s\n", strerror(errno));
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

static int gpio_set_edge(unsigned int gpio, const char *edge)
{
	int fd;
	char buf[255];

	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/edge", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "open %s failed: %s\n", buf, strerror(errno));
		return -1;
	}

	if (write(fd, edge, strlen(edge) + 1) <= 0) {
		fprintf(stderr, "edge: write failed: %s\n", strerror(errno));
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

int ntoy_setup_gpio(unsigned int pin, const char *dir, const char *edge)
{
	if (gpio_export(pin) < 0)
		return -1;

	if (dir)
		gpio_set_dir(pin, dir);

	if (edge)
		gpio_set_edge(pin, edge);

	return 0;
}

int ntoy_open_gpio(unsigned int pin)
{
	int fd;
	char buf[255];
	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", pin);

	fd = open(buf, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "value: write failed: %s\n", strerror(errno));
		return -1;
	}

	return fd;
}

int ntoy_add_gpio_handler(unsigned int pin, GpioEventCallback func)
{
	int fd;
	char buf[255];
	GIOChannel *channel;

	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", pin);

	fd = open(buf, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		fprintf(stderr, "value: write failed: %s\n", strerror(errno));
		return -1;
	}

	channel = g_io_channel_unix_new(fd);
	if (!channel) {
		close(fd);
		return -1;
	}

	g_io_channel_set_close_on_unref(channel, TRUE);
	g_io_add_watch(channel, G_IO_PRI, gpio_event_cb, func);
	g_io_channel_unref(channel);

	return 0;
}
