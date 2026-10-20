/*
 * led.c
 *
 *  Created on: 2026/10/21
 *      Author: sata
 */

#include "stm32h7xx_hal.h"
#include "led.h"
#include "../../Core/Inc/main.h"

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
