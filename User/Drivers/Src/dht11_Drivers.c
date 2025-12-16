#include "dht11_Drivers.h"

/**
 * @brief  设置DHT11引脚模式为输出模式
 */
void DHT11_SetPinOutput(void)
{
    //1.配置IO
    GPIO_InitTypeDef    GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = DHT11_GPIO_PIN;      //引脚
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;       //设置为输出模式
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;      //推挽输出
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;   //速度
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;    //无上下拉
    GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);

    //2.设置引脚输出高电平
    GPIO_SetBits(DHT11_GPIO_PORT, DHT11_GPIO_PIN);//（开漏下写1=放开，让上拉拉高）

}

 /**
 * @brief  设置DHT11引脚模式为输入模式
 */
void DHT11_SetPinInput(void)
{
    //1.配置IO
    GPIO_InitTypeDef    GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = DHT11_GPIO_PIN;      //引脚
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;        //设置为输入模式
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;    //无上下拉
    GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

/**
 * @brief 延时函数初始化DWT用于延时
 */
void DWT_Delay_Init(void) {
    // 检查跟踪使能位是否已设置
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
        // 启用DWT和ITM单元的跟踪功能
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        // 复位循环计数器
        DWT->CYCCNT = 0;
        // 启用循环计数器
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

/**
 * @brief 微秒级精确延时函数
 * @param us 延时时间，单位为微秒(μs)
 * @return 无
 */
void DWT_Delay_us(uint32_t us) {
    uint32_t startTick = DWT->CYCCNT;                    // 记录起始时刻的计数值
    uint32_t delayTicks = us * (SystemCoreClock / 1000000);  // 计算需要的时钟周期数
    // 等待直到经过足够的时钟周期
    while ((DWT->CYCCNT - startTick) < delayTicks);
}

/**
 * @brief 毫秒级精确延时函数
 * @param ms 延时时间，单位为毫秒(ms)
 * @return 无
 */
void DWT_Delay_ms(uint32_t ms) {
    uint32_t startTick = DWT->CYCCNT;                    // 记录起始时刻的计数值
    uint32_t delayTicks = ms * (SystemCoreClock / 1000);  // 计算需要的时钟周期数
    // 等待直到经过足够的时钟周期
    while ((DWT->CYCCNT - startTick) < delayTicks);
}

/**
 * @brief 发送DHT11起始信号
 */
void DHT11_Start(void)
{
    //设置引脚为输出模式
    DHT11_SetPinOutput();

    //拉低引脚至少18ms
    DHT11_LOW();
    DWT_Delay_ms(18);

    //拉高引脚20~40us
    DHT11_HIGH();
    DWT_Delay_us(30);

    //设置引脚为输入模式，等待DHT11响应
    DHT11_SetPinInput();
}

/**
 * @brief 检查DHT11响应信号
 */
uint8_t DHT11_CheckResponse(void)
{
    uint32_t time = 0;

    //1) 等待从高变低（DHT11拉低约80us）
    time = 0;
    while(DHT11_READ() == 1)
    {
        if(++time > 100) //等待100us后超时
        {
            return 1; //无响应
        }
        DWT_Delay_us(1);
    }

    //2) 等待从低变高（DHT11拉高约80us）
    time = 0;
    while(DHT11_READ() == 0)
    {
        if(++time > 100) //等待100us后超时
        {
            return 2; //无响应
        }
        DWT_Delay_us(1);
    }

    //3) 等待从高变低，准备接收数据
    time = 0;
    while (DHT11_READ() == 1)
    {
        if (++time > 100) //等待100us后超时
        {
            return 3; //无响应
        }
        DWT_Delay_us(1);
    }

    return 0; //响应成功
}

/**
 * @brief 读取DHT11发送的单个数据位
 * @return bool 读取到的位值
 */
static bool DHT11_ReadBit(void)
{
    uint32_t time = 0;  //超时计数器

    // 1)等待每位数据开始前的低电平信号结束
    time = 0;
    while(DHT11_READ() == 0)
    {
        if(++time > 100) //等待100us后超时
        {
            return 0; //超时，返回0
        }
        DWT_Delay_us(1);
    }

    // 2)等待高电平信号结束，同时计算持续时间
    time = 0;
    while (DHT11_READ() == 1)
    {
        if (++time > 100) //等待100us后超时
        {
            return 0; //超时，返回0
        }
        DWT_Delay_us(1);
    }

    // 根据高电平持续时间判断数据位
    // 如果高电平持续时间大于30μs，则为数据位1，否则为数据位0
    // 【可调】标准阈值30μs，适用于大多数DHT11
    // 数据位0约27μs，数据位1约70μs，选择中间值30μs作为判断阈值
    return time > 30;
    
}

bool DHT11_Read(int *humidity, int *temperature)
{
    uint8_t data[5] = { 0 }; //存储接收到的5字节数据

    // ========== 第1步：主机发送开始信号 ==========
    DHT11_Start();

    // ========== 第2步：等待DHT11响应 ============
    DHT11_CheckResponse();

    // ========== 第3步：读取40位数据 =============
    // DHT11发送5字节数据，每字节8位，共40位
    for(int i = 0; i < 5; i++)
    {
        for(int j = 7 ; j > =0; j--)
        {
            //读取每一位数据，并存储到对应字节的对应位
            if(DHT11_ReadBit())
            {
                data[i] |= (1 << j); //读取到1，设置对应位为1
            }
            //否则对应位保持为0
        }
    }

    // ========== 第4步：数据校验 =================
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if(checksum != data[4])
    {
        return false; //校验失败，返回错误
    }

    // ========== 第5步：解析数据 ==========
    *humidity = data[0];
    *temperature = data[2];

    return true; //成功
}
