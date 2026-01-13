#include "main.h"

static uint16_t test_img[20*20];

int main(void)
{
	int humidity = 0;
	int temperature = 0;
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
		uint8_t td = FT6336_ReadReg(0x02);
    	printf("TD = %d\r\n", td);

		if(td)
		{
			uint8_t buf[6];
			FT6336_ReadMulti(0x03, buf, 6);

			printf("RAW: %02X %02X %02X %02X %02X %02X\r\n",
				buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
		}

		DWT_Delay_ms(200);
		
		// uint8_t td = FT6336_ReadReg(0x02);
		// DWT_Delay_ms(5);
		// if(td)
		// {
		// 		uint8_t buf[6];
		// 		FT6336_ReadMulti(0x03, buf, 6);

		// 		printf("RAW: %02X %02X %02X %02X %02X %02X\r\n",
		// 					buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
		// }
	}
}
	
