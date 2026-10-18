/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define OSC32_ENABLE_Pin GPIO_PIN_15
#define OSC32_ENABLE_GPIO_Port GPIOC
#define OSC_ENABLE_Pin GPIO_PIN_1
#define OSC_ENABLE_GPIO_Port GPIOH
#define MCU_EXRST_Pin GPIO_PIN_3
#define MCU_EXRST_GPIO_Port GPIOA
#define LCD_NCS_Pin GPIO_PIN_4
#define LCD_NCS_GPIO_Port GPIOC
#define LCD_RST_Pin GPIO_PIN_5
#define LCD_RST_GPIO_Port GPIOC
#define LCD_BL_Pin GPIO_PIN_0
#define LCD_BL_GPIO_Port GPIOB
#define LCD_DC_Pin GPIO_PIN_1
#define LCD_DC_GPIO_Port GPIOB
#define LCD_SDCS_Pin GPIO_PIN_2
#define LCD_SDCS_GPIO_Port GPIOB
#define SR_NCLR_Pin GPIO_PIN_14
#define SR_NCLR_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_11
#define LED2_GPIO_Port GPIOD
#define LED3_Pin GPIO_PIN_12
#define LED3_GPIO_Port GPIOD
#define LED0_Pin GPIO_PIN_2
#define LED0_GPIO_Port GPIOG
#define LED1_Pin GPIO_PIN_3
#define LED1_GPIO_Port GPIOG
#define DAC_NSMT_Pin GPIO_PIN_6
#define DAC_NSMT_GPIO_Port GPIOC
#define DAC_FMT_Pin GPIO_PIN_7
#define DAC_FMT_GPIO_Port GPIOC
#define OSC12M_EN_Pin GPIO_PIN_8
#define OSC12M_EN_GPIO_Port GPIOC
#define IOX_INT_Pin GPIO_PIN_6
#define IOX_INT_GPIO_Port GPIOD
#define UI_EN_Pin GPIO_PIN_7
#define UI_EN_GPIO_Port GPIOD
#define UI_NCS2_Pin GPIO_PIN_9
#define UI_NCS2_GPIO_Port GPIOG
#define UI_NCS1_Pin GPIO_PIN_10
#define UI_NCS1_GPIO_Port GPIOG
#define UI_NCS0_Pin GPIO_PIN_11
#define UI_NCS0_GPIO_Port GPIOG
#define EEPROM_NWP_Pin GPIO_PIN_5
#define EEPROM_NWP_GPIO_Port GPIOB
#define VDD33_ON_Pin GPIO_PIN_8
#define VDD33_ON_GPIO_Port GPIOB
#define VDD50_ON_Pin GPIO_PIN_9
#define VDD50_ON_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
