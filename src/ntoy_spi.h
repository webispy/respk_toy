#ifndef __NTOY_SPI_H__
#define __NTOY_SPI_H__

/**
 * Open a SPI device
 *
 * @param[in] channel 0 or 1
 * @param[in] mode SPI_MODE_0...SPI_MODE_3
 * @param[in] bits BITS_PER_WORD 1..N
 * @param[in] speed MAX_SPEED_HZ
 */
int ntoy_spi_open(unsigned int channel, unsigned char mode, unsigned char bits,
		  unsigned int speed);
int ntoy_spi_close(unsigned int channel);

int ntoy_spi_write(unsigned int channel, unsigned char *buf, int len);

int ntoy_spi_read_write(unsigned int channel, unsigned char *tx_buf,
			unsigned char *rx_buf, int len);

#endif