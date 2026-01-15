#include "ST6336.h"

/**
 * @brief 复位 FT6336（触摸芯片）
 * @note 典型时序：RST 拉低一小段时间，再拉高并等待芯片启动
 */
void FT6336_Reset(void)
{
    TP_RST_LOW();       //复位拉低
    DWT_Delay_ms(10);   //保持10ms
    TP_RST_HIGH();      //释放复位
    DWT_Delay_ms(100);  //等待芯片上电初始化完成
}


/**
 * @brief  读取 FT6336 的 1 个寄存器（单字节读）
 * @param  reg: 目标寄存器地址（FT6336 内部寄存器地址）
 * @return 读到的 1 字节数据
 *
 * @note   通信流程（主机）：
 *         [START] + [ADDR(W)] + [REG] + [ReSTART] + [ADDR(R)] + [DATA] + [STOP]
 *
 *         事件点（SPL事件）：
 *         EV5  : I2C_EVENT_MASTER_MODE_SELECT                   -> START 已发送
 *         EV6  : I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED     -> 地址(W)已发送且ACK，进入发送模式
 *         EV8_2: I2C_EVENT_MASTER_BYTE_TRANSMITTED              -> 一个字节发送完成（REG已发出）
 *         EV6  : I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED        -> 地址(R)已发送且ACK，进入接收模式
 *         EV7  : I2C_EVENT_MASTER_BYTE_RECEIVED                 -> 收到 1 字节数据（RXNE=1）
 */
uint8_t FT6336_ReadReg(uint8_t reg)
{
    uint8_t data;

    /* 0. 等待I2C总线空闲（BUSY=0）
     *    - 如果 BUSY=1，说明总线正在通信或被占用（上一次事务未结束、SDA被拉低等）
     *    - 必须等空闲再发起新的 START，避免异常或卡死
     */
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) == SET);

    /* 1. 产生起始信号 START
     *    - I2C 硬件自动生成 START 波形（SCL高时SDA从高到低）
     *    - 等待 EV5：主机模式选择完成（START已真正发出，SB置位）
     */
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    /* 2. 发送设备地址 + 写方向（W）
     *    - FT6336_ADDR 是 7bit 地址（0x38）
     *    - 线上发送是 8bit 地址字节： (7bit<<1) + R/W
     *    - I2C_Direction_Transmitter 表示写（R/W=0）
     *    - 等待 EV6：地址已发送且收到ACK，进入“主机发送模式”
     */
    I2C_Send7bitAddress(I2C1, FT6336_ADDR << 1, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    /* 3. 发送寄存器地址 reg
     *    - I2C_SendData: 将 1 个字节写入DR，由硬件发送出去
     *    - 这里的 reg 是“从机内部寄存器地址”，告诉从机接下来要读哪个寄存器
     *    - 等待 EV8_2：该字节发送完成（TXE+BTF等组合条件满足）
     */
    I2C_SendData(I2C1, reg);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    /* 4. 重新产生起始信号 ReSTART（重复起始）
     *    - 常见“读寄存器”流程：先用写方向发送寄存器地址，然后不释放总线直接 ReSTART 切换到读
     *    - 等待 EV5：ReSTART 已发送成功
     */
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    /* 5. 发送设备地址 + 读方向（R）
     *    - 仍然是 (7bit<<1)，但方向参数为 Receiver（R/W=1）
     *    - 等待 EV6：地址已发送且ACK，进入“主机接收模式”
     */
    I2C_Send7bitAddress(I2C1, FT6336_ADDR << 1, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    /* 6. 关闭 ACK，准备接收最后 1 字节
     *    - 对于“只读 1 字节”的情况，主机在接收该字节前需要 NACK（不应答）
     *    - SPL做法通常是：在读取前先 DISABLE ACK，使得读到这个字节后硬件自动发送 NACK
     *    - 表示“我只要这一个字节，读完就结束”
     */
    I2C_AcknowledgeConfig(I2C1, DISABLE);

    /* 7. 等待收到 1 字节数据（EV7）
     *    - EV7 表示 RXNE=1：接收寄存器里已有数据可读
     */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));

    /* 8. 读取数据
     *    - 读取 DR：把收到的 1 字节取出来
     */
    data = I2C_ReceiveData(I2C1);

    /* 9. 产生停止信号 STOP
     *    - 释放 I2C 总线，结束本次事务
     */
    I2C_GenerateSTOP(I2C1, ENABLE);

    /* 10. 重新打开 ACK（为下次多字节接收做准备）
     *     - 如果不重新打开，下一次接收可能会异常（一直NACK）
     */
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    return data;
}

