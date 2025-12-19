#include "spi.h"

/**
 * @brief  初始化SPI
 */
void SPI1_Init_Master(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    SPI_InitTypeDef SPI_InitStruct = {0};

    //1.使能时钟（GPIOA + SPI1）
    RCC_AHB1PeriphClockCmd(SPIx_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(SPIx_CLK, ENABLE);

    //2.GPIO 配置：PA5/6/7 复用 AF5
    GPIO_PinAFConfig(SPIx_GPIO, SPIx_SCK_PIN, SPIx_AF);
    GPIO_PinAFConfig(SPIx_GPIO, SPIx_MISO_PIN, SPIx_AF);
    GPIO_PinAFConfig(SPIx_GPIO, SPIx_MOSI_PIN, SPIx_AF);

    GPIO_InitStruct.GPIO_Pin = SPIx_SCK_PIN | SPIx_MISO_PIN | SPIx_MOSI_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SPIx_GPIO, &GPIO_InitStruct);

    //3.SPI参数配置
    SPI_I2S_DeInit(SPIx);   //将SPI外设恢复到默认复位状态,清除之前的所有配置
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //双线全双工模式(同时收发)
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;                       //主机模式(STM32作为主控)
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;                   //数据帧长度8位

    //SPI Mode0: 时钟空闲低电平,第1个边沿采样
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;      //时钟极性:空闲时SCK=0
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;    //时钟相位:第1个时钟边沿采样数据

    //NSS：软件管理（CS自己用GPIO控制）
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;

    //分频系数：FPCLK/16   先用慢一点更稳   公式：SPI时钟频率 = APB总线时钟 / 分频系数
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;

    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;   //数据传输从MSB位开始
    SPI_InitStruct.SPI_CRCPolynomial = 7;              //CRC值计算的多项式(本项目未启用CRC,此参数无效)

    SPI_Init(SPIx, &SPI_InitStruct);

    //使能SPI
    SPI_Cmd(SPIx, ENABLE);
}

/**
 * @brief SPI发送一个字节
 */
void SPI_SendByte(uint8_t data)
{
    //等待发送缓冲区空
    while(SPI_I2S_GetFlagStatus(SPIx,SPI_I2S_FLAG_TXE) == RESET);

    //发送一个字节
    SPI_I2S_SendData(SPIx, data);

    //等待总线不忙（确保发送完成）
    while(SPI_I2S_GetFlagStatus(SPIx,SPI_I2S_FLAG_BSY) == SET);
}


/**
 * @brief 发送一段数据
 */
void SPI_SendBuffer(const uint8_t* buf, uint16_t len)
{
    while(len--)
    {
        SPI_SendByte(*buf++);
    }
}

