/*
 * st7789.c
 *
 *  Created on: 2026/09/01
 *      Author: sata
 */

#include "lcd.h"
#include "main.h"
#include "string.h"

#define ST7789_LINE_BUFFER_SIZE    (ST7789_WIDTH * 2)

/*
 * DMAからアクセス可能なD2 SRAMへ配置する。
 *
 * linker script側で .RAM_D2 をD2 SRAMに割り当てる。
 */
// __attribute__((section(".RAM_D2"), aligned(32)))
static uint8_t st7789_line_buffer[ST7789_LINE_BUFFER_SIZE];

static volatile uint8_t st7789_dma_done = 0;
static volatile uint8_t st7789_dma_error = 0;

extern SPI_HandleTypeDef hspi1;

/*
 * Change these to match your CubeMX configuration.
 */
#define ST7789_CS_GPIO_Port    LCD_NCS_GPIO_Port
#define ST7789_CS_Pin          LCD_NCS_Pin

#define ST7789_DC_GPIO_Port    LCD_DC_GPIO_Port
#define ST7789_DC_Pin          LCD_DC_Pin

#define ST7789_RST_GPIO_Port   LCD_RST_GPIO_Port
#define ST7789_RST_Pin         LCD_RST_Pin


#define CS_LOW()    HAL_GPIO_WritePin(ST7789_CS_GPIO_Port, \
                                      ST7789_CS_Pin, \
                                      GPIO_PIN_RESET)

#define CS_HIGH()   HAL_GPIO_WritePin(ST7789_CS_GPIO_Port, \
                                      ST7789_CS_Pin, \
                                      GPIO_PIN_SET)

#define DC_COMMAND() HAL_GPIO_WritePin(ST7789_DC_GPIO_Port, \
                                        ST7789_DC_Pin, \
                                        GPIO_PIN_RESET)

#define DC_DATA()    HAL_GPIO_WritePin(ST7789_DC_GPIO_Port, \
                                       ST7789_DC_Pin, \
                                       GPIO_PIN_SET)

#define RST_LOW()   HAL_GPIO_WritePin(ST7789_RST_GPIO_Port, \
                                      ST7789_RST_Pin, \
                                      GPIO_PIN_RESET)

#define RST_HIGH()  HAL_GPIO_WritePin(ST7789_RST_GPIO_Port, \
                                      ST7789_RST_Pin, \
                                      GPIO_PIN_SET)


/* ST7789 commands */
#define ST7789_SWRESET   0x01
#define ST7789_SLPIN     0x10
#define ST7789_SLPOUT    0x11
#define ST7789_NORON     0x13

#define ST7789_INVOFF    0x20
#define ST7789_INVON     0x21

#define ST7789_DISPOFF   0x28
#define ST7789_DISPON    0x29

#define ST7789_CASET     0x2A
#define ST7789_RASET     0x2B
#define ST7789_RAMWR    0x2C

#define ST7789_MADCTL    0x36
#define ST7789_COLMOD    0x3A


static HAL_StatusTypeDef ST7789_WriteByte(uint8_t data)
{
    return HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
}


HAL_StatusTypeDef ST7789_WriteCommand(uint8_t cmd)
{
    HAL_StatusTypeDef st;
    CS_LOW();
    DC_COMMAND();
    st = ST7789_WriteByte(cmd);
    CS_HIGH();
    return st;
}


HAL_StatusTypeDef ST7789_WriteData(uint8_t *data, uint16_t size)
{
    HAL_StatusTypeDef st;
    CS_LOW();
    DC_DATA();
    st = HAL_SPI_Transmit(&hspi1, data, size, HAL_MAX_DELAY);
    CS_HIGH();
    return st;
}


static HAL_StatusTypeDef T7789_WriteDataByte(uint8_t data)
{
    DC_DATA();
    return ST7789_WriteByte(data);
}


/*
 * Hardware reset
 */
static void ST7789_Reset(void)
{
    RST_HIGH();
    HAL_Delay(5);

    RST_LOW();
    HAL_Delay(20);

    RST_HIGH();
    HAL_Delay(120);
}


/*
 * ST7789 initialization for
 * Adafruit 2.0" 320x240 display.
 *
 * This follows the basic initialization used
 * by Adafruit's ST7789 driver.
 */
