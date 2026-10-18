/*
 * app.c
 *
 *  Created on: 2026/10/19
 *      Author: sata
 */

#include "../Inc/main.h"

typedef enum {
    LED0,
    LED1,
    LED2,
    LED3
} LED_TypeDef;


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


int App(void)
{
    int i;

    // 3秒点灯、点滅、消灯
    for (i = LED0; i <= LED3; i++) {
        LED_On(i);
    }
    HAL_Delay(3000);
    for (int j = 0; j < 10; j++) {
        for (i = LED0; i <= LED3; i++) {
            LED_Toggle(i);
        }
        HAL_Delay(50);
    }

    while (1) {
        for (i = LED0; i <= LED3; i++) {
            LED_Toggle(i);
        }
        HAL_Delay(500);
    }
    return 0;
}
