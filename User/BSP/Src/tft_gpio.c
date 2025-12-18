#include "tft_gpio.h"

/**
 * @brief 时钟使能
 */
static void gpio_clock_enable(void)
{
    //使能GPIOA和GPIOB时钟
    RCC_AHB1PeriphClockCmd(
        RCC_AHB1Periph_GPIOA | 
        RCC_AHB1Periph_GPIOB, 
        ENABLE);
}

/**
 * @brief TFT GPIO初始化
 */
void TFT_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    //1.使能时钟
    gpio_clock_enable();

    //2.配置引脚
    // ========== LCD 输出脚：CS/DC/RST/LED ==========
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;          //输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;         //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      //速度50MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;       //无

    //CS
    GPIO_InitStructure.GPIO_Pin = LCD_CS_GPIO_PIN;
    GPIO_Init(LCD_CS_GPIO_PORT, &GPIO_InitStructure);

    //RST
    GPIO_InitStructure.GPIO_Pin = LCD_RST_GPIO_PIN;
    GPIO_Init(LCD_RST_GPIO_PORT, &GPIO_InitStructure);

    //LED
    GPIO_InitStructure.GPIO_Pin = LCD_LED_GPIO_PIN;
    GPIO_Init(LCD_LED_GPIO_PORT, &GPIO_InitStructure);

    //DC
    GPIO_InitStructure.GPIO_Pin = LCD_DC_GPIO_PIN;
    GPIO_Init(LCD_DC_GPIO_PORT, &GPIO_InitStructure);

    //给LCD默认状态
    LCD_CS_HIGH();     //CS=1 未选中
    LCD_DC_HIGH();     //DC=1 默认数据态
    LCD_RST_HIGH();    //RESET=1 不复位
    LCD_LED_HIGH();    //LED=1 背光打开

    // ========== TP 输出脚：RST ==========
    GPIO_InitStructure.GPIO_Pin = TP_RST_GPIO_PIN;
    GPIO_Init(TP_RST_GPIO_PORT, &GPIO_InitStructure);

    //给TP默认状态
    TP_RST_HIGH();    //RESET=1 不复位

    // ========== TP 输入脚：INT ==========
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;           //输入模式
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;          // 上拉:空闲时稳定为高电平
    GPIO_InitStructure.GPIO_Pin = TP_INT_GPIO_PIN;
    GPIO_Init(TP_INT_GPIO_PORT, &GPIO_InitStructure);
}