HAL_StatusTypeDef ST7789_Init(void)
{
    HAL_StatusTypeDef st;
    CS_HIGH();
    DC_DATA();

    ST7789_Reset();

    st = ST7789_WriteCommand(ST7789_SWRESET);
    HAL_Delay(150);

    st = ST7789_WriteCommand(ST7789_SLPOUT);
    HAL_Delay(120);

    /*
     * RGB565
     */
    st = ST7789_WriteCommand(ST7789_COLMOD);
    {
        uint8_t data = 0x55;
        st = ST7789_WriteData(&data, 1);
    }
    HAL_Delay(10);

    /*
     * Landscape 320 x 240
     *
     * MX | MV
     * 0x40 | 0x20 = 0x60
     */
    ST7789_WriteCommand(ST7789_MADCTL);
    {
        uint8_t data = 0x60;
        st = ST7789_WriteData(&data, 1);
    }

    /*
     * 320 columns
     */
    ST7789_WriteCommand(ST7789_CASET);
    {
        uint8_t data[] =
        {
            0x00, 0x00,
            0x01, 0x3F
        };
        st = ST7789_WriteData(data, 4);
    }

    /*
     * 240 rows
     */
    ST7789_WriteCommand(ST7789_RASET);
    {
        uint8_t data[] =
        {
            0x00, 0x00,
            0x00, 0xEF
        };
        st = ST7789_WriteData(data, 4);
    }

    /*
     * Inversion
     */
    st = ST7789_WriteCommand(ST7789_INVON);
    HAL_Delay(10);

    /*
     * Normal display
     */
    st = ST7789_WriteCommand(ST7789_NORON);
    HAL_Delay(10);

    /*
     * Display ON
     */
    st = ST7789_WriteCommand(ST7789_DISPON);
    HAL_Delay(10);

    return st;
}


/*
 * Set drawing window.
 */
void ST7789_SetAddressWindow(uint16_t x0,
                             uint16_t y0,
                             uint16_t x1,
                             uint16_t y1)
{
    uint8_t data[4];

    /*
     * Column address
     */
    ST7789_WriteCommand(ST7789_CASET);

    data[0] = x0 >> 8;
    data[1] = x0 & 0xFF;
    data[2] = x1 >> 8;
    data[3] = x1 & 0xFF;

    ST7789_WriteData(data, 4);

    /*
     * Row address
     */
    ST7789_WriteCommand(ST7789_RASET);

    data[0] = y0 >> 8;
    data[1] = y0 & 0xFF;
    data[2] = y1 >> 8;
    data[3] = y1 & 0xFF;

    ST7789_WriteData(data, 4);

    /*
     * Start memory write
     */
    ST7789_WriteCommand(ST7789_RAMWR);
}


/*
 * Draw one pixel.
 */
void ST7789_DrawPixel(uint16_t x,
                      uint16_t y,
                      uint16_t color)
{
    uint8_t data[2];

    if (x >= ST7789_WIDTH ||
        y >= ST7789_HEIGHT)
    {
        return;
    }

    ST7789_SetAddressWindow(x, y, x, y);

    data[0] = color >> 8;
    data[1] = color & 0xFF;

    ST7789_WriteData(data, 2);
}


/*
 * Fill rectangle.
 */
