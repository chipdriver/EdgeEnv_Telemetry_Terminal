#ifndef __DHT11_DRIVERS_H
#define __DHT11_DRIVERS_H
/********************************************
 *          包含头文件
 *******************************************/
#include "dht11_gpio.h"
#include <stdbool.h>
#include <stddef.h>
/***********************************
 *          宏定义
 **********************************/
#define DHT11_LOW()  GPIO_ResetBits(DHT11_GPIO_PORT, DHT11_GPIO_PIN)   //拉低DHT11引脚
#define DHT11_HIGH() GPIO_SetBits(DHT11_GPIO_PORT, DHT11_GPIO_PIN)     //拉高DHT11引脚,释放总线
#define DHT11_READ() GPIO_ReadInputDataBit(DHT11_GPIO_PORT, DHT11_GPIO_PIN) //读取DHT11引脚状态

/***********************************
 *       函数声明
 **********************************/
void DHT11_Init(void);
bool DHT11_Read(int *humidity, int *temperature);

#endif /* __DHT11_DRIVERS_H */