#include "tp_event.h"

/**
 * @brief 上下文初始化
 */
void TP_DOWNInit(TP_DownCtx *ctx)
{
    if(!ctx) return;    //参数保护：ctx 为NULL 直接返回，避免空指针
    ctx->was_pressed = 0;   //初始状态设为“上一轮未按下”
}

/**
 * @brief 轮询检测触摸“按下瞬间”（DOWN 事件）
 *
 * @details
 * 目标：把“持续的触摸状态(pressed/not pressed)”转换成“只触发一次的按下事件”。
 *
 * 输入：
 *   - ctx：用于保存历史状态（上一轮是否按下 was_pressed）
 *   - out：用于输出 DOWN 事件坐标（可传 NULL 表示不需要坐标）
 *
 * 依赖：
 *   - FT6336_ReadTouch_Filtered(&t)
 *       返回 1：当前检测到按下且坐标有效（t.x/t.y 可用）
 *       返回 0：当前无触摸 或 本次数据无效
 *
 * 输出/返回值：
 *   - 返回 1：本次检测到 DOWN（从“没按下”->“按下”的跳变）
 *   - 返回 0：本次没有 DOWN（要么一直按着，要么没按下）
 *
 * 行为特性：
 *   - 手指第一次落下：触发一次（返回1）
 *   - 手指一直按住不动：不会重复触发（一直返回0）
 *   - 松手后再按：会再次触发一次（返回1）
 */
uint8_t TP_PollDown(TP_DownCtx *ctx, TP_DownEvent *out)
{
    if (!ctx) return 0;                          // 保护：ctx 为空，无法保存历史状态，直接认为无事件

    FT6336_Touch_t t;                            // 临时变量：用于接收触摸坐标数据（本轮读取到的 x/y）
    uint8_t pressed = FT6336_ReadTouch_Filtered(&t); // 读取当前触摸状态：1=按下且坐标有效；0=没按下/无效

    // -------------------- 判定 DOWN：上一轮没按下 && 当前按下 --------------------
    // ctx->was_pressed 表示“上一轮”是否处于按下状态
    // pressed         表示“当前这一轮”是否处于按下状态
    // 只有从 0 -> 1 的跳变，才是“按下瞬间（DOWN）”
    if ((ctx->was_pressed == 0) && (pressed == 1))   // 条件：上一轮没按下，而这一轮按下
    {
        ctx->was_pressed = 1;                      // 更新历史：从现在开始，系统认为“已经处于按下状态”

        if (out)                                   // out 不为空才输出（允许用户只关心事件，不关心坐标）
        {
            out->x = t.x;                           // 输出 DOWN 时刻的 X 坐标
            out->y = t.y;                           // 输出 DOWN 时刻的 Y 坐标
        }

        return 1;                                   // 明确告诉上层：本轮产生了 DOWN 事件
    }

    // -------------------- 状态更新：松手后允许下一次再次产生 DOWN --------------------
    // 如果当前没按下（pressed==0），说明手指已经离开屏幕或本轮无效
    // 那么我们把历史状态清零，表示“已经松手”
    // 这样下次再按下时，就能再次满足 0->1，产生新的 DOWN
    if (pressed == 0)                               // 条件：当前没按下
    {
        ctx->was_pressed = 0;                       // 更新历史：标记为“未按下”，为下一次 DOWN 做准备
    }

    return 0;                                       // 默认情况：本轮没有 DOWN 事件（可能一直按着，也可能一直没按）
}



// 你可以调这个参数：连续多少次“未按下”才算真的松手
// 轮询周期 10ms 时：
//   2 次 ≈ 20ms（灵敏）
//   3 次 ≈ 30ms（更稳）
#define TP_RELEASE_CONFIRM    2


void TP_UpInit(TP_UpCtx *ctx)
{
    if (!ctx) return;                 // 保护：空指针直接返回

    ctx->was_pressed = 0;             // 初始认为“未按下”
    ctx->release_cnt = 0;             // 松手确认计数清零
    ctx->last_x = 0;                  // 最近坐标清零（可选）
    ctx->last_y = 0;
}

/**
 * @brief 轮询检测触摸“松开瞬间”（UP 事件）
 *
 * @details
 * 目标：把“持续的触摸状态(pressed/not pressed)”转换成“只触发一次的松开事件”。
 *
 * 依赖：
 *   - FT6336_ReadTouch_Filtered(&t)
 *       返回 1：当前按下且坐标有效
 *       返回 0：当前无触摸或本次数据无效
 *
 * 关键点（松手确认）：
 *   - 某些情况下会偶发读到 pressed=0（丢帧/干扰）
 *   - 为避免误判松手：要求连续 TP_RELEASE_CONFIRM 次 pressed=0
 *     才真正触发 UP
 *
 * 输出/返回值：
 *   - 返回 1：本轮产生 UP（确认松手成立）
 *   - 返回 0：本轮无 UP
 *
 * UP 坐标说明：
 *   - 松手时已经没有新坐标，因此输出“最后一次有效坐标(last_x/last_y)”
 */
uint8_t TP_PollUp(TP_UpCtx *ctx, TP_UpEvent *out)
{
    if (!ctx) return 0;                               // 保护：ctx为空无法保存历史状态

    FT6336_Touch_t t;                                 // 临时变量：接收本轮读取的坐标
    uint8_t pressed = FT6336_ReadTouch_Filtered(&t);  // 读取当前状态：1=按下且坐标有效；0=未按下/无效

    if (pressed)                                      // 如果当前确实按下
    {
        ctx->was_pressed = 1;                          // 标记：处于按下状态
        ctx->release_cnt = 0;                          // 清除“松手确认计数”（因为还在按着）

        ctx->last_x = t.x;                             // 记录最近一次有效坐标
        ctx->last_y = t.y;

        return 0;                                      // 按着不算 UP，本轮无事件
    }

    // 走到这里，说明 pressed == 0（当前读到未按下或无效）
    // 如果之前从未按下过（was_pressed==0），那只是一直没按，不会产生 UP
    if (ctx->was_pressed == 0)                         // 之前就没按下
    {
        ctx->release_cnt = 0;                          // 保守：计数清零
        return 0;                                      // 无 UP
    }

    // 走到这里：之前是按下状态，现在读到未按下 —— 可能是真松手，也可能是丢帧
    ctx->release_cnt++;                                // 累计连续“未按下”次数

    if (ctx->release_cnt < TP_RELEASE_CONFIRM)         // 未达到确认次数：先不触发 UP
    {
        return 0;                                      // 暂不认为松手
    }

    // 达到确认次数：正式判定 UP
    ctx->was_pressed = 0;                              // 状态回到未按下
    ctx->release_cnt = 0;                              // 计数清零，为下一次触摸准备

    if (out)                                           // 如果需要输出 UP 信息
    {
        out->x = ctx->last_x;                          // 输出最后一次有效坐标
        out->y = ctx->last_y;
    }

    return 1;                                          // 本轮产生 UP 事件
}
