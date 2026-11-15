/*
 * dac.c
 *
 *  Created on: 2026/11/16
 *      Author: sata
 */

/*
 * main.c の USER CODE 0 にある「pcm5102a_test.c」ブロック
 * (#include <math.h> ～ Pcm5102a_PlayTest の終わりまで) を、これに丸ごと置き換える。
 *
 * 変更点
 *  - CubeMXが生成済みの hsai_BlockA1 を再利用 (ピン設定はCubeMXのMspInitに任せる)
 *  - 自作ハンドル hsai_a と、推測で書いていた PE4/5/6 のGPIO設定を削除
 *  - クロックが来ていない場合に固まらないようタイムアウト付き (戻り値 -1)
 *  - USE_I2S_CKIN=0 で内部PLL1Q(100MHz)に切り替え、原因の切り分けができる
 */
#include <math.h>
#include <stdint.h>
#include "main.h"
#include "dac.h"

extern SAI_HandleTypeDef hsai_BlockA1;


#define USE_I2S_CKIN     1              /* 1: I2S_CKIN(12.288MHz)  0: 内部PLL1Q(切り分け用) */

/* DIN(SAI1_SD_A) をつないだピン。候補: PE6 / PD6 / PC1 / PB2 (すべてAF6)。実配線に合わせる */
#define SAI_SD_PORT      GPIOE
#define SAI_SD_PIN       GPIO_PIN_6

#define FS_HZ            48000U
#define TONE_HZ          480U           /* FS_HZ / TONE_HZ は整数になること */
#define PLAY_SECONDS     5U
#define AMPLITUDE        20000

#define FRAMES_PER_PERIOD (FS_HZ / TONE_HZ)

static int16_t sine_tbl[FRAMES_PER_PERIOD];


/********************************************//**
 * @brief	PCM5102用SAI初期化
 *
 ***********************************************/
static void Pcm5102a_SaiReinit(void)
{
    RCC_PeriphCLKInitTypeDef pclk = {0};
    GPIO_InitTypeDef gpio = {0};

    HAL_SAI_DeInit(&hsai_BlockA1);      /* CubeMXがPLL前提で初期化済みなので一旦戻す */

    /* SAI1-A のピンをAFに設定 (このプロジェクトのMspInitにはSAI用のGPIO設定が無い) */
    __HAL_RCC_SAI1_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF6_SAI1;
    gpio.Pin = GPIO_PIN_5;  HAL_GPIO_Init(GPIOE, &gpio);   /* SAI1_SCK_A = BCK  (PE5) */
    gpio.Pin = GPIO_PIN_4;  HAL_GPIO_Init(GPIOE, &gpio);   /* SAI1_FS_A  = LRCK (PE4) */
    gpio.Pin = SAI_SD_PIN;  HAL_GPIO_Init(SAI_SD_PORT, &gpio); /* SAI1_SD_A = DIN */

#if USE_I2S_CKIN
    gpio.Pin       = GPIO_PIN_9;        /* PC9 = I2S_CKIN (AF5) */
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = 5U;
    HAL_GPIO_Init(GPIOC, &gpio);

    pclk.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
    pclk.Sai1ClockSelection   = RCC_SAI1CLKSOURCE_PIN;
    hsai_BlockA1.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_MCKDIV;   /* Fs = 12.288M/256 = 48k */
    hsai_BlockA1.Init.Mckdiv         = 0;
#else
    pclk.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
    pclk.Sai1ClockSelection   = RCC_SAI1CLKSOURCE_PLL;               /* PLL1Q = 100MHz */
    hsai_BlockA1.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_48K;      /* HALが分周を計算 (ほぼ48k) */
#endif
    if (HAL_RCCEx_PeriphCLKConfig(&pclk) != HAL_OK) {
        Error_Handler();
    }

    hsai_BlockA1.Init.OutputDrive = SAI_OUTPUTDRIVE_ENABLE;

    /* I2S, 16bitデータ / 32bitスロット (BCK = 64fs), 2スロット */
    if (HAL_SAI_InitProtocol(&hsai_BlockA1, SAI_I2S_STANDARD,
                             SAI_PROTOCOL_DATASIZE_16BITEXTENDED, 2) != HAL_OK) {
        Error_Handler();
    }
}


/********************************************//**
 * @brief	fifo filling
 *
 ***********************************************/
/* FLVL >= 5 : FIFO full */
static inline int fifo_full(void)
{
    return ((hsai_BlockA1.Instance->SR & SAI_xSR_FLVL) >> SAI_xSR_FLVL_Pos) >= 5U;
}

/********************************************//**
 * @brief	PCM5102再生テスト
 * @return  0=再生完了, -1=タイムアウト(SAIが動いていない=クロックが来ていない可能性)
 *
 ***********************************************/
int Pcm5102a_PlayTest(void)
{
    uint32_t i, idx = 0;
    const uint32_t total_words = FS_HZ * PLAY_SECONDS * 2U;
    uint32_t t0;
    int ret = 0;

    for (i = 0; i < FRAMES_PER_PERIOD; i++) {
        sine_tbl[i] = (int16_t)(AMPLITUDE * sinf(2.0f * 3.14159265f * (float)i / (float)FRAMES_PER_PERIOD));
    }

    Pcm5102a_SaiReinit();

    /* 起動時のアンダーランを避けるためFIFOを先に埋める */
    while (!fifo_full()) {
        hsai_BlockA1.Instance->DR = (uint16_t)sine_tbl[(idx >> 1) % FRAMES_PER_PERIOD];
        idx++;
    }

    __HAL_SAI_ENABLE(&hsai_BlockA1);

    t0 = HAL_GetTick();
    while (idx < total_words) {
        if (!fifo_full()) {
            hsai_BlockA1.Instance->DR = (uint16_t)sine_tbl[(idx >> 1) % FRAMES_PER_PERIOD];
            idx++;
        }
        if ((HAL_GetTick() - t0) > (PLAY_SECONDS * 1000U + 500U)) {
            ret = -1;                   /* 想定時間内に送り終わらない */
            break;
        }
    }

    if (ret == 0) {
        while ((hsai_BlockA1.Instance->SR & SAI_xSR_FLVL) != 0U) {
            if ((HAL_GetTick() - t0) > (PLAY_SECONDS * 1000U + 1000U)) { ret = -1; break; }
        }
        HAL_Delay(2);
    }
    __HAL_SAI_DISABLE(&hsai_BlockA1);
    return ret;
}
