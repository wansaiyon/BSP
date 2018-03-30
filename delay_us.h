/*
 * delay_us.h
 *
 *  Created on: 2017Äê8ÔÂ20ÈÕ
 *      Author: wansaiyon
 */

#ifndef _DELAY_US_H_
#define _DELAY_US_H_

#include "stm32f4xx_hal.h"


void delay_init(void);
void delay_us(uint32_t nus);
void delay_ms(uint16_t nms);
void delay_s(uint16_t ns);

#endif /* DRIVERS_BSP_DELAY_US_DELAY_US_H_ */
