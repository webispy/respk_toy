#include <stdio.h>
#include <unistd.h>

#include "ntoy_matrix.h"

static void simple_set_row(void)
{
	ntoy_matrix_clear();
	ntoy_matrix_set_row(0, 0x00000001);
	ntoy_matrix_set_row(1, 0x00000022);
	ntoy_matrix_set_row(2, 0x10000303);
	ntoy_matrix_set_row(3, 0x01004004);
	ntoy_matrix_set_row(4, 0x00150005);
	ntoy_matrix_set_row(5, 0x00610006);
	ntoy_matrix_set_row(6, 0x07ff00f7);
	ntoy_matrix_set_row(7, 0x01010101);
	ntoy_matrix_draw();
}

static void simple_set_pixel(void)
{
	ntoy_matrix_clear();

	ntoy_matrix_set_pixel(0, 0, 1);
	ntoy_matrix_set_pixel(0, 7, 1);

	ntoy_matrix_set_pixel(1, 8, 1);
	ntoy_matrix_set_pixel(1, 15, 1);

	ntoy_matrix_set_pixel(2, 16, 1);
	ntoy_matrix_set_pixel(2, 23, 1);

	ntoy_matrix_set_pixel(3, 24, 1);
	ntoy_matrix_set_pixel(3, 31, 1);

	ntoy_matrix_set_pixel(4, 1, 1);
	ntoy_matrix_set_pixel(4, 6, 1);

	ntoy_matrix_set_pixel(5, 9, 1);
	ntoy_matrix_set_pixel(5, 14, 1);

	ntoy_matrix_set_pixel(6, 17, 1);
	ntoy_matrix_set_pixel(6, 22, 1);

	ntoy_matrix_set_pixel(7, 25, 1);
	ntoy_matrix_set_pixel(7, 30, 1);

	ntoy_matrix_draw();
}

static void simple_art(void)
{
	/**
	 * http://dotmatrixtool.com/
	 *  - Width: 32
	 *  - Height: 8
	 *  - Byte Order: row major
	 *  - Endian: big endian
	 */

	ntoy_matrix_clear();
	ntoy_matrix_set_row(0, 0x00000000);
	ntoy_matrix_set_row(1, 0x7e423800);
	ntoy_matrix_set_row(2, 0x4224446c);
	ntoy_matrix_set_row(3, 0x5a180492);
	ntoy_matrix_set_row(4, 0x5a181800);
	ntoy_matrix_set_row(5, 0x42241044);
	ntoy_matrix_set_row(6, 0x7e420038);
	ntoy_matrix_set_row(7, 0x00001000);
	ntoy_matrix_draw();
}

static void simple_xy_loop(void)
{
	int i, j;

	for (i = 0; i < NTOY_NUM_ROWS; i++) {
		for (j = 0; j < NTOY_NUM_COLS; j++) {
			ntoy_matrix_set_pixel(i, j, 1);
			ntoy_matrix_draw();
			usleep(1 * 1000);
			ntoy_matrix_set_pixel(i, j, 0);
			ntoy_matrix_draw();
			usleep(1 * 1000);
		}
	}

	for (j = 0; j < NTOY_NUM_COLS; j++) {
		for (i = 0; i < NTOY_NUM_ROWS; i++) {
			ntoy_matrix_set_pixel(i, j, 1);
			ntoy_matrix_draw();
			usleep(5 * 1000);
			ntoy_matrix_set_pixel(i, j, 0);
			ntoy_matrix_draw();
			usleep(5 * 1000);
		}
	}
}

static void next_key(void)
{
	char buf[255];

	printf("\n\nPress ENTER key to continue\n");
	if (fgets(buf, 255, stdin) == NULL)
		return;
}

int main(int argc, char *argv[])
{
	printf("Start\n");

	if (ntoy_matrix_open() < 0) {
		printf("matrix open fail\n");
		return -1;
	}

	printf("\n\nstart test\n");

	simple_set_row();
	next_key();

	simple_set_pixel();
	next_key();

	simple_art();
	next_key();

	simple_xy_loop();
	next_key();

	ntoy_matrix_close();
}
