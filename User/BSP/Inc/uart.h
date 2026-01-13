#ifndef __UART_H__
#define __UART_H__
/**
 * 头文件
 */
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stdio.h"

/**
 * 函数声明
 */
void UART1_Init(void);                  //UART1初始化
void UART1_SendByte(uint8_t b);         //发送一个字节
void UART1_SendString(const char *s);   //发送字符串


#endif 