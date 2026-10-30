/*
 * vdd.h
 *
 *  Created on: 2026/10/31
 *      Author: sata
 */

#ifndef BSP_VDD_H_
#define BSP_VDD_H_

typedef enum {
    VDD50 = 1,
    VDD33 = 2
} VDD_TypeDef;

extern void VDD_OutputEnable(VDD_TypeDef vdd);
extern void VDD_OutputDisable(VDD_TypeDef vdd);

#endif /* BSP_VDD_H_ */
