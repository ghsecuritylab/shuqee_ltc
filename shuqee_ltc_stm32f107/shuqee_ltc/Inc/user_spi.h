#ifndef __USER_SPI_H
#define __USER_SPI_H

#include "stm32f1xx_hal.h"
#include "user_config.h"

extern void user_spi_init(void);


extern void spi1_write_byte(uint8_t tx_data);

#endif /* __USER_SPI_H */