/**
 * @brief 连续读取 FT6336 从 reg 开始的 len 个寄存器字节（多字节读）
 * @param reg：起始寄存器地址（FT6336 内部寄存器地址）
 * @param buf：数据缓冲区指针，用于存放读取到的数据
 * @param len：需要读取的字节数（连续 len 个字节）
 * @return None
 * 
 * @note I2C 典型“寄存器连续都”流程（主机）：
 *      [START] + [ADDR(W)] + [REG] + [ResSTART] + [ADDR(R)] + [DATA0...DATASn] + [STOP]
 * 
 *      事件点（SPL事件）：
 *      EV5  ：I2C_EVENT_MASTER_MODE_SELECT                   -> START/ResSTART 已发送
 *      EV6  : I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED     -> 地址(W)已发送且ACK，进入发送模式
 *      EV8_2: I2C_EVENT_MASTER_BYTE_TRANSMITTED              -> 发送 1 字节完成（寄存器地址REG已发出）
 *      EV6  : I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED        -> 地址(R)已发送且ACK，进入接收模式
 *      EV7  : I2C_EVENT_MASTER_BYTE_RECEIVED                 -> 收到 1 字节数据（RXNE=1）
 * @warning 后续要加上“超时机制”，否则某一步没等到时间会while卡死
 */
void FT6336_ReadMulti(uint8_t reg,uint8_t *buf,uint8_t len)
{
    uint8_t i;

    /*  0.等待I2C总线空闲（BUSY = 0)
     *  - BUSY = 1 说明总线被占用，可能上一次通信未STOP，或SDA被外设拉低等
     *  - 必须确保空闲再开始新的事务，避免 START 发不出去或状态机异常
     */
    while(I2C_GetFlagStatus(I2C1,I2C_FLAG_BUSY) == SET);

    /**  1.产生起始信号START
     *      - 硬件自动生成START 波形（SCL高电平时SDA从高->低）
     *      - 等待EV5:主机模式选择完成（START已真正发出，进入Master状态）
     */
    I2C_GenerateSTART(I2C1,ENABLE);
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT));
    
    /** 2.发送设备地址 + 写方向（W）
     *      - FT6336_ADDR为7bit地址（0x38），需要 << 1组成地址字节高7位
     *      - Direction_Transmitter 表示写（R/W=0）
     *      - 等待 EV6：地址已发送且收到ACK，进入“主机发送模式”
     */
    I2C_Send7bitAddress(I2C1,FT6336_ADDR << 1,I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    /**  3.发送起始寄存器地址 reg
     *      - 告诉从机：接下来要从哪个寄存器开始读
     *      - 等待EV8_2：该字节发送完成
     */
    I2C_SendData(I2C1,reg);
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    /**  4.产生重复起始信号ReSTART
     *      - “寄存器读”惯例：先用写把寄存器地址发给从机，然后不释放总线直接 ReSTART 切换到读
     *      - 等待 EV5：ReSTART 已发送成功
     */
    I2C_GenerateSTART(I2C1,ENABLE);
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT));

    /**  5.发送设备地址 + 读方向（R）
     *      - Direction_Receiver 表示读（R/W = 1）
     *      - 等待EV6:地址已发送且收到ACK，进入“主机接收模式”
     */
    I2C_Send7bitAddress(I2C1,FT6336_ADDR << 1,I2C_Direction_Receiver);
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    /**  6.连续接收 len 个字节
     *      - 前 len-1 个字节：保持 ACK=ENABLE，表示“还要继续读”
     *      - 最后 1 个字节  ：在读取前关闭ACK，并发STOP，表示“这是最后一个字节，读完就结束” 
     */
    for(i = 0; i < len ; i++)
    {
        /*如果这是最后一个字节*/
        if( i == (uint8_t)(len - 1))
        {
            /** 最后1字节：关闭ACK -> 硬件将对最后1字节发送NACK，告诉从机停止发送
             *  同时提前产生STOP -> 读完最后字节释放总线
             */
            I2C_AcknowledgeConfig(I2C1,DISABLE);
            I2C_GenerateSTOP(I2C1,ENABLE);
        }

        /*等待EV7:接收寄存器RXNE=1，说明已收到一个字节*/
        while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_RECEIVED));

        /*读取DR：取出收到的字节存放到缓冲区*/
        buf[i] = I2C_ReceiveData(I2C1);
    }

    /**  7.重新打开ACK（位下次多字节接收做准备）
     *      -如果不恢复ACK，下一次读多字节会一直NACK导致异常
     */
    I2C_AcknowledgeConfig(I2C1,ENABLE);
}

