#ifndef __NTOY_MATRIX_H__
#define __NTOY_MATRIX_H__

#include <glib.h>
#include <gio/gio.h>

#define NTOY_NUM_MATRIX 4

#define NTOY_MATRIX_NUM_ROWS 8
#define NTOY_MATRIX_NUM_COLS 32

#define NTOY_MATRIX_MIN_BRIGHTNESS 0
#define NTOY_MATRIX_MAX_BRIGHTNESS 15

int ntoy_matrix_open(void);
int ntoy_matrix_close(void);

int ntoy_matrix_raw_set_pixel(int row, int col, int value);
int ntoy_matrix_raw_set_row(int row, unsigned int value);
void ntoy_matrix_raw_clear(void);
void ntoy_matrix_raw_update(void);

int ntoy_matrix_set_brightness(int value);

int ntoy_matrix_draw_open(void);
int ntoy_matrix_draw_flush(void);
int ntoy_matrix_draw_close(void);

int ntoy_matrix_draw_fill(int value);
int ntoy_matrix_draw_pixel(int y, int x, int value);
int ntoy_matrix_draw_line(int y, int x, int h, int w, int value);
int ntoy_matrix_draw_text(int y, int x, const char *text, int value);

int ntoy_matrix_dbus_init(GDBusConnection *conn);

#endif
