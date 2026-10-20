/*
 * led.h
 *
 *  Created on: 2026/10/21
 *      Author: sata
 */

#ifndef BSP_LED_H_
#define BSP_LED_H_

#include "stm32h7xx_hal.h"


typedef enum {
    LED0,
    LED1,
    LED2,
    LED3
} LED_TypeDef;


extern void LED_On(LED_TypeDef led);
extern void LED_Off(LED_TypeDef led);
extern void LED_Toggle(LED_TypeDef led);

#endif /* BSP_LED_H_ */