/**
 * @brief 读取FT6336第一个触摸点（点数 + X/Y)
 * @param t:输出结构体
 * @return 1=有触摸并成功读取坐标；0=无触摸
 */
uint8_t FT6336_ReadTouch(FT6336_Touch_t *t)
{
    uint8_t td;

    //参数检查
    if(t == 0) return 0;

    // 1.读触摸点数寄存器 0x02(TD_STATUS)
    FT6336_ReadMulti(0x02,&td,1);
    td &= 0x0F; //低4位：点数

    t->points = td;
    t->touched = (td > 0) ? 1: 0;

    if(td == 0)
    {
        t->x = 0;
        t->y = 0;
        return 0;
    }

    // 2.从0x03连续读6个字节（常用：XH,XL,YH,YL+其他信息）
    FT6336_ReadMulti(0x03,t->raw,6);

    // 3.解析 X/Y（高4位常是事件/flag，坐标高位在低4位）
    t->x = ((uint16_t)(t->raw[0] & 0x0F) << 8 ) | t->raw[1];
    t->y = ((uint16_t)(t->raw[2] & 0x0F) << 8 ) | t->raw[3];

    ST7789_DrawPixel(t->x, t->y, 0xFFFF);   // 白点

    return 1;
}

/**
 * @brief 读取FT6336第一个触摸点（点数 + X/Y）并做最小滤波（Step7）
 * @param t: 输出结构体
 * @return 1=有触摸且坐标有效；0=无触摸或本次数据无效/被丢弃
 *
 * @note 这是在 FT6336_ReadTouch() 基础上做“最小改动”的增强版：
 *       - 松手清历史
 *       - 限范围（越界丢弃）
 *       - 防跳点（突变过大丢弃）
 *       - 可选：轻量平滑（更稳，但会略“粘手”）
 */
uint8_t FT6336_ReadTouch_Filtered(FT6336_Touch_t *t)
{
    // --- 参数检查 ---
    if(t == 0) return 0;

    // --- 状态历史（用于防跳点/平滑）
    static uint16_t last_x = 0, last_y = 0;
    static uint8_t  has_last = 0;

    // 跳点阈值（可调：50~120）
    const uint16_t JUMP_TH = 80;

    // 是否启用平滑（1=开，0=关）
    const uint8_t  ENABLE_SMOOTH = 1;

    uint8_t td = 0;

    // 1) 读触摸点数寄存器 0x02(TD_STATUS)
    FT6336_ReadMulti(0x02, &td, 1);
    td &= 0x0F; // 若确认低4位是点数就保留；
    t->points  = td;
    t->touched = (td > 0) ? 1 : 0;

    if(td == 0)
    {
        // 松手：清历史，避免下一次触摸被上次 last_x/last_y 影响
        has_last = 0;

        t->x = 0;
        t->y = 0;
        return 0;
    }

    // 2) 从0x03连续读6个字节（XH,XL,YH,YL + 其他信息）
    FT6336_ReadMulti(0x03, t->raw, 6);

    // 3) 解析 X/Y（高4位常是事件/flag，坐标高位在低4位）
    uint16_t nx = ((uint16_t)(t->raw[0] & 0x0F) << 8) | t->raw[1];
    uint16_t ny = ((uint16_t)(t->raw[2] & 0x0F) << 8) | t->raw[3];

    // --- Step7-A：限范围（越界丢弃）---
    if(nx >= LCD_W || ny >= LCD_H)
    {
        return 0;
    }

    // --- Step7-B：防跳点（突变过大丢弃）---
    if(has_last)
    {
        uint16_t dx = (nx > last_x) ? (nx - last_x) : (last_x - nx);
        uint16_t dy = (ny > last_y) ? (ny - last_y) : (last_y - ny);

        if((uint16_t)(dx + dy) > JUMP_TH)
        {
            return 0;
        }

        // --- Step7-C：可选平滑（更稳，略“粘手”）---
        if(ENABLE_SMOOTH)
        {
            // 3/4旧 + 1/4新（简单 IIR）
            nx = (uint16_t)((last_x * 3 + nx) / 4);
            ny = (uint16_t)((last_y * 3 + ny) / 4);
        }
    }

    // 更新历史
    last_x = nx;
    last_y = ny;
    has_last = 1;

    // 写回输出
    t->x = nx;
    t->y = ny;

    // 可视化验证：你要是想保持“驱动层纯净”，可以用宏包起来
    ST7789_DrawPixel(t->x, t->y, 0xFFFF);

    return 1;
}

