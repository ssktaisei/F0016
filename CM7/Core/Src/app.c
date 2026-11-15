/*
 * app.c
 *
 *  Created on: 2026/10/19
 *      Author: sata
 */

#include "../Inc/main.h"
#include "../../Drivers/BSP/led.h"
#include "../../Drivers/BSP/lcd.h"
#include "../../Drivers/BSP/dac.h"

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


/*
 * pcm5102_audio_sai.c
 * STM32H757 (CM7側想定) の SAI1 Block A (I2Sプロトコル) + PCM5102 で
 * DMA循環により 1kHz正弦波を 48kHz/16bit ステレオで出力する
 *
 * ---- CubeMX設定 (SAI1 Block A) ----
 *   Mode / Protocol : Master Transmitter / I2S Standard (I2S/PCM Protocol)
 *   Audio Frequency : 48 kHz
 *   Data Size       : 16 Bits
 *   Slot            : 2スロット(ステレオ)、スロットサイズ 16bit (DataSize)
 *   Master Clock Out: Disable (PCM5102のSCKはGND接続で内蔵PLL動作)
 *   DMA (SAI1_A TX) : Circular, Peripheral/Memory Data Width = Half Word,
 *                     Memory increment = ON
 *   SAIカーネルクロック: fs x 256 の整数倍に近い周波数にする
 *                     (例: 49.152MHz = 12.288MHz x 4。PLL設定で誤差を小さく)
 *
 * ---- PCM5102 結線 ----
 *   BCK  <- SAI1_SCK_A
 *   LRCK <- SAI1_FS_A
 *   DIN  <- SAI1_SD_A
 *   SCK  -> GND, FMT -> GND, FLT -> GND, DEMP -> GND, XSMT -> 3.3V
 *
 * ---- 使い方 ----
 *   MX_SAI1_Init() の後に Audio_Start(); を呼ぶ。
 */

#include "main.h"
#include <math.h>
#include <string.h>

extern SAI_HandleTypeDef hsai_BlockA1;
#define AUDIO_SAI_HANDLE   (&hsai_BlockA1)

#define SAMPLE_RATE_HZ     48000U
#define TONE_FREQ_HZ       1000U
#define FRAMES_PER_HALF    256U
#define CH                 2U
#define BUF_HALFWORDS      (FRAMES_PER_HALF * CH * 2U)
#define AMPLITUDE          12000

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/*
 * DMAバッファ: DTCMは不可。AXI SRAM(D1)等に置く(CubeIDE標準の.bssはAXI SRAM)。
 * CM4からSAIを使う場合はD2のSRAM(SRAM1/2)に置くこと。
 */
static int16_t s_audio_buf[BUF_HALFWORDS] __attribute__((aligned(32)));

static float s_phase = 0.0f;
static const float s_phase_inc =
    2.0f * (float)M_PI * (float)TONE_FREQ_HZ / (float)SAMPLE_RATE_HZ;

static void fill_half(uint32_t half)
{
    int16_t *p = &s_audio_buf[half * FRAMES_PER_HALF * CH];

    for (uint32_t i = 0; i < FRAMES_PER_HALF; i++) {
        int16_t v = (int16_t)(sinf(s_phase) * AMPLITUDE);
        *p++ = v;   /* L */
        *p++ = v;   /* R */
        s_phase += s_phase_inc;
        if (s_phase >= 2.0f * (float)M_PI) {
            s_phase -= 2.0f * (float)M_PI;
        }
    }

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_CleanDCache_by_Addr((uint32_t *)&s_audio_buf[half * FRAMES_PER_HALF * CH],
                            FRAMES_PER_HALF * CH * sizeof(int16_t));
#endif
}

void Audio_Start(void)
{
    s_phase = 0.0f;
    memset(s_audio_buf, 0, sizeof(s_audio_buf));
    fill_half(0);
    fill_half(1);

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_CleanDCache_by_Addr((uint32_t *)s_audio_buf, sizeof(s_audio_buf));
#endif

    /* Sizeはデータ個数(16bitなのでハーフワード数) */
    if (HAL_SAI_Transmit_DMA(AUDIO_SAI_HANDLE,
                             (uint8_t *)s_audio_buf,
                             BUF_HALFWORDS) != HAL_OK) {
        Error_Handler();
    }
}

void Audio_Stop(void)
{
    HAL_SAI_DMAStop(AUDIO_SAI_HANDLE);
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai->Instance == AUDIO_SAI_HANDLE->Instance) {
        fill_half(0);
    }
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai->Instance == AUDIO_SAI_HANDLE->Instance) {
        fill_half(1);
    }
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
    (void)hsai;
    /* アンダーラン等。必要なら再起動やログ出力 */
}

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
    volatile int a = Pcm5102a_PlayTest();

    /*
    * LCD initialization
    */
    ST7789_Init();

    ST7789_FillScreen(ST7789_RED);
    HAL_Delay(1000);


    /*
    * Green
    */
    ST7789_FillScreen(ST7789_GREEN);
    //  HAL_Delay(1000);

    /*
    * Blue
    */
    ST7789_FillScreen(ST7789_BLUE);
    //  HAL_Delay(1000);

    /*
    * Black
    */
    ST7789_FillScreen(ST7789_BLACK);

    /*
    * Draw some rectangles
    */
    ST7789_FillRect(
        10, 10,
        100, 50,
        ST7789_RED
    );

    ST7789_FillRect(
        110, 70,
        100, 50,
        ST7789_GREEN
    );

    ST7789_FillRect(
        50, 140,
        220, 50,
        ST7789_BLUE
    );

    HAL_Delay(1000);


    for (int i = 0; i < 5; i++) {
        LCD_WriteCommand(0x21);  /* INVON */
        HAL_Delay(500);
        LCD_WriteCommand(0x20);  /* INVOFF */
        HAL_Delay(500);
    }

    Audio_Start();

    while (1) {
        HAL_GPIO_WritePin(VDD33_ON_GPIO_Port, VDD33_ON_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(VDD50_ON_GPIO_Port, VDD50_ON_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
        HAL_Delay(5000);

        HAL_GPIO_WritePin(VDD33_ON_GPIO_Port, VDD33_ON_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(VDD50_ON_GPIO_Port, VDD50_ON_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET);
        HAL_Delay(5000);
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
