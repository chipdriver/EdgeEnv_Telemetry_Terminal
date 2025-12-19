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

#define SPIx_SCK_PIN    GPIO_Pin_5                  //SPI1 SCK引脚
#define SPIx_MISO_PIN   GPIO_Pin_6                  //SPI1 MISO引脚
#define SPIx_MOSI_PIN   GPIO_Pin_7                  //SPI1 MOSI引脚

#define SPIx_AF        GPIO_AF_SPI1                //SPI1 复用功能
/*************************************
 *          SPI函数声明
 ************************************/
void SPI1_Init_Master(void); //初始化SPI1主机模式
void SPI_SendByte(uint8_t data);   //通过SPI发送一个字节数据
void SPI_SendBuffer(const uint8_t* buf, uint16_t len); //通过SPI发送一组字节数据

#endif /*__SPI_H_*/


