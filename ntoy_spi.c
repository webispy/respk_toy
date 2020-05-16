#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/spi/spidev.h>

#include "ntoy_spi.h"

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

int ntoy_spi_open(unsigned int bus, unsigned int cs, unsigned char mode,
		  unsigned char bits, unsigned int speed)
{
	int fd = -1;
	char buf[255];
	int ret;

	if (mode > 3)
		return -1;

	snprintf(buf, sizeof(buf), "/dev/spidev%d.%d", bus, cs);

	fd = open(buf, O_SYNC | O_WRONLY);
	if (fd < 0)
		return fd;

	ret = spi_setup(fd, mode, bits, speed);
	if (ret < 0) {
		close(fd);
		return ret;
	}

	return fd;
}

int ntoy_spi_write(int fd, unsigned char *buf, int len)
{
	struct spi_ioc_transfer data;

	if (!buf)
		return -1;

	if (len <= 0)
		return -1;

	memset(&data, 0, sizeof(data));

	data.tx_buf = (unsigned long)buf;
	data.rx_buf = (unsigned long)NULL;
	data.len = len;

	return ioctl(fd, SPI_IOC_MESSAGE(1), &data);
}

int ntoy_spi_read_write(int fd, unsigned char *tx_buf, unsigned char *rx_buf,
			int len)
{
	struct spi_ioc_transfer data;

	if (!tx_buf || !rx_buf)
		return -1;

	if (len <= 0)
		return -1;

	memset(&data, 0, sizeof(data));

	data.tx_buf = (unsigned long)tx_buf;
	data.rx_buf = (unsigned long)rx_buf;
	data.len = len;

	return ioctl(fd, SPI_IOC_MESSAGE(1), &data);
}
