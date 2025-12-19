#include "st7789.h"

/**
 * @brief 写命令
 * @param cmd 命令
 */
void LCD_WriteCmd(uint8_t cmd)
{
    LCD_CS_LOW();   //选中屏
    LCD_DC_LOW();   //命令模式
    SPI_SendByte(cmd); //发送命令
    LCD_CS_HIGH();  //取消选中屏
}

/**
 * @brief 写1字节数据
 * @param data 数据
 */
void LCD_WriteData8(uint8_t data)
{
    LCD_CS_LOW();   //选中屏
    LCD_DC_HIGH();  //数据模式
    SPI_SendByte(data); //发送数据
    LCD_CS_HIGH();  //取消选中屏
}

/**
 * @brief 写多个字节数据
 * @param buf 数据指针
 * @param len 数据大小
 */
void LCD_WriteDataBuf(uint8_t *buf, uint32_t len)
{
    LCD_CS_LOW();       //选中屏
    LCD_DC_HIGH();      //数据模式
    SPI_SendBuffer(buf, len); //发送数据
    LCD_CS_HIGH();      //取消选中屏
}

/**
 * @brief 复位LCD屏幕
 * @note 通过RST引脚硬件复位ST7789
 *       复位时序: 高→低(20ms)→高(120ms)
 */
void LCD_Reset(void)
{
    // 1. 拉高RST引脚 (确保初始状态)
    LCD_RST_HIGH();
    DWT_Delay_ms(1);        // 稳定1ms

    // 2. 拉低RST引脚 (触发复位)
    LCD_RST_LOW();
    DWT_Delay_ms(20);       // 保持低电平20ms (复位脉冲)

    // 3. 拉高RST引脚 (退出复位状态)
    LCD_RST_HIGH();
    DWT_Delay_ms(120);      // 等待120ms让LCD完成内部初始化
}