void ST7789_FillRect(uint16_t x,
                     uint16_t y,
                     uint16_t w,
                     uint16_t h,
                     uint16_t color)
{
#if 1
    uint32_t pixels;
    uint32_t i;

    if (x >= ST7789_WIDTH ||
        y >= ST7789_HEIGHT)
    {
        return;
    }

    if ((x + w) > ST7789_WIDTH)
    {
        w = ST7789_WIDTH - x;
    }

    if ((y + h) > ST7789_HEIGHT)
    {
        h = ST7789_HEIGHT - y;
    }

    if (w == 0 || h == 0)
    {
        return;
    }

    ST7789_SetAddressWindow(
        x,
        y,
        x + w - 1,
        y + h - 1
    );

    /*
     * Send pixels.
     *
     * This version sends one pixel at a time.
     * It is simple but not particularly fast.
     */
    CS_LOW();
    DC_DATA();

    for (i = 0; i < ((uint32_t)w * h); i++)
    {
        ST7789_WriteByte(color >> 8);
        ST7789_WriteByte(color & 0xFF);
    }

    CS_HIGH();
#else
    static uint8_t line_buf[ST7789_WIDTH * 2];

    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT)
        return;

    if (x + w > ST7789_WIDTH)
        w = ST7789_WIDTH - x;

    if (y + h > ST7789_HEIGHT)
        h = ST7789_HEIGHT - y;

    if (w == 0 || h == 0)
        return;

    /* 1ライン分のRGB565データを作る */
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    for (uint16_t i = 0; i < w; i++)
    {
        line_buf[i * 2 + 0] = hi;
        line_buf[i * 2 + 1] = lo;
    }

    ST7789_SetAddressWindow(
        x,
        y,
        x + w - 1,
        y + h - 1
    );

    CS_LOW();
    DC_DATA();

    for (uint16_t row = 0; row < h; row++)
    {
        HAL_SPI_Transmit(
            &hspi1,
            line_buf,
            w * 2,
            HAL_MAX_DELAY
        );
    }

    CS_HIGH();
#endif
}


/*
 * Fill entire display.
 */
void ST7789_FillScreen(uint16_t color)
{
    ST7789_FillRect(
        0,
        0,
        ST7789_WIDTH,
        ST7789_HEIGHT,
        color
    );
}


/*
 * Convert 24-bit RGB to RGB565.
 */
uint16_t ST7789_Color565(uint8_t r,
                         uint8_t g,
                         uint8_t b)
{
    return ((r & 0xF8) << 8) |
           ((g & 0xFC) << 3) |
           ((b & 0xF8) >> 3);
}

static void ST7789_CleanDCache(void *addr, uint32_t size)
{
    uintptr_t start = (uintptr_t)addr;
    uintptr_t end   = start + size;

    start &= ~((uintptr_t)31);
    end = (end + 31) & ~((uintptr_t)31);

    SCB_CleanDCache_by_Addr(
        (uint32_t *)start,
        (int32_t)(end - start)
    );
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1)
    {
        st7789_dma_done = 1;
    }
}


void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1)
    {
        st7789_dma_error = 1;
        st7789_dma_done = 1;
    }
}

static HAL_StatusTypeDef ST7789_TransmitDMA(
    uint8_t *data,
    uint16_t size)
{
    st7789_dma_done = 0;
    st7789_dma_error = 0;

    ST7789_CleanDCache(data, size);

    HAL_StatusTypeDef status;

    status = HAL_SPI_Transmit_DMA(
        &hspi1,
        data,
        size
    );

    if (status != HAL_OK)
    {
        return status;
    }

    /*
     * 今回は同期的なFillRectなので、
     * DMA完了まで待つ。
     */
    while (!st7789_dma_done)
    {
        /*
         * 必要ならここで他の処理をする。
         */
    }

    if (st7789_dma_error)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

void ST7789_FillRectDMA(uint16_t x,
                        uint16_t y,
                        uint16_t w,
                        uint16_t h,
                        uint16_t color)
{
    if (x >= ST7789_WIDTH ||
        y >= ST7789_HEIGHT)
    {
        return;
    }

    if (x + w > ST7789_WIDTH)
    {
        w = ST7789_WIDTH - x;
    }

    if (y + h > ST7789_HEIGHT)
    {
        h = ST7789_HEIGHT - y;
    }

    if (w == 0 || h == 0)
    {
        return;
    }

    /*
     * 1ライン分のRGB565を作る
     */
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    for (uint16_t i = 0; i < w; i++)
    {
        st7789_line_buffer[i * 2 + 0] = hi;
        st7789_line_buffer[i * 2 + 1] = lo;
    }

    /*
     * 描画範囲を設定
     */
    ST7789_SetAddressWindow(
        x,
        y,
        x + w - 1,
        y + h - 1
    );

    /*
     * ここからLCDのRAMへ連続転送
     */
    CS_LOW();
    DC_DATA();

    for (uint16_t row = 0; row < h; row++)
    {
        if (ST7789_TransmitDMA(
                st7789_line_buffer,
                w * 2) != HAL_OK)
        {
            break;
        }
    }

    CS_HIGH();
}
