#include "main.h"

static uint16_t test_img[20*20];

int main(void)
{
	int humidity = 0;
	int temperature = 0;
	FT6336_Touch_t t;							//第1个触摸点
	SystemInit();									//初始化系统
	DWT_Delay_Init();							//初始化DWT延时函数
	DHT11_Init();									//初始化DHT11
	TFT_GPIO_Init();							//初始化TFT GPIO
	SPI1_Init_Master(); 					//初始化SPI1主机模式
	ST7789_MinInit();							//ST7789最小初始化
	ST7789_FillColor(0x0000); 		// 黑底
	ST7789_SetRotation(0);				//设置屏幕旋转方向
	UART1_Init();									//串口初始化
	I2C1_Init_ForTouch();					//I2C1初始化
	FT6336_Reset();

	
	while(1)
	{
		/*DHT11_Read(&humidity, &temperature); //读取DHT11数据*/
		if(FT6336_ReadTouch_Filtered(&t))
		{
			printf("x = %u y = %u points = %u\r\n",t.x,t.y,t.points);
		}

		DWT_Delay_ms(20);
	}
}
	
