#ifndef DHT11_GPIO_H
#define DHT11_GPIO_H

#include "stm32f4xx.h"

/**********************************************
 *              宏定义
 **********************************************/
#define DHT11_GPIO_PORT        GPIOA                   //DHT11所连接的GPIO端口
#define DHT11_GPIO_PIN         GPIO_Pin_1              //DHT11所连接的
/**********************************************
 *              函数声明
 **********************************************/
void DHT11_GPIO_Init(void);//DHT11 GPIO初始化函数声明 


#endif /* DHT11_GPIO_H */
