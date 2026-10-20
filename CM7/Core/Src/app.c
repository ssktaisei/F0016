/*
 * app.c
 *
 *  Created on: 2026/10/19
 *      Author: sata
 */

#include "../Inc/main.h"
#include "../../Drivers/BSP/led.h"

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



/**
 * @brief	main routine
 *
 * @return	int
 */
int App(void)
{
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

    while (1) {
        for (int i = LED0; i <= LED3; i++) {
            LED_Toggle(i);
        }
        HAL_Delay(500);
    }
    return 0;
}
