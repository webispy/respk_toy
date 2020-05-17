#ifndef __NTOY_SPI_H__
#define __NTOY_SPI_H__

int ntoy_spi_open(unsigned int bus, unsigned int cs, unsigned char mode,
		  unsigned char bits, unsigned int speed);

int ntoy_spi_write(int fd, unsigned char *buf, int len);

int ntoy_spi_read_write(int fd, unsigned char *tx_buf, unsigned char *rx_buf,
			int len);

#endif