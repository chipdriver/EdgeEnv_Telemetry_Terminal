#include "dht11_gpio.h"

void DHT11_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    //1.开启GPIO的时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    //2.配置引脚
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1; //PA1
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT; //输出模式
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; //速度50MHz
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD; //开漏输出
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; //不使用内部上拉
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    //3.初始化GPIO引脚状态
    GPIO_SetBits(GPIOA, GPIO_Pin_1);
}

