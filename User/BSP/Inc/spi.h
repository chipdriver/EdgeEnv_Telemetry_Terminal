#ifndef __SPI_H_
#define __SPI_H_

/*************************************
 *          头文件
 ************************************/
#include "tft_gpio.h"

/*************************************
 *          宏定义
 ************************************/
#define SPIx            SPI1                        //使用SPI1
#define SPIx_CLK        RCC_APB2Periph_SPI1         //SPI1时钟

#define SPIx_GPIO       GPIOA                       //SPI1 GPIO端口
#define SPIx_GPIO_CLK   RCC_AHB1Periph_GPIOA        //SPI1 GPIO时钟

#define SPIx_SCK_PIN    GPIO_PIN_5                  //SPI1 SCK引脚
#define SPIx_MISO_PIN   GPIO_PIN_6                  //SPI1 MISO引脚
#define SPIx_MOSI_PIN   GPIO_PIN_7                  //SPI1 MOSI引脚

#define SPIx_AF        GPIO_AF_SPI1                //SPI1 复用功能
/*************************************
 *          SPI函数声明
 ************************************/

#endif /*__SPI_H_*/


