#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#include "stm32f1xx_hal.h"
#include "user_config.h"
#include "user_spi.h"

extern void spi_flash_init(void);
extern void spi_flash_read(uint8_t* p_buffer,uint32_t read_addr,uint16_t num_byte_to_read);   //��ȡflash
extern void spi_flash_write(uint8_t* p_buffer,uint32_t write_addr,uint16_t num_byte_to_write);//д��flash
extern void spi_flash_erase_chip(void);    	  //��Ƭ����
extern void spi_flash_erase_sector(uint32_t dst_addr);//��������
extern void spi_flash_powerdown(void);           //�������ģʽ
extern void spi_flash_wakeup(void);			  //����

#endif /* __SPI_FLASH_H */
