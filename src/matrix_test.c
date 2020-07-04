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

static void simple_art(int max_loop)
{
	int i;
	int count;
	int dir = 0;

	unsigned int rows[8] = {
		0x00000000, 0x7e423800, 0x4224446c, 0x5a180492,
		0x5a181800, 0x42241044, 0x7e420038, 0x00001000
	};
	unsigned int first, last;

	/**
	 * http://dotmatrixtool.com/
	 *  - Width: 32
	 *  - Height: 8
	 *  - Byte Order: row major
	 *  - Endian: big endian
	 */

	ntoy_matrix_clear();

	for (count = 1; count <= max_loop; count++) {
		/* Draw */
		for (i = 0; i < 8; i++)
			ntoy_matrix_set_row(i, rows[i]);

		ntoy_matrix_draw();
		usleep(20 * 1000);

		/* Rotate */
		first = rows[0];
		last = rows[7];
		for (i = 0; i < 8; i++) {
			if (dir == 0)
				rows[i] = (rows[i] << 1) |
					  ((rows[i] >> 31) & 0x01);
			else if (dir == 1)
				rows[i] = ((rows[i] & 0xFFFFFFFF) >> 1) |
					  (rows[i] << 31);
			else if (dir == 2) {
				if (i < 7)
					rows[i] = rows[i + 1];
			} else if (dir == 3) {
				if (i < 7)
					rows[7 - i] = rows[7 - i - 1];
			}
		}

		if (dir == 2)
			rows[7] = first;
		else if (dir == 3)
			rows[0] = last;

		if (count % 32 == 0) {
			dir++;
			if (dir > 3)
				dir = 0;

			printf("change dir: %d\n", dir);
		}
	}
}

static void _draw_bar(int index, int value)
{
	int y;
	int flag;

	for (y = 0; y < NTOY_NUM_ROWS; y++) {
		if (8 - value > y)
			flag = 0;
		else
			flag = 1;

		/* width: 3 */
		ntoy_matrix_set_pixel(y, index * 4, flag);
		ntoy_matrix_set_pixel(y, index * 4 + 1, flag);
		ntoy_matrix_set_pixel(y, index * 4 + 2, flag);

		/* space */
		ntoy_matrix_set_pixel(y, index * 4 + 3, 0);
	}
}

static void simple_vertical_bar(void)
{
	int bar[8] = { 3, 2, 1, 4, 5, 6, 7, 8 };
	int i;
	int count;

	ntoy_matrix_clear();

	for (count = 1; count < 10000; count++) {
		for (i = 0; i < 8; i++)
			_draw_bar(i, bar[i]);

		ntoy_matrix_draw();
		usleep(50 * 1000);

		for (i = 0; i < 8; i++) {
			bar[i] += (rand() % 3 - 1);
			if (bar[i] > 8)
				bar[i] = 8;
			if (bar[i] < 0)
				bar[i] = 0;
		}
	}
}

static void simple_xy_loop(void)
{
	int i, j;

	for (i = 0; i < NTOY_NUM_ROWS; i++) {
		for (j = 0; j < NTOY_NUM_COLS; j++) {
			ntoy_matrix_set_pixel(i, j, 1);
			ntoy_matrix_draw();
			usleep(10 * 1000);
			ntoy_matrix_set_pixel(i, j, 0);
			ntoy_matrix_draw();
			usleep(5 * 1000);
		}
	}

	for (j = 0; j < NTOY_NUM_COLS; j++) {
		for (i = 0; i < NTOY_NUM_ROWS; i++) {
			ntoy_matrix_set_pixel(i, j, 1);
			ntoy_matrix_draw();
			usleep(10 * 1000);
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
		printf("matrix open failed\n");
		return -1;
	}

	printf("\n\nstart test\n");

	if (argc == 2) {
		printf("test = %d\n", argv[1][0] - '0');

		if (argv[1][0] == '0') {
			simple_set_row();
			next_key();
			simple_set_pixel();
			next_key();
		} else if (argv[1][0] == '1') {
			simple_xy_loop();
		} else if (argv[1][0] == '2') {
			simple_art(1);
		} else if (argv[1][0] == '3') {
			simple_art(10000);
		} else if (argv[1][0] == '4') {
			simple_vertical_bar();
		}
	}

	ntoy_matrix_close();
}
