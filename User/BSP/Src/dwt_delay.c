#include "dwt_delay.h"

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