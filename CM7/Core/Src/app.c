/*
 * app.c
 *
 *  Created on: 2026/10/19
 *      Author: sata
 */

#include "../Inc/main.h"
#include "../../Drivers/BSP/led.h"
#include "../../Drivers/BSP/lcd.h"

// HAL Handlers extern
extern I2C_HandleTypeDef hi2c1;
extern QSPI_HandleTypeDef hqspi;
extern SAI_HandleTypeDef hsai_BlockA1;
extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi6;
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart4;
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern SDRAM_HandleTypeDef hsdram1;

// あとで別ファイルに書き出す
#if 1
#define LCD_WIDTH    320
#define LCD_HEIGHT   240

/* RGB565 */
#define LCD_BLACK    0x0000
#define LCD_RED      0xF800
#define LCD_GREEN    0x07E0
#define LCD_BLUE     0x001F

/* ST7789 commands */
#define ST7789_SWRESET  0x01
#define ST7789_SLPOUT   0x11
#define ST7789_NORON    0x13
#define ST7789_INVON    0x21
#define ST7789_DISPON   0x29
#define ST7789_CASET    0x2A
#define ST7789_RASET    0x2B
#define ST7789_RAMWR    0x2C
#define ST7789_MADCTL   0x36
#define ST7789_COLMOD   0x3A

/* CubeMXが生成したmain.hのピン名を使用 */
#define LCD_CS_LOW()    HAL_GPIO_WritePin(LCD_NCS_GPIO_Port, LCD_NCS_Pin,  GPIO_PIN_RESET)
#define LCD_CS_HIGH()   HAL_GPIO_WritePin(LCD_NCS_GPIO_Port, LCD_NCS_Pin,  GPIO_PIN_SET)
#define LCD_DC_CMD()    HAL_GPIO_WritePin(LCD_DC_GPIO_Port,  LCD_DC_Pin,   GPIO_PIN_RESET)
#define LCD_DC_DATA()   HAL_GPIO_WritePin(LCD_DC_GPIO_Port,  LCD_DC_Pin,   GPIO_PIN_SET)
#define LCD_RST_LOW()   HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin,  GPIO_PIN_RESET)
#define LCD_RST_HIGH()  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin,  GPIO_PIN_SET)
/* 1ライン分のRGB565バッファ。DMAを使わないので配置先・キャッシュ操作は不要 */
static uint8_t lcd_line_buf[LCD_WIDTH * 2];

static void LCD_WriteCommand(uint8_t cmd);
static void LCD_WriteData(const uint8_t *data, uint16_t size);
static void LCD_Reset(void);
static void LCD_Init(void);
static void LCD_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
static void LCD_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
static void LCD_FillScreen(uint16_t color);

static void LCD_WriteCommand(uint8_t cmd)
{
  LCD_CS_LOW();
  LCD_DC_CMD();
  HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
  LCD_CS_HIGH();
}

static void LCD_WriteData(const uint8_t *data, uint16_t size)
{
  LCD_CS_LOW();
  LCD_DC_DATA();
  HAL_SPI_Transmit(&hspi1, (uint8_t *)data, size, HAL_MAX_DELAY);
  LCD_CS_HIGH();
}

static void LCD_Reset(void)
{
    LCD_RST_HIGH();
    HAL_Delay(5);
    LCD_RST_LOW();
    HAL_Delay(20);
    LCD_RST_HIGH();
    HAL_Delay(120);
}

static void LCD_Init(void)
{
#if 1

    LCD_CS_HIGH();
    LCD_DC_DATA();

    LCD_Reset();

    LCD_WriteCommand(ST7789_SWRESET);
    HAL_Delay(150);

    LCD_WriteCommand(ST7789_SLPOUT);
    HAL_Delay(120);

    /*
     * RGB565
     */
    LCD_WriteCommand(ST7789_COLMOD);

    {
        uint8_t data = 0x55;
        LCD_WriteData(&data, 1);
    }

    HAL_Delay(10);

    /*
     * Landscape 320 x 240
     *
     * MX | MV
     * 0x40 | 0x20 = 0x60
     */
    LCD_WriteCommand(ST7789_MADCTL);

    {
        uint8_t data = 0x60;
        LCD_WriteData(&data, 1);
    }

    /*
     * 320 columns
     */
    LCD_WriteCommand(ST7789_CASET);

    {
        uint8_t data[] =
        {
            0x00, 0x00,
            0x01, 0x3F
        };

        LCD_WriteData(data, 4);
    }

    /*
     * 240 rows
     */
    LCD_WriteCommand(ST7789_RASET);

    {
        uint8_t data[] =
        {
            0x00, 0x00,
            0x00, 0xEF
        };

        LCD_WriteData(data, 4);
    }

    /*
     * Inversion
     */
    LCD_WriteCommand(ST7789_INVON);
    HAL_Delay(10);

    /*
     * Normal display
     */
    LCD_WriteCommand(ST7789_NORON);
    HAL_Delay(10);

    /*
     * Display ON
     */
    LCD_WriteCommand(ST7789_DISPON);
    HAL_Delay(10);

#else
    uint8_t d;

    LCD_CS_HIGH();
    LCD_DC_DATA();
    LCD_Reset();

    LCD_WriteCommand(ST7789_SWRESET);
    HAL_Delay(150);

    LCD_WriteCommand(ST7789_SLPOUT);
    HAL_Delay(120);

    LCD_WriteCommand(ST7789_COLMOD);   /* RGB565 */
    d = 0x55;
    LCD_WriteData(&d, 1);
    HAL_Delay(10);

    LCD_WriteCommand(ST7789_MADCTL);   /* Landscape 320x240 (MX|MV) */
    d = 0x60;
    LCD_WriteData(&d, 1);

    LCD_SetAddressWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

    LCD_WriteCommand(ST7789_INVON);
    HAL_Delay(10);
    LCD_WriteCommand(ST7789_NORON);
    HAL_Delay(10);
    LCD_WriteCommand(ST7789_DISPON);
    HAL_Delay(10);
#endif
}

