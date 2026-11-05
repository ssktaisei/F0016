/*
 * vdd.c
 *
 *  Created on: 2026/10/31
 *      Author: sata
 */

#include "vdd.h"
#include "main.h"


void VDD_OutputEnable(VDD_TypeDef vdd)
{
    switch (vdd) {
    case VDD50:
        HAL_GPIO_WritePin(VDD50_ON_GPIO_Port, VDD50_ON_Pin, GPIO_PIN_SET);
        break;
    case VDD33:
        HAL_GPIO_WritePin(VDD33_ON_GPIO_Port, VDD33_ON_Pin, GPIO_PIN_SET);
        break;
    default:
        break;
    }
}

void VDD_OutputDisable(VDD_TypeDef vdd)
{
    switch (vdd) {
    case VDD50:
        HAL_GPIO_WritePin(VDD50_ON_GPIO_Port, VDD50_ON_Pin, GPIO_PIN_RESET);
        break;
    case VDD33:
        HAL_GPIO_WritePin(VDD33_ON_GPIO_Port, VDD33_ON_Pin, GPIO_PIN_RESET);
        break;
    default:
        break;
    }
}
