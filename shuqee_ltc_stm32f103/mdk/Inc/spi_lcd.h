#ifndef __SPI_LCD_H
#define __SPI_LCD_H

#include "stm32f1xx_hal.h"
#include "user_config.h"
#include "user_spi.h"

#define X_MAX_PIXEL	        128
#define Y_MAX_PIXEL	        160

#define RED  	0xf800
#define GREEN	0x07e0
#define BLUE 	0x001f
#define WHITE	0xffff
#define BLACK	0x0000
#define YELLOW  0xFFE0
#define GRAY0   0xEF7D   	//��ɫ0 3165 00110 001011 00101
#define GRAY1   0x8410      	//��ɫ1      00000 000000 00000
#define GRAY2   0x4208      	//��ɫ2  1111111111011111

extern void spi_lcd_init(void);
extern void lcd_test(void);

#endif /* __SPI_LCD_H */
