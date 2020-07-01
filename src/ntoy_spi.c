#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/spi/spidev.h>

#include "ntoy_spi.h"

struct spi_info {
	int fd;
	unsigned int speed;
	unsigned char bits;
};

struct spi_info channels[2] = {
	/* Channel default settings ( 1 MHz, 8 Bits per word) */
	{ -1, 1000000, 8 }, /* Channel 0 */
	{ -1, 1000000, 8 } /* Channel 1 */
};

static int spi_setup(int fd, unsigned char mode, unsigned char bits,
		     unsigned int speed)
{
	int ret;

	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret < 0)
		return ret;

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret < 0)
		return ret;

	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret < 0)
		return ret;

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret < 0)
		return ret;

	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret < 0)
		return ret;

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret < 0)
		return ret;

	return 0;
}

int ntoy_spi_open(unsigned int channel, unsigned char mode, unsigned char bits,
		  unsigned int speed)
{
	int fd = -1;
	char buf[255];
	int ret;

	if (mode > 3)
		return -1;

	if (channel > 1)
		return -1;

	snprintf(buf, sizeof(buf), "/dev/spidev0.%d", channel);

	fd = open(buf, O_SYNC | O_RDWR);
	if (fd < 0)
		return fd;

	ret = spi_setup(fd, mode, bits, speed);
	if (ret < 0) {
		close(fd);
		return ret;
	}

	channels[channel].fd = fd;
	channels[channel].bits = bits;
	channels[channel].speed = speed;

	return fd;
}

int ntoy_spi_close(unsigned int channel)
{
	if (channel > 1)
		return -1;

	if (channels[channel].fd < 0)
		return -1;

	close(channels[channel].fd);

	channels[channel].fd = -1;

	return 0;
}

int ntoy_spi_write(unsigned int channel, unsigned char *buf, int len)
{
	return ntoy_spi_read_write(channel, buf, NULL, len);
}

int ntoy_spi_read_write(unsigned int channel, unsigned char *tx_buf,
			unsigned char *rx_buf, int len)
{
	struct spi_ioc_transfer data;

	if (channel > 1)
		return -1;

	if (channels[channel].fd < 0)
		return -1;

	if (len <= 0)
		return -1;

	memset(&data, 0, sizeof(data));

	if (tx_buf)
		data.tx_buf = (unsigned long)tx_buf;
	else
		data.tx_buf = (unsigned long)NULL;

	if (rx_buf)
		data.rx_buf = (unsigned long)rx_buf;
	else
		data.rx_buf = (unsigned long)NULL;

	data.len = len;
	data.bits_per_word = channels[channel].bits;
	data.delay_usecs = 0;
	data.speed_hz = channels[channel].speed;

	return ioctl(channels[channel].fd, SPI_IOC_MESSAGE(1), &data);
}
