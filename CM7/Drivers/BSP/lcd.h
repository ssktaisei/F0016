/*
 * lcd.h
 *
 *  Created on: 2026/10/21
 *      Author: sata
 */

#ifndef BSP_LCD_H_
#define BSP_LCD_H_

/*
 * st7789.h
 *
 *  Created on: 2026/09/01
 *      Author: sata
 */

#ifndef ST7789_H
#define ST7789_H

#include "main.h"
#include <stdint.h>

#define ST7789_WIDTH    320
#define ST7789_HEIGHT   240

/* RGB565 */
#define ST7789_BLACK    0x0000
#define ST7789_WHITE    0xFFFF
#define ST7789_RED      0xF800
#define ST7789_GREEN    0x07E0
#define ST7789_BLUE     0x001F
#define ST7789_YELLOW   0xFFE0
#define ST7789_CYAN     0x07FF
#define ST7789_MAGENTA  0xF81F

HAL_StatusTypeDef ST7789_Init(void);

HAL_StatusTypeDef ST7789_WriteCommand(uint8_t cmd);
HAL_StatusTypeDef ST7789_WriteData(uint8_t *data, uint16_t size);

void ST7789_SetAddressWindow(uint16_t x0,
                             uint16_t y0,
                             uint16_t x1,
                             uint16_t y1);

void ST7789_DrawPixel(uint16_t x,
                      uint16_t y,
                      uint16_t color);

void ST7789_FillRect(uint16_t x,
                     uint16_t y,
                     uint16_t w,
                     uint16_t h,
                     uint16_t color);

void ST7789_FillScreen(uint16_t color);

uint16_t ST7789_Color565(uint8_t r,
                         uint8_t g,
                         uint8_t b);

void ST7789_FillRectDMA(uint16_t x,
                        uint16_t y,
                        uint16_t w,
                        uint16_t h,
                        uint16_t color);

#endif

#endif /* BSP_LCD_H_ */
