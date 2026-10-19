/*
 * app.c
 *
 *  Created on: 2026/10/19
 *      Author: sata
 */

#include "../Inc/main.h"

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


typedef enum {
    LED0,
    LED1,
    LED2,
    LED3
} LED_TypeDef;


static void LED_On(LED_TypeDef led);
static void LED_Off(LED_TypeDef led);
static void LED_Toggle(LED_TypeDef led);


/**
 * @brief	LED ON
 *
 * @param	led
 */
void LED_On(LED_TypeDef led)
{
    GPIO_TypeDef *port;
    uint16_t pin;

    switch (led)
    {
    case LED0:
        port = LED0_GPIO_Port;
        pin = LED0_Pin;
        break;
    case LED1:
        port = LED1_GPIO_Port;
        pin = LED1_Pin;
        break;
    case LED2:
        port = LED2_GPIO_Port;
        pin = LED2_Pin;
        break;
    case LED3:
        port = LED3_GPIO_Port;
        pin = LED3_Pin;
        break;
    default:
        return;
    }
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}


/**
 * @brief	LED OFF
 *
 * @param	led
 */
void LED_Off(LED_TypeDef led)
{
    GPIO_TypeDef *port;
    uint16_t pin;

    switch (led)
    {
    case LED0:
        port = LED0_GPIO_Port;
        pin = LED0_Pin;
        break;
    case LED1:
        port = LED1_GPIO_Port;
        pin = LED1_Pin;
        break;
    case LED2:
        port = LED2_GPIO_Port;
        pin = LED2_Pin;
        break;
    case LED3:
        port = LED3_GPIO_Port;
        pin = LED3_Pin;
        break;
    default:
        return;
    }
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}


/**
 * @brief	LED Toggle
 *
 * @param	led
 */
void LED_Toggle(LED_TypeDef led)
{
    GPIO_TypeDef *port;
    uint16_t pin;

    switch (led)
    {
    case LED0:
        port = LED0_GPIO_Port;
        pin = LED0_Pin;
        break;
    case LED1:
        port = LED1_GPIO_Port;
        pin = LED1_Pin;
        break;
    case LED2:
        port = LED2_GPIO_Port;
        pin = LED2_Pin;
        break;
    case LED3:
        port = LED3_GPIO_Port;
        pin = LED3_Pin;
        break;
    default:
        return;
    }
    HAL_GPIO_TogglePin(port, pin);
}


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
