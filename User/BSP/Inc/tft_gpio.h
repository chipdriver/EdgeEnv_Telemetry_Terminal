#ifndef __TFT_GPIO_H__
#define __TFT_GPIO_H__
/****************************************
 *              头文件引用
 ****************************************/
#include "stm32f4xx.h"

/****************************************
 *              宏定义
 ****************************************/

 //LCD控制引脚定义
#define LCD_CS_GPIO_PORT        GPIOA           //CS
#define LCD_CS_GPIO_PIN         GPIO_Pin_4
#define LCD_DC_GPIO_PORT        GPIOB           //DC
#define LCD_DC_GPIO_PIN         GPIO_Pin_1
#define LCD_RST_GPIO_PORT       GPIOB           //RESET
#define LCD_RST_GPIO_PIN        GPIO_Pin_0
#define LCD_LED_GPIO_PORT       GPIOB           //LED  背光
#define LCD_LED_GPIO_PIN        GPIO_Pin_10

 //TP（FT6336U）控制脚
#define TP_RST_GPIO_PORT       GPIOB           //RESET
#define TP_RST_GPIO_PIN        GPIO_Pin_2
#define TP_INT_GPIO_PORT       GPIOB           //INT
#define TP_INT_GPIO_PIN        GPIO_Pin_12

 //GPIO操作宏定义
#define LCD_CS_LOW()        GPIO_ResetBits(LCD_CS_GPIO_PORT,LCD_CS_GPIO_PIN)     //CS=0
#define LCD_CS_HIGH()       GPIO_SetBits(LCD_CS_GPIO_PORT,LCD_CS_GPIO_PIN)       //CS=1

#define LCD_DC_LOW()        GPIO_ResetBits(LCD_DC_GPIO_PORT,LCD_DC_GPIO_PIN)     //DC=0
#define LCD_DC_HIGH()       GPIO_SetBits(LCD_DC_GPIO_PORT,LCD_DC_GPIO_PIN)       //DC=1

#define LCD_RST_LOW()       GPIO_ResetBits(LCD_RST_GPIO_PORT,LCD_RST_GPIO_PIN)   //RESET=0
#define LCD_RST_HIGH()      GPIO_SetBits(LCD_RST_GPIO_PORT,LCD_RST_GPIO_PIN)     //RESET=1

#define LCD_LED_LOW()      GPIO_ResetBits(LCD_LED_GPIO_PORT,LCD_LED_GPIO_PIN)   //LED=0
#define LCD_LED_HIGH()     GPIO_SetBits(LCD_LED_GPIO_PORT,LCD_LED_GPIO_PIN)     //LED=1

#define TP_RST_LOW()       GPIO_ResetBits(TP_RST_GPIO_PORT,TP_RST_GPIO_PIN)     //RESET=0
#define TP_RST_HIGH()      GPIO_SetBits(TP_RST_GPIO_PORT,TP_RST_GPIO_PIN)       //RESET=1

#define TP_INT_READ()      GPIO_ReadInputDataBit(TP_INT_GPIO_PORT,TP_INT_GPIO_PIN) //读取INT状态

/****************************************
 *              函数声明
 ****************************************/
void TFT_GPIO_Init(void);
#endif /*__TFT_GPIO_H__*/