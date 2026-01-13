#ifndef __I2C_H__
#define __I2C_H__

/**
 *  头文件
 */
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_i2c.h"
#include <stdbool.h>
#include "uart.h"
/**
 * 函数声明
 */
void I2C1_Init_ForTouch(void);
void I2C_ScanBus(); //I2C测通
#endif //__I2C_H__