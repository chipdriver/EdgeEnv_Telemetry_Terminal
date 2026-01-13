#include "i2c.h"

/**
 * @brief I2C1 初始化 （PB6 = SCL,PB7 = SDC,SPL）
 *  用于FT6336U 这类I2C触摸芯片
 */
void I2C1_Init_ForTouch(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    I2C_InitTypeDef I2C_InitStruct;

    //1.开时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);

    //2.配置PB6/PB7为I2C复用功能AF4
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;//复用功能
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;//开漏输出
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;//上拉，通常模块也有上拉，开着更稳
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init(GPIOB,&GPIO_InitStruct);

    //3.绑定AF：PB6->I2C1_SCL，PB7->I2C1_SDA
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource7,GPIO_AF_I2C1);

    //4. /* 4) 先 DeInit，再 Init（避免之前状态残留） */
    I2C_DeInit(I2C1);

    //5.I2C参数
    //F4的SPL里ClockSpeed 仍然有效（内部会结合PCLK1）
    I2C_InitStruct.I2C_ClockSpeed               = 100000;           //100khz先稳
    I2C_InitStruct.I2C_Mode                     = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle                = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1              = 0x00;             //主机模式随便
    I2C_InitStruct.I2C_Ack                      = I2C_Ack_Enable;   //读取需要ACK
    I2C_InitStruct.I2C_AcknowledgedAddress      = I2C_AcknowledgedAddress_7bit;

    I2C_Init(I2C1,&I2C_InitStruct);

    //6.使能 I2C1
    I2C_Cmd(I2C1,ENABLE);
}

/**
 * @brief 测试ACK
 * @param I2C I2C 外设指针
 * @param devbit 设备的7位地址
 * @return true 该地址有设备应答（ACK）
 * @return flase 没有设备应答（NACK）或者超时
 * @note   这个函数只做一件事：发START + 地址，看从机是否ACK
 */
static bool I2C_TestACK(I2C_TypeDef *I2Cx,uint8_t dev7bit) 
{
    /*-------------------- 1) 等待总线空闲 --------------------*/
    //I2C_FLAG_BUSY = 1 表示总线忙（可能正在通信/或被卡住）
    while(I2C_GetFlagStatus(I2Cx,I2C_FLAG_BUSY) == SET );

    /*-------------------- 2) 发送 START --------------------*/
    //产生起始信号（START）
    I2C_GenerateSTART(I2Cx,ENABLE);

    //等待进入主机模式
    while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_MODE_SELECT));

    /*-------------------- 3) 发送 7 位地址 + 写方向 --------------------*/
    //只要从机会 ACK，这里就会进入“已选中从机”的状态
    //注意：SPL 这里传入的是7位地址（dev7bit<<1 在库内部处理）
    I2C_Send7bitAddress(I2Cx,(uint8_t)(dev7bit << 1),I2C_Direction_Transmitter);

    /*-------------------- 4) 等待 ACK 或 NACK --------------------*/
    //SR1的两个关键标志：
    // - ADDR：地址发送并收到ACK（说明设备存在）
    // - AF  : 应答失败（NACK）
    //加一个简单超时，避免死等（比如总线没接，设备不在）
    uint32_t timeout = 50000;

    while(((I2Cx->SR1 & (I2C_SR1_ADDR | I2C_SR1_AF)) == 0 ) && (--timeout));

    //如果收到了ACK->ADDR置位
    if( I2Cx->SR1 & I2C_SR1_ADDR)
    {
        //清ADDR的标准做法：先读SR1，再读SR2
        //（这是I2C硬件要求，不清会卡状态机）
        (void)I2Cx->SR1;
        (void)I2Cx->SR2;

        //发送STOP,释放总线
        I2C_GenerateSTOP(I2Cx,ENABLE);

        return true;
    }

    //如果收到了NACK -> AF 置位 
    if( I2Cx->SR1 & I2C_SR1_AF)
    {
        //清AF标志，否则下次通信会异常
        I2Cx->SR1 &= (uint16_t)~I2C_SR1_AF; 
    }

    //无论NACK/超时，都发送STOP释放总线
    I2C_GenerateSTOP(I2Cx,ENABLE);

    return false;   //没有设备或总线异常
}

/**
 * @brief 扫描I2C总线上的所有设备地址，并打印出来
 * @param I2Cx I2C 外设指针（I2C1/I2C2/I2C3）
 * @note  I2C的7位地址范围通常是 0x08~0x77
 */
void I2C_ScanBus(I2C_TypeDef *I2Cx)
{
    //打印扫描开始提示
     printf("\r\n[I2C] Scan start...\r\n");

    // 遍历所有可能的 7 位地址（保留地址通常不扫）
    for (uint8_t addr = 0x08; addr <= 0x77; addr++)
    {
        // 对每个地址发一次“地址探测”
        if (I2C_TestACK(I2Cx, addr))
        {
            // 若 ACK，则打印该地址
            printf("[I2C] Found device at 0x%02X\r\n", addr);
        }
    }

    //扫描完成提示
    printf("[I2C] Scan done.\r\n");
}