static void LCD_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  uint8_t d[4];

  LCD_WriteCommand(ST7789_CASET);
  d[0] = x0 >> 8; d[1] = x0 & 0xFF;
  d[2] = x1 >> 8; d[3] = x1 & 0xFF;
  LCD_WriteData(d, 4);

  LCD_WriteCommand(ST7789_RASET);
  d[0] = y0 >> 8; d[1] = y0 & 0xFF;
  d[2] = y1 >> 8; d[3] = y1 & 0xFF;
  LCD_WriteData(d, 4);

  LCD_WriteCommand(ST7789_RAMWR);
}

static void LCD_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    if (x + w > LCD_WIDTH)  w = LCD_WIDTH  - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
    if (w == 0 || h == 0) return;

    /* 1ライン分のデータを作る */
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    for (uint16_t i = 0; i < w; i++)
    {
        lcd_line_buf[i * 2 + 0] = hi;
        lcd_line_buf[i * 2 + 1] = lo;
    }

    LCD_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    /* RAMWR後はCS・DCを保持したまま、ラインごとにブロッキング送信 */
    LCD_CS_LOW();
    LCD_DC_DATA();
    for (uint16_t row = 0; row < h; row++) {
        if (HAL_SPI_Transmit(&hspi1, lcd_line_buf, w * 2, HAL_MAX_DELAY) != HAL_OK) {
            break;
        }
    }
    LCD_CS_HIGH();
}

static void LCD_FillScreen(uint16_t color)
{
    LCD_FillRect(0, 0, LCD_WIDTH, LCD_HEIGHT, color);
}
#endif

/**
 * @brief	main routine
 *
 * @return	int
 */
int App(void)
{
#if 0
    // 3秒点灯、点滅、消灯
    for (int i = LED0; i <= LED3; i++) {
        LED_On(i);
    }
    HAL_Delay(3000);
    for (int i = 0; i < 10; i++) {
        for (int j = LED0; j <= LED3; j++) {
            LED_Toggle(j);
        }
        HAL_Delay(50);
    }
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 65535 / 2);  /* 明るさ50% */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    LCD_Init();

    LCD_FillScreen(LCD_RED);
    HAL_Delay(1000);

    while (1) {
        for (int i = LED0; i <= LED3; i++) {
            LED_Toggle(i);
        }
        HAL_Delay(500);
    }
    return 0;
#else


    /*
    * LCD initialization
    */
    ST7789_Init();

    /*
    * Red
    */
    ST7789_FillScreen(ST7789_RED);
    HAL_Delay(1000);
    while (1) {
        LCD_WriteCommand(0x21);  /* INVON */
        HAL_Delay(500);
        LCD_WriteCommand(0x20);  /* INVOFF */
        HAL_Delay(500);
    }

    // /*
    // * Green
    // */
    // ST7789_FillScreen(ST7789_GREEN);
    // //  HAL_Delay(1000);

    // /*
    // * Blue
    // */
    // ST7789_FillScreen(ST7789_BLUE);
    // //  HAL_Delay(1000);

    // /*
    // * Black
    // */
    // ST7789_FillScreen(ST7789_BLACK);

    // /*
    // * Draw some rectangles
    // */
    // ST7789_FillRect(
    //     10, 10,
    //     100, 50,
    //     ST7789_RED
    // );

    // ST7789_FillRect(
    //     110, 70,
    //     100, 50,
    //     ST7789_GREEN
    // );

    // ST7789_FillRect(
    //     50, 140,
    //     220, 50,
    //     ST7789_BLUE
    // );

    // HAL_Delay(1000);

    // // DMA
    // ST7789_FillScreen(ST7789_RED);
    // ST7789_FillScreen(ST7789_GREEN);
    // ST7789_FillScreen(ST7789_BLUE);
    // ST7789_FillScreen(ST7789_BLACK);

    // ST7789_FillRectDMA(
    //     10, 10,
    //     100, 50,
    //     ST7789_RED
    // );

    // ST7789_FillRectDMA(
    //     110, 70,
    //     100, 50,
    //     ST7789_GREEN
    // );

    // ST7789_FillRectDMA(
    //     50, 140,
    //     220, 50,
    //     ST7789_BLUE
    // );
    return 0;
#endif
}
