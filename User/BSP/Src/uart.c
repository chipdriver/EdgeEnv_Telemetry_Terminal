#include "uart.h"

/**
 * @brief USART1初始化
 * @note 
 *  PA9(TX)   PA10(RX)    115200  8N1
 */
void UART1_Init(void)
{
    //结构体
    GPIO_InitTypeDef GPIO_InitStruct    = { 0 };            //IO配置结构体
    USART_InitTypeDef USART_InitStruct  = { 0 };            //USART1配置结构体

    //1.时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);    //GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);   //USART1时钟    

    //2.PA9/PA10 复用为 USART1（AF7）
    GPIO_InitStruct.GPIO_Pin    = GPIO_Pin_9 | GPIO_Pin_10;     //引脚配置
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_AF;                 //复用模式
    GPIO_InitStruct.GPIO_Speed  = GPIO_Speed_50MHz;             //输出速度
    GPIO_InitStruct.GPIO_OType  = GPIO_OType_PP;                //推挽输出
    GPIO_InitStruct.GPIO_PuPd   = GPIO_PuPd_UP;                 //上拉
    GPIO_Init(GPIOA,&GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);     // PA9 复用为 USART1_TX（AF7）
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);    // PA10 复用为 USART1_RX（AF7）

    //3.USART配置
    USART_InitStruct.USART_BaudRate             = 115200;                           //波特率
    USART_InitStruct.USART_WordLength           = USART_WordLength_8b;              //数据位长度：8位
    USART_InitStruct.USART_StopBits             = USART_StopBits_1;                 //停止位：1位
    USART_InitStruct.USART_Parity               = USART_Parity_No;                  //校验位：无校验
    USART_InitStruct.USART_HardwareFlowControl  = USART_HardwareFlowControl_None;   //硬件流控：无（不使用RTS/CTS）
    USART_InitStruct.USART_Mode                 = USART_Mode_Tx | USART_Mode_Rx;    //工作模式：发送 + 接收
    USART_Init(USART1,&USART_InitStruct);                                           //初始化USART1外设寄存器

    //4.使能USART
    USART_Cmd(USART1,ENABLE);       //使能USART

}

/**
 * @brief   发送1字节（阻塞）
 * @param   b发送的8位数据
 */
void UART1_SendByte(uint8_t b)
{
    USART_SendData(USART1,b);                                       // 将 1 字节写入 USART1 数据寄存器（DR），开始发送
    
    //等待发送数据寄存器空(TXE = 1)
    //TXE = 1 表示：DR 已经被移入移位寄存器，新的数据可以继续写入DR
    //注意：TXE = 1 不代表“这一字节已经完全发出到线上”，只代表“可以写下一个字节”
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);     
}

/**
 * @brief 发送字符串（阻塞方式，通过 USART1）
 * @param s 要发送的字符串首地址（以 '\0' 结尾的字符数组）
 */
void UART1_SendString(const char *s)
{
    while(*s)                           //只要当前字符不是'\0',就继续发送
    {
        UART1_SendByte((uint8_t)*s++); //取出当前字符发送 -> 然后指针s自增，指向下一个字符
    }
}

int fputc(int ch, FILE *f)
{
    USART_SendData(USART1,(uint8_t)ch);
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
    return ch;
}