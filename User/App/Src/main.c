#include "main.h"

int main(void)
{
	int humidity = 0;
	int temperature = 0;
	SystemInit();	//初始化系统
	DHT11_Init();//初始化DHT11
	

	
	while(1)
	{

		DHT11_Read(&humidity, &temperature); //读取DHT11数据
	}
	
}

